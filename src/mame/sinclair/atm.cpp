// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*******************************************************************************************

MicroART ATM (clone of Spectrum)

NOTES:
	Current implementation based on ATM Turbo 2+. If anybody wants to validate ATM1, existing
	code must be moved to atmtb2_state not modified.

TODO:
	* ports read
	* ATM2+ (compare to ATM2) has only 1M RAM vs 512K
	* Mem masks are hardcoded to 1M RAM
	* better handling of SHADOW ports
	* validate screen timings

*******************************************************************************************/

#include "emu.h"
#include "spec128.h"
#include "specpls3.h"

#include "beta_m.h"
#include "glukrs.h"
#include "bus/centronics/ctronics.h"
#include "sound/ay8910.h"

namespace {

#define LOG_MEM   (1U << 1)
#define LOG_VIDEO (1U << 2)
#define LOG_WARN  (1U << 3)

#define VERBOSE ( /*LOG_MEM | LOG_VIDEO |*/ LOG_WARN )
#include "logmacro.h"

#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGVIDEO(...) LOGMASKED(LOG_VIDEO, __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)

static constexpr u8 ROM_MASK = 0x7;
static constexpr u8 PEN_RAM_MASK = 0x40;
static constexpr u8 PEN_DOS7FFD_MASK = 0x80;

class atm_state : public spectrum_128_state
{
public:
	atm_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank_view0(*this, "bank_view0")
		, m_bank_view1(*this, "bank_view1")
		, m_bank_view2(*this, "bank_view2")
		, m_bank_view3(*this, "bank_view3")
		, m_bank_rom(*this, "bank_rom%u", 0U)
		, m_char_rom(*this, "charrom")
		, m_beta(*this, BETA_DISK_TAG)
		, m_centronics(*this, "centronics")
		, m_glukrs(*this, "glukrs")
		, m_palette(*this, "palette")
	{ }

	void atm(machine_config &config);
	void atmtb2(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;
	void video_start() override;

	rectangle get_screen_area() override;
	u8 get_border_color(u16 hpos, u16 vpos) override;
	void spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;

private:
	u8 beta_neutral_r(offs_t offset);
	u8 beta_enable_r(offs_t offset);
	u8 beta_disable_r(offs_t offset);

	void atm_ula_w(offs_t offset, u8 data);
	void atm_port_ffff_w(offs_t offset, u8 data);
	void atm_port_ff77_w(offs_t offset, u8 data);
	void atm_port_fff7_w(offs_t offset, u8 data);
	void atm_port_7ffd_w(offs_t offset, u8 data);

	void atm_io(address_map &map);
	void atm_mem(address_map &map);
	void atm_switch(address_map &map);

	void atm_update_video_mode();
	void atm_update_screen_lo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atm_update_screen_hi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atm_update_screen_tx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atm_update_memory();

	memory_view m_bank_view0;
	memory_view m_bank_view1;
	memory_view m_bank_view2;
	memory_view m_bank_view3;
	required_memory_bank_array<4> m_bank_rom;
	optional_region_ptr<u8> m_char_rom; // required for ATM2, absent in ATM1
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	required_device<beta_disk_device> m_beta;
	required_device<centronics_device> m_centronics;
	required_device<glukrs_device> m_glukrs;
	required_device<device_palette_interface> m_palette;

	bool is_shadow_active() { return m_beta->is_active(); }
	u8 &pen_page(u8 bank) { return m_pages_map[BIT(m_port_7ffd_data, 4)][bank]; }

	bool m_pen;           // PEN - extended memory manager
	bool m_cpm;
	u8 m_pages_map[2][4]; // map: 0,1

	bool m_pen2;          // palette selector
	u8 m_rg = 0b011;      // 0:320x200lo, 2:640:200hi, 3:256x192zx, 6:80x25txt
	u8 m_br3;
};

void atm_state::atm_update_memory()
{
	using views_link = std::reference_wrapper<memory_view>;
	views_link views[] = { m_bank_view0, m_bank_view1, m_bank_view2, m_bank_view3 };
	LOGMEM("7FFD.%d = %X:", BIT(m_port_7ffd_data, 4), (m_port_7ffd_data & 0x07));
	for (auto bank = 0; bank < 4 ; bank++)
	{
		u8 page = pen_page(bank);
		if (!m_pen)
			page = ROM_MASK;

		if (page & PEN_RAM_MASK)
		{
			if (page & PEN_DOS7FFD_MASK)
				page = (page & 0xf8) | (m_port_7ffd_data & 0x07);
			page = page & 0x3f; // TODO size dependent
			m_bank_ram[bank]->set_entry(page);
			views[bank].get().disable();
			LOGMEM(" RA(%X>%X)", m_bank_ram[bank]->entry(), page);
		}
		else
		{
			if ((page & PEN_DOS7FFD_MASK) && !BIT(page, 1))
				page = (page & ~1) | is_shadow_active();
			page = page & ROM_MASK;
			m_bank_rom[bank]->set_entry(page);
			views[bank].get().select(0);
			LOGMEM(" RO(%X>%X)", m_bank_rom[bank]->entry(), page);
		}
	}
	LOGMEM("\n");
}

void atm_state::atm_ula_w(offs_t offset, u8 data)
{
	m_br3 = ~offset & 0x08;
	spectrum_128_state::spectrum_ula_w(offset, data);
}

void atm_state::atm_port_ffff_w(offs_t offset, u8 data)
{
	if(!is_shadow_active())
		return;

	if (m_pen2)
	{
		m_beta->param_w(data);
	}
	else
	{
		// Must read current ULA value (which is doesn't work now) from the BUS.
		// Good enough as non-border case is too complicated and possibly no software uses it.
		u8 pen = get_border_color(m_screen->hpos(), m_screen->vpos());
		m_palette->set_pen_color(pen,
			(BIT(~data, 1) * 0xaa) | (BIT(~data, 6) * 0x55),
			(BIT(~data, 4) * 0xaa) | (BIT(~data, 7) * 0x55),
			(BIT(~data, 0) * 0xaa) | (BIT(~data, 5) * 0x55));
	}
}

void atm_state::atm_port_7ffd_w(offs_t offset, u8 data)
{
	/* disable paging */
	if (BIT(m_port_7ffd_data, 5))
		return;

	m_port_7ffd_data = data;
	atm_update_memory();

	m_screen->update_now();
	m_screen_location = m_ram->pointer() + ((BIT(m_port_7ffd_data, 3) ? 7 : 5) << 14);
}

void atm_state::atm_port_ff77_w(offs_t offset, u8 data)
{
	if (!is_shadow_active())
		return;

	m_pen = BIT(offset, 8);
	m_cpm = BIT(offset, 9);
	m_pen2 = BIT(offset, 14);
	LOGMASKED(LOG_VIDEO | LOG_MEM, "PEN %s, CPM %s, PEN2 %s\n", m_pen ? "on" : "off", m_cpm ? "off" : "on", m_pen2 ? "off" : "on");
	atm_update_memory();

	m_maincpu->set_clock(X1_128_SINCLAIR / 10 * (1 << BIT(data, 3))); // 0 - 3.5MHz, 1 - 7MHz

	int rg = data & 0x07;
	if ( m_rg ^ rg )
	{
		m_rg = rg;
		atm_update_video_mode();
	}
}

void atm_state::atm_port_fff7_w(offs_t offset, u8 data)
{
	if (!is_shadow_active())
		return;

	if (BIT(data, 7))
		m_glukrs->enable();
	else
		m_glukrs->disable();

	u8 bank = offset >> 14;
	u8 page = (data & 0xc0) | (~data & 0x3f);

	LOGMEM("PEN%s.%s = %X %s%d: %02X\n", (page | PEN_DOS7FFD_MASK) ? "+" : "!", BIT(m_port_7ffd_data, 4), data, (page & PEN_RAM_MASK) ? "RAM" : "ROM", bank, page & 0x3f);
	pen_page(bank) = page;
	atm_update_memory();
}

rectangle atm_state::get_screen_area()
{
	switch (m_rg)
	{
		case 0b110: // 80x25txt
		case 0b010: // 640x200
			return rectangle { 208, 208 + 639, 76, 76 + 199 };
			break;
		case 0b000: // 320x200
			return rectangle { 104, 104 + 319, 76, 76 + 199 };
			break;
		case 0b011: // 256x192
		default:
			return rectangle { 136, 136 + 255, 80, 80 + 191 };
			break;
	}
}

u8 atm_state::get_border_color(u16 hpos, u16 vpos)
{
	return m_br3 | (m_port_fe_data & 0x07);
}

void atm_state::atm_update_video_mode()
{
	bool zx_scale = BIT(m_rg, 0);
	bool double_width = BIT(m_rg, 1) && !zx_scale;
	u8 border_x = (40 - (32 * !zx_scale)) << double_width;
	u8 border_y = (40 - (4 * !zx_scale));
	rectangle scr = get_screen_area();
	m_screen->configure(448 << double_width, 312, {scr.left() - border_x, scr.right() + border_x, scr.top() - border_y, scr.bottom() + border_y}, m_screen->frame_period().as_attoseconds());
	LOGVIDEO("Video mode: %d\n", m_rg);

	//spectrum_palette(m_palette);
}

void atm_state::spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch (m_rg)
	{
		case 0b110: // txt
			atm_update_screen_tx(screen, bitmap, cliprect);
			break;
		case 0b010: // 640x200
			atm_update_screen_hi(screen, bitmap, cliprect);
			break;
		case 0b000: // 320x200
			atm_update_screen_lo(screen, bitmap, cliprect);
			break;
		case 0b011: // 256x192
		default:    // + unsupported
			spectrum_128_state::spectrum_update_screen(screen, bitmap, cliprect);
			break;
	}
}

void atm_state::atm_update_screen_lo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 y = vpos - get_screen_area().top();
		for (u16 hpos = cliprect.left(); hpos <= cliprect.right(); hpos++)
		{
			u16 x = hpos - get_screen_area().left();
			u8 *scr = m_screen_location;
			if (!BIT(x, 1)) scr -= 4 << 14;
			if (BIT(x, 2)) scr += 0x2000;
			scr += (x >> 3) + y * 40;
			u8 pix_pair = *scr;
			pix_pair = x & 1
				? (((pix_pair & 0x80) >> 1) | (pix_pair & 0x38)) >> 3
				: ((pix_pair & 0x40) >> 3) | (pix_pair & 0x07);
			bitmap.pix(vpos, hpos) = pix_pair;
		}
	}
}

void atm_state::atm_update_screen_hi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 y = vpos - get_screen_area().top();
		for (u16 hpos = cliprect.left() & 0xfff8; hpos <= cliprect.right();)
		{
			u16 x = hpos - get_screen_area().left();
			u8 *scr = m_screen_location + (x >> 4) + y * 40;
			if (BIT(x, 3)) scr += 0x2000;

			u8 attr = *(scr - (4 << 14));
			u8 fg = ((attr & 0x40) >> 3) | (attr & 0x07);
			u8 bg = (((attr & 0x80) >> 1) | (attr & 0x38)) >> 3;

			u8 chunk = *scr;
			for (u8 i = 0x80; i; i >>= 1)
			{
				bitmap.pix(vpos, hpos++) = (chunk & i) ? fg : bg;
			}
		}
	}
}

void atm_state::atm_update_screen_tx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 y = vpos - get_screen_area().top();
		for (u16 hpos = cliprect.left() & 0xfff8; hpos <= cliprect.right();)
		{
			u16 x = hpos - get_screen_area().left();
			u8 *symb_location = m_screen_location + 0x1c0 + (x >> 4) + ((y >> 3) * 64);
			u8 *attr_location = symb_location - (4 << 14) + BIT(x, 3);
			if (BIT(x, 3))
				symb_location += 0x2000;
			else
				attr_location += 0x2000;

			u8 attr = *attr_location;
			u8 fg = ((attr & 0x40) >> 3) | (attr & 0x07);
			u8 bg = (((attr & 0x80) >> 1) | (attr & 0x38)) >> 3;

			u8 chunk = *(m_char_rom + (*symb_location << 3) + (y & 0x07));
			for (u8 i = 0x80; i; i >>= 1)
			{
				bitmap.pix(vpos, hpos++) = (chunk & i) ? fg : bg;
			}
		}
	}
}

u8 atm_state::beta_neutral_r(offs_t offset)
{
	return m_program.read_byte(offset);
}

u8 atm_state::beta_enable_r(offs_t offset)
{
	if (!machine().side_effects_disabled()) {
		u8 page = pen_page(0);
		if (!(page & PEN_RAM_MASK) && !is_shadow_active()) {
			m_beta->enable();
			atm_update_memory();
		}
	}
	return m_program.read_byte(offset + 0x3d00);
}

u8 atm_state::beta_disable_r(offs_t offset)
{
	if (!machine().side_effects_disabled()) {
		if (is_shadow_active() && m_cpm) {
			m_beta->disable();
			atm_update_memory();
		}
	}
	return m_program.read_byte(offset + 0x4000);
}

void atm_state::atm_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw(m_bank_ram[0]);
	map(0x0000, 0x3fff).view(m_bank_view0);
	m_bank_view0[0](0x0000, 0x3fff).bankr(m_bank_rom[0]);

	map(0x4000, 0x7fff).bankrw(m_bank_ram[1]);
	map(0x4000, 0x7fff).view(m_bank_view1);
	m_bank_view1[0](0x4000, 0x7fff).bankr(m_bank_rom[1]);

	map(0x8000, 0xbfff).bankrw(m_bank_ram[2]);
	map(0x8000, 0xbfff).view(m_bank_view2);
	m_bank_view2[0](0x8000, 0xbfff).bankr(m_bank_rom[2]);

	map(0xc000, 0xffff).bankrw(m_bank_ram[3]);
	map(0xc000, 0xffff).view(m_bank_view3);
	m_bank_view3[0](0xc000, 0xffff).bankr(m_bank_rom[3]);
}

void atm_state::atm_io(address_map &map)
{
	map.unmap_value_high();
	map(0x001f, 0x001f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	map(0x003f, 0x003f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	map(0x005f, 0x005f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	map(0x007f, 0x007f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	map(0x00ff, 0x00ff).mirror(0xff00).r(m_beta, FUNC(beta_disk_device::state_r));
	map(0x00ff, 0x00ff).mirror(0xff00).w(FUNC(atm_state::atm_port_ffff_w));
	map(0x00f6, 0x00f6).select(0xff08).rw(FUNC(atm_state::spectrum_ula_r), FUNC(atm_state::atm_ula_w));
	map(0x00fb, 0x00fb).mirror(0xff00).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x00fd, 0x00fd).mirror(0xff00).w(FUNC(atm_state::atm_port_7ffd_w));
	map(0x0077, 0x0077).select(0xff00).w(FUNC(atm_state::atm_port_ff77_w));
	map(0x00f7, 0x00f7).select(0xff00).w(FUNC(atm_state::atm_port_fff7_w));
	map(0xdff7, 0xdff7).w(m_glukrs, FUNC(glukrs_device::address_w));
	map(0xdef7, 0xdef7).w(m_glukrs, FUNC(glukrs_device::address_w)); // TODO shadow only
	map(0xbff7, 0xbff7).rw(m_glukrs, FUNC(glukrs_device::data_r), FUNC(glukrs_device::data_w));
	map(0xbef7, 0xbef7).rw(m_glukrs, FUNC(glukrs_device::data_r), FUNC(glukrs_device::data_w)); // TODO shadow only
	map(0xfadf, 0xfadf).mirror(0x0500).nopr(); // TODO 0xfadf, 0xfbdf, 0xffdf Kempston Mouse
	map(0x8000, 0x8000).mirror(0x3ffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc000).mirror(0x3ffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}

void atm_state::atm_switch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(atm_state::beta_neutral_r)); // Overlap with previous because we want real addresses on the 3e00-3fff range
	map(0x3d00, 0x3dff).r(FUNC(atm_state::beta_enable_r));
	map(0x4000, 0xffff).r(FUNC(atm_state::beta_disable_r));
}


void atm_state::machine_start()
{
	spectrum_128_state::machine_start();

	save_item(NAME(m_pen));
	save_item(NAME(m_cpm));
	save_item(NAME(m_pages_map));
	save_item(NAME(m_pen2));
	save_item(NAME(m_rg));
	save_item(NAME(m_br3));

	// reconfigure ROMs
	memory_region *rom = memregion("maincpu");
	for (auto i = 0; i < 4; i++)
		m_bank_rom[i]->configure_entries(0, 8, rom->base() + 0x10000, 0x4000);
	m_bank_ram[0]->configure_entries(0, m_ram->size() / 0x4000, m_ram->pointer(), 0x4000);

	m_maincpu->space(AS_PROGRAM).specific(m_program);
}

void atm_state::machine_reset()
{
	m_beta->enable();
	m_glukrs->disable();

	m_port_7ffd_data = 0;
	m_port_1ffd_data = -1;

	m_br3 = 0;
	atm_port_ff77_w(0x4000, 3); // CPM=0(on), PEN=0(off), PEN2=1(off); vmode: zx
}

void atm_state::video_start()
{
	spectrum_state::video_start();
	m_screen_location = m_ram->pointer() + (5 << 14);
	m_contention_pattern = {};
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,            // 8 x 8 characters
	96,              // 96 characters
	1,               // 1 bits per pixel
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	8 * 8            // every char takes 8 bytes
};

static const gfx_layout atm_charlayout =
{
	8, 8,            // 8 x 8 characters
	256,             // 96 characters
	1,               // 1 bits per pixel
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	8 * 8            // every char takes 8 bytes
};

static GFXDECODE_START( gfx_atm )
	GFXDECODE_ENTRY( "maincpu", 0x1fd00, spectrum_charlayout, 7, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_atmtb2 )
	GFXDECODE_ENTRY( "charrom", 0, atm_charlayout, 7, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x13d00, spectrum_charlayout, 7, 1 )
GFXDECODE_END

void atm_state::atm(machine_config &config)
{
	spectrum_128(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &atm_state::atm_mem);
	m_maincpu->set_addrmap(AS_IO, &atm_state::atm_io);
	m_maincpu->set_addrmap(AS_OPCODES, &atm_state::atm_switch);
	m_maincpu->nomreq_cb().set_nop();

	m_screen->set_raw(X1_128_SINCLAIR / 5, 448, 312, {get_screen_area().left() - 40, get_screen_area().right() + 40, get_screen_area().top() - 40, get_screen_area().bottom() + 40});
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_atm);

	BETA_DISK(config, m_beta, 0);
	GLUKRS(config, m_glukrs);

	CENTRONICS(config, m_centronics, centronics_devices, "covox");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	config.device_remove("exp");
}

void atm_state::atmtb2(machine_config &config)
{
	atm(config);

	// 1M in ATM2+ only. TODO mem masks for custom size
	m_ram->set_default_size("1M");//.set_extra_options("128K,256K,512K,1M");
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_atmtb2);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( atm )
	ROM_REGION(0x030000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1", "v.1.03")
	ROMX_LOAD( "atm103.rom", 0x020000, 0x10000, CRC(4912e249) SHA1(a4adff05bb215dd126c47201b36956115b8fed76), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "v.1.06 joined")
	ROMX_LOAD( "atm106.rom", 0x020000, 0x10000, CRC(75350b37) SHA1(2afc9994f026645c74b6c4b35bcee2e0bc0d6edc), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v3", "v.1.06")
	ROMX_LOAD( "atm106-1.rom", 0x020000, 0x4000, CRC(658c98f1) SHA1(1ec694795aa6cac10147e58f38a9db0bdf7ed89b), ROM_BIOS(2))
	ROMX_LOAD( "atm106-2.rom", 0x024000, 0x4000, CRC(8fe367f9) SHA1(56de8fd39061663b9c315b74fd3c31acddae279c), ROM_BIOS(2))
	ROMX_LOAD( "atm106-3.rom", 0x028000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(2))
	ROMX_LOAD( "atm106-4.rom", 0x02c000, 0x4000, CRC(f352f2ab) SHA1(6045500ab01be708cef62327e9821b4a358a4673), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v4", "v.1.03rs")
	ROMX_LOAD( "atm103rs.rom", 0x020000, 0x10000, CRC(cdec1dfb) SHA1(08190807c6b110cb2e657d8e7d0ad18668915375), ROM_BIOS(3))
ROM_END

ROM_START( atmtb2 )
	ROM_REGION(0x030000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v1.07.13")
	ROM_SYSTEM_BIOS(0, "v1.07.12", "BIOS v1.07.12, CP/M v2.2, TR-DOS v5.03") // joined dump
	ROMX_LOAD( "atmtb2.rom",   0x020000, 0x10000,CRC(05218c26) SHA1(71ed9864e7aa85131de97cf1e53dc152e7c79488), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.07.12a", "BIOS v1.07.12, CP/M v2.2, TR-DOS v5.03 (split)")
	ROMX_LOAD( "atmtb2-1.rom", 0x020000, 0x4000, CRC(658c98f1) SHA1(1ec694795aa6cac10147e58f38a9db0bdf7ed89b), ROM_BIOS(1))
	ROMX_LOAD( "atmtb2-2.rom", 0x024000, 0x4000, CRC(bc3f6b2b) SHA1(afa9df63857141fef270e2c97e12d2edc60cf919), ROM_BIOS(1))
	ROMX_LOAD( "atmtb2-3.rom", 0x028000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(1))
	ROMX_LOAD( "atmtb2-4.rom", 0x02c000, 0x4000, CRC(5869d8c4) SHA1(c3e198138f528ac4a8dff3c76cd289fd4713abff), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v1.07.13", "BIOS v1.07.13, CP/M v2.2, TR-DOS v5.03")
	ROMX_LOAD( "atmtb213.rom", 0x020000, 0x10000, CRC(34a91d53) SHA1(8f0af0f3c0ff1644535f20545c73d01576d6e52f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v1.37", "Dual eXtra v1.37XT: BIOS v1.07.15, CP/M v2.2, TR-DOS v5.04R")
	ROMX_LOAD( "atmtb2x37xt.rom", 0x010000, 0x20000, CRC(e5ef44d9) SHA1(3fbb9ace7cb031e7365c19e4f8b67ed366e24064), ROM_BIOS(3))

	ROM_REGION(0x01000, "keyboard", ROMREGION_ERASEFF)
	// XT Keyboard
	ROM_LOAD( "rf2ve3.rom",  0x0000, 0x0580, CRC(35e0f9ec) SHA1(adcf14758fab8472cfa0167af7e8326c66416416))
	// AT Keyboard
	ROM_LOAD( "rfat710.rom", 0x0600, 0x0680, CRC(03734365) SHA1(6cb6311727fad9bc4ccb18919c3c39b37529b8e6))

	ROM_REGION(0x08000, "charrom", ROMREGION_ERASEFF)
	// Char gen rom
	ROM_LOAD( "sgen.rom", 0x0000, 0x0800, CRC(1f4387d6) SHA1(93b3774dc8a486643a1bdd48c606b0c84fa0e22b))
ROM_END

} // Anonymous namespace


/*    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT      CLASS      INIT        COMPANY     FULLNAME              FLAGS */
COMP( 1991, atm,     spec128, 0,      atm,     spec_plus, atm_state, empty_init, "MicroART", "ATM-Turbo (ATM-CP)", MACHINE_NOT_WORKING)
COMP( 1992, atmtb2,  spec128, 0,      atmtb2,  spec_plus, atm_state, empty_init, "MicroART", "ATM-Turbo 2",        MACHINE_SUPPORTS_SAVE)
//COMP( 1993, atmtb2p, spec128, 0,      atmtb2p, spec_plus, atm_state, empty_init, "MicroART", "ATM-Turbo 2+",       MACHINE_NOT_WORKING) // only supports 1M RAM vs. 512K in atmtb2
