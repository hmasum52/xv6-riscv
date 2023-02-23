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

// use a fixed size array where you can index 
// by the page's physical address divided by page size
int page_ref_count[PHYSTOP / PGSIZE];

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
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  page_ref_count[(uint64)r / PGSIZE] = 1;

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// increase the reference count of the page
void
kincrease_ref(uint64 pa)
{
  // panic if the page is not allocated
  if(page_ref_count[(uint64)pa / PGSIZE] == 0)
    panic("kincreaseref");
  acquire(&kmem.lock);
  page_ref_count[(uint64)pa / PGSIZE]++;
  release(&kmem.lock);
}

// decrease the reference count of the page
void
kdecrease_ref(uint64 pa)
{
  // panic if the page is not allocated
  if(page_ref_count[(uint64)pa / PGSIZE] == 0)
    panic("kdecreaseref");
  acquire(&kmem.lock);
  page_ref_count[(uint64)pa / PGSIZE]--;
  release(&kmem.lock);
  // garbage collect the page if the reference count is 0
  if(page_ref_count[(uint64)pa / PGSIZE] == 0)
    kfree((void*)pa);
}

// print number of pages in freelist
uint64
kpages_in_freelist(void){
  struct run *r;
  int count = 0;
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    count++;
    r = r->next;
  }
  release(&kmem.lock);
  return count;
}
