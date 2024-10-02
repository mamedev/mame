// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
/***************************************************************************

  Apple Macintosh Display Card 4•8 (MDC 1.0.1, model 630-0400)
  Apple Macintosh Display Card 8•24 (MDC 1.2)

  Cards have the same framebuffer, CRTC, clock synthesizer, and RAMDAC,
  but use different ROMs and support different monitor profiles.

  When 1 MB VRAM is installed, 24-bit direct color is available at up
  to 640×480 resolution, 8-bit indexed color is available at all
  supported resolutions, and 1:2:1 convolution is used for interlaced
  modes with indexed color.  When 512 kB VRAM is installed, 8-bit
  indexed color is available at up to 640×480 resolution, 4-bit indexed
  color is available at all supported resolutions, and 1:2:1
  convolution will not be used.

  Monitor type changes take effect on had reset.  MDC 1.2 defaults to
  the “Page-White Gamma” profile for the 21" and 16" color monitors,
  which affects white balance.  Use the Monitors control panel to switch
  to the “Uncorrected Gamma” profile if you don’t like it.

  System 6 will hang on start if 1 MB VRAM is installed, a PAL monitor
  or encoder is connected, and the card has not been set up.  To avoid
  this, start the system with a different monitor connected, use the
  Monitors control panel to select a color mode and resolution, and shut
  down the system cleanly.  After this, a PAL monitor or encoder can be
  connected.  System 7 does not suffer from this issue.

  The CRTC counts half-lines vertically, which doesn’t integrate very
  well with MAME’s screen device.  The screen device also lacks any
  support for interlaced modes.  To make interlaced modes usable, a few
  simplifying assumptions are made:
  * Assume the framebuffer controller’s interlaced mode will be set when
    when CRTC is configured for interlaced modes (and vice versa).
  * Assume NTSC-like structure where frame starts with even field where
    vertical sync coincides with horizontal sync.
  * Assume the framebuffer controller and RAMDAC are configured for the
    same pixel format.

  TODO:
  * Precise interrupt timing.
  * Precise timing for odd field flag.
  * Remaining CRTC registers.
  * Interlaced modes.

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// palette implementation
	uint32_t palette_entries() const noexcept override;

private:
	static constexpr offs_t VRAM_MAX = 0x10'0000 / 4; // chip supports 2M but card can only use 1M

	TIMER_CALLBACK_MEMBER(vbl_start);
	void set_vbl_timer();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template <uint8_t Mode>
	void update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template <uint8_t Mode, bool Convolution, bool Mono>
	void update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
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

	bool ctrl_sense2() const { return BIT(m_control, 11); }
	bool ctrl_sense1() const { return BIT(m_control, 10); }
	bool ctrl_sense0() const { return BIT(m_control, 9); }
	bool ctrl_transfer() const { return BIT(m_control, 6); }
	bool ctrl_convolution() const { return BIT(m_control, 5); }
	bool ctrl_interlace() const { return BIT(m_control, 4); }
	bool ctrl_rgb() const { return BIT(m_control, 2); }

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

	uint16_t m_hhalf, m_hactive, m_hbporch, m_hsync, m_hfporch;
	uint16_t m_vactive, m_vbporch, m_vsync, m_vfporch;
	uint32_t m_vbl_disable;
	uint16_t m_halfline_pixels;

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
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void mac_48gc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
};

class nubus_824gc_device : public jmfb_device
{
public:
	nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


INPUT_PORTS_START( 48gc )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x0f, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Macintosh Two-Page Monitor (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Macintosh Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Macintosh RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Macintosh Two-Page Monitor (B&W 21\" 1152\u00d7870)")
	PORT_CONFSETTING(   0x04, u8"NTSC Monitor (512\u00d7384, 640\u00d7480)") // requires interlace modes
	PORT_CONFSETTING(   0x05, u8"Macintosh Portrait Display (640\u00d7870)")
	PORT_CONFSETTING(   0x06, u8"Macintosh Hi-Res Display (12-14\" 640\u00d7480)")
	PORT_CONFSETTING(   0x0b, u8"NTSC Encoder (512\u00d7384, 640\u00d7480)") // requires interlace modes
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
	PORT_CONFSETTING(   0x04, u8"NTSC Monitor (512\u00d7384, 640\u00d7480)") // requires interlace modes
	PORT_CONFSETTING(   0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")
	PORT_CONFSETTING(   0x0a, u8"PAL Encoder (640\u00d7480, 768\u00d7576)") // requires interlace modes
	PORT_CONFSETTING(   0x0b, u8"NTSC Encoder (512\u00d7384, 640\u00d7480)") // requires interlace modes
	PORT_CONFSETTING(   0x0d, u8"Mac 16\" Color Display (832\u00d7624)")
	PORT_CONFSETTING(   0x1e, u8"PAL Monitor (640\u00d7480, 768\u00d7576)") // requires interlace modes
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

uint32_t jmfb_device::palette_entries() const noexcept
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

	m_timer = timer_alloc(FUNC(jmfb_device::vbl_start), this);

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
	save_item(NAME(m_halfline_pixels));
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
		switch (vramsize % 3)
		{
		case 0:
			break;
		case 1:
			m_vram_view[1].install_readwrite_handler(
					slotspace + (vramsize / 3 * 4), slotspace + (vramsize / 3 * 4) + 3,
					read32s_delegate(
						*this,
						NAME(([this, vramsize] (offs_t offset, uint32_t mem_mask) -> uint32_t
						{
							auto const color = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (vramsize - 1);
							return uint32_t(color[0]) << 16;
						}))),
					write32s_delegate(
						*this,
						NAME(([this, vramsize] (offs_t offset, uint32_t data, uint32_t mem_mask)
						{
							auto const color = util::big_endian_cast<uint8_t>(&m_vram[0]) + (vramsize - 1);
							if (ACCESSING_BITS_16_23)
								color[0] = uint8_t(data >> 16);
						}))));
			break;
		case 2:
			m_vram_view[1].install_readwrite_handler(
					slotspace + (vramsize / 3 * 4), slotspace + (vramsize / 3 * 4) + 3,
					read32s_delegate(
						*this,
						NAME(([this, vramsize] (offs_t offset, uint32_t mem_mask) -> uint32_t
						{
							auto const color = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (vramsize - 2);
							return (uint32_t(color[0]) << 16) | (uint32_t(color[1]) << 8);
						}))),
					write32s_delegate(
						*this,
						NAME(([this, vramsize] (offs_t offset, uint32_t data, uint32_t mem_mask)
						{
							auto const color = util::big_endian_cast<uint8_t>(&m_vram[0]) + (vramsize - 2);
							if (ACCESSING_BITS_16_23)
								color[0] = uint8_t(data >> 16);
							if (ACCESSING_BITS_8_15)
								color[1] = uint8_t(data >> 8);
						}))));
			break;
		}

		m_clut_addr_read = BIT(config, 5);
	}

	m_vram_view.select(0);

	std::fill_n(&m_vram[0], VRAM_MAX, 0);
	m_vbl_disable = 1;
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
	m_halfline_pixels = 576;

	m_multiplier = 190;
	m_modulus = 19;
	m_pdiv = 1;
}

/***************************************************************************

  Apple 4*8 Graphics Card section

***************************************************************************/

TIMER_CALLBACK_MEMBER(jmfb_device::vbl_start)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	set_vbl_timer();
}

void jmfb_device::set_vbl_timer()
{
	// TODO: precise VBL timing in interlaced modes if half-line split is offset (likely doesn't matter in practice)
	rectangle const &visarea = screen().visible_area();
	int const height = screen().height();
	if ((visarea.bottom() + 1) < height)
	{
		m_timer->adjust(screen().time_until_pos(visarea.bottom() + 1));
	}
	else
	{
		m_timer->adjust(screen().time_until_pos(visarea.bottom() + 1 - height));
	}
}

uint32_t jmfb_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!ctrl_transfer())
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	switch (m_ramdac_mode)
	{
	case 0x0: // 1bpp
		update_screen<0x0>(screen, bitmap, cliprect);
		break;

	case 0x4: // 2bpp
		update_screen<0x4>(screen, bitmap, cliprect);
		break;

	case 0x8: // 4 bpp
		update_screen<0x8>(screen, bitmap, cliprect);
		break;

	case 0xc: // 8 bpp
		update_screen<0xc>(screen, bitmap, cliprect);
		break;

	case 0xd: // 24 bpp
		update_screen<0xd>(screen, bitmap, cliprect);
		break;

	default:
		throw emu_fatalerror("%s: Unsupported RAMDAC mode %d\n", tag(), m_ramdac_mode);
	}

	return 0;
}

template <uint8_t Mode>
void jmfb_device::update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!ctrl_convolution())
	{
		if (!f_monitors[m_monitor_type].mono)
			update_screen<Mode, false, false>(screen, bitmap, cliprect);
		else
			update_screen<Mode, false, true>(screen, bitmap, cliprect);
	}
	else
	{
		if (!f_monitors[m_monitor_type].mono)
			update_screen<Mode, true, false>(screen, bitmap, cliprect);
		else
			update_screen<Mode, true, true>(screen, bitmap, cliprect);
	}
}

template <uint8_t Mode, bool Convolution, bool Mono>
void jmfb_device::update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO: interlaced mode
	auto const baseoffset = m_base << (((0xd == Mode) ? 6 : 5) + (Convolution ? 1 : 0));
	auto const screenbase = util::big_endian_cast<uint8_t const>(&m_vram[0]) + baseoffset;
	auto const stride = m_stride << ((0xd == Mode) ? 3 : 2);

	auto const trans =
		[] (rgb_t color)
		{
			return !Mono ? color : rgb_t(color.b(), color.b(), color.b());
		};

	rectangle const &visarea = screen.visible_area();
	int const xstart = visarea.left();
	int xres = visarea.width();
	if (0x0 == Mode)
	{
		xres /= 8;
	}
	else if (0x4 == Mode)
	{
		xres /= 4;
	}
	else if (0x8 == Mode)
	{
		xres /= 2;
	}

	int y = cliprect.top();
	while (y < visarea.top())
	{
		std::fill_n(&bitmap.pix(y++, xstart), visarea.width(), 0);
	}
	while ((y <= cliprect.bottom()) && (y <= visarea.bottom()))
	{
		auto source = screenbase + ((y - visarea.top()) * stride);
		uint32_t *scanline = &bitmap.pix(y, xstart);
		y++;
		for (int x = 0; x <= xres; x++)
		{
			if (0xd == Mode) // 24bpp
			{
				if (!f_monitors[m_monitor_type].mono)
					*scanline++ = rgb_t(source[0], source[1], source[2]);
				else
					*scanline++ = rgb_t(source[2], source[2], source[2]);
				source += 3;
			}
			else if (!Convolution)
			{
				uint8_t const pixels = *source++;

				if (0x0 == Mode) // 1bpp
				{
					*scanline++ = trans(pen_color(BIT(pixels, 7)));
					*scanline++ = trans(pen_color(BIT(pixels, 6)));
					*scanline++ = trans(pen_color(BIT(pixels, 5)));
					*scanline++ = trans(pen_color(BIT(pixels, 4)));
					*scanline++ = trans(pen_color(BIT(pixels, 3)));
					*scanline++ = trans(pen_color(BIT(pixels, 2)));
					*scanline++ = trans(pen_color(BIT(pixels, 1)));
					*scanline++ = trans(pen_color(BIT(pixels, 0)));
				}
				else if (0x4 == Mode) // 2bpp
				{
					*scanline++ = trans(pen_color(BIT(pixels, 6, 2)));
					*scanline++ = trans(pen_color(BIT(pixels, 4, 2)));
					*scanline++ = trans(pen_color(BIT(pixels, 2, 2)));
					*scanline++ = trans(pen_color(BIT(pixels, 0, 2)));
				}
				else if (0x8 == Mode) // 4bpp
				{
					*scanline++ = trans(pen_color(BIT(pixels, 4, 4)));
					*scanline++ = trans(pen_color(BIT(pixels, 0, 4)));
				}
				else if (0xc == Mode) // 8bpp
				{
					*scanline++ = trans(pen_color(pixels));
				}
			}
			else
			{
				uint8_t const pixabove = source[0 * stride];
				uint8_t const pixels = source[1 * stride];
				uint8_t const pixbelow = source[2 * stride];
				source++;

				if (0x0 == Mode) // 1bpp
				{
					for (int p = 7; p >= 0; p--)
					{
						rgb_t const a = pen_color(BIT(pixabove, p));
						rgb_t const b = pen_color(BIT(pixels, p));
						rgb_t const c = pen_color(BIT(pixbelow, p));
						*scanline++ = trans(
								rgb_t(
									(a.r() + (uint16_t(b.r()) << 1) + c.r() + 2) >> 2,
									(a.g() + (uint16_t(b.g()) << 1) + c.g() + 2) >> 2,
									(a.b() + (uint16_t(b.b()) << 1) + c.b() + 2) >> 2));
					}
				}
				else if (0x4 == Mode) // 2bpp
				{
					for (int p = 6; p >= 0; p -= 2)
					{
						rgb_t const a = pen_color(BIT(pixabove, p, 2));
						rgb_t const b = pen_color(BIT(pixels, p, 2));
						rgb_t const c = pen_color(BIT(pixbelow, p, 2));
						*scanline++ = trans(
								rgb_t(
									(a.r() + (uint16_t(b.r()) << 1) + c.r() + 2) >> 2,
									(a.g() + (uint16_t(b.g()) << 1) + c.g() + 2) >> 2,
									(a.b() + (uint16_t(b.b()) << 1) + c.b() + 2) >> 2));
					}
				}
				else if (0x8 == Mode) // 4bpp
				{
					for (int p = 4; p >= 0; p -= 4)
					{
						rgb_t const a = pen_color(BIT(pixabove, p, 4));
						rgb_t const b = pen_color(BIT(pixels, p, 4));
						rgb_t const c = pen_color(BIT(pixbelow, p, 4));
						*scanline++ = trans(
								rgb_t(
									(a.r() + (uint16_t(b.r()) << 1) + c.r() + 2) >> 2,
									(a.g() + (uint16_t(b.g()) << 1) + c.g() + 2) >> 2,
									(a.b() + (uint16_t(b.b()) << 1) + c.b() + 2) >> 2));
					}
				}
				else if (0xc == Mode) // 8bpp
				{
					rgb_t const a = pen_color(pixabove);
					rgb_t const b = pen_color(pixels);
					rgb_t const c = pen_color(pixbelow);
					*scanline++ = trans(
							rgb_t(
								(a.r() + (uint16_t(b.r()) << 1) + c.r() + 2) >> 2,
								(a.g() + (uint16_t(b.g()) << 1) + c.g() + 2) >> 2,
								(a.b() + (uint16_t(b.b()) << 1) + c.b() + 2) >> 2));
				}
			}
		}
	}
	while (y <= cliprect.bottom())
	{
		std::fill_n(&bitmap.pix(y++, xstart), visarea.width(), 0);
	}
}

void jmfb_device::update_crtc()
{
	// Vertical values are always in half-lines.
	// In progressive modes, we give the screen device numbers of full lines.
	// In interlaced modes we let the screen device base horizontal timing on half-lines.
	int const vtotal = m_vactive + m_vbporch + m_vsync + m_vfporch;
	if (vtotal && m_vactive && m_multiplier && m_modulus)
	{
		bool const interlace = vtotal % 2;
		bool const convolution = ctrl_convolution();

		int const vstart = m_vsync + m_vbporch;
		int const vlines = vtotal >> (interlace ? 0 : 1);
		int const top = vstart >> (interlace ? 0 : 1);
		int const height = m_vactive >> (interlace ? 0 : 1);

		int const divider = 256 - m_preload;
		XTAL const refclk = 20_MHz_XTAL / m_modulus;
		XTAL const vcoout = refclk * m_multiplier;
		XTAL const pixclk = vcoout / (1 << m_pdiv);
		XTAL const dacclk = pixclk / divider;
		LOGCLKGEN("reference clock %d VCO output %d pixel clock %d RAMDAC longword load clock %d\n",
				refclk.value(), vcoout.value(), pixclk.value(), dacclk.value());

		int const htotal = m_hactive + m_hbporch + m_hsync + m_hfporch + 8;
		int const hstart = m_hbporch + m_hsync + 4;
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
		int const left = (hstart << scale >> (convolution ? 2 : 0)) / divider;
		int const width = (hactive << scale >> (convolution ? 2 : 0)) / divider;
		m_halfline_pixels = (m_hhalf << scale >> (convolution ? 2 : 0)) / divider;
		LOGCRTC("vertical total %d start %d active %d horizontal total %d start %d active %d (mode %x %d/%d)\n",
				vtotal, vstart, m_vactive, htotal, hstart, hactive, m_ramdac_mode, width, hpixels);

		int const frametotal = hpixels * vlines;

		screen().configure(
				hpixels, vlines,
				rectangle(left, left + width - 1, top, top + height - 1),
				attotime::from_ticks(frametotal << (convolution ? 2 : 0) >> (interlace ? 1 : 0), pixclk).attoseconds());

		set_vbl_timer();
	}
}

uint32_t jmfb_device::jmfb_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00/4:
		{
			uint16_t sense = f_monitors[m_monitor_type].sense[0];
			if (ctrl_sense2())
				sense &= f_monitors[m_monitor_type].sense[1];
			if (ctrl_sense1())
				sense &= f_monitors[m_monitor_type].sense[2];
			if (ctrl_sense0())
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
	case 0x08/4: return m_hhalf;     // half line length from start of active area
	case 0x0c/4: return m_hactive;   // active pixel cells - 2
	case 0x10/4: return m_hbporch;   // horizontal back porch - 2
	case 0x14/4: return m_hsync;     // horizontal sync pulse width - 2
	case 0x18/4: return m_hfporch;   // horizontal front porch - 2

	case 0x24/4: return m_vactive;   // active lines * 2
	case 0x28/4: return m_vbporch;   // vertical back porch * 2
	case 0x2c/4: return m_vsync;     // vertical sync pulse width * 2
	case 0x30/4: return m_vfporch;   // vertical front porch * 2

	case 0xc0/4:                     // beam position/status
		{
			// TODO: remaining two flags - interrupt status?
			rectangle const &visarea = screen().visible_area();
			int const vpos = screen().vpos();
			int const hpos = screen().hpos();
			int const hsplit = visarea.left() + m_halfline_pixels;
			int const vtotal = m_vactive + m_vbporch + m_vsync + m_vfporch;
			uint8_t result = 0x0f;

			int halfline;
			int truehpos;
			if (vtotal % 2)
			{
				// screen device configured to count half-lines vertically
				int const oddfield = screen().frame_number() % 2;
				int const oddhalfline = vpos % 2;
				truehpos = (hpos + ((oddfield != oddhalfline) ? screen().width() : 0)) >> 1;

				// adjust half line count in case half line position is offset
				if ((oddfield == oddhalfline) && (truehpos >= hsplit))
				{
					halfline = vpos + 1;
					if (halfline >= vtotal)
					{
						halfline -= vtotal;
					}
				}
				else if ((oddfield != oddhalfline) && (truehpos < hsplit))
				{
					halfline = vpos - 1;
					if (halfline < 0)
					{
						halfline += vtotal;
					}
				}
				else
				{
					halfline = vpos;
				}

				// set odd field flag
				if (oddfield)
				{
					// TODO: confirm where this flips - on sync or on front porch?
					result |= 0x10; // odd field
				}
			}
			else
			{
				// vertical counts are in half-lines but screen device uses full lines
				halfline = (vpos << 1) | ((hpos >= hsplit) ? 1 : 0);
				truehpos = hpos;
			}

			// set horizontal beam position flag
			if ((truehpos < visarea.left()) || (truehpos > visarea.right()))
			{
				result |= 0x20; // horizontal blanking
			}

			// set appropriate vertical beam position flag (active low)
			if (halfline < m_vsync)
			{
				result &= ~0x04; // sync pulse
			}
			else if (halfline < (m_vsync + m_vbporch))
			{
				result &= ~0x02; // back porch
			}
			else if (halfline < (m_vsync + m_vbporch + m_vactive))
			{
				result &= ~0x01; // active
			}
			else
			{
				result &= ~0x08; // front porch
			}

			return result;
		}

	case 0xcc/4: // TODO: What is this?  Waits for bit 3 to clear before clearing interrupts.
		return 0;

	default:
		LOGCRTC("%s: read unimplemented CRTC register %02x/4\n", machine().describe_context(), offset * 4);
		return 0;
	}
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
		LOG("%s: %04x to control (%spixel clock %x sense %x transfer %x convolution %x interlace %x refresh %x RGB %x RAM %dk)\n",
				machine().describe_context(), data,
				BIT(data, 15) ? "reset " : "",
				BIT(data, 12, 3),
				BIT(data, 9, 3),
				BIT(data, 6),
				BIT(data, 5),
				BIT(data, 4),
				BIT(data, 3),
				BIT(data, 2),
				BIT(data, 0) ? 256 : 128);
		m_control = data & 0x7fff; // video reset bit needs to read as zero
		m_vram_view.select(ctrl_rgb() ? 1 : 0); // packed RGB mode
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
	case 0x08/4: // half line length from start of active area
		LOGCRTC("%s: %d to half line cells\n", machine().describe_context(), data);
		m_hhalf = data & 0x0fff;
		update_crtc();
		break;

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
