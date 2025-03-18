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

static constexpr u32 C7M  = 7833600;
static constexpr u32 C15M = (C7M * 2);

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
	return INPUT_PORTS_NAME( rbv );
}

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void rbv_device::map(address_map &map)
{
	map(0x00000000, 0x00000007).rw(FUNC(rbv_device::dac_r), FUNC(rbv_device::dac_w)).mirror(0x00f00000);
	map(0x00002000, 0x00003fff).rw(m_pseudovia, FUNC(pseudovia_device::read), FUNC(pseudovia_device::write)).mirror(0x00f00000);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void rbv_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_size(1024, 768);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(FUNC(rbv_device::screen_update));
	m_screen->screen_vblank().set(m_pseudovia, FUNC(pseudovia_device::slot_irq_w<0x40>));
	config.set_default_layout(layout_monitors);

	PALETTE(config, m_palette).set_entries(256);

	APPLE_PSEUDOVIA(config, m_pseudovia, C15M);
	m_pseudovia->readvideo_handler().set(FUNC(rbv_device::via2_video_config_r));
	m_pseudovia->writevideo_handler().set(FUNC(rbv_device::via2_video_config_w));
	m_pseudovia->irq_callback().set(FUNC(rbv_device::via2_irq_w));
}

//-------------------------------------------------
//  rbv_device - constructor
//-------------------------------------------------

rbv_device::rbv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, RBV, tag, owner, clock),
	write_6015(*this),
	write_irq(*this),
	m_montype(*this, "MONTYPE"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
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

	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));

	m_pal_address = m_pal_idx = 0;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rbv_device::device_reset()
{
	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
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

void rbv_device::pseudovia_recalc_irqs()
{
	// check slot interrupts and bubble them down to IFR
	uint8_t slot_irqs = (~m_pseudovia_regs[2]) & 0x78;
	slot_irqs &= (m_pseudovia_regs[0x12] & 0x78);

	if (slot_irqs)
	{
		m_pseudovia_regs[3] |= 2; // any slot
	}
	else // no slot irqs, clear the pending bit
	{
		m_pseudovia_regs[3] &= ~2; // any slot
	}

	uint8_t ifr = (m_pseudovia_regs[3] & m_pseudovia_ier) & 0x1b;

	if (ifr != 0)
	{
		m_pseudovia_regs[3] = ifr | 0x80;
		m_pseudovia_ifr = ifr | 0x80;

		write_irq(ASSERT_LINE);
	}
	else
	{
		write_irq(CLEAR_LINE);
	}
}

u8 rbv_device::via2_video_config_r()
{
	return m_montype->read() << 3;
}

void rbv_device::via2_video_config_w(u8 data)
{
	m_video_config = data;
}

void rbv_device::via2_irq_w(int state)
{
	write_irq(state);
}

uint8_t rbv_device::pseudovia_r(offs_t offset)
{
	int data = 0;

	if (offset < 0x100)
	{
		data = m_pseudovia_regs[offset];

		if (offset == 0x10)
		{
			data &= ~0x38;
			data |= (m_montype->read() << 3);
		}

		// bit 7 of these registers always reads as 0 on pseudo-VIAs
		if ((offset == 0x12) || (offset == 0x13))
		{
			data &= ~0x80;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 13: // IFR
			data = m_pseudovia_ifr;
			break;

		case 14: // IER
			data = m_pseudovia_ier;
			break;

		default:
			logerror("pseudovia_r: Unknown pseudo-VIA register %d access\n", offset);
			break;
		}
	}
	return data;
}

void rbv_device::pseudovia_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		switch (offset)
		{
		case 0x02:
			m_pseudovia_regs[offset] |= (data & 0x40);
			pseudovia_recalc_irqs();
			break;

		case 0x03:           // write here to ack
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;
				m_pseudovia_ifr |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
				m_pseudovia_ifr &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		case 0x10:
			m_pseudovia_regs[offset] = data;
			break;

		case 0x12:
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		case 0x13:
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_regs[offset] |= data & 0x7f;

				if (data == 0xff)
					m_pseudovia_regs[offset] = 0x1f; // I don't know why this is special, but the IIci ROM's POST demands it
			}
			else // 1 bits write 0s
			{
				m_pseudovia_regs[offset] &= ~(data & 0x7f);
			}
			break;

		default:
			m_pseudovia_regs[offset] = data;
			break;
		}
	}
	else
	{
		offset >>= 9;

		switch (offset)
		{
		case 13: // IFR
			if (data & 0x80)
			{
				data = 0x7f;
			}
			pseudovia_recalc_irqs();
			break;

		case 14:             // IER
			if (data & 0x80) // 1 bits write 1s
			{
				m_pseudovia_ier |= data & 0x7f;
			}
			else // 1 bits write 0s
			{
				m_pseudovia_ier &= ~(data & 0x7f);
			}
			pseudovia_recalc_irqs();
			break;

		default:
			logerror("pseudovia_w: Unknown extended pseudo-VIA register %d access\n", offset);
			break;
		}
	}
}

u8 rbv_device::dac_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return m_pal_address;

	default:
		return 0;
	}
}

void rbv_device::dac_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_pal_address = data;
		m_pal_idx = 0;
		break;

	case 4:
		switch (m_pal_idx)
		{
		case 0:
			m_palette->set_pen_red_level(m_pal_address, data);
			break;
		case 1:
			m_palette->set_pen_green_level(m_pal_address, data);
			break;
		case 2:
			m_palette->set_pen_blue_level(m_pal_address, data);
			break;
		}
		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_pal_address++;
		}
		break;
	}
}

u32 rbv_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int hres, vres;
	uint8_t const *vram8 = (uint8_t *)m_ram_ptr;

	switch (m_montype->read())
	{
	case 1: // 15" portrait display
		hres = 640;
		vres = 870;
		break;

	case 2: // 12" RGB
		hres = 512;
		vres = 384;
		break;

	case 6: // 13" RGB
	default:
		hres = 640;
		vres = 480;
		break;
	}

	// video disabled?
	if (m_video_config & 0x40)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	const pen_t *pens = m_palette->pens();

	switch (m_video_config & 7)
	{
	case 0: // 1bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres; x += 8)
			{
				uint8_t const pixels = vram8[(y * (hres / 8)) + ((x / 8) ^ 3)];

				*scanline++ = pens[0xfe | (pixels >> 7)];
				*scanline++ = pens[0xfe | ((pixels >> 6) & 1)];
				*scanline++ = pens[0xfe | ((pixels >> 5) & 1)];
				*scanline++ = pens[0xfe | ((pixels >> 4) & 1)];
				*scanline++ = pens[0xfe | ((pixels >> 3) & 1)];
				*scanline++ = pens[0xfe | ((pixels >> 2) & 1)];
				*scanline++ = pens[0xfe | ((pixels >> 1) & 1)];
				*scanline++ = pens[0xfe | (pixels & 1)];
			}
		}
	}
	break;

	case 1: // 2bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);
			for (int x = 0; x < hres / 4; x++)
			{
				uint8_t const pixels = vram8[(y * (hres / 4)) + (BYTE4_XOR_BE(x))];

				*scanline++ = pens[0xfc | ((pixels >> 6) & 3)];
				*scanline++ = pens[0xfc | ((pixels >> 4) & 3)];
				*scanline++ = pens[0xfc | ((pixels >> 2) & 3)];
				*scanline++ = pens[0xfc | (pixels & 3)];
			}
		}
	}
	break;

	case 2: // 4bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres / 2; x++)
			{
				uint8_t const pixels = vram8[(y * (hres / 2)) + (BYTE4_XOR_BE(x))];

				*scanline++ = pens[0xf0 | (pixels >> 4)];
				*scanline++ = pens[0xf0 | (pixels & 0xf)];
			}
		}
	}
	break;

	case 3: // 8bpp
	{
		for (int y = 0; y < vres; y++)
		{
			uint32_t *scanline = &bitmap.pix(y);

			for (int x = 0; x < hres; x++)
			{
				uint8_t const pixels = vram8[(y * hres) + (BYTE4_XOR_BE(x))];
				*scanline++ = pens[pixels];
			}
		}
	}
	break;
	}

	return 0;
}
