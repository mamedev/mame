// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    memarray.cpp

    Generic memory array accessor helper.

***************************************************************************/

#include "emu.h"
#include "memarray.h"


//**************************************************************************
//  MEMORY ARRAY HELPER
//**************************************************************************

//-------------------------------------------------
//  memory_array - constructor
//-------------------------------------------------

memory_array::memory_array()
	: m_base(nullptr),
		m_bytes(0),
		m_membits(0),
		m_endianness(ENDIANNESS_LITTLE),
		m_bytes_per_entry(0),
		m_read_entry(nullptr),
		m_write_entry(nullptr)
{
}


//-------------------------------------------------
//  set - configure the parameters
//-------------------------------------------------

void memory_array::set(void *base, u32 bytes, int membits, endianness_t endianness, int bpe)
{
	// validate inputs
	assert(base != nullptr);
	assert(bytes > 0);
	assert(membits == 8 || membits == 16 || membits == 32 || membits == 64);
	assert(bpe == 1 || bpe == 2 || bpe == 4);

	// populate direct data
	m_base = base;
	m_bytes = bytes;
	m_membits = membits;
	m_endianness = endianness;
	m_bytes_per_entry = bpe;

	// derive data
	switch (bpe*1000 + membits*10 + (endianness == ENDIANNESS_LITTLE ? 0 : 1))
	{
		case 1*1000 + 8*10 + 0:     m_read_entry = &memory_array::read8_from_8;       m_write_entry = &memory_array::write8_to_8;        break;
		case 1*1000 + 8*10 + 1:     m_read_entry = &memory_array::read8_from_8;       m_write_entry = &memory_array::write8_to_8;        break;
		case 1*1000 + 16*10 + 0:    m_read_entry = &memory_array::read8_from_16le;    m_write_entry = &memory_array::write8_to_16le;     break;
		case 1*1000 + 16*10 + 1:    m_read_entry = &memory_array::read8_from_16be;    m_write_entry = &memory_array::write8_to_16be;     break;
		case 1*1000 + 32*10 + 0:    m_read_entry = &memory_array::read8_from_32le;    m_write_entry = &memory_array::write8_to_32le;     break;
		case 1*1000 + 32*10 + 1:    m_read_entry = &memory_array::read8_from_32be;    m_write_entry = &memory_array::write8_to_32be;     break;
		case 1*1000 + 64*10 + 0:    m_read_entry = &memory_array::read8_from_64le;    m_write_entry = &memory_array::write8_to_64le;     break;
		case 1*1000 + 64*10 + 1:    m_read_entry = &memory_array::read8_from_64be;    m_write_entry = &memory_array::write8_to_64be;     break;

		case 2*1000 + 8*10 + 0:     m_read_entry = &memory_array::read16_from_8le;    m_write_entry = &memory_array::write16_to_8le;     break;
		case 2*1000 + 8*10 + 1:     m_read_entry = &memory_array::read16_from_8be;    m_write_entry = &memory_array::write16_to_8be;     break;
		case 2*1000 + 16*10 + 0:    m_read_entry = &memory_array::read16_from_16;     m_write_entry = &memory_array::write16_to_16;      break;
		case 2*1000 + 16*10 + 1:    m_read_entry = &memory_array::read16_from_16;     m_write_entry = &memory_array::write16_to_16;      break;
		case 2*1000 + 32*10 + 0:    m_read_entry = &memory_array::read16_from_32le;   m_write_entry = &memory_array::write16_to_32le;    break;
		case 2*1000 + 32*10 + 1:    m_read_entry = &memory_array::read16_from_32be;   m_write_entry = &memory_array::write16_to_32be;    break;
		case 2*1000 + 64*10 + 0:    m_read_entry = &memory_array::read16_from_64le;   m_write_entry = &memory_array::write16_to_64le;    break;
		case 2*1000 + 64*10 + 1:    m_read_entry = &memory_array::read16_from_64be;   m_write_entry = &memory_array::write16_to_64be;    break;

		case 4*1000 + 8*10 + 0:     m_read_entry = &memory_array::read32_from_8le;    m_write_entry = &memory_array::write32_to_8le;     break;
		case 4*1000 + 8*10 + 1:     m_read_entry = &memory_array::read32_from_8be;    m_write_entry = &memory_array::write32_to_8be;     break;
		case 4*1000 + 16*10 + 0:    m_read_entry = &memory_array::read32_from_16le;   m_write_entry = &memory_array::write32_to_16le;    break;
		case 4*1000 + 16*10 + 1:    m_read_entry = &memory_array::read32_from_16be;   m_write_entry = &memory_array::write32_to_16be;    break;
		case 4*1000 + 32*10 + 0:    m_read_entry = &memory_array::read32_from_32;     m_write_entry = &memory_array::write32_to_32;      break;
		case 4*1000 + 32*10 + 1:    m_read_entry = &memory_array::read32_from_32;     m_write_entry = &memory_array::write32_to_32;      break;
		case 4*1000 + 64*10 + 0:    m_read_entry = &memory_array::read32_from_64le;   m_write_entry = &memory_array::write32_to_64le;    break;
		case 4*1000 + 64*10 + 1:    m_read_entry = &memory_array::read32_from_64be;   m_write_entry = &memory_array::write32_to_64be;    break;

		default:    throw emu_fatalerror("Illegal memory bits/bus width combo in memory_array");
	}
}


//-------------------------------------------------
//  set - additional setter variants
//-------------------------------------------------

void memory_array::set(const memory_share &share, int bpe)
{
	set(share.ptr(), share.bytes(), share.bitwidth(), share.endianness(), bpe);
}

void memory_array::set(const memory_array &array)
{
	set(array.base(), array.bytes(), array.membits(), array.endianness(), array.bytes_per_entry());
}


//-------------------------------------------------
//  piecewise configuration
//-------------------------------------------------

void memory_array::set_membits(int membits)
{
	set(m_base, m_bytes, membits, m_endianness, m_bytes_per_entry);
}

void memory_array::set_endianness(endianness_t endianness)
{
	set(m_base, m_bytes, m_membits, endianness, m_bytes_per_entry);
}


//-------------------------------------------------
//  read8_from_*/write8_to_* - entry read/write
//  helpers for 1 byte-per-entry
//-------------------------------------------------

u32 memory_array::read8_from_8(int index) const { return reinterpret_cast<u8 *>(m_base)[index]; }
void memory_array::write8_to_8(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[index] = data; }

u32 memory_array::read8_from_16le(int index) const { return reinterpret_cast<u8 *>(m_base)[BYTE_XOR_LE(index)]; }
void memory_array::write8_to_16le(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[BYTE_XOR_LE(index)] = data; }
u32 memory_array::read8_from_16be(int index) const { return reinterpret_cast<u8 *>(m_base)[BYTE_XOR_BE(index)]; }
void memory_array::write8_to_16be(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[BYTE_XOR_BE(index)] = data; }

u32 memory_array::read8_from_32le(int index) const { return reinterpret_cast<u8 *>(m_base)[BYTE4_XOR_LE(index)]; }
void memory_array::write8_to_32le(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[BYTE4_XOR_LE(index)] = data; }
u32 memory_array::read8_from_32be(int index) const { return reinterpret_cast<u8 *>(m_base)[BYTE4_XOR_BE(index)]; }
void memory_array::write8_to_32be(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[BYTE4_XOR_BE(index)] = data; }

u32 memory_array::read8_from_64le(int index) const { return reinterpret_cast<u8 *>(m_base)[BYTE8_XOR_BE(index)]; }
void memory_array::write8_to_64le(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[BYTE8_XOR_BE(index)] = data; }
u32 memory_array::read8_from_64be(int index) const { return reinterpret_cast<u8 *>(m_base)[BYTE8_XOR_BE(index)]; }
void memory_array::write8_to_64be(int index, u32 data) { reinterpret_cast<u8 *>(m_base)[BYTE8_XOR_BE(index)] = data; }


//-------------------------------------------------
//  read16_from_*/write16_to_* - entry read/write
//  helpers for 2 bytes-per-entry
//-------------------------------------------------

u32 memory_array::read16_from_8le(int index) const { return read8_from_8(index*2) | (read8_from_8(index*2+1) << 8); }
void memory_array::write16_to_8le(int index, u32 data) { write8_to_8(index*2, data); write8_to_8(index*2+1, data >> 8); }
u32 memory_array::read16_from_8be(int index) const { return (read8_from_8(index*2) << 8) | read8_from_8(index*2+1); }
void memory_array::write16_to_8be(int index, u32 data) { write8_to_8(index*2, data >> 8); write8_to_8(index*2+1, data); }

u32 memory_array::read16_from_16(int index) const { return reinterpret_cast<u16 *>(m_base)[index]; }
void memory_array::write16_to_16(int index, u32 data) { reinterpret_cast<u16 *>(m_base)[index] = data; }

u32 memory_array::read16_from_32le(int index) const { return reinterpret_cast<u16 *>(m_base)[BYTE_XOR_LE(index)]; }
void memory_array::write16_to_32le(int index, u32 data) { reinterpret_cast<u16 *>(m_base)[BYTE_XOR_LE(index)] = data; }
u32 memory_array::read16_from_32be(int index) const { return reinterpret_cast<u16 *>(m_base)[BYTE_XOR_BE(index)]; }
void memory_array::write16_to_32be(int index, u32 data) { reinterpret_cast<u16 *>(m_base)[BYTE_XOR_BE(index)] = data; }

u32 memory_array::read16_from_64le(int index) const { return reinterpret_cast<u16 *>(m_base)[BYTE4_XOR_LE(index)]; }
void memory_array::write16_to_64le(int index, u32 data) { reinterpret_cast<u16 *>(m_base)[BYTE4_XOR_LE(index)] = data; }
u32 memory_array::read16_from_64be(int index) const { return reinterpret_cast<u16 *>(m_base)[BYTE4_XOR_BE(index)]; }
void memory_array::write16_to_64be(int index, u32 data) { reinterpret_cast<u16 *>(m_base)[BYTE4_XOR_BE(index)] = data; }


//-------------------------------------------------
//  read32_from_*/write32_to_* - entry read/write
//  helpers for 4 bytes-per-entry
//-------------------------------------------------

u32 memory_array::read32_from_8le(int index) const { return read16_from_8le(index*2) | (read16_from_8le(index*2+1) << 16); }
void memory_array::write32_to_8le(int index, u32 data) { write16_to_8le(index*2, data); write16_to_8le(index*2+1, data >> 16); }
u32 memory_array::read32_from_8be(int index) const { return (read16_from_8be(index*2) << 16) | read16_from_8be(index*2+1); }
void memory_array::write32_to_8be(int index, u32 data) { write16_to_8be(index*2, data >> 16); write16_to_8be(index*2+1, data); }

u32 memory_array::read32_from_16le(int index) const { return read16_from_16(index*2) | (read16_from_16(index*2+1) << 16); }
void memory_array::write32_to_16le(int index, u32 data) { write16_to_16(index*2, data); write16_to_16(index*2+1, data >> 16); }
u32 memory_array::read32_from_16be(int index) const { return (read16_from_16(index*2) << 16) | read16_from_16(index*2+1); }
void memory_array::write32_to_16be(int index, u32 data) { write16_to_16(index*2, data >> 16); write16_to_16(index*2+1, data); }

u32 memory_array::read32_from_32(int index) const { return reinterpret_cast<u32 *>(m_base)[index]; }
void memory_array::write32_to_32(int index, u32 data) { reinterpret_cast<u32 *>(m_base)[index] = data; }

u32 memory_array::read32_from_64le(int index) const { return reinterpret_cast<u32 *>(m_base)[BYTE_XOR_LE(index)]; }
void memory_array::write32_to_64le(int index, u32 data) { reinterpret_cast<u32 *>(m_base)[BYTE_XOR_LE(index)] = data; }
u32 memory_array::read32_from_64be(int index) const { return reinterpret_cast<u32 *>(m_base)[BYTE_XOR_BE(index)]; }
void memory_array::write32_to_64be(int index, u32 data) { reinterpret_cast<u32 *>(m_base)[BYTE_XOR_BE(index)] = data; }
