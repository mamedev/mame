// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        SED1520 LCD controller

        TODO:
        - busy flag

***************************************************************************/

#include "emu.h"
#include "video/sed1520.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SED1520 = &device_creator<sed1520_device>;


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  sed1520_device - constructor
//-------------------------------------------------

sed1520_device::sed1520_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SED1520, "SED1520", tag, owner, clock, "sed1520", __FILE__), m_lcd_on(0), m_busy(0), m_page(0), m_column(0), m_old_column(0), m_start_line(0), 
	m_adc(0), m_static_drive(0), m_modify_write(false),
	m_screen_update_func(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sed1520_device::device_start()
{
	// state saving
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_busy));
	save_item(NAME(m_page));
	save_item(NAME(m_column));
	save_item(NAME(m_old_column));
	save_item(NAME(m_start_line));
	save_item(NAME(m_adc));
	save_item(NAME(m_static_drive));
	save_item(NAME(m_modify_write));
	save_item(NAME(m_vram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sed1520_device::device_reset()
{
	m_lcd_on = 0;
	m_busy = 0;
	m_page = 3;
	m_column = 0;
	m_old_column = 0;
	m_start_line = 0;
	m_adc = 1;
	m_static_drive = 0;
	m_modify_write = false;
	memset(m_vram, 0x00, sizeof(m_vram));
}


//**************************************************************************
//  device interface
//**************************************************************************

UINT32 sed1520_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_lcd_on)
	{
		if (m_screen_update_func)
			m_screen_update_func(*this, bitmap, cliprect, m_vram, m_start_line, m_adc);
	}
	else if (m_static_drive)
		return UPDATE_HAS_NOT_CHANGED;
	else
		bitmap.fill(0, cliprect);

	return 0;
}

READ8_MEMBER(sed1520_device::read)
{
	if (offset & 0x01)
		return data_read(space, 0);
	else
		return status_read(space, 0);
}

WRITE8_MEMBER(sed1520_device::write)
{
	if (offset & 0x01)
		data_write(space, 0, data);
	else
		control_write(space, 0, data);
}

WRITE8_MEMBER(sed1520_device::control_write)
{
	if((data & 0xfe) == 0xae)            // display on/off
		m_lcd_on = data & 0x01;
	else if((data & 0xe0) == 0xc0)       // set start line
		m_start_line = data & 0x1f;
	else if((data & 0xfc) == 0xb8)       // set page address
		m_page = data & 0x03;
	else if((data & 0x80) == 0x00)       // set column address
		m_column = data % 80;
	else if((data & 0xfe) == 0xa0)       // select ADC
		m_adc = data & 0x01;
	else if((data & 0xfe) == 0xa4)       // static drive on/off
		m_static_drive = data & 0x01;
	else if((data & 0xfe) == 0xa8)       // select duty
		;
	else if(data == 0xe0)                // read-modify-write on
	{
		m_modify_write = true;
		m_old_column = m_column;
	}
	else if(data == 0xee)                // read-modify-write off
	{
		m_modify_write = false;
		m_column = m_old_column;
	}
	else if(data == 0xe2)                // reset
	{
		m_start_line = m_column = 0;
		m_page = 3;
	}
	else
		logerror("%s: invalid SED1520 command: %x\n", tag(), data);
}

READ8_MEMBER(sed1520_device::status_read)
{
	UINT8 data = (m_busy << 7) | (m_adc << 6) | (m_lcd_on << 5);
	return data;
}

WRITE8_MEMBER(sed1520_device::data_write)
{
	m_vram[(m_page * 80 + m_column) % sizeof(m_vram)] = data;
	m_column = (m_column + 1) % 80;
}

READ8_MEMBER(sed1520_device::data_read)
{
	UINT8 data = m_vram[(m_page * 80 + m_column) % sizeof(m_vram)];
	if (!m_modify_write)
		m_column = (m_column + 1) % 80;
	return data;
}
