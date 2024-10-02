// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*******************************************************************************************

Sprinter Sp2000 (Peters Plus Ltd)

Hardware:
- CPU                   Z84C15 (21MHz/3.5MHz)
- RAM                   4Mb (64Mb)
- Fast RAM              64Kb
- ROM                   256Kb
- Video RAM             256Kb (512Kb)
- FDD controller        WD1793
- Support FDD:          3,5" disk (1.44Mb/720Kb) / 5,25" disk (720Kb)
- CMOS                  DALLAS
- HDD controller        IDE/AT
- Keyboard controler    101key/AT
- Mouse controller      MS-Mouse
- Slots                 ISA-8
- Audio out             AY-3-8910 (in PLD), Stereo 8 bit (16 bit)
- Video out             TV, CGA analog monitor, RGB
- Graphic mode          320x256x256, 640x256x16, Spectrum standard screen
- Text mode             80x32x16

Refs:
    https://web.archive.org/web/20030208004427/http://www.petersplus.com/sprinter/

TODO:
- ISA memory slots
- fully untied from Spectrum parent
- better rendering (currently not fully discovered) in Game Configuration
- ? detect loading Configuration by checksum, not by presents in fastram

*******************************************************************************************/

#include "emu.h"

#include "beta_m.h"
#include "spec128.h"

#include "bus/ata/atapicdr.h"
#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "bus/spectrum/zxbus.h"
#include "cpu/z80/z84c015.h"
#include "machine/ds128x.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

#include "speaker.h"
#include "tilemap.h"

#include "sprinter.lh"


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

class sprinter_state : public spectrum_128_state
{
public:
	sprinter_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_isa(*this, "isa%u", 0U)
		, m_rtc(*this, "rtc")
		, m_ata(*this, "ata%u", 1U)
		, m_beta(*this, BETA_DISK_TAG)
		, m_ay8910(*this, "ay8912")
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_kbd(*this, "kbd")
		, m_io_line(*this, "IO_LINE%u", 0U)
		, m_io_mouse(*this, "mouse_input%u", 1U)
		, m_io_turbo(*this, "TURBO")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vram(*this, "vram", 0x40000, ENDIANNESS_LITTLE)
		, m_fastram(*this, "fastram", 0x10000, ENDIANNESS_LITTLE)
		, m_bank0_fastram(*this, "bank0_fastram")
		, m_bank_view0(*this, "bank_view0")
		, m_bank_view3(*this, "bank_view3")
		, m_turbo_led(*this, "turbo_led")
	{ }

	void sprinter(machine_config &config);

	INPUT_CHANGED_MEMBER(turbo_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	void map_io(address_map &map) ATTR_COLD;
	void map_mem(address_map &map) ATTR_COLD;
	void map_fetch(address_map &map) ATTR_COLD;
	u8 m1_r(offs_t offset);

	void init_taps();

	void update_memory();
	void update_cpu();
	void update_video(bool is312);

	virtual TIMER_CALLBACK_MEMBER(irq_on) override;
	virtual TIMER_CALLBACK_MEMBER(irq_off) override;
	TIMER_CALLBACK_MEMBER(cbl_tick);
	TIMER_CALLBACK_MEMBER(acc_tick);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_update_graph(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_update_game(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tile(u8* mode, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_symbol(u8* mode, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flash);
	u8* as_mode(u8 a, u8 b);
	std::pair<u8, u8> lookback_scroll(u8 a, u8 b);

	required_device<z84c015_device> m_maincpu;
	required_device_array<isa8_device, 2> m_isa;

private:
	enum accel_state : u8
	{
		MODE_AND   = 0xa6, // and (hl)
		MODE_XOR   = 0xae, // xor (hl)
		MODE_OR    = 0xb6, //  or (hl)
		MODE_NOP   = 0xbe  //  cp (hl)
	};
	enum access_state : u8
	{
		ACCEL_OFF = 0,
		ACCEL_GO,
		ACCEL_ON
	};

	static constexpr XTAL X_SP                 = 42_MHz_XTAL; // TODO X1 after spectrumless

	static constexpr u16  SPRINT_WIDTH         = 896;
	static constexpr u16  SPRINT_BORDER_RIGHT  = 48;
	static constexpr u16  SPRINT_SCREEN_XSIZE  = 640;
	static constexpr u16  SPRINT_BORDER_LEFT   = 48;
	static constexpr u16  SPRINT_XVIS          = SPRINT_BORDER_RIGHT + SPRINT_SCREEN_XSIZE + SPRINT_BORDER_LEFT;

	static constexpr u16  SPRINT_HEIGHT        = 320;
	static constexpr u16  SPRINT_BORDER_TOP    = 16;
	static constexpr u16  SPRINT_SCREEN_YSIZE  = 256;
	static constexpr u16  SPRINT_BORDER_BOTTOM = 16;
	static constexpr u16  SPRINT_YVIS          = SPRINT_BORDER_TOP + SPRINT_SCREEN_YSIZE + SPRINT_BORDER_BOTTOM;

	static constexpr u16 BANK_RAM_MASK         = 1 << 8;
	static constexpr u16 BANK_FASTRAM_MASK     = 1 << 9;
	static constexpr u16 BANK_ISA_MASK         = 1 << 10;
	static constexpr u16 BANK_WRDISBL_MASK     = 1 << 12;

	bool acc_ena()     const { return BIT(m_all_mode, 0); }
	bool cbl_mode()    const { return BIT(m_cbl_xx, 7); }
	bool cbl_stereo()  const { return BIT(m_cbl_xx, 6); }
	bool cbl_mode16()  const { return BIT(m_cbl_xx, 5); }
	bool cbl_int_ena() const { return BIT(m_cbl_xx, 4); }
	u8 ram1()          const { return m_ram_pages[0xe9 - 0xc0]; }
	u8 ram2()          const { return m_ram_pages[0xea - 0xc0]; }


	u8 dcp_r(offs_t offset);
	void dcp_w(offs_t offset, u8 data);

	u8 bootstrap_r(offs_t offset);
	void bootstrap_w(offs_t offset, u8 data);
	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	void vram_w(offs_t offset, u8 data);
	void update_int(bool recalculate);
	u8 isa_r(offs_t offset);
	void isa_w(offs_t offset, u8 data);
	void do_mem_wait(u8 cpu_taken);

	void check_accel(bool is_read, offs_t offset, u8 &data);
	void accel_control_r(u8 data);
	void do_accel_block(bool is_read);
	void accel_mem_r(offs_t offset);
	void accel_mem_w(offs_t offset, u8 data);
	u8 &accel_buffer(u8 idx);
	void update_accel_buffer(u8 idx, u8 data);

	u8 kbd_fe_r(offs_t offset);
	void on_kbd_data(int state);

	required_device<ds12885_device> m_rtc;
	required_device_array<ata_interface_device, 2> m_ata;
	required_device<beta_disk_device> m_beta;
	required_device<ay8910_device> m_ay8910;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_device<pc_kbdc_device> m_kbd;
	required_ioport_array<8> m_io_line;
	required_ioport_array<3> m_io_mouse;
	required_ioport m_io_turbo;
	required_device<device_palette_interface> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	tilemap_t *m_tilemap;
	memory_region *m_rom;
	memory_share_creator<u8> m_vram;
	memory_share_creator<u8> m_fastram;
	memory_bank_creator m_bank0_fastram;
	memory_view m_bank_view0;
	memory_view m_bank_view3;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	output_finder<> m_turbo_led;

	TILE_GET_INFO_MEMBER(get_tile_info);

	u8 *m_dcp_location;
	u8 m_ram_pages[0x40] = {}; // 0xc0 - 0xff
	u16 m_pages[4] = {}; // internal state for faster calculations

	bool    m_z80_m1;
	offs_t  m_z80_addr;
	u8      m_z80_data;
	bool    m_deferring;
	bool	m_skip_write;
	std::list<std::pair<u16, u16>> m_ints;

	u8 m_conf;
	bool m_conf_loading;
	bool m_starting;
	bool m_dos; // 0-on, 1-off
	bool m_cash_on;

	u8 m_cnf;
	bool m_rom_sys;
	bool m_ram_sys;
	bool m_sys_pg;
	bool m_turbo;
	bool m_turbo_hard;
	bool m_arom16;
	u8 m_rom_rg;
	u8 m_pn;
	u8 m_sc;
	u8 m_all_mode;
	u8 m_port_y;
	u8 m_rgmod;
	u8 m_pg3;
	u8 m_isa_addr_ext;
	std::pair<s8, s8> m_hold;
	u8 m_kbd_data_cnt;
	bool m_in_out_cmd;

	bool m_ata_selected; // 0-primary, 1-secondary
	u8 m_ata_data_latch;

	// Accelerator
	u8 m_prf_d;
	u8 m_rgacc;
	u8 m_acc_cnt;
	u8 m_accel_buffer[256] = {};
	bool m_alt_acc;
	u16 m_aagr;
	u8 m_xcnt;
	u8 m_xagr;
	u8 m_acc_dir;
	accel_state m_fn_acc;
	access_state m_access_state;

	// Covox Blaster
	u8 m_cbl_xx;
	u16 m_cbl_data[256] = {};
	u8 m_cbl_cnt;
	u8 m_cbl_wa;
	bool m_cbl_wae;
	emu_timer *m_cbl_timer = nullptr;
	emu_timer *m_acc_timer = nullptr;
	bool m_hold_irq;
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
		const u8 pg0 = 0x20 // 0xe0
			| ((sc0 || !m_ram_sys || cash_on || !nmi_ena) << 3)
			| (((m_arom16 && !(sc0 && m_ram_sys)) || (cash_on && nmi_ena)) << 2)
			| (((BIT(spr_, 1) && sc_lc) || !m_ram_sys || !nmi_ena) << 1)
			| ((BIT(spr_, 0) && sc_lc) || !m_ram_sys || !nmi_ena);

		m_pages[0] = BANK_RAM_MASK | m_ram_pages[pg0];
		m_bank_ram[0]->set_entry(m_pages[0] & 0xff);
		if (sc0 && m_ram_sys)
			m_bank_view0.disable();
		else
		{
			m_pages[0] |= BANK_WRDISBL_MASK;
			m_bank_view0.select(0);
		}
	}

	m_pages[1] = BANK_RAM_MASK | ram1();
	m_bank_ram[1]->set_entry(ram1());
	m_pages[2] = BANK_RAM_MASK | ram2();
	m_bank_ram[2]->set_entry(ram2());

	// 0xd0, 0xf0
	m_pg3 = (BIT(~m_pn, 7) << 5) | 0x10 | (((BIT(m_sc, 4) && BIT(~m_cnf, 7)) || (BIT(m_cnf, 7) && BIT(m_pn, 6))) << 3) | (m_pn & 0x07);
	m_pages[3] = m_starting ? 0x40 : m_ram_pages[m_pg3];
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

void sprinter_state::update_cpu()
{
	m_turbo_led = m_turbo && m_turbo_hard;
	m_maincpu->set_clock_scale((m_turbo && m_turbo_hard) ? 6 : 1); // 1 - 21MHz, 0 - 3.5MHz
}

void sprinter_state::update_video(bool is312)
{
	const u16 vtotal = SPRINT_HEIGHT - (8 * is312);
	m_screen->configure(SPRINT_WIDTH, vtotal, m_screen->visible_area(), HZ_TO_ATTOSECONDS(X_SP / 3) * SPRINT_WIDTH * vtotal);
	update_int(true);
}

u32 sprinter_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_conf)
		screen_update_game(screen, bitmap, cliprect);
	else
		screen_update_graph(screen, bitmap, cliprect);

	return 0;
}

void sprinter_state::screen_update_graph(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool flash = BIT(screen.frame_number(), 4);
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom();)
	{
		const u16 scr_height = screen.height();
		const u16 b8 = (scr_height + vpos - SPRINT_BORDER_TOP - m_hold.second) % scr_height;
		for (u16 hpos = cliprect.left(); hpos <= cliprect.right();)
		{
			const u16 a16 = (SPRINT_WIDTH + hpos - SPRINT_BORDER_LEFT - m_hold.first) % SPRINT_WIDTH;
			u8* mode = as_mode(a16 >> 4, b8 >> 3);
			const rectangle tile {hpos, std::min(cliprect.right(), hpos + 15 - (a16 & 15)), vpos, std::min(cliprect.bottom(), vpos + 7 - (b8 & 7))};
			if(BIT(mode[0], 4))
				draw_symbol(mode, bitmap, tile, flash);
			else
				draw_tile(mode, bitmap, tile);

			hpos += tile.width();
		}
		vpos += 8 - (b8 & 7);
	}
}

void sprinter_state::draw_tile(u8* mode, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 *pix = &(bitmap.pix(cliprect.top(), cliprect.left()));
	const u16 pal = BIT(mode[0], 6, 2) << 8;
	u16 x = (BIT(mode[0], 0, 4) << 6) | (BIT(mode[1], 0, 3) << 3);
	u8 y = BIT(mode[1], 3, 5) << 3;
	const bool lowres = BIT(mode[2], 2);
	if (lowres)
	{
		x += 4 * BIT(mode[2], 0);
		y += 4 * BIT(mode[2], 1);
	}
	for (auto dy = cliprect.top(); dy <= cliprect.bottom(); dy++)
	{
		for (auto dx = cliprect.left(); dx <= cliprect.right(); dx++)
		{
			const u8 color = m_vram[(y + (((dy - m_hold.second) & 7) >> lowres)) * 1024 + x + (((dx - m_hold.first) & 15) >> (1 + lowres))];
			*pix++ = pal + (BIT(mode[0], 5) ? color : (((dx - m_hold.first) & 1) ? (color & 0x0f) : (color >> 4)));
		}
		pix += SPRINT_WIDTH - cliprect.width();
	}
}

void sprinter_state::draw_symbol(u8* mode, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flash)
{
	rectangle partrect = cliprect;
	do
	{
		const bool is_partial = cliprect.right() != partrect.right();
		if (is_partial)
		{
			partrect.setx(partrect.right() + 1, cliprect.right());
			mode += 1024;
		}
		else if (!BIT(mode[0], 5))
		{
			if (BIT(cliprect.left() - m_hold.first, 3))
				mode += 1024;
			else
				partrect.setx(partrect.left(), std::min(cliprect.right(), cliprect.left() + 7 - ((cliprect.left() - m_hold.first) & 7)));
		}

		u16 *pix = &(bitmap.pix(partrect.top(), partrect.left()));
		u8 attr = 0;
		u16 pal = 0x400;
		if (~mode[0] & 0xfc) // !blank
		{
			attr = m_vram[(mode[2] << 10) | (BIT(mode[0], 0, 4) << 6) | (BIT(m_pn, 3) << 5) | 0b11000 | BIT(mode[0], 6, 2)];
			if ((BIT(mode[0], 5, 3) == 7) && ((mode[0] & 0x0c) != 0x0c)) // border
				pal = 0x400 | ((m_port_fe_data & 0x07) << 3) | (m_port_fe_data & 0x07);
		}

		for (auto dy = partrect.top(); dy <= partrect.bottom(); dy++)
		{
			const u8 symb = (~mode[0] & 0xfc) ? m_vram[(mode[1] << 10) | (BIT(mode[0], 0, 4) << 6) | (BIT(m_pn, 3) << 5) | (BIT(mode[0], 6, 2) << 3) | ((dy - m_hold.second) & 7)] : 0;
			for (auto dx = partrect.left(); dx <= partrect.right(); dx++)
			{
				if (BIT(mode[0], 5, 3) != 7)
				{
					const u8 bit = 1 << (7 - (((dx - m_hold.first) >> BIT(mode[0], 5)) & 7));
					pal = attr + 0x400 + 0x100 * bool(symb & bit) + 0x200 * flash;
				}
				*pix++ = pal;
			}
			pix += SPRINT_WIDTH - partrect.width();
		}
	} while (cliprect.right() != partrect.right());
}

 // Game Config - used in Thunder in the Deep
void sprinter_state::screen_update_game(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		const u16 scr_height = screen.height();
		const u8 b = ((scr_height + vpos - SPRINT_BORDER_TOP - m_hold.second) % scr_height) >> 3;
		const u8 a = ((SPRINT_WIDTH + cliprect.left() - SPRINT_BORDER_LEFT - m_hold.first) % SPRINT_WIDTH) >> 4;
		std::pair<u8, u8> scroll = lookback_scroll(a, b);

		u8* mode = nullptr;
		u16 pal, x;
		u8 y;
		u16 *pix = &(bitmap.pix(vpos, cliprect.left()));

		for (u16 hpos = cliprect.left(); hpos <= cliprect.right(); hpos++)
		{
			u16 a16 = (SPRINT_WIDTH + hpos + (scroll.first << 1) - SPRINT_BORDER_LEFT - m_hold.first) % SPRINT_WIDTH;
			if ((mode != nullptr) && BIT(mode[0], 2) && (((a16 - (scroll.first << 1)) & 15) == 0))
			{
				scroll = {mode[3] & 0x0f, mode[3] >> 4};
				a16 = (SPRINT_WIDTH + hpos + (scroll.first << 1) - SPRINT_BORDER_LEFT - m_hold.first) % SPRINT_WIDTH;
			}
			const u16 b8 = (scr_height + vpos + scroll.second - SPRINT_BORDER_TOP - m_hold.second) % scr_height;

			if (mode == nullptr)
			{
				mode = as_mode(a16 >> 4, b8 >> 3);
				pal = BIT(mode[0], 6, 2) << 8;
				x = ((BIT(mode[0], 0, 2) << 8) | mode[1]);
				y = mode[2];
			}

			if ((BIT(mode[0], 5, 3) == 7))
				*pix++ = 0x400 | (((mode[0] & 0x0c) == 0x0c) ? 0 : ((m_port_fe_data & 0x07) << 3) | (m_port_fe_data & 0x07));
			else
				*pix++ = pal + m_vram[(y + (b8 & 7)) * 1024 + x + ((a16 & 15) >> 1)];

			if ((a16 & 15) == 15)
			{
				if (BIT(mode[0], 2))
					scroll = {mode[3] & 0x0f, mode[3] >> 4};
				mode = nullptr;
			}
		}
	}
}

u8* sprinter_state::as_mode(u8 a, u8 b)
{
	const u32 line1 = (1 + a * 2 + 0x80 * (m_rgmod & 1)) * 1024;
	const u16 la = 0x300 + b * 4;

	return m_vram + line1 + la;
}

std::pair<u8, u8> sprinter_state::lookback_scroll(u8 a, u8 b)
{
	for (auto v = b; b; b--, a = 55)
	{
		for (auto h = a; a; a--)
		{
			const u8* mode = as_mode(h, v);
			if (BIT(mode[0], 2) && (h != a) && (v != b))
				return { mode[3] & 0x0f, mode[3] >> 4 };
		}
	}

	return { 0, 0 };
}

u8 sprinter_state::dcp_r(offs_t offset)
{
	if (m_starting) m_starting = 0;

	if (!machine().side_effects_disabled())
	{
		if (((offset & 0x7f) == 0x7b))
		{
			m_cash_on = BIT(offset, 7);
			update_memory();
		}
		do_mem_wait(4);
	}

	const u16 dcp_offset = (BIT(m_cnf, 3, 2) << 12) | (0 << 11) | (m_dos << 10) | (1 << 9) | (BIT(offset, 14, 2) << 7) | (BIT(offset, 13) << 4) | (BIT(offset, 7) << 3) | (offset & 0x67);
	const u8 dcpp = m_dcp_location[dcp_offset];
	u8 data = 0xff;
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
		data = m_beta->state_r() & m_io_joy1->read();
		break;

	case 0x1c:
		data = m_rtc->data_r();
		break;

	case 0x20:
		if (!machine().side_effects_disabled() && BIT(~offset, 8))
		{
			const u16 tmp = m_ata[m_ata_selected]->cs0_r(0);
			m_ata_data_latch = tmp >> 8;
			data = tmp;
		}
		else
			data = m_ata_data_latch;
		break;
	case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		if (BIT(~offset, 8))
			data = m_ata[m_ata_selected]->cs0_r(dcpp & 0x07);
		break;
	case 0x28:
		if (BIT(~offset, 8))
			data = m_ata[m_ata_selected]->cs1_r(6);
		break;
	case 0x29:
		if (BIT(~offset, 8))
			data = m_ata[m_ata_selected]->cs1_r(7);
		break;

	case 0x40:
		data = kbd_fe_r(offset);
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

	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		data = m_ram_pages[dcpp - 0xc0];
		break;
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		data = m_ram_pages[m_pg3];
		break;

	default:
		LOGIO("IOr: %02X / %X > %x\n", dcpp, offset, data);
		break;
	}

	return data;
}

void sprinter_state::dcp_w(offs_t offset, u8 data)
{
	if (m_starting)
		return;

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
	do_mem_wait(4);

	const u16 dcp_offset = (BIT(m_cnf, 3, 2) << 12) | (0 << 11) | (m_dos << 10) | (0 << 9) | (BIT(offset, 14, 2) << 7) | (BIT(offset, 13) << 4) | (BIT(offset, 7) << 3) | (offset & 0x67);
	const u8 dcpp = m_dcp_location[dcp_offset];
	if ((dcpp >= 0xc0) && (dcpp < 0xf0))
		m_ram_pages[dcpp - 0xc0] = data;
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
	case 0x16:
	case 0x17:
		m_beta->turbo_w(dcpp & 1);
		if (data & 2)
			m_beta->disable();
		else
			m_beta->enable();
		break;

	case 0x1b:
		if (data & 0x80)
			; // RESET
		if (data & 0x40)
			; // AEN
		m_isa_addr_ext = data & 0x3f;
		break;

	case 0x1d:
		m_rtc->address_w(data);
		break;
	case 0x1e:
		m_rtc->data_w(data);
		break;

	case 0x20:
		if (BIT(offset, 8))
			m_ata[m_ata_selected]->cs0_w(0, (data << 8) | m_ata_data_latch);
		else
			m_ata_data_latch = data;
		break;
	case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		if (BIT(offset, 8))
			m_ata[m_ata_selected]->cs0_w(dcpp & 0x07, data);
		break;
	case 0x28:
		if (BIT(offset, 8))
			m_ata[m_ata_selected]->cs1_w(6, data);
		break;
	case 0x2a: // HDD1 - secondary
		m_ata_selected = 1;
		break;
	case 0x2b: // HDD2 - primary
		m_ata_selected = 0;
		break;
	case 0x2c: // 320
	case 0x2d: // 312
		update_video(dcpp & 1);
		break;
	case 0x2e:
		if (m_conf)
			machine().schedule_hard_reset();
		else
		{
			m_conf_loading = 1;
			machine().schedule_soft_reset();
		}
		break;

	case 0x88:
		if (cbl_mode())
			m_cbl_data[m_cbl_wa++] = data << 8;
		else
		{
			m_ldac->write(data << 8);
			m_rdac->write(data << 8);
		}
		break;

	case 0x89:
	{
		m_cbl_xx = data;
		m_cbl_cnt = 0;
		m_cbl_wa = 0;
		m_cbl_wae = cbl_mode16();
		const u8 divs[16] = {13, 9, 0, 0, 0, 0, 0, 0, 27, 19, 13, 9, 6, 4, 3, 1};
		const attotime rate = (cbl_mode() && divs[m_cbl_xx & 15]) ? attotime::from_ticks(divs[m_cbl_xx & 15] + 1, X_SP / 192) : attotime::never;
		m_cbl_timer->adjust(rate, 0, rate);
		break;
	}

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
	case 0xc8:
		m_sc = data;
		if (BIT(m_cnf, 6)) m_sc = 0;      // CNF_SC_RESET
		update_memory();
		break;
	case 0xc1: // 7FFD
	case 0xc9:
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
		m_hold = {(7 - (data & 0x0f)) * 2, 7 - (data >> 4)};
		break;
	case 0xc4:
	case 0xcc:
		m_port_y = data;
		break;
	case 0xc5:
	case 0xcd:
		m_screen->update_now();
		m_rgmod = data;
		update_int(true);
		break;

	case 0xc6: // CNF/SYS
	case 0xce:
		m_ram_sys = BIT(~offset, 6);
		if (BIT(data, 1))
		{
			m_turbo = BIT(data, 0);
			update_cpu();
		}
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

	case 0xc7:
	case 0xcf:
		m_alt_acc = 1;
		m_aagr = (offset & 0x300) | data;
		m_xcnt = BIT(offset, 10, 6);
		m_xagr = 0;
		break;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		update_memory();
		break;
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		m_ram_pages[m_pg3] = data;
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

void sprinter_state::accel_control_r(u8 data)
{
	const bool is_prefix = (data == 0xcb) || (data == 0xdd) || (data == 0xed) || (data == 0xfd);
	if (acc_ena() && !is_prefix && !m_prf_d) // neither prefix nor prefixed
	{
		if ((((data & 0x1b) == 0x00) || ((data & 0x1b) == 0x09) || ((data & 0x1b) == 0x12) || ((data & 0x1b) == 0x1b))
			&& (((data & 0xe4) == 0x40) || ((data & 0xe4) == 0x64)))
		{
			switch(data & 7)
			{
				case 0: m_acc_dir = 0b00000000; break; // LD B,B
				case 1: m_acc_dir = 0b00100101; break; // LD C,C % % fill by constant
				case 2: m_acc_dir = 0b00001001; break; // LD D,D % % load count accelerator
				case 3: m_acc_dir = 0b00010101; break; // LD E,E % % fill by constant VERTICAL
				case 4: m_acc_dir = 0b01000001; break; // LD H,H % % duble byte fn
				case 5: m_acc_dir = 0b00100111; break; // LD L,L % % copy line
				case 6: m_acc_dir = 0b00000000; break; // HALT
				case 7: m_acc_dir = 0b00010111; break; // LD A,A % % copy line VERTICAL
			}
			m_fn_acc = MODE_NOP;
		}
		else
		{
			const accel_state state_candidate = static_cast<accel_state>(data);
			switch(state_candidate)
			{
				case MODE_AND:
				case MODE_XOR:
				case MODE_OR:
					m_fn_acc = state_candidate;
					break;
				default:
					break;
			}
		}
	}
	m_prf_d = is_prefix;
}

TIMER_CALLBACK_MEMBER(sprinter_state::acc_tick)
{
	const bool is_block_op = BIT(m_acc_dir, 2);
	if (m_access_state == ACCEL_GO)
	{
		m_acc_cnt = m_rgacc;
		m_access_state = ACCEL_ON;
	}

	const bool is_read = param & 1;
	if (is_block_op)
	{
		do_accel_block(is_read);
	}
	if (BIT(m_acc_dir, 3))
	{
		m_rgacc = m_z80_data;
		LOGACCEL("Accel buffer: %d\n", m_rgacc ? m_rgacc : 256);
	}
	else if (BIT(m_acc_dir, 6) && !is_read)
	{
		accel_mem_w(m_z80_addr ^ 1, m_z80_data);
	}

	if (m_acc_cnt == 1 || !is_block_op)
	{
		m_acc_timer->reset();
		m_access_state = ACCEL_OFF;
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
	else
	{
		m_acc_timer->adjust(attotime::from_ticks(6, X_SP), is_read);
		m_acc_cnt--;
	}
}

void sprinter_state::check_accel(bool is_read, offs_t offset, u8 &data)
{
	const bool is_ram = m_pages[BIT(offset, 14, 2)] & BANK_RAM_MASK;
	if (is_read && m_in_out_cmd && !m_z80_m1)
	{
		if (data == 0x1f && is_ram)
			data = 0x0f;
		m_in_out_cmd = false;
	}

	const bool accel_go_case = m_access_state == ACCEL_OFF && !m_z80_m1 && m_acc_dir && acc_ena();
	if (is_ram && (!accel_go_case || m_deferring))
	{
		do_mem_wait(3);
	}
	if (accel_go_case)
	{
		if (!m_deferring)
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_access_state = ACCEL_GO;
			m_acc_timer->adjust(attotime::from_ticks(6, X_SP), is_read);
			m_z80_addr = offset;
			m_z80_data = data;

			if (BIT(m_acc_dir, 2))
			{
				m_skip_write = !is_read;
				m_maincpu->defer_access();
				m_deferring = true;
			}
		}
		else
		{
			if (is_read)
			{
				data = m_z80_data;
			}
			else if (is_ram)
			{
				m_skip_write = true;
			}
			m_deferring = false;
		}
	}
}

void sprinter_state::do_accel_block(bool is_read)
{
	const bool ram_wr = BIT(m_acc_dir, 1);
	if (is_read)
	{
		accel_mem_r(m_z80_addr);
		if (ram_wr)
		{
			update_accel_buffer(m_acc_cnt, m_z80_data);
		}
	}
	else
	{
		if (ram_wr)
		{
			m_z80_data = accel_buffer(m_acc_cnt);

			const u8 pg = m_pages[BIT(m_z80_addr, 14, 2)];
			if (pg == 0xfd)
			{
				if (!cbl_mode16())
				{
					m_cbl_data[m_cbl_wa++] = (m_z80_data << 8);
				}
				else
				{
					if (m_cbl_wae)
						m_cbl_data[m_cbl_wa] = m_z80_data;
					else
					{
						m_cbl_data[m_cbl_wa] |= ((m_z80_data ^ 0x80) << 8);
						m_cbl_wa++;
					}
					m_cbl_wae = !m_cbl_wae;
				}
			}
		}
		accel_mem_w(m_z80_addr, m_z80_data);
	}

	if (BIT(m_acc_dir, 4)) // graph line
		m_port_y++;
	else
		m_z80_addr++;
}

void sprinter_state::accel_mem_r(offs_t offset)
{
	if (m_pages[BIT(offset, 14, 2)] & BANK_RAM_MASK)
	{
		m_z80_data = m_program.read_byte(offset);
	}
}

void sprinter_state::accel_mem_w(offs_t offset, u8 data)
{
	if (~m_pages[BIT(offset, 14, 2)] & (BANK_FASTRAM_MASK | BANK_ISA_MASK | BANK_WRDISBL_MASK))
	{
		m_program.write_byte(offset, data);
	}
}

u8 &sprinter_state::accel_buffer(u8 idx)
{
	if (m_alt_acc)
	{
		idx = m_xcnt;
		const u16 xcnt_agr = ((m_xcnt << 8) | m_xagr) + m_aagr;
		m_xcnt = xcnt_agr >> 8;
		m_xagr = xcnt_agr & 0xff;
	}

	return m_accel_buffer[idx];
}

void sprinter_state::update_accel_buffer(u8 idx, u8 data)
{
	switch (m_fn_acc)
	{
		case MODE_AND: accel_buffer(idx) &= data; break;
		case MODE_OR:  accel_buffer(idx) |= data; break;
		case MODE_XOR: accel_buffer(idx) ^= data; break;
		case MODE_NOP: accel_buffer(idx) = data; break;
		default: assert(false); break;
	}
}

u8 sprinter_state::bootstrap_r(offs_t offset)
{
	const u16 addr = offset & 0xffff;
	return m_conf_loading
		? m_fastram[addr]
		: m_program.read_byte(0x10000 | addr);
}

void sprinter_state::bootstrap_w(offs_t offset, u8 data)
{
	if (m_conf_loading)
	{
		m_conf_loading = 0;
		m_conf = !(m_maincpu->csbr_r() & 0x0f); // cs0 disabled => loader reads config from fastram (which is Game Config)
		m_ram_pages[0x2e] = m_conf ? 0x41 : 0x00;
		machine().schedule_soft_reset();
	}
	else
		m_program.write_byte(0x10000 | u16(offset), data);
}

u8 sprinter_state::ram_r(offs_t offset)
{
	const u8 bank = BIT(offset, 14, 2);
	return ((m_pages[bank] & 0xf0) == 0x50)
		? m_ram->pointer()[(0x50 << 14) + m_port_y * 1024 + (offset & 0x3ff)]
		: reinterpret_cast<u8 *>(m_bank_ram[bank]->base())[offset & 0x3fff];
}

void sprinter_state::ram_w(offs_t offset, u8 data)
{
	if (m_skip_write)
	{
		m_skip_write = false;
		return;
	}

	const u8 bank = BIT(offset, 14, 2);
	const u8 page = m_pages[bank] & 0xff;
	if ((bank == 3) && (m_sc == 0x10) && (m_pages[3] == (BANK_RAM_MASK | 0xa0)))
		machine().schedule_soft_reset();

	if ((page & 0xf0) == 0x50)
	{
		const u32 vaddr = m_port_y * 1024 + (offset & 0x3ff);
		if (BIT(~page, 2))
			m_ram->pointer()[(0x50 << 14) + vaddr] = data;
		if (!(BIT(page, 3) && (data == 0xff)))
			vram_w(vaddr, data);
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
				vram_w(vxa, data);
			}
		}
		reinterpret_cast<u8 *>(m_bank_ram[bank]->base())[offset & 0x3fff] = data;
	}
}

void sprinter_state::vram_w(offs_t offset, u8 data)
{
	m_screen->update_now();
	const u16 laddr = offset & 0x3ff;
	const bool is_int_updated = (laddr >= 0x300) && (laddr < 0x3a0) && ((offset & 0x403) == 0x400) && (((m_vram[offset] & 0xfc) == 0xfc) || ((data & 0xfc) == 0xfc)) && (m_vram[offset] ^ data);
	m_vram[offset] = data;

	const u8 col = (offset >> 3) & 0x7f;
	const u8 row = offset >> 13;
	m_tilemap->mark_tile_dirty(row * 128 + col);
	m_gfxdecode->gfx(1)->mark_dirty(row * 128 * 8 + col);

	if (is_int_updated)
		update_int(true);
	else if (laddr >= 0x3e0)
	{
		const u16 pen = BIT(offset, 2, 3) * 256 + (offset >> 10);
		const u32 p_red = offset & ~0x3;
		m_palette->set_pen_color(pen, rgb_t(m_vram[p_red], m_vram[p_red + 1], m_vram[p_red + 2]));
	}
}

u8 sprinter_state::isa_r(offs_t offset)
{
	const u8 ctrl = m_ram_pages[m_pg3];
	if ((ctrl & 0xf9) == 0xd0)
		return BIT(ctrl, 2) // D:2 0-mem, 1-io
			? m_isa[BIT(ctrl, 1)]->io_r((m_isa_addr_ext << 14) | offset)
			: m_isa[BIT(ctrl, 1)]->mem_r((m_isa_addr_ext << 14) | offset);

	return 0xff;
}

void sprinter_state::isa_w(offs_t offset, u8 data)
{
	const u8 ctrl = m_ram_pages[m_pg3];
	if ((ctrl & 0xf9) == 0xd0)
	{
		if (BIT(ctrl, 2))
			m_isa[BIT(ctrl, 1)]->io_w((m_isa_addr_ext << 14) | offset, data);
		else
			m_isa[BIT(ctrl, 1)]->mem_w((m_isa_addr_ext << 14) | offset, data);
	}
}

void sprinter_state::update_int(bool recalculate)
{
	if (recalculate)
		m_ints.clear();

	const u8 height = m_screen->height() / 8;
	if (m_ints.empty())
	{
		for (auto scr_b = 0; scr_b < height; scr_b++)
		{
			bool pre_int = false;
			const u8 b = (scr_b + height - 2) % height; // 2-top border
			for (auto scr_a = 0; scr_a <= 55; scr_a++)
			{
				const u8 a = (scr_a + 56 - 6) % 56; // 3-left border, 3-teared blank?
				const u8* line1 = as_mode(a, b);
				if ((*line1 & 0xfd) == 0xfd)
					pre_int = true;
				else
				{
					if (pre_int)
						m_ints.push_back({scr_b * 8 + 7, scr_a * 16}); // +7=bottom of the tile
					pre_int = false;
				}
			}
		}
	}

	if (!m_ints.empty())
	{
		attotime min = attotime::never;
		for (std::list<std::pair<u16, u16>>::iterator it=m_ints.begin(); it != m_ints.end(); ++it)
			min = std::min(min, m_screen->time_until_pos((*it).first, (*it).second));
		m_irq_on_timer->adjust(min);
	}
}

u8 sprinter_state::m1_r(offs_t offset)
{
	m_z80_m1 = 1;
	u8 data = m_program.read_byte(offset);
	m_z80_m1 = 0;

	if (!machine().side_effects_disabled())
	{
		m_in_out_cmd = !m_prf_d && (data & 0xf7) == 0xd3; // d3/db - only non-prefixed
		accel_control_r(data);
	}

	return data;
}

void sprinter_state::map_fetch(address_map &map)
{
	// Overlap with previous because we want real addresses on the 3e00-3fff range
	map(0x0000, 0x3fff).mirror(0x10000).lr8(NAME([this](offs_t offset)
	{
		return m1_r(offset + 0x10000);
	}));
	map(0x3d00, 0x3dff).mirror(0x10000).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled() && m_dos && BIT(m_pn, 4))
		{
			m_dos = 0;
			update_memory();
		}
		return m1_r(offset + 0x13d00);
	}));
	map(0x4000, 0xffff).mirror(0x10000).lr8(NAME([this](offs_t offset)
	{
		if (!machine().side_effects_disabled() && !m_dos)
		{
			m_dos = 1;
			update_memory();
		}
		return m1_r(offset + 0x14000);
	}));
}

void sprinter_state::map_mem(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(sprinter_state::bootstrap_r), FUNC(sprinter_state::bootstrap_w));  // bootstrap
	map(0x10000, 0x1ffff).rw(FUNC(sprinter_state::ram_r), FUNC(sprinter_state::ram_w));

	map(0x10000, 0x13fff).view(m_bank_view0);
	m_bank_view0[0](0x10000, 0x13fff).nopw(); // RAM RO
	m_bank_view0[1](0x10000, 0x13fff).nopw().bankr(m_bank_rom[0]);
	m_bank_view0[2](0x10000, 0x13fff).bankrw(m_bank0_fastram);

	map(0x1c000, 0x1ffff).view(m_bank_view3);
	m_bank_view3[0](0x1c000, 0x1ffff).rw(FUNC(sprinter_state::isa_r), FUNC(sprinter_state::isa_w)); // ISA
}

void sprinter_state::map_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(sprinter_state::dcp_r), FUNC(sprinter_state::dcp_w));
}

void sprinter_state::init_taps()
{
	address_space &prg = m_maincpu->space(AS_PROGRAM);
	prg.install_read_tap(0x10000, 0x1ffff, "accel_read", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			check_accel(true, offset, data);
		}
	});
	prg.install_write_tap(0x10000, 0x1ffff, "accel_write", [this](offs_t offset, u8 &data, u8 mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			check_accel(false, offset, data);
		}
	});
}

void sprinter_state::machine_start()
{
	m_isa[0]->space(isa8_device::AS_ISA_IO).unmap_value_high();
	m_isa[1]->space(isa8_device::AS_ISA_IO).unmap_value_high();

	spectrum_128_state::machine_start();

	m_turbo_led.resolve();

	save_item(NAME(m_ram_pages));
	save_item(NAME(m_pages));
	save_item(NAME(m_z80_m1));
	save_item(NAME(m_z80_addr));
	save_item(NAME(m_z80_data));
	save_item(NAME(m_deferring));
	save_item(NAME(m_skip_write));
	save_item(NAME(m_conf));
	save_item(NAME(m_conf_loading));
	save_item(NAME(m_starting));
	save_item(NAME(m_dos));
	save_item(NAME(m_cash_on));
	save_item(NAME(m_cnf));
	save_item(NAME(m_rom_sys));
	save_item(NAME(m_ram_sys));
	save_item(NAME(m_sys_pg));
	save_item(NAME(m_turbo));
	save_item(NAME(m_turbo_hard));
	save_item(NAME(m_arom16));
	save_item(NAME(m_rom_rg));
	save_item(NAME(m_pn));
	save_item(NAME(m_sc));
	save_item(NAME(m_all_mode));
	save_item(NAME(m_port_y));
	save_item(NAME(m_rgmod));
	save_item(NAME(m_pg3));
	save_item(NAME(m_isa_addr_ext));
	//save_item(NAME(m_hold));
	save_item(NAME(m_kbd_data_cnt));
	save_item(NAME(m_in_out_cmd));
	save_item(NAME(m_ata_selected));
	save_item(NAME(m_ata_data_latch));
	save_item(NAME(m_prf_d));
	save_item(NAME(m_rgacc));
	save_item(NAME(m_acc_cnt));
	save_item(NAME(m_accel_buffer));
	save_item(NAME(m_alt_acc));
	save_item(NAME(m_aagr));
	save_item(NAME(m_xcnt));
	save_item(NAME(m_xagr));
	save_item(NAME(m_acc_dir));
	//save_item(NAME(m_fn_acc));
	//save_item(NAME(m_access_state));
	save_item(NAME(m_cbl_xx));
	save_item(NAME(m_cbl_data));
	save_item(NAME(m_cbl_cnt));
	save_item(NAME(m_cbl_wa));
	save_item(NAME(m_cbl_wae));
	save_item(NAME(m_hold_irq));

	m_beta->enable();

	// reconfigure ROMs
	m_rom = memregion("maincpu");
	m_bank_rom[0]->configure_entries(0, m_rom->bytes() / 0x4000, m_rom->base(), 0x4000);
	m_bank0_fastram->configure_entries(0, m_fastram.bytes() / 0x4000, m_fastram.target(), 0x4000);
	for (auto i = 0; i < 4; i++)
		m_bank_ram[i]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);

	m_dcp_location = m_ram->pointer() + (0x40 << 14);
	m_maincpu->space(AS_PROGRAM).specific(m_program);

	const u8 port_default[0x40] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Cx - SYS PORTS COPIES
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // Dx - RAM PAGES
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x00, 0x05, 0x02, 0x41, 0xff, 0x00, 0x00, 0x41, // Ex - ROM PAGES
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // Fx - RAM PAGES
	};
	std::copy(std::begin(port_default), std::end(port_default), std::begin(m_ram_pages));

	m_all_mode = 0x00;   // c3
	m_port_y   = 0x00;   // c4
	m_rgmod    = 0x00;   // c5
	m_hold     = {0, 0}; // cb
	m_conf_loading = 1;
	m_conf = 0;
}

void sprinter_state::machine_reset()
{
	m_acc_timer->reset();
	m_cbl_timer->reset();

	spectrum_128_state::machine_reset();

	m_deferring = false;
	m_skip_write = false;
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
	m_isa_addr_ext = 0;

	m_access_state = ACCEL_OFF;
	m_prf_d = false;
	m_acc_dir = 0;
	m_alt_acc = 0;

	m_cbl_xx = 0;
	m_cbl_wa = 0;
	m_hold_irq = 0;

	m_ata_selected = 0;

	m_kbd_data_cnt = 0;
	m_in_out_cmd = false;
	m_turbo_hard = 1;

	if (m_conf_loading)
	{
		m_bank_rom[0]->set_entry(0x0c);
		m_bank_view0.select(1);
		m_bank_view3.disable();
	}
	else
	{
		update_memory();
		update_video(0);
	}
}

void sprinter_state::device_post_load()
{
	spectrum_128_state::device_post_load();
	m_ints.clear();
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

static const gfx_layout sprinter_tiles =
{
	8, 8,
	128 * 32 * 8,
	8,
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 1024 * 8) },
	8 * 8
};

static GFXDECODE_START( gfx_sprinter )
	GFXDECODE_RAM( "vram", 0x2c0, sprinter_charlayout, 0x70f, 1 )
	GFXDECODE_RAM( "vram", 0,     sprinter_tiles,      0x100, 256 )
GFXDECODE_END

TILE_GET_INFO_MEMBER(sprinter_state::get_tile_info)
{
	const u8 col = tile_index % 128;
	const u8 row = tile_index / 128;
	tileinfo.set(1, row * 128 * 8 + col, 0, 0);
}

void sprinter_state::video_start()
{
	spectrum_state::video_start();

	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprinter_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);

	m_contention_pattern = {};
	init_taps();

	m_acc_timer = timer_alloc(FUNC(sprinter_state::acc_tick), this);
	m_cbl_timer = timer_alloc(FUNC(sprinter_state::cbl_tick), this);
}

static void sprinter_ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_FIXED_CDROM); // TODO must be ATAPI_CDROM
	device.option_add("dvdrom", ATAPI_FIXED_DVDROM);
}

u8 sprinter_state::kbd_fe_r(offs_t offset)
{
	u8 data = 0xff;

	u8 oi = offset >> 8;
	u8 shifts = 0xff;
	for (u8 i = 0; i < 8; i++, oi >>= 1)
	{
		const u8 line_data = m_io_line[i]->read();
		shifts &= line_data;
		if ((oi & 1) == 0)
			data &= line_data;
	}

	if (((offset & 0x0100) == 0) && BIT(~shifts, 6))
		data &= ~0x01; // CS

	if (((offset & 0x8000) == 0) && BIT(~shifts, 7))
		data &= ~0x02; // SS

	data |= 0xe0;
	data ^= 0x40;

	/* cassette input from wav */
	if (m_cassette->input() > 0.0038 )
		data &= ~0x40;

	if (cbl_mode())
	{
		data &= ~0xa0;
		data |= (m_screen->vpos() >= (SPRINT_BORDER_TOP + SPRINT_SCREEN_YSIZE)) << 5;
		data |= (cbl_int_ena() ? (m_cbl_cnt ^ m_cbl_wa) : m_cbl_cnt) & 0x80;
	}

	return data;
}

void sprinter_state::on_kbd_data(int state)
{
	if (state && ((m_all_mode & 0x09) == 0x09))
	{
		m_kbd_data_cnt++;
		m_kbd_data_cnt %= 11;
		if (!m_kbd_data_cnt)
			irq_on(0);
	}
}

void sprinter_state::do_mem_wait(u8 cpu_taken = 0)
{
	if (m_turbo && m_turbo_hard)
	{
		u8 over = m_maincpu->total_cycles() % 6;
		over = over ? (6 - over) : 0;
		m_maincpu->adjust_icount(-(over + (6 - cpu_taken)));
	}
}

TIMER_CALLBACK_MEMBER(sprinter_state::irq_on)
{
	if (!m_hold_irq)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_irq_off_timer->adjust(attotime::from_ticks(32, m_maincpu->unscaled_clock()));
	}
	update_int(false);
}

TIMER_CALLBACK_MEMBER(sprinter_state::irq_off)
{
	m_irq_off_timer->reset(); // in case it's called from INT Ack, not by timer itself
	m_hold_irq = 0;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(sprinter_state::cbl_tick)
{
	u16 left = m_cbl_data[m_cbl_cnt++];
	u16 right = cbl_stereo() ? m_cbl_data[m_cbl_cnt++] : left;
	if (cbl_mode16())
	{
		using std::swap;
		swap(left, right);
	}
	m_ldac->write(left);
	m_rdac->write(right);

	if (cbl_int_ena() && !(m_cbl_cnt & 0x7f))
	{
		m_hold_irq = 1;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_irq_off_timer->reset();
	}
}

INPUT_CHANGED_MEMBER(sprinter_state::turbo_changed)
{
	m_turbo_hard = !m_turbo_hard;
	update_cpu();
}

INPUT_PORTS_START( sprinter )
	/* PORT_NAME =  KEY Mode    CAPS Mode    SYMBOL Mode   EXT Mode   EXT+Shift Mode   BASIC Mode  */
	PORT_START("IO_LINE0") /* 0xFEFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)  PORT_CHAR(UCHAR_SHIFT_1) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z    Z    :      LN       BEEP   COPY") PORT_CODE(KEYCODE_Z)      PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(':')
																	 PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"x    X    £      EXP      INK    CLEAR") PORT_CODE(KEYCODE_X)   PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(U'£')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c    C    ?      LPRINT   PAPER  CONT") PORT_CODE(KEYCODE_C)      PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v    V    /      LLIST    FLASH  CLS") PORT_CODE(KEYCODE_V)       PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR('/')
																	 PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line0") PORT_CODE(KEYCODE_BACKSLASH) PORT_CODE(KEYCODE_SLASH)  PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE1") /* 0xFDFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a    A    STOP   READ      ~     NEW") PORT_CODE(KEYCODE_A)      PORT_CHAR('a') PORT_CHAR('A')// PORT_CHAR('~')
																	 PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s    S    NOT    RESTORE   |     SAVE") PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')// PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d    D    STEP   DATA      \\    DIM") PORT_CODE(KEYCODE_D)      PORT_CHAR('d') PORT_CHAR('D')// PORT_CHAR('\\')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f    F    TO     SGN       {     FOR") PORT_CODE(KEYCODE_F)      PORT_CHAR('f') PORT_CHAR('F')// PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g    G    THEN   ABS       }     GOTO") PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')// PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line1") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE2") /* 0xFBFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q    Q    <=     SIN      ASN      PLOT") PORT_CODE(KEYCODE_Q)   PORT_CHAR('q') PORT_CHAR('Q')
																	 PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w    W    <>     COS      ACS      DRAW") PORT_CODE(KEYCODE_W)   PORT_CHAR('w') PORT_CHAR('W')
																	 PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e    E    >=     TAN      ATN      REM") PORT_CODE(KEYCODE_E)    PORT_CHAR('e') PORT_CHAR('E')
																	 PORT_CODE(KEYCODE_END)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r    R    <      INT      VERIFY   RUN") PORT_CODE(KEYCODE_R)    PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t    T    >      RND      MERGE    RAND") PORT_CODE(KEYCODE_T)   PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line2") PORT_CODE(KEYCODE_HOME) PORT_CODE(KEYCODE_INSERT) PORT_CODE(KEYCODE_END)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE3") /* 0xF7FE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1   EDIT       !    BLUE     DEF FN") PORT_CODE(KEYCODE_1)       PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR('1') PORT_CHAR('!')
														   PORT_CODE(KEYCODE_F1)
															   PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2   CAPS LOCK  @    RED      FN")     PORT_CODE(KEYCODE_2)       PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR('2') PORT_CHAR('@')
														   PORT_CODE(KEYCODE_F2)
															   PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3   TRUE VID   #    MAGENTA  LINE")   PORT_CODE(KEYCODE_3)       PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR('3') PORT_CHAR('#')
														   PORT_CODE(KEYCODE_F3)
															   PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4   INV VID    $    GREEN    OPEN#")  PORT_CODE(KEYCODE_4)       PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR('4') PORT_CHAR('$')
														   PORT_CODE(KEYCODE_F4)
															   PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5   Left       %    CYAN     CLOSE#") PORT_CODE(KEYCODE_5)       PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR('5') PORT_CHAR('%')
														   PORT_CODE(KEYCODE_F5)
															   PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line3") PORT_CODE(KEYCODE_TAB) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_PGDN) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE4") /* 0xEFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0   DEL        _    BLACK    FORMAT") PORT_CODE(KEYCODE_0)       PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHAR('0') PORT_CHAR('_')
														   PORT_CODE(KEYCODE_F10)
															   PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9   GRAPH      )             POINT")  PORT_CODE(KEYCODE_9)       PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CHAR('9') PORT_CHAR(')')
														   PORT_CODE(KEYCODE_F9)
															   PORT_CODE(KEYCODE_DEL)
																		  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8   Right      (             CAT")    PORT_CODE(KEYCODE_8)       PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CHAR('8') PORT_CHAR('(')
														   PORT_CODE(KEYCODE_F8)
															   PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD)
																		  PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7   Up         '    WHITE    ERASE")  PORT_CODE(KEYCODE_7)       PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CHAR('7') PORT_CHAR('\'')
														   PORT_CODE(KEYCODE_F7)
															   PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6   Down       &    YELLOW   MOVE")   PORT_CODE(KEYCODE_6)       PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHAR('6') PORT_CHAR('&')
														   PORT_CODE(KEYCODE_F6)
															   PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line4") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_UP) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_DOWN) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line4") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE5") /* 0xDFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p    P    \"     TAB      (c)    PRINT") PORT_CODE(KEYCODE_P)    PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('"')
																	 PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o    O    ;      PEEK     OUT    POKE") PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(';')
																	 PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i    I    AT     CODE     IN     INPUT") PORT_CODE(KEYCODE_I)    PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u    U    OR     CHR$     ]      IF") PORT_CODE(KEYCODE_U)       PORT_CHAR('u') PORT_CHAR('U')// PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y    Y    AND    STR$     [      RETURN") PORT_CODE(KEYCODE_Y)   PORT_CHAR('y') PORT_CHAR('Y')// PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line5") PORT_CODE(KEYCODE_QUOTE) PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE6") /* 0xBFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l    L    =      USR      ATTR     LET") PORT_CODE(KEYCODE_L)    PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('=')
																	 PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k    K    +      LEN      SCREEN$  LIST") PORT_CODE(KEYCODE_K)   PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR('+')
																	 PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j    J    -      VAL      VAL$     LOAD") PORT_CODE(KEYCODE_J)   PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR('-')
																	 PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h    H    ^      SQR      CIRCLE   GOSUB") PORT_CODE(KEYCODE_H)  PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line6") PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE7") /* 0x7FFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)                                       PORT_CHAR(' ')
														   PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SYMBOL SHIFT") PORT_CODE(KEYCODE_RCONTROL) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
														   PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m    M    .      PI       INVERSE  PAUSE") PORT_CODE(KEYCODE_M)        PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR('.')
																	 PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n    N    ,      INKEY$   OVER     NEXT") PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(',')
																	 PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b    B    *      BIN      BRIGHT   BORDER") PORT_CODE(KEYCODE_B)       PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR('*')
																	 PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CS Line7") PORT_CODE(KEYCODE_ESC) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SS Line7") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_COMMA) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)


	PORT_START("mouse_input1")
	PORT_BIT(0xff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(30)

	PORT_START("mouse_input2")
	PORT_BIT(0xff, 0, IPT_MOUSE_Y) PORT_INVERT PORT_SENSITIVITY(30)

	PORT_START("mouse_input3")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Middle mouse button") PORT_CODE(MOUSECODE_BUTTON3)

	PORT_START("JOY1")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1) PORT_CODE(JOYCODE_Y_UP_SWITCH)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(1) PORT_CODE(JOYCODE_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)        PORT_PLAYER(1) PORT_CODE(JOYCODE_BUTTON2)

	PORT_START("TURBO")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TURBO") PORT_CODE(KEYCODE_F12) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, sprinter_state, turbo_changed, 0)
INPUT_PORTS_END

void sprinter_state::sprinter(machine_config &config)
{
	spectrum_128(config);
	config.device_remove("palette");
	config.set_default_layout(layout_sprinter);

	m_ram->set_default_size("64M");

	Z84C015(config.replace(), m_maincpu, X_SP / 12); // 3.5MHz default
	m_maincpu->set_m1_map(&sprinter_state::map_fetch);
	m_maincpu->set_memory_map(&sprinter_state::map_mem);
	m_maincpu->set_io_map(&sprinter_state::map_io);
	m_maincpu->nomreq_cb().set_nop();
	m_maincpu->set_irq_acknowledge_callback(NAME([](device_t &, int){ return 0xff; }));
	m_maincpu->irqack_cb().set(FUNC(sprinter_state::irq_off));

	ISA8(config, m_isa[0], X_SP / 5);
	m_isa[0]->set_custom_spaces();
	zxbus_device &zxbus(ZXBUS(config, "zxbus", 0));
	zxbus.set_iospace(m_isa[0], isa8_device::AS_ISA_IO);
	ZXBUS_SLOT(config, "zxbus2isa", 0, "zxbus", zxbus_cards, nullptr);

	ISA8(config, m_isa[1], X_SP / 5);
	m_isa[1]->set_custom_spaces();
	ISA8_SLOT(config, "isa8", 0, m_isa[1], pc_isa8_cards, nullptr, false);

	m_screen->set_raw(X_SP / 3, SPRINT_WIDTH, SPRINT_HEIGHT, { 0, SPRINT_XVIS - 1, 0, SPRINT_YVIS - 1 });
	m_screen->set_screen_update(FUNC(sprinter_state::screen_update));

	PALETTE(config, "palette", palette_device::BLACK).set_entries(256 * 8);

	PC_KBDC(config, m_kbd, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_kbd->out_data_cb().set(m_maincpu, FUNC(z84c015_device::rxa_w)); // KBD_DATR
	m_kbd->out_clock_cb().set(m_maincpu, FUNC(z84c015_device::rxca_w)); // KBD_CLKR
	m_kbd->out_clock_cb().append(m_maincpu, FUNC(z84c015_device::txca_w));
	m_kbd->out_clock_cb().append(FUNC(sprinter_state::on_kbd_data));

	m_maincpu->set_clk_trg<0>(X_SP / 48);
	m_maincpu->set_clk_trg<1>(X_SP / 48);
	m_maincpu->set_clk_trg<2>(X_SP / 48);

	rs232_port_device &m_rs232(RS232_PORT(config, "rs232", default_rs232_devices, "microsoft_mouse"));
	m_rs232.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	m_rs232.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	m_rs232.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	m_rs232.rxd_handler().set(m_maincpu, FUNC(z84c015_device::rxb_w)); // MOUSE_D
	m_maincpu->out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd)); // TXDB
	m_maincpu->zc_callback<0>().set(m_maincpu, FUNC(z84c015_device::rxcb_w)); // CLK_COM1
	m_maincpu->zc_callback<0>().append(m_maincpu, FUNC(z84c015_device::txcb_w));
	m_maincpu->zc_callback<2>().set(m_maincpu, FUNC(z84c015_device::trg3));

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

	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5);
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5);

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
