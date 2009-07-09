/***************************************************************************

Quiz Punch 2 (C)1989 Space Computer

Preliminary driver by Luca Elia

- It uses an unknown DIP40 device for protection, that supplies
  the address to jump to (same as mosaic.c) and handles the EEPROM

PCB Layout
----------

|---------------------------------------------|
|U1    U26        6116                   32MHz|
|                             U120 U119       |
|U2                                  U118 U117|
|      U27        6116                        |
|                                             |
|                                             |
|      U28        6116        Z80             |
|                         U2A     93C46   U111|
|                                      6264   |
|      U29                    *           8MHz|
|  U20            6116                        |
|                                6116         |
|  U21 U30                                    |
|                 6116                        |
|  U22  6116                           DSW1(8)|
|VOL    YM2203 Z80                            |
|    YM3014     MAHJONG28                     |
|---------------------------------------------|
Notes:
      * - Unknown DIP40 chip. +5V on pin 17, GND on pin 22
          pins 4,3,2 of 93C46 tied to unknown chip on pins 23,24,25
      All clocks unknown, PCB not working
      Possibly Z80's @ 4MHz and YM2203 @ 2MHz
      PCB marked 'Ducksan Trading Co. Ltd. Made In Korea'

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"

#define VERBOSE_PROTECTION_LOG 0

/***************************************************************************
                                Video Hardware
***************************************************************************/

static UINT8   *bg_ram,  *fg_ram;
static tilemap *bg_tmap, *fg_tmap;

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT16 code = bg_ram[ tile_index * 2 ] + bg_ram[ tile_index * 2 + 1 ] * 256;
	SET_TILE_INFO(0, code, 0, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT16 code  = fg_ram[ tile_index * 4 ] + fg_ram[ tile_index * 4 + 1 ] * 256;
	UINT8  color = fg_ram[ tile_index * 4 + 2 ];
	SET_TILE_INFO(1, code, color & 0x0f, 0);
}

static WRITE8_HANDLER( bg_ram_w )
{
	bg_ram[offset] = data;
	tilemap_mark_tile_dirty(bg_tmap, offset/2);
}

static WRITE8_HANDLER( fg_ram_w )
{
	fg_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tmap, offset/4);
}

static VIDEO_START(quizpun2)
{
	bg_tmap = tilemap_create(	machine, get_bg_tile_info, tilemap_scan_rows,	8,16, 0x20,0x20	);
	fg_tmap = tilemap_create(	machine, get_fg_tile_info, tilemap_scan_rows,	8,16, 0x20,0x20	);

	tilemap_set_transparent_pen(bg_tmap, 0);
	tilemap_set_transparent_pen(fg_tmap, 0);
}

static VIDEO_UPDATE(quizpun2)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (input_code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (input_code_pressed(KEYCODE_Q))	msk |= 1;
		if (input_code_pressed(KEYCODE_W))	msk |= 2;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, bg_tmap,  TILEMAP_DRAW_OPAQUE, 0);
	else					bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, fg_tmap, 0, 0);

	return 0;
}


/***************************************************************************
                                Protection

    ROM checksum:   write 0x80 | (0x00-0x7f), write 0, read 2 bytes
    Read address:   write 0x80 | param1 & 0x07f (0x00), write param2 & 0x7f, read 2 bytes
    Read EEPROM:    write 0x20 | (0x00-0x0f), write 0, read 8 bytes
    Write EEPROM:   write 0x00 | (0x00-0x0f), write 0, write 8 bytes

***************************************************************************/

typedef enum { STATE_IDLE = 0, STATE_ADDR_R, STATE_ROM_R, STATE_EEPROM_R, STATE_EEPROM_W } prot_state;
static struct {
	prot_state state;
	int wait_param;
	int param;
	int cmd;
	int addr;
} prot;

static MACHINE_RESET( quizpun2 )
{
	prot.state = STATE_IDLE;
	prot.wait_param = 0;
	prot.param = 0;
	prot.cmd = 0;
	prot.addr = 0;
}

static void log_protection( const address_space *space, const char *warning )
{
	logerror("%04x: protection - %s (state %x, wait %x, param %02x, cmd %02x, addr %02x)\n", cpu_get_pc(space->cpu), warning,
		prot.state,
		prot.wait_param,
		prot.param,
		prot.cmd,
		prot.addr
	);
}

static READ8_HANDLER( quizpun2_protection_r )
{
	UINT8 ret;

	switch ( prot.state )
	{
		case STATE_ROM_R:		// Checksum of MCU addresses 0-ff (0x8e9c^0xffff expected)
			if      (prot.addr == 0xfe)	ret = 0x8e ^ 0xff;
			else if (prot.addr == 0xff)	ret = 0x9c ^ 0xff;
			else						ret = 0x00;
			break;

		case STATE_ADDR_R:		// Address to jump to (big endian!)
			switch ( prot.param )
			{
				case 0x19:	// Print
					ret = 0x0b95 >> ((prot.addr & 1) ? 0 : 8);
					break;

				case 0x44:	// Clear screen?
					ret = 0x1bd9 >> ((prot.addr & 1) ? 0 : 8);	// needed, but should also clear the screen
					break;

				case 0x45:	// Backup RAM check
					ret = 0x2242 >> ((prot.addr & 1) ? 0 : 8);
					break;

				default:
					log_protection(space, "unknown address");
					ret = 0x2e59 >> ((prot.addr & 1) ? 0 : 8);	// return the address of: XOR A, RET
			}
			break;

		case STATE_EEPROM_R:		// EEPROM read
		{
			UINT8 *eeprom = memory_region(space->machine, "eeprom");
			ret = eeprom[prot.addr];
			break;
		}

		default:
			log_protection(space, "unknown read");
			ret = 0x00;
	}

#if VERBOSE_PROTECTION_LOG
	log_protection(space, "info READ");
#endif

	prot.addr++;

	return ret;
}

static WRITE8_HANDLER( quizpun2_protection_w )
{
	switch ( prot.state )
	{
		case STATE_EEPROM_W:
		{
			UINT8 *eeprom = memory_region(space->machine, "eeprom");
			eeprom[prot.addr] = data;
			prot.addr++;
			if ((prot.addr % 8) == 0)
				prot.state = STATE_IDLE;
			break;
		}

		default:
			if (prot.wait_param)
			{
				prot.param = data;
				prot.wait_param = 0;

				// change state:

				if (prot.cmd & 0x80)
				{
					if (prot.param == 0x00)
					{
						prot.state = STATE_ROM_R;
						prot.addr = (prot.cmd & 0x7f) * 2;
					}
					else if (prot.cmd == 0x80)
					{
						prot.state = STATE_ADDR_R;
						prot.addr = 0;
					}
					else
						log_protection(space, "unknown command");
				}
				else if (prot.cmd >= 0x00 && prot.cmd <= 0x0f )
				{
					prot.state = STATE_EEPROM_W;
					prot.addr = (prot.cmd & 0x0f) * 8;
				}
				else if (prot.cmd >= 0x20 && prot.cmd <= 0x2f )
				{
					prot.state = STATE_EEPROM_R;
					prot.addr = (prot.cmd & 0x0f) * 8;
				}
				else
				{
					prot.state = STATE_IDLE;
					log_protection(space, "unknown command");
				}
			}
			else
			{
				prot.cmd = data;
				prot.wait_param = 1;
			}
			break;
	}

#if VERBOSE_PROTECTION_LOG
	log_protection(space, "info WRITE");
#endif
}


/***************************************************************************
                            Memory Maps - Main CPU
***************************************************************************/

static WRITE8_HANDLER( quizpun2_rombank_w )
{
	UINT8 *ROM = memory_region(space->machine, "maincpu");
	memory_set_bankptr(space->machine,  1, &ROM[ 0x10000 + 0x2000 * (data & 0x1f) ] );
}

static WRITE8_HANDLER( quizpun2_irq_ack )
{
	cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
}

static WRITE8_HANDLER( quizpun2_soundlatch_w )
{
	soundlatch_w(space, 0, data);
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( quizpun2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0x9fff ) AM_ROMBANK(1)

	AM_RANGE( 0xa000, 0xbfff ) AM_RAM_WRITE( fg_ram_w ) AM_BASE( &fg_ram )	// 4 * 800
	AM_RANGE( 0xc000, 0xc7ff ) AM_RAM_WRITE( bg_ram_w ) AM_BASE( &bg_ram )	// 4 * 400
	AM_RANGE( 0xc800, 0xcfff ) AM_RAM										//

	AM_RANGE( 0xd000, 0xd3ff ) AM_RAM_WRITE( paletteram_xRRRRRGGGGGBBBBB_le_w )  AM_BASE( &paletteram )
	AM_RANGE( 0xe000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizpun2_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x40, 0x40 ) AM_WRITE( quizpun2_irq_ack )
	AM_RANGE( 0x50, 0x50 ) AM_WRITE( quizpun2_soundlatch_w )
	AM_RANGE( 0x60, 0x60 ) AM_WRITE( quizpun2_rombank_w )
	AM_RANGE( 0x80, 0x80 ) AM_READ_PORT( "DSW" )
	AM_RANGE( 0x90, 0x90 ) AM_READ_PORT( "IN0" )
	AM_RANGE( 0xa0, 0xa0 ) AM_READ_PORT( "IN1" )
	AM_RANGE( 0xe0, 0xe0 ) AM_READWRITE( quizpun2_protection_r, quizpun2_protection_w )
ADDRESS_MAP_END


/***************************************************************************
                            Memory Maps - Sound CPU
***************************************************************************/

static ADDRESS_MAP_START( quizpun2_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0xf7ff ) AM_ROM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizpun2_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE( SMH_NOP )	// IRQ end
	AM_RANGE( 0x20, 0x20 ) AM_WRITE( SMH_NOP )	// NMI end
	AM_RANGE( 0x40, 0x40 ) AM_READ( soundlatch_r )
	AM_RANGE( 0x60, 0x61 ) AM_DEVREADWRITE("ym", ym2203_r, ym2203_w )
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( quizpun2 )
	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x30, "Play Time" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x16x8 =
{
	8, 16,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP16(0,8*8) },
	8*16*8
};

static const gfx_layout layout_8x16x2 =
{
	8, 16,
	RGN_FRAC(1, 1),
	2,
	{ 0,1 },
	{ STEP4(3*2,-2),STEP4(7*2,-2) },
	{ STEP16(0,8*2) },
	8*16*2
};

static GFXDECODE_START( quizpun2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x16x8, 0,  1*2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x16x2, 0, 64*2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x16x2, 0, 64*2 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_DRIVER_START( quizpun2 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2)	// 4 MHz?
	MDRV_CPU_PROGRAM_MAP(quizpun2_map)
	MDRV_CPU_IO_MAP(quizpun2_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)	// 4 MHz?
	MDRV_CPU_PROGRAM_MAP(quizpun2_sound_map)
	MDRV_CPU_IO_MAP(quizpun2_sound_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
	// NMI generated by main CPU

	MDRV_MACHINE_RESET( quizpun2 )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_GFXDECODE(quizpun2)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(quizpun2)
	MDRV_VIDEO_UPDATE(quizpun2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2203, XTAL_8MHz / 4 )	// 2 MHz?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( quizpun2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "u111", 0x00000, 0x08000, CRC(14bdaffc) SHA1(7fb5988ea565d7cbe3c8e2cdb9402d3cf81507d7) )
	ROM_LOAD( "u117", 0x10000, 0x10000, CRC(e9d1d05e) SHA1(c24104e023d12db8c9199d3e18750414aa511e40) )
	ROM_LOAD( "u118", 0x20000, 0x10000, CRC(1f232707) SHA1(3f5f44611f25c556521333f15daf3e2128cc1538) BAD_DUMP )	// fails rom check
	ROM_LOAD( "u119", 0x30000, 0x10000, CRC(c73b82f7) SHA1(d5c683440e9db46dd5859b519b3f32da80352626) )
	ROM_LOAD( "u120", 0x40000, 0x10000, CRC(700648b8) SHA1(dfa824166dfe7361d7c2ab0d8aa1ada882916cb9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u22", 0x00000, 0x10000, CRC(f40768b5) SHA1(4410f71850357ec1d10a3a114bb540966e72781b) )

	ROM_REGION( 0x40000, "gfx1", 0 )	// 8x16x8
    ROM_LOAD( "u21", 0x00000, 0x10000, CRC(8ac86759) SHA1(2eac9ceee4462ce905aa08ff4f5a6215e0b6672f) )
    ROM_LOAD( "u20", 0x10000, 0x10000, CRC(67640a46) SHA1(5b33850afbb89db9ce9044a578423bfe3a55420d) )
	ROM_LOAD( "u29", 0x20000, 0x10000, CRC(cd8ff05b) SHA1(25e5be914fe49ff96a3c04de0c0e266a79068930) )
    ROM_LOAD( "u30", 0x30000, 0x10000, CRC(8612b443) SHA1(1033a378b21023eca471f43309d49461494b5ea1) )

	ROM_REGION( 0x6000, "gfx2", 0 )	// 8x16x2
	ROM_LOAD( "u26", 0x1000, 0x1000, CRC(151de8af) SHA1(2159ab030043e69d63cc9fbbc772f5bae8ab3f9d) )
	ROM_CONTINUE(    0x0000, 0x1000 )
	ROM_LOAD( "u27", 0x3000, 0x1000, CRC(2afdafea) SHA1(4c116a1e8a91f2e309646063139763b837e24bc7) )
	ROM_CONTINUE(    0x2000, 0x1000 )
	ROM_LOAD( "u28", 0x5000, 0x1000, CRC(c8bd85ad) SHA1(e7f0882f669edea1bb4634c263872f63da6a3290) )
	ROM_CONTINUE(    0x4000, 0x1000 )

	ROM_REGION( 0x20000, "gfx3", 0 )	// 8x16x2
	ROM_LOAD( "u1", 0x00000, 0x10000, CRC(58506040) SHA1(9d8bed2585e8f188a20270fccd9cfbdb91e48599) )
	ROM_LOAD( "u2", 0x10000, 0x10000, CRC(9294a19c) SHA1(cd7109262e5f68b946c84aa390108bcc47ee1300) )

	ROM_REGION( 0x80, "eeprom", 0 )	// EEPROM (tied to the unknown DIP40)
	ROM_LOAD( "93c46", 0x00, 0x80, CRC(4d244cc8) SHA1(6593d5b7ac1ebb77fee4648ad1d3d9b59a25fdc8) )

	ROM_REGION( 0x2000, "unknown", 0 )
	ROM_LOAD( "u2a", 0x0000, 0x2000, CRC(13afc2bd) SHA1(0d9c8813525dfc7a844e72d2cf84261db3d10a23) )
ROM_END

GAME( 1989, quizpun2, 0, quizpun2, quizpun2, 0, ROT270, "Space Computer", "Quiz Punch 2", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION )
