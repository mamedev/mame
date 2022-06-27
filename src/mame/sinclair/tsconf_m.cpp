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

#define VM static_cast<v_mode>(BIT(m_regs[V_CONFIG], 0, 2))

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

static constexpr u32 tmp_tile_oversized_to_code(u16 code)
{
	return code / 64 * 64 * 8 + (code % 64);
}

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

void tsconf_state::tsconf_palette(palette_device &palette) const
{
	rgb_t colors[256] = {0};
	palette.set_pen_colors(0, colors);
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
		m_banks[0]->set_entry(page0);
		m_bank0_rom.disable();
	}
	else
	{
		m_banks[4]->set_entry(page0 & 0x1f);
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
void tsconf_state::spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_regs[V_CONFIG], 5))
	{
		if (VM == VM_ZX)
			tsconf_UpdateZxScreenBitmap(screen, bitmap, cliprect);
		else if (VM == VM_TXT)
			tsconf_UpdateTxtBitmap(bitmap, cliprect);
		else
			tsconf_UpdateGfxBitmap(bitmap, cliprect);
	}
	else
	{
		bitmap.fill(get_border_color(), cliprect);
	}

	if (!BIT(m_regs[V_CONFIG], 4))
	{
		screen.priority().fill(0, cliprect);
		if (BIT(m_regs[TS_CONFIG], 5))
			m_ts_tilemap[TM_TILES0]->draw(screen, bitmap, cliprect,
										  BIT(m_regs[TS_CONFIG], 2) ? TILEMAP_DRAW_ALL_CATEGORIES : TILEMAP_DRAW_CATEGORY(1), 1);

		if (BIT(m_regs[TS_CONFIG], 6))
			m_ts_tilemap[TM_TILES1]->draw(screen, bitmap, cliprect,
										  BIT(m_regs[TS_CONFIG], 3) ? TILEMAP_DRAW_ALL_CATEGORIES : TILEMAP_DRAW_CATEGORY(1), 2);

		if (BIT(m_regs[TS_CONFIG], 7))
		{
			// draw_sprites(screen, bitmap, cliprect);
			//  Avoid frequent expensive updates for now. Currently once per frame
			if (cliprect.bottom() == get_screen_area().bottom() && cliprect.right() == get_screen_area().right())
				draw_sprites(screen, bitmap, get_screen_area());
		}
	}
}

void tsconf_state::tsconf_UpdateZxScreenBitmap(screen_device &screen_d, bitmap_ind16 &bitmap, const rectangle &screen)
{
	u8 pal_offset = m_regs[PAL_SEL] << 4;
	u8 *screen_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]);
	u8 *attrs_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]) + 0x1800;
	bool invert_attrs = u64(screen_d.frame_number() / m_frame_invert_count) & 1;
	for (u16 vpos = screen.top(); vpos <= screen.bottom(); vpos++)
	{
		u16 hpos = screen.left();
		u16 x = hpos - get_screen_area().left();
		u16 y = vpos - get_screen_area().top();
		u8 *scr = &screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		u8 *attr = &attrs_location[((y & 0xf8) << 2) | (x >> 3)];
		u16 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= screen.right())
		{
			u16 ink = pal_offset | ((*attr >> 3) & 0x08) | (*attr & 0x07);
			u16 pap = pal_offset | ((*attr >> 3) & 0x0f);
			u8 pix8 = (invert_attrs && (*attr & 0x80)) ? ~*scr : *scr;

			for (u8 b = 0x80 >> (x & 0x07); b != 0 && hpos <= screen.right(); b >>= 1, x++, hpos++)
				*pix++ = (pix8 & b) ? ink : pap;
			scr++;
			attr++;
		}
	}
}

void tsconf_state::tsconf_UpdateTxtBitmap(bitmap_ind16 &bitmap, const rectangle &screen)
{
	u8 *font_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE] ^ 0x01);
	u8 pal_offset = m_regs[PAL_SEL] << 4;
	for (u16 vpos = screen.top(); vpos <= screen.bottom(); vpos++)
	{
		u16 hpos = screen.left();
		u16 x = hpos - get_screen_area().left();
		u16 y = vpos - get_screen_area().top();
		u16 y_offset = (OFFS_512(G_Y_OFFS_L) + y) & 0x1ff;

		// TODO? u16 x_offset = OFFS_512(G_X_OFFS_L);
		u8 *text_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]) + (y_offset / 8 * 256 + x / 8);
		u16 *pix = &(bitmap.pix(vpos, hpos));
		while (hpos <= screen.right())
		{
			u8 font_color = *(text_location + 128) & 0x0f;
			u8 bg_color = (*(text_location + 128) & 0xf0) >> 4;
			u8 char_x = *(font_location + (*text_location * 8) + (y_offset % 8));
			for (u8 b = 0x80 >> (x & 0x07); b != 0 && hpos <= screen.right(); b >>= 1, x++, hpos++)
				*pix++ = pal_offset | ((char_x & b) ? font_color : bg_color);
			text_location++;
		}
	}
}

void tsconf_state::tsconf_UpdateGfxBitmap(bitmap_ind16 &bitmap, const rectangle &screen)
{
	u8 pal_offset = m_regs[PAL_SEL] << 4;
	for (u16 vpos = screen.top(); vpos <= screen.bottom(); vpos++)
	{
		u16 y_offset = (0x200 + OFFS_512(G_Y_OFFS_L) + m_gfx_y_frame_offset + vpos) & 0x1ff;
		u16 x_offset = (OFFS_512(G_X_OFFS_L) + (screen.left() - get_screen_area().left())) & 0x1ff;
		u8 *video_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]) + ((y_offset * 512 + x_offset) >> (2 - VM));
		u16 *bm = &(bitmap.pix(vpos, screen.left()));
		s16 width = screen.width();
		if (VM == VM_16C)
		{
			if (x_offset & 1)
			{
				*bm++ = pal_offset | (*video_location++ & 0x0f);
				x_offset++;
				width--;
			}
			for (; width > 0; width -= 2, x_offset += 2)
			{
				if (x_offset == 512)
					video_location -= 256;
				u8 pix = *video_location++;
				*bm++ = pal_offset | (pix >> 4);
				if (width != 1)
					*bm++ = pal_offset | (pix & 0x0f);
			}
		}
		else // VM_256C
		{
			for (; width > 0; width--, x_offset++)
			{
				if (x_offset == 512)
					video_location -= 512;
				*bm++ = *video_location++;
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
void tsconf_state::draw_sprites(screen_device &screen_d, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle screen = get_screen_area();

	u8 layer = 0;
	u8 *sinfo = m_sfile->pointer() + 1;
	u8 sid = 1;
	// Higher Sprite draws on top. Prepare to iterate backwards.
	for (; sid < 85 && layer < 3; sid++)
	{
		if (BIT(*sinfo, 6))
			layer++;
		sinfo += 6;
	}
	sinfo -= 1;

	for (; sid; sid--)
	{
		s16 y = *sinfo++;
		y |= BIT(*sinfo, 0) << 8;
		y += screen.top() - (y >= screen.height() ? 512 : 0);
		u8 height8 = BIT(*sinfo, 1, 3);
		layer -= BIT(*sinfo, 6);
		if (!BIT(*sinfo, 5))
		{
			// sprite disabled -> move to previous
			sinfo -= 7;
		}
		else
		{
			bool flipy = BIT(*sinfo++, 7);
			s16 x = *sinfo++;
			x |= BIT(*sinfo, 0) << 8;
			x += screen.left() - (x >= screen.width() ? 512 : 0);
			u8 width8 = BIT(*sinfo, 1, 3);
			bool flipx = BIT(*sinfo++, 7);
			u16 code = *sinfo++;
			code |= BIT(*sinfo, 0, 4) << 8;
			u8 pal = BIT(*sinfo, 4, 4);
			sinfo -= 11;

			u32 pmask = layer ? (layer == 1 ? GFX_PMASK_2 : 0) : (GFX_PMASK_1 | GFX_PMASK_2);

			u8 tile_row = code / 64 + flipy * height8;
			for (auto iy = y; iy <= y + height8 * 8; iy = iy + 8)
			{
				u8 tile_col = (code % 64) + flipx * width8;
				for (auto ix = x; ix <= x + width8 * 8; ix = ix + 8)
				{
					m_gfxdecode->gfx(TM_SPRITES)->prio_transpen(bitmap, cliprect, tmp_tile_oversized_to_code((tile_row % 64) * 64 + (tile_col % 64)), pal, flipx, flipy, ix, iy, screen_d.priority(), pmask, 0);
					tile_col += flipx ? -1 : 1;
				}
				tile_row += flipy ? -1 : 1;
			}
		}
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
				m_sfile->write(addr_w - 512, data);
			else
				tsconf_port_xxaf_w((addr_w - 1024) << 8, data);
		}
	}

	if (bank > 0 || (W0_WE && W0_RAM))
		ram_page_write(m_regs[PAGE0 + bank], offset, data);
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
		if (ram_addr >= PAGE4K(m_regs[T0_G_PAGE]) && ram_addr < PAGE4K(m_regs[T0_G_PAGE] + 8))
			m_gfxdecode->gfx(TM_TILES0)->mark_all_dirty();

		if (ram_addr >= PAGE4K(m_regs[T1_G_PAGE]) && ram_addr < PAGE4K(m_regs[T1_G_PAGE] + 8))
			m_gfxdecode->gfx(TM_TILES1)->mark_all_dirty();
	}

	if (ram_addr >= PAGE4K(m_regs[V_PAGE]) && ram_addr < PAGE4K(m_regs[V_PAGE] + 1))
		m_ts_tilemap[TM_TS_CHAR]->mark_all_dirty();

	if (ram_addr >= PAGE4K(m_regs[m_regs[V_PAGE] ^ 0x01]) && ram_addr < PAGE4K(m_regs[m_regs[V_PAGE] ^ 0x01] + 1))
		m_gfxdecode->gfx(TM_TS_CHAR)->mark_all_dirty();

	if (ram_addr >= PAGE4K(m_regs[SG_PAGE]) && ram_addr < PAGE4K(m_regs[SG_PAGE] + 8))
		m_gfxdecode->gfx(TM_SPRITES)->mark_all_dirty();

	m_ram->write(ram_addr, data);
}

u16 tsconf_state::ram_read16(offs_t offset)
{
	return ((m_ram->read(offset & 0xfffffffe)) << 8) | m_ram->read(offset | 1);
}

void tsconf_state::ram_write16(offs_t offset, u16 data)
{
	ram_page_write(0, offset & 0xfffffffe, data >> 8);
	ram_page_write(0, offset | 1, data & 0xff);
}

u16 tsconf_state::spi_read16()
{
	return (tsconf_port_57_zctr_r(0) << 8) | tsconf_port_57_zctr_r(0);
}

void tsconf_state::cram_write(u16 offset, u8 data)
{
	u16 dest = offset & 0x1ff;
	m_cram->write(dest, data);
	u8 pen = dest >> 1;
	rgb_t rgb = from_pwm((m_cram->read(dest | 1) << 8 | m_cram->read(dest & 0x1fe)));
	m_palette->set_pen_color(pen, rgb);
};

void tsconf_state::cram_write16(offs_t offset, u16 data)
{
	cram_write(offset & 0xfffe, data >> 8);
	cram_write(offset | 1, data & 0xff);
};

void tsconf_state::sfile_write16(offs_t offset, u16 data)
{
	u16 dest = offset & 0x1fe;
	m_sfile->write(dest, data >> 8);
	m_sfile->write(dest | 1, data & 0xff);
};

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
	tsconf_port_xxaf_w(BORDER << 8, (data & 0x07) | (m_regs[PAL_SEL] << 4));
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

void tsconf_state::tsconf_port_xxaf_w(offs_t port, u8 data)
{
	u8 nreg = port >> 8;

	// 1. Updates which effect is delayed till next scanline
	bool delay_update = true;
	switch (nreg)
	{
	// more registers which marked as *1 in the xls, but rest need to be tested
	case G_X_OFFS_L:
	case G_X_OFFS_H:
	case G_Y_OFFS_L:
	case G_Y_OFFS_H:
		m_scanline_delayed_regs_update[static_cast<tsconf_regs>(nreg)] = data;
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
		m_banks[1]->set_entry(data);
		break;

	case PAGE2:
		m_banks[2]->set_entry(data);
		break;

	case PAGE3:
		m_banks[3]->set_entry(data);
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
	case V_CONFIG:
	case V_PAGE:
		tsconf_update_video_mode();
		break;

	case T_MAP_PAGE:
		m_ts_tilemap[TM_TILES0]->mark_all_dirty();
		m_ts_tilemap[TM_TILES1]->mark_all_dirty();
		break;

	case T0_G_PAGE:
		m_gfxdecode->gfx(TM_TILES0)->set_source(m_ram->pointer() + PAGE4K(data));
		break;

	case T0_X_OFFSET_L:
	case T0_X_OFFSET_H:
		m_ts_tilemap[TM_TILES0]->set_scrollx(OFFS_512(T0_X_OFFSET_L));
		break;

	case T0_Y_OFFSET_L:
	case T0_Y_OFFSET_H:
		m_ts_tilemap[TM_TILES0]->set_scrolly(OFFS_512(T0_Y_OFFSET_L));
		break;

	case T1_G_PAGE:
		m_gfxdecode->gfx(TM_TILES1)->set_source(m_ram->pointer() + PAGE4K(data));
		break;

	case T1_X_OFFSET_L:
	case T1_X_OFFSET_H:
		m_ts_tilemap[TM_TILES1]->set_scrollx(OFFS_512(T1_X_OFFSET_L));
		break;

	case T1_Y_OFFSET_L:
	case T1_Y_OFFSET_H:
		m_ts_tilemap[TM_TILES1]->set_scrolly(OFFS_512(T1_Y_OFFSET_L));
		break;

	case SG_PAGE:
		m_gfxdecode->gfx(TM_SPRITES)->set_source(m_ram->pointer() + PAGE4K(data));
		break;

	case SYS_CONFIG:
		// 0 - 3.5MHz, 1 - 7MHz, 2 - 14MHz, 3 - reserved
		m_maincpu->set_clock(3.5_MHz_XTAL * (1 << (data & 0x03)));
		m_regs[CACHE_CONFIG] = BIT(data, 2) ? 0x0f : 0x00;
		break;

	case HS_INT:
	case VS_INT_L:
	case VS_INT_H:
		update_frame_timer();
		break;

	case FMAPS:
	case PAL_SEL:
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
	u8 data = 0xff;
	if (m_port_f7_ext == PS2KEYBOARDS_LOG && m_port_f7_gluk_reg == 0xf0)
		data = m_keyboard->read();
	else if (m_port_f7_ext != DISABLED)
		data = m_glukrs->read(m_port_f7_gluk_reg);

	return data;
}

void tsconf_state::tsconf_port_f7_w(offs_t offset, u8 data)
{
	auto m_l = offset >> 12;
	if (m_l == 6) // EF
	{
		m_port_f7_ext = (data & 0x80) ? CONF_VERSION : DISABLED;
	}
	else if (m_port_f7_ext != DISABLED)
	{
		if (m_l == 5) // DF
		{
			// 0x0E..0xEF
			m_port_f7_gluk_reg = data;
		}
		else if (m_l == 3) // BF
		{
			if (m_port_f7_gluk_reg == 0xf0)
			{
				u8 m_fx[0xf] = {0xff};
				m_port_f7_ext = static_cast<gluk_ext>(data);
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
					logerror("Gluk extention not supported %x\n", m_port_f7_gluk_reg);
					break;
				}
				for (u8 i = 0; i < 0xf; i++)
				{
					m_glukrs->write(0xf0 + i, m_fx[i]);
				}
			}
			else
			{
				m_glukrs->write(m_port_f7_gluk_reg, data);
			}
		}
	}
}

void tsconf_state::tsconf_port_77_zctr_w(offs_t port, u8 data)
{
	m_sdcard->spi_ss_w(BIT(data, 0));
	m_zctl_cs = BIT(data, 1);
}

u8 tsconf_state::tsconf_port_77_zctr_r(offs_t port)
{
	return 0x02 | !m_sdcard->get_card_present();
}

void tsconf_state::tsconf_port_57_zctr_w(offs_t port, u8 data)
{
	if (!m_zctl_cs)
	{
		for (u8 m = 0x80; m; m >>= 1)
		{
			m_sdcard->spi_clock_w(CLEAR_LINE); // 0-S R
			m_sdcard->spi_mosi_w(data & m ? 1 : 0);
			m_sdcard->spi_clock_w(ASSERT_LINE); // 1-L W
		}
	}
}

u8 tsconf_state::tsconf_port_57_zctr_r(offs_t port)
{
	tsconf_port_57_zctr_w(0, 0xff);
	return m_zctl_cs ? 0xff : m_zctl_di;
}

void tsconf_state::tsconf_spi_miso_w(u8 data)
{
	m_zctl_di <<= 1;
	m_zctl_di |= data;
}

void tsconf_state::update_frame_timer()
{
	u16 vpos = OFFS_512(VS_INT_L);
	u16 hpos = m_regs[HS_INT];
	if (hpos > 0 && vpos <= 319 && hpos <= 223)
		// Only if not overlapping with scanline. Otherwise we need to prioritize.
		m_frame_irq_timer->adjust(m_screen->time_until_pos(vpos, hpos << 1));
	else
		m_frame_irq_timer->adjust(attotime::never);

	m_gfx_y_frame_offset = -get_screen_area().top();
}

INTERRUPT_GEN_MEMBER(tsconf_state::tsconf_vblank_interrupt)
{
	update_frame_timer();
	m_line_irq_timer->adjust(attotime::zero);
}

void tsconf_state::dma_ready(int line)
{
	if (BIT(m_regs[INT_MASK], 4))
	{
		m_maincpu->set_input_line_and_vector(line, ASSERT_LINE, 0xfb);
		m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32 * (1 << (m_regs[SYS_CONFIG] & 0x03))));
	}
}

TIMER_CALLBACK_MEMBER(tsconf_state::irq_frame)
{
	if (BIT(m_regs[INT_MASK], 0))
	{
		m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xff);
		m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32 * (1 << (m_regs[SYS_CONFIG] & 0x03))));
	}
}

TIMER_CALLBACK_MEMBER(tsconf_state::irq_scanline)
{
	u16 screen_vpos = m_screen->vpos();
	m_line_irq_timer->adjust(m_screen->time_until_pos(screen_vpos + 1));
	if (BIT(m_regs[INT_MASK], 1))
	{
		m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xfd);
		// Not quite precise. Scanline can't be skipped.
		m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32 * (1 << (m_regs[SYS_CONFIG] & 0x03))));
	}
	if (BIT(m_regs[INT_MASK], 0) && OFFS_512(VS_INT_L) == screen_vpos && m_regs[HS_INT] == 0)
	{
		m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xff);
		m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32 * (1 << (m_regs[SYS_CONFIG] & 0x03))));
	}

	m_screen->update_now();
	for (const auto &[reg, val] : m_scanline_delayed_regs_update)
	{
		m_regs[reg] = val;
		switch (reg)
		{
		case G_Y_OFFS_L:
		case G_Y_OFFS_H:
			m_gfx_y_frame_offset = screen_vpos < get_screen_area().top()
				? -get_screen_area().top()
				: -screen_vpos;
			break;

		default:
			break;
		}
	}
	m_scanline_delayed_regs_update.clear();
}

u8 tsconf_state::beta_neutral_r(offs_t offset)
{
	return m_program->read_byte(offset);
}

u8 tsconf_state::beta_enable_r(offs_t offset)
{
	if (!W0_RAM && m_banks[4]->entry() == 3)
	{
		if (m_beta->started() /*&& !m_beta->is_active()*/)
		{
			m_beta->enable();
			tsconf_update_bank0();
		}
	}

	return m_program->read_byte(offset + 0x3d00);
}

u8 tsconf_state::beta_disable_r(offs_t offset)
{
	if (m_beta->started() && m_beta->is_active())
	{
		m_beta->disable();
		tsconf_update_bank0();
	}

	return m_program->read_byte(offset + 0x4000);
}
