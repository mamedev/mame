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
//  DEBUGGING
//**************************************************************************

#define LOG_ALLOCS      (0)



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
	memory_entry *      m_next;             // link to the next entry
	memory_entry *      m_prev;             // link to the previous entry
	size_t              m_size;             // size of the allocation (not including this header)
	void *              m_base;             // base of the allocation
	const char *        m_file;             // file the allocation was made from
	int                 m_line;             // line number within that file
	UINT64              m_id;               // unique id

	static const int    k_hash_prime = 6151;

	static UINT64       s_curid;            // current ID
	static osd_lock *   s_lock;             // lock for managing the list
	static bool         s_lock_alloc;       // set to true temporarily during lock allocation
	static bool         s_tracking;         // set to true when tracking is live
	static memory_entry *s_hash[k_hash_prime];// hash table based on pointer
	static memory_entry *s_freehead;        // pointer to the head of the free list

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

// dummy zeromem object
const zeromem_t zeromem = { };

// globals for memory_entry
UINT64 memory_entry::s_curid = 1;
osd_lock *memory_entry::s_lock = NULL;
bool memory_entry::s_lock_alloc = false;
bool memory_entry::s_tracking = false;
memory_entry *memory_entry::s_hash[memory_entry::k_hash_prime] = { NULL };
memory_entry *memory_entry::s_freehead = NULL;

// wrapper for the global resource pool to help ensure construction order
resource_pool &global_resource_pool()
{
	static resource_pool s_pool(6151);
	return s_pool;
};


//**************************************************************************
//  GLOBAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  malloc_file_line - allocate memory with file
//  and line number information
//-------------------------------------------------

void *malloc_file_line(size_t size, const char *file, int line)
{
	// allocate the memory and fail if we can't
	void *result = osd_malloc(size);
	if (result == NULL)
		return NULL;

	// add a new entry
	memory_entry::allocate(size, result, file, line);

#ifdef MAME_DEBUG
	// randomize the memory
	rand_memory(result, size);
#endif

	return result;
}


//-------------------------------------------------
//  malloc_array_file_line - allocate memory with
//  file and line number information, and a hint
//  that this object is an array
//-------------------------------------------------

void *malloc_array_file_line(size_t size, const char *file, int line)
{
	// allocate the memory and fail if we can't
	void *result = osd_malloc_array(size);
	if (result == NULL)
		return NULL;

	// add a new entry
	memory_entry::allocate(size, result, file, line);

#ifdef MAME_DEBUG
	// randomize the memory
	rand_memory(result, size);
#endif

	return result;
}


//-------------------------------------------------
//  free_file_line - free memory with file
//  and line number information
//-------------------------------------------------

void free_file_line(void *memory, const char *file, int line)
{
	// ignore NULL frees/deletes
	if (memory == NULL)
		return;

	// find the memory entry
	memory_entry *entry = memory_entry::find(memory);

	// warn about untracked frees
	if (entry == NULL)
	{
		fprintf(stderr, "Error: attempt to free untracked memory in %s(%d)!\n", file, line);
		osd_break_into_debugger("Error: attempt to free untracked memory");
		return;
	}

#ifdef MAME_DEBUG
	// clear memory to a bogus value
	memset(memory, 0xfc, entry->m_size);
#endif

	// free the entry and the memory
	memory_entry::release(entry);
	osd_free(memory);
}


//-------------------------------------------------
//  dump_unfreed_mem - called from the exit path
//  of any code that wants to check for unfreed
//  memory
//-------------------------------------------------

void track_memory(bool track)
{
	memory_entry::s_tracking = track;
}


//-------------------------------------------------
//  dump_unfreed_mem - called from the exit path
//  of any code that wants to check for unfreed
//  memory
//-------------------------------------------------

void dump_unfreed_mem()
{
#ifdef MAME_DEBUG
	memory_entry::report_unfreed();
#endif
}



//**************************************************************************
//  RESOURCE POOL
//**************************************************************************

//-------------------------------------------------
//  resource_pool - constructor for a new resource
//  pool
//-------------------------------------------------

resource_pool::resource_pool(int hash_size)
	: m_hash_size(hash_size),
		m_listlock(osd_lock_alloc()),
		m_hash(new resource_pool_item *[hash_size]),
		m_ordered_head(NULL),
		m_ordered_tail(NULL)
{
	memset(m_hash, 0, hash_size * sizeof(m_hash[0]));
}


//-------------------------------------------------
//  ~resource_pool - destructor for a resource
//  pool; make sure all tracked objects are
//  deleted
//-------------------------------------------------

resource_pool::~resource_pool()
{
	clear();
	if (m_listlock != NULL)
		osd_lock_free(m_listlock);
	delete[] m_hash;
}


//-------------------------------------------------
//  add - add a new item to the resource pool
//-------------------------------------------------

void resource_pool::add(resource_pool_item &item)
{
	osd_lock_acquire(m_listlock);

	// insert into hash table
	int hashval = reinterpret_cast<FPTR>(item.m_ptr) % m_hash_size;
	item.m_next = m_hash[hashval];
	m_hash[hashval] = &item;

	// fetch the ID of this item's pointer; some implementations put hidden data
	// before, so if we don't find it, check 4 bytes ahead
	memory_entry *entry = memory_entry::find(item.m_ptr);
	if (entry == NULL)
		entry = memory_entry::find(reinterpret_cast<UINT8 *>(item.m_ptr) - sizeof(size_t));
	assert(entry != NULL);
	item.m_id = entry->m_id;
	if (LOG_ALLOCS)
		fprintf(stderr, "#%06d, add %d bytes (%s:%d)\n", (UINT32)entry->m_id, static_cast<UINT32>(entry->m_size), entry->m_file, (int)entry->m_line);

	// find the entry to insert after
	resource_pool_item *insert_after;
	for (insert_after = m_ordered_tail; insert_after != NULL; insert_after = insert_after->m_ordered_prev)
		if (insert_after->m_id < item.m_id)
			break;

	// insert into the appropriate spot
	if (insert_after != NULL)
	{
		item.m_ordered_next = insert_after->m_ordered_next;
		if (item.m_ordered_next != NULL)
			item.m_ordered_next->m_ordered_prev = &item;
		else
			m_ordered_tail = &item;
		item.m_ordered_prev = insert_after;
		insert_after->m_ordered_next = &item;
	}
	else
	{
		item.m_ordered_next = m_ordered_head;
		if (item.m_ordered_next != NULL)
			item.m_ordered_next->m_ordered_prev = &item;
		else
			m_ordered_tail = &item;
		item.m_ordered_prev = NULL;
		m_ordered_head = &item;
	}

	osd_lock_release(m_listlock);
}


//-------------------------------------------------
//  remove - remove a specific item from the
//  resource pool
//-------------------------------------------------

void resource_pool::remove(void *ptr)
{
	// ignore NULLs
	if (ptr == NULL)
		return;

	// search for the item
	osd_lock_acquire(m_listlock);

	int hashval = reinterpret_cast<FPTR>(ptr) % m_hash_size;
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
			else
				m_ordered_tail = deleteme->m_ordered_prev;

			// delete the object and break
			if (LOG_ALLOCS)
				fprintf(stderr, "#%06d, delete %d bytes\n", (UINT32)deleteme->m_id, static_cast<UINT32>(deleteme->m_size));
			delete deleteme;
			break;
		}

	osd_lock_release(m_listlock);
}


//-------------------------------------------------
//  find - find a specific item in the resource
//  pool
//-------------------------------------------------

resource_pool_item *resource_pool::find(void *ptr)
{
	// search for the item
	osd_lock_acquire(m_listlock);

	int hashval = reinterpret_cast<FPTR>(ptr) % m_hash_size;
	resource_pool_item *item;
	for (item = m_hash[hashval]; item != NULL; item = item->m_next)
		if (item->m_ptr == ptr)
			break;

	osd_lock_release(m_listlock);

	return item;
}


//-------------------------------------------------
//  contains - return true if given ptr is
//  contained by one of the objects in the pool
//-------------------------------------------------

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


//-------------------------------------------------
//  clear - remove all items from a resource pool
//-------------------------------------------------

void resource_pool::clear()
{
	osd_lock_acquire(m_listlock);

	// important: delete from earliest to latest; this allows objects to clean up after
	// themselves if they wish
	while (m_ordered_head != NULL)
		remove(m_ordered_head->m_ptr);

	osd_lock_release(m_listlock);
}



//**************************************************************************
//  MEMORY ENTRY
//**************************************************************************

//-------------------------------------------------
//  acquire_lock - acquire the memory entry lock,
//  creating a new one if needed
//-------------------------------------------------

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


//-------------------------------------------------
//  release_lock - release the memory entry lock
//-------------------------------------------------

void memory_entry::release_lock()
{
	osd_lock_release(s_lock);
}


//-------------------------------------------------
//  allocate - allocate a new memory entry
//-------------------------------------------------

memory_entry *memory_entry::allocate(size_t size, void *base, const char *file, int line)
{
	acquire_lock();

	// if we're out of free entries, allocate a new chunk
	if (s_freehead == NULL)
	{
		// create a new chunk, and fail if we can't
		memory_entry *entry = reinterpret_cast<memory_entry *>(osd_malloc_array(memory_block_alloc_chunk * sizeof(memory_entry)));
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
	entry->m_file = s_tracking ? file : NULL;
	entry->m_line = s_tracking ? line : 0;
	entry->m_id = s_curid++;
	if (LOG_ALLOCS)
		fprintf(stderr, "#%06d, alloc %d bytes (%s:%d)\n", (UINT32)entry->m_id, static_cast<UINT32>(entry->m_size), entry->m_file, (int)entry->m_line);

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


//-------------------------------------------------
//  find - find a memory entry
//-------------------------------------------------

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


//-------------------------------------------------
//  release - release a memory entry
//-------------------------------------------------

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


//-------------------------------------------------
//  report_unfreed - print a list of unfreed
//  memory to the target file
//-------------------------------------------------

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
				fprintf(stderr, "#%06d, nofree %d bytes (%s:%d)\n", (UINT32)entry->m_id, static_cast<UINT32>(entry->m_size), entry->m_file, (int)entry->m_line);
			}

	release_lock();

	if (total > 0)
		fprintf(stderr, "a total of %u bytes were not freed\n", total);
}
