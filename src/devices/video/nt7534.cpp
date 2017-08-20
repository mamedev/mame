// license:BSD-3-Clause
// copyright-holders:Felipe Sanches, Sandro Ronco
/***************************************************************************

        NT7534 LCD controller

        TODO:
        - emulate osc pin, determine video timings and busy flag duration from it

***************************************************************************/

#include "emu.h"
#include "video/nt7534.h"

//#define VERBOSE 1
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
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nt7534_device::device_start()
{
	m_busy_timer = timer_alloc(TIMER_BUSY);

	// state saving
	save_item(NAME(m_busy_flag));
	save_item(NAME(m_page));
	save_item(NAME(m_column));
	save_item(NAME(m_dr));
	save_item(NAME(m_ir));
	save_item(NAME(m_display_on));
	save_item(NAME(m_direction));
	save_item(NAME(m_data_len));
	save_item(NAME(m_ddram));
	save_item(NAME(m_nibble));
	save_item(NAME(m_rs_state));
	save_item(NAME(m_rw_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nt7534_device::device_reset()
{
	memset(m_ddram, 0x00, sizeof(m_ddram));

	m_dr         = 0;
	m_ir         = 0;
	m_display_on = false;
	m_direction  = 1;
	m_data_len   = 8;
	m_nibble     = false;
	m_rs_state   = 0;
	m_rw_state   = 0;

	set_busy_flag(1520);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nt7534_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_BUSY:
			m_busy_flag = false;
			break;
	}
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

	for (uint16_t x = 0; x < 132; x++){
		for (uint8_t y = 0; y < 65; y++){
			uint8_t page = y/8;
			if(y==64 || x==80){
				bitmap.pix16(y, x) = 1;
			} else {
				bitmap.pix16(y, x) = BIT(m_ddram[page*132 + x], y%8);
			}
		}
	}

	return 0;
}

READ8_MEMBER(nt7534_device::read)
{
	switch (offset & 0x01)
	{
		case 0: return control_read(space, 0);
		case 1: return data_read(space, 0);
	}

	return 0;
}

WRITE8_MEMBER(nt7534_device::write)
{
	switch (offset & 0x01)
	{
		case 0: control_write(space, 0, data);  break;
		case 1: data_write(space, 0, data);     break;
	}
}

WRITE8_MEMBER(nt7534_device::control_write)
{
	if (m_data_len == 4)
	{
		update_nibble(0, 0);

		if (m_nibble)
		{
			m_ir = data & 0xf0;
			return;
		}
		else
		{
			m_ir |= ((data >> 4) & 0x0f);
		}
	}
	else
	{
		m_ir = data;
	}

        if (m_ir == 0xE2){
		// Reset
		memset(m_ddram, 0x00, sizeof(m_ddram));
		m_page = 0;
		m_column = 0;
		LOG("NT7534: Reset \n");
		return;
	}
        else if ((m_ir & 0xFE) == 0xAE){
		// Display ON/OFF
		m_display_on = m_ir & 1;
		LOG("NT7534: Display %s\n", m_display_on ? "ON" : "OFF");
		return;
	}
	else if ((m_ir & 0xf0) == 0xb0)
	{
		// set Page Address
		m_page = m_ir & 0x0f;

		LOG("NT7534: set Page address %x\n", m_page);
		return;
	}
	else if ((m_ir & 0xf0) == 0x00)
	{
		// Set column address MSB
		m_column = (m_column & 0x0f) | ((m_ir & 0x0f) << 4);

		LOG("NT7534: set column address MSB %x\n", (m_column >> 4) & 0x0f);
		return;
	}
	else if ((m_ir & 0xf0) == 0x10)
	{
		// Set column address LSB
		m_column = (m_column & 0xf0) | (m_ir & 0x0f);

		LOG("NT7534: set column address LSB %x\n", m_column & 0x0f);
		return;
	}
}

READ8_MEMBER(nt7534_device::control_read)
{
	if (m_data_len == 4)
	{
		if (!machine().side_effect_disabled())
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

WRITE8_MEMBER(nt7534_device::data_write)
{
//	if (m_busy_flag)
//	{
//		logerror("NT7534: Ignoring data write %02x due to busy flag\n", data);
//		return;
//	}

	if (m_data_len == 4)
	{
		update_nibble(1, 0);

		if (m_nibble)
		{
			m_dr = data & 0xf0;
			return;
		}
		else
		{
			m_dr |= ((data >> 4) & 0x0f);
		}
	}
	else
	{
		m_dr = data;
	}

	LOG("NT7534: RAM write %x %x '%c'\n", m_page*132+m_column, m_dr, isprint(m_dr) ? m_dr : '.');

	m_ddram[m_page*132 + m_column] = m_dr;

	if (m_column < 131){
		m_column++;
	}

	set_busy_flag(41);
}

READ8_MEMBER(nt7534_device::data_read)
{
	uint8_t data = m_ddram[m_page*132+m_column];
	if (m_column < 131){
		m_column++;
	}

	if (m_data_len == 4)
	{
		if (!machine().side_effect_disabled())
			update_nibble(1, 1);

		if (m_nibble)
			return data & 0xf0;
		else
			data = (data << 4) & 0xf0;
	}

	return data;
}
