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
    6800            Trackball Reset. Double duties as a watchdog.
    7000          Input port 1    Bit 0-3 Trackball Vertical Position
                                Bit 4   Player 2 Hook Left
                                Bit 5   Player 2 Hook Right
                                Bit 6   Upright/Cocktail DIP Switch
                               Bit 7   Coin 2
    7800          Input port 2    Bit 0-3 Trackball Horizontal Positon
                               Bit 4   Player 1 Hook Left
                               Bit 5   Player 1 Hook Right
                               Bit 6   Start
                               Bit 7   Coin 1
    8000-ffff       ROM


    Sound Board:

    0000-07ff       RAM
    1000-1001       YM2203
                Port A D7 Read  is ticket sensor
                Port B D7 Write is ticket dispenser enable
                Port B D6 looks like a diagnostics LED to indicate that
                          the PCB is operating. It's pulsated by the
                          sound CPU. It is kind of pointless to emulate it.
    2000            Not hooked up according to the schematics
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

#define MASTER_CLOCK		8000000		/* 8MHz crystal */


/*************************************
 *
 *  NMI is to trigger the self test.
 *  We use a fake input port to tie
 *  that event to a keypress
 *
 *************************************/

static INTERRUPT_GEN( capbowl_interrupt )
{
	if (input_port_read(device->machine(), "SERVICE") & 1)						/* get status of the F2 key */
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);	/* trigger self test */
}



/*************************************
 *
 *  Partial updating
 *
 *************************************/

static TIMER_CALLBACK( capbowl_update )
{
	int scanline = param;

	machine.primary_screen->update_partial(scanline - 1);
	scanline += 32;
	if (scanline > 240) scanline = 32;
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(scanline), FUNC(capbowl_update), scanline);
}


/*************************************
 *
 *  Graphics ROM banking
 *
 *************************************/

WRITE8_MEMBER(capbowl_state::capbowl_rom_select_w)
{
	// 2009-11 FP: shall we add a check to be sure that bank < 6?
	memory_set_bank(machine(), "bank1", ((data & 0x0c) >> 1) + (data & 0x01));
}



/*************************************
 *
 *  Trackball input handlers
 *
 *************************************/

READ8_MEMBER(capbowl_state::track_0_r)
{
	return (input_port_read(machine(), "IN0") & 0xf0) | ((input_port_read(machine(), "TRACKY") - m_last_trackball_val[0]) & 0x0f);
}


READ8_MEMBER(capbowl_state::track_1_r)
{
	return (input_port_read(machine(), "IN1") & 0xf0) | ((input_port_read(machine(), "TRACKX") - m_last_trackball_val[1]) & 0x0f);
}


WRITE8_MEMBER(capbowl_state::track_reset_w)
{

	/* reset the trackball counters */
	m_last_trackball_val[0] = input_port_read(machine(), "TRACKY");
	m_last_trackball_val[1] = input_port_read(machine(), "TRACKX");

	watchdog_reset_w(space, offset, data);
}



/*************************************
 *
 *  Sound commands
 *
 *************************************/

WRITE8_MEMBER(capbowl_state::capbowl_sndcmd_w)
{
	device_set_input_line(m_audiocpu, M6809_IRQ_LINE, HOLD_LINE);
	soundlatch_w(space, offset, data);
}



/*************************************
 *
 *  Handler called by the 2203 emulator
 *  when the internal timers cause an IRQ
 *
 *************************************/

static void firqhandler( device_t *device, int irq )
{
	capbowl_state *state = device->machine().driver_data<capbowl_state>();
	device_set_input_line(state->m_audiocpu, 1, irq ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  NVRAM
 *
 *************************************/

void capbowl_state::init_nvram(nvram_device &nvram, void *base, size_t size)
{
	/* invalidate nvram to make the game initialize it.
      A 0xff fill will cause the game to malfunction, so we use a
      0x01 fill which seems OK */
	memset(base, 0x01, size);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( capbowl_map, AS_PROGRAM, 8, capbowl_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x4000) AM_WRITEONLY AM_BASE(m_rowaddress)
	AM_RANGE(0x4800, 0x4800) AM_WRITE(capbowl_rom_select_w)
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x5800, 0x5fff) AM_READWRITE(capbowl_tms34061_r, capbowl_tms34061_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(capbowl_sndcmd_w)
	AM_RANGE(0x6800, 0x6800) AM_WRITE(track_reset_w)	/* + watchdog */
	AM_RANGE(0x7000, 0x7000) AM_READ(track_0_r)			/* + other inputs */
	AM_RANGE(0x7800, 0x7800) AM_READ(track_1_r)			/* + other inputs */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( bowlrama_map, AS_PROGRAM, 8, capbowl_state )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(bowlrama_blitter_r, bowlrama_blitter_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITEONLY AM_BASE(m_rowaddress)
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x5800, 0x5fff) AM_READWRITE(capbowl_tms34061_r, capbowl_tms34061_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(capbowl_sndcmd_w)
	AM_RANGE(0x6800, 0x6800) AM_WRITE(track_reset_w)	/* + watchdog */
	AM_RANGE(0x7000, 0x7000) AM_READ(track_0_r)			/* + other inputs */
	AM_RANGE(0x7800, 0x7800) AM_READ(track_1_r)			/* + other inputs */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, capbowl_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x1001) AM_DEVREADWRITE_LEGACY("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x2000, 0x2000) AM_WRITENOP				/* Not hooked up according to the schematics */
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE_LEGACY("dac", dac_w)
	AM_RANGE(0x7000, 0x7000) AM_READ(soundlatch_r)
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
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )			   /* is Upright only */
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
 *  Sound definitions
 *
 *************************************/

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_DEVICE_HANDLER("ticket", ticket_dispenser_r),
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_DEVICE_HANDLER("ticket", ticket_dispenser_w),  /* Also a status LED. See memory map above */
	},
	firqhandler
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( capbowl )
{
	capbowl_state *state = machine.driver_data<capbowl_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");

	state->save_item(NAME(state->m_blitter_addr));
	state->save_item(NAME(state->m_last_trackball_val[0]));
	state->save_item(NAME(state->m_last_trackball_val[1]));
}

static MACHINE_RESET( capbowl )
{
	capbowl_state *state = machine.driver_data<capbowl_state>();

	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(32), FUNC(capbowl_update), 32);

	state->m_blitter_addr = 0;
	state->m_last_trackball_val[0] = 0;
	state->m_last_trackball_val[1] = 0;
}


static MACHINE_CONFIG_START( capbowl, capbowl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(capbowl_map)
	MCFG_CPU_VBLANK_INT("screen", capbowl_interrupt)

	MCFG_CPU_ADD("audiocpu", M6809E, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START(capbowl)
	MCFG_MACHINE_RESET(capbowl)
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", capbowl_state, init_nvram)

	MCFG_TICKET_DISPENSER_ADD("ticket", 100, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_VIDEO_START(capbowl)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(360, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 359, 0, 244)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_UPDATE_STATIC(capbowl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK/2)
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.07)
	MCFG_SOUND_ROUTE(1, "mono", 0.07)
	MCFG_SOUND_ROUTE(2, "mono", 0.07)
	MCFG_SOUND_ROUTE(3, "mono", 0.75)

	MCFG_SOUND_ADD("dac", DAC, 0)
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
	ROM_LOAD( "u6",           0x08000, 0x8000, CRC(14924c96) SHA1(d436c5115873c9c2bc7657acff1cf7d99c0c5d6d) )
	ROM_LOAD( "gr0",          0x10000, 0x8000, CRC(ef53ca7a) SHA1(219dc342595bfd23c1336f3e167e40ff0c5e7994) )
	ROM_LOAD( "gr1",          0x18000, 0x8000, CRC(27ede6ce) SHA1(14aa31cbcf089419b5b2ea8d57e82fc51895fc2e) )
	ROM_LOAD( "gr2",          0x20000, 0x8000, CRC(e49238f4) SHA1(ac76f1a761d6b0765437fb7367442667da7bb373) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound",        0x8000, 0x8000, CRC(8c9c3b8a) SHA1(f3cdf42ef19012817e6b7966845f9ede39f61b07) )
ROM_END


ROM_START( capbowl2 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "progrev3.u6",  0x08000, 0x8000, CRC(9162934a) SHA1(7542dd68a2aa55ad4f03b23ae2313ed6a34ae145) )
	ROM_LOAD( "gr0",          0x10000, 0x8000, CRC(ef53ca7a) SHA1(219dc342595bfd23c1336f3e167e40ff0c5e7994) )
	ROM_LOAD( "gr1",          0x18000, 0x8000, CRC(27ede6ce) SHA1(14aa31cbcf089419b5b2ea8d57e82fc51895fc2e) )
	ROM_LOAD( "gr2",          0x20000, 0x8000, CRC(e49238f4) SHA1(ac76f1a761d6b0765437fb7367442667da7bb373) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound",        0x8000, 0x8000, CRC(8c9c3b8a) SHA1(f3cdf42ef19012817e6b7966845f9ede39f61b07) )
ROM_END


ROM_START( capbowl3 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "bowl30.bin",   0x08000, 0x8000, CRC(32e30928) SHA1(db47b6ace949d86aa1cdd1e5c7a5981f30b590af) )
	ROM_LOAD( "bfb.gr0",      0x10000, 0x8000, CRC(2b5eb091) SHA1(43976bfa9fbe9694c7274f113641f671fa32bbb7) )
	ROM_LOAD( "bfb.gr1",      0x18000, 0x8000, CRC(880e4e1c) SHA1(9f88b26877596667f1ac4e0083795bf266712879) )
	ROM_LOAD( "bfb.gr2",      0x20000, 0x8000, CRC(f3d2468d) SHA1(0348ee5d0000b753ad90a525048d05bfb552bee1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound-r2.bin",  0x8000, 0x8000, CRC(43ac1658) SHA1(1fab23d649d0c565ef1a7f45b30806f9d1bb4afd) )
ROM_END


ROM_START( capbowl4 )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "bfb.u6",        0x08000, 0x8000, CRC(79f1d083) SHA1(36e9a90403fc9b876d7660ee46c5fbb855321769) )
	ROM_LOAD( "bfb.gr0",       0x10000, 0x8000, CRC(2b5eb091) SHA1(43976bfa9fbe9694c7274f113641f671fa32bbb7) )
	ROM_LOAD( "bfb.gr1",       0x18000, 0x8000, CRC(880e4e1c) SHA1(9f88b26877596667f1ac4e0083795bf266712879) )
	ROM_LOAD( "bfb.gr2",       0x20000, 0x8000, CRC(f3d2468d) SHA1(0348ee5d0000b753ad90a525048d05bfb552bee1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bfb.u30",     0x8000, 0x8000, CRC(6fe2c4ff) SHA1(862823264d243be590fd29a228a32e7a0a818e57) )
ROM_END


ROM_START( clbowl )
	ROM_REGION( 0x28000, "maincpu", 0 )
	ROM_LOAD( "u6.cl",        0x08000, 0x8000, CRC(91e06bc4) SHA1(efa54328417f971cc482a4529d05331a3baffc1a) )
	ROM_LOAD( "gr0.cl",       0x10000, 0x8000, CRC(899c8f15) SHA1(dbb4a9c015b5e64c62140f0c99b87da2793ae5c1) )
	ROM_LOAD( "gr1.cl",       0x18000, 0x8000, CRC(0ac0dc4c) SHA1(61afa3af1f84818b940b5c6f6a8cfb58ca557551) )
	ROM_LOAD( "gr2.cl",       0x20000, 0x8000, CRC(251f5da5) SHA1(063001cfb68e3ec35baa24eed186214e26d55b82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.cl",     0x8000, 0x8000, CRC(1eba501e) SHA1(684bdc18cf5e01a86d8018a3e228ec34e5dec57d) )
ROM_END


ROM_START( bowlrama )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6",           0x08000, 0x08000, CRC(7103ad55) SHA1(92dccc5e6df3e18fc8cdcb67ef14d50ce5eb8b2c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u30",          0x8000, 0x8000, CRC(f3168834) SHA1(40b7fbe9c15cc4442f4394b71c0666185afe4c8d) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "ux7",          0x00000, 0x40000, CRC(8727432a) SHA1(a81d366c5f8df0bdb97e795bba7752e6526ddba0) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static DRIVER_INIT( capbowl )
{
	UINT8 *ROM = machine.region("maincpu")->base();

	/* configure ROM banks in 0x0000-0x3fff */
	memory_configure_bank(machine, "bank1", 0, 6, &ROM[0x10000], 0x4000);
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1988, capbowl,  0,       capbowl,  capbowl, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 1)", 0 )
GAME( 1988, capbowl2, capbowl, capbowl,  capbowl, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 2)", 0 )
GAME( 1988, capbowl3, capbowl, capbowl,  capbowl, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 3)", 0 )
GAME( 1988, capbowl4, capbowl, capbowl,  capbowl, capbowl,  ROT270, "Incredible Technologies / Capcom", "Capcom Bowling (set 4)", 0 )
GAME( 1989, clbowl,   capbowl, capbowl,  capbowl, capbowl,  ROT270, "Incredible Technologies / Capcom", "Coors Light Bowling", 0 )
GAME( 1991, bowlrama, 0,       bowlrama, capbowl, 0,        ROT270, "P&P Marketing", "Bowl-O-Rama", 0 )
