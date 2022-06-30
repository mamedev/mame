// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
/***************************************************************************

  Apple Macitosh Display Card 4•8 (model 630-0400)
  Apple Macitosh Display Card 8•24

  Cards have the same framebuffer, CRTC, clock synthesizer, and RAMDAC,
  but use different ROMs and support different monitor profiles.

  The 4•8 shipped with less RAM by default, and as supplied it could not
  support higher bit depths.  We always emulate it as though it has been
  upgraded to maximum supported RAM.

  Monitor type changes take effect on had reset.  The 8•24 defaults to the
  “Page-White Gamma” profile for the 21" and 16" color monitors, which
  affects white balance.  Use the Monitors control panel to switch to the
  “Uncorrected Gamma” profile if you don’t like it.

  TODO:
  * Proper interrupt timing.
  * CRTC status registers.
  * Interlaced modes.
  * 1:2:1 convolution.

***************************************************************************/

#include "emu.h"
#include "nubus_48gc.h"

#include "layout/generic.h"
#include "screen.h"

#include <algorithm>

#define LOG_CRTC    (1U << 1)
#define LOG_RAMDAC  (1U << 2)
#define LOG_CLUT    (1U << 3)
#define LOG_CLKGEN  (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_CRTC | LOG_RAMDAC | LOG_CLKGEN)
//#define LOG_OUTPUT_FUNC osd_printf_error
#include "logmacro.h"

#define LOGCRTC(...)     LOGMASKED(LOG_CRTC, __VA_ARGS__)
#define LOGRAMDAC(...)   LOGMASKED(LOG_RAMDAC, __VA_ARGS__)
#define LOGCLUT(...)     LOGMASKED(LOG_CLUT, __VA_ARGS__)
#define LOGCLKGEN(...)   LOGMASKED(LOG_CLKGEN, __VA_ARGS__)


#define GC48_SCREEN_NAME    "screen"
#define GC48_ROM_REGION     "48gc_rom"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jmfb_device

class jmfb_device :
		public device_t,
		public device_nubus_card_interface,
		public device_video_interface,
		public device_palette_interface
{
protected:
	// construction/destruction
	jmfb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// palette implementation
	uint32_t palette_entries() const override;

private:
	static constexpr offs_t VRAM_MAX = 0x10'0000 / 4; // chip supports 2M but card can only use 1M

	TIMER_CALLBACK_MEMBER(vbl_tick);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_crtc();

	uint32_t jmfb_r(offs_t offset);
	uint32_t crtc_r(offs_t offset);
	uint32_t ramdac_r(offs_t offset);
	void jmfb_w(offs_t offset, uint32_t data);
	void crtc_w(offs_t offset, uint32_t data);
	void ramdac_w(offs_t offset, uint32_t data);
	void clkgen_w(offs_t offset, uint32_t data);

	uint32_t rgb_unpack(offs_t offset, uint32_t mem_mask = ~0);
	void rgb_pack(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	required_ioport m_config;
	memory_view m_vram_view;
	emu_timer *m_timer;

	bool m_configured;
	bool m_clut_addr_read;
	uint8_t m_monitor_type;

	std::unique_ptr<uint32_t []> m_vram;
	uint16_t m_control;
	uint16_t m_preload;
	uint32_t m_base, m_stride;

	uint8_t m_colors[3], m_clutcnt, m_clutoffs;
	uint8_t m_ramdac_mode, m_ramdac_conv;

	uint16_t m_hactive, m_hbporch, m_hsync, m_hfporch;
	uint16_t m_vactive, m_vbporch, m_vsync, m_vfporch;
	uint32_t m_vbl_disable, m_toggle;

	uint16_t m_multiplier;
	uint16_t m_modulus;
	uint8_t m_pdiv;
};

class nubus_48gc_device : public jmfb_device
{
public:
	nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	void mac_48gc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

class nubus_824gc_device : public jmfb_device
{
public:
	nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
};


INPUT_PORTS_START( 48gc )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x0f, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Macintosh Two-Page Monitor (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Macintosh Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Macintosh RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Macintosh Two-Page Monitor (B&W 21\" 1152\u00d7870)")
	//PORT_CONFSETTING(   0x04, u8"NTSC Monitor (512\u00d7384, 640\u00d7480)") requires interlace modes
	PORT_CONFSETTING(   0x05, u8"Macintosh Portrait Display (640\u00d7870)")
	PORT_CONFSETTING(   0x06, u8"Macintosh Hi-Res Display (12-14\" 640\u00d7480)")
	//PORT_CONFSETTING(   0x0b, u8"NTSC Encoder (512\u00d7384, 640\u00d7480)") requires interlace modes
	PORT_CONFNAME(0x10, 0x00, u8"VRAM size")
	PORT_CONFSETTING(   0x00, u8"512 kB (4\u20228)")
	PORT_CONFSETTING(   0x10, u8"1 MB (8\u202224)")
	PORT_CONFNAME(0x20, 0x00, u8"CLUT address read")
	PORT_CONFSETTING(   0x00, "Disable")
	PORT_CONFSETTING(   0x20, "Enable")
INPUT_PORTS_END


INPUT_PORTS_START( 824gc )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x0f, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Mac 21\" Color Display (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Mac RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")
	//PORT_CONFSETTING(   0x04, u8"NTSC Monitor (512\u00d7384, 640\u00d7480)") requires interlace modes
	PORT_CONFSETTING(   0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")
	//PORT_CONFSETTING(   0x0b, u8"NTSC Encoder (512\u00d7384, 640\u00d7480)") requires interlace modes
	PORT_CONFSETTING(   0x0d, u8"Mac 16\" Color Display (832\u00d7624)")
	PORT_CONFNAME(0x10, 0x10, u8"VRAM size")
	PORT_CONFSETTING(   0x00, u8"512 kB (4\u20228)")
	PORT_CONFSETTING(   0x10, u8"1 MB (8\u202224)")
	PORT_CONFNAME(0x20, 0x00, u8"CLUT address read")
	PORT_CONFSETTING(   0x00, "Disable")
	PORT_CONFSETTING(   0x20, "Enable")
INPUT_PORTS_END


ROM_START( gc48 )
	ROM_REGION(0x8000, GC48_ROM_REGION, 0)
	ROM_LOAD( "3410801.bin",  0x0000, 0x8000, CRC(e283da91) SHA1(4ae21d6d7bbaa6fc7aa301bee2b791ed33b1dcf9) )
ROM_END

ROM_START( gc824 )
	ROM_REGION(0x8000, GC48_ROM_REGION, 0)
	ROM_LOAD( "3410868.bin",  0x000000, 0x008000, CRC(57f925fa) SHA1(4d3c0632711b7b31c8e0c5cfdd7ec1904f178336) ) /* Label: "341-0868 // (C)APPLE COMPUTER // INC. 1986-1991 // ALL RIGHTS // RESERVED    W5" */
ROM_END


// TODO: find a better place for this table to live
struct mac_monitor_info { bool mono; unsigned sense[4]; };
mac_monitor_info const f_monitors[] = {
									//                          512×385     640×480     640×870     832×624     1024×768    1152×870                            640×480
									//                          60.15 Hz    66.67 Hz    75.08 Hz    74.55 Hz    74.93 Hz    75.08 Hz    59.94 Hz    55.98 Hz    59.94 Hz    50.00 Hz
									//                                                  portrait                                                                interlaced  interlaced
	{ false, { 0, 0, 0, 0 } },      //  0: RGB 21"                                                                          yes
	{ true,  { 1, 1, 1, 0 } },      //  1: Full-Page (B&W 15")                          yes
	{ false, { 2, 2, 0, 2 } },      //  2: RGB 12"              yes
	{ true,  { 3, 3, 1, 2 } },      //  3: Two-Page (B&W 21")                                                               yes
	{ false, { 4, 0, 4, 4 } },      //  4: NTSC Monitor                                                                                                         yes
	{ false, { 5, 1, 5, 4 } },      //  5: RGB 15"                                      yes
	{ false, { 6, 2, 4, 6 } },      //  6: Hi-Res (12-14")                  yes
	{ false, { 6, 0, 0, 6 } },      //  7: Multiple Scan 14"                yes                     yes
	{ false, { 6, 0, 4, 6 } },      //  8: Multiple Scan 16"                yes                     yes         yes
	{ false, { 6, 2, 0, 6 } },      //  9: Multiple Scan 21"                yes                     yes         yes         yes
	{ false, { 7, 0, 0, 0 } },      // 10: PAL Encoder                                                                                                                      yes
	{ false, { 7, 1, 1, 0 } },      // 11: NTSC Encoder                                                                                                         yes
	{ false, { 7, 1, 1, 6 } },      // 12: VGA/Super VGA                                                                                yes         yes
	{ false, { 7, 2, 5, 2 } },      // 13: RGB 16"                                                  yes
	{ false, { 7, 3, 0, 0 } },      // 14: PAL Monitor                                                                                                                      yes
	{ false, { 7, 3, 4, 4 } } };    // 15: RGB 19"                                                              yes


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void jmfb_device::device_add_mconfig(machine_config &config)
{
	config.set_default_layout(layout_monitors);

	screen_device &screen(SCREEN(config, GC48_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(jmfb_device::screen_update));
	screen.set_raw(20_MHz_XTAL / 21 * 127 / 4, 864, 0, 640, 525, 0, 480);
	//screen.set_raw(20_MHz_XTAL / 19 * 190 / 2, 1'456, 0, 1'152, 915, 0, 870);
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_48gc_device::device_rom_region() const
{
	return ROM_NAME( gc48 );
}

const tiny_rom_entry *nubus_824gc_device::device_rom_region() const
{
	return ROM_NAME( gc824 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nubus_48gc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 48gc );
}

ioport_constructor nubus_824gc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( 824gc );
}

//-------------------------------------------------
//  palette_entries - entries in color palette
//-------------------------------------------------

uint32_t jmfb_device::palette_entries() const
{
	return 256;
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  jmfb_device - constructor
//-------------------------------------------------

jmfb_device::jmfb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_config(*this, "CONFIG"),
	m_vram_view(*this, "vram"),
	m_timer(nullptr)
{
	set_screen(*this, GC48_SCREEN_NAME);
}

nubus_48gc_device::nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jmfb_device(mconfig, NUBUS_MDC48, tag, owner, clock)
{
}

nubus_824gc_device::nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jmfb_device(mconfig, NUBUS_MDC824, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void jmfb_device::device_start()
{
	install_declaration_rom(GC48_ROM_REGION);

	uint32_t const slotspace = get_slotspace();

	LOG("[JMFB %p] slotspace = %x\n", this, slotspace);

	m_vram = std::make_unique<uint32_t []>(VRAM_MAX);
	install_view(slotspace, slotspace + 0x1f'ffff, m_vram_view);

	nubus().install_device(
			slotspace + 0x20'0000, slotspace + 0x20'000f,
			read32sm_delegate(*this, FUNC(jmfb_device::jmfb_r)), write32sm_delegate(*this, FUNC(jmfb_device::jmfb_w)));
	nubus().install_device(
			slotspace + 0x20'0100, slotspace + 0x20'01ff,
			read32sm_delegate(*this, FUNC(jmfb_device::crtc_r)), write32sm_delegate(*this, FUNC(jmfb_device::crtc_w)));
	nubus().install_device(
			slotspace + 0x200200, slotspace + 0x20020f,
			read32sm_delegate(*this, FUNC(jmfb_device::ramdac_r)), write32sm_delegate(*this, FUNC(jmfb_device::ramdac_w)));
	nubus().install_writeonly_device(
			slotspace + 0x200300, slotspace + 0x20033f,
			write32sm_delegate(*this, FUNC(jmfb_device::clkgen_w)));

	m_timer = timer_alloc(FUNC(jmfb_device::vbl_tick), this);

	m_configured = false;
	m_clut_addr_read = false;
	m_monitor_type = 0;

	m_ramdac_mode = 0;
	m_ramdac_conv = 0;

	save_item(NAME(m_monitor_type));
	save_pointer(NAME(m_vram), VRAM_MAX);
	save_item(NAME(m_control));
	save_item(NAME(m_preload));
	save_item(NAME(m_base));
	save_item(NAME(m_stride));
	save_item(NAME(m_colors));
	save_item(NAME(m_clutcnt));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_ramdac_mode));
	save_item(NAME(m_ramdac_conv));
	save_item(NAME(m_hactive));
	save_item(NAME(m_hbporch));
	save_item(NAME(m_hsync));
	save_item(NAME(m_hfporch));
	save_item(NAME(m_vactive));
	save_item(NAME(m_vbporch));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vfporch));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_toggle));
	save_item(NAME(m_multiplier));
	save_item(NAME(m_modulus));
	save_item(NAME(m_pdiv));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jmfb_device::device_reset()
{
	if (!m_configured)
	{
		m_configured = true;
		ioport_value const config = m_config->read();

		m_monitor_type = config & 0x0f;
		if (m_monitor_type > std::size(f_monitors))
		{
			throw emu_fatalerror("%s: Invalid monitor selection %d\n", tag(), m_monitor_type);
		}

		uint32_t const slotspace = get_slotspace();
		uint32_t const vramsize = VRAM_MAX * 4 / (BIT(config, 4) ? 1 : 2);
		m_vram_view[0].install_ram(slotspace, slotspace + vramsize - 1, &m_vram[0]);
		m_vram_view[1].install_readwrite_handler(
				slotspace, slotspace + (vramsize / 3 * 4) - 1,
				read32s_delegate(*this, FUNC(jmfb_device::rgb_unpack)), write32s_delegate(*this, FUNC(jmfb_device::rgb_pack)));
		// TODO: in packed RGB mode, there are one or two bytes that aren't a multiple of 3 - handle them

		m_clut_addr_read = BIT(config, 5);
	}

	m_vram_view.select(0);

	std::fill_n(&m_vram[0], VRAM_MAX, 0);
	m_vbl_disable = 1;
	m_toggle = 0;
	m_control = 0x0002;
	m_preload = 256 - 8;
	m_base = 0;
	m_stride = 80 / 4;

	m_clutoffs = 0;
	m_clutcnt = 0;
	m_ramdac_mode = 0;
	m_ramdac_conv = 0;

	m_hactive = 286;
	m_hbporch = 22;
	m_hsync = 30;
	m_hfporch = 18;

	m_vactive = 1740;
	m_vbporch = 78;
	m_vsync = 6;
	m_vfporch = 6;
	m_multiplier = 190;
	m_modulus = 19;
	m_pdiv = 1;
}

/***************************************************************************

  Apple 4*8 Graphics Card section

***************************************************************************/

TIMER_CALLBACK_MEMBER(jmfb_device::vbl_tick)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	// TODO: determine correct timing for vertical blanking interrupt
	m_timer->adjust(screen().time_until_pos(screen().visible_area().bottom()));
}

uint32_t jmfb_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const trans =
		[mono = f_monitors[m_monitor_type].mono] (rgb_t color)
		{
			return !mono ? color : rgb_t(color.b(), color.b(), color.b());
		};
	auto const screenbase = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_base << 5);
	int const xres = screen.visible_area().right();

	switch (m_ramdac_mode)
	{
	case 0x0: // 1bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			auto const rowbase = screenbase + (y * m_stride * 4);
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x <= xres/8; x++)
			{
				uint8_t const pixels = rowbase[x];

				*scanline++ = trans(pen_color(BIT(pixels, 7)));
				*scanline++ = trans(pen_color(BIT(pixels, 6)));
				*scanline++ = trans(pen_color(BIT(pixels, 5)));
				*scanline++ = trans(pen_color(BIT(pixels, 4)));
				*scanline++ = trans(pen_color(BIT(pixels, 3)));
				*scanline++ = trans(pen_color(BIT(pixels, 2)));
				*scanline++ = trans(pen_color(BIT(pixels, 1)));
				*scanline++ = trans(pen_color(BIT(pixels, 0)));
			}
		}
		break;

	case 0x4: // 2bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			auto const rowbase = screenbase + (y * m_stride * 4);
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x <= xres/4; x++)
			{
				uint8_t const pixels = rowbase[x];

				*scanline++ = trans(pen_color(BIT(pixels, 6, 2)));
				*scanline++ = trans(pen_color(BIT(pixels, 4, 2)));
				*scanline++ = trans(pen_color(BIT(pixels, 2, 2)));
				*scanline++ = trans(pen_color(BIT(pixels, 0, 2)));
			}
		}
		break;

	case 0x8: // 4 bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			auto const rowbase = screenbase + (y * m_stride * 4);
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x <= xres/2; x++)
			{
				uint8_t const pixels = rowbase[x];

				*scanline++ = trans(pen_color(BIT(pixels, 4, 4)));
				*scanline++ = trans(pen_color(BIT(pixels, 0, 4)));
			}
		}
		break;

	case 0xc: // 8 bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			auto const rowbase = screenbase + (y * m_stride * 4);
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x <= xres; x++)
			{
				*scanline++ = trans(pen_color(rowbase[x]));
			}
		}
		break;

	case 0xd: // 24 bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			auto source = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_base << 6) + (y * m_stride * 8);
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x <= xres; x++)
			{
				if (!f_monitors[m_monitor_type].mono)
					*scanline++ = rgb_t(source[0], source[1], source[2]);
				else
					*scanline++ = rgb_t(source[2], source[2], source[2]);
				source += 3;
			}
		}
		break;

	default:
		throw emu_fatalerror("%s: Unsupported RAMDAC mode %d\n", tag(), m_ramdac_mode);
	}

	return 0;
}

void jmfb_device::update_crtc()
{
	int const vtotal = m_vactive + m_vbporch + m_vsync + m_vfporch;
	int const height = m_vactive;
	if (vtotal && height && m_multiplier && m_modulus)
	{
		bool const interlace = BIT(m_control, 4);
		bool const convolution = BIT(m_control, 5);
		int const divider = 256 - m_preload;
		XTAL const refclk = 20_MHz_XTAL / m_modulus;
		XTAL const vcoout = refclk * m_multiplier;
		XTAL const pixclk = vcoout / (1 << m_pdiv);
		XTAL const dacclk = pixclk / divider;
		LOGCLKGEN("reference clock %d VCO output %d pixel clock %d RAMDAC longword load clock %d\n",
				refclk.value(), vcoout.value(), pixclk.value(), dacclk.value());

		int const htotal = m_hactive + m_hbporch + m_hsync + m_hfporch + 8;
		int const hactive = m_hactive + 2;

		int scale = 0;
		switch (m_ramdac_mode)
		{
		case 0x0: // 1bpp - 32 pixels/longword
			scale = 5;
			break;
		case 0x4: // 2bpp - 16 pixels/longword
			scale = 4;
			break;
		case 0x8: // 4bpp - 8 pixels/longword
			scale = 3;
			break;
		case 0xc: // 8bpp - 4 pixels/longword
			scale = 2;
			break;
		case 0xd: // 24bpp - 1 pixel/longword
			scale = 0;
			break;
		}
		int const hpixels = (htotal << scale >> (convolution ? 2 : 0)) / divider;
		int const width = (hactive << scale >> (convolution ? 2 : 0)) / divider;
		LOGCRTC("horizontal total %d active %d (mode %x %d/%d)\n",
				htotal, hactive, m_ramdac_mode, width, hpixels);

		int const frametotal = hpixels * vtotal >> (interlace ? 0 : 1);

		screen().configure(
				hpixels, vtotal >> (interlace ? 0 : 1),
				rectangle(0, width - 1, 0, (height >> (interlace ? 0 : 1)) - 1),
				attotime::from_ticks(frametotal, pixclk).attoseconds());

		// TODO: determine correct timing for vertical blanking interrupt
		m_timer->adjust(screen().time_until_pos(height - 1, 0));
	}
}

uint32_t jmfb_device::jmfb_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00/4:
		{
			uint16_t sense = f_monitors[m_monitor_type].sense[0];
			if (BIT(m_control, 11))
				sense &= f_monitors[m_monitor_type].sense[1];
			if (BIT(m_control, 10))
				sense &= f_monitors[m_monitor_type].sense[2];
			if (BIT(m_control, 9))
				sense &= f_monitors[m_monitor_type].sense[3];
			return (m_control & 0xf1ff) | (sense << 9);
		}

	case 0x04/4: return m_preload;  // RAMDAC longword load clock divider preload
	case 0x08/4: return m_base;     // base - 32-byte increments for indexed, 64-byte increments for direct
	case 0x0c/4: return m_stride;   // stride - 4-byte increments for indexed, 8-byte increments for direct

	default:
		LOG("%s: read unimplemented JMFB register %x/4\n", machine().describe_context(), offset * 4);
		return 0;
	}
}

uint32_t jmfb_device::crtc_r(offs_t offset)
{
//  printf("%s crtc_r: @ %x, mask %08x\n", machine().describe_context().c_str(), offset, mem_mask);

	switch (offset)
	{
	case 0x0c/4: return m_hactive;   // active pixel cells - 2
	case 0x10/4: return m_hbporch;   // horizontal back porch - 2
	case 0x14/4: return m_hsync;     // horizontal sync pulse width - 2
	case 0x18/4: return m_hfporch;   // horizontal front porch - 2

	case 0x24/4: return m_vactive;   // active lines * 2
	case 0x28/4: return m_vbporch;   // vertical back porch * 2
	case 0x2c/4: return m_vsync;     // vertical sync pulse width * 2
	case 0x30/4: return m_vfporch;   // vertical front porch * 2

	case 0xc0/4: // seems to be frame position flags or something?
		m_toggle ^= 0xffffffff;
		return m_toggle;
	}

	return 0;
}

uint32_t jmfb_device::ramdac_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00/4: // CLUT address
		// The firmware explicitly checks whether it can read the value written to the CLUT address
		// What difference this makes is not known
		return m_clut_addr_read ? m_clutoffs : 0;

	default:
		LOGRAMDAC("%s: read unimplemented RAMDAC register %x/4\n", machine().describe_context(), offset * 4);
		return 0;
	}
}

void jmfb_device::jmfb_w(offs_t offset, uint32_t data)
{
	data &= 0xffff; // 16 bits wide, but lane select is ignored and firmware relies on smearing
	switch (offset)
	{
	case 0x00/4: // control
		LOG("%s: %04x to control (sense %x convolution %x interlace %x RGB %x RAM %dk)\n",
				machine().describe_context(), data,
				BIT(data, 9, 3), BIT(data, 5), BIT(data, 4), BIT(data, 2), BIT(data, 0) ? 256 : 128);
		m_control = data;
		m_vram_view.select(BIT(data, 2)); // packed RGB mode
		break;

	case 0x04/4: // RAMDAC longword load clock divider preload
		LOG("%s: 256-%d to preload\n", machine().describe_context(), data & 0xff);
		m_preload = data & 0xff;
		update_crtc();
		break;

	case 0x08/4: // base - 32-byte increments for indexed, 64-byte increments for direct
		LOG("%s: %x to base\n", machine().describe_context(), data);
		m_base = data;
		break;

	case 0x0c/4: // stride - 4-byte increments for indexed, 8-byte increments for direct
		LOG("%s: %x to stride\n", machine().describe_context(), data);
		m_stride = data;
		break;
	}
}

void jmfb_device::crtc_w(offs_t offset, uint32_t data)
{
	data &= 0xffff; // 16 bits wide, but lane select is ignored and firmware relies on smearing
	switch (offset)
	{
	case 0x0c/4: // active pixel cells - 2
		LOGCRTC("%s: %d+2 to active cells\n", machine().describe_context(), data);
		m_hactive = data & 0x0fff;
		update_crtc();
		break;

	case 0x10/4: // horizontal back porch - 2
		LOGCRTC("%s: %d+2 to horizontal back porch\n", machine().describe_context(), data);
		m_hbporch = data & 0x0fff;
		update_crtc();
		break;

	case 0x14/4: // horizontal sync pulse width - 2
		LOGCRTC("%s: %d+2 to horizontal sync pulse width\n", machine().describe_context(), data);
		m_hsync = data & 0x0fff;
		update_crtc();
		break;

	case 0x18/4: // horizontal front porch - 2
		LOGCRTC("%s: %d+2 to horizontal front porch\n", machine().describe_context(), data);
		m_hfporch = data & 0x0fff;
		update_crtc();
		break;

	case 0x24/4: // active lines * 2
		LOGCRTC("%s: %d/2 to active lines\n", machine().describe_context(), data);
		m_vactive = data & 0x0fff;
		update_crtc();
		break;

	case 0x28/4: // vertical back porch * 2
		LOGCRTC("%s: %d/2 to vertical back porch\n", machine().describe_context(), data);
		m_vbporch = data & 0x0fff;
		update_crtc();
		break;

	case 0x2c/4: // vertical sync pulse width * 2
		LOGCRTC("%s: %d/2 to vertical sync pulse width\n", machine().describe_context(), data);
		m_vsync = data & 0x0fff;
		update_crtc();
		break;

	case 0x30/4: // vertical front porch * 2
		LOGCRTC("%s: %d/2 to vertical front porch\n", machine().describe_context(), data);
		m_vfporch = data & 0x0fff;
		update_crtc();
		break;

	case 0x3c/4: // bit 1 = VBL disable (1=no interrupts)
		m_vbl_disable = (data & 2) ? 1 : 0;
		break;

	case 0x48/4: // write here to clear interrupt
		lower_slot_irq();
		break;

	default:
		LOGCRTC("%s: %03x to unimplemented CRTC register %x/4\n", machine().describe_context(), data, offset * 4);
	}
}

void jmfb_device::ramdac_w(offs_t offset, uint32_t data)
{
	data &= 0xff; // 8 bits wide, but lane select is ignored and firmware relies on smearing
	switch (offset)
	{
	case 0x00/4: // CLUT address
		LOGCLUT("%s: %u to RAMDAC color address\n", machine().describe_context(), data);
		m_clutoffs = data;
		m_clutcnt = 0;
		break;

	case 0x04/4: // CLUT data
		m_colors[m_clutcnt++] = data & 0xff;
		if (m_clutcnt == 3)
		{
			LOGCLUT("%s: RAMDAC color %u = %02x %02x %02x\n", machine().describe_context(), m_clutoffs, m_colors[0], m_colors[1], m_colors[2]);
			set_pen_color(m_clutoffs, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
			m_clutoffs++;
			m_clutcnt = 0;
		}
		break;

	case 0x08/4: // RAMDAC mode control
		m_ramdac_mode = (data >> 1) & 0xf;
		m_ramdac_conv = BIT(data, 0);
		LOGRAMDAC("%s: %02x to RAMDAC control (mode %x convolution %x)\n",
				machine().describe_context(), data, m_ramdac_mode, m_ramdac_conv);
		update_crtc();
		break;

	default:
		LOGRAMDAC("%s: %02x to unimplemented RAMDAC register %x/4\n", machine().describe_context(), data, offset * 4);
	}
}

void jmfb_device::clkgen_w(offs_t offset, uint32_t data)
{
	data &= 0x0f; // four bits wide, but lane select is ignored and firmware relies on smearing
	switch (offset)
	{
	case 0x00/4:
	case 0x04/4:
	case 0x08/4:
	case 0x0c/4:
		m_multiplier &= ~(0x0f << ((offset & 3) * 4));
		m_multiplier |= data << ((offset & 3) * 4);
		LOGCLKGEN("%s: %d to multiplier\n", machine().describe_context(), m_multiplier);
		update_crtc();
		break;

	case 0x10/4:
	case 0x14/4:
	case 0x18/4:
		m_modulus &= ~(0x0f << ((offset & 3) * 4));
		m_modulus |= data << ((offset & 3) * 4);
		LOGCLKGEN("%s: %d to reference clock divider\n", machine().describe_context(), m_modulus);
		update_crtc();
		break;

	case 0x24/4:
		m_pdiv = data;
		LOGCLKGEN("%s: 1<<%d to pixel clock divider\n", machine().describe_context(), m_pdiv);
		update_crtc();
		break;

	default:
		LOGCRTC("%s: %x to unimplemented clock synthesiser register %x/4\n", machine().describe_context(), data, offset * 4);
	}
}

uint32_t jmfb_device::rgb_unpack(offs_t offset, uint32_t mem_mask)
{
	auto const color = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (offset * 3);
	return (uint32_t(color[0]) << 16) | (uint32_t(color[1]) << 8) | uint32_t(color[2]);
}

void jmfb_device::rgb_pack(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	auto const color = util::big_endian_cast<uint8_t>(&m_vram[0]) + (offset * 3);
	if (ACCESSING_BITS_16_23)
		color[0] = uint8_t(data >> 16);
	if (ACCESSING_BITS_8_15)
		color[1] = uint8_t(data >> 8);
	if (ACCESSING_BITS_0_7)
		color[2] = uint8_t(data);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_MDC48,  device_nubus_card_interface, nubus_48gc_device,  "nb_mdc48",  "Apple Macintosh Display Card 4/8 (MDC 1.0.1)")
DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_MDC824, device_nubus_card_interface, nubus_824gc_device, "nb_mdc824", "Apple Macintosh Display Card 8/24 (MDC 1.2)")
