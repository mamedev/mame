// license:BSD-3-Clause
// copyright-holders:Felipe Sanches, Sandro Ronco
/***************************************************************************

        Novatek NT7534 LCD controller

        TODO:
        - determine video timings and busy flag duration

***************************************************************************/

#include "emu.h"
#include "nt7534.h"

#define VERBOSE 0
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NT7534,    nt7534_device,    "nt7534", "NT7534 LCD Controller")


//-------------------------------------------------
//  nt7534_device - constructor
//-------------------------------------------------

nt7534_device::nt7534_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nt7534_device(mconfig, NT7534, tag, owner, clock)
{
}

nt7534_device::nt7534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_pixel_update_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nt7534_device::device_start()
{
	m_busy_timer = timer_alloc(FUNC(nt7534_device::clear_busy_flag), this);

	m_pixel_update_cb.resolve();

	// state saving
	save_item(NAME(m_busy_flag));
	save_item(NAME(m_page));
	save_item(NAME(m_column));
	save_item(NAME(m_dr));
	save_item(NAME(m_ir));
	save_item(NAME(m_display_on));
	save_item(NAME(m_display_start_line));
	save_item(NAME(m_direction));
	save_item(NAME(m_data_len));
	save_item(NAME(m_ddram));
	save_item(NAME(m_nibble));
	save_item(NAME(m_rs_state));
	save_item(NAME(m_rw_state));
	save_item(NAME(m_backup_column));
	save_item(NAME(m_read_modify_write));
	save_item(NAME(m_adc));
	save_item(NAME(m_reverse));
	save_item(NAME(m_entire_display_on));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nt7534_device::device_reset()
{
	memset(m_ddram, 0x00, sizeof(m_ddram));

	m_page       = 0;
	m_column     = 0;
	m_dr         = 0;
	m_ir         = 0;
	m_display_on = false;
	m_entire_display_on = false;
	m_direction  = 1;
	m_adc        = true;
	m_read_modify_write = false;
	m_reverse  = false;
	m_data_len   = 8;
	m_nibble     = false;
	m_rs_state   = 0;
	m_rw_state   = 0;

	set_busy_flag(1520);
}


//-------------------------------------------------
//  clear_busy_flag -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(nt7534_device::clear_busy_flag)
{
	m_busy_flag = false;
}


//**************************************************************************
//  HELPERS
//**************************************************************************

void nt7534_device::set_busy_flag(uint16_t usec)
{
	m_busy_flag = true;
	m_busy_timer->adjust( attotime::from_usec( usec ) );
}

void nt7534_device::update_nibble(int rs, int rw)
{
	if (m_rs_state != rs || m_rw_state != rw)
	{
		m_rs_state = rs;
		m_rw_state = rw;
		m_nibble = false;
	}

	m_nibble = !m_nibble;
}

//**************************************************************************
//  device interface
//**************************************************************************

uint32_t nt7534_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (uint16_t x = 0; x < 132; x++)
	{
		uint16_t px = m_adc ? x : 131 - x;
		for (uint8_t y = 0; y < 65; y++)
		{
			uint8_t py = (y + m_display_start_line - 32) % 65;
			uint8_t page = py/8;
			bitmap.pix(y, x) = BIT(m_ddram[page*132 + px], py%8);
		}
	}

	return 0;
}

uint8_t nt7534_device::read(offs_t offset)
{
	switch (offset & 0x01)
	{
		case 0: return control_read();
		case 1: return data_read();
	}

	return 0;
}

void nt7534_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x01)
	{
		case 0: control_write(data);  break;
		case 1: data_write(data);     break;
	}
}

void nt7534_device::control_write(uint8_t data)
{
	if (m_data_len == 4)
	{
		update_nibble(0, 0);

		if (m_nibble)
		{
			m_ir = data & 0xF0;
			return;
		}
		else
		{
			m_ir |= ((data >> 4) & 0x0F);
		}
	}
	else
	{
		m_ir = data;
	}

		if (m_ir == 0xE2)
	{
		// Reset
		memset(m_ddram, 0x00, sizeof(m_ddram));
		m_page = 0;
		m_column = 0;
		LOG("Reset \n");
		return;
	}
		else if ((m_ir & 0xFE) == 0xAE)
	{
		// Display ON/OFF
		m_display_on = m_ir & 1;
		LOG("Display %s\n", m_display_on ? "ON" : "OFF");
		return;
	}
		else if ((m_ir & 0xC0) == 0x40)
	{
		// Display Start Line Set
		m_display_start_line = m_ir & 0x3F;
		LOG("Display Start Line: %d\n", m_display_start_line);
		return;
	}
	else if ((m_ir & 0xF0) == 0xB0)
	{
		// set Page Address
		m_page = m_ir & 0x0F;
		if (m_page <= 0x08)
			LOG("Set Page address %x\n", m_page);
		else
			LOG("Set Page address %x (invalid)\n", m_page);
		return;
	}
	else if ((m_ir & 0xF0) == 0x10)
	{
		// Set column address MSB
		m_column = (m_column & 0x0F) | ((m_ir & 0x0F) << 4);

		LOG("Set column address MSB %x\n", (m_column >> 4) & 0x0F);
		return;
	}
	else if ((m_ir & 0xF0) == 0x00)
	{
		// Set column address LSB
		m_column = (m_column & 0xF0) | (m_ir & 0x0F);

		LOG("Set column address LSB %x\n", m_column & 0x0F);
		return;
	}
	else if ((m_ir & 0xFE) == 0xA0)
	{
		// ADC Select
		m_adc = m_ir & 1;
		LOG("ADC: %d\n", m_adc);
		return;
	}
	else if ((m_ir & 0xFE) == 0xA6)
	{
		// Normal/Reverse Display
		m_reverse = m_ir & 1;
		LOG("Display Reverse ? %s\n", m_reverse ? "Yes" : "No");
		return;
	}
	else if ((m_ir & 0xFE) == 0xA4)
	{
		// Entire display ON
		m_entire_display_on = m_ir & 1;
		LOG("Entire Display ON ? %s\n", m_entire_display_on ? "Yes" : "No");
		return;
	}
	else if (m_ir == 0xE0)
	{
		// Enable Read-Modify-Write
		m_read_modify_write = true;
		m_backup_column = m_column;
		LOG("Enable Read-Modify-Write. Backup column: %d\n", m_backup_column);
		return;
	}
	else if (m_ir == 0xEE)
	{
		// Disable Read-Modify-Write
		m_read_modify_write = false;
		m_column = m_backup_column; // restore column value
		LOG("Disable Read-Modify-Write.\n");
		return;
	}
}

uint8_t nt7534_device::control_read()
{
	if (m_data_len == 4)
	{
		if (!machine().side_effects_disabled())
			update_nibble(0, 1);

		if (m_nibble)
			return (m_busy_flag ? 0x80 : 0);
		else
			return 0; //TODO: review this.
	}
	else
	{
		return (m_busy_flag ? 0x80 : 0);
	}
}

void nt7534_device::data_write(uint8_t data)
{
//  if (m_busy_flag)
//  {
//      logerror("Ignoring data write %02x due to busy flag\n", data);
//      return;
//  }

	if (m_data_len == 4)
	{
		update_nibble(1, 0);

		if (m_nibble)
		{
			m_dr = data & 0xF0;
			return;
		}
		else
		{
			m_dr |= ((data >> 4) & 0x0F);
		}
	}
	else
	{
		m_dr = data;
	}

	LOG("RAM write %x %x '%c'\n", m_page*132 + m_column, m_dr, isprint(m_dr) ? m_dr : '.');

	if (m_page*132 + m_column < std::size(m_ddram))
		m_ddram[m_page*132 + m_column] = m_dr;

	if (m_column < 131)
		m_column++;

	set_busy_flag(41);
}

uint8_t nt7534_device::data_read()
{
	if (m_page*132 + m_column >= std::size(m_ddram))
		return 0;

	uint8_t data = m_ddram[m_page*132 + m_column];
	if (m_read_modify_write == false && m_column < 131)
		m_column++;

	if (m_data_len == 4)
	{
		if (!machine().side_effects_disabled())
			update_nibble(1, 1);

		if (m_nibble)
			return data & 0xF0;
		else
			data = (data << 4) & 0xF0;
	}

	return data;
}
