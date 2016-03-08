// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

    Novag Super Constellation Chess Computer (model 844)

- UMC UM6502C @ 4 MHz (8MHz XTAL), 600Hz IRQ(source unknown?)
- 2*2KB RAM TC5516APL-2 battery-backed, 2*32KB ROM custom label
- TTL, 24 LEDs, 8*8 chessboard buttons
- external port for clock and printer, not emulated here

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/nvram.h"

// internal artwork
#include "supercon.lh"


class supercon_state : public driver_device
{
public:
	supercon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
};


/* Address maps */

static ADDRESS_MAP_START(supercon_mem, AS_PROGRAM, 8, supercon_state)
	AM_RANGE( 0x0000, 0x0fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE( 0x2000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */

static INPUT_PORTS_START( supercon )
INPUT_PORTS_END

/* Machine driver */

static MACHINE_CONFIG_START( supercon, supercon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_8MHz/2)
	MCFG_CPU_PERIODIC_INT_DRIVER(supercon_state, irq0_line_hold, 600) // ?
	MCFG_CPU_PROGRAM_MAP(supercon_mem)
	
	MCFG_NVRAM_ADD_0FILL("nvram")
	
	MCFG_DEFAULT_LAYOUT(layout_supercon)
MACHINE_CONFIG_END

/* ROM definition */

ROM_START(supercon)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_8441", 0x0000, 0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0) ) // label obscured by Q.C. sticker
	ROM_LOAD("novag_8442", 0x8000, 0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe) )
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT     INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1984, supercon,  0,      0,      supercon,  supercon, driver_device, 0, "Novag", "Super Constellation", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
