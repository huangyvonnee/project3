#include "vm/page.h"
#include "threads/thread.h"
#include <list.h>
#include "threads/palloc.h"
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <synch.h>

#define MAX_FRAMES 367

struct frame* frames[MAX_FRAMES];
struct lock ft_lock;

enum frame_status
  {
    FRAME_USED,					/* Frame is in use. */
    FRAME_FREE,					/* Frame is not in use. */
  };

	
struct frame {
	// struct list_elem elem;		 To put in the list 
	uint8_t *upage;       	/* Pointer to page */
	tid_t owner;              	/* Thread owner */
	enum frame_status status; 	/* Status of the frame */
};

void frametable_init(void);
void *falloc_get_frame();
void falloc_free_frame(void *page);