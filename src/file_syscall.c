/*
 * File-related system call implementations.
 * New for SOL2.
 */

#include <types.h>
#include <clock.h>
/* #include <kern/errno.h>
#include <kern/limits.h>
#include <kern/stat.h> */
#include <kern/unistd.h>
/* #include <kern/fcntl.h>
#include <kern/seek.h> */
#include <lib.h>
/*#include <synch.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h> */
#include <syscall.h>
#include <copyinout.h>


/*
 * sys_open
 * just copies in the filename, then passes work to file_open.
 */
/* int
sys_open(userptr_t filename, int flags, int mode, int *retval)
{
	char fname[PATH_MAX];
	int result;

	result = copyinstr(filename, fname, sizeof(fname), NULL);
	if (result) {
		return result;
	}

	return file_open(fname, flags, mode, retval);
}

 */


/////////////////////////////////////////////////////////////////////////
/*
 * sys_read
 * translates the fd into its openfile, then calls VOP_READ.
 */
/* int
sys_read(int fd, userptr_t buf, size_t size, int *retval)
{
  struct iovec iov;
	struct uio useruio;
	struct openfile *file;
	int result;
*/
	/* better be a valid file descriptor */
/*	result = filetable_findfile(fd, &file);
	if (result) {
		return result;
	}

	lock_acquire(file->of_lock);

	if (file->of_accmode == O_WRONLY) {
		lock_release(file->of_lock);
		return EBADF;
	}
*/
	/* set up a uio with the buffer, its size, and the current offset */
//    uio_uinit(&iov, &useruio, buf, size, file->of_offset, UIO_READ);
  
  // mk_useruio(&useruio, buf, size, file->of_offset, UIO_READ);

	/* does the read */
/*	result = VOP_READ(file->of_vnode, &useruio);
	if (result) {
		lock_release(file->of_lock);
		return result;
	}
*/
	/* set the offset to the updated offset in the uio */
/*	file->of_offset = useruio.uio_offset;

	lock_release(file->of_lock);
*/	
	/*
	 * The amount read is the size of the buffer originally, minus
	 * how much is left in it.
	 */
/*	*retval = size - useruio.uio_resid;

	return 0;
} */

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


/////////////////////////////////////////////////////////////////////////
/*
 * sys_write
 * translates the fd into its openfile, then calls VOP_WRITE.
 */
/* int
sys_write(int fd, userptr_t buf, size_t size, int *retval)
{
  struct iovec iov;
	struct uio useruio;
	struct openfile *file;
	int result;

	result = filetable_findfile(fd, &file);
	if (result) {
		return result;
	}

	lock_acquire(file->of_lock);

	if (file->of_accmode == O_RDONLY) {
		lock_release(file->of_lock);
		return EBADF;
	}
*/
	/* set up a uio with the buffer, its size, and the current offset */
//    uio_uinit(&iov, &useruio, buf, size, file->of_offset, UIO_WRITE);
	
  //mk_useruio(&useruio, buf, size, file->of_offset, UIO_WRITE);

	/* does the write */
/*	result = VOP_WRITE(file->of_vnode, &useruio);
	if (result) {
		lock_release(file->of_lock);
		return result;
	}
*/
	/* set the offset to the updated offset in the uio */
/*	file->of_offset = useruio.uio_offset;

	lock_release(file->of_lock);
*/
	/*
	 * the amount written is the size of the buffer originally,
	 * minus how much is left in it.
	 */
/*	*retval = size - useruio.uio_resid;

	return 0;
} */


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






/////////////////////////////////////////////////////////////////////////





/* 
 * sys_close
 * just pass off the work to file_close.
 */
/* int
sys_close(int fd)
{
	return file_close(fd);
}
 */

/////////////////////////////////////////////////////////////////////////
/* int
file_open(char *filename, int flags, int mode, int *retfd)
{
	struct vnode *vn;
	struct openfile *file;
	int result;
	
	result = vfs_open(filename, flags, mode, &vn);
	if (result) {
		return result;
	}

	file = kmalloc(sizeof(struct openfile));
	if (file == NULL) {
		vfs_close(vn);
		return ENOMEM;
	}
 */
	/* initialize the file struct */
/* 	file->of_lock = lock_create("file lock");
	if (file->of_lock == NULL) {
		vfs_close(vn);
		kfree(file);
		return ENOMEM;
	}
	file->of_vnode = vn;
	file->of_offset = 0;
	file->of_accmode = flags & O_ACCMODE;
	file->of_refcount = 1;
 */
	/* vfs_open checks for invalid access modes */
/* 	KASSERT(file->of_accmode==O_RDONLY ||
	        file->of_accmode==O_WRONLY ||
	        file->of_accmode==O_RDWR);
 */
	/* place the file in the filetable, getting the file descriptor */
/* 	result = filetable_placefile(file, retfd);
	if (result) {
		lock_destroy(file->of_lock);
		kfree(file);
		vfs_close(vn);
		return result;
	}

	return 0;
} */
//////////////////////////////////////////////////////////////////////