/***************************************************************************

    bitstream.h

    Helper classes for reading/writing at the bit level.

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

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "osdcore.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// helper class for reading from a bit buffer
class bitstream_in
{
public:
	// construction/destruction
	bitstream_in(const void *src, UINT32 srclength);

	// getters
	bool overflow() const { return ((m_doffset - m_bits / 8) > m_dlength); }
	UINT32 read_offset() const;

	// operations
	UINT32 read(int numbits);
	UINT32 peek(int numbits);
	void remove(int numbits);
	UINT32 flush();

private:
	// internal state
	UINT32			m_buffer;		// current bit accumulator
	int				m_bits;			// number of bits in the accumulator
	const UINT8 *	m_read;			// read pointer
	UINT32			m_doffset;		// byte offset within the data
	UINT32			m_dlength;		// length of the data
};


// helper class for writing to a bit buffer
class bitstream_out
{
public:
	// construction/destruction
	bitstream_out(void *dest, UINT32 destlength);

	// getters
	bool overflow() const { return (m_doffset > m_dlength); }

	// operations
	void write(UINT32 newbits, int numbits);
	UINT32 flush();

private:
	// internal state
	UINT32			m_buffer;			// current bit accumulator
	int				m_bits;				// number of bits in the accumulator
	UINT8 *			m_write;			// write pointer
	UINT32			m_doffset;			// byte offset within the data
	UINT32			m_dlength;			// length of the data
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  bitstream_in - constructor
//-------------------------------------------------

inline bitstream_in::bitstream_in(const void *src, UINT32 srclength)
	: m_buffer(0),
	  m_bits(0),
	  m_read(reinterpret_cast<const UINT8 *>(src)),
	  m_doffset(0),
	  m_dlength(srclength)
{
}


//-------------------------------------------------
//  peek - fetch the requested number of bits
//  but don't advance the input pointer
//-------------------------------------------------

inline UINT32 bitstream_in::peek(int numbits)
{
	// fetch data if we need more
	if (numbits > m_bits)
	{
		while (m_bits <= 24)
		{
			if (m_doffset < m_dlength)
				m_buffer |= m_read[m_doffset] << (24 - m_bits);
			m_doffset++;
			m_bits += 8;
		}
	}

	// return the data
	return m_buffer >> (32 - numbits);
}


//-------------------------------------------------
//  remove - advance the input pointer by the
//  specified number of bits
//-------------------------------------------------

inline void bitstream_in::remove(int numbits)
{
	m_buffer <<= numbits;
	m_bits -= numbits;
}


//-------------------------------------------------
//  read - fetch the requested number of bits
//-------------------------------------------------

inline UINT32 bitstream_in::read(int numbits)
{
	UINT32 result = peek(numbits);
	remove(numbits);
	return result;
}


//-------------------------------------------------
//  read_offset - return the current read offset
//-------------------------------------------------

inline UINT32 bitstream_in::read_offset() const
{
	UINT32 result = m_doffset;
	int bits = m_bits;
	while (bits >= 8)
	{
		result--;
		bits -= 8;
	}
	return result;
}


//-------------------------------------------------
//  flush - flush to the nearest byte
//-------------------------------------------------

inline UINT32 bitstream_in::flush()
{
	while (m_bits >= 8)
	{
		m_doffset--;
		m_bits -= 8;
	}
	m_bits = m_buffer = 0;
	return m_doffset;
}


//-------------------------------------------------
//  bitstream_out - constructor
//-------------------------------------------------

inline bitstream_out::bitstream_out(void *dest, UINT32 destlength)
	: m_buffer(0),
	  m_bits(0),
	  m_write(reinterpret_cast<UINT8 *>(dest)),
	  m_doffset(0),
	  m_dlength(destlength)
{
}



//-------------------------------------------------
//  write - write the given number of bits to the
//  data stream
//-------------------------------------------------

inline void bitstream_out::write(UINT32 newbits, int numbits)
{
	// flush the buffer if we're going to overflow it
	if (m_bits + numbits > 32)
		while (m_bits >= 8)
		{
			if (m_doffset < m_dlength)
				m_write[m_doffset] = m_buffer >> 24;
			m_doffset++;
			m_buffer <<= 8;
			m_bits -= 8;
		}

	// shift the bits to the top
	newbits <<= 32 - numbits;

	// now shift it down to account for the number of bits we already have and OR them in
	m_buffer |= newbits >> m_bits;
	m_bits += numbits;
}


//-------------------------------------------------
//  flush - output remaining bits and return the
//  final output size in bytes
//-------------------------------------------------

inline UINT32 bitstream_out::flush()
{
	while (m_bits > 0)
	{
		if (m_doffset < m_dlength)
			m_write[m_doffset] = m_buffer >> 24;
		m_doffset++;
		m_buffer <<= 8;
		m_bits -= 8;
	}
	m_bits = m_buffer = 0;
	return m_doffset;
}


#endif
