// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

Atari 2600 cart with DPC+

the DPC+ adds an ARM CPU amongst other things

***************************************************************************/


#include "emu.h"
#include "dpcplus.h"





// cart device

const device_type A26_ROM_DPCPLUS = &device_creator<a26_rom_dpcplus_device>;


a26_rom_dpcplus_device::a26_rom_dpcplus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: a26_rom_f8_device(mconfig, A26_ROM_DPCPLUS, "Atari 2600 ROM Cart DPC+", tag, owner, clock, "a2600_dpcplus", __FILE__)
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_dpcplus_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void a26_rom_dpcplus_device::device_reset()
{
	m_base_bank = 0;
}

static ADDRESS_MAP_START( dpcplus_arm7_map, AS_PROGRAM, 32, a26_rom_dpcplus_device )
	AM_RANGE(0x00000000, 0x00003fff) AM_RAM
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( a26_dpcplus )
	MCFG_CPU_ADD("arm", ARM7, 20000000)    // ? type ? speed
	MCFG_CPU_PROGRAM_MAP(dpcplus_arm7_map)
MACHINE_CONFIG_END

machine_config_constructor a26_rom_dpcplus_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a26_dpcplus );
}


READ8_MEMBER(a26_rom_dpcplus_device::read_rom)
{
	return a26_rom_f8_device::read_rom(space, offset);
}

WRITE8_MEMBER(a26_rom_dpcplus_device::write_bank)
{
	a26_rom_f8_device::write_bank(space, offset, data);
}
