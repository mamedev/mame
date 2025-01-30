// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "tsconf.h"

#define PAGE4K(_r) ((_r) << 14)

#define W0_RAM (BIT(m_regs[MEM_CONFIG], 3))
#define NW0_MAP (BIT(m_regs[MEM_CONFIG], 2))
#define W0_WE (BIT(m_regs[MEM_CONFIG], 1))
#define ROM128 (BIT(m_regs[MEM_CONFIG], 0))
#define OFFS_512(_rl) (((m_regs[_rl + 1] & 1) << 8) | m_regs[_rl])

#define VM v_mode(BIT(m_regs[V_CONFIG], 0, 2))

static constexpr rectangle screen_area[4] = {
	rectangle(tsconf_state::with_hblank(52), tsconf_state::with_hblank(256 + 51), tsconf_state::with_vblank(48), tsconf_state::with_vblank(192 + 47)), // 52|256|52 x 48-192-48
	rectangle(tsconf_state::with_hblank(20), tsconf_state::with_hblank(320 + 19), tsconf_state::with_vblank(44), tsconf_state::with_vblank(200 + 43)), // 20|320|20 x 44-200-44
	rectangle(tsconf_state::with_hblank(20), tsconf_state::with_hblank(320 + 19), tsconf_state::with_vblank(24), tsconf_state::with_vblank(240 + 23)), // 20|320|20 x 24-240-24
	rectangle(tsconf_state::with_hblank(00), tsconf_state::with_hblank(360 - 01), tsconf_state::with_vblank(00), tsconf_state::with_vblank(288 - 01))  // 00|360|00 x 00-288-00
};

enum v_mode : u8
{
	VM_ZX = 0,
	VM_16C,
	VM_256C,
	VM_TXT
};

// https://github.com/tslabs/zx-evo/blob/master/pentevo/vdac/vdac1/cpld/top.v
static constexpr u8 pwm_to_rgb[32] = {
	0, 10, 21, 31, 42, 53, 63, 74,
	85, 95, 106, 117, 127, 138, 149, 159,
	170, 181, 191, 202, 213, 223, 234, 245,
	255, 255, 255, 255, 255, 255, 255, 255};

static constexpr rgb_t from_pwm(u16 pwm15)
{
	return rgb_t(pwm_to_rgb[BIT(pwm15, 10, 5)], pwm_to_rgb[BIT(pwm15, 5, 5)], pwm_to_rgb[BIT(pwm15, 0, 5)]);
}

rectangle tsconf_state::get_screen_area()
{
	rectangle info = screen_area[BIT(m_regs[V_CONFIG], 6, 2)];
	if (VM == VM_TXT)
		info.set_width(info.width() << 1);
	return info;
}

void tsconf_state::tsconf_update_bank0()
{
	u8 page0 = m_regs[PAGE0];
	if (!NW0_MAP)
	{
		/* ROM: 0-SYS, 1-DOS, 2-128, 3-48 */
		page0 = m_beta->started() && m_beta->is_active() ? ROM128 : (0x02 | ROM128);
		page0 |= (m_regs[PAGE0] & 0xfc);
	}

	if (W0_RAM)
	{
		m_bank_ram[0]->set_entry(page0);
		m_bank0_rom.disable();
	}
	else
	{
		m_bank_rom[0]->set_entry(page0 & 0x1f);
		m_bank0_rom.select(0);
	}
}

void tsconf_state::tsconf_update_video_mode()
{
	rectangle visarea = screen_area[3];
	if (VM == VM_TXT)
	{
		// scale screen for Text Mode
		rectangle origin_screen = screen_area[BIT(m_regs[V_CONFIG], 6, 2)];
		visarea.set_width(visarea.width() + origin_screen.width());
		m_gfxdecode->gfx(TM_TS_CHAR)->set_source(m_ram->pointer() + PAGE4K(m_regs[V_PAGE] ^ 0x01));
		m_ts_tilemap[TM_TS_CHAR]->mark_all_dirty();
	}

	m_ts_tilemap[TM_TILES0]->set_scrolldx(get_screen_area().left(), 0);
	m_ts_tilemap[TM_TILES0]->set_scrolldy(get_screen_area().top(), 0);
	m_ts_tilemap[TM_TILES1]->set_scrolldx(get_screen_area().left(), 0);
	m_ts_tilemap[TM_TILES1]->set_scrolldy(get_screen_area().top(), 0);

	m_screen->configure(visarea.max_x + 1, visarea.max_y + 1, visarea, m_screen->frame_period().as_attoseconds());
}

u8 tsconf_state::get_border_color(u16 hpos, u16 vpos)
{
	return m_regs[BORDER];
}

u32 tsconf_state::get_vpage_offset()
{
	return PAGE4K(m_regs[V_PAGE] & ((VM == VM_16C) ? 0xf8 : 0xf0));
}

u32 tsconf_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle scr = get_screen_area();
	rectangle vis = screen.visible_area();
	if (vis != scr)
	{
		rectangle bsides[4] = {
			rectangle(vis.left(),      vis.right(),    vis.top(),        scr.top() - 1),
			rectangle(vis.left(),      scr.left() - 1, scr.top(),        scr.bottom()),
			rectangle(scr.right() + 1, vis.right(),    scr.top(),        scr.bottom()),
			rectangle(vis.left(),      vis.right(),    scr.bottom() + 1, vis.bottom())
		};
		for (auto i = 0; i < 4; i++)
		{
			rectangle border = bsides[i] & cliprect;
			if (!border.empty())
				bitmap.fill(m_palette->pen_color(get_border_color()), border);
		}
	}

	scr &= cliprect;
	if (!scr.empty())
		tsconf_update_screen(screen, bitmap, scr);

	return 0;
}

/*
Layered as:
 + Border - already updated with screen_update_spectrum()
 + Graphics
 + Sprites 0
 + Tiles 0
 + Sprites 1
 + Tiles 1
 + Sprites 2
*/
void tsconf_state::tsconf_update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_regs[V_CONFIG], 5))
	{
		if (VM == VM_ZX)
		{
			tsconf_draw_zx(screen, bitmap, cliprect);
		}
		else if (VM == VM_TXT)
		{
			tsconf_draw_txt(bitmap, cliprect);
		}
		else
		{
			tsconf_draw_gfx(bitmap, cliprect);
		}
	}
	else
	{
		bitmap.fill(m_palette->pen_color(get_border_color()), cliprect);
	}

	if (!BIT(m_regs[V_CONFIG], 4))
	{
		screen.priority().fill(0, cliprect);
		if (BIT(m_regs[TS_CONFIG], 5))
		{
			m_ts_tilemap[TM_TILES0]->draw(
					screen, bitmap, cliprect,
					BIT(m_regs[TS_CONFIG], 2) ? TILEMAP_DRAW_ALL_CATEGORIES : TILEMAP_DRAW_CATEGORY(1), 1, 0);
		}

		if (BIT(m_regs[TS_CONFIG], 6))
		{
			m_ts_tilemap[TM_TILES1]->draw(
					screen, bitmap, cliprect,
					BIT(m_regs[TS_CONFIG], 3) ? TILEMAP_DRAW_ALL_CATEGORIES : TILEMAP_DRAW_CATEGORY(1), 2, 0);
		}

		if (BIT(m_regs[TS_CONFIG], 7))
		{
			draw_sprites(screen, bitmap, get_screen_area());
		}
	}
}

void tsconf_state::tsconf_draw_zx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 pal_offset = m_regs[PAL_SEL] << 4;
	u8 *screen_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]);
	u8 *attrs_location = screen_location + 0x1800;
	bool invert_attrs = u64(screen.frame_number() / m_frame_invert_count) & 1;
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
		u16 x = hpos - get_screen_area().left();
		u16 y = vpos - get_screen_area().top();
		u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		u8 *attr = &attrs_location[((y & 0xf8) << 2) | (x >> 3)];
		u32 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= cliprect.right())
		{
			u16 ink = pal_offset | ((*attr >> 3) & 0x08) | (*attr & 0x07);
			u16 pap = pal_offset | ((*attr >> 3) & 0x0f);
			u8 pix8 = (invert_attrs && (*attr & 0x80)) ? ~*scr : *scr;

			for (u8 b = 0x80 >> (x & 0x07); b != 0 && hpos <= cliprect.right(); b >>= 1, x++, hpos++)
				*pix++ = m_palette->pen_color((pix8 & b) ? ink : pap);
			scr++;
			attr++;
		}
	}
}

void tsconf_state::tsconf_draw_txt(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 *font_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE] ^ 0x01);
	u8 pal_offset = m_regs[PAL_SEL] << 4;
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
		u16 x = hpos - get_screen_area().left();
		u16 y = vpos - get_screen_area().top();
		u16 y_offset = (OFFS_512(G_Y_OFFS_L) + y) & 0x1ff;

		// TODO? u16 x_offset = OFFS_512(G_X_OFFS_L);
		u8 *text_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]) + (y_offset / 8 * 256 + x / 8);
		u32 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= cliprect.right())
		{
			u8 font_color = *(text_location + 128) & 0x0f;
			u8 bg_color = (*(text_location + 128) & 0xf0) >> 4;
			u8 char_x = *(font_location + (*text_location * 8) + (y_offset % 8));
			for (u8 b = 0x80 >> (x & 0x07); b != 0 && hpos <= cliprect.right(); b >>= 1, x++, hpos++)
				*pix++ = m_palette->pen_color(pal_offset | ((char_x & b) ? font_color : bg_color));
			text_location++;
		}
	}
}

void tsconf_state::tsconf_draw_gfx(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 pal_offset = m_regs[PAL_SEL] << 4;
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 y_offset = (OFFS_512(G_Y_OFFS_L) + m_gfx_y_frame_offset + vpos) & 0x1ff;
		u16 x_offset = (OFFS_512(G_X_OFFS_L) + (cliprect.left() - get_screen_area().left())) & 0x1ff;
		u8 *video_location = m_ram->pointer() + get_vpage_offset() + ((y_offset * 512 + x_offset) >> (2 - VM));
		u32 *bm = &(bitmap.pix(vpos, cliprect.left()));
		s16 width = cliprect.width();
		if (VM == VM_16C)
		{
			if (x_offset & 1)
			{
				*bm++ = m_palette->pen_color(pal_offset | (*video_location++ & 0x0f));
				x_offset++;
				width--;
			}
			for (; width > 0; width -= 2, x_offset += 2)
			{
				if (x_offset == 512)
					video_location -= 256;
				u8 pix = *video_location++;
				rgb_t pen = m_palette->pen_color(pal_offset | (pix >> 4));
				*bm++ = pen;
				if (width != 1)
				{
					pen = m_palette->pen_color(pal_offset | (pix & 0x0f));
					*bm++ = pen;
				}
			}
		}
		else // VM_256C
		{
			for (; width > 0; width--, x_offset++)
			{
				if (x_offset == 512)
					video_location -= 512;
				rgb_t pen = m_palette->pen_color(*video_location++);
				*bm++ = pen;
			}
		}
	}
}

/*
SFILE   Reg.16  7       6       5       4       3       2       1       0
0       R0L     Y[7:0]
1       R0H     YF      LEAP    ACT     -       YS[2:0]                 Y[8]
2       R1L     X[7:0]
3       R1H     XF      -       -       -       XS[2:0]                 X[8]
4       R2L     TNUM[7:0]
5       R2H     SPAL[7:4]                       TNUM[11:8]
*/
void tsconf_state::draw_sprites(screen_device &screen_d, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_sprites_cache.empty())
	{
		const rectangle screen = get_screen_area();
		u8 layer = 0;
		u8 *sinfo = m_sfile->pointer();
		for (u8 sid = 0; sid < 85 && layer < 3; sid++)
		{
			s16 y = *sinfo++;
			y |= BIT(*sinfo, 0) << 8;
			y += screen.top() - (y >= screen.height() ? 512 : 0);
			const u8 height8 = BIT(*sinfo, 1, 3);
			const bool leap_next = BIT(*sinfo, 6);
			if (!BIT(*sinfo, 5))
			{
				// sprite disabled -> move to next
				sinfo += 5;
			}
			else
			{
				const bool flipy = BIT(*sinfo++, 7);
				s16 x = *sinfo++;
				x |= BIT(*sinfo, 0) << 8;
				x += screen.left() - (x >= screen.width() ? 512 : 0);
				const u8 width8 = BIT(*sinfo, 1, 3);
				const bool flipx = BIT(*sinfo++, 7);
				u16 code = *sinfo++;
				code |= BIT(*sinfo, 0, 4) << 8;
				const u8 pal = BIT(*sinfo++, 4, 4);

				u8 tile_row = code / 64 + flipy * height8;
				const u32 pmask = (~1) << layer;
				for (auto iy = y; iy <= y + height8 * 8; iy = iy + 8)
				{
					u8 tile_col = (code % 64) + flipx * width8;
					for (auto ix = x; ix <= x + width8 * 8; ix = ix + 8)
					{
						sprite_data spr = {};
						spr.code = (tile_row % 64) * 64 + (tile_col % 64);
						spr.color = pal;
						spr.flipx = flipx;
						spr.flipy = flipy;
						spr.destx = ix;
						spr.desty = iy;
						spr.pmask = pmask;

						m_sprites_cache.push_back(spr);
						tile_col += flipx ? -1 : 1;
					}
					tile_row += flipy ? -1 : 1;
				}
			}
			layer += leap_next;
		}
	}

	// Higher Sprite draws on top. Iterate backwards.
	for (auto spr = m_sprites_cache.rbegin(); spr != m_sprites_cache.rend(); ++spr)
	{
		m_gfxdecode->gfx(TM_SPRITES)->prio_transpen(bitmap, cliprect,
			spr->code, spr->color, spr->flipx, spr->flipy, spr->destx, spr->desty,
			screen_d.priority(), spr->pmask, 0);
	}

}

void tsconf_state::ram_bank_write(u8 bank, offs_t offset, u8 data)
{
	if (BIT(m_regs[FMAPS], 4))
	{
		offs_t machine_addr = PAGE4K(bank) + offset;
		offs_t fmap_addr = BIT(m_regs[FMAPS], 0, 4) << 12;
		if ((machine_addr >= fmap_addr) && (machine_addr < (fmap_addr + 256 * 5)))
		{
			u16 addr_w = machine_addr - fmap_addr;
			if (addr_w < 512)
				cram_write(addr_w, data);
			else if (addr_w < 1024)
			{
				m_sprites_cache.clear();
				m_sfile->write(addr_w - 512, data);
			}
			else
				tsconf_port_xxaf_w((addr_w - 1024) << 8, data);
		}
	}

	if (bank > 0 || (W0_WE && W0_RAM))
		ram_page_write(m_regs[PAGE0 + bank], offset, data);
}

static int tiles_offset_to_raw(int t_offset)
{
	return bitswap<17>(t_offset, 16, 15, 14, 13, 12, 11, 7, 6, 5, 4, 3, 2, 10, 9, 8, 1, 0) << 1;
}

void tsconf_state::ram_page_write(u8 page, offs_t offset, u8 data)
{
	u32 ram_addr = PAGE4K(page) + offset;
	if (ram_addr >= PAGE4K(m_regs[T_MAP_PAGE]) && ram_addr < (PAGE4K(m_regs[T_MAP_PAGE] + 1)))
	{
		// TODO invalidate sprites, not entire map
		m_ts_tilemap[offset & 128 ? TM_TILES1 : TM_TILES0]->mark_all_dirty();
	}
	else
	{
		const int t0_offset = ram_addr - PAGE4K(m_regs[T0_G_PAGE] & 0xf8);
		if ((t0_offset >= 0) && (t0_offset < PAGE4K(8)))
		{
			const int raw_offset = tiles_offset_to_raw(t0_offset);
			m_tiles_raw[0][raw_offset] = data >> 4;
			m_tiles_raw[0][raw_offset + 1] = data & 0x0f;
			m_ts_tilemap[TM_TILES0]->mark_all_dirty();
		}

		const int t1_offset = ram_addr - PAGE4K(m_regs[T1_G_PAGE] & 0xf8);
		if ((t1_offset >= 0) && (t1_offset < PAGE4K(8)))
		{
			const int raw_offset = tiles_offset_to_raw(t1_offset);
			m_tiles_raw[1][raw_offset] = data >> 4;
			m_tiles_raw[1][raw_offset + 1] = data & 0x0f;
			m_ts_tilemap[TM_TILES1]->mark_all_dirty();
		}
	}

	if (ram_addr >= get_vpage_offset() && ram_addr < get_vpage_offset() + PAGE4K((VM == VM_16C) ? 8 : 16))
		m_ts_tilemap[TM_TS_CHAR]->mark_all_dirty();

	if (ram_addr >= PAGE4K(m_regs[m_regs[V_PAGE] ^ 0x01]) && ram_addr < PAGE4K(m_regs[m_regs[V_PAGE] ^ 0x01] + 1))
		m_gfxdecode->gfx(TM_TS_CHAR)->mark_all_dirty();

	const int spr_offset = ram_addr - PAGE4K(m_regs[SG_PAGE] & 0xf8);
	if ((spr_offset >= 0) && (spr_offset < PAGE4K(8)))
	{
		const int raw_offset = tiles_offset_to_raw(spr_offset);
		m_sprites_raw[raw_offset] = data >> 4;
		m_sprites_raw[raw_offset + 1] = data & 0x0f;
	}

	m_ram->pointer()[ram_addr] = data;
}

u16 tsconf_state::ram_read16(offs_t offset)
{
	return (m_ram->pointer()[offset & ~offs_t(1)] << 8) | m_ram->pointer()[offset | 1];
}

void tsconf_state::ram_write16(offs_t offset, u16 data)
{
	ram_page_write(0, offset & ~offs_t(1), data >> 8);
	ram_page_write(0, offset | 1, data & 0xff);
}

u16 tsconf_state::spi_read16()
{
	const u16 data_hi = tsconf_port_57_zctr_r() << 8;
	return data_hi | tsconf_port_57_zctr_r();
}

void tsconf_state::cram_write(u16 offset, u8 data)
{
	u16 dest = offset & 0x1ff;
	m_cram->write(dest, data);
	u8 pen = dest >> 1;
	rgb_t rgb = from_pwm((m_cram->read(dest | 1) << 8 | m_cram->read(dest & 0x1fe)));
	m_palette->set_pen_color(pen, rgb);
}

void tsconf_state::cram_write16(offs_t offset, u16 data)
{
	cram_write(offset & 0x1fe, data >> 8);
	cram_write(offset | 1, data & 0xff);
}

void tsconf_state::sfile_write16(offs_t offset, u16 data)
{
	m_sprites_cache.clear();

	u16 dest = offset & 0x1fe;
	m_sfile->write(dest, data >> 8);
	m_sfile->write(dest | 1, data & 0xff);
}

u8 tsconf_state::tsconf_port_xx1f_r(offs_t offset) {
	return m_beta->started() && m_beta->is_active()
			? m_beta->status_r()
			: 0x00; // TODO kempston read
}

void tsconf_state::tsconf_port_7ffd_w(u8 data)
{
	// LOCK? BIT(data, 5);
	u8 page3 = (m_regs[PAGE3] & ~0x07) | BIT(data, 0, 3); // 128K: 0..2 -> 0..2
	switch (BIT(m_regs[MEM_CONFIG], 6, 2))
	{
	case 4:
		page3 = (page3 & ~0x20) | (data & 0x20); // 1024K: 5 -> 5
		[[fallthrough]];
	case 0:
		page3 = (page3 & ~0x18) | (BIT(data, 6, 2) << 3); // 512K: 6..7 -> 3..4
		break;
	default:
		break;
	}
	tsconf_port_xxaf_w(PAGE3 << 8, page3);
	tsconf_port_xxaf_w(MEM_CONFIG << 8, (m_regs[MEM_CONFIG] & 0xfe) | BIT(data, 4)); // ROM128
	tsconf_port_xxaf_w(V_PAGE << 8, BIT(data, 3) ? 7 : 5);
}

void tsconf_state::tsconf_ula_w(offs_t offset, u8 data)
{
	spectrum_ula_w(offset, data);
	tsconf_port_xxaf_w(BORDER << 8, 0xf0 | (data & 0x07));
}

u8 tsconf_state::tsconf_port_xxaf_r(offs_t port)
{
	u8 nreg = port >> 8;
	u8 data = 0xff;

	switch (nreg)
	{
	case V_CONFIG:
		data = 0b01000011; // PWR_UP, !FDRVER, 5bit VDAC
		break;
	case PAGE2:
	case PAGE3:
		data = m_regs[nreg];
		break;

	case DMA_CTRL: // DMAStatus
		data = m_dma->is_ready() ? 0x00 : 0x80;
		break;

	case 0x30: // FRCnt0
	case 0x31: // FRCnt1
	case 0x32: // FRCnt2
	default:
		logerror("'tsconf': unmapped reg read %02X\n", nreg);
		break;
	}

	// LOGWARN("'tsconf': reg read %02X = %02x\n", nreg, data);
	return data;
}

void tsconf_state::copy_tiles_to_raw(const u8 *tiles_src, u8 *raw_target)
{
	for (u32 ln = 0; ln < PAGE4K(8); ln += 4)
	{
		int targ = tiles_offset_to_raw(ln);
		for (u8 x = 0; x < 4; ++x)
		{
			const u8 data = tiles_src[ln + x];
			raw_target[targ + (x << 1)] = data >> 4;
			raw_target[targ + (x << 1) + 1] = data & 0x0f;
		}
	}
}

void tsconf_state::tsconf_port_xxaf_w(offs_t port, u8 data)
{
	u8 nreg = port >> 8;

	// 1. Updates which effect is delayed till next scanline
	bool delay_update = true;
	switch (nreg)
	{
	case V_CONFIG:
	case V_PAGE:
	case G_X_OFFS_L:
	case G_X_OFFS_H:
	case G_Y_OFFS_L:
	case G_Y_OFFS_H:
	case PAL_SEL:
	case T0_G_PAGE:
	case T1_G_PAGE:
	case T0_X_OFFSET_L:
	case T0_X_OFFSET_H:
	case T1_X_OFFSET_L:
	case T1_X_OFFSET_H:
		m_scanline_delayed_regs_update[tsconf_regs(nreg)] = data;
		break;

	default:
		delay_update = false;
		break;
	}
	if (delay_update)
		return;

	bool val_changed = m_regs[nreg] != data;

	// 2. Updates which require some pre-work before change
	switch (nreg)
	{
	case BORDER:
		if (val_changed)
			m_screen->update_now();
		break;

	default:
		break;
	}

	m_regs[nreg] = data;
	// 3. Even same value must be processed
	switch (nreg)
	{
	case MEM_CONFIG:
	case PAGE0:
		tsconf_update_bank0();
		break;

	case PAGE1:
		m_bank_ram[1]->set_entry(data);
		break;

	case PAGE2:
		m_bank_ram[2]->set_entry(data);
		break;

	case PAGE3:
		m_bank_ram[3]->set_entry(data);
		break;

	case DMAS_ADDRESS_L:
		m_dma->set_saddr_l(data);
		break;

	case DMAS_ADDRESS_H:
		m_dma->set_saddr_h(data);
		break;

	case DMAS_ADDRESS_X:
		m_dma->set_saddr_x(data);
		break;

	case DMAD_ADDRESS_L:
		m_dma->set_daddr_l(data);
		break;

	case DMAD_ADDRESS_H:
		m_dma->set_daddr_h(data);
		break;

	case DMAD_ADDRESS_X:
		m_dma->set_daddr_x(data);
		break;

	case DMA_LEN:
		m_dma->set_block_len(data);
		break;

	case DMA_NUM_L:
		m_dma->set_block_num_l(data);
		break;

	case DMA_NUM_H:
		m_dma->set_block_num_h(data);
		break;

	case DMA_CTRL:
		m_dma->start_tx(((BIT(data, 7) << 3) | (data & 0x07)), BIT(data, 5), BIT(data, 4), BIT(data, 3));
		break;

	default:
		break;
	}

	// 3. Can skip update if value not changed
	if (!val_changed)
		return;

	switch (nreg)
	{
	case T_MAP_PAGE:
		m_ts_tilemap[TM_TILES0]->mark_all_dirty();
		m_ts_tilemap[TM_TILES1]->mark_all_dirty();
		break;

	case T0_Y_OFFSET_L:
	case T0_Y_OFFSET_H:
		m_ts_tilemap[TM_TILES0]->set_scrolly(OFFS_512(T0_Y_OFFSET_L));
		break;

	case T1_Y_OFFSET_L:
	case T1_Y_OFFSET_H:
		m_ts_tilemap[TM_TILES1]->set_scrolly(OFFS_512(T1_Y_OFFSET_L));
		break;

	case SG_PAGE:
		copy_tiles_to_raw(m_ram->pointer() + PAGE4K(data & 0xf8), m_sprites_raw.target());
		break;

	case SYS_CONFIG:
		// 0 - 3.5MHz, 1 - 7MHz, 2 - 14MHz, 3 - reserved
		m_maincpu->set_clock_scale(1 << (data & 0x03));
		m_regs[CACHE_CONFIG] = BIT(data, 2) ? 0x0f : 0x00;
		break;

	case HS_INT:
	case VS_INT_L:
	case VS_INT_H:
		update_frame_timer();
		break;

	case FMAPS:
	case TS_CONFIG:
	case INT_MASK:
	// TODO
	case FDD_VIRT:
	case CACHE_CONFIG:
		break;

	default:
		break;
	}
}

u8 tsconf_state::tsconf_port_f7_r(offs_t offset)
{
	// BFF7
	return  (m_port_f7_ext == PS2KEYBOARDS_LOG && m_glukrs->address_r() == 0xf0)
			? m_keyboard->read()
			: m_glukrs->data_r();
}

void tsconf_state::tsconf_port_f7_w(offs_t offset, u8 data)
{
	auto m_l = offset >> 12;
	if (m_l == 6) // EF
	{
		m_glukrs->disable();
		if (BIT(data, 7))
		{
			m_glukrs->enable();
			m_port_f7_ext = CONF_VERSION;
		}
	}
	else if (m_glukrs->is_active())
	{
		if (m_l == 5) // DF
		{
			// 0x0E..0xEF
			m_glukrs->address_w(data);
		}
		else if (m_l == 3) // BF
		{
			if (m_glukrs->address_r() == 0xf0)
			{
				u8 m_fx[0xf] = {0xff};
				m_port_f7_ext = gluk_ext(data);
				switch (m_port_f7_ext)
				{
				case CONF_VERSION:
				{
					strcpy((char *)m_fx, "M.A.M.E.");
					PAIR16 m_ver;
					m_ver.w = ((22 << 9) | (02 << 5) | 8); // y.m.d
					m_fx[0x0c] = m_ver.b.l;
					m_fx[0x0d] = m_ver.b.h;
					break;
				}
				case BOOTLOADER_VERSION:
				case PS2KEYBOARDS_LOG:
					break;
				default:
					logerror("Gluk extention not supported %x\n", m_port_f7_ext);
					break;
				}
				for (u8 i = 0; i < 0xf; i++)
				{
					m_glukrs->address_w(0xf0 + i);
					m_glukrs->data_w(m_fx[i]);
				}
				m_glukrs->address_w(0xf0);
			}
			else
			{
				m_glukrs->data_w(data);
			}
		}
	}
}

void tsconf_state::tsconf_port_77_zctr_w(u8 data)
{
	m_sdcard->spi_ss_w(BIT(data, 0));
	m_zctl_cs = BIT(data, 1);
}

u8 tsconf_state::tsconf_port_77_zctr_r()
{
	return 0x02 | (m_sdcard->get_card_present() ? 0x00 : 0x01);
}

void tsconf_state::tsconf_port_57_zctr_w(u8 data)
{
	if (!m_zctl_cs)
	{
		for (u8 m = 0x80; m; m >>= 1)
		{
			m_sdcard->spi_mosi_w(data & m ? 1 : 0);
			m_sdcard->spi_clock_w(CLEAR_LINE); // 0-S R
			m_sdcard->spi_clock_w(ASSERT_LINE); // 1-L W
		}
	}
}

u8 tsconf_state::tsconf_port_57_zctr_r()
{
	if (m_zctl_cs)
		return 0xff;

	u8 data = m_zctl_di;
	if (!machine().side_effects_disabled())
		tsconf_port_57_zctr_w(0xff);

	return data;
}

void tsconf_state::tsconf_spi_miso_w(u8 data)
{
	m_zctl_di <<= 1;
	m_zctl_di |= data;
}

void tsconf_state::tsconf_ay_address_w(u8 data)
{
	if ((m_mod_ay->read() == 1) && ((data & 0xfe) == 0xfe))
		m_ay_selected = data & 1;
	else
		m_ay[m_ay_selected]->address_w(data);
}

IRQ_CALLBACK_MEMBER(tsconf_state::irq_vector)
{
	u8 vector = 0xff;
	if (m_int_mask & 1)
	{
		m_int_mask &= ~1;
	}
	else if (m_int_mask & 2)
	{
		m_int_mask &= ~2;
		vector = 0xfd;
	}
	else if (m_int_mask & 4)
	{
		m_int_mask &= ~4;
		vector = 0xfb;
	}

	if (!m_int_mask)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

	return vector;
}

TIMER_CALLBACK_MEMBER(tsconf_state::irq_off)
{
	m_int_mask &= ~1;
}

void tsconf_state::update_frame_timer()
{
	u16 vpos = OFFS_512(VS_INT_L);
	u16 hpos = m_regs[HS_INT];
	attotime next;
	if (vpos <= 319 && hpos <= 223)
	{
		next = m_screen->time_until_pos(vpos, hpos << 1);
		if (next >= m_screen->frame_period())
		{
			next = attotime::zero;
		}
	}
	else
	{
		next = attotime::never;
	}

	m_frame_irq_timer->adjust(next);
}

INTERRUPT_GEN_MEMBER(tsconf_state::tsconf_vblank_interrupt)
{
	update_frame_timer();
	m_gfx_y_frame_offset = -get_screen_area().top();
	m_scanline_irq_timer->adjust(attotime::zero);
}

void tsconf_state::dma_ready(int line)
{
	if (BIT(m_regs[INT_MASK], 2))
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_int_mask |= 4;
	}
}

TIMER_CALLBACK_MEMBER(tsconf_state::irq_frame)
{
	if (BIT(m_regs[INT_MASK], 0))
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_irq_off_timer->adjust(attotime::from_ticks(32, m_maincpu->unscaled_clock()));
		m_int_mask |= 1;
	}
}

TIMER_CALLBACK_MEMBER(tsconf_state::irq_scanline)
{
	if (BIT(m_regs[INT_MASK], 1))
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_int_mask |= 2;
	}

	u16 screen_vpos = m_screen->vpos();
	m_scanline_irq_timer->adjust(m_screen->time_until_pos(screen_vpos + 1));
	if (!m_scanline_delayed_regs_update.empty())
		m_screen->update_now();
	for (const auto &[reg, val] : m_scanline_delayed_regs_update)
	{
		m_regs[reg] = val;
		switch (reg)
		{
		case V_CONFIG:
		case V_PAGE:
			tsconf_update_video_mode();
			break;

		case G_Y_OFFS_L:
		case G_Y_OFFS_H:
			m_gfx_y_frame_offset = screen_vpos < get_screen_area().top()
					? -get_screen_area().top()
					: -screen_vpos;
			break;

		case T0_G_PAGE:
			copy_tiles_to_raw(m_ram->pointer() + PAGE4K(val & 0xf8), m_tiles_raw[0].target());
			m_ts_tilemap[TM_TILES0]->mark_all_dirty();
			break;

		case T1_G_PAGE:
			copy_tiles_to_raw(m_ram->pointer() + PAGE4K(val & 0xf8), m_tiles_raw[1].target());
			m_ts_tilemap[TM_TILES1]->mark_all_dirty();
			break;

		case T0_X_OFFSET_L:
		case T0_X_OFFSET_H:
			m_ts_tilemap[TM_TILES0]->set_scrollx(OFFS_512(T0_X_OFFSET_L));
			break;

		case T1_X_OFFSET_L:
		case T1_X_OFFSET_H:
			m_ts_tilemap[TM_TILES1]->set_scrollx(OFFS_512(T1_X_OFFSET_L));
			break;

		case G_X_OFFS_L:
		case G_X_OFFS_H:
		case PAL_SEL:
		default:
			break;
		}
	}
	m_scanline_delayed_regs_update.clear();
}

u8 tsconf_state::beta_neutral_r(offs_t offset)
{
	return m_program.read_byte(offset);
}

u8 tsconf_state::beta_enable_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (!W0_RAM && m_bank_rom[0]->entry() == 3)
		{
			if (m_beta->started() && !m_beta->is_active())
			{
				m_beta->enable();
				tsconf_update_bank0();
			}
		}
	}
	return m_program.read_byte(offset + 0x3d00);
}

u8 tsconf_state::beta_disable_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_beta->started() && m_beta->is_active())
		{
			m_beta->disable();
			tsconf_update_bank0();
		}
	}
	return m_program.read_byte(offset + 0x4000);
}
