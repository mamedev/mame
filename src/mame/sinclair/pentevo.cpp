// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

ZX Evolution: BASECONF machine driver.
Implementation: Revision C

Hobby computer ZX Evolution is Spectrum-compatible with extensions.

Hardware (ZX Evolution):
- Z80 3.5 MHz (classic mode)/ 7 MHz (turbo mode without CPU wait circles)/ 14 MHz (mega turbo with CPU wait circles);
- 4 Mb RAM, 512Kb ROM;
- MiniITX board (172x170mm), 2 ZXBUS slots, power ATX or +5,+12V;
- Based on fpga (Altera EP1K50);
- Peripheral MCU ATMEGA128;
- PS/2 keyboard and mouse support;
- Floppy (WDC1793) Beta-disk compatible interface, IDE (one channel, up to 2 devices on master/slave mode), SD(HC) card, RS232;
- Sound: AY, Beeper, Covox (PWM);
- Real-time clock.

Refs:
ZxEvo: http://nedopc.com/zxevo/zxevo_eng.php
		Principal scheme (rev. C) :: http://nedopc.com/zxevo/zxevo_sch_revc.pdf
		Montage scheme (rev. C) :: http://nedopc.com/zxevo/zxevo_mon_revc.pdf

TODO:
	* Keyboard enabled
	* zx 16c
	* NMI?

*******************************************************************************************/

#include "atm.h"

#include "glukrs.h"
#include "machine/pckeybrd.h"
#include "machine/spi_sdcard.h"
#include "machine/timer.h"

namespace {

#define LOG_MEM   (1U << 1)
#define LOG_VIDEO (1U << 2)
#define LOG_WARN  (1U << 3)

#define VERBOSE ( /*LOG_GENERAL | LOG_MEM | LOG_VIDEO |*/ LOG_WARN )
#include "logmacro.h"

#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGVIDEO(...) LOGMASKED(LOG_VIDEO, __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)

class pentevo_state : public atm_state
{
public:
	pentevo_state(const machine_config &mconfig, device_type type, const char *tag)
		: atm_state(mconfig, type, tag)
		, m_gfxdecode(*this, "gfxdecode")
		, m_char_ram(*this, "char_ram")
		, m_glukrs(*this, "glukrs")
		, m_sdcard(*this, "sdcard")
		, m_keyboard(*this, "pc_keyboard")
		, m_io_mouse(*this, "mouse_input%u", 1U)
	{ }

	void pentevo(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;
	void video_start() override;

private:
	void init_mem_write();
	void pentevo_io(address_map &map);

	u8 nemo_ata_r(u8 cmd);
	void nemo_ata_w(u8 cmd, u8 data);

	void atm_port_ffff_w(offs_t offset, u8 data) override;
	void pentevo_port_f7f7_w(offs_t offset, u8 data);
	void pentevo_port_fbf7_w(offs_t offset, u8 data);
	void pentevo_port_eff7_w(offs_t offset, u8 data);
	u8 atm_port_bf_r(offs_t offset);
	void atm_port_bf_w(offs_t offset, u8 data);
	u8 atm_port_0fbd_r(offs_t offset);
	void atm_port_0fbd_w(offs_t offset, u8 data);
	u8 atm_port_1fbd_r(offs_t offset);
	void atm_port_1fbd_w(offs_t offset, u8 data);
	u8 spi_port_77_r(offs_t offset);
	void spi_port_77_w(offs_t offset, u8 data);
	u8 spi_port_57_r(offs_t offset);
	void spi_port_57_w(offs_t offset, u8 data);
	void spi_miso_w(u8 data);

	u8 gluk_data_r(offs_t offset);
	void gluk_data_w(offs_t offset, u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(nmi_check_callback);
	void nmi_on();

	bool is_shadow_active() override { return BIT(m_port_bf_data, 0) || is_dos_active(); }
	bool is_gluk_active() {return is_shadow_active() || BIT(m_port_eff7_data, 7); }
	bool is_port_7ffd_locked() override { return !is_pent1024() && BIT(m_port_7ffd_data, 5); }
	bool is_pent1024() { return !BIT(m_port_eff7_data, 2); }

	void atm_update_cpu() override;
	u8 merge_ram_with_7ffd(u8 ram_page) override;

	void spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void pentevo_update_screen_zxhw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pentevo_update_screen_zx16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pentevo_update_screen_tx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ram_device> m_char_ram;

	required_device<glukrs_device> m_glukrs;
	required_device<spi_sdcard_sdhc_device> m_sdcard;
	required_device<at_keyboard_device> m_keyboard;
	required_ioport_array<3> m_io_mouse;

	u8 m_port_bf_data;
	u8 m_port_eff7_data;

	u8 m_gluk_ext;
	bool m_ata_data_hi_ready;
	bool m_nmi_exit;

	u8 m_zctl_di = 0;
	u8 m_zctl_cs = 0;
};


/******************************************************************************
 * ZX Evolution: BASECONF
 * ***************************************************************************/

void pentevo_state::atm_update_cpu()
{
	u8 multiplier = BIT(m_port_ff77_data, 3) ? 4 : (2 - BIT(m_port_eff7_data, 4));
	m_maincpu->set_clock(X1_128_SINCLAIR / 10 * multiplier);
}

void pentevo_state::atm_port_ffff_w(offs_t offset, u8 data)
{
	if(!is_shadow_active())
		return;

	if (BIT(m_port_bf_data, 5) && !m_pen2)
	{
		u8 pen = get_border_color(m_screen->hpos(), m_screen->vpos());
		m_palette->set_pen_color(pen,
			(BIT(~data, 1) * 0x88) | (BIT(~data, 6) * 0x44) | (BIT(~offset,  9) * 0x22) | (BIT(~offset, 14) * 0x11),
			(BIT(~data, 4) * 0x88) | (BIT(~data, 7) * 0x44) | (BIT(~offset, 12) * 0x22) | (BIT(~offset, 15) * 0x11),
			(BIT(~data, 0) * 0x88) | (BIT(~data, 5) * 0x44) | (BIT(~offset,  8) * 0x22) | (BIT(~offset, 13) * 0x11));
	}
	else
	{
		atm_state::atm_port_ffff_w(offset, data);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pentevo_state::nmi_check_callback)
{
	if (m_io_nmi->read() & 0x01)
		nmi_on();
}

void pentevo_state::nmi_on()
{
	m_bank_ram[0]->set_entry(0xff & rom_pages_mask);
	m_bank_view0.select(1);
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

u8 pentevo_state::merge_ram_with_7ffd(u8 ram_page)
{
	u8 page = atm_state::merge_ram_with_7ffd(ram_page);
	if (is_pent1024())
		page = (page & ~0x38) | ((m_port_7ffd_data & 0xe0) >> 2);

	return page;
}

void pentevo_state::pentevo_port_eff7_w(offs_t offset, u8 data)
{
	if (is_shadow_active())
		return;

	m_port_eff7_data = data;
	// BIT(data, 3) force ROM
	atm_update_cpu();
}

void pentevo_state::pentevo_port_f7f7_w(offs_t offset, u8 data)
{
	if(!is_shadow_active())
		return;

	u8 bank = offset >> 14;
	u16 page = (pen_page(bank) & PEN_DOS7FFD_MASK) | PEN_RAMNROM_MASK | u8(~data);

	LOGMEM("EVO%s=%X RAM%d%s%02X\n", BIT(m_port_7ffd_data, 4), data, bank, (page & PEN_DOS7FFD_MASK) ? "+" : " ", page & ram_pages_mask);
	pen_page(bank) = page;
	atm_update_memory();
}

void pentevo_state::pentevo_port_fbf7_w(offs_t offset, u8 data)
{
	if(!is_shadow_active())
		return;

	u8 bank = offset >> 14;
	pen_page(bank) = BIT(data, 0)
		? (pen_page(bank) | PEN_WRDISBL_MASK) : (pen_page(bank) & ~PEN_WRDISBL_MASK);
}

void pentevo_state::atm_port_bf_w(offs_t offset, u8 data)
{
	if (BIT(m_port_bf_data, 3) && !BIT(data, 3))
		nmi_on();

	m_port_bf_data = data;
}

u8 pentevo_state::atm_port_bf_r(offs_t offset)
{
	return m_port_bf_data & 0x1f;
}

u8 pentevo_state::atm_port_0fbd_r(offs_t offset)
{
	u8 opt = offset >> 8;
	if (opt <= 0x07)
		return ~(m_pages_map[BIT(opt, 2)][opt & 0x03] & 0xff);
	else if (opt == 0x08 || opt == 0x09)
	{
		u8 data = 0;
		for (s8 i = 7; i >= 0; i--)
			data = (data << 1) | bool(m_pages_map[BIT(i, 2)][i & 0x03] & (opt == 0x08 ? PEN_RAMNROM_MASK : PEN_DOS7FFD_MASK));
		return data;
	}
	else if (opt == 0x0a)
		return m_port_7ffd_data;
	else if (opt == 0x0b)
		return m_port_eff7_data;
	else if (opt == 0x0c)
		return (m_pen2 << 7) | (m_cpm_n << 6) | (m_pen << 5) | (is_dos_active() << 4) | (BIT(m_port_ff77_data, 4) << 3) | m_rg;
	else
	{
		LOGWARN("#%X read\n", 0x00bd | offset);
		return 0xff;
	}
}

void pentevo_state::atm_port_0fbd_w(offs_t offset, u8 data)
{
	u8 opt = offset >> 8;
	if (opt == 0x00 || opt == 0x01)
	{
		//TODO NMI trap - no usecase to test yet
		LOGWARN("#%X (NMI) < %X\n", 0x00bd | offset, data);
	}
	else
	{
		LOGWARN("#%X < %X\n", 0x00bd | offset, data);
	}
}

u8 pentevo_state::atm_port_1fbd_r(offs_t offset)
{
	u8 opt = offset >> 8;
	if (opt == 0x03) // #13BD
		return m_beta->is_active(); //0x00;
	else
	{
		LOGWARN("#%X read\n", 0x10bd | offset);
		return 0xff;
	}
}

void pentevo_state::atm_port_1fbd_w(offs_t offset, u8 data)
{
	u8 opt = offset >> 8;
	if (opt == 3)
	{
		if ((data & 0x0f) && m_beta->is_active())
		{
			m_bank_ram[0]->set_entry(0xfe);
			m_bank_view0.disable();
		}
	}
	else
	{
		LOGWARN("#%X < %X\n", 0x10bd | offset, data);
	}
}

void pentevo_state::spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_rg == 0b011 && (BIT(m_port_eff7_data, 0) || BIT(m_port_eff7_data, 5)))
	{
		if (BIT(m_port_eff7_data, 5))
			pentevo_update_screen_zxhw(screen, bitmap, cliprect);
		else
			pentevo_update_screen_zx16(screen, bitmap, cliprect);
	}
	else if (m_rg == 0b111)
		pentevo_update_screen_tx(screen, bitmap, cliprect);
	else
		atm_state::spectrum_update_screen(screen, bitmap, cliprect);
}

void pentevo_state::pentevo_update_screen_zxhw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool invert_attrs = u64(screen.frame_number() / m_frame_invert_count) & 1;
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
		u16 x = hpos - get_screen_area().left();
		u16 y = vpos - get_screen_area().top();
		u8 *scr = &m_screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		u8 *attr = &scr[0x2000];
		u16 *pix = &(bitmap.pix(vpos, hpos));

		while (hpos <= cliprect.right())
		{
			u16 ink = ((*attr >> 3) & 0x08) | (*attr & 0x07);
			u16 pap = (*attr >> 3) & 0x0f;
			u8 pix8 = (invert_attrs && (*attr & 0x80)) ? ~*scr : *scr;

			for (u8 b = (0x80 >> (x % 8)); b; b >>= 1, x++, hpos++)
				*pix++ = (pix8 & b) ? ink : pap;
			scr++;
			attr++;
		}
	}
}

void pentevo_state::pentevo_update_screen_zx16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO attrs not decoded
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
		u16 x = hpos - get_screen_area().left();
		u16 y = vpos - get_screen_area().top();
		u8 *scr = &m_screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		u16 *pix = &(bitmap.pix(vpos, hpos));

		while (hpos <= cliprect.right())
		{
			u16 ink = 0;
			u16 pap = 7;
			u8 pix8 = *scr;

			for (u8 b = (0x80 >> (x % 8)); b; b >>= 1, x++, hpos++)
				*pix++ = (pix8 & b) ? ink : pap;
			scr++;
		}
	}
}

void pentevo_state::pentevo_update_screen_tx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8* screen_location = m_ram->pointer() + ((BIT(m_port_7ffd_data, 3) ? 10 : 8) << 14);
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 y = vpos - get_screen_area().top();
		for (u16 hpos = cliprect.left() & 0xfff8; hpos <= cliprect.right();)
		{
			u16 x = hpos - get_screen_area().left();
			u8 *symb_location = screen_location + 0x1c0 + (x >> 4) + ((y >> 3) * 64);
			u8 *attr_location = symb_location + 0x2000 + BIT(x, 3);
			if (BIT(x, 3))
				symb_location += 0x1000;
			else
				attr_location += 0x1000;

			u8 attr = *attr_location;
			u8 fg = ((attr & 0x40) >> 3) | (attr & 0x07);
			u8 bg = (((attr & 0x80) >> 1) | (attr & 0x38)) >> 3;

			u8 chunk = m_char_ram->read((*symb_location << 3) + (y & 0x07));
			for (u8 i = 0x80; i; i >>= 1)
			{
				bitmap.pix(vpos, hpos++) = (chunk & i) ? fg : bg;
			}
		}
	}
}

u8 pentevo_state::nemo_ata_r(u8 cmd)
{
	bool data_read = (cmd & 0x7) == 0;
	u8 data;
	if (data_read && m_ata_data_hi_ready)
	{
		data = m_ata_data_latch;
		m_ata_data_hi_ready = false;
	}
	else
	{
		data = atm_state::ata_r(cmd << 5);
		m_ata_data_hi_ready = data_read;
	}

	return data;
}

void pentevo_state::nemo_ata_w(u8 cmd, u8 data)
{
	bool data_write = (cmd & 0x7) == 0;
	if (data_write && !m_ata_data_hi_ready)
	{
		m_ata_data_latch = data;
		m_ata_data_hi_ready = true;
	}
	else
	{
		atm_state::ata_w(cmd << 5, data);
		m_ata_data_hi_ready = false;
	}
}

u8 pentevo_state::spi_port_77_r(offs_t offset)
{
	return is_shadow_active() ? 0xff : 0x00;
}

void pentevo_state::spi_port_77_w(offs_t offset, u8 data)
{
	if (is_shadow_active())
		return atm_port_ff77_w(0x77 | offset, data);

	m_sdcard->spi_ss_w(BIT(data, 0));
	m_zctl_cs = BIT(data, 1);
}

u8 pentevo_state::spi_port_57_r(offs_t offset)
{
	if (m_zctl_cs)
		return 0xff;

	u8 din = m_zctl_di;
	spi_port_57_w(0, 0xff);
	return din;
}

void pentevo_state::spi_port_57_w(offs_t offset, u8 data)
{
	if (is_shadow_active() && BIT(offset, 15))
	{
		// same as #77 but shadow
		m_sdcard->spi_ss_w(BIT(data, 0));
		m_zctl_cs = BIT(data, 1);
	}
	else if (!m_zctl_cs)
	{
		for (u8 m = 0x80; m; m >>= 1)
		{
			m_sdcard->spi_clock_w(CLEAR_LINE); // 0-S R
			m_sdcard->spi_mosi_w(data & m ? 1 : 0);
			m_sdcard->spi_clock_w(ASSERT_LINE); // 1-L W
		}
	}
}

void pentevo_state::spi_miso_w(u8 data)
{
	m_zctl_di <<= 1;
	m_zctl_di |= data;
}

u8 pentevo_state::gluk_data_r(offs_t offset)
{
	if (offset & 0x0100) // #BF no-shadow port
	{
		if (is_shadow_active() || !BIT(m_port_eff7_data, 7))
			return 0xff;
	}
	else if (!is_shadow_active()) // #BE shadow port
		return 0xff;

	return m_gluk_ext == 2
		? m_keyboard->read()
		: m_glukrs->data_r();
}

void pentevo_state::gluk_data_w(offs_t offset, u8 data)
{
	if (offset & 0x0100) // #BF no-shadow port
	{
		if (is_shadow_active() || !BIT(m_port_eff7_data, 7))
			return atm_port_fff7_w(0xbff7, data);
	}
	else if (!is_shadow_active()) // #BE shadow port
		return;

	u8 addr = m_glukrs->address_r();
	if (addr >= 0xf0 && addr <= 0xf0)
	{
		m_gluk_ext = data;
		u8 m_fx[0xf] = {0x00};
		if (data == 0 || data == 1) // BASECONF_VERSION + BOOTLOADER_VERSION
		{
			strcpy((char *)m_fx, "M.A.M.E.");
			PAIR16 m_ver;
			m_ver.w = ((22 << 9) | (9 << 5) | 3); // y.m.d
			m_fx[0x0c] = m_ver.b.l;
			m_fx[0x0d] = m_ver.b.h;
		}

		for (u8 i = 0; i < 0xf; i++)
		{
			m_glukrs->address_w(0xf0 + i);
			m_glukrs->data_w(m_fx[i]);
		}
		m_glukrs->address_w(addr);
	}
	else
	{
		m_glukrs->data_w(data);
	}
}


void pentevo_state::pentevo_io(address_map &map)
{
	map.unmap_value_high();

	map(0x001f, 0x001f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	map(0x003f, 0x003f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	map(0x005f, 0x005f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	map(0x007f, 0x007f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	map(0x00ff, 0x00ff).mirror(0xff00).r(m_beta, FUNC(beta_disk_device::state_r));

	map(0x00ff, 0x00ff).select(0xff00).w(FUNC(pentevo_state::atm_port_ffff_w));
	map(0x00f6, 0x00f6).select(0xff08).rw(FUNC(pentevo_state::spectrum_ula_r), FUNC(pentevo_state::atm_ula_w));
	map(0x00fb, 0x00fb).mirror(0xff00).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x00fd, 0x00fd).mirror(0xff00).w(FUNC(pentevo_state::atm_port_7ffd_w));

	map(0x0077, 0x0077).select(0xff00).w(FUNC(pentevo_state::atm_port_ff77_w));
	map(0x3ff7, 0x3ff7).select(0xc000).w(FUNC(pentevo_state::atm_port_fff7_w));     // ATM
	map(0x37f7, 0x37f7).select(0xc000).w(FUNC(pentevo_state::pentevo_port_f7f7_w)); // PENTEVO
	map(0x3bf7, 0x3bf7).select(0xc000).w(FUNC(pentevo_state::pentevo_port_fbf7_w)); // RO
	map(0xeff7, 0xeff7).w(FUNC(pentevo_state::pentevo_port_eff7_w));

	// Gluk
	map(0xdef7, 0xdef7).lw8(NAME([this](offs_t offset, u8 data) { if (is_shadow_active()) m_glukrs->address_w(data); } ));
	map(0xdff7, 0xdff7).lw8(NAME([this](offs_t offset, u8 data) { if (!is_shadow_active() && BIT(m_port_eff7_data, 7)) m_glukrs->address_w(data); } ));
	map(0xbef7, 0xbef7).select(0x0100).rw(FUNC(pentevo_state::gluk_data_r), FUNC(pentevo_state::gluk_data_w));

	// Configuration
	map(0x00bf, 0x00bf).select(0xff00).rw(FUNC(pentevo_state::atm_port_bf_r), FUNC(pentevo_state::atm_port_bf_w));
	map(0x00be, 0x00be).select(0x0f00).r(FUNC(pentevo_state::atm_port_0fbd_r));
	map(0x00bd, 0x00bd).select(0x0f00).rw(FUNC(pentevo_state::atm_port_0fbd_r), FUNC(pentevo_state::atm_port_0fbd_w));
	map(0x10be, 0x10be).select(0x0f00).r(FUNC(pentevo_state::atm_port_1fbd_r));
	map(0x10bd, 0x10bd).select(0x0f00).rw(FUNC(pentevo_state::atm_port_1fbd_r), FUNC(pentevo_state::atm_port_1fbd_w));

	map(0x8000, 0x8000).mirror(0x3ffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc000).mirror(0x3ffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));

	// HDD: NEMO
	map(0x0010, 0x0010).select(0xffe0).lrw8(NAME([this](offs_t offset) { return nemo_ata_r(offset >> 5); })
		, NAME([this](offs_t offset, u8 data) { nemo_ata_w(offset >> 5, data); }));
	map(0x0011, 0x0011).mirror(0xff00).lrw8(NAME([this]() { m_ata_data_hi_ready = false; return m_ata_data_latch; })
		, NAME([this](offs_t offset, u8 data) { m_ata_data_hi_ready = true; m_ata_data_latch = data; }));
	map(0x00c8, 0x00c8).mirror(0xff00).lrw8(NAME([this]() { return m_ata->cs1_r(6 /* ? */); })
		, NAME([this](offs_t offset, u8 data) { m_ata->cs1_w(6, data); }));

	// SPI SD-card
	map(0x0077, 0x0077).select(0xff00).rw(FUNC(pentevo_state::spi_port_77_r), FUNC(pentevo_state::spi_port_77_w));
	map(0x0057, 0x0057).select(0xff00).rw(FUNC(pentevo_state::spi_port_57_r), FUNC(pentevo_state::spi_port_57_w));

	// Mouse
	map(0xfadf, 0xfadf).lr8(NAME([this]() { return 0x80 | (m_io_mouse[2]->read() & 0x07); }));
	map(0xfbdf, 0xfbdf).lr8(NAME([this]() { return  m_io_mouse[0]->read(); }));
	map(0xffdf, 0xffdf).lr8(NAME([this]() { return ~m_io_mouse[1]->read(); }));
}

void pentevo_state::init_mem_write()
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	mem.install_write_tap(0x0000, 0xffff, "mem_wait_w", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			if (BIT(m_port_bf_data, 2))
			{
				m_char_ram->write(offset & 0x7ff, data);
				m_gfxdecode->gfx(0)->mark_dirty((offset & 0x7ff) / 8);
			}
		}
		return data;
	});
}

void pentevo_state::machine_start()
{
	atm_state::machine_start();

	save_item(NAME(m_port_bf_data));
	save_item(NAME(m_port_eff7_data));
	save_item(NAME(m_gluk_ext));
	save_item(NAME(m_ata_data_hi_ready));
	save_item(NAME(m_nmi_exit));
	save_item(NAME(m_zctl_di));
	save_item(NAME(m_zctl_cs));

	init_mem_write();
}

void pentevo_state::machine_reset()
{
	m_port_eff7_data = 0;
	atm_state::machine_reset();

	m_glukrs->enable();

	m_port_bf_data = 0;

	m_nmi_exit = false;
	m_ata_data_hi_ready = false;
	m_gluk_ext = 0xff;
	m_zctl_cs = 1;
	m_zctl_di = 0xff;

	m_keyboard->write(0xff);
	while (m_keyboard->read() != 0) { /* invalidate buffer */ }
}

void pentevo_state::video_start()
{
	atm_state::video_start();
	m_char_location = m_char_ram->pointer();
	m_gfxdecode->gfx(0)->set_source(m_char_location);
}

INPUT_PORTS_START( pentevo )
	PORT_INCLUDE( spec_plus )

	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(30)

	PORT_START("mouse_input3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)

INPUT_PORTS_END

void pentevo_state::pentevo(machine_config &config)
{
	atmtb2(config);
	m_maincpu->set_addrmap(AS_IO, &pentevo_state::pentevo_io);
	TIMER(config, "nmi_timer").configure_periodic(FUNC(pentevo_state::nmi_check_callback), attotime::from_hz(50));

	m_ram->set_default_size("4M");
	RAM(config, m_char_ram).set_default_size("2048").set_default_value(0);

	GLUKRS(config, m_glukrs);
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set(FUNC(pentevo_state::spi_miso_w));

	AT_KEYB(config, m_keyboard, pc_keyboard_device::KEYBOARD_TYPE::AT, 3);
}


ROM_START( pentevo )
	ROM_REGION(0x090000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v0.59.02fe_tr503")

	// http://svn.zxevo.ru/revision.php?repname=pentevo&path=%2From%2Fzxevo_fe.rom&rev=1012&peg=1021
	ROM_SYSTEM_BIOS(0, "v0.59.02fe_tr503", "Reset Service v0.59.02 (FE), TR-DOS v5.03")
	ROMX_LOAD( "zxevo_05902fe_trd503.rom", 0x010000, 0x80000, CRC(f7f29dd8) SHA1(dfbcce980cf6244a9491aedf68ebb372677b0593), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v0.59.02fe", "Reset Service v0.59.02 (FE), NEO-DOS v0.53")
	ROMX_LOAD( "zxevo_05902fe.rom", 0x010000, 0x80000, CRC(df144c82) SHA1(e48b8a95576e0123764ff8cc34d9373dc95159bf), ROM_BIOS(1))

	// http://svn.zxevo.ru/revision.php?repname=pentevo&path=%2Fcfgs%2Fstandalone_base_trdemu%2Ftrunk%2Fzxevo_fw.bin&rev=994&peg=1021
	ROM_REGION(0x0C280, "fw", ROMREGION_ERASEFF)
	ROM_LOAD( "zxevo_fw.bin", 0x0000, 0xC280, CRC(aefbd8e5) SHA1(ac9a551ba15eeead76b5527fd5d23d824ae5176f))
ROM_END

} // Anonymous namespace


/*    YEAR  NAME        PARENT   COMPAT MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME                  FLAGS */
COMP( 2009, pentevo,    spec128, 0,     pentevo, pentevo, pentevo_state, empty_init, "NedoPC",   "ZX Evolution: BASECONF", 0)
