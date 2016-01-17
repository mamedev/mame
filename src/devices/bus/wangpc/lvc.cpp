// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC Low-Resolution Video Controller emulation

**********************************************************************/

/*

    TODO:

    - cursor
    - scroll
    - option bit 1?

*/

#include "lvc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

#define OPTION_ID       0x10

#define MC6845_TAG      "mc6845"
#define SCREEN_TAG      "screen"

#define RAM_SIZE        0x8000

#define OPTION_VRAM     BIT(m_option, 0)
#define OPTION_UNKNOWN  BIT(m_option, 1)
#define OPTION_80_COL   BIT(m_option, 2)
#define OPTION_VSYNC    BIT(m_option, 3)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WANGPC_LVC = &device_creator<wangpc_lvc_device>;


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( wangpc_lvc_device::crtc_update_row )
{
	offs_t scroll_y = (((m_scroll >> 8) + 0x15) & 0xff) * 0x80;

	if (OPTION_80_COL)
	{
		for (int column = 0; column < x_count; column++)
		{
			offs_t addr = scroll_y + (m_scroll & 0x3f) + ((ma / 80) * 0x480) + (((ra & 0x0f) << 7) | (column & 0x7f));
			UINT16 data = m_video_ram[addr & 0x7fff];

			for (int bit = 0; bit < 8; bit++)
			{
				int x = (column * 8) + bit;
				int color = (BIT(data, 15) << 1) | BIT(data, 7);

				if (column == cursor_x) color = 0x03;

				bitmap.pix32(vbp + y, hbp + x) = de ? m_palette[color] : rgb_t::black;

				data <<= 1;
			}
		}
	}
	else
	{
		//offs_t addr = scroll_y + ((m_scroll & 0x3f) << 1) + ((ma / 40) * 0x480) + (((ra & 0x0f) << 7));
		offs_t addr = scroll_y + ((m_scroll & 0x3f) << 1) + (y * 0x80);

		for (int column = 0; column < x_count; column++)
		{
			UINT32 data = (m_video_ram[(addr + 1) & 0x7fff] << 16) | m_video_ram[addr & 0x7fff];

			for (int bit = 0; bit < 8; bit++)
			{
				int x = (column * 8) + bit;
				int color = (BIT(data, 31) << 3) | (BIT(data, 23) << 2) | (BIT(data, 15) << 1) | BIT(data, 7);

				if (column == cursor_x) color = 0x03;

				bitmap.pix32(vbp + y, hbp + x) = de ? m_palette[color] : rgb_t::black;

				data <<= 1;
			}

			addr += 2;
		}
	}
}

WRITE_LINE_MEMBER( wangpc_lvc_device::vsync_w )
{
	if (OPTION_VSYNC && state)
	{
		set_irq(ASSERT_LINE);
	}
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( wangpc_lvc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wangpc_lvc )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(80*8, 25*9)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*9-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845_1, SCREEN_TAG, XTAL_14_31818MHz/16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(wangpc_lvc_device, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(wangpc_lvc_device, vsync_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wangpc_lvc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wangpc_lvc );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq -
//-------------------------------------------------

inline void wangpc_lvc_device::set_irq(int state)
{
	m_irq = state;

	m_bus->irq3_w(m_irq);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_lvc_device - constructor
//-------------------------------------------------

wangpc_lvc_device::wangpc_lvc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_LVC, "Wang PC Low Resolution Video Card", tag, owner, clock, "wangpc_lvc", __FILE__),
	device_wangpcbus_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_TAG),
	m_video_ram(*this, "video_ram"),
	m_option(0), m_scroll(0),
	m_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_lvc_device::device_start()
{
	// allocate memory
	m_video_ram.allocate(RAM_SIZE);

	// state saving
	save_item(NAME(m_option));
	save_item(NAME(m_scroll));
	save_item(NAME(m_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_lvc_device::device_reset()
{
	m_option = 0;

	set_irq(CLEAR_LINE);
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

UINT16 wangpc_lvc_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	if (OPTION_VRAM && (offset >= 0xe0000/2) && (offset < 0xf0000/2))
	{
		offs_t addr = offset & 0x7fff;

		data = m_video_ram[addr];
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_lvc_device::wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (OPTION_VRAM && (offset >= 0xe0000/2) && (offset < 0xf0000/2))
	{
		offs_t addr = offset & 0x7fff;

		m_video_ram[addr] = data;
	}
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

UINT16 wangpc_lvc_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x02/2:
			data = 0xff00 | m_crtc->register_r(space, 0);
			break;

		case 0x30/2:
			data = 0xffe3;
			data |= m_crtc->de_r() << 2;
			data |= m_crtc->vsync_r() << 3;
			data |= m_crtc->hsync_r() << 4;
			break;

		case 0xfe/2:
			data = 0xff00 | (m_irq << 7) | OPTION_ID;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_lvc_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			if (ACCESSING_BITS_0_7)
			{
				m_crtc->address_w(space, 0, data & 0xff);
			}
			break;

		case 0x02/2:
			if (ACCESSING_BITS_0_7)
			{
				m_crtc->register_w(space, 0, data & 0xff);
			}
			break;

		case 0x10/2:
			if (ACCESSING_BITS_0_7)
			{
				if (LOG) logerror("LVC option %02x\n", data & 0xff);
				m_option = data & 0xff;

				if (OPTION_80_COL)
				{
					m_crtc->set_clock(XTAL_14_31818MHz / 8);
				}
				else
				{
					m_crtc->set_clock(XTAL_14_31818MHz / 16);
				}
			}
			break;

		case 0x20/2:
			if (LOG) logerror("LVC scroll %04x\n", data);
			m_scroll = data;
			break;

		case 0x40/2: case 0x42/2: case 0x44/2: case 0x46/2: case 0x48/2: case 0x4a/2: case 0x4c/2: case 0x4e/2:
		case 0x50/2: case 0x52/2: case 0x55/2: case 0x56/2: case 0x58/2: case 0x5a/2: case 0x5c/2: case 0x5e/2:
			{
				offs_t index = offset & 0x0f;

				int i = BIT(data, 15);
				int r = BIT(data, 11) ? (i ? 0xff : 0x80) : 0;
				int g = BIT(data, 7) ? (i ? 0xff : 0x80) : 0;
				int b = BIT(data, 3) ? (i ? 0xff : 0x80) : 0;

				m_palette[index] = rgb_t(r, g, b);
			}
			break;

		case 0x70/2:
			set_irq(CLEAR_LINE);
			break;
		}
	}
}
