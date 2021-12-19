// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "includes/tsconf.h"

#define W0_RAM (BIT(MEM_CONFIG, 3))
#define NW0_MAP (BIT(MEM_CONFIG, 2))
#define W0_WE (BIT(MEM_CONFIG, 1))
#define ROM128 (BIT(MEM_CONFIG, 0))
#define RRES (BIT(V_CONFIG, 6, 2))

#define VM static_cast<v_mode>(BIT(V_CONFIG, 0, 2))

static constexpr rectangle resolution_info[5] = {
	rectangle(52, 256 + 51, 48, 192 + 47), // 52|256|52 x 48-192-48
	rectangle(20, 320 + 19, 44, 200 + 43), // 20|320|20 x 44-200-44
	rectangle(20, 320 + 19, 24, 240 + 23), // 20|320|20 x 24-240-24
	rectangle(0, 360 - 1, 0, 288 - 1),	   //  0|360|0  x  0-288-0
	rectangle(0, 640 - 1, 0, 240 - 1)	   // text
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

void tsconf_state::tsconf_update_bank1()
{

	//W0_WE
	if (NW0_MAP)
	{
		m_ROMSelection = PAGE0;
	}
	else
	{
		/* ROM: 0-SYS, 1-DOS, 2-128, 3-48 */
		if (m_beta->started() && m_beta->is_active())
		{
			m_ROMSelection = ROM128;
		}
		else
		{
			m_ROMSelection = 0x02 | ROM128;
		}
		m_ROMSelection |= (PAGE0 & 0xfc);
	}

	uint8_t *rom0;
	if (W0_RAM)
	{
		rom0 = m_ram->pointer() + PAGE(m_ROMSelection);
		m_bank1->set_base(rom0);
	}
	else
	{
		rom0 = &m_p_rom[0x10000 + PAGE(m_ROMSelection & 0x1f)];
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
		resolution = resolution_info[4];
		m_gfxdecode->gfx(1)->set_source(messram + PAGE(V_PAGE ^ 0x01));
		break;
	case VM_ZX: // Zx
	{
		m_screen_location = messram + ((m_port_7ffd_data & 8) ? PAGE(7) : PAGE(5));
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
		// Border + Main Graphics
		if (!BIT(V_CONFIG, 5))
		{
			if (VM == VM_ZX)
			{
				// Zx palette is stored at 0xf0. Adjust bitmaps colors.
				for (auto y = cliprect.top(); y <= cliprect.bottom(); y++)
				{
					u16 *bm = &m_screen_bitmap.pix(y, 0);
					u16 *border_bm = &m_border_bitmap.pix(y, 0);
					for (auto x = cliprect.left(); x <= cliprect.right(); x++)
					{
						*bm++ |= 0xf0;
						*border_bm++ |= 0xf0;
					}
				}
				spectrum_state::screen_update_spectrum(screen, bitmap, cliprect);
			}
			else if (VM == VM_TXT)
			{
				copyscrollbitmap(bitmap, m_screen_bitmap, 0, nullptr, 0, nullptr, resolution_info[4]);
			}
			else
			{
				rectangle resolution = resolution_info[RRES];
				if (m_border_bitmap.valid() && resolution.left() > 0 && resolution.top() > 0)
					copyscrollbitmap(bitmap, m_border_bitmap, 0, nullptr, 0, nullptr, cliprect);
				copyscrollbitmap(bitmap, m_screen_bitmap, 0, nullptr, 0, nullptr, resolution);
			}
		}

		// Tiles & Sprites
		// TODO layers
		if (!BIT(V_CONFIG, 4))
		{
			if (BIT(TS_CONFIG, 5))
			{
				m_ts_tilemap[1]->draw(screen, bitmap, cliprect, 0);
			}
			if (BIT(TS_CONFIG, 6))
			{
				m_ts_tilemap[2]->draw(screen, bitmap, cliprect, 0);
			}
			if (BIT(TS_CONFIG, 7))
			{
				draw_sprites(screen, bitmap, cliprect);
			}
		}
	}
	return 0;
}

void tsconf_state::spectrum_UpdateScreenBitmap(bool eof)
{
	u8 pal_offset = PAL_SEL << 4;
	if (VM == VM_ZX)
	{
		spectrum_state::spectrum_UpdateScreenBitmap(eof);
	}
	else if (VM == VM_TXT)
	{
		u8 *messram = m_ram->pointer();
		u16 y = m_screen->vpos();
		u8 *font_location = messram + PAGE(V_PAGE ^ 0x01);
		u8 *text_location = messram + PAGE(V_PAGE) + (y / 8 * 256); // OFFSETs
		u16 *bm = &m_screen_bitmap.pix(y, 0);
		for (auto x = 0; x < 80; x++)
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
		rectangle resolution = resolution_info[RRES];
		if (resolution.contains(resolution.xcenter(), m_screen->vpos()))
		{
			u16 line_len = VM == VM_16C ? 256 : 512;
			u16 y = m_screen->vpos() - resolution.top();
			u16 offset = ((y + ((G_Y_OFFS_H & 1) << 8) + G_Y_OFFS_L) * line_len) +
						 ((((G_X_OFFS_H & 1) << 8 /* .. 16/256 */) + G_X_OFFS_L) >> 1);
			u8 *video_location = m_ram->pointer() + PAGE(V_PAGE) + offset;
			u16 *bm = &m_screen_bitmap.pix(m_screen->vpos(), resolution.left());
			for (auto x = resolution.left(); x <= resolution.right(); x++)
			{
				u8 pix = *video_location;
				if (VM == VM_16C)
				{
					*bm++ = (pix >> 4) + pal_offset;
					pix &= 0x0f;
					x++;
					*bm++ = (pix & 0x07) + pal_offset;
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

void tsconf_state::spectrum_UpdateBorderBitmap()
{
	rectangle resolution = resolution_info[RRES];
	if (VM != VM_TXT && resolution.left() > 0 && resolution.top() > 0)
	{
		spectrum_state::spectrum_UpdateBorderBitmap();
	}
}

void tsconf_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u8 i = 0; i < 85; i++)
	{
		logerror("Draw Sprites ... TODO\n");
	}
}

void tsconf_state::ram_bank_write(u8 bank, offs_t offset, u8 data)
{
	offs_t machine_addr = PAGE(bank) + offset;
	offs_t fmap_addr = BIT(FMAPS, 0, 4) << 12;
	if (BIT(FMAPS, 4) && (machine_addr >= fmap_addr) && (machine_addr < (fmap_addr + 512)))
	{
		cram_write(machine_addr - fmap_addr, data);
	}
	else if (bank > 0 || (W0_WE && W0_RAM))
	{
		ram_page_write(REG(REGNUM(PAGE0) + bank), offset, data);
	}
}

void tsconf_state::ram_page_write(u8 page, offs_t offset, u8 data)
{
	u32 ram_addr = RAM_PAGE_OFFST(page, offset);
	if (ram_addr >= PAGE(T_MAP_PAGE) && ram_addr < (PAGE(T_MAP_PAGE + 1)))
	{
		//TODO invalidate sprites, not entire map
		m_ts_tilemap[1]->mark_all_dirty();
		m_ts_tilemap[2]->mark_all_dirty();
	}
	else
	{
		if (ram_addr >= PAGE(T0_G_PAGE) && ram_addr < PAGE(T0_G_PAGE + 8))
		{
			m_ts_tilemap[1]->mark_all_dirty();
		}
		if (ram_addr >= PAGE(T1_G_PAGE) && ram_addr < PAGE(T1_G_PAGE + 8))
		{
			m_ts_tilemap[2]->mark_all_dirty();
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
	REG(MEM_CONFIG) = (REG(MEM_CONFIG) & 0xfe) | BIT(data, 4); // ROM128
	REG(V_PAGE) = BIT(data, 3) ? 7 : 5;

	/* update memory */
	tsconf_update_bank1();
	tsconf_update_video_mode();
}

void tsconf_state::tsconf_port_fe_w(offs_t offset, u8 data)
{
	BORDER = (data & 0x07) | 0xf0;
	spectrum_port_fe_w(offset, data);
}

u8 tsconf_state::tsconf_port_xxaf_r(offs_t port)
{
	u8 nreg = port >> 8;
	u8 data = 0xff;

	switch (nreg)
	{
	case 0x00:
		data = 0b01000011; // PWR_UP, !FDRVER, 5bit VDAC
		break;
	case 0x12: // PAGE2
	case 0x13: // PAGE3
		data = REG(nreg);
		break;

	case 0x27: // DMAStatus
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
	REG(nreg) = data;

	if (nreg == REGNUM(V_CONFIG) || nreg == REGNUM(V_PAGE) || nreg == REGNUM(PAL_SEL))
	{
		tsconf_update_video_mode();
	}
	else if (nreg == REGNUM(FMAPS))
	{
	}
	else if (nreg == REGNUM(TS_CONFIG))
	{
	}
	else if (nreg == REGNUM(G_X_OFFS_L) || nreg == REGNUM(G_X_OFFS_H) || nreg == REGNUM(G_Y_OFFS_L) || nreg == REGNUM(G_Y_OFFS_H))
	{
	}
	else if (nreg == REGNUM(T_MAP_PAGE))
	{
		m_ts_tilemap[1]->mark_all_dirty();
		m_ts_tilemap[2]->mark_all_dirty();
	}
	else if (nreg == REGNUM(T0_G_PAGE))
	{
		m_gfxdecode->gfx(2)->set_source(m_ram->pointer() + PAGE(T0_G_PAGE));
		m_ts_tilemap[1]->mark_all_dirty();
	}
	else if ((nreg >= REGNUM(T0_X_OFFSER_L) && nreg <= REGNUM(T0_Y_OFFSER_H)))
	{
		m_ts_tilemap[1]->set_scrollx((T0_X_OFFSER_H << 8) | T0_X_OFFSER_L);
		m_ts_tilemap[1]->set_scrolly((T0_Y_OFFSER_H << 8) | T0_Y_OFFSER_L);
	}
	else if (nreg == REGNUM(T1_G_PAGE))
	{
		m_gfxdecode->gfx(3)->set_source(m_ram->pointer() + PAGE(T1_G_PAGE));
		m_ts_tilemap[2]->mark_all_dirty();
	}
	else if ((nreg >= REGNUM(T1_X_OFFSER_L) && nreg <= REGNUM(T1_Y_OFFSER_H)))
	{
		m_ts_tilemap[2]->set_scrollx((T1_X_OFFSER_H << 8) | T1_X_OFFSER_L);
		m_ts_tilemap[2]->set_scrolly((T1_Y_OFFSER_H << 8) | T1_Y_OFFSER_L);
	}
	else if (nreg == REGNUM(MEM_CONFIG))
	{
		m_port_7ffd_data = (m_port_7ffd_data & 0xef) | (ROM128 << 4);
		tsconf_update_bank1();
	}
	else if (nreg == REGNUM(PAGE0))
	{
		tsconf_update_bank1();
	}
	else if (nreg == REGNUM(PAGE1))
	{
		m_bank2->set_base(m_ram->pointer() + PAGE(PAGE1));
	}
	else if (nreg == REGNUM(PAGE2))
	{
		m_bank3->set_base(m_ram->pointer() + PAGE(PAGE2));
	}
	else if (nreg == REGNUM(PAGE3))
	{
		m_bank4->set_base(m_ram->pointer() + PAGE(PAGE3));
	}
	else if (nreg == REGNUM(BORDER))
	{
		tsconf_port_fe_w(0, (m_port_fe_data & 0xf8) | (data & 0x07));
	}
	else if (nreg == REGNUM(DMAS_ADDRESS_L))
	{
		m_dma->set_saddr_l(data);
	}
	else if (nreg == REGNUM(DMAS_ADDRESS_H))
	{
		m_dma->set_saddr_h(data);
	}
	else if (nreg == REGNUM(DMAS_ADDRESS_X))
	{
		m_dma->set_saddr_x(data);
	}
	else if (nreg == REGNUM(DMAD_ADDRESS_L))
	{
		m_dma->set_daddr_l(data);
	}
	else if (nreg == REGNUM(DMAD_ADDRESS_H))
	{
		m_dma->set_daddr_h(data);
	}
	else if (nreg == REGNUM(DMAD_ADDRESS_X))
	{
		m_dma->set_daddr_x(data);
	}
	else if (nreg == REGNUM(DMA_LEN))
	{
		m_dma->set_block_len(data);
	}
	else if (nreg == REGNUM(DMA_NUM_L))
	{
		m_dma->set_block_num_l(data);
	}
	else if (nreg == REGNUM(DMA_NUM_H))
	{
		m_dma->set_block_num_h(data);
	}
	else if (nreg == REGNUM(DMA_CTRL))
	{
		m_dma->start_tx(((BIT(data, 7) << 3) | (data & 0x07)), BIT(data, 5), BIT(data, 4), BIT(data, 3));
	}
	else if (nreg == REGNUM(SYS_CONFIG))
	{
		/* TODO
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
		*/
	}
	else if (nreg == REGNUM(HS_INT) || nreg == REGNUM(VS_INT_L) || nreg == REGNUM(VS_INT_H) || nreg == REGNUM(INT_MASK))
	{
	}
	else
	{
		logerror("Unsupported reg write: %02X = %02x\n", nreg, data);
	}
}

u8 tsconf_state::tsconf_port_f7_cmos_r(offs_t offset)
{
	// BFF7
	u8 data;
	if (m_gluk_ext == PS2KEYBOARDS_LOG && m_gluk_reg == 0xf0)
	{
		data = m_keyboard->read();
	}
	else if (m_gluk_ext != DISABLED)
	{
		data = m_cmos->read(m_gluk_reg);
	}
	else
	{
		data = 0xff;
	}
	return data;
}

void tsconf_state::tsconf_port_f7_cmos_w(offs_t offset, u8 data)
{
	auto m_l = offset >> 12;
	if (m_l == 6) // EF
	{
		m_gluk_ext = (data & 0x80) ? CONF_VERSION : DISABLED;
	}
	else if (m_gluk_ext != DISABLED)
	{
		if (m_l == 5) // DF
		{
			// 0x0E..0xEF
			m_gluk_reg = data;
			if (m_gluk_reg == 0x00 || m_gluk_reg == 0x02 || m_gluk_reg == 0x04 || m_gluk_reg == 0x06 || m_gluk_reg == 0x07 || m_gluk_reg == 0x08 || m_gluk_reg == 0x09)
			{
				system_time curtime;
				machine().current_datetime(curtime);
				m_cmos->write(0x00, dec_2_bcd(curtime.local_time.second));
				m_cmos->write(0x02, dec_2_bcd(curtime.local_time.minute));
				m_cmos->write(0x04, dec_2_bcd(curtime.local_time.hour));
				m_cmos->write(0x06, dec_2_bcd(curtime.local_time.weekday));
				m_cmos->write(0x07, dec_2_bcd(curtime.local_time.mday));
				m_cmos->write(0x08, dec_2_bcd(curtime.local_time.month));
				m_cmos->write(0x09, dec_2_bcd(curtime.local_time.year % 100));
			}
		}
		else if (m_l == 3) // BF
		{
			if (m_gluk_reg == 0xf0)
			{
				u8 m_fx[0xf] = {0xff};
				m_gluk_ext = static_cast<gluk_ext>(data);
				switch (m_gluk_ext)
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
					logerror("Gluk extention not supported %x\n", m_gluk_reg);
					break;
				}
				for (u8 i = 0; i < 0xf; i++)
				{
					m_cmos->write(0xf0 + i, m_fx[i]);
				}
			}
			else
			{
				m_cmos->write(m_gluk_reg, data);
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
