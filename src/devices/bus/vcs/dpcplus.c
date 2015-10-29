// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

Atari 2600 cart with DPC+

the DPC+ adds an ARM CPU, including video improvements plus an extra synthesizer.

Some info on the DPC+ hardware can be found on Darrell Spice Jr's guides:
http://atariage.com/forums/blog/148/entry-11811-dpcarm-part-6-dpc-cartridge-layout/
http://atariage.com/forums/blog/148/entry-11883-dpcarm-part-7-6507arm-exchange-of-information/
http://atariage.com/forums/blog/148/entry-11903-dpcarm-part-8-multiple-functions/
http://atariage.com/forums/blog/148/entry-11935-dpcarm-part-9-functional-menu/
http://atariage.com/forums/blog/148/entry-11964-dpcarm-part-10-score-timer-display/
http://atariage.com/forums/blog/148/entry-11988-dpcarm-part-12-gamepad-support/

map:
	Bankswitching uses addresses $FFF6-$FFFB

	* ARM RAM mapped at $40000000 in this area
	$0000-$0BFF: DPC+ driver (not accessible by 2600 itself) (copied to $40000000 - $40000bff on startup by ARM)
	$0C00-$1BFF: Bank 0 (each bank can map to 0x1000 - 0x1fff in 6507 space, like other carts)
	$1C00-$2BFF: Bank 1
	$2C00-$3BFF: Bank 2
	$3C00-$4BFF: Bank 3
	$4C00-$5BFF: Bank 4
	$5C00-$6BFF: Bank 5 (default bank is bank 5)
	$6C00-$7BFF: Display Data (indirect access) (copied to $40000C00 - $40001bff on startup by ARM)
	$7C00-$7FFF: Synth Frequency Data (not accessible by 2600 itself) (copied to $40001C00 - $40001fff on startup by ARM)

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
	m_base_bank = 5;
}


READ8_MEMBER(a26_rom_dpcplus_device::read8_r)
{
	return m_rom[offset + (m_base_bank * 0x1000)];
}


READ32_MEMBER(a26_rom_dpcplus_device::armrom_r)
{
	UINT32 ret = (m_rom[offset * 4 + 3] << 24) |
   		         (m_rom[offset * 4 + 2] << 16) |
		         (m_rom[offset * 4 + 1] << 8) |
	             (m_rom[offset * 4 + 0] << 0);
	return ret;
}

WRITE32_MEMBER(a26_rom_dpcplus_device::armrom_w)
{

}

READ32_MEMBER(a26_rom_dpcplus_device::arm_E01FC088_r)
{
	return 0xffffffff;
}

static ADDRESS_MAP_START( dpcplus_arm7_map, AS_PROGRAM, 32, a26_rom_dpcplus_device )
	// todo: implement all this correctly
	AM_RANGE(0x00000000, 0x00007fff) AM_READWRITE(armrom_r,armrom_w) // flash, 32k
	AM_RANGE(0x40000000, 0x40001fff) AM_RAM // sram, 8k

	AM_RANGE(0xE01FC088, 0xE01FC08b) AM_READ(arm_E01FC088_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( a26_dpcplus )
	MCFG_CPU_ADD("arm", ARM7, 70000000)
	MCFG_CPU_PROGRAM_MAP(dpcplus_arm7_map)
MACHINE_CONFIG_END

machine_config_constructor a26_rom_dpcplus_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a26_dpcplus );
}

void a26_rom_dpcplus_device::check_bankswitch(offs_t offset)
{
	switch (offset)
	{
	case 0x0FF6: m_base_bank = 0; break;
	case 0x0FF7: m_base_bank = 1; break;
	case 0x0FF8: m_base_bank = 2; break;
	case 0x0FF9: m_base_bank = 3; break;
	case 0x0FFa: m_base_bank = 4; break;
	case 0x0FFb: m_base_bank = 5; break;
	default: break;
	}
}

READ8_MEMBER(a26_rom_dpcplus_device::read_rom)
{
	UINT8 retvalue = read8_r(space, offset + 0xc00); // banks start at 0xc00

	check_bankswitch(offset);

	return retvalue;
}

WRITE8_MEMBER(a26_rom_dpcplus_device::write_bank)
{
	check_bankswitch(offset);
//	a26_rom_f8_device::write_bank(space, offset, data);
}
