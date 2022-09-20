#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <mips/trapframe.h>
#include <current.h>
#include <synch.h>
#include <kern/wait.h>

/*
 * system calls for process management
 */
int 
sys_read(int fd, userptr_t buf_ptr, size_t size) {
  int i;
  char *p = (char *)buf_ptr;

  if (fd!=STDIN_FILENO) {
    kprintf("sys_read supported only to stdin\n");
    return -1;
  }

  for (i=0; i<(int)size; i++) {
    p[i] = getch();
    if (p[i] < 0) 
      return i;
  }

  return (int)size;
}


int 
sys_write(int fd, userptr_t buf_ptr, int size) {
  int i;
  char *p = (char *)buf_ptr;

  if (fd!=STDOUT_FILENO && fd!=STDERR_FILENO) {
    kprintf("sys_write supported only to stdout\n");
    return -1;
  }

  for (i=0; i<(int)size; i++) {
    putch(p[i]);
  }

  return (int)size;
}


void sys__exit(int status){   
   struct addrspace *as;
   struct proc *p = curproc;
   /* for now, just include this to keep the compiler from complaining about
      an unused variable */
   (void)status;
 
   DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",status);

 
   KASSERT(curproc->p_addrspace != NULL);
   as_deactivate();
   /*
    * clear p_addrspace before calling as_destroy. Otherwise if
    * as_destroy sleeps (which is quite possible) when we
    * come back we'll be calling as_activate on a
    * half-destroyed address space. This tends to be
    * messily fatal.
    */
   as = curproc_setas(NULL);
   as_destroy(as);
 
   /* detach this thread from its process */
   /* note: curproc cannot be used after this call */
   proc_remthread(curthread);
 
   /* if this is the last user process in the system, proc_destroy()
      will wake up the kernel menu thread */
   proc_destroy(p);
   
   thread_exit();
   /* thread_exit() does not return, so we should never get here */
   panic("return from thread_exit in sys_exit\n");
  
}

