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

static constexpr rectangle resolution_info[4] = {
	rectangle(52, 256 + 51, 48, 192 + 47), // 52|256|52 x 48-192-48
	rectangle(20, 320 + 19, 44, 200 + 43), // 20|320|20 x 44-200-44
	rectangle(20, 320 + 19, 24, 240 + 23), // 20|320|20 x 24-240-24
	rectangle(0, 360 - 1, 0, 288 - 1)      //  0|360|0  x  0-288-0
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

rectangle tsconf_state::get_resolution_info()
{
	rectangle info = resolution_info[BIT(m_regs[V_CONFIG], 6, 2)];
	if (VM == VM_TXT)
	{
		info.set_origin(0, 0);
		info.set_width(info.width() << 1);
	}
	return info;
}

void tsconf_state::tsconf_palette(palette_device &palette) const
{
	rgb_t colors[256] = {0};
	palette.set_pen_colors(0, colors);
}

void tsconf_state::tsconf_update_bank1()
{

	//W0_WE
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

	uint8_t *rom0;
	if (W0_RAM)
	{
		rom0 = m_ram->pointer() + PAGE4K(m_ROMSelection);
		m_bank1->set_base(rom0);
	}
	else
	{
		rom0 = &m_p_rom[0x10000 + PAGE4K(m_ROMSelection & 0x1f)];
		m_bank1->set_base(rom0);
	}
	m_ram_0000 = W0_WE ? rom0 : nullptr;
}

void tsconf_state::tsconf_update_video_mode()
{
	rectangle resolution = resolution_info[3];
	u8 *messram = m_ram->pointer();
	switch (VM)
	{
	case VM_TXT: // Text Mode
		resolution = get_resolution_info();
		m_gfxdecode->gfx(1)->set_source(messram + PAGE4K(m_regs[V_PAGE] ^ 0x01));
		break;
	case VM_ZX: // Zx
	{
		m_screen_location = messram + ((m_port_7ffd_data & 8) ? PAGE4K(7) : PAGE4K(5));
		break;
	}
	default:
		break;
	}

	m_screen->configure(resolution.width(), resolution.height(), resolution, HZ_TO_ATTOSECONDS(50));
}

uint32_t tsconf_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_screen_bitmap.valid())
	{
		// Border
		if (m_border_bitmap.valid())
		{
			if (BIT(m_regs[V_CONFIG], 5) || VM != VM_ZX)
			{
				copyscrollbitmap(bitmap, m_border_bitmap, 0, nullptr, 0, nullptr, cliprect);
			}
		}

		// Main Graphics
		rectangle resolution = get_resolution_info();
		if (!BIT(m_regs[V_CONFIG], 5))
		{
			if (VM == VM_ZX)
			{
				// Zx palette is stored at 0xf0. Adjust bitmaps colors.
				for (auto y = cliprect.top(); y <= cliprect.bottom(); y++)
				{
					u16 *bm = &m_screen_bitmap.pix(y, 0);
					for (auto x = cliprect.left(); x <= cliprect.right(); x++)
					{
						*bm++ |= 0xf0;
					}
				}
				spectrum_state::screen_update_spectrum(screen, bitmap, cliprect);
			}
			else
			{
				copyscrollbitmap(bitmap, m_screen_bitmap, 0, nullptr, 0, nullptr, resolution);
			}
		}
		// Tiles & Sprites
		// TODO layers
		if (!BIT(m_regs[V_CONFIG], 4))
		{
			if (BIT(m_regs[TS_CONFIG], 5))
			{
				m_ts_tilemap[1]->draw(screen, bitmap, resolution, 0);
			}
			if (BIT(m_regs[TS_CONFIG], 6))
			{
				m_ts_tilemap[2]->draw(screen, bitmap, resolution, 0);
			}
			if (BIT(m_regs[TS_CONFIG], 7))
			{
				draw_sprites(screen, bitmap, resolution);
			}
		}
	}
	return 0;
}

void tsconf_state::spectrum_UpdateScreenBitmap(bool eof)
{
	if (!BIT(m_regs[V_CONFIG], 5))
	{
		if (VM == VM_ZX)
		{
			spectrum_state::spectrum_UpdateScreenBitmap(eof);
		}
		else if (!eof)
		{
			u8 pal_offset = m_regs[PAL_SEL] << 4;
			rectangle resolution = get_resolution_info();
			if (VM == VM_TXT)
			{
				u8 *messram = m_ram->pointer();
				u16 y = m_screen->vpos();
				u8 *font_location = messram + PAGE4K(m_regs[V_PAGE] ^ 0x01);
				u8 *text_location = messram + PAGE4K(m_regs[V_PAGE]) + (y / 8 * 256); // OFFSETs
				u16 *bm = &m_screen_bitmap.pix(y, 0);
				for (auto x = 0; x < resolution.width() / 8; x++)
				{
					u8 char_x = *(font_location + (*text_location * 8) + (y % 8));
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
				if (m_screen->vpos() >= resolution.top())
				{
					u16 y = m_screen->vpos() - resolution.top();
					u32 offset = ((y + ((m_regs[G_Y_OFFS_H] & 1) << 8) + m_regs[G_Y_OFFS_L]) * 512) +
								 ((m_regs[G_X_OFFS_H] & 1) << 8) + m_regs[G_X_OFFS_L];
					if (VM == VM_16C)
					{
						// FIXME wouldn't work for odd offsets
						offset >>= 1;
					}
					u8 *video_location = &m_ram->pointer()[PAGE4K(m_regs[V_PAGE]) + offset];
					u16 *bm = &m_screen_bitmap.pix(m_screen->vpos(), resolution.left());
					for (auto x = resolution.left(); x <= resolution.right(); x++)
					{
						u8 pix = *video_location;
						if (VM == VM_16C)
						{
							*bm++ = (pix >> 4) + pal_offset;
							*bm++ = (pix & 0x0f) + pal_offset;
							x++;
						}
						else
						{
							*bm++ = pix;
						}
						video_location++;
					}
				}
			}
		}
	}
}

u16 tsconf_state::get_border_color()
{
	return m_regs[BORDER];
}

void tsconf_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//u8 *messram = m_ram->pointer();
	//u8 *sprites_location = messram + PAGE4K(m_regs[SG_PAGE]);
	for (u8 i = 0; i < 85; i++)
	{
		logerror("Draw Sprites ... TODO\n");
	}
}

void tsconf_state::ram_bank_write(u8 bank, offs_t offset, u8 data)
{
	offs_t machine_addr = PAGE4K(bank) + offset;
	offs_t fmap_addr = BIT(m_regs[FMAPS], 0, 4) << 12;
	if (BIT(m_regs[FMAPS], 4) && (machine_addr >= fmap_addr) && (machine_addr < (fmap_addr + 512)))
	{
		cram_write(machine_addr - fmap_addr, data);
	}
	else if (bank > 0 || (W0_WE && W0_RAM))
	{
		ram_page_write(m_regs[PAGE0 + bank], offset, data);
	}
}

void tsconf_state::ram_page_write(u8 page, offs_t offset, u8 data)
{
	u32 ram_addr = PAGE4K(page) + offset;
	if (ram_addr >= PAGE4K(m_regs[T_MAP_PAGE]) && ram_addr < (PAGE4K(m_regs[T_MAP_PAGE] + 1)))
	{
		//TODO invalidate sprites, not entire map
		m_ts_tilemap[1]->mark_all_dirty();
		m_ts_tilemap[2]->mark_all_dirty();
	}
	else
	{
		if (ram_addr >= PAGE4K(m_regs[T0_G_PAGE]) && ram_addr < PAGE4K(m_regs[T0_G_PAGE] + 8))
		{
			m_ts_tilemap[1]->mark_all_dirty();
		}
		if (ram_addr >= PAGE4K(m_regs[T1_G_PAGE]) && ram_addr < PAGE4K(m_regs[T1_G_PAGE] + 8))
		{
			m_ts_tilemap[2]->mark_all_dirty();
		}
		if (ram_addr >= PAGE4K(m_regs[SG_PAGE]) && ram_addr < PAGE4K(m_regs[SG_PAGE] + 8))
		{
			m_ts_tilemap[3]->mark_all_dirty();
		}
	}

	m_ram->write(ram_addr, data);
}

u16 tsconf_state::ram_read16(offs_t offset)
{
	return ((m_ram->read(offset & 0xfffffffe)) << 8) | m_ram->read(offset | 1);
}

void tsconf_state::ram_write16(offs_t offset, u16 data)
{
	m_ram->write(offset & 0xfffffffe, data >> 8);
	m_ram->write(offset | 1, data & 0xff);
}

u16 tsconf_state::spi_read16()
{
	return (tsconf_port_57_zctr_r(0) << 8) | tsconf_port_57_zctr_r(0);
}

void tsconf_state::cram_write(u16 offset, u8 data)
{
	m_cram->write(offset, data);
	u8 pen = offset >> 1;
	rgb_t rgb = from_pwm((m_cram->read(offset | 1) << 8 | m_cram->read(offset & 0xfffe)));
	m_palette->set_pen_color(pen, rgb);
};

void tsconf_state::cram_write16(offs_t offset, u16 data)
{
	cram_write(offset & 0xfffe, data >> 8);
	cram_write(offset | 1, data & 0xff);
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
	tsconf_update_bank1();
	tsconf_update_video_mode();
}

void tsconf_state::tsconf_port_fe_w(offs_t offset, u8 data)
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
	m_regs[nreg] = data;

	switch (nreg)
	{
	case V_CONFIG:
	case V_PAGE:
	case PAL_SEL:
		tsconf_update_video_mode();
		break;

	case T_MAP_PAGE:
		m_ts_tilemap[1]->mark_all_dirty();
		m_ts_tilemap[2]->mark_all_dirty();
		break;

	case T0_G_PAGE:
		m_gfxdecode->gfx(2)->set_source(m_ram->pointer() + PAGE4K(data));
		m_ts_tilemap[1]->mark_all_dirty();
		break;

	case T0_X_OFFSER_L:
	case T0_X_OFFSER_H:
	case T0_Y_OFFSER_L:
	case T0_Y_OFFSER_H:
		m_ts_tilemap[1]->set_scrollx((m_regs[T0_X_OFFSER_H] << 8) | m_regs[T0_X_OFFSER_L]);
		m_ts_tilemap[1]->set_scrolly((m_regs[T0_Y_OFFSER_H] << 8) | m_regs[T0_Y_OFFSER_L]);
		break;

	case T1_G_PAGE:
		m_gfxdecode->gfx(3)->set_source(m_ram->pointer() + PAGE4K(data));
		m_ts_tilemap[2]->mark_all_dirty();
		break;

	case T1_X_OFFSER_L:
	case T1_X_OFFSER_H:
	case T1_Y_OFFSER_L:
	case T1_Y_OFFSER_H:
		m_ts_tilemap[2]->set_scrollx((m_regs[T1_X_OFFSER_H] << 8) | m_regs[T1_X_OFFSER_L]);
		m_ts_tilemap[2]->set_scrolly((m_regs[T1_Y_OFFSER_H] << 8) | m_regs[T1_Y_OFFSER_L]);
		break;

	case SG_PAGE:
		m_gfxdecode->gfx(4)->set_source(m_ram->pointer() + PAGE4K(data));
		m_ts_tilemap[3]->mark_all_dirty();
		break;

	case MEM_CONFIG:
		m_port_7ffd_data = (m_port_7ffd_data & 0xef) | (ROM128 << 4);
		tsconf_update_bank1();
		break;

	case PAGE0:
		tsconf_update_bank1();
		break;

	case PAGE1:
		m_bank2->set_base(m_ram->pointer() + PAGE4K(data));
		break;

	case PAGE2:
		m_bank3->set_base(m_ram->pointer() + PAGE4K(data));
		break;

	case PAGE3:
		m_bank4->set_base(m_ram->pointer() + PAGE4K(data));
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
		switch (data & 0x03)
		{
		case 2:
			m_maincpu->set_clock(X1);
			break;
		case 1:
			m_maincpu->set_clock(X1 / 2);
			break;
		case 0:
		default:
			m_maincpu->set_clock(X1 / 4);
			break;
		}
		break;

	case FMAPS:
	case TS_CONFIG:
	case G_X_OFFS_L:
	case G_X_OFFS_H:
	case G_Y_OFFS_L:
	case G_Y_OFFS_H:
	case HS_INT:
	case VS_INT_L:
	case VS_INT_H:
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
					m_ver.w = ((21 << 9) | (12 << 5) | 15);
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
			tsconf_update_bank1();
		}
	}

	return m_program->read_byte(offset + 0x3d00);
}

u8 tsconf_state::beta_disable_r(offs_t offset)
{
	if (m_beta->started() && m_beta->is_active())
	{
		m_beta->disable();
		tsconf_update_bank1();
	}

	return m_program->read_byte(offset + 0x4000);
}
