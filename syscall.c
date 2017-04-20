#include  "userprog/syscall.h"
#include  <string.h>
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "lib/user/syscall.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/process.h"
#include <assert.h>
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/directory.h"
#include "vm/frame.h"

static void syscall_handler (struct intr_frame *);
//anika driving
//method to check validity of pointers given by user 
void check_valid(uint32_t *pointer);

void
syscall_init (void) 
{ 
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, 	syscall_handler, "syscall");
}

//latrell driving
//switch case handles each syscall
//checks the validity of each pointer, which
//it gets by moving the copy of esp, which is set 
//to an int pointer to make moving it up easier
static void syscall_handler (struct intr_frame *f) {
  int result;
  bool ret;
  pid_t temp_pid;
  uint32_t  *my_esp = (uint32_t *) f->esp;

  check_valid(my_esp);

  switch(*my_esp)
    {
    //call the appropriate system call 
    //based on the copy of f->esp
	//and sets the intr_frame's return register 
    //equal to the system call's return value
	case(SYS_HALT):
		//halt process
  		halt();
		break;
	case(SYS_EXIT):
		//call exit with the second argument 
	    //on the stack pointer 
		check_valid((my_esp + 1));
		int exit_status = (int) *(my_esp + 1);
		exit((int) *(my_esp + 1));
		f->eax = (int) exit_status;		
		break;
	case(SYS_EXEC):
		//executes a new process
		check_valid((my_esp + 1));
		check_valid(*(my_esp +1));
		temp_pid = exec((const char *) *(my_esp + 1));
		f->eax = (pid_t) temp_pid;
        break;
	case(SYS_WAIT):
		//waits on the process with the 
	    //specific pid_t to die
		//pid_t should 1st arg on stack
		check_valid((my_esp + 1));
		temp_pid = wait((pid_t) *(my_esp + 1));
		f->eax = (int) temp_pid;
		break;
	case(SYS_CREATE):
		//creates a new file based on the filename 
	    //and size of the file
		//given as first and second arguments
		check_valid((my_esp + 1));
		check_valid(*(my_esp +1));
		check_valid((my_esp + 2));
        ret = create((const char *)  *(my_esp + 1) , 
        	(unsigned) *(my_esp + 2));
        f->eax = (bool) ret;
		break;
	case(SYS_REMOVE):
		//removes a file from the filesys, file name 
	    //given as first argument on stack
		check_valid((my_esp + 1));
		check_valid(*(my_esp +1));
		ret = remove((const char *) *(my_esp + 1));
		f->eax = (bool) ret;
		break;
	case(SYS_OPEN):
		//opens a file, based on the filename 
	    //(1st arg on stack)
		check_valid((my_esp + 1));
		check_valid(*(my_esp +1));
		result = open((const char *) *(my_esp + 1));
		f->eax = (int) result;
		break;
	case(SYS_FILESIZE):
		//simply returns size of file, file descriptor 
	    //is the first arg on the stack
		check_valid((my_esp + 1));
		result = filesize((int)*(my_esp + 1));
		f->eax = (int) result; 
		break;
	case(SYS_READ):
		// read the specified file, fd given as 
	    //first arg on stack
		//buffer is second arg on, file size 
	    //is the third
		check_valid((my_esp + 1));
		check_valid((my_esp + 2));
		check_valid((my_esp + 3));
		result = read((int) *(my_esp + 1),(void *) 
			*(my_esp + 2), (unsigned) *(my_esp + 3));
		f->eax = (int) result;
		break;
	case(SYS_WRITE):
		//writes to the file specified by the 
	    //fd, buffer, and size
		//all args given on stack 
		check_valid((my_esp + 1));
		check_valid((my_esp + 2));
		check_valid(*(my_esp + 2));
		check_valid((my_esp + 3));
		result = write((int) *(my_esp + 1),(void *) 
			*(my_esp + 2), (unsigned) *(my_esp + 3));
		f->eax = (int) result;
		break;
	case(SYS_SEEK):
		//close's specified file, fd on stack
		check_valid((my_esp + 1));
		check_valid((my_esp + 2));
		seek((int)*(my_esp + 1), (unsigned) 
			*(my_esp + 2));
		break;
	case(SYS_TELL):
		//returns position of next byte to read
		check_valid((my_esp +1));
		result = tell((int)*(my_esp + 1));;
		f->eax = (unsigned) result; 
		break;
	case(SYS_CLOSE):
		//close's file, given by fd on stack 
		check_valid((my_esp +1));
		close((int)*(my_esp + 1));
		break;
  
  }
}

//anika driving
//function to check if the pointers are 
//valid (!null, !vaddr, pagedir !=NULL) and exit(-1) if not
void check_valid(uint32_t *pointer) {
	if (pointer == NULL)
		exit(-1);
	if (!is_user_vaddr(pointer))
		exit(-1);
	if (pagedir_get_page(thread_current()->pagedir, 
		pointer) == NULL)
		exit(-1);
}	

//latrell driving
//correlates with wait
//child has to tell parent it is done and parent
//has to tell it it's okay to exit
//requires synchronization because of the 
//access to file that cannot be corrupte
void
exit(int status)
{
	//print the exit status of the thread
	printf("%s: exit(%d)\n", thread_name(), status);
	//parse the current thread's child threads to release
	//all the parent's (current thread) children by telling
	//viable status sema to stop waiting
	struct list_elem *e;
	for (e = list_begin (&(thread_current()->child_threads)); e != list_end 
	 (&(thread_current()->child_threads)); e = list_next (e)) {
	      struct thread *t = list_entry(e, struct thread, child_elem);
	      sema_up(&(t->viable_status));
	}
	lock_acquire(&filesys_lock);
	//close the executable file pointer associated with the 
	//the current thread
	file_close(thread_current()->executable);
	int i;
	//parse the current thread's array of open files
	//and close all of them
	for (i = 2; i < 128; i++) {
	  if (thread_current()->files[i] != NULL)
	   file_close(thread_current()->files[i]);

	 }
	lock_release(&filesys_lock);
	//set the thread's exit status to the status passed in
	thread_current()->exit_status = status;
	//tell thread (parent) to stop waiting on child
	sema_up(&(thread_current()->parent_wait));
	//parent waiting on child's viable status
	sema_down(&(thread_current()->viable_status));
	//exit thread
	thread_exit();
}

void 
halt(){
    shutdown_power_off();
}

//anika driving
//if the length of the command line is 0
//there is no file name to execute, exit(-1)
//execute the process
//tell the current thread (parent) to start waiting
//for the successfully loaded file
//if the load status of the thread is true, return 
//the pid of the process, otherwise return -1
pid_t 
exec(const char *cmd_line) {
	if (strlen(cmd_line) == 0)
			exit(-1);
   pid_t pid = process_execute(cmd_line);
   sema_down(&(thread_current()->exec_sema));
   if (thread_current()->load_status)
   		return pid;
   return -1;
}

//latrell driving
//process waits for child to finish
//returns the pid if successful
//all of this is done in process_wait()
int
wait(pid_t pid) {
    return process_wait(pid);
}

//anika driving
//as long as the file is not initially empty
//and has a valid name, create the file, and return
//whether it was successfully created
//synchronize around the creation of the file
bool 
create (const char *file, unsigned initial_size) {
	bool ret = false;
    if((off_t) initial_size >= 0 && strlen(file)){
	lock_acquire(&filesys_lock);
		ret = filesys_create(file, (off_t) initial_size);
	lock_release(&filesys_lock);
     }
        return ret;
}

//anika driving
//return whether removal of the file was successful
//and synchronize around the removal
//requires synchronization because of the 
//access to file that cannot be corrupte
bool
remove (const char *file) {
	lock_acquire(&filesys_lock);
		bool ret = filesys_remove(file);
	lock_release(&filesys_lock);
	return ret;
}

//anika and latrell worked on this
//as long as the file passed in is not null,
//open the file, check to make sure that was
//successful 
//if the file is null, release the lock
//and return -1
//otherwise, put the thread in the next available
//spot in the array of open files the thread holds
//requires synchronization because of the 
//access to file that cannot be corrupte
int 
open (const char *file) {
	lock_acquire(&filesys_lock);
	struct file *f;
	if (file != NULL) {
		f = filesys_open(file);
	
		if (f == NULL) {
			lock_release(&filesys_lock);
			return -1;
		}
                struct thread *t = thread_current();
                int i;
                for (i = 2; i < 128; i++){
                	if (t->files[i] == NULL) {
                		t->files[i] = f;
                		lock_release(&filesys_lock);
                		return i;
                    }
                 }
    }
					lock_release(&filesys_lock);
	return -1;
}

//anika driving
//get the size in bytes of the file at the 
//index passed in and return it
//requires synchronization because of the 
//access to file that cannot be corrupte
int
filesize (int fd) {
	if(fd < 0 || fd > 128) 
		return -1;
	lock_acquire(&filesys_lock);
		off_t ret = file_length(thread_current()->files[fd]);
	lock_release(&filesys_lock);
	return (int) ret;
}

//anika and latrell worked on this
//check validity of buffer just in case
//if fd is 0 or 1-- those are STDIN & STDOUT
//get thread at the index fd in the current 
//thread's array of open files
//if the file obtained is null, release lock
//and return -1, otherwise read the file and 
//and return the number of bytes read
//requires synchronization because of the 
//access to file that cannot be corrupte
int 
read (int fd, void *buffer, unsigned size) {
	if (fd < 0 || fd > 128)
		return -1;
	check_valid(buffer);
	if (fd == 0)
		input_getc();
	if (fd == 1) 
		return -1;
	lock_acquire(&filesys_lock);
		struct file *f = thread_current()->files[fd];
		if (f == NULL) {
			lock_release(&filesys_lock);
			return -1;
		}
		off_t ret = file_read(f, buffer, (off_t) size);
	lock_release(&filesys_lock);
	return (int) ret;
} 

//anika and latrell worked on this
//find thread at the index given in the thread's 
//array of open files
//if that file is null, release the lock and exit(-1)
//otherwise write the file and return the size of it
//requires synchronization because of the 
//access to file that cannot be corrupte
int 
write (int fd, const void *buffer, unsigned size) {
	if(fd < 0 || fd > 128) 
		return -1;
	if(fd == 1){
		putbuf(buffer, size);
	} else {
	lock_acquire(&filesys_lock);
		struct file *f = thread_current()->files[fd];
                if(f == NULL){
                	lock_release(&filesys_lock);
                    exit(-1);
                }
		size = (int) file_write(f, buffer, size);
	lock_release(&filesys_lock); }
	return size;
} 

//anika driving
//find file at the index given in the 
//current thread's array of open files
//and seek the file at the position given
//requires synchronization because of the 
//access to file that cannot be corrupted
void 
seek (int fd, unsigned position) {
	if (fd < 0 || fd > 128)
		exit(-1);
	lock_acquire(&filesys_lock);
		struct file *f = thread_current()->files[fd];
		file_seek(f, (off_t) position);
	lock_release(&filesys_lock);
}

//latrell driving
//find file at the index given in the 
//current thread's array of open files
//return size of tell
//requires synchronization because of the 
//access to file that cannot be corrupted
unsigned 
tell (int fd) {
	if(fd < 0 || fd > 128) 
		return -1;
	lock_acquire(&filesys_lock);
		struct file *f = thread_current()->files[fd];
		off_t ret = file_tell(f);
	lock_release(&filesys_lock);
	return (unsigned) ret;
} 	

//anika driving
//find file at the index given in the 
//current thread's array of open files
//close the file
//set the file pointer at that position
//to null because it is now empty
//if the file was not found, release the lock
//and return -1
//requires synchronization because of the 
//access to file that cannot be corrupted
void 
close (int fd) {
	if (fd < 0 || fd > 128)
		exit(-1);
	lock_acquire(&filesys_lock);
                if(thread_current()->files[fd] != NULL) {
					file_close(thread_current()->files[fd]);
					thread_current()->files[fd] = (struct file *) NULL;
				}
				else {
					lock_release(&filesys_lock);
					exit(-1);
				}
	lock_release(&filesys_lock);
}






