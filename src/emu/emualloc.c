/***************************************************************************

    emualloc.c

    Memory allocation helpers for the core emulator.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emucore.h"
#include "coreutil.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// align all allocated memory to this size
const int memory_align = 16;

// number of memory_entries to allocate in a block
const int memory_block_alloc_chunk = 256;



//**************************************************************************
//  MACROS
//**************************************************************************

// enable deletion
#undef delete



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// this struct is allocated in pools to track memory allocations
// it must be a POD type!!
class memory_entry
{
public:
	memory_entry *		m_next;				// link to the next entry
	memory_entry *		m_prev;				// link to the previous entry
	size_t				m_size;				// size of the allocation (not including this header)
	void *				m_base;				// base of the allocation
	const char *		m_file;				// file the allocation was made from
	int					m_line;				// line number within that file
	int					m_id;				// unique id

	static const int	k_hash_prime = 193;

	static int			s_curid;			// current ID
	static osd_lock *	s_lock;				// lock for managing the list
	static bool			s_lock_alloc;		// set to true temporarily during lock allocation
	static memory_entry *s_hash[k_hash_prime];// hash table based on pointer
	static memory_entry *s_freehead;		// pointer to the head of the free list

	static memory_entry *allocate(size_t size, void *base, const char *file, int line);
	static memory_entry *find(void *ptr);
	static void release(memory_entry *entry);
	static void report_unfreed();

private:
	static void acquire_lock();
	static void release_lock();
};



//**************************************************************************
//  GLOBALS
//**************************************************************************

// global resource pool to handle allocations outside of the emulator context
resource_pool global_resource_pool;

// dummy zeromem object
const zeromem_t zeromem = { };

// globals for memory_entry
int memory_entry::s_curid = 0;
osd_lock *memory_entry::s_lock = NULL;
bool memory_entry::s_lock_alloc = false;
memory_entry *memory_entry::s_hash[memory_entry::k_hash_prime] = { NULL };
memory_entry *memory_entry::s_freehead = NULL;



//**************************************************************************
//  GLOBAL HELPERS
//**************************************************************************

/*-------------------------------------------------
    malloc_file_line - allocate memory with file
    and line number information
-------------------------------------------------*/

void *malloc_file_line(size_t size, const char *file, int line)
{
	// allocate the memory and fail if we can't
	void *result = osd_malloc(size);
	if (result == NULL)
		return NULL;

#ifdef MAME_DEBUG
	// add a new entry
	memory_entry::allocate(size, result, file, line);

	// randomize the memory
	rand_memory(result, size);
#endif

	return result;
}


/*-------------------------------------------------
    free_file_line - free memory with file
    and line number information
-------------------------------------------------*/

void free_file_line(void *memory, const char *file, int line)
{
#ifdef MAME_DEBUG
	// find the memory entry
	memory_entry *entry = memory_entry::find(memory);

	// warn about untracked frees
	if (entry == NULL)
	{
		fprintf(stderr, "Error: attempt to free untracked memory in %s(%d)!\n", file, line);
		osd_break_into_debugger("Error: attempt to free untracked memory");
		return;
	}

	// free the entry and the memory
	if (entry != NULL)
		memory_entry::release(entry);
#endif

	osd_free(memory);
}


/*-------------------------------------------------
    dump_unfreed_mem - called from the exit path
    of any code that wants to check for unfreed
    memory
-------------------------------------------------*/

void dump_unfreed_mem(void)
{
#ifdef MAME_DEBUG
	memory_entry::report_unfreed();
#endif
}



//**************************************************************************
//  RESOURCE POOL
//**************************************************************************

/*-------------------------------------------------
    resource_pool - constructor for a new resource
    pool
-------------------------------------------------*/

resource_pool::resource_pool()
	: m_listlock(osd_lock_alloc()),
	  m_ordered_head(NULL)
{
	memset(m_hash, 0, sizeof(m_hash));
}


/*-------------------------------------------------
    ~resource_pool - destructor for a resource
    pool; make sure all tracked objects are
    deleted
-------------------------------------------------*/

resource_pool::~resource_pool()
{
	clear();
	if (m_listlock != NULL)
		osd_lock_free(m_listlock);
}


/*-------------------------------------------------
    add - add a new item to the resource pool
-------------------------------------------------*/

void resource_pool::add(resource_pool_item &item)
{
	osd_lock_acquire(m_listlock);

	// insert into hash table
	int hashval = reinterpret_cast<FPTR>(item.m_ptr) % k_hash_prime;
	item.m_next = m_hash[hashval];
	m_hash[hashval] = &item;

	// insert into ordered list
	item.m_ordered_next = m_ordered_head;
	if (m_ordered_head != NULL)
		m_ordered_head->m_ordered_prev = &item;
	m_ordered_head = &item;

	osd_lock_release(m_listlock);
}


/*-------------------------------------------------
    remove - remove a specific item from the
    resource pool
-------------------------------------------------*/

void resource_pool::remove(void *ptr)
{
	// ignore NULLs
	if (ptr == NULL)
		return;

	// search for the item
	osd_lock_acquire(m_listlock);

	int hashval = reinterpret_cast<FPTR>(ptr) % k_hash_prime;
	for (resource_pool_item **scanptr = &m_hash[hashval]; *scanptr != NULL; scanptr = &(*scanptr)->m_next)

		// must match the pointer
		if ((*scanptr)->m_ptr == ptr)
		{
			// remove from hash table
			resource_pool_item *deleteme = *scanptr;
			*scanptr = deleteme->m_next;

			// remove from ordered list
			if (deleteme->m_ordered_prev != NULL)
				deleteme->m_ordered_prev->m_ordered_next = deleteme->m_ordered_next;
			else
				m_ordered_head = deleteme->m_ordered_next;
			if (deleteme->m_ordered_next != NULL)
				deleteme->m_ordered_next->m_ordered_prev = deleteme->m_ordered_prev;

			// delete the object and break
			delete deleteme;
			break;
		}

	osd_lock_release(m_listlock);
}


/*-------------------------------------------------
    find - find a specific item in the resource
    pool
-------------------------------------------------*/

resource_pool_item *resource_pool::find(void *ptr)
{
	// search for the item
	osd_lock_acquire(m_listlock);

	int hashval = reinterpret_cast<FPTR>(ptr) % k_hash_prime;
	resource_pool_item *item;
	for (item = m_hash[hashval]; item != NULL; item = item->m_next)
		if (item->m_ptr == ptr)
			break;

	osd_lock_release(m_listlock);

	return item;
}


/*-------------------------------------------------
    contains - return true if given ptr is
    contained by one of the objects in the pool
-------------------------------------------------*/

bool resource_pool::contains(void *_ptrstart, void *_ptrend)
{
	UINT8 *ptrstart = reinterpret_cast<UINT8 *>(_ptrstart);
	UINT8 *ptrend = reinterpret_cast<UINT8 *>(_ptrend);

	// search for the item
	osd_lock_acquire(m_listlock);

	resource_pool_item *item = NULL;
	for (item = m_ordered_head; item != NULL; item = item->m_ordered_next)
	{
		UINT8 *objstart = reinterpret_cast<UINT8 *>(item->m_ptr);
		UINT8 *objend = objstart + item->m_size;
		if (ptrstart >= objstart && ptrend <= objend)
			goto found;
	}

found:
	osd_lock_release(m_listlock);

	return (item != NULL);
}


/*-------------------------------------------------
    clear - remove all items from a resource pool
-------------------------------------------------*/

void resource_pool::clear()
{
	osd_lock_acquire(m_listlock);

	// important: delete in reverse order of adding
	while (m_ordered_head != NULL)
		remove(m_ordered_head->m_ptr);

	osd_lock_release(m_listlock);
}



//**************************************************************************
//  MEMORY ENTRY
//**************************************************************************

/*-------------------------------------------------
    acquire_lock - acquire the memory entry lock,
    creating a new one if needed
-------------------------------------------------*/

void memory_entry::acquire_lock()
{
	// allocate a lock on first usage
	// note that osd_lock_alloc() may re-enter this path, so protect against recursion!
	if (s_lock == NULL)
	{
		if (s_lock_alloc)
			return;
		s_lock_alloc = true;
		s_lock = osd_lock_alloc();
		s_lock_alloc = false;
	}
	osd_lock_acquire(s_lock);
}


/*-------------------------------------------------
    release_lock - release the memory entry lock
-------------------------------------------------*/

void memory_entry::release_lock()
{
	osd_lock_release(s_lock);
}


/*-------------------------------------------------
    allocate - allocate a new memory entry
-------------------------------------------------*/

memory_entry *memory_entry::allocate(size_t size, void *base, const char *file, int line)
{
	acquire_lock();

	// if we're out of free entries, allocate a new chunk
	if (s_freehead == NULL)
	{
		// create a new chunk, and fail if we can't
		memory_entry *entry = reinterpret_cast<memory_entry *>(osd_malloc(memory_block_alloc_chunk * sizeof(memory_entry)));
		if (entry == NULL)
		{
			release_lock();
			return NULL;
		}

		// add all the entries to the list
		for (int entrynum = 0; entrynum < memory_block_alloc_chunk; entrynum++)
		{
			entry->m_next = s_freehead;
			s_freehead = entry++;
		}
	}

	// grab a free entry
	memory_entry *entry = s_freehead;
	s_freehead = entry->m_next;

	// populate it
	entry->m_size = size;
	entry->m_base = base;
	entry->m_file = file;
	entry->m_line = line;
	entry->m_id = s_curid++;

	// add it to the alloc list
	int hashval = reinterpret_cast<FPTR>(base) % k_hash_prime;
	entry->m_next = s_hash[hashval];
	if (entry->m_next != NULL)
		entry->m_next->m_prev = entry;
	entry->m_prev = NULL;
	s_hash[hashval] = entry;

	release_lock();
	return entry;
}


/*-------------------------------------------------
    find - find a memory entry
-------------------------------------------------*/

memory_entry *memory_entry::find(void *ptr)
{
	// NULL maps to nothing
	if (ptr == NULL)
		return NULL;

	// scan the list under the lock
	acquire_lock();

	int hashval = reinterpret_cast<FPTR>(ptr) % k_hash_prime;
	memory_entry *entry;
	for (entry = s_hash[hashval]; entry != NULL; entry = entry->m_next)
		if (entry->m_base == ptr)
			break;

	release_lock();
	return entry;
}


/*-------------------------------------------------
    release - release a memory entry
-------------------------------------------------*/

void memory_entry::release(memory_entry *entry)
{
	acquire_lock();

	// remove ourselves from the alloc list
	int hashval = reinterpret_cast<FPTR>(entry->m_base) % k_hash_prime;
	if (entry->m_prev != NULL)
		entry->m_prev->m_next = entry->m_next;
	else
		s_hash[hashval] = entry->m_next;
	if (entry->m_next != NULL)
		entry->m_next->m_prev = entry->m_prev;

	// add ourself to the free list
	entry->m_next = s_freehead;
	s_freehead = entry;

	release_lock();
}


/*-------------------------------------------------
    report_unfreed - print a list of unfreed
    memory to the target file
-------------------------------------------------*/

void memory_entry::report_unfreed()
{
	acquire_lock();

	// check for leaked memory
	UINT32 total = 0;

	for (int hashnum = 0; hashnum < k_hash_prime; hashnum++)
		for (memory_entry *entry = s_hash[hashnum]; entry != NULL; entry = entry->m_next)
			if (entry->m_file != NULL)
			{
				if (total == 0)
					fprintf(stderr, "--- memory leak warning ---\n");
				total += entry->m_size;
				fprintf(stderr, "allocation #%06d, %d bytes (%s:%d)\n", entry->m_id, static_cast<UINT32>(entry->m_size), entry->m_file, (int)entry->m_line);
			}

	release_lock();

	if (total > 0)
		fprintf(stderr, "a total of %d bytes were not free()'d\n", total);
}
