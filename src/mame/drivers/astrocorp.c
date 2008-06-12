/***************************************************************************

                      -= Astro Corp. Hardware =-

                driver by   Luca Elia (l.elia@tin.it)

CPU:    68000
GFX:    ASTRO V01 0005 (512 sprites, each made of N x M tiles. Tiles are 16x16x8)
SOUND:  OKI M6295 (AD-65)
OTHER:  ASTRO 0001B, EEPROM

***************************************************************************/

#include "driver.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"

/***************************************************************************
                              Sprites Format

    Offset:    Bits:                  Value:

        0.w    f--- ---- ---- ----    Show This Sprite
               -edc ba98 7654 3210    X

        1.w                           Code

        2.w    f--- ---- ---- ----    -
               -edc ba98 7654 3210    Y

        3.w    fedc ba98 ---- ----    X Size
               ---- ---- 7654 3210    Y Size

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	UINT16 *source = spriteram16;
	UINT16 *finish = spriteram16 + spriteram_size/2;

	for ( ; source < finish; source += 8/2 )
	{
		int x,y;

		int	sx		=	source[ 0x0/2 ];
		int	code	=	source[ 0x2/2 ];
		int	sy		=	source[ 0x4/2 ];
		int	attr	=	source[ 0x6/2 ];

		int dimx	=	(attr >> 8) & 0xff;
		int dimy	=	(attr >> 0) & 0xff;

		if (!dimx && !dimy)
			return;

		if (!(sx & 0x8000))
			continue;

		sx = (sx & 0x3fff) - (sx & 0x4000);
		sy = (sy & 0x3fff) - (sy & 0x4000);

		for (y = 0 ; y < dimy ; y++)
		{
			for (x = 0 ; x < dimx ; x++)
			{
				drawgfx(bitmap,machine->gfx[0],
						code++, 0,
						0, 0,
						sx + x * 16, sy + y * 16,
						cliprect, TRANSPARENCY_PEN, 0xff);
			}
		}
	}
}

static UINT16 astrocorp_screen_enable;

static VIDEO_UPDATE(astrocorp)
{
	if (astrocorp_screen_enable & 1)
	{
		fillbitmap(bitmap,screen->machine->pens[0xff],cliprect);
		draw_sprites(screen->machine,bitmap,cliprect);
	}
	else
		fillbitmap(bitmap,get_black_pen(screen->machine),cliprect);

	return 0;
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

static READ16_HANDLER( astrocorp_eeprom_r )
{
	return 0xfff7 | (eeprom_read_bit() << 3);
}

static WRITE16_HANDLER( astrocorp_eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// latch the bit
		eeprom_write_bit(data & 0x01);

		// reset line asserted: reset.
		eeprom_set_cs_line((data & 0x04) ? CLEAR_LINE : ASSERT_LINE );

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
	}
}

static WRITE16_HANDLER( astrocorp_sound_bank_w )
{
	if (ACCESSING_BITS_8_15)
	{
		OKIM6295_set_bank_base(0, 0x40000 * ((data >> 8) & 1) );
//      logerror("CPU #0 PC %06X: OKI bank %08X\n",activecpu_get_pc(),data);
	}
}

static WRITE16_HANDLER( astrocorp_outputs_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(0,	(data & 0x0004));	// coin counter
		set_led_status(0,	(data & 0x0008));	// you win
		if (				(data & 0x0010))	dispensed_tickets++;	// coin out
		set_led_status(1,	(data & 0x0020));	// coin/hopper jam
	}
	if (ACCESSING_BITS_8_15)
	{
		set_led_status(2,	(data & 0x0100));	// bet
		set_led_status(3,	(data & 0x0800));	// start
		set_led_status(4,	(data & 0x1000));	// ? select/choose
		set_led_status(5,	(data & 0x2000));	// ? select/choose
		set_led_status(6,	(data & 0x4000));	// look
	}
//  popmessage("%04X",data);
}

static WRITE16_HANDLER( astrocorp_enable_w )
{
	COMBINE_DATA( &astrocorp_screen_enable );
//  popmessage("%04X",data);
	if (data & (~1))
		logerror("CPU #0 PC %06X: screen enable = %04X\n",activecpu_get_pc(),data);
}

static READ16_HANDLER( astrocorp_unk_r )
{
	return 0xffff;	// bit 3?
}

// 5-6-5 Palette: BBBBB-GGGGGG-RRRRR
static WRITE16_HANDLER( astrocorp_palette_w )
{
	COMBINE_DATA( &paletteram16[offset] );
	palette_set_color_rgb( machine, offset,
		pal5bit((paletteram16[offset] >>  0) & 0x1f),
		pal6bit((paletteram16[offset] >>  5) & 0x3f),
		pal5bit((paletteram16[offset] >> 11) & 0x1f)
	);
}

static ADDRESS_MAP_START( astrocorp_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x050000, 0x050fff ) AM_RAM AM_BASE( &spriteram16 ) AM_SIZE( &spriteram_size )
	AM_RANGE( 0x052000, 0x052001 ) AM_WRITE( SMH_NOP )
	AM_RANGE( 0x054000, 0x054001 ) AM_READ_PORT( "INPUTS" )
	AM_RANGE( 0x058000, 0x058001 ) AM_WRITE( astrocorp_eeprom_w )
	AM_RANGE( 0x05a000, 0x05a001 ) AM_WRITE( astrocorp_outputs_w )
	AM_RANGE( 0x05e000, 0x05e001 ) AM_READ( astrocorp_eeprom_r )
	AM_RANGE( 0x060000, 0x0601ff ) AM_RAM_WRITE( astrocorp_palette_w ) AM_BASE( &paletteram16 )
	AM_RANGE( 0x070000, 0x073fff ) AM_RAM
	AM_RANGE( 0x080000, 0x080001 ) AM_WRITE( astrocorp_sound_bank_w )
	AM_RANGE( 0x0a0000, 0x0a0001 ) AM_WRITE( astrocorp_enable_w )
	AM_RANGE( 0x0d0000, 0x0d0001 ) AM_READWRITE( astrocorp_unk_r, OKIM6295_data_0_msb_w )
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( showhand )
	PORT_START_TAG("INPUTS")	// 54000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1     )	PORT_IMPULSE(1)	// coin
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON3   )	PORT_NAME("Payout")	PORT_CODE(KEYCODE_F1) // payout (must be 0 on startup)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// ?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START2    )	PORT_NAME("Bet / Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON3   )	PORT_NAME("Look / Small")
	PORT_SERVICE_NO_TOGGLE( 0x0020,   IP_ACTIVE_LOW )	// settings
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// ?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_SPECIAL   )	// coin sensor
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_BUTTON2   )	PORT_NAME("Yes / Big")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// hold1?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_BUTTON1   )	PORT_NAME("Select")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_START1    )	PORT_NAME("Start / Take")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SERVICE1  )	PORT_NAME("Reset Settings")	// when 1 in test mode: reset settings (must be 0 on startup)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN   )	// ?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN2     )	// key in
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SPECIAL   )	// coin sensor
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_16x16x8 =
{
	16, 16,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( astrocorp )
	{ REGION_GFX1, 0, &layout_16x16x8, 0, 1 },
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static const UINT16 showhand_default_eeprom[] =	{0x0001,0x0007,0x000a,0x0003,0x0000,0x0009,0x0003,0x0000,0x0002,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000};

static NVRAM_HANDLER( showhand )
{
	if (read_or_write)
		eeprom_save(file);
	else
	{
		eeprom_init(&eeprom_interface_93C46);

		if (file) eeprom_load(file);
		else
		{
			/* Set the EEPROM to Factory Defaults */
			eeprom_set_data((UINT8*)showhand_default_eeprom,sizeof(showhand_default_eeprom));
		}
	}
}

static MACHINE_DRIVER_START( astrocorp )
	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, XTAL_25_601712MHz / 2)
	MDRV_CPU_PROGRAM_MAP(astrocorp_map,0)
	MDRV_CPU_VBLANK_INT("main", irq4_line_hold)

	MDRV_NVRAM_HANDLER(showhand)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(astrocorp)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_UPDATE(astrocorp)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, XTAL_20MHz/16)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7low)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Show Hand
(C) 1999? Astro Corp.

PCB CHE-B50-4002A 88 94V-0 0002

CPU     1x MC68HC000FN12 (main)
        1x pLSI1016-60LJ (main)
        1x ASTRO V01 0005 (custom)
        1x AD-65 (equivalent to OKI6295)(sound)
        1x ASTRO 0001B (custom)
        1x oscillator 20.000MHz
        1x oscillator 25.601712MHz

ROMs    2x 27C512 (1,2)
        2x M27C801 (3,4)
        1x M27C4001 (5)
        1x 93C46 (not dumped)

Note    1x 28x2 JAMMA edge connector
        1x 18x2 edge connector
        1x 10x2 edge connector
        1x pushbutton
        1x trimmer (volume)
        1x 2x2 switches dip

Hardware info by f205v

***************************************************************************/

ROM_START( showhand )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "1-8.even.u16", 0x00000, 0x10000, CRC(cf34bf0d) SHA1(72ad7ca63ef89451b2572d64cccfa764b9d9b353) )
	ROM_LOAD16_BYTE( "2-8.odd.u17",  0x00001, 0x10000, CRC(dd031c36) SHA1(198d0e685dd2d824a04c787f8a17c173efa272d9) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "4.even.u26", 0x000000, 0x100000, CRC(8a706e42) SHA1(989688ee3a5e4fc11fb502e43c9d6012488982ee) )
	ROM_LOAD16_BYTE( "3.odd.u26",  0x000001, 0x100000, CRC(a624b750) SHA1(fc5b09f8a10cba5fb2474e1edd62a0400177a5ad) )

	ROM_REGION( 0x80000, REGION_SOUND1, 0 )
	ROM_LOAD( "5", 0x00000, 0x80000, CRC(e6987122) SHA1(fb3e7c2399057c64b5c496a393f6f22a1e54c844) )
ROM_END

static DRIVER_INIT( showhand )
{
/*
    UINT16 *rom = (UINT16*)memory_region(REGION_CPU1);

    rom[0x0a1a/2] = 0x6000; // hopper jam

    rom[0x1494/2] = 0x4e71; // enable full test mode

    rom[0x12f6/2] = 0x6000; // rom error
    rom[0x4916/2] = 0x6000; // rom error
*/
}

GAME( 1999?, showhand, 0, astrocorp, showhand, showhand, ROT0, "Astro Corp.", "Show Hand", 0 )
