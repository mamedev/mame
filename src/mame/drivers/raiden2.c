/* raiden 2 board test note 17/04/08 (based on test by dox)

 rom banking is at 6c9, bit 0x80
  -- the game only writes this directly at startup, must be written indirectly by
     one of the protection commands? or mirrored?
  value of 0x80 puts 0x00000-0x1ffff at 0x20000 - 0x3ffff
  value of 0x00 puts 0x20000-0x3ffff at 0x20000 - 0x3ffff

 TODO (xsedae):
 - sprite DMA params doesn't quite work well with this and zeroteam (hardcode src to 0xf000 and size to 0x800).
 - sprite ROMs are badly dumped (half size);
 - sprite X kludge needs to be removed somehow;
 - apparently the only real usage of the sprite protection in xsedae is on the field map (sprite goes in the wrong direction);

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
 X Se Dae Quiz

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

 Video emulation - used to be more complete than it is now, tile banking is
 currently broken.

 Low Priority

*/

#define ADDRESS_MAP_MODERN
#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "includes/raiden2.h"

static UINT16 rps(running_machine *machine)
{
	return cpu_get_reg(machine->device("maincpu"), NEC_CS);
}

static UINT16 rpc(running_machine *machine)
{
	return cpu_get_reg(machine->device("maincpu"), NEC_IP);
}

WRITE16_MEMBER(raiden2_state::cop_pgm_data_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_program[cop_latch_addr] = data;
	int idx = cop_latch_addr >> 3;
	cop_func_trigger[idx] = cop_latch_trigger;
	cop_func_value[idx]   = cop_latch_value;
	cop_func_mask[idx]    = cop_latch_mask;
}

WRITE16_MEMBER(raiden2_state::cop_pgm_addr_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	assert(data < 0x100);
	cop_latch_addr = data;
}

WRITE16_MEMBER(raiden2_state::cop_pgm_value_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_latch_value = data;
}

WRITE16_MEMBER(raiden2_state::cop_pgm_mask_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_latch_mask = data;
}

WRITE16_MEMBER(raiden2_state::cop_pgm_trigger_w)
{
	assert(ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15);
	cop_latch_trigger = data;
}

WRITE16_MEMBER(raiden2_state::cop_dma_adr_rel_w)
{
	COMBINE_DATA(&cop_dma_adr_rel);
}

WRITE16_MEMBER(raiden2_state::cop_dma_v1_w)
{
	COMBINE_DATA(&cop_dma_v1);
}

WRITE16_MEMBER(raiden2_state::cop_dma_v2_w)
{
	COMBINE_DATA(&cop_dma_v2);
}

WRITE16_MEMBER(raiden2_state::cop_dma_v3_w)
{
	COMBINE_DATA(&cop_dma_v3);
}

WRITE16_MEMBER(raiden2_state::cop_dma_mode_w)
{
	COMBINE_DATA(&cop_dma_mode);
}

WRITE16_MEMBER(raiden2_state::cop_dma_adr_w)
{
	COMBINE_DATA(&cop_dma_adr);
}

WRITE16_MEMBER(raiden2_state::cop_dma_size_w)
{
	COMBINE_DATA(&cop_dma_size);
}

WRITE16_MEMBER(raiden2_state::cop_dma_trigger_w)
{
	//  logerror("COP DMA mode=%x adr=%x size=%x vals=%x %x %x\n", cop_dma_mode, cop_dma_adr, cop_dma_size, cop_dma_v1, cop_dma_v2, cop_dma_v3);

	switch(cop_dma_mode) {
	case 0x14: {
		int rsize = 32*(0x7f-cop_dma_size);
		int radr = 64*cop_dma_adr - rsize;
		for(int i=0; i<rsize; i+=2)
			sprites[i/2] = space.read_word(radr+i);
		sprites_cur_start = rsize;
		break;
	}
	case 0x82: {
		UINT32 src,dst,size;
		int i;

		src = (cop_dma_adr << 6) + (cop_dma_adr_rel * 0x400);
		dst = (cop_dma_v3 << 6);
		size = ((cop_dma_size << 5) - (cop_dma_v3 << 6) + 0x20)/2;

		//printf("%08x %08x %08x\n",src,dst,size);

		for(i = 0;i < size;i++)
		{
			space.write_word(dst, space.read_word(src));
			src+=2;
			dst+=2;
		}

		break;
	}
	case 0x09: {
		UINT32 src,dst,size;
		int i;

		src = (cop_dma_adr << 6);
		dst = (cop_dma_v3 << 6);
		size = ((cop_dma_size << 5) - (cop_dma_v3 << 6) + 0x20)/2;

//      printf("%08x %08x %08x\n",src,dst,size);

		for(i = 0;i < size;i++)
		{
			space.write_word(dst, space.read_word(src));
			src+=2;
			dst+=2;
		}

		break;
	}
	}
}

WRITE16_MEMBER(raiden2_state::cop_itoa_low_w)
{
	cop_itoa = (cop_itoa & ~UINT32(mem_mask)) | (data & mem_mask);

	int digits = cop_itoa_digit_count;
	UINT32 val = cop_itoa;

	if(digits > 9)
		digits = 9;
	for(int i=0; i<digits; i++)
		if(!val && i)
			cop_itoa_digits[i] = 0x20;
		else {
			cop_itoa_digits[i] = 0x30 | (val % 10);
			val = val / 10;
		}
	cop_itoa_digits[9] = 0;
}

WRITE16_MEMBER(raiden2_state::cop_itoa_high_w)
{
	cop_itoa = (cop_itoa & ~(mem_mask << 16)) | ((data & mem_mask) << 16);
}

WRITE16_MEMBER(raiden2_state::cop_itoa_digit_count_w)
{
	COMBINE_DATA(&cop_itoa_digit_count);
}

READ16_MEMBER(raiden2_state::cop_itoa_digits_r)
{
	return cop_itoa_digits[offset*2] | (cop_itoa_digits[offset*2+1] << 8);
}

READ16_MEMBER(raiden2_state::cop_status_r)
{
	return cop_status;
}

READ16_MEMBER(raiden2_state::cop_angle_r)
{
	return cop_angle;
}

READ16_MEMBER(raiden2_state::cop_dist_r)
{
	return cop_dist;
}

WRITE16_MEMBER(raiden2_state::cop_scale_w)
{
	COMBINE_DATA(&cop_scale);
}

READ16_MEMBER(raiden2_state::cop_reg_high_r)
{
	return cop_regs[offset] >> 16;
}

WRITE16_MEMBER(raiden2_state::cop_reg_high_w)
{
	cop_regs[offset] = (cop_regs[offset] & ~(mem_mask << 16)) | ((data & mem_mask) << 16);
}

READ16_MEMBER(raiden2_state::cop_reg_low_r)
{
	return cop_regs[offset];
}

WRITE16_MEMBER(raiden2_state::cop_reg_low_w)
{
	cop_regs[offset] = (cop_regs[offset] & ~UINT32(mem_mask)) | (data & mem_mask);
}

WRITE16_MEMBER(raiden2_state::cop_cmd_w)
{
	cop_status &= 0x7fff;

	switch(data) {
	case 0x0205:   // 0205 0006 ffeb 0000 - 0188 0282 0082 0b8e 098e 0000 0000 0000
		space.write_dword(cop_regs[0] + 4 + offset*4, space.read_dword(cop_regs[0] + 4 + offset*4) + space.read_dword(cop_regs[0] + 16 + offset*4));
		break;

	case 0x130e:
	case 0x138e: { // 130e 0005 bf7f 0010 - 0984 0aa4 0d82 0aa2 039b 0b9a 0b9a 0a9a
		int dx = space.read_dword(cop_regs[1]+4) - space.read_dword(cop_regs[0]+4);
		int dy = space.read_dword(cop_regs[1]+8) - space.read_dword(cop_regs[0]+8);
		if(!dy) {
			cop_status |= 0x8000;
			cop_angle = 0;
		} else {
			cop_angle = atan(double(dx)/double(dy)) * 128 / M_PI;
			if(dy<0)
				cop_angle += 0x80;
		}
		dx = dx >> 16;
		dy = dy >> 16;
		cop_dist = sqrt((double)(dx*dx+dy*dy));

		if(data & 0x0080) {
			space.write_byte(cop_regs[0]+0x34, cop_angle);
			space.write_word(cop_regs[0]+0x38, cop_dist);
		}
		break;
	}

	case 0x3bb0: { // 3bb0 0004 007f 0038 - 0f9c 0b9c 0b9c 0b9c 0b9c 0b9c 0b9c 099c
		// called systematically after 130e/138e, no results expected it seems
		break;
	}

	case 0x42c2: { // 42c2 0005 fcdd 0040 - 0f9a 0b9a 0b9c 0b9c 0b9c 029c 0000 0000
		int div = space.read_word(cop_regs[0]+0x36);
		if(!div)
			div = 1;
		space.write_word(cop_regs[0]+0x38, (space.read_word(cop_regs[0]+0x38) << (5-cop_scale)) / div);
		break;
	}

	case 0x8100: { // 8100 0007 fdfb 0080 - 0b9a 0b88 0888 0000 0000 0000 0000 0000
		double angle = (space.read_word(cop_regs[0]+0x34) & 0xff) * M_PI / 128;
		double amp = 65536*(space.read_word(cop_regs[0]+0x36) & 0xff);
		space.write_dword(cop_regs[0] + 16, int(amp*sin(angle)) >> (5-cop_scale));
		break;
	}

	case 0x8900: { // 8900 0007 fdfb 0088 - 0b9a 0b8a 088a 0000 0000 0000 0000 0000
		double angle = (space.read_word(cop_regs[0]+0x34) & 0xff) * M_PI / 128;
		double amp = 65536*(space.read_word(cop_regs[0]+0x36) & 0xff);
		space.write_dword(cop_regs[0] + 20, int(amp*cos(angle)) >> (5-cop_scale));
		break;
	}

	case 0x5205:   // 5205 0006 fff7 0050 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		space.write_dword(cop_regs[1], space.read_dword(cop_regs[0]));
		break;

	case 0x5a05:   // 5a05 0006 fff7 0058 - 0180 02e0 03a0 00a0 03a0 0000 0000 0000
		space.write_dword(cop_regs[1], space.read_dword(cop_regs[0]));
		break;

	case 0xf205:   // f205 0006 fff7 00f0 - 0182 02e0 03c0 00c0 03c0 0000 0000 0000
		space.write_dword(cop_regs[2], space.read_dword(cop_regs[0]+4));
		break;

		// raidndx only
	case 0x7e05:
		space.write_dword(0x470, space.read_dword(cop_regs[3]));
		// Actually, wherever the bank selection actually is
		// And probably 8 bytes too, but they zero all the rest
		break;

	default:
		logerror("pcall %04x (%04x:%04x) [%x %x %x %x]\n", data, rps(space.machine), rpc(space.machine), cop_regs[0], cop_regs[1], cop_regs[2], cop_regs[3]);
	}
}

//  case 0x6ca:
//      logerror("select bank %d %04x\n", (data >> 15) & 1, data);
//      memory_set_bank(space.machine, "bank1", (data >> 15) & 1);


static void combine32(UINT32 *val, int offset, UINT16 data, UINT16 mem_mask)
{
	UINT16 *dest = (UINT16 *)val + BYTE_XOR_LE(offset);
	COMBINE_DATA(dest);
}



/* SPRITE DRAWING (move to video file) */

void raiden2_state::draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect ,int pri_mask )
{
	const UINT16 *source = sprites + sprites_cur_start/2 - 4;

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


	while( source>sprites ){
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

WRITE16_MEMBER(raiden2_state::raiden2_background_w)
{
	COMBINE_DATA(&back_data[offset]);
	tilemap_mark_tile_dirty(background_layer, offset);
}

WRITE16_MEMBER(raiden2_state::raiden2_midground_w)
{
	COMBINE_DATA(&mid_data[offset]);
	tilemap_mark_tile_dirty(midground_layer,offset);
}

WRITE16_MEMBER(raiden2_state::raiden2_foreground_w)
{
	COMBINE_DATA(&fore_data[offset]);
	tilemap_mark_tile_dirty(foreground_layer,offset);
}

WRITE16_MEMBER(raiden2_state::raiden2_text_w)
{
	COMBINE_DATA(&text_data[offset]);
	tilemap_mark_tile_dirty(text_layer, offset);
}

WRITE16_MEMBER(raiden2_state::tilemap_enable_w)
{
	COMBINE_DATA(&raiden2_tilemap_enable);
}

WRITE16_MEMBER(raiden2_state::tile_scroll_w)
{
	COMBINE_DATA(scrollvals + offset);
	data = scrollvals[offset];

	tilemap_t *tm = 0;
	switch(offset/2) {
	case 0: tm = background_layer; break;
	case 1: tm = midground_layer; break;
	case 2: tm = foreground_layer; break;
	}
	if(offset & 1)
		tilemap_set_scrolly(tm, 0, data);
	else
		tilemap_set_scrollx(tm, 0, data);
}

WRITE16_MEMBER(raiden2_state::tile_bank_01_w)
{
	if(ACCESSING_BITS_0_7) {
		int new_bank;
		new_bank = 0 | (data & 1);
		if(new_bank != bg_bank) {
			bg_bank = new_bank;
			tilemap_mark_all_tiles_dirty(background_layer);
		}

		new_bank = 2 | ((data >> 1) & 1);
		if(new_bank != mid_bank) {
			mid_bank = new_bank;
			tilemap_mark_all_tiles_dirty(midground_layer);
		}
	}
}

WRITE16_MEMBER(raiden2_state::cop_tile_bank_2_w)
{
	if(ACCESSING_BITS_8_15) {
		int new_bank = 4 | (data >> 14);
		if(new_bank != fg_bank) {
			fg_bank = new_bank;
			tilemap_mark_all_tiles_dirty(foreground_layer);
		}
	}
}

/* TILEMAP RELATED (move to video file) */

static TILE_GET_INFO( get_back_tile_info )
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	int tile = state->back_data[tile_index];
	int color = (tile >> 12) | (0 << 4);

	tile = (tile & 0xfff) | (state->bg_bank << 12);

	SET_TILE_INFO(1,tile+0x0000,color,0);
}

static TILE_GET_INFO( get_mid_tile_info )
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	int tile = state->mid_data[tile_index];
	int color = (tile >> 12) | (2 << 4);

	tile = (tile & 0xfff) | (state->mid_bank << 12);

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_fore_tile_info )
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	int tile = state->fore_data[tile_index];
	int color = (tile >> 12) | (1 << 4);

	tile = (tile & 0xfff) | (state->fg_bank << 12);

	SET_TILE_INFO(1,tile,color,0);
}

static TILE_GET_INFO( get_text_tile_info )
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	int tile = state->text_data[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO(0,tile,color,0);
}

/* VIDEO START (move to video file) */

static VIDEO_START( raiden2 )
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	state->text_layer       = tilemap_create(machine, get_text_tile_info, tilemap_scan_rows,  8, 8, 64,32 );
	state->background_layer = tilemap_create(machine, get_back_tile_info, tilemap_scan_rows, 16,16, 32,32 );
	state->midground_layer  = tilemap_create(machine, get_mid_tile_info,  tilemap_scan_rows, 16,16, 32,32 );
	state->foreground_layer = tilemap_create(machine, get_fore_tile_info, tilemap_scan_rows, 16,16, 32,32 );

	tilemap_set_transparent_pen(state->midground_layer, 15);
	tilemap_set_transparent_pen(state->foreground_layer, 15);
	tilemap_set_transparent_pen(state->text_layer, 15);
}

/* VIDEO UPDATE (move to video file) */

static VIDEO_UPDATE ( raiden2 )
{
	raiden2_state *state = screen->machine->driver_data<raiden2_state>();
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	if (!input_code_pressed(screen->machine, KEYCODE_Q))
	{
		if (!(state->raiden2_tilemap_enable & 1))
			tilemap_draw(bitmap, cliprect, state->background_layer, 0, 0);
	}

	if (!input_code_pressed(screen->machine, KEYCODE_W))
	{
		if (!(state->raiden2_tilemap_enable & 2))
			tilemap_draw(bitmap, cliprect, state->midground_layer, 0, 0);
	}

	if (!input_code_pressed(screen->machine, KEYCODE_E))
	{
		if (!(state->raiden2_tilemap_enable & 4))
			tilemap_draw(bitmap, cliprect, state->foreground_layer, 0, 0);
	}

	if (!input_code_pressed(screen->machine, KEYCODE_S))
	{
		//if (!(raiden2_tilemap_enable & 0x10))
			state->draw_sprites(screen->machine, bitmap, cliprect, 0);
	}

	if (!input_code_pressed(screen->machine, KEYCODE_A))
	{
		if (!(state->raiden2_tilemap_enable & 8))
			tilemap_draw(bitmap, cliprect, state->text_layer, 0, 0);
	}

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


WRITE16_MEMBER(raiden2_state::sprcpt_adr_w)
{
	combine32(&sprcpt_adr, offset, data, mem_mask);
}

WRITE16_MEMBER(raiden2_state::sprcpt_data_1_w)
{
	combine32(sprcpt_data_1+sprcpt_adr, offset, data, mem_mask);
}

WRITE16_MEMBER(raiden2_state::sprcpt_data_2_w)
{
	combine32(sprcpt_data_2+sprcpt_adr, offset, data, mem_mask);
}

WRITE16_MEMBER(raiden2_state::sprcpt_data_3_w)
{
	combine32(sprcpt_data_3+sprcpt_idx, offset, data, mem_mask);
	if(offset == 1) {
		sprcpt_idx ++;
		if(sprcpt_idx == 6)
			sprcpt_idx = 0;
	}
}

WRITE16_MEMBER(raiden2_state::sprcpt_data_4_w)
{
	combine32(sprcpt_data_4+sprcpt_idx, offset, data, mem_mask);
	if(offset == 1) {
		sprcpt_idx ++;
		if(sprcpt_idx == 4)
			sprcpt_idx = 0;
	}
}

WRITE16_MEMBER(raiden2_state::sprcpt_val_1_w)
{
	combine32(sprcpt_val+0, offset, data, mem_mask);
}

WRITE16_MEMBER(raiden2_state::sprcpt_val_2_w)
{
	combine32(sprcpt_val+1, offset, data, mem_mask);
}

WRITE16_MEMBER(raiden2_state::sprcpt_flags_1_w)
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

WRITE16_MEMBER(raiden2_state::sprcpt_flags_2_w)
{
	COMBINE_DATA(&sprcpt_flags2);
	if(offset == 0) {
		if(sprcpt_flags2 & 0x8000) {
			// Reset decryption -> redo it
		}
	}
}



void raiden2_state::common_reset()
{
	bg_bank=0;
	fg_bank=6;
	mid_bank=1;
}

static MACHINE_RESET(raiden2)
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	state->common_reset();
	sprcpt_init();
	MACHINE_RESET_CALL(seibu_sound);

	memory_set_bank(machine, "mainbank", 1);

	//cop_init();
}

static MACHINE_RESET(zeroteam)
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	state->bg_bank = 0;
	state->fg_bank = 2;
	state->mid_bank = 1;
	sprcpt_init();
	MACHINE_RESET_CALL(seibu_sound);

	memory_set_bank(machine, "mainbank", 1);

	//cop_init();
}

static MACHINE_RESET(xsedae)
{
	raiden2_state *state = machine->driver_data<raiden2_state>();
	state->bg_bank = 0;
	state->fg_bank = 2;
	state->mid_bank = 1;
	sprcpt_init();
	MACHINE_RESET_CALL(seibu_sound);

	//memory_set_bank(machine, "mainbank", 1);

	//cop_init();
}

READ16_MEMBER(raiden2_state::raiden2_sound_comms_r)
{
	switch(offset)
	{
		case (0x008/2):	return seibu_main_word_r(&space,2,0xffff);
		case (0x00c/2):	return seibu_main_word_r(&space,3,0xffff);
		case (0x014/2): return seibu_main_word_r(&space,5,0xffff);
	}

	return 0xffff;
}

WRITE16_MEMBER(raiden2_state::raiden2_sound_comms_w)
{
	switch(offset)
	{
		case (0x000/2):	{ seibu_main_word_w(&space,0,data,0x00ff); break; }
		case (0x004/2):	{ seibu_main_word_w(&space,1,data,0x00ff); break; }
		case (0x010/2):	{ seibu_main_word_w(&space,4,data,0x00ff); break; }
		case (0x018/2):	{ seibu_main_word_w(&space,6,data,0x00ff); break; }
	}
}

WRITE16_MEMBER(raiden2_state::raiden2_bank_w)
{
	if(ACCESSING_BITS_8_15) {
		logerror("select bank %d %04x\n", (data >> 15) & 1, data);
		memory_set_bank(space.machine, "mainbank", !((data >> 15) & 1));
	}
}

/* MEMORY MAPS */
static ADDRESS_MAP_START( raiden2_cop_mem, ADDRESS_SPACE_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00420, 0x00421) AM_WRITE(cop_itoa_low_w)
	AM_RANGE(0x00422, 0x00423) AM_WRITE(cop_itoa_high_w)
	AM_RANGE(0x00424, 0x00425) AM_WRITE(cop_itoa_digit_count_w)
	AM_RANGE(0x00428, 0x00429) AM_WRITE(cop_dma_v1_w)
	AM_RANGE(0x0042a, 0x0042b) AM_WRITE(cop_dma_v2_w)
	AM_RANGE(0x00432, 0x00433) AM_WRITE(cop_pgm_data_w)
	AM_RANGE(0x00434, 0x00435) AM_WRITE(cop_pgm_addr_w)
	AM_RANGE(0x00438, 0x00439) AM_WRITE(cop_pgm_value_w)
	AM_RANGE(0x0043a, 0x0043b) AM_WRITE(cop_pgm_mask_w)
	AM_RANGE(0x0043c, 0x0043d) AM_WRITE(cop_pgm_trigger_w)
	AM_RANGE(0x00444, 0x00445) AM_WRITE(cop_scale_w)
	AM_RANGE(0x00470, 0x00471) AM_WRITE(cop_tile_bank_2_w)

	AM_RANGE(0x00476, 0x00477) AM_WRITE(cop_dma_adr_rel_w)
	AM_RANGE(0x00478, 0x00479) AM_WRITE(cop_dma_adr_w)
	AM_RANGE(0x0047a, 0x0047b) AM_WRITE(cop_dma_size_w)
	AM_RANGE(0x0047c, 0x0047d) AM_WRITE(cop_dma_v3_w)
	AM_RANGE(0x0047e, 0x0047f) AM_WRITE(cop_dma_mode_w)
	AM_RANGE(0x004a0, 0x004a7) AM_READWRITE(cop_reg_high_r, cop_reg_high_w)
	AM_RANGE(0x004c0, 0x004c7) AM_READWRITE(cop_reg_low_r, cop_reg_low_w)
	AM_RANGE(0x00500, 0x00505) AM_WRITE(cop_cmd_w)
	AM_RANGE(0x00590, 0x00599) AM_READ(cop_itoa_digits_r)
	AM_RANGE(0x005b0, 0x005b1) AM_READ(cop_status_r)
	AM_RANGE(0x005b2, 0x005b3) AM_READ(cop_dist_r)
	AM_RANGE(0x005b4, 0x005b5) AM_READ(cop_angle_r)

	AM_RANGE(0x0061c, 0x0061d) AM_WRITE(tilemap_enable_w)
	AM_RANGE(0x00620, 0x0062b) AM_WRITE(tile_scroll_w)
	AM_RANGE(0x006a0, 0x006a3) AM_WRITE(sprcpt_val_1_w)
	AM_RANGE(0x006a4, 0x006a7) AM_WRITE(sprcpt_data_3_w)
	AM_RANGE(0x006a8, 0x006ab) AM_WRITE(sprcpt_data_4_w)
	AM_RANGE(0x006ac, 0x006af) AM_WRITE(sprcpt_flags_1_w)
	AM_RANGE(0x006b0, 0x006b3) AM_WRITE(sprcpt_data_1_w)
	AM_RANGE(0x006b4, 0x006b7) AM_WRITE(sprcpt_data_2_w)
	AM_RANGE(0x006b8, 0x006bb) AM_WRITE(sprcpt_val_2_w)
	AM_RANGE(0x006bc, 0x006bf) AM_WRITE(sprcpt_adr_w)
	AM_RANGE(0x006ca, 0x006cb) AM_WRITE(raiden2_bank_w)
	AM_RANGE(0x006cc, 0x006cd) AM_WRITE(tile_bank_01_w)
	AM_RANGE(0x006ce, 0x006cf) AM_WRITE(sprcpt_flags_2_w)
	AM_RANGE(0x006fc, 0x006fd) AM_WRITE(cop_dma_trigger_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( raiden2_mem, ADDRESS_SPACE_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_IMPORT_FROM( raiden2_cop_mem )

	AM_RANGE(0x00700, 0x0071f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("CONTROLS")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x00800, 0x0cfff) AM_RAM

	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(raiden2_background_w) AM_BASE(back_data)
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(raiden2_foreground_w) AM_BASE(fore_data)
    AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(raiden2_midground_w)  AM_BASE(mid_data)
    AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(raiden2_text_w) AM_BASE(text_data)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */

	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM AM_WRITE_LEGACY(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("mainbank")
	AM_RANGE(0x40000, 0xfffff) AM_ROM AM_REGION("mainprg", 0x40000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zeroteam_mem, ADDRESS_SPACE_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x00470, 0x00471) AM_WRITENOP
	AM_RANGE(0x006cc, 0x006cd) AM_WRITENOP

	AM_IMPORT_FROM( raiden2_cop_mem )

	AM_RANGE(0x00700, 0x0071f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("CONTROLS")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x0b800, 0x0bfff) AM_RAM_WRITE(raiden2_background_w) AM_BASE(back_data)
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM_WRITE(raiden2_foreground_w) AM_BASE(fore_data)
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM_WRITE(raiden2_midground_w) AM_BASE(mid_data)
    AM_RANGE(0x0d000, 0x0dfff) AM_RAM_WRITE(raiden2_text_w) AM_BASE(text_data)
	AM_RANGE(0x0e000, 0x0efff) AM_RAM AM_WRITE_LEGACY(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x00800, 0x0ffff) AM_RAM
	AM_RANGE(0x10000, 0x1ffff) AM_RAM

	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("mainbank")
	AM_RANGE(0x40000, 0xfffff) AM_ROM AM_REGION("mainprg", 0x40000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xsedae_mem, ADDRESS_SPACE_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x00470, 0x00471) AM_WRITENOP
	AM_RANGE(0x006cc, 0x006cd) AM_WRITENOP

	AM_IMPORT_FROM( raiden2_cop_mem )

	AM_RANGE(0x00700, 0x0071f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("CONTROLS")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x0b800, 0x0bfff) AM_RAM_WRITE(raiden2_background_w) AM_BASE(back_data)
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM_WRITE(raiden2_foreground_w) AM_BASE(fore_data)
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM_WRITE(raiden2_midground_w) AM_BASE(mid_data)
    AM_RANGE(0x0d000, 0x0dfff) AM_RAM_WRITE(raiden2_text_w) AM_BASE(text_data)
	AM_RANGE(0x0e000, 0x0efff) AM_RAM AM_WRITE_LEGACY(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x00800, 0x0ffff) AM_RAM
	AM_RANGE(0x10000, 0x1ffff) AM_RAM

	AM_RANGE(0x20000, 0xfffff) AM_ROM AM_REGION("mainprg", 0x20000)
ADDRESS_MAP_END


READ16_MEMBER(raiden2_state::rdx_v33_system_r )
{
	return input_port_read(space.machine, "SYSTEM");
}


READ16_MEMBER(raiden2_state::r2_playerin_r )
{
	return input_port_read(space.machine, "INPUT");
}

READ16_MEMBER(raiden2_state::rdx_v33_unknown_r )
{
	return 0x0000;
}

READ16_MEMBER(raiden2_state::rdx_v33_unknown2_r )
{
	return 0x0000;
}

READ16_MEMBER(raiden2_state::nzerotea_unknown_r )
{
	return 0xffff;
}


/* INPUT PORTS */

static INPUT_PORTS_START( raiden2 )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("CONTROLS")	/* IN0/1 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")	/* Dip switches  */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x3000, "200000 500000" )
	PORT_DIPSETTING(      0x2000, "400000 1000000" )
	PORT_DIPSETTING(      0x1000, "1000000 3000000" )
	PORT_DIPSETTING(      0x0000, "No Extend" )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")	/* START BUTTONS */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( raidendx )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("CONTROLS")	/* IN0/1 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")	/* Dip switches  */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) /* Manual shows "Not Used" */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) /* Manual shows "Not Used" */
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")	/* START BUTTONS */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( zeroteam )
	PORT_INCLUDE( raiden2 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW0" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xsedae )
	PORT_INCLUDE( raiden2 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW0" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
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

static MACHINE_CONFIG_START( raiden2, raiden2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,XTAL_32MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(raiden2_mem)
	MCFG_CPU_VBLANK_INT("screen", raiden2_interrupt)

	MCFG_MACHINE_RESET(raiden2)

	SEIBU2_RAIDEN2_SOUND_SYSTEM_CPU(14318180/4)


	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate *//2)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 30*8-1)
	MCFG_GFXDECODE(raiden2)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(raiden2)
	MCFG_VIDEO_UPDATE(raiden2)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_RAIDEN2_INTERFACE(28636360/8,28636360/28,1,2)


/* Sound hardware infos: Z80 and YM2151 are clocked at XTAL_28_63636MHz/8 */
/* The 2 Oki M6295 are clocked at XTAL_28_63636MHz/28 and pin 7 is high for both */

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xsedae, raiden2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(xsedae_mem)

	MCFG_MACHINE_RESET(xsedae)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 32*8-1)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( zeroteam, raiden2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,XTAL_32MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(zeroteam_mem)
	MCFG_CPU_VBLANK_INT("screen", raiden2_interrupt)

	MCFG_MACHINE_RESET(zeroteam)

	SEIBU_NEWZEROTEAM_SOUND_SYSTEM_CPU(14318180/4)


	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate *//2)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 32*8-1)
	MCFG_GFXDECODE(raiden2)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(raiden2)
	MCFG_VIDEO_UPDATE(raiden2)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

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
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("prg1",   0x000001, 0x80000, CRC(4609b5f2) SHA1(272d2aa75b8ea4d133daddf42c4fc9089093df2e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "snd",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
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

ROM_START( raiden2a )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2e",  0x000001, 0x80000, CRC(458d619c) SHA1(842bf0eeb5d192a6b188f4560793db8dad697683) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
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

ROM_START( raiden2b )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2j",  0x000001, 0x80000, CRC(e4e4fb4c) SHA1(7ccf33fe9a1cddf0c7e80d7ed66d615a828b3bb9) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
ROM_END

ROM_START( raiden2c )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu1",   0x000000, 0x80000, CRC(c1fc70f5) SHA1(a054f5ae9583972c406d9cf871340d5e072d71a3) ) /* Italian set */
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu2",   0x000001, 0x80000, CRC(28d5365f) SHA1(21efe29c2d373229c2ff302d86e59c2c94fa6d03) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5",  0x000000, 0x08000, CRC(5db9f922) SHA1(8257aab98657fe44df19d2a48d85fcf65b3d98c6) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
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

ROM_START( raiden2d )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("r2_prg_0.bin",   0x000000, 0x80000, CRC(2abc848c) SHA1(1df4276d0074fcf1267757fa0b525a980a520f3d) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2_prg_1.bin",   0x000001, 0x80000, CRC(509ade43) SHA1(7cdee7bb00a6a1c7899d10b96385d54c261f6f5a) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2_snd.bin", 0x000000, 0x08000, CRC(6bad0a3e) SHA1(eb7ae42353e1984cd60b569c26cdbc3b025a7da6) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2_fx0.bin",	0x000000, 0x020000, CRC(c709bdf6) SHA1(0468d90412b7590d67eaadc0a5e3537cd5e73943) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2_voi1.bin", 0x00000, 0x40000, CRC(488d050f) SHA1(fde2fd64fea6bc39e1a42885d21d362bc6be2ac2) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
ROM_END

ROM_START( raiden2e )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("r2.1",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2.2",  0x000001, 0x80000, CRC(bf7577ec) SHA1(98576af78760b8aef1ef3efe1ba963977c89d225) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2.5", 0x000000, 0x08000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2.7", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2.6", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
ROM_END

ROM_START( raiden2f ) // same as raiden2e, different region
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu_1.uo211",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu_2.uo212",  0x000001, 0x80000, CRC(beb71ddb) SHA1(471399ead1cdc27ac2a1139f9616f828efd14626) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2.5", 0x000000, 0x08000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2.7", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "r2.6", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */
ROM_END


/* Raiden DX sets */

ROM_START( raidndx )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.4n",   0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.4p",   0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.6n",   0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.6p",   0x000003, 0x80000, CRC(2a2003e8) SHA1(f239b351759babe4683d16e745a5ac2f3c2ab06b) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

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
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidndxa1 )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("dx_1h.4n",   0x000000, 0x80000, BAD_DUMP CRC(7624c36b) SHA1(84c17f2988031210d06536710e1eac558f4290a1) ) // bad
	ROM_LOAD32_BYTE("dx_2h.4p",   0x000001, 0x80000, CRC(4940fdf3) SHA1(c87e307ed7191802583bee443c7c8e4f4e33db25) )
	ROM_LOAD32_BYTE("dx_3h.6n",   0x000002, 0x80000, CRC(6c495bcf) SHA1(fb6153ecc443dabc829dda6f8d11234ad48de88a) )
	ROM_LOAD32_BYTE("dx_4h.6k",   0x000003, 0x80000, CRC(9ed6335f) SHA1(66975204b120915f23258a431e19dbc017afd912) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

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
ROM_END

ROM_START( raidndxa2 )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.bin",   0x000000, 0x80000, CRC(22b155ae) SHA1(388151e2c8fb301bd5bc66a974e9fe16816ae0bc) )
	ROM_LOAD32_BYTE("2d.bin",   0x000001, 0x80000, CRC(2be98ca8) SHA1(491e990405b0ad3de45bdbcc2453af9215ae19c8) )
	ROM_LOAD32_BYTE("3d.bin",   0x000002, 0x80000, CRC(b4785576) SHA1(aa5eee7b0c635c6d18a7fc1e037bf570a677dd90) )
	ROM_LOAD32_BYTE("4d.bin",   0x000003, 0x80000, CRC(5a77f7b4) SHA1(aa757e6308893ca63963170c5b1743de7c7ab034) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

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
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidndxj )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("rdxj_1.bin",   0x000000, 0x80000, CRC(b5b32885) SHA1(fb3c592b2436d347103c17bd765176062be95fa2) )
	ROM_LOAD32_BYTE("rdxj_2.bin",   0x000001, 0x80000, CRC(7efd581d) SHA1(4609a0d8afb3d62a38b461089295efed47beea91) )
	ROM_LOAD32_BYTE("rdxj_3.bin",   0x000002, 0x80000, CRC(55ec0e1d) SHA1(6be7f268df51311a817c1c329a578b38abb659ae) )
	ROM_LOAD32_BYTE("rdxj_4.bin",   0x000003, 0x80000, CRC(f8fb31b4) SHA1(b72fd7cbbebcf3d1b2253c309fcfa60674776467) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

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
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidndxu )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1a.u1210", 0x000000, 0x80000, CRC(53e63194) SHA1(a957330e14649cf46ad27fb99c460576c59e60b1) )
	ROM_LOAD32_BYTE("2a.u1211", 0x000001, 0x80000, CRC(ec8d1647) SHA1(5ceae132c6c09d6bb8565e9141ee1170bbdfd5fc) )
	ROM_LOAD32_BYTE("3a.u129",  0x000002, 0x80000, CRC(7dbfd73d) SHA1(43cb1dbc3ccbded64fc300c262d1fd528e0391a2) )
	ROM_LOAD32_BYTE("4a.u1212", 0x000003, 0x80000, CRC(cb41a459) SHA1(532f0ed00a5b50a7537e5f48884d632aa5b92fb0) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

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
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidndxg )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.u1210", 0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.u1211", 0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.u129",  0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.u1212", 0x000003, 0x80000, CRC(6bde6edc) SHA1(c3565a55b858c10659fd9b93b1cd92bc39e6446d) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.u1110", 0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

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
	ROM_LOAD( "pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END


/* Zero Team sets */

ROM_START( zeroteam )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.5k",   0x000000, 0x40000, CRC(25aa5ba4) SHA1(40e6047620fbd195c87ac3763569af099096eff9) )
	ROM_LOAD32_BYTE("3.6k",   0x000002, 0x40000, CRC(ec79a12b) SHA1(515026a2fca92555284ac49818499af7395783d3) )
	ROM_LOAD32_BYTE("2.6l",   0x000001, 0x40000, CRC(54f3d359) SHA1(869744185746d55c60d2f48eabe384a8499e00fd) )
	ROM_LOAD32_BYTE("4.5l",   0x000003, 0x40000, CRC(a017b8d0) SHA1(4a93ff1ab18f4b61c7ef580995f64840c19ce6b9) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.5s",	0x000000,	0x010000,	CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.5r",	0x000001,	0x010000,	CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a
ROM_END

ROM_START( zeroteama )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.bin",   0x000000, 0x40000, CRC(bd7b3f3a) SHA1(896413901a429d0efa3290f61920063c81730e9b) )
	ROM_LOAD32_BYTE("3.bin",   0x000002, 0x40000, CRC(19e02822) SHA1(36c9b887eaa9b9b67d65c55e8f7eefd08fe0be15) )
	ROM_LOAD32_BYTE("2.bin",   0x000001, 0x40000, CRC(0580b7e8) SHA1(d4416264aa5acdaa781ebcf51f128b3e665cc903) )
	ROM_LOAD32_BYTE("4.bin",   0x000003, 0x40000, CRC(cc666385) SHA1(23a8878315b6009dcc1f27e49572e5be29f6a1a6) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.bin",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.bin",	0x000000,	0x010000, CRC(eb10467f) SHA1(fc7d576dc41bc878ff20f0370e669e19d54fcefb) )
	ROM_LOAD16_BYTE( "8.bin",	0x000001,	0x010000, CRC(a0b2a09a) SHA1(9b1f6c732000b84b1ad635f332ebead5d65cc491) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin
ROM_END

ROM_START( zeroteams )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1_sel.bin",   0x000000, 0x40000, CRC(d99d6273) SHA1(21dccd5d71c720b8364406835812b3c9defaff6c) )
	ROM_LOAD32_BYTE("3_sel.bin",   0x000002, 0x40000, CRC(0a9fe0b1) SHA1(3588fe19788f77d07e9b5ab8182b94362ffd0024) )
	ROM_LOAD32_BYTE("2_sel.bin",   0x000001, 0x40000, CRC(4e114e74) SHA1(fcccbb68c6b7ffe8d109ed3a1ec9120d338398f9) )
	ROM_LOAD32_BYTE("4_sel.bin",   0x000003, 0x40000, CRC(0df8ba94) SHA1(e07dce6cf3c3cfe1ea3b7f01e18833c1da5ed1dc) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5_sel.bin",  0x000000, 0x08000, CRC(ed91046c) SHA1(de815c999aeeb814d3f091d5a9ac34ea9a388ddb) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.5s",	0x000000,	0x010000,	CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.5r",	0x000001,	0x010000,	CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin
ROM_END

/* set contained only program roms, was marked as 'non-encrytped' but program isn't encrypted anyway?! */
ROM_START( zeroteamb )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("z1",   0x000000, 0x40000, CRC(157743d0) SHA1(f9c84c9025319f76807ef0e79f1ee1599f915b45) )
	ROM_LOAD32_BYTE("z3",   0x000002, 0x40000, CRC(fea7e4e8) SHA1(08c4bdff82362ae4bcf86fa56fcfc384bbf82b71) )
	ROM_LOAD32_BYTE("z2",   0x000001, 0x40000, CRC(21d68f62) SHA1(8aa85b38e8f36057ef6c7dce5a2878958ce93ce8) )
	ROM_LOAD32_BYTE("z4",   0x000003, 0x40000, CRC(ce8fe6c2) SHA1(69627867c7866e43e771ab85014553117044d18d) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "sound",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.5s",	0x000000,	0x010000,	CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.5r",	0x000001,	0x010000,	CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.pcm", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a
ROM_END

ROM_START( zeroteamc )
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("b1.024",   0x000000, 0x40000, CRC(528de3b9) SHA1(9ca8cdc0212f2540e852d20ab4c04f68b967d024) )
	ROM_LOAD32_BYTE("b3.023",   0x000002, 0x40000, CRC(3688739a) SHA1(f98f461fb8e7804b3b4020a5e3762d36d6458a62) )
	ROM_LOAD32_BYTE("b2.025",   0x000001, 0x40000, CRC(5176015e) SHA1(6b372564b2f1b1f56cae0c98f4ca588b784bfa3d) )
	ROM_LOAD32_BYTE("b4.026",   0x000003, 0x40000, CRC(c79925cb) SHA1(aaff9f626ec61bc0ff038ebd722fe361dccc49fb) )

	ROM_REGION( 0x40000, "user2", 0 )	/* COPX */
	ROM_LOAD( "copx-d2",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.bin",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "b7.072",	0x000000,	0x010000, CRC(30ec0241) SHA1(a0d0be9458bf97cb9764fb85c988bb816710475e) )
	ROM_LOAD16_BYTE( "b8.077",	0x000001,	0x010000, CRC(e18b3a75) SHA1(3d52bba8d47d0d9108ee79014fd64d6e856a3fde) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "back-1",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "back-2",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "obj-1",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "obj-2",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "6.105", 0x00000, 0x40000,  CRC(b4a6e899) SHA1(175ab656db3c3258ff10eede89890f62435d2298) )
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
	ROM_REGION( 0x200000, "mainprg", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.u024",   0x000000, 0x40000, CRC(185437f9) SHA1(e46950b6a549d11dc57105dd7d9cb512a8ecbe70) )
	ROM_LOAD32_BYTE("2.u025",   0x000001, 0x40000, CRC(a2b052df) SHA1(e8bf9ab3d5d4e601ea9386e1f2d4e017b025407e) )
	ROM_LOAD32_BYTE("3.u023",   0x000002, 0x40000, CRC(293fd6c1) SHA1(8b1a231f4bedbf9c0f347330e13fdf092b9888b4) )
	ROM_LOAD32_BYTE("4.u026",   0x000003, 0x40000, CRC(5adf20bf) SHA1(42a0bb5a460c656675b2c432c043fc61a9049276) )

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )	/* COPX */
	/* Not populated */

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u1110",  0x000000, 0x08000, CRC(2dc2f81a) SHA1(0f6605042e0e295b4256b43dbdf5d53daebe1a9a) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_CONTINUE(0x20000,0x10000) // TODO
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "5.u077",	0x000001,	0x010000, CRC(478deced) SHA1(88cd72cb76bbc1c4255c3dfae4b9a10af9b050b2) )
	ROM_LOAD16_BYTE( "6.u072",	0x000000,	0x010000, CRC(a788402d) SHA1(8a1ac4760cf75cd2e32c1d15f36ad15cce3d411b) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg-1.u075",   0x000000, 0x100000, CRC(ac087560) SHA1(b6473b20c55ec090961cfc46a024b3c5b707ec25) )
	ROM_LOAD( "7.u0714",     0x100000, 0x080000, CRC(296105dc) SHA1(c2b80d681646f504b03c2dde13e37b1d820f82d2) )

	ROM_REGION( 0x800000, "gfx3", 0 ) /* sprite gfx (not encrypted) */
	ROM_LOAD32_WORD( "obj-1.u0811",  0x000000, 0x200000, CRC(6ae993eb) SHA1(d9713c79eacb4b3ce5e82dd3ce39003e3a433d8f) )
	ROM_LOAD32_WORD( "obj-2.u082",   0x000002, 0x200000, CRC(26c806ee) SHA1(899a76a1b3f933c6f5cb6b5dcdf5b58e1b7e49c6) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* ADPCM samples */
	ROM_LOAD( "9.u105", 0x00000, 0x40000, CRC(a7a0c5f9) SHA1(7882681ac152642aa4f859071f195842068b214b) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )	/* ADPCM samples */
ROM_END

static DRIVER_INIT (raiden2)
{
	memory_configure_bank(machine, "mainbank", 0, 2, machine->region("mainprg")->base(), 0x20000);
	raiden2_decrypt_sprites(machine);
}

static DRIVER_INIT (xsedae)
{
	/* doesn't have banking */
	//memory_configure_bank(machine, "mainbank", 0, 2, machine->region("mainprg")->base(), 0x20000);
}

/* GAME DRIVERS */

// rev numbers at end of the line just indicate which sets are the same code revisions (just a region byte change), they don't reflect the actual order of release
GAME( 1993, raiden2,  0,       raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden II (set 1, US Fabtek)",     GAME_NOT_WORKING) // rev 1
GAME( 1993, raiden2a, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden II (set 2, Metrotainment)", GAME_NOT_WORKING) //  ^
GAME( 1993, raiden2b, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu",                         "Raiden II (set 3, Japan)",         GAME_NOT_WORKING) //  ^
GAME( 1993, raiden2c, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu",                         "Raiden II (set 4, Italy)",         GAME_NOT_WORKING) // rev 2
GAME( 1993, raiden2d, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu",                         "Raiden II (set 5, Easy Version)",  GAME_NOT_WORKING) // rev 3
GAME( 1993, raiden2e, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu",                         "Raiden II (set 6)",                GAME_NOT_WORKING) // rev 4
GAME( 1993, raiden2f, raiden2, raiden2,  raiden2,  raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden II (set 7, US Fabtek)",     GAME_NOT_WORKING) //  ^

GAME( 1994, raidndx,  0,       raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu",                         "Raiden DX (UK)",                   GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1994, raidndxa1,raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden DX (Asia set 1)",           GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1994, raidndxa2,raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden DX (Asia set 2)",           GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1994, raidndxj, raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu",                         "Raiden DX (Japan)",                GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1994, raidndxu, raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)",        "Raiden DX (US)",                   GAME_NOT_WORKING|GAME_NO_SOUND)
GAME( 1994, raidndxg, raidndx, raiden2,  raidendx, raiden2,  ROT270, "Seibu Kaihatsu (Tuning license)",        "Raiden DX (Germany)",              GAME_NOT_WORKING|GAME_NO_SOUND)

GAME( 1993, zeroteam, 0,       zeroteam, zeroteam,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 1)", GAME_NOT_WORKING)
GAME( 1993, zeroteama,zeroteam,zeroteam, zeroteam,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 2)", GAME_NOT_WORKING)
GAME( 1993, zeroteamb,zeroteam,zeroteam, zeroteam,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 3)", GAME_NOT_WORKING)
GAME( 1993, zeroteamc,zeroteam,zeroteam, zeroteam,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team (set 4)", GAME_NOT_WORKING)
GAME( 1993, zeroteams,zeroteam,zeroteam, zeroteam,  raiden2,  ROT0,   "Seibu Kaihatsu", "Zero Team Selection", GAME_NOT_WORKING)

GAME( 1995, xsedae,   0,       xsedae,   xsedae,  xsedae,   ROT0,   "Dream Island",   "X Se Dae Quiz", GAME_NOT_WORKING)
