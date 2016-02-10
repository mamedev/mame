// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/********************************************************************************************************

    Seibu Protected 1993-94 era hardware, V30 based (sequel to the 68k based hardware)

    TODO:
    * zeroteam - sort-DMA doesn't seem to work too well, sprite-sprite priorities are broken as per now

    * xsedae - it does an "8-liner"-style scroll during attract, doesn't work too well.

    * sprite chip is the same as seibuspi.c and feversoc.c, needs device-ification and merging.

    * sprite chip also uses first entry for "something" that isn't sprite, some of them looks clipping
      regions (150 - ff in zeroteam, 150 - 0 and 150 - 80 in raiden2). Latter probably do double buffering
      on odd/even frames, by updating only top or bottom part of screen.

===========================================================================================================

raiden 2 board test note 17/04/08 (based on test by dox)

 rom banking is at 6c9, bit 0x80
  -- the game only writes this directly at startup, must be written indirectly by
     one of the protection commands? or mirrored?
  value of 0x80 puts 0x00000-0x1ffff at 0x20000 - 0x3ffff
  value of 0x00 puts 0x20000-0x3ffff at 0x20000 - 0x3ffff


===========================================================================================================

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

 + variants

Some of these games were also released on updated PCBs
which usually featured vastly inferior sound hardware
 (see the V33 based version of Raiden II/DX New)


Protection Notes:
 These games use the 2nd (and 3rd) generation of Seibu's 'COP' protection,
 utilizing the external 'COPX_D2' and 'COPX_D3' lookup roms (probably for
 math operations)  These chips, marked (c)1992 RISE Corp. are not thought
 to be the actual MCU which is probably internal to one of the Seibu
 customs.

 The games in legionna.c use (almost?) the same protection chips.

********************************************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "includes/raiden2.h"


void raiden2_state::machine_start()
{
	save_item(NAME(bg_bank));
	save_item(NAME(fg_bank));
	save_item(NAME(mid_bank));
	save_item(NAME(tx_bank));
	save_item(NAME(raiden2_tilemap_enable));
	save_item(NAME(prg_bank));
	save_item(NAME(cop_bank));

	save_item(NAME(sprite_prot_x));
	save_item(NAME(sprite_prot_y));
	save_item(NAME(dst1));
	save_item(NAME(cop_spr_maxx));
	save_item(NAME(cop_spr_off));


	save_item(NAME(scrollvals));

	save_item(NAME(sprite_prot_src_addr));

}

/*
UINT16 raiden2_state::rps()
{
    return m_maincpu->state_int(NEC_CS);
}

UINT16 raiden2_state::rpc()
{
    return m_maincpu->state_int(NEC_IP);
}
*/

int cnt=0, ccol = -1;


WRITE16_MEMBER(raiden2_state::m_videoram_private_w)
{
	//AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(raiden2_background_w) AM_SHARE("back_data")
	//AM_RANGE(0x0d800, 0x0dfff) AM_RAM_WRITE(raiden2_foreground_w) AM_SHARE("fore_data")
	//AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(raiden2_midground_w)  AM_SHARE("mid_data")
	//AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_WRITE(raiden2_text_w) AM_SHARE("text_data")

	if (offset < 0x800 / 2)
	{
		raiden2_background_w(space, offset, data, 0xffff);
	}
	else if (offset < 0x1000 /2)
	{
		offset -= 0x800 / 2;
		raiden2_foreground_w(space, offset, data, 0xffff);
	}
	else if (offset < 0x1800/2)
	{
		offset -= 0x1000 / 2;
		raiden2_midground_w(space, offset, data, 0xffff);
	}
	else if (offset < 0x2800/2)
	{
		offset -= 0x1800 / 2;
		raiden2_text_w(space, offset, data, 0xffff);
	}
}



void raiden2_state::combine32(UINT32 *val, int offset, UINT16 data, UINT16 mem_mask)
{
	UINT16 *dest = (UINT16 *)val + BYTE_XOR_LE(offset);
	COMBINE_DATA(dest);
}



/* SPRITE DRAWING (move to video file) */

void raiden2_state::draw_sprites(const rectangle &cliprect)
{
	UINT16 *source = sprites + (0x1000/2)-4;
	sprite_buffer.fill(0xf, cliprect);

	gfx_element *gfx = m_gfxdecode->gfx(2);

	/*
	  00 fhhh Fwww ppcc cccc   h = height f=flipy w = width F = flipx p = priority c = color
	  02 nnnn nnnn nnnn nnnn   n = tileno
	  04 xxxx xxxx xxxx xxxx   x = xpos
	  06 yyyy yyyy yyyy yyyy   y = ypos
	 */

	while( source >= sprites ){
		int tile_number = source[1];
		int sx = source[2];
		int sy = source[3];
		int colr;
		int xtiles, ytiles;
		int ytlim, xtlim;
		int xflip, yflip;
		int xstep, ystep;
		int pri;

		ytlim = (source[0] >> 12) & 0x7;
		xtlim = (source[0] >> 8 ) & 0x7;

		xflip = (source[0] >> 15) & 0x1;
		yflip = (source[0] >> 11) & 0x1;

		colr = source[0] & 0x3f;

		pri = (source[0] >> 6) & 3;

		colr |= pri << (14-4);

		ytlim += 1;
		xtlim += 1;

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
				/* note this wraparound handling could be wrong if some of the COP maths is wrong */

#define ZEROTEAM_MASK_X (0x1ff) // causes a blank square in the corner of zero team, but otherwise the thrusters of the ship in the r2 intro are clipped, using 0x8000 as a sign bit instead of this logic works for r2, but not zero team
#define ZEROTEAM_MASK_Y (0x1ff)



						gfx->transpen(
						sprite_buffer,
						cliprect,
						tile_number,
						colr,
						yflip,xflip,
						(sx+xstep*xtiles)&ZEROTEAM_MASK_X,(sy+ystep*ytiles)&ZEROTEAM_MASK_Y, 15);


						gfx->transpen(
						sprite_buffer,
						cliprect,
						tile_number,
						colr,
						yflip,xflip,
						((sx+xstep*xtiles)&ZEROTEAM_MASK_X)-0x200,(sy+ystep*ytiles)&ZEROTEAM_MASK_Y, 15);


						gfx->transpen(
						sprite_buffer,
						cliprect,
						tile_number,
						colr,
						yflip,xflip,
						(sx+xstep*xtiles)&ZEROTEAM_MASK_X,((sy+ystep*ytiles)&ZEROTEAM_MASK_Y)-0x200, 15);


						gfx->transpen(
						sprite_buffer,
						cliprect,
						tile_number,
						colr,
						yflip,xflip,
						((sx+xstep*xtiles)&ZEROTEAM_MASK_X)-0x200,((sy+ystep*ytiles)&ZEROTEAM_MASK_Y)-0x200, 15);


				tile_number++;
			}
		}

		source -= 4;
	}

}

/* VIDEO RELATED WRITE HANDLERS (move to video file) */

WRITE16_MEMBER(raiden2_state::raiden2_background_w)
{
	COMBINE_DATA(&back_data[offset]);
	background_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(raiden2_state::raiden2_midground_w)
{
	COMBINE_DATA(&mid_data[offset]);
	midground_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(raiden2_state::raiden2_foreground_w)
{
	COMBINE_DATA(&fore_data[offset]);
	foreground_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(raiden2_state::raiden2_text_w)
{
	COMBINE_DATA(&text_data[offset]);
	text_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(raiden2_state::tilemap_enable_w)
{
	COMBINE_DATA(&raiden2_tilemap_enable);
}

WRITE16_MEMBER(raiden2_state::tile_scroll_w)
{
	COMBINE_DATA(scrollvals + offset);
	data = scrollvals[offset];

	tilemap_t *tm = nullptr;
	switch(offset/2) {
	case 0: tm = background_layer; break;
	case 1: tm = midground_layer; break;
	case 2: tm = foreground_layer; break;
	}
	if(offset & 1)
		tm->set_scrolly(0, data);
	else
		tm->set_scrollx(0, data);
}

WRITE16_MEMBER(raiden2_state::tile_bank_01_w)
{
	if(ACCESSING_BITS_0_7) {
		int new_bank;
		new_bank = 0 | ((data & 1)<<1);
		if(new_bank != bg_bank) {
			bg_bank = new_bank;
			background_layer->mark_all_dirty();
		}

		new_bank = 1 | (data & 2);
		if(new_bank != mid_bank) {
			mid_bank = new_bank;
			midground_layer->mark_all_dirty();
		}
	}
}

READ16_MEMBER(raiden2_state::cop_tile_bank_2_r)
{
	return cop_bank;
}

WRITE16_MEMBER(raiden2_state::cop_tile_bank_2_w)
{
	COMBINE_DATA(&cop_bank);

	if(ACCESSING_BITS_8_15) {
		int new_bank = 4 | (data >> 14);
		if(new_bank != fg_bank) {
			fg_bank = new_bank;
			foreground_layer->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(raiden2_state::raidendx_cop_bank_2_w)
{
	COMBINE_DATA(&cop_bank);

	int new_bank = 4 | ((cop_bank >> 4) & 3);
	if(new_bank != fg_bank) {
		fg_bank = new_bank;
		foreground_layer->mark_all_dirty();
	}

	/* mainbank2 coming from 6c9 ? */
	int bb = cop_bank >> 12;
	membank("mainbank1")->set_entry(bb + 16);
	membank("mainbank2")->set_entry(3);
}



/* TILEMAP RELATED (move to video file) */

TILE_GET_INFO_MEMBER(raiden2_state::get_back_tile_info)
{
	int tile = back_data[tile_index];
	int color = (tile >> 12) | (0 << 4);

	tile = (tile & 0xfff) | (bg_bank << 12);

	SET_TILE_INFO_MEMBER(1,tile+0x0000,color,0);
}

TILE_GET_INFO_MEMBER(raiden2_state::get_mid_tile_info)
{
	int tile = mid_data[tile_index];
	int color = (tile >> 12) | (2 << 4);

	tile = (tile & 0xfff) | (mid_bank << 12);

	SET_TILE_INFO_MEMBER(1,tile,color,0);
}

TILE_GET_INFO_MEMBER(raiden2_state::get_fore_tile_info)
{
	int tile = fore_data[tile_index];
	int color = (tile >> 12) | (1 << 4);

	tile = (tile & 0xfff) | (fg_bank << 12);

	SET_TILE_INFO_MEMBER(1,tile,color,0);
}

TILE_GET_INFO_MEMBER(raiden2_state::get_text_tile_info)
{
	int tile = text_data[tile_index];
	int color = (tile>>12)&0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(0,tile + tx_bank * 0x1000,color,0);
}

/* VIDEO START (move to video file) */


VIDEO_START_MEMBER(raiden2_state,raiden2)
{
	back_data = make_unique_clear<UINT16[]>(0x800/2);
	fore_data =  make_unique_clear<UINT16[]>(0x800/2);
	mid_data =  make_unique_clear<UINT16[]>(0x800/2);
	text_data =  make_unique_clear<UINT16[]>(0x1000/2);

	save_pointer(NAME(back_data.get()), 0x800/2);
	save_pointer(NAME(fore_data.get()), 0x800/2);
	save_pointer(NAME(mid_data.get()), 0x800/2);
	save_pointer(NAME(text_data.get()), 0x1000/2);

	text_layer       = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden2_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64,32 );
	background_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden2_state::get_back_tile_info),this), TILEMAP_SCAN_ROWS, 16,16, 32,32 );
	midground_layer  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden2_state::get_mid_tile_info),this),  TILEMAP_SCAN_ROWS, 16,16, 32,32 );
	foreground_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(raiden2_state::get_fore_tile_info),this), TILEMAP_SCAN_ROWS, 16,16, 32,32 );
}

/* screen_update_raiden2 (move to video file) */

void raiden2_state::blend_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind16 &source, int layer)
{
	if(layer == -1)
		return;

	const pen_t *pens = &m_palette->pen(0);
	layer <<= 14;
	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		const UINT16 *src = &source.pix16(y, cliprect.min_x);
		UINT32 *dst = &bitmap.pix32(y, cliprect.min_x);
		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			UINT16 val = *src++;
			if((val & 0xc000) == layer && (val & 0x000f) != 0x000f) {
				val &= 0x07ff;

				if(blend_active[val])
					*dst = alpha_blend_r32(*dst, pens[val], 0x7f);
				else
					*dst = pens[val];
			}
			dst++;
		}
	}
}

void raiden2_state::tilemap_draw_and_blend(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tilemap)
{
	tilemap->draw(screen, tile_buffer, cliprect, 0, 0);
	blend_layer(bitmap, cliprect, tile_buffer, 0);
}

UINT32 raiden2_state::screen_update_raiden2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	if (!(raiden2_tilemap_enable & 16)) {
		draw_sprites(cliprect);

		blend_layer(bitmap, cliprect, sprite_buffer, cur_spri[0]);
	}

	if (!(raiden2_tilemap_enable & 1))
		tilemap_draw_and_blend(screen, bitmap, cliprect, background_layer);

	if (!(raiden2_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, sprite_buffer, cur_spri[1]);

	if (!(raiden2_tilemap_enable & 2))
		tilemap_draw_and_blend(screen, bitmap, cliprect, midground_layer);

	if (!(raiden2_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, sprite_buffer, cur_spri[2]);

	if (!(raiden2_tilemap_enable & 4))
		tilemap_draw_and_blend(screen, bitmap, cliprect, foreground_layer);

	if (!(raiden2_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, sprite_buffer, cur_spri[3]);

	if (!(raiden2_tilemap_enable & 8))
		tilemap_draw_and_blend(screen, bitmap, cliprect, text_layer);

	if (!(raiden2_tilemap_enable & 16))
		blend_layer(bitmap, cliprect, sprite_buffer, cur_spri[4]);

	if (machine().input().code_pressed_once(KEYCODE_Z))
		if (m_raiden2cop) m_raiden2cop->dump_table();

	return 0;
}




/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(raiden2_state::raiden2_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xc0/4);   /* VBL */
}




// Sprite encryption key upload

static UINT32 sprcpt_adr, sprcpt_idx;

static UINT16 sprcpt_flags2;
static UINT32 sprcpt_val[2], sprcpt_flags1;
static UINT32 sprcpt_data_1[0x100], sprcpt_data_2[0x40], sprcpt_data_3[6], sprcpt_data_4[4];

void raiden2_state::sprcpt_init(void)
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
	tx_bank = 0;
}

MACHINE_RESET_MEMBER(raiden2_state,raiden2)
{
	common_reset();
	sprcpt_init();

	membank("mainbank1")->set_entry(2);
	membank("mainbank2")->set_entry(3);

	prg_bank = 0;
	//cop_init();
}

MACHINE_RESET_MEMBER(raiden2_state,raidendx)
{
	common_reset();
	sprcpt_init();

	membank("mainbank1")->set_entry(16);
	membank("mainbank2")->set_entry(3);

	prg_bank = 0x08;

	//cop_init();
}

MACHINE_RESET_MEMBER(raiden2_state,zeroteam)
{
	bg_bank = 0;
	fg_bank = 2;
	mid_bank = 1;
	tx_bank = 0;
	sprcpt_init();

	membank("mainbank1")->set_entry(2);
	membank("mainbank2")->set_entry(3);

	prg_bank = 0;
	//cop_init();
}

MACHINE_RESET_MEMBER(raiden2_state,xsedae)
{
	bg_bank = 0;
	fg_bank = 2;
	mid_bank = 1;
	tx_bank = 0;
	sprcpt_init();
}

READ16_MEMBER(raiden2_state::raiden2_sound_comms_r)
{
	return m_seibu_sound->main_word_r(space,(offset >> 1) & 7,0xffff);
}

WRITE16_MEMBER(raiden2_state::raiden2_sound_comms_w)
{
	m_seibu_sound->main_word_w(space,(offset >> 1) & 7,data,0x00ff);
}

WRITE16_MEMBER(raiden2_state::raiden2_bank_w)
{
	if(ACCESSING_BITS_8_15) {
		int bb = (~data >> 15) & 1;
		logerror("select bank %d %04x\n", (data >> 15) & 1, data);
		membank("mainbank1")->set_entry(bb*2);
		membank("mainbank2")->set_entry(bb*2+1);
		prg_bank = ((data >> 15) & 1);
	}
}


WRITE16_MEMBER(raiden2_state::sprite_prot_x_w)
{
	sprite_prot_x = data;
	//popmessage("%04x %04x",sprite_prot_x,sprite_prot_y);
}

WRITE16_MEMBER(raiden2_state::sprite_prot_y_w)
{
	sprite_prot_y = data;
	//popmessage("%04x %04x",sprite_prot_x,sprite_prot_y);
}

WRITE16_MEMBER(raiden2_state::sprite_prot_src_seg_w)
{
	sprite_prot_src_addr[0] = data;
}

READ16_MEMBER(raiden2_state::sprite_prot_src_seg_r)
{
	return sprite_prot_src_addr[0];
}

WRITE16_MEMBER(raiden2_state::sprite_prot_src_w)
{
	sprite_prot_src_addr[1] = data;
	UINT32 src = (sprite_prot_src_addr[0]<<4)+sprite_prot_src_addr[1];

	int x = INT16((space.read_dword(src+0x08) >> 16) - (sprite_prot_x));
	int y = INT16((space.read_dword(src+0x04) >> 16) - (sprite_prot_y));

	UINT16 head1 = space.read_word(src+cop_spr_off);
	UINT16 head2 = space.read_word(src+cop_spr_off+2);

	int w = (((head1 >> 8 ) & 7) + 1) << 4;
	int h = (((head1 >> 12) & 7) + 1) << 4;

	UINT16 flag = x-w/2 > -w && x-w/2 < cop_spr_maxx+w && y-h/2 > -h && y-h/2 < 256+h ? 1 : 0;

	flag = (space.read_word(src) & 0xfffe) | flag;
	space.write_word(src, flag);

	if(flag & 1)
	{
		space.write_word(dst1,   head1);
		space.write_word(dst1+2, head2);
		space.write_word(dst1+4, x-w/2);
		space.write_word(dst1+6, y-h/2);

		dst1 += 8;
	}
	//printf("[%08x] %08x %08x %04x %04x\n",src,dx,dy,dst1,dst2);
	//  debugger_break(machine());
}

READ16_MEMBER(raiden2_state::sprite_prot_dst1_r)
{
	return dst1;
}

READ16_MEMBER(raiden2_state::sprite_prot_maxx_r)
{
	return cop_spr_maxx;
}

READ16_MEMBER(raiden2_state::sprite_prot_off_r)
{
	return cop_spr_off;
}

WRITE16_MEMBER(raiden2_state::sprite_prot_dst1_w)
{
	dst1 = data;
}

WRITE16_MEMBER(raiden2_state::sprite_prot_maxx_w)
{
	cop_spr_maxx = data;
}

WRITE16_MEMBER(raiden2_state::sprite_prot_off_w)
{
	cop_spr_off = data;
}

/* MEMORY MAPS */
static ADDRESS_MAP_START( raiden2_cop_mem, AS_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x0041c, 0x0041d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_angle_target_w) // angle target (for 0x6200 COP macro)
	AM_RANGE(0x0041e, 0x0041f) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_angle_step_w)   // angle step   (for 0x6200 COP macro)
	AM_RANGE(0x00420, 0x00421) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_itoa_low_w)
	AM_RANGE(0x00422, 0x00423) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_itoa_high_w)
	AM_RANGE(0x00424, 0x00425) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_itoa_digit_count_w)
	AM_RANGE(0x00428, 0x00429) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_dma_v1_w)
	AM_RANGE(0x0042a, 0x0042b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_dma_v2_w)
	AM_RANGE(0x00432, 0x00433) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_data_w)
	AM_RANGE(0x00434, 0x00435) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_addr_w)
	AM_RANGE(0x00436, 0x00437) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_hitbox_baseadr_w)
	AM_RANGE(0x00438, 0x00439) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_value_w)
	AM_RANGE(0x0043a, 0x0043b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_mask_w)
	AM_RANGE(0x0043c, 0x0043d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pgm_trigger_w)
	AM_RANGE(0x00444, 0x00445) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_scale_w)
	AM_RANGE(0x00450, 0x00451) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_ram_addr_hi_w)
	AM_RANGE(0x00452, 0x00453) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_ram_addr_lo_w)
	AM_RANGE(0x00454, 0x00455) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_lookup_hi_w)
	AM_RANGE(0x00456, 0x00457) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_lookup_lo_w)
	AM_RANGE(0x00458, 0x00459) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_param_w)
	AM_RANGE(0x0045a, 0x0045b) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pal_brightness_val_w) //palette DMA brightness val, used by X Se Dae / Zero Team
	AM_RANGE(0x0045c, 0x0045d) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_pal_brightness_mode_w)  //palette DMA brightness mode, used by X Se Dae / Zero Team (sets to 5)
	AM_RANGE(0x00470, 0x00471) AM_READWRITE(cop_tile_bank_2_r,cop_tile_bank_2_w) // implementaton of this varies between games, external hookup?

	AM_RANGE(0x00476, 0x00477) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_dma_adr_rel_w)
	AM_RANGE(0x00478, 0x00479) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_src_w)
	AM_RANGE(0x0047a, 0x0047b) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_size_w)
	AM_RANGE(0x0047c, 0x0047d) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_dst_w)
	AM_RANGE(0x0047e, 0x0047f) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_dma_mode_r, cop_dma_mode_w)
	AM_RANGE(0x004a0, 0x004ad) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_reg_high_r, cop_reg_high_w)
	AM_RANGE(0x004c0, 0x004cd) AM_DEVREADWRITE("raiden2cop", raiden2cop_device, cop_reg_low_r, cop_reg_low_w)
	AM_RANGE(0x00500, 0x00505) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_cmd_w)
	AM_RANGE(0x00580, 0x00581) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_collision_status_r)
	AM_RANGE(0x00582, 0x00587) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_collision_status_val_r)
	AM_RANGE(0x00588, 0x00589) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_collision_status_stat_r)
	AM_RANGE(0x00590, 0x00599) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_itoa_digits_r)
	AM_RANGE(0x005b0, 0x005b1) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_status_r)
	AM_RANGE(0x005b2, 0x005b3) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_dist_r)
	AM_RANGE(0x005b4, 0x005b5) AM_DEVREAD("raiden2cop", raiden2cop_device, cop_angle_r)

	/* I think all this block is part of the video chip */
	AM_RANGE(0x00600, 0x0064f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
//  AM_RANGE(0x0061c, 0x0061d) AM_WRITE(tilemap_enable_w)
//  AM_RANGE(0x00620, 0x0062b) AM_WRITE(tile_scroll_w)
	AM_RANGE(0x006a0, 0x006a3) AM_WRITE(sprcpt_val_1_w)
	AM_RANGE(0x006a4, 0x006a7) AM_WRITE(sprcpt_data_3_w)
	AM_RANGE(0x006a8, 0x006ab) AM_WRITE(sprcpt_data_4_w)
	AM_RANGE(0x006ac, 0x006af) AM_WRITE(sprcpt_flags_1_w)
	AM_RANGE(0x006b0, 0x006b3) AM_WRITE(sprcpt_data_1_w)
	AM_RANGE(0x006b4, 0x006b7) AM_WRITE(sprcpt_data_2_w)
	AM_RANGE(0x006b8, 0x006bb) AM_WRITE(sprcpt_val_2_w)
	AM_RANGE(0x006bc, 0x006bf) AM_WRITE(sprcpt_adr_w)
	AM_RANGE(0x006c0, 0x006c1) AM_READWRITE(sprite_prot_off_r, sprite_prot_off_w)
	AM_RANGE(0x006c2, 0x006c3) AM_READWRITE(sprite_prot_src_seg_r, sprite_prot_src_seg_w)
	AM_RANGE(0x006c6, 0x006c7) AM_WRITE(sprite_prot_dst1_w)
	AM_RANGE(0x006ca, 0x006cb) AM_WRITE(raiden2_bank_w)
	AM_RANGE(0x006cc, 0x006cd) AM_WRITE(tile_bank_01_w)
	AM_RANGE(0x006ce, 0x006cf) AM_WRITE(sprcpt_flags_2_w)
	AM_RANGE(0x006d8, 0x006d9) AM_WRITE(sprite_prot_x_w)
	AM_RANGE(0x006da, 0x006db) AM_WRITE(sprite_prot_y_w)
	AM_RANGE(0x006dc, 0x006dd) AM_READWRITE(sprite_prot_maxx_r, sprite_prot_maxx_w)
	AM_RANGE(0x006de, 0x006df) AM_WRITE(sprite_prot_src_w)
	/* end video block */

	AM_RANGE(0x006fc, 0x006fd) AM_DEVWRITE("raiden2cop", raiden2cop_device,cop_dma_trigger_w)
	AM_RANGE(0x006fe, 0x006ff) AM_DEVWRITE("raiden2cop", raiden2cop_device, cop_sort_dma_trig_w) // sort-DMA trigger

	AM_RANGE(0x00762, 0x00763) AM_READ(sprite_prot_dst1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( raiden2_mem, AS_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP //irq ack / sprite buffering?

	AM_IMPORT_FROM( raiden2_cop_mem )

	AM_RANGE(0x00700, 0x0071f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("P1_P2")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x00800, 0x0bfff) AM_RAM

	AM_RANGE(0x0c000, 0x0cfff) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM // _WRITE(raiden2_background_w) AM_SHARE("back_data")
	AM_RANGE(0x0d800, 0x0dfff) AM_RAM // _WRITE(raiden2_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM // _WRITE(raiden2_midground_w)  AM_SHARE("mid_data")
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM // _WRITE(raiden2_text_w) AM_SHARE("text_data")
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM /* Stack area */

	AM_RANGE(0x10000, 0x1efff) AM_RAM
	AM_RANGE(0x1f000, 0x1ffff) AM_RAM //_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x20000, 0x2ffff) AM_ROMBANK("mainbank1")
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("mainbank2")
	AM_RANGE(0x40000, 0xfffff) AM_ROM AM_REGION("maincpu", 0x40000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( raidendx_mem, AS_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00470, 0x00471) AM_READWRITE(cop_tile_bank_2_r,raidendx_cop_bank_2_w)
	AM_RANGE(0x004d0, 0x004d7) AM_RAM //???
	AM_RANGE(0x00600, 0x0064f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read_alt, write_alt)
//  AM_RANGE(0x006ca, 0x006cb) AM_WRITENOP
	AM_IMPORT_FROM( raiden2_mem )
ADDRESS_MAP_END

static ADDRESS_MAP_START( zeroteam_mem, AS_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x00470, 0x00471) AM_WRITENOP
	AM_RANGE(0x006cc, 0x006cd) AM_WRITENOP

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP // irq ack / sprite buffering?

	AM_IMPORT_FROM( raiden2_cop_mem )

	AM_RANGE(0x00700, 0x0071f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("P1_P2")
	AM_RANGE(0x00748, 0x00749) AM_READ_PORT("P3_P4")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x00800, 0x0b7ff) AM_RAM
	AM_RANGE(0x0b800, 0x0bfff) AM_RAM // _WRITE(raiden2_background_w) AM_SHARE("back_data")
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM // _WRITE(raiden2_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM // _WRITE(raiden2_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x0d000, 0x0dfff) AM_RAM // _WRITE(raiden2_text_w) AM_SHARE("text_data")
	AM_RANGE(0x0e000, 0x0efff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0f000, 0x0ffff) AM_RAM AM_SHARE("sprites")
	AM_RANGE(0x10000, 0x1ffff) AM_RAM

	AM_RANGE(0x20000, 0x2ffff) AM_ROMBANK("mainbank1")
	AM_RANGE(0x30000, 0x3ffff) AM_ROMBANK("mainbank2")
	AM_RANGE(0x40000, 0xfffff) AM_ROM AM_REGION("maincpu", 0x40000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xsedae_mem, AS_PROGRAM, 16, raiden2_state )
	AM_RANGE(0x00000, 0x003ff) AM_RAM

	AM_RANGE(0x00470, 0x00471) AM_WRITENOP
	AM_RANGE(0x006cc, 0x006cd) AM_WRITENOP

	AM_RANGE(0x0068e, 0x0068f) AM_WRITENOP //irq ack / sprite buffering?

	AM_IMPORT_FROM( raiden2_cop_mem )

	AM_RANGE(0x00700, 0x0071f) AM_READWRITE(raiden2_sound_comms_r,raiden2_sound_comms_w)

	AM_RANGE(0x00740, 0x00741) AM_READ_PORT("DSW")
	AM_RANGE(0x00744, 0x00745) AM_READ_PORT("P1_P2")
	AM_RANGE(0x00748, 0x00749) AM_READ_PORT("P3_P4")
	AM_RANGE(0x0074c, 0x0074d) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x00800, 0x0b7ff) AM_RAM
	AM_RANGE(0x0b800, 0x0bfff) AM_RAM // _WRITE(raiden2_background_w) AM_SHARE("back_data")
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM // _WRITE(raiden2_foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM // _WRITE(raiden2_midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x0d000, 0x0dfff) AM_RAM // _WRITE(raiden2_text_w) AM_SHARE("text_data")
	AM_RANGE(0x0e000, 0x0efff) AM_RAM // _DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0f000, 0x0ffff) AM_RAM AM_SHARE("sprites")

	AM_RANGE(0x10000, 0x1ffff) AM_RAM

	AM_RANGE(0x20000, 0xfffff) AM_ROM AM_REGION("maincpu", 0x20000)
ADDRESS_MAP_END


/* INPUT PORTS */

static INPUT_PORTS_START( raiden2 )
	SEIBU_COIN_INPUTS_INVERT    /* coin inputs read through sound cpu */

	PORT_START("P1_P2") /* IN0/1 */
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

	PORT_START("DSW")   /* Dip switches  */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) ) // dipsw sheets say this is hard but service mode says easy
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) ) // vice versa of above
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x3000, "200000 500000" )
	PORT_DIPSETTING(      0x2000, "400000 1000000" )
	PORT_DIPSETTING(      0x1000, "1000000 3000000" )
	PORT_DIPSETTING(      0x0000, "No Extend" )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:!8") /* Test Mode */

	PORT_START("SYSTEM")    /* START BUTTONS */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( raidendx )
	PORT_INCLUDE( raiden2 )

	PORT_MODIFY("DSW")  /* Dip switches  */
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!5") /* Manual shows "Not Used" */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!6") /* Manual shows "Not Used" */
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( zeroteam )
	PORT_INCLUDE( raiden2 )

	PORT_MODIFY("COIN")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(4)

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P3_P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW") // not the same as raiden2/dx: coins, difficulty, lives and bonus lives all differ!
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!4,!5,!6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Starting Coin" ) PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "X 2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0800, "4" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x3000, "1000000" )
	PORT_DIPSETTING(      0x2000, "2000000" )
	PORT_DIPSETTING(      0x1000, "Every 1000000" )
	PORT_DIPSETTING(      0x0000, "No Extend" )
	PORT_DIPNAME( 0x4000, 0x4000, "Demo Sound" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:!8") // marked as unused
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0700, 0x0700, "Cabinet Setting" ) PORT_DIPLOCATION("SW3:!1,!2,!3")
	PORT_DIPSETTING(    0x0700, "2P" )
	PORT_DIPSETTING(    0x0600, "3P 3Slot" )
	PORT_DIPSETTING(    0x0500, "4P 4Slot" )
	PORT_DIPSETTING(    0x0400, "3P 2Slot" )
	PORT_DIPSETTING(    0x0300, "2P x2" )
	PORT_DIPSETTING(    0x0200, "4P 2Slot" )
	PORT_DIPSETTING(    0x0100, "2P Freeplay" )
	PORT_DIPSETTING(    0x0000, "4P Freeplay" )
	PORT_SERVICE( 0x0800, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW3:!4") // marked as test mode
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!5") // marked as unused
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!6") // marked as unused
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!7") // marked as unused
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:!8") // marked as unused
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xsedae )
	PORT_INCLUDE( raiden2 )

	PORT_START("P3_P4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

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
	GFXDECODE_ENTRY( "gfx3", 0x00000, raiden2_spritelayout, 0x000, 4096 ) // really 128, but using the top bits for priority
GFXDECODE_END


/* MACHINE DRIVERS */

static MACHINE_CONFIG_START( raiden2, raiden2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,XTAL_32MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(raiden2_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", raiden2_state,  raiden2_interrupt)

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,raiden2)

	SEIBU2_RAIDEN2_SOUND_SYSTEM_CPU(XTAL_28_63636MHz/8)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(XTAL_32MHz/4,512,0,40*8,282,0,30*8) /* hand-tuned to match ~55.47 */
	MCFG_SCREEN_UPDATE_DRIVER(raiden2_state, screen_update_raiden2)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", raiden2)
	MCFG_PALETTE_ADD("palette", 2048)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(raiden2_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(raiden2_state, tile_scroll_w))

	MCFG_RAIDEN2COP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(raiden2_state, m_videoram_private_w))
	MCFG_ITOA_UNUSED_DIGIT_VALUE(0x20)

	MCFG_VIDEO_START_OVERRIDE(raiden2_state,raiden2)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_RAIDEN2_INTERFACE(XTAL_28_63636MHz/8,XTAL_28_63636MHz/28,1,2)
	// the sound z80 has /NMI, /BUSREQ and /WAIT tied high/unused


/* Sound hardware infos: Z80 and YM2151 are clocked at XTAL_28_63636MHz/8 */
/* The 2 Oki M6295 are clocked at XTAL_28_63636MHz/28 and pin 7 is high for both */

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xsedae, raiden2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(xsedae_mem)

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,xsedae)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0, 32*8-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( raidendx, raiden2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(raidendx_mem)

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,raidendx)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( zeroteam, raiden2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30,XTAL_32MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(zeroteam_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", raiden2_state,  raiden2_interrupt)

	MCFG_MACHINE_RESET_OVERRIDE(raiden2_state,zeroteam)

	SEIBU_NEWZEROTEAM_SOUND_SYSTEM_CPU(XTAL_28_63636MHz/8)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
//  MCFG_SCREEN_REFRESH_RATE(55.47)    /* verified on pcb */
	MCFG_SCREEN_RAW_PARAMS(XTAL_32MHz/4,512,0,40*8,282,0,32*8) /* hand-tuned to match ~55.47 */
	MCFG_SCREEN_UPDATE_DRIVER(raiden2_state, screen_update_raiden2)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", raiden2)
	MCFG_PALETTE_ADD("palette", 2048)
	//MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(raiden2_state, tilemap_enable_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(raiden2_state, tile_scroll_w))

	MCFG_RAIDEN2COP_ADD("raiden2cop")
	MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(WRITE16(raiden2_state, m_videoram_private_w))
	MCFG_ITOA_UNUSED_DIGIT_VALUE(0x20)

	MCFG_VIDEO_START_OVERRIDE(raiden2_state,raiden2)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(XTAL_28_63636MHz/8, 1320000/* ? */)
MACHINE_CONFIG_END

/* ROM LOADING */
/*
Raiden II

(C) 1993 RAIDEN II SEIBU KAIHATSU INC.,o
|----------------------------------------------------------|
|      1    2   3   4   5   6    7      8      9     10    |
|LA4460    M6295  PCM  Z8400A  6116    BATTERY3.6v        A|
|   YM2151 M6295   6    5      6116    28.6360 MHz        B|
|     VOL   YM3014                                         |
|HB-45A     YM3012 |------|                               C|
|HB-2      NJM4560 |SIE150| LH5116    |---------|          |
|RC220             |      | LH5116    | SEI252  |         D|
|RC220             |------| LH5116    |SB05-106 |          |
|RC220                      LH5116    |(QFP208) |         E|
|J                                    |         |         F|
|A                                    |---------|          |
|M    DSW2(8)                                             G|
|M    DSW1(8)                                   LH522258   |
|A          |---------|OBJ-1    OBJ-2           LH522258  H|
|           | SEI360  |                         LH522258  J|
|           |SB06-1937|OBJ-3    OBJ-4           LH522258  K|
|           |(QFP160) |                     |---------|   L|
|           |         |      1              |SEI1000  |   M|
| |------|  |---------|  1x      3x         |SB01-001 |   N|
| |SEI200|         32MHz     2              |(QFP184) |    |
| |      |CXK5863        2x      4x         |         |   P|
| |------|CXK5863                           |---------|    |
|                                                         Q|
|                        PAL2 PAL1             |----|     R|
|                                              |V30 |      |
|      BG-1       BG-2   7   COPX-D2           |----|     S|
|----------------------------------------------------------|
Notes:
      V30 clock    - 16.000MHz [32/2]. Chip is stamped "NEC D70116HG-16 V30 NEC '84" (QFP52)
      Z80 clock    - 3.579545MHz [28.63636/8]
      YM2151 clock - 3.579545MHz [28.63636/8]
      Yamaha DAC   -
        early boards: ym3014 mono dac, no NJM4560
        later boards: ym3012 stereo dac plus NJM4560, each with a capacitor on top
      M6295 clocks - 1.022MHz [28.63636/28] and pin 7 HIGH (both)
      LH52258      - Sharp LH52258 32k x8 SRAM (= 62256)
      CXK5863      - Sony CXK5863 8k x8 SRAM (= 6264)
      6116         - 2k x8 SRAM
      LH5116       - 2k x8 SRAM
      HB-45A       - Seibu custom ceramic module sound DAC (SIP20)
      HB-2         - Seibu custom ceramic module connected to coin counters (SIP10)
      RC220        - Custom resistor network module used for inputs (SIP14)
      VSync        - 55.4859Hz  \
      HSync        - 15.5586kHz / not measured but assumed same as R2DX
      PAL1         - MMIPAL16L8B stamped 'JJ4B01' (DIP20)
      PAL2         - AMI 18CV8 stamped 'JJ4B02' (DIP20)
      ROMs         - *PCM      - 2M MaskROM stamped 'RAIDEN 2 PCM' at location U1018 (DIP32), pcb labeled VOI2
                     6         - 23C020 MASK ROM labelled 'SEIBU 6' at location U1017 (DIP32), pcb labeled VOI1
                     5         - 27C512 EPROM labelled 'SEIBU 5' at location U1110 (DIP28)
                     *OBJ-1     - 16M MaskROM stamped 'RAIDEN 2 OBJ-1' at location U0811 (DIP42)
                     *OBJ-2     - 16M MaskROM stamped 'RAIDEN 2 OBJ-2' at location U082 (DIP42)
                     *OBJ-3     - 16M MaskROM stamped 'RAIDEN 2 OBJ-3' at location U0837 (DIP42)
                     *OBJ-4     - 16M MaskROM stamped 'RAIDEN 2 OBJ-4' at location U0836 (DIP42)
                 /   1x        - 27C2001 EPROM labelled 'SEIBU 1' at location U1210 (DIP32)
     Early boards|   2x        - 27C2001 EPROM labelled 'SEIBU 2' at location U1211 (DIP32)
                 |   3x        - 27C2001 EPROM labelled 'SEIBU 3' at location U129 (DIP32)
                 \   4x        - 27C2001 EPROM labelled 'SEIBU 4' at location U1212 (DIP32)
     Later boards/   1         - 27C402 or 27C4096 EPROM labelled 'SEIBU 1' at location U0211 (DIP40)
                 \   2         - 27C402 or 27C4096 EPROM labelled 'SEIBU 2' at location U0212 (DIP40)
                     *BG-1      - 16M MaskROM stamped 'RAIDEN 2 BG-1' at location U075 (DIP42)
                     *BG-2      - 16M MaskROM stamped 'RAIDEN 2 BG-2' at location U0714 (DIP42)
                     7         - 27C210 EPROM labelled 'SEIBU 7' at location U0724 (DIP40)
                     *COPX-D2  - 2M MaskROM stamped 'COPX-D2' at location U0313 (DIP40)

                     * = these ROMs are soldered-in

      SEIBU Custom ICs -
                        SIE150 (QFP100) - z80 interface
                        SEI252 SB05-106 (QFP208) - fg/sprite gfx and its decryption
                        SEI0200 TC110G21AF 0076 (QFP100) - bg gfx
                        SEI360 SB06-1937 (QFP160) - logic and i/o array
                        SEI1000 SB01-001 (QFP184) - main protection

*/

/* Note: some raiden 2 fabtek usa boards (the one Hammad sent to LN and Balrog, at least) have the
    ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
    z80 sound rom as used in raiden2hk instead of the
    ROM_LOAD( "snd.u1110",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
    rom from raiden2. Slight version difference, and I don't know which is older/newer. - LN

ROMSET organization:
Note: type numbers are NOT NECESSARILY in chronological version order YET.
SETNAME   LONGNAME       PRG TYPES   SND(u1110) TYPE   VOICE(u1017) TYPE  FX0(u0724) TYPE  Notes
raiden2   (set 1 fabtek) 1 1'        1(f51a28f9)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
raiden2hk (set 2 metro)  1 2'        2(8f130589)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
raiden2j  (set 3 japan)  1 3'        2(8f130589)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
raiden2i  (set 4 italy)  2 4'        3(5db9f922)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore
(trap15: one of these four above has aama serial 0587600)
raiden2e  (set 5 easy)   3 5'        4(6bad0a3e)       2(488d050f)        2(c709bdf6)      red fighter on hiscore
raiden2ea (set 6 easy)   4 6'        5(f5f835af)       3(fab9f8e4)        3(c7aa4d00)      red fighter on hiscore
raiden2eu (set 7 easy fabtek) 4 7'   5(f5f835af)       3(fab9f8e4)        3(c7aa4d00)      red fighter on hiscore
raiden2eua (set 8 easy fabtek) 3 8'   6(6d362472)       3(fab9f8e4)        3(c7aa4d00)      red fighter on hiscore, sn 0003068, aama 0557135
^ this set has 4 prg roms: 1 and 3 correspond to seibu1/prg0 and 2 and 4 correspond to seibu2/prg1
balrog+ln (set x fabtek) 1 1'        2(8f130589)       1(fb0fca23)        1(c9ec9469)      sepia fighter on hiscore, sn 0012739, aama 0600565, not in mame yet due to roms matching mix of sets 1 and 2

differences amongst SND/u1110 roms:
   First half end, last byte before ff fill ending at 7fff
   |     Last byte before ff fill ending at 8fff
   |     |     Last byte before ff fill ending at ffff
   |     |     |
1: 62e8  8faf  f56b
2: 62b8  8faf  f56b
3: 62a9  8faf  f56b
4: 623e  8ee7  f4dd
5: 620a  8ee7  f4d7
6: 64b8  8e1f  f4db
<LordNlptp> btw my guess is the code versions go from newest to oldest, 1 to 6, though I need more serial numbers to be sure
<LordNlptp> 6 has a larger main code chunk because i think they accidentally included some stuff they didn't actually use, which was removed on later versions
<LordNlptp> and it would not surprise me in the least if the code/player data is ALMOST the same as the zt version but with support for the second msm6295

*/

ROM_START( raiden2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("prg1.u0212",   0x000001, 0x80000, CRC(4609b5f2) SHA1(272d2aa75b8ea4d133daddf42c4fc9089093df2e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "snd.u1110",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END


ROM_START( raiden2sw ) // original board with serial # 0008307
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu_1.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu_2.u0212",   0x000001, 0x80000, CRC(59abc2ec) SHA1(45f2dbd2dd46f5da07dae0dc486772f8e61f4c43) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu_5.u1110",  0x000000, 0x08000, CRC(c2028ba2) SHA1(f6a9322b669ff82dea6ecf52ad3bd5d0901cce1b) ) // 99.993896% match
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu_7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu_6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
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

ROM_START( raiden2hk )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2e.u0212",  0x000001, 0x80000, CRC(458d619c) SHA1(842bf0eeb5d192a6b188f4560793db8dad697683) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
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

ROM_START( raiden2j )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("prg0.u0211",   0x000000, 0x80000, CRC(09475ec4) SHA1(05027f2d8f9e11fcbd485659eda68ada286dae32) ) // rom1
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("rom2j.u0212",  0x000001, 0x80000, CRC(e4e4fb4c) SHA1(7ccf33fe9a1cddf0c7e80d7ed66d615a828b3bb9) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5.u1110",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2i )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu1.u0211",   0x000000, 0x80000, CRC(c1fc70f5) SHA1(a054f5ae9583972c406d9cf871340d5e072d71a3) ) /* Italian set */
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu2.u0212",   0x000001, 0x80000, CRC(28d5365f) SHA1(21efe29c2d373229c2ff302d86e59c2c94fa6d03) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5.c.u1110",  0x000000, 0x08000, CRC(5db9f922) SHA1(8257aab98657fe44df19d2a48d85fcf65b3d98c6) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
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
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("r2_prg_0.u0211",   0x000000, 0x80000, CRC(2abc848c) SHA1(1df4276d0074fcf1267757fa0b525a980a520f3d) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2_prg_1.u0212",   0x000001, 0x80000, CRC(509ade43) SHA1(7cdee7bb00a6a1c7899d10b96385d54c261f6f5a) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2_snd.u1110", 0x000000, 0x08000, CRC(6bad0a3e) SHA1(eb7ae42353e1984cd60b569c26cdbc3b025a7da6) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2_fx0.u0724",   0x000000, 0x020000, CRC(c709bdf6) SHA1(0468d90412b7590d67eaadc0a5e3537cd5e73943) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "r2_voi1.u1017", 0x00000, 0x40000, CRC(488d050f) SHA1(fde2fd64fea6bc39e1a42885d21d362bc6be2ac2) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2ea )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("r2.1.u0211",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("r2.2.u0212",  0x000001, 0x80000, CRC(bf7577ec) SHA1(98576af78760b8aef1ef3efe1ba963977c89d225) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2.5.u1110", 0x000000, 0x08000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2.7.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "r2.6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2eu ) // same as raiden2ea, different region
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("seibu_1.u0211",  0x000000, 0x80000, CRC(d7041be4) SHA1(3cf97132fba6f7b00c9059265f4e9f0bf1505b71) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu_2.u0212",  0x000001, 0x80000, CRC(beb71ddb) SHA1(471399ead1cdc27ac2a1139f9616f828efd14626) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "r2.5.u1110", 0x000000, 0x08000, CRC(f5f835af) SHA1(5be82ebc582d0da919e9ae1b9e64528bb295efc7) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "r2.7.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "r2.6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2eua ) // sort of a mixture of raiden2e easy set with voice rom of raiden2ea and 2f and a unique sound rom
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("seibu__1.27c020j.u1210",   0x000000, 0x40000, CRC(ed1514e3) SHA1(296125bfe3c4f3033f7aa319dd8554bc978c4a00) )
	ROM_RELOAD(0x100000, 0x40000)
	ROM_LOAD32_BYTE("seibu__2.27c2001.u1211",   0x000001, 0x40000, CRC(bb6ecf2a) SHA1(d4f628e9d0ed2897654f05a8a2541e1ed3faf8dd) )
	ROM_RELOAD(0x100001, 0x40000)
	ROM_LOAD32_BYTE("seibu__3.27c2001.u129",   0x000002, 0x40000, CRC(6a01d52c) SHA1(983b914592ab9d9c058bebb5bccf5c882e2b82de) )
	ROM_RELOAD(0x100002, 0x40000)
	ROM_LOAD32_BYTE("seibu__4.27c2001.u1212",   0x000003, 0x40000, CRC(e54bfa37) SHA1(4fabb23503fd9245a10cded15a6880415ca5ffd7) )
	ROM_RELOAD(0x100003, 0x40000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu__5.27c512.u1110", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu__7.fx0.27c210.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu__6.voice1.23c020.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END


ROM_START( raiden2g ) // this is the same code revision as raiden2eua but a german region
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("raiden_2_1.bin",   0x000000, 0x40000, CRC(ed1514e3) SHA1(296125bfe3c4f3033f7aa319dd8554bc978c4a00) )
	ROM_RELOAD(0x100000, 0x40000)
	ROM_LOAD32_BYTE("raiden_2_2.bin",   0x000001, 0x40000, CRC(bb6ecf2a) SHA1(d4f628e9d0ed2897654f05a8a2541e1ed3faf8dd) )
	ROM_RELOAD(0x100001, 0x40000)
	ROM_LOAD32_BYTE("raiden_2_3.bin",   0x000002, 0x40000, CRC(6a01d52c) SHA1(983b914592ab9d9c058bebb5bccf5c882e2b82de) )
	ROM_RELOAD(0x100002, 0x40000)
	ROM_LOAD32_BYTE("raiden_2_4.bin",   0x000003, 0x40000, CRC(81273f33) SHA1(074cedf44cc5286649cc101bce0b48d40234e472) )
	ROM_RELOAD(0x100003, 0x40000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "raiden_2_5.bin", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "raiden_2_7.bin", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_6.bin", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2nl )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("1_u0211.bin",   0x000000, 0x80000, CRC(53be3dd0) SHA1(304d118423e4085eea3b883bd625d90d21bb2054) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("2_u0212.bin",  0x000001, 0x80000, CRC(88829c08) SHA1(ecdfbafeeffcd009bbc4cf5bf797bcd4b5bfcf50) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5_u1110.bin",  0x000000, 0x08000, CRC(8f130589) SHA1(e58c8beaf9f27f063ffbcb0ab4600123c25ce6f3) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "6_u1017.bin", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2f ) // original board with serial # 12476 that matches raiden2nl set except the region and Audio CPU
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("1_u0211.bin",   0x000000, 0x80000, CRC(53be3dd0) SHA1(304d118423e4085eea3b883bd625d90d21bb2054) )   // == raiden2nl
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("seibu2_u0212.bin",  0x000001, 0x80000, CRC(8dcd8a8d) SHA1(be0681d5867d8b4f5fb78946a896d89827a71e8e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5_u1110.bin",  0x000000, 0x08000, CRC(f51a28f9) SHA1(7ae2e2ba0c8159a544a8fd2bb0c2c694ba849302) )   // == raiden2
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "6_u1017.bin", 0x00000, 0x40000, CRC(fb0fca23) SHA1(4b2217b121a66c5ab6015537609cf908ffedaf86) ) /* PCB silkscreened VOICE1 */

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END

ROM_START( raiden2u )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE("1.u0211",  0x000000, 0x80000, CRC(b16df955) SHA1(9b7fd85cf2f2c9fea657f3c38abafa93673b3933) )
	ROM_RELOAD(0x100000, 0x80000)
	ROM_LOAD16_BYTE("2.u0212",  0x000001, 0x80000, CRC(2a14b112) SHA1(84cd9891b5be0b71b2bae3487ad38bed3045305e) )
	ROM_RELOAD(0x100001, 0x80000)

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Soldered MASK ROM */

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu5.u1110", 0x000000, 0x08000, CRC(6d362472) SHA1(a362e500bb9492affde1f7a4da7e08dd16e755df) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu7.u0724", 0x000000, 0x020000, CRC(c7aa4d00) SHA1(9ad99d3891598c1ea3f12318400ee67666da56dd) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "raiden_2_seibu_bg-1.u0714", 0x000000, 0x200000, CRC(e61ad38e) SHA1(63b06cd38db946ad3fc5c1482dc863ef80b58fec) ) /* Soldered MASK ROM */
	ROM_LOAD( "raiden_2_seibu_bg-2.u075",  0x200000, 0x200000, CRC(a694a4bb) SHA1(39c2614d0effc899fe58f735604283097769df77) ) /* Soldered MASK ROM */

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-3.u0837", 0x400000, 0x200000, CRC(897a0322) SHA1(abb2737a2446da5b364fc2d96524b43d808f4126) ) /* Soldered MASK ROM */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-4.u0836", 0x400002, 0x200000, CRC(b676e188) SHA1(19cc838f1ccf9c4203cd0e5365e5d99ff3a4ff0f) ) /* Soldered MASK ROM */

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu6.u1017", 0x00000, 0x40000, CRC(fab9f8e4) SHA1(b1eff154c4f766b2d272ac6a57f8d54c9e39e3bb) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "raiden_2_pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Soldered MASK ROM */

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "jj4b02__ami18cv8-15.u0342.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "jj4b01__mmipal16l8bcn.u0341.jed", 0x0000, 0x335, CRC(d1a039af) SHA1(f88ff8674d5be17ae9085b51aefcf6abf0574883) )
ROM_END


ROM_START( raiden2dx ) // this set is very weird, it's Raiden II on a Raiden DX board, I'm assuming for now that it uses Raiden DX graphics, but could be wrong.
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("u1210.bin", 0x000000, 0x80000, CRC(413241e0) SHA1(50fa501db91412baea474a8faf8ad483f3a119c7) )
	ROM_LOAD32_BYTE("prg1_u1211.bin", 0x000001, 0x80000, CRC(93491f56) SHA1(2239980fb7267906e4c3985703c2dc2932b23705) )
	ROM_LOAD32_BYTE("u129.bin",  0x000002, 0x80000, CRC(e0932b6c) SHA1(04f1ca885d220e802023042438f63e40e4106696) )
	ROM_LOAD32_BYTE("u1212.bin", 0x000003, 0x80000, CRC(505423f4) SHA1(d8e65580deec05dd84c4cf3074cb690e3764c625) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "u1110.bin",  0x000000, 0x08000,  CRC(b8ad8fe7) SHA1(290896f811f717ef6e3ec2152d4db98a9fe9b310) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	//ROM_LOAD( "fx0_u0724.bin",    0x000000,   0x020000,   CRC(ded3c718) SHA1(c722ec45cd1b2dab23aac14e9113e0e9697830d3) ) // bad dump
	ROM_LOAD( "7_u0724.bin", 0x000000, 0x020000, CRC(c9ec9469) SHA1(a29f480a1bee073be7a177096ef58e1887a5af24) ) /* PCB silkscreened FX0 */

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

/* Raiden DX sets */

ROM_START( raidendx )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.4n",   0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.4p",   0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.6n",   0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.6p",   0x000003, 0x80000, CRC(2a2003e8) SHA1(f239b351759babe4683d16e745a5ac2f3c2ab06b) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidendxa1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("dx_1h.4n",   0x000000, 0x80000, BAD_DUMP CRC(7624c36b) SHA1(84c17f2988031210d06536710e1eac558f4290a1) ) // bad
	ROM_LOAD32_BYTE("dx_2h.4p",   0x000001, 0x80000, CRC(4940fdf3) SHA1(c87e307ed7191802583bee443c7c8e4f4e33db25) )
	ROM_LOAD32_BYTE("dx_3h.6n",   0x000002, 0x80000, CRC(6c495bcf) SHA1(fb6153ecc443dabc829dda6f8d11234ad48de88a) )
	ROM_LOAD32_BYTE("dx_4h.6k",   0x000003, 0x80000, CRC(9ed6335f) SHA1(66975204b120915f23258a431e19dbc017afd912) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) )
ROM_END

ROM_START( raidendxa2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.bin",   0x000000, 0x80000, CRC(22b155ae) SHA1(388151e2c8fb301bd5bc66a974e9fe16816ae0bc) )
	ROM_LOAD32_BYTE("2d.bin",   0x000001, 0x80000, CRC(2be98ca8) SHA1(491e990405b0ad3de45bdbcc2453af9215ae19c8) )
	ROM_LOAD32_BYTE("3d.bin",   0x000002, 0x80000, CRC(b4785576) SHA1(aa5eee7b0c635c6d18a7fc1e037bf570a677dd90) )
	ROM_LOAD32_BYTE("4d.bin",   0x000003, 0x80000, CRC(5a77f7b4) SHA1(aa757e6308893ca63963170c5b1743de7c7ab034) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidendxk )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("rdxj_1.bin",   0x000000, 0x80000, CRC(b5b32885) SHA1(fb3c592b2436d347103c17bd765176062be95fa2) )
	ROM_LOAD32_BYTE("rdxj_2.bin",   0x000001, 0x80000, CRC(7efd581d) SHA1(4609a0d8afb3d62a38b461089295efed47beea91) )
	ROM_LOAD32_BYTE("rdxj_3.bin",   0x000002, 0x80000, CRC(55ec0e1d) SHA1(6be7f268df51311a817c1c329a578b38abb659ae) )
	ROM_LOAD32_BYTE("rdxj_4.bin",   0x000003, 0x80000, CRC(f8fb31b4) SHA1(b72fd7cbbebcf3d1b2253c309fcfa60674776467) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidendxu )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1a.u1210", 0x000000, 0x80000, CRC(53e63194) SHA1(a957330e14649cf46ad27fb99c460576c59e60b1) )
	ROM_LOAD32_BYTE("2a.u1211", 0x000001, 0x80000, CRC(ec8d1647) SHA1(5ceae132c6c09d6bb8565e9141ee1170bbdfd5fc) )
	ROM_LOAD32_BYTE("3a.u129",  0x000002, 0x80000, CRC(7dbfd73d) SHA1(43cb1dbc3ccbded64fc300c262d1fd528e0391a2) )
	ROM_LOAD32_BYTE("4a.u1212", 0x000003, 0x80000, CRC(cb41a459) SHA1(532f0ed00a5b50a7537e5f48884d632aa5b92fb0) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END

ROM_START( raidendxg )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1d.u1210", 0x000000, 0x80000, CRC(14d725fc) SHA1(f12806f64f069fdc4ee29b309a32f7ca00b36f93) )
	ROM_LOAD32_BYTE("2d.u1211", 0x000001, 0x80000, CRC(5e7e45cb) SHA1(94eff893b5335c522f1c063c3175b9bac87b0a25) )
	ROM_LOAD32_BYTE("3d.u129",  0x000002, 0x80000, CRC(f0a47e67) SHA1(8cbd21993077b2e01295db6e343cae9e0e4bfefe) )
	ROM_LOAD32_BYTE("4d.u1212", 0x000003, 0x80000, CRC(6bde6edc) SHA1(c3565a55b858c10659fd9b93b1cd92bc39e6446d) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.u1110", 0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "7.u0724", 0x000000, 0x020000, CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj-3.u0837", 0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836", 0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END


ROM_START( raidendxnl )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("u1210_4n.bin", 0x000000, 0x80000, CRC(c589019a) SHA1(9bdd7f7d0bca16d67ba234d8a1fed5d2c8ab7191) )
	ROM_LOAD32_BYTE("u1211_4p.bin", 0x000001, 0x80000, CRC(b2222254) SHA1(b0e41d88111a96f0c0fb11b20ea99f436e8d493d) )
	ROM_LOAD32_BYTE("u129_6n.bin",  0x000002, 0x80000, CRC(60f04634) SHA1(50f1b721a017d879838d920cf5d5355aa024e09b) )
	ROM_LOAD32_BYTE("u1212_6p.bin", 0x000003, 0x80000, CRC(21ec37cc) SHA1(6da629e2bb5bd4c2192156af017148e99e274544) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "u1110_5b.bin", 0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "seibu_7b_u724.bin", 0x000000, 0x020000, CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj-3.u0837", 0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836", 0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "seibu_6_u1017.bin", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END


ROM_START( raidendxj )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("rdxj_1.u1211", 0x000000, 0x80000, CRC(5af382e1) SHA1(a11fc181da322f484815f55a510ce7e6c7df2d60) )
	ROM_LOAD32_BYTE("rdxj_2.u0212", 0x000001, 0x80000, CRC(899966fc) SHA1(0f91c2b05a44afb4c4b74e115a8fa530fb6d6414) )
	ROM_LOAD32_BYTE("rdxj_3.u129",  0x000002, 0x80000, CRC(e7f08013) SHA1(1f99672d8fdbda847c6552da210c417b21ca78ac) )
	ROM_LOAD32_BYTE("rdxj_4.u1212", 0x000003, 0x80000, CRC(78037e1f) SHA1(8d9c4188ca808e670e330e70e906bb1d27e36492) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313", 0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "rdxj_5.u1110", 0x000000, 0x08000, CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "rdxj_7.u0724", 0x000000, 0x020000, CRC(ec31fa10) SHA1(e39c9d95699dbeb21e3661d863eee503c9011bbc) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back-1.u075",  0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back-2.u0714", 0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-1.u0811", 0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "raiden_2_seibu_obj-2.u082",  0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj-3.u0837", 0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj-4.u0836", 0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "rdxj_6.u1017", 0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "pcm.u1018", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END



ROM_START( raidendxch )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("rdxc_1.u1210", 0x000000, 0x80000, CRC(2154c6ae) SHA1(dc794f8ddbd8a6267db37fe4e3ed44e06e9b84b7) )
	ROM_LOAD32_BYTE("rdxc_2.u1211", 0x000001, 0x80000, CRC(73bb74b7) SHA1(2f197adbe89d96c9e75054c568c380fdd2e80162))
	ROM_LOAD32_BYTE("rdxc_3.u129",  0x000002, 0x80000, CRC(50f0a6aa) SHA1(68579f8e73fe06b458368ac9cac0b33370cf3b4e))
	ROM_LOAD32_BYTE("rdxc_4.u1212", 0x000003, 0x80000, CRC(00071e70) SHA1(8a03ea0e650936e48cdd21ff84132742649920fe) )

	// no other roms present with this set, so the ones below could be wrong
	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.6s",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) ) /* Shared with original Raiden 2 */

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "dx_5.5b",  0x000000, 0x08000,  CRC(8c46857a) SHA1(8b269cb20adf960ba4eb594d8add7739dbc9a837) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD( "dx_7.4s",    0x000000,   0x020000,   CRC(c73986d4) SHA1(d29345077753bda53560dedc95dd23f329e521d9) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "dx_back1.1s",   0x000000, 0x200000, CRC(90970355) SHA1(d71d57cd550a800f583550365102adb7b1b779fc) )
	ROM_LOAD( "dx_back2.2s",   0x200000, 0x200000, CRC(5799af3e) SHA1(85d6532abd769da77bcba70bd2e77915af40f987) )

	ROM_REGION32_LE( 0x800000, "gfx3", 0 ) /* sprite gfx (encrypted) */
	ROM_LOAD32_WORD( "obj1",        0x000000, 0x200000, CRC(ff08ef0b) SHA1(a1858430e8171ca8bab785457ef60e151b5e5cf1) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "obj2",        0x000002, 0x200000, CRC(638eb771) SHA1(9774cc070e71668d7d1d20795502dccd21ca557b) ) /* Shared with original Raiden 2 */
	ROM_LOAD32_WORD( "dx_obj3.4k",  0x400000, 0x200000, CRC(ba381227) SHA1(dfc4d659aca1722a981fa56a31afabe66f444d5d) )
	ROM_LOAD32_WORD( "dx_obj4.6k",  0x400002, 0x200000, CRC(65e50d19) SHA1(c46147b4132abce7314b46bf419ce4773e024b05) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_6.3b",   0x00000, 0x40000, CRC(9a9196da) SHA1(3d1ee67fb0d40a231ce04d10718f07ffb76db455) )

	ROM_REGION( 0x100000, "oki2", 0 )   /* ADPCM samples */
	ROM_LOAD( "dx_pcm.3a", 0x00000, 0x40000, CRC(8cf0d17e) SHA1(0fbe0b1e1ca5360c7c8329331408e3d799b4714c) ) /* Shared with original Raiden 2 */
ROM_END



/* Zero Team sets */
/* Zero team is slightly older hardware (early 93 instead of late 93) but
almost identical to raiden 2 with a few key differences:
Zero Team:                     Raiden 2:
BG/FG roms marked MUSHA        BG/FG roms marked RAIDEN 2
SEI251 fg/sprite gate array    SEI252 fg/sprite gate array
about 15 74xx logic chips      SEI360 gate array
3x dipswitch arrays            2x dipswitch arrays
4x 8bit program roms           2x 16bit program roms (some older pcbs have 4x 8bit like zt)
YM3812 plus Y3014              YM2151 plus Y3012 plus NJM4550 (some older pcbs have YM2151, Y3014)
1x OKI M6295 & voice rom       2x OKI M6295s & 2x voice roms
2x 8bit licensee bgroms        1x 16bit licensee bgrom
2x fg/sprite mask roms         4x fg/sprite mask roms
4x pals (two are stacked)      2x pals
*/
/* ZERO TEAM Seibu Kaihatsu 1993

(C) 1993 ZERO TEAM SEIBU KAIHATSU INC.,o
|----------------------------------------------------------|
|      1    2   3   4   5   6    7      8      9     10    |
|LA4460    M6295   6   Z8400A          BATTERY3.6v        A|
|   YM3812       LH5116 5                                 B|
|     VOL   YM3014                                         |
|HB-45A         |------|                               C|
|HB-2           |SIE150|    LH5116    |---------|          |
|RC220          |      |    LH5116    | SEI251  | 28.6360 D|
|RC220          |------|              |SB03-012 | MHz      |
|RC220                                |(QFP208) |         E|
|J                     OBJ-2    OBJ-1 |         |         F|
|A                                    |---------|          |
|M                                                        G|
|M                                              LH522258   |
|A                                              LH522258  H|
|                                               LH522258  J|
|               DSW1(8) PAL2  1       2         LH522258  K|
|           DSW2(8)     PAL14 4       3     |---------|   L|
|           DSW3(8) PAL3                    |SEI1000  |   M|
|                           COPX-D2         |SB01-001 |   N|
|                                           |(QFP184) |    |
|         CXK5863                           |         |   P|
|         CXK5863      |------|             |---------|    |
|                      |SEI200|                           Q|
|                      |      |  8             |----|     R|
|                      |------|                |V30 |      |
|       BACK-2     BACK-1        7             |----|     S|
|----------------------------------------------------------|
Notes:
      V30 clock    - 16.000MHz [32/2]. Chip is stamped "NEC D70116HG-16 V30 NEC '84" (QFP52)
      Z80 clock    - 3.579545MHz [28.63636/8]
      YM3812 clock - 3.579545MHz [28.63636/8]
      Yamaha DAC   - ym3014 mono dac
      M6295 clocks - 1.022MHz [28.63636/28] and pin 7 HIGH
      LH52258      - Sharp LH52258 32k x8 SRAM (= 62256)
      CXK5863      - Sony CXK5863 8k x8 SRAM (= 6264)
      6116         - 2k x8 SRAM
      LH5116       - 2k x8 SRAM
      HB-45A       - Seibu custom ceramic module sound DAC (SIP20)
      HB-2         - Seibu custom ceramic module connected to coin counters (SIP10)
      RC220        - Custom resistor network module used for inputs (SIP14)
      VSync        - 55.4859Hz  \
      HSync        - 15.5586kHz / not measured but assumed same as Raiden 2 DX
      PAL14        - Two pals in a stack, along with a resistor and wires to sei0200 and the sie150
                     'V3C004X'  (DIP20), has a resistor between one pin and gnd
                     <unknown, maybe V3C001, under above pal> (DIP20) u0310
      PAL2         - TIBPAL16L8-25CN stamped 'V3C002' (DIP20) u0322
      PAL3         - AMI 18CV8P-15 stamped 'V3C003' (DIP20) u0619
      ROMs         - 6         - 27C020 EPROM labelled 'SEIBU 6' at location U105 (DIP32), pcb labeled VOICE
                     5         - 27C512 EPROM labelled 'SEIBU 5' at location U1110 (DIP28)
                     *OBJ-1    - 16Mbit TC5316200BP MaskROM stamped 'MUSHA OBJ-1' at location U0811 (DIP42)
                     *OBJ-2    - 16Mbit TC5316200BP MaskROM stamped 'MUSHA OBJ-2' at location U082 (DIP42)
                     1         - 27C020 EPROM labelled 'SEIBU 1' at location U024 (DIP32)
                     2         - 27C020 EPROM labelled 'SEIBU 2' at location U025 (DIP32)
                     3         - 27C020 EPROM labelled 'SEIBU 3' at location U023 (DIP32)
                     4         - 27C020 EPROM labelled 'SEIBU 4' at location U026 (DIP32)
                     *BACK-1   - 8Mbit TC538200AP MaskROM stamped 'MUSHA BACK-1' at location U075 (DIP42)
                     *BACK-2   - 4Mbit TC534200AP MaskROM stamped 'MUSHA BACK-2' at location U0714 (DIP40)
                     7         - 27C512 EPROM labelled 'SEIBU 7' at location U072 (DIP28)
                     8         - 27C512 EPROM labelled 'SEIBU 8' at location U077 (DIP28)
                     *COPX-D2  - 2M MaskROM stamped 'COPX-D2' at location U0313 (DIP40)

                     * = these ROMs are soldered-in

      SEIBU Custom ICs -
                        SIE150 (QFP100) - z80 interface
                        SEI251 SB03-012 (QFP208) - fg/sprite gfx and its decryption
                        SEI0200 TC110G21AF 0076 (QFP100) - bg gfx
                        SEI1000 SB01-001 (QFP184) - main protection

*/


ROM_START( zeroteam ) // Fabtek, US licensee, displays 'USA' under zero team logo, board had serial 'Seibu Kaihatsu No. 0001468' on it, as well as AAMA 0458657
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("seibu__1.u024.5k",   0x000000, 0x40000, CRC(25aa5ba4) SHA1(40e6047620fbd195c87ac3763569af099096eff9) ) // alternate label "1"
	ROM_LOAD32_BYTE("seibu__3.u023.6k",   0x000002, 0x40000, CRC(ec79a12b) SHA1(515026a2fca92555284ac49818499af7395783d3) ) // alternate label "3"
	ROM_LOAD32_BYTE("seibu__2.u025.6l",   0x000001, 0x40000, CRC(54f3d359) SHA1(869744185746d55c60d2f48eabe384a8499e00fd) ) // alternate label "2"
	ROM_LOAD32_BYTE("seibu__4.u026.5l",   0x000003, 0x40000, CRC(a017b8d0) SHA1(4a93ff1ab18f4b61c7ef580995f64840c19ce6b9) ) // alternate label "4"

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "seibu__5.u1110.5b",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // // alternate label "5"
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "seibu__7.u072.5s",    0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) ) // alternate label "7"
	ROM_LOAD16_BYTE( "seibu__8.u077.5r",    0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) ) // alternate label "8"

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "seibu__6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // alternate label "6"

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteama ) // No licensee, original japan?
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.u024.5k",   0x000000, 0x40000, CRC(bd7b3f3a) SHA1(896413901a429d0efa3290f61920063c81730e9b) )
	ROM_LOAD32_BYTE("3.u023.6k",   0x000002, 0x40000, CRC(19e02822) SHA1(36c9b887eaa9b9b67d65c55e8f7eefd08fe0be15) )
	ROM_LOAD32_BYTE("2.u025.6l",   0x000001, 0x40000, CRC(0580b7e8) SHA1(d4416264aa5acdaa781ebcf51f128b3e665cc903) )
	ROM_LOAD32_BYTE("4.u026.5l",   0x000003, 0x40000, CRC(cc666385) SHA1(23a8878315b6009dcc1f27e49572e5be29f6a1a6) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.a.u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.a.u072.5s", 0x000000,   0x010000, CRC(eb10467f) SHA1(fc7d576dc41bc878ff20f0370e669e19d54fcefb) )
	ROM_LOAD16_BYTE( "8.a.u077.5r", 0x000001,   0x010000, CRC(a0b2a09a) SHA1(9b1f6c732000b84b1ad635f332ebead5d65cc491) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

/* this set, consisting of updated program roms, is a later version or hack of zero team to incorporate the writing
of the fg sei251 'key data' to the pcb on bootup (like raiden 2 does) rather than relying on the sram to hold the
keys as programmed from factory (or via the suicide revival kit below); hence this romset is immune to the common
problem of the 3.6v lithium battery dying and the missing keys to cause the sprites to show up as gibberish */
// note: it is possible *but not proven* that this specific set in mame is a frankenstein-hybrid of the japan and us
// sets, using the sound and char roms from us set and code from later japan set. This would make sense if it was dumped
// from a 'fixed, suicide free' modified us board where someone swapped in the later suicideless japan code roms.
ROM_START( zeroteamb ) // No licensee, later japan?
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1b.u024.5k",   0x000000, 0x40000, CRC(157743d0) SHA1(f9c84c9025319f76807ef0e79f1ee1599f915b45) )
	ROM_LOAD32_BYTE("3b.u023.6k",   0x000002, 0x40000, CRC(fea7e4e8) SHA1(08c4bdff82362ae4bcf86fa56fcfc384bbf82b71) )
	ROM_LOAD32_BYTE("2b.u025.6l",   0x000001, 0x40000, CRC(21d68f62) SHA1(8aa85b38e8f36057ef6c7dce5a2878958ce93ce8) )
	ROM_LOAD32_BYTE("4b.u026.5l",   0x000003, 0x40000, CRC(ce8fe6c2) SHA1(69627867c7866e43e771ab85014553117044d18d) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.u1110.5b",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.u072.5s",   0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.u077.5r",   0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteamc ) // Liang Hwa, Taiwan licensee, no special word under logo on title
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("b1.u024.5k",   0x000000, 0x40000, CRC(528de3b9) SHA1(9ca8cdc0212f2540e852d20ab4c04f68b967d024) )
	ROM_LOAD32_BYTE("b3.u023.6k",   0x000002, 0x40000, CRC(3688739a) SHA1(f98f461fb8e7804b3b4020a5e3762d36d6458a62) )
	ROM_LOAD32_BYTE("b2.u025.6l",   0x000001, 0x40000, CRC(5176015e) SHA1(6b372564b2f1b1f56cae0c98f4ca588b784bfa3d) )
	ROM_LOAD32_BYTE("b4.u026.5l",   0x000003, 0x40000, CRC(c79925cb) SHA1(aaff9f626ec61bc0ff038ebd722fe361dccc49fb) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.c.u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "b7.u072.5s",  0x000000,   0x010000, CRC(30ec0241) SHA1(a0d0be9458bf97cb9764fb85c988bb816710475e) )
	ROM_LOAD16_BYTE( "b8.u077.5r",  0x000001,   0x010000, CRC(e18b3a75) SHA1(3d52bba8d47d0d9108ee79014fd64d6e856a3fde) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "6.c.u105.4a", 0x00000, 0x40000,  CRC(b4a6e899) SHA1(175ab656db3c3258ff10eede89890f62435d2298) )

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

ROM_START( zeroteamd ) // Dream Soft, Korea licensee, no special word under logo on title; board had serial 'no 1041' on it.
	// this is weird, on other zt sets the rom order is 1 3 2 4, but this one is 1 3 4 2. blame seibu or whoever marked the roms, which were labeled in pen
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.d.u024.5k",   0x000000, 0x40000, CRC(6cc279be) SHA1(63143ba3105d24d133e60ffdb3edc2ceb2d5dc5b) )
	ROM_LOAD32_BYTE("3.d.u023.6k",   0x000002, 0x40000, CRC(0212400d) SHA1(28f77b5fddb9d724b735c3ff2255bd518b166e67) )
	ROM_LOAD32_BYTE("4.d.u025.6l",   0x000001, 0x40000, CRC(08813ebb) SHA1(454779cec2fd0e71b72f7161e7d9334893ee42de) )
	ROM_LOAD32_BYTE("2.d.u026.5l",   0x000003, 0x40000, CRC(9236129d) SHA1(8561ab62e3593cd9353d9ffddedbdb77e9ae2c45) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "512kb.u1110.5b",  0x000000, 0x08000, CRC(efc484ca) SHA1(c34b8e3e7f4c2967bc6414348993478ed637d338) ) // this is a soldered mask rom on this pcb version! the contents match the taiwan version eprom; the mask rom has no label
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "512kb.u072.5s",   0x000000,   0x010000, CRC(30ec0241) SHA1(a0d0be9458bf97cb9764fb85c988bb816710475e) ) // this is a soldered mask rom on this pcb version! the contents match the taiwan version eprom; the mask rom has no label
	ROM_LOAD16_BYTE( "512kb.u077.5r",   0x000001,   0x010000, CRC(e18b3a75) SHA1(3d52bba8d47d0d9108ee79014fd64d6e856a3fde) ) // this is a soldered mask rom on this pcb version! the contents match the taiwan version eprom; the mask rom has no label

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "8.u105.4a", 0x00000, 0x40000,  CRC(b4a6e899) SHA1(175ab656db3c3258ff10eede89890f62435d2298) ) // same rom as '6' labeled one in zeroteamc above but has '8' written on label in pen

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END
// A version of the above exists (which dr.kitty used to own) which DOES have 'Korea' under the logo on title, needs dumping

ROM_START( zeroteams ) // No license, displays 'Selection' under logo
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1_sel.bin",   0x000000, 0x40000, CRC(d99d6273) SHA1(21dccd5d71c720b8364406835812b3c9defaff6c) )
	ROM_LOAD32_BYTE("3_sel.bin",   0x000002, 0x40000, CRC(0a9fe0b1) SHA1(3588fe19788f77d07e9b5ab8182b94362ffd0024) )
	ROM_LOAD32_BYTE("2_sel.bin",   0x000001, 0x40000, CRC(4e114e74) SHA1(fcccbb68c6b7ffe8d109ed3a1ec9120d338398f9) )
	ROM_LOAD32_BYTE("4_sel.bin",   0x000003, 0x40000, CRC(0df8ba94) SHA1(e07dce6cf3c3cfe1ea3b7f01e18833c1da5ed1dc) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5_sel.bin",  0x000000, 0x08000, CRC(ed91046c) SHA1(de815c999aeeb814d3f091d5a9ac34ea9a388ddb) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.u072.5s",   0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.u077.5r",   0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.bin

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
ROM_END

/*
"Zero Team Suicide Revival Kit"

As the name implies, this is used to give life again to a "suicided" ZT PCB, where the 3.6v
lithium battery which backs up the FG/sprite encryption keys has died, and the sprites display
as garbage blocks.
To use: replace the 3.6v battery with a working one, and then remove the normal four code roms
and install these instead.
Boot the pcb, it should rewrite the sei251 decryption keys and display a message on screen.
Next, turn off power and reinsert the old code roms, and the pcb should now have working sprites.
*/

ROM_START( zeroteamsr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("zteam1.u24",   0x000000, 0x40000, CRC(c531e009) SHA1(731881fca3dc0a8269ecdd295ba7119d93c892e7) )
	ROM_LOAD32_BYTE("zteam3.u23",   0x000002, 0x40000, CRC(1f988808) SHA1(b1fcb8c96e57c4942bc032d42408d7289c6a3681) )
	ROM_LOAD32_BYTE("zteam2.u25",   0x000001, 0x40000, CRC(b7234b93) SHA1(35bc093e8ad4bce1d2130a392ed1b9487a5642a1) )
	ROM_LOAD32_BYTE("zteam4.u26",   0x000003, 0x40000, CRC(c2d26708) SHA1(d65191b40f5dd7cdbbc004e2de10134db6092fd1) )

	ROM_REGION( 0x40000, "user2", 0 )   /* COPX */
	ROM_LOAD( "copx-d2.u0313.6n",   0x00000, 0x40000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.5c",  0x000000, 0x08000, CRC(7ec1fbc3) SHA1(48299d6530f641b18764cc49e283c347d0918a47) ) // 5.5c
	ROM_CONTINUE(0x10000,0x8000)
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "7.u072.5s",   0x000000,   0x010000,   CRC(9f6aa0f0) SHA1(1caad7092c07723d12a07aa363ae2aa69cb6be0d) )
	ROM_LOAD16_BYTE( "8.u077.5r",   0x000001,   0x010000,   CRC(68f7dddc) SHA1(6938fa974c6ef028751982fdabd6a3820b0d30a8) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "musha_back-1.u075.4s",   0x000000, 0x100000, CRC(8b7f9219) SHA1(3412b6f8a4fe245e521ddcf185a53f2f4520eb57) )
	ROM_LOAD( "musha_back-2.u0714.2s",   0x100000, 0x080000, CRC(ce61c952) SHA1(52a843c8ba428b121fab933dd3b313b2894d80ac) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (encrypted) (diff encrypt to raiden2? ) */
	ROM_LOAD32_WORD( "musha_obj-1.u0811.6f",  0x000000, 0x200000, CRC(45be8029) SHA1(adc164f9dede9a86b96a4d709e9cba7d2ad0e564) )
	ROM_LOAD32_WORD( "musha_obj-2.u082.5f",  0x000002, 0x200000, CRC(cb61c19d) SHA1(151a2ce9c32f3321a974819e9b165dddc31c8153) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "6.u105.4a", 0x00000, 0x40000,  CRC(48be32b1) SHA1(969d2191a3c46871ee8bf93088b3cecce3eccf0c) ) // 6.4a

	ROM_REGION( 0x10000, "pals", 0 )    /* PALS */
	ROM_LOAD( "v3c001.pal.u0310.jed", 0x0000, 0x288, NO_DUMP) // located UNDER v3c004x, unknown pal type
	ROM_LOAD( "v3c002.tibpal16l8-25.u0322.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c003.ami18cv8p-15.u0619.jed", 0x0000, 0x288, NO_DUMP)
	ROM_LOAD( "v3c004x.ami18cv8pc-25.u0310.jed", 0x0000, 0x288, NO_DUMP) // located piggybacking on v3c001 and attached to some rework wires
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
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v30 main cpu */
	ROM_LOAD32_BYTE("1.u024",   0x000000, 0x40000, CRC(185437f9) SHA1(e46950b6a549d11dc57105dd7d9cb512a8ecbe70) )
	ROM_LOAD32_BYTE("2.u025",   0x000001, 0x40000, CRC(a2b052df) SHA1(e8bf9ab3d5d4e601ea9386e1f2d4e017b025407e) )
	ROM_LOAD32_BYTE("3.u023",   0x000002, 0x40000, CRC(293fd6c1) SHA1(8b1a231f4bedbf9c0f347330e13fdf092b9888b4) )
	ROM_LOAD32_BYTE("4.u026",   0x000003, 0x40000, CRC(5adf20bf) SHA1(42a0bb5a460c656675b2c432c043fc61a9049276) )

	ROM_REGION( 0x40000, "user2", ROMREGION_ERASEFF )   /* COPX */
	/* Not populated */

	ROM_REGION( 0x30000, "audiocpu", ROMREGION_ERASEFF ) /* 64k code for sound Z80 */
	ROM_LOAD( "8.u1110",  0x000000, 0x08000, CRC(2dc2f81a) SHA1(0f6605042e0e295b4256b43dbdf5d53daebe1a9a) )
	ROM_CONTINUE(0x10000,0x8000)
	ROM_CONTINUE(0x20000,0x10000) // TODO
	ROM_COPY( "audiocpu", 0x000000, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* chars */
	ROM_LOAD16_BYTE( "6.u072.5s",   0x000000,   0x010000, CRC(a788402d) SHA1(8a1ac4760cf75cd2e32c1d15f36ad15cce3d411b) )
	ROM_LOAD16_BYTE( "5.u077.5r",   0x000001,   0x010000, CRC(478deced) SHA1(88cd72cb76bbc1c4255c3dfae4b9a10af9b050b2) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* background gfx */
	ROM_LOAD( "bg-1.u075",   0x000000, 0x100000, CRC(ac087560) SHA1(b6473b20c55ec090961cfc46a024b3c5b707ec25) )
	ROM_LOAD( "7.u0714",     0x100000, 0x080000, CRC(296105dc) SHA1(c2b80d681646f504b03c2dde13e37b1d820f82d2) )

	ROM_REGION32_LE( 0x800000, "gfx3", ROMREGION_ERASEFF ) /* sprite gfx (not encrypted) */
	ROM_LOAD32_WORD( "obj-1.u0811",  0x000000, 0x200000, CRC(6ae993eb) SHA1(d9713c79eacb4b3ce5e82dd3ce39003e3a433d8f) )
	ROM_LOAD32_WORD( "obj-2.u082",   0x000002, 0x200000, CRC(26c806ee) SHA1(899a76a1b3f933c6f5cb6b5dcdf5b58e1b7e49c6) )

	ROM_REGION( 0x100000, "oki1", 0 )   /* ADPCM samples */
	ROM_LOAD( "9.u105.4a", 0x00000, 0x40000, CRC(a7a0c5f9) SHA1(7882681ac152642aa4f859071f195842068b214b) )

	ROM_REGION( 0x100000, "oki2", ROMREGION_ERASEFF )   /* ADPCM samples */
ROM_END

const UINT16 raiden2_state::raiden_blended_colors[] = {
	// bridge tunnel entrance shadow
	0x380,

	// cloud
	0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3c6, 0x3c7, 0x3c8, 0x3c9, 0x3ca, 0x3cb, 0x3cc, 0x3cd, 0x3ce,

	// engine
	0x3d0, 0x3d1, 0x3d2, 0x3d3, 0x3d4, 0x3d5, 0x3d6, 0x3d7, 0x3d8, 0x3d9, 0x3da, 0x3db, 0x3dc, 0x3dd, 0x3de,

	// level 1 boss legs
	0x3f0, 0x3f1, 0x3f2, 0x3f3, 0x3f4, 0x3f5, 0x3f6, 0x3f7, 0x3f8, 0x3f9, 0x3fa, 0x3fb, 0x3fc, 0x3fd, 0x3fe,

	// water
	0x4f8, 0x4f9, 0x4fa, 0x4fb, 0x4fc, 0x4fd, 0x4fe,
	0x5c8, 0x5c9, 0x5ca, 0x5cb, 0x5cc, 0x5cd, 0x5ce,

	// wall shadow
	0x5de,

	// glass roof
	0x5e8, 0x5e9, 0x5ea, 0x5eb, 0x5ec, 0x5ed, 0x5ee,

	// house shadow plus stage 3 boss green pools
	0x5f8, 0x5f9, 0x5fa, 0x5fb, 0x5fc, 0x5fd, 0x5fe,

	// water and trees
	0x6c8, 0x6c9, 0x6ca, 0x6cb, 0x6cc, 0x6cd, 0x6ce,
	0x6d8, 0x6d9, 0x6da, 0x6db, 0x6dc, 0x6dd, 0x6de,
	0x6e8, 0x6e9, 0x6ea, 0x6eb, 0x6ec, 0x6ed, 0x6ee,
	0x6f8, 0x6f9, 0x6fa, 0x6fb, 0x6fc, 0x6fd, 0x6fe,

	// stage end panel, raiden dx logo plus misc stuff
	0x70d, 0x70e,
	0x71c, 0x71d, 0x71e,
	0x72d, 0x72e,
	0x73d, 0x73e,
	0x74d, 0x74e,
	0x75c,
	0x76c, 0x76d, 0x76e,
	0x77d, 0x77e,

	// logo in attract mode
	0x7c8, 0x7c9, 0x7ca, 0x7cb, 0x7cc, 0x7cd, 0x7ce,

	0xffff,
};

void raiden2_state::init_blending(const UINT16 *table)
{
	for(auto & elem : blend_active)
		elem = false;
	while(*table != 0xffff)
		blend_active[*table++] = true;
}

DRIVER_INIT_MEMBER(raiden2_state,raiden2)
{
	init_blending(raiden_blended_colors);
	static const int spri[5] = { 0, 1, 2, 3, -1 };
	cur_spri = spri;
	membank("mainbank1")->configure_entries(0, 4, memregion("maincpu")->base(), 0x10000);
	membank("mainbank2")->configure_entries(0, 4, memregion("maincpu")->base(), 0x10000);
	raiden2_decrypt_sprites(machine());
}

DRIVER_INIT_MEMBER(raiden2_state,raidendx)
{
	init_blending(raiden_blended_colors);
	static const int spri[5] = { 0, 1, 2, 3, -1 };
	cur_spri = spri;
	membank("mainbank1")->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x10000);
	membank("mainbank2")->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x10000);
	raiden2_decrypt_sprites(machine());
}

const UINT16 raiden2_state::xsedae_blended_colors[] = {
	0xffff,
};

DRIVER_INIT_MEMBER(raiden2_state,xsedae)
{
	init_blending(xsedae_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	cur_spri = spri;
	/* doesn't have banking */
}

const UINT16 raiden2_state::zeroteam_blended_colors[] = {
	// Player selection
	0x37e,
	// Boss spear shadow
	0x38e,
	// Scaffolding shadow
	0x52e,
	// Road brightening
	0x5de,

	0xffff
};


DRIVER_INIT_MEMBER(raiden2_state,zeroteam)
{
	init_blending(zeroteam_blended_colors);
	static const int spri[5] = { -1, 0, 1, 2, 3 };
	cur_spri = spri;
	membank("mainbank1")->configure_entries(0, 4, memregion("maincpu")->base(), 0x10000);
	membank("mainbank2")->configure_entries(0, 4, memregion("maincpu")->base(), 0x10000);
	zeroteam_decrypt_sprites(machine());
}

/* GAME DRIVERS */

// rev numbers at end of the line just indicate which sets are the same code revisions (just a region byte change), they don't reflect the actual order of release
GAME( 1993, raiden2,    0,        raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (US, set 1)", MACHINE_SUPPORTS_SAVE ) // rev 1
GAME( 1993, raiden2u,   raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (US, set 2)", MACHINE_SUPPORTS_SAVE ) // ?
GAME( 1993, raiden2hk,  raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden II (Hong Kong)", MACHINE_SUPPORTS_SAVE ) //  rev 1
GAME( 1993, raiden2sw,  raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Switzerland)", MACHINE_SUPPORTS_SAVE ) // rev 1
GAME( 1993, raiden2j,   raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Japan)", MACHINE_SUPPORTS_SAVE ) //  rev 1
GAME( 1993, raiden2i,   raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Italy)", MACHINE_SUPPORTS_SAVE ) // rev 2
GAME( 1993, raiden2nl,  raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (Holland)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, raiden2f,   raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (France)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, raiden2e,   raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (easy version, Korea?)", MACHINE_SUPPORTS_SAVE ) // rev 3 (Region 0x04) - Korea?, if regions are the same as RDX, no license or region message tho
GAME( 1993, raiden2ea,  raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu", "Raiden II (easy version, Japan?)", MACHINE_SUPPORTS_SAVE ) // rev 4 (Region 0x00) - Should be Japan, but the easy sets have no 'FOR USE IN JAPAN ONLY' display even when region is 00
GAME( 1993, raiden2eu,  raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (easy version, US set 2)", MACHINE_SUPPORTS_SAVE ) //  ^
GAME( 1993, raiden2eua, raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden II (easy version, US set 1)", MACHINE_SUPPORTS_SAVE ) // rev 3 and 4 mix?
GAME( 1993, raiden2g,   raiden2,  raiden2,  raiden2,  raiden2_state, raiden2,  ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden II (easy version, Germany)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, raiden2dx,  raiden2,  raidendx, raiden2,  raiden2_state, raidendx, ROT270, "Seibu Kaihatsu", "Raiden II (harder, Raiden DX hardware)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, raidendx,   0,        raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (UK)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxa1, raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden DX (Hong Kong, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxa2, raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu (Metrotainment license)", "Raiden DX (Hong Kong, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxk,  raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxu,  raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu (Fabtek license)", "Raiden DX (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxg,  raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu (Tuning license)", "Raiden DX (Germany)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxnl, raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Holland)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxj,  raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu", "Raiden DX (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, raidendxch, raidendx, raidendx, raidendx, raiden2_state, raidendx, ROT270, "Seibu Kaihatsu (Ideal International Development Corp license) ", "Raiden DX (China)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, zeroteam,   0,        zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu (Fabtek license)", "Zero Team USA (US)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteama,  zeroteam, zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team (Japan?, earlier?)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteamb,  zeroteam, zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team (Japan?, later batteryless)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // reprograms the sprite decrypt data of the SEI251 on every boot, like raiden2 does. hack?
GAME( 1993, zeroteamc,  zeroteam, zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu (Liang Hwa license)", "Zero Team (Taiwan)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteamd,  zeroteam, zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu (Dream Soft license)", "Zero Team (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteams,  zeroteam, zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team Selection", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1993, zeroteamsr, zeroteam, zeroteam, zeroteam, raiden2_state, zeroteam, ROT0,   "Seibu Kaihatsu", "Zero Team Suicide Revival Kit", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // reprograms the sprite decrypt data of the SEI251 only, no game code

GAME( 1995, xsedae,     0,        xsedae,   xsedae,   raiden2_state, xsedae,   ROT0,   "Dream Island", "X Se Dae Quiz (Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
