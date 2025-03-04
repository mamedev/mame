// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc88va.h"

#include <iostream>


#define LOG_IDP     (1U << 1) // TSP data
#define LOG_FB      (1U << 2) // framebuffer strips (verbose)
#define LOG_KANJI   (1U << 3) // empty Kanji data warnings
#define LOG_CRTC    (1U << 4)
#define LOG_COLOR   (1U << 5) // current color mode
#define LOG_TEXT    (1U << 6) // text strips (verbose)

#define VERBOSE (LOG_GENERAL | LOG_IDP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGIDP(...)       LOGMASKED(LOG_IDP, __VA_ARGS__)
#define LOGFB(...)        LOGMASKED(LOG_FB, __VA_ARGS__)
#define LOGKANJI(...)     LOGMASKED(LOG_KANJI, __VA_ARGS__)
#define LOGCRTC(...)      LOGMASKED(LOG_CRTC, __VA_ARGS__)
#define LOGCOLOR(...)     LOGMASKED(LOG_COLOR, __VA_ARGS__)
#define LOGTEXT(...)      LOGMASKED(LOG_TEXT, __VA_ARGS__)

void pc88va_state::video_start()
{
	const u32 gvram_size = 0x40000;
	m_gvram = std::make_unique<uint8_t[]>(gvram_size);
	std::fill_n(m_gvram.get(), gvram_size, 0);

	const u32 kanjiram_size = 0x4000;
	m_kanji_ram = std::make_unique<uint8_t[]>(kanjiram_size);
	m_gfxdecode->gfx(2)->set_source(m_kanji_ram.get());
	m_gfxdecode->gfx(3)->set_source(m_kanji_ram.get());
	m_vrtc_irq_line = 432;

	for (int i = 0; i < 2; i++)
		m_screen->register_screen_bitmap(m_graphic_bitmap[i]);

	save_item(NAME(m_screen_ctrl_reg));
	save_item(NAME(m_gfx_ctrl_reg));
	save_item(NAME(m_color_mode));
	save_item(NAME(m_pltm));
	save_item(NAME(m_pltp));

	save_item(NAME(m_text_transpen));
	save_pointer(NAME(m_video_pri_reg), 2);
	save_pointer(NAME(m_gvram), gvram_size);
	save_pointer(NAME(m_kanji_ram), kanjiram_size);

	save_item(NAME(m_vrtc_irq_line));
}

void pc88va_state::video_reset()
{
	m_text_transpen = 0;
}

void pc88va_state::palette_init(palette_device &palette) const
{
	// default palette
	const u16 default_palette[16] = {
		0x0000, 0x001f, 0x03e0, 0x03ff, 0xfc00, 0xfc1f, 0xffe0, 0xffff,
		0x7def, 0x0015, 0x02a0, 0x02b5, 0xac00, 0xac15, 0xaea0, 0xaeb5
	};
	int i, pal_base;
	for (i = 0; i < 16; i++)
	{
		const u16 color = default_palette[i];
		u8 b = pal5bit((color & 0x001f));
		u8 r = pal5bit((color & 0x03e0) >> 5);
		u8 g = pal6bit((color & 0xfc00) >> 10);
		for (pal_base = 0; pal_base < 2; pal_base ++)
			palette.set_pen_color(i + pal_base * 16, r, g, b);
	}
}

uint32_t pc88va_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t pri, cur_pri_lv;
	uint32_t screen_pri;
	bitmap.fill(0, cliprect);

	// don't bother if we are under DSPOFF command
	if(m_tsp.disp_on == false)
		return 0;

	// rollback to V1/V2 mode
	if (!BIT(m_pltm, 2))
	{
		// BIOS doesn't explicitly set a color mode for the V1/V2 Mode printout at POST
		// pc8801_state::screen_update(bitmap, cliprect);
		//return 0;
	}

	/*
	m_video_pri_reg[0]
	xxxx ---- ---- ---- priority 3
	---- xxxx ---- ---- priority 2
	---- ---- xxxx ---- priority 1
	---- ---- ---- xxxx priority 0
	m_video_pri_reg[1]
	---- ---- xxxx ---- priority 5
	---- ---- ---- xxxx priority 4

	Note that orthogonality level is actually REVERSED than the level number it indicates, so we have to play a little with the data for an easier usage ...
	*/

	// TODO: this should be calculated only in video_pri_w for cache
	screen_pri = (m_video_pri_reg[1] & 0x00f0) >> 4; // priority 5
	screen_pri|= (m_video_pri_reg[1] & 0x000f) << 4; // priority 4
	screen_pri|= (m_video_pri_reg[0] & 0xf000) >> 4; // priority 3
	screen_pri|= (m_video_pri_reg[0] & 0x0f00) << 4; // priority 2
	screen_pri|= (m_video_pri_reg[0] & 0x00f0) << 12; // priority 1
	screen_pri|= (m_video_pri_reg[0] & 0x000f) << 20; // priority 0

	//popmessage("%08x", screen_pri);

	for(pri = 0; pri < 6; pri++)
	{
		cur_pri_lv = (screen_pri >> (pri * 4)) & 0xf;

		if(cur_pri_lv & 8) // enable layer
		{
			// TODO: ballbrkr and tetris definitely wants this to have an higher priority
			if(pri <= 1) // (direct color mode, priority 5 and 4)
			{
				switch(cur_pri_lv & 3)
				{
					// 8 = graphic 0
					case 0:
						draw_graphic_layer(bitmap, cliprect, 0);
						break;
					// 9 = graphic 1
					case 1:
						draw_graphic_layer(bitmap, cliprect, 1);
						break;
					default:
						popmessage("Undocumented pri level %d", cur_pri_lv);
				}
			}
			else
			{
				switch(cur_pri_lv & 3) // (palette color mode)
				{
					case 0: draw_text(bitmap, cliprect); break;
					case 1: draw_sprites(bitmap,cliprect); break;
					/* A = graphic 0 */
					case 2:
						draw_graphic_layer(bitmap, cliprect, 0);
						break;
					/* B = graphic 1 */
					case 3:
						draw_graphic_layer(bitmap, cliprect, 1);
						break;
				}
			}
		}
	}

	return 0;
}

/****************************************
 * Drawing fns
 ***************************************/

inline u8 pc88va_state::get_layer_pal_bank(u8 which)
{
	if (!(BIT(m_pltm, 1)))
		return (m_pltm & 1) << 4;

	if (m_pltm == 3)
		return 0;

	// TODO: text and sprites may be joined when either one is selected in PLTP
	// olteus sets sprite = 1, wants text to follow with the other bank too
	// ballbrkr goes the other way around: writes 0, wants sprites to be 1
	return (m_pltp == which) << 4;
}

void pc88va_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// punt if we are under SPROFF
	if(m_tsp.spr_on == false)
		return;

	uint16_t const *const tvram = m_tvram;

	int offs = m_tsp.spr_offset;
	const u8 layer_pal_bank = get_layer_pal_bank(1);

	// top to bottom for priority (shanghai menuing)
	for(int i = 0x100 - 8; i >= 0; i -= 8)
	{
		int spr_count;

		int ysize = (tvram[(offs + i + 0) / 2] & 0xfc00) >> 10;
		int sw = (tvram[(offs + i + 0) / 2] & 0x200) >> 9;
		int yp = (tvram[(offs + i + 0) / 2] & 0x1ff);
		int xsize = (tvram[(offs + i + 2) / 2] & 0xf800) >> 11;
		int md = (tvram[(offs + i + 2) / 2] & 0x400) >> 10;
		int xp = (tvram[(offs + i + 2) / 2] & 0x3ff);
		int spda = (tvram[(offs + i + 4) / 2] & 0xffff);

		const bool is_cursor_sprite =
			i == m_tsp.curn
			&& m_tsp.curn_blink == true
			&& (m_screen->frame_number() % (m_tsp.blink * 2)) >= m_tsp.blink;

		// assume cursor sprite just going transparent with blink effect
		if(!sw || is_cursor_sprite)
			continue;

		// shanghai hand (4bpp) and rtype beam sparks (1bpp) wants this arrangement
		// Apparently TSP runs on its own translation unit for addresses
		if (spda & 0x8000)
			spda -= 0x4000;

		spda <<= 1;

		// TODO: verify this disabled code path
		// makes more sense without the sign?
		if(0)
		{
			// olteus wants this off
			if(yp & 0x100)
			{
				yp &= 0xff;
				yp = 0x100 - yp;
			}

			// TODO: shinraba needs wrap-around during gameplay (touching left edge of screen).
			if(xp & 0x200)
			{
				xp &= 0x1ff;
				xp = 0x200 - xp;
			}
		}

		if(md) // 1bpp mode
		{
			int fg_col = (tvram[(offs + i + 6) / 2] & 0xf0) >> 4;
			int bc = (tvram[(offs + i + 6) / 2] & 0x08) >> 3;

			// sprites don't draw if color is zero
			// (shanghai exit from menuing)
			if (!fg_col && !bc)
				continue;

			xsize = (xsize + 1) * 32;
			ysize = (ysize + 1) * 4;

			spr_count = 0;

			for(int y_i = 0; y_i < ysize; y_i++)
			{
				for(int x_i = 0; x_i < xsize; x_i+=16)
				{
					for(int x_s = 0; x_s < 16; x_s++)
					{
						int res_x = xp + x_i + x_s;
						// TODO: MG actually doubles Y size
						int res_y = (yp + y_i) << m_tsp.spr_mg;

						if (!cliprect.contains(res_x, res_y))
							continue;

						const u32 data_offset = ((spda + spr_count) & 0xffff) / 2;
						u8 pen = (bitswap<16>(tvram[data_offset],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8) >> (15 - x_s)) & 1;

						pen = pen & 1 ? fg_col : (bc) ? 8 : 0;

						if(pen != 0)
							bitmap.pix(res_y, res_x) = m_palette->pen(pen + layer_pal_bank);
					}

					spr_count += 2;
				}
			}
		}
		else
		{
			// 4bpp mode
			xsize = (xsize + 1) * 8;
			ysize = (ysize + 1) * 4;

			spr_count = 0;

			for(int y_i = 0; y_i < ysize; y_i++)
			{
				for(int x_i = 0; x_i < xsize; x_i += 4)
				{
					for(int x_s = 0; x_s < 4; x_s ++)
					{
						int res_x = xp + x_i + x_s;
						int res_y = (yp + y_i) << m_tsp.spr_mg;

						if (!cliprect.contains(res_x, res_y))
							continue;

						const u32 data_offset = ((spda + spr_count) & 0xffff) / 2;

						int pen = (bitswap<16>(tvram[data_offset],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8)) >> (12 - (x_s * 4)) & 0xf;

						//if (pen != 0 && pen != m_text_transpen)
						if (pen != 0)
							bitmap.pix(res_y, res_x) = m_palette->pen(pen + layer_pal_bank);
					}

					spr_count += 2;
				}
			}
		}
	}
}

// TODO: handcrafted kanji ROM causes this, should be simplified by a more accurate dump
// (or a better rearrange from driver_init, but that implies getting everything in place).
uint32_t pc88va_state::calc_kanji_rom_addr(uint8_t jis1, uint8_t jis2, int x, int y)
{
	// famista uses jis1 = 0x2c for the text box lines
	// xak2 also wants jis1 bit 3 arranged this way for 8x16 English menuing plus other stuff
	if(jis1 < 0x30)
		return ((jis2 & 0x60) << 8) + ((jis1 & 0x07) << 10) + ((jis2 & 0x1f) << 5) + ((jis1 & 0x8) << 15);

	if((jis1 & 0xf0) == 0x30)
		return ((jis2 & 0x60) << 10) + ((jis1 & 0x0f) << 10) + ((jis2 & 0x1f) << 5);

	if((jis1 & 0xf0) == 0x40)
		return 0x4000 + ((jis2 & 0x60) << 10) + ((jis1 & 0x0f) << 10) + ((jis2 & 0x1f) << 5);

	LOGKANJI("%d %d %02x %02x\n",x, y, jis1, jis2);

	return 0;
}

/*
 * Text is handled in strips at the location stored in TSP
 * [+00] Frame buffer start address (VSA)
 * [+02] ---- ---- ---- -xxx Upper VSA
 * [+04] ---- -xxx xxxx xxxx Frame buffer height (VH)
 * [+06] <reserved>
 * [+08] ---- --xx xxxx xxxx Frame buffer width (VW), in bytes
 * [+0a] xxxx ---- ---- ---- background color
 *       ---- xxxx ---- ---- foreground color
 *       ---- ---- ---x xxxx display mode
 * [+0c] ---x xxxx ---- ---- Raster address offset
 * [+0e] <reserved>
 * [+10] Split screen start address (RSA)
 * [+12] <reserved> / Upper RSA
 * [+14] Split screen height (RH)
 * [+16] Split screen width (RW)
 * [+18] Split screen vertical start position (RYP)
 * [+1a] Split screen horizontal start position (RXP)
 * [+1c] <reserved>
 * [+1e] <reserved>
 */
void pc88va_state::draw_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// punt if text disable is on
	if(m_td == true)
		return;

	uint16_t const *const tvram = m_tvram;
	uint8_t const *const kanji = memregion("kanji")->base();

	LOGTEXT("=== Start TEXT frame\n");

	// four layers
	for (int layer_n = 0; layer_n < 4; layer_n ++)
	{
		uint16_t const *const tsp_regs = &tvram[(layer_n * 0x20 + m_tsp.tvram_vreg_offset) / 2];

		const u32 vsa = tsp_regs[0 / 2]; // | (tsp_regs[2 / 2] & 7) << 16;
		const u32 rsa = tsp_regs[0x10 / 2]; // | (tsp_regs[0x12 / 2] & 7) << 16;

		// as-is it doesn't make much sense to enable either of these two,
		// since TVRAM is really 16-bit address lines here.
		// Since we are in IDP domain it may be used on another system ...
		if (tsp_regs[2 / 2] || tsp_regs[0x12 / 2])
			popmessage("Upper VSA enabled!");

		const u8 layer_pal_bank = get_layer_pal_bank(0);

		const u8 attr_mode = tsp_regs[0xa / 2] & 0x1f;
		// TODO: check this out, diverges with documentation
		const u8 screen_fg_col = (tsp_regs[0xa / 2] & 0xf000) >> 12;
		const u8 screen_bg_col = (tsp_regs[0xa / 2] & 0x0f00) >> 8;

		// TODO: how even vh/vw can run have all these bytes?
		const u8 vh = (tsp_regs[4 / 2] & 0x7ff);
		const u16 vw = (tsp_regs[8 / 2] & 0x3ff) / 2;


		if (vh == 0 || vw == 0)
		{
			LOGTEXT("\t%d skip VW = %d VH = %d\n"
				, layer_n
				, vw
				, vh
			);
			continue;
		}

		LOGTEXT("\t%d %08x VSA - %08x RSA| %02x DISPLAY MODE|%d VW x %d VH\n"
			, layer_n
			, vsa
			, rsa
			, attr_mode
			, vw
			, vh
		);

		const u16 rh = tsp_regs[0x14 /2];
		const u16 rw = tsp_regs[0x16 /2];
		const u16 ryp = tsp_regs[0x18 / 2];
		const u16 rxp = tsp_regs[0x1a / 2];

		const int raster_offset = (tsp_regs[0x0c / 2] >> 8) & 0x1f;

		LOGTEXT("\t%d RXP x %d RYP|%d RW x %d RH %d\n", rxp, ryp, rw, rh, raster_offset);

		rectangle split_cliprect(rxp, rxp + rw - 1, ryp, ryp + rh - 1);
		split_cliprect &= cliprect;

		for(int y = 0; y < vh; y++)
		{
			int y_base = y * 16 + ryp - raster_offset;

			// TODO: consult with OG
			if (!split_cliprect.contains(rxp, y_base) &&
				!split_cliprect.contains(rxp, y_base + 16))
				continue;

			for(int x = 0; x < vw; x++)
			{
				int x_base = x * 8;
				if (!split_cliprect.contains(x_base, y_base) &&
					!split_cliprect.contains(x_base, y_base + 16))
					continue;

				// TODO: understand where VSA comes into equation
				const u32 cur_offset = ((rsa >> 1) + x + (y * vw));

				uint16_t attr = (tvram[cur_offset + (m_tsp.attr_offset / 2)] & 0x00ff);

				uint8_t fg_col, bg_col, secret, reverse;
				//uint8_t blink,dwidc,dwid,uline,hline;
				fg_col = bg_col = reverse = secret = 0; //blink = dwidc = dwid = uline = hline = 0;

				// TODO: convert to functional programming
				switch(attr_mode)
				{
					/*
					xxxx ---- foreground color
					---- xxxx background color
					*/
					case 0:
						fg_col = (attr & 0x0f) >> 0;
						bg_col = (attr & 0xf0) >> 4;
						break;
					/*
					xxxx ---- foreground color
					---- x--- horizontal line
					---- -x-- reverse
					---- --x- blink
					---- ---x secret (hide text)
					background color is defined by screen control table values
					*/
					case 1:
						fg_col = (attr & 0xf0) >> 4;
						bg_col = screen_bg_col;
						//hline = (attr & 0x08) >> 3;
						reverse = (attr & 0x04) >> 2;
						//blink = (attr & 0x02) >> 1;
						secret = (attr & 0x01) >> 0;
						break;
					/*
					x--- ---- dwidc
					-x-- ---- dwid
					--x- ---- uline
					---x ---- hline
					---- -x-- reverse
					---- --x- blink
					---- ---x secret (hide text)
					background and foreground colors are defined by screen control table values
					*/
					case 2:
						fg_col = screen_fg_col;
						bg_col = screen_bg_col;
						//dwidc = (attr & 0x80) >> 7;
						//dwid = (attr & 0x40) >> 6;
						//uline = (attr & 0x20) >> 5;
						//hline = (attr & 0x10) >> 4;
						reverse = (attr & 0x04) >> 2;
						//blink = (attr & 0x02) >> 1;
						secret = (attr & 0x01) >> 0;
						break;
					/*
					---- x--- mixes between mode 0 and 2

					xxxx 1--- foreground color
					---- 1xxx background color
					2)
					x--- 0--- dwidc
					-x-- 0--- dwid
					--x- 0--- uline
					---x 0--- hline
					---- 0x-- reverse
					---- 0-x- blink
					---- 0--x secret (hide text)
					background and foreground colors are defined by screen control table values
					*/
					case 3:
						{
							// TODO: similar to 3301 drawing (where it should save previous attribute setup)
							if(attr & 0x8)
							{
								fg_col = (attr & 0xf0) >> 4;
								bg_col = (attr & 0x07) >> 0;
							}
							else
							{
								fg_col = screen_fg_col;
								bg_col = screen_bg_col;
								//dwidc = (attr & 0x80) >> 7;
								//dwid = (attr & 0x40) >> 6;
								//uline = (attr & 0x20) >> 5;
								//hline = (attr & 0x10) >> 4;
								reverse = (attr & 0x04) >> 2;
								//blink = (attr & 0x02) >> 1;
								secret = (attr & 0x01) >> 0;
							}
						}
						break;
					/*
					x--- ---- blink
					-xxx ---- background color
					---- xxxx foreground color
					*/
					case 4:
						fg_col = (attr & 0x0f) >> 0;
						bg_col = (attr & 0x70) >> 4;
						//blink = (attr & 0x80) >> 7;
						break;
					/*
					x--- ---- blink
					-xxx ---- background color
					---- xxxx foreground color
					hline is enabled if foreground color is 1 or 9
					*/
					case 5:
						fg_col = (attr & 0x0f) >> 0;
						bg_col = (attr & 0x70) >> 4;
						//blink = (attr & 0x80) >> 7;
						//if((fg_col & 7) == 1)
							//hline = 1;
						break;
					default:
						popmessage("Illegal text tilemap attribute mode %02x",attr_mode);
						return;
				}

				// TODO: more functional programming
				if ((tvram[cur_offset] & 0xff00) == 0) // ANK
				{
					u32 tile_num = ((tvram[cur_offset] & 0xff)  * 16) | 0x40000;

					for(int yi = 0; yi < 16; yi++)
					{
						for(int xi = 0; xi < 8; xi++)
						{
							int res_x = x_base + xi;
							int res_y = y_base + yi;

							if(!split_cliprect.contains(res_x, res_y))
								continue;

							int pen = kanji[yi + tile_num] >> (7-xi) & 1;

							if(reverse)
								pen = pen & 1 ? bg_col : fg_col;
							else
								pen = pen & 1 ? fg_col : bg_col;

							if(secret) { pen = 0; } //hide text

							if(pen != 0 && pen != m_text_transpen)
								bitmap.pix(res_y, res_x) = m_palette->pen(pen + layer_pal_bank);
						}
					}
				}
				else if ((tvram[cur_offset] & 0x00ff) == 0x56)
				{
					// famista draws cursor/team letters/referee calls with PCG
					// boomer draws HP bar on gameplay
					// both shifts bits 5-6 by 1 for upper tiles to be properly accessed,
					// cfr. gfxdecoding where there's an hole every 0x20 chars
					// (no way to access these?)
					const u8 base_tile = (tvram[cur_offset] & 0xff00) >> 8;
					u32 tile_num = ((base_tile & 0x1f) + ((base_tile & 0x60) << 1)) * 0x20;
					u8 lr_half_gfx = BIT(base_tile, 7);

					for(int yi = 0; yi < 16; yi++)
					{
						for(int xi = 0; xi < 8; xi++)
						{
							int res_x = x_base + xi;
							int res_y = y_base + yi;

							if(!split_cliprect.contains(res_x, res_y))
								continue;

							int pen = m_kanji_ram[(( yi * 2 ) + lr_half_gfx) + tile_num] >> (7 - xi) & 1;

							if(reverse)
								pen = pen & 1 ? bg_col : fg_col;
							else
								pen = pen & 1 ? fg_col : bg_col;

							if(secret) { pen = 0; } //hide text

							if(pen != 0 && pen != m_text_transpen)
								bitmap.pix(res_y, res_x) = m_palette->pen(pen + layer_pal_bank);
						}
					}
				}
				else // kanji
				{
					uint8_t jis1 = (tvram[cur_offset] & 0x7f) + 0x20;
					uint8_t jis2 = (tvram[cur_offset] & 0x7f00) >> 8;
					uint16_t lr_half_gfx = ((tvram[cur_offset] & 0x8000) >> 15);

					uint32_t tile_num = calc_kanji_rom_addr(jis1, jis2, x, y);

					for(int yi = 0; yi < 16; yi++)
					{
						for(int xi = 0; xi < 8; xi++)
						{
							int res_x = x_base + xi;
							int res_y = y_base + yi;

							if(!split_cliprect.contains(res_x, res_y))
								continue;

							int pen = kanji[((yi*2)+lr_half_gfx)+tile_num] >> (7-xi) & 1;

							if(reverse)
								pen = pen & 1 ? bg_col : fg_col;
							else
								pen = pen & 1 ? fg_col : bg_col;

							if(secret) { pen = 0; } //hide text

							if(pen != 0)
								bitmap.pix(res_y, res_x) = m_palette->pen(pen + layer_pal_bank);
						}
					}
				}
			}
		}
	}
}

/*
 * as above, graphics are stored in 4 strips in I/O $200-$27f
 * [+00] xxxx xxxx xxxx xx-- frame buffer start address (FSA), 4 byte boundary
 * [+02] ---- ---- ---- --xx FSA upper bit address
 *                           \- fixed at 0x20000 for frame buffer 1
 * [+04] ---- -xxx xxxx xx-- frame buffer width (FBW), 4 byte boundary
 * [+06] ---- --xx xxxx xxxx frame buffer height (FBL) - 1
 *                           \- cannot be set for frame buffer 1 (?)
 * [+08] ---- ---- ---x xxxx X dot offset (fractional shift)
 *                           \- changes depending on single/multiplane mode
 * [+0a] ---- -xxx xxxx xx-- X scroll offset (OFX), 4 byte boundary
 * [+0c] ---- --xx xxxx xxxx Y scroll offset (OFY)
 * [+0e] xxxx xxxx xxxx xx-- Display start address (DSA)
 * [+10] ---- ---- ---- --xx DSA upper bit address
 * [+12] ---- ---x xxxx xxxx Sub screen height (DSH)
 * [+14] <reserved>
 * [+16] ---- ---x xxxx xxxx Sub screen display position (DSP)
 */

void pc88va_state::draw_graphic_layer(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 which)
{
	// disable graphic B if screen 0 only setting is enabled
	if (which && !m_ymmd)
		return;

	// ditto for 5bpp color mode
	const bool is_5bpp = m_pltm == 7;
	if (is_5bpp && which == 1)
		return;

	uint16_t const *const fb_regs = m_fb_regs;

	const u8 gfx_ctrl = (m_gfx_ctrl_reg >> (which * 8)) & 0x13;

	// TODO: xak2 wants independent doubled Y axis on setup menu & Micro Cabin logo
	// i.e. 200 lines draw on a 400 lines canvas
	const u32 pixel_size = 0x10000 >> BIT(gfx_ctrl, 4);

	const u8 layer_pal_bank = get_layer_pal_bank(2 + which);

	LOGFB("=== %02x GFX MODE graphic %s color bank %02x\n"
		, gfx_ctrl
		, which ? "B" : "A"
		, layer_pal_bank
	);

	m_graphic_bitmap[which].fill(0, cliprect);

	const int layer_inc = (!is_5bpp) + 1;
	const int layer_fixed = is_5bpp + 1;

	for (int layer_n = which; layer_n < 4; layer_n += layer_inc)
	{
		uint16_t const *const fb_strip_regs = &fb_regs[(layer_n * 0x20) / 2];
		const u16 fbw = fb_strip_regs[0x04 / 2] & 0x7fc;

		// assume that a fbw = 0 don't generate a layer at all
		if (fbw == 0)
		{
			LOGFB("%d FBW = 0\n", layer_n);
			continue;
		}

		// on layer = 1 fsa is always 0x20000, cfr. shanghai
		// (almost likely an HW quirk, described in the docs)
		// also animefrm swaps this with layer 2 (main canvas)
		const u32 fsa = (layer_n == layer_fixed) ? 0x20000
			: (fb_strip_regs[0x00 / 2] & 0xfffc) | ((fb_strip_regs[0x02 / 2] & 0x3) << 16) >> 1;

		const u16 fbl = (fb_strip_regs[0x06 / 2] & 0x3ff) + 1;
		const u8 x_dot_offs = fb_strip_regs[0x08 / 2];
		const u16 ofx = fb_strip_regs[0x0a / 2] & 0x7fc;
		const u16 ofy = fb_strip_regs[0x0c / 2] & 0x3ff;
		const u32 dsa = ((fb_strip_regs[0x0e / 2] & 0xfffc) | ((fb_strip_regs[0x10 / 2] & 0x3) << 16));
		const u16 dsh = fb_strip_regs[0x12 / 2] & 0x1ff;
		const u16 dsp = fb_strip_regs[0x16 / 2] & 0x1ff;

		LOGFB("%d %08x FSA|\n\t%d FBW | %d FBL |\n\t %d OFX (%d dot)| %d OFY|\n\t %08x DSA|\n\t %04x (%d) DSH | %04x (%d) DSP\n"
			, layer_n
			, fsa << 1
			, fbw
			, fbl
			, ofx
			, x_dot_offs
			, ofy
			, dsa
			, dsh
			, dsh
			, dsp
			, dsp
		);

		rectangle split_cliprect(cliprect.min_x, cliprect.max_x, dsp, dsh + dsp - 1);
		split_cliprect &= cliprect;

		// animefrm uses layer 1 with fbl = 1 for the color cycling bar on top
		// rtype definitely don't want to be clipped on fbw, assume valid for pitch calc only.
		rectangle fb_cliprect(cliprect.min_x, cliprect.max_x, dsp, dsp + fbl - 1);
		split_cliprect &= fb_cliprect;

		if (!m_dm)
		{
			switch(gfx_ctrl & 3)
			{
				case 1: draw_packed_gfx_4bpp(m_graphic_bitmap[which], split_cliprect, fsa, dsa, layer_pal_bank, fbw, fbl); break;
			}
		}
		else
		{
			switch(gfx_ctrl & 3)
			{
				//case 0: draw_indexed_gfx_1bpp(bitmap, cliprect, dsa, layer_pal_bank); break;
				case 1: draw_indexed_gfx_4bpp(m_graphic_bitmap[which], split_cliprect, fsa, dsa, layer_pal_bank, fbw, fbl); break;
				case 2:
					if (m_pltm == 7)
					{
						draw_packed_gfx_5bpp(m_graphic_bitmap[which], split_cliprect, fsa, dsa, layer_pal_bank, fbw, fbl);
					}
					else
						draw_direct_gfx_8bpp(m_graphic_bitmap[which], split_cliprect, fsa, fbw, fbl);
					break;
				case 3: draw_direct_gfx_rgb565(m_graphic_bitmap[which], split_cliprect, fsa, fbw, fbl); break;
			}
		}
	}

	// TODO: we eventually need primask_copyrozbitmap_trans here, or a custom copy, depending on what the "transpen" registers really do.
	copyrozbitmap_trans(
		bitmap, cliprect, m_graphic_bitmap[which],
		0, 0,
		pixel_size, 0, 0, pixel_size,
		false, 0
	);
}

void pc88va_state::draw_indexed_gfx_1bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u8 pal_base)
{
	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 line_offset = (((y * 640) / 8) + fb_start_offset) & 0x3ffff;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
		{
			u16 x_char = (x >> 3);
			u32 bitmap_offset = line_offset + x_char;

			for (int xi = 0; xi < 8; xi ++)
			{
				uint32_t color = (m_gvram[bitmap_offset] >> (7 - xi)) & 1;
				int res_x = x + xi;

				if(color && cliprect.contains(res_x, y))
					bitmap.pix(y, res_x) = m_palette->pen(color + pal_base);
			}
		}
	}
}

void pc88va_state::draw_indexed_gfx_4bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u32 display_start_offset, u8 pal_base, u16 fb_width, u16 fb_height)
{
//  const u16 y_min = std::max(cliprect.min_y, y_start);
//  const u16 y_max = std::min(cliprect.max_y, y_min + fb_height);

	//printf("%d %d %d %08x %d\n", y_min, y_max, fb_width, start_offset, fb_height);

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 line_offset = ((y * fb_width) + fb_start_offset) & 0x3ffff;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			u16 x_char = (x >> 1);
			u32 bitmap_offset = line_offset + x_char;

			for (int xi = 0; xi < 2; xi ++)
			{
				u8 color = (m_gvram[bitmap_offset] >> (xi ? 0 : 4)) & 0xf;

				if(color && cliprect.contains(x + xi, y))
					bitmap.pix(y, x + xi) = m_palette->pen(color + pal_base);
			}
		}
	}
}

void pc88va_state::draw_packed_gfx_5bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u32 display_start_offset, u8 pal_base, u16 fb_width, u16 fb_height)
{
//  const u16 y_min = std::max(cliprect.min_y, y_start);
//  const u16 y_max = std::min(cliprect.max_y, y_min + fb_height);

	//printf("%d %d %d %08x %d\n", y_min, y_max, fb_width, start_offset, fb_height);

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 line_offset = ((y * fb_width) + fb_start_offset) & 0x3ffff;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u32 bitmap_offset = line_offset + x;

			u8 color = m_gvram[bitmap_offset] & 0x1f;

			if(color && cliprect.contains(x, y))
				bitmap.pix(y, x) = m_palette->pen(color);
		}
	}
}

void pc88va_state::draw_direct_gfx_8bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u16 fb_width, u16 fb_height)
{
//  const u16 y_min = std::max(cliprect.min_y, y_start);
//  const u16 y_max = std::min(cliprect.max_y, y_min + fb_height);

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 line_offset = ((y * fb_width) + fb_start_offset) & 0x3ffff;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u32 bitmap_offset = line_offset + x;

			uint32_t color = (m_gvram[bitmap_offset] & 0xff);

			// boomer suggests that transparency is calculated over just color = 0, may be settable?
			// TODO: may not be clamped to palNbit
			if(color && cliprect.contains(x, y))
			{
				u8 b = pal2bit(color & 0x03);
				u8 r = pal3bit((color & 0x1c) >> 2);
				u8 g = pal3bit((color & 0xe0) >> 5);
				bitmap.pix(y, x) = (b) | (g << 8) | (r << 16);
			}
		}
	}
}

void pc88va_state::draw_direct_gfx_rgb565(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u16 fb_width, u16 fb_height)
{
//  const u16 y_min = std::max(cliprect.min_y, y_start);
//  const u16 y_max = std::min(cliprect.max_y, y_min + fb_height);

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 line_offset = ((y * fb_width) + fb_start_offset) & 0x3ffff;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u32 bitmap_offset = (line_offset + x) << 1;

			uint16_t color = (m_gvram[bitmap_offset] & 0xff) | (m_gvram[bitmap_offset + 1] << 8);

			if(cliprect.contains(x, y))
			{
				u8 b = pal5bit((color & 0x001f));
				u8 r = pal5bit((color & 0x03e0) >> 5);
				u8 g = pal6bit((color & 0xfc00) >> 10);
				bitmap.pix(y, x) = (b) | (g << 8) | (r << 16);
			}
		}
	}
}

void pc88va_state::draw_packed_gfx_4bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 fb_start_offset, u32 display_start_offset, u8 pal_base, u16 fb_width, u16 fb_height)
{
//  const u16 y_min = std::max(cliprect.min_y, y_start);
//  const u16 y_max = std::min(cliprect.max_y, y_min + fb_height);

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const u32 line_offset = ((y * (fb_width >> 2)) + fb_start_offset) & 0x0ffff;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
		{
			u16 x_char = (x >> 3);
			u32 bitmap_offset = line_offset + x_char;

			for (int xi = 0; xi < 8; xi ++)
			{
				u8 color = 0;
				for (int bank_num = 0; bank_num < 4; bank_num ++)
					color |= ((m_gvram[bitmap_offset + bank_num * 0x10000] >> (7 - xi)) & 1) << bank_num;

				if(color && cliprect.contains(x + xi, y))
					bitmap.pix(y, x + xi) = m_palette->pen(color + pal_base);
			}
		}
	}
}

/****************************************
 * IDP - NEC μPD72022
 * "Intelligent Display Processor"
 ***************************************/

/*
 * x--- ---- LP Light-Pen signal detection
 * -x-- ---- VB Vertical Blank
 * --x- ---- SC Sprite control (sprite over/collision)
 * ---x ---- ER IDP Error
 *           \- set when required parameters aren't entered or are malformed,
 *              or mishandling of command/parameter ports. IDP won't accept any
 *              further commands and requires an EXIT to be issued to resume operations.
 * ---- x--- In the midst of execution of EMEN emulation development
 *           \- undocumented, PC88VA specific?
 * ---- -x-- BUSY set high when IDP is processing commands,
 *                clear when all commands in FIFO have been completed
 * ---- --x- OBF output data buffer full
 *           \- output FIFO from IDP to host (for commands that implies reading to param port),
 *              reset on host reading, causes ready pin low when flag is (un?)set.
 * ---- ---x IBF input data buffer full
 *           \- command/parameter FIFO, high when IDP cannot accept any more from host,
 *              causes ready pin low if any further param write is written (host stalls?).
 */
uint8_t pc88va_state::idp_status_r()
{
	u8 data = 0;

	data |= (m_screen->vblank()) ? 0x40 : 0x00;

	return data;
}


#define SYNC   0x10
#define DSPON  0x12
#define DSPOFF 0x13
#define DSPDEF 0x14
#define CURDEF 0x15
#define ACTSCR 0x16
#define CURS   0x1e
#define EMUL   0x8c
#define EXIT   0x88
#define SPRON  0x82
#define SPROFF 0x83
#define SPRSW  0x85
#define SPROV  0x81
#define SPWR   0x84

void pc88va_state::idp_command_w(uint8_t data)
{
	switch(data)
	{
		/* 0x10 - SYNC: sets CRTC values */
		case SYNC:   m_cmd = SYNC;  m_buf_size = 14; m_buf_index = 0; break;

		/* 0x12 - DSPON: set DiSPlay ON and set up tvram table vreg */
		case DSPON:  m_cmd = DSPON; m_buf_size = 3;  m_buf_index = 0; break;

		/* 0x13 - DSPOFF: set DiSPlay OFF */
		case DSPOFF: m_cmd = DSPOFF; m_tsp.disp_on = false; break;

		/* 0x14 - DSPDEF: set DiSPlay DEFinitions */
		case DSPDEF: m_cmd = DSPDEF; m_buf_size = 6; m_buf_index = 0; break;

		/* 0x15 - CURDEF: set CURsor DEFinition */
		case CURDEF: m_cmd = CURDEF; m_buf_size = 1; m_buf_index = 0; break;

		/* 0x16 - ACTSCR: set ACTive SCReen for CURSor command */
		case ACTSCR: m_cmd = ACTSCR; m_buf_size = 1; m_buf_index = 0; break;

		/* 0x15 - CURS: set CURSor position */
		case CURS:   m_cmd = CURS;   m_buf_size = 4; m_buf_index = 0; break;

		/* 0x8c - EMUL: set 3301 EMULation (PC88VA specific, undocumented in 72022 specs) */
		case EMUL:   m_cmd = EMUL;   m_buf_size = 4; m_buf_index = 0; break;

		/* 0x88 - EXIT: aborts current command attribute selection, or clear an ERror state */
		case EXIT:   m_cmd = EXIT; break;

		/* 0x82 - SPRON: set SPRite ON */
		case SPRON:  m_cmd = SPRON;  m_buf_size = 3; m_buf_index = 0; break;

		/* 0x83 - SPROFF: set SPRite OFF */
		case SPROFF: m_cmd = SPROFF; m_tsp.spr_on = false; break;

		/* 0x85 - SPRSW: flip SPRite SW bit */
		case SPRSW:  m_cmd = SPRSW;  m_buf_size = 1; m_buf_index = 0; break;

		/* 0x81 - SPROV: set SPRite OVerflow information */
		/*
		-x-- ---- Sprite Over flag
		--x- ---- Sprite Collision flag
		---x xxxx First sprite that caused Sprite Over event
		*/
		// TODO: data is readback in $146
		case SPROV:  m_cmd = SPROV; break;

		/* 0x84 - SPRW: SPRite Write */
		case SPWR:   m_cmd = SPWR; m_tsp.spwr_define = true; break;

		// TODO: 0x80 - SPRRD (same as SPWR but on read)
		// TODO: 0x1a - LPNR (returns current light pen latch)
		// TODO: 0x89 - MASK (ANDs successive VRAM writes for block write commands)
		// TODO: 0x8e/0x8f - DPLD (set internal DPTR0/DPTR1 address variables)
		// TODO: 0x8a - DPRD (get internal DPTR0 address)
		// TODO: 0x90/0x91/0x92/0x93 - RDAT (Read DATa)
		// TODO: 0x94/0x95/0x96/0x97 - WDAT (Write DATa)
		// TODO: 0x99/0x9a/0x9b - BLKTOT (DMA block output)
		// TODO: 0x9d/0x9e/0x9f - BLKTIN (DMA block input)

		default:
			m_cmd = 0x00;
			LOG("Unemulated IDP %02x cmd set\n", data);
			break;
	}
}

void pc88va_state::tsp_sprite_enable(u32 sprite_number, bool sprite_enable, bool blink_enable)
{
	const u32 target_offset = (m_tsp.spr_offset + sprite_number) / 2;

	m_tvram[target_offset] = (m_tvram[target_offset] & ~0x200) | (sprite_enable << 9);
	// disable blink if an override of CURN occurs
	if (m_tsp.spr_offset == m_tsp.curn)
		m_tsp.curn_blink = blink_enable;
}

/*
 * IDP SYNC command
 * xx-- ---- [0] RM raster mode
 * 00-- ----     \- non-interlace 640 x 400 24kHz
 * 01-- ----     \- interlace 640 x 400 15kHz
 * 10-- ----     \- vertical magnify 640 x 200 24kHz
 * 11-- ----     \- normal 640 x 200 15kHz
 *               \- PC88VA POST sets either 00 or 11, depending on the monitor setting
 * --x- ---- [0] EL Enable Light Pen IRQ
 * ---x ---- [0] EV Enable Vertical Blank IRQ
 *               \- Generated to INT pin line, assume same as uPD3301 where VRTC != INT
 *                  (therefore unconnected in PC88VA)
 * ---- x--- [0] VM VRAM Access Mode (0) Random access mode (1) serial access mode
 * ---- --xx [0] ILM, DPM Interleave and Dual Port Modes
 * ---- --00     \- standalone mode
 * ---- --01     \- interleave mode
 * ---- --10     \- dual-port mode (VM must be 1)
 * ---- --11     \- disabled
 *               \- Related to comms between host, IDP and VRAM, PC88VA POST sets 01.
 * -x-- ---- [1] RF video memory refresh (0) no refresh (1) refresh
 *               \- enabled by PC88VA POST
 * --x- ---- [1] EC external clock (1) output to DTCLK pin
 * ---x ---- [1] ES external sync (1) output sync signals to /HSYN and /VSYN pins
 *               \- enabled by PC88VA POST
 * ---- x--- [1] RV reverse screen (1) reverse color in text display
 *               \- sounds similar to the correlated function in uPD3301
 * ---- -xxx [1] RS Resolution Select, divides dot clock
 * ---- -000     \- divide by 4
 * ---- -001     \- divide by 3
 * ---- -010     \- divide by 2
 * ---- -011     \- divide by 1.5
 * ---- -100     \- divide by 1
 * ---- -1xx     \- disabled, no dot clock output
 *               \- NB: PC88VA POST sets 7
 * --xx xxxx [2] - LBL h blank start
 * --xx xxxx [3] - LBR h border start
 * xxxx xxxx [4] - HAD (h visible area - 1) / 4
 * --xx xxxx [5] - RBR h border end
 * --xx xxxx [6] - RBL h blank end
 * --xx xxxx [7] - HS h sync
 *               \- assume all params to be val - 1 / 4
 * --xx xxxx [8] - TBL v blank start
 * --xx xxxx [9] - TBR v border start
 * xxxx xxxx [A] - VAD (L) v visible area
 * -x-- ---- [B] - VAD (H) v visible area (bit 9)
 * --xx xxxx [B] - BBR v border end
 * --xx xxxx [C] - BBL v blank end
 * --xx xxxx [D] - VS v sync
 */
void pc88va_state::execute_sync_cmd()
{
	// olteus will punt loading on PC Engine OS if the vblank bit is completely off
	// illcity expects the actual IDP vblank bit to work, from setup menu to opening transition PC=0x418f6
	// upo wants precise vblank bit readouts plus something else (SGP irq?)

	rectangle visarea;
	attoseconds_t refresh;

	LOGCRTC("IDP SYNC: ");

	for (int i = 0; i < 15; i++)
		LOGCRTC("%02x ", m_buf_ram[i]);

	const u8 h_blank_start = (m_buf_ram[0x02] & 0x3f) + 1;
	const u8 h_border_start = (m_buf_ram[0x03] & 0x3f) + 1;
	const u16 h_vis_area = (m_buf_ram[0x04] + 1) * 4;
	const u8 h_border_end = (m_buf_ram[0x05] & 0x3f) + 1;
	const u8 h_blank_end = (m_buf_ram[0x06] & 0x3f) + 1;
	const u8 h_sync = (m_buf_ram[0x07] & 0x3f) + 1;

	LOGCRTC("\n\t");
	LOGCRTC("H blank start %d - end %d|", h_blank_start, h_blank_end);
	LOGCRTC("H visible area: %d|", h_vis_area);
	LOGCRTC("H border start %d - end %d|", h_border_start, h_border_end);
	LOGCRTC("H sync: %d", h_sync);

	LOGCRTC("\n\t");
	const u16 h_total =
		(h_blank_start + h_blank_end + h_border_start + h_border_end + h_sync) * 4 + h_vis_area;

	LOGCRTC("H Total calc = %d", h_total);
	LOGCRTC("\n\t");

	const u8 v_blank_start = m_buf_ram[0x08] & 0x3f;
	const u8 v_border_start = m_buf_ram[0x09] & 0x3f;
	const u16 v_vis_area = (m_buf_ram[0x0a]) | ((m_buf_ram[0x0b] & 0x40) << 2);
	const u8 v_border_end = m_buf_ram[0x0b] & 0x3f;
	const u8 v_blank_end = m_buf_ram[0x0c] & 0x3f;
	const u8 v_sync = (m_buf_ram[0x0d] & 0x3f);

	LOGCRTC("V blank start %d - end %d|", v_blank_start,  v_blank_end);
	LOGCRTC("V visible area: %d|", v_vis_area);
	LOGCRTC("V border start: %d - end %d|", v_border_start,  v_border_end);
	LOGCRTC("V sync: %d", v_sync);

	LOGCRTC("\n\t");
	m_vrtc_irq_line = v_blank_start + v_blank_end + v_vis_area + v_border_start + v_border_end;
	const u16 v_total = m_vrtc_irq_line + v_sync;

	LOGCRTC("V Total calc = %d (VRTC %d)\n", v_total, m_vrtc_irq_line);

	// punt with message if values are off (shouldn't happen)
	// TODO: more validation:
	// HS >= 4
	// LBL >= 3
	// HAD & 1 == 1
	// VS >= 4
	// TBL + TBR >= 16 for non-sprite display, >= 32 for sprite display
	// VAD >= 4
	// BBR + BBL >= 2
	if (h_vis_area <= 1 || v_vis_area <= 1 || h_total <= h_vis_area || v_total <= v_vis_area)
	{
		popmessage("CRTC assertion failed total (%d x %d) visible (%d x %d)", h_total, v_total, h_vis_area, v_vis_area);
		return;
	}

	visarea.set(0, h_vis_area - 1, 0, v_vis_area - 1);

	// TODO: interlace / vertical magnify, bit 7
	// TODO: actual clock source must be external, assume known PC-88 XTALs, a bit off compared to PC-88 with the values above
	const int clock_speed = BIT(m_buf_ram[0x00], 6) ? (31'948'800 / 4) : (28'636'363 / 2);

	refresh = HZ_TO_ATTOSECONDS(clock_speed) * h_vis_area * v_vis_area;

	m_screen->configure(h_total, v_total, visarea, refresh);
}

/*
 * xxxx x??? [0] lower text attribute table offset
 * ---- -xxx [1] upper text attribute, unused on PC88VA
 *           \- Multiplied by 256, lower 3 bits of [0] separated on datasheet (typo?)
 * ---- xxxx [2] BC Backdrop Color
 */
void pc88va_state::execute_dspon_cmd()
{
	m_tsp.tvram_vreg_offset = m_buf_ram[0] << 8;
	m_tsp.disp_on = true;
	LOGIDP("DSPON (%02x %02x %02x) %05x\n"
		, m_buf_ram[0]
		, m_buf_ram[1]
		, m_buf_ram[2]
		, m_tsp.tvram_vreg_offset | 0x40000
	);
}

/*
 * xxxx xxxx [0] ATROFF attribute offset (L)
 * xxxx xxxx [1] ^ (M)
 * ---- -xxx [2] ^ (H), possibly unused on PC88VA, signed in two's complement (!)
 * -xxx ---- [2] PITCH for character code
 * x--- ---- [2] (undocumented pitch bit?)
 * ---x xxxx [3] MRA Maximum Raster Address, line height + 1
 *               \- affects underline, disables layer if setting < 7
 * ---x xxxx [4] HRA Horizontal Raster Address, horizontal position for hline vertical attribute
 * ---x xxxx [5] BR Blinking Rate, affects both attribute and cursor in number of frames, max value on 0
 *               \- attribute x24 on, x8 off
 *               \- cursor x8 on, x8 off
 */
void pc88va_state::execute_dspdef_cmd()
{
	m_tsp.attr_offset = m_buf_ram[0] | m_buf_ram[1] << 8;
	// m_buf_ram[2] & 1 used on POST (?)
	if (m_buf_ram[2] & 0x6)
		popmessage("TSP: unimplemented (H) attribute ");

	m_tsp.pitch = (m_buf_ram[2] & 0x70) >> 4;
	if (BIT(m_buf_ram[2], 7))
		popmessage("TSP: undocumented PITCH bit set");

	m_tsp.line_height = (m_buf_ram[3] & 0x1f) + 1;
	if (m_tsp.line_height < 8)
		popmessage("TSP: line height disable");
	m_tsp.h_line_pos = m_buf_ram[4];
	m_tsp.blink = (m_buf_ram[5] & 0xf8);
	if (m_tsp.blink == 0)
		m_tsp.blink = 0x100;
	LOGIDP("DSPDEF (%02x %02x %02x %02x %02x %02x) %05x ATTR | %02x pitch | %02x line height| %02x hline | %d blink rate\n"
		, m_buf_ram[0], m_buf_ram[1], m_buf_ram[2], m_buf_ram[3], m_buf_ram[4], m_buf_ram[5]
		, m_tsp.attr_offset | 0x40000
		, m_tsp.pitch
		, m_tsp.line_height
		, m_tsp.h_line_pos
		, m_tsp.blink
	);
}

/*
 * xxxx x--- [0] CURN Sprite Cursor number (sprite RAM entry)
 * ---- --x- [0] CE show cursor bit (actively modifies the spriteram entry)
 * ---- ---x [0] BE Blink Enable
 */
void pc88va_state::execute_curdef_cmd()
{
	m_tsp.curn = (m_buf_ram[0] & 0xf8);
	const bool cursor_enable = bool(BIT(m_buf_ram[0], 1));
	const bool blink_enable = bool(BIT(m_buf_ram[0], 0));
	LOGIDP("CURDEF %02x|%d show|%d blink\n"
		, m_buf_ram[0] & 0xf8
		, cursor_enable
		, blink_enable
	);
	tsp_sprite_enable(m_tsp.curn, cursor_enable, blink_enable);
}

/*
 * -xx- ---- [0] strip layer ID for cursor (attribute?) and light pen
 */
void pc88va_state::execute_actscr_cmd()
{
	const u8 active_screen_area = (m_buf_ram[0] & 0x60) >> 5;
	LOGIDP("ACTSCR (%02x) %d\n", m_buf_ram[0], active_screen_area);
}

/*
 * xxxx xxxx [0] Cursor Position Y (lo word)
 * ---- -xxx [1] Cursor Position Y (hi word)
 * xxxx xxxx [2] Cursor Position X (lo word)
 * ---- --xx [3] Cursor Position X (hi word)
 */
void pc88va_state::execute_curs_cmd()
{
	m_tsp.cur_pos_y = m_buf_ram[0] | (m_buf_ram[1] & 0x7) << 8;
	m_tsp.cur_pos_x = m_buf_ram[2] | (m_buf_ram[3] & 0x3) << 8;
	LOGIDP("CURS X: %d Y: %d\n", m_tsp.cur_pos_x, m_tsp.cur_pos_y);
}

/*
 * [0] Emulate target strip ID x 32
 * [1] The number of chars
 * [2] The number of attributes
 * [3] The number of lines
 */
void pc88va_state::execute_emul_cmd()
{
	// TODO: PC88VA specific, starts 3301 video emulation
	//popmessage("TSP: executed EMUL command");
}

/*
 * xxxx xxxx [0] SAB Sprite Table Offset (lo word)
 * ---- -xxx [1] SAB hi word
 *           \- again multiplied by 256
 * xxxx x--- [2] HSPN: Maximum number of sprites in one raster (num + 1) for Sprite Over
 * ---- -x-- [2] ESP: Enable Sprite Interrupt
 *           \- causes INT pin high for sprite collision or maximum sprite detected
 * ---- --x- [2] MG: all sprites are 2x zoomed vertically when 1
 * ---- ---x [2] GR: 1 to enable the group collision detection
 */
void pc88va_state::execute_spron_cmd()
{

	m_tsp.spr_offset = m_buf_ram[0] << 8;
	m_tsp.spr_on = true;
	m_tsp.spr_mg = BIT(m_buf_ram[2], 1);
	LOGIDP("SPRON (%02x %02x %02x) %05x offs| %d max sprites| %d MG| %d GR|\n"
		, m_buf_ram[0]
		, m_buf_ram[1]
		, m_buf_ram[2]
		, m_tsp.spr_offset + 0x40000
		, (m_buf_ram[2] & 0xf8) + 1
		, m_tsp.spr_mg
		, bool(BIT(m_buf_ram[2], 0))
	);
	// TODO: reset sprite status
}

/*
 * Toggle an individual sprite in the sprite ram entry
 * xxxx x--- [0] SPN target sprite number
 * ---- --x- [0] SPSW sprite off/on switch
 */
void pc88va_state::execute_sprsw_cmd()
{
	const u8 spn = m_buf_ram[0] & 0xf8;
	const bool spsw = bool(BIT(m_buf_ram[0], 1));
	LOGIDP("SPRSW (%02x) %08x offset| %s SW\n"
		, m_buf_ram[0]
		, m_tsp.spr_offset + 0x40000
		, spsw ? "enable" : "disable"
	);
	tsp_sprite_enable(spn, spsw, false);
}

// 88va2d and friends during transitions
/*
 * xxxx x--- [0] target SPN
 * ---- -xxx [0] target attribute
 * Afterwards, data written to the param port will indefinitely write to sprite area
 * until command is cancelled, auto-incrementing per byte and eventually moving on to
 * the next SPN when attribute overruns past 7.
 */
void pc88va_state::execute_spwr_cmd(u8 data)
{
	if (m_tsp.spwr_define == true)
	{
		m_tsp.spwr_define = false;
		m_tsp.spwr_offset = data;
	}
	else
	{
		// TODO: likely going in FIFO full state
		// 88va2d delays on status & 0x1f being all off then checks for IBF before every data dispatch.
		const u32 target_offset = (m_tsp.spr_offset + m_tsp.spwr_offset) / 2;

		if (m_tsp.spwr_offset & 1)
			m_tvram[target_offset] = (m_tvram[target_offset] & 0x00ff) | (data << 8);
		else
			m_tvram[target_offset] = (m_tvram[target_offset] & 0xff00) | (data & 0xff);
		m_tsp.spwr_offset ++;
	}
}

void pc88va_state::idp_param_w(uint8_t data)
{
	if(m_cmd == DSPOFF || m_cmd == EXIT || m_cmd == SPROFF || m_cmd == SPROV) // no param commands
		return;

	// special: until a different command is issued just route to SPWR
	if (m_cmd == SPWR)
	{
		execute_spwr_cmd(data);
		return;
	}

	m_buf_ram[m_buf_index] = data;
	m_buf_index++;

	if(m_buf_index >= m_buf_size)
	{
		m_buf_index = 0;
		switch(m_cmd)
		{
			case SYNC:      execute_sync_cmd();    break;
			case DSPON:     execute_dspon_cmd();   break;
			case DSPDEF:    execute_dspdef_cmd(); break;
			case CURDEF:    execute_curdef_cmd(); break;
			case ACTSCR:    execute_actscr_cmd(); break;
			case CURS:      execute_curs_cmd();   break;
			case EMUL:      execute_emul_cmd();   break;
			case SPRON:     execute_spron_cmd();  break;
			case SPRSW:     execute_sprsw_cmd();   break;

			default:
				//printf("%02x\n",data);
				break;
		}
	}
}

/****************************************
 * I/O handlers
 ***************************************/

/*
 * $100
 * x--- ---- ---- ---- GDEN0 graphics display enable
 * -x-- ---- ---- ---- GVM superimpose if 1?
 * --x- ---- ---- ---- XVSP video signal output mode (0) inhibit scan signals
 * ---x ---- ---- ---- SYNCEN horizontal sync output (0) inhibit hsync
 * TODO: confirm following two
 * ---- x--- ---- ---- YMMD GVRAM mode (0) screen 0 only (1) screens 0 and 1
 * ---- -x-- ---- ---- DM gfx display mode (0) multiplane (1) single plane -> cfr. $153 GMSP
 * ---- ---- xx-- ---- RSM CRT raster scan mode
 * ---- ---- 0x-- ---- Non-interlace mode 0/1
 * ---- ---- 1x-- ---- Interlace mode 0/1
 * ---- ---- --x- ---- GDEN1 gfx display circuit reset (0) reset
 * ---- ---- ---x ---- SYNCM sync signal mode (0) internal (1) external
 * ---- ---- ---- --xx VW vertical resolution
 * ---- ---- ---- --00 400 lines
 * ---- ---- ---- --01 408 lines
 * ---- ---- ---- --10 200 lines
 * ---- ---- ---- --11 204 lines
 */
void pc88va_state::screen_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_screen_ctrl_reg);

	m_ymmd = bool(BIT(m_screen_ctrl_reg, 11));
	m_dm = bool(BIT(m_screen_ctrl_reg, 10));
	//                  YMMD           DM
	// mightmag 0xb060  (0) screen 0  (0) multiplane
}

u16 pc88va_state::screen_ctrl_r()
{
	return m_screen_ctrl_reg;
}

/*
 * $102
 * ---x --xx ---- ---- screen 1 regs
 * ---x ---- ---- ---- HW1 screen 1 hres (0) 640 dots (1) 320
 * ---- --xx ---- ---- PM1 screen 1 pixel mode
 * ---- --00 ---- ---- 1bpp
 * ---- --01 ---- ---- 4bpp
 * ---- --10 ---- ---- 8bpp
 * ---- --11 ---- ---- RGB565
 * ---- ---- ---x ---- HW0 screen 0 hres, as above
 * ---- ---- ---- --xx PM0 screen 0 pixel mode, as above
 */
void pc88va_state::gfx_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_gfx_ctrl_reg);
}

// upo wants a readback otherwise no layer appears on title/gameplay
u16 pc88va_state::gfx_ctrl_r()
{
	return m_gfx_ctrl_reg;
}

/*
 * $10c
 * --xx ---- ---- ---- BDM1/BDM0 color backdrop mode #
 * --00 ---- ---- ---- inner background color, outer transparent
 * --01 ---- ---- ---- inner transparent, outer background
 * --10 ---- ---- ---- inside/outside background color
 * --11 ---- ---- ---- inside/outside transparent
 *                     \- mode 0 only available on 24.8KHz monitor
 * ---- ---x xx-- ---- PLTM2/PLTM1/PLTM0 color palette mode
 * ---- ---0 xx-- ---- <V1/V2 Modes, cfr. pmode in $32 and pm00 in $31)>
 * ---- ---1 00-- ---- use palette bank 0
 * ---- ---1 01-- ---- use palette bank 1
 * ---- ---1 10-- ---- mixed mode
 * ---- ---1 11-- ---- combined 32-color mode
 * ---- ---- --xx ---- PLTP1/PLTP0 layer select for PLTM mixed mode
 * ---- ---- --00 ---- text layer
 * ---- ---- --01 ---- sprites
 * ---- ---- --10 ---- graphic 0
 * ---- ---- --11 ---- graphic 1
 * ---- ---- ---- xx-- BLKM1/BLKM0 color blink rate
 * ---- ---- ---- 00-- blink off
 * ---- ---- ---- 01-- blink every 32 frames
 * ---- ---- ---- 10-- blink every 64 frames
 * ---- ---- ---- 11-- blink every 128 frames
 * ---- ---- ---- --xx BLKD blink duty
 * ---- ---- ---- --00 12.5%
 * ---- ---- ---- --01 25%
 * ---- ---- ---- --10 50%
 * ---- ---- ---- --11 75%
 */
void pc88va_state::color_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_color_mode);

	const u8 bdm = (m_color_mode & 0x3000) >> 12;
	m_pltm = (m_color_mode & 0x01c0) >> 6;
	m_pltp = (m_color_mode & 0x0030) >> 4;
	const u8 blkm = (m_color_mode & 0x000c) >> 2;
	const u8 blkd = (m_color_mode & 0x0003) >> 0;
	LOGCOLOR("Color Mode (%04x & %04x)|%02x BDM|%s PLTM2|%02x PLTM1/PLTM0|%02x PLTP|%02x BLKM|%02x BLKD\n"
		, m_color_mode, mem_mask
		, bdm
		, m_pltm & 4 ? "V3 mode" : "V1/V2 mode"
		, m_pltm & 3
		, m_pltp
		, blkm
		, blkd
	);
	//        PLTM          - PLTP
	// rtype, shinraba:
	//        0x02 (mixed)    0x03 graphics 1
	// micromus, famista, shanghai, ballbrkr:
	//        0x02 (mixed)    0x02 graphics 0
	// olteus:
	//        0x02 (mixed)    0x01 sprites
	// mightmag:
	//        0x02 (mixed)    0x00 text
	// animefrm:
	//        0x03 (combined) 0x00 text
	// boomer (gameplay):
	//        0x01 (bank 1)   0x02 graphic 0 (left on for previous 0x02 - 0x02 mode)
	// illcity, xak2 (pre-loading screens):
	//        0x00 (bank 0)   0x00 text

}

void pc88va_state::palette_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_palram[offset]);

	const u16 color = m_palram[offset];
	u8 b = pal5bit((color & 0x001f));
	u8 r = pal5bit((color & 0x03e0) >> 5);
	u8 g = pal6bit((color & 0xfc00) >> 10);

	// TODO: docs suggests this arrangement but it's wrong
	// may be just one bit always on?
//  b = (m_palram[offset] & 0x001e) >> 1;
//  r = (m_palram[offset] & 0x03c0) >> 6;
//  g = (m_palram[offset] & 0x7800) >> 11;

	m_palette->set_pen_color(offset, r, g, b);
}

void pc88va_state::video_pri_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video_pri_reg[offset]);
}

void pc88va_state::text_transpen_w(offs_t offset, u16 data, u16 mem_mask)
{
	// TODO: understand what these are for, docs blabbers about text/sprite color separation?
	//  cfr. rogueall, olteus on disk swap screen
	// shanghai on winning animation (flips 0xf801 / 0x07c1, intentional?)
	COMBINE_DATA(&m_text_transpen);
	if (m_text_transpen & 0xfff0)
		popmessage("text transpen > 15 (%04x)", m_text_transpen);
}

/*
 * $14c-$14f Kanji CG ports
 * Alt method for access kanji ROM for drawing to graphic layers
 */
u8 pc88va_state::kanji_cg_r()
{
	uint8_t *kanji_rom = (uint8_t *)m_kanji_rom.target();

	// ANK
	if (m_kanji_cg_jis[1] == 0)
		return kanji_rom[0x40000 + (m_kanji_cg_jis[0] * 0x10) + (m_kanji_cg_line)];

	// PCG
	if (m_kanji_cg_jis[0] == 0x56)
	{
		// jis2 = 0x21 / 0x22 "PC" on hovered top status bar for animefrm
		// NB: software reverts the two chars once it gets upped to bitmap layer.
		const u32 pcg_addr = ((m_kanji_cg_jis[1] & 0x1f) + ((m_kanji_cg_jis[1] & 0x60) << 1)) * 0x20;
		return m_kanji_ram[pcg_addr + (m_kanji_cg_line << 1) + (m_kanji_cg_lr ^ 1)];
	}

	const u32 kanji_address = calc_kanji_rom_addr(m_kanji_cg_jis[0] + 0x20, m_kanji_cg_jis[1], 0, 0);

	return kanji_rom[kanji_address + (m_kanji_cg_line << 1) + (m_kanji_cg_lr ^ 1)];
}

void pc88va_state::kanji_cg_raster_w(u8 data)
{
	m_kanji_cg_line = data & 0xf;
	m_kanji_cg_lr = BIT(data, 5);
}

void pc88va_state::kanji_cg_address_w(offs_t offset, u8 data)
{
	m_kanji_cg_jis[offset] = data;
}

/*
 * $148 Text Control 1
 * x--- ---- TD Text Disable (1)
 * -xxx ---- VALT2/VALT1/VALT0 TVRAM access restriction (?)
 * -000 ---- No limit
 * -??? ---- value x4
 * ---- x--- ATM text attribute mode (1) V3 Mode (0) V1/V2
 * ---- -x-- ANKM character font mode (1) 16 (0) 8
 * ---- --x- IDP memory mode (1) word mode
 * ---- ---1 <unknown, always 1?>
 */
void pc88va_state::text_control_1_w(u8 data)
{
	m_td = bool(BIT(data, 7));

	if ((data & 0x7d) != 1)
		LOG("I/O $148 write %02x\n", data);
}


/****************************************
 * GVRAM
 ***************************************/

u8 pc88va_state::rop_execute(u8 plane_rop, u8 src, u8 dst, u8 pat)
{
	u8 res = 0;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(plane_rop, i))
		{
			u8 src_data = BIT(i, 0) ? src : ~src;
			u8 dst_data = BIT(i, 1) ? dst : ~dst;
			u8 pat_data = BIT(i, 2) ? pat : ~pat;
			res |= src_data & dst_data & pat_data;
		}
	}
	return res;
}

u8 pc88va_state::gvram_multiplane_r(offs_t offset)
{
	if (m_multiplane.aacc)
	{
		u32 address = (offset & 0x7fff) | (m_multiplane.gmap << 15);
		u8 res = 0xff;
		for (int plane = 0; plane < 4; plane++)
		{
			if (!BIT(m_multiplane.xrpm, plane))
			{
				const u8 src = m_gvram[address | plane * 0x10000];
				// Comparison enable
				if (m_multiplane.cmpen)
					res &= ~(src ^ m_multiplane.cmpr[plane]);
				else
					res &= src;

				// update on reads
				if (BIT(m_multiplane.pmod, 0) && !machine().side_effects_disabled())
				{
					m_multiplane.patr[plane][BIT(m_multiplane.prwp, plane)] = src;
				}
			}
		}

		// flip register write index on 16-bit mode
		if ((m_multiplane.pmod & 5) == 5 && !machine().side_effects_disabled())
		{
			m_multiplane.prwp ^= 0xf;
		}

		return res;
	}

	return m_gvram[offset];
}

void pc88va_state::gvram_multiplane_w(offs_t offset, u8 data)
{
	if (m_multiplane.aacc)
	{
		u32 address = (offset & 0x7fff) | (m_multiplane.gmap << 15);
		for (int plane = 0; plane < 4; plane++)
		{
			if (!BIT(m_multiplane.xwpm, plane))
			{
				switch(m_multiplane.wss & 3)
				{
					// ROP
					case 0:
					{
						const u8 src = m_gvram[address | plane * 0x10000];
						m_gvram[address | plane * 0x10000] = rop_execute(
							m_multiplane.rop[plane],
							src,
							data,
							m_multiplane.patr[plane][BIT(m_multiplane.prrp, plane)]
						);

						// update pattern on writes
						if (BIT(m_multiplane.pmod, 1))
						{
							m_multiplane.patr[plane][BIT(m_multiplane.prwp, plane)] = src;
						}
						break;
					}
					// Pattern
					case 1:
						m_gvram[address | plane * 0x10000] = m_multiplane.patr[plane][BIT(m_multiplane.prrp, plane)];
						break;
					// Normal writes
					case 2:
						m_gvram[address | plane * 0x10000] = data;
						break;
					// NOP
					case 3:
						break;
				}
			}
		}

		// flip register indices on 16-bit mode
		if (BIT(m_multiplane.pmod, 2))
		{
			m_multiplane.prrp ^= 0xf;
			if (BIT(m_multiplane.pmod, 1))
				m_multiplane.prwp ^= 0xf;
		}
		return;
	}

	m_gvram[offset] = data;
}

u8 pc88va_state::gvram_singleplane_r(offs_t offset)
{
	// apparently no side effects on reads
	return m_gvram[offset];
}

void pc88va_state::gvram_singleplane_w(offs_t offset, u8 data)
{
	const u8 page_bank = BIT(offset, 17);
	switch(m_singleplane.wss & 3)
	{
		// ROP
		case 0:
		{
			const u8 src = m_gvram[offset];
			m_gvram[offset] = rop_execute(
				m_singleplane.rop[page_bank],
				src,
				data,
				m_singleplane.patr[page_bank]
			);
			break;
		}
		// Pattern
		case 1:
			m_gvram[offset] = m_singleplane.patr[page_bank];
			break;
		// Normal writes
		case 2:
			m_gvram[offset] = data;
			break;
		// NOP
		case 3:
			break;
	}
}

