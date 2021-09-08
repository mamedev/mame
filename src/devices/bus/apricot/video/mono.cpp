// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN Monochrome Display Option

    TODO:
    - Support monitor selection

***************************************************************************/

#include "emu.h"
#include "mono.h"
#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_MONO_DISPLAY, apricot_mono_display_device, "apricot_mono_display", "Apricot Monochrome Display")

//-------------------------------------------------
//  character viewer
//-------------------------------------------------

static const gfx_layout apricot_charlayout =
{
	10, 16,
	1792,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 15, 14 },
	{ STEP16(0, 16) },
	16*16
};

static GFXDECODE_START( gfx )
GFXDECODE_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void apricot_mono_display_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_size(800, 400);
	screen.set_visarea(0, 800-1, 0, 400-1);
	screen.set_refresh_hz(72);
	screen.set_screen_update(FUNC(apricot_mono_display_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	HD6845S(config, m_crtc, 24_MHz_XTAL / 10); // actually MB89321B (15 MHz for white monitor, 24 MHz for green)
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(10);
	m_crtc->set_update_row_callback(FUNC(apricot_mono_display_device::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(apricot_mono_display_device::crtc_de_w));
	m_crtc->out_vsync_callback().set(FUNC(apricot_mono_display_device::crtc_vsync_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_mono_display_device - constructor
//-------------------------------------------------

apricot_mono_display_device::apricot_mono_display_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APRICOT_MONO_DISPLAY, tag, owner, clock),
	device_apricot_video_interface(mconfig, *this),
	m_crtc(*this, "crtc"),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_mono_display_device::device_start()
{
	// allocate 64k vram
	m_vram = std::make_unique<uint16_t[]>(0x8000);

	// init gfxdecode
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, apricot_charlayout, reinterpret_cast<uint8_t *>(m_vram.get()), 0, 1, 0));

	// register for save states
	save_pointer(NAME(m_vram), 0x8000);
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));
	save_item(NAME(m_portx));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_mono_display_device::device_reset()
{
	// a reset forces all bits to a logic high
	m_portb = 0xff;
	m_portc = 0xff;
	m_portx = 0xff;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool apricot_mono_display_device::mem_r(offs_t offset, uint16_t &data, uint16_t mem_mask)
{
	data = m_vram[offset];
	return true;
}

bool apricot_mono_display_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_vram[offset] = data;
	return true;
}

bool apricot_mono_display_device::io_r(offs_t offset, uint16_t &data, uint16_t mem_mask)
{
	offset <<= 1;

	if (BIT(m_portx, 0) == 0)
	{
		// apricot pc/xi compatible
		switch (offset)
		{
			case 0x4a: data = 0xff00 | m_portb; return true;
			case 0x4c: data = 0xff00 | m_portc; return true;
			case 0x6a: data = 0xff00 | m_crtc->register_r(); return true;
		}
	}
	else
	{
		// xen mode
		switch (offset)
		{
			case 0x80: data = 0xff00 | m_portb; return true;
			case 0x82: data = 0xff00 | m_portc; return true;
			case 0x8a: data = 0xff00 | m_crtc->register_r(); return true;
		}
	}

	return false;
}

bool apricot_mono_display_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;

	// only 8-bit writes
	if (mem_mask != 0x00ff)
		return false;

	if (BIT(m_portx, 0) == 0)
	{
		// apricot pc/xi compatible
		switch (offset)
		{
			case 0x4a: portb_w(data); return true;
			case 0x68: m_crtc->address_w(data); return true;
			case 0x6a: m_crtc->register_w(data); return true;
			case 0x6c: portx_w(data); return true;
		}
	}
	else
	{
		// xen mode
		switch (offset)
		{
			case 0x6c: portx_w(data); return true;
			case 0x80: portb_w(data); return true;
			case 0x88: m_crtc->address_w(data); return true;
			case 0x8a: m_crtc->register_w(data); return true;
		}
	}

	return false;
}

void apricot_mono_display_device::portb_w(uint8_t data)
{
	logerror("portb_w: %02x\n", data);

	// 765-----  not used
	// ---4----  video mode (alphanumeric/graphics)
	// ----3---  display on
	// -----21-  not used
	// -------0  crtc reset

	m_portb = data;

	if (BIT(m_portb, 0) == 0)
		m_crtc->reset();
}

void apricot_mono_display_device::portx_w(uint8_t data)
{
	logerror("portx_w: %02x\n", data);

	// 765432--  not used
	// ------1-  reverse video
	// -------0  mode (compatible or xen)

	m_portx = data;

	// forward mode to host
	m_slot->apvid_w(BIT(m_portx, 0));
}

uint32_t apricot_mono_display_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_gfxdecode->gfx(0)->mark_all_dirty();
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

// see drivers/apricot.cpp
MC6845_UPDATE_ROW( apricot_mono_display_device::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		uint16_t code = m_vram[(0xe000 >> 1) | (ma + i)];
		uint16_t offset = ((code & 0x7ff) << 5) | (ra << 1);
		uint16_t data = m_vram[offset >> 1];

		if (BIT(m_portb, 4))
		{
			int fill = 0;

			if (i == cursor_x) fill = 1; // cursor?
			if (BIT(code, 12) && BIT(data, 14)) fill = 1; // strike-through?
			if (BIT(code, 13) && BIT(data, 15)) fill = 1; // underline?

			// draw 10 pixels of the character
			for (int x = 0; x <= 10; x++)
			{
				int color = fill ? 1 : BIT(data, x);
				color ^= BIT(code, 15); // reverse?
				bitmap.pix(y, x + i*10) = pen[color ? 1 + BIT(code, 14) : 0];
			}
		}
		else
		{
			// draw 16 pixels of the cell
			for (int x = 0; x <= 16; x++)
				bitmap.pix(y, x + i*16) = pen[BIT(data, x)];
		}
	}
}

void apricot_mono_display_device::crtc_de_w(int state)
{
	m_portc &= 0xf7;
	m_portc |= (state << 3);
}

void apricot_mono_display_device::crtc_vsync_w(int state)
{
	m_portc &= 0x7f;
	m_portc |= (state << 7);
}
