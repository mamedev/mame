/* raiden 2 board test note 17/04/08 (based on test by dox)

 rom banking is at 6c9, bit 0x80
  -- the game only writes this directly at startup, must be written indirectly by
     one of the protection commands? or mirrored?
  value of 0x80 puts 0x00000-0x1ffff at 0x20000 - 0x3ffff
  value of 0x00 puts 0x20000-0x3ffff at 0x20000 - 0x3ffff

*/

/*
Raiden DX
Seibu Kaihatsu, 1994

This readme covers Raiden DX and to some extent Raiden II
which uses an almost identical PCB.

PCB Layout
----------

(C) 1993 RAIDEN II DX SEIBU KAIHATSU INC.,o
|----------------------------------------------------------|
|      1    2   3   4   5   6    7      8      9     10    |
|LA4460    M6295  PCM  Z80     6116                       A|
|   YM2151 M6295   6    5      6116    28.63636MHz        B|
|     VOL   YM3012                                         |
|HB-45A            |------|                               C|
|HB-2       4560   |SIE150| 6116      |---------|          |
|RC220             |      | 6116      | SEI252  |         D|
|                  |------| 6116      |SB05-106 |          |
|                           6116      |(QFP208) |         E|
|J                                    |         |         F|
|A    DSW2(8)                         |---------|          |
|M                                                        G|
|M    DSW1(8)                                   CXK58258   |
|A          |---------|OBJ-1    OBJ-2           CXK58258  H|
|           | SEI360  |                         CXK58258  J|
|           |SB06-1937|DX_OBJ-3 DX_OBJ-4        CXK58258  K|
|           |(QFP160) |  PAL1               |---------|   L|
|           |         |                     |SEI1000  |   M|
| |------|  |---------|  1H      3H         |SB01-001 |   N|
| |SEI200|         32MHz                    |(QFP184) |    |
| |      |CY7C185        2H      4H         |         |   P|
| |------|CY7C185                           |---------|    |
|                                                         Q|
|                        PAL2 PAL3             |----|     R|
|                                              |V30 |      |
| DX_BACK-1  DX_BACK-2   7   COPX-D2           |----|     S|
|----------------------------------------------------------|
Notes:
      V30 clock    - 16.000MHz [32/2]. Chip is stamped "NEC D70116HG-16 V30 NEC '84" (QFP52)
      Z80 clock    - 3.579545MHz [28.63636/8]
      YM2151 clock - 3.579545MHz [28.63636/8]
      M6295 clocks - 1.022MHz [28.63636/28] and pin 7 HIGH (both)
      CXK58258     - Sony CXK58258 32k x8 SRAM (= 62256)
      CY7C185      - Cypress CY7C185 8k x8 SRAM (= 6264)
      6116         - 2k x8 SRAM
      HB-45A       - Seibu custom ceramic module sound DAC (SIP20)
      HB-2         - Seibu custom ceramic module connected to coin counters (SIP10)
      RC220        - Custom resistor network module used for inputs (SIP14)
      VSync        - 55.4859Hz  \
      HSync        - 15.5586kHz / measured via EL4583
      PAL1         - AMI 18CV8 stamped 'JJ5004' (DIP20)
      PAL2         - AMI 18CV8 stamped 'JJ5002' (DIP20)
      PAL3         - AMI 18CV8 stamped 'JJ5001' (DIP20)
      ROMs         - *PCM      - 2M MaskROM stamped 'RAIDEN 2 PCM' at location U1018 (DIP32)
                     6         - 27C020 EPROM labelled 'SEIBU 6' at location U1017 (DIP32)
                     5         - 27C512 EPROM labelled 'SEIBU 5' at location U1110 (DIP28)
                     *OBJ-1    - 16M MaskROM stamped 'RAIDEN 2 OBJ-1' at location U0811 (DIP42)
                     *OBJ-2    - 16M MaskROM stamped 'RAIDEN 2 OBJ-2' at location U082 (DIP42)
                     DX_OBJ-3  - 16M MaskROM stamped 'DX OBJ-3' at location U0837 (DIP42)
                     DX_OBJ-4  - 16M MaskROM stamped 'DX OBJ-4' at location U0836 (DIP42)
                     1H        - 27C4001 EPROM labelled 'SEIBU 1H' at location U1210 (DIP32)
                     2H        - 27C4001 EPROM labelled 'SEIBU 2H' at location U1211 (DIP32)
                     3H        - 27C4001 EPROM labelled 'SEIBU 3H' at location U129 (DIP32)
                     4H        - 27C4001 EPROM labelled 'SEIBU 4H' at location U1212 (DIP32)
                     DX_BACK-1 - 16M MaskROM stamped 'DX BACK-1' at location U075 (DIP42)
                     DX_BACK-2 - 16M MaskROM stamped 'DX BACK-2' at location U0714 (DIP42)
                     7         - 27C210 EPROM labelled 'SEIBU 7' at location U0724 (DIP40)
                     *COPX-D2  - 2M MaskROM stamped 'COPX-D2' at location U0313 (DIP40)

                     * = these ROMs are soldered-in and match ROMs from the original Raiden II PCB

      SEIBU Custom ICs -
                        SIE150 (QFP100)
                        SEI252 SB05-106 (QFP208)
                        SEI0200 TC110G21AF 0076 (QFP100)
                        SEI360 SB06-1937 (QFP160)
                        SEI1000 SB01-001 (QFP184)


Games on this PCB / Similar PCBs
 Raiden 2
 Raiden DX
 Zero Team

 + varients

Some of these games were also released on updated PCBs
which usually featured vastly inferior sound hardware
 (see the V33 based version of Raiden II/DX New)


Protection Notes:
 These games use the 2nd (and 3rd) generation of Seibu's 'COP' protection,
 utilizing the external 'COPX_D2' and 'COPX_D3' lookup roms (probably for
 math operations)  These chips, marked (c)1992 RISE Corp. are not thought
 to be the actual MCU which is probably internal to one of the Seibu
 customs.

 The games in legionna.c use the same protection chips.

Current Problem(s) - in order of priority

 High Priority

 ROM banking - we don't know where the ROM bank registers are, this causes
 serious problems as it's hard to see which glitches are caused by
 protection, and which are caused by a lack of ROM banking.

 Protection - it isn't emulated, until it is the games will never work.

 Sprite Encryption - this is 99% complete for Raiden 2 / DX, just a few bad
 bits remain.  No decryption support for Zero Team yet.

 Sound - the main sets should use a variant of the Seibu Sound System, with
 an extra OKI6295, currently no sound is hooked up in these sets.  The V33
 set uses weaker sound hardware which is emulated.

 Video emulation - used to be more complete than it is now, tile banking is
 currently broken.

 Low Priority


*/

#include "driver.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "machine/seicop.h"
#include "includes/raiden2.h"


static tilemap_t *background_layer,*midground_layer,*foreground_layer,*text_layer;
static UINT16 *back_data,*fore_data,*mid_data, *w1ram;
static int bg_bank, fg_bank, mid_bank;
static int bg_col, fg_col, mid_col;

static int tick;
static UINT16 *mainram;

static int c_r[0xc000], c_w[0xc000];

static void combine32(UINT32 *val, int offset, UINT16 data, UINT16 mem_mask)
{
	UINT16 *dest = (UINT16 *)val + BYTE_XOR_LE(offset);
	COMBINE_DATA(dest);
}



/* SPRITE DRAWING (move to video file) */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect ,int pri_mask )
{
	const UINT16 *source = machine->generic.spriteram.u16 + 0x1000/2 - 4;
	const UINT16 *finish = machine->generic.spriteram.u16;

	const gfx_element *gfx = machine->gfx[2];

//  static int ytlim = 1;
//  static int xtlim = 1;

//  if ( input_code_pressed_once(machine, KEYCODE_Q) ) ytlim--;
//  if ( input_code_pressed_once(machine, KEYCODE_W) ) ytlim++;

//  if ( input_code_pressed_once(machine, KEYCODE_A) ) xtlim--;
//  if ( input_code_pressed_once(machine, KEYCODE_S) ) xtlim++;


	/*00 ???? ????  (colour / priority?)
      01 fhhh Fwww   h = height f=flipy w = width F = flipx
      02 nnnn nnnn   n = tileno
      03 nnnn nnnn   n = tile no
      04 xxxx xxxx   x = xpos
      05 xxxx xxxx   x = xpos
      06 yyyy yyyy   y = ypos
      07 yyyy yyyy   y = ypos

     */


	while( source>finish ){
		int tile_number = source[1];
		int sx = source[2];
		int sy = source[3];
		int colr;
		int xtiles, ytiles;
		int ytlim, xtlim;
		int xflip, yflip;
		int xstep, ystep;

		if (sx & 0x8000) sx -= 0x10000;
		if (sy & 0x8000) sy -= 0x10000;


		ytlim = (source[0] >> 12) & 0x7;
		xtlim = (source[0] >> 8) & 0x7;

		xflip = (source[0] >> 15) & 0x1;
		yflip = (source[0] >> 11) & 0x1;

		colr = source[0] & 0x3f;

		ytlim += 1;
		xtlim += 1;

		sx += 32;

		xstep = 16;
		ystep = 16;

		if (xflip)
		{
			ystep = -16;
			sy += ytlim*16-16;
		}

		if (yflip)
		{
			xstep = -16;
			sx += xtlim*16-16;
		}

		for (xtiles = 0; xtiles < xtlim; xtiles++)
		{
			for (ytiles = 0; ytiles < ytlim; ytiles++)
			{
				drawgfx_transpen(
						bitmap,
						cliprect,
						gfx,
						tile_number,
						colr,
						yflip,xflip,
						sx+xstep*xtiles,sy+ystep*ytiles,15);

				tile_number++;
			}
		}

		source-=4;
	}

}

/* VIDEO RELATED WRITE HANDLERS (move to video file) */

static WRITE16_HANDLER(raiden2_background_w)
{
	COMBINE_DATA(&back_data[offset]);
	tilemap_mark_tile_dirty(background_layer, offset);
}

static WRITE16_HANDLER(raiden2_midground_w)
{
	COMBINE_DATA(&mid_data[offset]);
	tilemap_mark_tile_dirty(midground_layer,offset);
}

static WRITE16_HANDLER(raiden2_foreground_w)
{
	COMBINE_DATA(&fore_data[offset]);
	tilemap_mark_tile_dirty(foreground_layer,offset);
}

static WRITE16_HANDLER(raiden2_text_w)
{
	COMBINE_DATA(&space->machine->generic.videoram.u16[offset]);
	tilemap_mark_tile_dirty(text_layer, offset);
}

/* TILEMAP RELATED (move to video file) */

static TILE_GET_INFO( get_back_tile_info )
{
	int tile = back_data[tile_index];
	int color = (tile >> 12) | (bg_col << 4);

	tile = (tile & 0xfff) | (bg_bank << 12);

	SET_TILE_INFO(1,tile+0x0000,color,0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	int tile = mid_data[tile_index];
	int color = (tile >> 12) | (mid_col << 4);

	tile = (tile & 0xfff) | (mid_bank << 12);

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	int tile = fore_data[tile_index];
	int color = (tile >> 12) | (fg_col << 4);


	tile = (tile & 0xfff) | (fg_bank << 12);
	//  tile = (tile & 0xfff) | (3<<12);  // 3000 intro (cliff) 1000 game (bg )

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	int tile = machine->generic.videoram.u16[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(0,tile,color,0);
}

#if 0
static void set_scroll(tilemap_t *tm, int plane)
{
	int x = mainram[0x620/2+plane*2+0];
	int y = mainram[0x620/2+plane*2+1];
	tilemap_set_scrollx(tm, 0, x);
	tilemap_set_scrolly(tm, 0, y);
}
#endif
/* VIDEO START (move to video file) */

static VIDEO_START( raiden2 )
{
	text_layer       = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows,  8, 8, 64,32 );
	background_layer = tilemap_create(machine, get_back_tile_info, tilemap_scan_rows, 16,16, 32,32 );
	midground_layer  = tilemap_create(machine, get_mid_tile_info,  tilemap_scan_rows, 16,16, 32,32 );
	foreground_layer = tilemap_create(machine, get_fore_tile_info, tilemap_scan_rows, 16,16, 32,32 );

	tilemap_set_transparent_pen(midground_layer, 15);
	tilemap_set_transparent_pen(foreground_layer, 15);
	tilemap_set_transparent_pen(text_layer, 15);

	tick = 0;
}

// XXX
// 000 bg = dis, mid = dis, fg = 4.012 // push 1/2 player button
// 002 bg = 0.0, mid = 1.2, fg = 6.1,  // level 1 start
// 000 bg = 0.0, mid = 1.2, fg = 6.1,  // level 1 run
// 013 bg = 0.0, mid = 3.0, fg = 7.1   // attract mode p1
// 012 bg = 0.0, mid = 3.0, fg = 6.1   // attract mode p1
// 000 bg = 0.0, mid = 1.1, fg = 5.1   // attract mode level 1
// 002 bg = 0.0, mid = dis, fg = 6.1   // high scores

/* VIDEO UPDATE (move to video file) */

static VIDEO_UPDATE ( raiden2 )
{

#if 0
	int info_1, info_2, info_3;

#if 0
	static UINT8 zz[16];
	static UINT8 zz2[32];
	static UINT8 zz3[12];
#endif

	info_1 = mainram[0x6cc/2] & 1;
	info_2 = (mainram[0x6cc/2] & 2)>>1;
	info_3 = (mainram[0x470/2] & 0xc000)>>14;

#if 1
	set_scroll(background_layer, 0);
	set_scroll(midground_layer,  1);
	set_scroll(foreground_layer, 2);
#endif

#if 0
	{
		int new_bank;
		new_bank = 0 | (mainram[0x6cc/2]&1);
		if(new_bank != bg_bank) {
			bg_bank = new_bank;
			tilemap_mark_all_tiles_dirty(background_layer);
		}

		new_bank = 2 | ((mainram[0x6cc/2]>>1)&1);
		if(new_bank != mid_bank) {
			mid_bank = new_bank;
			tilemap_mark_all_tiles_dirty(midground_layer);
		}

		new_bank = 4 | (mainram[0x470]>>14);
		if(new_bank != fg_bank) {
			fg_bank = new_bank;
			tilemap_mark_all_tiles_dirty(foreground_layer);
		}

		bg_col = 0;
		mid_col = 2;
		fg_col = 1;
	}
#endif
#if 0
	if(0 && memcmp(zz, mainram+0x620/2, 16)) {
		logerror("0:%04x %04x  1:%04x %04x 2:%04x %04x 3:%04x %04x\n",
				 mainram[0x622/2],
				 mainram[0x620/2],
				 mainram[0x626/2],
				 mainram[0x624/2],
				 mainram[0x62a/2],
				 mainram[0x628/2],
				 mainram[0x62e/2],
				 mainram[0x62c/2]);
		memcpy(zz, mainram+0x620/2, 16);
	}
	if(0 && memcmp(zz2, mainram+0x470/2, 32)) {
		logerror("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				 mainram[0x470/2] & 0xff, mainram[0x471/2] >> 8, mainram[0x472/2] & 0xff, mainram[0x473/2] >> 8,
				 mainram[0x474/2] & 0xff, mainram[0x475/2] >> 8, mainram[0x476/2] & 0xff, mainram[0x477/2] >> 8,
				 mainram[0x478/2] & 0xff, mainram[0x479/2] >> 8, mainram[0x47a/2] & 0xff, mainram[0x47b/2] >> 8,
				 mainram[0x47c/2] & 0xff, mainram[0x47d/2] >> 8, mainram[0x47e/2] & 0xff, mainram[0x47f/2] >> 8,
				 mainram[0x480/2] & 0xff, mainram[0x481/2] >> 8, mainram[0x482/2] & 0xff, mainram[0x483/2] >> 8,
				 mainram[0x484/2] & 0xff, mainram[0x485/2] >> 8, mainram[0x486/2] & 0xff, mainram[0x487/2] >> 8,
				 mainram[0x488/2] & 0xff, mainram[0x489/2] >> 8, mainram[0x48a/2] & 0xff, mainram[0x48b/2] >> 8,
				 mainram[0x48c/2] & 0xff, mainram[0x48d/2] >> 8, mainram[0x48e/2] & 0xff, mainram[0x48f/2] >> 8);
		memcpy(zz2, mainram+0x470/2, 32);
	}

#endif
#if 0
	if(memcmp(zz3, mainram+0x620/2, 12)) {
		logerror("%04x %04x  %04x %04x  %04x %04x\n",
				 mainram[0x622/2], mainram[0x620/2],
				 mainram[0x626/2], mainram[0x624/2],
				 mainram[0x62a/2], mainram[0x628/2]);
		memcpy(zz3, mainram+0x620/2, 12);
	}
#endif
#if 1
	tick++;
	if(tick == 15) {
		int mod = 0;
		tick = 0;

		if(input_code_pressed(screen->machine, KEYCODE_R)) {
			mod = 1;
			bg_bank++;
			if(bg_bank == 8)
				bg_bank = 0;
		}
		if(input_code_pressed(screen->machine, KEYCODE_T)) {
			mod = 1;
			if(bg_bank == 0)
				bg_bank = 8;
			bg_bank--;
		}
		if(input_code_pressed(screen->machine, KEYCODE_Y)) {
			mod = 1;
			mid_bank++;
			if(mid_bank == 8)
				mid_bank = 0;
		}
		if(input_code_pressed(screen->machine, KEYCODE_U)) {
			mod = 1;
			if(mid_bank == 0)
				mid_bank = 8;
			mid_bank--;
		}
		if(input_code_pressed(screen->machine, KEYCODE_O)) {
			mod = 1;
			fg_bank++;
			if(fg_bank == 8)
				fg_bank = 0;
		}
		if(input_code_pressed(screen->machine, KEYCODE_I)) {
			mod = 1;
			if(fg_bank == 0)
				fg_bank = 8;
			fg_bank--;
		}

		if(input_code_pressed(screen->machine, KEYCODE_D)) {
			mod = 1;
			bg_col++;
			if(bg_col == 8)
				bg_col = 0;
		}
		if(input_code_pressed(screen->machine, KEYCODE_F)) {
			mod = 1;
			if(bg_col == 0)
				bg_col = 8;
			bg_col--;
		}
		if(input_code_pressed(screen->machine, KEYCODE_G)) {
			mod = 1;
			mid_col++;
			if(mid_col == 8)
				mid_col = 0;
		}
		if(input_code_pressed(screen->machine, KEYCODE_H)) {
			mod = 1;
			if(mid_col == 0)
				mid_col = 8;
			mid_col--;
		}
		if(input_code_pressed(screen->machine, KEYCODE_J)) {
			mod = 1;
			fg_col++;
			if(fg_col == 8)
				fg_col = 0;
		}
		if(input_code_pressed(screen->machine, KEYCODE_K)) {
			mod = 1;
			if(fg_col == 0)
				fg_col = 8;
			fg_col--;
		}
		if(mod || input_code_pressed(screen->machine, KEYCODE_L)) {
			popmessage("b:%x.%x m:%x.%x f:%x.%x %d%d%d", bg_bank, bg_col, mid_bank, mid_col, fg_bank, fg_col, info_1, info_2, info_3);
			tilemap_mark_all_tiles_dirty(background_layer);
			tilemap_mark_all_tiles_dirty(midground_layer);
			tilemap_mark_all_tiles_dirty(foreground_layer);
		}
	}
#endif
#endif

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (!input_code_pressed(screen->machine, KEYCODE_Q))
		tilemap_draw(bitmap, cliprect, background_layer, 0, 0);
	if (!input_code_pressed(screen->machine, KEYCODE_W))
		tilemap_draw(bitmap, cliprect, midground_layer, 0, 0);
	if (!input_code_pressed(screen->machine, KEYCODE_E))
		tilemap_draw(bitmap, cliprect, foreground_layer, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect, 0);

	if (!input_code_pressed(screen->machine, KEYCODE_A))
		tilemap_draw(bitmap, cliprect, text_layer, 0, 0);

	return 0;
}




/*************************************
 *
 *  Interrupts
 *
 *************************************/

static INTERRUPT_GEN( raiden2_interrupt )
{

	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xc0/4);	/* VBL */
	logerror("VSYNC\n");
}




// Sprite encryption key upload

static UINT32 sprcpt_adr, sprcpt_idx;

static UINT16 sprcpt_flags2;
static UINT32 sprcpt_val[2], sprcpt_flags1;
static UINT32 sprcpt_data_1[0x100], sprcpt_data_2[0x40], sprcpt_data_3[6], sprcpt_data_4[4];

static void sprcpt_init(void)
{
	memset(sprcpt_data_1, 0, sizeof(sprcpt_data_1));
	memset(sprcpt_data_2, 0, sizeof(sprcpt_data_2));
	memset(sprcpt_data_3, 0, sizeof(sprcpt_data_3));
	memset(sprcpt_data_4, 0, sizeof(sprcpt_data_4));

	sprcpt_adr = 0;
	sprcpt_idx = 0;
}


WRITE16_HANDLER(sprcpt_adr_w)
{
	combine32(&sprcpt_adr, offset, data, mem_mask);
}

WRITE16_HANDLER(sprcpt_data_1_w)
{
	combine32(sprcpt_data_1+sprcpt_adr, offset, data, mem_mask);
}

WRITE16_HANDLER(sprcpt_data_2_w)
{
	combine32(sprcpt_data_2+sprcpt_adr, offset, data, mem_mask);
}

WRITE16_HANDLER(sprcpt_data_3_w)
{
	combine32(sprcpt_data_3+sprcpt_idx, offset, data, mem_mask);
	if(offset == 1) {
		sprcpt_idx ++;
		if(sprcpt_idx == 6)
			sprcpt_idx = 0;
	}
}

WRITE16_HANDLER(sprcpt_data_4_w)
{
	combine32(sprcpt_data_4+sprcpt_idx, offset, data, mem_mask);
	if(offset == 1) {
		sprcpt_idx ++;
		if(sprcpt_idx == 4)
			sprcpt_idx = 0;
	}
}

WRITE16_HANDLER(sprcpt_val_1_w)
{
	combine32(sprcpt_val+0, offset, data, mem_mask);
}

WRITE16_HANDLER(sprcpt_val_2_w)
{
	combine32(sprcpt_val+1, offset, data, mem_mask);
}

WRITE16_HANDLER(sprcpt_flags_1_w)
{
	combine32(&sprcpt_flags1, offset, data, mem_mask);
	if(offset == 1) {
		// bit 31: 1 = allow write on sprcpt data

		if(!(sprcpt_flags1 & 0x80000000U)) {
			// Upload finished
			if(1) {
				int i;
				logerror("sprcpt_val 1: %08x\n", sprcpt_val[0]);
				logerror("sprcpt_val 2: %08x\n", sprcpt_val[1]);
				logerror("sprcpt_data 1:\n");
				for(i=0; i<0x100; i++) {
					logerror(" %08x", sprcpt_data_1[i]);
					if(!((i+1) & 7))
						logerror("\n");
				}
				logerror("sprcpt_data 2:\n");
				for(i=0; i<0x40; i++) {
					logerror(" %08x", sprcpt_data_2[i]);
					if(!((i+1) & 7))
						logerror("\n");
				}
			}
		}
	}
}

WRITE16_HANDLER(sprcpt_flags_2_w)
{
	COMBINE_DATA(&sprcpt_flags2);
	if(offset == 0) {
		if(sprcpt_flags2 & 0x8000) {
			// Reset decryption -> redo it
		}
	}
}



// XXX
// write only: 4c0 4c1 500 501 502 503

static UINT16 handle_io_r(const address_space *space, int offset)
{
	logerror("io_r %04x, %04x (%x)\n", offset*2, mainram[offset], cpu_get_pc(space->cpu));
	return mainram[offset];
}

static void handle_io_w(const address_space *space, int offset, UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&mainram[offset]);
	switch(offset) {
	default:
		logerror("io_w %04x, %04x & %04x (%x)\n", offset*2, data, mem_mask, cpu_get_pc(space->cpu));
	}
}

static READ16_HANDLER(any_r)
{
	c_r[offset]++;

	if(offset >= 0x400/2 && offset < 0x800/2)
		return handle_io_r(space, offset);

	return mainram[offset];
}

static WRITE16_HANDLER(any_w)
{
	int show = 0;
	if(offset >= 0x400/2 && offset < 0x800/2)
		handle_io_w(space, offset, data, mem_mask);

	c_w[offset]++;
	//  logerror("mainram_w %04x, %02x (%x)\n", offset, data, cpu_get_pc(space->cpu));
	if(mainram[offset] != data && offset >= 0x400 && offset < 0x800) {
		if(0 &&
		   offset != 0x4c0/2 && offset != 0x500/2 &&
		   offset != 0x444/2 && offset != 0x6de/2 && offset != 0x47e/2 &&
		   offset != 0x4a0/2 && offset != 0x620/2 && offset != 0x6c6/2 &&
		   offset != 0x628/2 && offset != 0x62a/2)
			logerror("mainram_w %04x, %04x & %04x (%x)\n", offset*2, data, mem_mask, cpu_get_pc(space->cpu));
	}

	if(0 && c_w[offset]>1000 && !c_r[offset]) {
		if(offset != 0x4c0/2 && (offset<0x500/2 || offset > 0x503/2))
			logerror("mainram_w %04x, %04x & %04x [%d.%d] (%x)\n", offset*2, data, mem_mask, c_w[offset], c_r[offset], cpu_get_pc(space->cpu));
	}

	//  if(offset == 0x471 || (offset >= 0xb146 && offset < 0xb156))

	//  show = show || offset == 0xbfc || offset == 0xbfd || offset == 0xc10 || offset == 0xc11;
	//  show = offset >= 0x400 && offset < 0x800;
	//  show = offset >= 0x500 && offset < 0x504 && data != mainram[offset]; // Son?
	//  show = offset == 0x704 || offset == 0x710 || offset == 0x71c;

	if(show)
		logerror("mainram_w %04x, %04x & %04x (%x)\n", offset*2, data, mem_mask, cpu_get_pc(space->cpu));

	//  if(offset == 0x700)
	//      cpu_setbank(2, memory_region(space->machine, "user1")+0x20000*data);

	COMBINE_DATA(&mainram[offset]);
}

static WRITE16_HANDLER(w1x)
{
	COMBINE_DATA(&w1ram[offset]);
	if(0 && offset < 0x800/2)
		logerror("w1x %05x, %04x & %04x (%05x)\n", offset*2+0x10000, data, mem_mask, cpu_get_pc(space->cpu));
}

#ifdef UNUSED_FUNCTION
static void r2_dt(UINT16 sc, UINT16 cc, UINT16 ent, UINT16 tm, UINT16 x, UINT16 y)
{
	int bank = mainram[0x704/2];

#if 0
	switch(ent) {
	case 0x01: // bank 0 = intro (plane) on layer 3, tb=7/6 ; 1 = jeu, tb=0
		bank = 0;
		break;
	case 0x08: // bank 0 = intro (ciel) on layer 1, tb=0 ; 1 = jeu, tb=0
		bank = 0;
		break;
	case 0x09: // bank 0 = intro (cliff) on layer 2, tb=3 ; 1 = jeu, tb=0
		bank = 0;
		break;
	case 0x10: // bank 0 = jeu (route) on layer 1, tb=0, 1 = idem
		bank = 0;
		break;
	case 0x11: // bank 0 = ?jeu (route) on layer 1, tb=0, 1 = idem
		bank = 0;
		break;
	case 0x20: // bank 0 = jeu on layer 2, tb=1, 1 = idem with layer 3 required
		bank = 0;
		break;
	case 0x21: // bank 0 = ?jeu on layer 2, tb=1, 1 = idem with layer 3 required
		bank = 0;
		break;
	case 0x30: // bank 0 = jeu on layer 3
		bank = 0;
		break;
	case 0x31: // bank 0 = ?jeu on layer 3
		bank = 0;
		break;
	case 0x39: // bank 0 = porte avion on layer 3, tb=6; 1 = idem
		bank = 0;
		break;
	case 0x3a: // bank 0 = press start on layer 3, tb=4 ; 1 = idem
		bank = 1;
		break;
	default:
		logerror("  -> unknown %x, using bank 0\n", ent);
		bank = 0;
	}
#endif

	logerror("Draw tilemap %04x:%04x  %04x %04x  %04x %04x, bank %d\n",
			 sc, cc, ent, tm, x, y, bank);

	//  cpu_setbank(2, memory_region(machine, "user1")+0x20000*bank);
}

static void r2_6f6c(UINT16 cc, UINT16 v1, UINT16 v2)
{
//  int bank = 0;
	logerror("6f6c: 9800:%04x  %04x %04x\n",
			 cc, v1, v2);
	//  cpu_setbank(2, memory_region(machine, "user1")+0x20000*bank);
}
#endif

static void common_reset(void)
{
	bg_bank=0;
	fg_bank=6;
	mid_bank=1;
	bg_col=0;
	fg_col=1;
	mid_col=2;
}

static MACHINE_RESET(raiden2)
{
	common_reset();
	sprcpt_init();
	MACHINE_RESET_CALL(seibu_sound);

	//cop_init();
}


/* MEMORY MAPS */

static ADDRESS_MAP_START( raiden2_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00400, 0x007ff) AM_READWRITE(raiden2_mcu_r, raiden2_mcu_w) AM_BASE(&cop_mcu_ram)

	AM_RANGE(0x00000, 0x0bfff) AM_READWRITE(any_r, any_w) AM_BASE(&mainram)
//  AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x0c000, 0x0cfff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(raiden2_background_w) AM_BASE(&back_data)
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(raiden2_foreground_w) AM_BASE(&fore_data)
    AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(raiden2_midground_w)  AM_BASE(&mid_data)
    AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(raiden2_text_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */

	AM_RANGE(0x10000, 0x1efff) AM_RAM_WRITE(w1x) AM_BASE(&w1ram)
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("bank1")
	AM_RANGE(0x40000, 0xfffff) AM_ROMBANK("bank2")
ADDRESS_MAP_END

/* new zero team uses the copd3 protection... and uploads a 0x400 byte table, probably the mcu code, encrypted */


static UINT16 mcu_prog[0x400];
static int mcu_prog_offs = 0;

static WRITE16_HANDLER( mcu_prog_w )
{
	mcu_prog[mcu_prog_offs*2] = data;
}

static WRITE16_HANDLER( mcu_prog_w2 )
{
	mcu_prog[mcu_prog_offs*2+1] = data;

	// both new zero team and raiden2/dx v33 version upload the same table..
#if 1
    {
		char tmp[64];
        FILE *fp;
	    sprintf(tmp,"cop3_%s.data", space->machine->gamedrv->name);

		fp=fopen(tmp, "w+b");
        if (fp)
        {
            fwrite(mcu_prog, 0x400, 2, fp);
            fclose(fp);
        }
    }
#endif
}

static WRITE16_HANDLER( mcu_prog_offs_w )
{
	mcu_prog_offs = data;
}



static READ16_HANDLER( rdx_v33_system_r )
{
	return input_port_read(space->machine, "SYSTEM");
}


static READ16_HANDLER( r2_playerin_r )
{
	return input_port_read(space->machine, "INPUT");
}

static READ16_HANDLER( rdx_v33_unknown_r )
{
	return 0x0000;
}

static READ16_HANDLER( rdx_v33_unknown2_r )
{
	return 0x0000;
}

static READ16_HANDLER( nzerotea_unknown_r )
{
	return 0xffff;
}


static ADDRESS_MAP_START( nzerotea_mem, ADDRESS_SPACE_PROGRAM, 16 )
//  AM_RANGE(0x00400, 0x007ff) AM_READWRITE(raiden2_mcu_r, raiden2_mcu_w) AM_BASE(&cop_mcu_ram)

	/* results from cop? */
	AM_RANGE(0x00430, 0x00431) AM_READ(nzerotea_unknown_r)
	AM_RANGE(0x00432, 0x00433) AM_READ(nzerotea_unknown_r)
	AM_RANGE(0x00434, 0x00435) AM_READ(nzerotea_unknown_r)
	AM_RANGE(0x00436, 0x00437) AM_READ(nzerotea_unknown_r)
	AM_RANGE(0x006b0, 0x006b1) AM_WRITE(mcu_prog_w)
	AM_RANGE(0x006b2, 0x006b3) AM_WRITE(mcu_prog_w2)
	AM_RANGE(0x006b4, 0x006b5) AM_WRITENOP
	AM_RANGE(0x006b6, 0x006b7) AM_WRITENOP
	AM_RANGE(0x006bc, 0x006bd) AM_WRITE(mcu_prog_offs_w)
//  AM_RANGE(0x006d8, 0x006d9) AM_WRITE(bbbbll_w) // scroll?
	AM_RANGE(0x006dc, 0x006dd) AM_READ(nzerotea_unknown_r)
//  AM_RANGE(0x006de, 0x006df) AM_WRITE(mcu_unkaa_w) // mcu command related?
	//AM_RANGE(0x00700, 0x00701) AM_DEVWRITE("eeprom", rdx_v33_eeprom_w)
	AM_RANGE(0x00740, 0x00741) AM_READ(nzerotea_unknown_r)
	AM_RANGE(0x00744, 0x00745) AM_READ(r2_playerin_r)
	AM_RANGE(0x0074c, 0x0074d) AM_READ(rdx_v33_system_r)
	AM_RANGE(0x00762, 0x00763) AM_READ(nzerotea_unknown_r)

	AM_RANGE(0x00788, 0x00789) AM_READ(nzerotea_unknown_r)
	AM_RANGE(0x00794, 0x00795) AM_READ(nzerotea_unknown_r)

	AM_RANGE(0x00000, 0x0bfff) AM_READWRITE(any_r, any_w) AM_BASE(&mainram)
//  AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x0c000, 0x0cfff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(raiden2_background_w) AM_BASE(&back_data)
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(raiden2_foreground_w) AM_BASE(&fore_data)
    AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(raiden2_midground_w)  AM_BASE(&mid_data)
    AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(raiden2_text_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */

	AM_RANGE(0x10000, 0x1efff) AM_RAM_WRITE(w1x) AM_BASE(&w1ram)
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("bank1")
	AM_RANGE(0x40000, 0xfffff) AM_ROMBANK("bank2")
ADDRESS_MAP_END



/* INPUT PORTS */

static INPUT_PORTS_START( raiden2 )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("P1")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")	/* Dip switch A  */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, "Starting Coin" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "X 2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")	/* Dip switch B */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "200000 500000" )
	PORT_DIPSETTING(    0x20, "400000 1000000" )
	PORT_DIPSETTING(    0x10, "1000000 3000000" )
	PORT_DIPSETTING(    0x00, "No Extend" )
	PORT_DIPNAME( 0x40, 0x40, "Demo Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")	/* START BUTTONS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( raidendx )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("P1")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")	/* Dip switch A  */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x40, 0x40, "Starting Coin" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "X 2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")	/* Dip switch B  */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) /* Manual shows "Not Used" */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) /* Manual shows "Not Used" */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Demo Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")	/* START BUTTONS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout raiden2_charlayout =
{
	8,8,
	4096,
	4,
	{ 8,12,0,4 },
	{ 3,2,1,0,19,18,17,16 },
	{ STEP8(0,32) },
	32*8
};


static const gfx_layout raiden2_tilelayout =
{
	16,16,
	0x8000,
	4,
	{ 8,12,0,4 },
	{
		3,2,1,0,
		19,18,17,16,
		3+64*8, 2+64*8, 1+64*8, 0+64*8,
		19+64*8,18+64*8,17+64*8,16+64*8,
	},
	{ STEP16(0,32) },
	128*8
};

static const gfx_layout raiden2_spritelayout =
{
	16, 16,
	0x10000,
	4,
	{ STEP4(0,1) },
	{ 4, 0, 12, 8, 20, 16, 28, 24, 36, 32, 44, 40, 52, 48, 60, 56 },
	{ STEP16(0,64) },
	16*16*4
};

static GFXDECODE_START( raiden2 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, raiden2_charlayout,   0x700, 128 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, raiden2_tilelayout,   0x400, 128 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, raiden2_spritelayout, 0x000, 128 )
GFXDECODE_END


/* MACHINE DRIVERS */

static MACHINE_DRIVER_START( raiden2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V30,XTAL_32MHz/2) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(raiden2_mem)
	MDRV_CPU_VBLANK_INT("screen", raiden2_interrupt)

	MDRV_MACHINE_RESET(raiden2)

	SEIBU2_RAIDEN2_SOUND_SYSTEM_CPU(14318180/4)


	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate *//2)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
#if 1
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 43*8-1, 1, 30*8)
#else
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0,511,0,511)
#endif
	MDRV_GFXDECODE(raiden2)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(raiden2)
	MDRV_VIDEO_UPDATE(raiden2)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_RAIDEN2_INTERFACE(28636360/8,28636360/28,1,2)


/* Sound hardware infos: Z80 and YM2151 are clocked at XTAL_28_63636MHz/8 */
/* The 2 Oki M6295 are clocked at XTAL_28_63636MHz/28 and pin 7 is high for both */

MACHINE_DRIVER_END



static MACHINE_DRIVER_START( nzerotea )

	MDRV_IMPORT_FROM(raiden2)

	/* basic machine hardware */
	MDRV_DEVICE_MODIFY("maincpu") // also change it to a v33!
	MDRV_CPU_PROGRAM_MAP(nzerotea_mem)
	MDRV_CPU_VBLANK_INT("screen", raiden2_interrupt)
MACHINE_DRIVER_END

/* ROM LOADING */

/* Raiden II  Seibu Kaihatsu 1993

YM2151   OKI M6295 VOI2  Z8400A
         OKI M6295 VOI1  SND       2018
                  5816-15          6116
   YM3012         5816-15
                  5816-15
                  5816-15
          SIE150       SEI252

             OBJ1 OBJ2            34256-20
             OBJ3 OBJ4            34256-20
                                  34256-20
    SEI360                        34256-20
                  PRG0
                  PRG1
          32MHz
 SEI0200     7C185-35          SEI1000
             7C185-35

  BG1   BG2    7   COPX-D2      NEC V30

*/

ROM_START( raiden2 )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("prg1",   0x000001, 0x80000, CRC(4609b5f2) SHA1(272d2aa75b8ea4d133daddf42c4fc9089093df2e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "snd",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )


	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "px0",	0x000000,	0x020000,	CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x080000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "voi1", 0x00000, 0x80000, CRC(f340457b) SHA1(8169acb24c82f68d223a31af38ee36eb6cb3adf4) )

	ROM_REGION( 0x080000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "voi2", 0x00000, 0x80000, CRC(d321ff54) SHA1(b61e602525f36eb28a1408ffb124abfbb6a08706) )
ROM_END

/*

---------------------------------------
Raiden II by SEIBU KAIHATSU INC. (1993)
---------------------------------------
malcor

Location      Type      File ID    Checksum
-------------------------------------------
M6 U0211     27C240      ROM1        F9A9
M6 U0212     27C240      ROM2e       13B3    [ English  ]
M6 U0212     27C240      ROM2J       14BF    [ Japanese ]
B5 U1110     27C512      ROM5        1223
B3 U1017     27C2000     ROM6        DE25
S5 U0724     27C1024     ROM7        966D

*/

/* does this have worse sound hardware or are we just missing roms? */

ROM_START( raiden2a )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2e",  0x000001, 0x80000, CRC(458d619c) SHA1(842bf0eeb5d192a6b188f4560793db8dad697683) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rom5",  0x000000, 0x10000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "px0",	0x000000,	0x020000,	CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // rom7

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	/* not from this set, assumed to be the same */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "rom6", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

ROM_START( raiden2b )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2j",  0x000001, 0x80000, CRC(e4e4fb4c) SHA1(7ccf33fe9a1cddf0c7e80d7ed66d615a828b3bb9) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rom5",  0x000000, 0x10000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "px0",	0x000000,	0x020000,	CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // rom7

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	/* not from this set, assumed to be the same */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "rom6", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

/*

Raiden II (Japan version)
(c) 1993 Seibu Kaihatsu Inc.,

CPU:          D70116HG-16 V30/Z8400AB1 Z80ACPU
SOUND:        YM2151
VOICE:        M6295 x2
OSC:          32.000/28.6364MHz
CUSTOM:       SEI150
              SEI252
              SEI360
              SEI1000
              SEI0200
              COPX-D2 ((c)1992 RISE CORP)

---------------------------------------------------
 filemanes          devices       kind
---------------------------------------------------
 RD2_1.211          27C4096       V30 main prg.
 RD2_2.212          27C4096       V30 main prg.
 RD2_5.110          27C512        Z80 sound prg.
 RD2_PCM.018        27C2001       M6295 data
 RD2_6.017          27C2001       M6295 data
 RD2_7.724          27C1024       fix chr.
 RD2_BG1.075        57C16200      bg  chr.
 RD2_BG2.714        57C16200      bg  chr.
 RD2_OBJ1.811       57C16200      obj chr.
 RD2_OBJ2.082       57C16200      obj chr.
 RD2_OBJ3.837       57C16200      obj chr.
 RD2_OBJ4.836       57C16200      obj chr.
---------------------------------------------------

*/

/* same v30 program roms as the raiden2b set but *yet another* sound program rom! */



ROM_START( raiden2c )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2j",  0x000001, 0x80000, CRC(e4e4fb4c) SHA1(7ccf33fe9a1cddf0c7e80d7ed66d615a828b3bb9) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rd2_5.110",  0x000000, 0x10000,  CRC(c2028ba2) SHA1(f6a9322b669ff82dea6ecf52ad3bd5d0901cce1b) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "px0",	0x000000,	0x020000,	CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) // rom7

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	/* not from this set, assumed to be the same */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "rom6", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2_voi2.bin", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2d )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu1",   0x000000, 0x80000, CRC(c1fc70f5) SHA1(a054f5ae9583972c406d9cf871340d5e072d71a3) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu2",   0x000001, 0x80000, CRC(28d5365f) SHA1(21efe29c2d373229c2ff302d86e59c2c94fa6d03) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5",  0x000000, 0x10000, CRC(5db9f922) SHA1(8257aab98657fe44df19d2a48d85fcf65b3d98c6) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7",	0x000000,	0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu6", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2_voi2.bin", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END


/*

Raiden 2, Seibu License, Easy Version

According to DragonKnight Zero's excellent Raiden 2
FAQ this PCB is the easy version.

The different versions may be identified by the high score
screen. The easy version has the Raiden MK-II in colour
on a black background whereas the hard version has a sepia shot
of an ascending fighter.

The entire FAQ is available here:
http://www.gamefaqs.com/coinop/arcade/game/10729.html

*/

ROM_START( raiden2e )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("r2_prg_0.bin",   0x000000, 0x80000, CRC(2abc848c) SHA1(1df4276d0074fcf1267757fa0b525a980a520f3d) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2_prg_1.bin",   0x000001, 0x80000, CRC(509ade43) SHA1(7cdee7bb00a6a1c7899d10b96385d54c261f6f5a) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2_snd.bin",  0x000000, 0x10000, CRC(6bad0a3e) SHA1(eb7ae42353e1984cd60b569c26cdbc3b025a7da6) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2_fx0.bin",	0x000000,	0x020000,	CRC(c709bdf6) SHA1(0468d90412b7590d67eaadc0a5e3537cd5e73943) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2_voi1.bin", 0x00000, 0x40000, CRC(488d050f) SHA1(fde2fd64fea6bc39e1a42885d21d362bc6be2ac2) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2_voi2.bin", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raiden2f )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("r2.1",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2.2",  0x000001, 0x80000, CRC(bf7577ec) SHA1(98576af78760b8aef1ef3efe1ba963977c89d225) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2.5",  0x000000, 0x10000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2.7",	0x000000,	0x020000,	CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "bg1",   0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) )
	ROM_LOAD( "bg2",   0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	/* not from this set, assumed to be the same */
	ROM_LOAD32_WORD( "obj1",  0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) )
	ROM_LOAD32_WORD( "obj2",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) )
	ROM_LOAD32_WORD( "obj3",  0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) )
	ROM_LOAD32_WORD( "obj4",  0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2.6", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "voi2", 0x00000, 0x80000, CRC(d321ff54) SHA1(b61e602525f36eb28a1408ffb124abfbb6a08706) )
ROM_END

/* Raiden DX sets */

ROM_START( raidndx )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("dx_1h.4n",   0x000000, 0x80000, BAD_DUMP CRC(7624c36b) SHA1(84c17f2988031210d06536710e1eac558f4290a1) ) // bad
	ROM_LOAD32_BYTE("dx_2h.4p",   0x000001, 0x80000, CRC(4940fdf3) SHA1(c87e307ed7191802583bee443c7c8e4f4e33db25) )
	ROM_LOAD32_BYTE("dx_3h.6n",   0x000002, 0x80000, CRC(6c495bcf) SHA1(fb6153ecc443dabc829dda6f8d11234ad48de88a) )
	ROM_LOAD32_BYTE("dx_4h.6k",   0x000003, 0x80000, CRC(9ed6335f) SHA1(66975204b120915f23258a431e19dbc017afd912) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x10000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",	0x000000,	0x020000,	CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END

ROM_START( raidndxj )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("rdxj_1.bin",   0x000000, 0x80000, CRC(b5b32885) SHA1(fb3c592b2436d347103c17bd765176062be95fa2) )
	ROM_LOAD32_BYTE("rdxj_2.bin",   0x000001, 0x80000, CRC(7efd581d) SHA1(4609a0d8afb3d62a38b461089295efed47beea91) )
	ROM_LOAD32_BYTE("rdxj_3.bin",   0x000002, 0x80000, CRC(55ec0e1d) SHA1(6be7f268df51311a817c1c329a578b38abb659ae) )
	ROM_LOAD32_BYTE("rdxj_4.bin",   0x000003, 0x80000, CRC(f8fb31b4) SHA1(b72fd7cbbebcf3d1b2253c309fcfa60674776467) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x10000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",	0x000000,	0x020000,	CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	/* not from this set, assumed to be the same */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )
	/* not from this set, assumed to be the same */

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END

ROM_START( raidndxt )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.u1210", 0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.u1211", 0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.u129",  0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.u1212", 0x000003, 0x80000, CRC(6bde6edc) SHA1(c3565a55b858c10659fd9b93b1cd92bc39e6446d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.u1110", 0x000000, 0x10000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "7.u0724", 0x000000, 0x020000, CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj-3.u0837", 0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836", 0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END

ROM_START( raidndxm )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.bin",   0x000000, 0x80000, CRC(22b155ae) SHA1(388151e2c8fb301bd5bc66a974e9fe16816ae0bc) )
	ROM_LOAD32_BYTE("2d.bin",   0x000001, 0x80000, CRC(2be98ca8) SHA1(491e990405b0ad3de45bdbcc2453af9215ae19c8) )
	ROM_LOAD32_BYTE("3d.bin",   0x000002, 0x80000, CRC(b4785576) SHA1(aa5eee7b0c635c6d18a7fc1e037bf570a677dd90) )
	ROM_LOAD32_BYTE("4d.bin",   0x000003, 0x80000, CRC(5a77f7b4) SHA1(aa757e6308893ca63963170c5b1743de7c7ab034) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x10000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",	0x000000,	0x020000,	CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	/* not from this set, assumed to be the same */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	/* not from this set, assumed to be the same */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )
	/* not from this set, assumed to be the same */

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
		/* not from this set, assumed to be the same */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END


/* Zero Team sets */

ROM_START( zeroteam )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.5k",   0x000000, 0x40000, CRC(25aa5ba4) SHA1(40e6047620fbd195c87ac3763569af099096eff9) )
	ROM_LOAD32_BYTE("3.6k",   0x000002, 0x40000, CRC(ec79a12b) SHA1(515026a2fca92555284ac49818499af7395783d3) )
	ROM_LOAD32_BYTE("2.6l",   0x000001, 0x40000, CRC(54f3d359) SHA1(869744185746d55c60d2f48eabe384a8499e00fd) )
	ROM_LOAD32_BYTE("4.5l",   0x000003, 0x40000, CRC(a017b8d0) SHA1(4a93ff1ab18f4b61c7ef580995f64840c19ce6b9) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",  0x000000, 0x10000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.5s",	0x000000,	0x010000,	CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.5r",	0x000001,	0x010000,	CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

ROM_START( zeroteama )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.bin",   0x000000, 0x40000, CRC(bd7b3f3a) SHA1(896413901a429d0efa3290f61920063c81730e9b) )
	ROM_LOAD32_BYTE("3.bin",   0x000002, 0x40000, CRC(19e02822) SHA1(36c9b887eaa9b9b67d65c55e8f7eefd08fe0be15) )
	ROM_LOAD32_BYTE("2.bin",   0x000001, 0x40000, CRC(0580b7e8) SHA1(d4416264aa5acdaa781ebcf51f128b3e665cc903) )
	ROM_LOAD32_BYTE("4.bin",   0x000003, 0x40000, CRC(cc666385) SHA1(23a8878315b6009dcc1f27e49572e5be29f6a1a6) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.bin",  0x000000, 0x10000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.bin",	0x000000,	0x010000, CRC(eb10467f) SHA1(fc7d576dc41bc878ff20f0370e669e19d54fcefb) )
	ROM_LOAD16_BYTE( "8.bin",	0x000001,	0x010000, CRC(a0b2a09a) SHA1(9b1f6c732000b84b1ad635f332ebead5d65cc491) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

ROM_START( zeroteams )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1_sel.bin",   0x000000, 0x40000, CRC(d99d6273) SHA1(21dccd5d71c720b8364406835812b3c9defaff6c) )
	ROM_LOAD32_BYTE("3_sel.bin",   0x000002, 0x40000, CRC(0a9fe0b1) SHA1(3588fe19788f77d07e9b5ab8182b94362ffd0024) )
	ROM_LOAD32_BYTE("2_sel.bin",   0x000001, 0x40000, CRC(4e114e74) SHA1(fcccbb68c6b7ffe8d109ed3a1ec9120d338398f9) )
	ROM_LOAD32_BYTE("4_sel.bin",   0x000003, 0x40000, CRC(0df8ba94) SHA1(e07dce6cf3c3cfe1ea3b7f01e18833c1da5ed1dc) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5_sel.bin",  0x000000, 0x10000, CRC(ed91046c) SHA1(de815c999aeeb814d3f091d5a9ac34ea9a388ddb) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.5s",	0x000000,	0x010000,	CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.5r",	0x000001,	0x010000,	CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

/* set contained only program roms, was marked as 'non-encrytped' but program isn't encrypted anyway?! */
ROM_START( zeroteamb )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("z1",   0x000000, 0x40000, CRC(157743d0) SHA1(f9c84c9025319f76807ef0e79f1ee1599f915b45) )
	ROM_LOAD32_BYTE("z3",   0x000002, 0x40000, CRC(fea7e4e8) SHA1(08c4bdff82362ae4bcf86fa56fcfc384bbf82b71) )
	ROM_LOAD32_BYTE("z2",   0x000001, 0x40000, CRC(21d68f62) SHA1(8aa85b38e8f36057ef6c7dce5a2878958ce93ce8) )
	ROM_LOAD32_BYTE("z4",   0x000003, 0x40000, CRC(ce8fe6c2) SHA1(69627867c7866e43e771ab85014553117044d18d) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",  0x000000, 0x10000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.5s",	0x000000,	0x010000,	CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.5r",	0x000001,	0x010000,	CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

ROM_START( zeroteamc )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("b1.024",   0x000000, 0x40000, CRC(528de3b9) SHA1(9ca8cdc0212f2540e852d20ab4c04f68b967d024) )
	ROM_LOAD32_BYTE("b3.023",   0x000002, 0x40000, CRC(3688739a) SHA1(f98f461fb8e7804b3b4020a5e3762d36d6458a62) )
	ROM_LOAD32_BYTE("b2.025",   0x000001, 0x40000, CRC(5176015e) SHA1(6b372564b2f1b1f56cae0c98f4ca588b784bfa3d) )
	ROM_LOAD32_BYTE("b4.026",   0x000003, 0x40000, CRC(c79925cb) SHA1(aaff9f626ec61bc0ff038ebd722fe361dccc49fb) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.bin",  0x000000, 0x10000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "b7.072",	0x000000,	0x010000, CRC(30ec0241) SHA1(a0d0be9458bf97cb9764fb85c988bb816710475e) )
	ROM_LOAD16_BYTE( "b8.077",	0x000001,	0x010000, CRC(e18b3a75) SHA1(3d52bba8d47d0d9108ee79014fd64d6e856a3fde) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.105", 0x00000, 0x40000,  CRC(b4a6e899) SHA1(175ab656db3c3258ff10eede89890f62435d2298) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

/* Different hardware, uses COPX-D3 for protection  */
ROM_START( nzerotea )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg1",   0x000000, 0x80000, CRC(3c7d9410) SHA1(25f2121b6c2be73f11263934266901ed5d64d2ee) )
	ROM_LOAD16_BYTE("prg2",   0x000001, 0x80000, CRC(6cba032d) SHA1(bf5d488cd578fff09e62e3650efdee7658033e3f) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",  0x000000, 0x10000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "fix1",	0x000000,	0x010000,	CRC(0c4895b0) SHA1(f595dbe5a19edb8a06ea60105ee26b95db4a2619) )
	ROM_LOAD16_BYTE( "fix2",	0x000001,	0x010000,	CRC(07d8e387) SHA1(52f54a6a4830592784cdf643a5f255aa3db53e50) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END



/*

Raiden DX
Seibu Kaihatsu, 1993/1996

Note! PCB seems like an updated version. It uses _entirely_ SMD technology and
is smaller than the previous hardware. I guess the game is still popular, so
Seibu re-manufactured it using newer technology to meet demand.
Previous version hardware is similar to Heated Barrel/Legionairre/Seibu Cup Soccer etc.
It's possible that the BG and OBJ ROMs from this set can be used to complete the
previous (incomplete) dump that runs on the V30 hardware, since most GFX chips are the same.

PCB ID: (C) 1996 JJ4-China-Ver2.0 SEIBU KAIHATSU INC., MADE IN JAPAN
CPU   : NEC 70136AL-16 (V33)
SOUND : Oki M6295
OSC   : 28.636360MHz
RAM   : CY7C199-15 (28 Pin SOIC, x11)
        Breakdown of RAM locations...
                                     (x2 near SIE150)
                                     (x3 near SEI252)
                                     (x2 near SEI0200)
                                     (x4 near SEI360)

DIPs  : 8 position (x1)
        1-6 OFF   (NOT USED)
        7   OFF = Normal Mode  , ON = Test/Setting Mode
        8   OFF = Normal Screen, ON = FLIP Screen

OTHER : Controls are 8-way + 3 Buttons
        Amtel 93C46 EEPROM (SOIC8)
        PALCE16V8 (x1, near BG ROM, SOIC20)
        SEIBU SEI360 SB06-1937   (160 pin PQFP)
        SEIBI SIE150             (100 pin PQFP, Note SIE, not a typo)
        SEIBU SEI252             (208 pin PQFP)
        SEIBU SEI333             (208 pin PQFP)
        SEIBU SEI0200 TC110G21AF (100 pin PQFP)

        Note: Most of the custom SEIBU chips are the same as the ones used on the
              previous version hardware.

ROMs  :   (filename is PCB label, extension is PCB 'u' location)

              ROM                ROM                 Probably               Byte
Filename      Label              Type                Used...        Note    C'sum
---------------------------------------------------------------------------------
PCM.099       RAIDEN-X SOUND     LH538100  (SOP32)   Oki Samples      0     8539h
FIX.613       RAIDEN-X FIX       LH532048  (SOP40)   ? (BG?)          1     182Dh
COPX_D3.357   RAIDEN-X 333       LH530800A (SOP32)   Protection?      2     CEE4h
PRG.223       RAIDEN-X CHR-4A1   MX23C3210 (SOP44)   V33 program      3     F276h
OBJ1.724      RAIDEN-X CHR1      MX23C3210 (SOP44)   Motion Objects   4     4148h
OBJ2.725      RAIDEN-X CHR2      MX23C3210 (SOP44)   Motion Objects   4     00C3h
BG.612        RAIDEN-X CHR3      MX23C3210 (SOP44)   Backgrounds      5     3280h


Notes
0. Located near Oki M6295
1. Located near SEI0200 and BG ROM
2. Located near SEI333
3. Located near V33 and SEI333
4. Located near V33 and SEI252
5. Located near FIX ROM and SEI0200

*/


ROM_START( r2dx_v33 )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* v33 main cpu */
	ROM_LOAD("prg.223",   0x000000, 0x400000, CRC(b3dbcf98) SHA1(30d6ec2090531c8c579dff74c4898889902d7d87) )

	ROM_REGION( 0x20000, "cpu1", ROMREGION_ERASE00 ) /* 64k code for sound Z80 */
	/* nothing?  no z80*/

	ROM_REGION( 0x040000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "fix.613",	0x000000,	0x040000,	CRC(3da27e39) SHA1(3d446990bf36dd0a3f8fadb68b15bed54904c8b5) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg.612",   0x000000, 0x400000, CRC(162c61e9) SHA1(bd0a6a29804b84196ba6bf3402e9f30a25da9269) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1.724",  0x000000, 0x400000, CRC(7d218985) SHA1(777241a533defcbea3d7e735f309478d260bad52) )
	ROM_LOAD32_WORD( "obj2.725",  0x000002, 0x400000, CRC(b09434d9) SHA1(da75252b7693ab791fece4c10b8a4910edb76c88) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "pcm.099", 0x00000, 0x100000, CRC(97ca2907) SHA1(bfe8189300cf72089d0beaeab8b1a0a1a4f0a5b6) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPDX */
	ROM_LOAD( "copx_d3.357",   0x00000, 0x20000, CRC(fa2cf3ad) SHA1(13eee40704d3333874b6e3da9ee7d969c6dc662a) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-r2dx_v33.bin", 0x0000, 0x0080, CRC(ba454777) SHA1(101c5364e8664d17bfb1e759515d135a2673d67e) )
ROM_END

/*

X Se Dae Quiz
Seibu/Dream Island, 1995

This game runs on a Zero Team PCB

PCB Layout
ZERO TEAM-V2 SEIBU KAIHATSU INC.
|----------------------------------------|
|LA4460 YM2151 M6295 9  Z80    Y         |
|HB-46A1 YM3014 SEI150 8                 |
|VOL      6116     6116  28.6362MHz 6116 |
|                  6116   SEI251    6116 |
|J HB-2                            62256 |
|A                  OBJ-2  OBJ-1   62256 |
|M                                 62256 |
|M      SW1(8) PAL    1      3     62256 |
|A             PAL    4      2  SEI1000  |
|   SW2(8)     6264                      |
|          PAL 6264          X           |
|   SW3(8)       SEI0200  5    D71011    |
|         7     BG-1      6         V30  |
|----------------------------------------|
Notes:
      PCB is identical to standard Zero Team PCB
      with the following differences....
      1. X - location for COPX ROM, not populated
      2. Y - location for battery, not populated
      3. NEC V30 and NEC D71011 are located on a sub board and
         the surface-mounted V30 (UPD70116) is not populated
      4. ROM7 is located in a 8M-DIP42 to 4M-DIP40 adapter and is a 27C4002 EPROM
      5. ROM8 has the top 4 pins hanging out of the DIP28 socket and is a 27C1001
         EPROM. Pins 30,31 & 32 are tied together and pin 2 is tied to the SEI150
         with a wire.
*/

ROM_START( xsedae )
	ROM_REGION( 0x200000, "user1", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.u024",   0x000000, 0x40000, CRC(185437f9) SHA1(e46950b6a549d11dc57105dd7d9cb512a8ecbe70) )
	ROM_LOAD32_BYTE("3.u023",   0x000002, 0x40000, CRC(293fd6c1) SHA1(8b1a231f4bedbf9c0f347330e13fdf092b9888b4) )
	ROM_LOAD32_BYTE("2.u025",   0x000001, 0x40000, CRC(a2b052df) SHA1(e8bf9ab3d5d4e601ea9386e1f2d4e017b025407e) )
	ROM_LOAD32_BYTE("4.u026",   0x000003, 0x40000, CRC(5adf20bf) SHA1(42a0bb5a460c656675b2c432c043fc61a9049276) )

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )	/* COPDX */
	/* Not populated */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u1110",  0x000000, 0x20000, CRC(2dc2f81a) SHA1(0f6605042e0e295b4256b43dbdf5d53daebe1a9a) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "5.u077",	0x000000,	0x010000, CRC(478deced) SHA1(88cd72cb76bbc1c4255c3dfae4b9a10af9b050b2) )
	ROM_LOAD16_BYTE( "6.u072",	0x000001,	0x010000, CRC(a788402d) SHA1(8a1ac4760cf75cd2e32c1d15f36ad15cce3d411b) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg-1.u075",   0x000000, 0x100000, CRC(ac087560) SHA1(b6473b20c55ec090961cfc46a024b3c5b707ec25) )
	ROM_LOAD( "7.u0714",     0x100000, 0x080000, CRC(296105dc) SHA1(c2b80d681646f504b03c2dde13e37b1d820f82d2) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (not encrypted) */
	ROM_LOAD32_WORD( "obj-1.u0811",  0x000000, 0x100000, CRC(e65f1b4e) SHA1(b04be9af41ce868e64071715252c4ff228891cf0) )
	ROM_LOAD32_WORD( "obj-2.u082",   0x000002, 0x100000, CRC(e753e7ad) SHA1(643ab39ac1b7df686a16b1ed6fdcb686720ca8e8) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "9.u105", 0x00000, 0x40000, CRC(a7a0c5f9) SHA1(7882681ac152642aa4f859071f195842068b214b) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

static DRIVER_INIT (raiden2)
{
	/* wrong , there must be some banking this just stops it crashing */
	UINT8 *RAM = memory_region(machine, "user1");

	memory_set_bankptr(machine, "bank1",&RAM[0x100000]);
	memory_set_bankptr(machine, "bank2",&RAM[0x040000]);

	raiden2_decrypt_sprites(machine);
}

static DRIVER_INIT (xsedae)
{
	/* wrong , there must be some banking this just stops it crashing */
	UINT8 *RAM = memory_region(machine, "user1");
	memory_set_bankptr(machine, "bank1",&RAM[0x100000]);
	memory_set_bankptr(machine, "bank2",&RAM[0x040000]);
}


static WRITE16_DEVICE_HANDLER( rdx_v33_eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		eeprom_set_clock_line(device, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		eeprom_write_bit(device, data & 0x20);
		eeprom_set_cs_line(device, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);

		if (data&0xc7) logerror("eeprom_w extra bits used %04x\n",data);
	}
	else
	{
		logerror("eeprom_w MSB used %04x",data);
	}
}


static ADDRESS_MAP_START( rdx_v33_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x003ff) AM_RAM // vectors copied here

	/* results from cop? */
	AM_RANGE(0x00430, 0x00431) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00432, 0x00433) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00434, 0x00435) AM_READ(rdx_v33_unknown_r)
	AM_RANGE(0x00436, 0x00437) AM_READ(rdx_v33_unknown_r)

//  AM_RANGE(0x00620, 0x00621) AM_WRITE(scroll_w) // scroll1
//  AM_RANGE(0x00622, 0x00623) AM_WRITE(scroll_w) // scroll1

//  AM_RANGE(0x00624, 0x00625) AM_WRITE(scroll_w) // scroll2
//  AM_RANGE(0x00626, 0x00627) AM_WRITE(scroll_w) // scroll2

//  AM_RANGE(0x00628, 0x00629) AM_WRITE(scroll_w) // scroll3
//  AM_RANGE(0x0062a, 0x0062b) AM_WRITE(scroll_w) // scroll3

	AM_RANGE(0x006b0, 0x006b1) AM_WRITE(mcu_prog_w)
	AM_RANGE(0x006b2, 0x006b3) AM_WRITE(mcu_prog_w2)
	AM_RANGE(0x006b4, 0x006b5) AM_WRITENOP
	AM_RANGE(0x006b6, 0x006b7) AM_WRITENOP
	AM_RANGE(0x006bc, 0x006bd) AM_WRITE(mcu_prog_offs_w)
//  AM_RANGE(0x006d8, 0x006d9) AM_WRITE(bbbbll_w) // scroll?
	AM_RANGE(0x006dc, 0x006dd) AM_READ(rdx_v33_unknown2_r)
//  AM_RANGE(0x006de, 0x006df) AM_WRITE(mcu_unkaa_w) // mcu command related?
	AM_RANGE(0x00700, 0x00701) AM_DEVWRITE("eeprom", rdx_v33_eeprom_w)
	AM_RANGE(0x00740, 0x00741) AM_READ(rdx_v33_unknown2_r)
	AM_RANGE(0x00744, 0x00745) AM_READ(r2_playerin_r)
	AM_RANGE(0x0074c, 0x0074d) AM_READ(rdx_v33_system_r)
	AM_RANGE(0x00762, 0x00763) AM_READ(rdx_v33_unknown2_r)

	AM_RANGE(0x00780, 0x00781) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff) // single OKI chip on this version

	AM_RANGE(0x00800, 0x0087f) AM_RAM // copies eeprom here?
	AM_RANGE(0x00880, 0x0bfff) AM_RAM

	AM_RANGE(0x0c000, 0x0cfff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(raiden2_background_w) AM_BASE(&back_data)
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(raiden2_foreground_w) AM_BASE(&fore_data)
    AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(raiden2_midground_w)  AM_BASE(&mid_data)
    AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(raiden2_text_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */
	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	/* not sure of bank sizes etc. */
	AM_RANGE(0x20000, 0x2ffff) AM_ROMBANK("bank1")
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("bank2")
	AM_RANGE(0x40000, 0x4ffff) AM_ROMBANK("bank3")
	AM_RANGE(0x50000, 0x5ffff) AM_ROMBANK("bank4")
	AM_RANGE(0x60000, 0x6ffff) AM_ROMBANK("bank5")
	AM_RANGE(0x70000, 0x7ffff) AM_ROMBANK("bank6")
	AM_RANGE(0x80000, 0x8ffff) AM_ROMBANK("bank7")
	AM_RANGE(0x90000, 0x9ffff) AM_ROMBANK("bank8")
	AM_RANGE(0xa0000, 0xaffff) AM_ROMBANK("bank9")
	AM_RANGE(0xb0000, 0xbffff) AM_ROMBANK("bank10")
	AM_RANGE(0xc0000, 0xcffff) AM_ROMBANK("bank11")
	AM_RANGE(0xd0000, 0xdffff) AM_ROMBANK("bank12")
	AM_RANGE(0xe0000, 0xeffff) AM_ROMBANK("bank13")
	AM_RANGE(0xf0000, 0xfffff) AM_ROMBANK("bank14")
ADDRESS_MAP_END


static INPUT_PORTS_START( rdx_v33 )
   PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Test Mode" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( nzerotea )

	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	//PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Test Mode" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INTERRUPT_GEN( rdx_v33_interrupt )
{
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xc0/4);	/* VBL */
	logerror("VSYNC\n");
}

static MACHINE_RESET( rdx_v33 )
{
	common_reset();
}

static MACHINE_DRIVER_START( rdx_v33 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V33, 32000000/2 ) // ?
	MDRV_CPU_PROGRAM_MAP(rdx_v33_map)
	MDRV_CPU_VBLANK_INT("screen", rdx_v33_interrupt)

	MDRV_MACHINE_RESET(rdx_v33)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(5*8, 43*8-1, 1, 30*8)

	MDRV_GFXDECODE(raiden2)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(raiden2)
	MDRV_VIDEO_UPDATE(raiden2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, 1000000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



static DRIVER_INIT(rdx_v33)
{
	UINT8 *prg = memory_region(machine, "maincpu");
	memory_set_bankptr(machine, "bank1",&prg[0x020000]);
	memory_set_bankptr(machine, "bank2",&prg[0x030000]);
	memory_set_bankptr(machine, "bank3",&prg[0x040000]);
	memory_set_bankptr(machine, "bank4",&prg[0x050000]);
	memory_set_bankptr(machine, "bank5",&prg[0x060000]);
	memory_set_bankptr(machine, "bank6",&prg[0x070000]);
	memory_set_bankptr(machine, "bank7",&prg[0x080000]);
	memory_set_bankptr(machine, "bank8",&prg[0x090000]);
	memory_set_bankptr(machine, "bank9",&prg[0x0a0000]);
	memory_set_bankptr(machine, "bank10",&prg[0x0b0000]);
	memory_set_bankptr(machine, "bank11",&prg[0x0c0000]);
	memory_set_bankptr(machine, "bank12",&prg[0x0d0000]);
	memory_set_bankptr(machine, "bank13",&prg[0x0e0000]);
	memory_set_bankptr(machine, "bank14",&prg[0x0f0000]);

	raiden2_decrypt_sprites(machine);
}


/* GAME DRIVERS */

GAME( 1993, raiden2,  0,       raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 1, US Fabtek)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raiden2a, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 2, Metrotainment)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raiden2b, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 3, Japan)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raiden2c, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 4, Japan)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raiden2d, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 5)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raiden2e, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 6, Easy Version)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raiden2f, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu", "Raiden 2 (set 7)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raidndx,  0,       raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden DX (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raidndxm, raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden DX (Metrotainment license)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raidndxj, raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden DX (Japan)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, raidndxt, raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden DX (Tuning license)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, zeroteam, 0,       raiden2,  raiden2,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, zeroteama,zeroteam,raiden2,  raiden2,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, zeroteamb,zeroteam,raiden2,  raiden2,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 3)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, zeroteamc,zeroteam,raiden2,  raiden2,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 4)", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1993, zeroteams,zeroteam,raiden2,  raiden2,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team Selection", GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1995, xsedae,   0,       raiden2,  raiden2,  xsedae,   ROT0,   "Dream Island",   "X Se Dae Quiz", GAME_NOT_WORKING|GAME_NO_SOUND)

// 'V33 system type_b' - uses V33 CPU, COPX-D3 external protection rom, but still has the proper sound system
GAME( 1993, nzerotea, zeroteam,  nzerotea, nzerotea,  raiden2,  ROT0,   "Seibu Kaihatsu", "New Zero Team", GAME_NOT_WORKING|GAME_NO_SOUND) // this uses a v33 and COPD3

// newer PCB, with V33 CPU and COPD3 protection, but weak sound hardware. - was marked as Raiden DX New in the rom dump, but boots as Raiden 2 New version, is it switchable?
GAME( 1996, r2dx_v33, 0, rdx_v33,  rdx_v33, rdx_v33,  ROT270, "Seibu Kaihatsu", "Raiden 2 / DX (newer V33 PCB)", GAME_NOT_WORKING|GAME_NO_SOUND)
