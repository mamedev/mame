// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*******************************************************************************************

MicroART ATM (clone of Spectrum)

NOTES:
    Current implementation based on ATM Turbo 2+. If anybody wants to validate ATM1, existing
    code must be moved to atmtb2_state not modified.

*******************************************************************************************/

#include "emu.h"
#include "atm.h"

#include "bus/ata/atapicdr.h"
#include "bus/ata/hdd.h"
#include "sound/ay8910.h"

#define LOG_MEM   (1U << 1)
#define LOG_VIDEO (1U << 2)
#define LOG_WARN  (1U << 3)

#define VERBOSE ( /*LOG_GENERAL | LOG_MEM | LOG_VIDEO |*/ LOG_WARN )
#include "logmacro.h"

#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGVIDEO(...) LOGMASKED(LOG_VIDEO, __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)

void atm_state::atm_update_cpu()
{
	m_maincpu->set_clock(X1_128_SINCLAIR / 10 * (1 << BIT(m_port_77_data, 3))); // 0 - 3.5MHz, 1 - 7MHz
}

void atm_state::atm_update_io()
{
	if (is_dos_active())
		m_io_view.select(0);
	else
		m_io_view.disable();
}

void atm_state::atm_update_memory()
{
	using views_link = std::reference_wrapper<memory_view>;
	views_link views[] = { m_bank_view0, m_bank_view1, m_bank_view2, m_bank_view3 };
	LOGMEM("PEN%d.%X ", BIT(m_port_7ffd_data, 4), (m_port_7ffd_data & 0x07));
	for (auto bank = 0; bank < 4 ; bank++)
	{
		u16 page = atm_update_memory_get_page(bank);
		const char* is_dos7ffd = page & PEN_DOS7FFD_MASK ? "+" : " ";
		if (page & PEN_RAMNROM_MASK)
		{
			if (page & PEN_DOS7FFD_MASK)
				page = merge_ram_with_7ffd(page);
			LOGMEM("RA%s%X ", is_dos7ffd, page & ram_pages_mask);
			m_bank_ram[bank]->set_entry(page & ram_pages_mask);
			if (page & PEN_WRDISBL_MASK)
				views[bank].get().select(1);
			else
				views[bank].get().disable();
		}
		else
		{
			if (page & PEN_DOS7FFD_MASK)
				page = (page & ~1) | is_dos_active();
			LOGMEM("RO%s%X ", is_dos7ffd, page & rom_pages_mask);
			m_bank_rom[bank]->set_entry(page & rom_pages_mask);
			views[bank].get().select(0);
		}
	}
	LOGMEM("\n");
}

u16 atm_state::atm_update_memory_get_page(u8 bank)
{
	return m_pen ? pen_page(bank) : (u16) (~PEN_RAMNROM_MASK & ~PEN_DOS7FFD_MASK);
}

void atm_state::atm_ula_w(offs_t offset, u8 data)
{
	m_br3 = ~offset & 0x08;
	spectrum_128_state::spectrum_ula_w(offset, data);
}

void atm_state::atm_port_ff_w(offs_t offset, u8 data)
{
	if (m_pen2)
	{
		m_beta_drive_selected = data;
		m_beta->param_w(data);
	}
	else
	{
		// Must read current ULA value (which is doesn't work now) from the BUS.
		// Good enough as non-border case is too complicated and possibly no software uses it.
		u8 pen = get_border_color(m_screen->hpos(), m_screen->vpos());
		m_palette_data[pen] = data;
		m_palette->set_pen_color(pen,
			(BIT(~data, 1) * 0xaa) | (BIT(~data, 6) * 0x55),
			(BIT(~data, 4) * 0xaa) | (BIT(~data, 7) * 0x55),
			(BIT(~data, 0) * 0xaa) | (BIT(~data, 5) * 0x55));
	}
}

void atm_state::atm_port_7ffd_w(offs_t offset, u8 data)
{
	if (is_port_7ffd_locked())
		return;

	m_port_7ffd_data = data;
	atm_update_memory();

	m_screen->update_now();
	m_screen_location = m_ram->pointer() + ((BIT(m_port_7ffd_data, 3) ? 7 : 5) << 14);
}

void atm_state::atm_port_77_w(offs_t offset, u8 data)
{
	m_port_77_data = data;

	m_pen = BIT(offset, 8);
	m_cpm_n = BIT(offset, 9);
	atm_update_io();

	m_pen2 = BIT(offset, 14);
	LOGMASKED(LOG_VIDEO | LOG_MEM, "PEN %s, CPM %s, PEN2 %s\n", m_pen ? "on" : "off", m_cpm_n ? "off" : "on", m_pen2 ? "off" : "on");
	atm_update_memory();

	atm_update_cpu();

	int rg = data & 0x07;
	if ( m_rg ^ rg )
	{
		m_rg = rg;
		atm_update_video_mode();
	}
}

void atm_state::atm_port_f7_w(offs_t offset, u8 data)
{
	u8 bank = offset >> 14;
	u16 page = (u16(data & 0xc0) << 8) | u8(~data & 0x3f);

	LOGMEM("ATM%s=%X %s%d%s%02X\n", BIT(m_port_7ffd_data, 4), data, (page & PEN_RAMNROM_MASK) ? "RAM" : "ROM", bank, (page & PEN_DOS7FFD_MASK) ? "+" : " ", page & 0x3f);
	pen_page(bank) = page;
	atm_update_memory();
	atm_update_io();
}

INTERRUPT_GEN_MEMBER(atm_state::atm_interrupt)
{
	// 14395=64*224+59 z80(3.5Hz) clocks between INT and screen paper begins. Screen clock is 7Hz.
	m_irq_on_timer->adjust(m_screen->time_until_pos(80 - 64, 80) - m_screen->clocks_to_attotime(118));
}

rectangle atm_state::get_screen_area()
{
	switch (m_rg)
	{
		case 0b111:
		case 0b110: // 80x25txt
		case 0b010: // 640x200
			return rectangle { 80, 80 + 639, 80, 80 + 199 };
			break;
		case 0b000: // 320x200
			return rectangle { 80, 80 + 319, 80, 80 + 199 };
			break;
		case 0b011: // 256x192zx
		default:
			return rectangle { 80, 80 + 255, 80, 80 + 191 };
			break;
	}
}

u8 atm_state::get_border_color(u16 hpos, u16 vpos)
{
	return m_br3 | (m_port_fe_data & 0x07);
}

void atm_state::atm_update_video_mode()
{
	bool zx_scale = m_rg & 1;
	bool double_width = BIT(m_rg, 1) && !zx_scale;
	u8 border_x = (40 - (32 * !zx_scale)) << double_width;
	u8 border_y = (40 - (4 * !zx_scale));
	rectangle scr = get_screen_area();
	m_screen->configure(448 << double_width, m_screen->height(), {scr.left() - border_x, scr.right() + border_x, scr.top() - border_y, scr.bottom() + border_y}, m_screen->frame_period().as_attoseconds());
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

			u8 chunk = *(m_char_location + (*symb_location << 3) + (y & 0x07));
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
	if (!machine().side_effects_disabled() && !m_beta->is_active())
	{
		bool is_rom0 = !(atm_update_memory_get_page(0) & PEN_RAMNROM_MASK);
		if (is_rom0 || !m_cpm_n)
		{
			m_beta->enable();
			atm_update_memory();
			atm_update_io();
		}
	}
	return beta_neutral_r(offset + 0x3d00);
}

u8 atm_state::beta_disable_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && m_beta->is_active())
	{
		if (m_cpm_n)
		{
			m_beta->disable();
			atm_update_memory();
			atm_update_io();
		}
	}
	return beta_neutral_r(offset + 0x4000);
}

u8 atm_state::ata_r(offs_t offset)
{
	u8 ata_offset = BIT(offset, 5, 3);
	u16 data = m_ata->cs0_r(ata_offset);

	if (!machine().side_effects_disabled() && !ata_offset)
		m_ata_data_latch = data >> 8;

	return data & 0xff;
}

void atm_state::ata_w(offs_t offset, u8 data)
{
	u8 ata_offset = BIT(offset, 5, 3);
	u16 ata_data = data;
	if (!ata_offset)
		ata_data |= m_ata_data_latch << 8;

	m_ata->cs0_w(ata_offset, ata_data);
}

template <u8 Bank> void atm_state::atm_ram_w(offs_t offset, u8 data)
{
	if (m_rg == 0b011 && (m_bank_ram[Bank]->entry() == (BIT(m_port_7ffd_data, 3) ? 7 : 5)) && offset < 0x1b00)
		m_screen->update_now();

	((u8*)m_bank_ram[Bank]->base())[offset] = data;
}

void atm_state::atm_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_bank_ram[0]).w(FUNC(atm_state::atm_ram_w<0>));
	map(0x0000, 0x3fff).view(m_bank_view0);
	m_bank_view0[0](0x0000, 0x3fff).bankr(m_bank_rom[0]).nopw();
	m_bank_view0[1](0x0000, 0x3fff).nopw(); // RO RAM

	map(0x4000, 0x7fff).bankr(m_bank_ram[1]).w(FUNC(atm_state::atm_ram_w<1>));
	map(0x4000, 0x7fff).view(m_bank_view1);
	m_bank_view1[0](0x4000, 0x7fff).bankr(m_bank_rom[1]).nopw();
	m_bank_view1[1](0x4000, 0x7fff).nopw();

	map(0x8000, 0xbfff).bankr(m_bank_ram[2]).w(FUNC(atm_state::atm_ram_w<2>));
	map(0x8000, 0xbfff).view(m_bank_view2);
	m_bank_view2[0](0x8000, 0xbfff).bankr(m_bank_rom[2]).nopw();
	m_bank_view2[1](0x8000, 0xbfff).nopw();

	map(0xc000, 0xffff).bankr(m_bank_ram[3]).w(FUNC(atm_state::atm_ram_w<3>));
	map(0xc000, 0xffff).view(m_bank_view3);
	m_bank_view3[0](0xc000, 0xffff).bankr(m_bank_rom[3]).nopw();
	m_bank_view3[1](0xc000, 0xffff).nopw();
}

void atm_state::atm_io(address_map &map)
{
	map.unmap_value_high();

	// PORTS: Always
	map(0x00f6, 0x00f6).select(0xff08).rw(FUNC(atm_state::spectrum_ula_r), FUNC(atm_state::atm_ula_w));
	map(0x00fb, 0x00fb).mirror(0xff00).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x00fd, 0x00fd).mirror(0xff00).w(FUNC(atm_state::atm_port_7ffd_w));

	map(0xfadf, 0xfadf).mirror(0x0500).nopr(); // TODO 0xfadf, 0xfbdf, 0xffdf Kempston Mouse
	map(0x8000, 0x8000).mirror(0x3ffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xc000, 0xc000).mirror(0x3ffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));

	// PORTS: Shadow
	map(0x0000, 0xffff).view(m_io_view);
	m_io_view[0](0x001f, 0x001f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::status_r), FUNC(beta_disk_device::command_w));
	m_io_view[0](0x003f, 0x003f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::track_r), FUNC(beta_disk_device::track_w));
	m_io_view[0](0x005f, 0x005f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::sector_r), FUNC(beta_disk_device::sector_w));
	m_io_view[0](0x007f, 0x007f).mirror(0xff00).rw(m_beta, FUNC(beta_disk_device::data_r), FUNC(beta_disk_device::data_w));
	m_io_view[0](0x00ff, 0x00ff).mirror(0xff00).r(m_beta, FUNC(beta_disk_device::state_r)).w(FUNC(atm_state::atm_port_ff_w));

	m_io_view[0](0x0077, 0x0077).select(0xff00).w(FUNC(atm_state::atm_port_77_w));
	m_io_view[0](0x00f7, 0x00f7).select(0xff00).w(FUNC(atm_state::atm_port_f7_w));

	// A: .... .... nnn0 1111
	m_io_view[0](0x000f, 0x000f).select(0xffe0).rw(FUNC(atm_state::ata_r), FUNC(atm_state::ata_w));
	// A: .... ...1 0000 1111
	m_io_view[0](0x010f, 0x010f).mirror(0xfe00).lrw8(NAME([this](offs_t offset) { return m_ata_data_latch; })
		, NAME([this](offs_t offset, u8 data) { m_ata_data_latch = data; }));
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

	save_item(NAME(m_port_77_data));
	save_item(NAME(m_pen));
	save_item(NAME(m_cpm_n));
	save_item(NAME(m_pages_map));
	save_item(NAME(m_pen2));
	save_item(NAME(m_rg));
	save_item(NAME(m_br3));
	save_item(NAME(m_beta_drive_selected));

	// reconfigure ROMs
	memory_region *rom = memregion("maincpu");
	rom_pages_mask = (rom->bytes() - 0x10001) / 0x4000;
	for (auto i = 0; i < 4; i++)
		m_bank_rom[i]->configure_entries(0, rom_pages_mask + 1, rom->base() + 0x10000, 0x4000); // todo dyn

	ram_pages_mask = (m_ram->size() - 1) / 0x4000;
	m_bank_ram[0]->configure_entries(0, ram_pages_mask + 1, m_ram->pointer(), 0x4000);

	m_maincpu->space(AS_PROGRAM).specific(m_program);
}

void atm_state::machine_reset()
{
	m_beta->enable();
	m_beta_drive_selected = 0;

	m_port_7ffd_data = 0;
	m_port_1ffd_data = -1;
	m_port_77_data = 0;

	m_br3 = 0;
	m_palette_data = { 0xff };
	atm_port_77_w(0x4000, 3); // m_port_77_data: CPM=0(on), PEN=0(off), PEN2=1(off); vmode: zx
}

void atm_state::video_start()
{
	spectrum_state::video_start();
	m_screen_location = m_ram->pointer() + (5 << 14);
	m_char_location = m_char_rom;
	subdevice<gfxdecode_device>("gfxdecode")->gfx(0)->set_source(m_char_location);
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
	256,             // 256 characters
	1,               // 1 bits per pixel
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	8 * 8            // every char takes 8 bytes
};

static GFXDECODE_START( gfx_atm )
	GFXDECODE_ENTRY( "maincpu", 0, atm_charlayout, 7, 1 ) // charrom
	GFXDECODE_ENTRY( "maincpu", 0x1fd00, spectrum_charlayout, 7, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_atmtb2 )
	GFXDECODE_ENTRY( "maincpu", 0, atm_charlayout, 7, 1 ) // charrom
	GFXDECODE_ENTRY( "maincpu", 0x13d00, spectrum_charlayout, 7, 1 )
GFXDECODE_END

static void atm_ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_CDROM);
}

void atm_state::atm(machine_config &config)
{
	spectrum_128(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &atm_state::atm_mem);
	m_maincpu->set_addrmap(AS_IO, &atm_state::atm_io);
	m_maincpu->set_addrmap(AS_OPCODES, &atm_state::atm_switch);
	m_maincpu->set_vblank_int("screen", FUNC(atm_state::atm_interrupt));
	m_maincpu->nomreq_cb().set_nop();

	m_screen->set_raw(X1_128_SINCLAIR / 5, 448, 312, {get_screen_area().left() - 40, get_screen_area().right() + 40, get_screen_area().top() - 40, get_screen_area().bottom() + 40});
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_atm);

	BETA_DISK(config, m_beta, 0);
	ATA_INTERFACE(config, m_ata).options(atm_ata_devices, nullptr, nullptr, false);

	CENTRONICS(config, m_centronics, centronics_devices, "covox");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	config.device_remove("exp");
}


/******************************************************************************
 * ATM Turbo 2
 * ***************************************************************************/
void atm_state::atmtb2(machine_config &config)
{
	atm(config);

	m_ram->set_default_size("512K").set_extra_options("128K,256K");

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_atmtb2);
}


/******************************************************************************
 * ATM Turbo 2+
 * ***************************************************************************/
void atm_state::atmtb2plus(machine_config &config)
{
	atmtb2(config);

	m_ram->set_default_size("1M").set_extra_options("128K,256K,512K");
}


/***************************************************************************
 * Game driver(s)
 * ************************************************************************/
ROM_START( atm )
	ROM_REGION(0x020000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1", "v.1.03")
	ROMX_LOAD( "atm103.rom", 0x010000, 0x10000, CRC(4912e249) SHA1(a4adff05bb215dd126c47201b36956115b8fed76), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "v.1.03rs")
	ROMX_LOAD( "atm103rs.rom", 0x010000, 0x10000, CRC(cdec1dfb) SHA1(08190807c6b110cb2e657d8e7d0ad18668915375), ROM_BIOS(1))
ROM_END

ROM_START( atmtb2 )
	ROM_REGION(0x020000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1.06", "BIOS v1.06") // joined dump
	ROMX_LOAD( "atm106.rom", 0x010000, 0x10000, CRC(75350b37) SHA1(2afc9994f026645c74b6c4b35bcee2e0bc0d6edc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.06a", "BIOS v1.06 (split)")
	ROMX_LOAD( "atm106-1.rom", 0x010000, 0x4000, CRC(658c98f1) SHA1(1ec694795aa6cac10147e58f38a9db0bdf7ed89b), ROM_BIOS(1))
	ROMX_LOAD( "atm106-2.rom", 0x014000, 0x4000, CRC(8fe367f9) SHA1(56de8fd39061663b9c315b74fd3c31acddae279c), ROM_BIOS(1))
	ROMX_LOAD( "atm106-3.rom", 0x018000, 0x4000, CRC(124ad9e0) SHA1(d07fcdeca892ee80494d286ea9ea5bf3928a1aca), ROM_BIOS(1))
	ROMX_LOAD( "atm106-4.rom", 0x01c000, 0x4000, CRC(f352f2ab) SHA1(6045500ab01be708cef62327e9821b4a358a4673), ROM_BIOS(1))

	ROM_REGION(0x01000, "keyboard", ROMREGION_ERASEFF)
	ROM_LOAD( "rf2ve3.rom",  0x0000, 0x0580, CRC(35e0f9ec) SHA1(adcf14758fab8472cfa0167af7e8326c66416416)) // XT Keyboard
	ROM_LOAD( "rfat710.rom", 0x0600, 0x0680, CRC(03734365) SHA1(6cb6311727fad9bc4ccb18919c3c39b37529b8e6)) // AT Keyboard

	ROM_REGION(0x08000, "charrom", ROMREGION_ERASEFF) // Char gen rom
	ROM_LOAD( "sgen.rom", 0x0000, 0x0800, CRC(1f4387d6) SHA1(93b3774dc8a486643a1bdd48c606b0c84fa0e22b))
ROM_END

ROM_START( atmtb2plus )
	ROM_REGION(0x030000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v1.37")
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
	ROM_LOAD( "rf2ve3.rom",  0x0000, 0x0580, CRC(35e0f9ec) SHA1(adcf14758fab8472cfa0167af7e8326c66416416)) // XT Keyboard
	ROM_LOAD( "rfat710.rom", 0x0600, 0x0680, CRC(03734365) SHA1(6cb6311727fad9bc4ccb18919c3c39b37529b8e6)) // AT Keyboard

	ROM_REGION(0x08000, "charrom", ROMREGION_ERASEFF) // Char gen rom
	ROM_LOAD( "sgen.rom", 0x0000, 0x0800, CRC(1f4387d6) SHA1(93b3774dc8a486643a1bdd48c606b0c84fa0e22b))
ROM_END

/*    YEAR  NAME        PARENT   COMPAT MACHINE     INPUT      CLASS          INIT        COMPANY     FULLNAME                  FLAGS */
COMP( 1991, atm,        spec128, 0,     atm,        spec_plus, atm_state,     empty_init, "MicroART", "ATM-Turbo (ATM-CP)",     MACHINE_NOT_WORKING)
COMP( 1992, atmtb2,     spec128, 0,     atmtb2,     spec_plus, atm_state,     empty_init, "MicroART", "ATM-Turbo 2",            MACHINE_SUPPORTS_SAVE)
COMP( 1993, atmtb2plus, spec128, 0,     atmtb2plus, spec_plus, atm_state,     empty_init, "MicroART", "ATM-Turbo 2+",           MACHINE_SUPPORTS_SAVE)
