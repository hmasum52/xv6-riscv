// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct{
  int cnt[PHYSTOP/PGSIZE];
  struct spinlock lock;
} phy_addr_ref;

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&phy_addr_ref.lock, "phy_addr_ref");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    // in kfree, 1 will be subtracted from cnt[]
    // here it must be set to 1 first, otherwise it will be
    // reduced to a negative number
    phy_addr_ref.cnt[(uint64)p / PGSIZE] = 1;
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&phy_addr_ref.lock);
  phy_addr_ref.cnt[(uint64)pa / PGSIZE]--;
  if(phy_addr_ref.cnt[(uint64)pa / PGSIZE] > 0){
    release(&phy_addr_ref.lock);
    return;
  }
  release(&phy_addr_ref.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    acquire(&phy_addr_ref.lock);
    phy_addr_ref.cnt[(uint64)r / PGSIZE] = 1;
    release(&phy_addr_ref.lock);
  }
  release(&kmem.lock);

 /*  acquire(&phy_addr_ref.lock);
  if(r)
    phy_addr_ref.cnt[(uint64)r / PGSIZE] = 1;
  release(&phy_addr_ref.lock); */

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int k_increase_ref_cnt(uint64 pa){
  //if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
  if ((pa % PGSIZE) != 0 || (char *)pa < end || pa >= PHYSTOP)
    return -1;
  acquire(&phy_addr_ref.lock);
  ++phy_addr_ref.cnt[pa / PGSIZE];
  release(&phy_addr_ref.lock);
  return 0;
}

void *alloc_cow_page(pagetable_t pagetable, uint64 va)
{
  if (va % PGSIZE != 0)
    // panic("alloc_cow_page: va is not page aligned");
    return 0; // error

  pte_t *pte;
  if ((pte = walk(pagetable, va, 0)) == 0)
    // panic("alloc_cow_page: pte should exist");
    return 0; // error

  uint64 pa = PTE2PA(*pte);

  if ( phy_addr_ref.cnt[pa/PGSIZE] == 1)
  {
    *pte |= PTE_W;
    *pte &= ~PTE_COW;
    // only one precess use this page
    // no need to copy
    return (void *)pa;
  }

  char *mem;
  if ((mem = kalloc()) == 0)
    // panic("alloc_cow_page: out of memory");
    return 0; // error

  // copy the content of old page to new page
  memmove(mem, (char *)pa, PGSIZE);

  // clear PTE_V to avoid remap panic in mappages function
  *pte &= ~PTE_V;

  int flags = PTE_FLAGS(*pte);
  flags |= PTE_W;    // enable write for new page
  flags &= ~PTE_COW; // disable COW for new page

  // map new page
  if (mappages(pagetable, va, PGSIZE, (uint64)mem, flags) != 0)
  {
    kfree(mem);    // free the new page
    *pte |= PTE_V; // restore the old page
    return 0;      // error
  }

  // decrease ref_cnt of old page
  // will be freed if ref_cnt == 0
  kfree((char *)PGROUNDDOWN(pa));
  return mem;
}

int 
k_pages_in_freelist(void){
  int cnt = 0;
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    cnt++;
    r = r->next;
  }
  release(&kmem.lock);
  return cnt;
}
