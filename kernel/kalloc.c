// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem{
  struct spinlock lock;
  struct run *freelist;
};

struct kmem kms[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    initlock(&kms[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

int
Cpuid()
{
  int id;
  push_off();
  id = cpuid();
  pop_off();
  return id;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int cpuid = Cpuid();

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kms[cpuid].lock);
  r->next = kms[cpuid].freelist;
  kms[cpuid].freelist = r;
  release(&kms[cpuid].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int cpuid = Cpuid();

  acquire(&kms[cpuid].lock);
  r = kms[cpuid].freelist;
  if(r) {
    kms[cpuid].freelist = r->next;
  }
  else {
    /* search across every cpu's freelist to find a free page */
    for (int i = 0; i < NCPU; i++) {
      if (kms[i].freelist) {
        acquire(&kms[i].lock);
        r = kms[i].freelist;
        kms[i].freelist = r->next;
        r->next = (struct run*)0;
        release(&kms[i].lock);
        break;
      }
    }
  }
  release(&kms[cpuid].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
