// license:BSD-3-Clause
// copyright-holders: Scott Stone
/***************************************************************************

learnwin.cpp

Learning-Window Teaching Computer by VTech / Spiel Master by Yuvo (German)

Info from Kevin Horton (Kevtris):
The -081 on the 'speech' cart board had that mystery sp0256 next to it with its markings ground off.
The chip with the marking ground off is the most common SP0256 chip, the SP0256-AL2 chip
(it was common enough to be sold by electronic stores like Radio Shack in the USA)
and used on the currah speech cart for c64 and many many other places.

The rom for this exists in MAME as:
ROM_LOAD( "sp0256-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
On this type of v-tech/yeno speech cart, it had its internal rom disabled by tying one of the pins high on the board.

    Readme for the SPR128A chips:
    Chip Label - filename Address UndersideMark Type
    SPR128A-080 - sp128_80.bin 0000-3FFF 27-0643 L.window self-mapped
    SPR128A-081 - sp128_81.bin 0000-3FFF 27-0644 L.window self-mapped
    SPR128A-093 - sp128_93.bin 0000-3FFF 27-0754-00 L.window self-mapped

    Chip Label - Product
    SPR128A-080 - Learning Window system (German)
    SPR128A-081 - Learning Window speech cart (German)
    SPR128A-093 - Learning Window cart Alphabet Salat (German)

Part Numbers and Descriptions
SPR128A-046    -  Learning Window system US (rev 1)
SPR128A-047    -  Learning Window speech cart US (rev 1)
SPR128A-049    -  Learning Window cart Number Power  US
SPR128A-050    -  Learning Window cart Alphabet Soup  U.S.
SPR128A-055    -  Learning Window cart IQ Builder U.S.
SPR128A-069    -  Learning Window system french
SPR128A-077    -  Learning Window system US (rev 2)
SPR128A-080    -  Learning Window system (German)
SPR128A-081    -  Learning Window speech cart (German)
SPR128A-088    -  Learning Window system US (rev 3)
SPR128A-089    -  Learning Window speech cart US (rev 2)
SPR128A-093    -  Learning Window cart Alphabet Salat (German)
SPR128A-099    -  Learning Window speech cart US (rev 3)

***************************************************************************/


#include "emu.h"

#include "sound/sp0256.h"

#include "screen.h"
#include "speaker.h"


namespace {

class learnwin_state : public driver_device
{
public:
	learnwin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void learnwin(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};


static INPUT_PORTS_START( learnwin )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void learnwin_state::machine_start()
{
}


void learnwin_state::machine_reset()
{
}


void learnwin_state::learnwin(machine_config &config)
{
	/* video hardware */
//  screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
//  screen.set_refresh_hz(60);
//  screen.set_size(48, 32);
//  screen.set_visarea(0, 47, 0, 31);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( learnwin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sp128_88.bin",  0x0000, 0x4000, CRC(b71719b0) SHA1(22dc76d21a7ea29398b502d049a44d9aeeb97a87) ) // system
	ROM_LOAD( "sp128_99.bin",  0x4000, 0x4000, CRC(525e0d14) SHA1(444df5d31308f3e8dc8608d1c04277f506d4308b) ) // speech cart
	ROM_LOAD( "sp0256-al2.bin",0x8000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) ) // speech cart voice chip
	ROM_LOAD( "toshiba_t7984", 0xc000, 0x2000, NO_DUMP ) // unknown - possible MCU
ROM_END

ROM_START( learnwin2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sp128_77.bin",  0x0000, 0x4000, CRC(2c1d66e5) SHA1(2d25f80ace39d07d815f873541abb4071958b519) ) // system
	ROM_LOAD( "sp128_89.bin",  0x4000, 0x4000, CRC(455fd900) SHA1(3909e8b7267b53e07e2d03e1d81efab66efedafa) ) // speech cart
	ROM_LOAD( "sp0256-al2.bin",0x8000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) ) // speech cart voice chip
	ROM_LOAD( "toshiba_t7984", 0xc000, 0x2000, NO_DUMP ) // unknown - possible MCU
ROM_END

ROM_START( learnwin1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sp128_46.bin",  0x0000, 0x4000, CRC(a200771f) SHA1(347eda0a284457a82225c607c3bdd7baea0945c1) ) // system
	ROM_LOAD( "sp128_47.bin",  0x4000, 0x4000, CRC(c177641b) SHA1(c4d32019c4453f661f803ee3f018bdc9427de548) ) // speech cart
	ROM_LOAD( "sp0256-al2.bin",0x8000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) ) // speech cart voice chip
	ROM_LOAD( "toshiba_t7984", 0xc000, 0x2000, NO_DUMP ) // unknown - possible MCU
ROM_END

ROM_START( learnwinf )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sp128.069",     0x0000, 0x4000, CRC(af6076e1) SHA1(4efca85b86ca03724ee23360868de30cb2aa10ca) ) // system
	ROM_LOAD( "sp128_xx.bin",  0x4000, 0x4000, NO_DUMP ) // speech cart
	ROM_LOAD( "sp0256-al2.bin",0x8000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) ) // speech cart voice chip
	ROM_LOAD( "toshiba_t7984", 0xc000, 0x2000, NO_DUMP ) // unknown - possible MCU
ROM_END

ROM_START( spielmast )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sp128_80.bin",  0x0000, 0x4000, CRC(e5dcc22f) SHA1(7da642d52e980ef917e56b7488e2b8d1b63bd757) ) // system
	ROM_LOAD( "sp128_81.bin",  0x4000, 0x4000, CRC(9f761217) SHA1(d85bc443a7d5856335f3e4cf8742512555345af8) ) // speech cart
	ROM_LOAD( "sp0256-al2.bin",0x8000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) ) // speech cart voice chip
	ROM_LOAD( "toshiba_t7984", 0xc000, 0x2000, NO_DUMP ) // unknown - possible MCU
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1986, learnwin,  0,        0, learnwin, learnwin, learnwin_state, empty_init, "VTech", "Learning-Window Teaching Machine (Rev 3)",  MACHINE_IS_SKELETON )
COMP( 1986, learnwin2, learnwin, 0, learnwin, learnwin, learnwin_state, empty_init, "VTech", "Learning-Window Teaching Machine (Rev 2)",  MACHINE_IS_SKELETON )
COMP( 1986, learnwin1, learnwin, 0, learnwin, learnwin, learnwin_state, empty_init, "VTech", "Learning-Window Teaching Machine (Rev 1)",  MACHINE_IS_SKELETON )
COMP( 1986, learnwinf, learnwin, 0, learnwin, learnwin, learnwin_state, empty_init, "VTech", "Learning-Window Teaching Machine (French)", MACHINE_IS_SKELETON )
COMP( 1986, spielmast, learnwin, 0, learnwin, learnwin, learnwin_state, empty_init, "Yuvo",  "Spiel Master (German)",                     MACHINE_IS_SKELETON )
