// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

HD44102 Dot Matrix Liquid Crystal Graphic Display Column Driver

Not to be confused with HD44100.
Includes 4*50*8bit RAM.

TODO:
- properly emulate CS pins if needed? (there's 3, for enabling read and/or write)
- add BS pin when needed (4-bit mode)
- busy flag
- reset state
- what happens if Y address is invalid? (set to > 49)

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
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd44102_device - constructor
//-------------------------------------------------

hd44102_device::hd44102_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, HD44102, tag, owner, clock),
	m_sx(0),
	m_sy(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd44102_device::device_start()
{
	// zerofill
	m_status = 0;
	m_output = 0;
	m_page = 0;
	m_x = 0;
	m_y = 0;

	memset(m_ram, 0, sizeof(m_ram));
	memset(m_render_buf, 0, sizeof(m_render_buf));

	// register for state saving
	save_item(NAME(m_ram));
	save_item(NAME(m_render_buf));
	save_item(NAME(m_status));
	save_item(NAME(m_output));
	save_item(NAME(m_page));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_sx));
	save_item(NAME(m_sy));
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

u8 hd44102_device::read(offs_t offset)
{
	return (offset & 0x01) ? data_r() : status_r();
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void hd44102_device::write(offs_t offset, u8 data)
{
	(offset & 0x01) ? data_w(data) : control_w(data);
}


//-------------------------------------------------
//  status_r - status read
//-------------------------------------------------

u8 hd44102_device::status_r()
{
	return m_status;
}


//-------------------------------------------------
//  control_w - control write
//-------------------------------------------------

void hd44102_device::control_w(u8 data)
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
		break;
	}
}


//-------------------------------------------------
//  count_up_or_down -
//-------------------------------------------------

void hd44102_device::count_up_or_down()
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


//-------------------------------------------------
//  data_r - data read
//-------------------------------------------------

u8 hd44102_device::data_r()
{
	u8 data = m_output;
	m_output = m_ram[m_x][m_y];
	count_up_or_down();

	return data;
}


//-------------------------------------------------
//  data_w - data write
//-------------------------------------------------

void hd44102_device::data_w(u8 data)
{
	m_ram[m_x][m_y] = data;
	count_up_or_down();
}


//-------------------------------------------------
//  render - render the pixels
//-------------------------------------------------

const u8 *hd44102_device::render()
{
	memset(m_render_buf, 0, sizeof(m_render_buf));

	if (!(m_status & STATUS_DISPLAY_OFF))
	{
		for (int x = 0; x < 50; x++)
		{
			int z = m_page << 3;

			for (int y = 0; y < 32; y++)
			{
				m_render_buf[z * 50 + x] = BIT(m_ram[z >> 3][x], z & 7);
				z = (z + 1) & 0x1f;
			}
		}
	}

	return m_render_buf;
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

u32 hd44102_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 *src = render();

	for (int x = 0; x < 50; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			int sx = m_sx + x;
			int sy = m_sy + y;

			if (cliprect.contains(sx, sy))
				bitmap.pix(sy, sx) = src[y * 50 + x];
		}
	}

	return 0;
}
