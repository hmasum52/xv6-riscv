#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
// my system calls
extern uint64 sys_trace(void);
extern uint64 sys_sysinfo(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static struct{
  uint64 (*func)(void); // pointer to system function
  char* name;
} syscalls[] = {
    [SYS_fork]    {sys_fork, "fork"},
    [SYS_exit]    {sys_exit ,"exit"},
    [SYS_wait]    {sys_wait, "wait"},
    [SYS_pipe]    {sys_pipe, "pipe"},
    [SYS_read]    {sys_read, "read"},
    [SYS_kill]    {sys_kill, "kill"},
    [SYS_exec]    {sys_exec, "exec"},
    [SYS_fstat]   {sys_fstat, "fstat"},
    [SYS_chdir]   {sys_chdir, "chdir"},
    [SYS_dup]     {sys_dup,"dup"},
    [SYS_getpid]  {sys_getpid, "getpid"},
    [SYS_sbrk]    {sys_sbrk, "sbrk"},
    [SYS_sleep]   {sys_sleep, "sleep"},
    [SYS_uptime]  {sys_uptime, "uptime"},
    [SYS_open]    {sys_open, "open"},
    [SYS_write]   {sys_write, "write"},
    [SYS_mknod]   {sys_mknod, "mknod"},
    [SYS_unlink]  {sys_unlink, "unlink"},
    [SYS_link]    {sys_link ,"link"},
    [SYS_mkdir]   {sys_mkdir, "mkdir"},
    [SYS_close]   {sys_close, "close"},
    // my system calls
    [SYS_trace]   {sys_trace, "trace"},
    [SYS_sysinfo] {sys_sysinfo, "sysinfo"},
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num].func) {
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    p->trapframe->a0 = syscalls[num].func();
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }

  // trace
  // check if the current system call's number is the same
  // as the argument pass in the trace system
  if (p->sys_call_number == num)
  {
    // print process id, the name of the system call and
    // the return value of the current system call
    printf("pid: %d, syscall: %s, return value: %d\n", p->pid, syscalls[num].name, p->trapframe->a0);
  }
}
