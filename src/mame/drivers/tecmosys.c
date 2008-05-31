/* Tecmo System
 Driver by Farfetch & David Haywood

can't do anything with this, its protected and expects to read back 68k code :-(

T.Slanina 20040530 :
 - preliminary gfx decode,
 - Angel Eyes - patched interrupt level1 vector
 - EEPROM r/w
 - txt layer
 - added hacks to see more gfx (press Z or X)
 - palette (press X in angel eyes to see 'color bar chack'(!))
 - watchdog (?) simulation

 20080528
 - Removed ROM patches and debug keypresses
 - Added protection simulation in machine/tecmosys.c
 - Fixed inputs
 - Added watchdog

   To enter test mode, you have to press the test switch before you insert any coins.

*/


/*

Deroon Dero Dero
Tecmo, 1996

This game is a Puyo Puyo rip-off.

PCB Layout
----------

TECMO SYSTEM BOARD A
|-------------------------------------------------------------------------|
|  LM324  UPC452C      16.9MHz        |--------|    |--------|  6264  |
| TA8205 LM324  YAC513  YMF262 YMZ280B  |TECMO  |    |TECMO  |  6264  |
|        LM324  M6295  UPC452C          |AA03-8431    |AA02-1927        |
|                      YAC512          |        |    |        |        |
|                                        |--------|    |--------|        |
|        Z80  6264 28MHz 14.31818MHz                  |--------|        |
|                    16MHz              62256          |TECMO  |        |
|            TA8030                    62256          |AA02-1927  6264  |
|                                                      |        |  6264  |
|J  93C46                              |--------|    |--------|        |
|A                                      |TECMO  |    |--------|        |
|M                                      |AA03-8431    |TECMO  |        |
|M          68000                        |        |    |AA02-1927        |
|A                                      |--------|    |        |  6264  |
|                  PAL              6116              |--------|  6264  |
|                                    6116              |--------|        |
|  |--------|                                          |TECMO  |        |
|  |TECMO  |                      PAL                |AA02-1927        |
|  |AA03-8431  62256                                  |        |  6264  |
|  |        |  62256                                  |--------|  6264  |
|  |--------|                                |---------|                |
|                                              |TECMO    |                |
|                                              |AA03-8431|                |
|                                              |        |                |
|                                              |---------|          424260|
|                                              62256 62256          424260|
|-------------------------------------------------------------------------|
Notes:
68000 @ 16MHz
Z80 @ 8MHz [16/2]
YMZ280B @ 16.9MHz
YMF262 @ 14.31818MHz
OKI M6295 @ 2MHz [16/8]. Pin 7 HIGH

Game Board
----------

TECMO SYSTEM BOARD B2
|-------------------------------------------------------------------------|
|    T201_DIP42_MASK.UBB1                                                |
| |----|                                              T202_DIP42_MASK.UBC1|
| |*  |                                                                  |
| |----|                                                                  |
|                                                                        |
|  T003_2M_EPROM.UZ1                        T101_SOP44.UAH1            |
|                                                                        |
|                                            T301_DIP42_MASK.UBD1        |
|                                                                        |
|                                                                        |
|                                                                        |
|  T401_DIP42_MASK.UYA1      T104_SOP44.UCL1    T001_4M_EPROM.UPAU1    |
|                              T103_SOP44.UBL1                            |
|  T501_DIP32_MASK.UAD1      T102_SOP44.UAL1                            |
|                                                  T002_4M_EPROM.UPAL1    |
|-------------------------------------------------------------------------|
Notes:
      * - Unknown QFP64 microcontroller marked 'TECMO SC432146FU E23D 185 SSAB9540B'
          Clocks: pin 33 - 8MHz, pin 31: 8MHz, pin 29 - 2MHz
          GND on pins 49, 23, 24, 27
          Power on pins 55, 25
          Note - Pins 25 and 27 are tied to some jumpers, so these
          appear to be some kind of configuration setting.

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

EPROMs:
t001upau.bin - Main program (even) (27c4001)
t002upal.bin - Main program (odd)  (27c4001)

t003uz1.bin - Sound program (27c2001)

Mask ROMs:
t101uah1.j66 - Graphics (23c16000 SOP)
t102ual1.j67 |
t103ubl1.j08 |
t104ucl1.j68 /

t201ubb1.w61 - Graphics (23c8000)
t202ubc1.w62 /

t301ubd1.w63 - Graphics (23c8000)

t401uya1.w16 - YMZ280B Samples (23c16000)

t501uad1.w01 - M6295 Samples (23c4001)

*/

/*

Touki Denshou -Angel Eyes-
(c)1996 Tecmo
Tecmo System Board

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

EPROMs:
aeprge-2.pal - Main program (even) (27c4001)
aeprgo-2.pau - Main program (odd)  (27c4001)

aesprg-2.z1 - Sound program (27c1001)

Mask ROMs:
ae100h.ah1 - Graphics (23c32000/16000 SOP)
ae100.al1  |
ae101h.bh1 |
ae101.bl1  |
ae102h.ch1 |
ae102.cl1  |
ae104.el1  |
ae105.fl1  |
ae106.gl1  /

ae200w74.ba1 - Graphics (23c16000)
ae201w75.bb1 |
ae202w76.bc1 /

ae300w36.bd1 - Graphics (23c4000)

ae400t23.ya1 - YMZ280B Samples (23c16000)
ae401t24.yb1 /

ae500w07.ad1 - M6295 Samples (23c4001)

*/

#include "driver.h"
#include "machine/eeprom.h"
#include "tecmosys.h"
#include "cpu/m68000/m68k.h"
#include "sound/okim6295.h"
#include "sound/262intf.h"
#include "sound/ymz280b.h"
#include "deprecat.h"

static UINT16* tecmosys_spriteram;
static UINT16* tilemap_paletteram16;
static UINT16* bg2tilemap_ram;
static UINT16* bg1tilemap_ram;
static UINT16* bg0tilemap_ram;
static UINT16* fgtilemap_ram;


static MACHINE_RESET( deroon );

static tilemap *bg0tilemap;
static TILE_GET_INFO( get_bg0tile_info )
{

	SET_TILE_INFO(
			1,
			bg0tilemap_ram[2*tile_index+1],
			(bg0tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((bg0tilemap_ram[2*tile_index]&0xc0)>>6));
}

static tilemap *bg1tilemap;
static TILE_GET_INFO( get_bg1tile_info )
{

	SET_TILE_INFO(
			2,
			bg1tilemap_ram[2*tile_index+1],
			(bg1tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((bg1tilemap_ram[2*tile_index]&0xc0)>>6));
}

static tilemap *bg2tilemap;
static TILE_GET_INFO( get_bg2tile_info )
{

	SET_TILE_INFO(
			3,
			bg2tilemap_ram[2*tile_index+1],
			(bg2tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((bg2tilemap_ram[2*tile_index]&0xc0)>>6));
}


static tilemap *txt_tilemap;
static TILE_GET_INFO( get_tile_info )
{

	SET_TILE_INFO(
			0,
			fgtilemap_ram[2*tile_index+1],
			(fgtilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((fgtilemap_ram[2*tile_index]&0xc0)>>6));
}


// It looks like this needs a synch between z80 and 68k ??? See z80:006A-0091
static READ16_HANDLER( sound_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return soundlatch2_r( machine,  0 );
	}

	return 0;
}

static WRITE16_HANDLER( sound_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(machine,0x00,data & 0xff);
		cpunum_set_input_line(machine, 1,INPUT_LINE_NMI,PULSE_LINE);
	}
}

/*
	880000 and 880002 might be video related,
	see sub @ 68k:002e5e where they are written if the screen is set to inverted.
	Also, irq code at 22c4 :
	- 880000 & 00, execute irq code
	- 880000 & 01, scroll?
	- 880000 & 03, crash
*/
static WRITE16_HANDLER( unk880000_w )
{
	switch( offset )
	{
		case 0x22/2:
			watchdog_reset( machine );
			break;

		default:
			//logerror( "unk880000_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, activecpu_get_pc() );
			break;
	}
}

static READ16_HANDLER( unk880000_r )
{
	//logerror( "unk880000_r( %06x ) @ %06x\n", (offset * 2 ) +0x880000, activecpu_get_pc() );
	return 0;
}

static READ16_HANDLER( eeprom_r )
{
	 return ((eeprom_read_bit() & 0x01) << 11);
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(SMH_ROM)
	AM_RANGE(0x200000, 0x20ffff) AM_READ(SMH_RAM) // work ram
	AM_RANGE(0x210000, 0x210001) AM_READ(SMH_NOP) // single byte overflow on stack defined as 0x210000
	AM_RANGE(0x300000, 0x3013ff) AM_READ(SMH_RAM) // bg0 ram
	AM_RANGE(0x400000, 0x4013ff) AM_READ(SMH_RAM) // bg1 ram
	AM_RANGE(0x500000, 0x5013ff) AM_READ(SMH_RAM) // bg2 ram
	AM_RANGE(0x700000, 0x703fff) AM_READ(SMH_RAM) // fix ram   (all these names from test screen)
	AM_RANGE(0x800000, 0x80ffff) AM_READ(SMH_RAM) // obj ram
	AM_RANGE(0x880000, 0x88000b) AM_READ(unk880000_r)
	AM_RANGE(0x900000, 0x907fff) AM_READ(SMH_RAM) // obj pal
	AM_RANGE(0x980000, 0x9807ff) AM_READ(SMH_RAM) // bg pal
	AM_RANGE(0x980800, 0x980fff) AM_READ(SMH_RAM) // fix pal
	AM_RANGE(0xb80000, 0xb80001) AM_READ(prot_status_r)
	AM_RANGE(0xd00000, 0xd00001) AM_READ(input_port_0_word_r)
	AM_RANGE(0xd00002, 0xd00003) AM_READ(input_port_1_word_r)
	AM_RANGE(0xd80000, 0xd80001) AM_READ(eeprom_r)
	AM_RANGE(0xf00000, 0xf00001) AM_READ( sound_r )
	AM_RANGE(0xf80000, 0xf80001) AM_READ(prot_data_r)
ADDRESS_MAP_END

static WRITE16_HANDLER( eeprom_w )
{
	if ( ACCESSING_BITS_8_15 )
	{
		eeprom_write_bit(data & 0x0800);
		eeprom_set_cs_line((data & 0x0200) ? CLEAR_LINE : ASSERT_LINE );
		eeprom_set_clock_line((data & 0x0400) ? CLEAR_LINE: ASSERT_LINE );
	}
}


INLINE void set_color_555(pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(Machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}


WRITE16_HANDLER( tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w )
{
	COMBINE_DATA(&tilemap_paletteram16[offset]);
	set_color_555(offset+0x4000, 5, 10, 0, tilemap_paletteram16[offset]);
}


static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x200000, 0x20ffff) AM_WRITE(SMH_RAM) // work ram
	AM_RANGE(0x300000, 0x3013ff) AM_WRITE(SMH_RAM) AM_BASE(&bg0tilemap_ram) // bg0 ram
	AM_RANGE(0x400000, 0x4013ff) AM_WRITE(SMH_RAM) AM_BASE(&bg1tilemap_ram) // bg1 ram
	AM_RANGE(0x500000, 0x5013ff) AM_WRITE(SMH_RAM) AM_BASE(&bg2tilemap_ram) // bg2 ram
	AM_RANGE(0x700000, 0x703fff) AM_WRITE(SMH_RAM) AM_BASE(&fgtilemap_ram) // fix ram
	AM_RANGE(0x800000, 0x80ffff) AM_WRITE(SMH_RAM) AM_BASE(&tecmosys_spriteram) // obj ram
	AM_RANGE(0x900000, 0x907fff) AM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE(&paletteram16) // AM_WRITE(SMH_RAM) // obj pal

	//AM_RANGE(0x980000, 0x9807ff) AM_WRITE(SMH_RAM) // bg pal
	//AM_RANGE(0x980800, 0x980fff) AM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE(&paletteram16) // fix pal
	// the two above are as tested by the game code, I've only rolled them into one below to get colours to show right.
	AM_RANGE(0x980000, 0x980fff) AM_WRITE(tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE(&tilemap_paletteram16)

	AM_RANGE(0x880000, 0x88002f) AM_WRITE( unk880000_w )	// 10 byte dta@88000c, 880022=watchdog?
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(eeprom_w	)
	AM_RANGE(0xa80000, 0xa80005) AM_WRITE(SMH_RAM	)	// a80000-3 scroll? a80004 inverted ? 3 : 0
	AM_RANGE(0xb00000, 0xb00005) AM_WRITE(SMH_RAM	)	// b00000-3 scrool?, b00004 inverted ? 3 : 0
	AM_RANGE(0xb80000, 0xb80001) AM_WRITE(prot_status_w)
	AM_RANGE(0xc00000, 0xc00005) AM_WRITE(SMH_RAM	)	// c00000-3 scroll? c00004 inverted ? 13 : 10
	AM_RANGE(0xc80000, 0xc80005) AM_WRITE(SMH_RAM	)	// c80000-3 scrool? c80004 inverted ? 3 : 0
	AM_RANGE(0xe00000, 0xe00001) AM_WRITE( sound_w )
	AM_RANGE(0xe80000, 0xe80001) AM_WRITE(prot_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( deroon )
	PORT_START
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )		PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )			PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )			PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )			PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) 		PORT_PLAYER(1)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )		PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )			PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )			PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )			PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )			PORT_PLAYER(2)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout gfxlayout =
{
   8,8,
   RGN_FRAC(1,1),
   4,
   { 0,1,2,3 },
   { 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
   { 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
   8*8*4
};

static const gfx_layout gfxlayout2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
	  8*8*4*1+0*4, 8*8*4*1+1*4, 8*8*4*1+2*4, 8*8*4*1+3*4, 8*8*4*1+4*4, 8*8*4*1+5*4,8*8*4*1+6*4, 8*8*4*1+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  8*8*4*2+0*32, 8*8*4*2+1*32, 8*8*4*2+2*32, 8*8*4*2+3*32, 8*8*4*2+4*32, 8*8*4*2+5*32, 8*8*4*2+6*32, 8*8*4*2+7*32 },
	128*8
};




static GFXDECODE_START( tecmosys )
	GFXDECODE_ENTRY( REGION_GFX2, 0, gfxlayout,   0x4400, 0x40 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, gfxlayout2,  0x4000, 0x40 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, gfxlayout2,  0x4000, 0x40 )
	GFXDECODE_ENTRY( REGION_GFX5, 0, gfxlayout2,  0x4000, 0x40 )

GFXDECODE_END

static WRITE8_HANDLER( deroon_bankswitch_w )
{
	memory_set_bankptr( 1, memory_region(REGION_CPU2) + ((data-2) & 0x0f) * 0x4000 + 0x10000 );
}

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK1)
	AM_RANGE(0xe000, 0xf7ff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xe000, 0xf7ff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(YMF262_status_0_r)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_r)
	AM_RANGE(0x60, 0x60) AM_READ(YMZ280B_status_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(YMF262_register_A_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YMF262_data_A_0_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(YMF262_register_B_0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(YMF262_data_B_0_w)

	AM_RANGE(0x10, 0x10) AM_WRITE(OKIM6295_data_0_w)
	AM_RANGE(0x20, 0x20) AM_NOP

	AM_RANGE(0x30, 0x30) AM_WRITE(deroon_bankswitch_w)

	AM_RANGE(0x50, 0x50) AM_WRITE(soundlatch2_w)

	AM_RANGE(0x60, 0x60) AM_WRITE(YMZ280B_register_0_w)
	AM_RANGE(0x61, 0x61) AM_WRITE(YMZ280B_data_0_w)
ADDRESS_MAP_END

static VIDEO_START(deroon)
{
	txt_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,8,8,32*2,32*2);
	tilemap_set_transparent_pen(txt_tilemap,0);

//	bg0tilemap = tilemap_create(get_bg0tile_info,tilemap_scan_rows,16,16,32,40);
	bg0tilemap = tilemap_create(get_bg0tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(bg0tilemap,0);

//	bg1tilemap = tilemap_create(get_bg1tile_info,tilemap_scan_rows,16,16,32,40);
	bg1tilemap = tilemap_create(get_bg1tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(bg1tilemap,0);

//	bg2tilemap = tilemap_create(get_bg2tile_info,tilemap_scan_rows,16,16,32,40);
	bg2tilemap = tilemap_create(get_bg2tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(bg2tilemap,0);

}

static VIDEO_UPDATE(deroon)
{
	int i;
	//const gfx_element *gfx = Machine->gfx[0];
	UINT8 *gfxsrc    = memory_region       ( REGION_GFX1 );


	fillbitmap(bitmap,0x000,cliprect);


	tilemap_mark_all_tiles_dirty(bg1tilemap);
	tilemap_draw(bitmap,cliprect,bg1tilemap,0,0);

	tilemap_mark_all_tiles_dirty(bg0tilemap);
	tilemap_draw(bitmap,cliprect,bg0tilemap,0,0);



	for (i=0;i<0x10000/2;i+=8)
	{
		int xcnt,ycnt;
		int drawx, drawy;
		UINT16* dstptr;

		int x, y;
		int address;
		int xsize = 16;
		int ysize = 16;
		int colour;

		x = tecmosys_spriteram[i+0] & 0x3ff;
		y = tecmosys_spriteram[i+1] & 0x3ff;

	//	if (x&0x200) x-=0x400;
	//	if (y&0x200) y-=0x400;

		address =  tecmosys_spriteram[i+5]| ((tecmosys_spriteram[i+4]&0x000f)<<16);

		address<<=8;
		y -= 128;
 		x -= 96;

		//xsize = (tecmosys_spriteram[i+2] & 0x0ff0)>>4; // zoom?
		//ysize = (tecmosys_spriteram[i+3] & 0x0ff0)>>4; // zoom?

		ysize =  ((tecmosys_spriteram[i+6] & 0x00ff))*16;
		xsize =  (((tecmosys_spriteram[i+6] & 0xff00)>>8))*16;

		colour =  ((tecmosys_spriteram[i+4] & 0x3f00))>>8;



		if (tecmosys_spriteram[i+4] & 0x8000) continue;

		for (ycnt = 0; ycnt < ysize; ycnt++)
		{
			drawy = y + ycnt;


			for (xcnt = 0; xcnt < xsize; xcnt++)
			{
				drawx = x + xcnt;

				if ((drawx>=0 && drawx<320) && (drawy>=0 && drawy<240))
				{
					UINT8 data;

					dstptr = BITMAP_ADDR16(bitmap, drawy, drawx);


					data =  (gfxsrc[address]);


					if(data) dstptr[0] = data + (colour*0x100);
				}



				address++;

			}
		}

	}

	tilemap_mark_all_tiles_dirty(bg2tilemap);
	tilemap_draw(bitmap,cliprect,bg2tilemap,0,0);


	tilemap_mark_all_tiles_dirty(txt_tilemap);
	tilemap_draw(bitmap,cliprect,txt_tilemap,0,0);

	return 0;
}

/*
>>> R.Belmont wrote:
> Here's the sound info (I got it playing in M1, I
> didn't bother "porting" it since the main game doesn't
> even boot).
>
> memory map:
> 0000-7fff: fixed program ROM
> 8000-bfff: banked ROM
> e000-f7ff: work RAM
>
> I/O ports:

> 0-3: YMF262 OPL3
> 0x10: OKIM6295
> 0x30: bank select, in 0x4000 byte units based at the
> start of the ROM (so 2 = 0x8000).
> 0x40: latch from 68000
> 0x50: latch to 68000
> 0x60/0x61: YMZ280B
>
> IRQ from YMF262 goes to Z80 IRQ.
>
> NMI is asserted when the 68000 writes a command.
>
> Z80 clock appears to be 8 MHz (music slows down in
> "intense" sections if it's 4 MHz, and the crystals are
> all in the area of 16 MHz).
>
> The YMZ280B samples for both games may be misdumped,
> deroon has lots of "bad" noises but tkdensho only has
> a few.
*/


static void sound_irq(running_machine *machine, int irq)
{
	/* IRQ */
	cpunum_set_input_line(machine, 1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const struct YMF262interface ymf262_interface =
{
	sound_irq		/* irq */
};


static const struct YMZ280Binterface ymz280b_interface =
{
	REGION_SOUND1,
	0	/* irq */
};

static MACHINE_DRIVER_START( deroon )
	MDRV_CPU_ADD(M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)
	MDRV_WATCHDOG_VBLANK_INIT(5) // guess

	/* audio CPU */
	MDRV_CPU_ADD(Z80, 16000000/2 )	/* 8 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(readport,writeport)

	MDRV_GFXDECODE(tecmosys)

	MDRV_NVRAM_HANDLER(93C46)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x4000+0x800)

	MDRV_VIDEO_START(deroon)
	MDRV_MACHINE_RESET(deroon)
	MDRV_VIDEO_UPDATE(deroon)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YMF262, 14318180)
	MDRV_SOUND_CONFIG(ymf262_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
	MDRV_SOUND_ROUTE(2, "left", 1.0)
	MDRV_SOUND_ROUTE(3, "right", 1.0)

	MDRV_SOUND_ADD(OKIM6295, 14318180/2048*132)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD(YMZ280B, 16900000)
	MDRV_SOUND_CONFIG(ymz280b_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.30)
	MDRV_SOUND_ROUTE(1, "right", 0.30)
MACHINE_DRIVER_END

ROM_START( deroon )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) // Main Program
	ROM_LOAD16_BYTE( "t001upau.bin", 0x00000, 0x80000, CRC(14b92c18) SHA1(b47b8c828222a3f7c0fe9271899bd38171d972fb) )
	ROM_LOAD16_BYTE( "t002upal.bin", 0x00001, 0x80000, CRC(0fb05c68) SHA1(5140592e15414770fb46d5ac9ba8f76e3d4ab323) )

	ROM_REGION( 0x048000, REGION_CPU2, 0 ) // Sound Porgram
	ROM_LOAD( "t003uz1.bin", 0x000000, 0x008000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )
	ROM_CONTINUE(            0x010000, 0x038000 ) /* banked part */

	ROM_REGION( 0x2000000, REGION_GFX1, ROMREGION_ERASE00 ) // Sprites (non-tile based)
	/* all these roms need verifying, they could be half size */

	ROM_LOAD16_BYTE( "t101uah1.j66", 0x0000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD16_BYTE( "t102ual1.j67", 0x0000001, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )

	ROM_LOAD16_BYTE( "t103ubl1.j08", 0x0800001, 0x200000, BAD_DUMP CRC(75431ec5) SHA1(c03e724c15e1fe7a0a385332f849e9ac9d149887) ) // half size?
	/* game attempts to draw sprites from 0xc00000 region on screen after you press start */

	ROM_LOAD16_BYTE( "t104ucl1.j68", 0x1000001, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, REGION_GFX2, ROMREGION_DISPOSE ) // 8x8 4bpp tiles
	ROM_LOAD( "t301ubd1.w63", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_ERASE00) // 16x16 4bpp tiles
	/* not used? */

	ROM_REGION( 0x100000, REGION_GFX4, ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t201ubb1.w61", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )

	ROM_REGION( 0x100000, REGION_GFX5, ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t202ubc1.w62", 0x000000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 ) // YMZ280B Samples
	ROM_LOAD( "t401uya1.w16", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x080000, REGION_SOUND2, 0 ) // M6295 Samples
	ROM_LOAD( "t501uad1.w01", 0x000000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

ROM_START( tkdensho )
	ROM_REGION( 0x600000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "aeprge-2.pal", 0x00000, 0x80000, CRC(25e453d6) SHA1(9c84e2af42eff5cc9b14c1759d5bab42fa7bb663) )
	ROM_LOAD16_BYTE( "aeprgo-2.pau", 0x00001, 0x80000, CRC(22d59510) SHA1(5ade482d6ab9a22df2ee8337458c22cfa9045c73) )

	ROM_REGION( 0x038000, REGION_CPU2, 0 ) // Sound Porgram
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x008000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_CONTINUE(            0x010000, 0x018000 ) /* banked part */

	ROM_REGION( 0x4000000, REGION_GFX1, ROMREGION_ERASE00 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD16_BYTE( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD16_BYTE( "ae100.al1",     0x0000001, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD16_BYTE( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD16_BYTE( "ae101.bl1",     0x0800001, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD16_BYTE( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD16_BYTE( "ae102.cl1",     0x1000001, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD16_BYTE( "ae104.el1",     0x2000001, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD16_BYTE( "ae105.fl1",     0x2800001, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD16_BYTE( "ae106.gl1",     0x3000001, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, REGION_GFX2, ROMREGION_DISPOSE ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, REGION_GFX4, ROMREGION_DISPOSE ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, REGION_GFX5, ROMREGION_DISPOSE ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x080000, REGION_SOUND2, 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x000000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

static MACHINE_RESET( deroon )
{
	device_read_ptr = 0;
	device_status = DS_IDLE;
}

void tecmosys_decramble(void)
{
	UINT8 *gfxsrc    = memory_region       ( REGION_GFX1 );
	size_t  srcsize = memory_region_length( REGION_GFX1 );
	int i;

	for (i=0; i < srcsize; i+=4)
	{
		UINT8 tmp[4];

		tmp[2] = ((gfxsrc[i+0]&0xf0)>>0) | ((gfxsrc[i+1]&0xf0)>>4); //  0,1,2,3  8,9,10, 11
		tmp[3] = ((gfxsrc[i+0]&0x0f)<<4) | ((gfxsrc[i+1]&0x0f)<<0); // 4,5,6,7, 12,13,14,15
		tmp[0] = ((gfxsrc[i+2]&0xf0)>>0) | ((gfxsrc[i+3]&0xf0)>>4);// 16,17,18,19,24,25,26,27
		tmp[1] = ((gfxsrc[i+2]&0x0f)<<4) | ((gfxsrc[i+3]&0x0f)>>0);// 20,21,22,23, 28,29,30,31

		gfxsrc[i+0] = tmp[0];
		gfxsrc[i+1] = tmp[1];
		gfxsrc[i+2] = tmp[2];
		gfxsrc[i+3] = tmp[3];

	}

}

static DRIVER_INIT( deroon )
{
	tecmosys_decramble();

	device_data = &deroon_data;
}

static DRIVER_INIT( tkdensho )
{
	tecmosys_decramble();

	device_data = &tkdensho_data;
}

GAME( 1996, deroon,      0, deroon, deroon, deroon,     ROT0, "Tecmo", "Deroon DeroDero", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, tkdensho,    0, deroon, deroon, tkdensho,   ROT0, "Tecmo", "Touki Denshou -Angel Eyes-", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS )

