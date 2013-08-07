/***************************************************************************

    memarray.h

    Generic memory array accessor helper.

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

****************************************************************************

	A memory array in this case is an array of 8, 16, or 32-bit data
	arranged logically.
	
	A memory array is stored in "natural" order, i.e., read/writes to it 
	are done via AM_RAM, or standard COMBINE_DATA, even if the width of
	the CPU is different from the array width.
	
	The read_entry/write_entry functions serve to read/write entries of
	the configured size regardless of the underlay width of the CPU's
	memory system.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MEMARRAY_H__
#define __MEMARRAY_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> memory_array

// memory information
class memory_array
{
public:
	// construction/destruction
	memory_array();
	memory_array(void *base, UINT32 bytes, int membits, endianness_t endianness, int bpe) { set(base, bytes, membits, endianness, bpe); }
	memory_array(const address_space &space, void *base, UINT32 bytes, int bpe) { set(space, base, bytes, bpe); }
	memory_array(const memory_share &share, int bpe) { set(share, bpe); }
	memory_array(const memory_array &helper) { set(helper); }

	// configuration
	void set(void *base, UINT32 bytes, int membits, endianness_t endianness, int bpe);
	void set(const address_space &space, void *base, UINT32 bytes, int bpe);
	void set(const memory_share &share, int bpe);
	void set(const memory_array &helper);

	// getters
	void *base() const { return m_base; }
	UINT32 bytes() const { return m_bytes; }
	int membits() const { return m_membits; }
	endianness_t endianness() const { return m_endianness; }
	int bytes_per_entry() const { return m_bytes_per_entry; }

	// readers and writers
	UINT32 read(int index) { return (this->*m_reader)(index); }
	void write(int index, UINT32 data) { (this->*m_writer)(index, data); }

private:
	// internal read/write helpers for 1 byte entries
	UINT32 read8_from_8(int index);     void write8_to_8(int index, UINT32 data);
	UINT32 read8_from_16le(int index);  void write8_to_16le(int index, UINT32 data);
	UINT32 read8_from_16be(int index);  void write8_to_16be(int index, UINT32 data);
	UINT32 read8_from_32le(int index);  void write8_to_32le(int index, UINT32 data);
	UINT32 read8_from_32be(int index);  void write8_to_32be(int index, UINT32 data);
	UINT32 read8_from_64le(int index);  void write8_to_64le(int index, UINT32 data);
	UINT32 read8_from_64be(int index);  void write8_to_64be(int index, UINT32 data);

	// internal read/write helpers for 2 byte entries
	UINT32 read16_from_8le(int index);  void write16_to_8le(int index, UINT32 data);
	UINT32 read16_from_8be(int index);  void write16_to_8be(int index, UINT32 data);
	UINT32 read16_from_16(int index);   void write16_to_16(int index, UINT32 data);
	UINT32 read16_from_32le(int index); void write16_to_32le(int index, UINT32 data);
	UINT32 read16_from_32be(int index); void write16_to_32be(int index, UINT32 data);
	UINT32 read16_from_64le(int index); void write16_to_64le(int index, UINT32 data);
	UINT32 read16_from_64be(int index); void write16_to_64be(int index, UINT32 data);

	// internal read/write helpers for 4 byte entries
	UINT32 read32_from_8le(int index);  void write32_to_8le(int index, UINT32 data);
	UINT32 read32_from_8be(int index);  void write32_to_8be(int index, UINT32 data);
	UINT32 read32_from_16le(int index); void write32_to_16le(int index, UINT32 data);
	UINT32 read32_from_16be(int index); void write32_to_16be(int index, UINT32 data);
	UINT32 read32_from_32(int index);   void write32_to_32(int index, UINT32 data);
	UINT32 read32_from_64le(int index); void write32_to_64le(int index, UINT32 data);
	UINT32 read32_from_64be(int index); void write32_to_64be(int index, UINT32 data);

	// internal state
	void *              m_base;
	UINT32              m_bytes;
	int                 m_membits;
	endianness_t        m_endianness;
	int                 m_bytes_per_entry;
	UINT32 (memory_array::*m_reader)(int);
	void (memory_array::*m_writer)(int, UINT32);
};



#endif  // __MEMARRAY_H__
