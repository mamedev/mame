/***************************************************************************

    hash.c

    Function to handle hash functions (checksums)

    Based on original idea by Farfetch'd

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

#include "emu.h"
#include "zlib.h"
#include "sha1.h"
#include <ctype.h>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hash_crc

// CRC-32 hash implementation
class hash_crc : public hash_base
{
public:
	// construction/destruction
    hash_crc();

	// operators
    operator UINT32() const { return (m_buffer[0] << 24) | (m_buffer[1] << 16) | (m_buffer[2] << 8) | m_buffer[3]; }

	// creation
    virtual void begin();
    virtual void buffer(const UINT8 *data, UINT32 length);
    virtual void end();

private:
    // internal state
    UINT8   m_buffer[4];
};



// ======================> hash_sha1

// SHA1 hash implementation
class hash_sha1 : public hash_base
{
public:
	// construction/destruction
    hash_sha1();

	// creation
    virtual void begin();
    virtual void buffer(const UINT8 *data, UINT32 length);
    virtual void end();

private:
    // internal state
    UINT8   m_buffer[20];
    struct sha1_ctx m_context;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const char *hash_collection::HASH_TYPES_CRC = "R";
const char *hash_collection::HASH_TYPES_CRC_SHA1 = "RS";
const char *hash_collection::HASH_TYPES_ALL = "RS";



//**************************************************************************
//  HASH BASE
//**************************************************************************

//-------------------------------------------------
//  hash_base - constructor
//-------------------------------------------------

hash_base::hash_base(char id, const char *name, UINT8 length, UINT8 *bufptr)
	: m_next(NULL),
	  m_name(name),
	  m_in_progress(false),
	  m_parse_error(false),
	  m_id(id),
	  m_length(length),
	  m_bufptr(bufptr)
{
	memset(m_bufptr, 0, length);
}


//-------------------------------------------------
//  fromhex - convert a character to a hex value
//-------------------------------------------------

int hash_base::fromhex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	else
		return -1;
}


//-------------------------------------------------
//  from_buffer - copy a raw buffer into the
//  current hash
//-------------------------------------------------

bool hash_base::from_buffer(const UINT8 *buffer, int buflen)
{
	// fail if we're too small
	if (buflen < m_length)
		return false;
	memcpy(m_bufptr, buffer, m_length);
	return true;
}


//-------------------------------------------------
//  string - output a string for the current hash
//-------------------------------------------------

const char *hash_base::string(astring &buffer)
{
	buffer.reset();
	for (int index = 0; index < m_length; index++)
		buffer.catprintf("%02x", m_bufptr[index]);
	return buffer;
}


//-------------------------------------------------
//  from_string - parse a string into the current
//  hash
//-------------------------------------------------

bool hash_base::from_string(const char *&string, int length)
{
	// reset the error and our buffer
	m_parse_error = false;
	memset(m_bufptr, 0, m_length);

	// special case for idiom HASH(1) to map to a dummy (0) hash
	if (string[0] == '1' && fromhex(string[1]) == -1)
	{
		string++;
		return true;
	}

	// fail if we're too short
	if (length < 2 * m_length)
		return false;

	// loop over bytes
	for (int index = 0; index < m_length; index++)
	{
		// parse the upper digit
		int upper = fromhex(string[0]);
		if (upper == -1)
		{
			m_parse_error = true;
			return false;
		}

		// parse the lower digit
		int lower = fromhex(string[1]);
		if (lower == -1)
		{
			m_parse_error = true;
			return false;
		}

		// set the byte and advance
		m_bufptr[index] = (upper << 4) | lower;
		string += 2;
	}
	return true;
}



//**************************************************************************
//  HASH CRC
//**************************************************************************

//-------------------------------------------------
//  hash_crc - constructor
//-------------------------------------------------

hash_crc::hash_crc()
	: hash_base(hash_collection::HASH_CRC, "crc", sizeof(m_buffer), m_buffer)
{
}


//-------------------------------------------------
//  begin - initialize state for hash computation
//-------------------------------------------------

void hash_crc::begin()
{
	m_in_progress = true;
	memset(m_buffer, 0, sizeof(m_buffer));
}


//-------------------------------------------------
//  buffer - hash a buffer's worth of data
//-------------------------------------------------

void hash_crc::buffer(const UINT8 *data, UINT32 length)
{
	UINT32 crc = crc32(*this, data, length);
	m_buffer[0] = crc >> 24;
	m_buffer[1] = crc >> 16;
	m_buffer[2] = crc >> 8;
	m_buffer[3] = crc >> 0;
}


//-------------------------------------------------
//  end - finish hash computation
//-------------------------------------------------

void hash_crc::end()
{
	m_in_progress = false;
}



//**************************************************************************
//  HASH SHA1
//**************************************************************************

//-------------------------------------------------
//  hash_sha1 - constructor
//-------------------------------------------------

hash_sha1::hash_sha1()
	: hash_base(hash_collection::HASH_SHA1, "sha1", sizeof(m_buffer), m_buffer)
{
}


//-------------------------------------------------
//  begin - initialize state for hash computation
//-------------------------------------------------

void hash_sha1::begin()
{
	m_in_progress = true;
	sha1_init(&m_context);
}


//-------------------------------------------------
//  buffer - hash a buffer's worth of data
//-------------------------------------------------

void hash_sha1::buffer(const UINT8 *data, UINT32 length)
{
	sha1_update(&m_context, length, data);
}


//-------------------------------------------------
//  end - finish hash computation
//-------------------------------------------------

void hash_sha1::end()
{
	sha1_final(&m_context);
	sha1_digest(&m_context, sizeof(m_buffer), m_buffer);
	m_in_progress = false;
}



//**************************************************************************
//  HASH COLLECTION
//**************************************************************************

//-------------------------------------------------
//  hash_collection - constructor
//-------------------------------------------------

hash_collection::hash_collection()
{
}


hash_collection::hash_collection(const char *string)
{
	from_internal_string(string);
}


hash_collection::hash_collection(const hash_collection &src)
{
	copyfrom(src);
}


//-------------------------------------------------
//  ~hash_collection - destructor
//-------------------------------------------------

hash_collection::~hash_collection()
{
}


//-------------------------------------------------
//  operator= - assignment operator
//-------------------------------------------------

hash_collection &hash_collection::operator=(const hash_collection &src)
{
	// ignore self-assignment
	if (this != &src)
		copyfrom(src);
	return *this;
}


//-------------------------------------------------
//  operator== - test for equality
//-------------------------------------------------

bool hash_collection::operator==(const hash_collection &rhs) const
{
	// look for a mismatch in any hash; do not fail if one is missing
	int matches = 0;
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
	{
		hash_base *rhs_hash = rhs.hash(hash->id());
		if (rhs_hash != NULL)
		{
			if (*hash != *rhs_hash)
				return false;
			matches++;
		}
	}

	// if all shared hashes match, return true
	return (matches > 0);
}


//-------------------------------------------------
//  hash - return a hash of the given type
//-------------------------------------------------

hash_base *hash_collection::hash(char type) const
{
	// look for a mismatch in any hash; do not fail if one is missing
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		if (hash->id() == type)
			return hash;
	return NULL;
}


//-------------------------------------------------
//  hash_types - return a list of hash types as
//  a string
//-------------------------------------------------

const char *hash_collection::hash_types(astring &buffer) const
{
	buffer.reset();
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		buffer.cat(hash->id());
	return buffer;
}


//-------------------------------------------------
//  reset - reset the hash collection to an empty
//  set of hashes and flags
//-------------------------------------------------

bool hash_collection::parse_errors() const
{
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		if (hash->parse_error())
			return true;
	return false;
}


//-------------------------------------------------
//  reset - reset the hash collection to an empty
//  set of hashes and flags
//-------------------------------------------------

void hash_collection::reset()
{
	m_hashlist.reset();
	m_flags.reset();
}


//-------------------------------------------------
//  add_from_buffer - add a new hash, importing
//  from a buffer
//-------------------------------------------------

hash_base *hash_collection::add_from_buffer(char type, const UINT8 *buffer, int bufflen)
{
	// nuke any existing hash with the same ID
	hash_base *existing = hash(type);
	if (existing != NULL)
		m_hashlist.remove(*existing);

	// first allocate by ID
	hash_base *newhash = alloc_by_id(type);
	if (newhash == NULL)
		return NULL;

	// then import
	if (!newhash->from_buffer(buffer, bufflen))
	{
		global_free(newhash);
		return NULL;
	}

	// and append to our list
	return &m_hashlist.append(*newhash);
}


//-------------------------------------------------
//  add_from_string - add a new hash, importing
//  from a string
//-------------------------------------------------

hash_base *hash_collection::add_from_string(char type, const char *buffer, int length)
{
	// nuke any existing hash with the same ID
	hash_base *existing = hash(type);
	if (existing != NULL)
		m_hashlist.remove(*existing);

	// first allocate by ID
	hash_base *newhash = alloc_by_id(type);
	if (newhash == NULL)
		return NULL;

	// then import
	if (!newhash->from_string(buffer, length))
	{
		global_free(newhash);
		return NULL;
	}

	// and append to our list
	return &m_hashlist.append(*newhash);
}


//-------------------------------------------------
//  remove - remove a hash of the given type
//-------------------------------------------------

bool hash_collection::remove(char type)
{
	// scan the list of hashes for a match
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		if (hash->id() == type)
		{
			m_hashlist.remove(*hash);
			return true;
		}

	// didn't find it
	return false;
}


//-------------------------------------------------
//  crc - return the CRC hash if present
//-------------------------------------------------

bool hash_collection::crc(UINT32 &result) const
{
	// attempt to find the CRC hash; if we fail, return false
	hash_base *crchash = hash(HASH_CRC);
	if (crchash == NULL)
		return false;

	// downcast to a hash_crc and convert to a UINT32
	result = *downcast<const hash_crc *>(crchash);
	return true;
}


//-------------------------------------------------
//  add_crc - add a CRC hash
//-------------------------------------------------

hash_base *hash_collection::add_crc(UINT32 crc)
{
	// expand to a buffer
	UINT8 buffer[4];
	buffer[0] = crc >> 24;
	buffer[1] = crc >> 16;
	buffer[2] = crc >> 8;
	buffer[3] = crc >> 0;

	// add it the standard way
	return add_from_buffer(HASH_CRC, buffer, sizeof(buffer));
}


//-------------------------------------------------
//  internal_string - convert set of hashes and
//  flags to a string in our internal compact
//  format
//-------------------------------------------------

const char *hash_collection::internal_string(astring &buffer) const
{
	astring temp;

	// output hashes first
	buffer.reset();
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		buffer.cat(hash->id()).cat(hash->string(temp));

	// append flags
	buffer.cat(m_flags);
	return buffer;
}


//-------------------------------------------------
//  macro_string - convert set of hashes and
//  flags to a string in the macroized format
//-------------------------------------------------

const char *hash_collection::macro_string(astring &buffer) const
{
	astring temp;

	// output hashes first
	buffer.reset();
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
	{
		buffer.cat(temp.cpy(hash->name()).toupper());
		buffer.cat("(").cat(hash->string(temp)).cat(") ");
	}

	// append flags
	if (flag(FLAG_NO_DUMP))
		buffer.cat("NO_DUMP ");
	if (flag(FLAG_BAD_DUMP))
		buffer.cat("BAD_DUMP ");
	return buffer.trimspace();
}


//-------------------------------------------------
//  from_internal_string - convert an internal
//  compact string to set of hashes and flags
//-------------------------------------------------

bool hash_collection::from_internal_string(const char *string)
{
	// start fresh
	reset();

	// determine the end of the string
	const char *stringend = string + strlen(string);
	const char *ptr = string;

	// loop until we hit it
	bool errors = false;
	while (ptr < stringend)
	{
		char c = *ptr++;
		char lc = tolower(c);

		// non-hex alpha values specify a hash type
		if (lc >= 'g' && lc <= 'z')
		{
			hash_base *hash = alloc_by_id(c);
			if (hash != NULL)
			{
				if (!hash->from_string(ptr, stringend - ptr))
					errors = true;
				m_hashlist.append(*hash);
			}
			else
				errors = true;
		}

		// hex values are ignored, though unexpected
		else if ((lc >= '0' && lc <= '9') || (lc >= 'a' && lc <= 'f'))
			errors = true;

		// anything else is a flag
		else
			m_flags.cat(c);
	}
	return !errors;
}


//-------------------------------------------------
//  begin - begin hashing
//-------------------------------------------------

void hash_collection::begin(const char *types)
{
	// by default use all types
	if (types == NULL)
	{
		m_hashlist.append(*alloc_by_id(HASH_CRC)).begin();
		m_hashlist.append(*alloc_by_id(HASH_SHA1)).begin();
	}

	// otherwise, just allocate the ones that are specified
	else
	{
		for (const char *scan = types; *scan != 0; scan++)
		{
			// nuke any existing hash of this type
			hash_base *existing = hash(*scan);
			if (existing != NULL)
				m_hashlist.remove(*existing);

			// append a new one
			m_hashlist.append(*alloc_by_id(*scan)).begin();
		}
	}
}


//-------------------------------------------------
//  buffer - add the given buffer to the hash
//-------------------------------------------------

void hash_collection::buffer(const UINT8 *data, UINT32 length)
{
	// buffer each hash appropriately
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		if (hash->in_progress())
			hash->buffer(data, length);
}


//-------------------------------------------------
//  end - stop hashing
//-------------------------------------------------

void hash_collection::end()
{
	// end each hash
	for (hash_base *hash = m_hashlist.first(); hash != NULL; hash = hash->next())
		if (hash->in_progress())
			hash->end();
}


//-------------------------------------------------
//  alloc_by_id - based on the ID character,
//  allocate a new hash
//-------------------------------------------------

hash_base *hash_collection::alloc_by_id(char id)
{
	switch (id)
	{
		case HASH_CRC:	return global_alloc(hash_crc);
		case HASH_SHA1:	return global_alloc(hash_sha1);
		default:		return NULL;
	}
}


//-------------------------------------------------
//  copyfrom - copy everything from another
//  collection
//-------------------------------------------------

void hash_collection::copyfrom(const hash_collection &src)
{
	// copy flags directly
	m_flags = src.m_flags;

	// rebuild the hashlist by copying from the source
	m_hashlist.reset();
	for (hash_base *hash = src.first(); hash != NULL; hash = hash->next())
		add_from_buffer(hash->id(), hash->buffer(), hash->length());
}
