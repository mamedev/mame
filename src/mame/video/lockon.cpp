// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Lock-On video hardware

***************************************************************************/

#include "emu.h"
#include "includes/lockon.h"
#include "cpu/nec/nec.h"
#include "video/resnet.h"

#define CURSOR_XPOS         168
#define CURSOR_YPOS         239
#define FRAMEBUFFER_MAX_X   431
#define FRAMEBUFFER_MAX_Y   (uint32_t)((FRAMEBUFFER_CLOCK / (FRAMEBUFFER_MAX_X-1)).dvalue() / (PIXEL_CLOCK/(HTOTAL*VTOTAL)).dvalue())


/*************************************
 *
 *  HD46505S-2 CRT Controller
 *
 *************************************/

READ16_MEMBER(lockon_state::lockon_crtc_r)
{
	return 0xffff;
}

WRITE16_MEMBER(lockon_state::lockon_crtc_w)
{
#if 0
	data &= 0xff;

	if (offset == 0)
	{
		switch (data)
		{
			case 0x00: osd_printf_debug("Horizontal Total         "); break;
			case 0x01: osd_printf_debug("Horizontal displayed     "); break;
			case 0x02: osd_printf_debug("Horizontal sync position "); break;
			case 0x03: osd_printf_debug("Horizontal sync width    "); break;
			case 0x04: osd_printf_debug("Vertical total           "); break;
			case 0x05: osd_printf_debug("Vertical total adjust    "); break;
			case 0x06: osd_printf_debug("Vertical displayed       "); break;
			case 0x07: osd_printf_debug("Vertical sync position   "); break;
			case 0x08: osd_printf_debug("Interlace mode           "); break;
			case 0x09: osd_printf_debug("Max. scan line address   "); break;
			case 0x0a: osd_printf_debug("Cursror start            "); break;
			case 0x0b: osd_printf_debug("Cursor end               "); break;
			case 0x0c: osd_printf_debug("Start address (h)        "); break;
			case 0x0d: osd_printf_debug("Start address (l)        "); break;
			case 0x0e: osd_printf_debug("Cursor (h)               "); break;
			case 0x0f: osd_printf_debug("Cursor (l)               "); break;
			case 0x10: osd_printf_debug("Light pen (h))           "); break;
			case 0x11: osd_printf_debug("Light pen (l)            "); break;
		}
	}
	else if (offset == 1)
	{
		osd_printf_debug("0x%.2x, (%d)\n",data, data);
	}
#endif
}

TIMER_CALLBACK_MEMBER(lockon_state::cursor_callback)
{
	if (m_main_inten)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // V30

	m_cursor_timer->adjust(m_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS));
}

/*************************************
 *
 *  Palette decoding
 *
 *************************************/

static const res_net_info lockon_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{RES_NET_AMP_NONE, 560, 0, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 0, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 0, 5, {4700, 2200, 1000, 470, 220}}
	}
};

static const res_net_info lockon_pd_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{RES_NET_AMP_NONE, 560, 580, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 580, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 580, 5, {4700, 2200, 1000, 470, 220}}
	}
};

void lockon_state::lockon_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 1024; ++i)
	{
		uint8_t r, g, b;
		uint8_t const p1 = color_prom[i];
		uint8_t const p2 = color_prom[i + 0x400];

		if (p2 & 0x80)
		{
			r = compute_res_net((p2 >> 2) & 0x1f, 0, lockon_net_info);
			g = compute_res_net(((p1 >> 5) & 0x7) | (p2 & 3) << 3, 1, lockon_net_info);
			b = compute_res_net((p1 & 0x1f), 2, lockon_net_info);
		}
		else
		{
			r = compute_res_net((p2 >> 2) & 0x1f, 0, lockon_pd_net_info);
			g = compute_res_net(((p1 >> 5) & 0x7) | (p2 & 3) << 3, 1, lockon_pd_net_info);
			b = compute_res_net((p1 & 0x1f), 2, lockon_pd_net_info);
		}

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*************************************
 *
 *  Character tilemap handling
 *
 *************************************/

WRITE16_MEMBER(lockon_state::lockon_char_w)
{
	m_char_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(lockon_state::get_lockon_tile_info)
{
	uint32_t tileno = m_char_ram[tile_index] & 0x03ff;
	uint32_t col = (m_char_ram[tile_index] >> 10) & 0x3f;

	col = (col & 0x1f) + (col & 0x20 ? 64 : 0);
	SET_TILE_INFO_MEMBER(0, tileno, col, 0);
}


/*******************************************************************************************

  Scene tilemap hardware

*******************************************************************************************/

WRITE16_MEMBER(lockon_state::lockon_scene_h_scr_w)
{
	m_scroll_h = data & 0x1ff;
}

WRITE16_MEMBER(lockon_state::lockon_scene_v_scr_w)
{
	m_scroll_v = data & 0x81ff;
}

void lockon_state::scene_draw(  )
{
	uint32_t y;

	/* 3bpp characters */
	const uint8_t *const gfx1 = memregion("gfx2")->base();
	const uint8_t *const gfx2 = gfx1 + 0x10000;
	const uint8_t *const gfx3 = gfx1 + 0x20000;
	const uint8_t *const clut = gfx1 + 0x30000;

	for (y = 0; y < FRAMEBUFFER_MAX_Y; ++y)
	{
		uint32_t x;
		uint32_t d0 = 0, d1 = 0, d2 = 0;
		uint32_t colour = 0;
		uint32_t y_offs;
		uint32_t x_offs;
		uint32_t y_gran;
		uint16_t *bmpaddr;
		uint32_t ram_mask = 0x7ff;

		y_offs = (y + m_scroll_v) & 0x1ff;

		/* Clamp - stops tilemap wrapping when screen is rotated */
		if (BIT(m_scroll_v, 15) && y_offs & 0x100)
			ram_mask = 0x7;

		x_offs = (m_scroll_h - 8) & 0x1ff;
		y_gran = y_offs & 7;

		if (x_offs & 7)
		{
			uint32_t tileidx;
			uint16_t addr = ((y_offs & ~7) << 3) + ((x_offs >> 3) & 0x3f);
			uint16_t ram_val = m_scene_ram[addr & ram_mask];

			colour = (clut[ram_val & 0x7fff] & 0x3f) << 3;
			tileidx = ((ram_val & 0x0fff) << 3) + y_gran;

			d0 = *(gfx1 + tileidx);
			d1 = *(gfx2 + tileidx);
			d2 = *(gfx3 + tileidx);
		}

		bmpaddr = &m_back_buffer->pix16(y);

		for (x = 0; x < FRAMEBUFFER_MAX_X; ++x)
		{
			uint32_t x_gran = (x_offs & 7) ^ 7;
			uint32_t col;

			if (!(x_offs & 7))
			{
				uint32_t tileidx;
				uint16_t addr = ((y_offs & ~7) << 3) + ((x_offs >> 3) & 0x3f);
				uint16_t ram_val = m_scene_ram[addr & ram_mask];

				colour = (clut[ram_val & 0x7fff] & 0x3f) << 3;
				tileidx = ((ram_val & 0x0fff) << 3) + y_gran;

				d0 = *(gfx1 + tileidx);
				d1 = *(gfx2 + tileidx);
				d2 = *(gfx3 + tileidx);
			}

			col = colour
					| (((d2 >> x_gran) & 1) << 2)
					| (((d1 >> x_gran) & 1) << 1)
					| ( (d0 >> x_gran) & 1);

			*bmpaddr++ = 0xa00 + col;

			x_offs = (x_offs + 1) & 0x1ff;
		}

	}
}

/*******************************************************************************************

    Ground Hardware

    Each framebuffer line corresponds to a three word entry in ground RAM,
    starting from offset 3:

       FEDCBA9876543210
    0 |.............xxx|  Tile line (A0-A2 GFX ROMs)
      |...........xx...|  Tile index (A6-A5 GFX ROMs)
      |........xxx.....|  ROM lut address (A6-A4)
      |.xxxxxxx........|  ROM lut address (A13-A7)
      |x...............|  /Line enable

    1 |........xxxxxxxx|  TZ2213 value
      |xxxxxxxx........|  X offset

    2 |........xxxxxxxx|  TZ2213 DX
      |.......x........|  Carry
      |x...............|  End of list marker

    An 8-bit ground control register controls the following:

       76543210
      |......xx|  LUT ROM A15-A14
      |....xx..|  LUT ROM select
      |..xx....|  CLUT ROM A13-A12
      |.x......|  GFX ROM A15, CLUT ROM A14
      |x.......|  GFX ROM bank select (always 0 - only 1 bank is present)

 *******************************************************************************************/

WRITE16_MEMBER(lockon_state::lockon_ground_ctrl_w)
{
	m_ground_ctrl = data & 0xff;
}

TIMER_CALLBACK_MEMBER(lockon_state::bufend_callback)
{
	m_ground->set_input_line_and_vector(0, HOLD_LINE, 0xff); // V30
	m_object->set_input_line(NEC_INPUT_LINE_POLL, ASSERT_LINE);
}

/* Get data for a each 8x8x3 ground tile */
#define GET_GROUND_DATA()                                                                \
{                                                                                        \
	uint32_t gfx_a4_3  = (ls163 & 0xc) << 1;                                               \
	uint32_t lut_addr  = lut_address + ((ls163 >> 4) & 0xf);                               \
	uint32_t gfx_a14_7 = lut_rom[lut_addr] << 7;                                           \
	clut_addr = (lut_rom[lut_addr] << 4) | clut_a14_12 | clut_a4_3 | (ls163 & 0xc) >> 2; \
	gfx_addr  = gfx_a15 | gfx_a14_7 | gfx_a6_5 | gfx_a4_3 | gfx_a2_0;                    \
	pal = (clut_rom[clut_addr] << 3);                                                    \
	rom_data1 = gfx_rom[gfx_addr];                                                       \
	rom_data2 = gfx_rom[gfx_addr + 0x10000];                                             \
	rom_data3 = gfx_rom[gfx_addr + 0x20000];                                             \
}

void lockon_state::ground_draw(  )
{
	/* ROM pointers */
	const uint8_t *const gfx_rom  = memregion("gfx4")->base();
	const uint8_t *const lut_rom  = gfx_rom + 0x30000 + ((m_ground_ctrl >> 2) & 0x3 ? 0x10000 : 0);
	const uint8_t *const clut_rom = gfx_rom + 0x50000;

	uint32_t lut_a15_14   = (m_ground_ctrl & 0x3) << 14;
	uint32_t clut_a14_12 = (m_ground_ctrl & 0x70) << 8;
	uint32_t gfx_a15 = (m_ground_ctrl & 0x40) << 9;
	uint32_t offs = 3;
	uint32_t y;

	/* TODO: Clean up and emulate CS of GFX ROMs? */
	for (y = 0; y < FRAMEBUFFER_MAX_Y; ++y)
	{
		uint16_t *bmpaddr = &m_back_buffer->pix16(y);
		uint8_t ls163;
		uint32_t clut_addr;
		uint32_t gfx_addr;
		uint8_t rom_data1 = 0;
		uint8_t rom_data2 = 0;
		uint8_t rom_data3 = 0;
		uint32_t pal = 0;
		uint32_t x;

		/* Draw this line? */
		if (!(m_ground_ram[offs] & 0x8000))
		{
			uint32_t gfx_a2_0  =  m_ground_ram[offs] & 0x0007;
			uint32_t gfx_a6_5  = (m_ground_ram[offs] & 0x0018) << 2;
			uint32_t clut_a4_3 = (m_ground_ram[offs] & 0x0018) >> 1;
			uint8_t   tz2213_x  = m_ground_ram[offs + 1] & 0xff;
			uint8_t   tz2213_dx = m_ground_ram[offs + 2] & 0xff;

			uint32_t lut_address = lut_a15_14 + ((m_ground_ram[offs] & 0x7fe0) >> 1);
			uint32_t cy = m_ground_ram[offs + 2] & 0x0100;
			uint32_t color;
			uint32_t gpbal2_0_prev;

			ls163 = m_ground_ram[offs + 1] >> 8;

			gpbal2_0_prev = ((ls163 & 3) << 1) | BIT(tz2213_x, 7);

			if (gpbal2_0_prev)
				GET_GROUND_DATA();

			for (x = 0; x < FRAMEBUFFER_MAX_X; x++)
			{
				uint32_t tz2213_cy;
				uint32_t gpbal2_0 = ((ls163 & 3) << 1) | BIT(tz2213_x, 7);

				/* Stepped into a new tile? */
				if (gpbal2_0 < gpbal2_0_prev)
					GET_GROUND_DATA();

				gpbal2_0_prev = gpbal2_0;

				color = pal;
				color += (rom_data1 >> gpbal2_0) & 0x1;
				color += ((rom_data2 >> gpbal2_0) & 0x1) << 1;
				color += ((rom_data3 >> gpbal2_0) & 0x1) << 2;

				*bmpaddr++ = 0x800 + color;

				/* Update the counters */
				tz2213_cy = (uint8_t)tz2213_dx > (uint8_t)~(tz2213_x);
				tz2213_x = (tz2213_x + tz2213_dx);

				/* Carry? */
				if (tz2213_cy || cy)
					++ls163;
			}
		}

		offs += 3;

		/* End of list marker */
		if (m_ground_ram[offs + 2] & 0x8000)
		{
			m_bufend_timer->adjust(attotime::from_hz(FRAMEBUFFER_CLOCK) * (FRAMEBUFFER_MAX_X * y));
		}
	}
}


/*******************************************************************************************

    Object hardware

    Customs (4 each, 1 per bitplane)
    TZA118 - Scaling
    TZ4203 - Objects with integrated line buffer.

       FEDCBA9876543210
    0 |......xxxxxxxxxx|  Y position
      |....xx..........|  Object Y size
      |..xx............|  Object X size
      |.x..............|  Y flip
      |x...............|  X flip
    1 |........xxxxxxxx|  X/Y scale
      |.xxxxxxx........|  Colour
      |x...............|  End of list marker
    2 |xxxxxxxxxxxxxxxx|  Chunk ROM address
    3 |.....xxxxxxxxxxx|  X position

*******************************************************************************************/

/*
   There's logic to prevent shadow pixels from being drawn against the scene tilemap,
   so that shadows don't appear against the sky.
*/
#define DRAW_OBJ_PIXEL(COLOR)                            \
do {                                                     \
	if (px < FRAMEBUFFER_MAX_X)                          \
	if (COLOR != 0xf)                                    \
	{                                                    \
		uint8_t clr = m_obj_pal_ram[(pal << 4) + COLOR];     \
		uint16_t *pix = (line + px);                       \
		if (!(clr == 0xff && ((*pix & 0xe00) == 0xa00))) \
			*pix = 0x400 + clr;          \
	}                                                    \
	px = (px + 1) & 0x7ff;                               \
} while(0)

void lockon_state::objects_draw(  )
{
	uint32_t offs;

	const uint8_t  *const romlut = memregion("user1")->base();
	const uint16_t *const chklut = (uint16_t*)memregion("user2")->base();
	const uint8_t  *const gfxrom = memregion("gfx5")->base();
	const uint8_t  *const sproms = memregion("proms")->base() + 0x800;

	for (offs = 0; offs < m_object_ram.bytes(); offs += 4)
	{
		uint32_t y;
		uint32_t xpos;
		uint32_t ypos;
		uint32_t xsize;
		uint32_t ysize;
		uint32_t xflip;
		uint32_t yflip;
		uint32_t scale;
		uint32_t pal;
		uint32_t lines;
		uint32_t opsta;
		uint32_t opsta15_8;

		/* Retrieve the object attributes */
		ypos    = m_object_ram[offs] & 0x03ff;
		xpos    = m_object_ram[offs + 3] & 0x07ff;
		ysize   = (m_object_ram[offs] >> 10) & 0x3;
		xsize   = (m_object_ram[offs] >> 12) & 0x3;
		yflip   = BIT(m_object_ram[offs], 14);
		xflip   = BIT(m_object_ram[offs], 15);
		scale   = m_object_ram[offs + 1] & 0xff;
		pal = (m_object_ram[offs + 1] >> 8) & 0x7f;
		opsta   = m_object_ram[offs + 2];

		if (m_iden)
		{
			m_obj_pal_ram[(pal << 4) + m_obj_pal_addr] = m_obj_pal_latch;
			break;
		}

		/* How many lines will this sprite occupy? The PAL @ IC154 knows... */
		lines = scale >> (3 - ysize);

		opsta15_8 = opsta & 0xff00;

		/* Account for line buffering */
		ypos -=1;

		for (y = 0; y < FRAMEBUFFER_MAX_Y; y++)
		{
			uint32_t cy = (y + ypos) & 0x3ff;
			uint32_t optab;
			uint32_t lutaddr;
			uint32_t tile;
			uint8_t   cnt;
			uint32_t yidx;
			uint16_t *line = &m_back_buffer->pix16(y);
			uint32_t px = xpos;

			/* Outside the limits? */
			if (cy & 0x300)
				continue;

			if ((cy  & 0xff) >= lines)
				break;

			lutaddr = (scale & 0x80 ? 0x8000 : 0) | ((scale & 0x7f) << 8) | (cy & 0xff);
			optab = romlut[lutaddr] & 0x7f;

			if (yflip)
				optab ^= 7;

			yidx = (optab & 7);

			/* Now calculate the lower 7-bits of the LUT ROM address. PAL @ IC157 does this */
			cnt = (optab >> 3) * (1 << xsize);

			if (xflip)
				cnt ^= 7 >> (3 - xsize);
			if (yflip)
				cnt ^= (0xf >> (3 - ysize)) * (1 << xsize);

			cnt = (cnt + (opsta & 0xff));

			/* Draw! */
			for (tile = 0; tile < (1 << xsize); ++tile)
			{
				uint16_t sc;
				uint16_t scl;
				uint32_t x;
				uint32_t tileaddr;
				uint16_t td0, td1, td2, td3;
				uint32_t j;
				uint32_t bank;

				scl = scale & 0x7f;
				tileaddr = (chklut[opsta15_8 + cnt] & 0x7fff);
				bank = ((tileaddr >> 12) & 3) * 0x40000;
				tileaddr = bank + ((tileaddr & ~0xf000) << 3);

				if (xflip)
					--cnt;
				else
					++cnt;

				/* Draw two 8x8 tiles */
				for (j = 0; j < 2; ++j)
				{
					/* Get tile data */
					uint32_t tileadd = tileaddr + (0x20000 * (j ^ xflip));

					/* Retrieve scale values from PROMs */
					sc = sproms[(scl << 4) + (tile * 2) + j];

					/* Data from ROMs is inverted */
					td3 = gfxrom[tileadd + yidx] ^ 0xff;
					td2 = gfxrom[tileadd + 0x8000 + yidx] ^ 0xff;
					td1 = gfxrom[tileadd + 0x10000 + yidx] ^ 0xff;
					td0 = gfxrom[tileadd + 0x18000 + yidx] ^ 0xff;

					if (scale & 0x80)
					{
						for (x = 0; x < 8; ++x)
						{
							uint8_t col;
							uint8_t pix = x;

							if (!xflip)
								pix ^= 0x7;

							col = BIT(td0, pix)
								| (BIT(td1, pix) << 1)
								| (BIT(td2, pix) << 2)
								| (BIT(td3, pix) << 3);

							DRAW_OBJ_PIXEL(col);

							if (BIT(sc, x))
								DRAW_OBJ_PIXEL(col);
						}
					}
					else
					{
						for (x = 0; x < 8; ++x)
						{
							uint8_t col;
							uint8_t pix = x;

							if (BIT(sc, x))
							{
								if (!xflip)
									pix ^= 0x7;

								col = BIT(td0, pix)
									| (BIT(td1, pix) << 1)
									| (BIT(td2, pix) << 2)
									| (BIT(td3, pix) << 3);

								DRAW_OBJ_PIXEL(col);
							}
						}
					}
				}
			}
		}

		/* Check for the end of list marker */
		if (m_object_ram[offs + 1] & 0x8000)
			return;
	}
}

/* The mechanism used by the object CPU to update the object ASICs palette RAM */
WRITE16_MEMBER(lockon_state::lockon_tza112_w)
{
	if (m_iden)
	{
		m_obj_pal_latch = data & 0xff;
		m_obj_pal_addr = offset & 0xf;
		objects_draw();
	}
}

READ16_MEMBER(lockon_state::lockon_obj_4000_r)
{
	m_object->set_input_line(NEC_INPUT_LINE_POLL, CLEAR_LINE);
	return 0xffff;
}

WRITE16_MEMBER(lockon_state::lockon_obj_4000_w)
{
	m_iden = data & 1;
}


/*******************************************************************************************

    Frame buffer rotation hardware

      FEDCBA9876543210
    0 |........xxxxxxxx|  X start address
      |.......x........|  Direction
    1 |........xxxxxxxx|  TZ2213 IC65 value
      |.......x........|  TZ2213 IC65 /enable
    2 |........xxxxxxxx|  TZ2213 IC65 delta
    3 |........xxxxxxxx|  TZ2213 IC106 delta
      |.......x........|  TZ2213 IC106 enable
    4 |.......xxxxxxxxx|  Y start address
    5 |........xxxxxxxx|  TZ2213 IC66 value
    6 |........xxxxxxxx|  TZ2213 IC66 delta
      |.......x........|  TZ2213 IC65 /enable
    7 |........xxxxxxxx|  TZ2213 IC107 delta
      |.......x........|  TZ2213 /enable
      |......x.........|  Direction

*******************************************************************************************/

WRITE16_MEMBER(lockon_state::lockon_fb_clut_w)
{
	rgb_t color;

	color = m_palette->pen_color(0x300 + (data & 0xff));
	m_palette->set_pen_color(0x400 + offset, color);
}

/* Rotation control register */
WRITE16_MEMBER(lockon_state::lockon_rotate_w)
{
	switch (offset & 7)
	{
		case 0: m_xsal  = data & 0x1ff; break;
		case 1: m_x0ll  = data & 0xff;  break;
		case 2: m_dx0ll = data & 0x1ff; break;
		case 3: m_dxll  = data & 0x1ff; break;

		case 4: m_ysal  = data & 0x1ff; break;
		case 5: m_y0ll  = data & 0xff;  break;
		case 6: m_dy0ll =   data & 0x1ff;   break;
		case 7: m_dyll  = data & 0x3ff; break;
	}
}

#define INCREMENT(ACC, CNT)              \
do {                                     \
	carry = (uint8_t)d##ACC > (uint8_t)~ACC; \
	ACC += d##ACC;                       \
	if (carry) ++CNT;                    \
} while(0)

#define DECREMENT(ACC, CNT)              \
do {                                     \
	carry = (uint8_t)d##ACC > (uint8_t)ACC;  \
	ACC -= d##ACC;                       \
	if (carry) --CNT;                    \
} while(0)

void lockon_state::rotate_draw( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint32_t y;

	/* Counters */
	uint32_t cxy = m_xsal & 0xff;
	uint32_t cyy = m_ysal & 0x1ff;

	/* Accumulator values and deltas */
	uint8_t axy  = m_x0ll & 0xff;
	uint8_t daxy = m_dx0ll & 0xff;
	uint8_t ayy  = m_y0ll & 0xff;
	uint8_t dayy = m_dy0ll & 0xff;
	uint8_t dayx = m_dyll & 0xff;
	uint8_t daxx = m_dxll & 0xff;

	uint32_t xy_up = BIT(m_xsal, 8);
	uint32_t yx_up = BIT(m_dyll, 9);
	uint32_t axx_en  = !BIT(m_dxll, 8);
	uint32_t ayx_en  = !BIT(m_dyll, 8);
	uint32_t axy_en  = !BIT(m_dx0ll, 8);
	uint32_t ayy_en  = !BIT(m_dy0ll, 8);

	for (y = 0; y <= cliprect.max_y; ++y)
	{
		uint32_t carry;
		uint16_t *dst = &bitmap.pix16(y);
		uint32_t x;

		uint32_t cx = cxy;
		uint32_t cy = cyy;

		uint8_t axx = axy;
		uint8_t ayx = ayy;

		for (x = 0; x <= cliprect.max_x; ++x)
		{
			cx &= 0x1ff;
			cy &= 0x1ff;

			*dst++ = m_front_buffer->pix16(cy, cx);

			if (axx_en)
				INCREMENT(axx, cx);
			else
				++cx;

			if (ayx_en)
			{
				if (yx_up)
					INCREMENT(ayx, cy);
				else
					DECREMENT(ayx, cy);
			}
			else
			{
				if (yx_up)
					++cy;
				else
					--cy;
			}
		}

		if (axy_en)
		{
			if (xy_up)
				INCREMENT(axy, cxy);
			else
				DECREMENT(axy, cxy);
		}
		else
		{
			if (xy_up)
				++cxy;
			else
				--cxy;
		}

		if (ayy_en)
			INCREMENT(ayy, cyy);
		else
			++cyy;

		cxy &= 0xff;
		cyy &= 0x1ff;
	}
}


/*******************************************************************************************

    HUD Drawing Hardware

    A sprite layer that uses 8x8x1bpp tiles to form bigger sprites

    0  |.......xxxxxxxxx|  Y Position
       |xxxxxxx.........|  Code
    1  |.......xxxxxxxxx|  X Position
       |....xxx.........|  Colour
       |.xxx............|  Sprite width (0=8, 1=16, 2=24, 3=32 pixels, etc.)
       |x...............|  End of list marker

*******************************************************************************************/

void lockon_state::hud_draw( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t   *tile_rom = memregion("gfx3")->base();
	uint32_t offs;

	for (offs = 0x0; offs <= m_hud_ram.bytes(); offs += 2)
	{
		uint32_t y;
		uint32_t y_pos;
		uint32_t x_pos;
		uint32_t y_size;
		uint32_t x_size;
		uint32_t layout;
		uint16_t colour;
		uint32_t code;
		uint32_t rom_a12_7;

		/* End of sprite list marker */
		if (m_hud_ram[offs + 1] & 0x8000)
			break;

		y_pos   = m_hud_ram[offs] & 0x1ff;
		x_pos   = m_hud_ram[offs + 1] & 0x1ff;
		x_size = (m_hud_ram[offs + 1] >> 12) & 7;
		code    = (m_hud_ram[offs] >> 9) & 0x7f;
		colour = 0x200 + ((m_hud_ram[offs + 1] >> 9) & 7);
		layout = (code >> 5) & 3;

		rom_a12_7 = (code & 0xfe) << 6;

		/* Account for line buffering */
		y_pos -= 1;

		if (layout == 3)
			y_size = 32;
		else if (layout == 2)
			y_size = 16;
		else
			y_size = 8;

		for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
		{
			uint32_t xt;
			uint32_t cy;

			cy = y_pos + y;

			if (cy < 0x200)
				continue;

			if ((cy & 0xff) == y_size)
				break;

			for (xt = 0; xt <= x_size; ++xt)
			{
				uint32_t rom_a6_3;
				uint32_t px;
				uint8_t   gfx_strip;

				if (layout == 3)
					rom_a6_3 = (BIT(cy, 4) << 3) | (BIT(cy, 3) << 2) | (BIT(xt, 1) << 1) | BIT(xt, 0);
				else if (layout == 2)
					rom_a6_3 = ((BIT(code, 0) << 3) | (BIT(xt, 1) << 2) | (BIT(cy, 3) << 1) | (BIT(xt, 0)));
				else
					rom_a6_3 = (BIT(code, 0) << 3) | (xt & 7);

				rom_a6_3 <<= 3;

				/* Get tile data */
				gfx_strip = tile_rom[rom_a12_7 | rom_a6_3 | (cy & 7)];

				if (gfx_strip == 0)
					continue;

				/* Draw */
				for (px = 0; px < 8; ++px)
				{
					uint32_t x = x_pos + (xt << 3) + px;

					if (x <= cliprect.max_x)
					{
						uint16_t *dst = &bitmap.pix16(y, x);

						if (BIT(gfx_strip, px ^ 7) && *dst > 255)
							*dst = colour;
					}
				}
			}
		}
	}
}


/*************************************
 *
 *  Driver video handlers
 *
 *************************************/

void lockon_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lockon_state::get_lockon_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap->set_transparent_pen(0);

	/* Allocate the two frame buffers for rotation */
	m_back_buffer = std::make_unique<bitmap_ind16>(512, 512);
	m_front_buffer = std::make_unique<bitmap_ind16>(512, 512);

	/* 2kB of object ASIC palette RAM */
	m_obj_pal_ram = std::make_unique<uint8_t[]>(2048);

	/* Timer for ground display list callback */
	m_bufend_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(lockon_state::bufend_callback),this));

	/* Timer for the CRTC cursor pulse */
	m_cursor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(lockon_state::cursor_callback),this));
	m_cursor_timer->adjust(m_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS));

	save_item(NAME(*m_back_buffer));
	save_item(NAME(*m_front_buffer));
	save_pointer(NAME(m_obj_pal_ram), 2048);
}

uint32_t lockon_state::screen_update_lockon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* If screen output is disabled, fill with black */
	if (!BIT(m_ctrl_reg, 7))
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	/* Scan out the frame buffer in rotated order */
	rotate_draw(bitmap, cliprect);

	/* Draw the character tilemap */
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* Draw the HUD */
	hud_draw(bitmap, cliprect);

	return 0;
}

WRITE_LINE_MEMBER(lockon_state::screen_vblank_lockon)
{
	// on falling edge
	if (!state)
	{
		/* Swap the frame buffers */
		m_front_buffer.swap(m_back_buffer);

		/* Draw the frame buffer layers */
		scene_draw();
		ground_draw();
		objects_draw();
	}
}
