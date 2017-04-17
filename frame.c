#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <list.h>
#include "threads/thread.h"
#include <string.h>
//Mohammad driving now
//Initializes the frametable, which is an array of pointers to frame structs.
//We decided to palloc and create all the frames at the start. This makes
//getting a frame easier and have less overhead.
void
frametable_init() {
	lock_init(&ft_lock);
	int numPages = 0;
	void *kpage = palloc_get_page(PAL_USER);
	while(numPages < MAX_FRAMES) {
		if(kpage == NULL) {
			printf("Page Allocation Error\n");
			exit(-1);
		}
		struct frame *fram;
		fram->upage = kpage;
		fram->status = FRAME_FREE;
		frames[numPages] = fram;
		numPages++;
		kpage = palloc_get_page(PAL_USER);
	}
}

//Gives the user program a frame. Goes through the frametable array and
//searches for the first frame that is not used. Is slow because linear
//search but is the easiest way to do it. If no frame is free, follow
//eviction protocol and swap things. Did not get around to it, but it
//would have been ___
void *
falloc_get_frame() {
	int i = 0;
	struct frame *fram = frames[i];
	lock_acquire(&ft_lock);
	while(fram->status != FRAME_FREE && i < MAX_FRAMES) {
		i++;
		fram = frames[i];
	}
	if(i < MAX_FRAMES) {
		fram->owner = thread_tid();
		fram->status = FRAME_USED;
		lock_release(&ft_lock);
		return fram;
	}
	else {
		//swap things
		lock_release(&ft_lock);
		return NULL;
	}
}
//Anika driving now
//Frees a frame that was previously in use. Given a pointer to a page,
//we go through the frametable and look for a for a frame that points
//to the page we were given. If we find it, free the page and reset
//the frame.
void
falloc_free_frame(void *page) {
	int i;
	lock_acquire(&ft_lock);
	for(i = 0; i < MAX_FRAMES; i++) {
		struct frame *fram = frames[i];
		if(fram->upage == page) {
			fram->owner = 0;
			fram->status = FRAME_FREE;
			palloc_free_page(page);
			fram->upage = palloc_get_page(PAL_USER);
			lock_release(&ft_lock);
			return;
		}
	}
	lock_release(&ft_lock);
}