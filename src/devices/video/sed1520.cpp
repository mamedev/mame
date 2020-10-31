// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        SED1520 LCD controller
        SED1560 LCD controller

        TODO:
        - busy flag

***************************************************************************/

#include "emu.h"
#include "video/sed1520.h"

#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SED1520, sed1520_device, "sed1520", "Epson SED1520")
DEFINE_DEVICE_TYPE(SED1560, sed1560_device, "sed1560", "Epson SED1560")


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  sed1520_device - constructor
//-------------------------------------------------

sed15xx_device_base::sed15xx_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t ddr_size, uint32_t page_size)
	: device_t(mconfig, type, tag, owner, clock)
	, m_ddr_size(ddr_size)
	, m_page_size(page_size)
	, m_lcd_on(0)
	, m_busy(0)
	, m_page(0)
	, m_column(0)
	, m_old_column(0)
	, m_start_line(0)
	, m_adc(0)
	, m_modify_write(false)
	, m_data(0)
	, m_duty(0)
{
}

sed1520_device::sed1520_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed15xx_device_base(mconfig, SED1520, tag, owner, clock, 320, 80)     // 2560-bit display RAM
	, m_screen_update_cb(*this)
{
}

sed1560_device::sed1560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed15xx_device_base(mconfig, SED1560, tag, owner, clock, 1349, 166)   // 166 × 65-bit display RAM
	, m_screen_update_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sed15xx_device_base::device_start()
{
	m_ddr = std::make_unique<uint8_t[]>(m_ddr_size);

	// state saving
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_busy));
	save_item(NAME(m_page));
	save_item(NAME(m_column));
	save_item(NAME(m_old_column));
	save_item(NAME(m_start_line));
	save_item(NAME(m_adc));
	save_item(NAME(m_modify_write));
	save_item(NAME(m_data));
	save_item(NAME(m_duty));
	save_pointer(NAME(m_ddr), m_ddr_size);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sed15xx_device_base::device_reset()
{
	m_lcd_on = 0;
	m_busy = 0;
	m_page = 3;
	m_column = 0;
	m_old_column = 0;
	m_start_line = 0;
	m_adc = 1;
	m_modify_write = false;
	m_data = 0;
	m_duty = 0;
	std::fill_n(m_ddr.get(), m_ddr_size, 0x00);
}

void sed1520_device::device_resolve_objects()
{
	m_screen_update_cb.resolve();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sed1520_device::device_start()
{
	sed15xx_device_base::device_start();

	// state saving
	save_item(NAME(m_static_drive));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sed1520_device::device_reset()
{
	sed15xx_device_base::device_reset();

	m_page = 3;
	m_static_drive = false;
}


void sed1560_device::device_resolve_objects()
{
	m_screen_update_cb.resolve();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sed1560_device::device_start()
{
	sed15xx_device_base::device_start();

	// state saving
	save_item(NAME(m_contrast));
	save_item(NAME(m_reverse));
	save_item(NAME(m_fill));
	save_item(NAME(m_line_inv));
	save_item(NAME(m_line_inv_num));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sed1560_device::device_reset()
{
	sed15xx_device_base::device_reset();

	m_page = 0;
	m_adc = 0;
	m_contrast = 0;
	m_reverse = false;
	m_fill = false;
	m_line_inv_num = 16;
	m_line_inv = false;
}

//**************************************************************************
//  device interface
//**************************************************************************

uint8_t sed15xx_device_base::read(offs_t offset)
{
	if (offset & 0x01)
		return data_read();
	else
		return status_read();
}

void sed15xx_device_base::write(offs_t offset, uint8_t data)
{
	if (offset & 0x01)
		data_write(data);
	else
		control_write(data);
}

uint8_t sed15xx_device_base::status_read()
{
	return (m_busy << 7) | (m_adc << 6) | (m_lcd_on << 5);
}

void sed15xx_device_base::data_write(uint8_t data)
{
	m_ddr[(m_page * m_page_size + m_column) % m_ddr_size] = data;
	if (m_column < m_page_size)
		m_column++;
}

uint8_t sed15xx_device_base::data_read()
{
	uint8_t data = m_data;
	m_data = m_ddr[(m_page * m_page_size + m_column) % m_ddr_size];
	if (!m_modify_write && m_column < m_page_size)
		m_column++;

	return data;
}


void sed1520_device::control_write(uint8_t data)
{
	if ((data & 0xfe) == 0xae)           // Display ON/OFF
		m_lcd_on = data & 0x01;
	else if ((data & 0xe0) == 0xc0)      // Set start line
		m_start_line = data & 0x1f;
	else if ((data & 0xfc) == 0xb8)      // Set page address
		m_page = data & 0x03;
	else if ((data & 0x80) == 0x00)      // Set column address
		m_column = data & 0x7f;
	else if ((data & 0xfe) == 0xa0)      // Select ADC
		m_adc = data & 0x01;
	else if ((data & 0xfe) == 0xa4)      // Static drive ON/OFF
		m_static_drive = data & 0x01;
	else if ((data & 0xfe) == 0xa8)      // Select duty
		m_duty = data & 0x01;
	else if (data == 0xe0)               // Read Modify Write ON
	{
		m_modify_write = true;
		m_old_column = m_column;
	}
	else if (data == 0xee)               // Read Modify Write OFF
	{
		m_modify_write = false;
		m_column = m_old_column;
	}
	else if (data == 0xe2)               // Reset
	{
		m_start_line = m_column = 0;
		m_page = 3;
	}
	else
		logerror("%s: invalid SED1520 command: %x\n", tag(), data);
}


void sed1560_device::control_write(uint8_t data)
{
	if ((data & 0xfe) == 0xae)           // Display ON/OFF
		m_lcd_on = BIT(data, 0);
	else if ((data & 0xc0) == 0x40)      // Initial display line
		m_start_line = data & 0x3f;
	else if ((data & 0xf0) == 0xb0)      // Set page
		m_page = data & 0x0f;
	else if ((data & 0xf0) == 0x00)      // Column address low nibble
		m_column = (m_column & 0xf0) | (data & 0x0f);
	else if ((data & 0xf0) == 0x10)      // Column address high nibble
		m_column = (m_column & 0x0f) | ((data << 4) & 0xf0);
	else if ((data & 0xfe) == 0xa0)      // Select ADC
		m_adc = BIT(data, 0);
	else if ((data & 0xfe) == 0xa6)      // Normal/reverse display
		m_reverse = BIT(data, 0);
	else if ((data & 0xfe) == 0xa4)      // Display all points ON/OFF
		m_fill = BIT(data, 0);
	else if ((data & 0xfe) == 0xa8)      // Select duty
		m_duty = (m_duty & 0x02) | (data & 0x01);
	else if ((data & 0xfe) == 0xaa)      // Duty + 1
		m_duty = (m_duty & 0x01) | ((data & 0x01) << 1);
	else if ((data & 0xf0) == 0x30)      // Set n-line inversion
	{
		m_line_inv = true;
		m_line_inv_num = data & 0x0f;
	}
	else if (data == 0x20)               // Cancel n-line inversion
		m_line_inv = false;
	else if (data == 0xe2)               // Reset
	{
		m_start_line = 0;
		m_column = 0;
		m_page = 0;
		m_line_inv_num = 16;
	}
	else if (data == 0xe0)               // Read Modify Write
	{
		m_modify_write = true;
		m_old_column = m_column;
	}
	else if (data == 0xee)               // End Modify Write
	{
		m_modify_write = false;
		m_column = m_old_column;
	}
	else if (data == 0xed)               // Power-on completion
		logerror("%s: Power-on completion\n", tag());
	else if ((data & 0xf0) == 0xc0)      // Output status set
		logerror("%s: Output status set %x\n", tag(), data & 0x0f);
	else if ((data & 0xfe) == 0x24)      // LCD power supply ON/OFF
		logerror("%s: LCD power supply %d\n", tag(), data & 0x01);
	else if ((data & 0xe0) == 0x80)      // Software contrast setting
		m_contrast = data;
	else
		logerror("%s: invalid SED1560 command: %x\n", tag(), data);
}


uint32_t sed1520_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_static_drive)
		return UPDATE_HAS_NOT_CHANGED;

	return m_screen_update_cb(bitmap, cliprect, m_lcd_on, m_ddr.get(), m_start_line, m_adc, m_duty);
}

uint32_t sed1560_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_screen_update_cb(bitmap, cliprect, m_lcd_on, m_ddr.get(), m_start_line, m_adc, m_duty, m_reverse, m_fill, m_contrast, m_line_inv, m_line_inv_num);
}
