/* Tecmo System
 Driver by Farfetch, David Haywood & Tomasz Slanina
  Protection simulation by nuapete

 ToDo:
  Dump / Decap MCUs to allow for proper protection emulation.
  Fix Sound (sound roms should be good, do they need descrambling, or is our sound core bad?
     some YMZ280B samples sound -terrible-)


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
|  LM324  UPC452C      16.9MHz          |--------|    |--------|    6264  |
| TA8205 LM324  YAC513 YMF262 YMZ280B   |TECMO   |    |TECMO   |    6264  |
|        LM324  M6295  UPC452C          |AA03-8431    |AA02-1927          |
|                      YAC512           |        |    |        |          |
|                                       |--------|    |--------|          |
|        Z80  6264 28MHz 14.31818MHz                  |--------|          |
|                    16MHz             62256          |TECMO   |          |
|            TA8030                    62256          |AA02-1927    6264  |
|                                                     |        |    6264  |
|J  93C46                               |--------|    |--------|          |
|A                                      |TECMO   |    |--------|          |
|M                                      |AA03-8431    |TECMO   |          |
|M          68000                       |        |    |AA02-1927          |
|A                                      |--------|    |        |    6264  |
|                  PAL              6116              |--------|    6264  |
|                                    6116             |--------|          |
|  |--------|                                         |TECMO   |          |
|  |TECMO   |                     PAL                 |AA02-1927          |
|  |AA03-8431  62256                                  |        |    6264  |
|  |        |  62256                                  |--------|    6264  |
|  |--------|                                  |---------|                |
|                                              |TECMO    |                |
|                                              |AA03-8431|                |
|                                              |         |                |
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
|    T201_DIP42_MASK.UBB1                                                 |
| |----|                                              T202_DIP42_MASK.UBC1|
| |*   |                                                                  |
| |----|                                                                  |
|                                                                         |
|  T003_2M_EPROM.UZ1                        T101_SOP44.UAH1               |
|                                                                         |
|                                            T301_DIP42_MASK.UBD1         |
|                                                                         |
|                                                                         |
|                                                                         |
|  T401_DIP42_MASK.UYA1      T104_SOP44.UCL1    T001_4M_EPROM.UPAU1       |
|                              T103_SOP44.UBL1                            |
|  T501_DIP32_MASK.UAD1      T102_SOP44.UAL1                              |
|                                                  T002_4M_EPROM.UPAL1    |
|-------------------------------------------------------------------------|
Notes:
      * - QFP64 microcontroller marked 'TECMO SC432146FU E23D 185 SSAB9540B'
          this is a 68HC11A8 with 8k ROM, 512 bytes EEPROM and 256 bytes on-chip RAM.
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

ROMs:

name            type
t001.upau1      27c040 dip32 eprom
t002.upal1      27c040 dip32 eprom
t003.uz1        27c2001 dip32 eprom

t101.uah1       23c16000 sop44 maskrom
t102.ual1       23c16000 sop44 maskrom
t103.ubl1       23c32000 sop44 maskrom
t104.ucl1       23c16000 sop44 maskrom
t201.ubb1       23c8000 dip42 maskrom
t202.ubc1       23c8000 dip42 maskrom
t301.ubd1       23c8000 dip42 maskrom
t401.uya1       23c16000 dip42 maskrom
t501.uad1       23c4001 dip32 maskrom

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

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "includes/tecmosys.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/262intf.h"
#include "sound/ymz280b.h"

static UINT16* tecmosys_spriteram;
static UINT16* tilemap_paletteram16;
static UINT16* bg2tilemap_ram;
static UINT16* bg1tilemap_ram;
static UINT16* bg0tilemap_ram;
static UINT16* fgtilemap_ram;
static UINT16* bg0tilemap_lineram;
static UINT16* bg1tilemap_lineram;
static UINT16* bg2tilemap_lineram;

static UINT16* tecmosys_a80000regs;
static UINT16* tecmosys_b00000regs;

static UINT16* tecmosys_c00000regs;
static UINT16* tecmosys_c80000regs;
static UINT16* tecmosys_880000regs;
static int tecmosys_spritelist;

static bitmap_t *sprite_bitmap;
static bitmap_t *tmp_tilemap_composebitmap;
static bitmap_t *tmp_tilemap_renderbitmap;


static tilemap_t *bg0tilemap;
static TILE_GET_INFO( get_bg0tile_info )
{

	SET_TILE_INFO(
			1,
			bg0tilemap_ram[2*tile_index+1],
			(bg0tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((bg0tilemap_ram[2*tile_index]&0xc0)>>6));
}

static WRITE16_HANDLER( bg0_tilemap_w )
{
	COMBINE_DATA(&bg0tilemap_ram[offset]);
	tilemap_mark_tile_dirty(bg0tilemap,offset/2);
}

static tilemap_t *bg1tilemap;
static TILE_GET_INFO( get_bg1tile_info )
{

	SET_TILE_INFO(
			2,
			bg1tilemap_ram[2*tile_index+1],
			(bg1tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((bg1tilemap_ram[2*tile_index]&0xc0)>>6));
}

static WRITE16_HANDLER( bg1_tilemap_w )
{
	COMBINE_DATA(&bg1tilemap_ram[offset]);
	tilemap_mark_tile_dirty(bg1tilemap,offset/2);
}

static tilemap_t *bg2tilemap;
static TILE_GET_INFO( get_bg2tile_info )
{

	SET_TILE_INFO(
			3,
			bg2tilemap_ram[2*tile_index+1],
			(bg2tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((bg2tilemap_ram[2*tile_index]&0xc0)>>6));
}

static WRITE16_HANDLER( bg2_tilemap_w )
{
	COMBINE_DATA(&bg2tilemap_ram[offset]);
	tilemap_mark_tile_dirty(bg2tilemap,offset/2);
}

static tilemap_t *txt_tilemap;
static TILE_GET_INFO( get_tile_info )
{

	SET_TILE_INFO(
			0,
			fgtilemap_ram[2*tile_index+1],
			(fgtilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((fgtilemap_ram[2*tile_index]&0xc0)>>6));
}

static WRITE16_HANDLER( fg_tilemap_w )
{
	COMBINE_DATA(&fgtilemap_ram[offset]);
	tilemap_mark_tile_dirty(txt_tilemap,offset/2);
}


// It looks like this needs a synch between z80 and 68k ??? See z80:006A-0091
static READ16_HANDLER( sound_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return soundlatch2_r( space,  0 );
	}

	return 0;
}

static WRITE16_HANDLER( sound_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, 0x00, data & 0xff);
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
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
	COMBINE_DATA(&tecmosys_880000regs[offset]);

	switch( offset )
	{
		case 0x00/2:
			break; // global x scroll for sprites?

		case 0x02/2:
			break; // global y scroll for sprites

		case 0x08/2:
			tecmosys_spritelist = data & 0x3; // which of the 4 spritelists to use (buffering)
			break;

		case 0x22/2:
			watchdog_reset( space->machine );
			//logerror( "watchdog_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, cpu_get_pc(space->cpu) );
			break;

		default:
			logerror( "unk880000_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, cpu_get_pc(space->cpu) );
			break;
	}
}

static READ16_HANDLER( unk880000_r )
{
	//UINT16 ret = tecmosys_880000regs[offset];

	logerror( "unk880000_r( %06x ) @ %06x = %04x\n", (offset * 2 ) +0x880000, cpu_get_pc(space->cpu), tecmosys_880000regs[offset] );

	/* this code allows scroll regs to be updated, but tkdensho at least resets perodically */

	switch( offset )
	{
		case 0:
			if ( video_screen_get_vpos(space->machine->primary_screen) >= 240) return 0;
			else return 1;

		default:
			return 0;
	}
}


static READ16_DEVICE_HANDLER( eeprom_r )
{
	 return ((eeprom_read_bit(device) & 0x01) << 11);
}


static WRITE16_DEVICE_HANDLER( eeprom_w )
{
	if ( ACCESSING_BITS_8_15 )
	{
		eeprom_write_bit(device, data & 0x0800);
		eeprom_set_cs_line(device, (data & 0x0200) ? CLEAR_LINE : ASSERT_LINE );
		eeprom_set_clock_line(device, (data & 0x0400) ? CLEAR_LINE: ASSERT_LINE );
	}
}


INLINE void set_color_555(running_machine *machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}


static WRITE16_HANDLER( tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w )
{
	COMBINE_DATA(&tilemap_paletteram16[offset]);
	set_color_555(space->machine, offset+0x4000, 5, 10, 0, tilemap_paletteram16[offset]);
}

static WRITE16_HANDLER( bg0_tilemap_lineram_w )
{
	COMBINE_DATA(&bg0tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg0 lineram %04x %04x",offset,data);
}

static WRITE16_HANDLER( bg1_tilemap_lineram_w )
{
	COMBINE_DATA(&bg1tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg1 lineram %04x %04x",offset,data);
}

static WRITE16_HANDLER( bg2_tilemap_lineram_w )
{
	COMBINE_DATA(&bg2tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg2 lineram %04x %04x",offset,data);
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM // work ram
	AM_RANGE(0x210000, 0x210001) AM_READNOP // single byte overflow on stack defined as 0x210000
	AM_RANGE(0x300000, 0x300fff) AM_RAM_WRITE(bg0_tilemap_w) AM_BASE(&bg0tilemap_ram) // bg0 ram
	AM_RANGE(0x301000, 0x3013ff) AM_RAM_WRITE(bg0_tilemap_lineram_w) AM_BASE(&bg0tilemap_lineram)// bg0 linescroll? (guess)

	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(bg1_tilemap_w) AM_BASE(&bg1tilemap_ram) // bg1 ram
	AM_RANGE(0x401000, 0x4013ff) AM_RAM_WRITE(bg1_tilemap_lineram_w) AM_BASE(&bg1tilemap_lineram)// bg1 linescroll? (guess)

	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(bg2_tilemap_w) AM_BASE(&bg2tilemap_ram) // bg2 ram
	AM_RANGE(0x501000, 0x5013ff) AM_RAM_WRITE(bg2_tilemap_lineram_w) AM_BASE(&bg2tilemap_lineram) // bg2 linescroll? (guess)

	AM_RANGE(0x700000, 0x703fff) AM_RAM_WRITE(fg_tilemap_w) AM_BASE(&fgtilemap_ram) // fix ram
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_BASE(&tecmosys_spriteram) // obj ram
	AM_RANGE(0x880000, 0x88000b) AM_READ(unk880000_r)
	AM_RANGE(0x900000, 0x907fff) AM_RAM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE_GENERIC(paletteram) // AM_WRITEONLY // obj pal

	//AM_RANGE(0x980000, 0x9807ff) AM_WRITEONLY // bg pal
	//AM_RANGE(0x980800, 0x980fff) AM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE_GENERIC(paletteram) // fix pal
	// the two above are as tested by the game code, I've only rolled them into one below to get colours to show right.
	AM_RANGE(0x980000, 0x980fff) AM_RAM_WRITE(tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE(&tilemap_paletteram16)

	AM_RANGE(0x880000, 0x88002f) AM_WRITE( unk880000_w ) AM_BASE(&tecmosys_880000regs)	// 10 byte dta@88000c, 880022=watchdog?
	AM_RANGE(0xa00000, 0xa00001) AM_DEVWRITE("eeprom", eeprom_w	)
	AM_RANGE(0xa80000, 0xa80005) AM_WRITEONLY AM_BASE(&tecmosys_a80000regs)	// a80000-3 scroll? a80004 inverted ? 3 : 0
	AM_RANGE(0xb00000, 0xb00005) AM_WRITEONLY AM_BASE(&tecmosys_b00000regs)	// b00000-3 scrool?, b00004 inverted ? 3 : 0
	AM_RANGE(0xb80000, 0xb80001) AM_READWRITE(tecmosys_prot_status_r, tecmosys_prot_status_w)
	AM_RANGE(0xc00000, 0xc00005) AM_WRITEONLY AM_BASE(&tecmosys_c00000regs)	// c00000-3 scroll? c00004 inverted ? 13 : 10
	AM_RANGE(0xc80000, 0xc80005) AM_WRITEONLY AM_BASE(&tecmosys_c80000regs)	// c80000-3 scrool? c80004 inverted ? 3 : 0
	AM_RANGE(0xd00000, 0xd00001) AM_READ_PORT("P1")
	AM_RANGE(0xd00002, 0xd00003) AM_READ_PORT("P2")
	AM_RANGE(0xd80000, 0xd80001) AM_DEVREAD("eeprom", eeprom_r)
	AM_RANGE(0xe00000, 0xe00001) AM_WRITE( sound_w )
	AM_RANGE(0xe80000, 0xe80001) AM_WRITE(tecmosys_prot_data_w)
	AM_RANGE(0xf00000, 0xf00001) AM_READ(sound_r)
	AM_RANGE(0xf80000, 0xf80001) AM_READ(tecmosys_prot_data_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( deroon )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )		PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )			PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )			PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )			PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )			PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )		PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )	PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )	PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )	PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )			PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )			PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )			PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )			PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
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
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout,   0x4400, 0x40 )
	GFXDECODE_ENTRY( "gfx3", 0, gfxlayout2,  0x4000, 0x40 )
	GFXDECODE_ENTRY( "gfx4", 0, gfxlayout2,  0x4000, 0x40 )
	GFXDECODE_ENTRY( "gfx5", 0, gfxlayout2,  0x4000, 0x40 )

GFXDECODE_END

static WRITE8_HANDLER( deroon_bankswitch_w )
{
	memory_set_bankptr(space->machine,  "bank1", memory_region(space->machine, "audiocpu") + ((data-2) & 0x0f) * 0x4000 + 0x10000 );
}

static WRITE8_HANDLER( tecmosys_oki_bank_w )
{
	UINT8 upperbank = (data & 0x30) >> 4;
	UINT8 lowerbank = (data & 0x03) >> 0;
	UINT8* region = memory_region(space->machine, "oki");

	memcpy( region+0x00000, region+0x80000 + lowerbank * 0x20000, 0x20000  );
	memcpy( region+0x20000, region+0x80000 + upperbank * 0x20000, 0x20000  );
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymf", ymf262_r, ymf262_w)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("oki", okim6295_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(tecmosys_oki_bank_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(deroon_bankswitch_w)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_r)
	AM_RANGE(0x50, 0x50) AM_WRITE(soundlatch2_w)
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("ymz", ymz280b_r, ymz280b_w)
ADDRESS_MAP_END


static VIDEO_START(deroon)
{
	sprite_bitmap = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);
	bitmap_fill(sprite_bitmap, NULL, 0x4000);

	tmp_tilemap_composebitmap = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);
	tmp_tilemap_renderbitmap = auto_bitmap_alloc(machine,320,240,BITMAP_FORMAT_INDEXED16);

	bitmap_fill(tmp_tilemap_composebitmap, NULL, 0x0000);
	bitmap_fill(tmp_tilemap_renderbitmap, NULL, 0x0000);


	txt_tilemap = tilemap_create(machine, get_tile_info,tilemap_scan_rows,8,8,32*2,32*2);
	tilemap_set_transparent_pen(txt_tilemap,0);

	bg0tilemap = tilemap_create(machine, get_bg0tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(bg0tilemap,0);

	bg1tilemap = tilemap_create(machine, get_bg1tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(bg1tilemap,0);

	bg2tilemap = tilemap_create(machine, get_bg2tile_info,tilemap_scan_rows,16,16,32,32);
	tilemap_set_transparent_pen(bg2tilemap,0);

}

static void tecmosys_render_sprites_to_bitmap(running_machine *machine, bitmap_t *bitmap, UINT16 extrax, UINT16 extray )
{
	UINT8 *gfxsrc    = memory_region       ( machine, "gfx1" );
	int i;

	/* render sprites (with priority information) to temp bitmap */
	bitmap_fill(sprite_bitmap, NULL, 0x0000);
	/* there are multiple spritelists in here, to allow for buffering */
	for (i=(tecmosys_spritelist*0x4000)/2;i<((tecmosys_spritelist+1)*0x4000)/2;i+=8)
	{
		int xcnt,ycnt;
		int drawx, drawy;
		UINT16* dstptr;

		int x, y;
		int address;
		int xsize = 16;
		int ysize = 16;
		int colour;
		int flipx, flipy;
		int priority;
		int zoomx, zoomy;

		x = tecmosys_spriteram[i+0]+386;
		y = (tecmosys_spriteram[i+1]+1);

		x-= extrax;
		y-= extray;

		y&=0x1ff;
		x&=0x3ff;

		if (x&0x200) x-=0x400;
		if (y&0x100) y-=0x200;

		address =  tecmosys_spriteram[i+5]| ((tecmosys_spriteram[i+4]&0x000f)<<16);

		address<<=8;

		flipx = (tecmosys_spriteram[i+4]&0x0040)>>6;
		flipy = (tecmosys_spriteram[i+4]&0x0080)>>7; // used by some move effects in tkdensho


		zoomx = (tecmosys_spriteram[i+2] & 0x0fff)>>0; // zoom?
		zoomy = (tecmosys_spriteram[i+3] & 0x0fff)>>0; // zoom?

		if ((!zoomx) || (!zoomy)) continue;

		ysize =  ((tecmosys_spriteram[i+6] & 0x00ff))*16;
		xsize =  (((tecmosys_spriteram[i+6] & 0xff00)>>8))*16;

		colour =  ((tecmosys_spriteram[i+4] & 0x3f00))>>8;

		priority = ((tecmosys_spriteram[i+4] & 0x0030))>>4;


		if (tecmosys_spriteram[i+4] & 0x8000) continue;

		for (ycnt = 0; ycnt < ysize; ycnt++)
		{
			int actualycnt = (ycnt * zoomy) >> 8;
			int actualysize = (ysize * zoomy) >> 8;

			if (flipy) drawy = y + (actualysize-1) - actualycnt;
			else drawy = y + actualycnt;


			for (xcnt = 0; xcnt < xsize; xcnt++)
			{
				int actualxcnt = (xcnt * zoomx) >> 8;
				int actualxsize = (xsize *zoomx) >> 8;

				if (flipx) drawx = x + (actualxsize-1) - actualxcnt;
				else drawx = x + actualxcnt;

				if ((drawx>=0 && drawx<320) && (drawy>=0 && drawy<240))
				{
					UINT8 data;

					dstptr = BITMAP_ADDR16(sprite_bitmap, drawy, drawx);


					data =  (gfxsrc[address]);


					if(data) dstptr[0] = (data + (colour*0x100)) | (priority << 14);
				}



				address++;

			}
		}

	}
}

static void tecmosys_tilemap_copy_to_compose(UINT16 pri)
{
	int y,x;
	UINT16 *srcptr;
	UINT16 *dstptr;
	for (y=0;y<240;y++)
	{
		srcptr = BITMAP_ADDR16(tmp_tilemap_renderbitmap, y, 0);
		dstptr = BITMAP_ADDR16(tmp_tilemap_composebitmap, y, 0);
		for (x=0;x<320;x++)
		{
			if ((srcptr[x]&0xf)!=0x0)
			    dstptr[x] =  (srcptr[x]&0x7ff) | pri;
		}
	}
}

static void tecmosys_do_final_mix(running_machine *machine, bitmap_t* bitmap)
{
	const pen_t *paldata = machine->pens;
	int y,x;
	UINT16 *srcptr;
	UINT16 *srcptr2;
	UINT32 *dstptr;


	for (y=0;y<240;y++)
	{

		srcptr = BITMAP_ADDR16(tmp_tilemap_composebitmap, y, 0);
		srcptr2 = BITMAP_ADDR16(sprite_bitmap, y, 0);


		dstptr = BITMAP_ADDR32(bitmap, y, 0);
		for (x=0;x<320;x++)
		{
			UINT16 pri, pri2;
			UINT16 penvalue;
			UINT16 penvalue2;
			UINT32 colour;
			UINT32 colour2;

			pri = srcptr[x] & 0xc000;
			pri2 = srcptr2[x] & 0xc000;

			penvalue = tilemap_paletteram16[srcptr[x]&0x7ff];
			colour =   paldata[(srcptr[x]&0x7ff) | 0x4000];

			if (srcptr2[x]&0x3fff)
			{
				penvalue2 = machine->generic.paletteram.u16[srcptr2[x]&0x3fff];
				colour2 = paldata[srcptr2[x]&0x3fff];
			}
			else
			{
				penvalue2 = tilemap_paletteram16[srcptr[x]&0x7ff];
				colour2 =   paldata[(srcptr[x]&0x7ff) | 0x4000];
			}

			if ((penvalue & 0x8000) && (penvalue2 & 0x8000)) // blend
			{
				int r,g,b;
				int r2,g2,b2;
				b = (colour & 0x000000ff) >> 0;
				g = (colour & 0x0000ff00) >> 8;
				r = (colour & 0x00ff0000) >> 16;

				b2 = (colour2 & 0x000000ff) >> 0;
				g2 = (colour2 & 0x0000ff00) >> 8;
				r2 = (colour2 & 0x00ff0000) >> 16;

				r = (r + r2) >> 1;
				g = (g + g2) >> 1;
				b = (b + b2) >> 1;

				dstptr[x] = b | (g<<8) | (r<<16);
			}
			else if (pri2 >= pri)
			{
				dstptr[x] = colour2;
			}
			else
			{
				dstptr[x] = colour;
			}
		}
	}
}

static VIDEO_UPDATE(deroon)
{

	bitmap_fill(bitmap,cliprect,screen->machine->pens[0x4000]);


	tilemap_set_scrolly( bg0tilemap, 0, tecmosys_c80000regs[1]+16);
	tilemap_set_scrollx( bg0tilemap, 0, tecmosys_c80000regs[0]+104);

	tilemap_set_scrolly( bg1tilemap, 0, tecmosys_a80000regs[1]+17);
	tilemap_set_scrollx( bg1tilemap, 0, tecmosys_a80000regs[0]+106);

	tilemap_set_scrolly( bg2tilemap, 0, tecmosys_b00000regs[1]+17);
	tilemap_set_scrollx( bg2tilemap, 0, tecmosys_b00000regs[0]+106);

	bitmap_fill(tmp_tilemap_composebitmap,cliprect,0);

	bitmap_fill(tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(tmp_tilemap_renderbitmap,cliprect,bg0tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(0x0000);

	bitmap_fill(tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(tmp_tilemap_renderbitmap,cliprect,bg1tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(0x4000);

	bitmap_fill(tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(tmp_tilemap_renderbitmap,cliprect,bg2tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(0x8000);

	bitmap_fill(tmp_tilemap_renderbitmap,cliprect,0);
	tilemap_draw(tmp_tilemap_renderbitmap,cliprect,txt_tilemap,0,0);
	tecmosys_tilemap_copy_to_compose(0xc000);


	tecmosys_do_final_mix(screen->machine, bitmap);

/*
    popmessage("%04x %04x %04x %04x | %04x %04x %04x %04x | %04x %04x %04x %04x  | %04x %04x %04x %04x  | %04x %04x %04x %04x  | %04x %04x %04x %04x",
        tecmosys_880000regs[0x0],  tecmosys_880000regs[0x1],  tecmosys_880000regs[0x2],  tecmosys_880000regs[0x3],
        tecmosys_880000regs[0x4],  tecmosys_880000regs[0x5],  tecmosys_880000regs[0x6],  tecmosys_880000regs[0x7],
        tecmosys_880000regs[0x8],  tecmosys_880000regs[0x9],  tecmosys_880000regs[0xa],  tecmosys_880000regs[0xb],
        tecmosys_880000regs[0xc],  tecmosys_880000regs[0xd],  tecmosys_880000regs[0xe],  tecmosys_880000regs[0xf],
        tecmosys_880000regs[0x10], tecmosys_880000regs[0x11], tecmosys_880000regs[0x12], tecmosys_880000regs[0x13],
        tecmosys_880000regs[0x14], tecmosys_880000regs[0x15], tecmosys_880000regs[0x16], tecmosys_880000regs[0x17]);
*/

//  popmessage("%04x %04x %04x | %04x %04x %04x",
//    tecmosys_c00000regs[0],     tecmosys_c00000regs[1],     tecmosys_c00000regs[2],
//    tecmosys_c80000regs[0],     tecmosys_c80000regs[1],     tecmosys_c80000regs[2]);

//  popmessage("%04x %04x %04x | %04x %04x %04x",
//    tecmosys_b00000regs[0],     tecmosys_b00000regs[1],     tecmosys_b00000regs[2],
//    tecmosys_a80000regs[0],     tecmosys_a80000regs[1],     tecmosys_a80000regs[2]);

	// prepare sprites for NEXT frame - causes 1 frame palette errors, but prevents sprite lag in tkdensho, which is correct?
	tecmosys_render_sprites_to_bitmap(screen->machine, bitmap, tecmosys_880000regs[0x0], tecmosys_880000regs[0x1]);


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


static void sound_irq(running_device *device, int irq)
{
	/* IRQ */
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ymf262_interface tecmosys_ymf262_interface =
{
	sound_irq		/* irq */
};


static MACHINE_DRIVER_START( deroon )
	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)
	MDRV_WATCHDOG_VBLANK_INIT(400) // guess

	MDRV_CPU_ADD("audiocpu", Z80, 16000000/2 )	/* 8 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(io_map)

	MDRV_GFXDECODE(tecmosys)

	MDRV_EEPROM_93C46_ADD("eeprom")

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.4458)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3000))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x4000+0x800)

	MDRV_VIDEO_START(deroon)
	MDRV_VIDEO_UPDATE(deroon)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymf", YMF262, 14318180)
	MDRV_SOUND_CONFIG(tecmosys_ymf262_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "rspeaker", 1.00)

	MDRV_SOUND_ADD("oki", OKIM6295, 16000000/8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ymz", YMZ280B, 16900000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_DRIVER_END


ROM_START( deroon )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Main Program
	ROM_LOAD16_BYTE( "t001.upau1", 0x00000, 0x80000, CRC(14b92c18) SHA1(b47b8c828222a3f7c0fe9271899bd38171d972fb) )
	ROM_LOAD16_BYTE( "t002.upal1", 0x00001, 0x80000, CRC(0fb05c68) SHA1(5140592e15414770fb46d5ac9ba8f76e3d4ab323) )

	ROM_REGION( 0x048000, "audiocpu", 0 ) // Sound Porgram
	ROM_LOAD( "t003.uz1", 0x000000, 0x008000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )
	ROM_CONTINUE(         0x010000, 0x038000 ) /* banked part */

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "deroon_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "deroon_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x2000000, "gfx1", ROMREGION_ERASE00 ) // Sprites (non-tile based)
	/* all these roms need verifying, they could be half size */

	ROM_LOAD16_BYTE( "t101.uah1", 0x0000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD16_BYTE( "t102.ual1", 0x0000001, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )
	/*                            0x8000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD16_BYTE( "t103.ubl1", 0x0800001, 0x400000, CRC(84e7da88) SHA1(b5c3234f33bb945cc9762b91db087153a0589cfb) )
	/*                            0x1000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD16_BYTE( "t104.ucl1", 0x1000001, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "t301.ubd1", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_ERASE00) // 16x16 4bpp tiles
	/* not used? */

	ROM_REGION( 0x100000, "gfx4", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t201.ubb1", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )

	ROM_REGION( 0x100000, "gfx5", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t202.ubc1", 0x000000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )

	ROM_REGION( 0x200000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "t401.uya1", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x100000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "t501.uad1", 0x080000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

ROM_START( tkdensho )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aeprge-2.pal", 0x00000, 0x80000, CRC(25e453d6) SHA1(9c84e2af42eff5cc9b14c1759d5bab42fa7bb663) )
	ROM_LOAD16_BYTE( "aeprgo-2.pau", 0x00001, 0x80000, CRC(22d59510) SHA1(5ade482d6ab9a22df2ee8337458c22cfa9045c73) )

	ROM_REGION( 0x038000, "audiocpu", 0 ) // Sound Porgram
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x008000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_CONTINUE(            0x010000, 0x018000 ) /* banked part */

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "tkdensho_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "tkdensho_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x4000000, "gfx1", ROMREGION_ERASE00 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD16_BYTE( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD16_BYTE( "ae100.al1",     0x0000001, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD16_BYTE( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD16_BYTE( "ae101.bl1",     0x0800001, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD16_BYTE( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD16_BYTE( "ae102.cl1",     0x1000001, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD16_BYTE( "ae104.el1",     0x2000001, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD16_BYTE( "ae105.fl1",     0x2800001, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD16_BYTE( "ae106.gl1",     0x3000001, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, "gfx5", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x800000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x100000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x080000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

ROM_START( tkdenshoa )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aeprge.pal", 0x00000, 0x80000, CRC(17a209ff) SHA1(b5dbea9868cbb89d4e27bf19fdb616ac256985b4) )
	ROM_LOAD16_BYTE( "aeprgo.pau", 0x00001, 0x80000, CRC(d265e6a1) SHA1(f39d8ce115f197a660f5210b2483108854eb12a9) )

	ROM_REGION( 0x038000, "audiocpu", 0 ) // Sound Porgram
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x008000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_CONTINUE(            0x010000, 0x018000 ) /* banked part */

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "tkdensho_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "tkdensho_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x4000000, "gfx1", ROMREGION_ERASE00 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD16_BYTE( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD16_BYTE( "ae100.al1",     0x0000001, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD16_BYTE( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD16_BYTE( "ae101.bl1",     0x0800001, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD16_BYTE( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD16_BYTE( "ae102.cl1",     0x1000001, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD16_BYTE( "ae104.el1",     0x2000001, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD16_BYTE( "ae105.fl1",     0x2800001, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD16_BYTE( "ae106.gl1",     0x3000001, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, "gfx5", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x800000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x100000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x080000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

static void tecmosys_decramble(running_machine *machine)
{
	UINT8 *gfxsrc    = memory_region       ( machine, "gfx1" );
	size_t  srcsize = memory_region_length( machine, "gfx1" );
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
	tecmosys_decramble(machine);
	tecmosys_prot_init(machine, 0);
}

static DRIVER_INIT( tkdensho )
{
	tecmosys_decramble(machine);
	tecmosys_prot_init(machine, 1);
}

static DRIVER_INIT( tkdensha )
{
	tecmosys_decramble(machine);
	tecmosys_prot_init(machine, 2);
}

GAME( 1995, deroon,           0, deroon, deroon, deroon,     ROT0, "Tecmo", "Deroon DeroDero", 0 )
GAME( 1996, tkdensho,         0, deroon, deroon, tkdensho,   ROT0, "Tecmo", "Touki Denshou -Angel Eyes- (VER. 960614)", 0 )
GAME( 1996, tkdenshoa, tkdensho, deroon, deroon, tkdensha,   ROT0, "Tecmo", "Touki Denshou -Angel Eyes- (VER. 960427)", 0 )
