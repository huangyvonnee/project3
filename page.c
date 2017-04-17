#include "vm/page.h"


/* Returns a hash value for page p. */
// Taken from PintOS documentation - A.8.5
unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  const struct page *p = hash_entry (p_, struct page, hash_elem);
  return hash_bytes (&p->addr, sizeof p->addr);
}

/* Returns true if page a precedes page b. */
// Taken from PintOS documentation - A.8.5
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
  struct page *a = hash_entry (a_, struct page, hash_elem);
  struct page *b = hash_entry (b_, struct page, hash_elem);

  return a->addr < b->addr;
}
 	
/* Returns the page containing the given virtual address,
   or a null pointer if no such page exists. */
// Taken from PintOS documentation - A.8.5
struct page *
page_lookup (void *address)
{
  struct page *p;
  struct hash_elem *e;

  p->addr = address;
  e = hash_find (&thread_current()->SPT, &p->hash_elem);
  return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
} 

//Initializes our hashmap that functions as our supplemental page table.
//We decided on a hashmap because of it's efficiency in storing data.
void
SPT_init(struct hash *SPT) 
{
  hash_init(SPT, page_hash, page_less, NULL);
}

//Initializes an entry for the supplemental page table. Called in
//load_segment(). Initializes all the instance fields in the page
//struct with the information in load_segment(). Also inserts the
//struct into the hashmap.
bool 
page_init (enum page_location page_loc, struct file *file, uint32_t ofs, 
            uint8_t *upage, uint32_t read_bytes, uint32_t zero_bytes, 
            bool writable)
{
  struct page *p;
  p->addr = upage;
  p->f = file;
  p->offset = ofs;
  p->pg_read_bytes = read_bytes;
  p->pg_zero_bytes = zero_bytes;
  p->writable = writable;
  p->pageloc = page_loc;

  if(hash_insert(&thread_current()->SPT, &p->hash_elem))
    return false;
  // need to do error checki-ng
  return true;
}
