// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*******************************************************************************************

Sprinter Sp2000 (Peters Plus Ltd)

TODO:
----------:hi:----------
	- mem contention
----------:med:---------
----------:low:---------
	- gfx_layouts
	- spectrum parentless

*******************************************************************************************/

#include "emu.h"

#include "beta_m.h"
#include "spec128.h"
#include "speaker.h"

#include "bus/ata/atapicdr.h"
#include "bus/ata/ataintf.h"
#include "bus/ata/idehd.h"
#include "bus/centronics/ctronics.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/tmpz84c015.h"
#include "machine/ds128x.h"
#include "sound/ay8910.h"

#define LOG_IO    (1U << 1)
#define LOG_MEM   (1U << 2)
#define LOG_ACCEL (1U << 3)
#define LOG_WARN  (1U << 4)

#define VERBOSE ( /*LOG_GENERAL | LOG_IO | LOG_MEM | LOG_ACCEL |*/ LOG_WARN )
#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO,    __VA_ARGS__)
#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGACCEL(...) LOGMASKED(LOG_ACCEL, __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)

namespace {

#define X_SP 42_MHz_XTAL // TODO X1 after spectrumless

#define SPRINT_WIDTH         896
#define SPRINT_BORDER_RIGHT  48
#define SPRINT_SCREEN_XSIZE  640
#define SPRINT_BORDER_LEFT   48
#define SPRINT_XVIS          (SPRINT_BORDER_RIGHT + SPRINT_SCREEN_XSIZE + SPRINT_BORDER_LEFT)

#define SPRINT_HEIGHT        320
#define SPRINT_BORDER_TOP    16
#define SPRINT_SCREEN_YSIZE  256
#define SPRINT_BORDER_BOTTOM 16
#define SPRINT_YVIS          (SPRINT_BORDER_TOP + SPRINT_SCREEN_YSIZE + SPRINT_BORDER_BOTTOM)


class sprinter_state : public spectrum_128_state
{
public:
	sprinter_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_ata(*this, "ata%u", 1U)
		, m_beta(*this, BETA_DISK_TAG)
		, m_ay8910(*this, "ay8912")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_kbd(*this, "kbd")
		, m_io_mouse(*this, "mouse_input%u", 1U)
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vram(*this, "vram", 0x40000, ENDIANNESS_LITTLE)
		, m_fastram(*this, "fastram", 0x10000, ENDIANNESS_LITTLE)
		, m_bank0_fastram(*this, "bank0_fastram")
		, m_bank_view0(*this, "bank_view0")
		, m_bank_view3(*this, "bank_view3")
	{ }

	void sprinter(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void map_io(address_map &map);
	void map_mem(address_map &map);
	void map_fetch(address_map &map);
	u8 m1_r(offs_t offset);

	void init_taps();

	void update_memory();

	INTERRUPT_GEN_MEMBER(sprinter_int);
	TIMER_CALLBACK_MEMBER(cbl_tick);

	void sprinter_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_update_txt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_update_graph(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<tmpz84c015_device> m_maincpu;

private:

	static constexpr u16 BANK_RAM_MASK     = 1 << 8;
	static constexpr u16 BANK_FASTRAM_MASK = 1 << 9;
	static constexpr u16 BANK_ISA_MASK     = 1 << 10;
	static constexpr u16 BANK_WRDISBL_MASK = 1 << 12;
	enum accel_state : u8
	{
		OFF_HALT   = 0x76, // halt
		OFF        = 0x40, // ld b,b
		SET_BUFFER = 0x52, // ld d,d
		FILL       = 0x49, // ld c,c
		FILL_VERT  = 0x5b, // ld e,e
		DOUBLE     = 0x64, // ld h,h
		COPY       = 0x6d, // ld l,l
		COPY_VERT  = 0x7f, // ld a,a

		MODE_AND   = 0xa6, // and (hl)
		MODE_XOR   = 0xae, // xor (hl)
		MODE_OR    = 0xb6, //  or (hl)
		MODE_NOP   = 0xbe  //  cp (hl)
	};

	u8 dcp_r(offs_t offset);
	void dcp_w(offs_t offset, u8 data);

	u8 ata_data_r();
	void ata_data_w(u8 data);

	u8 *m_dcp_location;
	u8 m_dcpp_data[0x100];

	u16 m_pages[4] = {}; // internal state for faster calculations
	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);

	u8 cbl_data_r();
	void covox_w(u8 left, u8 right);

	void accel_control_r(u8 data);
	void accel_r_tap(offs_t offset, u8 &data);
	void accel_w_tap(offs_t offset, u8 &data);
	void update_accel_buffer(u8 idx, u8 data);

	required_device<ds12885_device> m_rtc;
	required_device_array<ata_interface_device, 2> m_ata;
	required_device<beta_disk_device> m_beta;
	required_device<ay8910_device> m_ay8910;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<pc_kbdc_device> m_kbd;
	required_ioport_array<3> m_io_mouse;
	required_device<device_palette_interface> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	memory_share_creator<u8> m_vram;
	memory_share_creator<u8> m_fastram;
	memory_bank_creator m_bank0_fastram;
	memory_view m_bank_view0;
	memory_view m_bank_view3;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	bool m_z80_m1;

	bool m_starting;
	bool m_dos; // 0-on, 1-off
	bool m_cash_on;

	u8 m_cnf;
	bool m_rom_sys;
	bool m_ram_sys;
	bool m_sys_pg;
	bool m_arom16;
	u8 m_rom_rg;
	u8 m_pn;
	u8 m_sc;
	u8 m_all_mode;
	u8 m_port_y;
	u8 m_rgmod;
	u8 m_ram1;
	u8 m_ram2;
	u8 m_pg3;
	u8 m_hold;

	bool m_ata_selected; // 0-primary, 1-secondary
	bool m_ata_data_flip;
	u16 m_ata_data_latch;

	// Accelerator
	bool m_skip_write;
	u8 m_prf_d;
	u16 m_accel_buffer_size;
	u8 m_accel_buffer[256] = {};
	accel_state m_accel_state;
	accel_state m_accel_mode;

	// Covox Blaster
	u8 m_cbl_xx;
	u8 m_cbl_data[256] = {};
	u8 m_cbl_data_r_pos;
	u8 m_cbl_data_w_pos;
	emu_timer *m_cbl_timer = nullptr;
};

void sprinter_state::update_memory()
{
	const bool pre_rom = m_rom_sys || m_cash_on;
	const bool pre_cash = !m_cash_on;
	if (!pre_rom && pre_cash)
	{
		m_pages[0] = (m_rom_rg & 0x0f) ^ (!m_sys_pg << 3);
		m_bank_rom[0]->set_entry(m_pages[0]);
		m_bank_view0.select(1);
	}
	else if (pre_rom && !pre_cash)
	{
		m_pages[0] = BANK_FASTRAM_MASK | (m_rom_rg & 3);
		m_bank0_fastram->set_entry(m_pages[0] & 0xff);
		m_bank_view0.select(2);
	}
	else
	{
		const bool cash_on = 0;
		const bool nmi_ena = 1;
		const bool sc0 = BIT(m_sc, 0);
		const bool sc_lc = !(sc0 && m_ram_sys) && !cash_on;
		const u8 spr_ = BIT(m_sc, 1) ? 0 : ((m_dos << 1) | (BIT(m_pn, 4) || !m_dos));
		const u8 pg0 = 0xe0
			| ((sc0 || !m_ram_sys || cash_on || !nmi_ena) << 3)
			| (((m_arom16 && !(sc0 && m_ram_sys)) || (cash_on && nmi_ena)) << 2)
			| (((BIT(spr_, 1) && sc_lc) || !m_ram_sys || !nmi_ena) << 1)
			| ((BIT(spr_, 0) && sc_lc) || !m_ram_sys || !nmi_ena);

		m_pages[0] = BANK_RAM_MASK | m_dcpp_data[pg0];
		m_bank_ram[0]->set_entry(m_pages[0] & 0xff);
		if (sc0 && m_ram_sys)
			m_bank_view0.disable();
		else
		{
			m_pages[0] |= BANK_WRDISBL_MASK;
			m_bank_view0.select(0);
		}
	}

	m_pages[1] = BANK_RAM_MASK | m_ram1;
	m_bank_ram[1]->set_entry(m_ram1);
	m_pages[2] = BANK_RAM_MASK | m_ram2;
	m_bank_ram[2]->set_entry(m_ram2);

	m_pg3 = 0xc0 | (BIT(~m_pn, 7) << 5) | 0x10 | (((BIT(m_sc, 4) && BIT(~m_cnf, 7)) || (BIT(m_cnf, 7) && BIT(m_pn, 6))) << 3) | (m_pn & 0x07);
	m_pages[3] = (m_starting ? 0x40 : m_dcpp_data[m_pg3]);
	m_bank_ram[3]->set_entry(m_pages[3] & 0xff);
	if (BIT(m_sc, 4) && ((m_pages[3] & 0xf9) == 0xd0))
	{
		m_pages[3] |= BANK_ISA_MASK;
		m_bank_view3.select(0);
	}
	else
	{
		m_bank_ram[3]->set_entry(m_pages[3]);
		m_bank_view3.disable();
		m_pages[3] |= BANK_RAM_MASK;
	}

	LOGMEM("MEM: %x %x %x %x\n", m_pages[0], m_pages[1], m_pages[2], m_pages[3]);
}

void sprinter_state::sprinter_palette(palette_device &palette) const
{
	const rgb_t colors[256 * 8] = { palette_device::BLACK };
	palette.set_pen_colors(0, colors);
}

u32 sprinter_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_graph(screen, bitmap, cliprect);
	screen_update_txt(screen, bitmap, cliprect);

	return 0;
}

void sprinter_state::screen_update_txt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool flash = BIT(screen.frame_number(), 4);
	const s8 dy = 7 - (m_hold >> 4);
	const s8 dx = (7 - (m_hold & 0x0f)) * 2;
	for(u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		const u16 b8 = (SPRINT_HEIGHT + vpos - SPRINT_BORDER_TOP - dy) % SPRINT_HEIGHT;

		u8* line1 = nullptr;
		u8* mode = nullptr;
		u8 attr, symb;
		u16 *pix = &(bitmap.pix(vpos, cliprect.left()));
		for(u16 hpos = cliprect.left(); hpos <= cliprect.right(); hpos++)
		{
			const u16 a16 = (SPRINT_WIDTH + hpos - SPRINT_BORDER_LEFT - dx) % SPRINT_WIDTH;
			if ((line1 == nullptr) || (a16 & 7) == 0)
			{
				// 16x8 block descriptor
				const bool do_init = line1 == nullptr;
				if (do_init || ((a16 & 15) == 0))
				{
					line1 = m_vram + (1 + (a16 >> 4) * 2 + 0x80 * (m_rgmod & 1)) * 1024 + 0x300;
					mode = line1 + (b8 >> 3) * 4;
				}
				if (BIT(mode[0], 4))
				{
					if (!BIT(mode[0], 5) && (((a16 & 15) == 8) || (do_init && (a16 & 8))))
						mode = mode + 1024;
					attr = m_vram[(mode[2] << 10) | (BIT(mode[0], 0, 4) << 6) | (BIT(m_pn, 3) << 5) | 0b11000 | BIT(mode[0], 6, 2)];
					symb = m_vram[((mode[1] << 10) | (BIT(mode[0], 0, 4) << 6) | (BIT(m_pn, 3) << 5) | (BIT(mode[0], 6, 2) << 3) | (b8 & 7))];
				}
			}

			if (!BIT(mode[0], 4)) // skip graph block
			{
				hpos += 15 - (a16 & 15); // skip to the last of 16px block
				pix += 16 - (a16 & 15);
			}
			else
			{
				u16 pal;
				if (BIT(mode[0], 5, 3) == 7)
				{
					if ((mode[0] & 0x0c) == 0x0c)
						pal = 0x400;
					else
						pal = 0x400 | ((m_port_fe_data & 0x07) << 3) | (m_port_fe_data & 0x07);
				}
				else
				{
					const u8 bit = 1 << (7 - ((a16 >> BIT(mode[0], 5)) & 7));
					pal = attr + 0x400 + 0x100 * bool(symb & bit) + 0x200 * flash;
				}
				*pix++ = pal;

			}
		}
	}
}

void sprinter_state::screen_update_graph(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const s8 dy = 7 - (m_hold >> 4);
	const s8 dx = (7 - (m_hold & 0x0f)) * 2;
	for(u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		const u16 b8 = (SPRINT_HEIGHT + vpos - SPRINT_BORDER_TOP - dy) % SPRINT_HEIGHT;

		u8* line1 = nullptr;
		u8* mode = nullptr;
		u16 pal, x;
		u8 y;
		u16 *pix = &(bitmap.pix(vpos, cliprect.left()));
		for(u16 hpos = cliprect.left(); hpos <= cliprect.right(); hpos++)
		{
			const u16 a16 = (SPRINT_WIDTH + hpos - SPRINT_BORDER_LEFT - dx) % SPRINT_WIDTH;
			if ((line1 == nullptr) || ((a16 & 15) == 0))
			{
				// 16x8 block descriptor
				line1 = m_vram + (1 + (a16 >> 4) * 2 + 0x80 * (m_rgmod & 1)) * 1024 + 0x300;
				mode = line1 + (b8 >> 3) * 4;
				if(!BIT(mode[0], 4))
				{
					pal = BIT(mode[0], 6, 2) << 8;
					x = (BIT(mode[0], 0, 4) << 6) | (BIT(mode[1], 0, 3) << 3);
					y = BIT(mode[1], 3, 5) << 3;
				}
			}

			if (BIT(mode[0], 4)) // skip txt block
			{
				hpos += 15 - (a16 & 15); // skip to the last of 16px block
				pix += 16 - (a16 & 15);
			}
			else
			{
				const u8 color = m_vram[(y + (b8 & 7)) * 1024 + x + ((a16 & 15) >> 1)];
				*pix++ = pal + (BIT(mode[0], 5) ? color : ((a16 & 1) ? (color & 0x0f) : (color >> 4)));
			}
		}
	}
}

u8 sprinter_state::dcp_r(offs_t offset)
{
	if (m_starting) m_starting = 0;
	// TODO ee, ef z84 internal ports must be handled by cpu which is not implemented yet.
	if ((offset & 0xfe) == 0xee)
		return 0;

	if (!machine().side_effects_disabled() && ((offset & 0x7f) == 0x7b))
	{
		m_cash_on = BIT(offset, 7);
		update_memory();
	}

	const u16 dcp_offset = (BIT(m_cnf, 3, 2) << 12) | (0 << 11) | (m_dos << 10) | (1 << 9) | (BIT(offset, 14, 2) << 7) | (BIT(offset, 13) << 4) | (BIT(offset, 7) << 3) | (offset & 0x67);
	const u8 dcpp = m_dcp_location[dcp_offset];

	u8 data = m_dcpp_data[dcpp];
	switch (dcpp)
	{
	case 0x00: // no port
		data = 0xff;
		break;

	case 0x10:
		data = m_beta->status_r();
		break;
	case 0x11:
		data = m_beta->track_r();
		break;
	case 0x12:
		data = m_beta->sector_r();
		break;
	case 0x13:
		data = m_beta->data_r();
		break;
	case 0x15:
		data = m_beta->state_r();
		break;

	case 0x1c:
		data = m_rtc->read(1);
		break;

	case 0x20:
		data = ata_data_r();
		break;
	case 0x21 ... 0x27:
		data = m_ata[m_ata_selected]->cs0_r(dcpp & 0x07);
		break;
	case 0x28:
		data = m_ata[m_ata_selected]->cs1_r(6);
		break;
	case 0x29:
		data = m_ata[m_ata_selected]->cs1_r(7);
		break;

	case 0x40:
		data = spectrum_ula_r(offset) & ~0xa0;
		data |= (BIT(m_cbl_xx, 7) && (m_screen->vpos() >= (SPRINT_BORDER_TOP + SPRINT_SCREEN_YSIZE))) << 5;
		data |= m_cbl_data_r_pos & 0x80;
		break;

	case 0x52: // AY8910
		data = m_ay8910->data_r();
		break;

	case 0x58: // Kempston Mouse
		switch (offset >> 8)
		{
		case 0xfa:
			data = m_io_mouse[2]->read();
			break;
		case 0xfb:
			data = m_io_mouse[0]->read();
			break;
		case 0xff:
			data = m_io_mouse[1]->read();
			break;
		default:
			data = 0xff;
		}
		break;

	case 0x89:
		data = m_cbl_xx;
		break;

	// value ports
	case 0x88: // fastram
	case 0xc0 ... 0xff:
		break;

	default:
		LOGIO("IOr: %02X / %X > %x\n", dcpp, offset, data);
		break;
	}

	return data;
}

void sprinter_state::dcp_w(offs_t offset, u8 data)
{
	if (((offset & 0xfe) == 0xee) || m_starting)
		return;  // see: dcp_r

	if ((offset & 0xbf) == 0x3c)
	{
		m_rom_sys = BIT(~offset, 6);
		if (BIT(~data, 1))
			m_sys_pg = BIT(m_rom_rg, 4) || BIT(data, 0);
		update_memory();
	}
	if (!m_rom_sys && ((offset & 0xff) == 0x5c))
	{
		m_rom_rg = data;
		m_sys_pg |= BIT(m_rom_rg, 4);
		update_memory();
	}


	const u16 dcp_offset = (BIT(m_cnf, 3, 2) << 12) | (0 << 11) | (m_dos << 10) | (0 << 9) | (BIT(offset, 14, 2) << 7) | (BIT(offset, 13) << 4) | (BIT(offset, 7) << 3) | (offset & 0x67);
	u8 dcpp = m_dcp_location[dcp_offset];
	if (dcpp >= 0xf0) dcpp = 0xf0 | m_pg3;
	m_dcpp_data[dcpp] = data;

	switch (dcpp)
	{
	case 0x10:
		m_beta->command_w(data);
		break;
	case 0x11:
		m_beta->track_w(data);
		break;
	case 0x12:
		m_beta->sector_w(data);
		break;
	case 0x13:
		m_beta->data_w(data);
		break;
	case 0x14:
		m_beta->param_w(data);
		break;

	case 0x1d:
	case 0x1e:
		m_rtc->write(~dcpp & 1, data);
		break;

	case 0x20:
		ata_data_w(data);
		break;
	case 0x21 ... 0x27:
		m_ata[m_ata_selected]->cs0_w(dcpp & 0x07, data);
		break;
	case 0x28:
		m_ata[m_ata_selected]->cs1_w(6, data);
		break;
	case 0x2a: // HDD1 - secondary
		m_ata_data_flip = false;
		m_ata_selected = 1;
		break;
	case 0x2b: // HDD2 - primary
		m_ata_data_flip = false;
		m_ata_selected = 0;
		break;

	case 0x88:
		m_cbl_data[m_cbl_data_w_pos++] = data;
		if (BIT(~m_cbl_xx, 7))
			covox_w(data, data);
		break;

	case 0x89:
		m_cbl_xx = data;
		if (BIT(m_cbl_xx, 7))
		{
			XTAL freq = 16_kHz_XTAL; // 0 + undefined (2..7)
			switch (m_cbl_xx & 0x0f)
			{
			case 1:
				freq = 22_kHz_XTAL;
				break;
			case 8:
				freq = 7.8125_kHz_XTAL;
				break;
			case 9:
				freq = 10.9375_kHz_XTAL;
				break;
			case 10:
				freq = 15.625_kHz_XTAL;
				break;
			case 11:
				freq = 21.875_kHz_XTAL;
				break;
			case 12:
				freq = 31.25_kHz_XTAL;
				break;
			case 13:
				freq = 43.75_kHz_XTAL;
				break;
			case 14:
				freq = 54.6875_kHz_XTAL;
				break;
			case 15:
				freq = 109.375_kHz_XTAL;
				break;
			}
			m_cbl_data_r_pos = 0;
			m_cbl_timer->adjust(attotime::zero, 0, attotime::from_hz(freq));
		}
		else
			m_cbl_timer->adjust(attotime::never);
		break;

	case 0x8f: // rom & fastram page
		m_rom_rg = data;
		m_sys_pg = BIT(m_rom_rg, 4) || BIT(data, 0);
		update_memory();
		break;

	case 0x90: // AY8910
		m_ay8910->address_w(data);
		break;
	case 0x91:
		m_ay8910->data_w(data);
		break;

	case 0xc0: // 1FFD
		m_sc = data;
		if (BIT(m_cnf, 6)) m_sc = 0;      // CNF_SC_RESET
		update_memory();
		break;
	case 0xc1: // 7FFD
		m_pn = data;
		if (BIT(m_cnf, 5)) m_pn &= 0xc0;  // CNF_PN[5..0]_RESET
		if (BIT(~m_cnf, 7)) m_pn &= 0x1f; // CNF_PN[7..6]_RESET
		update_memory();
		break;
	case 0xc2:
		m_screen->update_now();
		spectrum_ula_w(offset, data);
		break;
	case 0xc3:
		m_all_mode = data;
		break;
	case 0xcb:
		m_hold = data;
		break;
	case 0xc4:
		m_port_y = data;
		break;
	case 0xc5:
		m_screen->update_now();
		m_rgmod = data;
		break;

	case 0xc6: // CNF/SYS
		m_ram_sys = BIT(~offset, 6);
		if (BIT(data, 1))
			m_maincpu->set_clock(X_SP / (BIT(data, 0) ? 2 : 12)); // 1 - 21MHz, 0 - 3.5MHz
		else
			m_arom16 = BIT(data, 0);

		if (BIT(data, 2))
		{
			m_cnf = data;
			if (BIT(m_cnf, 5)) m_pn &= 0xc0;  // CNF_PN[5..0]_RESET
			if (BIT(m_cnf, 6)) m_sc = 0;      // CNF_SC_RESET
			if (BIT(~m_cnf, 7)) m_pn &= 0x1f; // CNF_PN[7..6]_RESET
		}

		update_memory();
		break;

	case 0xe9:
		m_ram1 = data;
		update_memory();
		break;
	case 0xea:
		m_ram2 = data;
		update_memory();
		break;
	case 0xd0 ... 0xe8: // ROM/RAM pages
	case 0xeb ... 0xff:
		update_memory();
		break;

	// no port
	case 0x00:
		break;

	default:
		LOGIO("IOw: %02X / %X < %x\n", dcpp, offset, data);
		break;
	}
}

u8 sprinter_state::ata_data_r()
{
	if (!machine().side_effects_disabled())
	{
		m_ata_data_flip ^= true;
		if (m_ata_data_flip)
		{
			const u16 data = m_ata[m_ata_selected]->cs0_r(0);
			m_ata_data_latch = data >> 8;
			return data;
		}
	}

	return m_ata_data_latch;
}

void sprinter_state::ata_data_w(u8 data)
{
	m_ata_data_flip ^= true;
	if (m_ata_data_flip)
		m_ata_data_latch = data;
	else
		m_ata[m_ata_selected]->cs0_w(0, (data << 8) | m_ata_data_latch);
}

void sprinter_state::covox_w(u8 left, u8 right)
{
	m_centronics->write_strobe(1);
	m_centronics->write_autofd(0);
	m_cent_data_out->write(left);
	m_centronics->write_strobe(0);
	m_centronics->write_autofd(1);
	m_cent_data_out->write(right);
}

void sprinter_state::accel_control_r(u8 data)
{
	const bool is_prefix = (data == 0xcb) || (data == 0xdd) || (data == 0xed) || (data == 0xfd);
	if (!is_prefix && !m_prf_d) // neither prefix nor prefixed
	{
		const accel_state state_candidate = static_cast<accel_state>(data);
		switch(state_candidate)
		{
		case OFF_HALT:
		case OFF:
			m_accel_state = OFF;
			LOGACCEL("Accel: OFF\n");
			break;

		case SET_BUFFER:
		case FILL:
		case FILL_VERT:
		case COPY:
		case COPY_VERT:
		case DOUBLE:
			m_accel_state = state_candidate;
			m_accel_mode = MODE_NOP;
			break;

		case MODE_AND:
		case MODE_XOR:
		case MODE_OR:
			m_accel_mode = state_candidate;
			break;

		default:
			break;
		}
	}
	m_prf_d = is_prefix;
}

void sprinter_state::accel_r_tap(offs_t offset, u8 &data)
{
	const std::string m = m_accel_mode == MODE_AND ? "&" : m_accel_mode == MODE_OR ? "|" : m_accel_mode == MODE_XOR ? "^" : "";
	if (m_accel_state == SET_BUFFER)
	{
		m_accel_buffer_size = data ? data : 256;
		LOGACCEL("Accel buffer: %d\n", m_accel_buffer_size);
	}
	else if (m_pages[offset >> 14] & BANK_RAM_MASK) // block ops RAM only
	{
		if (m_accel_state == COPY)
		{
			LOGACCEL("Accel rCOPY: %s%02x\n", m, offset);
			for (auto i = 0; i < m_accel_buffer_size; i++)
			{
				const u16 addr = offset + i;
				data = (m_pages[addr >> 14] & BANK_RAM_MASK) ? ram_r(addr) : 0xff;
				update_accel_buffer(i, data);
			}
		}
		else if (m_accel_state == COPY_VERT)
		{
			LOGACCEL("Accel rCOPY_GR: %s%02x (%x)\n", m, offset, m_port_y);
			for (auto i = 0; i < m_accel_buffer_size; i++)
			{
				data = ram_r(offset);
				update_accel_buffer(i, data);
				m_port_y++;
			}
		}
		else
			return;

		const int icount = 6 * m_accel_buffer_size * m_maincpu->clock() / X_SP; // 6 42Mhz clocks each
		m_maincpu->adjust_icount(-icount);
	}
}

void sprinter_state::accel_w_tap(offs_t offset, u8 &data)
{
	if (m_accel_state == SET_BUFFER)
	{
		m_accel_buffer_size = data ? data : 256;
		LOGACCEL("Accel buffer: %d\n", m_accel_buffer_size);
	}
	else if (m_pages[offset >> 14] & BANK_RAM_MASK) // block ops RAM only
	{
		u16 ops = m_accel_buffer_size;
		if (m_accel_state == FILL)
		{
			LOGACCEL("Accel wFILL: %02x\n", offset);
			for (auto i = 0; i < m_accel_buffer_size; i++)
			{
				const u16 addr = offset + i;
				if ((m_pages[addr >> 14] & BANK_RAM_MASK) && (~m_pages[addr >> 14] & BANK_WRDISBL_MASK))
					ram_w(addr, data);
			}
		}
		else if (m_accel_state == FILL_VERT)
		{
			LOGACCEL("Accel wFILL_VERT: %02x (%x)\n", offset, m_port_y);
			for (auto i = 0; i < m_accel_buffer_size; i++)
			{
				ram_w(offset, data);
				m_port_y++;
			}
		}
		else if (m_accel_state == DOUBLE)
		{
			ram_w(offset, data);
			u16 addr = offset ^ 1;
			if ((m_pages[addr >> 14] & BANK_RAM_MASK) && (~m_pages[addr >> 14] & BANK_WRDISBL_MASK))
				ram_w(addr, data);
			ops = 2;
		}
		else if (m_accel_state == COPY)
		{
			LOGACCEL("Accel wCOPY: %02x\n", offset);
			for (auto i = 0; i < m_accel_buffer_size; i++)
			{
				u16 addr = offset + i;
				if ((m_pages[addr >> 14] & BANK_RAM_MASK) && (~m_pages[addr >> 14] & BANK_WRDISBL_MASK))
					ram_w(addr, m_accel_buffer[i]);
			}
		}
		else if (m_accel_state == COPY_VERT)
		{
			LOGACCEL("Accel wCOPY_VERT: %02x (%x)\n", offset, m_port_y);
			for (auto i = 0; i < m_accel_buffer_size; i++)
			{
				ram_w(offset, m_accel_buffer[i]);
				m_port_y++;
			}
		}
		else
			return;

		m_skip_write = true;
		const int icount = 6 * ops * m_maincpu->clock() / X_SP; // 6 42Mhz clocks each
		m_maincpu->adjust_icount(-icount);
	}
}

void sprinter_state::update_accel_buffer(u8 idx, u8 data)
{
	switch (m_accel_mode)
	{
	case MODE_AND:
		m_accel_buffer[idx] &= data;
		break;
	case MODE_OR:
		m_accel_buffer[idx] |= data;
		break;
	case MODE_XOR:
		m_accel_buffer[idx] ^= data;
		break;
	case MODE_NOP:
		m_accel_buffer[idx] = data;
		break;
	default:
		assert(false);
	}
}

u8 sprinter_state::ram_r(offs_t offset)
{
	const u8 bank = offset >> 14;
	if ((m_pages[bank] & 0xf0) == 0x50)
		return m_ram->pointer()[(0x50 << 14) + m_port_y * 1024 + (offset & 0x3ff)];
	else
		return reinterpret_cast<u8 *>(m_bank_ram[bank]->base())[offset & 0x3fff];
}

void sprinter_state::ram_w(offs_t offset, u8 data)
{
	if (m_skip_write)
	{
		m_skip_write = false;
		return;
	}

	const u8 bank = offset >> 14;
	const u8 page = m_pages[bank] & 0xff;
	if ((page & 0xf0) == 0x50)
	{
		const u16 vaddr = (offset & 0x03ff);
		u8* line = m_vram + (m_port_y * 1024);

		if (BIT(~page, 2))
			m_ram->pointer()[(0x50 << 14) + m_port_y * 1024 + (offset & 0x3ff)] = data;
		if (!(BIT(page, 3) && (data == 0xff)))
		{
			m_screen->update_now();
			line[vaddr] = data;
		}

		if (vaddr >= 0x3e0)
		{
			const u8 pal_num = BIT(vaddr, 2, 3);
			const u16 p_red = 0x3e0 + pal_num * 4;
			m_palette->set_pen_color(m_port_y + pal_num * 256, rgb_t(line[p_red], line[p_red + 1], line[p_red + 2]));
		}
	}
	else
	{
		if (~m_all_mode & 1)
		{
			const bool sp_scr = BIT(offset, 14) && (BIT(~offset, 15) || ((m_pg3 & 0x3d) == 0x35)); // B"1101X1"
			const bool zx_screen = sp_scr && !(BIT(offset, 13) && BIT(~m_port_y, 7)) && BIT(~m_port_y, 6);
			if (zx_screen)
			{
				const bool zxa15 = BIT(offset, 15) ? BIT(m_pg3, 1) : 0; // SP_SA
				const u8 zxs = m_port_y & 0x3f;
				const u32 vxa = (BIT(offset, 0, 8) << 10) | (BIT(zxs, 1, 4) << 6) | ((BIT(zxs, 0) ^ zxa15 ^ BIT(offset, 13)) << 5) | BIT(offset, 8, 5);

				m_screen->update_now();
				m_vram[vxa] = data;
			}
		}

		if (page == 0xfd)
			m_cbl_data[offset & 0xff] = data;

		reinterpret_cast<u8 *>(m_bank_ram[bank]->base())[offset & 0x3fff] = data;
	}
}

u8 sprinter_state::m1_r(offs_t offset)
{
	m_z80_m1 = 1;
	u8 data = m_program.read_byte(offset);
	m_z80_m1 = 0;

	if (!machine().side_effects_disabled() && (m_all_mode & 1))
		accel_control_r(data);

	return data;
}

void sprinter_state::map_fetch(address_map &map)
{
	// Overlap with previous because we want real addresses on the 3e00-3fff range
	map(0x0000, 0x3fff).lr8(NAME([this](u16 offset)
	{
		return m1_r(offset);
	}));
	map(0x3d00, 0x3dff).lr8(NAME([this](u16 offset)
	{
		if (!machine().side_effects_disabled() && m_dos && BIT(m_pn, 4))
		{
			m_dos = 0;
			update_memory();
		}
		return m1_r(offset + 0x3d00);
	}));
	map(0x4000, 0xffff).lr8(NAME([this](u16 offset)
	{
		if (!machine().side_effects_disabled() && !m_dos)
		{
			m_dos = 1;
			update_memory();
		}
		return m1_r(offset + 0x4000);
	}));
}

void sprinter_state::map_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sprinter_state::ram_r), FUNC(sprinter_state::ram_w));

	map(0x0000, 0x3fff).view(m_bank_view0);
	m_bank_view0[0](0x0000, 0x3fff).nopw(); // RAM RO
	m_bank_view0[1](0x0000, 0x3fff).nopw().bankr(m_bank_rom[0]);
	m_bank_view0[2](0x0000, 0x3fff).bankrw(m_bank0_fastram);

	map(0xc000, 0xffff).view(m_bank_view3);
	m_bank_view3[0](0xc000, 0xffff).noprw(); // ISA
}

void sprinter_state::map_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(sprinter_state::dcp_r), FUNC(sprinter_state::dcp_w));
}

void sprinter_state::init_taps()
{
	address_space &prg = m_maincpu->space(AS_PROGRAM);
	prg.install_read_tap(0x0000, 0xffff, "accel_read", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!machine().side_effects_disabled() && !m_z80_m1 && (m_all_mode & 1) && (m_accel_state != OFF))
			accel_r_tap(offset, data);
	});
	prg.install_write_tap(0x0000, 0xffff, "accel_write", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!m_z80_m1 && (m_all_mode & 1) && (m_accel_state != OFF))
			accel_w_tap(offset, data);
	});
}

void sprinter_state::machine_start()
{
	spectrum_128_state::machine_start();

	save_item(NAME(m_pages));
	save_item(NAME(m_dcpp_data));

	save_item(NAME(m_accel_buffer_size));
	//save_item(NAME(m_accel_buffer));
	//save_item(NAME(m_accel_state));
	//save_item(NAME(m_accel_mode));

	save_item(NAME(m_cbl_data));

	m_beta->enable();

	// reconfigure ROMs
	memory_region *rom = memregion("maincpu");
	m_bank_rom[0]->configure_entries(0, rom->bytes() / 0x4000, rom->base(), 0x4000);
	m_bank0_fastram->configure_entries(0, m_fastram.bytes() / 0x4000, m_fastram.target(), 0x4000);
	for (auto i = 0; i < 4; i++)
		m_bank_ram[i]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);

	m_dcp_location = m_ram->pointer() + (0x40 << 14);
	m_maincpu->space(AS_PROGRAM).specific(m_program);

	u8 port_default[16*4] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Cx - SYS PORTS COPIES
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // Dx - RAM PAGES
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x00, 0x05, 0x02, 0x41, 0xff, 0x00, 0x00, 0x41, // Ex - ROM PAGES
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // Fx - RAM PAGES
	};
	for (u8 i = 0; i < 16*4; i++)
		m_dcpp_data[0xc0 + i] = port_default[i];

	m_all_mode = 0x00; // c3
	m_port_y   = 0x00; // c4
	m_rgmod    = 0x00; // c5
	m_hold     = 0x77; // cb
	m_ram1     = 0x05; // e9
	m_ram2     = 0x02; // ea
}

void sprinter_state::machine_reset()
{
	m_cbl_timer->adjust(attotime::never);

	spectrum_128_state::machine_reset();

	m_starting = 1;
	m_dos = 1; // off
	m_rom_sys = 0;
	m_ram_sys = 0;
	m_sys_pg = 0;
	m_arom16 = 0;
	m_cnf = 0x00;
	m_pn = 0x00;
	m_sc = 0x00;
	m_rom_rg = 0x00;
	m_cash_on = 0;

	m_skip_write = false;
	m_prf_d = false;
	m_accel_state = OFF;

	m_cbl_xx = 0;
	m_cbl_data_r_pos = m_cbl_data_w_pos = 0;

	m_ata_selected = 0;
	m_ata_data_flip = false;

	update_memory();
}

static const gfx_layout sprinter_charlayout =
{
	8, 8,            // 8 x 8 characters
	256,             // 256 characters
	1,               // 1 bits per pixel
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	1024 * 8         // every char takes 8 bytes
};

static GFXDECODE_START( gfx_sprinter )
	GFXDECODE_RAM( "vram", 0x2c0, sprinter_charlayout, 0x70f, 1 )
GFXDECODE_END

void sprinter_state::video_start()
{
	spectrum_state::video_start();

	m_contention_pattern = {};
	init_taps();

	m_cbl_timer = timer_alloc(FUNC(sprinter_state::cbl_tick), this);
}

static void sprinter_ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_CDROM);
}

u8 sprinter_state::cbl_data_r()
{
	const bool cbl_mode16 = BIT(m_cbl_xx, 5);
	if (cbl_mode16)
		m_cbl_data_r_pos++;
	u8 data = m_cbl_data[m_cbl_data_r_pos++] ^ (cbl_mode16 << 7);
	return data;
}

TIMER_CALLBACK_MEMBER(sprinter_state::cbl_tick)
{
	u8 left = cbl_data_r();
	u8 right = BIT(m_cbl_xx, 6) ? cbl_data_r() : left;
	covox_w(left, right);

	if (BIT(m_cbl_xx, 4) && !(m_cbl_data_r_pos & 0x7f)) // CBL_INT_ENA
		irq_on(0);
}

INTERRUPT_GEN_MEMBER(sprinter_state::sprinter_int)
{
	// Pentagon's INT for now
	m_irq_on_timer->adjust(attotime(0, m_screen->time_until_pos(287, 375 * 2).as_attoseconds()));
}

INPUT_PORTS_START( sprinter )
	PORT_INCLUDE( spec128 )

	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_INVERT PORT_SENSITIVITY(30)

	PORT_START("mouse_input3")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)
INPUT_PORTS_END

void sprinter_state::sprinter(machine_config &config)
{
	spectrum_128(config);
	config.device_remove("palette");

	m_ram->set_default_size("64M");

	TMPZ84C015(config.replace(), m_maincpu, X_SP / 12); // 3.5MHz default
	m_maincpu->set_m1_map(&sprinter_state::map_fetch);
	m_maincpu->set_memory_map(&sprinter_state::map_mem);
	m_maincpu->set_io_map(&sprinter_state::map_io);
	m_maincpu->set_vblank_int("screen", FUNC(sprinter_state::sprinter_int));
	m_maincpu->nomreq_cb().set_nop();

	m_screen->set_raw(X_SP / 3, SPRINT_WIDTH, SPRINT_HEIGHT, { 0, SPRINT_XVIS - 1, 0, SPRINT_YVIS - 1 });
	m_screen->set_screen_update(FUNC(sprinter_state::screen_update));

	PALETTE(config, "palette", FUNC(sprinter_state::sprinter_palette), 256 * 8);

	PC_KBDC(config, m_kbd, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_kbd->out_data_cb().set(m_maincpu, FUNC(tmpz84c015_device::rxa_w)); // KBD_DATR
	m_kbd->out_clock_cb().set(m_maincpu, FUNC(tmpz84c015_device::rxca_w)); // KBD_CLKR
	m_kbd->out_clock_cb().append(m_maincpu, FUNC(tmpz84c015_device::txca_w));

	m_maincpu->set_clk_trg<0>(X_SP / 48);
	m_maincpu->set_clk_trg<1>(X_SP / 48);
	m_maincpu->set_clk_trg<2>(X_SP / 48);

	rs232_port_device &m_rs232(RS232_PORT(config, "rs232", default_rs232_devices, "microsoft_mouse"));
	m_rs232.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	m_rs232.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	m_rs232.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	m_rs232.rxd_handler().set(m_maincpu, FUNC(tmpz84c015_device::rxb_w)); // MOUSE_D
	m_maincpu->out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd)); // TXDB
	m_maincpu->zc_callback<0>().set(m_maincpu, FUNC(tmpz84c015_device::rxcb_w)); // CLK_COM1
	m_maincpu->zc_callback<0>().append(m_maincpu, FUNC(tmpz84c015_device::txcb_w));
	m_maincpu->zc_callback<2>().set(m_maincpu, FUNC(tmpz84c015_device::trg3));

	DS12885(config, m_rtc, XTAL(32'768)); // should be DS12887A
	ATA_INTERFACE(config, m_ata[0]).options(sprinter_ata_devices, "hdd", "hdd", false);
	ATA_INTERFACE(config, m_ata[1]).options(sprinter_ata_devices, "hdd", "hdd", false);

	BETA_DISK(config, m_beta, 0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ay8910_device &ay8910(AY8910(config.replace(), "ay8912", X_SP / 24));
	ay8910.add_route(0, "lspeaker", 0.50);
	ay8910.add_route(1, "lspeaker", 0.25);
	ay8910.add_route(1, "rspeaker", 0.25);
	ay8910.add_route(2, "rspeaker", 0.50);

	CENTRONICS(config, m_centronics, centronics_devices, "covox_stereo");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, m_cent_data_out));
	m_centronics->set_output_latch(cent_data_out);

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_sprinter);
}


ROM_START( sprinter )
	ROM_REGION(0x040000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v3.04.253")

	ROM_SYSTEM_BIOS(0, "v2.13.251", "BIOS v2.13, SETUP v251") // 11.10.2002
	ROMX_LOAD( "sp2k-2.13.251.rom", 0x000000, 0x40000, CRC(6495575f) SHA1(a9ca06b27e7c5b2b5b9ff8fc2d19ee24ed64c258), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v2.17.252", "BIOS v2.17, SETUP v252") // 03.03.2002
	ROMX_LOAD( "sp2k-2.17.252.rom", 0x000000, 0x40000, CRC(3c7f1025) SHA1(d5c3d10b3b67f9ef87d3ce8a52ae3c33b95b9171), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v3.00.253", "BIOS v3.00, SETUP v253") // 04.10.2002
	ROMX_LOAD( "sp2k-3.00.253.rom", 0x000000, 0x40000, CRC(193de3da) SHA1(428dcb1253a88e7b5aedcd68b5bf6d2487592e10), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v3.03.253", "BIOS v3.03, SETUP v253") // 02.05.2003
	ROMX_LOAD( "sp2k-3.03.253.rom", 0x000000, 0x40000, CRC(fe26f578) SHA1(ef6d0fe4ec1bae7bda572a4fb3b9497a8910b885), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "v3.04.253", "BIOS v3.04, SETUP v253") // 06.16.2003
	ROMX_LOAD( "sp2k-3.04.253.rom", 0x000000, 0x40000, CRC(1729cb5c) SHA1(fb4c9f80651aa87526f141839fb4d6cb86b654c7), ROM_BIOS(4))
ROM_END

} // Anonymous namespace


/*    YEAR  NAME        PARENT   COMPAT MACHINE   INPUT      CLASS           INIT        COMPANY                 FULLNAME           FLAGS */
// 1996 - Sp97 Prototype
COMP( 2000, sprinter,   spec128, 0,     sprinter, sprinter,  sprinter_state, empty_init, "Peters Plus, Ivan Mak", "Sprinter Sp2000", 0)
