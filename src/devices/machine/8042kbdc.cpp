// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*********************************************************************

    8042kbdc.cpp

    8042-based keyboard/mouse controller simulation

*********************************************************************/


#include "emu.h"
#include "machine/8042kbdc.h"


/***************************************************************************

    Constants & macros

***************************************************************************/

#define PS2_MOUSE_ON    1
#define KEYBOARD_ON     1

#define LOG_KEYBOARD    0
#define LOG_ACCESSES    0

DEFINE_DEVICE_TYPE(KBDC8042, kbdc8042_device, "kbdc8042", "8042 Keyboard/Mouse Controller")

//-------------------------------------------------
//  kbdc8042_device - constructor
//-------------------------------------------------

kbdc8042_device::kbdc8042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KBDC8042, tag, owner, clock)
	, m_keyboard_dev(*this, "at_keyboard")
	, m_mousex_port(*this, "MOUSEX")
	, m_mousey_port(*this, "MOUSEY")
	, m_mousebtn_port(*this, "MOUSEBTN")
	, m_system_reset_cb(*this)
	, m_gate_a20_cb(*this)
	, m_input_buffer_full_cb(*this)
	, m_input_buffer_full_mouse_cb(*this)
	, m_output_buffer_empty_cb(*this)
	, m_speaker_cb(*this)
{
	m_keybtype = KBDC8042_STANDARD;
	m_interrupttype = KBDC8042_SINGLE;
}

void kbdc8042_device::device_add_mconfig(machine_config &config)
{
	AT_KEYB(config, m_keyboard_dev, pc_keyboard_device::KEYBOARD_TYPE::AT, 1);
	m_keyboard_dev->keypress().set(FUNC(kbdc8042_device::keyboard_w));
}


/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void kbdc8042_device::device_start()
{
	// resolve callbacks
	m_system_reset_cb.resolve_safe();
	m_gate_a20_cb.resolve_safe();
	m_input_buffer_full_cb.resolve_safe();
	m_input_buffer_full_mouse_cb.resolve_safe();
	m_output_buffer_empty_cb.resolve_safe();
	m_speaker_cb.resolve_safe();
	m_operation_write_state = 0; /* first write to 0x60 might occur before anything can set this */
	memset(&m_keyboard, 0x00, sizeof(m_keyboard));
	memset(&m_mouse, 0x00, sizeof(m_mouse));
	m_mouse.sample_rate = 100;
	m_mouse.resolution = 3;
	m_mouse.on = true;
	m_sending = 0;
	m_last_write_to_control = 0;
	m_status_read_mode = 0;
	m_speaker = 0;
	m_offset1 = 0;

	m_update_timer = timer_alloc(TIMER_UPDATE);
	m_update_timer->adjust(attotime::never);
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void kbdc8042_device::device_reset()
{
	m_poll_delay = 10;

	/* ibmat bios wants 0x20 set! (keyboard locked when not set) 0x80 */
	m_inport = 0xa0;
	at_8042_set_outport(0xfe, 1);

	m_mouse_x = 0;
	m_mouse_y = 0;
	m_mouse_btn = 0;

	m_update_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
}

void kbdc8042_device::at_8042_set_outport(uint8_t data, int initial)
{
	uint8_t change = initial ? 0xFF : (m_outport ^ data);
	m_outport = data;
	if (change & 0x02)
	{
		if (!m_gate_a20_cb.isnull())
			m_gate_a20_cb(data & 0x02 ? 1 : 0);
	}
}

WRITE_LINE_MEMBER( kbdc8042_device::keyboard_w )
{
	if(state)
		at_8042_check_keyboard();
}

void kbdc8042_device::at_8042_receive(uint8_t data, bool mouse)
{
	if (LOG_KEYBOARD)
		logerror("at_8042_receive Received 0x%02x\n", data);

	m_data = data;
	if(!(m_speaker & 0x80) || mouse)
	{
		if (mouse)
			m_mouse.received = 1;
		else
			m_keyboard.received = 1;

		if (m_interrupttype == KBDC8042_SINGLE)
		{
			if (!m_input_buffer_full_cb.isnull())
				m_input_buffer_full_cb(1);
		}
		else
		{
			if (m_keyboard.received && (m_command & 1) && !m_input_buffer_full_cb.isnull())
			{
				m_input_buffer_full_cb(1);
			}
			if (m_mouse.received && (m_command & 2) && !m_input_buffer_full_mouse_cb.isnull())
			{
				m_input_buffer_full_mouse_cb(1);
			}
		}
	}
}

void kbdc8042_device::at_8042_check_keyboard()
{
	if (!m_keyboard.received && !m_mouse.received)
	{
		int data = m_keyboard_dev->read();
		if (data)
			at_8042_receive(data);
	}
}

void kbdc8042_device::at_8042_check_mouse()
{
	if ((m_keybtype == KBDC8042_PS2) && PS2_MOUSE_ON && !m_keyboard.received && !m_mouse.received)
	{
		if (m_mouse.reporting && (m_mouse.to_transmit == 0))
		{
			uint16_t x = m_mousex_port->read();
			uint16_t y = m_mousey_port->read();
			uint8_t buttons = m_mousebtn_port->read();

			uint16_t old_mouse_x = m_mouse_x;
			uint16_t old_mouse_y = m_mouse_y;
			uint16_t old_mouse_btn = m_mouse_btn;

			if(m_mouse_x == 0xffff)
			{
				old_mouse_x = x & 0x3ff;
				old_mouse_y = y & 0x3ff;
				old_mouse_btn = buttons;
			}

			m_mouse_x = x & 0x3ff;
			m_mouse_y = y & 0x3ff;
			m_mouse_btn = buttons;

			uint16_t dx = m_mouse_x - old_mouse_x;
			uint16_t dy = old_mouse_y - m_mouse_y;

			if (dx != 0 || dy != 0 || buttons != old_mouse_btn)
			{
				m_mouse.to_transmit = 3;
				m_mouse.from_transmit = 0;
				m_mouse.transmit_buf[0] = buttons | 0x08 | (BIT(dx, 8) << 4) | (BIT(dy, 8) << 5);
				m_mouse.transmit_buf[1] = dx & 0xff;
				m_mouse.transmit_buf[2] = dy & 0xff;
			}
		}

		if (m_mouse.to_transmit)
		{
			at_8042_receive(m_mouse.transmit_buf[m_mouse.from_transmit], true);
			m_mouse.to_transmit--;
			m_mouse.from_transmit = (m_mouse.from_transmit + 1) & (sizeof(m_mouse.transmit_buf) - 1);
		}
	}
}

void kbdc8042_device::at_8042_clear_keyboard_received()
{
	if (m_keyboard.received)
	{
		if (LOG_KEYBOARD)
			logerror("kbdc8042_8_r(): Clearing m_keyboard.received\n");
	}

	m_input_buffer_full_cb(0);
	m_input_buffer_full_mouse_cb(0);
	m_keyboard.received = 0;
	m_mouse.received = 0;
}

void kbdc8042_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_UPDATE)
	{
		at_8042_check_keyboard();
		if (m_mouse.on)
			at_8042_check_mouse();
	}
}

void kbdc8042_device::mouse_enqueue(uint8_t value)
{
	if (m_mouse.to_transmit < 8)
	{
		m_mouse.transmit_buf[(m_mouse.from_transmit + m_mouse.to_transmit) & (sizeof(m_mouse.transmit_buf) - 1)] = value;
		m_mouse.to_transmit++;
	}
}

/* **************************************************************************
 * Port 0x60 Input and Output Buffer (keyboard and mouse data)
 * Port 0x64 Read Status Register
 *           Write operation for controller
 *
 *  Output port controller:
 *      7: Keyboard data
 *      6: Keyboard clock
 *      5: Mouse buffer full
 *      4: Keyboard buffer full
 *      3: Mouse clock
 *      2: Mouse data
 *      1: 0 A20 cleared
 *      0: 0 system reset
 *
 *  Input port controller
 *      7: 0=Keyboard Locked
 *      6: 1 = Monochrome 0 = Color (true for real IBM, clones are undefined and use CMOS RAM data)
 *      5..2: reserved
 *      1: Mouse data in
 *      0: Keyboard data in
 */

uint8_t kbdc8042_device::data_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset) {
	case 0:
		data = m_data;
		at_8042_clear_keyboard_received();
		at_8042_check_keyboard();
		if (m_mouse.on)
			at_8042_check_mouse();
		break;

	case 1:
		data = m_speaker;
		data &= ~0xc0; /* AT BIOS don't likes this being set */

		/* polled for changes in ibmat bios */
		if (--m_poll_delay < 0)
		{
			if (m_keybtype != KBDC8042_PS2)
				m_poll_delay = 4; /* ibmat */
			else
				m_poll_delay = 8; /* ibm ps2m30 */
			m_offset1 ^= 0x10;
		}
		data = (data & ~0x10) | m_offset1;

		if (m_speaker & 1)
			data |= 0x20;
		else
			data &= ~0x20; /* ps2m30 wants this */
		break;

	case 2:
		if (m_out2)
			data |= 0x20;
		else
			data &= ~0x20;
		break;

	case 4:
		at_8042_check_keyboard();
		if (m_mouse.on)
			at_8042_check_mouse();

		if (m_keyboard.received || m_mouse.received)
			data |= 1;
		if (m_sending)
			data |= 2;

		m_sending = 0; /* quicker than normal */
		data |= 4; /* selftest ok */

		if (m_last_write_to_control)
			data |= 8;

		switch (m_status_read_mode) {
		case 0:
			if (!m_keyboard.on) data|=0x10;
			if (m_mouse.received) data|=0x20;
			break;
		case 1:
			data |= m_inport&0xf;
			break;
		case 2:
			data |= m_inport<<4;
			break;
		}
		break;
	}

	if (LOG_ACCESSES)
		logerror("kbdc8042_8_r(): offset=%d data=0x%02x\n", offset, (unsigned) data);
	return data;
}



void kbdc8042_device::data_w(offs_t offset, uint8_t data)
{
	switch (offset) {
	case 0:
		m_last_write_to_control = 0;
		m_status_read_mode = 0;
		switch (m_operation_write_state) {
		case 0:
			m_data = data;
			m_sending = 1;
			m_keyboard_dev->write(data);
			break;

		case 1:
			/* preceded by writing 0xD1 to port 60h
			 *  |7|6|5|4|3|2|1|0|  8042 Output Port
			 *   | | | | | | | `---- system reset line
			 *   | | | | | | `----- gate A20
			 *   | | | | `-------- undefined
			 *   | | | `--------- output buffer full
			 *   | | `---------- input buffer empty
			 *   | `----------- keyboard clock (output)
			 *   `------------ keyboard data (output)
			 */
			at_8042_set_outport(data, 0);
			break;

		case 2:
			/* preceded by writing 0xD2 to port 60h */
			at_8042_receive(data);
			break;

		case 3:
			/* preceded by writing 0xD3 to port 60h */
			m_data = data;
			break;

		case 4:
			/* preceded by writing 0xD4 to port 60h */
			m_data = data;
			if (m_mouse.receiving_sample_rate)
			{
				m_mouse.sample_rate = data;
				m_mouse.receiving_sample_rate = false;
				mouse_enqueue(0xfa);
				break;
			}
			if (m_mouse.receiving_resolution)
			{
				m_mouse.resolution = data;
				m_mouse.receiving_resolution = false;
				mouse_enqueue(0xfa);
				break;
			}

			switch (m_data)
			{
				case 0xff:
					logerror("Mouse reset command received\n");
					m_mouse.sample_rate = 100;
					m_mouse.from_transmit = 0;
					m_mouse.to_transmit = 0;
					m_mouse.reporting = false;
					if (m_mouse.on)
					{
						mouse_enqueue(0xfa);
						mouse_enqueue(0xaa);
						mouse_enqueue(0x00);
					}
					else
					{
						m_mouse.received = 1;
						m_data = 0xfa;
					}
					break;
				case 0xf6:
					mouse_enqueue(0xfa);
					break;
				case 0xf5:
					m_mouse.reporting = false;
					mouse_enqueue(0xfa);
					break;
				case 0xf4:
					m_mouse.reporting = true;
					mouse_enqueue(0xfa);
					break;
				case 0xf3:
					m_mouse.receiving_sample_rate = true;
					mouse_enqueue(0xfa);
					break;
				case 0xf2:
					mouse_enqueue(0xfa);
					mouse_enqueue(0x00);
					break;
				case 0xe8:
					mouse_enqueue(0xfa);
					m_mouse.receiving_resolution = true;
					break;
				case 0xe6:
					mouse_enqueue(0xfa);
					break;
				case 0xe9:
					mouse_enqueue(0xfa);
					mouse_enqueue(0x00);
					mouse_enqueue(m_mouse.resolution);
					mouse_enqueue(m_mouse.sample_rate);
					break;
				default:
					logerror("%s: Unknown mouse command: %02x\n", machine().describe_context(), m_data);
					break;
			}
			break;

		case 5:
			/* preceded by writing 0x60 to port 60h */
			m_command = data;
			break;
		}
		m_operation_write_state = 0;
		break;

	case 1:
		m_speaker = data;
		if (data & 0x80)
		{
			at_8042_check_keyboard();
			at_8042_clear_keyboard_received();
		}
		m_speaker &= ~0x80;
		if (!m_speaker_cb.isnull())
			m_speaker_cb((offs_t)0, m_speaker);

		break;

	case 4:
		m_last_write_to_control=1;

		/* switch based on the command */
		switch(data) {
		case 0x20:  /* current 8042 command byte is placed on port 60h */
			at_8042_receive(m_command);
			break;
		case 0x60:  /* next data byte is placed in 8042 command byte */
			m_operation_write_state = 5;
			break;
		case 0xa7:  /* disable auxilary interface */
			m_mouse.on = false;
			break;
		case 0xa8:  /* enable auxilary interface */
			m_mouse.on = true;
			break;
		case 0xa9:  /* test mouse */
			at_8042_receive(((m_keybtype == KBDC8042_PS2) && PS2_MOUSE_ON) ? 0x00 : 0xff);
			break;
		case 0xaa:  /* selftest */
			at_8042_receive(0x55);
			break;
		case 0xab:  /* test keyboard */
			at_8042_receive(KEYBOARD_ON ? 0x00 : 0xff);
			break;
		case 0xad:  /* disable keyboard interface */
			m_keyboard.on = 0;
			break;
		case 0xae:  /* enable keyboard interface */
			m_keyboard.on = 1;
			break;
		case 0xc0:  /* read input port */
			/*  |7|6|5|4|3 2 1 0|  8042 Input Port
			 *   | | | |    |
			 *   | | | |    `------- undefined
			 *   | | | |
			 *   | | | `--------- 1=enable 2nd 256k of Motherboard RAM
			 *   | | `---------- 1=manufacturing jumper installed
			 *   | `----------- 1=primary display is MDA, 0=CGA
			 *   `------------ 1=keyboard not inhibited; 0=inhibited
			 */
			at_8042_receive(m_inport);
			break;
		case 0xc1:  /* read input port 3..0 until write to 0x60 */
			m_status_read_mode = 1;
			break;
		case 0xc2:  /* read input port 7..4 until write to 0x60 */
			m_status_read_mode = 2;
			break;
		case 0xca: /* unknown used by savquest (read controller mode AT=0/PS2=1 ?) */
			at_8042_receive(1);
			break;
		case 0xd0:  /* read output port */
			at_8042_receive(m_outport);
			break;
		case 0xd1:
			/* write output port; next byte written to port 60h is placed on
			 * 8042 output port */
			m_operation_write_state = 1;
			return; /* instant delivery */
		case 0xd2:
			/* write keyboard output register; on PS/2 systems next port 60h
			 * write is written to port 60h output register as if initiated
			 * by a device; invokes interrupt if enabled */
			m_operation_write_state = 2;
			break;
		case 0xd3:
			/* write auxillary output register; on PS/2 systems next port 60h
			 * write is written to port 60h input register as if initiated
			 * by a device; invokes interrupt if enabled */
			m_operation_write_state = 3;
			break;
		case 0xd4:
			/* write auxillary device; on PS/2 systems the next data byte
			 * written to input register a port at 60h is sent to the
			 * auxiliary device  */
			m_operation_write_state = 4;
			break;
		case 0xe0:
			/* read test inputs; read T1/T0 test inputs into bit 1/0 */
			at_8042_receive(0x00);
			break;
		case 0xed:
			/* set/unset keyboard LEDs */
			at_8042_receive(0xfa);
			break;
		case 0xf0:
		case 0xf2:
		case 0xf4:
		case 0xf6:
		case 0xf8:
		case 0xfa:
		case 0xfc:
		case 0xfe:
			/* Commands 0xF0...0xFF causes certain output lines to be pulsed
			 * low for six milliseconds.  The bits pulsed low correspond to
			 * the bits low set in the command byte.  The only pulse that has
			 * an effect currently is bit 0, which pulses the CPU's reset line
			 */
			m_system_reset_cb(ASSERT_LINE);
			m_system_reset_cb(CLEAR_LINE);
			at_8042_set_outport(m_outport | 0x02, 0);
			break;
		default:
			if (data != 0xff)
			{
				logerror("%s: Unknown command: %02x\n", machine().describe_context(), data);
			}
		}
		m_sending = 1;
		break;
	}
}

WRITE_LINE_MEMBER(kbdc8042_device::write_out2)
{
	m_out2 = state;
}

INPUT_PORTS_START( kbdc8042_mouse )
	PORT_START("MOUSEX")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2)

	PORT_START("MOUSEY")
	PORT_BIT(0x3ff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0x3ff) PORT_KEYDELTA(2)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2")
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor kbdc8042_device::device_input_ports() const
{
	return ((m_keybtype == KBDC8042_PS2) && PS2_MOUSE_ON) ? INPUT_PORTS_NAME(kbdc8042_mouse) : nullptr;
}
