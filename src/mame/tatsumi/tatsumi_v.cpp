// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
#include "emu.h"
#include "tatsumi.h"
#include "screen.h"


/**********************************
 *
 * Common routines
 *
 *********************************/

// TODO: move into device
void tatsumi_state::hd6445_crt_w(offs_t offset, uint8_t data)
{
	if (offset==0)
		m_hd6445_address = data & 0x3f;
	if (offset==1)
	{
		m_hd6445_reg[m_hd6445_address] = data;

		static char const *const regnames[40] =
		{
			"Horizontal Total Characters", "Horizontal Displayed Characters", "Horizontal Sync Position",   "Sync Width",
			"Vertical Total Rows",         "Vertical Total Adjust",           "Vertical Displayed Rows",    "Vertical Sync Position",
			"Interlace Mode and Skew",     "Max Raster Address",              "Cursor 1 Start",             "Cursor 1 End",
			"Screen 1 Start Address (H)",  "Screen 1 Start Address (L)",      "Cursor 1 Address (H)",       "Cursor 1 Address (L)",
			"Light Pen (H) (RO)",          "Light Pen (L) (RO)",              "Screen 2 Start Position",    "Screen 2 Start Address (H)",
			"Screen 2 Start Address (L)",  "Screen 3 Start Position",         "Screen 3 Start Address (H)", "Screen 3 Start Address (L)",
			"Screen 4 Start Position",     "Screen 4 Start Address (H)",      "Screen 4 Start Address (L)", "Vertical Sync Position Adjust",
			"Light Pen Raster (RO)",       "Smooth Scrolling",                "Control 1",                  "Control 2",
			"Control 3",                   "Memory Width Offset",             "Cursor 2 Start",             "Cursor 2 End",
			"Cursor 2 Address (H)",        "Cursor 2 Address (L)",            "Cursor 1 Width",             "Cursor 2 Width"
		};

		if(m_hd6445_address < 40)
			logerror("HD6445: register %s [%02x R%d] set %02x\n",regnames[m_hd6445_address],m_hd6445_address,m_hd6445_address,data);
		else
			logerror("HD6445: illegal register access [%02x R%d] set %02x\n",m_hd6445_address,m_hd6445_address,data);
	}
}

uint16_t tatsumi_state::tatsumi_sprite_control_r(offs_t offset)
{
	return m_sprite_control_ram[offset];
}

void tatsumi_state::tatsumi_sprite_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sprite_control_ram[offset]);

	/* 0xe0 is bank switch, others unknown */
//  if ((offset==0xe0 && data&0xefff) || offset!=0xe0)
//      logerror("%s:  Tatsumi TZB215 sprite control %04x %08x\n", m_maincpu->pc(), offset, data);
}

// apply shadowing to underlying layers
// TODO: it might mix up with the lower palette bank instead (color bank 0x1400?)
void tatsumi_state::apply_shadow_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &shadow_bitmap, uint8_t xor_output)
{
	for(int y=cliprect.min_y;y<cliprect.max_y;y++)
	{
		for(int x=cliprect.min_x;x<cliprect.max_x;x++)
		{
			uint8_t shadow = shadow_bitmap.pix(y, x);
			// xor_output is enabled during Chen boss fight (where shadows have more brightness than everything else)
			// TODO: transition before fighting him should also black out all the background tilemaps too!?
			//       (more evidence that we need to mix with color bank 0x1400 instead of doing true RGB mixing).
			if(shadow ^ xor_output)
			{
				rgb_t shadow_pen = bitmap.pix(y, x);
				bitmap.pix(y, x) = rgb_t(shadow_pen.r() >> 1,shadow_pen.g() >> 1, shadow_pen.b() >> 1);
			}
		}
	}
}

void tatsumi_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(tatsumi_state::get_text_tile_info)
{
	int tile = m_videoram[tile_index];
	tileinfo.set(0, tile & 0xfff, tile >> 12, 0);
}

/**********************************
 *
 * Apache 3
 *
 *********************************/

void apache3_state::apache3_road_z_w(uint16_t data)
{
	m_apache3_road_z = data & 0xff;
}

void apache3_state::apache3_road_x_w(offs_t offset, uint8_t data)
{
	// Note: Double buffered. Yes, this is correct :)
	m_apache3_road_x_ram[data] = offset;
}

void apache3_state::draw_sky(bitmap_rgb32 &bitmap,const rectangle &cliprect, int palette_base, int start_offset)
{
	// all TODO
	if (start_offset&0x8000)
		start_offset=-(0x10000 - start_offset);

	start_offset=-start_offset;

start_offset-=48;
	for (int y=0; y<256; y++) {
		for (int x=0; x<320; x++) {
			int col=palette_base + y + start_offset;
			if (col<palette_base) col=palette_base;
			if (col>palette_base+127) col=palette_base+127;

			bitmap.pix(y, x) = m_palette->pen(col);
		}
	}
}

/* Draw the sky and ground, applying rotation (eventually). Experimental! */
void apache3_state::draw_ground(bitmap_rgb32 &dst, const rectangle &cliprect)
{
	if (0)
	{
		uint16_t gva = 0x180; // TODO
		uint8_t sky_val = m_apache3_rotate_ctrl[1] & 0xff;

		for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
		{
			uint16_t rgdb = 0;//m_apache3_road_x_ram[gva & 0xff];
			uint16_t gha = 0xf60; // test
			int ln = (((m_apache3_prom[gva & 0x7f] & 0x7f) + (m_apache3_road_z & 0x7f)) >> 5) & 3;

			if (gva & 0x100)
			{
				/* Sky */
				for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
				{
					dst.pix(y, x) = m_palette->pen(0x100 + (sky_val & 0x7f));

					/* Update horizontal counter? */
					gha = (gha + 1) & 0xfff;
				}
			}
			else
			{
				/* Ground */
				for (int x = cliprect.min_x; x <= cliprect.max_x; ++x)
				{
					uint16_t hval = (rgdb + gha) & 0xfff; // Not quite

					if (hval & 0x800)
						hval ^= 0x1ff; // TEST
					//else
						//hval = hval;

					uint8_t pixels = m_apache3_g_ram[(((gva & 0xff) << 7) | ((hval >> 2) & 0x7f))];
					int pix_sel = hval & 3;

					uint8_t colour = (pixels >> (pix_sel << 1)) & 3;
					colour = (BIT(hval, 11) << 4) | (colour << 2) | ln;

					/* Draw the pixel */
					dst.pix(y, x) = m_palette->pen(0x200 + colour);

					/* Update horizontal counter */
					gha = (gha + 1) & 0xfff;
				}
			}

			/* Update sky counter */
			sky_val++;
			gva = (gva + 1) & 0x1ff;
		}
	}
}


VIDEO_START_MEMBER(apache3_state,apache3)
{
	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tatsumi_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_apache3_road_x_ram = std::make_unique<uint8_t[]>(512);

	m_tx_layer->set_transparent_pen(0);
}

uint32_t apache3_state::screen_update_apache3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_rotatingsprites->update_cluts();

	m_tx_layer->set_scrollx(0,24);

	bitmap.fill(m_palette->pen(0), cliprect);
	screen.priority().fill(0, cliprect);
	m_rotatingsprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_sky(bitmap, cliprect, 256, m_apache3_rotate_ctrl[1]);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), 0);
//  draw_ground(bitmap, cliprect);
	m_rotatingsprites->draw_sprites(bitmap,cliprect,0, (m_sprite_control_ram[0x20]&0x1000) ? 0x1000 : 0);
	m_tx_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/**********************************
 *
 * Round Up 5
 *
 *********************************/

uint8_t roundup5_state::gfxdata_r(offs_t offset)
{
	if((m_control_word & 0x200) == 0x200)
	{
		offset += (m_control_word & 0x6000) << 2;

		return m_bg_gfxram[offset];
	}

	offset+=((m_control_word&0x0c00)>>10) * 0x8000;
	return m_tx_gfxram[offset];
}

void roundup5_state::gfxdata_w(offs_t offset, uint8_t data)
{
	if((m_control_word & 0x200) == 0x200)
	{
		offset += (m_control_word & 0x6000) << 2;
		m_bg_gfxram[offset] = data;
		return;
	}

	offset+=((m_control_word&0x0c00)>>10) * 0x8000;

	if (offset>=0x18000 && data)
		logerror("effective write to vram %06x %02x (control %04x)\n",offset,data,m_control_word);

	m_tx_gfxram[offset] = data;

	offset=offset%0x8000;

	m_gfxdecode->gfx(0)->mark_dirty(offset/8);
}

VIDEO_START_MEMBER(roundup5_state,roundup5)
{
	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tatsumi_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 128,64);
	m_tx_gfxram = std::make_unique<uint8_t[]>(0x20000);
	m_bg_gfxram = std::make_unique<uint8_t[]>(0x20000);

	m_tx_layer->set_transparent_pen(0);

	m_gfxdecode->gfx(0)->set_source(m_tx_gfxram.get());

	save_pointer(NAME(m_tx_gfxram), 0x20000);
	save_pointer(NAME(m_bg_gfxram), 0x20000);
}

void roundup5_state::draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
/*
0xf980 0x0008 0x8c80 0x4a00 - road right to below, width unknown (32 pixels guess)
0xfa80 0x0008 0x8c80 0x4a00 - road right to below, width unknown (32 pixels guess)

0xfb80 0x0008 0x8c80 0x4a00 - road in middle of screen, width unknown (32 pixels guess)

0xfc80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xfd80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xfe80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0xff80 0x0008 0x8c80 0x4a00 - road width above to left, same width as above (ie, xpos - 32)
0x0001                      - road half/width above to left, (ie, xpos - 16)
0x0081                      - road width to left as usual (xpos-16 from above, or 32 from above2)

0xfb0b 0x210b 0x8cf5 0x0dea - blue & left & right, with  blue|----|----|----|----|blue
in this mode changing columns 2 & 3 have no apparent effect
0xfb0b 0x7b09 0x8cf5 0x0dea - as above, but scaled up - perhaps 18 pixels shifted (twice that overall size)
0xfb0b 0x6c07 0x8cf5 0x0dea - as above, but scaled up - perhaps 40 pixels shifted from above
0xfb0b 0xaa06 0x8cf5 0x0dea - as above, but scaled up - perhaps 16 pixels shifted from above
0xfb0b 0xb005 0x8cf5 0x0dea - as above, but scaled up - perhaps 38 pixels shifted from above

b21 diff is 1a6
97b         20f
76c         c2
6aa         fa
5b0


0x0000 0x0008 0xxxxx 0xxxx - line starting at 0 for 128 pixels - 1 to 1 with road pixel data
0xff00 0x0008 0xxxxx 0xxxx - line starting at 32 for 128 pixels - 1 to 1 with road pixel data
0xfe00 0x0008 0xxxxx 0xxxx - line starting at 64 for 128 pixels - 1 to 1 with road pixel data



at standard zoom (0x800)
shift of 0x100 moves 32 pixels
so shift of 8 is assumed to move 1 pixel

at double zoom (0x1000)
assume shift of 0x100 only moves 16 pixels
so

0x100 * 0x400 => 0x40
0x100 * step 0x800 = must come out at 0x20
0x100 * step 0x1000 = must come out at 0x10
0x100 * step 0x2000 = 0x5

pos is 11.5 fixed point

-0x580 = middle
-0x180
-0x080
0
0x80

*/
	const uint16_t *data = m_road_ctrl_ram;

	// Road layer enable (?)
	if ((m_vregs[0x1]&0x1)==0)
		return;

	// Road data bank select (double buffered)
	if (m_road_vregs[0]&0x10)
		data+=0x400;

	// Apply clipping: global screen + local road y offsets
	int y = 256 - ((m_vregs[0xa/2] >> 8) + m_road_yclip[0]);
	data+=y*4;

	int visible_line=0;

	for ( ; y<cliprect.max_y+1; y++)
	{
		// TODO: tunnels road drawing has a different format?
		// shift is always 0x88** while data[3] is a variable argument with bit 15 always on
		int shift=data[0];
		int shift2=data[2];
		int pal = 4; //(data[3]>>8)&0xf;
		int step = swapendian_int16(data[1]);
		int samplePos=0;
		uint16_t const *const linedata=m_road_pixel_ram;// + (0x100 * pal);
		int startPos=0, endPos=0;

		int palette_byte;//=m_road_color_ram[visible_line/8];

		/*
		    Each road line consists of up to two sets of 128 pixel data that can be positioned
		    on the x-axis and stretched/compressed on the x-axis.  Any screen pixels to the left
		    of the first set are drawn with pen 0 of the road pixel data.  Any screen pixels to the
		    right of the second set line are drawn with pen 127 of the road pixel data.

		    The road control data is laid out as follows (4 words per screen line, with 2 banks):

		    Word 0: Line shift for 1st set - 13.3 signed fixed point value.
		    Word 1: Line scale - 5.11 fixed point value.  So 0x800 is 1:1, 0x400 is 1:2, etc
		    Word 2: Line shift for 2nd set - 13.3 signed fixed point value.
		    Word 3: ?

		    The scale is shared between both pixel sets.  The 2nd set is only used when the road
		    forks into two between stages.  The 2nd line shift is an offset from the last pixel
		    of the 1st set.  The 2nd line shift uses a different palette bank.

2nd road uses upper palette - confirmed by water stage.
offset is from last pixel of first road segment?
//last pixel of first road is really colour from 2nd road line?

		*/

		palette_byte=m_road_color_ram[visible_line/8];
		pal = 4 + ((palette_byte>>(visible_line%8))&1);

		visible_line++;

		if (shift&0x8000)
			shift=-(0x10000 - shift);
		shift=-shift;

		if (step)
			startPos=((shift<<8) + 0x80 )/ step;

		int x;

		/* Fill in left of road segment */
		for (x=0; (x < startPos) && (x < cliprect.max_x+1); x++)
		{
			int col = linedata[0]&0xf;
			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col);
		}

		/* If startpos is negative, clip it and adjust the sampling position accordingly */
		if (startPos<0)
		{
			samplePos=step*(0-startPos);
			startPos=0;
		}
		else
		{
			samplePos=0;
		}

		/* Fill in main part of road, then right-hand side edge */
		for (x=startPos; x < (cliprect.max_x + 1) && ((samplePos>>11)<0x80); x++)
		{
			// look up colour
			int col = linedata[(samplePos>>11)&0x7f]&0xf;

			/* Clamp if we have reached the end of the pixel data */
			//if ((samplePos>>11) > 0x7f)
			//  col=linedata[0x7f]&0xf;

			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col);

			samplePos+=step;
		}

		/* Now work out how many pixels until start of 2nd segment */
		startPos=x;

		if (shift2&0x8000)
			shift2=-(0x10000 - shift2);
		shift2=-shift2;

		if (step)
			endPos=((shift2<<8) + 0x80) / step;
		else
			endPos=0;
		endPos-=128;
		endPos=startPos+endPos;

		/* Fill pixels */
		for (x=startPos; x < (cliprect.max_x+1) && (x < endPos); x++)
		{
			int col = linedata[0x80]&0xf;

			/* Clamp if we have reached the end of the pixel data */
			//if ((samplePos>>11) > 0x7f)
			//  col=linedata[0x7f]&0xf;

			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col + 32);
		}

		if (endPos<0)
		{
			// end of left intersection (taking right turn)
			samplePos=step*(0-startPos);
		}
		else if (endPos<x)
		{
			// start of right intersection
			samplePos=step*(x-endPos);
		}
		else
		{
			// end of right intersection (taking right turn)
			samplePos=0; // todo
		}

		for (/*x=endPos*/; x < cliprect.max_x+1; x++)
		{
			// look up colour
			int col = linedata[((samplePos>>11)&0x7f) + 0x200]&0xf;

			/* Clamp if we have reached the end of the pixel data */
			if ((samplePos>>11) > 0x7f)
				col=linedata[0x7f + 0x200]&0xf;

			bitmap.pix(y, x) = m_palette->pen(256 + pal*16 + col + 32);

			samplePos+=step;
		}
		data+=4;
	}
}

// background layer landscape for Round Up 5
// two bitmap layers, back layer is 512 x 128, the other one is 512 x 64
// it's safe to assume that three monitor version will have a different arrangement here ...
void roundup5_state::draw_landscape(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t type)
{
	// TODO: guess, assume back layer having less scroll increment than front for parallax scrolling.
	// also notice that m_vregs[8/2] >> 8 is identical to [0x0c/2], always?
	uint16_t x_base = type ? m_bg_scrollx[0] : m_vregs[0xc/2];
	// TODO: maybe [0xa/2] applies here as well?
	uint16_t y_base = m_bg_scrolly[0] & 0x1ff;
	uint16_t y_scroll = 0x180 - y_base;
	uint32_t base_offset;
	uint16_t color_base = type ? 0x100 : 0x110;
	int ysize = type ? 64 : 128;

	base_offset = 0x10000 + type * 0x8000;
	if(type)
		y_scroll += 64;

	//popmessage("%04x %04x %04x",m_vregs[8/2],m_vregs[0xc/2],m_bg_scrollx[0]);

	for(int y = 0; y < ysize; y++)
	{
		for(int x = 0; x < 512; x++)
		{
			int res_x = (x_base + x) & 0x1ff;
			uint32_t color = m_bg_gfxram[(res_x >> 1)+y*256+base_offset];

			if(res_x & 1)
				color >>= 4;

			color &= 0xf;

			if(cliprect.contains(x, y+y_scroll) && color)
				bitmap.pix(y+y_scroll, x) = m_palette->pen(color+color_base);
		}
	}
}

uint32_t roundup5_state::screen_update_roundup5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int tx_start_addr;

	tx_start_addr = (m_hd6445_reg[0xc] << 8) | (m_hd6445_reg[0xd]);
	tx_start_addr &= 0x3fff;

	m_rotatingsprites->update_cluts();

	m_tx_layer->set_scrollx(0,24);
	m_tx_layer->set_scrolly(0,(tx_start_addr >> 4) | m_hd6445_reg[0x1d]);

	bitmap.fill(m_palette->pen(384), cliprect); // todo
	screen.priority().fill(0, cliprect);
	m_rotatingsprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_landscape(bitmap,cliprect,0);
	draw_landscape(bitmap,cliprect,1);
	draw_road(bitmap,cliprect);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), 0);
	if(m_control_word & 0x80) // enabled on map screen after a play
	{
		m_tx_layer->draw(screen, bitmap, cliprect, 0,0);
		m_rotatingsprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Full pass
	}
	else
	{
		m_rotatingsprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Full pass
		m_tx_layer->draw(screen, bitmap, cliprect, 0,0);
	}
	return 0;
}

/**********************************
 *
 * Cycle Warriors / Big Fight
 *
 *********************************/

/*
 * these video registers never changes
 *
 * Big Fight
 * 72f2 5af2 3af2 22fa
 *
 * Cycle Warriors
 * 5673 92c2 3673 267b
 *
 * Following is complete guesswork (since nothing changes it's very hard to pinpoint what these bits do :/)
 * Layer order is 3-1-2-0 ?
 * x--- -x-- ---- ---- one of these might be enable page select
 * ---- ---- x--- ---- tilemap size
 * ---x ---- ---- x--- one these might be color bank
 *
 */
void cyclwarr_state::video_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_config[offset]);
}

// mixing control (seems to be available only for Big Fight and Cycle Warriors)
// --x- ---- enabled in Big Fight, disabled in Cycle Warriors (unknown purpose)
// ---- -x-- enable shadow mixing
// ---- ---x if 1 invert shadows, i.e. shadows are drawn with original pen while non shadows are halved (Chen stage in Big Fight)
void cyclwarr_state::mixing_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mixing_control);
}

template<int Bank>
TILE_GET_INFO_MEMBER(cyclwarr_state::get_tile_info_bigfight)
{
	int tile = m_cyclwarr_videoram[Bank >> 1][tile_index&0x7fff];
	int bank = (m_bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	uint16_t tileno = (tile&0x3ff)|(bank<<10);
	// color is bits 12-13
	uint8_t color = (tile >> 12) & 0x3;

	// all layers but 0 wants this palette bank (fade in/out effects)
	// a similar result is obtainable with priority bit, but then it's wrong for
	// Big Fight CRT test (dark red background) and character name bio in attract mode (reference shows it doesn't fade in like rest of text)
	// TODO: likely an HW config sets this up
	if(Bank != 0)
		color |= 4;
	// bit 14: ignore transparency on this tile
	int opaque = ((tile >> 14) & 1) == 1;

	tileinfo.set(0, tileno, color, opaque ? TILE_FORCE_LAYER0 : 0);

	// bit 15: tile appears in front of sprites
	tileinfo.category = (tile >> 15) & 1;
	tileinfo.mask_data = &m_mask[tileno<<3];
}

// same as above but additionally apply per-scanline color banking
// TODO: split for simplicity, need to merge with above
template<int Bank>
TILE_GET_INFO_MEMBER(cyclwarr_state::get_tile_info_cyclwarr_road)
{
	int tile = m_cyclwarr_videoram[Bank >> 1][tile_index&0x7fff];
	int bank = (m_bigfight_a40000[0] >> (((tile&0xc00)>>10)*4))&0xf;
	uint16_t tileno = (tile&0x3ff)|(bank<<10);
	uint8_t color = (tile >> 12) & 0x3;
//  if(Bank != 0)
	color |= 4;
	int opaque = ((tile >> 14) & 1) == 1;

	tileinfo.set(0, tileno, color | m_road_color_bank, opaque ? TILE_FORCE_LAYER0 : 0);

	tileinfo.category = (tile >> 15) & 1;
	tileinfo.mask_data = &m_mask[((tile&0x3ff)|(bank<<10))<<3];
}

void cyclwarr_state::tile_expand()
{
	/*
	    Each tile (0x4000 of them) has a lookup table in ROM to build an individual 3-bit palette
	    from sets of 8 bit palettes!
	*/
	gfx_element *gx0 = m_gfxdecode->gfx(0);
	m_mask.resize(gx0->elements() << 3,0);
	uint8_t *dest;

	// allocate memory for the assembled data
	m_decoded_gfx = std::make_unique<uint8_t[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	dest = m_decoded_gfx.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const uint8_t *c0base = gx0->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const uint8_t *c0 = c0base;

			for (int x = 0; x < gx0->width(); x++)
			{
				uint8_t pix = (*c0++ & 7);
				uint8_t respix = m_cyclwarr_tileclut[(c << 3)|pix];
				*dest++ = respix;
				// Transparent pixels are set by both the tile pixel data==0 AND colour palette & 7 == 0
				m_mask[(c << 3) | (y & 7)] |= ((pix&0x7)!=0 || ((pix&0x7)==0 && (respix&0x7)!=0)) ? (0x80 >> (x & 7)) : 0;
			}
			c0base += gx0->rowbytes();
		}
	}

	gx0->set_raw_layout(m_decoded_gfx.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(256);
}


VIDEO_START_MEMBER(cyclwarr_state,cyclwarr)
{
	tile_expand();
	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<0>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_cyclwarr_road<1>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<2>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);
	m_layer[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<3>)),      TILEMAP_SCAN_ROWS, 8,8,  64,512);

	// set up scroll bases
	// TODO: more HW configs
	m_layer[3]->set_scrolldx(-8,-8);
	m_layer_page_size[3] = 0x200;
	m_layer[2]->set_scrolldx(-8,-8);
	m_layer_page_size[2] = 0x200;
	m_layer[1]->set_scrolldx(-8,-8);
	m_layer_page_size[1] = 0x200;
	m_layer[0]->set_scrolldx(-0x10,-0x10);
	m_layer_page_size[0] = 0x100;

	m_layer1_can_be_road = true;
}

VIDEO_START_MEMBER(cyclwarr_state,bigfight)
{
	tile_expand();
	m_layer[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<0>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<1>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<2>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);
	m_layer[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cyclwarr_state::get_tile_info_bigfight<3>)), TILEMAP_SCAN_ROWS, 8,8, 128,256);

	// set up scroll bases
	// TODO: more HW configs
	m_layer[3]->set_scrolldx(-8,-8);
	m_layer[2]->set_scrolldx(-8,-8);
	m_layer[1]->set_scrolldx(-8,-8);
	m_layer[0]->set_scrolldx(-0x10,-0x10);
	for(int i=0;i<4;i++)
		m_layer_page_size[i] = 0x200;

	m_layer1_can_be_road = false;
}


void cyclwarr_state::draw_bg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *src, const uint16_t* scrollx, const uint16_t* scrolly, const uint16_t layer_page_size, bool is_road, int hi_priority)
{
	rectangle clip;
	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;
	// TODO: both always enabled when this occurs
	bool rowscroll_enable = (scrollx[0] & 0x1000) == 0;
	bool colscroll_enable = (scrollx[0] & 0x2000) == 0;
	// this controls wraparound (tilemap can't go above a threshold)
	// TODO: Actually scrolly registers 0xf0 to 0xff are used (can split the tilemap furthermore?)
	uint16_t page_select = scrolly[0xff];

	for (int y=cliprect.min_y; y<=cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;
		int y_base = rowscroll_enable ? y : 0;
		int x_base = colscroll_enable ? y : 0;
		int src_y = (scrolly[y_base] & 0x7ff);
		int src_x = (scrollx[x_base] & 0x7ff);
		// apparently if this is on disables wraparound target
		int page_disable = scrolly[y_base] & 0x800;
		int cur_page = src_y + y;

		// special handling for cycle warriors road: it reads in scrolly table bits 15-13 an
		// additional tile color bank and per scanline.
		if(is_road == true)
		{
			if(scrolly[y_base] & 0x8000)
			{
				m_road_color_bank = (scrolly[y_base] >> 13) & 3;
				// road mode disables page wraparound
				page_disable = 1;
			}
			else
				m_road_color_bank = 0;

			if(m_road_color_bank != m_prev_road_bank)
			{
				m_prev_road_bank = m_road_color_bank;
				src->mark_all_dirty();
			}
		}

		// apply wraparound, if enabled tilemaps can't go above a certain threshold
		// cfr. Cycle Warriors scrolling text (ranking, ending), backgrounds when uphill,
		// Big Fight vertical scrolling in the morning Funnel stage (not the one chosen at start),
		// also Big Fight text garbage in the stage after Mevella joins you (forgot the name)
		if((cur_page - page_select) >= layer_page_size && page_disable == 0)
			src_y -= layer_page_size;

		src->set_scrollx(0,src_x);
		src->set_scrolly(0,src_y);
		src->draw(screen, bitmap, clip, TILEMAP_DRAW_CATEGORY(hi_priority), 0);
	}
}

void cyclwarr_state::draw_bg_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int hi_priority)
{
	draw_bg(screen, bitmap, cliprect, m_layer[3], &m_cyclwarr_videoram[1][0x000], &m_cyclwarr_videoram[1][0x100], m_layer_page_size[3], false, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[2], &m_cyclwarr_videoram[1][0x200], &m_cyclwarr_videoram[1][0x300], m_layer_page_size[2],false, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[1], &m_cyclwarr_videoram[0][0x000], &m_cyclwarr_videoram[0][0x100], m_layer_page_size[1],m_layer1_can_be_road, hi_priority);
	draw_bg(screen, bitmap, cliprect, m_layer[0], &m_cyclwarr_videoram[0][0x200], &m_cyclwarr_videoram[0][0x300], m_layer_page_size[0], false, hi_priority);
}

uint32_t cyclwarr_state::screen_update_cyclwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bigfight_bank=m_bigfight_a40000[0];
	if (m_bigfight_bank!=m_bigfight_last_bank)
	{
		for (int i = 0; i < 4; i++)
		{
			m_layer[i]->mark_all_dirty();
		}
		m_bigfight_last_bank=m_bigfight_bank;
	}
	m_rotatingsprites->update_cluts();

	bitmap.fill(m_palette->pen(0), cliprect);

#if 0
	popmessage("%04x %04x (%04x)|%04x %04x (%04x)|%04x %04x (%04x)|%04x %04x (%04x)"
														,m_cyclwarr_videoram[1][0x000],m_cyclwarr_videoram[1][0x100],m_cyclwarr_videoram[1][0x1ff]
														,m_cyclwarr_videoram[1][0x200],m_cyclwarr_videoram[1][0x300],m_cyclwarr_videoram[1][0x3ff]
														,m_cyclwarr_videoram[0][0x000],m_cyclwarr_videoram[0][0x100],m_cyclwarr_videoram[0][0x1ff]
														,m_cyclwarr_videoram[0][0x200],m_cyclwarr_videoram[0][0x300],m_cyclwarr_videoram[0][0x3ff]);
#endif

//  popmessage("%04x %04x %04x %04x",m_video_config[0],m_video_config[1],m_video_config[2],m_video_config[3]);

	screen.priority().fill(0, cliprect);
	m_rotatingsprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_bg_layers(screen, bitmap, cliprect, 0);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), m_mixing_control & 1);
	m_rotatingsprites->draw_sprites(bitmap,cliprect,0,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0);
	draw_bg_layers(screen, bitmap, cliprect, 1);
	return 0;
}
