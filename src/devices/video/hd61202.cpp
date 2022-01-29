// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

        Hitachi HD61202 LCD Driver

**********************************************************************/

#include "emu.h"
#include "hd61202.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(HD61202, hd61202_device, "hd61202", "Hitachi HD61202 LCD Driver")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd61202_device - constructor
//-------------------------------------------------

hd61202_device::hd61202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HD61202, tag, owner, clock)
	, m_screen_update_cb(*this)
{
}

void hd61202_device::device_resolve_objects()
{
	m_screen_update_cb.resolve();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd61202_device::device_start()
{
	save_item(NAME(m_bf));
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_out_data));
	save_item(NAME(m_page));
	save_item(NAME(m_addr));
	save_item(NAME(m_start_line));
	save_item(NAME(m_ddr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd61202_device::device_reset()
{
	m_bf = 0;
	m_lcd_on = 0;
	m_out_data = 0;
	m_page = 0;
	m_addr = 0;
	m_start_line = 0;
	std::fill(std::begin(m_ddr), std::end(m_ddr), 0);
}


//-------------------------------------------------
//  status_r - status register read
//-------------------------------------------------

uint8_t hd61202_device::status_r()
{
	// x--- ---- Busy
	// --x- ---- LCD on/off

	uint8_t data = 0;

	data |= (m_lcd_on << 5);
	data |= (m_bf << 7);

	return data;
}


//-------------------------------------------------
//  control_w - instruction register write
//-------------------------------------------------

void hd61202_device::control_w(uint8_t data)
{
	if ((data & 0xfe) == 0x3e)            // Display on/off
		m_lcd_on = data & 0x01;
	else if ((data & 0xf8) == 0xb8)       // Set page
		m_page = data & 0x07;
	else if ((data & 0xc0) == 0x40)       // Set address
		m_addr = data & 0x3f;
	else if ((data & 0xc0) == 0xc0)       // Set display start line
		m_start_line = data & 0x3f;
}


//-------------------------------------------------
//  data_r - data register read
//-------------------------------------------------

uint8_t hd61202_device::data_r()
{
	uint8_t data = m_out_data;
	m_out_data = m_ddr[(m_page * 0x40 + m_addr) & 0x1ff];
	m_addr++;

	return data;
}


//-------------------------------------------------
//  data_w - data register write
//-------------------------------------------------

void hd61202_device::data_w(uint8_t data)
{
	m_ddr[(m_page * 0x40 + m_addr) & 0x1ff] = data;
	m_addr++;
}

//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

uint32_t hd61202_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_screen_update_cb(bitmap, cliprect, m_lcd_on, m_start_line, m_ddr);
}
