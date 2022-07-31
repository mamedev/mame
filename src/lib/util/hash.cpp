// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    hash.cpp

    Function to handle hash functions (checksums)

    Based on original idea by Farfetch'd

***************************************************************************/

#include "hash.h"

#include "ioprocs.h"

#include <cassert>
#include <cctype>
#include <optional>


namespace util {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

char const *const hash_collection::HASH_TYPES_CRC = "R";
char const *const hash_collection::HASH_TYPES_CRC_SHA1 = "RS";
char const *const hash_collection::HASH_TYPES_ALL = "RS";


//**************************************************************************
//  HASH CREATOR
//**************************************************************************

class hash_collection::hash_creator
{
public:
	// constructor
	hash_creator(bool doing_crc32, bool doing_sha1)
	{
		if (doing_crc32)
			m_crc32_creator.emplace();
		if (doing_sha1)
			m_sha1_creator.emplace();
	}

	// add the given buffer to the hash
	void append(void const *buffer, std::size_t length)
	{
		// append to each active hash
		if (m_crc32_creator)
			m_crc32_creator->append(buffer, length);
		if (m_sha1_creator)
			m_sha1_creator->append(buffer, length);
	}

	// stop hashing
	void finish(hash_collection &hashes)
	{
		// finish up the CRC32
		if (m_crc32_creator)
		{
			hashes.add_crc(m_crc32_creator->finish());
			m_crc32_creator.reset();
		}

		// finish up the SHA1
		if (m_sha1_creator)
		{
			hashes.add_sha1(m_sha1_creator->finish());
			m_sha1_creator.reset();
		}
	}

private:
	std::optional<crc32_creator> m_crc32_creator;
	std::optional<sha1_creator> m_sha1_creator;
};


//**************************************************************************
//  HASH COLLECTION
//**************************************************************************

//-------------------------------------------------
//  hash_collection - constructor
//-------------------------------------------------

hash_collection::hash_collection()
	: m_has_crc32(false)
	, m_has_sha1(false)
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
	// match CRCs
	int matches = 0;
	if (m_has_crc32 && rhs.m_has_crc32)
	{
		if (m_crc32 != rhs.m_crc32)
			return false;
		matches++;
	}

	// match SHA1s
	if (m_has_sha1 && rhs.m_has_sha1)
	{
		if (m_sha1 != rhs.m_sha1)
			return false;
		matches++;
	}

	// if all shared hashes match, return true
	return (matches > 0);
}


//-------------------------------------------------
//  hash_types - return a list of hash types as
//  a string
//-------------------------------------------------

std::string hash_collection::hash_types() const
{
	std::string buffer;
	if (m_has_crc32)
		buffer.push_back(HASH_CRC);
	if (m_has_sha1)
		buffer.push_back(HASH_SHA1);
	return buffer;
}


//-------------------------------------------------
//  reset - reset the hash collection to an empty
//  set of hashes and flags
//-------------------------------------------------

void hash_collection::reset()
{
	m_flags.clear();
	m_has_crc32 = m_has_sha1 = false;
}


//-------------------------------------------------
//  add_from_string - add a new hash, importing
//  from a string
//-------------------------------------------------

bool hash_collection::add_from_string(char type, std::string_view string)
{
	// handle CRCs
	if (type == HASH_CRC)
		return m_has_crc32 = m_crc32.from_string(string);

	// handle SHA1s
	else if (type == HASH_SHA1)
		return m_has_sha1 = m_sha1.from_string(string);

	return false;
}


//-------------------------------------------------
//  remove - remove a hash of the given type
//-------------------------------------------------

bool hash_collection::remove(char type)
{
	bool result = false;

	// handle CRCs
	if (type == HASH_CRC)
	{
		result = m_has_crc32;
		m_has_crc32 = false;
	}

	// handle SHA1s
	else if (type == HASH_SHA1)
	{
		result = m_has_sha1;
		m_has_sha1 = false;
	}
	return result;
}


//-------------------------------------------------
//  internal_string - convert set of hashes and
//  flags to a string in our internal compact
//  format
//-------------------------------------------------

std::string hash_collection::internal_string() const
{
	std::string buffer;

	// handle CRCs
	if (m_has_crc32) {
		buffer.push_back(HASH_CRC);
		buffer.append(m_crc32.as_string());
	}
	// handle SHA1s
	if (m_has_sha1) {
		buffer.push_back(HASH_SHA1);
		buffer.append(m_sha1.as_string());
	}

	// append flags
	buffer.append(m_flags);
	return buffer;
}


//-------------------------------------------------
//  macro_string - convert set of hashes and
//  flags to a string in the macroized format
//-------------------------------------------------

std::string hash_collection::macro_string() const
{
	std::string buffer;

	// handle CRCs
	if (m_has_crc32)
		buffer.append("CRC(").append(m_crc32.as_string()).append(") ");

	// handle SHA1s
	if (m_has_sha1)
		buffer.append("SHA1(").append(m_sha1.as_string()).append(") ");

	// append flags
	if (flag(FLAG_NO_DUMP))
		buffer.append("NO_DUMP ");
	if (flag(FLAG_BAD_DUMP))
		buffer.append("BAD_DUMP ");

	// remove trailing space
	if (!buffer.empty())
	{
		assert(buffer.back() == ' ');
		buffer = buffer.substr(0, buffer.length() - 1);
	}
	return buffer;
}


//-------------------------------------------------
//  attribute_string - convert set of hashes and
//  flags to a string in XML attribute format
//-------------------------------------------------

std::string hash_collection::attribute_string() const
{
	std::string buffer;

	// handle CRCs
	if (m_has_crc32)
		buffer.append("crc=\"").append(m_crc32.as_string()).append("\" ");

	// handle SHA1s
	if (m_has_sha1)
		buffer.append("sha1=\"").append(m_sha1.as_string()).append("\" ");

	// append flags
	if (flag(FLAG_NO_DUMP))
		buffer.append("status=\"nodump\"");
	if (flag(FLAG_BAD_DUMP))
		buffer.append("status=\"baddump\"");

	// remove trailing space
	if (!buffer.empty() && buffer.back() == ' ')
		buffer = buffer.substr(0, buffer.length() - 1);
	return buffer;
}


//-------------------------------------------------
//  from_internal_string - convert an internal
//  compact string to set of hashes and flags
//-------------------------------------------------

bool hash_collection::from_internal_string(std::string_view string)
{
	// start fresh
	reset();

	// loop until we hit the end of the string
	bool errors = false;
	int skip_digits = 0;
	while (!string.empty())
	{
		char c = string[0];
		char uc = toupper(c);
		string.remove_prefix(1);

		// non-hex alpha values specify a hash type
		if (uc >= 'G' && uc <= 'Z')
		{
			skip_digits = 0;
			if (uc == HASH_CRC)
			{
				m_has_crc32 = true;
				errors = !m_crc32.from_string(string);
				skip_digits = 2 * sizeof(crc32_t);
			}
			else if (uc == HASH_SHA1)
			{
				m_has_sha1 = true;
				errors = !m_sha1.from_string(string);
				skip_digits = 2 * sizeof(sha1_t);
			}
			else
				errors = true;
		}

		// hex values are ignored, though unexpected
		else if ((uc >= '0' && uc <= '9') || (uc >= 'A' && uc <= 'F'))
		{
			if (skip_digits != 0)
				skip_digits--;
			else
				errors = true;
		}

		// anything else is a flag
		else if (skip_digits != 0)
			errors = true;
		else
			m_flags.push_back(c);
	}
	return !errors;
}


//-------------------------------------------------
//  create - begin hashing
//-------------------------------------------------

std::unique_ptr<hash_collection::hash_creator> hash_collection::create(const char *types)
{
	// by default use all types
	if (types == nullptr)
		return std::make_unique<hash_creator>(true, true);

	// otherwise, just allocate the ones that are specified
	else
		return std::make_unique<hash_creator>(strchr(types, HASH_CRC) != nullptr, strchr(types, HASH_SHA1) != nullptr);
}


//-------------------------------------------------
//  compute - hash a block of data
//-------------------------------------------------

void hash_collection::compute(const uint8_t *data, uint32_t length, const char *types)
{
	// begin
	std::unique_ptr<hash_creator> creator = create(types);

	// run the hashes
	creator->append(data, length);

	// end
	creator->finish(*this);
}


//-------------------------------------------------
//  compute - hash data from a stream
//-------------------------------------------------

std::error_condition hash_collection::compute(random_read &stream, uint64_t offset, size_t length, size_t &actual, const char *types)
{
	// begin
	std::unique_ptr<hash_creator> creator = create(types);

	// local buffer of arbitrary size
	uint8_t buffer[2048];

	// run the hashes
	actual = 0U;
	while (length)
	{
		// determine the size of the next chunk
		unsigned const chunk_length = std::min(length, sizeof(buffer));

		// read one chunk
		std::size_t bytes_read;
		std::error_condition err = stream.read_at(offset, buffer, chunk_length, bytes_read);
		if (err)
			return err;
		if (!bytes_read) // EOF?
			break;
		offset += bytes_read;
		length -= chunk_length;

		// append the chunk
		creator->append(buffer, bytes_read);
		actual += bytes_read;
	}

	// end
	creator->finish(*this);
	return std::error_condition();
}


//-------------------------------------------------
//  copyfrom - copy everything from another
//  collection
//-------------------------------------------------

void hash_collection::copyfrom(const hash_collection &src)
{
	// copy flags directly
	m_flags = src.m_flags;

	// copy hashes
	m_has_crc32 = src.m_has_crc32;
	m_crc32 = src.m_crc32;
	m_has_sha1 = src.m_has_sha1;
	m_sha1 = src.m_sha1;
}

} // namespace util
