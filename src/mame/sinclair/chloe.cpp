// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
	Chloe 280SE
**********************************************************************/

#include "emu.h"

#include "screen_ula.h"
#include "spec128.h"

#include "machine/spi_sdcard.h"
#include "screen.h"
#include "sound/ay8910.h"
#include "speaker.h"


#define LOG_IO    (1U << 1)
#define LOG_MEM   (1U << 2)
#define LOG_WARN  (1U << 3)

#define VERBOSE ( /*LOG_GENERAL | LOG_IO | LOG_MEM |*/ LOG_WARN )
#include "logmacro.h"

#define LOGIO(...)    LOGMASKED(LOG_IO,    __VA_ARGS__)
#define LOGMEM(...)   LOGMASKED(LOG_MEM,   __VA_ARGS__)
#define LOGWARN(...)  LOGMASKED(LOG_WARN,  __VA_ARGS__)


namespace {

#define TIMINGS_PERFECT     0

static const u16 CYCLES_HORIZ = 640;
static const u16 CYCLES_VERT = 480;
static const rectangle SCR_FULL = { 0, 640 - 1, 0, 240 - 1 }; // v-double ?
static const rectangle SCR_256x192 = {  64, 64 + (256 << 1) - 1, 24, 24 + 192 - 1 };

class chloe_state : public spectrum_128_state
{
public:
	chloe_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_128_state(mconfig, type, tag)
		, m_bank_ram(*this, "bank_ram%u", 0U)
		, m_bank0_view(*this, "bank0_view")
		, m_bank1_view(*this, "bank1_view")
		, m_regs_map(*this, "regs_map")
		, m_palette(*this, "palette")
		, m_ula(*this, "ula")
		, m_sdcard(*this, "sdcard")
	{}

	void chloe(machine_config &config);

	INPUT_CHANGED_MEMBER(on_divmmc_nmi);

protected:
	static const u8 BASIC48_ROM = 0x01;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override;

	void map_regs(address_map &map);
	void map_fetch(address_map &map);
	void map_mem(address_map &map);
	void map_io(address_map &map);

	u8 divmmc_neutral_r(offs_t offset);
	u8 divmmc_enable_r(offs_t offset);
	u8 divmmc_disable_r(offs_t offset);
	void port_7ffd_w(u8 data);
	void port_1ffd_w(u8 data);
	void port_f4_w(u8 data);
	void port_ff_w(u8 data);
	u8 spi_data_r();
	void spi_data_w(u8 data);
	void spi_miso_w(u8 data);

	void update_memory();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	TIMER_CALLBACK_MEMBER(spi_clock);

	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_uno_regs;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_bank_array_creator<8> m_bank_ram;
	memory_view m_bank0_view, m_bank1_view;
	required_device<address_map_bank_device> m_regs_map;
	required_device<device_palette_interface> m_palette;
	required_device<screen_ula_plus_device> m_ula;
	required_device<spi_sdcard_sdhc_device> m_sdcard;

	u8 m_port_f4_data;
	u8 m_port_ff_data;
	bool m_core_boot;
	u8 m_reg_selected;
	bool m_divmmc_paged;
	bool m_divmmc_on_delayed;
	u8 m_divmmc_reg;
	u8 m_uno_regs_data[256];
	u8 m_palpen_selected;

	emu_timer *m_spi_clock;
	int m_spi_clock_cycles;
	bool m_spi_clock_state;
	u8 m_spi_mosi_dat;
	u8 m_spi_miso_dat;
};


void chloe_state::update_memory()
{
	m_screen->update_now();
	m_ula->ula_shadow_en_w(BIT(m_port_7ffd_data, 3));

	const bool ext = BIT(m_port_ff_data, 7); // 0 - DOC 7xxxx=28+; 1 - EXT 6xxxx=24+
	m_bank0_view.disable();
	m_bank1_view.disable();
	for (auto i = 0; i < 8; ++i)
	{
		const bool paged = BIT(m_port_f4_data, i);

		u8 pg;
		if (i == 0 && (m_divmmc_paged || !paged))
		{
			m_bank0_view.select(0);
			if (m_divmmc_paged)
				pg = 12;
			else
				pg = 8 + BIT(m_port_7ffd_data, 4);
			m_bank_ram[0]->set_entry(pg << 1);
		}
		else if (i == 1 && (m_divmmc_paged || !paged))
		{
			if (m_divmmc_paged)
			{
				pg = 16;
				m_bank_ram[1]->set_entry((pg << 1) + (m_divmmc_reg & 0x0f));
			}
			else
			{
				m_bank1_view.select(0);
				pg = 8 + BIT(m_port_7ffd_data, 4);
				m_bank_ram[1]->set_entry((pg << 1) + 1);
			}
		}
		else if (paged)
		{
			pg = ((28 - 4 * ext) << 1) + i;
			m_bank_ram[i]->set_entry(pg);
		}
		else
		{
			if (i < 4)
				pg = 5;
			else if (i < 6)
				pg = 2;
			else
				pg = m_port_7ffd_data & 0x07;

			m_bank_ram[i]->set_entry((pg << 1) + (i & 1));
		}
	}
}

u32 chloe_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip256x192 = SCR_256x192;
	clip256x192 &= cliprect;

	screen.priority().fill(0, cliprect);
	m_ula->draw_border(bitmap, cliprect, m_port_fe_data & 0x07);

	const bool flash = u64(screen.frame_number() / m_frame_invert_count) & 1;
	m_ula->draw(screen, bitmap, clip256x192, flash, 0);

	return 0;
}

void chloe_state::port_7ffd_w(u8 data)
{
	if (m_port_7ffd_data & 0x20)
		return;

	m_port_7ffd_data = data;
	update_memory();
}

void chloe_state::port_1ffd_w(u8 data)
{
	if (m_port_7ffd_data & 0x20)
		return;

	m_port_1ffd_data = data;
}

void chloe_state::port_ff_w(u8 data)
{
	m_ula->port_ff_reg_w(data);

	m_port_ff_data = data;
	update_memory();
}

void chloe_state::port_f4_w(u8 data)
{
	m_port_f4_data = data;
	update_memory();
}

u8 chloe_state::spi_data_r()
{
	u8 din = m_spi_miso_dat;
	if (!machine().side_effects_disabled())
		spi_data_w(0xff);

	return din;
}

void chloe_state::spi_data_w(u8 data)
{
	m_spi_mosi_dat = data;
#if TIMINGS_PERFECT
	m_spi_clock_cycles = 8;
	m_spi_clock->adjust(m_maincpu->clocks_to_attotime(1) / 4, 0, m_maincpu->clocks_to_attotime(1) / 4);
#else
	for (u8 m = 0x80; m; m >>= 1)
	{
		m_sdcard->spi_mosi_w(m_spi_mosi_dat & m ? 1 : 0);
		m_sdcard->spi_clock_w(CLEAR_LINE);
		m_sdcard->spi_clock_w(ASSERT_LINE);
	}
#endif
}

void chloe_state::spi_miso_w(u8 data)
{
	m_spi_miso_dat <<= 1;
	m_spi_miso_dat |= data;
}

TIMER_CALLBACK_MEMBER(chloe_state::spi_clock)
{
	if (m_spi_clock_cycles > 0)
	{
		if (m_spi_clock_state)
		{
			m_sdcard->spi_clock_w(ASSERT_LINE);
			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_spi_mosi_dat, m_spi_clock_cycles - 1));
			m_sdcard->spi_clock_w(CLEAR_LINE);
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		m_spi_clock_state = false;
		m_spi_clock->reset();
	}
}


u8 chloe_state::divmmc_neutral_r(offs_t offset)
{
	return m_program.read_byte(offset);
}

u8 chloe_state::divmmc_enable_r(offs_t offset)
{
	// M1
	if (!machine().side_effects_disabled() && !m_divmmc_paged && (offset >= 0x3d00) && (BASIC48_ROM == BIT(m_port_7ffd_data, 4)))
	{
		m_divmmc_paged = 1;
		update_memory();
	}
	const u8 op = m_program.read_byte(offset);
	// after M1
	if (!machine().side_effects_disabled() && !m_divmmc_paged && (!offset || (BASIC48_ROM == BIT(m_port_7ffd_data, 4))))
	{
		m_divmmc_paged = 1;
		update_memory();
	}
	return op;
}

u8 chloe_state::divmmc_disable_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && m_divmmc_paged && (offset >= 0x4000))
	{
		m_divmmc_paged = 0;
		update_memory();
	}
	const u8 op = m_program.read_byte(offset);
	if (!machine().side_effects_disabled() && m_divmmc_paged)
	{
		m_divmmc_paged = 0;
		update_memory();
	}
	return op;
}

INPUT_CHANGED_MEMBER(chloe_state::on_divmmc_nmi)
{
	if (newval & 1)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void chloe_state::map_fetch(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(chloe_state::divmmc_neutral_r));
	map(0x0000, 0x0000).lr8(NAME([this]() { return divmmc_enable_r(0x0000); }));
	map(0x0008, 0x0008).lr8(NAME([this]() { return divmmc_enable_r(0x0008); }));
	map(0x0038, 0x0038).lr8(NAME([this]() { return divmmc_enable_r(0x0038); }));
	map(0x0066, 0x0066).lr8(NAME([this]() { return divmmc_enable_r(0x0066); }));
	map(0x04c6, 0x04c6).lr8(NAME([this]() { return divmmc_enable_r(0x04c6); }));
	map(0x0562, 0x0562).lr8(NAME([this]() { return divmmc_enable_r(0x0562); }));
	map(0x1ff8, 0x1fff).lr8(NAME([this](offs_t offset) { return divmmc_disable_r(0x1ff8 + offset); }));
	map(0x3d00, 0x3dff).lr8(NAME([this](offs_t offset) { return divmmc_enable_r(0x3d00 + offset); }));
	map(0x4000, 0xffff).lr8(NAME([this](offs_t offset) { return divmmc_disable_r(0x4000 + offset); }));
}

void chloe_state::map_mem(address_map &map)
{
	for (auto i = 0; i < 8; i++)
		map(0x0000 + i * 0x2000, 0x1fff + i * 0x2000).bankrw(m_bank_ram[i]);

	map(0x0000, 0x1fff).view(m_bank0_view);
	m_bank0_view[0](0x0000, 0x1fff).nopw();
	map(0x2000, 0x3fff).view(m_bank1_view);
	m_bank1_view[0](0x2000, 0x3fff).nopw();
}

void chloe_state::map_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).select(0xfffe).rw(FUNC(chloe_state::spectrum_ula_r), FUNC(chloe_state::spectrum_ula_w));
	map(0x00ff, 0x00ff).mirror(0xff00).w(FUNC(chloe_state::port_ff_w));
	map(0x7ffd, 0x7ffd).w(FUNC(chloe_state::port_7ffd_w));
	map(0x1ffd, 0x1ffd).w(FUNC(chloe_state::port_1ffd_w));
	map(0x00f4, 0x00f4).mirror(0xff00).w(FUNC(chloe_state::port_f4_w));

	map(0x00e3, 0x00e3).mirror(0xff00).lw8(NAME([this](u8 data) { m_divmmc_reg = data; update_memory(); }));
	map(0x00e7, 0x00e7).mirror(0xff00).lw8(NAME([this](u8 data) { m_sdcard->spi_ss_w(data & 1); }));
	map(0x00eb, 0x00eb).mirror(0xff00).rw(FUNC(chloe_state::spi_data_r), FUNC(chloe_state::spi_data_w));

	map(0x00fd, 0x00fd).lw8(NAME([this](u8 data)
	{
		if (data & 1)
		{
			LOGMEM("Core Boot Off\n");
			m_core_boot = 0;
			reset();
		}
	}));

	map(0xbffd, 0xbffd).w("ay8912", FUNC(ay8910_device::data_w));
	map(0xfffd, 0xfffd).rw("ay8912", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));

	map(0xbf3b, 0xbf3b).lw8(NAME([this](u8 data)
	{
		m_palpen_selected = data;
		if ((data & 0xc0) == 0x40)
			port_ff_w(data);
	}));
	map(0xff3b, 0xff3b).lrw8(NAME([this]()
	{
		rgb_t rgb = m_palette->pen_color(0xc0 | m_palpen_selected);
		return ((rgb.g() >> 5) << 5) | ((rgb.r() >> 5) << 2) | ((rgb.b() >> 6) << 0);
	}), NAME([this](u8 data)
		{
			if (m_palpen_selected < 64)
				m_palette->set_pen_color(0xc0 | m_palpen_selected, rgbexpand<3,3,3>((data << 1) | (((data >> 1) | data) & 1), 3, 6, 0));
			else if ((m_palpen_selected & 0xc0) == 0x40)
				m_ula->ulap_en_w(data & 1);
		}));
	map(0xfc3b, 0xfc3b).lrw8(NAME([this]() { return m_reg_selected; })
		, NAME([this](u8 data) { m_reg_selected = data; }));
	map(0xfd3b, 0xfd3b).lrw8(NAME([this]() { return m_uno_regs.read_byte(m_reg_selected); })
		, NAME([this](u8 data) { m_uno_regs.write_byte(m_reg_selected, data); }));

	map(0xfadf, 0xfadf).lr8(NAME([]() -> u8 { return 0xff; })); // buttons + wheel
	map(0xfbdf, 0xfbdf).lr8(NAME([]() -> u8 { return 0xff; })); // x-axis
	map(0xffdf, 0xffdf).lr8(NAME([]() -> u8 { return 0xff; })); // y-axis

	map(0x001f, 0x001f).mirror(0xff00).lr8(NAME([]() -> u8 { return 0x00; })); // kempston
}

void chloe_state::map_regs(address_map &map)
{
	map(0x00, 0xff).lrw8(NAME([this](offs_t offset)
	{
		if (machine().side_effects_disabled())
			LOGIO("rREG %02x\n", offset);
		return m_uno_regs_data[offset];
	}), NAME([this](offs_t offset, u8 data)
	{
		LOGIO("wREG %02x = %02x\n", offset, data);
		m_uno_regs_data[offset] = data;
	}));

	map(0x01, 0x01).lw8(NAME([this](u8 data)
	{
		m_uno_regs_data[0x01] = data;
		LOGMEM("UnoMapper %d\n", data);
	}));
	map(0x0b, 0x0b).lw8(NAME([this](u8 data)
	{
		m_uno_regs_data[0x0b] = data;
		m_maincpu->set_clock_scale(1 << BIT(data, 6, 2));
	}));
}


INPUT_PORTS_START(chloe)
	PORT_INCLUDE(spec_plus)

	PORT_MODIFY("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("NMI") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, chloe_state, on_divmmc_nmi, 0)
INPUT_PORTS_END


void chloe_state::machine_start()
{
	spectrum_128_state::machine_start();

	m_spi_clock = timer_alloc(FUNC(chloe_state::spi_clock), this);

	m_regs_map->space(AS_PROGRAM).specific(m_uno_regs);
	m_maincpu->space(AS_PROGRAM).specific(m_program);

	for (auto i = 0; i < 8; i++)
	{
		m_bank_ram[i]->configure_entries(0, m_ram->size() / 0x2000, m_ram->pointer(), 0x2000);
		m_bank_ram[i]->configure_entries( 8 << 1, 4, memregion("maincpu")->base(), 0x2000);
		m_bank_ram[i]->configure_entries(12 << 1, 2, memregion("maincpu")->base() + 0x8000, 0x2000);
	}

	m_core_boot = 1;

	//save_item(NAME(...));
}

void chloe_state::machine_reset()
{
	spectrum_128_state::machine_reset();

	m_spi_clock->reset();
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;

	m_port_f4_data = 0;
	m_port_ff_data = 0;
	m_port_7ffd_data = 0;
	m_port_1ffd_data = 0;
	m_divmmc_paged = 0;
	m_divmmc_on_delayed = 0;
	m_divmmc_reg = 0;

	update_memory();
}

static const gfx_layout chloe_charlayout =
{
	8, 8,            // 8 x 8 characters
	256,             // 128 characters
	1,               // 1 bits per pixel
	{ 0 },           // no bitplanes
	{ STEP8(0, 1) }, // x offsets
	{ STEP8(0, 8) }, // y offsets
	8 * 8            // every char takes 8 bytes
};

static GFXDECODE_START(gfx_chloe)
	GFXDECODE_ENTRY("maincpu", 0x2a00, chloe_charlayout, 7, 1)
GFXDECODE_END

void chloe_state::video_start()
{
	spectrum_128_state::video_start();
	m_contention_pattern = {}; // No contention for now

	const u8 *ram = m_ram->pointer();
	m_ula->set_host_ram_ptr(ram);
}


void chloe_state::chloe(machine_config &config)
{
	spectrum_128(config);
	config.device_remove("exp");
	config.device_remove("palette");
	m_ram->set_default_size("512K").set_default_value(0xff);

	Z80(config.replace(), m_maincpu, 28_MHz_XTAL / 8);
	m_maincpu->set_m1_map(&chloe_state::map_fetch);
	m_maincpu->set_memory_map(&chloe_state::map_mem);
	m_maincpu->set_io_map(&chloe_state::map_io);
	m_maincpu->set_vblank_int("screen", FUNC(chloe_state::spec_interrupt));
	m_maincpu->nomreq_cb().set_nop();

	ADDRESS_MAP_BANK(config, m_regs_map).set_map(&chloe_state::map_regs).set_options(ENDIANNESS_LITTLE, 8, 8, 0);

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set(FUNC(chloe_state::spi_miso_w));

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_chloe);
	m_screen->set_raw(28_MHz_XTAL / 2, CYCLES_HORIZ, CYCLES_VERT, SCR_FULL);
	m_screen->set_screen_update(FUNC(chloe_state::screen_update));
	m_screen->set_no_palette();

	PALETTE(config, m_palette, FUNC(chloe_state::spectrum_palette), 256);
	SCREEN_ULA_PLUS(config, m_ula, 0).set_raster_offset(SCR_256x192.left(), SCR_256x192.top()).set_palette(m_palette->device().tag(), 0x000, 0x000);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ay8912_device &ay8912(AY8912(config.replace(), "ay8912", 14_MHz_XTAL / 8));
	ay8912.add_route(0, "lspeaker", 0.50);
	ay8912.add_route(1, "lspeaker", 0.25);
	ay8912.add_route(1, "rspeaker", 0.25);
	ay8912.add_route(2, "rspeaker", 0.50);
}

ROM_START(chloe)
	ROM_REGION(0xc000, "maincpu", ROMREGION_ERASEFF)

	// SE/OS 1.0
	ROM_LOAD( "10_boot.rom", 0x0000, 0x4000, CRC(efbfe46e) SHA1(f5a86b56955661f72fa416e7e644de0b3afe6509))
	ROM_LOAD( "10_basic_42.rom", 0x4000, 0x4000, CRC(c6273eaa) SHA1(f09a26c50f5cfe454e4d56c920cdcc62bc4f90cb))
	ROM_LOAD( "10_dos_31.rom", 0x8000, 0x2000, CRC(67dfef09) SHA1(ba9616494071dfe65834d7db657e0d3bcce0b732))
ROM_END

} // Anonymous namespace

/*    YEAR   NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY               FULLNAME       FLAGS */
COMP( 1999,  chloe,   spec128, 0,      chloe,   chloe,  chloe_state,  empty_init,  "Chloe Corporation",  "Chloe 280SE", MACHINE_NOT_WORKING )
