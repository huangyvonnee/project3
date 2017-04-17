#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <list.h>
#include "threads/thread.h"
#include <string.h>

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

void *
falloc_get_frame() {
	int i = 0;
	struct frame *fram = frames[i];
	lock_acquire(&ft_lock);
	while(fram->status != FRAME_FREE) {
		i++;
		fram = frames[i];
	}
	fram->owner = thread_tid();
	fram->status = FRAME_USED;
	lock_release(&ft_lock);
	return fram;
}

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
			lock_release(&ft_lock);
			return;
		}
	}
	lock_release(&ft_lock);
}