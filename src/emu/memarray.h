// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    memarray.h

    Generic memory array accessor helper.

****************************************************************************

    A memory array in this case is an array of 8, 16, or 32-bit data
    arranged logically.

    A memory array is stored in "natural" order, i.e., read/writes to it
    are done via ram(), or standard COMBINE_DATA, even if the width of
    the CPU is different from the array width.

    The read_entry/write_entry functions serve to read/write entries of
    the configured size regardless of the underlay width of the CPU's
    memory system.

***************************************************************************/

#pragma once

#ifndef MAME_EMU_MEMARRAY_H
#define MAME_EMU_MEMARRAY_H


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
	memory_array(void *base, u32 bytes, int membits, endianness_t endianness, int bpe) { set(base, bytes, membits, endianness, bpe); }
	template <typename _Type> memory_array(std::vector<_Type> &array, endianness_t endianness, int bpe) { set(array, endianness, bpe); }
	memory_array(const memory_share &share, int bpe) { set(share, bpe); }
	memory_array(const memory_array &array) { set(array); }

	// configuration
	void set(void *base, u32 bytes, int membits, endianness_t endianness, int bpe);
	template <typename _Type> void set(std::vector<_Type> &array, endianness_t endianness, int bpe) { set(&array[0], array.size(), 8*sizeof(_Type), endianness, bpe); }
	void set(const memory_share &share, int bpe);
	void set(const memory_array &array);

	// piecewise configuration
	void set_membits(int membits);
	void set_endianness(endianness_t endianness);

	// getters
	void *base() const { return m_base; }
	u32 bytes() const { return m_bytes; }
	int membits() const { return m_membits; }
	endianness_t endianness() const { return m_endianness; }
	int bytes_per_entry() const { return m_bytes_per_entry; }

	// entry-level readers and writers
	u32 read(int index) const { return (this->*m_read_entry)(index); }
	void write(int index, u32 data) { (this->*m_write_entry)(index, data); }

	// byte/word/dword-level readers and writers
	u8 read8(offs_t offset) const { return reinterpret_cast<u8 *>(m_base)[offset]; }
	u16 read16(offs_t offset) const { return reinterpret_cast<u16 *>(m_base)[offset]; }
	u32 read32(offs_t offset) const { return reinterpret_cast<u32 *>(m_base)[offset]; }
	u64 read64(offs_t offset) const { return reinterpret_cast<u64 *>(m_base)[offset]; }
	void write8(offs_t offset, u8 data) { reinterpret_cast<u8 *>(m_base)[offset] = data; }
	void write16(offs_t offset, u16 data, u16 mem_mask = 0xffff) { COMBINE_DATA(&reinterpret_cast<u16 *>(m_base)[offset]); }
	void write32(offs_t offset, u32 data, u32 mem_mask = 0xffffffff) { COMBINE_DATA(&reinterpret_cast<u32 *>(m_base)[offset]); }
	void write64(offs_t offset, u64 data, u64 mem_mask = 0xffffffffffffffffU) { COMBINE_DATA(&reinterpret_cast<u64 *>(m_base)[offset]); }

private:
	// internal read/write helpers for 1 byte entries
	u32 read8_from_8(int index) const;     void write8_to_8(int index, u32 data);
	u32 read8_from_16le(int index) const;  void write8_to_16le(int index, u32 data);
	u32 read8_from_16be(int index) const;  void write8_to_16be(int index, u32 data);
	u32 read8_from_32le(int index) const;  void write8_to_32le(int index, u32 data);
	u32 read8_from_32be(int index) const;  void write8_to_32be(int index, u32 data);
	u32 read8_from_64le(int index) const;  void write8_to_64le(int index, u32 data);
	u32 read8_from_64be(int index) const;  void write8_to_64be(int index, u32 data);

	// internal read/write helpers for 2 byte entries
	u32 read16_from_8le(int index) const;  void write16_to_8le(int index, u32 data);
	u32 read16_from_8be(int index) const;  void write16_to_8be(int index, u32 data);
	u32 read16_from_16(int index) const;   void write16_to_16(int index, u32 data);
	u32 read16_from_32le(int index) const; void write16_to_32le(int index, u32 data);
	u32 read16_from_32be(int index) const; void write16_to_32be(int index, u32 data);
	u32 read16_from_64le(int index) const; void write16_to_64le(int index, u32 data);
	u32 read16_from_64be(int index) const; void write16_to_64be(int index, u32 data);

	// internal read/write helpers for 4 byte entries
	u32 read32_from_8le(int index) const;  void write32_to_8le(int index, u32 data);
	u32 read32_from_8be(int index) const;  void write32_to_8be(int index, u32 data);
	u32 read32_from_16le(int index) const; void write32_to_16le(int index, u32 data);
	u32 read32_from_16be(int index) const; void write32_to_16be(int index, u32 data);
	u32 read32_from_32(int index) const;   void write32_to_32(int index, u32 data);
	u32 read32_from_64le(int index) const; void write32_to_64le(int index, u32 data);
	u32 read32_from_64be(int index) const; void write32_to_64be(int index, u32 data);

	// internal state
	void *              m_base;
	u32                 m_bytes;
	int                 m_membits;
	endianness_t        m_endianness;
	int                 m_bytes_per_entry;
	u32 (memory_array::*m_read_entry)(int) const;
	void (memory_array::*m_write_entry)(int, u32);
};



#endif  // MAME_EMU_MEMARRAY_H
