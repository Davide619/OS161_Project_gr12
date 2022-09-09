#include <types.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <current.h>
#include <syscall.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <addrspace.h>
#include <copyinout.h>


int sys__exit(int status) {
  curproc->exit_status = status;
  as_destroy(curproc->p_addrspace);
  thread_exit();
}
