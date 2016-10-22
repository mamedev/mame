// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    memarray.h

    Generic memory array accessor helper.

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
	memory_array(void *base, uint32_t bytes, int membits, endianness_t endianness, int bpe) { set(base, bytes, membits, endianness, bpe); }
	template <typename _Type> memory_array(std::vector<_Type> &array, endianness_t endianness, int bpe) { set(array, endianness, bpe); }
	memory_array(const address_space &space, void *base, uint32_t bytes, int bpe) { set(space, base, bytes, bpe); }
	memory_array(const memory_share &share, int bpe) { set(share, bpe); }
	memory_array(const memory_array &array) { set(array); }

	// configuration
	void set(void *base, uint32_t bytes, int membits, endianness_t endianness, int bpe);
	template <typename _Type> void set(std::vector<_Type> &array, endianness_t endianness, int bpe) { set(&array[0], array.size(), 8*sizeof(_Type), endianness, bpe); }
	void set(const address_space &space, void *base, uint32_t bytes, int bpe);
	void set(const memory_share &share, int bpe);
	void set(const memory_array &array);

	// piecewise configuration
	void set_membits(int membits);
	void set_endianness(endianness_t endianness);

	// getters
	void *base() const { return m_base; }
	uint32_t bytes() const { return m_bytes; }
	int membits() const { return m_membits; }
	endianness_t endianness() const { return m_endianness; }
	int bytes_per_entry() const { return m_bytes_per_entry; }

	// entry-level readers and writers
	uint32_t read(int index) const { return (this->*m_read_entry)(index); }
	void write(int index, uint32_t data) { (this->*m_write_entry)(index, data); }

	// byte/word/dword-level readers and writers
	uint8_t read8(offs_t offset) const { return reinterpret_cast<uint8_t *>(m_base)[offset]; }
	uint16_t read16(offs_t offset) const { return reinterpret_cast<uint16_t *>(m_base)[offset]; }
	uint32_t read32(offs_t offset) const { return reinterpret_cast<uint32_t *>(m_base)[offset]; }
	uint64_t read64(offs_t offset) const { return reinterpret_cast<uint64_t *>(m_base)[offset]; }
	void write8(offs_t offset, uint8_t data) { reinterpret_cast<uint8_t *>(m_base)[offset] = data; }
	void write16(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) { COMBINE_DATA(&reinterpret_cast<uint16_t *>(m_base)[offset]); }
	void write32(offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff) { COMBINE_DATA(&reinterpret_cast<uint32_t *>(m_base)[offset]); }
	void write64(offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff)) { COMBINE_DATA(&reinterpret_cast<uint64_t *>(m_base)[offset]); }

private:
	// internal read/write helpers for 1 byte entries
	uint32_t read8_from_8(int index) const;     void write8_to_8(int index, uint32_t data);
	uint32_t read8_from_16le(int index) const;  void write8_to_16le(int index, uint32_t data);
	uint32_t read8_from_16be(int index) const;  void write8_to_16be(int index, uint32_t data);
	uint32_t read8_from_32le(int index) const;  void write8_to_32le(int index, uint32_t data);
	uint32_t read8_from_32be(int index) const;  void write8_to_32be(int index, uint32_t data);
	uint32_t read8_from_64le(int index) const;  void write8_to_64le(int index, uint32_t data);
	uint32_t read8_from_64be(int index) const;  void write8_to_64be(int index, uint32_t data);

	// internal read/write helpers for 2 byte entries
	uint32_t read16_from_8le(int index) const;  void write16_to_8le(int index, uint32_t data);
	uint32_t read16_from_8be(int index) const;  void write16_to_8be(int index, uint32_t data);
	uint32_t read16_from_16(int index) const;   void write16_to_16(int index, uint32_t data);
	uint32_t read16_from_32le(int index) const; void write16_to_32le(int index, uint32_t data);
	uint32_t read16_from_32be(int index) const; void write16_to_32be(int index, uint32_t data);
	uint32_t read16_from_64le(int index) const; void write16_to_64le(int index, uint32_t data);
	uint32_t read16_from_64be(int index) const; void write16_to_64be(int index, uint32_t data);

	// internal read/write helpers for 4 byte entries
	uint32_t read32_from_8le(int index) const;  void write32_to_8le(int index, uint32_t data);
	uint32_t read32_from_8be(int index) const;  void write32_to_8be(int index, uint32_t data);
	uint32_t read32_from_16le(int index) const; void write32_to_16le(int index, uint32_t data);
	uint32_t read32_from_16be(int index) const; void write32_to_16be(int index, uint32_t data);
	uint32_t read32_from_32(int index) const;   void write32_to_32(int index, uint32_t data);
	uint32_t read32_from_64le(int index) const; void write32_to_64le(int index, uint32_t data);
	uint32_t read32_from_64be(int index) const; void write32_to_64be(int index, uint32_t data);

	// internal state
	void *              m_base;
	uint32_t              m_bytes;
	int                 m_membits;
	endianness_t        m_endianness;
	int                 m_bytes_per_entry;
	uint32_t (memory_array::*m_read_entry)(int) const;
	void (memory_array::*m_write_entry)(int, uint32_t);
};



#endif  // __MEMARRAY_H__
