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

#include "mvc.h"



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
	rgb_t::black,
	rgb_t(0x00, 0x80, 0x00),
	rgb_t(0x00, 0xff, 0x00)
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WANGPC_MVC = &device_creator<wangpc_mvc_device>;


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( wangpc_mvc_device::crtc_update_row )
{
	for (int sx = 0; sx < 50; sx++)
	{
		offs_t addr = (y * 50) + sx;
		UINT16 data = m_bitmap_ram[addr];

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
		UINT16 code = m_video_ram[((ma + column) & 0x7ff)];
		UINT8 attr = code & 0xff;

		UINT8 new_ra = ra + 1;

		if (ATTR_SUPERSCRIPT)
		{
			new_ra = ra + 3;
		}
		else if (ATTR_SUBSCRIPT)
		{
			new_ra = ra;
		}

		offs_t addr = ((code >> 8) << 4) | (new_ra & 0x0f);
		UINT16 data = m_char_ram[addr & 0xfff];

		if ((column == cursor_x) || (!ra && ATTR_OVERSCORE) || ((ra == 9) && ATTR_UNDERSCORE))
		{
			data = 0xffff;
		}

		for (int bit = 0; bit < 10; bit++)
		{
			int x = (column * 10) + bit;
			int color = ((BIT(data, 9) & ~ATTR_BLANK) ^ ATTR_REVERSE);

			if ((color | bitmap.pix32(vbp + y, hbp + x)) & ATTR_BOLD) color = 2;
			if (color) bitmap.pix32(vbp + y, hbp + x) = de ? PALETTE_MVC[color] : rgb_t::black;

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
//  MACHINE_CONFIG_FRAGMENT( wangpc_mvc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wangpc_mvc )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(80*10, 25*12)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*10-1, 0, 25*12-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845_1, SCREEN_TAG, XTAL_14_31818MHz/16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(10)
	MCFG_MC6845_UPDATE_ROW_CB(wangpc_mvc_device, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(wangpc_mvc_device, vsync_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wangpc_mvc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wangpc_mvc );
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

wangpc_mvc_device::wangpc_mvc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_MVC, "Wang PC Medium Resolution Video Card", tag, owner, clock, "wangpc_mvc", __FILE__),
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

UINT16 wangpc_mvc_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

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

void wangpc_mvc_device::wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
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

UINT16 wangpc_mvc_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

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

void wangpc_mvc_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			m_crtc->address_w(space, 0, data & 0xff);
			break;

		case 0x02/2:
			m_crtc->register_w(space, 0, data & 0xff);
			break;

		case 0x10/2:
		case 0x12/2:
			if (LOG) logerror("MVC option %02x\n", data & 0xff);

			m_option = data & 0xff;
			break;
		}
	}
}
