// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "RAM Based Video" system ASIC
    Emulation by R. Belmont

    RBV is the direct ancestor of the VISA/V8/VASP/Sonora.
    It originated the pseudo-VIA and video found in those chips,
    albeit in a less powerful form.
*/

#include "emu.h"
#include "rbv.h"

#include "layout/generic.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RBV, rbv_device, "applerbv", "Apple RAM-Based Video")

static INPUT_PORTS_START( rbv )
	PORT_START("MONTYPE")
	PORT_CONFNAME(0x0f, 0x06, "Connected monitor")
	PORT_CONFSETTING( 0x01, "15\" Portrait Display (640x870)")
	PORT_CONFSETTING( 0x02, "12\" RGB (512x384)")
	PORT_CONFSETTING( 0x06, "13\" RGB (640x480)")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor rbv_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rbv);
}

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void rbv_device::map(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(m_bt478, FUNC(bt478_device::read), FUNC(bt478_device::write)).umask32(0xff000000).mirror(0x00f00000);
	map(0x00002000, 0x00003fff).rw(m_pseudovia, FUNC(pseudovia_device::read), FUNC(pseudovia_device::write)).mirror(0x00f00000);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void rbv_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(30.24_MHz_XTAL, 864, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(rbv_device::screen_update));
	m_screen->screen_vblank().set(m_pseudovia, FUNC(pseudovia_device::slot_irq_w<0x40>));
	config.set_default_layout(layout_monitors);

	BT478(config, m_bt478, 0);

	APPLE_PSEUDOVIA(config, m_pseudovia, DERIVED_CLOCK(1, 2));
	m_pseudovia->readvideo_handler().set(FUNC(rbv_device::via2_video_config_r));
	m_pseudovia->writevideo_handler().set(FUNC(rbv_device::via2_video_config_w));
	m_pseudovia->irq_callback().set(FUNC(rbv_device::via2_irq_w));
}

//-------------------------------------------------
//  rbv_device - constructor
//-------------------------------------------------

rbv_device::rbv_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, RBV, tag, owner, clock),
	write_6015(*this),
	write_irq(*this),
	m_io_montype(*this, "MONTYPE"),
	m_screen(*this, "screen"),
	m_bt478(*this, "bt478"),
	m_pseudovia(*this, "pseudovia")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rbv_device::device_start()
{
	m_6015_timer = timer_alloc(FUNC(rbv_device::mac_6015_tick), this);
	m_6015_timer->adjust(attotime::never);

	m_configured = false;
	m_hres = m_vres = 0;
	m_montype = 0;
	m_monochrome = false;

	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_montype));
	save_item(NAME(m_monochrome));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rbv_device::device_reset()
{
	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_ticks(2 * 640 * 407, clock()), 0, attotime::from_ticks(2 * 640 * 407, 31.3344_MHz_XTAL));

	if (!m_configured)
	{
		m_montype = m_io_montype->read() << 3;
		switch (m_montype >> 3)
		{
		case 1: // 15" portrait display
			m_hres = 640;
			m_vres = 870;
			m_monochrome = true;
			m_screen->configure(832, 918, rectangle(0, 639, 0, 869), HZ_TO_ATTOSECONDS(57.2832_MHz_XTAL / (832 * 918)));
			break;

		case 2: // 12" RGB
			m_hres = 512;
			m_vres = 384;
			m_monochrome = false;
			m_screen->configure(640, 407, rectangle(0, 511, 0, 383), HZ_TO_ATTOSECONDS(double(clock()) / (2 * 640 * 407)));
			break;

		case 6: // 13" RGB
		default:
			m_hres = 640;
			m_vres = 480;
			m_monochrome = false;
			m_screen->configure(864, 525, rectangle(0, 639, 0, 479), HZ_TO_ATTOSECONDS(30.24_MHz_XTAL / (864 * 525)));
			break;
		}
		m_configured = true;
	}
}

void rbv_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

TIMER_CALLBACK_MEMBER(rbv_device::mac_6015_tick)
{
	write_6015(CLEAR_LINE);
	write_6015(ASSERT_LINE);
}

template <u8 mask>
void rbv_device::slot_irq_w(int state)
{
	m_pseudovia->slot_irq_w<mask>(state);
}

template void rbv_device::slot_irq_w<0x40>(int state);
template void rbv_device::slot_irq_w<0x20>(int state);
template void rbv_device::slot_irq_w<0x10>(int state);
template void rbv_device::slot_irq_w<0x08>(int state);
template void rbv_device::slot_irq_w<0x04>(int state);
template void rbv_device::slot_irq_w<0x02>(int state);
template void rbv_device::slot_irq_w<0x01>(int state);

void rbv_device::asc_irq_w(int state)
{
	m_pseudovia->asc_irq_w(state);
}

u8 rbv_device::via2_video_config_r()
{
	return m_montype;
}

void rbv_device::via2_video_config_w(u8 data)
{
	m_video_config = data;
}

void rbv_device::via2_irq_w(int state)
{
	write_irq(state);
}

u32 rbv_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_monochrome)
		return update_screen<true>(screen, bitmap, cliprect);
	else
		return update_screen<false>(screen, bitmap, cliprect);
}

template <bool Mono>
u32 rbv_device::update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(m_ram_ptr);

	// video disabled?
	if (BIT(m_video_config, 6))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	auto const pen =
			[this] (unsigned n) -> rgb_t
			{
				rgb_t const val = m_bt478->pen_color(n);
				if (Mono)
					return rgb_t(val.b(), val.b(), val.b());
				else
					return val;
			};

	switch (m_video_config & 7)
	{
	case 0: // 1bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			u32 *scanline = &bitmap.pix(y, cliprect.left() & ~7);
			for (int x = cliprect.left() / 8; x <= cliprect.right() / 8; x++)
			{
				u8 const pixels = vram8[(y * (m_hres / 8)) + x];

				*scanline++ = pen(0xfe | BIT(pixels, 7));
				*scanline++ = pen(0xfe | BIT(pixels, 6));
				*scanline++ = pen(0xfe | BIT(pixels, 5));
				*scanline++ = pen(0xfe | BIT(pixels, 4));
				*scanline++ = pen(0xfe | BIT(pixels, 3));
				*scanline++ = pen(0xfe | BIT(pixels, 2));
				*scanline++ = pen(0xfe | BIT(pixels, 1));
				*scanline++ = pen(0xfe | BIT(pixels, 0));
			}
		}
		break;

	case 1: // 2bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			u32 *scanline = &bitmap.pix(y, cliprect.left() & ~3);
			for (int x = cliprect.left() / 4; x <= cliprect.right() / 4; x++)
			{
				u8 const pixels = vram8[(y * (m_hres / 4)) + x];

				*scanline++ = pen(0xfc | ((pixels >> 6) & 3));
				*scanline++ = pen(0xfc | ((pixels >> 4) & 3));
				*scanline++ = pen(0xfc | ((pixels >> 2) & 3));
				*scanline++ = pen(0xfc | (pixels & 3));
			}
		}
		break;

	case 2: // 4bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			u32 *scanline = &bitmap.pix(y, cliprect.left() & ~1);
			for (int x = cliprect.left() / 2; x <= cliprect.right() / 2; x++)
			{
				u8 const pixels = vram8[(y * (m_hres / 2)) + x];

				*scanline++ = pen(0xf0 | (pixels >> 4));
				*scanline++ = pen(0xf0 | (pixels & 0xf));
			}
		}
		break;

	case 3: // 8bpp
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			u32 *scanline = &bitmap.pix(y, cliprect.left());
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				u8 const pixels = vram8[(y * m_hres) + x];
				*scanline++ = pen(pixels);
			}
		}
		break;
	}

	return 0;
}
