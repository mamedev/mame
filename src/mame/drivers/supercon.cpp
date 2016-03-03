// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************
 Novag SuperConstellation Chess Computer

 CPU 6502
 Clock 4 MHz
 IRQ CLK 600 Hz

 RAM    0x0000, 0x0fff)
 ROM    0x2000, 0xffff)
 I/O    0x1c00              Unknown
        0x1d00              Unknown
        0x1e00              LED's and buttons
        0x1f00              LED's, buttons and buzzer


******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/beep.h"


class supercon_state : public driver_device
{
public:
	supercon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beep(*this, "beeper")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
};

#define MAIN_CLOCK 4000000 /* 4 MHz */
/* Address maps */

static ADDRESS_MAP_START(supercon_mem, AS_PROGRAM, 8, supercon_state)
	AM_RANGE( 0x0000, 0x0fff) AM_RAM
	AM_RANGE( 0x2000, 0x7fff) AM_ROM
	AM_RANGE( 0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Input ports */

static INPUT_PORTS_START( supercon )
INPUT_PORTS_END

/* Machine driver */
static MACHINE_CONFIG_START( supercon, supercon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(supercon_mem)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/* ROM definition */

ROM_START(supercon)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("supercon_low.bin",  0x0000,  0x8000, CRC(b853cf6e) SHA1(1a759072a5023b92c07f1fac01b7a21f7b5b45d0 ))
	ROM_LOAD("supercon_high.bin", 0x8000,  0x8000, CRC(c8f82331) SHA1(f7fd039f9a3344db9749931490ded9e9e309cfbe ))
ROM_END

/* Driver */

/*    YEAR  NAME          PARENT  COMPAT  MACHINE    INPUT       INIT      COMPANY  FULLNAME                     FLAGS */
CONS( 1983, supercon,     0,      0,      supercon,  supercon, driver_device,   0, "Novag", "SuperConstellation", MACHINE_IS_SKELETON )
