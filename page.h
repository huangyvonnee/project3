#include "lib/kernel/hash.h"
#include "threads/thread.h"
#include <inttypes.h>
//Anika driving now
//Our supplemental page table entries need to store their location.
enum page_location {
	IN_FRAME, 					/* Page is in frame. */
	IN_SWAP,					/* Page is in swap. */
	ON_DISK,					/* Page is on disk and not 
								   yet loaded. */
};

//The struct we used for our supplemental page table entry. Stores it's
//location, a hash_elem (because our supplemental page table is a hash map),
//the address of the actual page, the file it reads from, it's offset,
//how many bytes to read and how many bytes to zero, as well as a boolean
//that tells us if it's writable.
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
//Pearl driving now
unsigned page_hash (const struct hash_elem *p_, void *aux);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED);
struct page *page_lookup (void *address);
void SPT_init(struct hash *spt);
bool page_init (enum page_location page_loc, struct file *file, uint32_t ofs, 
				uint8_t *upage, uint32_t read_bytes, uint32_t zero_bytes, 
				bool writable);