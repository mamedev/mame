// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    TeleVideo TVI-912/TVI-920 terminals

    This was the first series of terminals from TeleVideo. The models differed
    from each other in the number and pattern of keys on the nondetachable
    keyboard. Those with a B suffix had a TTY-style keyboard, while the C
    suffix indicated a typewriter-style keyboard. The TVI-920 added a row of
    function keys but was otherwise mostly identical to the TVI-912.

*******************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
//#include "bus/rs232/rs232.h"
//#include "machine/ay31015.h"
//#include "video/tms9927.h"
#include "screen.h"

#define CHAR_WIDTH 14

class tv912_state : public driver_device
{
public:
	tv912_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

u32 tv912_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( prog_map, AS_PROGRAM, 8, tv912_state )
	AM_RANGE(0x000, 0xfff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, tv912_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( tv912c )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tv912 )
	MCFG_CPU_ADD("maincpu", I8035, XTAL_23_814MHz / 4)
	MCFG_CPU_PROGRAM_MAP(prog_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_23_814MHz, 105 * CHAR_WIDTH, 0, 80 * CHAR_WIDTH, 270, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(tv912_state, screen_update)
MACHINE_CONFIG_END

/**************************************************************************************************************

Televideo TVI-912C.
Chips: i8035, TMS9927NL, AY5-1013A (COM2502)
Crystals: 23.814 (divide by 4 for CPU clock)
Other: 1x 8-sw DIP, 1x 10-sw DIP (internal), 2x 10-sw DIP (available to user at the back)

***************************************************************************************************************/

ROM_START( tv912c )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "a49c1.bin",    0x0000, 0x1000, CRC(d21851bf) SHA1(28fe77a218a5eee11de376f5d16e9380b616b3ca) BAD_DUMP ) // last half is all FF

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "a3-2.bin",     0x0000, 0x0800, CRC(bb9a7fbd) SHA1(5f1c4d41b25bd3ca4dbc336873362935daf283da) )
ROM_END

COMP( 1978, tv912c, 0, 0, tv912, tv912c, tv912_state, 0, "TeleVideo Systems", "TVI-912C", MACHINE_IS_SKELETON )
