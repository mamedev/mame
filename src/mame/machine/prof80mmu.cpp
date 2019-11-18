// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Conitec PROF-80 Memory Management Unit emulation

**********************************************************************/

#include "emu.h"
#include "prof80mmu.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PROF80_MMU, prof80_mmu_device, "prof80_mmu", "PROF80 MMU")


void prof80_mmu_device::z80_program_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(prof80_mmu_device::program_r), FUNC(prof80_mmu_device::program_w));
}

void prof80_mmu_device::program_map(address_map &map)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  prof80_mmu_device - constructor
//-------------------------------------------------

prof80_mmu_device::prof80_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PROF80_MMU, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_program_space_config("program", ENDIANNESS_LITTLE, 8, 20, 0, address_map_constructor(FUNC(prof80_mmu_device::program_map), this))
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void prof80_mmu_device::device_start()
{
	// state saving
	save_item(NAME(m_blk));
	save_item(NAME(m_enabled));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector prof80_mmu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_space_config)
	};
}

//-------------------------------------------------
//  par_w -
//-------------------------------------------------

WRITE8_MEMBER( prof80_mmu_device::par_w )
{
	int bank = offset >> 12;

	m_blk[bank] = (data & 0x0f) ^ 0x0f;
}


//-------------------------------------------------
//  mme_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( prof80_mmu_device::mme_w )
{
	m_enabled = (state == 1);
}


//-------------------------------------------------
//  program_r - program space read
//-------------------------------------------------

READ8_MEMBER( prof80_mmu_device::program_r )
{
	if (m_enabled)
	{
		int bank = offset >> 12;
		offset |= m_blk[bank] << 16;
	}
	else
	{
		offset |= 0xf0000;
	}

	return this->space(AS_PROGRAM).read_byte(offset);
}


//-------------------------------------------------
//  program_w - program space write
//-------------------------------------------------

WRITE8_MEMBER( prof80_mmu_device::program_w )
{
	if (m_enabled)
	{
		int bank = offset >> 12;
		offset |= m_blk[bank] << 16;
	}
	else
	{
		offset |= 0xf0000;
	}

	this->space(AS_PROGRAM).write_byte(offset, data);
}
