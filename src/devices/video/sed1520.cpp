// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    SED1520 LCD controller
    SED1560 LCD controller
    EPL43102 LCD controller
    NT7502 LCD controller

    TODO:
    - busy flag

***************************************************************************/

#include "emu.h"
#include "sed1520.h"

#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SED1520, sed1520_device, "sed1520", "Epson SED1520 LCD Driver")
DEFINE_DEVICE_TYPE(SED1560, sed1560_device, "sed1560", "Epson SED1560 LCD Driver")
DEFINE_DEVICE_TYPE(EPL43102, epl43102_device, "epl43102", "Elan EPL43102 LCD Driver")
DEFINE_DEVICE_TYPE(NT7502, nt7502_device, "nt7502", "Novatek NT7502 LCD Driver")


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

sed1560_device::sed1560_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t ddr_size, uint32_t page_size)
	: sed15xx_device_base(mconfig, type, tag, owner, clock, ddr_size, page_size)
	, m_screen_update_cb(*this)
{
}

sed1560_device::sed1560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1560_device(mconfig, SED1560, tag, owner, clock, 1349, 166)   // 166 × 65-bit display RAM
{
}

epl43102_device::epl43102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1560_device(mconfig, EPL43102, tag, owner, clock, 714, 102)   // 102 × 43-bit display RAM (TODO: page map is not straightforward)
{
}

nt7502_device::nt7502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1560_device(mconfig, NT7502, tag, owner, clock, 1188, 166)   // 132 × 65-bit display RAM
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

void epl43102_device::device_start()
{
	sed1560_device::device_start();

	// state saving
	save_item(NAME(m_last_command));
}

void nt7502_device::device_start()
{
	sed1560_device::device_start();

	// state saving
	save_item(NAME(m_last_command));
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

void epl43102_device::device_reset()
{
	sed1560_device::device_reset();

	m_last_command = 0;
}

void nt7502_device::device_reset()
{
	sed1560_device::device_reset();

	m_last_command = 0;
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
	m_data = data;

	// no RAM write when column address is invalid
	if (m_column < m_page_size)
	{
		m_ddr[(m_page * m_page_size + m_column) % m_ddr_size] = data;
		m_column++;
	}
}

uint8_t sed15xx_device_base::data_read()
{
	uint8_t data = m_data;
	if (machine().side_effects_disabled())
		return data;

	if (m_column < m_page_size)
	{
		m_data = m_ddr[(m_page * m_page_size + m_column) % m_ddr_size];
		if (!m_modify_write)
			m_column++;
	}
	else
	{
		// invalid column, not sure what happens with data latch
		m_data = 0;
	}

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
		logerror("%s: invalid SED1520 command: %x\n", machine().describe_context(), data);
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
		logerror("%s: Power-on completion\n", machine().describe_context());
	else if ((data & 0xf0) == 0xc0)      // Output status set
		logerror("%s: Output status set %x\n", machine().describe_context(), data & 0x0f);
	else if ((data & 0xfe) == 0x24)      // LCD power supply ON/OFF
		logerror("%s: LCD power supply %d\n", machine().describe_context(), data & 0x01);
	else if ((data & 0xe0) == 0x80)      // Software contrast setting
		m_contrast = data;
	else
		logerror("%s: invalid SED1560 command: %x\n", machine().describe_context(), data);
}


void epl43102_device::control_write(uint8_t data)
{
	switch (m_last_command)
	{
	case 0x81:
		m_contrast = data & 0x3f;
		m_last_command = 0;
		break;

	case 0x82:
		logerror("%s: CL frequency = fOSC / %d\n", machine().describe_context(), BIT(data, 4) ? 32 : (data & 0x0f) + 1);
		m_last_command = 0;
		break;

	case 0x84:
		logerror("%s: Duty ratio = %d%s\n", machine().describe_context(), ((data & 0x07) + 1) * 8, BIT(data, 3) ? " + ICON" : "");
		m_duty = data & 0x07;
		m_last_command = 0;
		break;

	case 0x85:
		logerror("%s: LCD bias = %d%s\n", machine().describe_context(), ((data & 0x0e) >> 1) + 3, BIT(data, 0) ? ".5" : "");
		m_last_command = 0;
		break;

	case 0xad:
		if ((data & 0x03) == 0)
			logerror("%s: Status indicator off\n", machine().describe_context());
		else
			logerror("%s: Status indicator on (%s)\n", machine().describe_context(), (data & 0x03) == 0x01 ? "4-frame blinking" : (data & 0x03) == 0x02 ? "2-frame blinking" : "continuously");
		m_last_command = 0;
		break;

	default:
		if (data == 0x81 || data == 0x82 || data == 0x84 || data == 0x85 || data == 0xad)
		{
			// 2-byte instructions
			m_last_command = data;
		}
		else if ((data & 0xf0) == 0xc0)
			logerror("%s: COM output direction = %s\n", machine().describe_context(), BIT(data, 3) ? "reverse" : "normal");
		else if ((data & 0xf8) == 0x28)
			logerror("%s: Voltage converter %s, regulator %s, follower %s\n", machine().describe_context(), BIT(data, 2) ? "on" : "off", BIT(data, 1) ? "on" : "off", BIT(data, 0) ? "on" : "off");
		else if ((data & 0xf8) == 0x20)
			logerror("%s: Regulator resistor select = %d\n", machine().describe_context(), data & 0x07);
		else
			sed1560_device::control_write(data);
		break;
	}
}


void nt7502_device::control_write(uint8_t data)
{
	switch (m_last_command)
	{
	case 0x81:
		m_contrast = data & 0x3f;
		m_last_command = 0;
		break;

	case 0xad:
		if ((data & 0x03) == 0)
			logerror("%s: Status indicator off\n", machine().describe_context());
		else
			logerror("%s: Status indicator on (%s)\n", machine().describe_context(), (data & 0x03) == 0x01 ? "0.5-second blinking" : (data & 0x03) == 0x02 ? "1-second blinking" : "continuously");
		m_last_command = 0;
		break;

	default:
		if (data == 0x81 || data == 0xad)
		{
			// 2-byte instructions
			m_last_command = data;
		}
		else if ((data & 0xfe) == 0xa2)
			logerror("%s: LCD voltage bias = %s\n", machine().describe_context(), BIT(data, 0) ? "1/5, 1/6 or 1/7" : "1/7, 1/8 or 1/9");
		else if ((data & 0xf0) == 0xc0)
			logerror("%s: COM output direction = %s\n", machine().describe_context(), BIT(data, 3) ? "reverse" : "normal");
		else if ((data & 0xf8) == 0x28)
			logerror("%s: Voltage booster %s, regulator %s, follower %s\n", machine().describe_context(), BIT(data, 2) ? "on" : "off", BIT(data, 1) ? "on" : "off", BIT(data, 0) ? "on" : "off");
		else if ((data & 0xf8) == 0x20)
			logerror("%s: V0 voltage regulator internal resistor select = %d\n", machine().describe_context(), data & 0x07);
		else if (data == 0xac)
			logerror("%s: Static indicator off\n", machine().describe_context());
		else if (data == 0xe3)
			/* NOP command */;
		else if ((data & 0xf0) == 0xf0)
			logerror("%s: Test mode %s (%02X)\n", machine().describe_context(), data == 0xf0 ? "reset" : "on", data);
		else
			sed1560_device::control_write(data);
		break;
	}
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
