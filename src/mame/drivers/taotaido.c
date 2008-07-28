/***************************************************************************

  Tao Taido             (c) 1993 Video System


    driver by David Haywood - Dip Switches and Inputs by Stephane Humbert
    based on other Video System drivers

Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - Don't trust the test mode ! It shows inputs for 4 players as well as
    3 buttons for each player, while the game is a 2 players game with only
    2 buttons (punch and kick) for each player.
    IMO, it's a leftover from a previous game.
    If you want to test all inputs, turn TAOTAIDO_SHOW_ALL_INPUTS to 1.

  - The "Buy-In" features allows a player to recover some energy and reset timer
    by pressing his "Start" button (provided he has at least one credit).

  - 'taotaido' allows to play a 2 players game with a single credit while
    this isn't possible with 'taotaida'.
    Now, telling which version is the newest one is another story ;)

  - 'taotaido' seems to show you how to do the special moves, 'taotaida' doesn't
    and they don't seem to work in the same way (unless this is a bug)

  - Coin buttons act differently depending on the "Coin Slot" Dip Switch :

      * "Coin Slot" Dip Switch set to "Same" :

          . COIN1 : adds coin(s)/credit(s) depending on "Coinage" Dip Switch
          . COIN2 : adds 1 credit
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coinage" Dip Switch

      * "Coin Slot" Dip Switch set to "Individual" :

          . COIN1 : adds coin(s)/credit(s) for player 1 depending on "Coinage" Dip Switch
          . COIN2 : adds coin(s)/credit(s) for player 2 depending on "Coinage" Dip Switch
          . SERVICE1 : adds 1 credit for player 1

***************************************************************************/

/* Tao Taido
(c)1993 Video System

CPU:    68000-16
Sound:  Z80-B
        YM2610
OSC:    14.31818MHz
        20.0000MHz
        32.0000MHz
Chips:  VS9108
        VS920B
        VS9209 x2

****************************************************************************

zooming might be wrong

***************************************************************************/



#include "driver.h"
#include "sound/2610intf.h"

#define TAOTAIDO_SHOW_ALL_INPUTS	0

UINT16 *taotaido_spriteram;
UINT16 *taotaido_spriteram2;
UINT16 *taotaido_scrollram;
UINT16 *taotaido_bgram;

WRITE16_HANDLER( taotaido_sprite_character_bank_select_w );
WRITE16_HANDLER( taotaido_tileregs_w );
WRITE16_HANDLER( taotaido_bgvideoram_w );
VIDEO_START( taotaido );
VIDEO_UPDATE( taotaido );
VIDEO_EOF( taotaido );

static int pending_command;

static READ16_HANDLER( pending_command_r )
{
	/* Only bit 0 is tested */
	return pending_command;
}

static WRITE16_HANDLER( sound_command_w )
{
	if (ACCESSING_BITS_0_7)
	{
		pending_command = 1;
		soundlatch_w(machine,offset,data & 0xff);
		cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
	}
}
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(taotaido_bgvideoram_w) AM_BASE(&taotaido_bgram)	// bg ram?
	AM_RANGE(0xa00000, 0xa01fff) AM_RAM AM_BASE(&taotaido_spriteram)		// sprite ram
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_BASE(&taotaido_spriteram2)		// sprite tile lookup ram
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM						// main ram
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)	// palette ram
	AM_RANGE(0xffe000, 0xffe3ff) AM_RAM AM_BASE(&taotaido_scrollram)		// rowscroll / rowselect / scroll ram
	AM_RANGE(0xffff80, 0xffff81) AM_READ(input_port_0_word_r)	// player 1 inputs
	AM_RANGE(0xffff82, 0xffff83) AM_READ(input_port_1_word_r)	// player 2 inputs
	AM_RANGE(0xffff84, 0xffff85) AM_READ(input_port_2_word_r)	// system inputs
	AM_RANGE(0xffff86, 0xffff87) AM_READ(input_port_3_word_r)	// DSWA
	AM_RANGE(0xffff88, 0xffff89) AM_READ(input_port_4_word_r)	// DSWB
	AM_RANGE(0xffff8a, 0xffff8b) AM_READ(input_port_5_word_r)	// DSWC
	AM_RANGE(0xffff8c, 0xffff8d) AM_READ(SMH_RAM)			// unknown
	AM_RANGE(0xffff8e, 0xffff8f) AM_READ(input_port_6_word_r)	// jumpers
#if TAOTAIDO_SHOW_ALL_INPUTS
	AM_RANGE(0xffffa0, 0xffffa1) AM_READ(input_port_7_word_r)	// player 3 inputs (unused)
	AM_RANGE(0xffffa2, 0xffffa3) AM_READ(input_port_8_word_r)	// player 4 inputs (unused)
#endif
	AM_RANGE(0xffff00, 0xffff0f) AM_WRITE(taotaido_tileregs_w)
	AM_RANGE(0xffff10, 0xffff11) AM_WRITE(SMH_NOP)						// unknown
	AM_RANGE(0xffff20, 0xffff21) AM_WRITE(SMH_NOP)						// unknown - flip screen related
	AM_RANGE(0xffff40, 0xffff47) AM_WRITE(taotaido_sprite_character_bank_select_w)
	AM_RANGE(0xffffc0, 0xffffc1) AM_WRITE(sound_command_w)					// seems right
	AM_RANGE(0xffffe0, 0xffffe1) AM_READ(pending_command_r)	// guess - seems to be needed for all the sounds to work
ADDRESS_MAP_END

/* sound cpu - same as aerofgt */


static WRITE8_HANDLER( pending_command_clear_w )
{
	pending_command = 0;
}

static WRITE8_HANDLER( taotaido_sh_bankswitch_w )
{
	UINT8 *rom = memory_region(machine, RGNCLASS_CPU, "audio") + 0x10000;

	memory_set_bankptr(1,rom + (data & 0x03) * 0x8000);
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_READWRITE(SMH_BANK1, SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(YM2610_status_port_0_A_r, YM2610_control_port_0_A_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2610_data_port_0_A_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(YM2610_status_port_0_B_r, YM2610_control_port_0_B_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(YM2610_data_port_0_B_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(taotaido_sh_bankswitch_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( taotaido )
	PORT_START_TAG("IN0")	/* Player 1 controls (0xffff81.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// "Punch"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// "Kick"
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
#else
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")	/* Player 2 controls (0xffff83.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// "Punch"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// "Kick"
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
#else
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")	/* System inputs (0xffff85.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )	// see notes
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
#else
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	// "Test" in "test mode"
#endif
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )		// not working ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )	// see notes - SERVICE in "test mode"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	// VBLANK ? The game freezes when ON

	PORT_START_TAG("DSW0")	/* DSW A (0xffff87.b -> !0xfe2f6c.w or !0xfe30d0) */
	PORT_DIPNAME( 0x01, 0x01, "Coin Slot" )
	PORT_DIPSETTING(    0x01, "Same" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW1")	/* DSW B (0xffff89.b -> !0xfe73c2.w or !0xfe751c.w) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )	// check code at 0x0963e2 or 0x845e2
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("DSW2")	/* DSW C (0xffff8b.b -> !0xfe2f94.w or !0xfe30f8.w) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	// doesn't seem to be demo sounds
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Buy In" )			// see notes
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("JP0")	/* Jumpers (0xffff8f.b) */
	PORT_DIPNAME( 0x0f, 0x08, "Country" )
	PORT_DIPSETTING(    0x00, "US" )				// also (c) Mc O'River Inc
	PORT_DIPSETTING(    0x01, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x02, "Hong-Kong/Taiwan" )
//  PORT_DIPSETTING(    0x03, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x04, "Korea" )
//  PORT_DIPSETTING(    0x05, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x08, DEF_STR( World ) )
	/* 0x09 to 0x0f : DEF_STR( Japan ) */

#if TAOTAIDO_SHOW_ALL_INPUTS
	/* These inputs are only to fit the test mode - leftover from another game ? */
	PORT_START_TAG("IN3")	/* Player 3 inputs (0xffffa1.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN4")	/* Player 4 inputs (0xffffa3.b) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
INPUT_PORTS_END

static INPUT_PORTS_START( taotaida )
	PORT_INCLUDE(taotaido)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END


static const gfx_layout taotaido_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( taotaido )
	GFXDECODE_ENTRY( "gfx1", 0, taotaido_layout,  0x000, 256  ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, taotaido_layout,  0x300, 256  ) /* bg tiles */
GFXDECODE_END

static void irqhandler(running_machine *machine, int irq)
{
	cpunum_set_input_line(machine, 1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YM2610interface ym2610_interface =
{
	irqhandler
};

static MACHINE_DRIVER_START( taotaido )
	MDRV_CPU_ADD("main", M68000, 32000000/2)
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	/* audio CPU */
	MDRV_CPU_ADD("audio", Z80,20000000/4) // ??
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_port_map,0)
								/* IRQs are triggered by the YM2610 */

	MDRV_GFXDECODE(taotaido)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)

	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(taotaido)
	MDRV_VIDEO_UPDATE(taotaido)
	MDRV_VIDEO_EOF( taotaido )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("ym", YM2610, 8000000)
	MDRV_SOUND_CONFIG(ym2610_interface)
	MDRV_SOUND_ROUTE(0, "left",  0.25)
	MDRV_SOUND_ROUTE(0, "right", 0.25)
	MDRV_SOUND_ROUTE(1, "left",  1.0)
	MDRV_SOUND_ROUTE(2, "right", 1.0)
MACHINE_DRIVER_END


ROM_START( taotaido )
	ROM_REGION( 0x100000, RGNCLASS_CPU, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "1-u90.bin", 0x00000, 0x80000, CRC(a3ee30da) SHA1(920a83ce9192bf785bffdc041e280f1a420de4c9) )
	ROM_LOAD16_WORD_SWAP( "2-u91.bin", 0x80000, 0x80000, CRC(30b7e4fb) SHA1(15e1f6d252c736fdee33b691a0a1a45f0307bffb) )

	ROM_REGION( 0x30000, RGNCLASS_CPU, "audio", 0 ) /* z80 Code */
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )
	ROM_RELOAD ( 0x10000, 0x20000 )

	ROM_REGION( 0x100000, RGNCLASS_SOUND, "ym.deltat", 0 ) /* sound samples */
	ROM_LOAD( "u104.bin",     0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x200000, RGNCLASS_SOUND, "ym", 0 ) /* sound samples */
	ROM_LOAD( "u127.bin",     0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x600000, RGNCLASS_GFX, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, RGNCLASS_GFX, "gfx2", 0 ) /* BG Tiles */
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

ROM_START( taotaida )
	ROM_REGION( 0x100000, RGNCLASS_CPU, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "tt0-u90.bin", 0x00000, 0x80000, CRC(69d4cca7) SHA1(f1aba74fef8fe4271d19763f428fc0e2674d08b3) )
	ROM_LOAD16_WORD_SWAP( "tt1-u91.bin", 0x80000, 0x80000, CRC(41025469) SHA1(fa3a424ca3ecb513f418e436e4191ff76f6a0de1) )

	ROM_REGION( 0x30000, RGNCLASS_CPU, "audio", 0 ) /* z80 Code */
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )
	ROM_RELOAD ( 0x10000, 0x20000 )

	ROM_REGION( 0x100000, RGNCLASS_SOUND, "ym.deltat", 0 ) /* sound samples */
	ROM_LOAD( "u104.bin",     0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x200000, RGNCLASS_SOUND, "ym", 0 ) /* sound samples */
	ROM_LOAD( "u127.bin",     0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x600000, RGNCLASS_GFX, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, RGNCLASS_GFX, "gfx2", 0 ) /* BG Tiles */
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

GAME( 1993, taotaido, 0,        taotaido, taotaido, 0, ROT0, "Video System Co.", "Tao Taido (set 1)", GAME_NO_COCKTAIL )
GAME( 1993, taotaida, taotaido, taotaido, taotaida, 0, ROT0, "Video System Co.", "Tao Taido (set 2)", GAME_NO_COCKTAIL )
