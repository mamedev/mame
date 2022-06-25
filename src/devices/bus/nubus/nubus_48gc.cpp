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
  * RAM size configuration.
  * Interlaced modes.

***************************************************************************/

#include "emu.h"
#include "nubus_48gc.h"

#include "layout/generic.h"
#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


#define VRAM_SIZE  (0x20'0000)  // 2 megs, maxed out

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
	TIMER_CALLBACK_MEMBER(vbl_tick);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_crtc();

	uint32_t jmfb_r(offs_t offset, uint32_t mem_mask = ~0);
	uint32_t crtc_r(offs_t offset, uint32_t mem_mask = ~0);
	void jmfb_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void crtc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void ramdac_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void clkgen_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t rgb_unpack(offs_t offset, uint32_t mem_mask = ~0);
	void rgb_pack(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	required_ioport m_monitor;
	memory_view m_vram_view;
	emu_timer *m_timer;

	uint8_t m_monitor_type;

	std::vector<uint32_t> m_vram;
	uint32_t m_vbl_disable, m_toggle;
	uint8_t m_sense;
	uint16_t m_preload;
	uint32_t m_base, m_stride;

	uint8_t m_colors[3], m_count, m_clutoffs, m_mode;

	uint16_t m_hactive, m_hbporch, m_hsync, m_hfporch;
	uint16_t m_vactive, m_vbporch, m_vsync, m_vfporch;
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
	PORT_START("MONITOR")
	PORT_CONFNAME(0x0f, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Macintosh Two-Page Monitor (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Macintosh Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Macintosh RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Macintosh Two-Page Monitor (B&W 21\" 1152\u00d7870)")
	//PORT_CONFSETTING(   0x04, u8"NTSC Monitor") requires implementing interlace modes
	PORT_CONFSETTING(   0x05, u8"Macintosh Portrait Display (640\u00d7870)")
	PORT_CONFSETTING(   0x06, u8"Macintosh Hi-Res Display (12-14\" 640\u00d7480)")
INPUT_PORTS_END


INPUT_PORTS_START( 824gc )
	PORT_START("MONITOR")
	PORT_CONFNAME(0x0f, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Mac 21\" Color Display (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Mac RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")
	//PORT_CONFSETTING(   0x04, u8"NTSC Monitor") requires implementing interlace modes
	PORT_CONFSETTING(   0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")
	PORT_CONFSETTING(   0x0d, u8"Mac 16\" Color Display (832\u00d7624)")
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
	{ false, { 0, 0, 0, 0 } },      //  0: RGB 21"
	{ true,  { 1, 1, 1, 0 } },      //  1: Full-Page (B&W 15")
	{ false, { 2, 2, 0, 2 } },      //  2: RGB 12"
	{ true,  { 3, 3, 1, 2 } },      //  3: Two-Page (B&W 21")
	{ false, { 4, 0, 4, 4 } },      //  4: NTSC Monitor
	{ false, { 5, 1, 5, 4 } },      //  5: RGB 15"
	{ false, { 6, 2, 4, 6 } },      //  6: Hi-Res (12-14")
	{ false, { 6, 0, 0, 6 } },      //  7: Multiple Scan 14"
	{ false, { 6, 0, 4, 6 } },      //  8: Multiple Scan 16"
	{ false, { 6, 2, 0, 6 } },      //  9: Multiple Scan 21"
	{ false, { 7, 0, 0, 0 } },      // 10: PAL Encoder
	{ false, { 7, 1, 1, 0 } },      // 11: NTSC Encoder
	{ false, { 7, 1, 1, 6 } },      // 12: VGA/Super VGA
	{ false, { 7, 2, 5, 2 } },      // 13: RGB 16"
	{ false, { 7, 3, 0, 0 } },      // 14: PAL Monitor
	{ false, { 7, 3, 4, 4 } } };    // 15: RGB 19"


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
	m_monitor(*this, "MONITOR"),
	m_vram_view(*this, "vram"),
	m_timer(nullptr),
	m_vbl_disable(0), m_toggle(0),
	m_count(0), m_clutoffs(0), m_mode(0)
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

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	install_view(
			slotspace, slotspace + VRAM_SIZE - 1,
			m_vram_view);
	m_vram_view[0].install_ram(
			slotspace, slotspace + VRAM_SIZE - 1,
			&m_vram[0]);
	m_vram_view[1].install_readwrite_handler(
			slotspace, slotspace + VRAM_SIZE - 1,
			read32s_delegate(*this, FUNC(jmfb_device::rgb_unpack)), write32s_delegate(*this, FUNC(jmfb_device::rgb_pack)));
	m_vram_view.select(0);

	nubus().install_device(
			slotspace + 0x200000, slotspace + 0x20000f,
			read32s_delegate(*this, FUNC(jmfb_device::jmfb_r)), write32s_delegate(*this, FUNC(jmfb_device::jmfb_w)));
	nubus().install_device(
			slotspace + 0x200100, slotspace + 0x2001ff,
			read32s_delegate(*this, FUNC(jmfb_device::crtc_r)), write32s_delegate(*this, FUNC(jmfb_device::crtc_w)));
	nubus().install_writeonly_device(
			slotspace + 0x200200, slotspace + 0x20020f,
			write32s_delegate(*this, FUNC(jmfb_device::ramdac_w)));
	nubus().install_writeonly_device(
			slotspace + 0x200300, slotspace + 0x20033f,
			write32s_delegate(*this, FUNC(jmfb_device::clkgen_w)));

	m_timer = timer_alloc(FUNC(jmfb_device::vbl_tick), this);

	m_monitor_type = 0;
	m_mode = 0;

	save_item(NAME(m_monitor_type));
	save_item(NAME(m_vram));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_toggle));
	save_item(NAME(m_sense));
	save_item(NAME(m_preload));
	save_item(NAME(m_base));
	save_item(NAME(m_stride));
	save_item(NAME(m_colors));
	save_item(NAME(m_count));
	save_item(NAME(m_clutoffs));
	save_item(NAME(m_mode));
	save_item(NAME(m_hactive));
	save_item(NAME(m_hbporch));
	save_item(NAME(m_hsync));
	save_item(NAME(m_hfporch));
	save_item(NAME(m_vactive));
	save_item(NAME(m_vbporch));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vfporch));
	save_item(NAME(m_multiplier));
	save_item(NAME(m_modulus));
	save_item(NAME(m_pdiv));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jmfb_device::device_reset()
{
	m_vram_view.select(0);

	m_monitor_type = m_monitor->read();
	if (m_monitor_type > std::size(f_monitors))
	{
		throw emu_fatalerror("%s: Invalid monitor selection %d\n", tag(), m_monitor_type);
	}

	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_vbl_disable = 1;
	m_toggle = 0;
	m_sense = 0;
	m_preload = 256 - 8;
	m_base = 0;
	m_stride = 80 / 4;

	m_clutoffs = 0;
	m_count = 0;
	m_mode = 0;

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

	switch (m_mode)
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
			throw emu_fatalerror("%s: Unsupported RAMDAC mode %d\n", tag(), m_mode);
	}

	return 0;
}

void jmfb_device::update_crtc()
{
	int const vtotal = (m_vactive + m_vbporch + m_vsync + m_vfporch) / 2;
	int const height = m_vactive / 2;
	if (vtotal && height && m_multiplier && m_modulus)
	{
		int const divider = 256 - m_preload;
		XTAL const refclk = 20_MHz_XTAL / m_modulus;
		XTAL const vcoout = refclk * m_multiplier;
		XTAL const pixclk = vcoout / (1 << m_pdiv);
		XTAL const dacclk = pixclk / divider;
		LOG("reference clock %d VCO output %d pixel clock %d RAMDAC clock %d\n",
				refclk.value(), vcoout.value(), pixclk.value(), dacclk.value());

		int const htotal = (m_hactive + m_hbporch + m_hsync + m_hfporch + 8) * 32 / divider;
		int const hactive = (m_hactive + 2) * 32 / divider;

		int scale = 0;
		switch (m_mode)
		{
			case 0x0: // 1bpp:
				scale = 0;
				break;
			case 0x4: // 2bpp:
				scale = 1;
				break;
			case 0x8: // 4bpp:
				scale = 2;
				break;
			case 0xc: // 8bpp:
				scale = 3;
				break;
			case 0xd: // 24bpp:
				scale = 5;
				break;
		}
		int const hpixels = htotal >> scale;
		int const width = hactive >> scale;
		LOG("horizontal total %d active %d (mode %x %d/%d)\n",
				htotal, hactive, m_mode, width, hpixels);

		screen().configure(
				hpixels, vtotal,
				rectangle(0, width - 1, 0, height - 1),
				attotime::from_ticks(hpixels * vtotal, pixclk).attoseconds());

		// TODO: determine correct timing for vertical blanking interrupt
		m_timer->adjust(screen().time_until_pos(height - 1, 0));
	}
}

uint32_t jmfb_device::jmfb_r(offs_t offset, uint32_t mem_mask)
{
//  printf("%s jmfb_r: @ %x, mask %08x\n", machine().describe_context().c_str(), offset, mem_mask);

	switch (offset)
	{
		case 0x00/4:
			{
				uint32_t result = f_monitors[m_monitor_type].sense[0];
				if (BIT(m_sense, 2))
					result &= f_monitors[m_monitor_type].sense[1];
				if (BIT(m_sense, 1))
					result &= f_monitors[m_monitor_type].sense[2];
				if (BIT(m_sense, 0))
					result &= f_monitors[m_monitor_type].sense[3];
				return result << 9;
			}
	}

	return 0;
}

uint32_t jmfb_device::crtc_r(offs_t offset, uint32_t mem_mask)
{
//  printf("%s crtc_r: @ %x, mask %08x\n", machine().describe_context().c_str(), offset, mem_mask);

	switch (offset)
	{
		case 0xc0/4:
			m_toggle ^= 0xffffffff;
			return m_toggle;
	}

	return 0;
}

void jmfb_device::jmfb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x00/4: // control
			LOG("%s: %04x to control\n", machine().describe_context(), data);
			if (BIT(data, 7))
			{
				m_vram_view.select(BIT(data, 2)); // packed RGB mode
			}
			m_sense = (data >> 9) & 0x07;
			break;

		case 0x04/4:
			LOG("%s: %02x to preload\n", machine().describe_context(), data);
			m_preload = data & 0xff;
			update_crtc();
			break;

		case 0x08/4: // base
			LOG("%s: %x to base\n", machine().describe_context(), data);
			m_base = data & 0xffff;
			break;

		case 0x0c/4: // stride
			LOG("%s: %x to stride\n", machine().describe_context(), data);
			// this value is in DWORDs for 1-8 bpp and, uhh, strange for 24bpp
			m_stride = data & 0xffff;
			break;
	}
}

void jmfb_device::crtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x0c/4: // active pixel cells - 2
			LOG("%s: %d-2 to active cells\n", machine().describe_context(), data + 2);
			m_hactive = data;
			update_crtc();
			break;

		case 0x10/4: // horizontal back porch
			LOG("%s: %d to horizontal back porch\n", machine().describe_context(), data + 2);
			m_hbporch = data;
			update_crtc();
			break;

		case 0x14/4: // horizontal sync pulse
			LOG("%s: %d-2 to horizontal sync pulse\n", machine().describe_context(), data + 2);
			m_hsync = data;
			update_crtc();
			break;

		case 0x18/4: // horizontal front porch
			LOG("%s: %d-2 to horizontal front porch\n", machine().describe_context(), data + 2);
			m_hfporch = data;
			update_crtc();
			break;

		case 0x24/4: // active lines * 2
			LOG("%s: %d*2 to active lines\n", machine().describe_context(), data / 2);
			m_vactive = data;
			update_crtc();
			break;

		case 0x28/4: // vertical back porch * 2
			LOG("%s: %d*2 to vertical back porch\n", machine().describe_context(), data / 2);
			m_vbporch = data;
			update_crtc();
			break;

		case 0x2c/4: // vertical sync width * 2
			LOG("%s: %d*2 to vertical sync pulse width\n", machine().describe_context(), data / 2);
			m_vsync = data;
			update_crtc();
			break;

		case 0x30/4: // vertical front porch * 2
			LOG("%s: %d*2 to vertical front porch\n", machine().describe_context(), data / 2);
			m_vfporch = data;
			update_crtc();
			break;

		case 0x3c/4: // bit 1 = VBL disable (1=no interrupts)
			m_vbl_disable = (data & 2) ? 1 : 0;
			break;

		case 0x48/4: // write here to clear interrupt
			lower_slot_irq();
			break;
	}
}

void jmfb_device::ramdac_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x00/4: // CLUT address
			LOG("%s: %02x to CLUT address\n", machine().describe_context(), data & 0xff);
			m_clutoffs = data;
			m_count = 0;
			break;

		case 0x04/4: // CLUT data
			m_colors[m_count++] = data & 0xff;
			if (m_count == 3)
			{
				LOG("%s: RAMDAC: color %d = %02x %02x %02x\n", machine().describe_context(), m_clutoffs, m_colors[0], m_colors[1], m_colors[2]);
				set_pen_color(m_clutoffs, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
				m_clutoffs++;
				m_count = 0;
			}
			break;

		case 0x08/4: // mode control
			m_mode = (data >> 1) & 0xf;
			LOG("%s: %02x to RAMDAC mode (mode = %x)\n", machine().describe_context(), data, m_mode);
			update_crtc();
			break;
	}
}

void jmfb_device::clkgen_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x00/4:
		case 0x04/4:
		case 0x08/4:
		case 0x0c/4:
			m_multiplier &= ~(0x0f << ((offset & 3) * 4));
			m_multiplier |= (data & 0x0f) << ((offset & 3) * 4);
			LOG("%s: %d to multiplier\n", machine().describe_context(), m_multiplier);
			update_crtc();
			break;

		case 0x10/4:
		case 0x14/4:
		case 0x18/4:
			m_modulus &= ~(0x0f << ((offset & 3) * 4));
			m_modulus |= (data & 0x0f) << ((offset & 3) * 4);
			LOG("%s: %d to modulus\n", machine().describe_context(), m_modulus);
			if (offset == 0x318/4) // avoid bad intermediate values
				update_crtc();
			break;

		case 0x24/4:
			LOG("%s: 1<<%d to pixel cell divider\n", machine().describe_context(), data);
			m_pdiv = data & 0x0f;
			update_crtc();
			break;
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

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_MDC48,  device_nubus_card_interface, nubus_48gc_device,  "nb_mdc48",  "Apple Macintosh Display Card 4*8")
DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_MDC824, device_nubus_card_interface, nubus_824gc_device, "nb_mdc824", "Apple Macintosh Display Card 8*24")
