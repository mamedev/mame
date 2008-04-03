/******************************************************************************

    Video Hardware for Video System Games.

    Quiz & Variety Sukusuku Inufuku (Japan)
    (c)1998 Video System Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2003/08/09 -

    based on other Video System drivers

******************************************************************************/
/******************************************************************************

Quiz & Variety Sukusuku Inufuku
(c)1998 Video System

VSBB-31-1

CPU  : MC68HC000P-16
Sound: TMPZ84C000AP-8 YM2610 YM3016
OSC  : 32.0000MHz 14.31818MHz

ROMs:
U107.BIN     - Sound Program (27C1001)

U146.BIN     - Main Programs (27C240)
U147.BIN     |
LHMN5L28.148 / (32M Mask)

Others:
93C46 (EEPROM)
UMAG1 (ALTERA MAX EPM7128ELC84-10 BG9625)
PLD00?? (ALTERA EPM7032LC44-15 BA9631)
002 (PALCE16V8-10PC)
003 (PALCE16V8-15PC)

Custom Chips:
VS920A
VS920E
VS9210
VS9108 (Fujitsu CG10103)
(blank pattern for VS9210 and VS9108)

VSBB31-ROM

ROMs:
LHMN5KU6.U53 - 32M SOP Mask ROMs
LHMN5KU8.U40 |
LHMN5KU7.U8  |
LHMN5KUB.U34 |
LHMN5KUA.U36 |
LHMN5KU9.U38 /

******************************************************************************/
/******************************************************************************

TODO:

- User must initialize NVRAM at first boot in test mode (factory settings).

- Sometimes, sounds are not played (especially SFX), but this is a bug of real machine.

- Sound Code 0x08 remains unknown.

- Priority of tests and sprites seems to be correct, but I may have mistaken.

******************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/2610intf.h"


VIDEO_UPDATE( inufuku );
VIDEO_START( inufuku );

UINT16 *inufuku_bg_videoram;
UINT16 *inufuku_bg_rasterram;
UINT16 *inufuku_text_videoram;
UINT16 *inufuku_spriteram1;
UINT16 *inufuku_spriteram2;
size_t inufuku_spriteram1_size;
static UINT16 pending_command;

WRITE16_HANDLER( inufuku_paletteram_w );
READ16_HANDLER( inufuku_bg_videoram_r );
WRITE16_HANDLER( inufuku_bg_videoram_w );
READ16_HANDLER( inufuku_text_videoram_r );
WRITE16_HANDLER( inufuku_text_videoram_w );
WRITE16_HANDLER( inufuku_palettereg_w );
WRITE16_HANDLER( inufuku_scrollreg_w );


/******************************************************************************

    Sound CPU interface

******************************************************************************/

static WRITE16_HANDLER( inufuku_soundcommand_w )
{
	if (ACCESSING_BITS_0_7) {

		/* hack... sound doesn't work otherwise */
		if (data == 0x08) return;

		pending_command = 1;
		soundlatch_w(machine, 0, data & 0xff);
		cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static WRITE8_HANDLER( pending_command_clear_w )
{
	pending_command = 0;
}

static WRITE8_HANDLER( inufuku_soundrombank_w )
{
	UINT8 *ROM = memory_region(REGION_CPU2) + 0x10000;

	memory_set_bankptr(1, ROM + (data & 0x03) * 0x8000);
}


/******************************************************************************

    Machine initialization / Driver initialization

******************************************************************************/

static MACHINE_RESET( inufuku )
{
	;
}

static DRIVER_INIT( inufuku )
{
	pending_command = 1;
	inufuku_soundrombank_w(machine, 0, 0);
}


/******************************************************************************

    Input/Output port interface

******************************************************************************/

static READ16_HANDLER( inufuku_eeprom_r )
{
	UINT16 soundflag;
	UINT16 eeprom;
	UINT16 inputport;

	soundflag = pending_command ? 0x0000 : 0x0080;	// bit7
	eeprom = (EEPROM_read_bit() & 1) << 6;			// bit6
	inputport = readinputport(4) & 0xff3f;			// bit5-0

	return (soundflag | eeprom | inputport);
}

static WRITE16_HANDLER( inufuku_eeprom_w )
{
	// latch the bit
	EEPROM_write_bit(data & 0x0800);

	// reset line asserted: reset.
	EEPROM_set_cs_line((data & 0x2000) ? CLEAR_LINE : ASSERT_LINE);

	// clock line asserted: write latch or select next bit to read
	EEPROM_set_clock_line((data & 0x1000) ? ASSERT_LINE : CLEAR_LINE);
}


/******************************************************************************

    Main CPU memory handlers

******************************************************************************/

static ADDRESS_MAP_START( inufuku_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(SMH_ROM)				// main rom

	AM_RANGE(0x180000, 0x180001) AM_READ(input_port_0_word_r)
	AM_RANGE(0x180002, 0x180003) AM_READ(input_port_1_word_r)
	AM_RANGE(0x180004, 0x180005) AM_READ(input_port_2_word_r)
	AM_RANGE(0x180006, 0x180007) AM_READ(input_port_3_word_r)
	AM_RANGE(0x180008, 0x180009) AM_READ(inufuku_eeprom_r)		// eeprom + input_port_4_word_r
	AM_RANGE(0x18000a, 0x18000b) AM_READ(input_port_5_word_r)

	AM_RANGE(0x300000, 0x301fff) AM_READ(SMH_RAM)				// palette ram
	AM_RANGE(0x400000, 0x401fff) AM_READ(inufuku_bg_videoram_r)	// bg ram
	AM_RANGE(0x402000, 0x403fff) AM_READ(inufuku_text_videoram_r)// text ram
	AM_RANGE(0x580000, 0x580fff) AM_READ(SMH_RAM)				// sprite table + sprite attribute
	AM_RANGE(0x600000, 0x61ffff) AM_READ(SMH_RAM)				// cell table

	AM_RANGE(0x800000, 0xbfffff) AM_READ(SMH_ROM)				// data rom
	AM_RANGE(0xfd0000, 0xfdffff) AM_READ(SMH_RAM)				// work ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( inufuku_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(SMH_ROM)				// main rom

	AM_RANGE(0x100000, 0x100007) AM_WRITE(SMH_NOP)				// ?
	AM_RANGE(0x200000, 0x200001) AM_WRITE(inufuku_eeprom_w)		// eeprom
	AM_RANGE(0x280000, 0x280001) AM_WRITE(inufuku_soundcommand_w)	// sound command

	AM_RANGE(0x300000, 0x301fff) AM_WRITE(paletteram16_xGGGGGBBBBBRRRRR_word_w) AM_BASE(&paletteram16)		// palette ram
	AM_RANGE(0x380000, 0x3801ff) AM_WRITE(SMH_RAM) AM_BASE(&inufuku_bg_rasterram)							// bg raster ram
	AM_RANGE(0x400000, 0x401fff) AM_WRITE(inufuku_bg_videoram_w) AM_BASE(&inufuku_bg_videoram)				// bg ram
	AM_RANGE(0x402000, 0x403fff) AM_WRITE(inufuku_text_videoram_w) AM_BASE(&inufuku_text_videoram)			// text ram
	AM_RANGE(0x580000, 0x580fff) AM_WRITE(SMH_RAM) AM_BASE(&inufuku_spriteram1) AM_SIZE(&inufuku_spriteram1_size)	// sprite table + sprite attribute
	AM_RANGE(0x600000, 0x61ffff) AM_WRITE(SMH_RAM) AM_BASE(&inufuku_spriteram2)								// cell table

	AM_RANGE(0x780000, 0x780013) AM_WRITE(inufuku_palettereg_w)	// bg & text palettebank register
	AM_RANGE(0x7a0000, 0x7a0023) AM_WRITE(inufuku_scrollreg_w)	// bg & text scroll register
	AM_RANGE(0x7e0000, 0x7e0001) AM_WRITE(SMH_NOP)				// ?

	AM_RANGE(0x800000, 0xbfffff) AM_WRITE(SMH_ROM)				// data rom
	AM_RANGE(0xfd0000, 0xfdffff) AM_WRITE(SMH_RAM)				// work ram
ADDRESS_MAP_END


/******************************************************************************

    Sound CPU memory handlers

******************************************************************************/

static ADDRESS_MAP_START( inufuku_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_READ(SMH_ROM)
	AM_RANGE(0x7800, 0x7fff) AM_READ(SMH_RAM)
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_BANK1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( inufuku_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( inufuku_readport_sound, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r)
	AM_RANGE(0x08, 0x08) AM_READ(YM2610_status_port_0_A_r)
	AM_RANGE(0x09, 0x09) AM_READ(YM2610_read_port_0_r)
	AM_RANGE(0x0a, 0x0a) AM_READ(YM2610_status_port_0_B_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( inufuku_writeport_sound, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(inufuku_soundrombank_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(YM2610_control_port_0_A_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(YM2610_data_port_0_A_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(YM2610_control_port_0_B_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(YM2610_data_port_0_B_w)
ADDRESS_MAP_END


/******************************************************************************

    Port definitions

******************************************************************************/

static INPUT_PORTS_START( inufuku )
	PORT_START	// 0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START	// 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START	// 2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	// 3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START	// 4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_DIPNAME( 0x10, 0x10, "3P/4P" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// pending sound command

	PORT_START	// 5
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
INPUT_PORTS_END


/******************************************************************************

    Graphics definitions

******************************************************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( inufuku )
	GFXDECODE_ENTRY( REGION_GFX1, 0, tilelayout,    0, 256*16 )	// bg
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout,    0, 256*16 )	// text
	GFXDECODE_ENTRY( REGION_GFX3, 0, spritelayout,  0, 256*16 )	// sprite
GFXDECODE_END


/******************************************************************************

    Sound definitions

******************************************************************************/

static void irqhandler(int irq)
{
	cpunum_set_input_line(Machine, 1, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YM2610interface ym2610_interface =
{
	irqhandler,
	0,
	REGION_SOUND1
};


/******************************************************************************

    Machine driver

******************************************************************************/

static MACHINE_DRIVER_START( inufuku )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 32000000/2)	/* 16.00 MHz */
	MDRV_CPU_PROGRAM_MAP(inufuku_readmem, inufuku_writemem)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	MDRV_CPU_ADD(Z80, 32000000/4)		/* 8.00 MHz */
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(inufuku_readmem_sound, inufuku_writemem_sound)
	MDRV_CPU_IO_MAP(inufuku_readport_sound, inufuku_writeport_sound)
								/* IRQs are triggered by the YM2610 */

	MDRV_MACHINE_RESET(inufuku)
	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(2048, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 319-1, 1, 224-1)

	MDRV_GFXDECODE(inufuku)
	MDRV_PALETTE_LENGTH(4096)

	MDRV_VIDEO_START(inufuku)
	MDRV_VIDEO_UPDATE(inufuku)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2610, 32000000/4)
	MDRV_SOUND_CONFIG(ym2610_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.75)
	MDRV_SOUND_ROUTE(2, "mono", 0.75)
MACHINE_DRIVER_END


/******************************************************************************

    ROM definitions

******************************************************************************/

ROM_START( inufuku )
	ROM_REGION( 0x1000000, REGION_CPU1, 0 )	// main cpu + data
	ROM_LOAD16_WORD_SWAP( "u147.bin",     0x0000000, 0x080000, CRC(ab72398c) SHA1(f5dc266ffa936ea6528b46a34113f5e2f8141d71) )
	ROM_LOAD16_WORD_SWAP( "u146.bin",     0x0080000, 0x080000, CRC(e05e9bd4) SHA1(af0fdf31c2bdf851bf15c9de725dcbbb58464d54) )
	ROM_LOAD16_WORD_SWAP( "lhmn5l28.148", 0x0800000, 0x400000, CRC(802d17e7) SHA1(43b26efea65fd051c094d19784cb977ced39a1a0) )

	ROM_REGION( 0x0030000, REGION_CPU2, 0 )	// sound cpu
	ROM_LOAD( "u107.bin", 0x0000000, 0x020000, CRC(1744ef90) SHA1(e019f4ca83e21aa25710cc0ca40ffe765c7486c9) )
	ROM_RELOAD( 0x010000, 0x020000 )

	ROM_REGION( 0x0400000, REGION_GFX1, ROMREGION_DISPOSE )	// bg
	ROM_LOAD16_WORD_SWAP( "lhmn5ku8.u40", 0x0000000, 0x400000, CRC(8cbca80a) SHA1(063e9be97f5a1f021f8326f2994b51f9af5e1eaf) )

	ROM_REGION( 0x0400000, REGION_GFX2, ROMREGION_DISPOSE )	// text
	ROM_LOAD16_WORD_SWAP( "lhmn5ku7.u8",  0x0000000, 0x400000, CRC(a6c0f07f) SHA1(971803d1933d8296767d8766ea9f04dcd6ab065c) )

	ROM_REGION( 0x0c00000, REGION_GFX3, ROMREGION_DISPOSE )	// sprite
	ROM_LOAD16_WORD_SWAP( "lhmn5kub.u34", 0x0000000, 0x400000, CRC(7753a7b6) SHA1(a2e8747ce83ea5a57e2fe62f2452de355d7f48b6) )
	ROM_LOAD16_WORD_SWAP( "lhmn5kua.u36", 0x0400000, 0x400000, CRC(1ac4402a) SHA1(c15acc6fce4fe0b54e92d14c31a1bd78acf2c8fc) )
	ROM_LOAD16_WORD_SWAP( "lhmn5ku9.u38", 0x0800000, 0x400000, CRC(e4e9b1b6) SHA1(4d4ad85fbe6a442d4f8cafad748bcae4af6245b7) )

	ROM_REGION( 0x0400000, REGION_SOUND1, 0 )	// adpcm data
	ROM_LOAD( "lhmn5ku6.u53", 0x0000000, 0x400000, CRC(b320c5c9) SHA1(7c99da2d85597a3c008ed61a3aa5f47ad36186ec) )
ROM_END


/******************************************************************************

    Game drivers

******************************************************************************/

GAME( 1998, inufuku, 0, inufuku, inufuku, inufuku, ROT0, "Video System Co.", "Quiz & Variety Sukusuku Inufuku (Japan)", GAME_NO_COCKTAIL )
