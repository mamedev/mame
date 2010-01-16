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


/***************************************************************************
    CONSTANTS
***************************************************************************/

// align all allocated memory to this size
const int memory_align = 16;

// number of memory_entries to allocate in a block
const int memory_block_alloc_chunk = 256;



/***************************************************************************
    MACROS
***************************************************************************/

// enable deletion
#undef delete



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// this struct is allocated in pools to track memory allocations
// it must be a POD type!!
class memory_entry
{
public:
	memory_entry *		next;			// link to the next entry
	memory_entry *		prev;			// link to the previous entry
	size_t				size;			// size of the allocation (not including this header)
	void *				base;			// base of the allocation
	const char *		file;			// file the allocation was made from
	int					line;			// line number within that file
	int					id;				// unique id

	static const int	hash_prime = 193;

	static int			curid;			// current ID
	static osd_lock *	lock;			// lock for managing the list
	static bool			lock_alloc;		// set to true temporarily during lock allocation
	static memory_entry *hash[hash_prime];// hash table based on pointer
	static memory_entry *freehead;		// pointer to the head of the free list

	static memory_entry *allocate(size_t size, void *base, const char *file, int line);
	static memory_entry *find(void *ptr);
	static void release(memory_entry *entry);
	static void report_unfreed();

private:
	static void acquire_lock();
	static void release_lock();
};



/***************************************************************************
    GLOBALS
***************************************************************************/

// global resource pool to handle allocations outside of the emulator context
resource_pool global_resource_pool;

// dummy zeromem object
const zeromem_t zeromem = { };

// globals for memory_entry
int memory_entry::curid = 0;
osd_lock *memory_entry::lock = NULL;
bool memory_entry::lock_alloc = false;
memory_entry *memory_entry::hash[memory_entry::hash_prime] = { NULL };
memory_entry *memory_entry::freehead = NULL;



/***************************************************************************
    GLOBAL HELPERS
***************************************************************************/

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



/***************************************************************************
    RESOURCE POOL
***************************************************************************/

/*-------------------------------------------------
    resource_pool - constructor for a new resource
    pool
-------------------------------------------------*/

resource_pool::resource_pool()
	: listlock(osd_lock_alloc()),
	  ordered_head(NULL)
{
	memset(hash, 0, sizeof(hash));
}


/*-------------------------------------------------
    ~resource_pool - destructor for a resource
    pool; make sure all tracked objects are
    deleted
-------------------------------------------------*/

resource_pool::~resource_pool()
{
	clear();
	if (listlock != NULL)
		osd_lock_free(listlock);
}


/*-------------------------------------------------
    add - add a new item to the resource pool
-------------------------------------------------*/

void resource_pool::add(resource_pool_item &item)
{
	osd_lock_acquire(listlock);

	// insert into hash table
	int hashval = reinterpret_cast<FPTR>(item.ptr) % hash_prime;
	item.next = hash[hashval];
	hash[hashval] = &item;

	// insert into ordered list
	item.ordered_next = ordered_head;
	if (ordered_head != NULL)
		ordered_head->ordered_prev = &item;
	ordered_head = &item;

	osd_lock_release(listlock);
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
	osd_lock_acquire(listlock);

	int hashval = reinterpret_cast<FPTR>(ptr) % hash_prime;
	for (resource_pool_item **scanptr = &hash[hashval]; *scanptr != NULL; scanptr = &(*scanptr)->next)

		// must match the pointer
		if ((*scanptr)->ptr == ptr)
		{
			// remove from hash table
			resource_pool_item *deleteme = *scanptr;
			*scanptr = deleteme->next;

			// remove from ordered list
			if (deleteme->ordered_prev != NULL)
				deleteme->ordered_prev->ordered_next = deleteme->ordered_next;
			else
				ordered_head = deleteme->ordered_next;
			if (deleteme->ordered_next != NULL)
				deleteme->ordered_next->ordered_prev = deleteme->ordered_prev;

			// delete the object and break
			delete deleteme;
			break;
		}

	osd_lock_release(listlock);
}


/*-------------------------------------------------
    find - find a specific item in the resource
    pool
-------------------------------------------------*/

resource_pool_item *resource_pool::find(void *ptr)
{
	// search for the item
	osd_lock_acquire(listlock);

	int hashval = reinterpret_cast<FPTR>(ptr) % hash_prime;
	resource_pool_item *item;
	for (item = hash[hashval]; item != NULL; item = item->next)
		if (item->ptr == ptr)
			break;

	osd_lock_release(listlock);

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
	osd_lock_acquire(listlock);

	resource_pool_item *item = NULL;
	for (item = ordered_head; item != NULL; item = item->ordered_next)
	{
		UINT8 *objstart = reinterpret_cast<UINT8 *>(item->ptr);
		UINT8 *objend = objstart + item->size;
		if (ptrstart >= objstart && ptrend <= objend)
			goto found;
	}

found:
	osd_lock_release(listlock);

	return (item != NULL);
}


/*-------------------------------------------------
    clear - remove all items from a resource pool
-------------------------------------------------*/

void resource_pool::clear()
{
	osd_lock_acquire(listlock);

	// important: delete in reverse order of adding
	while (ordered_head != NULL)
		remove(ordered_head->ptr);

	osd_lock_release(listlock);
}



/***************************************************************************
    MEMORY ENTRY
***************************************************************************/

/*-------------------------------------------------
    acquire_lock - acquire the memory entry lock,
    creating a new one if needed
-------------------------------------------------*/

void memory_entry::acquire_lock()
{
	// allocate a lock on first usage
	// note that osd_lock_alloc() may re-enter this path, so protect against recursion!
	if (lock == NULL)
	{
		if (lock_alloc)
			return;
		lock_alloc = true;
		lock = osd_lock_alloc();
		lock_alloc = false;
	}
	osd_lock_acquire(lock);
}


/*-------------------------------------------------
    release_lock - release the memory entry lock
-------------------------------------------------*/

void memory_entry::release_lock()
{
	osd_lock_release(lock);
}


/*-------------------------------------------------
    allocate - allocate a new memory entry
-------------------------------------------------*/

memory_entry *memory_entry::allocate(size_t size, void *base, const char *file, int line)
{
	acquire_lock();

	// if we're out of free entries, allocate a new chunk
	if (freehead == NULL)
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
			entry->next = freehead;
			freehead = entry++;
		}
	}

	// grab a free entry
	memory_entry *entry = freehead;
	freehead = entry->next;

	// populate it
	entry->size = size;
	entry->base = base;
	entry->file = file;
	entry->line = line;
	entry->id = curid++;

	// add it to the alloc list
	int hashval = reinterpret_cast<FPTR>(base) % hash_prime;
	entry->next = hash[hashval];
	if (entry->next != NULL)
		entry->next->prev = entry;
	entry->prev = NULL;
	hash[hashval] = entry;

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

	int hashval = reinterpret_cast<FPTR>(ptr) % hash_prime;
	memory_entry *entry;
	for (entry = hash[hashval]; entry != NULL; entry = entry->next)
		if (entry->base == ptr)
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
	int hashval = reinterpret_cast<FPTR>(entry->base) % hash_prime;
	if (entry->prev != NULL)
		entry->prev->next = entry->next;
	else
		hash[hashval] = entry->next;
	if (entry->next != NULL)
		entry->next->prev = entry->prev;

	// add ourself to the free list
	entry->next = freehead;
	freehead = entry;

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

	for (int hashnum = 0; hashnum < hash_prime; hashnum++)
		for (memory_entry *entry = hash[hashnum]; entry != NULL; entry = entry->next)
			if (entry->file != NULL)
			{
				if (total == 0)
					fprintf(stderr, "--- memory leak warning ---\n");
				total += entry->size;
				fprintf(stderr, "allocation #%06d, %d bytes (%s:%d)\n", entry->id, static_cast<UINT32>(entry->size), entry->file, (int)entry->line);
			}

	release_lock();

	if (total > 0)
		fprintf(stderr, "a total of %d bytes were not free()'d\n", total);
}



/***************************************************************************
    STANDARD NEW/DELETE OPERATORS
***************************************************************************/

void *operator new(std::size_t size) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, NULL, 0);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}


void *operator new[](std::size_t size) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, NULL, 0);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}


void operator delete(void *ptr)
{
	if (ptr != NULL)
		free_file_line(ptr, NULL, 0);
}


void operator delete[](void *ptr)
{
	if (ptr != NULL)
		free_file_line(ptr, NULL, 0);
}



/***************************************************************************
    POOL NEW/DELETE OPERATORS
***************************************************************************/

void *operator new(std::size_t size, const char *file, int line) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}


void *operator new[](std::size_t size, const char *file, int line) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}


void operator delete(void *ptr, const char *file, int line)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}


void operator delete[](void *ptr, const char *file, int line)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}



/***************************************************************************
    POOL NEW/DELETE OPERATORS WITH ZEROING
***************************************************************************/

void *operator new(std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	memset(result, 0, size);
	return result;
}


void *operator new[](std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	memset(result, 0, size);
	return result;
}


void operator delete(void *ptr, const char *file, int line, const zeromem_t &)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}


void operator delete[](void *ptr, const char *file, int line, const zeromem_t &)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}
