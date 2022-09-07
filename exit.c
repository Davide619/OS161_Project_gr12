
#include <types.h>
#include <lib.h>
#include <proc.h>
#include <file.h>
#include <filedesc.h>
#include <thread.h>
#include <current.h>
#include <syscall.h>


int sys__exit(int status) {
  curproc->exit_status = status;
  as_destroy(curproc->p_addrspace);
  thread_exit();
}
