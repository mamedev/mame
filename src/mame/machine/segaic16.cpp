// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#include "emu.h"
#include "segaic16.h"
#include "video/resnet.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_MEMORY_MAP  (0)
#define LOG_MULTIPLY    (0)
#define LOG_DIVIDE      (0)
#define LOG_COMPARE     (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
const device_type SEGA_315_5195_MEM_MAPPER = &device_creator<sega_315_5195_mapper_device>;
const device_type SEGA_315_5248_MULTIPLIER = &device_creator<sega_315_5248_multiplier_device>;
const device_type SEGA_315_5249_DIVIDER = &device_creator<sega_315_5249_divider_device>;
const device_type SEGA_315_5250_COMPARE_TIMER = &device_creator<sega_315_5250_compare_timer_device>;



//**************************************************************************
//  MISC HELPERS
//**************************************************************************

//-------------------------------------------------
//  sega_16bit_common_base - constructor
//-------------------------------------------------

sega_16bit_common_base::sega_16bit_common_base(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_paletteram(*this, "paletteram"),
		m_open_bus_recurse(false),
		m_palette_entries(0),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
{
	palette_init();
}


//-------------------------------------------------
//  open_bus_r - return value from reading an
//  unmapped address
//-------------------------------------------------

READ16_MEMBER( sega_16bit_common_base::open_bus_r )
{
	// Unmapped memory returns the last word on the data bus, which is almost always the opcode
	// of the next instruction due to prefetch; however, since we may be encrypted, we actually
	// need to return the encrypted opcode, not the last decrypted data.

	// Believe it or not, this is actually important for Cotton, which has the following evil
	// code: btst #0,$7038f7, which tests the low bit of an unmapped address, which thus should
	// return the prefetched value.

	// prevent recursion
	if (m_open_bus_recurse)
		return 0xffff;

	// read original encrypted memory at that address
	m_open_bus_recurse = true;
	UINT16 result = space.read_word(space.device().safe_pc());
	m_open_bus_recurse = false;
	return result;
}


//-------------------------------------------------
//  palette_init - precompute weighted RGB values
//  for each input value 0-31
//-------------------------------------------------

void sega_16bit_common_base::palette_init()
{
	//
	//  Color generation details
	//
	//  Each color is made up of 5 bits, connected through one or more resistors like so:
	//
	//  Bit 0 = 1 x 3.9K ohm
	//  Bit 1 = 1 x 2.0K ohm
	//  Bit 2 = 1 x 1.0K ohm
	//  Bit 3 = 2 x 1.0K ohm
	//  Bit 4 = 4 x 1.0K ohm
	//
	//  Another data bit is connected by a tristate buffer to the color output through a
	//  470 ohm resistor. The buffer allows the resistor to have no effect (tristate),
	//  halve brightness (pull-down) or double brightness (pull-up). The data bit source
	//  is bit 15 of each color RAM entry.
	//

	// compute weight table for regular palette entries
	static const int resistances_normal[6] = { 3900, 2000, 1000, 1000/2, 1000/4, 0   };
	double weights_normal[6];
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_normal, weights_normal, 0, 0,
		0, nullptr, nullptr, 0, 0,
		0, nullptr, nullptr, 0, 0);

	// compute weight table for shadow/hilight palette entries
	static const int resistances_sh[6]     = { 3900, 2000, 1000, 1000/2, 1000/4, 470 };
	double weights_sh[6];
	compute_resistor_weights(0, 255, -1.0,
		6, resistances_sh, weights_sh, 0, 0,
		0, nullptr, nullptr, 0, 0,
		0, nullptr, nullptr, 0, 0);

	// compute R, G, B for each weight
	for (int value = 0; value < 32; value++)
	{
		int i4 = (value >> 4) & 1;
		int i3 = (value >> 3) & 1;
		int i2 = (value >> 2) & 1;
		int i1 = (value >> 1) & 1;
		int i0 = (value >> 0) & 1;
		m_palette_normal[value] = combine_6_weights(weights_normal, i0, i1, i2, i3, i4, 0);
		m_palette_shadow[value] = combine_6_weights(weights_sh, i0, i1, i2, i3, i4, 0);
		m_palette_hilight[value] = combine_6_weights(weights_sh, i0, i1, i2, i3, i4, 1);
	}
}


//-------------------------------------------------
//  paletteram_w - handle writes to palette RAM
//-------------------------------------------------

WRITE16_MEMBER( sega_16bit_common_base::paletteram_w )
{
	// compute the number of entries
	if (m_palette_entries == 0)
		m_palette_entries = memshare("paletteram")->bytes() / 2;

	// get the new value
	UINT16 newval = m_paletteram[offset];
	COMBINE_DATA(&newval);
	m_paletteram[offset] = newval;

	//     byte 0    byte 1
	//  sBGR BBBB GGGG RRRR
	//  x000 4321 4321 4321
	int r = ((newval >> 12) & 0x01) | ((newval << 1) & 0x1e);
	int g = ((newval >> 13) & 0x01) | ((newval >> 3) & 0x1e);
	int b = ((newval >> 14) & 0x01) | ((newval >> 7) & 0x1e);

	// normal colors
	m_palette->set_pen_color(offset + 0 * m_palette_entries, m_palette_normal[r],  m_palette_normal[g],  m_palette_normal[b]);
	m_palette->set_pen_color(offset + 1 * m_palette_entries, m_palette_shadow[r],  m_palette_shadow[g],  m_palette_shadow[b]);
	m_palette->set_pen_color(offset + 2 * m_palette_entries, m_palette_hilight[r], m_palette_hilight[g], m_palette_hilight[b]);
}



//**************************************************************************
//  315-5195 MEMORY MAPPER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5195_mapper_device - constructor
//-------------------------------------------------

sega_315_5195_mapper_device::sega_315_5195_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA_315_5195_MEM_MAPPER, "Sega 315-5195 Memory Mapper", tag, owner, clock, "sega_315_5195", __FILE__),
		m_cputag(nullptr),
		m_cpu(nullptr),
		m_space(nullptr),
		m_decrypted_space(nullptr),
		m_curregion(0)
{
}


//-------------------------------------------------
//  static_set_cputag - configuration helper
//  to set the tag of the CPU device
//-------------------------------------------------

void sega_315_5195_mapper_device::static_set_cputag(device_t &device, const char *cpu)
{
	sega_315_5195_mapper_device &mapper = downcast<sega_315_5195_mapper_device &>(device);
	mapper.m_cputag = cpu;
}


//-------------------------------------------------
//  static_set_mapper - configuration helper
//  to set the mapper function
//-------------------------------------------------

void sega_315_5195_mapper_device::static_set_mapper(device_t &device, mapper_delegate callback)
{
	sega_315_5195_mapper_device &mapper = downcast<sega_315_5195_mapper_device &>(device);
	mapper.m_mapper = callback;
}


//-------------------------------------------------
//  static_set_sound_readwrite - configuration
//  helper to set the sound read/write callbacks
//-------------------------------------------------

void sega_315_5195_mapper_device::static_set_sound_readwrite(device_t &device, sound_read_delegate read, sound_write_delegate write)
{
	sega_315_5195_mapper_device &mapper = downcast<sega_315_5195_mapper_device &>(device);
	mapper.m_sound_read = read;
	mapper.m_sound_write = write;
}


//-------------------------------------------------
//  write - handle a write to the memory mapper
//-------------------------------------------------

WRITE8_MEMBER( sega_315_5195_mapper_device::write )
{
	// wraps every 32 bytes
	offset &= 0x1f;

if (LOG_MEMORY_MAP) osd_printf_debug("(Write %02X = %02X)\n", offset, data);

	// remember the previous value and swap in the new one
	UINT8 oldval = m_regs[offset];
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
			if (!m_sound_write.isnull())
				m_sound_write(data);
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
				offs_t addr = (m_regs[0x0a] << 17) | (m_regs[0x0b] << 9) | (m_regs[0x0c] << 1);
				m_space->write_word(addr, (m_regs[0x00] << 8) | m_regs[0x01]);
			}
			else if (data == 0x02)
			{
				offs_t addr = (m_regs[0x07] << 17) | (m_regs[0x08] << 9) | (m_regs[0x09] << 1);
				UINT16 result = m_space->read_word(addr);
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

READ8_MEMBER( sega_315_5195_mapper_device::read )
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
			if (!m_sound_read.isnull())
				return m_sound_read();
			return 0xff;

		default:
			logerror("Unknown memory_mapper_r from address %02X\n", offset);
			break;
	}
	return (space.data_width() == 8) ? 0xff : machine().driver_data<sega_16bit_common_base>()->open_bus_r(space, 0, 0xffff);
}


//-------------------------------------------------
//  map_as_rom - map a region as ROM data
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_rom(UINT32 offset, UINT32 length, offs_t mirror, const char *bank_name, const char *decrypted_bank_name, offs_t rgnoffset, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	if (LOG_MEMORY_MAP)
	{
		osd_printf_debug("Map %06X-%06X (%06X) as ROM+%06X(%s)", info.start, info.end, info.mirror, rgnoffset, bank_name);
		if (!whandler.isnull()) osd_printf_debug(" with handler=%s", whandler.name());
		osd_printf_debug("\n");
	}

	// don't map if the start is past the end of the ROM region
	offs_t romsize = m_cpu->region()->bytes();
	if (rgnoffset < romsize)
	{
		// clamp the end to the ROM size
		offs_t romend = info.end;
		if (rgnoffset + romend + 1 - info.start >= romsize)
			romend = romsize - 1 - rgnoffset + info.start;

		// map now
		m_space->install_read_bank(info.start, romend, 0, info.mirror, bank_name);
		if (m_decrypted_space)
			m_decrypted_space->install_read_bank(info.start, romend, 0, info.mirror, decrypted_bank_name);

		// configure the bank
		memory_bank *bank = owner()->membank(bank_name);
		memory_bank *decrypted_bank = owner()->membank(decrypted_bank_name);
		UINT8 *memptr = m_cpu->region()->base() + rgnoffset;
		bank->set_base(memptr);

		// remember this bank, and decrypt if necessary
		m_banks[m_curregion].set(bank, decrypted_bank, info.start, romend, rgnoffset, memptr);
	}

	// either install a write handler if provided or unmap the region
	//
	// shdancer relies on this behaviour to prevent a write to ROM from
	// falling through to the memory-mapping registers and crashing the
	// game during stage 2-4 (see PC:$18a98). Protection maybe?
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, whandler);
	else
		m_space->unmap_write(info.start, info.end, 0, info.mirror);
}


//-------------------------------------------------
//  map_as_ram - map a region as RAM, with an
//  optional write handler
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_ram(UINT32 offset, UINT32 length, offs_t mirror, const char *bank_share_name, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	if (LOG_MEMORY_MAP)
	{
		osd_printf_debug("Map %06X-%06X (%06X) as RAM(%s)", info.start, info.end, info.mirror, bank_share_name);
		if (!whandler.isnull()) osd_printf_debug(" with handler=%s", whandler.name());
		osd_printf_debug("\n");
	}

	// map now
	m_space->install_read_bank(info.start, info.end, 0, info.mirror, bank_share_name);

	// either install a write handler or a write bank, as appropriate
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, whandler);
	else
		m_space->install_write_bank(info.start, info.end, 0, info.mirror, bank_share_name);

	// configure the bank
	memory_bank *bank = owner()->membank(bank_share_name);
	bank->set_base(owner()->memshare(bank_share_name)->ptr());

	// clear this rom bank reference
	m_banks[m_curregion].clear();
}


//-------------------------------------------------
//  map_as_handler - map a region as a pair of
//  read write handlers
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_handler(UINT32 offset, UINT32 length, offs_t mirror, read16_delegate rhandler, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	if (LOG_MEMORY_MAP)
	{
		osd_printf_debug("Map %06X-%06X (%06X) as handler", info.start, info.end, info.mirror);
		if (!rhandler.isnull()) osd_printf_debug(" read=%s", rhandler.name());
		if (!whandler.isnull()) osd_printf_debug(" write=%s", whandler.name());
		osd_printf_debug("\n");
	}

	// install read/write handlers
	if (!rhandler.isnull())
		m_space->install_read_handler(info.start, info.end, 0, info.mirror, rhandler);
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, whandler);

	// clear this rom bank reference
	m_banks[m_curregion].clear();
}


//-------------------------------------------------
//  configure_explicit - explicitly configure the
//  memory map
//-------------------------------------------------

void sega_315_5195_mapper_device::configure_explicit(const UINT8 *map_data)
{
	memcpy(&m_regs[0x10], map_data, 0x10);
	update_mapping();
}


//-------------------------------------------------
//  fd1094_state_change - handle notifications
//  of state changes
//-------------------------------------------------

void sega_315_5195_mapper_device::fd1094_state_change(UINT8 state)
{
	// iterate over regions and set the decrypted address of any ROM banks
	for (auto & elem : m_banks)
		elem.update();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5195_mapper_device::device_start()
{
	// bind our handlers
	m_mapper.bind_relative_to(*owner());
	m_sound_read.bind_relative_to(*owner());
	m_sound_write.bind_relative_to(*owner());

	// find our CPU
	m_cpu = siblingdevice<m68000_device>(m_cputag);
	if (m_cpu == nullptr)
		throw emu_fatalerror("Unable to find sibling device '%s'", m_cputag);

	// if we are mapping an FD1089, tell all the banks
	fd1089_base_device *fd1089 = dynamic_cast<fd1089_base_device *>(m_cpu);
	if (fd1089 != nullptr)
		for (auto & elem : m_banks)
			elem.set_decrypt(fd1089);

	// if we are mapping an FD1094, register for state change notifications and tell all the banks
	fd1094_device *fd1094 = dynamic_cast<fd1094_device *>(m_cpu);
	if (fd1094 != nullptr)
	{
		fd1094->notify_state_change(fd1094_device::state_change_delegate(FUNC(sega_315_5195_mapper_device::fd1094_state_change), this));
		for (auto & elem : m_banks)
			elem.set_decrypt(fd1094);
	}

	// find the address space that is to be mapped
	m_space = &m_cpu->space(AS_PROGRAM);
	if (m_space == nullptr)
		throw emu_fatalerror("Unable to find program address space on device '%s'", m_cputag);

	m_decrypted_space = m_cpu->has_space(AS_DECRYPTED_OPCODES) ? &m_cpu->space(AS_DECRYPTED_OPCODES) : nullptr;

	// register for saves
	save_item(NAME(m_regs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5195_mapper_device::device_reset()
{
	// hold the CPU in reset
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// clear registers and recompute the memory mapping
	memset(m_regs, 0, sizeof(m_regs));
	update_mapping();

	// release the CPU
	m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


//-------------------------------------------------
//  compute_region - determine region parameters
//  based on current configuration registers and
//  actual underlying bus connections
//-------------------------------------------------

void sega_315_5195_mapper_device::compute_region(region_info &info, UINT8 index, UINT32 length, UINT32 mirror, UINT32 offset)
{
	static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
	info.size_mask = region_size_map[m_regs[0x10 + 2 * index] & 3];
	info.base = (m_regs[0x11 + 2 * index] << 16) & ~info.size_mask;
	info.mirror = mirror & info.size_mask;
	info.start = info.base + (offset & info.size_mask);
	info.end = info.start + MIN(length - 1, info.size_mask);
}


//-------------------------------------------------
//  update_mapping - remap the entire CPU address
//  space based on updated mappings
//-------------------------------------------------

void sega_315_5195_mapper_device::update_mapping()
{
	if (LOG_MEMORY_MAP) osd_printf_debug("----\nRemapping:\n");

	// first reset everything back to the beginning
	m_space->install_readwrite_handler(0x000000, 0xffffff, read8_delegate(FUNC(sega_315_5195_mapper_device::read), this), write8_delegate(FUNC(sega_315_5195_mapper_device::write), this), 0x00ff);

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
	: m_bank(nullptr),
		m_decrypted_bank(nullptr),
		m_start(0),
		m_end(0),
		m_rgnoffs(~0),
		m_srcptr(nullptr),
		m_fd1089(nullptr)
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

void sega_315_5195_mapper_device::decrypt_bank::set(memory_bank *bank, memory_bank *decrypted_bank, offs_t start, offs_t end, offs_t rgnoffs, UINT8 *src)
{
	// ignore if not encrypted
	if (m_fd1089 == nullptr && m_fd1094_cache == nullptr)
		return;

	// ignore if nothing is changing
	if (bank == m_bank && start == m_start && end == m_end && rgnoffs == m_rgnoffs && src == m_srcptr)
		return;

	// if the start, end, or src change, throw away any cached data
	reset();

	// update to the current state
	m_bank = bank;
	m_decrypted_bank = decrypted_bank;
	m_start = start;
	m_end = end;
	m_rgnoffs = rgnoffs;
	m_srcptr = src;

	// configure the fd1094 cache
	if (m_fd1094_cache != nullptr)
		m_fd1094_cache->configure(m_start, m_end + 1 - m_start, m_rgnoffs);

	// force an update of what we have
	update();
}


//-------------------------------------------------
//  update - update the decrypted memory base
//  if this rom bank has been assigned
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::update()
{
	// if this isn't a valid state, don't try to do anything
	if (m_bank == nullptr || m_srcptr == nullptr)
		return;

	// fd1089 case
	if (m_fd1089 != nullptr)
	{
		m_fd1089_decrypted.resize((m_end + 1 - m_start) / 2);
		m_fd1089->decrypt(m_start, m_end + 1 - m_start, m_rgnoffs, &m_fd1089_decrypted[0], reinterpret_cast<UINT16 *>(m_srcptr));
		m_decrypted_bank->set_base(&m_fd1089_decrypted[0]);
	}

	// fd1094 case
	if (m_fd1094_cache != nullptr)
		m_decrypted_bank->set_base(m_fd1094_cache->decrypted_opcodes(m_fd1094_cache->fd1094().state()));
}



//**************************************************************************
//  315-5248 MULTIPLIER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5248_multiplier_device - constructor
//-------------------------------------------------

sega_315_5248_multiplier_device::sega_315_5248_multiplier_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA_315_5248_MULTIPLIER, "Sega 315-5248 Multiplier", tag, owner, clock, "sega_315_5248", __FILE__)
{
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

READ16_MEMBER( sega_315_5248_multiplier_device::read )
{
	switch (offset & 3)
	{
		// if bit 1 is 0, just return register values
		case 0: return m_regs[0];
		case 1: return m_regs[1];

		// if bit 1 is 1, return ther results
		case 2: return (INT16(m_regs[0]) * INT16(m_regs[1])) >> 16;
		case 3: return (INT16(m_regs[0]) * INT16(m_regs[1])) & 0xffff;
	}

	// should never get here
	return 0xffff;
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

WRITE16_MEMBER( sega_315_5248_multiplier_device::write )
{
	// only low bit matters
	COMBINE_DATA(&m_regs[offset & 1]);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5248_multiplier_device::device_start()
{
	save_item(NAME(m_regs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5248_multiplier_device::device_reset()
{
	memset(m_regs, 0, sizeof(m_regs));
}



//**************************************************************************
//  315-5249 DIVIDER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5249_divider_device - constructor
//-------------------------------------------------

sega_315_5249_divider_device::sega_315_5249_divider_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA_315_5248_MULTIPLIER, "Sega 315-5249 Divider", tag, owner, clock, "sega_315_5249", __FILE__)
{
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

READ16_MEMBER( sega_315_5249_divider_device::read )
{
	// 8 effective read registers
	switch (offset & 7)
	{
		case 0: return m_regs[0];   // dividend high
		case 1: return m_regs[1];   // dividend low
		case 2: return m_regs[2];   // divisor
		case 4: return m_regs[4];   // quotient (mode 0) or quotient high (mode 1)
		case 5: return m_regs[5];   // remainder (mode 0) or quotient low (mode 1)
		case 6: return m_regs[6];   // flags
	}
	return 0xffff;
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

WRITE16_MEMBER( sega_315_5249_divider_device::write )
{
	if (LOG_DIVIDE) logerror("divide_w(%X) = %04X\n", offset, data);

	// only 4 effective write registers
	switch (offset & 3)
	{
		case 0: COMBINE_DATA(&m_regs[0]); break;    // dividend high
		case 1: COMBINE_DATA(&m_regs[1]); break;    // dividend low
		case 2: COMBINE_DATA(&m_regs[2]); break;    // divisor/trigger
		case 3: break;
	}

	// if A4 line is high, divide, using A3 as the mode
	if (offset & 8)
		execute(offset & 4);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5249_divider_device::device_start()
{
	save_item(NAME(m_regs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5249_divider_device::device_reset()
{
	memset(m_regs, 0, sizeof(m_regs));
}


//-------------------------------------------------
//  execute - execute the divide
//-------------------------------------------------

void sega_315_5249_divider_device::execute(int mode)
{
	// clear the flags by default
	m_regs[6] = 0;

	// mode 0: signed divide, return 16-bit quotient/remainder
	if (mode == 0)
	{
		// perform signed divide
		INT32 dividend = INT32((m_regs[0] << 16) | m_regs[1]);
		INT32 divisor = INT16(m_regs[2]);
		INT32 quotient;

		// check for divide by 0, signal if we did
		if (divisor == 0)
		{
			quotient = dividend;//((INT32)(dividend ^ divisor) < 0) ? 0x8000 : 0x7fff;
			m_regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		// clamp to 16-bit signed, signal overflow if we did
		if (quotient < -32768)
		{
			quotient = -32768;
			m_regs[6] |= 0x8000;
		}
		else if (quotient > 32767)
		{
			quotient = 32767;
			m_regs[6] |= 0x8000;
		}

		// store quotient and remainder
		m_regs[4] = INT16(quotient);
		m_regs[5] = INT16(dividend - quotient * divisor);
	}

	// mode 1: unsigned divide, 32-bit quotient only
	else
	{
		// perform unsigned divide
		UINT32 dividend = UINT32((m_regs[0] << 16) | m_regs[1]);
		UINT32 divisor = UINT16(m_regs[2]);
		UINT32 quotient;

		// check for divide by 0, signal if we did
		if (divisor == 0)
		{
			quotient = dividend;//0x7fffffff;
			m_regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		// store 32-bit quotient
		m_regs[4] = quotient >> 16;
		m_regs[5] = quotient & 0xffff;
	}
}


//**************************************************************************
//  315-5250 COMPARE/TIMER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5250_compare_timer_device -
//  constructor
//-------------------------------------------------

sega_315_5250_compare_timer_device::sega_315_5250_compare_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA_315_5250_COMPARE_TIMER, "Sega 315-5250 Compare/Timer", tag, owner, clock, "sega_315_5250", __FILE__)
{
}


//-------------------------------------------------
//  static_set_timer_ack - configuration helper
//  to set the timer acknowledge function
//-------------------------------------------------

void sega_315_5250_compare_timer_device::static_set_timer_ack(device_t &device, timer_ack_delegate callback)
{
	sega_315_5250_compare_timer_device &timer = downcast<sega_315_5250_compare_timer_device &>(device);
	timer.m_timer_ack = callback;
}


//-------------------------------------------------
//  static_set_sound_readwrite - configuration
//  helper to set the sound read/write callbacks
//-------------------------------------------------

void sega_315_5250_compare_timer_device::static_set_sound_write(device_t &device, sound_write_delegate write)
{
	sega_315_5250_compare_timer_device &timer = downcast<sega_315_5250_compare_timer_device &>(device);
	timer.m_sound_write = write;
}


//-------------------------------------------------
//  clock - clock the timer
//-------------------------------------------------

bool sega_315_5250_compare_timer_device::clock()
{
	// if we're enabled, clock the upcounter
	int old_counter = m_counter;
	if (m_regs[10] & 1)
		m_counter++;

	// regardless of the enable, a value of 0xfff will generate the IRQ
	bool result = false;
	if (old_counter == 0xfff)
	{
		result = true;
		m_counter = m_regs[8] & 0xfff;
	}
	return result;
}


//-------------------------------------------------
//  read - read the registers
//-------------------------------------------------

READ16_MEMBER( sega_315_5250_compare_timer_device::read )
{
	if (LOG_COMPARE) logerror("compare_r(%X) = %04X\n", offset, m_regs[offset]);
	switch (offset & 15)
	{
		case 0x0:   return m_regs[0];
		case 0x1:   return m_regs[1];
		case 0x2:   return m_regs[2];
		case 0x3:   return m_regs[3];
		case 0x4:   return m_regs[4];
		case 0x5:   return m_regs[1];
		case 0x6:   return m_regs[2];
		case 0x7:   return m_regs[7];
		case 0x9:
		case 0xd:   interrupt_ack(); break;
	}
	return 0xffff;
}


//-------------------------------------------------
//  write - write to the registers
//-------------------------------------------------

WRITE16_MEMBER( sega_315_5250_compare_timer_device::write )
{
	if (LOG_COMPARE) logerror("compare_w(%X) = %04X\n", offset, data);
	switch (offset & 15)
	{
		case 0x0:   COMBINE_DATA(&m_regs[0]); execute(); break;
		case 0x1:   COMBINE_DATA(&m_regs[1]); execute(); break;
		case 0x2:   COMBINE_DATA(&m_regs[2]); execute(true); break;
		case 0x4:   m_regs[4] = 0; m_bit = 0; break;
		case 0x6:   COMBINE_DATA(&m_regs[2]); execute(); break;
		case 0x8:
		case 0xc:   COMBINE_DATA(&m_regs[8]); break;
		case 0x9:
		case 0xd:   interrupt_ack(); break;
		case 0xa:
		case 0xe:   COMBINE_DATA(&m_regs[10]); break;
		case 0xb:
		case 0xf:
			COMBINE_DATA(&m_regs[11]);
			if (!m_sound_write.isnull())
				m_sound_write(m_regs[11]);
			break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5250_compare_timer_device::device_start()
{
	// bind our handlers
	m_timer_ack.bind_relative_to(*owner());
	m_sound_write.bind_relative_to(*owner());

	// save states
	save_item(NAME(m_regs));
	save_item(NAME(m_counter));
	save_item(NAME(m_bit));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_315_5250_compare_timer_device::device_reset()
{
	memset(m_regs, 0, sizeof(m_regs));
	m_counter = 0;
	m_bit = 0;
}


//-------------------------------------------------
//  execute - execute the compare
//-------------------------------------------------

void sega_315_5250_compare_timer_device::execute(bool update_history)
{
	INT16 bound1 = INT16(m_regs[0]);
	INT16 bound2 = INT16(m_regs[1]);
	INT16 value = INT16(m_regs[2]);

	INT16 min = (bound1 < bound2) ? bound1 : bound2;
	INT16 max = (bound1 > bound2) ? bound1 : bound2;

	if (value < min)
	{
		m_regs[7] = min;
		m_regs[3] = 0x8000;
	}
	else if (value > max)
	{
		m_regs[7] = max;
		m_regs[3] = 0x4000;
	}
	else
	{
		m_regs[7] = value;
		m_regs[3] = 0x0000;
	}

	if (update_history)
		m_regs[4] |= (m_regs[3] == 0) << m_bit++;
}
