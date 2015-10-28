// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

Atari 2600 cart with DPC+

the DPC+ adds an ARM CPU amongst other things (display improvements, etc.)

map (according to a blogpost on atariage):
	NOTE: All banks are accessible via $F000

	* ARM RAM mapped at $40000000 in this area
	$0000-$0BFF: DPC+ driver (not accessible by 2600 itself)
	$0C00-$1BFF: Bank 0
	$1C00-$2BFF: Bank 1
	$2C00-$3BFF: Bank 2
	$3C00-$4BFF: Bank 3
	$4C00-$5BFF: Bank 4
	$5C00-$6BFF: Bank 5
	* ARM RAM mapped at $40000C00 in this area
	$6C00-$7BFF: Display Data (indirect access)
	* ARM RAM mapped at $40001C00 in this area
	$7C00-$7FFF: Frequency Data (not accessible by 2600 itself)

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
	// todo: implement all this correctly
	//AM_RANGE(0x00000000, 0x00007fff) AM_ROM // flash, 32k
	AM_RANGE(0x40000000, 0x40001fff) AM_RAM // sram, 8k
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( a26_dpcplus )
	MCFG_CPU_ADD("arm", ARM7, 70000000)    // correct type?
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
