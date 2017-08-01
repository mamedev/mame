// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dimemory.c

    Device memory interfaces.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "debug/debugcpu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

//const int TRIGGER_SUSPENDTIME = -4000;



//**************************************************************************
//  ADDRESS SPACE CONFIG
//**************************************************************************

//-------------------------------------------------
//  address_space_config - constructors
//-------------------------------------------------

address_space_config::address_space_config()
	: m_name("unknown"),
		m_endianness(ENDIANNESS_NATIVE),
		m_databus_width(0),
		m_addrbus_width(0),
		m_addrbus_shift(0),
		m_logaddr_width(0),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(nullptr),
		m_default_map(nullptr)
{
}

/*!
 @param name
 @param endian CPU endianness
 @param datawidth CPU parallelism bits
 @param addrwidth address bits
 @param addrshift
 @param internal
 @param defmap
 */
address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(internal),
		m_default_map(defmap)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, u8 logwidth, u8 pageshift, address_map_constructor internal, address_map_constructor defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_is_octal(false),
		m_internal_map(internal),
		m_default_map(defmap)
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, address_map_delegate internal, address_map_delegate defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(nullptr),
		m_default_map(nullptr),
		m_internal_map_delegate(std::move(internal)),
		m_default_map_delegate(std::move(defmap))
{
}

address_space_config::address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, u8 logwidth, u8 pageshift, address_map_delegate internal, address_map_delegate defmap)
	: m_name(name),
		m_endianness(endian),
		m_databus_width(datawidth),
		m_addrbus_width(addrwidth),
		m_addrbus_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_is_octal(false),
		m_internal_map(nullptr),
		m_default_map(nullptr),
		m_internal_map_delegate(std::move(internal)),
		m_default_map_delegate(std::move(defmap))
{
}


//**************************************************************************
//  MEMORY DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_memory_interface - constructor
//-------------------------------------------------

device_memory_interface::device_memory_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "memory")
{
	// configure the fast accessor
	device.interfaces().m_memory = this;
}


//-------------------------------------------------
//  ~device_memory_interface - destructor
//-------------------------------------------------

device_memory_interface::~device_memory_interface()
{
}


//-------------------------------------------------
//  static_set_addrmap - configuration helper
//  to connect an address map to a device
//-------------------------------------------------

void device_memory_interface::static_set_addrmap(device_t &device, int spacenum, address_map_constructor map)
{
	device_memory_interface *memory;
	if (!device.interface(memory))
		throw emu_fatalerror("MCFG_DEVICE_ADDRESS_MAP called on device '%s' with no memory interface", device.tag());
	memory->set_addrmap(spacenum, map);
}


//-------------------------------------------------
//  set_addrmap - connect an address map to a device
//-------------------------------------------------

void device_memory_interface::set_addrmap(int spacenum, address_map_constructor map)
{
	if (spacenum >= int(m_address_map.size()))
		m_address_map.resize(spacenum+1, nullptr);
	m_address_map[spacenum] = map;
}


//-------------------------------------------------
//  dump - dump memory tables to the given file in
//  human-readable format
//-------------------------------------------------

void device_memory_interface::dump(FILE *file) const
{
	for (auto const &space : m_addrspace)
		if (space) {
			fprintf(file,
					"\n\n"
					"====================================================\n"
					"Device '%s' %s address space read handler dump\n"
					"====================================================\n",
					device().tag(), space->name());
			space->dump_map(file, read_or_write::READ);

			fprintf(file,
					"\n\n"
					"====================================================\n"
					"Device '%s' %s address space write handler dump\n"
					"====================================================\n",
					device().tag(), space->name());
			space->dump_map(file, read_or_write::WRITE);
		}
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  phyiscal addresses; designed to be overridden
//  by the actual device implementation if address
//  translation is supported
//-------------------------------------------------

int device_memory_interface::memory_translate(int spacenum, int intention, offs_t &address) const
{
	// by default it maps directly
	return spacenum;
}


//-------------------------------------------------
//  interface_config_complete - perform final
//  memory configuration setup
//-------------------------------------------------

void device_memory_interface::interface_config_complete()
{
	const space_config_vector r = memory_space_config();
	for (const auto &entry : r) {
		if (entry.first >= int(m_address_config.size()))
			m_address_config.resize(entry.first + 1);
		m_address_config[entry.first] = entry.second;
	}
}


//-------------------------------------------------
//  interface_validity_check - perform validity
//  checks on the memory configuration
//-------------------------------------------------

void device_memory_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all address spaces
	for (int spacenum = 0; spacenum < int(m_address_config.size()); ++spacenum)
	{
		if (space_config(spacenum))
		{
			// construct the map
			::address_map addrmap(const_cast<device_t &>(device()), spacenum);

			// let the map check itself
			addrmap.map_validity_check(valid, spacenum);
		}
	}
}



//**************************************************************************
//  DEBUGGER MEMORY ACCESSORS
//**************************************************************************

//-------------------------------------------------
//  read_byte - return a byte from the specified
//  memory space
//-------------------------------------------------

u8 device_memory_interface::read_byte(int spacenum, offs_t address, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	// translate if necessary
	if (intention != TRANSLATE_NONE)
	{
		assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_READ);
		spacenum = translate(spacenum, intention, address);

		// if not mapped, return 0xff
		if (spacenum == AS_INVALID)
			return 0xff;
	}

	// otherwise, call the byte reading function for the translated address
	return space(spacenum).read_byte(address);
}


//-------------------------------------------------
//  read_word - return a word from the specified
//  memory space
//-------------------------------------------------

u16 device_memory_interface::read_word(int spacenum, offs_t address, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	if (!WORD_ALIGNED(address))
	{
		// if this is misaligned read, or if there are no word readers, just read two bytes
		u8 byte0 = read_byte(spacenum, address + 0, intention);
		u8 byte1 = read_byte(spacenum, address + 1, intention);

		// based on the endianness, the result is assembled differently
		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
			return byte0 | (byte1 << 8);
		else
			return byte1 | (byte0 << 8);
	}
	else
	{
		// otherwise, this proceeds like the byte case
		if (intention != TRANSLATE_NONE)
		{
			// translate if necessary
			assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_READ);
			spacenum = translate(spacenum, intention, address);

			// if not mapped, return 0xffff
			if (spacenum == AS_INVALID)
				return 0xffff;
		}

		// otherwise, call the byte reading function for the translated address
		return space(spacenum).read_word(address);
	}
}


//-------------------------------------------------
//  read_dword - return a dword from the specified
//  memory space
//-------------------------------------------------

u32 device_memory_interface::read_dword(int spacenum, offs_t address, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	if (!DWORD_ALIGNED(address))
	{
		// if this is a misaligned read, or if there are no dword readers, just read two words
		u16 word0 = read_word(spacenum, address + 0, intention);
		u16 word1 = read_word(spacenum, address + 2, intention);

		// based on the endianness, the result is assembled differently
		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
			return word0 | (word1 << 16);
		else
			return word1 | (word0 << 16);
	}
	else
	{
		// otherwise, this proceeds like the byte case
		if (intention != TRANSLATE_NONE)
		{
			// translate if necessary
			assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_READ);
			spacenum = translate(spacenum, intention, address);

			// if not mapped, return 0xffffffff
			if (spacenum == AS_INVALID)
				return 0xffffffff;
		}

		// otherwise, call the byte reading function for the translated address
		return space(spacenum).read_dword(address);
	}
}


//-------------------------------------------------
//  read_qword - return a qword from the specified
//  memory space
//-------------------------------------------------

u64 device_memory_interface::read_qword(int spacenum, offs_t address, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	if (!QWORD_ALIGNED(address))
	{
		// if this is a misaligned read, or if there are no qword readers, just read two dwords
		u32 dword0 = read_dword(spacenum, address + 0, intention);
		u32 dword1 = read_dword(spacenum, address + 4, intention);

		// based on the endianness, the result is assembled differently
		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
			return dword0 | (u64(dword1) << 32);
		else
			return dword1 | (u64(dword0) << 32);
	}
	else
	{
		// otherwise, this proceeds like the byte case
		if (intention != TRANSLATE_NONE)
		{
			// translate if necessary
			assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_READ);
			spacenum = translate(spacenum, intention, address);

			// if not mapped, return 0xffffffffffffffff
			if (spacenum == AS_INVALID)
				return ~u64(0);
		}

		// otherwise, call the byte reading function for the translated address
		return space(spacenum).read_qword(address);
	}
}


//-------------------------------------------------
//  read_memory - return 1,2,4 or 8 bytes
//  from the specified memory space
//-------------------------------------------------

u64 device_memory_interface::read_memory(int spacenum, offs_t address, int size, int intention)
{
	u64 result = ~u64(0) >> (64 - 8*size);
	switch (size)
	{
		case 1:     result = read_byte(spacenum, address, intention);    break;
		case 2:     result = read_word(spacenum, address, intention);    break;
		case 4:     result = read_dword(spacenum, address, intention);   break;
		case 8:     result = read_qword(spacenum, address, intention);   break;
	}
	return result;
}


//-------------------------------------------------
//  write_byte - write a byte to the specified
//  memory space
//-------------------------------------------------

void device_memory_interface::write_byte(int spacenum, offs_t address, u8 data, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	// translate if necessary
	if (intention != TRANSLATE_NONE)
	{
		assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_WRITE);
		spacenum = translate(spacenum, intention, address);

		// if not mapped, we're done
		if (spacenum == AS_INVALID)
			return;
	}

	// otherwise, call the byte reading function for the translated address
	space(spacenum).write_byte(address, data);

	if ((device().machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		device().machine().debugger().cpu().set_memory_modified(true);
}


//-------------------------------------------------
//  write_word - write a word to the specified
//  memory space
//-------------------------------------------------

void device_memory_interface::write_word(int spacenum, offs_t address, u16 data, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	// if this is a misaligned write, or if there are no word writers, just read two bytes
	if (!WORD_ALIGNED(address))
	{
		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
		{
			write_byte(spacenum, address + 0, data >> 0, intention);
			write_byte(spacenum, address + 1, data >> 8, intention);
		}
		else
		{
			write_byte(spacenum, address + 0, data >> 8, intention);
			write_byte(spacenum, address + 1, data >> 0, intention);
		}
	}

	// otherwise, this proceeds like the byte case
	else
	{
		// translate if necessary
		if (intention != TRANSLATE_NONE)
		{
			assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_WRITE);
			spacenum = translate(spacenum, intention, address);

			// if not mapped, we're done
			if (spacenum == AS_INVALID)
				return;
		}

		// otherwise, call the byte reading function for the translated address
		space(spacenum).write_word(address, data);

		if ((device().machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
			device().machine().debugger().cpu().set_memory_modified(true);
	}
}


//-------------------------------------------------
//  write_dword - write a dword to the specified
//  memory space
//-------------------------------------------------

void device_memory_interface::write_dword(int spacenum, offs_t address, u32 data, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	// if this is a misaligned write, or if there are no dword writers, just read two words
	if (!DWORD_ALIGNED(address))
	{
		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
		{
			write_word(spacenum, address + 0, data >> 0, intention);
			write_word(spacenum, address + 2, data >> 16, intention);
		}
		else
		{
			write_word(spacenum, address + 0, data >> 16, intention);
			write_word(spacenum, address + 2, data >> 0, intention);
		}
	}

	// otherwise, this proceeds like the byte case
	else
	{
		// translate if necessary
		if (intention != TRANSLATE_NONE)
		{
			assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_WRITE);
			spacenum = translate(spacenum, intention, address);

			// if not mapped, we're done
			if (spacenum == AS_INVALID)
				return;
		}

		// otherwise, call the byte reading function for the translated address
		space(spacenum).write_dword(address, data);

		if ((device().machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
			device().machine().debugger().cpu().set_memory_modified(true);
	}
}


//-------------------------------------------------
//  write_qword - write a qword to the specified
//  memory space
//-------------------------------------------------

void device_memory_interface::write_qword(int spacenum, offs_t address, u64 data, int intention)
{
	// mask against the logical byte mask
	address &= space(spacenum).logbytemask();

	// if this is a misaligned write, or if there are no qword writers, just read two dwords
	if (!QWORD_ALIGNED(address))
	{
		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
		{
			write_dword(spacenum, address + 0, data >> 0, intention);
			write_dword(spacenum, address + 4, data >> 32, intention);
		}
		else
		{
			write_dword(spacenum, address + 0, data >> 32, intention);
			write_dword(spacenum, address + 4, data >> 0, intention);
		}
	}

	// otherwise, this proceeds like the byte case
	else
	{
		// translate if necessary
		if (intention != TRANSLATE_NONE)
		{
			assert((intention & TRANSLATE_TYPE_MASK) == TRANSLATE_WRITE);
			spacenum = translate(spacenum, intention, address);

			// if not mapped, we're done
			if (spacenum == AS_INVALID)
				return;
		}

		// otherwise, call the byte reading function for the translated address
		space(spacenum).write_qword(address, data);

		if ((device().machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
			device().machine().debugger().cpu().set_memory_modified(true);
	}
}


//-------------------------------------------------
//  write_memory - write 1,2,4 or 8 bytes to the
//  specified memory space
//--------------------------------------------------

void device_memory_interface::write_memory(int spacenum, offs_t address, u64 data, int size, int intention)
{
	switch (size)
	{
		case 1:     write_byte(spacenum, address, data, intention);  break;
		case 2:     write_word(spacenum, address, data, intention);  break;
		case 4:     write_dword(spacenum, address, data, intention); break;
		case 8:     write_qword(spacenum, address, data, intention); break;
	}
}


//-------------------------------------------------
//  read_opcode - read 1,2,4 or 8 bytes at the
//  given offset from opcode space
//-------------------------------------------------

u64 device_memory_interface::read_opcode(int spacenum, offs_t address, int size)
{
	u64 result = ~u64(0) & (~u64(0) >> (64 - 8*size));

	// keep in logical range
	address &= space(spacenum).logbytemask();

	// if we're bigger than the address bus, break into smaller pieces
	if (size > space(spacenum).data_width() / 8)
	{
		int halfsize = size / 2;
		u64 r0 = read_opcode(spacenum, address + 0, halfsize);
		u64 r1 = read_opcode(spacenum, address + halfsize, halfsize);

		if (space(spacenum).endianness() == ENDIANNESS_LITTLE)
			return r0 | (r1 << (8 * halfsize));
		else
			return r1 | (r0 << (8 * halfsize));
	}

	// keep in physical range
	address &= space(spacenum).bytemask();

	// translate to physical first
	spacenum = translate(spacenum, TRANSLATE_FETCH_DEBUG, address);
	if (spacenum == AS_INVALID)
		return result;

	// switch off the size and handle unaligned accesses
	switch (size)
	{
		case 1:
			result = space(spacenum).read_byte(address);
			break;

		case 2:
			result = space(spacenum).read_word_unaligned(address);
			break;

		case 4:
			result = space(spacenum).read_dword_unaligned(address);
			break;

		case 6:
		case 8:
			result = space(spacenum).read_qword_unaligned(address);
			break;
	}

	return result;
}
