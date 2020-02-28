// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC PM-001B Medium-Resolution Video Controller emulation

**********************************************************************/

/*

    TODO:

    - character clock
    - blink

*/

#include "emu.h"
#include "mvc.h"

#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

#define OPTION_ID           0x15

#define MC6845_TAG          "mc6845"
#define SCREEN_TAG          "screen"

#define VIDEO_RAM_SIZE      0x800
#define CHAR_RAM_SIZE       0x1000
#define BITMAP_RAM_SIZE     0x4000

#define OPTION_VRAM         BIT(m_option, 0)
#define OPTION_VSYNC        BIT(m_option, 3)

#define ATTR_BLINK          BIT(attr, 0)
#define ATTR_REVERSE        BIT(attr, 1)
#define ATTR_BLANK          BIT(attr, 2)
#define ATTR_BOLD           BIT(attr, 3)
#define ATTR_OVERSCORE      BIT(attr, 4)
#define ATTR_UNDERSCORE     BIT(attr, 5)
#define ATTR_SUBSCRIPT      BIT(attr, 6)
#define ATTR_SUPERSCRIPT    BIT(attr, 7)

static const rgb_t PALETTE_MVC[] =
{
	rgb_t::black(),
	rgb_t(0x00, 0x80, 0x00),
	rgb_t(0x00, 0xff, 0x00)
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_MVC, wangpc_mvc_device, "wangpc_mvc", "Wang PC Medium Resolution Video Card")


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( wangpc_mvc_device::crtc_update_row )
{
	for (int sx = 0; sx < 50; sx++)
	{
		offs_t addr = (y * 50) + sx;
		uint16_t data = m_bitmap_ram[addr];

		for (int bit = 0; bit < 16; bit++)
		{
			int x = (sx * 16) + bit;
			int color = BIT(data, 15) && de;

			bitmap.pix32(vbp + y, hbp + x) = PALETTE_MVC[color];

			data <<= 1;
		}
	}

	for (int column = 0; column < x_count; column++)
	{
		uint16_t code = m_video_ram[((ma + column) & 0x7ff)];
		uint8_t attr = code & 0xff;

		uint8_t new_ra = ra + 1;

		if (ATTR_SUPERSCRIPT)
		{
			new_ra = ra + 3;
		}
		else if (ATTR_SUBSCRIPT)
		{
			new_ra = ra;
		}

		offs_t addr = ((code >> 8) << 4) | (new_ra & 0x0f);
		uint16_t data = m_char_ram[addr & 0xfff];

		if ((column == cursor_x) || (!ra && ATTR_OVERSCORE) || ((ra == 9) && ATTR_UNDERSCORE))
		{
			data = 0xffff;
		}

		for (int bit = 0; bit < 10; bit++)
		{
			int x = (column * 10) + bit;
			int color = ((BIT(data, 9) & ~ATTR_BLANK) ^ ATTR_REVERSE);

			if ((color | bitmap.pix32(vbp + y, hbp + x)) & ATTR_BOLD) color = 2;
			if (color) bitmap.pix32(vbp + y, hbp + x) = de ? PALETTE_MVC[color] : rgb_t::black();

			data <<= 1;
		}
	}
}

WRITE_LINE_MEMBER( wangpc_mvc_device::vsync_w )
{
	if (OPTION_VSYNC && state)
	{
		set_irq(ASSERT_LINE);
	}
}

//-------------------------------------------------
//  machine_config( wangpc_mvc )
//-------------------------------------------------

void wangpc_mvc_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen.set_size(80*10, 25*12);
	screen.set_visarea(0, 80*10-1, 0, 25*12-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_refresh_hz(60);

	MC6845_1(config, m_crtc, XTAL(14'318'181)/16);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(10);
	m_crtc->set_update_row_callback(FUNC(wangpc_mvc_device::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(wangpc_mvc_device::vsync_w));
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq -
//-------------------------------------------------

inline void wangpc_mvc_device::set_irq(int state)
{
	m_irq = state;

	m_bus->irq3_w(m_irq);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_mvc_device - constructor
//-------------------------------------------------

wangpc_mvc_device::wangpc_mvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_MVC, tag, owner, clock),
	device_wangpcbus_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_TAG),
	m_video_ram(*this, "video_ram"),
	m_char_ram(*this, "char_ram"),
	m_bitmap_ram(*this, "bitmap_ram"),
	m_option(0),
	m_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_mvc_device::device_start()
{
	// allocate memory
	m_video_ram.allocate(VIDEO_RAM_SIZE);
	m_char_ram.allocate(CHAR_RAM_SIZE);
	m_bitmap_ram.allocate(BITMAP_RAM_SIZE);

	// state saving
	save_item(NAME(m_option));
	save_item(NAME(m_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_mvc_device::device_reset()
{
	m_option = 0;

	set_irq(CLEAR_LINE);
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

uint16_t wangpc_mvc_device::wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (OPTION_VRAM)
	{
		if (offset >= 0xe0000/2 && offset < 0xe8000/2)
		{
			data = m_bitmap_ram[offset & 0x3fff];
		}
		else if (offset >= 0xf0000/2 && offset < 0xf1000/2)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		else if (offset >= 0xf2000/2 && offset < 0xf4000/2)
		{
			data = m_char_ram[offset & 0xfff];
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_mvc_device::wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (OPTION_VRAM)
	{
		if (offset >= 0xe0000/2 && offset < 0xe8000/2)
		{
			m_bitmap_ram[offset & 0x3fff] = data;
		}
		else if (offset >= 0xf0000/2 && offset < 0xf1000/2)
		{
			m_video_ram[offset & 0x7ff] = data;
		}
		else if (offset >= 0xf2000/2 && offset < 0xf4000/2)
		{
			m_char_ram[offset & 0xfff] = data;
		}
	}
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_mvc_device::wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0xfe/2:
			data = 0xff00 | (m_irq << 7) | OPTION_ID;

			set_irq(CLEAR_LINE);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_mvc_device::wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			m_crtc->address_w(data & 0xff);
			break;

		case 0x02/2:
			m_crtc->register_w(data & 0xff);
			break;

		case 0x10/2:
		case 0x12/2:
			if (LOG) logerror("MVC option %02x\n", data & 0xff);

			m_option = data & 0xff;
			break;
		}
	}
}
