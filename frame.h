#include "vm/page.h"
#include "threads/thread.h"
#include <list.h>
#include "threads/palloc.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <synch.h>
//Yvonne driving now
//Found through running the tests - the number of frames allocated in the
//user pool was always 367. We could get this number ourselves by looping
//palloc_get_page(PAL_USER) as much as possible and counting that number,
//but we did not know how to initialize an array with that value given the
//fact that we get that value in our frametable_init() method.
#define MAX_FRAMES 367

//Array of pointers to frames that functions as our frametable. We decided
//to use an array as it gives us the simplest implementation, albeit being
//fairly inefficient.
struct frame* frames[MAX_FRAMES];

//Lock for synchronization purposes.
struct lock ft_lock;

//Tells us the status of the frame - whether it is being used or not.
enum frame_status
  {
    FRAME_USED,					/* Frame is in use. */
    FRAME_FREE,					/* Frame is not in use. */
  };

	
//Our actual frame struct - has a pointer to the page it would contain,
//has the TID of the thread that owns it, and it's status.
struct frame {
	uint8_t *upage;       	/* Pointer to page */
	tid_t owner;              	/* Thread owner */
	enum frame_status status; 	/* Status of the frame */
};
// Pearl driving now
void frametable_init(void);
void *falloc_get_frame();
void falloc_free_frame(void *page);