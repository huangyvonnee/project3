#include "lib/kernel/hash.h"
#include "threads/thread.h"
#include <inttypes.h>

enum page_location {
	IN_FRAME, 					/* Page is in frame. */
	IN_SWAP,					/* Page is in swap. */
	ON_DISK,					/* Page is on disk and not 
								   yet loaded. */
};

struct page
  {
  	enum page_location pageloc;	/* Location of the page. */
    struct hash_elem hash_elem; /* Hash table element. */
    void *addr;                 /* Virtual address. */
  	struct file *f;				/* File struct */
  	uint32_t offset;			/* Offset */
  	uint32_t pg_read_bytes;		/* Number of bytes to read */
  	uint32_t pg_zero_bytes;		/* Number of bytes to initialize to 0 */
  	bool writable;				/* Should file be writable or not*/
  };

unsigned page_hash (const struct hash_elem *p_, void *aux);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED);
struct page *page_lookup (void *address);
void SPT_init(struct hash *spt);
bool page_init (enum page_location page_loc, struct file *file, uint32_t ofs, 
				uint8_t *upage, uint32_t read_bytes, uint32_t zero_bytes, 
				bool writable);