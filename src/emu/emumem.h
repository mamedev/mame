// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.h

    Functions which handle device memory accesses.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_EMUMEM_H
#define MAME_EMU_EMUMEM_H

using s8 = std::int8_t;
using u8 = std::uint8_t;
using s16 = std::int16_t;
using u16 = std::uint16_t;
using s32 = std::int32_t;
using u32 = std::uint32_t;
using s64 = std::int64_t;
using u64 = std::uint64_t;


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum { TOTAL_MEMORY_BANKS = 512 };

// address space names for common use
constexpr int AS_PROGRAM = 0; // program address space
constexpr int AS_DATA    = 1; // data address space
constexpr int AS_IO      = 2; // I/O address space
constexpr int AS_OPCODES = 3; // (decrypted) opcodes, when separate from data accesses

// read or write constants
enum class read_or_write
{
	READ = 1,
	WRITE = 2,
	READWRITE = 3
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// private classes declared in emumem.cpp
class address_table;
class address_table_read;
class address_table_setoffset;
class address_table_write;

// offsets and addresses are 32-bit (for now...)
typedef u32 offs_t;

// address map constructors are delegates that build up an address_map
using address_map_constructor = named_delegate<void (address_map &)>;

// struct with function pointers for accessors; use is generally discouraged unless necessary
struct data_accessors
{
	u8      (*read_byte)(address_space &space, offs_t address);
	u16     (*read_word)(address_space &space, offs_t address);
	u16     (*read_word_masked)(address_space &space, offs_t address, u16 mask);
	u32     (*read_dword)(address_space &space, offs_t address);
	u32     (*read_dword_masked)(address_space &space, offs_t address, u32 mask);
	u64     (*read_qword)(address_space &space, offs_t address);
	u64     (*read_qword_masked)(address_space &space, offs_t address, u64 mask);

	void    (*write_byte)(address_space &space, offs_t address, u8 data);
	void    (*write_word)(address_space &space, offs_t address, u16 data);
	void    (*write_word_masked)(address_space &space, offs_t address, u16 data, u16 mask);
	void    (*write_dword)(address_space &space, offs_t address, u32 data);
	void    (*write_dword_masked)(address_space &space, offs_t address, u32 data, u32 mask);
	void    (*write_qword)(address_space &space, offs_t address, u64 data);
	void    (*write_qword_masked)(address_space &space, offs_t address, u64 data, u64 mask);
};


// ======================> read_delegate

// declare delegates for each width
typedef device_delegate<u8 (address_space &, offs_t, u8)> read8_delegate;
typedef device_delegate<u16 (address_space &, offs_t, u16)> read16_delegate;
typedef device_delegate<u32 (address_space &, offs_t, u32)> read32_delegate;
typedef device_delegate<u64 (address_space &, offs_t, u64)> read64_delegate;


// ======================> write_delegate

// declare delegates for each width
typedef device_delegate<void (address_space &, offs_t, u8, u8)> write8_delegate;
typedef device_delegate<void (address_space &, offs_t, u16, u16)> write16_delegate;
typedef device_delegate<void (address_space &, offs_t, u32, u32)> write32_delegate;
typedef device_delegate<void (address_space &, offs_t, u64, u64)> write64_delegate;

// ======================> setoffset_delegate

typedef device_delegate<void (address_space &, offs_t)> setoffset_delegate;

// =====================-> Width -> types
template<int Width> struct handler_entry_size {};
template<> struct handler_entry_size<0> { typedef u8  uX; typedef read8_delegate  READ; typedef write8_delegate  WRITE; };
template<> struct handler_entry_size<1> { typedef u16 uX; typedef read16_delegate READ; typedef write16_delegate WRITE; };
template<> struct handler_entry_size<2> { typedef u32 uX; typedef read32_delegate READ; typedef write32_delegate WRITE; };
template<> struct handler_entry_size<3> { typedef u64 uX; typedef read64_delegate READ; typedef write64_delegate WRITE; };


// ======================> address offset -> byte offset

constexpr offs_t memory_offset_to_byte(offs_t offset, int AddrShift) { return AddrShift < 0 ? offset << iabs(AddrShift) : offset >> iabs(AddrShift); }

// ======================> generic read/write decomposition routines

// generic direct read
template<int Width, int AddrShift, int Endian, int TargetWidth, bool Aligned, typename T> typename handler_entry_size<TargetWidth>::uX  memory_read_generic(T rop, offs_t address, typename handler_entry_size<TargetWidth>::uX mask)
{
	using TargetType = typename handler_entry_size<TargetWidth>::uX;
	using NativeType = typename handler_entry_size<Width>::uX;

	constexpr u32 TARGET_BYTES = 1 << TargetWidth;
	constexpr u32 TARGET_BITS = 8 * TARGET_BYTES;
	constexpr u32 NATIVE_BYTES = 1 << Width;
	constexpr u32 NATIVE_BITS = 8 * NATIVE_BYTES;
	constexpr u32 NATIVE_STEP = AddrShift >= 0 ? NATIVE_BYTES << iabs(AddrShift) : NATIVE_BYTES >> iabs(AddrShift);
	constexpr u32 NATIVE_MASK = Width + AddrShift >= 0 ? (1 << (Width + AddrShift)) - 1 : 0;

	// equal to native size and aligned; simple pass-through to the native reader
	if (NATIVE_BYTES == TARGET_BYTES && (Aligned || (address & NATIVE_MASK) == 0))
		return rop(address & ~NATIVE_MASK, mask);

	// if native size is larger, see if we can do a single masked read (guaranteed if we're aligned)
	if (NATIVE_BYTES > TARGET_BYTES)
	{
		u32 offsbits = 8 * (memory_offset_to_byte(address, AddrShift) & (NATIVE_BYTES - (Aligned ? TARGET_BYTES : 1)));
		if (Aligned || (offsbits + TARGET_BITS <= NATIVE_BITS))
		{
			if (Endian != ENDIANNESS_LITTLE) offsbits = NATIVE_BITS - TARGET_BITS - offsbits;
			return rop(address & ~NATIVE_MASK, (NativeType)mask << offsbits) >> offsbits;
		}
	}

	// determine our alignment against the native boundaries, and mask the address
	u32 offsbits = 8 * (memory_offset_to_byte(address, AddrShift) & (NATIVE_BYTES - 1));
	address &= ~NATIVE_MASK;

	// if we're here, and native size is larger or equal to the target, we need exactly 2 reads
	if (NATIVE_BYTES >= TARGET_BYTES)
	{
		// little-endian case
		if (Endian == ENDIANNESS_LITTLE)
		{
			// read lower bits from lower address
			TargetType result = 0;
			NativeType curmask = (NativeType)mask << offsbits;
			if (curmask != 0) result = rop(address, curmask) >> offsbits;

			// read upper bits from upper address
			offsbits = NATIVE_BITS - offsbits;
			curmask = mask >> offsbits;
			if (curmask != 0) result |= rop(address + NATIVE_STEP, curmask) << offsbits;
			return result;
		}

		// big-endian case
		else
		{
			// left-justify the mask to the target type
			constexpr u32 LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT = ((NATIVE_BITS >= TARGET_BITS) ? (NATIVE_BITS - TARGET_BITS) : 0);
			NativeType result = 0;
			NativeType ljmask = (NativeType)mask << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
			NativeType curmask = ljmask >> offsbits;

			// read upper bits from lower address
			if (curmask != 0) result = rop(address, curmask) << offsbits;
			offsbits = NATIVE_BITS - offsbits;

			// read lower bits from upper address
			curmask = ljmask << offsbits;
			if (curmask != 0) result |= rop(address + NATIVE_STEP, curmask) >> offsbits;

			// return the un-justified result
			return result >> LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
		}
	}

	// if we're here, then we have 2 or more reads needed to get our final result
	else
	{
		// compute the maximum number of loops; we do it this way so that there are
		// a fixed number of loops for the compiler to unroll if it desires
		constexpr u32 MAX_SPLITS_MINUS_ONE = TARGET_BYTES / NATIVE_BYTES - 1;
		TargetType result = 0;

		// little-endian case
		if (Endian == ENDIANNESS_LITTLE)
		{
				// read lowest bits from first address
			NativeType curmask = mask << offsbits;
			if (curmask != 0) result = rop(address, curmask) >> offsbits;

			// read middle bits from subsequent addresses
			offsbits = NATIVE_BITS - offsbits;
			for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
			{
				address += NATIVE_STEP;
				curmask = mask >> offsbits;
				if (curmask != 0) result |= (TargetType)rop(address, curmask) << offsbits;
				offsbits += NATIVE_BITS;
			}

			// if we're not aligned and we still have bits left, read uppermost bits from last address
			if (!Aligned && offsbits < TARGET_BITS)
			{
				curmask = mask >> offsbits;
				if (curmask != 0) result |= (TargetType)rop(address + NATIVE_STEP, curmask) << offsbits;
			}
		}

		// big-endian case
		else
		{
			// read highest bits from first address
			offsbits = TARGET_BITS - (NATIVE_BITS - offsbits);
			NativeType curmask = mask >> offsbits;
			if (curmask != 0) result = (TargetType)rop(address, curmask) << offsbits;

			// read middle bits from subsequent addresses
			for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
			{
				offsbits -= NATIVE_BITS;
				address += NATIVE_STEP;
				curmask = mask >> offsbits;
				if (curmask != 0) result |= (TargetType)rop(address, curmask) << offsbits;
			}

			// if we're not aligned and we still have bits left, read lowermost bits from the last address
			if (!Aligned && offsbits != 0)
			{
				offsbits = NATIVE_BITS - offsbits;
				curmask = mask << offsbits;
				if (curmask != 0) result |= rop(address + NATIVE_STEP, curmask) >> offsbits;
			}
		}
		return result;
	}
}

// generic direct write
template<int Width, int AddrShift, int Endian, int TargetWidth, bool Aligned, typename T> void memory_write_generic(T wop, offs_t address, typename handler_entry_size<TargetWidth>::uX data, typename handler_entry_size<TargetWidth>::uX mask)
{
	using NativeType = typename handler_entry_size<Width>::uX;

	constexpr u32 TARGET_BYTES = 1 << TargetWidth;
	constexpr u32 TARGET_BITS = 8 * TARGET_BYTES;
	constexpr u32 NATIVE_BYTES = 1 << Width;
	constexpr u32 NATIVE_BITS = 8 * NATIVE_BYTES;
	constexpr u32 NATIVE_STEP = AddrShift >= 0 ? NATIVE_BYTES << iabs(AddrShift) : NATIVE_BYTES >> iabs(AddrShift);
	constexpr u32 NATIVE_MASK = Width + AddrShift >= 0 ? (1 << (Width + AddrShift)) - 1 : 0;

	// equal to native size and aligned; simple pass-through to the native writer
	if (NATIVE_BYTES == TARGET_BYTES && (Aligned || (address & NATIVE_MASK) == 0))
		return wop(address & ~NATIVE_MASK, data, mask);

	// if native size is larger, see if we can do a single masked write (guaranteed if we're aligned)
	if (NATIVE_BYTES > TARGET_BYTES)
	{
		u32 offsbits = 8 * (memory_offset_to_byte(address, AddrShift) & (NATIVE_BYTES - (Aligned ? TARGET_BYTES : 1)));
		if (Aligned || (offsbits + TARGET_BITS <= NATIVE_BITS))
		{
			if (Endian != ENDIANNESS_LITTLE) offsbits = NATIVE_BITS - TARGET_BITS - offsbits;
			return wop(address & ~NATIVE_MASK, (NativeType)data << offsbits, (NativeType)mask << offsbits);
		}
	}

	// determine our alignment against the native boundaries, and mask the address
	u32 offsbits = 8 * (memory_offset_to_byte(address, AddrShift) & (NATIVE_BYTES - 1));
	address &= ~NATIVE_MASK;

	// if we're here, and native size is larger or equal to the target, we need exactly 2 writes
	if (NATIVE_BYTES >= TARGET_BYTES)
	{
		// little-endian case
		if (Endian == ENDIANNESS_LITTLE)
		{
			// write lower bits to lower address
			NativeType curmask = (NativeType)mask << offsbits;
			if (curmask != 0) wop(address, (NativeType)data << offsbits, curmask);

			// write upper bits to upper address
			offsbits = NATIVE_BITS - offsbits;
			curmask = mask >> offsbits;
			if (curmask != 0) wop(address + NATIVE_STEP, data >> offsbits, curmask);
		}

		// big-endian case
		else
		{
			// left-justify the mask and data to the target type
			constexpr u32 LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT = ((NATIVE_BITS >= TARGET_BITS) ? (NATIVE_BITS - TARGET_BITS) : 0);
			NativeType ljdata = (NativeType)data << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
			NativeType ljmask = (NativeType)mask << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
				// write upper bits to lower address
			NativeType curmask = ljmask >> offsbits;
			if (curmask != 0) wop(address, ljdata >> offsbits, curmask);
				// write lower bits to upper address
			offsbits = NATIVE_BITS - offsbits;
			curmask = ljmask << offsbits;
			if (curmask != 0) wop(address + NATIVE_STEP, ljdata << offsbits, curmask);
		}
	}

	// if we're here, then we have 2 or more writes needed to get our final result
	else
	{
		// compute the maximum number of loops; we do it this way so that there are
		// a fixed number of loops for the compiler to unroll if it desires
		constexpr u32 MAX_SPLITS_MINUS_ONE = TARGET_BYTES / NATIVE_BYTES - 1;

		// little-endian case
		if (Endian == ENDIANNESS_LITTLE)
		{
			// write lowest bits to first address
			NativeType curmask = mask << offsbits;
			if (curmask != 0) wop(address, data << offsbits, curmask);

			// write middle bits to subsequent addresses
			offsbits = NATIVE_BITS - offsbits;
			for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
			{
				address += NATIVE_STEP;
				curmask = mask >> offsbits;
				if (curmask != 0) wop(address, data >> offsbits, curmask);
				offsbits += NATIVE_BITS;
			}

			// if we're not aligned and we still have bits left, write uppermost bits to last address
			if (!Aligned && offsbits < TARGET_BITS)
			{
				curmask = mask >> offsbits;
				if (curmask != 0) wop(address + NATIVE_STEP, data >> offsbits, curmask);
			}
		}

		// big-endian case
		else
		{
			// write highest bits to first address
			offsbits = TARGET_BITS - (NATIVE_BITS - offsbits);
			NativeType curmask = mask >> offsbits;
			if (curmask != 0) wop(address, data >> offsbits, curmask);

			// write middle bits to subsequent addresses
			for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
			{
				offsbits -= NATIVE_BITS;
				address += NATIVE_STEP;
				curmask = mask >> offsbits;
				if (curmask != 0) wop(address, data >> offsbits, curmask);
			}

			// if we're not aligned and we still have bits left, write lowermost bits to the last address
			if (!Aligned && offsbits != 0)
			{
				offsbits = NATIVE_BITS - offsbits;
				curmask = mask << offsbits;
				if (curmask != 0) wop(address + NATIVE_STEP, data << offsbits, curmask);
			}
		}
	}
}


// ======================> memory_access_cache

// memory_access_cache contains state data for cached access
template<int Width, int AddrShift, int Endian> class memory_access_cache
{
	friend class address_table;
	using NativeType = typename handler_entry_size<Width>::uX;
	static constexpr u32 NATIVE_BYTES = 1 << Width;
	static constexpr u32 NATIVE_MASK = Width + AddrShift >= 0 ? (1 << (Width + AddrShift)) - 1 : 0;

public:
	using cache_update_delegate = delegate<offs_t (memory_access_cache<Width, AddrShift, Endian> &, offs_t)>;

	// cache_range is an internal class that is part of a list of start/end ranges
	class cache_range
	{
	public:
		// construction
		cache_range(): m_addrstart(0),m_addrend(~0) { }

		inline bool operator==(cache_range val) noexcept
		{   // return true if _Left and _Right identify the same thread
			return (m_addrstart == val.m_addrstart) && (m_addrend == val.m_addrend);
		}

		// internal state
		offs_t                  m_addrstart;            // starting offset of the range
		offs_t                  m_addrend;              // ending offset of the range
	};

	// construction/destruction
	memory_access_cache(address_space &space);
	~memory_access_cache();

	// getters
	address_space &space() const { return m_space; }
	u8 *ptr() const { return m_ptr; }

	// see if an address is within bounds, or attempt to update it if not
	bool address_is_valid(offs_t address) { return EXPECTED(address >= m_addrstart && address <= m_addrend) || set_cache_region(address); }

	// force a recomputation on the next read
	void force_update() { m_addrend = 0; m_addrstart = 1; }
	void force_update(u16 if_match) { if (m_entry == if_match) force_update(); }

	// accessor methods

	void *read_ptr(offs_t address) {
		if (address_is_valid(address))
			return &m_ptr[memory_offset_to_byte(address & m_addrmask, AddrShift)];
		return nullptr;
	}

	u8 read_byte(offs_t address) { return Width == 0 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xff); }
	u16 read_word(offs_t address) { return Width == 1 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_word(offs_t address, u16 mask) { return memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u16 read_word_unaligned(offs_t address) { return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_word_unaligned(offs_t address, u16 mask) { return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_dword(offs_t address) { return Width == 2 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_dword(offs_t address, u32 mask) { return memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_dword_unaligned(offs_t address) { return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_dword_unaligned(offs_t address, u32 mask) { return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_qword(offs_t address) { return Width == 3 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_qword(offs_t address, u64 mask) { return memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_qword_unaligned(offs_t address) { return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_qword_unaligned(offs_t address, u64 mask) { return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }


	void remove_intersecting_ranges(offs_t start, offs_t end);

private:
	// internal helpers
	bool set_cache_region(offs_t address);
	cache_range *find_range(offs_t address, u16 &entry);

	// internal state
	address_space &             m_space;
	u8 *                        m_ptr;                  // cache access data pointer
	offs_t                      m_addrmask;             // address mask
	offs_t                      m_addrstart;            // minimum valid address
	offs_t                      m_addrend;              // maximum valid address
	u16                         m_entry;                // live entry
	std::list<cache_range>     m_rangelist[TOTAL_MEMORY_BANKS];  // list of ranges for each entry

	NativeType read_native(offs_t address, NativeType mask = ~NativeType(0));
};


// ======================> address_space_config

// describes an address space and provides basic functions to map addresses to bytes
class address_space_config
{
public:
	// construction/destruction
	address_space_config();
	address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift = 0, address_map_constructor internal = address_map_constructor(), address_map_constructor defmap = address_map_constructor());
	address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, u8 logwidth, u8 pageshift, address_map_constructor internal = address_map_constructor(), address_map_constructor defmap = address_map_constructor());

	// getters
	const char *name() const { return m_name; }
	endianness_t endianness() const { return m_endianness; }
	int data_width() const { return m_data_width; }
	int addr_width() const { return m_addr_width; }
	int addr_shift() const { return m_addr_shift; }

	// Actual alignment of the bus addresses
	int alignment() const { int bytes = m_data_width / 8; return m_addr_shift < 0 ? bytes >> -m_addr_shift : bytes << m_addr_shift; }

	// Address delta to byte delta helpers
	inline offs_t addr2byte(offs_t address) const { return (m_addr_shift < 0) ? (address << -m_addr_shift) : (address >> m_addr_shift); }
	inline offs_t byte2addr(offs_t address) const { return (m_addr_shift > 0) ? (address << m_addr_shift) : (address >> -m_addr_shift); }

	// address-to-byte conversion helpers
	inline offs_t addr2byte_end(offs_t address) const { return (m_addr_shift < 0) ? ((address << -m_addr_shift) | ((1 << -m_addr_shift) - 1)) : (address >> m_addr_shift); }
	inline offs_t byte2addr_end(offs_t address) const { return (m_addr_shift > 0) ? ((address << m_addr_shift) | ((1 << m_addr_shift) - 1)) : (address >> -m_addr_shift); }

	// state
	const char *        m_name;
	endianness_t        m_endianness;
	u8                  m_data_width;
	u8                  m_addr_width;
	s8                  m_addr_shift;
	u8                  m_logaddr_width;
	u8                  m_page_shift;
	bool                m_is_octal;                 // to determine if messages/debugger will show octal or hex

	address_map_constructor m_internal_map;
	address_map_constructor m_default_map;
};


// ======================> address_space

// address_space holds live information about an address space
class address_space
{
	friend class address_table;
	friend class address_table_read;
	friend class address_table_write;
	friend class address_table_setoffset;
	template<int Width, int AddrShift, int Endian> friend class memory_access_cache;
	friend class memory_bank;
	friend class memory_block;

protected:
	// construction/destruction
	address_space(memory_manager &manager, device_memory_interface &memory, int spacenum, bool large);

public:
	virtual ~address_space();

	// getters
	device_t &device() const { return m_device; }
	const char *name() const { return m_name; }
	int spacenum() const { return m_spacenum; }
	address_map *map() const { return m_map.get(); }

	template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian> *cache() const {
		if(AddrShift != m_config.addr_shift())
			fatalerror("Requesting cache() with address shift %d while the config says %d\n", AddrShift, m_config.addr_shift());
		if(8 << Width != m_config.data_width())
			fatalerror("Requesting cache() with data width %d while the config says %d\n", 8 << Width, m_config.data_width());
		if(Endian != m_config.endianness())
			fatalerror("Requesting cache() with endianness %s while the config says %s\n",
					   endianness_names[Endian], endianness_names[m_config.endianness()]);

		return static_cast<memory_access_cache<Width, AddrShift, Endian> *>(m_cache);
	}

	int data_width() const { return m_config.data_width(); }
	int addr_width() const { return m_config.addr_width(); }
	int alignment() const { return m_config.alignment(); }
	endianness_t endianness() const { return m_config.endianness(); }
	int addr_shift() const { return m_config.addr_shift(); }
	u64 unmap() const { return m_unmap; }
	bool is_octal() const { return m_config.m_is_octal; }

	offs_t addrmask() const { return m_addrmask; }
	u8 addrchars() const { return m_addrchars; }
	offs_t logaddrmask() const { return m_logaddrmask; }
	u8 logaddrchars() const { return m_logaddrchars; }

	// debug helpers
	const char *get_handler_string(read_or_write readorwrite, offs_t byteaddress);
	bool log_unmap() const { return m_log_unmap; }
	void set_log_unmap(bool log) { m_log_unmap = log; }
	void dump_map(FILE *file, read_or_write readorwrite);

	// watchpoint enablers
	virtual void enable_read_watchpoints(bool enable = true) = 0;
	virtual void enable_write_watchpoints(bool enable = true) = 0;

	// general accessors
	virtual void accessors(data_accessors &accessors) const = 0;
	virtual void *get_read_ptr(offs_t address) = 0;
	virtual void *get_write_ptr(offs_t address) = 0;

	// read accessors
	template<int AccessWidth> inline typename handler_entry_size<AccessWidth>::uX read_sized(offs_t address, typename handler_entry_size<AccessWidth>::uX mask);

	virtual u8 read_byte(offs_t address) = 0;
	virtual u16 read_word(offs_t address) = 0;
	virtual u16 read_word(offs_t address, u16 mask) = 0;
	virtual u16 read_word_unaligned(offs_t address) = 0;
	virtual u16 read_word_unaligned(offs_t address, u16 mask) = 0;
	virtual u32 read_dword(offs_t address) = 0;
	virtual u32 read_dword(offs_t address, u32 mask) = 0;
	virtual u32 read_dword_unaligned(offs_t address) = 0;
	virtual u32 read_dword_unaligned(offs_t address, u32 mask) = 0;
	virtual u64 read_qword(offs_t address) = 0;
	virtual u64 read_qword(offs_t address, u64 mask) = 0;
	virtual u64 read_qword_unaligned(offs_t address) = 0;
	virtual u64 read_qword_unaligned(offs_t address, u64 mask) = 0;

	// write accessors
	virtual void write_byte(offs_t address, u8 data) = 0;
	virtual void write_word(offs_t address, u16 data) = 0;
	virtual void write_word(offs_t address, u16 data, u16 mask) = 0;
	virtual void write_word_unaligned(offs_t address, u16 data) = 0;
	virtual void write_word_unaligned(offs_t address, u16 data, u16 mask) = 0;
	virtual void write_dword(offs_t address, u32 data) = 0;
	virtual void write_dword(offs_t address, u32 data, u32 mask) = 0;
	virtual void write_dword_unaligned(offs_t address, u32 data) = 0;
	virtual void write_dword_unaligned(offs_t address, u32 data, u32 mask) = 0;
	virtual void write_qword(offs_t address, u64 data) = 0;
	virtual void write_qword(offs_t address, u64 data, u64 mask) = 0;
	virtual void write_qword_unaligned(offs_t address, u64 data) = 0;
	virtual void write_qword_unaligned(offs_t address, u64 data, u64 mask) = 0;

	// Set address. This will invoke setoffset handlers for the respective entries.
	virtual void set_address(offs_t address) = 0;

	// address-to-byte conversion helpers
	offs_t address_to_byte(offs_t address) const { return m_config.addr2byte(address); }
	offs_t address_to_byte_end(offs_t address) const { return m_config.addr2byte_end(address); }
	offs_t byte_to_address(offs_t address) const { return m_config.byte2addr(address); }
	offs_t byte_to_address_end(offs_t address) const { return m_config.byte2addr_end(address); }

	// umap ranges (short form)
	void unmap_read(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READ, false); }
	void unmap_write(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::WRITE, false); }
	void unmap_readwrite(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READWRITE, false); }
	void nop_read(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READ, true); }
	void nop_write(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::WRITE, true); }
	void nop_readwrite(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READWRITE, true); }

	// install ports, banks, RAM (short form)
	void install_read_port(offs_t addrstart, offs_t addrend, const char *rtag) { install_read_port(addrstart, addrend, 0, rtag); }
	void install_write_port(offs_t addrstart, offs_t addrend, const char *wtag) { install_write_port(addrstart, addrend, 0, wtag); }
	void install_readwrite_port(offs_t addrstart, offs_t addrend, const char *rtag, const char *wtag) { install_readwrite_port(addrstart, addrend, 0, rtag, wtag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_read_bank(addrstart, addrend, 0, tag); }
	void install_write_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_write_bank(addrstart, addrend, 0, tag); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_readwrite_bank(addrstart, addrend, 0, tag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_read_bank(addrstart, addrend, 0, bank); }
	void install_write_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_write_bank(addrstart, addrend, 0, bank); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_readwrite_bank(addrstart, addrend, 0, bank); }
	void install_rom(offs_t addrstart, offs_t addrend, void *baseptr = nullptr) { install_rom(addrstart, addrend, 0, baseptr); }
	void install_writeonly(offs_t addrstart, offs_t addrend, void *baseptr = nullptr) { install_writeonly(addrstart, addrend, 0, baseptr); }
	void install_ram(offs_t addrstart, offs_t addrend, void *baseptr = nullptr) { install_ram(addrstart, addrend, 0, baseptr); }

	// install ports, banks, RAM (with mirror/mask)
	void install_read_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag) { install_readwrite_port(addrstart, addrend, addrmirror, rtag, nullptr); }
	void install_write_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *wtag) { install_readwrite_port(addrstart, addrend, addrmirror, nullptr, wtag); }
	void install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_read_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *tag) { install_bank_generic(addrstart, addrend, addrmirror, tag, nullptr); }
	void install_write_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *tag) { install_bank_generic(addrstart, addrend, addrmirror, nullptr, tag); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *tag)  { install_bank_generic(addrstart, addrend, addrmirror, tag, tag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *bank) { install_bank_generic(addrstart, addrend, addrmirror, bank, nullptr); }
	void install_write_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *bank) { install_bank_generic(addrstart, addrend, addrmirror, nullptr, bank); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *bank)  { install_bank_generic(addrstart, addrend, addrmirror, bank, bank); }
	void install_rom(offs_t addrstart, offs_t addrend, offs_t addrmirror, void *baseptr = nullptr) { install_ram_generic(addrstart, addrend, addrmirror, read_or_write::READ, baseptr); }
	void install_writeonly(offs_t addrstart, offs_t addrend, offs_t addrmirror, void *baseptr = nullptr) { install_ram_generic(addrstart, addrend, addrmirror, read_or_write::WRITE, baseptr); }
	void install_ram(offs_t addrstart, offs_t addrend, offs_t addrmirror, void *baseptr = nullptr) { install_ram_generic(addrstart, addrend, addrmirror, read_or_write::READWRITE, baseptr); }

	// install device memory maps
	template <typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(address_map &map), u64 unitmask = 0, int cswidth = 0) {
		address_map_constructor delegate(map, "dynamic_device_install", &device);
		install_device_delegate(addrstart, addrend, device, delegate, unitmask, cswidth);
	}

	void install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &map, u64 unitmask = 0, int cswidth = 0);

	// install setoffset handler
	void install_setoffset_handler(offs_t addrstart, offs_t addrend, setoffset_delegate sohandler, u64 unitmask = 0, int cswidth = 0) { install_setoffset_handler(addrstart, addrend, 0, 0, 0, sohandler, unitmask, cswidth); }
	void install_setoffset_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, setoffset_delegate sohandler, u64 unitmask = 0, int cswidth = 0);

	// install new-style delegate handlers (short form)
	void install_read_handler(offs_t addrstart, offs_t addrend, read8_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read8_delegate rhandler, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }
	void install_read_handler(offs_t addrstart, offs_t addrend, read16_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read16_delegate rhandler, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }
	void install_read_handler(offs_t addrstart, offs_t addrend, read32_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read32_delegate rhandler, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }
	void install_read_handler(offs_t addrstart, offs_t addrend, read64_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read64_delegate rhandler, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }

	// install new-style delegate handlers (with mirror/mask)
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0);

	// setup
	void prepare_map();
	void populate_from_map(address_map *map = nullptr);
	void allocate_memory();
	void locate_memory();

	void invalidate_read_caches();
	void invalidate_read_caches(u16 entry);
	void invalidate_read_caches(offs_t start, offs_t end);

private:
	// internal helpers
	virtual address_table_read &read() = 0;
	virtual address_table_write &write() = 0;
	virtual address_table_setoffset &setoffset() = 0;

	void populate_map_entry(const address_map_entry &entry, read_or_write readorwrite);
	void populate_map_entry_setoffset(const address_map_entry &entry);
	void unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, bool quiet);
	void install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, void *baseptr);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *rbank, memory_bank *wbank);
	void adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror);
	void *find_backing_memory(offs_t addrstart, offs_t addrend);
	bool needs_backing_store(const address_map_entry &entry);
	memory_bank &bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite);
	memory_bank *bank_find_anonymous(offs_t bytestart, offs_t byteend) const;
	address_map_entry *block_assign_intersecting(offs_t bytestart, offs_t byteend, u8 *base);
	void check_optimize_all(const char *function, int width, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror, u64 &nunitmask, int &ncswidth);
	void check_optimize_mirror(const char *function, offs_t addrstart, offs_t addrend, offs_t addrmirror, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror);
	void check_address(const char *function, offs_t addrstart, offs_t addrend);

protected:
	// private state
	const address_space_config &m_config;       // configuration of this space
	device_t &              m_device;           // reference to the owning device
	std::unique_ptr<address_map> m_map;         // original memory map
	offs_t                  m_addrmask;         // physical address mask
	offs_t                  m_logaddrmask;      // logical address mask
	u64                     m_unmap;            // unmapped value
	int                     m_spacenum;         // address space index
	bool                    m_log_unmap;        // log unmapped accesses in this space?
	void *                  m_cache;           // fast cache-access read info
	const char *            m_name;             // friendly name of the address space
	u8                      m_addrchars;        // number of characters to use for physical addresses
	u8                      m_logaddrchars;     // number of characters to use for logical addresses
	std::function<void *(void *)> m_cache_alloc;
	std::function<void (void *)> m_cache_delete;
	std::function<void (void *)> m_cache_invalidate_read_caches;
	std::function<void (void *, u16)> m_cache_invalidate_read_cache_entry;
	std::function<void (void *, offs_t, offs_t)> m_cache_invalidate_read_cache_range;

	template<int Width, int AddrShift, int Endian> void cache_init();

private:
	memory_manager &        m_manager;          // reference to the owning manager
};


// ======================> memory_block

// a memory block is a chunk of RAM associated with a range of memory in a device's address space
class memory_block
{
	DISABLE_COPYING(memory_block);

public:
	// construction/destruction
	memory_block(address_space &space, offs_t start, offs_t end, void *memory = nullptr);
	~memory_block();

	// getters
	running_machine &machine() const { return m_machine; }
	offs_t addrstart() const { return m_addrstart; }
	offs_t addrend() const { return m_addrend; }
	u8 *data() const { return m_data; }

	// is the given range contained by this memory block?
	bool contains(address_space &space, offs_t addrstart, offs_t addrend) const
	{
		return (&space == &m_space && m_addrstart <= addrstart && m_addrend >= addrend);
	}

private:
	// internal state
	running_machine &       m_machine;              // need the machine to free our memory
	address_space &         m_space;                // which address space are we associated with?
	offs_t                  m_addrstart, m_addrend; // start/end for verifying a match
	u8 *                    m_data;                 // pointer to the data for this block
	std::vector<u8>         m_allocated;            // pointer to the actually allocated block
};


// ======================> memory_bank

// a memory bank is a global pointer to memory that can be shared across devices and changed dynamically
class memory_bank
{
	// a bank reference is an entry in a list of address spaces that reference a given bank
	class bank_reference
	{
	public:
		// construction/destruction
		bank_reference(address_space &space, read_or_write readorwrite)
			: m_space(space),
				m_readorwrite(readorwrite) { }

		// getters
		address_space &space() const { return m_space; }

		// does this reference match the space+read/write combination?
		bool matches(const address_space &space, read_or_write readorwrite) const
		{
			return (&space == &m_space && (readorwrite == read_or_write::READWRITE || readorwrite == m_readorwrite));
		}

	private:
		// internal state
		address_space &         m_space;            // address space that references us
		read_or_write           m_readorwrite;      // used for read or write?
	};

	// a bank_entry contains a pointer
	struct bank_entry
	{
		u8 *    m_ptr;
	};

public:
	// construction/destruction
	memory_bank(address_space &space, int index, offs_t start, offs_t end, const char *tag = nullptr);
	~memory_bank();

	// getters
	running_machine &machine() const { return m_machine; }
	int index() const { return m_index; }
	int entry() const { return m_curentry; }
	bool anonymous() const { return m_anonymous; }
	offs_t addrstart() const { return m_addrstart; }
	void *base() const { return *m_baseptr; }
	const char *tag() const { return m_tag.c_str(); }
	const char *name() const { return m_name.c_str(); }

	// compare a range against our range
	bool matches_exactly(offs_t addrstart, offs_t addrend) const { return (m_addrstart == addrstart && m_addrend == addrend); }
	bool fully_covers(offs_t addrstart, offs_t addrend) const { return (m_addrstart <= addrstart && m_addrend >= addrend); }
	bool is_covered_by(offs_t addrstart, offs_t addrend) const { return (m_addrstart >= addrstart && m_addrend <= addrend); }
	bool straddles(offs_t addrstart, offs_t addrend) const { return (m_addrstart < addrend && m_addrend > addrstart); }

	// track and verify address space references to this bank
	bool references_space(const address_space &space, read_or_write readorwrite) const;
	void add_reference(address_space &space, read_or_write readorwrite);

	// set the base explicitly
	void set_base(void *base);

	// configure and set entries
	void configure_entry(int entrynum, void *base);
	void configure_entries(int startentry, int numentries, void *base, offs_t stride);
	void set_entry(int entrynum);

private:
	// internal helpers
	void invalidate_references();
	void expand_entries(int entrynum);

	// internal state
	running_machine &       m_machine;              // need the machine to free our memory
	u8 **                   m_baseptr;              // pointer to our base pointer in the global array
	u16                     m_index;                // array index for this handler
	bool                    m_anonymous;            // are we anonymous or explicit?
	offs_t                  m_addrstart;            // start offset
	offs_t                  m_addrend;              // end offset
	int                     m_curentry;             // current entry
	std::vector<bank_entry> m_entry;                // array of entries (dynamically allocated)
	std::string             m_name;                 // friendly name for this bank
	std::string             m_tag;                  // tag for this bank
	std::vector<std::unique_ptr<bank_reference>> m_reflist;          // linked list of address spaces referencing this bank
};


// ======================> memory_share

// a memory share contains information about shared memory region
class memory_share
{
public:
	// construction/destruction
	memory_share(u8 width, size_t bytes, endianness_t endianness, void *ptr = nullptr)
		: m_ptr(ptr),
			m_bytes(bytes),
			m_endianness(endianness),
			m_bitwidth(width),
			m_bytewidth(width <= 8 ? 1 : width <= 16 ? 2 : width <= 32 ? 4 : 8)
	{ }

	// getters
	void *ptr() const { return m_ptr; }
	size_t bytes() const { return m_bytes; }
	endianness_t endianness() const { return m_endianness; }
	u8 bitwidth() const { return m_bitwidth; }
	u8 bytewidth() const { return m_bytewidth; }

	// setters
	void set_ptr(void *ptr) { m_ptr = ptr; }

private:
	// internal state
	void *                  m_ptr;                  // pointer to the memory backing the region
	size_t                  m_bytes;                // size of the shared region in bytes
	endianness_t            m_endianness;           // endianness of the memory
	u8                      m_bitwidth;             // width of the shared region in bits
	u8                      m_bytewidth;            // width in bytes, rounded up to a power of 2

};


// ======================> memory_region

// memory region object
class memory_region
{
	DISABLE_COPYING(memory_region);

	friend class memory_manager;
public:
	// construction/destruction
	memory_region(running_machine &machine, const char *name, u32 length, u8 width, endianness_t endian);

	// getters
	running_machine &machine() const { return m_machine; }
	u8 *base() { return (m_buffer.size() > 0) ? &m_buffer[0] : nullptr; }
	u8 *end() { return base() + m_buffer.size(); }
	u32 bytes() const { return m_buffer.size(); }
	const char *name() const { return m_name.c_str(); }

	// flag expansion
	endianness_t endianness() const { return m_endianness; }
	u8 bitwidth() const { return m_bitwidth; }
	u8 bytewidth() const { return m_bytewidth; }

	// data access
	u8 &as_u8(offs_t offset = 0) { return m_buffer[offset]; }
	u16 &as_u16(offs_t offset = 0) { return reinterpret_cast<u16 *>(base())[offset]; }
	u32 &as_u32(offs_t offset = 0) { return reinterpret_cast<u32 *>(base())[offset]; }
	u64 &as_u64(offs_t offset = 0) { return reinterpret_cast<u64 *>(base())[offset]; }

private:
	// internal data
	running_machine &       m_machine;
	std::string             m_name;
	std::vector<u8>         m_buffer;
	endianness_t            m_endianness;
	u8                      m_bitwidth;
	u8                      m_bytewidth;
};



// ======================> memory_manager

// holds internal state for the memory system
class memory_manager
{
	friend class address_space;
	friend memory_region::memory_region(running_machine &machine, const char *name, u32 length, u8 width, endianness_t endian);
public:
	// construction/destruction
	memory_manager(running_machine &machine);
	void initialize();

	// getters
	running_machine &machine() const { return m_machine; }
	const std::unordered_map<std::string, std::unique_ptr<memory_bank>> &banks() const { return m_banklist; }
	const std::unordered_map<std::string, std::unique_ptr<memory_region>> &regions() const { return m_regionlist; }
	const std::unordered_map<std::string, std::unique_ptr<memory_share>> &shares() const { return m_sharelist; }

	// pointers to a bank pointer (internal usage only)
	u8 **bank_pointer_addr(u8 index) { return &m_bank_ptr[index]; }

	// regions
	memory_region *region_alloc(const char *name, u32 length, u8 width, endianness_t endian);
	void region_free(const char *name);
	memory_region *region_containing(const void *memory, offs_t bytes) const;

private:
	// internal helpers
	void bank_reattach();
	void allocate(device_memory_interface &memory);

	// internal state
	running_machine &           m_machine;              // reference to the machine
	bool                        m_initialized;          // have we completed initialization?

	u8 *                        m_bank_ptr[TOTAL_MEMORY_BANKS];  // array of bank pointers

	std::vector<std::unique_ptr<memory_block>>   m_blocklist;            // head of the list of memory blocks

	std::unordered_map<std::string,std::unique_ptr<memory_bank>>    m_banklist;             // data gathered for each bank
	u16                      m_banknext;             // next bank to allocate

	std::unordered_map<std::string, std::unique_ptr<memory_share>>   m_sharelist;            // map for share lookups

	std::unordered_map<std::string, std::unique_ptr<memory_region>>  m_regionlist;           // list of memory regions
};



//**************************************************************************
//  MACROS
//**************************************************************************

// space read/write handler function macros
#define READ8_MEMBER(name)              u8     name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 mem_mask)
#define WRITE8_MEMBER(name)             void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 data, ATTR_UNUSED u8 mem_mask)
#define READ16_MEMBER(name)             u16    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 mem_mask)
#define WRITE16_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 data, ATTR_UNUSED u16 mem_mask)
#define READ32_MEMBER(name)             u32    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 mem_mask)
#define WRITE32_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 data, ATTR_UNUSED u32 mem_mask)
#define READ64_MEMBER(name)             u64    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 mem_mask)
#define WRITE64_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 data, ATTR_UNUSED u64 mem_mask)

#define DECLARE_READ8_MEMBER(name)      u8     name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 mem_mask = 0xff)
#define DECLARE_WRITE8_MEMBER(name)     void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 data, ATTR_UNUSED u8 mem_mask = 0xff)
#define DECLARE_READ16_MEMBER(name)     u16    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 mem_mask = 0xffff)
#define DECLARE_WRITE16_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 data, ATTR_UNUSED u16 mem_mask = 0xffff)
#define DECLARE_READ32_MEMBER(name)     u32    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 mem_mask = 0xffffffff)
#define DECLARE_WRITE32_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 data, ATTR_UNUSED u32 mem_mask = 0xffffffff)
#define DECLARE_READ64_MEMBER(name)     u64    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 mem_mask = 0xffffffffffffffffU)
#define DECLARE_WRITE64_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 data, ATTR_UNUSED u64 mem_mask = 0xffffffffffffffffU)

#define SETOFFSET_MEMBER(name)          void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset)
#define DECLARE_SETOFFSET_MEMBER(name)  void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset)

// device delegate macros
#define READ8_DELEGATE(_class, _member)                     read8_delegate(FUNC(_class::_member), this)
#define WRITE8_DELEGATE(_class, _member)                    write8_delegate(FUNC(_class::_member), this)
#define READ16_DELEGATE(_class, _member)                    read16_delegate(FUNC(_class::_member), this)
#define WRITE16_DELEGATE(_class, _member)                   write16_delegate(FUNC(_class::_member), this)
#define READ32_DELEGATE(_class, _member)                    read32_delegate(FUNC(_class::_member), this)
#define WRITE32_DELEGATE(_class, _member)                   write32_delegate(FUNC(_class::_member), this)
#define READ64_DELEGATE(_class, _member)                    read64_delegate(FUNC(_class::_member), this)
#define WRITE64_DELEGATE(_class, _member)                   write64_delegate(FUNC(_class::_member), this)

#define READ8_DEVICE_DELEGATE(_device, _class, _member)     read8_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE8_DEVICE_DELEGATE(_device, _class, _member)    write8_delegate(FUNC(_class::_member), (_class *)_device)
#define READ16_DEVICE_DELEGATE(_device, _class, _member)    read16_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE16_DEVICE_DELEGATE(_device, _class, _member)   write16_delegate(FUNC(_class::_member), (_class *)_device)
#define READ32_DEVICE_DELEGATE(_device, _class, _member)    read32_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE32_DEVICE_DELEGATE(_device, _class, _member)   write32_delegate(FUNC(_class::_member), (_class *)_device)
#define READ64_DEVICE_DELEGATE(_device, _class, _member)    read64_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE64_DEVICE_DELEGATE(_device, _class, _member)   write64_delegate(FUNC(_class::_member), (_class *)_device)


// helper macro for merging data with the memory mask
#define COMBINE_DATA(varptr)            (*(varptr) = (*(varptr) & ~mem_mask) | (data & mem_mask))

#define ACCESSING_BITS_0_7              ((mem_mask & 0x000000ffU) != 0)
#define ACCESSING_BITS_8_15             ((mem_mask & 0x0000ff00U) != 0)
#define ACCESSING_BITS_16_23            ((mem_mask & 0x00ff0000U) != 0)
#define ACCESSING_BITS_24_31            ((mem_mask & 0xff000000U) != 0)
#define ACCESSING_BITS_32_39            ((mem_mask & 0x000000ff00000000U) != 0)
#define ACCESSING_BITS_40_47            ((mem_mask & 0x0000ff0000000000U) != 0)
#define ACCESSING_BITS_48_55            ((mem_mask & 0x00ff000000000000U) != 0)
#define ACCESSING_BITS_56_63            ((mem_mask & 0xff00000000000000U) != 0)

#define ACCESSING_BITS_0_15             ((mem_mask & 0x0000ffffU) != 0)
#define ACCESSING_BITS_16_31            ((mem_mask & 0xffff0000U) != 0)
#define ACCESSING_BITS_32_47            ((mem_mask & 0x0000ffff00000000U) != 0)
#define ACCESSING_BITS_48_63            ((mem_mask & 0xffff000000000000U) != 0)

#define ACCESSING_BITS_0_31             ((mem_mask & 0xffffffffU) != 0)
#define ACCESSING_BITS_32_63            ((mem_mask & 0xffffffff00000000U) != 0)


// macros for accessing bytes and words within larger chunks

// read/write a byte to a 16-bit space
#define BYTE_XOR_BE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0))
#define BYTE_XOR_LE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1))

// read/write a byte to a 32-bit space
#define BYTE4_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0))
#define BYTE4_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3))

// read/write a word to a 32-bit space
#define WORD_XOR_BE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))
#define WORD_XOR_LE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,2))

// read/write a byte to a 64-bit space
#define BYTE8_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(7,0))
#define BYTE8_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,7))

// read/write a word to a 64-bit space
#define WORD2_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0))
#define WORD2_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6))

// read/write a dword to a 64-bit space
#define DWORD_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0))
#define DWORD_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4))


// helpers for checking address alignment
#define WORD_ALIGNED(a)                 (((a) & 1) == 0)
#define DWORD_ALIGNED(a)                (((a) & 3) == 0)
#define QWORD_ALIGNED(a)                (((a) & 7) == 0)

// inline templates
template<> inline u8  address_space::read_sized<0>(offs_t address, u8  mask) { return read_byte(address); }
template<> inline u16 address_space::read_sized<1>(offs_t address, u16 mask) { return read_word(address, mask); }
template<> inline u32 address_space::read_sized<2>(offs_t address, u32 mask) { return read_dword(address, mask); }
template<> inline u64 address_space::read_sized<3>(offs_t address, u64 mask) { return read_qword(address, mask); }

template<int Width, int AddrShift, int Endian> typename handler_entry_size<Width>::uX memory_access_cache<Width, AddrShift, Endian>::read_native(offs_t address, typename handler_entry_size<Width>::uX mask)
{
	if (address_is_valid(address))
		return *reinterpret_cast<NativeType *>(&m_ptr[memory_offset_to_byte(address & m_addrmask, AddrShift)]);
	return m_space.read_sized<Width>(address, mask);
}

#endif  /* MAME_EMU_EMUMEM_H */
