/*******************************************************************************************

MJ-8956 HW games (c) 1989 Jaleco / NMK

driver by Angelo Salese, based on early work by David Haywood

Similar to the NMK16 board,but without sprites.

Notes(general):
-I think that the 0xf0000-0xfffff area is shared with the MCU because:
\-The first version of the MCU programs (daireika/mjzoomin) jump in that area.
  Then the MCU upload a 68k code;the programs are likely to be the same for all the games,
  for example if the program jumps to $f0020 it means a sample request.
\-input ports located there.Program doesn't check these locations at P.O.S.T. and doesn't
  give any work ram error.
\-Additionally all the games have a MCU protection which involves various RAM areas,
  that controls mahjong panel inputs.Protection is simulated by
  adding a value to these RAM areas according to what button is pressed.

-Understand what are RANGE registers for.They seems to be related to the scroll
 registers.
-In all the games there are square gaps during gameplay.
-Fix kakumei2 GFX3 rom region,maybe bad dump (half length)?
-Fully understand priorities...
-Fix daireika/urashima GFX rom loading.
-Fix the dip-switches in the first version of this board.
-There could be timing issues caused by MCU simulation at $80004.
-daireika/kakumei2 have problems with the tx layer vertical scrolling during the attract
 mode, likely to be a paging issue...

Notes (1st MCU ver.):
-$f000e is bogus,maybe the program snippets can modify this value,or the MCU itself can
 do that,returning the contents of D0 register seems enough for now...
 Update: Improved it for the new mcu simulation,now it pulls all the values from 0x00 to
 0x0f,it seems
-$f030e is a mirror for $f000e in urashima.
-Why urashima A0/7 registers parameters seems to have wrong source/destination addresses
 during MCU operations?Different HW?
-I need more space for MCU code...that's why I've used an extra jmp when entering
 into mcu code,so we can debug the first version freely without being teased about
 memory space...
 BTW,the real HW is using a sort of bankswitch or I'm missing something?
-$f0020 is for the sound program,same for all games,for example mjzoomin hasn't any clear
 write to $80040 area and the program jumps to $f0020 when there should be a sample.
-urashima scroll registers aren't used at all.Investigate on it...
-mjzoomin controls during the gameplay don't react as expected,something to do
 with the protection(it probably reads a button pressed).

============================================================================================
Debug cheats:

-(suchipi)
*
$fe87e: bonus timer,used as a count-down.
*
$f079a: finish match now
*
During gameplay,set $f0400 to 6 then set $f07d4 to 1 to advance to next
level.
*
$f06a6-$f06c0: Your tiles
$f06c6-$f06e0: COM tiles
---- ---- --xx ----: Defines kind
---- ---- ---- xxxx: Defines number

============================================================================================
daireika 68k irq table vectors
lev 1 : 0x64 : 0000 049e -
lev 2 : 0x68 : 0000 04ae -
lev 3 : 0x6c : 0000 049e -
lev 4 : 0x70 : 0000 091a -
lev 5 : 0x74 : 0000 0924 -
lev 6 : 0x78 : 0000 092e -
lev 7 : 0x7c : 0000 0938 -

mjzoomin 68k irq table vectors
lev 1 : 0x64 : 0000 048a -
lev 2 : 0x68 : 0000 049a - vblank
lev 3 : 0x6c : 0000 048a -
lev 4 : 0x70 : 0000 09ba - "write to Text RAM" (?)
lev 5 : 0x74 : 0000 09c4 - "write to Text RAM" (?)
lev 6 : 0x78 : 0000 09ce - "write to Text RAM" (?)
lev 7 : 0x7c : 0000 09d8 - "write to Text RAM" (?)

kakumei/kakumei2/suchipi 68k irq table vectors
lev 1 : 0x64 : 0000 0506 - rte
lev 2 : 0x68 : 0000 050a - vblank
lev 3 : 0x6c : 0000 051c - rte
lev 4 : 0x70 : 0000 0520 - rte
lev 5 : 0x74 : 0000 0524 - rte
lev 6 : 0x78 : 0000 0524 - rte
lev 7 : 0x7c : 0000 0524 - rte

Board:  MJ-8956

CPU:    68000-8
        M50747 (not dumped)
Sound:  M6295
OSC:    12.000MHz
        4.000MHz
*******************************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

static tilemap *sc3_tilemap, *sc0_tilemap,*sc1_tilemap,*sc2_tilemap;
static UINT16 *sc3_vram, *sc0_vram,*sc1_vram,*sc2_vram;
static UINT16 *jm_regs,*jm_ram,*jm_mcu_code;
static UINT16 *jm_scrollram,*jm_vregs;
static UINT16 sc0bank,pri;
/*
MCU program number,different for each game(n.b. the numbering scheme is *mine*,do not
take it seriously...):
0x11 = daireika
0x12 = urashima
0x13 = mjzoomin
0x21 = kakumei
0x22 = kakumei2
0x23 = suchipi

xxxx ---- MCU program revision
---- xxxx MCU program number assignment for each game.
*/
static UINT8 mcu_prg;
#define DAIREIKA_MCU (0x11)
#define URASHIMA_MCU (0x12)
#define MJZOOMIN_MCU (0x13)
#define KAKUMEI_MCU  (0x21)
#define KAKUMEI2_MCU (0x22)
#define SUCHIPI_MCU  (0x23)

static int respcount;

static TILEMAP_MAPPER( bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

static TILE_GET_INFO( get_sc0_tile_info )
{
	int code = sc0_vram[tile_index];
	SET_TILE_INFO(
			3,
			(code & 0xfff) + ((sc0bank & 3) << 12),
			code >> 12,
			0);
}

static TILE_GET_INFO( get_sc3_tile_info )
{
	int code = sc3_vram[tile_index];
	SET_TILE_INFO(
			0,
			code & 0xfff,
			code >> 12,
			0);
}

static TILE_GET_INFO( get_sc2_tile_info )
{
	int code = sc2_vram[tile_index];
	SET_TILE_INFO(
			1,
			code & 0xfff,
			code >> 12,
			0);
}

static TILE_GET_INFO( get_sc1_tile_info )
{
	int code = sc1_vram[tile_index];
	SET_TILE_INFO(
			2,
			code & 0xfff,
			code >> 12,
			0);
}

static VIDEO_START( jalmah )
{
	sc0_tilemap = tilemap_create(get_sc0_tile_info,bg_scan,16,16,256,32);
	sc1_tilemap = tilemap_create(get_sc1_tile_info,bg_scan,16,16,256,32);
	sc2_tilemap = tilemap_create(get_sc2_tile_info,bg_scan,16,16,256,32);
	sc3_tilemap = tilemap_create(get_sc3_tile_info,tilemap_scan_cols,8,8,256,32);

	jm_scrollram = auto_malloc(0x80);
	jm_vregs = auto_malloc(0x40);

	tilemap_set_transparent_pen(sc0_tilemap,15);
	tilemap_set_transparent_pen(sc1_tilemap,15);
	tilemap_set_transparent_pen(sc2_tilemap,15);
	tilemap_set_transparent_pen(sc3_tilemap,15);
}

#define MCU_READ(_number_,_bit_,_offset_,_retval_)\
if((0xffff - input_port_read_indexed(machine, _number_)) & _bit_) { jm_regs[_offset_] = _retval_; }

/*RAM-based protection handlings*/
static void daireika_mcu_run(running_machine *machine)
{
	static UINT16 prg_prot;

	if((0xffff - input_port_read_indexed(machine,0)) & 0x0008)//service_mode
	{
		jm_regs[0x000/2] = input_port_read_indexed(machine,2);
		jm_regs[0x002/2] = input_port_read_indexed(machine,3);
		jm_regs[0x004/2] = input_port_read_indexed(machine,4);
		jm_regs[0x006/2] = input_port_read_indexed(machine,5);
		jm_regs[0x008/2] = input_port_read_indexed(machine,6);
		jm_regs[0x00a/2] = input_port_read_indexed(machine,7);
	}
	else
	{
		jm_regs[0x000/2] = 0x0000;
		MCU_READ(3,0x0001,0x000/2,0x00);/*FF*/
		MCU_READ(4,0x0400,0x000/2,0x01);/*A*/
		MCU_READ(4,0x1000,0x000/2,0x02);/*B*/
		MCU_READ(4,0x0200,0x000/2,0x03);/*C*/
		MCU_READ(4,0x0800,0x000/2,0x04);/*D*/
		MCU_READ(4,0x0004,0x000/2,0x05);/*E*/
		MCU_READ(4,0x0010,0x000/2,0x06);/*F*/
		MCU_READ(4,0x0002,0x000/2,0x07);/*G*/
		MCU_READ(4,0x0008,0x000/2,0x08);/*H*/
		MCU_READ(3,0x0400,0x000/2,0x09);/*I*/
		MCU_READ(3,0x1000,0x000/2,0x0a);/*J*/
		MCU_READ(3,0x0200,0x000/2,0x0b);/*K*/
		MCU_READ(3,0x0800,0x000/2,0x0c);/*L*/
		MCU_READ(3,0x0004,0x000/2,0x0d);/*M*/
		MCU_READ(3,0x0010,0x000/2,0x0e);/*N*/
		MCU_READ(2,0x0200,0x000/2,0x0f);/*RON   (trusted)*/
		MCU_READ(2,0x1000,0x000/2,0x10);/*REACH (trusted)*/
		MCU_READ(2,0x0400,0x000/2,0x11);/*KAN            */
		MCU_READ(3,0x0008,0x000/2,0x12);/*PON            */
		MCU_READ(3,0x0002,0x000/2,0x13);/*CHI   (trusted)*/
		MCU_READ(2,0x0004,0x000/2,0x14);/*START1*/
	}
	jm_regs[0x00c/2] = mame_rand(machine) & 0xffff;
	prg_prot++;
	if(prg_prot > 0x10) { prg_prot = 0; }
	jm_regs[0x00e/2] = prg_prot;

}

static void urashima_mcu_run(running_machine *machine)
{
	static UINT16 prg_prot;

	if((0xffff - input_port_read_indexed(machine,0)) & 0x0008)//service_mode
	{
		jm_regs[0x300/2] = input_port_read_indexed(machine,2);
		jm_regs[0x302/2] = input_port_read_indexed(machine,3);
		jm_regs[0x304/2] = input_port_read_indexed(machine,4);
		jm_regs[0x306/2] = input_port_read_indexed(machine,5);
		jm_regs[0x308/2] = input_port_read_indexed(machine,6);
		jm_regs[0x30a/2] = input_port_read_indexed(machine,7);
	}
	else
	{
		jm_regs[0x300/2] = 0x0000;
		MCU_READ(3,0x0001,0x300/2,0x00);/*FF*/
		MCU_READ(4,0x0400,0x300/2,0x01);/*A*/
		MCU_READ(4,0x1000,0x300/2,0x02);/*B*/
		MCU_READ(4,0x0200,0x300/2,0x03);/*C*/
		MCU_READ(4,0x0800,0x300/2,0x04);/*D*/
		MCU_READ(4,0x0004,0x300/2,0x05);/*E*/
		MCU_READ(4,0x0010,0x300/2,0x06);/*F*/
		MCU_READ(4,0x0002,0x300/2,0x07);/*G*/
		MCU_READ(4,0x0008,0x300/2,0x08);/*H*/
		MCU_READ(3,0x0400,0x300/2,0x09);/*I*/
		MCU_READ(3,0x1000,0x300/2,0x0a);/*J*/
		MCU_READ(3,0x0200,0x300/2,0x0b);/*K*/
		MCU_READ(3,0x0800,0x300/2,0x0c);/*L*/
		MCU_READ(3,0x0004,0x300/2,0x0d);/*M*/
		MCU_READ(3,0x0010,0x300/2,0x0e);/*N*/
		MCU_READ(2,0x0200,0x300/2,0x0f);/*RON   (trusted)*/
		MCU_READ(2,0x1000,0x300/2,0x10);/*REACH (trusted)*/
		MCU_READ(2,0x0400,0x300/2,0x11);/*KAN            */
		MCU_READ(3,0x0008,0x300/2,0x12);/*PON            */
		MCU_READ(3,0x0002,0x300/2,0x13);/*CHI   (trusted)*/
		MCU_READ(2,0x0004,0x300/2,0x14);/*START1*/
	}
	jm_regs[0x30c/2] = mame_rand(machine) & 0xffff;
	prg_prot++;
	if(prg_prot > 0x10) { prg_prot = 0; }
	jm_regs[0x30e/2] = prg_prot;
}

static void second_mcu_run(running_machine *machine)
{
	if((0xffff - input_port_read_indexed(machine,1)) & 0x0004)//service_mode
	{
		jm_regs[0x200/2] = input_port_read_indexed(machine,2);
		jm_regs[0x202/2] = input_port_read_indexed(machine,3);
		jm_regs[0x204/2] = input_port_read_indexed(machine,4);
	}
	else
	{
		jm_regs[0x200/2] = 0x0000;
		MCU_READ(3,0x0001,0x200/2,0x00);/*FF*/
		MCU_READ(4,0x0400,0x200/2,0x01);/*A*/
		MCU_READ(4,0x1000,0x200/2,0x02);/*B*/
		MCU_READ(4,0x0200,0x200/2,0x03);/*C*/
		MCU_READ(4,0x0800,0x200/2,0x04);/*D*/
		MCU_READ(4,0x0004,0x200/2,0x05);/*E*/
		MCU_READ(4,0x0010,0x200/2,0x06);/*F*/
		MCU_READ(4,0x0002,0x200/2,0x07);/*G*/
		MCU_READ(4,0x0008,0x200/2,0x08);/*H*/
		MCU_READ(3,0x0400,0x200/2,0x09);/*I*/
		MCU_READ(3,0x1000,0x200/2,0x0a);/*J*/
		MCU_READ(3,0x0200,0x200/2,0x0b);/*K*/
		MCU_READ(3,0x0800,0x200/2,0x0c);/*L*/
		MCU_READ(3,0x0004,0x200/2,0x0d);/*M*/
		MCU_READ(3,0x0010,0x200/2,0x0e);/*N*/
		MCU_READ(2,0x0200,0x200/2,0x0f);/*RON*/
		MCU_READ(2,0x1000,0x200/2,0x10);/*REACH*/
		MCU_READ(2,0x0400,0x200/2,0x11);/*KAN*/
		MCU_READ(3,0x0008,0x200/2,0x12);/*PON*/
		MCU_READ(3,0x0002,0x200/2,0x13);/*CHI*/
		MCU_READ(2,0x0004,0x200/2,0x14);/*START1*/

//      MCU_READ(2,0x0004,0x7b8/2,0x03);/*START1(correct?)  */
	}
	jm_regs[0x20c/2] = mame_rand(machine) & 0xffff; //kakumei2

}

#define jalmah_tilemap_draw(_tilemap_) \
	tilemap_draw(bitmap,cliprect,_tilemap_,(opaque & 1) ? 0 : TILEMAP_DRAW_OPAQUE,0); \
	if(!opaque) { opaque = 1; }

static VIDEO_UPDATE( jalmah )
{
	int opaque = 0;
	#ifdef MAME_DEBUG
	static UINT8 sc_db;
	#endif

	switch(mcu_prg)
	{
		/*
        #define DAIREIKA_MCU (0x11)
        #define URASHIMA_MCU (0x12)
        #define MJZOOMIN_MCU (0x13)
        #define KAKUMEI_MCU  (0x21)
        #define KAKUMEI2_MCU (0x22)
        #define SUCHIPI_MCU  (0x23)
        */
		case MJZOOMIN_MCU:
		case DAIREIKA_MCU: daireika_mcu_run(screen->machine); break;
		case URASHIMA_MCU: urashima_mcu_run(screen->machine); break;
		case KAKUMEI_MCU:
		case KAKUMEI2_MCU:
		case SUCHIPI_MCU:  second_mcu_run(screen->machine); break;
	}
	tilemap_set_scrollx( sc0_tilemap, 0, jm_scrollram[0] + ((jm_vregs[0] & 3) ? ((jm_scrollram[4] & 0x200) * 4) : 0));
	tilemap_set_scrollx( sc1_tilemap, 0, jm_scrollram[1] + ((jm_vregs[1] & 3) ? ((jm_scrollram[5] & 0x200) * 4) : 0));
	tilemap_set_scrollx( sc2_tilemap, 0, jm_scrollram[2] + ((jm_vregs[2] & 3) ? ((jm_scrollram[6] & 0x200) * 4) : 0));
	tilemap_set_scrollx( sc3_tilemap, 0, jm_scrollram[3] + ((jm_vregs[3] & 3) ? ((jm_scrollram[7] & 0x200) * 4) : 0));
	tilemap_set_scrolly( sc0_tilemap, 0, jm_scrollram[4] & 0x1ff);
	tilemap_set_scrolly( sc1_tilemap, 0, jm_scrollram[5] & 0x1ff);
	tilemap_set_scrolly( sc2_tilemap, 0, jm_scrollram[6] & 0x1ff);
	tilemap_set_scrolly( sc3_tilemap, 0, jm_scrollram[7] & 0x1ff);

	#ifdef MAME_DEBUG
	if(input_code_pressed_once(JOYCODE_Y_UP_SWITCH))
		sc_db++;
	if(input_code_pressed_once(JOYCODE_Y_DOWN_SWITCH))
		sc_db--;
	if(sc_db > 3)
		sc_db = 3;
	//popmessage("%04x %04x %04x %04x %04x %04x",jm_vregs[0],jm_vregs[1],jm_vregs[2],jm_vregs[3],sc0bank,pri);
	popmessage("%04d %04d %04x %02x %01x",jm_scrollram[0+sc_db],jm_scrollram[4+sc_db],jm_vregs[0+sc_db],sc_db,pri);
	#endif
	/*
    Case by case priorities:
    kakumei: title screen     = 0/23  (0-1 unused)
             attract mode     = 0/0123 (?)
             card table       = 0/013 (2 unused)
             gameplay         = 0/0123
    kakumei2:title screen     = 0/0123
             attract mode     = 4/013 (2 unused) also 4/0213
             card table       = 9/013 (2 unused)
             character screen = 9/013 (2 unused)
             gameplay         = 2/03  (1-2 unused) also 2/013 (2 unused)
             continue screen  = 9/013 (2 unused)
    suchipi: title screen     = 0/03  (1-2 unused)
             attract mode     = 6/103 (2 unused) also 6/1023
             card table       = 1/013 (2 unused)
             character screen = 4/0213
             victory screen   = 6/0123 also 6/1023
             gameplay         = d/0123
    mjzoomin doesn't seem to use the priority number (0/0123)
    daireika/urashima uses priority number with the protection device
    daireika: gameplay        = x/103 (2 unused),might be 6
    good results:
    0/0123
    4/0213
    6/1023
    */
	if(!(pri & 2) || (pri & 0x8)) { jalmah_tilemap_draw(sc0_tilemap); }
	if(!(pri & 4) || (pri & 0x8)) { jalmah_tilemap_draw(sc1_tilemap); }
	if(!(pri & 2) || (pri & 0x8)) { jalmah_tilemap_draw(sc2_tilemap); }
	if(pri & 4 && (!(pri & 0x8))) { jalmah_tilemap_draw(sc1_tilemap); }
	if(pri & 2 && (!(pri & 0x8))) { jalmah_tilemap_draw(sc0_tilemap); }
	if(pri & 2 && (!(pri & 0x8))) { jalmah_tilemap_draw(sc2_tilemap); }
	jalmah_tilemap_draw(sc3_tilemap);
	return 0;
}


static WRITE16_HANDLER( sc0_vram_w )
{
	COMBINE_DATA(&sc0_vram[offset]);
	tilemap_mark_tile_dirty(sc0_tilemap,offset);
}

static WRITE16_HANDLER( sc1_vram_w )
{
	COMBINE_DATA(&sc1_vram[offset]);
	tilemap_mark_tile_dirty(sc1_tilemap,offset);
}

static WRITE16_HANDLER( sc3_vram_w )
{
	COMBINE_DATA(&sc3_vram[offset]);
	tilemap_mark_tile_dirty(sc3_tilemap,offset);
}

static WRITE16_HANDLER( sc2_vram_w )
{
	COMBINE_DATA(&sc2_vram[offset]);
	tilemap_mark_tile_dirty(sc2_tilemap,offset);
}

static WRITE16_HANDLER( jalmah_tilebank_w )
{
	/*
     xxxx ---- fg bank (used by suchipi)
     ---- xxxx Priority number (trusted,see mjzoomin)
    */
	//popmessage("Write to tilebank %02x",data);
	if (ACCESSING_BITS_0_7)
	{
		if (sc0bank != ((data & 0xf0) >> 4))
		{
			sc0bank = (data & 0xf0) >> 4;
			tilemap_mark_all_tiles_dirty(sc0_tilemap);
		}
		if (pri != (data & 0x0f))
			pri = data & 0x0f;
	}
}

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( jalmah_reg_w )
{
	COMBINE_DATA(&jm_regs[offset]);
}
#endif

static WRITE16_HANDLER( jalmah_scroll_w )
{
	logerror("[%04x]<-%04x\n",(offset+0x10)*2,data);
	switch(offset+(0x10))
	{
		/*These 4 are just video regs,see mjzoomin test*/
		/*
            ---x ---- Always on,8x8 tiles switch?
            ---- --xx RANGE registers
        */
		case (0x24/2): jm_vregs[0] = data; break;
		case (0x2c/2): jm_vregs[1] = data; break;
		case (0x34/2): jm_vregs[2] = data; break;
		case (0x3c/2): jm_vregs[3] = data; break;

		case (0x20/2): jm_scrollram[0] = data; break;
		case (0x28/2): jm_scrollram[1] = data; break;
		case (0x30/2): jm_scrollram[2] = data; break;
		case (0x38/2): jm_scrollram[3] = data; break;
		case (0x22/2): jm_scrollram[4] = data; break;
		case (0x2a/2): jm_scrollram[5] = data; break;
		case (0x32/2): jm_scrollram[6] = data; break;
		case (0x3a/2): jm_scrollram[7] = data; break;
		//default:    popmessage("[%04x]<-%04x",offset+0x10,data);
	}
}

static UINT8 oki_rom,oki_bank;

static WRITE16_HANDLER( jalmah_okirom_w )
{
	if(ACCESSING_BITS_0_7)
	{
		UINT8 *oki = memory_region(REGION_SOUND1);
		oki_rom = data & 1;
		memcpy(&oki[0x20000], &oki[(oki_rom * 0x80000) + (oki_bank * 0x20000) + 0x40000], 0x20000);
	}

	/*No game can use this,and btw what "ZA" stands for?*/
	if(data & 2)
		mame_printf_debug("ZA = 2\n");
}

static WRITE16_HANDLER( jalmah_okibank_w )
{
	if(ACCESSING_BITS_0_7)
	{
		UINT8 *oki = memory_region(REGION_SOUND1);
		oki_bank = data & 3;
		memcpy(&oki[0x20000], &oki[(oki_rom * 0x80000) + (oki_bank * 0x20000) + 0x40000], 0x20000);
	}
}

static WRITE16_HANDLER( jalmah_flip_screen_w )
{
	/*---- ----x flip screen*/
	flip_screen_set(data & 1);

//  popmessage("%04x",data);
}

static ADDRESS_MAP_START( jalmah, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ(input_port_0_word_r)
	AM_RANGE(0x080002, 0x080003) AM_READ(input_port_1_word_r)
	//       0x080004, 0x080005  MCU read,different for each game
	AM_RANGE(0x080010, 0x080011) AM_WRITE(jalmah_flip_screen_w)
	//       0x080012, 0x080013  MCU write related,same for each game
	//       0x080014, 0x080015  MCU write related,same for each game
/**/AM_RANGE(0x080016, 0x080017) AM_READ(SMH_RAM) AM_WRITE(jalmah_tilebank_w)
	AM_RANGE(0x080018, 0x080019) AM_WRITE(jalmah_okibank_w)
	AM_RANGE(0x08001a, 0x08001b) AM_WRITE(jalmah_okirom_w)
	//       0x08001c, 0x08001d  write 1 by urashima (?)
/**/AM_RANGE(0x080020, 0x08003f) AM_READ(SMH_RAM) AM_WRITE(jalmah_scroll_w)
	AM_RANGE(0x080040, 0x080041) AM_READWRITE(OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w)
	//       0x084000, 0x084001  ?
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE(&paletteram16) /* Palette RAM */
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(sc0_vram_w) AM_BASE(&sc0_vram)
	AM_RANGE(0x094000, 0x097fff) AM_RAM_WRITE(sc1_vram_w) AM_BASE(&sc1_vram)
	AM_RANGE(0x098000, 0x09bfff) AM_RAM_WRITE(sc2_vram_w) AM_BASE(&sc2_vram)
	AM_RANGE(0x09c000, 0x09ffff) AM_RAM_WRITE(sc3_vram_w) AM_BASE(&sc3_vram)
	AM_RANGE(0x0a0000, 0x0a3fff) AM_RAM_WRITE(sc3_vram_w) /*urashima mirror*/
	AM_RANGE(0x0f0000, 0x0f0fff) AM_RAM AM_BASE(&jm_regs)/*shared with MCU*/
	AM_RANGE(0x0f1000, 0x0fffff) AM_RAM AM_BASE(&jm_ram)
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_BASE(&jm_mcu_code)/*extra RAM for MCU code prg (NOT ON REAL HW!!!)*/
ADDRESS_MAP_END




static INPUT_PORTS_START( jalmah )
	/*System port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Dip-SW port*/
	PORT_START
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )    // Coin2 is always 1C/1C.
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Mahjong Panel ports*/
	PORT_START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) 	PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) 	PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED ) 		PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) 		 PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )		 PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )		 PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )		 PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K )		 PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I )		 PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )		 PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J )		 PORT_PLAYER(2)
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jalmah2 )
	/*System port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Dip-SW port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin / 99 Credits" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Mahjong Panel ports*/
	PORT_START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Mahjong Flip Flop?")	PORT_CODE(KEYCODE_2) //? seems a button,affects continue countdown
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jalmah2a )
	/*System port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Dip-SW port*/
	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin / 99 Credits" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	/*Mahjong Panel ports*/
	PORT_START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xe9fb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Mahjong Flip Flop?")	PORT_CODE(KEYCODE_2) //? seems a button,affects continue countdown
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0xe1e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0xe1e1, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( jalmah )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout, 0x300, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout, 0x200, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, tilelayout, 0x100, 16 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, tilelayout, 0x000, 16 )
GFXDECODE_END

static MACHINE_RESET (daireika)
{
	respcount = 0;
}

static MACHINE_DRIVER_START( jalmah )
	MDRV_CPU_ADD_TAG("main" , M68000, 8000000) /* 68000-8 */
	MDRV_CPU_PROGRAM_MAP(jalmah,0)
	MDRV_CPU_VBLANK_INT("main", irq2_line_hold)

	MDRV_GFXDECODE(jalmah)


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x400)
	MDRV_MACHINE_RESET(daireika)

	MDRV_VIDEO_START(jalmah)
	MDRV_VIDEO_UPDATE(jalmah)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, 4000000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7low)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/*

Urashima Mahjong
(c) 1989 UPL

*/

ROM_START ( urashima )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )		/* 68000 code */
	ROM_LOAD16_BYTE( "um-2.15d",  0x00000, 0x20000, CRC(a90a47e3) SHA1(2f912001e9177cce8c3795f3d299115b80fdca4e) )
	ROM_RELOAD(                   0x40000, 0x20000 )
	ROM_LOAD16_BYTE( "um-1.15c",  0x00001, 0x20000, CRC(5f5c8f39) SHA1(cef663965c3112f87788d6a871e609c0b10ef9a2) )
	ROM_RELOAD(                   0x40001, 0x20000 )

	ROM_REGION( 0x140000, REGION_SOUND1, ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "um-3.22c",	   0x00000, 0x80000, CRC(9fd8c8fa) SHA1(0346f74c03a4daa7a84b64c9edf0e54297c82fd9) )
	ROM_RELOAD(                0x40000, 0x80000 )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "um-5.22j",		0x000000, 0x020000, CRC(991776a2) SHA1(56740553d7d26aaeb9bec8557727030950bb01f7) )	/* 8x8 tiles */

	ROM_REGION( 0x080000, REGION_GFX2, ROMREGION_DISPOSE ) /* 16x16 Tiles */

	ROM_REGION( 0x080000, REGION_GFX3, ROMREGION_DISPOSE )

	ROM_REGION( 0x100000, REGION_GFX4, ROMREGION_DISPOSE ) /* BG3 */
	ROM_LOAD( "um-6.2l",	0x000000, 0x080000, CRC(076be5b5) SHA1(77444025f149a960137d3c79abecf9b30defa341) )
	ROM_LOAD( "um-7.4l",    0x080000, 0x080000, CRC(d2a68cfb) SHA1(eb6cb1fad306b697b2035a31ad48e8996722a032) )

	ROM_REGION( 0x0240, REGION_USER1, 0 )
	ROM_LOAD( "um-10.2b",      0x0000, 0x0100, CRC(cfdbb86c) SHA1(588822f6308a860937349c9106c2b4b1a75823ec) )	/* unknown */
	ROM_LOAD( "um-11.2c",      0x0100, 0x0100, CRC(ff5660cf) SHA1(a4635dcf9d6dd637ea4f36f1ad233db0bd039731) )	/* unknown */
	ROM_LOAD( "um-12.20c",     0x0200, 0x0020, CRC(bdb66b02) SHA1(8755244de638d7e835e35e08c62b0612958e6ca5) )	/* unknown */
	ROM_LOAD( "um-13.10l",     0x0220, 0x0020, CRC(4ce07ec0) SHA1(5f5744ddc7f258307f036fde4c0a8e6271b2d1f9) )	/* unknown */
ROM_END

/*

Mahjong Daireikai (JPN Ver.)
(c)1989 Jaleco / NMK

*/

ROM_START( daireika )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj1.bin", 0x00001, 0x20000, CRC(3b4e8357) SHA1(1ad3e40ec6b6ff4f1c9c09d7b530f67b460151d8) )
	ROM_RELOAD(                 0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "mj2.bin", 0x00000, 0x20000, CRC(c54d2f9b) SHA1(d59fc5a9e5bbb96b3b6a43378f4f2215c368b671) )
	ROM_RELOAD(                 0x40000, 0x20000 )

	ROM_REGION( 0x140000, REGION_SOUND1, ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "mj3.bin", 0x00000, 0x80000, CRC(65bb350c) SHA1(e77866f2d612a0973adc616717e7c89a37d6c48e) )
	ROM_RELOAD(          0x40000, 0x80000 )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "mj14.bin", 0x00000, 0x10000, CRC(c84c5577) SHA1(6437368d3be39739d62158590ecd373aa070a9b2) )
	ROM_LOAD( "mj13.bin", 0x10000, 0x10000, CRC(c54bca14) SHA1(ee9c99858817aedd70bd6266b7a71c3c5ad00607) )

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "mj10.bin", 0x00000, 0x80000, CRC(1f5509a5) SHA1(4dcdee0e159956cf73f5f85ce278479be2a9ca9f) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "mj11.bin",  0x00000, 0x20000, CRC(14867c51) SHA1(b282b5048a55c9ad72ceb0d23f010a0fee78704f) )
	ROM_COPY( REGION_GFX4, 0x20000, 0x20000, 0x20000 )/*mj10.bin*/

	ROM_REGION( 0x40000, REGION_GFX2, 0 ) /* BG1 */
	ROM_COPY( REGION_GFX3, 0x00000, 0x00000, 0x20000 )/*mj10.bin*/
	ROM_LOAD( "mj12.bin", 0x20000, 0x20000, CRC(236f809f) SHA1(9e15dd8a810a9d4f7f75f084d6bd277ea7d0e40a) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Channel Zoom In (JPN Ver.)
(c)1990 Jaleco

*/

ROM_START( mjzoomin )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "zoomin-1.bin", 0x00001, 0x20000, CRC(b8b04d30) SHA1(abb163a9965421b4d92114bba974ccb13bb57f5a) )
	ROM_RELOAD(                      0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "zoomin-2.bin", 0x00000, 0x20000, CRC(c7eb982c) SHA1(9006ded2aa1fef38bde114110d76b20747c32658) )
	ROM_RELOAD(                      0x40000, 0x20000 )

	ROM_REGION( 0x140000, REGION_SOUND1, ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "zoomin-3.bin", 0x00000, 0x80000, CRC(07d7b8cd) SHA1(e05ce80ffb945b04f93f8c49d0c840b0bff6310b) )
	ROM_RELOAD(               0x40000, 0x80000 )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "zoomin14.bin", 0x00000, 0x20000, CRC(4e32aa45) SHA1(450a3449ca8b4f0dfe8b62cceaee9366eaf3dc3d) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "zoomin13.bin", 0x00000, 0x20000, CRC(888d79fe) SHA1(eb9671d4c7608edd1231dc0cae47aab2430cbd66) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "zoomin12.bin", 0x00000, 0x40000, CRC(b0b94554) SHA1(10490b7475810910140ce075e62f604b914e5511) )

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "zoomin10.bin", 0x00000, 0x80000, CRC(40aec575) SHA1(ef7a3c7a94523c5967ab774936b873c9629e0e44) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei (JPN Ver.)
(c)1990 Jaleco


*/

ROM_START( kakumei )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-re-1.bin", 0x00001, 0x20000, CRC(b90215be) SHA1(10384237f734836acefb4b5f53a6ddd9054d63ff) )
	ROM_RELOAD(                     0x40001, 0x20000 )
	ROM_LOAD16_BYTE( "mj-re-2.bin", 0x00000, 0x20000, CRC(37eff266) SHA1(1d9e88c0270daadfafff1f73eb617e77b1d199d6) )
	ROM_RELOAD(                     0x40000, 0x20000 )

	ROM_REGION( 0x140000, REGION_SOUND1, ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "rom3.bin", 0x00000, 0x40000, CRC(c9b7a526) SHA1(edec57e66d4ff601c8fdef7b1405af84a3f3d883) )
	ROM_RELOAD(           0x40000, 0x40000 )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "rom14.bin", 0x00000, 0x20000, CRC(63e88dd6) SHA1(58734c8caf1b1ddc4cf0437ffd8109292b76c4e1) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "rom13.bin", 0x00000, 0x20000, CRC(9bef4fc2) SHA1(6598ab9dba513efcda01e47cc7752b47a97f2c6a) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "rom12.bin", 0x00000, 0x40000, CRC(31620a61) SHA1(11593ca7760e1a628e63aa48d9ad3800cf7af275) )

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "rom10.bin", 0x00000, 0x80000, CRC(88366377) SHA1(163a08415a631c8a09a0a55bc2819988d850f2ad) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Mahjong Kakumei2 Princess League (JPN Ver.)
(c)1992 Jaleco

*/

ROM_START( kakumei2 )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mj-8956.1", 0x00001, 0x40000, CRC(db4ce32f) SHA1(1ae13627b9922143f462b1c3bbed87374f6e1667) )
	ROM_LOAD16_BYTE( "mj-8956.2", 0x00000, 0x40000, CRC(0f942507) SHA1(7ec2fbeb9a34dfc80c4df3de8397388db13f5c7c) )

	ROM_REGION( 0x140000, REGION_SOUND1, ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "92000-01.3", 0x00000, 0x80000, CRC(4b0ed440) SHA1(11961d217a41f92b60d5083a5e346c245f7db620) )
	ROM_RELOAD(            0x40000, 0x80000 )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "mj-8956.14", 0x00000, 0x20000, CRC(2b2fe999) SHA1(d9d601e2c008791f5bff6e7b1340f754dd094201) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "mj-8956.13", 0x00000, 0x20000, CRC(afe93cf4) SHA1(1973dc5821c6df68e20f8a84b5c9ae281dd3f85f) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "mj-8956.12", 0x00000, 0x20000, BAD_DUMP CRC(43f7853d) SHA1(54fb523b27e79aa295900c478f09cc080fea0adf) )
	/*0x20000-0x40000 used maybe above rom is bad? */

	ROM_REGION( 0x80000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "92000-02.10", 0x00000, 0x80000, CRC(338fa9b2) SHA1(05ba4b3c44249cf92be238bf53d6345dc49b0881) )

	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END

/*

Idol Janshi Su-Chi-Pi Special
(c)Jaleco 1994

CPU  : M68000P10
Sound: OKI M6295
OSC  : 12.000MHz 4.000MHz

MJ-8956
YSP-40101 171

1.bin - Main program ver.1.2 (27c2001)
2.bin - Main program ver.1.2 (27c2001)

3.bin - Sound data (27c4000)
4.bin - Sound data (27c4000)

7.bin  (27c4000) \
8.bin  (27c4000) |
9.bin  (27c4000) |
10.bin (27c4000) |
                 |- Graphics
12.bin (27c2001) |
                 |
13.bin (27c1001) |
                 |
14.bin (27c1001) /

pr92000a.prm (82s129) \
pr92000b.prm (82s129) |- Not dumped
pr93035.prm  (82s123) /

Custom chips:
GS-9000406 9345K5005 (80pin QFP) x4
GS-9000404 9248EP004 (44pin QFP)

Other chips:
MO-92000 (64pin DIP)
NEC D65012GF303 9050KX016 (80pin QFP) x4

*/

ROM_START( suchipi )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x40000, CRC(e37cc745) SHA1(73b3314d27a0332068e0d2bbc08d7401e371da1b) )
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x40000, CRC(42ecf88a) SHA1(7bb85470bc9f94c867646afeb91c4730599ea299) )

	ROM_REGION( 0x140000, REGION_SOUND1, ROMREGION_ERASEFF ) /* Samples */
	ROM_LOAD( "3.bin", 0x00000, 0x80000, CRC(691b5387) SHA1(b8bc9f904eab7653566042b18d89276d537ba586) )
	ROM_RELOAD(        0x40000, 0x80000 )
	ROM_LOAD( "4.bin", 0xc0000, 0x80000, CRC(3fe932a1) SHA1(9e768b901738ee9eba207a67c4fd19efb0035a68) )

	ROM_REGION( 0x20000, REGION_GFX1, 0 ) /* BG0 */
	ROM_LOAD( "14.bin", 0x00000, 0x20000, CRC(e465a540) SHA1(10e19599ab90b0c0b6ef6ee41f16620bd1ba6800) )

	ROM_REGION( 0x20000, REGION_GFX2, 0 ) /* BG1 */
	ROM_LOAD( "13.bin", 0x00000, 0x20000, CRC(99466044) SHA1(ca31b58a5d4656f95d80ddb9bc1f9a53f5f2446c) )

	ROM_REGION( 0x40000, REGION_GFX3, 0 ) /* BG2 */
	ROM_LOAD( "12.bin", 0x00000, 0x40000, CRC(146596eb) SHA1(f85e92e6dc9ebef5e67d28f1d450225cd2a2abaa) )

	ROM_REGION( 0x200000, REGION_GFX4, 0 ) /* BG3 */
	ROM_LOAD( "7.bin",  0x000000, 0x80000, CRC(18caf6f3) SHA1(3df6b257867487adcba1a05c8745413d9a15c3d7) )
	ROM_LOAD( "8.bin",  0x080000, 0x80000, CRC(0403399a) SHA1(8d39a68b3a1a431afe93ff485e837389a4502d0c) )
	ROM_LOAD( "9.bin",  0x100000, 0x80000, CRC(8a348246) SHA1(13516c48bdbe8d78e7517473ef2835a4dea2ce93) )
	ROM_LOAD( "10.bin", 0x180000, 0x80000, CRC(2b0d1afd) SHA1(40009b450901567052aa63c4629a2f7a10343e63) )

	/* the 3 missing proms should be the same as the other games */
	ROM_REGION( 0x220, REGION_USER1, 0 ) /* Proms */
	ROM_LOAD( "mj15.bpr", 0x000, 0x100, CRC(ebac41f9) SHA1(9d1629d977849663392cbf03a3ddf76665f88608) )
	ROM_LOAD( "mj16.bpr", 0x100, 0x100, CRC(8d5dc1f6) SHA1(9f723e7cd44f8c09ec30b04725644346484ec753) )
	ROM_LOAD( "mj17.bpr", 0x200, 0x020, CRC(a17c3e8a) SHA1(d7969fad7cec9c792c53aa457f4ad764a727e0a5) )
ROM_END


/******************************************************************************************

MCU simulations

******************************************************************************************/

static READ16_HANDLER( urashima_mcu_r )
{
	static const int resp[] = {	0x99, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x9c, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x23, 0x63, 0x00,
							0x3e, 0x7e, 0x00,
							0x35, 0x75, 0x00,
							0x21, 0x61, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

logerror("%04x: mcu_r %02x\n",activecpu_get_pc(),res);

	return res;
}

/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
static WRITE16_HANDLER( urashima_mcu_w )
{
	if(ACCESSING_BITS_0_7 && data)
	{
		//jm_regs[0x30e/2] = ?

		//popmessage("%04x %02x",jm_regs[0x030e/2],data);

		/*******************************************************
        1st M68k code uploaded by the MCU (sound prg)
        *******************************************************/
		jm_regs[0x0320/2] = 0x4ef9;
		jm_regs[0x0322/2] = 0x0010;
		jm_regs[0x0324/2] = 0x2000;//jmp $102000

		jm_mcu_code[0x2000/2] = 0x0040;
		jm_mcu_code[0x2002/2] = 0x0080;//ori $80,D0
		jm_mcu_code[0x2004/2] = 0x33c0;
		jm_mcu_code[0x2006/2] = 0x0008;
		jm_mcu_code[0x2008/2] = 0x0040;
		jm_mcu_code[0x200a/2] = 0x4e71;//0x6100;
		jm_mcu_code[0x200c/2] = 0x4e71;//0x000c;
		jm_mcu_code[0x200e/2] = 0x33fc;
		jm_mcu_code[0x2010/2] = 0x0010;
		jm_mcu_code[0x2012/2] = 0x0008;
		jm_mcu_code[0x2014/2] = 0x0040;
		jm_mcu_code[0x2016/2] = 0x4e75;
		jm_mcu_code[0x2018/2] = 0x3239;
		jm_mcu_code[0x201a/2] = 0x0008;
		jm_mcu_code[0x201c/2] = 0x0040;
		jm_mcu_code[0x201e/2] = 0x0241;
		jm_mcu_code[0x2020/2] = 0x0001;
		jm_mcu_code[0x2022/2] = 0x66f4;
		jm_mcu_code[0x2024/2] = 0x4e75;
		/*******************************************************
        1st alt M68k code uploaded by the MCU (Input test mode)
        *******************************************************/
		/*similar to mjzoomin but with offset summed with 0x300?*/
		/*tx scrollx = $200*/
		jm_regs[0x03c6/2] = 0x6008;//bra $+10
		jm_regs[0x03d0/2] = 0x4ef9;
		jm_regs[0x03d2/2] = 0x0010;
		jm_regs[0x03d4/2] = 0x0000;//jmp $100000
		jm_mcu_code[0x0000/2] = 0x33fc;
		jm_mcu_code[0x0002/2] = 0x0400;
		jm_mcu_code[0x0004/2] = 0x0008;
		jm_mcu_code[0x0006/2] = 0x0038;
		/*priority = 5(Something that shows the text layer,to be checked after that the priority works )*/
		jm_mcu_code[0x0008/2] = 0x33fc;
		jm_mcu_code[0x000a/2] = 0x0005;
		jm_mcu_code[0x000c/2] = 0x0008;
		jm_mcu_code[0x000e/2] = 0x0016;//move.w #
		jm_mcu_code[0x0010/2] = 0xd0fc;
		jm_mcu_code[0x0012/2] = 0x0060;//adda.w $60,A0
		jm_mcu_code[0x0014/2] = 0x92fc;
		jm_mcu_code[0x0016/2] = 0x0200;//suba.w $200,A1
		jm_mcu_code[0x0018/2] = 0x32d8;//move.w (A0)+,(A1)+
		jm_mcu_code[0x001a/2] = 0x51c9;
		jm_mcu_code[0x001c/2] = 0xfffc;//dbra D1,f00ca
		jm_mcu_code[0x001e/2] = 0x4e75;//rts

		/*******************************************************
        2nd M68k code uploaded by the MCU (tile upload)
        *******************************************************/
		jm_regs[0x03ca/2] = 0x4ef9;
		jm_regs[0x03cc/2] = 0x0010;
		jm_regs[0x03ce/2] = 0x0800;//jmp $100800
		jm_mcu_code[0x0800/2] = 0x32da;
		jm_mcu_code[0x0802/2] = 0x51c8;
		jm_mcu_code[0x0804/2] = 0xfffc;
		jm_mcu_code[0x0806/2] = 0x4e75;
		/*******************************************************
        3rd M68k code uploaded by the MCU (palette upload)
        *******************************************************/
		jm_regs[0x03c0/2] = 0x4ef9;
		jm_regs[0x03c2/2] = 0x0010;
		jm_regs[0x03c4/2] = 0x1000;//jmp $101000
		/*
        palette additional protection:
        A1:88200 = x:88000
        A1:88000 = x:88600
        TODO: optimize this...
        */
		jm_mcu_code[0x1000/2] = 0x92fc;
		jm_mcu_code[0x1002/2] = 0x0200;//suba.w $200,A1
		jm_mcu_code[0x1004/2] = 0xb3fc;
		jm_mcu_code[0x1006/2] = 0x0008;
		jm_mcu_code[0x1008/2] = 0x8000;
		jm_mcu_code[0x100a/2] = 0x6d00;
		jm_mcu_code[0x100c/2] = 0x003c;
		jm_mcu_code[0x100e/2] = 0x33c2;
		jm_mcu_code[0x1010/2] = 0x0010;
		jm_mcu_code[0x1012/2] = 0x17fe; //move.w D2,$1017fe
		jm_mcu_code[0x1014/2] = 0x33c1;
		jm_mcu_code[0x1016/2] = 0x0010;
		jm_mcu_code[0x1018/2] = 0x17fc; //move.w D1,$1017fc
		jm_mcu_code[0x101a/2] = 0x720f;
		jm_mcu_code[0x101c/2] = 0x740f; //moveq $07,D2
		jm_mcu_code[0x101e/2] = 0x23c8;
		jm_mcu_code[0x1020/2] = 0x0010;
		jm_mcu_code[0x1022/2] = 0x17f0;
		jm_mcu_code[0x1024/2] = 0x2050; //movea (A0),A0
		jm_mcu_code[0x1026/2] = 0x32d8;
		jm_mcu_code[0x1028/2] = 0x51ca;
		jm_mcu_code[0x102a/2] = 0xfffc;
		jm_mcu_code[0x102c/2] = 0x2079;
		jm_mcu_code[0x102e/2] = 0x0010;
		jm_mcu_code[0x1030/2] = 0x17f0;
		jm_mcu_code[0x1032/2] = 0xd0fc;
		jm_mcu_code[0x1034/2] = 0x0004;//adda.w $4,A0
		jm_mcu_code[0x1036/2] = 0x51c9;
		jm_mcu_code[0x1038/2] = 0xffe4;
		jm_mcu_code[0x103a/2] = 0x3439;
		jm_mcu_code[0x103c/2] = 0x0010;
		jm_mcu_code[0x103e/2] = 0x17fe;
		jm_mcu_code[0x1040/2] = 0x3239;
		jm_mcu_code[0x1042/2] = 0x0010;
		jm_mcu_code[0x1044/2] = 0x17fc;
		jm_mcu_code[0x1046/2] = 0x4e75;
		jm_mcu_code[0x1048/2] = 0xd2fc;
		jm_mcu_code[0x104a/2] = 0x0800;
		jm_mcu_code[0x104c/2] = 0x6000;
		jm_mcu_code[0x104e/2] = 0xffc0;
	}
}

static READ16_HANDLER( daireika_mcu_r )
{
	static const int resp[] = {	0x99, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x9c, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x23, 0x63, 0x00,
							0x3e, 0x7e, 0x00,
							0x35, 0x75, 0x00,
							0x21, 0x61, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	logerror("%04x: mcu_r %02x\n",activecpu_get_pc(),res);

	return res;
}

/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
static WRITE16_HANDLER( daireika_mcu_w )
{
	if(ACCESSING_BITS_0_7 && data)
	{
		/*MCU program upload complete/upload kind*/
		//jm_regs[0x000e/2] = 0x0005;

		/*******************************************************
        1st M68k code uploaded by the MCU.
        *******************************************************/
		jm_regs[0x0140/2] = 0x4e75; 	//rts

		/*
        jm_regs[0x0140/2] = 0x4ef9;
        jm_regs[0x0142/2] = 0x0010;
        jm_regs[0x0144/2] = 0x1000;//jmp $101000
        //jm_regs[0x00c6/2] = 0x4e75;//rts
        jm_mcu_code[0x1000/2] = 0x33c2;
        jm_mcu_code[0x1002/2] = 0x0010;
        jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
        jm_mcu_code[0x1006/2] = 0x23c8;
        jm_mcu_code[0x1008/2] = 0x0010;
        jm_mcu_code[0x100a/2] = 0x17f0;
        jm_mcu_code[0x100c/2] = 0x2050; //movea (A0),A0
        jm_mcu_code[0x100e/2] = 0x22d8;
        jm_mcu_code[0x1010/2] = 0x51ca;
        jm_mcu_code[0x1012/2] = 0xfffc;
        jm_mcu_code[0x1014/2] = 0x3439;
        jm_mcu_code[0x1016/2] = 0x0010;
        jm_mcu_code[0x1018/2] = 0x17fe;
        jm_mcu_code[0x101a/2] = 0x2079;
        jm_mcu_code[0x101c/2] = 0x0010;
        jm_mcu_code[0x101e/2] = 0x17f0;
        jm_mcu_code[0x1020/2] = 0xd0fc;
        jm_mcu_code[0x1022/2] = 0x0004;//adda.w $4,A0
        jm_mcu_code[0x1024/2] = 0x4e75;*/
		/*******************************************************
        2nd M68k code uploaded by the MCU.
        *******************************************************/
		jm_regs[0x0020/2] = 0x4ef9;
		jm_regs[0x0022/2] = 0x0010;
		jm_regs[0x0024/2] = 0x2000;//jmp $102000
		jm_mcu_code[0x2000/2] = 0x0040;
		jm_mcu_code[0x2002/2] = 0x0080;//ori $80,D0
		jm_mcu_code[0x2004/2] = 0x33c0;
		jm_mcu_code[0x2006/2] = 0x0008;
		jm_mcu_code[0x2008/2] = 0x0040;
		jm_mcu_code[0x200a/2] = 0x6100;
		jm_mcu_code[0x200c/2] = 0x000c;
		jm_mcu_code[0x200e/2] = 0x33fc;
		jm_mcu_code[0x2010/2] = 0x0010;
		jm_mcu_code[0x2012/2] = 0x0008;
		jm_mcu_code[0x2014/2] = 0x0040;
		jm_mcu_code[0x2016/2] = 0x4e75;
		jm_mcu_code[0x2018/2] = 0x3239;
		jm_mcu_code[0x201a/2] = 0x0008;
		jm_mcu_code[0x201c/2] = 0x0040;
		jm_mcu_code[0x201e/2] = 0x0241;
		jm_mcu_code[0x2020/2] = 0x0001;
		jm_mcu_code[0x2022/2] = 0x66f4;
		jm_mcu_code[0x2024/2] = 0x4e75;
		/*******************************************************
        3rd M68k code uploaded by the MCU.
        see mjzoomin_mcu_w
        *******************************************************/
		jm_regs[0x00c6/2] = 0x6000;
		jm_regs[0x00c8/2] = 0x0008;//bra +$8,needed because we have only two bytes here
		             			   //and we need three...
		jm_regs[0x00d0/2] = 0x4ef9;
		jm_regs[0x00d2/2] = 0x0010;
		jm_regs[0x00d4/2] = 0x0000;//jmp $100000
		//jm_regs[0x00cc/2] = 0x4e75;//rts //needed? we can use jmp instead of jsr...
		jm_mcu_code[0x0000/2] = 0x2050;//movea.l (A0),A0
		jm_mcu_code[0x0002/2] = 0x32d8;//move.w (A0)+,(A1)+
		jm_mcu_code[0x0004/2] = 0x51c9;
		jm_mcu_code[0x0006/2] = 0xfffc;//dbra D1,f00ca
		jm_mcu_code[0x0008/2] = 0x4e75;//rts
		/*******************************************************
        4th M68k code uploaded by the MCU
        They seem video code cleaning functions
        *******************************************************/
		//108800
		jm_regs[0x0100/2] = 0x4ef9;
		jm_regs[0x0102/2] = 0x0010;
		jm_regs[0x0104/2] = 0x8800;

		jm_mcu_code[0x8800/2] = 0x4df9;
		jm_mcu_code[0x8802/2] = 0x0009;
		jm_mcu_code[0x8804/2] = 0x0000; //lea.w #90000,A6
		jm_mcu_code[0x8806/2] = 0x323c;
		jm_mcu_code[0x8808/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x880a/2] = 0x3cbc;
		jm_mcu_code[0x880c/2] = 0x0000; //move.w #$0000,(A6)
		jm_mcu_code[0x880e/2] = 0xdcfc;
		jm_mcu_code[0x8810/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x8812/2] = 0x51c9;
		jm_mcu_code[0x8814/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x8816/2] = 0x4df9;
		jm_mcu_code[0x8818/2] = 0x0000;
		jm_mcu_code[0x881a/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x881c/2] = 0x323c;
		jm_mcu_code[0x881e/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x8820/2] = 0x4e75; //rts

		//108880
		jm_regs[0x0108/2] = 0x4ef9;
		jm_regs[0x010a/2] = 0x0010;
		jm_regs[0x010c/2] = 0x8880;

		jm_mcu_code[0x8880/2] = 0x4df9;
		jm_mcu_code[0x8882/2] = 0x0009;
		jm_mcu_code[0x8884/2] = 0x4000; //lea.w #94000,A6
		jm_mcu_code[0x8886/2] = 0x323c;
		jm_mcu_code[0x8888/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x888a/2] = 0x3cbc;
		jm_mcu_code[0x888c/2] = 0x0000; //move.w #$0000,(A6)
		jm_mcu_code[0x888e/2] = 0xdcfc;
		jm_mcu_code[0x8890/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x8892/2] = 0x51c9;
		jm_mcu_code[0x8894/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x8896/2] = 0x4df9;
		jm_mcu_code[0x8898/2] = 0x0000;
		jm_mcu_code[0x889a/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x889c/2] = 0x323c;
		jm_mcu_code[0x889e/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x88a0/2] = 0x4e75; //rts

		//108900
		jm_regs[0x0110/2] = 0x4ef9;
		jm_regs[0x0112/2] = 0x0010;
		jm_regs[0x0114/2] = 0x8900;

		jm_mcu_code[0x8900/2] = 0x4df9;
		jm_mcu_code[0x8902/2] = 0x0009;
		jm_mcu_code[0x8904/2] = 0x8000; //lea.w #98000,A6
		jm_mcu_code[0x8906/2] = 0x323c;
		jm_mcu_code[0x8908/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x890a/2] = 0x3cbc;
		jm_mcu_code[0x890c/2] = 0xf0ff; //move.w #$f0ff,(A6)
		jm_mcu_code[0x890e/2] = 0xdcfc;
		jm_mcu_code[0x8910/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x8912/2] = 0x51c9;
		jm_mcu_code[0x8914/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x8916/2] = 0x4df9;
		jm_mcu_code[0x8918/2] = 0x0000;
		jm_mcu_code[0x891a/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x891c/2] = 0x323c;
		jm_mcu_code[0x891e/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x8920/2] = 0x4e75; //rts

		/*TX function?*/
		jm_regs[0x0126/2] = 0x4ef9;
		jm_regs[0x0128/2] = 0x0010;
		jm_regs[0x012a/2] = 0x8980;

		//pri $f0590
		jm_mcu_code[0x8980/2] = 0x33fc;
		jm_mcu_code[0x8982/2] = 0x0006;
		jm_mcu_code[0x8984/2] = 0x000f;
		jm_mcu_code[0x8986/2] = 0x0590; //move.w #$6,$f0590 (pri n)
		jm_mcu_code[0x8988/2] = 0x4df9;
		jm_mcu_code[0x898a/2] = 0x0009;
		jm_mcu_code[0x898c/2] = 0xc000; //lea.w #9c000,A6
		jm_mcu_code[0x898e/2] = 0x323c;
		jm_mcu_code[0x8990/2] = 0x1fff; //move.w #$1fff,D1
		jm_mcu_code[0x8992/2] = 0x3cbc;
		jm_mcu_code[0x8994/2] = 0x0020; //move.w #$0020,(A6)
		jm_mcu_code[0x8996/2] = 0xdcfc;
		jm_mcu_code[0x8998/2] = 0x0002; //adda.w #$0002,A6
		jm_mcu_code[0x899a/2] = 0x51c9;
		jm_mcu_code[0x899c/2] = 0xfff6; //dbra D1
		jm_mcu_code[0x899e/2] = 0x4df9;
		jm_mcu_code[0x89a0/2] = 0x0000;
		jm_mcu_code[0x89a2/2] = 0x0000; //lea.w #0,A6
		jm_mcu_code[0x89a4/2] = 0x323c;
		jm_mcu_code[0x89a6/2] = 0x0000; //move.w #$0,D1
		jm_mcu_code[0x89a8/2] = 0x4e75; //rts

/*
        jm_regs[0x0100/2] = 0x4ef9;
        jm_regs[0x0102/2] = 0x0010;
        jm_regs[0x0104/2] = 0x1000;//jmp $101000
        //jm_regs[0x00c6/2] = 0x4e75;//rts
        jm_mcu_code[0x1000/2] = 0x33c2;
        jm_mcu_code[0x1002/2] = 0x0010;
        jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
        jm_mcu_code[0x1006/2] = 0x23c8;
        jm_mcu_code[0x1008/2] = 0x0010;
        jm_mcu_code[0x100a/2] = 0x17f0;
        jm_mcu_code[0x100c/2] = 0x2050; //movea (A0),A0
        jm_mcu_code[0x100e/2] = 0x22d8;
        jm_mcu_code[0x1010/2] = 0x51ca;
        jm_mcu_code[0x1012/2] = 0xfffc;
        jm_mcu_code[0x1014/2] = 0x3439;
        jm_mcu_code[0x1016/2] = 0x0010;
        jm_mcu_code[0x1018/2] = 0x17fe;
        jm_mcu_code[0x101a/2] = 0x2079;
        jm_mcu_code[0x101c/2] = 0x0010;
        jm_mcu_code[0x101e/2] = 0x17f0;
        jm_mcu_code[0x1020/2] = 0xd0fc;
        jm_mcu_code[0x1022/2] = 0x0004;//adda.w $4,A0
        jm_mcu_code[0x1024/2] = 0x4e75;*/
		/*******************************************************
        5th M68k code uploaded by the MCU
        *******************************************************/
		jm_regs[0x00c0/2] = 0x4ef9;
		jm_regs[0x00c2/2] = 0x0010;
		jm_regs[0x00c4/2] = 0x1000;//jmp $101000
		//jm_regs[0x00c6/2] = 0x4e75;//rts
		jm_mcu_code[0x1000/2] = 0x33c2;
		jm_mcu_code[0x1002/2] = 0x0010;
		jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
		jm_mcu_code[0x1006/2] = 0x33c1;
		jm_mcu_code[0x1008/2] = 0x0010;
		jm_mcu_code[0x100a/2] = 0x17fc; //move.w D1,$1017fc
		jm_mcu_code[0x100c/2] = 0x720f;
		jm_mcu_code[0x100e/2] = 0x740f; //moveq $07,D2
		jm_mcu_code[0x1010/2] = 0x23c8;
		jm_mcu_code[0x1012/2] = 0x0010;
		jm_mcu_code[0x1014/2] = 0x17f0;
		jm_mcu_code[0x1016/2] = 0x2050; //movea (A0),A0
		jm_mcu_code[0x1018/2] = 0x32d8;
		jm_mcu_code[0x101a/2] = 0x51ca;
		jm_mcu_code[0x101c/2] = 0xfffc;
		jm_mcu_code[0x101e/2] = 0x2079;
		jm_mcu_code[0x1020/2] = 0x0010;
		jm_mcu_code[0x1022/2] = 0x17f0;
		jm_mcu_code[0x1024/2] = 0xd0fc;
		jm_mcu_code[0x1026/2] = 0x0004;//adda.w $4,A0
		jm_mcu_code[0x1028/2] = 0x51c9;
		jm_mcu_code[0x102a/2] = 0xffe4;
		jm_mcu_code[0x102c/2] = 0x3439;
		jm_mcu_code[0x102e/2] = 0x0010;
		jm_mcu_code[0x1030/2] = 0x17fe;
		jm_mcu_code[0x1032/2] = 0x3239;
		jm_mcu_code[0x1034/2] = 0x0010;
		jm_mcu_code[0x1036/2] = 0x17fc;
		jm_mcu_code[0x1038/2] = 0x4e75;
		/*******************************************************
        6th M68k code uploaded by the MCU (tile upload)
        *******************************************************/
		jm_regs[0x00ca/2] = 0x4ef9;
		jm_regs[0x00cc/2] = 0x0010;
		jm_regs[0x00ce/2] = 0x1800;//jmp $101800
		//jm_regs[0x00c6/2] = 0x4e75;//rts

		jm_mcu_code[0x1800/2] = 0x22da;//move.l (A2)+,(A1)+
		jm_mcu_code[0x1802/2] = 0xb5fc;
		jm_mcu_code[0x1804/2] = 0x0002;
		jm_mcu_code[0x1806/2] = 0x6600;
		jm_mcu_code[0x1808/2] = 0x6706;
		jm_mcu_code[0x180a/2] = 0x51c8;
		jm_mcu_code[0x180c/2] = 0xfff4;//dbra D0,f00ca
		jm_mcu_code[0x180e/2] = 0x4e75;//rts
		jm_mcu_code[0x1810/2] = 0xd4fc;
		jm_mcu_code[0x1812/2] = 0x0a00;
		jm_mcu_code[0x1814/2] = 0x60ea;
	}
}

static READ16_HANDLER( mjzoomin_mcu_r )
{
	static const int resp[] = {	0x9c, 0xd8, 0x00,
							0x2a, 0x6a, 0x00,
							0x99, 0xd8, 0x00,
							0x2f, 0x6f, 0x00,
							0x22, 0x62, 0x00,
							0x25, 0x65, 0x00,
							0x35, 0x75, 0x00,
							0x36, 0x36, 0x00,
							0x21, 0x61, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	logerror("%04x: mcu_r %02x\n",activecpu_get_pc(),res);

	return res;
}

/*4eb9*/
/*
data value is REQ under mjzoomin video test menu.It is related to the MCU?
*/
static WRITE16_HANDLER( mjzoomin_mcu_w )
{
	if(ACCESSING_BITS_0_7 && data)
	{
		/*******************************************************
        1st M68k code uploaded by the MCU(Service Mode PC=2a56).
        Program passes some parameters before entering into the sub-routine (jsr)
        D1 = 0xf
        A0 = 1026e
        A1 = 88600
        (A0) is the vector number for take the real palette address.
        *******************************************************/
		jm_regs[0x00c6/2] = 0x4ef9;
		jm_regs[0x00c8/2] = 0x0010;
		jm_regs[0x00ca/2] = 0x0000;//jsr $100000
		//jm_regs[0x00cc/2] = 0x4e75;//rts //needed? we can use jmp instead of jsr...
		jm_mcu_code[0x0000/2] = 0x2050;//movea.l (A0),A0
		jm_mcu_code[0x0002/2] = 0x32d8;//move.w (A0)+,(A1)+
		jm_mcu_code[0x0004/2] = 0x51c9;
		jm_mcu_code[0x0006/2] = 0xfffc;//dbra D1,f00ca
		jm_mcu_code[0x0008/2] = 0x4e75;//rts
		/*******************************************************
        2nd M68k code uploaded by the MCU (Sound read/write)
        (Note:copied from suchipi,sound makes the game slower
        so I think I'm missing something here)
        *******************************************************/
		jm_regs[0x0020/2] = 0x4ef9;
		jm_regs[0x0022/2] = 0x0010;
		jm_regs[0x0024/2] = 0x1800;//jmp $101800
		jm_mcu_code[0x1800/2] = 0x0040;
		jm_mcu_code[0x1802/2] = 0x0080;//ori $80,D0
		jm_mcu_code[0x1804/2] = 0x33c0;
		jm_mcu_code[0x1806/2] = 0x0008;
		jm_mcu_code[0x1808/2] = 0x0040;
		jm_mcu_code[0x180a/2] = 0x4e71;//= 0x6100;
		jm_mcu_code[0x180c/2] = 0x4e71;//= 0x000c;
		jm_mcu_code[0x180e/2] = 0x33fc;
		jm_mcu_code[0x1810/2] = 0x0010;
		jm_mcu_code[0x1812/2] = 0x0008;
		jm_mcu_code[0x1814/2] = 0x0040;
		jm_mcu_code[0x1816/2] = 0x4e75;
		jm_mcu_code[0x1818/2] = 0x3239;
		jm_mcu_code[0x181a/2] = 0x0008;
		jm_mcu_code[0x181c/2] = 0x0040;
		jm_mcu_code[0x181e/2] = 0x0241;
		jm_mcu_code[0x1820/2] = 0x0001;
		jm_mcu_code[0x1822/2] = 0x66f4;
		jm_mcu_code[0x1824/2] = 0x4e75;
		/*******************************************************
        3rd M68k code uploaded by the MCU(palette upload, 99,(9)%
        sure on this ;-)
        *******************************************************/
		jm_regs[0x00c0/2] = 0x4ef9;
		jm_regs[0x00c2/2] = 0x0010;
		jm_regs[0x00c4/2] = 0x1000;//jmp $101000
		//jm_regs[0x00c6/2] = 0x4e75;//rts
		jm_mcu_code[0x1000/2] = 0x33c2;
		jm_mcu_code[0x1002/2] = 0x0010;
		jm_mcu_code[0x1004/2] = 0x17fe; //move.w D2,$1017fe
		jm_mcu_code[0x1006/2] = 0x33c1;
		jm_mcu_code[0x1008/2] = 0x0010;
		jm_mcu_code[0x100a/2] = 0x17fc; //move.w D1,$1017fc
		jm_mcu_code[0x100c/2] = 0x720f;
		jm_mcu_code[0x100e/2] = 0x740f; //moveq $07,D2
		jm_mcu_code[0x1010/2] = 0x23c8;
		jm_mcu_code[0x1012/2] = 0x0010;
		jm_mcu_code[0x1014/2] = 0x17f0;
		jm_mcu_code[0x1016/2] = 0x2050; //movea (A0),A0
		jm_mcu_code[0x1018/2] = 0x32d8;
		jm_mcu_code[0x101a/2] = 0x51ca;
		jm_mcu_code[0x101c/2] = 0xfffc;
		jm_mcu_code[0x101e/2] = 0x2079;
		jm_mcu_code[0x1020/2] = 0x0010;
		jm_mcu_code[0x1022/2] = 0x17f0;
		jm_mcu_code[0x1024/2] = 0xd0fc;
		jm_mcu_code[0x1026/2] = 0x0004;//adda.w $4,A0
		jm_mcu_code[0x1028/2] = 0x51c9;
		jm_mcu_code[0x102a/2] = 0xffe4;
		jm_mcu_code[0x102c/2] = 0x3439;
		jm_mcu_code[0x102e/2] = 0x0010;
		jm_mcu_code[0x1030/2] = 0x17fe;
		jm_mcu_code[0x1032/2] = 0x3239;
		jm_mcu_code[0x1034/2] = 0x0010;
		jm_mcu_code[0x1036/2] = 0x17fc;
		jm_mcu_code[0x1038/2] = 0x4e75;
	}
}

static READ16_HANDLER( kakumei_mcu_r )
{
	static const int resp[] = {	0x8a, 0xd8, 0x00,
							0x3c, 0x7c, 0x00,
							0x99, 0xd8, 0x00,
							0x25, 0x65, 0x00,
							0x36, 0x76, 0x00,
							0x35, 0x75, 0x00,
							0x2f, 0x6f, 0x00,
							0x31, 0x71, 0x00,
							0x3e, 0x7e, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	//popmessage("%04x: mcu_r %02x",activecpu_get_pc(),res);

	return res;
}

static READ16_HANDLER( suchipi_mcu_r )
{
	static const int resp[] = {	0x8a, 0xd8, 0x00,
							0x3c, 0x7c, 0x00,
							0x99, 0xd8, 0x00,
							0x25, 0x65, 0x00,
							0x36, 0x76, 0x00,
							0x35, 0x75, 0x00,
							0x2f, 0x6f, 0x00,
							0x31, 0x71, 0x00,
							0x3e, 0x7e, 0x00 };
	int res;

	res = resp[respcount++];
	if (respcount >= sizeof(resp)/sizeof(resp[0])) respcount = 0;

	//popmessage("%04x: mcu_r %02x",activecpu_get_pc(),res);

	return res;
}

static DRIVER_INIT( urashima )
{
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, urashima_mcu_r );
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80012, 0x80013, 0, 0, urashima_mcu_w );
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf0320, 0xf0321, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0xf0322 - 0xf0320));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf03c0, 0xf03c5, 0, 0, SMH_BANK2, SMH_BANK2 );
	memory_set_bankptr(2, auto_malloc(0xf03c6 - 0xf03c0));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf03c6, 0xf03e5, 0, 0, SMH_BANK3, SMH_BANK3 );
	memory_set_bankptr(3, auto_malloc(0xf03e6 - 0xf03c6));
	mcu_prg = 0x12;
}

static DRIVER_INIT( daireika )
{
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, daireika_mcu_r );
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80012, 0x80013, 0, 0, daireika_mcu_w );
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf0140, 0xf0141, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0xf0142 - 0xf0140));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf0020, 0xf0025, 0, 0, SMH_BANK2, SMH_BANK2 );
	memory_set_bankptr(2, auto_malloc(0xf0026 - 0xf0020));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf00c0, 0xf00d5, 0, 0, SMH_BANK3, SMH_BANK3 );
	memory_set_bankptr(3, auto_malloc(0xf00d6 - 0xf00c0));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf0100, 0xf0131, 0, 0, SMH_BANK4, SMH_BANK4 );
	memory_set_bankptr(4, auto_malloc(0xf0132 - 0xf0100));
	mcu_prg = 0x11;
}

static DRIVER_INIT( mjzoomin )
{
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, mjzoomin_mcu_r );
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80012, 0x80013, 0, 0, mjzoomin_mcu_w );
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf00c0, 0xf00c5, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0xf00c6 - 0xf00c0));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf00c6, 0xf00d1, 0, 0, SMH_BANK2, SMH_BANK2 );
	memory_set_bankptr(2, auto_malloc(0xf00d2 - 0xf00c6));
	memory_install_readwrite16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xf0020, 0xf002f, 0, 0, SMH_BANK3, SMH_BANK3 );
	memory_set_bankptr(3, auto_malloc(0xf0030 - 0xf0020));
	mcu_prg = 0x13;
}

static DRIVER_INIT( kakumei )
{
	memory_install_read16_handler(machine, 0,  ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, kakumei_mcu_r );
	mcu_prg = 0x21;
}

static DRIVER_INIT( kakumei2 )
{
	memory_install_read16_handler(machine, 0,  ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, kakumei_mcu_r );
	mcu_prg = 0x22;
}

static DRIVER_INIT( suchipi )
{
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x80004, 0x80005, 0, 0, suchipi_mcu_r );
	mcu_prg = 0x23;
}

/*First version of the MCU*/
/*Pseudo-random stuff is not 100% working*/
GAME( 1989, daireika, 0, jalmah, jalmah, daireika, ROT0, "Jaleco / NMK", "Mahjong Daireikai",			        GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
/*Protection controls tilemap scrolling on this,you can't see anything unless you use fake scroll register values...*/
GAME( 1989, urashima, 0, jalmah, jalmah, urashima, ROT0, "UPL",	      "Urashima Mahjong", 					GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

/*No known gameplay bugs for the following games:*/
GAME( 1990, mjzoomin, 0, jalmah, jalmah, mjzoomin, ROT0, "Jaleco",       "Mahjong Channel Zoom In",            GAME_IMPERFECT_GRAPHICS )
/*Second version of the MCU*/
GAME( 1990, kakumei,  0, jalmah, jalmah2, kakumei,  ROT0, "Jaleco",       "Mahjong Kakumei",                      GAME_IMPERFECT_GRAPHICS )
GAME( 1992, kakumei2, 0, jalmah, jalmah2a, kakumei2, ROT0, "Jaleco",       "Mahjong Kakumei 2 - Princess League",  GAME_IMPERFECT_GRAPHICS )
GAME( 1993, suchipi,  0, jalmah, jalmah2a, suchipi,  ROT0, "Jaleco",       "Idol Janshi Su-Chi-Pie Special",       GAME_IMPERFECT_GRAPHICS )
