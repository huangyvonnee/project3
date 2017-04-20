#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

//anika driving
//global lock for the access of any files in
//syscalls and used in process.c
struct lock filesys_lock;


#endif /* userprog/syscall.h */
