/*******************************************************************************
 WWF Wrestlefest (C) 1991 Technos Japan  (drivers/wwfwfest.c)
********************************************************************************
 driver by David Haywood

 Special Thanks to:

 Richard Bush & the Rest of the Raine Team - Raine's WWF Wrestlefest driver on
 which some of this driver has been based.

********************************************************************************

 Hardware:

 Primary CPU : 68000 - 12MHz

 Sound CPUs : Z80 - 3.579MHz

 Sound Chips : YM2151, M6295

 4 Layers from now on if mentioned will be referred to as

 BG0 - Background Layer 0
 BG1 - Background Layer 1
 SPR - Sprites
 FG0 - Foreground / Text Layer

 Priorities of BG0, BG1 and SPR can be changed

********************************************************************************

 Change Log:
 20 Jun 2001 | Did Pretty Much everything else, the game is now playable.
 19 Jun 2001 | Started the driver, based on Raine, the WWF Superstars driver,
             | and the Double Dragon 3 Driver, got most of the basics done,
             | the game will boot showing some graphics.

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "includes/wwfwfest.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"

#define MASTER_CLOCK		XTAL_24MHz
#define CPU_CLOCK			MASTER_CLOCK / 2
#define PIXEL_CLOCK		MASTER_CLOCK / 4

/*- in this file -*/


static WRITE8_DEVICE_HANDLER( oki_bankswitch_w );




/*******************************************************************************
 Memory Maps
********************************************************************************
 Pretty Straightforward

 still some unknown writes however, sound cpu memory map is the same as dd3
*******************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, wwfwfest_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0c1fff) AM_RAM_WRITE(wwfwfest_fg0_videoram_w) AM_BASE(m_fg0_videoram)	/* FG0 Ram - 4 bytes per tile */
	AM_RANGE(0x0c2000, 0x0c3fff) AM_RAM AM_SHARE("spriteram")						/* SPR Ram */
	AM_RANGE(0x080000, 0x080fff) AM_RAM_WRITE(wwfwfest_bg0_videoram_w) AM_BASE(m_bg0_videoram)	/* BG0 Ram - 4 bytes per tile */
	AM_RANGE(0x082000, 0x082fff) AM_RAM_WRITE(wwfwfest_bg1_videoram_w) AM_BASE(m_bg1_videoram)	/* BG1 Ram - 2 bytes per tile */
	AM_RANGE(0x100000, 0x100007) AM_WRITE(wwfwfest_scroll_write)
	AM_RANGE(0x10000a, 0x10000b) AM_WRITE(wwfwfest_flipscreen_w)
	AM_RANGE(0x140000, 0x140003) AM_WRITE(wwfwfest_irq_ack_w)
	AM_RANGE(0x14000c, 0x14000d) AM_WRITE(wwfwfest_soundwrite)
	AM_RANGE(0x140010, 0x140011) AM_WRITE(wwfwfest_1410_write)
	AM_RANGE(0x140020, 0x140021) AM_READ_PORT("P1")
	AM_RANGE(0x140022, 0x140023) AM_READ_PORT("P2")
	AM_RANGE(0x140024, 0x140025) AM_READ_PORT("P3")
	AM_RANGE(0x140026, 0x140027) AM_READ_PORT("P4")
	AM_RANGE(0x180000, 0x18ffff) AM_READWRITE(wwfwfest_paletteram16_xxxxBBBBGGGGRRRR_word_r,wwfwfest_paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x1c0000, 0x1c3fff) AM_RAM /* Work Ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, wwfwfest_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xd800, 0xd800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE_LEGACY("oki", oki_bankswitch_w)
ADDRESS_MAP_END

/*******************************************************************************
 Read / Write Handlers
********************************************************************************
 as used by the above memory map
*******************************************************************************/

WRITE16_MEMBER(wwfwfest_state::wwfwfest_irq_ack_w)
{
	if (offset == 0)
		cputag_set_input_line(machine(), "maincpu", 3, CLEAR_LINE);

	else
		cputag_set_input_line(machine(), "maincpu", 2, CLEAR_LINE);
}

WRITE16_MEMBER(wwfwfest_state::wwfwfest_flipscreen_w)
{
	flip_screen_set(machine(), data&1);
}

/*- Palette Reads/Writes -*/

READ16_MEMBER(wwfwfest_state::wwfwfest_paletteram16_xxxxBBBBGGGGRRRR_word_r)
{
	offset = (offset & 0x000f) | (offset & 0x7fc0) >> 2;
	return m_generic_paletteram_16[offset];
}

WRITE16_MEMBER(wwfwfest_state::wwfwfest_paletteram16_xxxxBBBBGGGGRRRR_word_w)
{
	offset = (offset & 0x000f) | (offset & 0x7fc0) >> 2;
	paletteram16_xxxxBBBBGGGGRRRR_word_w (space, offset, data, mem_mask);
}

/*- Priority Control -*/


WRITE16_MEMBER(wwfwfest_state::wwfwfest_1410_write)
{
	m_pri = data;
}

/*- Scroll Control -*/

WRITE16_MEMBER(wwfwfest_state::wwfwfest_scroll_write)
{
	switch (offset) {
		case 0x00:
			m_bg0_scrollx = data;
			break;
		case 0x01:
			m_bg0_scrolly = data;
			break;
		case 0x02:
			m_bg1_scrollx = data;
			break;
		case 0x03:
			m_bg1_scrolly = data;
			break;
	}
}

/*- Sound Related (from dd3) -*/

static WRITE8_DEVICE_HANDLER( oki_bankswitch_w )
{
	downcast<okim6295_device *>(device)->set_bank_base((data & 1) * 0x40000);
}

WRITE16_MEMBER(wwfwfest_state::wwfwfest_soundwrite)
{
	soundlatch_w(space,1,data & 0xff);
	cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE );
}

/*******************************************************************************
 Input Ports
********************************************************************************
 There are 4 players, 2 sets of dipswitches and 2 misc
*******************************************************************************/

/* DIPs are spread across the other input ports */
static CUSTOM_INPUT( dsw_3f_r )
{
	const char *tag = (const char *)param;
	return input_port_read(field.machine(), tag) & 0x3f;
}

static CUSTOM_INPUT( dsw_c0_r )
{
	const char *tag = (const char *)param;
	return (input_port_read(field.machine(), tag) & 0xc0) >> 6;
}


static INPUT_PORTS_START( wwfwfest )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x3000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dsw_c0_r, "DSW2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3f00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dsw_3f_r, "DSW2")

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x3f00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dsw_3f_r, "DSW1")

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0300, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dsw_c0_r, "DSW1")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Nb:  There are actually 3 dips on the board, 2 * 8, and 1 *4 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C )  )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C )  )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C )  )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C )  )
	PORT_DIPNAME( 0x04, 0x04, "Buy In Price"  ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "1 Coin" )
	PORT_DIPSETTING(    0x00, "As start price" )
	PORT_DIPNAME( 0x08, 0x08, "Regain Power Price"  ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1 Coin" )
	PORT_DIPSETTING(    0x00, "As start price" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Continue_Price )  ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Coin" )
	PORT_DIPSETTING(    0x00, "As start price" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen )  ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "FBI Logo" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Players ) ) PORT_DIPLOCATION("SW2:3,4") /* Nothing listed for seting ON/ON IE:0x00 */
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x60, "Clear Stage Power Up" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x60, "24" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPNAME( 0x80, 0x80, "Championship Game" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "4th" )
	PORT_DIPSETTING(    0x80, "5th" )
INPUT_PORTS_END

/*******************************************************************************
 Graphic Decoding
*******************************************************************************/
static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 8*8+1, 8*8+0, 16*8+1, 16*8+0, 24*8+1, 24*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout tile_layout =
{
	16,16,	/* 16*16 tiles */
	4096,	/* 8192 tiles */
	4,	/* 4 bits per pixel */
	{ 8, 0, 0x40000*8+8 , 0x40000*8+0 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*8, 16*9, 16*10, 16*11, 16*12, 16*13, 16*14, 16*15 },
	64*8	/* every tile takes 64 consecutive bytes */
};

static const gfx_layout sprite_layout = {
	16,16,	/* 16*16 tiles */
	RGN_FRAC(1,4),
	4,	/* 4 bits per pixel */
	{ 0, 0x200000*8, 2*0x200000*8 , 3*0x200000*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every tile takes 32 consecutive bytes */
};

static GFXDECODE_START( wwfwfest )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x0000, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout,   0x0400, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout,     0x1000, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout,     0x0c00, 16 )
GFXDECODE_END

/*******************************************************************************
 Interrupt Function
*******************************************************************************/

static TIMER_DEVICE_CALLBACK( wwfwfest_scanline )
{
	int scanline = param;

	/* An interrupt is generated every 16 scanlines */
	if (scanline % 16 == 0)
	{
		if (scanline > 0)
			timer.machine().primary_screen->update_partial(scanline - 1);
		cputag_set_input_line(timer.machine(), "maincpu", 2, ASSERT_LINE);
	}

	/* Vblank is raised on scanline 248 */
	if (scanline == 248)
	{
		timer.machine().primary_screen->update_partial(scanline - 1);
		cputag_set_input_line(timer.machine(), "maincpu", 3, ASSERT_LINE);
	}
}

/*******************************************************************************
 Sound Stuff..
********************************************************************************
 Straight from Ddragon 3 with some adjusted volumes
*******************************************************************************/

static void dd3_ymirq_handler(device_t *device, int irq)
{
	cputag_set_input_line(device->machine(), "audiocpu", 0 , irq ? ASSERT_LINE : CLEAR_LINE );
}

static const ym2151_interface ym2151_config =
{
	dd3_ymirq_handler
};

/*******************************************************************************
 Machine Driver(s)
*******************************************************************************/

static MACHINE_CONFIG_START( wwfwfest, wwfwfest_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, CPU_CLOCK)	/* 24 crystal, 12 rated chip */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", wwfwfest_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 320, 272, 8, 248)	/* HTOTAL and VTOTAL are guessed */
	MCFG_SCREEN_UPDATE_STATIC(wwfwfest)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE(wwfwfest)
	MCFG_PALETTE_LENGTH(8192)

	MCFG_VIDEO_START(wwfwfest)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, XTAL_3_579545MHz)
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.45)
	MCFG_SOUND_ROUTE(1, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki", 1024188, OKIM6295_PIN7_HIGH) /* Verified - Pin 7 tied to +5VDC */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( wwfwfstb, wwfwfest )
	MCFG_VIDEO_START(wwfwfstb)
MACHINE_CONFIG_END

/*******************************************************************************
 Rom Loaders / Game Drivers
********************************************************************************
 2 sets supported,
 wwfwfest - US? Set (Tecmo License / Distribution?)
 wwfwfstj - Japan? Set

 readme / info files below

--------------------------------------------------------------------------------
 wwfwfstj: README.TXT
--------------------------------------------------------------------------------
 Wrestlefest
 Technos 1991

 TA-0031
                                                         68000-12
  31J0_IC1  6264 6264 31A14-2 31A13-2 6264 6264 31A12-0  24MHz
  31J1_IC2
                   TJ-002          TJ-004
                                   6264                   SW1
                   28MHz                                  SW2
                                                          SW3
                                             61C16-35
                        61C16-35             61C16-35
  31J2_IC8
  31J3_IC9
  31J4_IC10
  31J5_IC11
  31J6_IC12
  31J7_IC13
  31J8_IC14    TJ-003                31A11-2  M6295   31J10_IC73
  31J9_IC15    61C16-35 61C16-35     Z80      YM2151


  Clock Crystals:

  X1 - 28.000 MHz
  X2 - 3.579545 MHz (for Z80)
  X3 - 24.000 MHz (for 68000)
  X4 - 1.056 MHz (not used, initially intended for OKI6295?)


  The mask roms at IC1 and IC2 have the same pinouts as a MX27C4100 or M27C400
  except pin 1 is not A17 but instead not used (not connected).

*******************************************************************************/

ROM_START( wwfwfest )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "31a13-2.ic19", 0x00001, 0x40000, CRC(7175bca7) SHA1(992b47a787b5bc2a5a381ec78b8dfaf7d42c614b) )
	ROM_LOAD16_BYTE( "31a14-2.ic18", 0x00000, 0x40000, CRC(5d06bfd1) SHA1(39a93da662158aa5a9953dcabfcb47c2fc196dc7) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "31a12-0.ic33", 0x00000, 0x20000, CRC(d0803e20) SHA1(b68758e9a5522396f831a3972571f8aed54c64de) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END

ROM_START( wwfwfesta )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "wf_18.rom", 0x00000, 0x40000, CRC(933ea1a0) SHA1(61da142cfa7abd3b77ab21979c061a078c0d0c63) )
	ROM_LOAD16_BYTE( "wf_19.rom", 0x00001, 0x40000, CRC(bd02e3c4) SHA1(7ae63e48caf9919ce7b63b4c5aa9474ba8c336da) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "wf_33.rom",    0x00000, 0x20000, CRC(06f22615) SHA1(2e9418e372da85ea597977d912d8b35753655f4e) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END

ROM_START( wwfwfestb )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "3",      0x00000, 0x40000, CRC(ea73369c) SHA1(be614a342f9014251810fa30ec56fec03f7c8ef3) )
	ROM_LOAD16_BYTE( "2",      0x00001, 0x40000, CRC(632bb3a4) SHA1(9c04fed5aeefc683810cfbd9b3318e155ed9813f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "1",             0x00000, 0x10000, CRC(d9e8cda2) SHA1(754c73cd341d51ffd35cdb62155a3f061416c9ba) )

	ROM_REGION( 0x80000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "wf_73a.rom",    0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "4",             0x00000, 0x20000, CRC(520ef575) SHA1(99a5e9b94e9234851c6b504d58939ad84e0d6589) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16) */
	ROM_LOAD( "wf_09.rom",    0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "wf_08.rom",    0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "wf_11.rom",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "wf_10.rom",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "wf_12.rom",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "wf_13.rom",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "wf_15.rom",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "wf_14.rom",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD16_BYTE( "5",     0x40000, 0x20000, CRC(35e4d6eb) SHA1(d2a12bde268bc0734e6806ff5302b8c3dcc17280) ) /* 0 */
	ROM_LOAD16_BYTE( "6",     0x40001, 0x20000, CRC(a054a5b2) SHA1(d6ed5d5a20acb7cdbaee8e3f520873650529c0ae) ) /* 1 */
	ROM_LOAD16_BYTE( "7",     0x00000, 0x20000, CRC(101f0136) SHA1(2ccd641e49cdd3f5243ebe8c52c492842d62f5b8) ) /* 2 */
	ROM_LOAD16_BYTE( "8",     0x00001, 0x20000, CRC(7b2ecba7) SHA1(1ed2451132448930ac4afcdc67ca14e3e922863e) ) /* 3 */
ROM_END

ROM_START( wwfwfestj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Main CPU  (68000) */
	ROM_LOAD16_BYTE( "31j13-0.ic19", 0x00001, 0x40000, CRC(2147780d) SHA1(9a7a5db06117f3780e084d3f0c7b642ff8a9db55) )
	ROM_LOAD16_BYTE( "31j14-0.ic18", 0x00000, 0x40000, CRC(d76fc747) SHA1(5f6819bc61756d1df4ac0776ac420a59c438cf8a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU (Z80)  */
	ROM_LOAD( "31a11-2.ic42", 0x00000, 0x10000, CRC(5ddebfea) SHA1(30073963e965250d94f0dc3bd261a054850adf95) )

	ROM_REGION( 0x80000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "31j10.ic73",   0x00000, 0x80000, CRC(6c522edb) SHA1(8005d59c94160638ba2ea7caf4e991fff03003d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG0 Tiles (8x8) */
	ROM_LOAD( "31j12-0.ic33", 0x00000, 0x20000, CRC(f4821fe0) SHA1(e5faa9860e9d4e75393b64ca85a8bfc4852fd4fd) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* SPR Tiles (16x16), 27080 Mask ROM's */
	ROM_LOAD( "31j3.ic9",     0x000000, 0x100000, CRC(e395cf1d) SHA1(241f98145e295993c9b6a44dc087a9b61fbc9a6f) ) /* Tiles 0 */
	ROM_LOAD( "31j2.ic8",     0x100000, 0x100000, CRC(b5a97465) SHA1(08d82c29a5c02b83fdbd0bad649b74eb35ab7e54) ) /* Tiles 1 */
	ROM_LOAD( "31j5.ic11",    0x200000, 0x100000, CRC(2ce545e8) SHA1(82173e58a8476a6fe9d2c990fce1f71af117a0ea) ) /* Tiles 0 */
	ROM_LOAD( "31j4.ic10",    0x300000, 0x100000, CRC(00edb66a) SHA1(926606d1923936b6e75391b1ab03b369d9822d13) ) /* Tiles 1 */
	ROM_LOAD( "31j6.ic12",    0x400000, 0x100000, CRC(79956cf8) SHA1(52207263620a6b6dde66d3f8749b772577899ea5) ) /* Tiles 0 */
	ROM_LOAD( "31j7.ic13",    0x500000, 0x100000, CRC(74d774c3) SHA1(a723ac5d481bf91b12e17652fbb2d869c886dec0) ) /* Tiles 1 */
	ROM_LOAD( "31j9.ic15",    0x600000, 0x100000, CRC(dd387289) SHA1(2cad42d4e7cd1a49346f844058ae18c38bc686a8) ) /* Tiles 0 */
	ROM_LOAD( "31j8.ic14",    0x700000, 0x100000, CRC(44abe127) SHA1(c723e1dea117534e976d2d383e634faf073cd57b) ) /* Tiles 1 */

	ROM_REGION( 0x80000, "gfx3", 0 ) /* BG0 / BG1 Tiles (16x16) */
	ROM_LOAD( "31j0.ic1",     0x40000, 0x40000, CRC(8a12b450) SHA1(2e15c949efcda8bb6f11afe3ff07ba1dee9c771c) ) /* 0,1 */
	ROM_LOAD( "31j1.ic2",     0x00000, 0x40000, CRC(82ed7155) SHA1(b338e1150ffe3277c11d4d6e801a7d3bd7c58492) ) /* 2,3 */
ROM_END

GAME( 1991, wwfwfest,  0,        wwfwfest, wwfwfest, 0, ROT0, "Technos Japan",                 "WWF WrestleFest (US set 1)",   GAME_SUPPORTS_SAVE )
GAME( 1991, wwfwfesta, wwfwfest, wwfwfest, wwfwfest, 0, ROT0, "Technos Japan (Tecmo license)", "WWF WrestleFest (US Tecmo)",   GAME_SUPPORTS_SAVE )
GAME( 1991, wwfwfestb, wwfwfest, wwfwfstb, wwfwfest, 0, ROT0, "bootleg",                       "WWF WrestleFest (US bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1991, wwfwfestj, wwfwfest, wwfwfest, wwfwfest, 0, ROT0, "Technos Japan",                 "WWF WrestleFest (Japan)",      GAME_SUPPORTS_SAVE )
