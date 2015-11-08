// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

Harmony / Melody cart for the A2600

The Harmony cart is a 'modern' A2600 cartridge, used for homebrew etc. It has
an SD slot and can be connected to a PC, roms can be transfered to it with
software on the PC side.  It uses an ARM7TDMI-S LPC2103 @ 70 Mhz to emulate
the mapper behavior of other cartridges.  It has an SD card slot for storing
game data.

The Melody version of the cartridge has been used for several recent A2600
commercial releases as well as some reproductions due to it's ability to be
programmed as any other cartridge type.  This lacks the SD slot?

The 'DPC+' games by SpiceWare run on a Harmony / Melody cart, DPC+ seems to
be a virtual 'software mapper' programmed on the ARM rather than a real mapper.


There is also a 'Harmony Encore' cartridge which adds support for some of the
games the original couldn't handle due to them having larger ROMs and more
complex banking schemes (Stella's Stocking etc.)

some Harmony cart details can be found at
http://atariage.com/forums/topic/156500-latest-harmony-cart-software/


DPC+ notes
----------

Some info on the Harmony / Melody when configured as DPC+ hardware can be found on Darrell Spice Jr's guides:
http://atariage.com/forums/blog/148/entry-11811-dpcarm-part-6-dpc-cartridge-layout/
http://atariage.com/forums/blog/148/entry-11883-dpcarm-part-7-6507arm-exchange-of-information/
http://atariage.com/forums/blog/148/entry-11903-dpcarm-part-8-multiple-functions/
http://atariage.com/forums/blog/148/entry-11935-dpcarm-part-9-functional-menu/
http://atariage.com/forums/blog/148/entry-11964-dpcarm-part-10-score-timer-display/
http://atariage.com/forums/blog/148/entry-11988-dpcarm-part-12-gamepad-support/

map:
	Bankswitching uses addresses $FFF6-$FFFB

	* ARM RAM mapped at $40000000 in this area
	$0000-$0BFF: HARMONY/MELODY driver (not accessible by 2600 itself) (copied to $40000000 - $40000bff on startup by ARM)
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
#include "harmony_melody.h"





// cart device

const device_type A26_ROM_HARMONY = &device_creator<a26_rom_harmony_device>;


a26_rom_harmony_device::a26_rom_harmony_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: a26_rom_f8_device(mconfig, A26_ROM_HARMONY, "Atari 2600 ROM Cart HARMONY/MELODY", tag, owner, clock, "a2600_harmony", __FILE__),
						m_cpu(*this, "arm")
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_harmony_device::device_start()
{
	save_item(NAME(m_base_bank));
}

static ADDRESS_MAP_START( harmony_arm7_map, AS_PROGRAM, 32, a26_rom_harmony_device )
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( a26_harmony )
	MCFG_CPU_ADD("arm", LPC2103, 70000000)
	MCFG_CPU_PROGRAM_MAP(harmony_arm7_map)
MACHINE_CONFIG_END

machine_config_constructor a26_rom_harmony_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a26_harmony );
}

// actually if the ARM code is doing this and providing every opcode to the main CPU based
// on bus activity then we shouldn't be doing and of this here (if the ROM is actually
// the internal Flash rom of the ARM CPU then the A2600 CPU won't be able to see it directly
// at all?)
//
// instead we need the ARM monitoring the bus at all times and supplying the code on
// demand transparent to the main CPU? is this theory correct?

void a26_rom_harmony_device::device_reset()
{
	m_base_bank = 5;

	memcpy(m_cpu->m_flash, m_rom, 0x8000);
	m_cpu->reset();
}

READ8_MEMBER(a26_rom_harmony_device::read8_r)
{
	return m_rom[offset + (m_base_bank * 0x1000)];
}


void a26_rom_harmony_device::check_bankswitch(offs_t offset)
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

READ8_MEMBER(a26_rom_harmony_device::read_rom)
{
	UINT8 retvalue = read8_r(space, offset + 0xc00); // banks start at 0xc00

	check_bankswitch(offset);

	return retvalue;
}

WRITE8_MEMBER(a26_rom_harmony_device::write_bank)
{
	check_bankswitch(offset);
//	a26_rom_f8_device::write_bank(space, offset, data);
}
