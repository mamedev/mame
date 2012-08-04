/***************************************************************************

    Sega 16-bit common hardware

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

#include "emu.h"
#include "segaic16.h"
#include "video/resnet.h"
#include "includes/segas16.h"		// needed for fd1094 calls


//**************************************************************************
//	DEBUGGING
//**************************************************************************

#define LOG_MEMORY_MAP	(1)
#define LOG_MULTIPLY	(0)
#define LOG_DIVIDE		(0)
#define LOG_COMPARE		(0)



//**************************************************************************
//	CONSTANTS
//**************************************************************************

// device type definition
const device_type SEGA_MEM_MAPPER = &device_creator<sega_315_5195_mapper_device>;



//**************************************************************************
//	MISC HELPERS
//**************************************************************************

READ16_HANDLER( segaic16_open_bus_r )
{
	static UINT8 recurse = 0;
	UINT16 result;

	// Unmapped memory returns the last word on the data bus, which is almost always the opcode
	// of the next instruction due to prefetch; however, since we may be encrypted, we actually
	// need to return the encrypted opcode, not the last decrypted data.

	// Believe it or not, this is actually important for Cotton, which has the following evil
	// code: btst #0,$7038f7, which tests the low bit of an unmapped address, which thus should
	// return the prefetched value.

	// prevent recursion
	if (recurse)
		return 0xffff;

	// read original encrypted memory at that address
	recurse = 1;
	result = space->read_word(cpu_get_pc(&space->device()));
	recurse = 0;
	return result;
}



//**************************************************************************
//	315-5195 MEMORY MAPPER
//**************************************************************************

//-------------------------------------------------
//  sega_315_5195_mapper_device - constructor
//-------------------------------------------------

sega_315_5195_mapper_device::sega_315_5195_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA_MEM_MAPPER, "Sega Memory Mapper", tag, owner, clock),
	  m_cputag(NULL),
	  m_cpu(NULL),
	  m_space(NULL),
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
//	helper to set the sound read/write callbacks
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

if (LOG_MEMORY_MAP) mame_printf_debug("(Write %02X = %02X)\n", offset, data);

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

		case 0x07:	case 0x08:	case 0x09:
			// writes here latch a 68000 address for writing
			break;

		case 0x0a:	case 0x0b:	case 0x0c:
			// writes here latch a 68000 address for reading
			break;

		case 0x10:	case 0x11:
		case 0x12:	case 0x13:
		case 0x14:	case 0x15:
		case 0x16:	case 0x17:
		case 0x18:	case 0x19:
		case 0x1a:	case 0x1b:
		case 0x1c:	case 0x1d:
		case 0x1e:	case 0x1f:
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
	return (space.data_width() == 8) ? 0xff : segaic16_open_bus_r(&space, 0, 0xffff);
}


//-------------------------------------------------
//  map_as_rom - map a region as ROM data
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_rom(UINT32 offset, UINT32 length, offs_t mirror, const char *bank_name, offs_t rgnoffset, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	if (LOG_MEMORY_MAP)
	{
		mame_printf_debug("Map %06X-%06X (%06X) as ROM+%06X(%s)", info.start, info.end, info.mirror, rgnoffset, bank_name);
		if (!whandler.isnull()) mame_printf_debug(" with handler=%s", whandler.name());
		mame_printf_debug("\n");
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

		// configure the bank
		memory_bank *bank = owner()->membank(bank_name);
		UINT8 *memptr = m_cpu->region()->base() + rgnoffset;
		bank->set_base(memptr);
		
		// remember this bank, and decrypt if necessary
		m_banks[m_curregion].set(bank, info.start, romend, rgnoffset, memptr);
	}

	// install a write handler if provided
	if (!whandler.isnull())
		m_space->install_write_handler(info.start, info.end, 0, info.mirror, whandler);
}


//-------------------------------------------------
//  map_as_ram - map a region as RAM, with an
//	optional write handler
//-------------------------------------------------

void sega_315_5195_mapper_device::map_as_ram(UINT32 offset, UINT32 length, offs_t mirror, const char *bank_share_name, write16_delegate whandler)
{
	// determine parameters
	region_info info;
	compute_region(info, m_curregion, length, mirror, offset);
	if (LOG_MEMORY_MAP)
	{
		mame_printf_debug("Map %06X-%06X (%06X) as RAM(%s)", info.start, info.end, info.mirror, bank_share_name);
		if (!whandler.isnull()) mame_printf_debug(" with handler=%s", whandler.name());
		mame_printf_debug("\n");
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
		mame_printf_debug("Map %06X-%06X (%06X) as handler", info.start, info.end, info.mirror);
		if (!rhandler.isnull()) mame_printf_debug(" read=%s", rhandler.name());
		if (!whandler.isnull()) mame_printf_debug(" write=%s", whandler.name());
		mame_printf_debug("\n");
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
//	memory map
//-------------------------------------------------

void sega_315_5195_mapper_device::configure_explicit(const UINT8 *map_data)
{
	memcpy(&m_regs[0x10], map_data, 0x10);
	update_mapping();
}


//-------------------------------------------------
//  fd1094_state_change - handle notifications
//	of state changes
//-------------------------------------------------

void sega_315_5195_mapper_device::fd1094_state_change(UINT8 state)
{
	// iterate over regions and set the decrypted address of any ROM banks
	for (int index = 0; index < ARRAY_LENGTH(m_banks); index++)
		m_banks[index].update();
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
	if (m_cpu == NULL)
		throw emu_fatalerror("Unable to find sibling device '%s'", m_cputag);
	
	// if we are mapping an FD1089, tell all the banks
	fd1089_base_device *fd1089 = dynamic_cast<fd1089_base_device *>(m_cpu);
	if (fd1089 != NULL)
		for (int banknum = 0; banknum < ARRAY_LENGTH(m_banks); banknum++)
			m_banks[banknum].set_decrypt(fd1089);

	// if we are mapping an FD1094, register for state change notifications and tell all the banks
	fd1094_device *fd1094 = dynamic_cast<fd1094_device *>(m_cpu);
	if (fd1094 != NULL)
	{
		fd1094->notify_state_change(fd1094_device::state_change_delegate(FUNC(sega_315_5195_mapper_device::fd1094_state_change), this));
		for (int banknum = 0; banknum < ARRAY_LENGTH(m_banks); banknum++)
			m_banks[banknum].set_decrypt(fd1094);
	}

	// find the address space that is to be mapped
	m_space = m_cpu->space(AS_PROGRAM);
	if (m_space == NULL)
		throw emu_fatalerror("Unable to find program address space on device '%s'", m_cputag);
	
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
//	based on current configuration registers and
//	actual underlying bus connections
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
	if (LOG_MEMORY_MAP) mame_printf_debug("----\nRemapping:\n");

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


//-------------------------------------------------
//  decrypt_bank - constructor
//-------------------------------------------------

sega_315_5195_mapper_device::decrypt_bank::decrypt_bank()
	: m_bank(NULL), 
	  m_start(0), 
	  m_end(0), 
	  m_rgnoffs(~0),
	  m_srcptr(NULL),
	  m_fd1089(NULL),
	  m_fd1094_cache(NULL)
{
	// invalidate all states
	reset();
}


//-------------------------------------------------
//  ~decrypt_bank - destructor
//-------------------------------------------------

sega_315_5195_mapper_device::decrypt_bank::~decrypt_bank()
{
	// delete any allocated cache
	global_free(m_fd1094_cache);
}


//-------------------------------------------------
//  set_decrypt - configure the decryption target
//	CPU
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::set_decrypt(fd1089_base_device *fd1089)
{
	// set the fd1089 pointer
	m_fd1089 = fd1089;
	
	// clear out all fd1094 stuff
	delete m_fd1094_cache;
	m_fd1094_cache = NULL;
}

void sega_315_5195_mapper_device::decrypt_bank::set_decrypt(fd1094_device *fd1094)
{
	// set the fd1094 pointer and allocate a decryption cache
	m_fd1094_cache = global_alloc(fd1094_decryption_cache(*fd1094));
	
	// clear out all fd1089 stuff
	m_fd1089 = NULL;
	m_fd1089_decrypted.reset();
}


//-------------------------------------------------
//  set - set the parameters of this bank after
//	a change
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::set(memory_bank *bank, offs_t start, offs_t end, offs_t rgnoffs, UINT8 *src)
{
	// ignore if not encrypted
	if (m_fd1089 == NULL && m_fd1094_cache == NULL)
		return;
	
	// ignore if nothing is changing
	if (bank == m_bank && start == m_start && end == m_end && rgnoffs == m_rgnoffs && src == m_srcptr)
		return;

	// if the start, end, or src change, throw away any cached data
	reset();

	// update to the current state
	m_bank = bank;
	m_start = start;
	m_end = end;
	m_rgnoffs = rgnoffs;
	m_srcptr = src;
	
	// configure the fd1094 cache
	if (m_fd1094_cache != NULL)
		m_fd1094_cache->configure(m_start, m_end + 1 - m_start, m_rgnoffs);

	// force an update of what we have
	update();
}


//-------------------------------------------------
//  update - update the decrypted memory base
//	if this rom bank has been assigned
//-------------------------------------------------

void sega_315_5195_mapper_device::decrypt_bank::update()
{
	// if this isn't a valid state, don't try to do anything
	if (m_bank == NULL || m_srcptr == NULL)
		return;
	
	// fd1089 case
	if (m_fd1089 != NULL)
	{
		m_fd1089_decrypted.resize((m_end + 1 - m_start) / 2);
		m_fd1089->decrypt(m_start, m_end + 1 - m_start, m_rgnoffs, m_fd1089_decrypted, reinterpret_cast<UINT16 *>(m_srcptr));
		m_bank->set_base_decrypted(m_fd1089_decrypted);
	}
	
	// fd1094 case
	if (m_fd1094_cache != NULL)
		m_bank->set_base_decrypted(m_fd1094_cache->decrypted_opcodes(m_fd1094_cache->fd1094().state()));
}



/*************************************
 *
 *  Multiply chip - 315-5248
 *
 *************************************/

typedef struct _ic_315_5248_state ic_315_5248_state ;
struct _ic_315_5248_state
{
	UINT16	regs[4];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ic_315_5248_state *_315_5248_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == _315_5248);

	return (ic_315_5248_state *)downcast<legacy_device_base *>(device)->token();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

READ16_DEVICE_HANDLER ( segaic16_multiply_r )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);

	offset &= 3;
	switch (offset)
	{
		case 0:	return ic_315_5248->regs[0];
		case 1:	return ic_315_5248->regs[1];
		case 2:	return ((INT16)ic_315_5248->regs[0] * (INT16)ic_315_5248->regs[1]) >> 16;
		case 3:	return ((INT16)ic_315_5248->regs[0] * (INT16)ic_315_5248->regs[1]) & 0xffff;
	}
	return 0xffff;
}


WRITE16_DEVICE_HANDLER( segaic16_multiply_w )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);

	offset &= 3;
	switch (offset)
	{
		case 0:	COMBINE_DATA(&ic_315_5248->regs[0]);	break;
		case 1:	COMBINE_DATA(&ic_315_5248->regs[1]);	break;
		case 2:	COMBINE_DATA(&ic_315_5248->regs[0]);	break;
		case 3:	COMBINE_DATA(&ic_315_5248->regs[1]);	break;
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ic_315_5248 )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);

	device->save_item(NAME(ic_315_5248->regs));
}

static DEVICE_RESET( ic_315_5248 )
{
	ic_315_5248_state *ic_315_5248 = _315_5248_get_safe_token(device);
	int i;

	for (i = 0; i < 4; i++)
		ic_315_5248->regs[i] = 0;
}

/*************************************
 *
 *  Divide chip - 315-5249
 *
 *************************************/

typedef struct _ic_315_5249_state ic_315_5249_state ;
struct _ic_315_5249_state
{
	UINT16	regs[8];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ic_315_5249_state *_315_5249_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == _315_5249);

	return (ic_315_5249_state *)downcast<legacy_device_base *>(device)->token();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static void update_divide( device_t *device, int mode )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);

	// clear the flags by default
	ic_315_5249->regs[6] = 0;

	// if mode 0, store quotient/remainder
	if (mode == 0)
	{
		INT32 dividend = (INT32)((ic_315_5249->regs[0] << 16) | ic_315_5249->regs[1]);
		INT32 divisor = (INT16)ic_315_5249->regs[2];
		INT32 quotient, remainder;

		// perform signed divide
		if (divisor == 0)
		{
			quotient = dividend;//((INT32)(dividend ^ divisor) < 0) ? 0x8000 : 0x7fff;
			ic_315_5249->regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		remainder = dividend - quotient * divisor;

		// clamp to 16-bit signed
		if (quotient < -32768)
		{
			quotient = -32768;
			ic_315_5249->regs[6] |= 0x8000;
		}
		else if (quotient > 32767)
		{
			quotient = 32767;
			ic_315_5249->regs[6] |= 0x8000;
		}

		// store quotient and remainder
		ic_315_5249->regs[4] = quotient;
		ic_315_5249->regs[5] = remainder;
	}

	// if mode 1, store 32-bit quotient
	else
	{
		UINT32 dividend = (UINT32)((ic_315_5249->regs[0] << 16) | ic_315_5249->regs[1]);
		UINT32 divisor = (UINT16)ic_315_5249->regs[2];
		UINT32 quotient;

		// perform unsigned divide
		if (divisor == 0)
		{
			quotient = dividend;//0x7fffffff;
			ic_315_5249->regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		// store 32-bit quotient
		ic_315_5249->regs[4] = quotient >> 16;
		ic_315_5249->regs[5] = quotient & 0xffff;
	}
}

READ16_DEVICE_HANDLER ( segaic16_divide_r )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);

	// 8 effective read registers
	offset &= 7;
	switch (offset)
	{
		case 0:	return ic_315_5249->regs[0];	// dividend high
		case 1:	return ic_315_5249->regs[1];	// dividend low
		case 2:	return ic_315_5249->regs[2];	// divisor
		case 4: 	return ic_315_5249->regs[4];	// quotient (mode 0) or quotient high (mode 1)
		case 5:	return ic_315_5249->regs[5];	// remainder (mode 0) or quotient low (mode 1)
		case 6: 	return ic_315_5249->regs[6];	// flags
	}

	return 0xffff;
}


WRITE16_DEVICE_HANDLER( segaic16_divide_w )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);
	int a4 = offset & 8;
	int a3 = offset & 4;

	if (LOG_DIVIDE) logerror("divide_w(%X) = %04X\n", offset, data);

	// only 4 effective write registers
	offset &= 3;
	switch (offset)
	{
		case 0:	COMBINE_DATA(&ic_315_5249->regs[0]); break;	// dividend high
		case 1:	COMBINE_DATA(&ic_315_5249->regs[1]); break;	// dividend low
		case 2:	COMBINE_DATA(&ic_315_5249->regs[2]); break;	// divisor/trigger
		case 3:	break;
	}

	// if a4 line is high, divide, using a3 as the mode
	if (a4) update_divide(device, a3);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ic_315_5249 )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);

	device->save_item(NAME(ic_315_5249->regs));
}

static DEVICE_RESET( ic_315_5249 )
{
	ic_315_5249_state *ic_315_5249 = _315_5249_get_safe_token(device);
	int i;

	for (i = 0; i < 8; i++)
		ic_315_5249->regs[i] = 0;
}

/*************************************
 *
 *  Compare/timer chip - 315-5250
 *
 *************************************/

typedef struct _ic_315_5250_state ic_315_5250_state ;
struct _ic_315_5250_state
{
	UINT16	regs[16];
	UINT16	counter;
	UINT8	      bit;
	_315_5250_sound_callback       sound_w;
	_315_5250_timer_ack_callback   timer_ack;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ic_315_5250_state *_315_5250_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == _315_5250);

	return (ic_315_5250_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const ic_315_5250_interface *_315_5250_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == _315_5250));
	return (const ic_315_5250_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

int segaic16_compare_timer_clock( device_t *device )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);
	int old_counter = ic_315_5250->counter;
	int result = 0;

	// if we're enabled, clock the upcounter
	if (ic_315_5250->regs[10] & 1)
		ic_315_5250->counter++;

	// regardless of the enable, a value of 0xfff will generate the IRQ
	if (old_counter == 0xfff)
	{
		result = 1;
		ic_315_5250->counter = ic_315_5250->regs[8] & 0xfff;
	}
	return result;
}


static void update_compare( device_t *device, int update_history )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);
	int bound1 = (INT16)ic_315_5250->regs[0];
	int bound2 = (INT16)ic_315_5250->regs[1];
	int value = (INT16)ic_315_5250->regs[2];
	int min = (bound1 < bound2) ? bound1 : bound2;
	int max = (bound1 > bound2) ? bound1 : bound2;

	if (value < min)
	{
		ic_315_5250->regs[7] = min;
		ic_315_5250->regs[3] = 0x8000;
	}
	else if (value > max)
	{
		ic_315_5250->regs[7] = max;
		ic_315_5250->regs[3] = 0x4000;
	}
	else
	{
		ic_315_5250->regs[7] = value;
		ic_315_5250->regs[3] = 0x0000;
	}

	if (update_history)
		ic_315_5250->regs[4] |= (ic_315_5250->regs[3] == 0) << ic_315_5250->bit++;
}


static void timer_interrupt_ack( device_t *device )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	if (ic_315_5250->timer_ack)
		(*ic_315_5250->timer_ack)(device->machine());
}


READ16_DEVICE_HANDLER ( segaic16_compare_timer_r )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	offset &= 0xf;
	if (LOG_COMPARE) logerror("compare_r(%X) = %04X\n", offset, ic_315_5250->regs[offset]);
	switch (offset)
	{
		case 0x0:	return ic_315_5250->regs[0];
		case 0x1:	return ic_315_5250->regs[1];
		case 0x2:	return ic_315_5250->regs[2];
		case 0x3:	return ic_315_5250->regs[3];
		case 0x4:	return ic_315_5250->regs[4];
		case 0x5:	return ic_315_5250->regs[1];
		case 0x6:	return ic_315_5250->regs[2];
		case 0x7:	return ic_315_5250->regs[7];
		case 0x9:
		case 0xd:	timer_interrupt_ack(device); break;
	}
	return 0xffff;
}


WRITE16_DEVICE_HANDLER ( segaic16_compare_timer_w )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	offset &= 0xf;
	if (LOG_COMPARE) logerror("compare_w(%X) = %04X\n", offset, data);
	switch (offset)
	{
		case 0x0:	COMBINE_DATA(&ic_315_5250->regs[0]); update_compare(device, 0); break;
		case 0x1:	COMBINE_DATA(&ic_315_5250->regs[1]); update_compare(device, 0); break;
		case 0x2:	COMBINE_DATA(&ic_315_5250->regs[2]); update_compare(device, 1); break;
		case 0x4:	ic_315_5250->regs[4] = 0; ic_315_5250->bit = 0; break;
		case 0x6:	COMBINE_DATA(&ic_315_5250->regs[2]); update_compare(device, 0); break;
		case 0x8:
		case 0xc:	COMBINE_DATA(&ic_315_5250->regs[8]); break;
		case 0x9:
		case 0xd:	timer_interrupt_ack(device); break;
		case 0xa:
		case 0xe:	COMBINE_DATA(&ic_315_5250->regs[10]); break;
		case 0xb:
		case 0xf:
			COMBINE_DATA(&ic_315_5250->regs[11]);
			if (ic_315_5250->sound_w)
				(*ic_315_5250->sound_w)(device->machine(), ic_315_5250->regs[11]);
			break;
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ic_315_5250 )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);
	const ic_315_5250_interface *intf = _315_5250_get_interface(device);

	ic_315_5250->sound_w = intf->sound_write_callback;
	ic_315_5250->timer_ack = intf->timer_ack_callback;

	device->save_item(NAME(ic_315_5250->counter));
	device->save_item(NAME(ic_315_5250->bit));
	device->save_item(NAME(ic_315_5250->regs));
}

static DEVICE_RESET( ic_315_5250 )
{
	ic_315_5250_state *ic_315_5250 = _315_5250_get_safe_token(device);

	memset(&ic_315_5250, 0, sizeof(ic_315_5250));
}



DEVICE_GET_INFO( ic_315_5248 )
{
	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers ---
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ic_315_5248_state);					break;

		// --- the following bits of info are returned as pointers to data or functions ---
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ic_315_5248);		break;
		case DEVINFO_FCT_STOP:					// Nothing									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ic_315_5248);		break;

		// --- the following bits of info are returned as NULL-terminated strings ---
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega 315-5248");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Custom IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( ic_315_5249 )
{
	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers ---
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ic_315_5249_state);					break;

		// --- the following bits of info are returned as pointers to data or functions ---
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ic_315_5249);		break;
		case DEVINFO_FCT_STOP:					// Nothing									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ic_315_5249);		break;

		// --- the following bits of info are returned as NULL-terminated strings ---
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega 315-5249");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Custom IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( ic_315_5250 )
{
	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers ---
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ic_315_5250_state);					break;

		// --- the following bits of info are returned as pointers to data or functions ---
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ic_315_5250);		break;
		case DEVINFO_FCT_STOP:					// Nothing									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ic_315_5250);		break;

		// --- the following bits of info are returned as NULL-terminated strings ---
		case DEVINFO_STR_NAME:					strcpy(info->s, "Sega 315-5250");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Sega Custom IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_DEVICE(_315_5248, ic_315_5248);
DEFINE_LEGACY_DEVICE(_315_5249, ic_315_5249);
DEFINE_LEGACY_DEVICE(_315_5250, ic_315_5250);
