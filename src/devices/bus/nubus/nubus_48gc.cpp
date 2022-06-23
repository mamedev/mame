// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
/***************************************************************************

  Apple 4*8 Graphics Card (model 630-0400) emulation
  Apple 8*24 Graphics Card emulation (cards have the same framebuffer chip
      w/different ROMs and RAMDACs, apparently)

  TODO:
  * Work out why some monitors need magic multiply or divide by two to
    get the right RAMDAC clock - inferring it from the reference clock
    modulus is definitely wrong.
  * When a monochrome monitor is connected, it sends the intensity to the
    blue channel - maybe detect this and map blue channel to white?
  * The 8•24 card uses a strange off-white palette with some monitors,
    including the 21" and 16" RGB displays.
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


static INPUT_PORTS_START( 48gc )
	PORT_START("MONITOR")
	PORT_CONFNAME(0x07, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Macintosh Two-Page Monitor (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Macintosh Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Macintosh RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Macintosh Two-Page Monitor (B&W 21\" 1152\u00d7870)")
	//PORT_CONFSETTING(   0x04, u8"NTSC Monitor") requires implementing interlace modes
	PORT_CONFSETTING(   0x05, u8"Macintosh Portrait Display (640\u00d7870)")
	PORT_CONFSETTING(   0x06, u8"Macintosh Hi-Res Display (12-14\" 640\u00d7480)")
INPUT_PORTS_END


static INPUT_PORTS_START( 824gc )
	PORT_START("MONITOR")
	PORT_CONFNAME(0x07, 0x06, u8"Attached monitor")
	PORT_CONFSETTING(   0x00, u8"Mac 21\" Color Display (1152\u00d7870)")
	PORT_CONFSETTING(   0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")
	PORT_CONFSETTING(   0x02, u8"Mac RGB Display (12\" 512\u00d7384)")
	PORT_CONFSETTING(   0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")
	//PORT_CONFSETTING(   0x04, u8"NTSC Monitor") requires implementing interlace modes
	PORT_CONFSETTING(   0x05, u8"Mac 16\" Color Display (832\u00d7624)")
	PORT_CONFSETTING(   0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")
INPUT_PORTS_END


ROM_START( gc48 )
	ROM_REGION(0x8000, GC48_ROM_REGION, 0)
	ROM_LOAD( "3410801.bin",  0x0000, 0x8000, CRC(e283da91) SHA1(4ae21d6d7bbaa6fc7aa301bee2b791ed33b1dcf9) )
ROM_END

ROM_START( gc824 )
	ROM_REGION(0x8000, GC48_ROM_REGION, 0)
	ROM_LOAD( "3410868.bin",  0x000000, 0x008000, CRC(57f925fa) SHA1(4d3c0632711b7b31c8e0c5cfdd7ec1904f178336) ) /* Label: "341-0868 // (C)APPLE COMPUTER // INC. 1986-1991 // ALL RIGHTS // RESERVED    W5" */
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_48GC,  nubus_48gc_device,  "nb_48gc",  "Apple Macintosh Display Card 4*8")
DEFINE_DEVICE_TYPE(NUBUS_824GC, nubus_824gc_device, "nb_824gc", "Apple Macintosh Display Card 8*24")


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
	m_vbl_disable(0), m_toggle(0), m_stride(0), m_base(0),
	m_count(0), m_clutoffs(0), m_mode(0)
{
	set_screen(*this, GC48_SCREEN_NAME);
}

nubus_48gc_device::nubus_48gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jmfb_device(mconfig, NUBUS_48GC, tag, owner, clock)
{
}

nubus_824gc_device::nubus_824gc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jmfb_device(mconfig, NUBUS_824GC, tag, owner, clock)
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

	m_timer = timer_alloc(FUNC(jmfb_device::vbl_tick), this);

	save_item(NAME(m_vram));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_toggle));
	save_item(NAME(m_stride));
	save_item(NAME(m_base));
	save_item(NAME(m_registers));
	save_item(NAME(m_preload));
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

void nubus_48gc_device::device_start()
{
	jmfb_device::device_start();

	uint32_t const slotspace = get_slotspace();
	nubus().install_device(
			slotspace + 0x200000, slotspace + 0x2003ff,
			read32s_delegate(*this, FUNC(nubus_48gc_device::jmfb_r)), write32s_delegate(*this, FUNC(nubus_48gc_device::mac_48gc_w)));
}

void nubus_824gc_device::device_start()
{
	jmfb_device::device_start();

	uint32_t const slotspace = get_slotspace();
	nubus().install_device(
			slotspace + 0x200000, slotspace + 0x2003ff,
			read32s_delegate(*this, FUNC(nubus_824gc_device::jmfb_r)), write32s_delegate(*this, FUNC(nubus_824gc_device::mac_824gc_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void jmfb_device::device_reset()
{
	m_vram_view.select(0);

	std::fill(m_vram.begin(), m_vram.end(), 0);
	m_vbl_disable = 1;
	m_toggle = 0;
	m_stride = 80 / 4;
	m_base = 0;
	m_preload = 256 - 8;

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
	auto const screenbase = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_base << 5);
	int const xres = screen.visible_area().right();

	switch (m_mode)
	{
		case 0: // 1bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				auto const rowbase = screenbase + (y * m_stride * 4);
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x <= xres/8; x++)
				{
					uint8_t const pixels = rowbase[x];

					*scanline++ = pen_color(BIT(pixels, 7));
					*scanline++ = pen_color(BIT(pixels, 6));
					*scanline++ = pen_color(BIT(pixels, 5));
					*scanline++ = pen_color(BIT(pixels, 4));
					*scanline++ = pen_color(BIT(pixels, 3));
					*scanline++ = pen_color(BIT(pixels, 2));
					*scanline++ = pen_color(BIT(pixels, 1));
					*scanline++ = pen_color(BIT(pixels, 0));
				}
			}
			break;

		case 1: // 2bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				auto const rowbase = screenbase + (y * m_stride * 4);
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x <= xres/4; x++)
				{
					uint8_t const pixels = rowbase[x];

					*scanline++ = pen_color(BIT(pixels, 6, 2));
					*scanline++ = pen_color(BIT(pixels, 4, 2));
					*scanline++ = pen_color(BIT(pixels, 2, 2));
					*scanline++ = pen_color(BIT(pixels, 0, 2));
				}
			}
			break;

		case 2: // 4 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				auto const rowbase = screenbase + (y * m_stride * 4);
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x <= xres/2; x++)
				{
					uint8_t const pixels = rowbase[x];

					*scanline++ = pen_color(BIT(pixels, 4, 4));
					*scanline++ = pen_color(BIT(pixels, 0, 4));
				}
			}
			break;

		case 3: // 8 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				auto const rowbase = screenbase + (y * m_stride * 4);
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x <= xres; x++)
				{
					*scanline++ = pen_color(rowbase[x]);
				}
			}
			break;

		case 4: // 24 bpp
			for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
			{
				auto source = util::big_endian_cast<uint8_t const>(&m_vram[0]) + (m_base << 6) + (y * m_stride * 8);
				uint32_t *scanline = &bitmap.pix(y);
				for (int x = 0; x <= xres; x++)
				{
					*scanline++ = rgb_t(source[0], source[1], source[2]);
					source += 3;
				}
			}
			break;
	}

	return 0;
}

void jmfb_device::update_crtc()
{
	int const vtotal = (m_vactive + m_vbporch + m_vsync + m_vfporch) / 2;
	int const height = m_vactive / 2;
	if (vtotal && height && m_multiplier)
	{
		// FIXME: where does this multiply/divide by 2 come from?
		// This is obviously not correct by any definition.
		int scale = 0;
		switch (m_modulus)
		{
		case 15:
			scale = -1;
			break;
		case 21:
			scale = 0;
			break;
		case 19:
		case 22:
			scale = 1;
			break;
		default:
			throw emu_fatalerror("%s: Unknown clock modulus %d\n", tag(), m_modulus);
		}
		int const divider = 256 - unsigned(m_preload);
		int htotal = ((m_hactive + m_hbporch + m_hsync + m_hfporch + 8) << (m_pdiv + scale)) / divider;
		int width = ((m_hactive + 2) << (m_pdiv + scale)) / divider;
		switch (m_mode)
		{
			case 0: // 1bpp:
				htotal <<= 3;
				width <<= 3;
				break;
			case 1: // 2bpp:
				htotal <<= 2;
				width <<= 2;
				break;
			case 2: // 4bpp:
				htotal <<= 1;
				width <<= 1;
				break;
			case 3: // 8bpp:
				break;
			case 4: // 24bpp:
				htotal >>= 2;
				width >>= 2;
				break;
		}
		XTAL const pixclock = 20_MHz_XTAL / m_modulus * m_multiplier / (1 << m_pdiv);
		screen().configure(
				htotal, vtotal,
				rectangle(0, width - 1, 0, height - 1),
				attotime::from_ticks(htotal * vtotal, pixclock).attoseconds());

		// TODO: determine correct timing for vertical blanking interrupt
		m_timer->adjust(screen().time_until_pos(height - 1, 0));
	}
}

uint32_t jmfb_device::jmfb_r(offs_t offset, uint32_t mem_mask)
{
//  printf("%s 48gc_r: @ %x, mask %08x\n", machine().describe_context().c_str(), offset, mem_mask);

	switch (offset)
	{
		case 0x000/4:
			return m_monitor->read() << 9;

		case 0x1c0/4:
			m_toggle ^= 0xffffffff;
			return m_toggle;
	}

	return 0;
}

void jmfb_device::jmfb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_registers[offset & 0xff]);

	switch (offset)
	{
		case 0x000/4: // control
			LOG("%s: %04x to control\n", machine().describe_context(), data);
			if (BIT(data, 7))
			{
				m_vram_view.select(BIT(data, 2)); // packed RGB mode
			}
			break;

		case 0x004/4:
			LOG("%s: %02x to preload\n", machine().describe_context(), data);
			m_preload = data;
			update_crtc();
			break;

		case 0x008/4: // base
			LOG("%s: %x to base\n", machine().describe_context(), data);
			m_base = data;
			break;

		case 0x00c/4: // stride
			LOG("%s: %x to stride\n", machine().describe_context(), data);
			// this value is in DWORDs for 1-8 bpp and, uhh, strange for 24bpp
			m_stride = data;
			break;

		case 0x10c/4: // active pixel cells - 2
			LOG("%s: %d-2 to active cells\n", machine().describe_context(), data + 2);
			m_hactive = data;
			update_crtc();
			break;

		case 0x110/4: // horizontal back porch
			LOG("%s: %d to horizontal back porch\n", machine().describe_context(), data + 2);
			m_hbporch = data;
			update_crtc();
			break;

		case 0x114/4: // horizontal sync pulse
			LOG("%s: %d-2 to horizontal sync pulse\n", machine().describe_context(), data + 2);
			m_hsync = data;
			update_crtc();
			break;

		case 0x118/4: // horizontal front porch
			LOG("%s: %d-2 to horizontal front porch\n", machine().describe_context(), data + 2);
			m_hfporch = data;
			update_crtc();
			break;

		case 0x124/4: // active lines * 2
			LOG("%s: %d*2 to active lines\n", machine().describe_context(), data / 2);
			m_vactive = data;
			update_crtc();
			break;

		case 0x128/4: // vertical back porch * 2
			LOG("%s: %d*2 to vertical back porch\n", machine().describe_context(), data / 2);
			m_vbporch = data;
			update_crtc();
			break;

		case 0x12c/4: // vertical sync width * 2
			LOG("%s: %d*2 to vertical sync pulse width\n", machine().describe_context(), data / 2);
			m_vsync = data;
			update_crtc();
			break;

		case 0x130/4: // vertical front porch * 2
			LOG("%s: %d*2 to vertical front porch\n", machine().describe_context(), data / 2);
			m_vfporch = data;
			update_crtc();
			break;

		case 0x13c/4: // bit 1 = VBL disable (1=no interrupts)
			m_vbl_disable = (data & 2) ? 1 : 0;
			break;

		case 0x148/4: // write 1 here to clear interrupt
			if (data == 1)
			{
				lower_slot_irq();
			}
			break;

		case 0x300/4:
		case 0x304/4:
		case 0x308/4:
		case 0x30c/4:
			m_multiplier &= ~(0x0f << ((offset & 3) * 4));
			m_multiplier |= (data & 0x0f) << ((offset & 3) * 4);
			LOG("%s: %d to multiplier\n", machine().describe_context(), m_multiplier);
			update_crtc();
			break;

		case 0x310/4:
		case 0x314/4:
		case 0x318/4:
			m_modulus &= ~(0x0f << ((offset & 3) * 4));
			m_modulus |= (data & 0x0f) << ((offset & 3) * 4);
			LOG("%s: %d to modulus\n", machine().describe_context(), m_modulus);
			if (offset == 0x318/4) // avoid bad intermediate values
				update_crtc();
			break;

		case 0x324/4:
			LOG("%s: 1<<%d to pixel cell divider\n", machine().describe_context(), data);
			m_pdiv = data & 0x0f;
			update_crtc();
			break;

		default:
			break;
	}
}

void nubus_48gc_device::mac_48gc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x200/4:   // DAC control
			dac_ctrl_w(data >> 24);
			break;

		case 0x204/4:   // DAC data
			dac_data_w(data >> 24);
			break;

		case 0x208/4:   // mode control
			mode_w(data, !((data >> 5) & 0x3));
			break;

		default:
			jmfb_w(offset, data, mem_mask);
	}
}

void nubus_824gc_device::mac_824gc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
		case 0x200/4:   // DAC control
			dac_ctrl_w(data & 0xff);
			break;

		case 0x204/4:   // DAC data
			dac_data_w(data & 0xff);
			break;

		case 0x208/4:   // mode control
			mode_w(data, BIT(data, 1));
			break;

		default:
			jmfb_w(offset, data, mem_mask);
	}
}

void jmfb_device::dac_ctrl_w(uint8_t data)
{
	LOG("%s: %02x to DAC control\n", machine().describe_context(), data);
	m_clutoffs = data;
	m_count = 0;
}

void jmfb_device::dac_data_w(uint8_t data)
{
	m_colors[m_count++] = data;

	if (m_count == 3)
	{
		LOG("%s: RAMDAC: color %d = %02x %02x %02x\n", machine().describe_context(), m_clutoffs, m_colors[0], m_colors[1], m_colors[2]);
		set_pen_color(m_clutoffs, rgb_t(m_colors[0], m_colors[1], m_colors[2]));
		m_clutoffs++;
		m_count = 0;
	}
}

void jmfb_device::mode_w(uint32_t data, bool rgb)
{
	m_mode = (data >> 3) & 0x3;
	if ((m_mode == 3) & rgb)    // mode 3 can be 8 or 24 bpp
		m_mode = 4;
	LOG("%s: %02x to mode (m_mode = %d)\n", machine().describe_context(), data, m_mode);
	update_crtc();
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
