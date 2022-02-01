// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "includes/tsconf.h"

#define PAGE4K(_r) ((_r) << 14)

#define W0_RAM (BIT(m_regs[MEM_CONFIG], 3))
#define NW0_MAP (BIT(m_regs[MEM_CONFIG], 2))
#define W0_WE (BIT(m_regs[MEM_CONFIG], 1))
#define ROM128 (BIT(m_regs[MEM_CONFIG], 0))

#define VM static_cast<v_mode>(BIT(m_regs[V_CONFIG], 0, 2))

static constexpr rectangle screen_area[4] = {
	rectangle(TSCONF_SCREEN_HBLANK + 52, TSCONF_SCREEN_HBLANK + 256 + 51, TSCONF_SCREEN_VBLANK + 48, TSCONF_SCREEN_VBLANK + 192 + 47), // 52|256|52 x 48-192-48
	rectangle(TSCONF_SCREEN_HBLANK + 20, TSCONF_SCREEN_HBLANK + 320 + 19, TSCONF_SCREEN_VBLANK + 44, TSCONF_SCREEN_VBLANK + 200 + 43), // 20|320|20 x 44-200-44
	rectangle(TSCONF_SCREEN_HBLANK + 20, TSCONF_SCREEN_HBLANK + 320 + 19, TSCONF_SCREEN_VBLANK + 24, TSCONF_SCREEN_VBLANK + 240 + 23), // 20|320|20 x 24-240-24
	rectangle(TSCONF_SCREEN_HBLANK + 00, TSCONF_SCREEN_HBLANK + 360 - 01, TSCONF_SCREEN_VBLANK + 00, TSCONF_SCREEN_VBLANK + 288 - 01)  // 00|360|00 x 00-288-00
};

enum v_mode : u8
{
	VM_ZX = 0,
	VM_16C,
	VM_256C,
	VM_TXT
};

static constexpr u16 tmp_tile_oversized_to_code(u16 code)
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
	{
		info.set_width(info.width() << 1);
	}
	return info;
}

void tsconf_state::tsconf_palette(palette_device &palette) const
{
	rgb_t colors[256] = {0};
	palette.set_pen_colors(0, colors);
}

void tsconf_state::tsconf_update_bank0()
{
	if (NW0_MAP)
	{
		m_ROMSelection = m_regs[PAGE0];
	}
	else
	{
		/* ROM: 0-SYS, 1-DOS, 2-128, 3-48 */
		m_ROMSelection = m_beta->started() && m_beta->is_active() ? ROM128 : (0x02 | ROM128);
		m_ROMSelection |= (m_regs[PAGE0] & 0xfc);
	}

	if (W0_RAM)
	{
		m_banks[0]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);
		m_banks[0]->set_entry(m_ROMSelection);
	}
	else
	{
		memory_region *rom = memregion("maincpu");
		u8 rom_banks = (rom->bytes() - 0x10000) / 0x4000;
		m_banks[0]->configure_entries(0, rom_banks, rom->base() + 0x10000, 0x4000);
		m_banks[0]->set_entry(m_ROMSelection & 0x1f);
	}
}

void tsconf_state::tsconf_update_video_mode()
{
	rectangle visarea = screen_area[3];
	u8 *messram = m_ram->pointer();
	switch (VM)
	{
	case VM_TXT: // Text Mode
	{
		// scale screen for Text Mode
		rectangle origin_screen = screen_area[BIT(m_regs[V_CONFIG], 6, 2)];
		visarea.set_width(visarea.width() + origin_screen.width());
		m_gfxdecode->gfx(TM_TS_CHAR)->set_source(messram + PAGE4K(m_regs[V_PAGE] ^ 0x01));
		m_ts_tilemap[TM_TS_CHAR]->mark_all_dirty();
		break;
	}
	case VM_ZX: // Zx
	{
		m_screen_location = messram + ((m_port_7ffd_data & 8) ? PAGE4K(7) : PAGE4K(5));
		break;
	}
	default:
		break;
	}

	m_ts_tilemap[TM_TILES0]->set_scrolldx(get_screen_area().left(), 0);
	m_ts_tilemap[TM_TILES0]->set_scrolldy(get_screen_area().top(), 0);
	m_ts_tilemap[TM_TILES1]->set_scrolldx(get_screen_area().left(), 0);
	m_ts_tilemap[TM_TILES1]->set_scrolldy(get_screen_area().top(), 0);

	m_screen->configure(visarea.max_x + 1, visarea.max_y + 1, visarea, HZ_TO_ATTOSECONDS(50));
}

uint32_t tsconf_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void to_display(rectangle area, unsigned int &x, unsigned int &y)
{
	if (y < area.top() || y > area.bottom())
	{
		x = area.left();
		y = area.top();
	}
	else if (x < area.left())
	{
		x = area.left();
	}
	else if (x > area.right())
	{
		x = area.left();
		y++;
		to_display(area, x, y);
	}
}

void tsconf_state::spectrum_UpdateZxScreenBitmap(bool eof)
{
	unsigned int to_x = m_screen->hpos();
	unsigned int to_y = m_screen->vpos();
	to_display(m_screen->visible_area(), to_x, to_y);

	if ((m_previous_screen_x == to_x) && (m_previous_screen_y == to_y) && !eof)
		return;

	bitmap_ind16 *bm = &m_screen->curbitmap().as_ind16();
	if (bm->valid())
	{
		u16 border_color = get_border_color();
		bool border_only = VM != VM_ZX || BIT(m_regs[V_CONFIG], 5);
		do
		{
			rectangle screen = get_screen_area();
			u16 x = m_previous_screen_x - screen.left();
			u16 y = m_previous_screen_y - screen.top();

			if (screen.contains(m_previous_screen_x, m_previous_screen_y) && !border_only)
			{
				if ((x & 7) == 0)
				{
					u16 *pix = &bm->pix(m_previous_screen_y, m_previous_screen_x);
					u8 attr = *(m_screen_location + ((y & 0xF8) << 2) + (x >> 3) + 0x1800);
					u8 scr = *(m_screen_location + ((y & 7) << 8) + ((y & 0x38) << 2) + ((y & 0xC0) << 5) + (x >> 3));
					u16 ink = ((attr & 0x07) + ((attr >> 3) & 0x08)) | 0xf0;
					u16 pap = ((attr >> 3) & 0x0f) | 0xf0;

					if (m_flash_invert && (attr & 0x80))
						scr = ~scr;

					for (uint8_t b = 0x80; b != 0; b >>= 1)
						*pix++ = (scr & b) ? ink : pap;
				}
			}
			else
			{
				bm->pix(m_previous_screen_y, m_previous_screen_x) = border_color;
			}

			to_display(m_screen->visible_area(), ++m_previous_screen_x, m_previous_screen_y);
		} while (!((m_previous_screen_x == to_x) && (m_previous_screen_y == to_y)));
	}
}

/*
Layered as:
 + Border
 + Graphics
 + Sprites 0
 + Tiles 0
 + Sprites 1
 + Tiles 1
 + Sprites 2
*/
void tsconf_state::spectrum_UpdateScreenBitmap(bool eof)
{
	rectangle screen = get_screen_area();
	s16 y = m_previous_screen_y - screen.top();

	spectrum_UpdateZxScreenBitmap(eof);
	if (!BIT(m_regs[V_CONFIG], 5) && VM != VM_ZX)
	{
		//TODO consider perfom update not based on current line but from previous saved.
		if (y == 0)
			m_rendering_gfx_y_offset = ((m_regs[G_Y_OFFS_H] & 1) << 8) + m_regs[G_Y_OFFS_L];

		if (y >= 0 && y < screen.height())
		{
			//assert(!m_screen->vblank()); // possibly something wrong with vblank() - occasionally 32 reported as blank!
			u8 pal_offset = m_regs[PAL_SEL] << 4;
			u16 *bm = &m_screen->curbitmap().as_ind16().pix(y + screen.top(), screen.left());
			if (VM == VM_TXT)
			{
				// TODO u16 x_offset = ((m_regs[G_X_OFFS_H] & 1) << 8) + m_regs[G_X_OFFS_L];
				u8 *font_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE] ^ 0x01);
				u8 *text_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]) + (m_rendering_gfx_y_offset / 8 * 256);
				for (auto x = 0; x < screen.width() / 8; x++)
				{
					u8 char_x = *(font_location + (*text_location * 8) + (m_rendering_gfx_y_offset % 8));
					u8 font_color = *(text_location + 128) & 0x0f;
					u8 bg_color = (*(text_location + 128) & 0xf0) >> 4;
					for (auto i = 7; i >= 0; i--)
					{
						*bm++ = (BIT(char_x, i) ? font_color : bg_color) + pal_offset;
					}
					text_location++;
				}
			}
			else
			{
				u16 x_offset = ((m_regs[G_X_OFFS_H] & 1) << 8) + m_regs[G_X_OFFS_L];
				u8 *video_location = m_ram->pointer() + PAGE4K(m_regs[V_PAGE]) + ((m_rendering_gfx_y_offset * 512 + x_offset) >> (2 - VM));
				if (VM == VM_16C)
				{
					if (x_offset & 1)
					{
						*bm++ = (*video_location++ & 0x0f) + pal_offset;
						x_offset++;
					}
					for (auto x = 0; x < screen.width() - 1; x += 2)
					{
						if (x_offset == 512)
							video_location -= 256;
						u8 pix = *video_location++;
						*bm++ = (pix >> 4) + pal_offset;
						*bm++ = (pix & 0x0f) + pal_offset;
						x_offset += 2;
					}
				}
				else // VM_256C
				{
					for (auto x = 0; x < screen.width(); x++)
					{
						if (x_offset == 512)
							video_location -= 512;
						*bm++ = *video_location++;
						x_offset++;
					}
				}
			}
			m_rendering_gfx_y_offset = (m_rendering_gfx_y_offset + 1) & 0x1ff;
		}
	}

	if (!BIT(m_regs[V_CONFIG], 4))
	{
		if (m_screen->vpos() == m_previous_tsu_vpos)
			return;

		bool draw = false;
		if (m_screen->vpos() == screen.bottom())
		{
			m_previous_tsu_vpos = screen.top();
			screen.min_y = m_previous_tsu_vpos;
			draw = true;
		}
		else if (y >= 0 && y < screen.height())
		{
			// Update unrendered above excluding current line.
			screen.sety(m_previous_tsu_vpos, m_screen->vpos() - 1);
			// Too expencive to draw every line. Batch 8+ for now.
			draw = screen.height() > 7;
			if (!draw && (screen.bottom() + 1) == get_screen_area().bottom())
			{
				screen.max_y++;
				draw = true;
			}
			if (draw)
				m_previous_tsu_vpos = m_screen->vpos();
		}

		if (draw)
		{
			m_screen->priority().fill(0, screen);
			if (BIT(m_regs[TS_CONFIG], 5))
				m_ts_tilemap[TM_TILES0]->draw(*m_screen, m_screen->curbitmap().as_ind16(), screen,
											  BIT(m_regs[TS_CONFIG], 2) ? TILEMAP_DRAW_ALL_CATEGORIES : TILEMAP_DRAW_CATEGORY(1), 1);

			if (BIT(m_regs[TS_CONFIG], 6))
				m_ts_tilemap[TM_TILES1]->draw(*m_screen, m_screen->curbitmap().as_ind16(), screen,
											  BIT(m_regs[TS_CONFIG], 3) ? TILEMAP_DRAW_ALL_CATEGORIES : TILEMAP_DRAW_CATEGORY(1), 2);

			if (BIT(m_regs[TS_CONFIG], 7))
				draw_sprites(screen);
		}
	}
}

void tsconf_state::spectrum_UpdateBorderBitmap()
{
	spectrum_UpdateScreenBitmap();
}

u16 tsconf_state::get_border_color()
{
	return m_regs[BORDER];
}

/*
SFILE	Reg.16	7		6		5		4		3		2		1		0
0		R0L		Y[7:0]
1		R0H		YF		LEAP	ACT		-		YS[2:0]					Y[8]
2		R1L		X[7:0]
3		R1H		XF		-		-		-		XS[2:0]					X[8]
4		R2L		TNUM[7:0]
5		R2H		SPAL[7:4]						TNUM[11:8]
*/
void tsconf_state::draw_sprites(const rectangle &cliprect)
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
					m_gfxdecode->gfx(TM_SPRITES)->prio_transpen(m_screen->curbitmap().as_ind16(), cliprect, tmp_tile_oversized_to_code((tile_row % 64) * 64 + (tile_col % 64)), pal, flipx, flipy, ix, iy, m_screen->priority(), pmask, 0);
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
		//TODO invalidate sprites, not entire map
		m_ts_tilemap[offset & 64 ? TM_TILES1 : TM_TILES0]->mark_all_dirty();
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
	/* disable paging */
	if (m_port_7ffd_data & 0x20)
		return;

	/* store new state */
	m_port_7ffd_data = data;
	m_regs[MEM_CONFIG] = (m_regs[MEM_CONFIG] & 0xfe) | BIT(data, 4); // ROM128
	m_regs[V_PAGE] = BIT(data, 3) ? 7 : 5;

	/* update memory */
	tsconf_update_bank0();
	tsconf_update_video_mode();
}

void tsconf_state::tsconf_ula_w(offs_t offset, u8 data)
{
	m_regs[BORDER] = (data & 0x07) | 0xf0;
	spectrum_ula_w(offset, data);
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

	//LOGWARN("'tsconf': reg read %02X = %02x\n", nreg, data);
	return data;
}

void tsconf_state::tsconf_port_xxaf_w(offs_t port, u8 data)
{
	u8 nreg = port >> 8;
	bool val_changed = m_regs[nreg] != data;

	m_regs[nreg] = data;
	switch (nreg)
	{
	case V_CONFIG:
	case V_PAGE:
		if (val_changed)
			tsconf_update_video_mode();
		break;

	case G_Y_OFFS_L:
	case G_Y_OFFS_H:
		m_rendering_gfx_y_offset = ((m_regs[G_Y_OFFS_H] & 1) << 8) + m_regs[G_Y_OFFS_L];
		break;

	case T_MAP_PAGE:
		m_ts_tilemap[TM_TILES0]->mark_all_dirty();
		m_ts_tilemap[TM_TILES1]->mark_all_dirty();
		break;

	case T0_G_PAGE:
		m_gfxdecode->gfx(TM_TILES0)->set_source(m_ram->pointer() + PAGE4K(data));
		break;

	case T0_X_OFFSER_L:
	case T0_X_OFFSER_H:
		m_ts_tilemap[TM_TILES0]->set_scrollx((m_regs[T0_X_OFFSER_H] << 8) | m_regs[T0_X_OFFSER_L]);
		break;

	case T0_Y_OFFSER_L:
	case T0_Y_OFFSER_H:
		m_ts_tilemap[TM_TILES0]->set_scrolly((m_regs[T0_Y_OFFSER_H] << 8) | m_regs[T0_Y_OFFSER_L]);
		break;

	case T1_G_PAGE:
		m_gfxdecode->gfx(TM_TILES1)->set_source(m_ram->pointer() + PAGE4K(data));
		break;

	case T1_X_OFFSER_L:
	case T1_X_OFFSER_H:
		m_ts_tilemap[TM_TILES1]->set_scrollx((m_regs[T1_X_OFFSER_H] << 8) | m_regs[T1_X_OFFSER_L]);
		break;

	case T1_Y_OFFSER_L:
	case T1_Y_OFFSER_H:
		m_ts_tilemap[TM_TILES1]->set_scrolly((m_regs[T1_Y_OFFSER_H] << 8) | m_regs[T1_Y_OFFSER_L]);
		break;

	case SG_PAGE:
		m_gfxdecode->gfx(TM_SPRITES)->set_source(m_ram->pointer() + PAGE4K(data));
		break;

	case MEM_CONFIG:
		m_port_7ffd_data = (m_port_7ffd_data & 0xef) | (ROM128 << 4);
		tsconf_update_bank0();
		break;

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

	case BORDER:
		spectrum_UpdateBorderBitmap();
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

	case SYS_CONFIG:
		// 0 - 3.5MHz, 1 - 7MHz, 2 - 14MHz, 3 - reserved
		m_maincpu->set_clock(3.5_MHz_XTAL * (1 << (data & 0x03)));
		m_regs[CACHE_CONFIG] = BIT(data, 2) ? 0x0f : 0x00;
		break;

	case HS_INT:
	case VS_INT_L:
	case VS_INT_H:
		if (val_changed)
			update_frame_timer();
		break;

	case FMAPS:
	case PAL_SEL:
	case TS_CONFIG:
	case G_X_OFFS_L:
	case G_X_OFFS_H:
	case INT_MASK:
		break;

	default:
		logerror("Unsupported reg write: %02X = %02x\n", nreg, data);
		break;
	}
}

u8 tsconf_state::tsconf_port_f7_r(offs_t offset)
{
	// BFF7
	u8 data = 0xff;
	if (m_port_f7_ext == PS2KEYBOARDS_LOG && m_port_f7_gluk_reg == 0xf0)
	{
		data = m_keyboard->read();
	}
	else if (m_port_f7_ext != DISABLED)
	{
		data = m_glukrs->read(m_port_f7_gluk_reg);
	}

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
					m_ver.w = ((22 << 9) | (01 << 5) | 19); // y.m.d
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
			m_sdcard->spi_clock_w(CLEAR_LINE); //0-S R
			m_sdcard->spi_mosi_w(data & m ? 1 : 0);
			m_sdcard->spi_clock_w(ASSERT_LINE); //1-L W
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
	u16 vpos = ((m_regs[VS_INT_H] & 0x01) << 8) | m_regs[VS_INT_L];
	u16 hpos = m_regs[HS_INT];
	if (hpos > 0 && vpos <= 319 && hpos <= 223)
	{
		// Only if not overlapping with scanline. Otherwise we need to prioritize.
		m_frame_irq_timer->adjust(m_screen->time_until_pos(vpos, hpos << 1));
	}
	else
	{
		m_frame_irq_timer->adjust(attotime::never);
	}
}

INTERRUPT_GEN_MEMBER(tsconf_state::tsconf_vblank_interrupt)
{
	update_frame_timer();
	m_line_irq_timer->adjust(attotime::zero);
}

void tsconf_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_IRQ_FRAME:
	{
		if (BIT(m_regs[INT_MASK], 0))
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xff);
			m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
		}
		break;
	}
	case TIMER_IRQ_SCANLINE:
	{
		u16 screen_vpos = m_screen->vpos();
		m_line_irq_timer->adjust(m_screen->time_until_pos(screen_vpos + 1));
		if (BIT(m_regs[INT_MASK], 1))
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xfd);
			// Not quite precise. Scanline can't be skipped.
			m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
		}
		u16 vpos = ((m_regs[VS_INT_H] & 0x01) << 8) | m_regs[VS_INT_L];
		u16 hpos = m_regs[HS_INT];
		if (BIT(m_regs[INT_MASK], 0) && vpos == screen_vpos && hpos == 0)
		{
			m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, 0xff);
			m_irq_off_timer->adjust(m_maincpu->clocks_to_attotime(32));
		}
		spectrum_UpdateScreenBitmap();
		break;
	}
	default:
		spectrum_state::device_timer(timer, id, param);
	}
}

u8 tsconf_state::beta_neutral_r(offs_t offset)
{
	return m_program->read_byte(offset);
}

u8 tsconf_state::beta_enable_r(offs_t offset)
{
	if (m_ROMSelection == 3)
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
