// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic

/*
 * IBM Monochrome Display and Printer Adapter (MDA)
 * Hercules Graphics Card (HGC)
 * EC1840.0002 (MDA)
 *
 * TODO:
 *  - EC1840 testing
 */

#include "emu.h"
#include "mda.h"

#include "machine/pc_lpt.h"
#include "video/mc6845.h"

#include "dipalette.h"
#include "screen.h"

#define LOG_REGW (1U << 1)
#define LOG_REGR (1U << 2)

//#define VERBOSE (LOG_REGW)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

static constexpr XTAL MDA_CLOCK(16'257'000);

enum mode_mask : u8
{
	MODE_HIRES = 0x01, // high resolution mode
	MODE_GFX   = 0x02, // graphics mode (Hercules only)
	MODE_VIDEO = 0x08, // video enable
	MODE_BLINK = 0x20, // blink enable
	MODE_PG1   = 0x80, // page select (Hercules only)
};

enum status_mask : u8
{
	STATUS_HSYNC = 0x01,
	STATUS_VIDEO = 0x08,
	STATUS_VSYNC = 0x80, // 0=retrace, 1=active (Hercules only)
};

class isa8_mda_device
	: public device_t
	, public device_isa8_card_interface
	, public device_palette_interface
{
public:
	isa8_mda_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: isa8_mda_device(mconfig, ISA8_MDA, tag, owner, clock)
	{
	}

protected:
	isa8_mda_device(machine_config const &mconfig, device_type type, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_isa8_card_interface(mconfig, *this)
		, device_palette_interface(mconfig, *this)
		, m_screen(*this, "screen")
		, m_crtc(*this, "mc6845")
		, m_lpt(*this, "lpt")
		, m_gfx(*this, "gfx")
		, m_font(*this, "font")
		, m_vram(nullptr)
	{
	}

	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface implementation
	virtual u32 palette_entries() const noexcept override { return 4; }

	virtual void isa_pio_map(address_map &map);
	virtual u8 status_r();
	virtual void mode_w(u8 data);

	virtual MC6845_UPDATE_ROW(update_row);

	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<pc_lpt_device> m_lpt;
	required_device<gfxdecode_device> m_gfx;

	required_region_ptr<u8> m_font;
	std::unique_ptr<u8[]> m_vram;

	u8 m_mode;   // mode control register
	u8 m_status; // status register

	// internal state
	u8 m_frame;
};

DEFINE_DEVICE_TYPE_PRIVATE(ISA8_MDA, device_isa8_card_interface, isa8_mda_device, "isa_ibm_mda", "IBM Monochrome Display and Printer Adapter")

static std::tuple<unsigned, unsigned> decode_colors(u8 att, bool const blink)
{
	switch (att)
	{
	case 0x00:
	case 0x08:
	case 0x80:
	case 0x88:
		// no display
		return std::tuple<unsigned, unsigned>(0, 0);
	case 0x70:
	case 0x78:
		// reverse: black on green
		return std::tuple<unsigned, unsigned>(0, 2);
	case 0xf0:
	case 0xf8:
		// reverse: black on bright green (or green if blinking enabled)
		return std::tuple<unsigned, unsigned>(0, blink ? 2 : 3);
	default:
		// default: green or bright green on black
		return std::tuple<unsigned, unsigned>(BIT(att, 3) ? 3 : 2, 0);
	}
}

static gfx_layout const pc_16_charlayout =
{
	8, 16,                  // 8 x 16 characters
	256,                    // 256 characters
	1,                      // 1 bit per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8                     // every char takes 2 x 8 bytes
};

static gfx_layout const pc_8_charlayout =
{
	8, 8,                   // 8 x 8 characters
	512,                    // 512 characters
	1,                      // 1 bit per pixel
	{ 0 },                  // no bitplanes
	// x offsets
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	// y offsets
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                     // every char takes 8 bytes
};

static GFXDECODE_START( gfx_mda )
	GFXDECODE_ENTRY( "font", 0x0000, pc_16_charlayout, 1, 1 )
	GFXDECODE_ENTRY( "font", 0x1000, pc_8_charlayout, 1, 1 )
GFXDECODE_END

ROM_START( mda )
	// IBM 1501981(CGA) and 1501985(MDA) Character rom
	ROM_REGION(0x08100,"font", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x02000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) // "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA"
ROM_END

tiny_rom_entry const *isa8_mda_device::device_rom_region() const
{
	return ROM_NAME( mda );
}

void isa8_mda_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MDA_CLOCK, 882, 0, 720, 370, 0, 350);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, MDA_CLOCK / 9);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(isa8_mda_device::update_row));
	m_crtc->out_hsync_callback().set(
		[this](int state)
		{
			if (state)
				m_status |= STATUS_HSYNC;
			else
				m_status &= ~STATUS_HSYNC;
		});
	m_crtc->out_vsync_callback().set([this](int state) { m_frame += state; });

	GFXDECODE(config, m_gfx, *this, gfx_mda);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set([this](int state) { m_isa->irq7_w(state); });
}

void isa8_mda_device::device_start()
{
	set_isa_device();

	save_item(NAME(m_mode));
	save_item(NAME(m_status));
	save_item(NAME(m_frame));

	// allow derived devices to allocate vram
	if (!m_vram)
	{
		m_vram = std::make_unique<u8[]>(0x1000);

		save_pointer(NAME(m_vram), 0x1000);
	}

	m_isa->install_device(0x3b0, 0x3bf, *this, &isa8_mda_device::isa_pio_map);

	m_isa->install_bank(0xb0000, 0xb0fff, m_vram.get());
	m_isa->install_bank(0xb1000, 0xb1fff, m_vram.get());
	m_isa->install_bank(0xb2000, 0xb2fff, m_vram.get());
	m_isa->install_bank(0xb3000, 0xb3fff, m_vram.get());
	m_isa->install_bank(0xb4000, 0xb4fff, m_vram.get());
	m_isa->install_bank(0xb5000, 0xb5fff, m_vram.get());
	m_isa->install_bank(0xb6000, 0xb6fff, m_vram.get());
	m_isa->install_bank(0xb7000, 0xb7fff, m_vram.get());

	set_pen_color(0, rgb_t{ 0x00,0x00,0x00 }); // black
	set_pen_color(1, rgb_t{ 0x00,0x55,0x00 }); // dark green
	set_pen_color(2, rgb_t{ 0x00,0xaa,0x00 }); // green
	set_pen_color(3, rgb_t{ 0x00,0xff,0x00 }); // bright green
}

void isa8_mda_device::device_reset()
{
	m_mode = 0;
	m_status = 0xf0;
	m_frame = 0;
}

void isa8_mda_device::isa_pio_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0, 0x0).mirror(0x6).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x1, 0x1).mirror(0x6).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8, 0x8).w(FUNC(isa8_mda_device::mode_w));
	map(0xa, 0xa).r(FUNC(isa8_mda_device::status_r));
	map(0xc, 0xe).rw(m_lpt, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write));
}

void isa8_mda_device::mode_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: mode_w 0x%02x\n", machine().describe_context(), data);

	m_mode = data;
}

u8 isa8_mda_device::status_r()
{
	// fake pixel stream
	if (!machine().side_effects_disabled())
		m_status ^= STATUS_VIDEO;

	LOGMASKED(LOG_REGR, "%s: status_r 0x%02x\n", machine().describe_context(), m_status);

	return m_status;
}

MC6845_UPDATE_ROW(isa8_mda_device::update_row)
{
	// check video enabled
	if (!(m_mode & MODE_VIDEO))
		return;

	// text mode, 80x25 characters in 9x14 cells
	bool const blink = m_mode & MODE_BLINK;
	offs_t const font_row = (ra & 0x08) ? 0x800 | (ra & 0x07) : ra;
	u32 *pixel = &bitmap.pix(y);

	for (unsigned i = 0; i < x_count; i++)
	{
		offs_t const offset = ((ma + i) << 1) & 0x0fff;
		u8 const chr = m_vram[offset + 0];
		u8 const att = m_vram[offset + 1];

		bool duplicate = ((chr & 0xe0) == 0xc0); // duplicate 8th pixel for some characters
		u8 data = m_font[chr * 8 + font_row];    // read font data

		auto const [fg, bg] = decode_colors(att, blink);

		// underline
		if ((ra == 12) && (BIT(att, 0, 3) == 1))
		{
			duplicate = true;
			data = 0xff;
		}

		// blink characters every 16 frames
		if ((m_frame & 0x10) && blink && BIT(att, 7))
			data = 0;

		// blink cursor every 8 frames
		if ((i == cursor_x) && (m_frame & 0x08))
		{
			duplicate = true;
			data = 0xff;
		}

		// output first 8 pixels
		*pixel++ = pen_color(BIT(data, 7) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 6) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 5) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 4) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 3) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 2) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 1) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 0) ? fg : bg);

		// output 9th pixel
		if (duplicate)
			*pixel++ = pen_color(BIT(data, 0) ? fg : bg);
		else
			*pixel++ = pen_color(bg);
	}
}

class isa8_hercules_device : public isa8_mda_device
{
public:
	isa8_hercules_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: isa8_mda_device(mconfig, ISA8_HERCULES, tag, owner, clock)
	{
	}

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void isa_pio_map(address_map &map) override;
	virtual void mode_w(u8 data) override;
	void config_w(u8 data);

	virtual MC6845_UPDATE_ROW(update_row) override;

private:
	enum config_mask : u8
	{
		CONFIG_GFXEN = 0x01, // graphics mode enable
		CONFIG_PG1EN = 0x02, // page 1 enable
	};
	u8 m_config; // configuration register
};

DEFINE_DEVICE_TYPE_PRIVATE(ISA8_HERCULES, device_isa8_card_interface, isa8_hercules_device, "isa_hercules", "Hercules Graphics Card")

ROM_START( hercules )
	ROM_REGION(0x1000, "font", 0)
	ROM_SYSTEM_BIOS(0, "cp437", "Code page 437")
	ROMX_LOAD("um2301.bin", 0x0000, 0x1000, CRC(0827bdac) SHA1(15f1aceeee8b31f0d860ff420643e3c7f29b5ffc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mzv", "Mazovia (Polish)") // dumped from a Taiwanese-made card using the SiS 86C22 chip
	ROMX_LOAD("hgc_mzv_2301.bin", 0x0000, 0x1000, CRC(9431b9e0) SHA1(3279dfeed4a0f5daa7b57d455c96eafdcbb6bf41), ROM_BIOS(1))
ROM_END

tiny_rom_entry const *isa8_hercules_device::device_rom_region() const
{
	return ROM_NAME(hercules);
}

static GFXDECODE_START( gfx_hercules )
	GFXDECODE_ENTRY( "font", 0x0000, pc_16_charlayout, 1, 1 )
GFXDECODE_END

void isa8_hercules_device::device_add_mconfig(machine_config &config)
{
	isa8_mda_device::device_add_mconfig(config);

	m_crtc->out_vsync_callback().append(
		[this](int state)
		{
			if (state)
				m_status &= ~STATUS_VSYNC;
			else
				m_status |= STATUS_VSYNC;
		});

	m_gfx->set_info(gfx_hercules);
}

void isa8_hercules_device::device_start()
{
	m_vram = std::make_unique<u8[]>(0x10000);

	isa8_mda_device::device_start();

	save_item(NAME(m_config));

	save_pointer(NAME(m_vram), 0x10000);

	m_isa->install_bank(0xb0000, 0xb7fff, m_vram.get());
}

void isa8_hercules_device::device_reset()
{
	isa8_mda_device::device_reset();

	m_status = 0;
	m_config = 0;
}

void isa8_hercules_device::isa_pio_map(address_map &map)
{
	isa8_mda_device::isa_pio_map(map);

	map(0xf, 0xf).w(FUNC(isa8_hercules_device::config_w));
}

void isa8_hercules_device::mode_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: mode_w 0x%02x\n", machine().describe_context(), data);

	if (!(m_config & CONFIG_GFXEN))
		data &= ~MODE_GFX;

	if (!(m_config & CONFIG_PG1EN))
		data &= ~MODE_PG1;

	m_mode = data;

	m_crtc->set_unscaled_clock(MDA_CLOCK / ((m_mode & MODE_GFX) ? 16 : 9));
	m_crtc->set_hpixels_per_column((m_mode & MODE_GFX) ? 16 : 9);
}

void isa8_hercules_device::config_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: config_w 0x%02x\n", machine().describe_context(), data);

	// TODO: does clearing GFXEN force text mode?

	if ((data ^ m_config) & CONFIG_PG1EN)
	{
		if (data & CONFIG_PG1EN)
			m_isa->install_bank(0xb8000, 0xbffff, &m_vram[0x8000]);
		else
			m_isa->unmap_bank(0xb8000, 0xbffff);
	}

	m_config = data;
}

MC6845_UPDATE_ROW(isa8_hercules_device::update_row)
{
	// check video enabled
	if (!(m_mode & MODE_VIDEO))
		return;

	if (m_mode & MODE_GFX)
	{
		// graphics, 720x348 pixels, 2 x 32KiB pages
		u8 const *vram = &m_vram[((m_mode & MODE_PG1) ? 0x8000 : 0x0000) | (((ra & 0x03) << 13) + ma * 2)];
		u32 *pixel = &bitmap.pix(y);

		for (int i = 0; i < x_count; i++)
		{
			u8 data = *vram++;

			*pixel++ = pen_color(BIT(data, 7) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 6) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 5) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 4) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 3) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 2) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 1) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 0) ? 2 : 0);

			data = *vram++;

			*pixel++ = pen_color(BIT(data, 7) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 6) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 5) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 4) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 3) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 2) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 1) ? 2 : 0);
			*pixel++ = pen_color(BIT(data, 0) ? 2 : 0);
		}
	}
	else
		isa8_mda_device::update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
}

class isa8_ec1840_0002_device
	: public device_t
	, public device_isa8_card_interface
	, public device_palette_interface
{
public:
	isa8_ec1840_0002_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA8_EC1840_0002, tag, owner, clock)
		, device_isa8_card_interface(mconfig, *this)
		, device_palette_interface(mconfig, *this)
		, m_screen(*this, "screen")
		, m_crtc(*this, "mc6845")
		, m_vram(nullptr)
		, m_font(nullptr)
	{
	}

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface implementation
	virtual u32 palette_entries() const noexcept override { return 4; }

	void isa_pio_map(address_map &map);
	u8 status_r();
	void mode_w(u8 data);

	MC6845_UPDATE_ROW(update_row);

private:
	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;

	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_font;

	u8 m_mode;   // mode control register
	u8 m_status; // status register

	// internal state
	u8 m_frame;
};

DEFINE_DEVICE_TYPE_PRIVATE(ISA8_EC1840_0002, device_isa8_card_interface, isa8_ec1840_0002_device, "ec1840_0002", "EC1840.0002 (MDA)")

void isa8_ec1840_0002_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MDA_CLOCK, 792, 0, 640, 370, 0, 350);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, MDA_CLOCK / 8);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(isa8_ec1840_0002_device::update_row));
	m_crtc->out_hsync_callback().set(
		[this](int state)
		{
			if (state)
				m_status |= STATUS_HSYNC;
			else
				m_status &= ~STATUS_HSYNC;
		});
	m_crtc->out_vsync_callback().set([this](int state) { m_frame += state; });
}

void isa8_ec1840_0002_device::device_start()
{
	set_isa_device();

	m_vram = std::make_unique<u8[]>(0x1000);
	m_font = std::make_unique<u8[]>(0x2000);

	save_item(NAME(m_mode));
	save_item(NAME(m_status));
	save_item(NAME(m_frame));

	save_pointer(NAME(m_vram), 0x1000);
	save_pointer(NAME(m_font), 0x2000);

	m_isa->install_device(0x3b0, 0x3bf, *this, &isa8_ec1840_0002_device::isa_pio_map);

	m_isa->install_bank(0xb0000, 0xb0fff, m_vram.get());
	m_isa->install_bank(0xb1000, 0xb1fff, m_vram.get());
	m_isa->install_bank(0xb2000, 0xb2fff, m_vram.get());
	m_isa->install_bank(0xb3000, 0xb3fff, m_vram.get());
	m_isa->install_bank(0xb4000, 0xb4fff, m_vram.get());
	m_isa->install_bank(0xb5000, 0xb5fff, m_vram.get());
	m_isa->install_bank(0xb6000, 0xb6fff, m_vram.get());
	m_isa->install_bank(0xb7000, 0xb7fff, m_vram.get());

	m_isa->install_bank(0xdc000, 0xddfff, m_font.get());
	m_isa->install_bank(0xde000, 0xdffff, m_font.get());

	set_pen_color(0, rgb_t{ 0x00,0x00,0x00 }); // black
	set_pen_color(1, rgb_t{ 0x00,0x55,0x00 }); // dark green
	set_pen_color(2, rgb_t{ 0x00,0xaa,0x00 }); // green
	set_pen_color(3, rgb_t{ 0x00,0xff,0x00 }); // bright green
}

void isa8_ec1840_0002_device::device_reset()
{
	m_mode = 0;
	m_status = 0xf0;
	m_frame = 0;
}

void isa8_ec1840_0002_device::isa_pio_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0, 0x0).mirror(0x6).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x1, 0x1).mirror(0x6).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x8, 0x8).w(FUNC(isa8_ec1840_0002_device::mode_w));
	map(0xa, 0xa).r(FUNC(isa8_ec1840_0002_device::status_r));
}

void isa8_ec1840_0002_device::mode_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: mode_w 0x%02x\n", machine().describe_context(), data);

	m_mode = data;
}

u8 isa8_ec1840_0002_device::status_r()
{
	// fake pixel stream
	if (!machine().side_effects_disabled())
		m_status ^= STATUS_VIDEO;

	LOGMASKED(LOG_REGR, "%s: status_r 0x%02x\n", machine().describe_context(), m_status);

	return m_status;
}

MC6845_UPDATE_ROW(isa8_ec1840_0002_device::update_row)
{
	// check video enabled
	if (!(m_mode & MODE_VIDEO))
		return;

	// text mode, 80x25 characters in 8x14 cells
	bool const blink = m_mode & MODE_BLINK;
	u32 *pixel = &bitmap.pix(y);

	for (unsigned i = 0; i < x_count; i++)
	{
		offs_t const offset = ((ma + i) << 1) & 0x0fff;
		u8 const chr = m_vram[offset + 0];
		u8 const att = m_vram[offset + 1];

		u8 data = m_font[(ra + chr * 16) << 1];

		auto const [fg, bg] = decode_colors(att, blink);

		// underline
		if ((ra == 12) && (BIT(att, 0, 3) == 1))
			data = 0xff;

		// blink characters every 16 frames
		if ((m_frame & 0x10) && blink && BIT(att, 7))
			data = 0;

		// blink cursor every 8 frames
		if ((i == cursor_x) && (m_frame & 0x08))
			data = 0xff;

		*pixel++ = pen_color(BIT(data, 7) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 6) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 5) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 4) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 3) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 2) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 1) ? fg : bg);
		*pixel++ = pen_color(BIT(data, 0) ? fg : bg);
	}
}
