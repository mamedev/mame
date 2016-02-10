// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    cstrpool.c

    Constant string pool helper class.

***************************************************************************/

#include <assert.h>

#include "cstrpool.h"


//**************************************************************************
//  CONST STRING POOL
//**************************************************************************

//-------------------------------------------------
//  const_string_pool - constructor
//-------------------------------------------------

const_string_pool::const_string_pool()
{
}


//-------------------------------------------------
//  add - add a string to the string pool
//-------------------------------------------------

const char *const_string_pool::add(const char *string)
{
	// if NULL or a small number (for some hash strings), just return as-is
	if (FPTR(string) < 0x100)
		return string;

	// scan to find space
	for (pool_chunk *chunk = m_chunklist.first(); chunk != nullptr; chunk = chunk->next())
	{
		const char *result = chunk->add(string);
		if (result != nullptr)
			return result;
	}

	// no space anywhere, create a new pool and prepend it (so it gets used first)
	const char *result = m_chunklist.prepend(*global_alloc(pool_chunk)).add(string);
	assert(result != nullptr);
	return result;
}


//-------------------------------------------------
//  contains - determine if the given string
//  pointer lives in the pool
//-------------------------------------------------

bool const_string_pool::contains(const char *string)
{
	// if NULL or a small number (for some hash strings), then yes, effectively
	if (FPTR(string) < 0x100)
		return true;

	// scan to find it
	for (pool_chunk *chunk = m_chunklist.first(); chunk != nullptr; chunk = chunk->next())
		if (chunk->contains(string))
			return true;

	return false;
}

/**
 * @fn  const_string_pool::pool_chunk::pool_chunk()
 *
 * @brief   -------------------------------------------------
 *            pool_chunk - constructor
 *          -------------------------------------------------.
 */

const_string_pool::pool_chunk::pool_chunk()
	: m_next(nullptr),
		m_used(0)
{
}

/**
 * @fn  const char *const_string_pool::pool_chunk::add(const char *string)
 *
 * @brief   -------------------------------------------------
 *            add - add a string to this pool
 *          -------------------------------------------------.
 *
 * @param   string  The string to add.
 *
 * @return  null if it fails, else a char*.
 */

const char *const_string_pool::pool_chunk::add(const char *string)
{
	// get the length of the string (no string can be longer than a full pool)
	int bytes = strlen(string) + 1;
	assert(bytes < POOL_SIZE);

	// if too big, return NULL
	if (m_used + bytes > POOL_SIZE)
		return nullptr;

	// allocate, copy, and return the memory
	char *dest = &m_buffer[m_used];
	m_used += bytes;
	memcpy(dest, string, bytes);
	return dest;
}
