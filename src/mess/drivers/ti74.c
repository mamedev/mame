// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  TI-74 BASICALC


  TODO:
  - x

***************************************************************************/

#include "emu.h"
#include "cpu/tms7000/tms7000.h"
#include "video/hd44780.h"

#include "ti74.lh"


class ti74_state : public driver_device
{
public:
	ti74_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<tms70c46_device> m_maincpu;

	virtual void machine_reset();
	virtual void machine_start();
	DECLARE_PALETTE_INIT(ti74);
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(ti74_state, ti74)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}



/***************************************************************************

  I/O, Memory Maps

***************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, ti74_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( ti74 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void ti74_state::machine_reset()
{
}

void ti74_state::machine_start()
{
}

static MACHINE_CONFIG_START( ti74, ti74_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS70C46, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*16, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9*2-1)
	MCFG_DEFAULT_LAYOUT(layout_ti74)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(ti74_state, ti74)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)
MACHINE_CONFIG_END



/***************************************************************************

  ROM Definitions

***************************************************************************/

ROM_START( ti74 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tms70c46.bin", 0xf000, 0x1000, CRC(55a2f7c0) SHA1(530e3de42f2e304c8f4805ad389f38a459ec4e33) ) // internal cpu rom

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "ti74.bin",     0x0000, 0x8000, CRC(019aaa2f) SHA1(04a1e694a49d50602e45a7834846de4d9f7d587d) ) // system rom, banked
ROM_END


COMP( 1985, ti74, 0, 0, ti74, ti74, driver_device, 0, "Texas Instruments", "TI-74 BASICALC", GAME_IS_SKELETON )
