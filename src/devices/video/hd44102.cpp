// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver emulation

**********************************************************************/

#include "emu.h"
#include "hd44102.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CONTROL_DISPLAY_OFF         0x38
#define CONTROL_DISPLAY_ON          0x39
#define CONTROL_COUNT_DOWN_MODE     0x3a
#define CONTROL_COUNT_UP_MODE       0x3b
#define CONTROL_Y_ADDRESS_MASK      0x3f
#define CONTROL_X_ADDRESS_MASK      0xc0
#define CONTROL_DISPLAY_START_PAGE  0x3e


#define STATUS_BUSY                 0x80    /* not supported */
#define STATUS_COUNT_UP             0x40
#define STATUS_DISPLAY_OFF          0x20
#define STATUS_RESET                0x10    /* not supported */


// device type definition
DEFINE_DEVICE_TYPE(HD44102, hd44102_device, "hd44102", "Hitachi HD44102 LCD Controller")


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  count_up_or_down -
//-------------------------------------------------

inline void hd44102_device::count_up_or_down()
{
	if (m_status & STATUS_COUNT_UP)
	{
		if (++m_y > 49) m_y = 0;
	}
	else
	{
		if (--m_y < 0) m_y = 49;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd44102_device - constructor
//-------------------------------------------------

hd44102_device::hd44102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HD44102, tag, owner, clock),
		device_video_interface(mconfig, *this),
		m_cs2(0),
		m_page(0),
		m_x(0),
		m_y(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd44102_device::device_start()
{
	// register for state saving
	save_item(NAME(m_ram[0]));
	save_item(NAME(m_ram[1]));
	save_item(NAME(m_ram[2]));
	save_item(NAME(m_ram[3]));
	save_item(NAME(m_status));
	save_item(NAME(m_output));
	save_item(NAME(m_cs2));
	save_item(NAME(m_page));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd44102_device::device_reset()
{
	m_status = STATUS_DISPLAY_OFF | STATUS_COUNT_UP;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

READ8_MEMBER( hd44102_device::read )
{
	uint8_t data = 0;

	if (m_cs2)
	{
		data = (offset & 0x01) ? data_r(space, offset) : status_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( hd44102_device::write )
{
	if (m_cs2)
	{
		(offset & 0x01) ? data_w(space, offset, data) : control_w(space, offset, data);
	}
}


//-------------------------------------------------
//  status_r - status read
//-------------------------------------------------

READ8_MEMBER( hd44102_device::status_r )
{
	return m_status;
}


//-------------------------------------------------
//  control_w - control write
//-------------------------------------------------

WRITE8_MEMBER( hd44102_device::control_w )
{
	if (m_status & STATUS_BUSY) return;

	switch (data)
	{
	case CONTROL_DISPLAY_OFF:
		LOG("HD44102 Display Off\n");

		m_status |= STATUS_DISPLAY_OFF;
		break;

	case CONTROL_DISPLAY_ON:
		LOG("HD44102 Display On\n");

		m_status &= ~STATUS_DISPLAY_OFF;
		break;

	case CONTROL_COUNT_DOWN_MODE:
		LOG("HD44102 Count Down Mode\n");

		m_status &= ~STATUS_COUNT_UP;
		break;

	case CONTROL_COUNT_UP_MODE:
		LOG("HD44102 Count Up Mode\n");

		m_status |= STATUS_COUNT_UP;
		break;

	default:
		{
			const int x = (data & CONTROL_X_ADDRESS_MASK) >> 6;
			const int y = data & CONTROL_Y_ADDRESS_MASK;

			if ((data & CONTROL_Y_ADDRESS_MASK) == CONTROL_DISPLAY_START_PAGE)
			{
				LOG("HD44102 Display Start Page %u\n", x);

				m_page = x;
			}
			else if (y > 49)
			{
				logerror("HD44102 Invalid Address X %u Y %u (%02x)!\n", data, x, y);
			}
			else
			{
				LOG("HD44102 Address X %u Y %u (%02x)\n", data, x, y);

				m_x = x;
				m_y = y;
			}
		}
	}
}


//-------------------------------------------------
//  data_r - data read
//-------------------------------------------------

READ8_MEMBER( hd44102_device::data_r )
{
	uint8_t data = m_output;

	m_output = m_ram[m_x][m_y];

	count_up_or_down();

	return data;
}


//-------------------------------------------------
//  data_w - data write
//-------------------------------------------------

WRITE8_MEMBER( hd44102_device::data_w )
{
	m_ram[m_x][m_y] = data;

	count_up_or_down();
}


//-------------------------------------------------
//  cs2_w - chip select 2 write
//-------------------------------------------------

WRITE_LINE_MEMBER( hd44102_device::cs2_w )
{
	m_cs2 = state;
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

uint32_t hd44102_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 50; y++)
	{
		int z = m_page << 3;

		for (int x = 0; x < 32; x++)
		{
			uint8_t data = m_ram[z / 8][y];

			int sy = m_sy + z;
			int sx = m_sx + y;

			if (cliprect.contains(sx, sy))
			{
				int color = (m_status & STATUS_DISPLAY_OFF) ? 0 : BIT(data, z % 8);

				bitmap.pix16(sy, sx) = color;
			}

			z++;
			z %= 32;
		}
	}
	return 0;
}
