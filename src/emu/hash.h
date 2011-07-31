/***************************************************************************

    hash.h

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

#pragma once

#ifndef __HASH_H__
#define __HASH_H__


//**************************************************************************
//  MACROS
//**************************************************************************

// use these to define compile-time internal-format hash strings
#define CRC(x)				"R" #x
#define SHA1(x)				"S" #x
#define NO_DUMP				"!"
#define BAD_DUMP			"^"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> hash_base

// base class for all hash types, which does most of the heavy lifting
class hash_base
{
	friend class simple_list<hash_base>;

public:
    // construction/destruction
    hash_base(char id, const char *name, UINT8 length, UINT8 *bufptr);
    virtual ~hash_base() { }

    // operators
    bool operator==(const hash_base &rhs) const { return (m_length == rhs.m_length && memcmp(m_bufptr, rhs.m_bufptr, m_length) == 0); }
    bool operator!=(const hash_base &rhs) const { return (m_length != rhs.m_length || memcmp(m_bufptr, rhs.m_bufptr, m_length) != 0); }

    // getters
    hash_base *next() const { return m_next; }
    char id() const { return m_id; }
    const char *name() const { return m_name; }
    int length() const { return m_length; }
    bool in_progress() const { return m_in_progress; }
    bool parse_error() const { return m_parse_error; }
    UINT8 byte(int index) const { return (index >= 0 && index < m_length) ? m_bufptr[index] : 0; }

    // buffer conversion
    const UINT8 *buffer() { return m_bufptr; }
    bool from_buffer(const UINT8 *buffer, int buflen);

    // string conversion
    const char *string(astring &buffer);
    bool from_string(const char *&string,  int length);

    // creation
    virtual void begin() = 0;
    virtual void buffer(const UINT8 *data, UINT32 length) = 0;
    virtual void end() = 0;

protected:
	// internal helpers
    int fromhex(char c);

    // internal state
    hash_base *		m_next;
    const char *	m_name;
    bool			m_in_progress;
    bool			m_parse_error;
    char			m_id;
    UINT8			m_length;
    UINT8 *     	m_bufptr;
};


// ======================> hash_collection

// a collection of the various supported hashes and flags
class hash_collection
{
public:
	// hash types are identified by non-hex alpha values (G-Z)
	static const char HASH_CRC = 'R';
	static const char HASH_SHA1 = 'S';

	// common combinations for requests
	static const char *HASH_TYPES_CRC;
	static const char *HASH_TYPES_CRC_SHA1;
	static const char *HASH_TYPES_ALL;

	// flags are identified by punctuation marks
	static const char FLAG_NO_DUMP = '!';
	static const char FLAG_BAD_DUMP = '^';

	// construction/destruction
    hash_collection();
    hash_collection(const char *string);
    hash_collection(const hash_collection &src);
    ~hash_collection();

    // operators
    hash_collection &operator=(const hash_collection &src);
    bool operator==(const hash_collection &rhs) const;
    bool operator!=(const hash_collection &rhs) const { return !(*this == rhs); }

	// getters
    bool flag(char flag) const { return (m_flags.chr(0, flag) != -1); }
    hash_base *hash(char type) const;
    hash_base *first() const { return m_hashlist.first(); }
    const char *hash_types(astring &buffer) const;
    bool parse_errors() const;

	// hash manipulators
	void reset();
	hash_base *add_from_buffer(char type, const UINT8 *buffer, int bufflen);
	hash_base *add_from_string(char type, const char *buffer, int length);
	bool remove(char type);

	// CRC-specific helpers
	bool crc(UINT32 &result) const;
	hash_base *add_crc(UINT32 crc);

	// string conversion
    const char *internal_string(astring &buffer) const;
    const char *macro_string(astring &buffer) const;
    bool from_internal_string(const char *string);

	// creation
    void begin(const char *types = NULL);
    void buffer(const UINT8 *data, UINT32 length);
    void end();
    void compute(const UINT8 *data, UINT32 length, const char *types = NULL) { begin(types); buffer(data, length); end(); }

private:
	// internal helpers
    static hash_base *alloc_by_id(char id);
    void copyfrom(const hash_collection &src);

	// internal state
	astring					m_flags;
    simple_list<hash_base>	m_hashlist;
};


#endif	/* __HASH_H__ */
