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

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/2610intf.h"
#include "includes/inufuku.h"


/******************************************************************************

    Sound CPU interface

******************************************************************************/

WRITE16_MEMBER(inufuku_state::inufuku_soundcommand_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* hack... sound doesn't work otherwise */
		if (data == 0x08)
			return;

		m_pending_command = 1;
		soundlatch_w(space, 0, data & 0xff);
		device_set_input_line(m_audiocpu, INPUT_LINE_NMI, PULSE_LINE);
	}
}

WRITE8_MEMBER(inufuku_state::pending_command_clear_w)
{
	m_pending_command = 0;
}

WRITE8_MEMBER(inufuku_state::inufuku_soundrombank_w)
{
	memory_set_bank(machine(), "bank1", data & 0x03);
}

/******************************************************************************

    Input/Output port interface

******************************************************************************/

static CUSTOM_INPUT( soundflag_r )
{
	inufuku_state *state = field.machine().driver_data<inufuku_state>();
	UINT16 soundflag = state->m_pending_command ? 0 : 1;

	return soundflag;
}

/******************************************************************************

    Main CPU memory handlers

******************************************************************************/

static ADDRESS_MAP_START( inufuku_map, AS_PROGRAM, 16, inufuku_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM			// main rom

	AM_RANGE(0x100000, 0x100007) AM_WRITENOP	// ?

	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("P1")
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("P2")
	AM_RANGE(0x180004, 0x180005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x180006, 0x180007) AM_READ_PORT("P4")
	AM_RANGE(0x180008, 0x180009) AM_READ_PORT("EXTRA")
	AM_RANGE(0x18000a, 0x18000b) AM_READ_PORT("P3")

	AM_RANGE(0x200000, 0x200001) AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0x280000, 0x280001) AM_WRITE(inufuku_soundcommand_w)	// sound command

	AM_RANGE(0x300000, 0x301fff) AM_RAM_WRITE(paletteram16_xGGGGGBBBBBRRRRR_word_w) AM_SHARE("paletteram")						// palette ram
	AM_RANGE(0x380000, 0x3801ff) AM_WRITEONLY AM_BASE(m_bg_rasterram)									// bg raster ram
	AM_RANGE(0x400000, 0x401fff) AM_READWRITE(inufuku_bg_videoram_r, inufuku_bg_videoram_w) AM_BASE(m_bg_videoram)		// bg ram
	AM_RANGE(0x402000, 0x403fff) AM_READWRITE(inufuku_tx_videoram_r, inufuku_tx_videoram_w) AM_BASE(m_tx_videoram)		// text ram
	AM_RANGE(0x580000, 0x580fff) AM_RAM AM_BASE_SIZE(m_spriteram1, m_spriteram1_size)							// sprite table + sprite attribute
	AM_RANGE(0x600000, 0x61ffff) AM_RAM AM_BASE(m_spriteram2)											// cell table

	AM_RANGE(0x780000, 0x780013) AM_WRITE(inufuku_palettereg_w)	// bg & text palettebank register
	AM_RANGE(0x7a0000, 0x7a0023) AM_WRITE(inufuku_scrollreg_w)	// bg & text scroll register
	AM_RANGE(0x7e0000, 0x7e0001) AM_WRITENOP					// ?

	AM_RANGE(0x800000, 0xbfffff) AM_ROM	// data rom
	AM_RANGE(0xfd0000, 0xfdffff) AM_RAM // work ram
ADDRESS_MAP_END


/******************************************************************************

    Sound CPU memory handlers

******************************************************************************/

static ADDRESS_MAP_START( inufuku_sound_map, AS_PROGRAM, 8, inufuku_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( inufuku_sound_io_map, AS_IO, 8, inufuku_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(inufuku_soundrombank_w)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE_LEGACY("ymsnd", ym2610_r, ym2610_w)
ADDRESS_MAP_END

/******************************************************************************

    Port definitions

******************************************************************************/

static INPUT_PORTS_START( inufuku )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_DIPNAME( 0x10, 0x10, "3P/4P" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(soundflag_r, NULL)	// pending sound command

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)

	PORT_START("P3")
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
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,    0, 256*16 )	// bg
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    0, 256*16 )	// text
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,  0, 256*16 )	// sprite
GFXDECODE_END


/******************************************************************************

    Sound definitions

******************************************************************************/

static void irqhandler( device_t *device, int irq )
{
	inufuku_state *state = device->machine().driver_data<inufuku_state>();
	device_set_input_line(state->m_audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};


/******************************************************************************

    Machine driver

******************************************************************************/

static MACHINE_START( inufuku )
{
	inufuku_state *state = machine.driver_data<inufuku_state>();
	UINT8 *ROM = machine.region("audiocpu")->base();

	memory_configure_bank(machine, "bank1", 0, 4, &ROM[0x10000], 0x8000);
	memory_set_bank(machine, "bank1", 0);

	state->m_audiocpu = machine.device("audiocpu");

	state->save_item(NAME(state->m_pending_command));
	state->save_item(NAME(state->m_bg_scrollx));
	state->save_item(NAME(state->m_bg_scrolly));
	state->save_item(NAME(state->m_tx_scrollx));
	state->save_item(NAME(state->m_tx_scrolly));
	state->save_item(NAME(state->m_bg_raster));
	state->save_item(NAME(state->m_bg_palettebank));
	state->save_item(NAME(state->m_tx_palettebank));
}

static MACHINE_RESET( inufuku )
{
	inufuku_state *state = machine.driver_data<inufuku_state>();

	state->m_pending_command = 1;
	state->m_bg_scrollx = 0;
	state->m_bg_scrolly = 0;
	state->m_tx_scrollx = 0;
	state->m_tx_scrolly = 0;
	state->m_bg_raster = 0;
	state->m_bg_palettebank = 0;
	state->m_tx_palettebank = 0;
}

static MACHINE_CONFIG_START( inufuku, inufuku_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 32000000/2)	/* 16.00 MHz */
	MCFG_CPU_PROGRAM_MAP(inufuku_map)
	MCFG_CPU_VBLANK_INT("screen", irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 32000000/4)		/* 8.00 MHz */
	MCFG_CPU_PROGRAM_MAP(inufuku_sound_map)
	MCFG_CPU_IO_MAP(inufuku_sound_io_map)
								/* IRQs are triggered by the YM2610 */

	MCFG_MACHINE_START(inufuku)
	MCFG_MACHINE_RESET(inufuku)

	MCFG_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(2048, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 319-1, 1, 224-1)
	MCFG_SCREEN_UPDATE_STATIC(inufuku)

	MCFG_GFXDECODE(inufuku)
	MCFG_PALETTE_LENGTH(4096)

	MCFG_VIDEO_START(inufuku)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 32000000/4)
	MCFG_SOUND_CONFIG(ym2610_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.75)
	MCFG_SOUND_ROUTE(2, "mono", 0.75)
MACHINE_CONFIG_END


/******************************************************************************

    ROM definitions

******************************************************************************/

ROM_START( inufuku )
	ROM_REGION( 0x1000000, "maincpu", 0 )	// main cpu + data
	ROM_LOAD16_WORD_SWAP( "u147.bin",     0x0000000, 0x080000, CRC(ab72398c) SHA1(f5dc266ffa936ea6528b46a34113f5e2f8141d71) )
	ROM_LOAD16_WORD_SWAP( "u146.bin",     0x0080000, 0x080000, CRC(e05e9bd4) SHA1(af0fdf31c2bdf851bf15c9de725dcbbb58464d54) )
	ROM_LOAD16_WORD_SWAP( "lhmn5l28.148", 0x0800000, 0x400000, CRC(802d17e7) SHA1(43b26efea65fd051c094d19784cb977ced39a1a0) )

	ROM_REGION( 0x0030000, "audiocpu", 0 )	// sound cpu
	ROM_LOAD( "u107.bin", 0x0000000, 0x020000, CRC(1744ef90) SHA1(e019f4ca83e21aa25710cc0ca40ffe765c7486c9) )
	ROM_RELOAD( 0x010000, 0x020000 )

	ROM_REGION( 0x0400000, "gfx1", 0 )	// bg
	ROM_LOAD16_WORD_SWAP( "lhmn5ku8.u40", 0x0000000, 0x400000, CRC(8cbca80a) SHA1(063e9be97f5a1f021f8326f2994b51f9af5e1eaf) )

	ROM_REGION( 0x0400000, "gfx2", 0 )	// text
	ROM_LOAD16_WORD_SWAP( "lhmn5ku7.u8",  0x0000000, 0x400000, CRC(a6c0f07f) SHA1(971803d1933d8296767d8766ea9f04dcd6ab065c) )

	ROM_REGION( 0x0c00000, "gfx3", 0 )	// sprite
	ROM_LOAD16_WORD_SWAP( "lhmn5kub.u34", 0x0000000, 0x400000, CRC(7753a7b6) SHA1(a2e8747ce83ea5a57e2fe62f2452de355d7f48b6) )
	ROM_LOAD16_WORD_SWAP( "lhmn5kua.u36", 0x0400000, 0x400000, CRC(1ac4402a) SHA1(c15acc6fce4fe0b54e92d14c31a1bd78acf2c8fc) )
	ROM_LOAD16_WORD_SWAP( "lhmn5ku9.u38", 0x0800000, 0x400000, CRC(e4e9b1b6) SHA1(4d4ad85fbe6a442d4f8cafad748bcae4af6245b7) )

	ROM_REGION( 0x0400000, "ymsnd", 0 )	// adpcm data
	ROM_LOAD( "lhmn5ku6.u53", 0x0000000, 0x400000, CRC(b320c5c9) SHA1(7c99da2d85597a3c008ed61a3aa5f47ad36186ec) )
ROM_END


/******************************************************************************

    Game drivers

******************************************************************************/

GAME( 1998, inufuku, 0, inufuku, inufuku, 0, ROT0, "Video System Co.", "Quiz & Variety Sukusuku Inufuku (Japan)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
