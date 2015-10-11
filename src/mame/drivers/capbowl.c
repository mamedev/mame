// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

    driver by Zsolt Vasvari

    Games supported:
        * Capcom/Coors Light Bowling
        * Bowlorama

    Known issues:
        * none

****************************************************************************

    CPU Board:

    0000-3fff     3 Graphics ROMS mapped in using 0x4800 (Coors Light Bowling only)
    0000-001f       Turbo board area (Bowl-O-Rama only) See Below.
    4000          Display row selected
    4800          Graphics ROM select
    5000-57ff     Battery backed up RAM (Saves machine state after shut off)
               Enter setup menu by holding down the F2 key on the
               high score screen
    5800-5fff       TMS34061 area

               First 0x20 bytes of each row provide a 16 color palette for this
               row. 2 bytes per color: 0000RRRR GGGGBBBB.

               Remaining 0xe0 bytes contain 2 pixels each for a total of
               448 pixels, but only 360 seem to be displayed.
               (Each row appears vertically because the monitor is rotated)

    6000          Sound command
    6800          Trackball Reset. Double duties as a watchdog.
    7000          Input port 1  Bit 0-3 Trackball Vertical Position
                                Bit 4   Player 2 Hook Left
                                Bit 5   Player 2 Hook Right
                                Bit 6   Upright/Cocktail DIP Switch
                                Bit 7   Coin 2
    7800          Input port 2  Bit 0-3 Trackball Horizontal Positon
                                Bit 4   Player 1 Hook Left
                                Bit 5   Player 1 Hook Right
                                Bit 6   Start
                                Bit 7   Coin 1
    8000-ffff     ROM


    Sound Board:

    0000-07ff       RAM
    1000-1001       YM2203
                Port A D7 Read  is ticket sensor
                Port B D7 Write is ticket dispenser enable
                Port B D6 Write is Sound OK LED
    2000            Sound watchdog clear
    6000            DAC write
    7000            Sound command read (0x34 is used to dispense a ticket)
    8000-ffff       ROM


    Turbo Board Layout (Plugs in place of GR0):

    Bowl-O-Rama Copyright 1991 P&P Marketing
                Marquee says "EXIT Entertainment"

                This portion: Mike Appolo with the help of Andrew Pines.
                Andrew was one of the game designers for Capcom Bowling,
                Coors Light Bowling, Strata Bowling, and Bowl-O-Rama.

                This game was an upgrade for Capcom Bowling and included a
                "Turbo PCB" that had a GAL address decoder / data mask

    Memory Map for turbo board (where GR0 is on Capcom Bowling PCBs:

    0000        Read Mask
    0001-0003       Unused
    0004        Read Data
    0005-0007       Unused
    0008        GR Address High Byte (GR17-16)
    0009-0016       Unused
    0017            GR Address Middle Byte (GR15-0 written as a word to 0017-0018)
    0018        GR address Low byte
    0019-001f       Unused

***************************************************************************/

#include "emu.h"
#include "machine/ticket.h"
#include "cpu/m6809/m6809.h"
#include "includes/capbowl.h"
#include "sound/2203intf.h"
#include "sound/dac.h"

#define MASTER_CLOCK        XTAL_8MHz


/*************************************
 *
 *  NMI is to trigger the self test.
 *  We use a fake input port to tie
 *  that event to a keypress
 *
 *************************************/

INTERRUPT_GEN_MEMBER(capbowl_state::interrupt)
{
	if (ioport("SERVICE")->read() & 1)                      /* get status of the F2 key */
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);    /* trigger self test */
}



/*************************************
 *
 *  Partial updating
 *
 *************************************/

void capbowl_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_UPDATE:
		update(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in capbowl_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(capbowl_state::update)
{
	int scanline = param;

	m_screen->update_partial(scanline - 1);
	scanline += 32;
	if (scanline > 240) scanline = 32;
	m_update_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


/*************************************
 *
 *  Graphics ROM banking
 *
 *************************************/

WRITE8_MEMBER(capbowl_state::capbowl_rom_select_w)
{
	// 2009-11 FP: shall we add a check to be sure that bank < 6?
	membank("bank1")->set_entry(((data & 0x0c) >> 1) + (data & 0x01));
}



/*************************************
 *
 *  Trackball input handlers
 *
 *************************************/

READ8_MEMBER(capbowl_state::track_0_r)
{
	return (ioport("IN0")->read() & 0xf0) | ((ioport("TRACKY")->read() - m_last_trackball_val[0]) & 0x0f);
}


READ8_MEMBER(capbowl_state::track_1_r)
{
	return (ioport("IN1")->read() & 0xf0) | ((ioport("TRACKX")->read() - m_last_trackball_val[1]) & 0x0f);
}


WRITE8_MEMBER(capbowl_state::track_reset_w)
{
	/* reset the trackball counters */
	m_last_trackball_val[0] = ioport("TRACKY")->read();
	m_last_trackball_val[1] = ioport("TRACKX")->read();

	watchdog_reset_w(space, offset, data);
}



/*************************************
 *
 *  Sound commands
 *
 *************************************/

WRITE8_MEMBER(capbowl_state::sndcmd_w)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	soundlatch_byte_w(space, offset, data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( capbowl_map, AS_PROGRAM, 8, capbowl_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x4000) AM_WRITEONLY AM_SHARE("rowaddress")
	AM_RANGE(0x4800, 0x4800) AM_WRITE(capbowl_rom_select_w)
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x5800, 0x5fff) AM_READWRITE(tms34061_r, tms34061_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(sndcmd_w)
	AM_RANGE(0x6800, 0x6800) AM_WRITE(track_reset_w) AM_READNOP   /* + watchdog */
	AM_RANGE(0x7000, 0x7000) AM_READ(track_0_r)         /* + other inputs */
	AM_RANGE(0x7800, 0x7800) AM_READ(track_1_r)         /* + other inputs */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( bowlrama_map, AS_PROGRAM, 8, capbowl_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(bowlrama_blitter_r, bowlrama_blitter_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITEONLY AM_SHARE("rowaddress")
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x5800, 0x5fff) AM_READWRITE(tms34061_r, tms34061_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(sndcmd_w)
	AM_RANGE(0x6800, 0x6800) AM_WRITE(track_reset_w) AM_READNOP    /* + watchdog */
	AM_RANGE(0x7000, 0x7000) AM_READ(track_0_r)         /* + other inputs */
	AM_RANGE(0x7800, 0x7800) AM_READ(track_1_r)         /* + other inputs */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, capbowl_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x1001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x2000, 0x2000) AM_WRITENOP /* watchdog */
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x7000, 0x7000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( capbowl )
	PORT_START("IN0")
	/* low 4 bits are for the trackball */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) /* This version of Bowl-O-Rama */
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )             /* is Upright only */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	/* low 4 bits are for the trackball */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("TRACKY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(40) PORT_REVERSE

	PORT_START("TRACKX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(40)

	PORT_START("SERVICE")
	/* This fake input port is used to get the status of the F2 key, */
	/* and activate the test mode, which is triggered by a NMI */
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void capbowl_state::machine_start()
{
	m_update_timer = timer_alloc(TIMER_UPDATE);

	save_item(NAME(m_blitter_addr));
	save_item(NAME(m_last_trackball_val));
}

void capbowl_state::machine_reset()
{
	m_update_timer->adjust(m_screen->time_until_pos(32), 32);

	m_blitter_addr = 0;
	m_last_trackball_val[0] = 0;
	m_last_trackball_val[1] = 0;
}


static MACHINE_CONFIG_START( capbowl, capbowl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(capbowl_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", capbowl_state,  interrupt)
	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_ASTABLE(100000.0, 100000.0, 0.1e-6) * 15.5) // ~0.3s

	MCFG_CPU_ADD("audiocpu", M6809E, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_map)
//  MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_ASTABLE(100000.0, 100000.0, 0.1e-6) * 15.5) // TODO

	MCFG_NVRAM_ADD_RANDOM_FILL("nvram")

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(360, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 359, 0, 244)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_UPDATE_DRIVER(capbowl_state, screen_update)
	MCFG_SCREEN_ORIENTATION(ROT270)

	MCFG_DEVICE_ADD("tms34061", TMS34061, 0)
	MCFG_TMS34061_ROWSHIFT(8)  /* VRAM address is (row << rowshift) | col */
	MCFG_TMS34061_VRAM_SIZE(0x10000) /* size of video RAM */
	MCFG_TMS34061_INTERRUPT_CB(INPUTLINE("maincpu", M6809_FIRQ_LINE))      /* interrupt gen callback */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK/2)
	MCFG_YM2203_IRQ_HANDLER(INPUTLINE("audiocpu", M6809_FIRQ_LINE))
	MCFG_AY8910_PORT_A_READ_CB(DEVREAD8("ticket", ticket_dispenser_device, read))
	MCFG_AY8910_PORT_B_WRITE_CB(DEVWRITE8("ticket", ticket_dispenser_device, write))  /* Also a status LED. See memory map above */
	MCFG_SOUND_ROUTE(0, "mono", 0.07)
	MCFG_SOUND_ROUTE(1, "mono", 0.07)
	MCFG_SOUND_ROUTE(2, "mono", 0.07)
	MCFG_SOUND_ROUTE(3, "mono", 0.75)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bowlrama, capbowl )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bowlrama_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 359, 0, 239)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( capbowl )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "u6",  0x08000, 0x8000, CRC(14924c96) SHA1(d436c5115873c9c2bc7657acff1cf7d99c0c5d6d) )
	ROM_LOAD( "gr0", 0x10000, 0x8000, CRC(ef53ca7a) SHA1(219dc342595bfd23c1336f3e167e40ff0c5e7994) )
	ROM_LOAD( "gr1", 0x18000, 0x8000, CRC(27ede6ce) SHA1(14aa31cbcf089419b5b2ea8d57e82fc51895fc2e) )
	ROM_LOAD( "gr2", 0x20000, 0x8000, CRC(e49238f4) SHA1(ac76f1a761d6b0765437fb7367442667da7bb373) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.u30", 0x8000, 0x8000, CRC(8c9c3b8a) SHA1(f3cdf42ef19012817e6b7966845f9ede39f61b07) )
ROM_END


ROM_START( capbowl2 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "program_rev_3_u6.u6", 0x08000, 0x8000, CRC(9162934a) SHA1(7542dd68a2aa55ad4f03b23ae2313ed6a34ae145) )
	ROM_LOAD( "gr0",                 0x10000, 0x8000, CRC(ef53ca7a) SHA1(219dc342595bfd23c1336f3e167e40ff0c5e7994) )
	ROM_LOAD( "gr1",                 0x18000, 0x8000, CRC(27ede6ce) SHA1(14aa31cbcf089419b5b2ea8d57e82fc51895fc2e) )
	ROM_LOAD( "gr2",                 0x20000, 0x8000, CRC(e49238f4) SHA1(ac76f1a761d6b0765437fb7367442667da7bb373) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.u30", 0x8000, 0x8000, CRC(8c9c3b8a) SHA1(f3cdf42ef19012817e6b7966845f9ede39f61b07) )
ROM_END


ROM_START( capbowl3 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "3.0_bowl.u6",   0x08000, 0x8000, CRC(32e30928) SHA1(db47b6ace949d86aa1cdd1e5c7a5981f30b590af) ) /* Capcom label, labeled as "3.0 BOWL" */
	ROM_LOAD( "grom0-gr0.gr0", 0x10000, 0x8000, CRC(2b5eb091) SHA1(43976bfa9fbe9694c7274f113641f671fa32bbb7) ) /* I.T. label */
	ROM_LOAD( "grom1-gr1.gr1", 0x18000, 0x8000, CRC(880e4e1c) SHA1(9f88b26877596667f1ac4e0083795bf266712879) ) /* I.T. label */
	ROM_LOAD( "grom2-gr2.gr2", 0x20000, 0x8000, CRC(f3d2468d) SHA1(0348ee5d0000b753ad90a525048d05bfb552bee1) ) /* I.T. label */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound_r2_u30.u30", 0x8000, 0x8000, CRC(43ac1658) SHA1(1fab23d649d0c565ef1a7f45b30806f9d1bb4afd) ) /* labeled as "SOUND (R-2) U30" */
ROM_END


ROM_START( capbowl4 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "bfb.u6",        0x08000, 0x8000, CRC(79f1d083) SHA1(36e9a90403fc9b876d7660ee46c5fbb855321769) )
	ROM_LOAD( "grom0-gr0.gr0", 0x10000, 0x8000, CRC(2b5eb091) SHA1(43976bfa9fbe9694c7274f113641f671fa32bbb7) ) /* I.T. label */
	ROM_LOAD( "grom1-gr1.gr1", 0x18000, 0x8000, CRC(880e4e1c) SHA1(9f88b26877596667f1ac4e0083795bf266712879) ) /* I.T. label */
	ROM_LOAD( "grom2-gr2.gr2", 0x20000, 0x8000, CRC(f3d2468d) SHA1(0348ee5d0000b753ad90a525048d05bfb552bee1) ) /* I.T. label */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bfb.u30",     0x8000, 0x8000, CRC(6fe2c4ff) SHA1(862823264d243be590fd29a228a32e7a0a818e57) )
ROM_END


ROM_START( clbowl )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "cb8_prg.u6",              0x08000, 0x8000, CRC(91e06bc4) SHA1(efa54328417f971cc482a4529d05331a3baffc1a) ) /* Capcom label */
	ROM_LOAD( "coors_bowling_grom0.gr0", 0x10000, 0x8000, CRC(899c8f15) SHA1(dbb4a9c015b5e64c62140f0c99b87da2793ae5c1) ) /* I.T. label */
	ROM_LOAD( "coors_bowling_grom1.gr1", 0x18000, 0x8000, CRC(0ac0dc4c) SHA1(61afa3af1f84818b940b5c6f6a8cfb58ca557551) ) /* I.T. label */
	ROM_LOAD( "coors_bowling_grom2.gr2", 0x20000, 0x8000, CRC(251f5da5) SHA1(063001cfb68e3ec35baa24eed186214e26d55b82) ) /* I.T. label */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "coors_bowling_sound.u30", 0x8000, 0x8000, CRC(1eba501e) SHA1(684bdc18cf5e01a86d8018a3e228ec34e5dec57d) )
ROM_END


ROM_START( bowlrama )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bowl-o-rama_rev_1.0_u6.u6",   0x08000, 0x08000, CRC(7103ad55) SHA1(92dccc5e6df3e18fc8cdcb67ef14d50ce5eb8b2c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bowl-o-rama_rev_1.0_u30.u30", 0x08000, 0x08000, CRC(f3168834) SHA1(40b7fbe9c15cc4442f4394b71c0666185afe4c8d) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "bowl-o-rama_rev_1.0_ux7.ux7", 0x00000, 0x40000, CRC(8727432a) SHA1(a81d366c5f8df0bdb97e795bba7752e6526ddba0) ) /* located on daughter card add-on */
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(capbowl_state,capbowl)
{
	UINT8 *ROM = memregion("maincpu")->base();

	/* configure ROM banks in 0x0000-0x3fff */
	membank("bank1")->configure_entries(0, 6, &ROM[0x10000], 0x4000);
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1988, capbowl,  0,       capbowl,  capbowl, capbowl_state, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, capbowl2, capbowl, capbowl,  capbowl, capbowl_state, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, capbowl3, capbowl, capbowl,  capbowl, capbowl_state, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, capbowl4, capbowl, capbowl,  capbowl, capbowl_state, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, clbowl,   capbowl, capbowl,  capbowl, capbowl_state, capbowl,  ROT270, "Incredible Technologies / Capcom", "Coors Light Bowling",    MACHINE_SUPPORTS_SAVE )
GAME( 1991, bowlrama, 0,       bowlrama, capbowl, driver_device, 0,        ROT270, "P&P Marketing",                    "Bowl-O-Rama Rev 1.0",    MACHINE_SUPPORTS_SAVE )
