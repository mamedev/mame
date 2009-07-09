/***************************************************************************

    Green Beret
    Konami

    driver by Nicola Salmoria
    correct rom naming information by Belgium Dump Team (17/06/2003)

    Games supported:
        * Green Beret
        * Rush'n Attack (US)
        * Green Beret (bootleg)
        * Mr. Goemon (Japan)

    gberetb is a bootleg hacked to run on different hardware.

****************************************************************************

    Memory map

    0000-bfff   ROM
    c000-c7ff   Color RAM
    c800-cfff   Video RAM
    d000-d0c0   Sprites (bank 0)
    d100-d1c0   Sprites (bank 1)
    d200-dfff   RAM
    e000-e01f   ZRAM1 line scroll registers
    e020-e03f   ZRAM2 bit 8 of line scroll registers

    read:
    f200      DSW1
                    bit 0-1 lives
                    bit 2   cocktail/upright cabinet (0 = upright)
                    bit 3-4 bonus
                    bit 5-6 difficulty
                    bit 7   demo sounds
    f400      DSW2
                    bit 0 = screen flip
                    bit 1 = single/dual upright controls
    f600      DSW0
                    bit 0-1-2-3 coins per play Coin1
                    bit 4-5-6-7 coins per play Coin2
    f601      IN1 player 2 controls
    f602      IN0 player 1 controls
    f603      IN2
                    bit 0-1-2 coin  bit 3 1 player start  bit 4 2 players start

    write:
    e040      ?
    e041      ?
    e042      ?
    e043      bit 3 = sprite RAM bank select; other bits = ?
    e044      bit 0-2 = interrupt control; bit 3 = flip screen
    f000      ?
    f200      SN76496 command
    f400      SN76496 trigger (write command to f200, then write to this location
                                to cause the chip to read it)
    f600      watchdog reset

****************************************************************************

    Interrupts

    The game uses both IRQ (mode 1) and NMI.

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/sn76496.h"
#include "konamipt.h"

extern UINT8 *gberet_scrollram;

extern WRITE8_HANDLER( gberet_videoram_w );
extern WRITE8_HANDLER( gberet_colorram_w );
extern WRITE8_HANDLER( gberet_scroll_w );
extern WRITE8_HANDLER( gberetb_scroll_w );
extern WRITE8_HANDLER( gberet_sprite_bank_w );

extern PALETTE_INIT( gberet );
extern VIDEO_START( gberet );
extern VIDEO_UPDATE( gberet );
extern VIDEO_UPDATE( gberetb );


static UINT8 nmi_enable, irq_enable;


/* Interrupt Generators */

static INTERRUPT_GEN( gberet_interrupt )
{
	if (cpu_getiloops(device) == 0)
	{
		if (irq_enable)
			cpu_set_input_line(device, 0, HOLD_LINE);
	}

	if (cpu_getiloops(device) % 2)
	{
		if (nmi_enable)
			cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
	}
}


/* Read/Write Handlers */

static WRITE8_HANDLER( gberet_coin_counter_w )
{
	/* bits 0/1 = coin counters */
	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);
}

static WRITE8_HANDLER( gberet_flipscreen_w )
{
	nmi_enable = data & 0x01;
	irq_enable = data & 0x04;

	flip_screen_set(space->machine, data & 0x08);
}

static WRITE8_HANDLER( mrgoemon_coin_counter_w )
{
	int offs;

	/* bits 0/1 = coin counters */
	coin_counter_w(0, data & 0x01);
	coin_counter_w(1, data & 0x02);

	/* bits 5-7 = ROM bank select */
	offs = 0x10000 + ((data & 0xe0) >> 5) * 0x800;
	memory_set_bankptr(space->machine, 1, &memory_region(space->machine, "maincpu")[offs]);
}

static WRITE8_HANDLER( mrgoemon_flipscreen_w )
{
	nmi_enable = data & 0x01;
	irq_enable = data & 0x02;

	flip_screen_set(space->machine, data & 0x08);
}

/* Memory Maps */

static ADDRESS_MAP_START( gberet_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(gberet_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(gberet_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xd0ff) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0xd100, 0xd1ff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0xd200, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe03f) AM_RAM_WRITE(gberet_scroll_w) AM_BASE(&gberet_scrollram)
	AM_RANGE(0xe040, 0xe042) AM_WRITENOP // ???
	AM_RANGE(0xe043, 0xe043) AM_WRITE(gberet_sprite_bank_w)
	AM_RANGE(0xe044, 0xe044) AM_WRITE(gberet_flipscreen_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(gberet_coin_counter_w)
	AM_RANGE(0xf200, 0xf200) AM_READ_PORT("DSW2") AM_WRITENOP			// Loads the snd command into the snd latch
	AM_RANGE(0xf400, 0xf400) AM_READ_PORT("DSW3") AM_DEVWRITE("sn", sn76496_w)	// This address triggers the SN chip to read the data port.
	AM_RANGE(0xf600, 0xf600) AM_READ_PORT("DSW1")
	AM_RANGE(0xf601, 0xf601) AM_READ_PORT("P2")
	AM_RANGE(0xf602, 0xf602) AM_READ_PORT("P1")
	AM_RANGE(0xf603, 0xf603) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf600, 0xf600) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gberetb_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(gberet_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(gberet_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe03f) AM_RAM
	AM_RANGE(0xe040, 0xe043) AM_WRITENOP // ???
	AM_RANGE(0xe044, 0xe044) AM_WRITE(gberet_flipscreen_w)
	AM_RANGE(0xe800, 0xe8ff) AM_RAM
	AM_RANGE(0xe900, 0xe9ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP				// coin counter not supported
	AM_RANGE(0xf200, 0xf200) AM_READ_PORT("DSW2")
	AM_RANGE(0xf400, 0xf400) AM_DEVWRITE("sn", sn76496_w)
	AM_RANGE(0xf600, 0xf600) AM_READ_PORT("P2")
	AM_RANGE(0xf601, 0xf601) AM_READ_PORT("DSW1")
	AM_RANGE(0xf602, 0xf602) AM_READ_PORT("P1")
	AM_RANGE(0xf603, 0xf603) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf800, 0xf800) AM_NOP						// IRQ/NMI acknowledge
	AM_RANGE(0xf900, 0xf901) AM_WRITE(gberetb_scroll_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mrgoemon_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM_WRITE(gberet_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(gberet_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xd000, 0xd0ff) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0xd100, 0xd1ff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0xd200, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe03f) AM_RAM_WRITE(gberet_scroll_w) AM_BASE(&gberet_scrollram)
	AM_RANGE(0xe040, 0xe042) AM_WRITENOP // ???
	AM_RANGE(0xe043, 0xe043) AM_WRITE(gberet_sprite_bank_w)
	AM_RANGE(0xe044, 0xe044) AM_WRITE(mrgoemon_flipscreen_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(mrgoemon_coin_counter_w)
	AM_RANGE(0xf200, 0xf200) AM_READ_PORT("DSW2") AM_WRITENOP				// Loads the snd command into the snd latch
	AM_RANGE(0xf400, 0xf400) AM_READ_PORT("DSW3") AM_DEVWRITE("sn", sn76496_w)		// This address triggers the SN chip to read the data port.
	AM_RANGE(0xf600, 0xf600) AM_READ_PORT("DSW1") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xf601, 0xf601) AM_READ_PORT("P2")
	AM_RANGE(0xf602, 0xf602) AM_READ_PORT("P1")
	AM_RANGE(0xf603, 0xf603) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf800, 0xffff) AM_ROMBANK(1)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( gberet )
	PORT_START("P1")
	KONAMI8_MONO_B12_UNK					// button 1 = knife, button 2 = shoot

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "30K 70K 70K+" )
	PORT_DIPSETTING(    0x10, "40K 80K 80K+" )
	PORT_DIPSETTING(    0x08, "50K 100K 100K+" )
	PORT_DIPSETTING(    0x00, "50K 200K 200K+" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( gberetb )
	PORT_START("P1")
	KONAMI8_MONO_B12_UNK					// button 1 = knife, button 2 = shoot

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "30K 70K 70K+" )
	PORT_DIPSETTING(    0x10, "40K 80K 80K+" )
	PORT_DIPSETTING(    0x08, "50K 100K 100K+" )
	PORT_DIPSETTING(    0x00, "50K 200K 200K+" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mrgoemon )
	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "20K 60K+" )
	PORT_DIPSETTING(    0x10, "30K 70K+" )
	PORT_DIPSETTING(    0x08, "40K 80K+" )
	PORT_DIPSETTING(    0x00, "50K 90K+" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	512,	/* 512 sprites */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
		32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		64*8+0*32, 64*8+1*32, 64*8+2*32, 64*8+3*32, 64*8+4*32, 64*8+5*32, 64*8+6*32, 64*8+7*32 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout gberetb_charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	4,		/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 6*4, 7*4, 0*4, 1*4, 2*4, 3*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout gberetb_spritelayout =
{
	16,16,	/* 16*16 sprites */
	512,	/* 512 sprites */
	4,		/* 4 bits per pixel */
	{ 0*0x4000*8, 1*0x4000*8, 2*0x4000*8, 3*0x4000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

/* Graphics Decode Information */

static GFXDECODE_START( gberet )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gberetb )
	GFXDECODE_ENTRY( "gfx1", 0, gberetb_charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gberetb_spritelayout, 16*16, 16 )
GFXDECODE_END

/* Machine Drivers */

static MACHINE_DRIVER_START( gberet )
	// basic machine hardware
	MDRV_CPU_ADD("maincpu", Z80, 18432000/6)	// X1S (generated by a custom IC)
	MDRV_CPU_PROGRAM_MAP(gberet_map)
	MDRV_CPU_VBLANK_INT_HACK(gberet_interrupt, 32)	// 1 IRQ + 16 NMI (generated by a custom IC)

	// video hardware

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(30)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(gberet)
	MDRV_PALETTE_LENGTH(2*16*16)

	MDRV_PALETTE_INIT(gberet)
	MDRV_VIDEO_START(gberet)
	MDRV_VIDEO_UPDATE(gberet)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn", SN76496, 18432000/12)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gberetb )
	MDRV_IMPORT_FROM(gberet)

	// basic machine hardware
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gberetb_map)
	MDRV_CPU_VBLANK_INT_HACK(gberet_interrupt, 16)	// 1 IRQ + 8 NMI

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	// video hardware
	MDRV_GFXDECODE(gberetb)

	MDRV_VIDEO_UPDATE(gberetb)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mrgoemon )
	MDRV_IMPORT_FROM(gberet)

	// basic machine hardware
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(mrgoemon_map)
	MDRV_CPU_VBLANK_INT_HACK(gberet_interrupt, 16)	// 1 IRQ + 8 NMI

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
MACHINE_DRIVER_END

/* ROMs */

ROM_START( gberet )
	ROM_REGION( 0x10000, "maincpu", 0 )	// 64k for code
	ROM_LOAD( "577l03.10c",   0x0000, 0x4000, CRC(ae29e4ff) SHA1(5c66de1403c5df5b6647bb37e26070ffd33590e8) )
	ROM_LOAD( "577l02.8c",    0x4000, 0x4000, CRC(240836a5) SHA1(b76f3789f152198bf8a9a366378d664e683c6c9d) )
	ROM_LOAD( "577l01.7c",    0x8000, 0x4000, CRC(41fa3e1f) SHA1(90d1463e16b0f52c01078be044ce3672d4acebff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "577l07.3f",    0x00000, 0x4000, CRC(4da7bd1b) SHA1(54adba9ae086852902d78ab36039498aae50d7a9) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "577l06.5e",    0x00000, 0x4000, CRC(0f1cb0ca) SHA1(094004e70c05df8cd486d0854c258fa766e2925d) )
	ROM_LOAD( "577l05.4e",    0x04000, 0x4000, CRC(523a8b66) SHA1(5f2bcf2b702fe05f8a022b6284cb2d0a5b5f222f) )
	ROM_LOAD( "577l08.4f",    0x08000, 0x4000, CRC(883933a4) SHA1(b565842edf09feeb2c4ac44ad58331757586b6aa) )
	ROM_LOAD( "577l04.3e",    0x0c000, 0x4000, CRC(ccecda4c) SHA1(cac053cab68cb420edd408ce032143db7abc29f5) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "577h09.2f",    0x0000, 0x0020, CRC(c15e7c80) SHA1(c0e8a01e63ed8cf20b33456b68890313b387ad23) ) // palette
	ROM_LOAD( "577h11.6f",    0x0020, 0x0100, CRC(2a1a992b) SHA1(77cff7c9c8433f999a87776021935864cf9dccb4) ) // characters
	ROM_LOAD( "577h10.5f",    0x0120, 0x0100, CRC(e9de1e53) SHA1(406b8dfe54e6176082005cc5545e79c098672547) ) // sprites
ROM_END

ROM_START( rushatck )
	ROM_REGION( 0x10000, "maincpu", 0 )	// 64k for code
	ROM_LOAD( "577h03.10c",   0x0000, 0x4000, CRC(4d276b52) SHA1(ba5d61c89fd2db4b303b81deccc887561156cbe3) )
	ROM_LOAD( "577h02.8c",    0x4000, 0x4000, CRC(b5802806) SHA1(0e4698ecfb9eda916703165ea5d55516fdef5fe4) )
	ROM_LOAD( "577h01.7c",    0x8000, 0x4000, CRC(da7c8f3d) SHA1(eb61eedee169f67db93407ad0fe8a195089b7e3a) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "577h07.3f",    0x00000, 0x4000, CRC(03f9815f) SHA1(209c76fd36d1b5672992c55e24d3cf77d4c5a0aa) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "577l06.5e",    0x00000, 0x4000, CRC(0f1cb0ca) SHA1(094004e70c05df8cd486d0854c258fa766e2925d) )
	ROM_LOAD( "577h05.4e",    0x04000, 0x4000, CRC(9d028e8f) SHA1(4faa47152a6c1da0024bb03fbcf7baf0540e891e) )
	ROM_LOAD( "577l08.4f",    0x08000, 0x4000, CRC(883933a4) SHA1(b565842edf09feeb2c4ac44ad58331757586b6aa) )
	ROM_LOAD( "577l04.3e",    0x0c000, 0x4000, CRC(ccecda4c) SHA1(cac053cab68cb420edd408ce032143db7abc29f5) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "577h09.2f",    0x0000, 0x0020, CRC(c15e7c80) SHA1(c0e8a01e63ed8cf20b33456b68890313b387ad23) ) // palette
	ROM_LOAD( "577h11.6f",    0x0020, 0x0100, CRC(2a1a992b) SHA1(77cff7c9c8433f999a87776021935864cf9dccb4) ) // characters
	ROM_LOAD( "577h10.5f",    0x0120, 0x0100, CRC(e9de1e53) SHA1(406b8dfe54e6176082005cc5545e79c098672547) ) // sprites
ROM_END

ROM_START( gberetb )
	ROM_REGION( 0x10000, "maincpu", 0 )	// 64k for code
	ROM_LOAD( "2-ic82.10g",   0x0000, 0x8000, CRC(6d6fb494) SHA1(0d01c86ed7a8962ee3e1056a8d41584ad1406f0f) )
	ROM_LOAD( "3-ic81.10f",   0x8000, 0x4000, CRC(f1520a0a) SHA1(227b2d2e1fc0e81ae02e663a3089e7399612e3cf) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "1-ic92.12c",   0x00000, 0x4000, CRC(b0189c87) SHA1(29202978b07bf059b88bf206d8fafc80e0cdb6dc) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "7-1c8.2b",     0x00000, 0x4000, CRC(86334522) SHA1(f2907d136dbfdb92cbd550524b4453755f6244b6) )
	ROM_LOAD( "6-ic9.2c",     0x04000, 0x4000, CRC(bda50d3e) SHA1(c6f5a15270a69464e977926d056b31dcec8b41c3) )
	ROM_LOAD( "5-ic10.2d",    0x08000, 0x4000, CRC(6a7b3881) SHA1(795bfb1fbc11ceac687b15e98574feb650e2f674) )
	ROM_LOAD( "4-ic11.2e",    0x0c000, 0x4000, CRC(3fb186c9) SHA1(40ce0447014af3f5b5b88648ab7e43a955bd1274) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "577h09",       0x0000, 0x0020, CRC(c15e7c80) SHA1(c0e8a01e63ed8cf20b33456b68890313b387ad23) ) // palette
	ROM_LOAD( "577h11.6f",    0x0020, 0x0100, CRC(2a1a992b) SHA1(77cff7c9c8433f999a87776021935864cf9dccb4) ) // characters
	ROM_LOAD( "577h10.5f",    0x0120, 0x0100, CRC(e9de1e53) SHA1(406b8dfe54e6176082005cc5545e79c098672547) ) // sprites
ROM_END

ROM_START( mrgoemon )
	ROM_REGION( 0x14000, "maincpu", 0 )	// 64k for code + banked ROM
	ROM_LOAD( "621d01.10c",   0x00000, 0x8000, CRC(b2219c56) SHA1(274160be5dabbbfa61af71d92bddffbb56eadab6) )
	ROM_LOAD( "621d02.12c",   0x08000, 0x4000, CRC(c3337a97) SHA1(6fd5f365b2624a37f252c202cd97877705b4a6c2) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "621a05.6d",    0x00000, 0x4000, CRC(f0a6dfc5) SHA1(395024ebfff550b0da393096483196fb1152a077) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "621d03.4d",    0x00000, 0x8000, CRC(66f2b973) SHA1(7e906f258a5f4928f9615c6ea176efbca659b3a7) )
	ROM_LOAD( "621d04.5d",    0x08000, 0x8000, CRC(47df6301) SHA1(e675c070e46993d3453c2ddadc49ec8b84cec854) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "621a06.5f",    0x0000, 0x0020, CRC(7c90de5f) SHA1(8ac5708e72e32f3d79ccde0cbaedefc34f8ac57e) ) // palette
	ROM_LOAD( "621a08.7f",    0x0020, 0x0100, CRC(2fb244dd) SHA1(ceb909ad96c0dabc8684e69b028f4287e227c351) ) // characters
	ROM_LOAD( "621a07.6f",    0x0120, 0x0100, CRC(3980acdc) SHA1(f4e0bd74bccd77b84096c38bc70cf488a42d9562) ) // sprites
ROM_END

/* Game Drivers */

GAME( 1985, gberet,   0,      gberet,   gberet,   0, ROT0, "Konami",  "Green Beret", 0 )
GAME( 1985, rushatck, gberet, gberet,   gberet,   0, ROT0, "Konami",  "Rush'n Attack (US)", 0 )
GAME( 1985, gberetb,  gberet, gberetb,  gberetb,  0, ROT0, "bootleg", "Green Beret (bootleg)", 0 )
GAME( 1986, mrgoemon, 0,      mrgoemon, mrgoemon, 0, ROT0, "Konami",  "Mr. Goemon (Japan)", 0 )
