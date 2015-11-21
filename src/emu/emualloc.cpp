// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emualloc.c

    Memory allocation helpers for the core emulator.

***************************************************************************/

#include "emucore.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_ALLOCS      (0)



//**************************************************************************
//  GLOBALS
//**************************************************************************

UINT64 resource_pool::s_id = 0;



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
		m_hash(hash_size),
		m_ordered_head(NULL),
		m_ordered_tail(NULL)
{
	memset(&m_hash[0], 0, hash_size*sizeof(m_hash[0]));
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
}


//-------------------------------------------------
//  add - add a new item to the resource pool
//-------------------------------------------------

void resource_pool::add(resource_pool_item &item, size_t size, const char *type)
{
	osd_lock_acquire(m_listlock);

	// insert into hash table
	int hashval = reinterpret_cast<FPTR>(item.m_ptr) % m_hash_size;
	item.m_next = m_hash[hashval];
	m_hash[hashval] = &item;

	// fetch the ID of this item's pointer; some implementations put hidden data
	// before, so if we don't find it, check 4 bytes ahead
	item.m_id = ++s_id;
	if (LOG_ALLOCS)
		fprintf(stderr, "#%06d, add %s, %d bytes\n", (UINT32)item.m_id, type, UINT32(size));

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
			global_free(deleteme);
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
