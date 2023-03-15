// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    Casio GT913 keyboard controller (HLE)

    This is the keyboard controller portion of the GT913.
    The actual keyboard keys (as opposed to console buttons) have two
    contacts per key, which allows the controller to detect the velocity
    of the keypress. The detected velocity is read as a 7-bit value
    from the data port, along with the actual key scan code.

    Right now, velocity is just simulated using an (optional) analog
    control. The keyboard FIFO size is also basically a guess based on
    the CTK-551's 16-key polyphony.

***************************************************************************/

#include "emu.h"
#include "gt913_kbd.h"
#include "keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GT913_KBD_HLE, gt913_kbd_hle_device, "gt913_kbd_hle", "Casio GT913F keyboard controller (HLE)")

gt913_kbd_hle_device::gt913_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GT913_KBD_HLE, tag, owner, clock),
	device_matrix_keyboard_interface(mconfig, *this, "FI0", "FI1", "FI2", "FI3", "FI4", "FI5", "FI6", "FI7", "FI8", "FI9", "FI10", "KI0", "KI1", "KI2"),
	m_velocity(*this, "VELOCITY"),
	m_irq_cb(*this)
{
}

void gt913_kbd_hle_device::device_start()
{
	m_irq_cb.resolve_safe();

	save_item(NAME(m_status));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_read));
	save_item(NAME(m_fifo_write));
}

void gt913_kbd_hle_device::device_reset()
{
	m_status = 0x0000;
	std::memset(m_fifo, 0xff, sizeof(m_fifo));
	m_fifo_read = m_fifo_write = 0;

	reset_key_state();
	start_processing(attotime::from_hz(9600));
}

void gt913_kbd_hle_device::key_add(uint8_t row, uint8_t column, int state)
{
	m_fifo[m_fifo_write] = (row << 3) | (column & 7);
	if (state)
		m_fifo[m_fifo_write] |= 0x80;

	if (((m_fifo_write + 1) & 15) != m_fifo_read)
	{
		(++m_fifo_write) &= 15;
		update_status();
	}
}

void gt913_kbd_hle_device::update_status()
{
	if (m_fifo_read == m_fifo_write)
		m_status &= 0x7fff;
	else
		m_status |= 0x8000;

	if (BIT(m_status, 15) && BIT(m_status, 14))
		m_irq_cb(1);
	else
		m_irq_cb(0);
}

uint16_t gt913_kbd_hle_device::read()
{
	if (m_fifo_read == m_fifo_write)
		return 0xff00;

	uint16_t data = (m_fifo[m_fifo_read] << 8) | m_velocity.read_safe(0x7f);

	if (!machine().side_effects_disabled())
	{
		if (m_fifo_read != m_fifo_write)
		{
			(++m_fifo_read) &= 15;
			update_status();
		}
	}

	return data;
}

void gt913_kbd_hle_device::status_w(uint16_t data)
{
	m_status = data;
	update_status();
}
