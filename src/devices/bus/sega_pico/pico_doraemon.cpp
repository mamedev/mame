// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

   Doraemon figure peripheral, required by Sega Pico game "HPC-6081 - Itsudemo Issho Doraemon Set (Japan)"

   A hidden test mode can be activated by covering sensors for pages 1, 3, and 5,
   while leaving the other sensors exposed. If the machine configuration "Test Mode Pages" is enabled,
   the driver forces this page setup.

***************************************************************************/

#include "emu.h"

#include "pico_doraemon.h"
#include "pico_ps2.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PICO_DORAEMON, pico_doraemon_device, "pico_doraemon", "Pico Doraemon")

static INPUT_PORTS_START( pico_doraemon )
	PORT_START("BUTTONS")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Left Hand")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Right Hand")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Pocket")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Nose")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Microphone")
INPUT_PORTS_END

ioport_constructor pico_doraemon_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pico_doraemon );
}

pico_doraemon_device::pico_doraemon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PICO_DORAEMON, tag, owner, clock)
	, device_pico_ps2_slot_interface(mconfig, *this)
	, m_io_buttons(*this, "BUTTONS")
{
}

void pico_doraemon_device::device_start()
{
}

void pico_doraemon_device::device_reset()
{
	m_i = 0;
	m_data = 0;
	m_prev_data = 0;
	m_is_write_done = false;
}

void pico_doraemon_device::device_add_mconfig(machine_config &config)
{
}

uint8_t pico_doraemon_device::ps2_r(offs_t offset)
{
	// All commands expect bit 5 to be set when battery is high,
	// otherwise a warning screen is shown
	uint8_t state = 0x20;

	if (m_prev_data == BUTTON_STATUS)
	{
		state |= m_io_buttons->read() & 0x1f;
	}
	uint8_t n = m_i > 8 ? 7 : (m_i - 1);

	// Send one bit at a time
	return (state & (1 << n)) != 0;
}

void pico_doraemon_device::ps2_w(offs_t offset, uint8_t data)
{
	switch (data)
	{
		case 0x00:
			// Fallthrough
		case 0x20:
			if (m_is_write_done)
			{
				m_is_write_done = false;
				m_i = 0;
			}
			if (data == 0x20)
			{
				m_data |= (1 << m_i);
			}
			break;
		case 0x40:
			// Fallthrough
		case 0x60:
			// Last bit of command is always terminated with an additional write of 0x60
			if (!m_is_write_done)
			{
				m_i++;
				if (m_i == 8)
				{
					m_is_write_done = true;
					m_prev_data = m_data;
					m_data = 0;
					LOG("ps2_w cmd=%02x\n", m_prev_data);
				}
			}
	}
}
