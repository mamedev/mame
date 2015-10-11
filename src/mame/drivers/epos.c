// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Epos games

    driver by Zsolt Vasvari


    Notes:

    - To walk in IGMO, hold down button 2.
    - Super Glob seems like a later revision of The Glob, the most obvious
      difference being an updated service mode.
    - These games don't have cocktail mode.
    - The divisor 4 was derived using the timing loop used to split the screen
      in the middle.  This loop takes roughly 24200 cycles, giving
      2500 + (24200 - 2500) * 2 * 60 = 2754000 = 2.75MHz for the CPU speed,
      assuming 60 fps and a 2500 cycle VBLANK period.
      This should be easy to check since the schematics are available, .
    - I think theglob2 is earlier than theglob.  They only differ in one routine,
      but it appears to be a bug fix.  Also, theglob3 appears to be even older.

    To Do:

    - Super Blob uses a busy loop during the color test to split the screen
      between the two palettes.  This effect is not emulated, but since both
      halfs of the palette are identical, this is not an issue.  See $039c.
      The other games have a different color test, not using the busy loop.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "includes/epos.h"

WRITE8_MEMBER(epos_state::dealer_decrypt_rom)
{
	if (offset & 0x04)
		m_counter = (m_counter + 1) & 0x03;
	else
		m_counter = (m_counter - 1) & 0x03;

//  logerror("PC %08x: ctr=%04x\n",space.device().safe_pc(), m_counter);

	membank("bank1")->set_entry(m_counter);

	// is the 2nd bank changed by the counter or it always uses the 1st key?
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( epos_map, AS_PROGRAM, 8, epos_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( dealer_map, AS_PROGRAM, 8, epos_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x6000, 0x6fff) AM_ROMBANK("bank2")
	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

/*************************************
 *
 *  Main CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( io_map, AS_IO, 8, epos_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("SYSTEM") AM_WRITE(epos_port_1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("INPUTS") AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("UNK")
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("aysnd", ay8910_device, address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dealer_io_map, AS_IO, 8, epos_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x20, 0x24) AM_WRITE(dealer_decrypt_rom)
	AM_RANGE(0x34, 0x34) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x38, 0x38) AM_READ_PORT("DSW")
	AM_RANGE(0x3C, 0x3C) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


/*
   ROMs U01-U03 are checked with the same code in a loop.
   There's a separate ROM check for banked U04 at 30F3.
   It looks like dealer/revenger uses ppi8255 to control bankswitching.
*/
WRITE8_MEMBER(epos_state::write_prtc)
{
	membank("bank2")->set_entry(data & 0x01);
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* I think the upper two bits of port 1 are used as a simple form of protection,
   so that ROMs couldn't be simply swapped.  Each game checks these bits and halts
   the processor if an unexpected value is read. */

static INPUT_PORTS_START( megadon )
	PORT_START("DSW")

// There are odd port mappings (old=new)
// 02=10, 04=40, 08=02, 10=20, 20=04, 40=08

	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x02, 0x00, "Fuel Consumption" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Enemy Fire Rate" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x04, 0x00, "Rotation" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x08, 0x08, "ERG" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Game Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Arcade" )
	PORT_DIPSETTING(    0x80, "Contest" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SPECIAL )   /* this has to be HI */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SPECIAL )   /* this has to be HI */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( suprglob )
	PORT_START("DSW")

// There are odd port mappings (old=new)
// 02=10, 04=40, 08=20, 10=02, 20=04, 40=08

	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x26, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x22, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x24, "7" )
	PORT_DIPSETTING(    0x26, "8" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "10000 + Difficulty * 10000" )
	PORT_DIPSETTING(    0x08, "90000 + Difficulty * 10000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* this has to be LO */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SPECIAL )   /* this has to be HI */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( igmo )
	PORT_START("DSW")

// There are odd port mappings (old=new)
// 02=10, 04=40, 08=20, 10=02, 20=04, 40=08

	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x22, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x22, "80000" )
	PORT_DIPNAME( 0x8c, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x84, "6" )
	PORT_DIPSETTING(    0x88, "7" )
	PORT_DIPSETTING(    0x8c, "8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SPECIAL )   /* this has to be HI */
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SPECIAL )   /* this has to be HI */

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( catapult )
	PORT_INCLUDE(igmo)
		PORT_MODIFY("DSW")

// There are odd port mappings (old=new)
// 02=08, 04=20, 08=40, 10=02, 20=10, 40=04

	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x50, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x50, "6" )
	PORT_DIPNAME( 0x22, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x22, "4" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x08, "60000" )
	PORT_DIPSETTING(    0x0c, "80000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( dealer )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) //cancel
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) //draw
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) //stand
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) //play
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) //coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_START_MEMBER(epos_state,epos)
{
	save_item(NAME(m_palette));
	save_item(NAME(m_counter));
}

void epos_state::machine_reset()
{
	m_palette = 0;
	m_counter = 0;
}


MACHINE_START_MEMBER(epos_state,dealer)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &ROM[0x0000], 0x10000);
	membank("bank2")->configure_entries(0, 2, &ROM[0x6000], 0x1000);

	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);

	MACHINE_START_CALL_MEMBER(epos);
}

static MACHINE_CONFIG_START( epos, epos_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 11000000/4)    /* 2.75 MHz (see notes) */
	MCFG_CPU_PROGRAM_MAP(epos_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", epos_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(272, 241)
	MCFG_SCREEN_VISIBLE_AREA(0, 271, 0, 235)
	MCFG_SCREEN_UPDATE_DRIVER(epos_state, screen_update_epos)
	MCFG_SCREEN_ORIENTATION(ROT270)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 11000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( dealer, epos_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 11000000/4)    /* 2.75 MHz (see notes) */
	MCFG_CPU_PROGRAM_MAP(dealer_map)
	MCFG_CPU_IO_MAP(dealer_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", epos_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("INPUTS"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(epos_state, write_prtc))

	MCFG_MACHINE_START_OVERRIDE(epos_state,dealer)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(272, 241)
	MCFG_SCREEN_VISIBLE_AREA(0, 271, 0, 235)
	MCFG_SCREEN_UPDATE_DRIVER(epos_state, screen_update_epos)
	MCFG_SCREEN_ORIENTATION(ROT270)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 11000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( megadon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732u10b.bin",   0x0000, 0x1000, CRC(af8fbe80) SHA1(2d7857616462112fe17343a9357ee51d8f965a0f) )
	ROM_LOAD( "2732u09b.bin",   0x1000, 0x1000, CRC(097d1e73) SHA1(b6141155b2c63c33a367dd18fe53ff9f01b99380) )
	ROM_LOAD( "2732u08b.bin",   0x2000, 0x1000, CRC(526784da) SHA1(7d9f43dc6975a018bec95982029ce7ac9f675869) )
	ROM_LOAD( "2732u07b.bin",   0x3000, 0x1000, CRC(5b060910) SHA1(98a719bf0ffe8010437565de681aaefa647d9a6c) )
	ROM_LOAD( "2732u06b.bin",   0x4000, 0x1000, CRC(8ac8af6d) SHA1(53c123f0e9f0443737c39c01dbdb685189cffa92) )
	ROM_LOAD( "2732u05b.bin",   0x5000, 0x1000, CRC(052bb603) SHA1(eb74a9563f44cca50dc2c475e4a376ed14e4f56f) )
	ROM_LOAD( "2732u04b.bin",   0x6000, 0x1000, CRC(9b8b7e92) SHA1(051ad9a8ba51740a865e3c95a738658b30bbbe60) )
	ROM_LOAD( "2716u11b.bin",   0x7000, 0x0800, CRC(599b8b61) SHA1(e687c6f475a0fead3e47f05b1d1b3b29cf4a83a1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.bin",     0x0000, 0x0020, CRC(c779ea99) SHA1(7702ae3684579950b36274ea91d4267c96faeeb8) )
ROM_END


ROM_START( catapult )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "co3223.u10",     0x0000, 0x1000, CRC(50abcfd2) SHA1(13ce04addc7bcaa1ec6659da26b1c13ed9dc28f9) )
	ROM_LOAD( "co3223.u09",     0x1000, 0x1000, CRC(fd5a9a1c) SHA1(512374e8450459537ba2cc41e7d0178052445316) )
	ROM_LOAD( "co3223.u08",     0x2000, 0x1000, BAD_DUMP CRC(4bfc36f3) SHA1(b916805eed40cfeff0c1b0cb3cdcbcc6e362a236)  ) /* BADADDR xxxx-xxxxxxx */
	ROM_LOAD( "co3223.u07",     0x3000, 0x1000, CRC(4113bb99) SHA1(3cebb874dae211d75082209e913d4afa4f621de1) )
	ROM_LOAD( "co3223.u06",     0x4000, 0x1000, CRC(966bb9f5) SHA1(1a217c6f7a88c58e0deae0290bc5ddd2789d18eb) )
	ROM_LOAD( "co3223.u05",     0x5000, 0x1000, CRC(65f9fb9a) SHA1(63b616a736d9e39a8f2f76889fd7c5fe4128a966) )
	ROM_LOAD( "co3223.u04",     0x6000, 0x1000, CRC(648453bc) SHA1(8e4538aedad4d32bd046aad474dbcc689ee8fe53) )
	ROM_LOAD( "co3223.u11",     0x7000, 0x0800, CRC(08fb8c28) SHA1(0b08cc2727a54e0ad7472234be0f637b46bc3253) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "co3223.u66",     0x0000, 0x0020, CRC(e7de76a7) SHA1(101ce85459a59c0d01ce3ea96480f1f8413a788e) )
ROM_END


ROM_START( suprglob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u10",            0x0000, 0x1000, CRC(c0141324) SHA1(a54bd71da233eb22f45da630693fddd5a0bcf25b) )
	ROM_LOAD( "u9",             0x1000, 0x1000, CRC(58be8128) SHA1(534f0a093b3ff577a2a5461498bc11ce14dc6d97) )
	ROM_LOAD( "u8",             0x2000, 0x1000, CRC(6d088c16) SHA1(0929ea1b58eab997b5d9c9642f8b47557a4045f1) )
	ROM_LOAD( "u7",             0x3000, 0x1000, CRC(b2768203) SHA1(9de52f4dbe6a46ea1b9b7f9cf70378211d372353) )
	ROM_LOAD( "u6",             0x4000, 0x1000, CRC(976c8f46) SHA1(120c76eff8c04ccb5ad945c4333e8c9de0cbc3af) )
	ROM_LOAD( "u5",             0x5000, 0x1000, CRC(340f5290) SHA1(2e5fa0c41d1626e5a435f2c55eec0bcdcb004223) )
	ROM_LOAD( "u4",             0x6000, 0x1000, CRC(173bd589) SHA1(25690a0c3cd0e017f8d220d8fbf2eaeb86f05fc5) )
	ROM_LOAD( "u11",            0x7000, 0x0800, CRC(d45b740d) SHA1(54c15f378b6d91ea1aba0a51921178bb15854079) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( theglob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "globu10.bin",    0x0000, 0x1000, CRC(08fdb495) SHA1(739efa676b5a3df36a6061382aeb8c2d495ba23f) )
	ROM_LOAD( "globu9.bin",     0x1000, 0x1000, CRC(827cd56c) SHA1(3aedc1cefb463cf6b31befb33e50c832dc2e3941) )
	ROM_LOAD( "globu8.bin",     0x2000, 0x1000, CRC(d1219966) SHA1(571349f9c978fdcf826a0c66c3fb11a9e27b240a) )
	ROM_LOAD( "globu7.bin",     0x3000, 0x1000, CRC(b1649da7) SHA1(1509d48a72e545195e45d1170cdb113c6aecc8d9) )
	ROM_LOAD( "globu6.bin",     0x4000, 0x1000, CRC(b3457e67) SHA1(1347bdf085ad69879f9a9e7e4ed1ca4869e8e8cd) )
	ROM_LOAD( "globu5.bin",     0x5000, 0x1000, CRC(89d582cd) SHA1(f331c7a2fce606153992abb312c5406251a7fb3b) )
	ROM_LOAD( "globu4.bin",     0x6000, 0x1000, CRC(7ee9fdeb) SHA1(a8e0dd5d1cdcff132edc0eb182b66656ce244fa1) )
	ROM_LOAD( "globu11.bin",    0x7000, 0x0800, CRC(9e05dee3) SHA1(751799b23f0e664f59d3785b438ec3ae9f5bab2c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( theglob2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "611293.u10",     0x0000, 0x1000, CRC(870af7ce) SHA1(f901619663313a72997f30ccecdeac8294fe200e) )
	ROM_LOAD( "611293.u9",      0x1000, 0x1000, CRC(a3679782) SHA1(fbc26ae98e2bf10272d61159b084d78a6f410374) )
	ROM_LOAD( "611293.u8",      0x2000, 0x1000, CRC(67499d1a) SHA1(dce7041df5ed1847e0ffc82672d09e00b16de3a9) )
	ROM_LOAD( "611293.u7",      0x3000, 0x1000, CRC(55e53aac) SHA1(20a428db287e8b7fb55cb9fe1a1ed0196481114c) )
	ROM_LOAD( "611293.u6",      0x4000, 0x1000, CRC(c64ad743) SHA1(572ff6acb9b2281581974646e96699d7d2388aff) )
	ROM_LOAD( "611293.u5",      0x5000, 0x1000, CRC(f93c3203) SHA1(8cb88b5202e99d206eccf7d25e168cf23acee19b) )
	ROM_LOAD( "611293.u4",      0x6000, 0x1000, CRC(ceea0018) SHA1(511430539429ef0e5368f7b605f2e680ca9038bc) )
	ROM_LOAD( "611293.u11",     0x7000, 0x0800, CRC(6ac83f9b) SHA1(b1e8482ec04107f0e595a714b7c0f70571aca6e5) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( theglob3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "theglob3.u10",   0x0000, 0x1000, CRC(969cfaf6) SHA1(b63226b8694640d6452bca12755780d1b52d1d3c) )
	ROM_LOAD( "theglob3.u9",    0x1000, 0x1000, CRC(8e6c010a) SHA1(ec9627742ce52eb29bbafc9d0555d16ac7146f2e) )
	ROM_LOAD( "theglob3.u8",    0x2000, 0x1000, CRC(1c1ca5c8) SHA1(6e5f9d7f9f016a72003433375c806c5f921ed423) )
	ROM_LOAD( "theglob3.u7",    0x3000, 0x1000, CRC(a54b9d22) SHA1(3db96d1f55642ecf1ebc76387cac76e8f9721919) )
	ROM_LOAD( "theglob3.u6",    0x4000, 0x1000, CRC(5a6f82a9) SHA1(ea92ad949373e8b1f06c65f243ceedad2fdcd934) )
	ROM_LOAD( "theglob3.u5",    0x5000, 0x1000, CRC(72f935db) SHA1(d7023cf5f16a77a42590a9c97c2690ac0e3d282a) )
	ROM_LOAD( "theglob3.u4",    0x6000, 0x1000, CRC(81db53ad) SHA1(a1e4aa8e08ca0f585b3638a3849a465977d44af0) )
	ROM_LOAD( "theglob3.u11",   0x7000, 0x0800, CRC(0e2e6359) SHA1(f231637ad4c997406989cf5a701d26c95e69171e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, CRC(f4f6ddc5) SHA1(cab915acbefb5f451f538dd538bf9b3dd14bb1f5) )
ROM_END


ROM_START( igmo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "igmo-u10.732",   0x0000, 0x1000, CRC(a9f691a4) SHA1(e3f2dc41bd8760fc52e99b7e9faa12c7cf51ffe0) )
	ROM_LOAD( "igmo-u9.732",    0x1000, 0x1000, CRC(3c133c97) SHA1(002b5aff6b947b6a9cbabeed5be798c1ddf2bda1) )
	ROM_LOAD( "igmo-u8.732",    0x2000, 0x1000, CRC(5692f8d8) SHA1(6ab50775dff49330a85fbfb2d4d4c3a2e54df3d1) )
	ROM_LOAD( "igmo-u7.732",    0x3000, 0x1000, CRC(630ae2ed) SHA1(0c293b6192e703b16ed20c277c706ae90773f477) )
	ROM_LOAD( "igmo-u6.732",    0x4000, 0x1000, CRC(d3f20e1d) SHA1(c0e0b542ac020adc085ec90c2462c6544098447e) )
	ROM_LOAD( "igmo-u5.732",    0x5000, 0x1000, CRC(e26bb391) SHA1(ba0e44c02fbb36e18e0d779d46bb992e6aba6cf1) )
	ROM_LOAD( "igmo-u4.732",    0x6000, 0x1000, CRC(762a4417) SHA1(7fed5221950e3e1ce41c0b4ded44597a242a0177) )
	ROM_LOAD( "igmo-u11.716",   0x7000, 0x0800, CRC(8c675837) SHA1(2725729693960b53ea01ebffa0a81df2cd425890) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, NO_DUMP )   /* missing */
ROM_END


ROM_START( dealer )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "u1.bin",         0x0000, 0x2000, CRC(e06f3563) SHA1(0d58cd1f2e1ca89adb9c64d7dd520bb1f2d50f1a) )
	ROM_LOAD( "u2.bin",         0x2000, 0x2000, CRC(726bbbd6) SHA1(3538f3d655899c2a0f984c43fb7545ea4be1b231) )
	ROM_LOAD( "u3.bin",         0x4000, 0x2000, CRC(ab721455) SHA1(a477da0590e0431172baae972e765473e19dcbff) )
	ROM_LOAD( "u4.bin",         0x6000, 0x2000, CRC(ddb903e4) SHA1(4c06a2048b1c6989c363b110a17c33180025b9c8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, NO_DUMP )   /* missing */
ROM_END

/*

Revenger EPOS 1984

EPOS TRISTAR 9000



   8910   Z80A    4116  4116
                  4116  4116
                  4116  4116
    6116          4116  4116
    6116          4116  4116
    U4            4116  4116     74S189
    U3            4116  4116     74S189
    U2            4116  4116
    U1        8255
                             22.1184MHz
*/

ROM_START( revenger )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "r06124.u1",    0x0000, 0x2000, CRC(fad1a2a5) BAD_DUMP SHA1(a31052c91fe67e2e90441abc40b6483f921ecfe3) )
	ROM_LOAD( "r06124.u2",    0x2000, 0x2000, CRC(a8e0ee7b) BAD_DUMP SHA1(f6f78e8ce40eab07de461b364876c1eb4a78d96e) )
	ROM_LOAD( "r06124.u3",    0x4000, 0x2000, CRC(cca414a5) BAD_DUMP SHA1(1c9dd3ff63d57e9452e63083cdbd7f5d693bb686) )
	ROM_LOAD( "r06124.u4",    0x6000, 0x2000, CRC(0b81c303) BAD_DUMP SHA1(9022d18dec11312eb4bb471c22b563f5f897b4f7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.u66",     0x0000, 0x0020, NO_DUMP )   /* missing */
ROM_END

DRIVER_INIT_MEMBER(epos_state,dealer)
{
	UINT8 *rom = memregion("maincpu")->base();
	int A;

	/* Key 0 */
	for (A = 0; A < 0x8000; A++)
		rom[A] = BITSWAP8(rom[A] ^ 0xbd, 2,6,4,0,5,7,1,3 );

	/* Key 1 */
	for (A = 0; A < 0x8000; A++)
		rom[A + 0x10000] = BITSWAP8(rom[A], 7,5,4,6,3,2,1,0 );

	/* Key 2 */
	for (A = 0; A < 0x8000; A++)
		rom[A + 0x20000] = BITSWAP8(rom[A] ^ 1, 7,6,5,4,3,0,2,1 );

	/* Key 3 */
	for (A = 0; A < 0x8000; A++)
		rom[A + 0x30000] = BITSWAP8(rom[A] ^ 1, 7,5,4,6,3,0,2,1 );

	/*
	    there is not enough data to determine key 3.
	    the code in question is this:

	    [this is the data as decrypted by Key 1]
	    2F58: 55 5C 79
	    2F5B: 55 F7 79
	    2F5E: 55 CD 79

	    it must become

	    2F58: 32 3e 78  ld (793e),a
	    2F5B: 32 xx 78  ld (79xx),a
	    2F5E: 32 xx 78  ld (79xx),a

	    the obvious solution is a combination of key 1 and key 2.
	*/
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, megadon,  0,        epos,   megadon, driver_device,  0,       ROT270, "Epos Corporation (Photar Industries license)", "Megadon", MACHINE_SUPPORTS_SAVE )
GAME( 1982, catapult, 0,        epos,   catapult, driver_device, 0,       ROT270, "Epos Corporation", "Catapult", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* bad rom, hold f2 for test mode */
GAME( 1983, suprglob, 0,        epos,   suprglob, driver_device, 0,       ROT270, "Epos Corporation", "Super Glob", MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglob,  suprglob, epos,   suprglob, driver_device, 0,       ROT270, "Epos Corporation", "The Glob", MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglob2, suprglob, epos,   suprglob, driver_device, 0,       ROT270, "Epos Corporation", "The Glob (earlier)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglob3, suprglob, epos,   suprglob, driver_device, 0,       ROT270, "Epos Corporation", "The Glob (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, igmo,     0,        epos,   igmo, driver_device,     0,       ROT270, "Epos Corporation", "IGMO", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, dealer,   0,        dealer, dealer, epos_state,   dealer,   ROT270, "Epos Corporation", "The Dealer", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, revenger, 0,        dealer, dealer, epos_state,   dealer,   ROT270, "Epos Corporation", "Revenger", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
