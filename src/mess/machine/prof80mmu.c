// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Conitec PROF-80 Memory Management Unit emulation

**********************************************************************/

#include "prof80mmu.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PROF80_MMU = &device_creator<prof80_mmu_device>;


DEVICE_ADDRESS_MAP_START( z80_program_map, 8, prof80_mmu_device )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(program_r, program_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, prof80_mmu_device )
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  prof80_mmu_device - constructor
//-------------------------------------------------

prof80_mmu_device::prof80_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PROF80_MMU, "PROF80_MMU", tag, owner, clock, "prof80_mmu", __FILE__),
		device_memory_interface(mconfig, *this),
		m_program_space_config("program", ENDIANNESS_LITTLE, 8, 20, 0, *ADDRESS_MAP_NAME(program_map))
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

const address_space_config *prof80_mmu_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_space_config : NULL;
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
