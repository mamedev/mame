// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 315-5195 memory mapper

***************************************************************************/

#include "emu.h"
#include "315_5195.h"

#include <algorithm>

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE         (0)

#include "logmacro.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SEGA_315_5195_MEM_MAPPER, sega_315_5195_mapper_device, "sega_315_5195", "Sega 315-5195 Memory Mapper")



//**************************************************************************
//  315-5195 MEMORY MAPPER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5195_mapper_device - constructor
//-------------------------------------------------

sega_315_5195_mapper_device::sega_315_5195_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGA_315_5195_MEM_MAPPER, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_cpuregion(*this, finder_base::DUMMY_TAG)
	, m_mapper(*this)
	, m_pbf_callback(*this)
	, m_mcu_int_callback(*this)
	, m_space(nullptr)
	, m_decrypted_space(nullptr)
	, m_decryption_banks(*this, "decrypted bank %u", 0U)
	, m_curregion(0)
	, m_to_sound(0)
	, m_from_sound(0)
	, m_open_bus_recurse(false)
{
}


//-------------------------------------------------
//  open_bus_r - return value from reading an
//  unmapped address
//-------------------------------------------------

u16 sega_315_5195_mapper_device::open_bus_r()
{
	// Unmapped memory returns the last word on the data bus, which is almost always the opcode
	// of the next instruction due to prefetch; however, since we may be encrypted, we actually
	// need to return the encrypted opcode, not the last decrypted data.

	// Believe it or not, this is actually important for Cotton, which has the following evil
	// code: btst #0,$7038f7, which tests the low bit of an unmapped address, which thus should
	// return the prefetched value.

	// prevent recursion
	if (!machine().side_effects_disabled())
	{
		if (m_open_bus_recurse)
			return 0xffff;

		// read original encrypted memory at that address
		m_open_bus_recurse = true;
		const u16 result = m_space->read_word(m_cpu->pc());
		m_open_bus_recurse = false;
		return result;
	}
	return 0xffff;
}


//-------------------------------------------------
//  write - handle a write to the memory mapper
//-------------------------------------------------

void sega_315_5195_mapper_device::write(offs_t offset, u8 data)
{
	// wraps every 32 bytes
	offset &= 0x1f;

	LOG("(Write %02X = %02X)\n", offset, data);

	// remember the previous value and swap in the new one
	const u8 oldval = m_regs[offset];
	m_regs[offset] = data;

	// switch off the offset
	switch (offset)
	{
		case 0x02:
			// misc commands
			//   00 - resume execution after 03
			//   03 - maybe controls halt and reset lines together?
			if ((oldval ^ m_regs[offset]) & 3)
			{
				// fd1094_machine_init calls device_reset on the CPU, so we must do this afterwards
				m_cpu->set_input_line(INPUT_LINE_RESET, (m_regs[offset] & 3) == 3 ? ASSERT_LINE : CLEAR_LINE);
			}
			break;

		case 0x03:
			// write through to the sound chip
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(sega_315_5195_mapper_device::write_to_sound), this), data);
			break;

		case 0x04:
			// controls IRQ lines to 68000, negative logic -- write $B to signal IRQ4
			if ((m_regs[offset] & 7) != 7)
				for (int irqnum = 0; irqnum < 8; irqnum++)
					m_cpu->set_input_line(irqnum, (irqnum == (~m_regs[offset] & 7)) ? HOLD_LINE : CLEAR_LINE);
			break;

		case 0x05:
			// read/write control
			//   01 - write data latched in 00,01 to 2 * (address in 0A,0B,0C)
			//   02 - read data into latches 00,01 from 2 * (address in 07,08,09)
			if (data == 0x01)
			{
				const offs_t addr = (m_regs[0x0a] << 17) | (m_regs[0x0b] << 9) | (m_regs[0x0c] << 1);
				m_space->write_word(addr, (m_regs[0x00] << 8) | m_regs[0x01]);
			}
			else if (data == 0x02)
			{
				const offs_t addr = (m_regs[0x07] << 17) | (m_regs[0x08] << 9) | (m_regs[0x09] << 1);
				const u16 result = m_space->read_word(addr);
				m_regs[0x00] = result >> 8;
				m_regs[0x01] = result;
			}
			break;

		case 0x07:  case 0x08:  case 0x09:
			// writes here latch a 68000 address for writing
			break;

		case 0x0a:  case 0x0b:  case 0x0c:
			// writes here latch a 68000 address for reading
			break;

		case 0x10:  case 0x11:
		case 0x12:  case 0x13:
		case 0x14:  case 0x15:
		case 0x16:  case 0x17:
		case 0x18:  case 0x19:
		case 0x1a:  case 0x1b:
		case 0x1c:  case 0x1d:
		case 0x1e:  case 0x1f:
			if (oldval != data)
				update_mapping();
			break;

		default:
			logerror("Unknown memory_mapper_w to address %02X = %02X\n", offset, data);
			break;
	}
}


//-------------------------------------------------
//  read - handle a read from the memory mapper
//-------------------------------------------------

u8 sega_315_5195_mapper_device::read(address_space &space, offs_t offset)
{
	// wraps every 32 bytes
	offset &= 0x1f;

	// switch off the offset
	switch (offset)
	{
		case 0x00:
		case 0x01:
			// data latches - return the values latched
			return m_regs[offset];

		case 0x02:
			// various input bits from the 68000
			//   01 - ????
			//   02 - ????
			//   04 - ????
			//   08 - ????
			//   40 - set if busy processing a read/write request
			// Together, 01+02 == 00 if the 68000 is halted
			// Together, 01+02+04+08 == 0F if the 68000 is executing
			return (m_regs[0x02] & 3) == 3 ? 0x00 : 0x0f;

		case 0x03:
			// this returns data that the sound CPU writes
			if (!m_mcu_int_callback.isnull() && !machine().side_effects_disabled())
				m_mcu_int_callback(CLEAR_LINE);
			return m_from_sound;

		default:
			logerror("Unknown memory_mapper_r from address %02X\n", offset);
			break;
	}
	return (space.data_width() == 8) ? 0xff : open_bus_r();
}


//-------------------------------------------------
//  map_as_rom - map a region as ROM data
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_rom(u32 offset, u32 length, offs_t mirror, const char *bank_name, const char *decrypted_bank_name, offs_t rgnoffset, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	LOG("Map %06X-%06X (%06X) as ROM+%06X(%s) with handler=%s\n", info.start, info.end, info.mirror, rgnoffset, bank_name,
		whandler.isnull() ? "none" : whandler.name());

	// don't map if the start is past the end of the ROM region
	const offs_t romsize = m_cpuregion->bytes();
	if (rgnoffset < romsize)
	{
		// clamp the end to the ROM size
		offs_t romend = info.end;
		if (rgnoffset + romend + 1 - info.start >= romsize)
			romend = romsize - 1 - rgnoffset + info.start;

		// find the memory
		void *memptr = m_cpuregion->base() + rgnoffset;

		// remember this bank, and decrypt if necessary
		m_banks[m_curregion].set(info.start, romend, rgnoffset, memptr, m_decryption_banks[m_curregion].target());

		// map now
		m_space->install_rom(info.start, romend, info.mirror, memptr);
		if (m_decrypted_space)
			m_decrypted_space->install_read_bank(info.start, romend, info.mirror, m_decryption_banks[m_curregion].target());
	}

	// either install a write handler if provided or unmap the region
	//
	// shdancer relies on this behaviour to prevent a write to ROM from
	// falling through to the memory-mapping registers and crashing the
	// game during stage 2-4 (see PC:$18a98). Protection maybe?
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, 0, whandler);
	else
		m_space->unmap_write(info.start, info.end | info.mirror);
}


//-------------------------------------------------
//  map_as_ram - map a region as RAM, with an
//  optional write handler
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_ram(u32 offset, u32 length, offs_t mirror, const char *bank_share_name, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	LOG("Map %06X-%06X (%06X) as RAM(%s) with handler=%s\n", info.start, info.end, info.mirror, bank_share_name,
		whandler.isnull() ? "none" : whandler.name());

	// map now
	m_space->install_ram(info.start, info.end, info.mirror, owner()->memshare(bank_share_name)->ptr());

	// either install a write handler or a write bank, as appropriate
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, 0, whandler);

	// clear this rom bank reference
	m_banks[m_curregion].clear();
}


//-------------------------------------------------
//  map_as_region - map a region as ROM, with an
//  optional write handler
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_region(u32 offset, u32 length, offs_t mirror, const char *region_name, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	LOG("Map %06X-%06X (%06X) as REGION(%s) with handler=%s\n", info.start, info.end, info.mirror, region_name,
		whandler.isnull() ? "none" : whandler.name());

	// map now
	m_space->install_rom(info.start, info.end, info.mirror, owner()->memregion(region_name)->base());

	// either install a write handler or a write bank, as appropriate
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, 0, whandler);

	// clear this rom bank reference
	m_banks[m_curregion].clear();
}


//-------------------------------------------------
//  map_as_handler - map a region as a pair of
//  read write handlers
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_handler(u32 offset, u32 length, offs_t mirror, read16_delegate rhandler, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	LOG("Map %06X-%06X (%06X) as handler read=%s write=%s\n", info.start, info.end, info.mirror,
		rhandler.isnull() ? "none" : rhandler.name(),
		whandler.isnull() ? "none" : whandler.name());

	// install read/write handlers
	if (!rhandler.isnull())
		m_space->install_read_handler(info.start, info.end, 0, info.mirror, 0, rhandler);
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, 0, whandler);

	// clear this rom bank reference
	m_banks[m_curregion].clear();
}


//-------------------------------------------------
//  configure_explicit - explicitly configure the
//  memory map
//-------------------------------------------------

void sega_315_5195_mapper_device::configure_explicit(const u8 *map_data)
{
	memcpy(&m_regs[0x10], map_data, 0x10);
	update_mapping();
}


//-------------------------------------------------
//  fd1094_state_change - handle notifications
//  of state changes
//-------------------------------------------------

void sega_315_5195_mapper_device::fd1094_state_change(u8 state)
{
	// iterate over regions and set the decrypted address of any ROM banks
	for (int i=0; i != 8; i++)
		m_banks[i].update(m_decryption_banks[i].target());
}


//-------------------------------------------------
//  write_to_sound - write data for the sound CPU
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sega_315_5195_mapper_device::write_to_sound)
{
	m_to_sound = param;
	if (!m_pbf_callback.isnull())
		m_pbf_callback(ASSERT_LINE);
}


//-------------------------------------------------
//  write_from_sound - handle writes from the
//  sound CPU
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sega_315_5195_mapper_device::write_from_sound)
{
	m_from_sound = param;
	if (!m_mcu_int_callback.isnull())
		m_mcu_int_callback(ASSERT_LINE);
}


//-------------------------------------------------
//  pread - sound CPU read handler
//-------------------------------------------------

u8 sega_315_5195_mapper_device::pread()
{
	if (!m_pbf_callback.isnull() && !machine().side_effects_disabled())
		m_pbf_callback(CLEAR_LINE);
	return m_to_sound;
}


//-------------------------------------------------
//  pwrite - sound CPU write handler
//-------------------------------------------------

void sega_315_5195_mapper_device::pwrite(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(sega_315_5195_mapper_device::write_from_sound), this), data);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5195_mapper_device::device_start()
{
	// bind our handlers
	m_mapper.resolve();
	m_pbf_callback.resolve();
	m_mcu_int_callback.resolve();

	// if we are mapping an FD1089, tell all the banks
	fd1089_base_device *fd1089 = dynamic_cast<fd1089_base_device *>(m_cpu.target());
	if (fd1089 != nullptr)
		for (auto & elem : m_banks)
			elem.set_decrypt(fd1089);

	// if we are mapping an FD1094, register for state change notifications and tell all the banks
	fd1094_device *fd1094 = dynamic_cast<fd1094_device *>(m_cpu.target());
	if (fd1094 != nullptr)
	{
		fd1094->notify_state_change(fd1094_device::state_change_delegate(&sega_315_5195_mapper_device::fd1094_state_change, this));
		for (auto & elem : m_banks)
			elem.set_decrypt(fd1094);
	}

	// find the address space that is to be mapped
	m_space = &m_cpu->space(AS_PROGRAM);
	if (m_space == nullptr)
		throw emu_fatalerror("Unable to find program address space on device '%s'", m_cpu.finder_tag());

	m_decrypted_space = m_cpu->has_space(AS_OPCODES) ? &m_cpu->space(AS_OPCODES) : nullptr;

	// register for saves
	save_item(NAME(m_regs));
	save_item(NAME(m_to_sound));
	save_item(NAME(m_from_sound));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5195_mapper_device::device_reset()
{
	// hold the CPU in reset
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// clear registers and recompute the memory mapping
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	update_mapping();

	// release the CPU
	m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	m_to_sound = 0;
	m_from_sound = 0;
	if (!m_pbf_callback.isnull())
		m_pbf_callback(CLEAR_LINE);
	if (!m_mcu_int_callback.isnull())
		m_mcu_int_callback(CLEAR_LINE);
}


//-------------------------------------------------
//  compute_region - determine region parameters
//  based on current configuration registers and
//  actual underlying bus connections
//-------------------------------------------------

void sega_315_5195_mapper_device::compute_region(region_info &info, u8 index, u32 length, u32 mirror, u32 offset)
{
	static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
	info.size_mask = region_size_map[m_regs[0x10 + 2 * index] & 3];
	info.base = (m_regs[0x11 + 2 * index] << 16) & ~info.size_mask;
	info.mirror = mirror & info.size_mask;
	info.start = info.base + (offset & info.size_mask);
	info.end = info.start + std::min(length - 1, info.size_mask);
}


//-------------------------------------------------
//  update_mapping - remap the entire CPU address
//  space based on updated mappings
//-------------------------------------------------

void sega_315_5195_mapper_device::update_mapping()
{
	LOG("---- Remapping:\n");

	// first reset everything back to the beginning
	m_space->unmap_readwrite(0x000000, 0xffffff);
	m_space->install_read_handler(0x000000, 0xffffff, read8m_delegate(*this, FUNC(sega_315_5195_mapper_device::read)), 0x00ff);
	m_space->install_write_handler(0x000000, 0xffffff, write8sm_delegate(*this, FUNC(sega_315_5195_mapper_device::write)), 0x00ff);

	// loop over the regions
	for (int index = 7; index >= 0; index--)
	{
		// note the current region and call the mapper to find out what to do
		m_curregion = index;
		m_mapper(*this, index);
	}
}


//**************************************************************************
//  DECRYPT BANK HELPER CLASS
//**************************************************************************

//-------------------------------------------------
//  decrypt_bank - constructor
//-------------------------------------------------

sega_315_5195_mapper_device::decrypt_bank::decrypt_bank()
	: m_start(0)
	, m_end(0)
	, m_rgnoffs(~0)
	, m_srcptr(nullptr)
	, m_fd1089(nullptr)
{
	// invalidate all states
	reset();
}


//-------------------------------------------------
//  ~decrypt_bank - destructor
//-------------------------------------------------

sega_315_5195_mapper_device::decrypt_bank::~decrypt_bank()
{
}


//-------------------------------------------------
//  set_decrypt - configure the decryption target
//  CPU
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::set_decrypt(fd1089_base_device *fd1089)
{
	// set the fd1089 pointer
	m_fd1089 = fd1089;

	// clear out all fd1094 stuff
	m_fd1094_cache.reset();
}

void sega_315_5195_mapper_device::decrypt_bank::set_decrypt(fd1094_device *fd1094)
{
	// set the fd1094 pointer and allocate a decryption cache
	m_fd1094_cache = std::make_unique<fd1094_decryption_cache>(*fd1094);

	// clear out all fd1089 stuff
	m_fd1089 = nullptr;
	m_fd1089_decrypted.clear();
}


//-------------------------------------------------
//  set - set the parameters of this bank after
//  a change
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::set(offs_t start, offs_t end, offs_t rgnoffs, void *src, memory_bank *bank)
{
	// ignore if not encrypted
	if (m_fd1089 == nullptr && m_fd1094_cache == nullptr)
		return;

	// ignore if nothing is changing
	if (start == m_start && end == m_end && rgnoffs == m_rgnoffs && src == m_srcptr)
		return;

	// if the start, end, or src change, throw away any cached data
	reset();

	// update to the current state
	m_start = start;
	m_end = end;
	m_rgnoffs = rgnoffs;
	m_srcptr = reinterpret_cast<u8 *>(src);

	// configure the fd1094 cache
	if (m_fd1094_cache != nullptr)
		m_fd1094_cache->configure(m_start, m_end + 1 - m_start, m_rgnoffs);

	// force an update of what we have
	update(bank);
}


//-------------------------------------------------
//  update - update the decrypted memory base
//  if this rom bank has been assigned
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::update(memory_bank *bank)
{
	// if this isn't a valid state, don't try to do anything
	if (m_srcptr == nullptr)
		return;

	// fd1089 case
	if (m_fd1089 != nullptr)
	{
		m_fd1089_decrypted.resize((m_end + 1 - m_start) / 2);
		m_fd1089->decrypt(m_start, m_end + 1 - m_start, m_rgnoffs, &m_fd1089_decrypted[0], reinterpret_cast<u16 *>(m_srcptr));
		bank->set_base(&m_fd1089_decrypted[0]);
	}

	// fd1094 case
	if (m_fd1094_cache != nullptr)
		bank->set_base(m_fd1094_cache->decrypted_opcodes(m_fd1094_cache->fd1094().state()));
}
