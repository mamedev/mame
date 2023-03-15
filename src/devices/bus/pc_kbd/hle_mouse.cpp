// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * PS/2 mouse high-level emulation.
 *
 * This device emulates a mouse with three buttons, using the original three-
 * byte PS/2 mouse protocol. Serial I/O is driven at 10kHz by a 40kHz timer
 * to generate somewhat accurate clock rising and falling edges, as well as
 * sampling or writing the data line in the middle of each high or low cycle
 * as expected by the protocol.
 *
 * The original IBM PS/2 mouse had only two buttons and the documented protocol
 * reflects this, however it also allows a third button to be added without any
 * significant changes. IBM later produced three-button mice which apparently
 * took advantage of this, making it about as standard as it gets.
 *
 * Microsoft introduced the IntelliMouse in 1996 which adds another two buttons
 * and a scroll wheel, requiring a change to the protocol from three to four
 * byte data packets. The IntelliMouse protocol is only enabled after sending
 * a specific sequence of "set sample rate" commands, without which the mouse
 * uses the original protocol.
 *
 * Sources:
 *
 *   https://web.archive.org/web/20180126072045/http://www.computer-engineering.org/ps2mouse/
 *   https://wiki.osdev.org/PS/2_Mouse
 *   https://www.win.tue.nl/~aeb/linux/kbd/scancodes-13.html
 *   http://read.pudn.com/downloads136/ebook/579116/docs/Designing%20a%20low%20cost%20CY7C63723%20combination%20mouse.pdf
 *
 * TODO
 *   - IntelliMouse device/protocol (4-byte packet, 5 buttons, scroll wheel)
 *   - configurable clock (10kHz-16.7kHz)
 *   - receive parity error handling
 */

#include "emu.h"
#include "hle_mouse.h"

#define LOG_GENERAL (1U << 0)
#define LOG_RXTX    (1U << 1)
#define LOG_COMMAND (1U << 2)
#define LOG_REPORT  (1U << 3)
#define LOG_STATE   (1U << 4)

//#define VERBOSE (LOG_COMMAND)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(HLE_PS2_MOUSE, hle_ps2_mouse_device, "hle_ps2_mouse", "HLE PS/2 Mouse")

ALLOW_SAVE_TYPE(hle_ps2_mouse_device::serial_state);

INPUT_PORTS_START(hle_ps2_mouse_device)
	PORT_START("mouse_x_axis")
	PORT_BIT(0xffff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("mouse_y_axis")
	PORT_BIT(0xffff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(100)

	PORT_START("mouse_buttons")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Left Button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Right Button")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Middle Button")
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static constexpr attotime serial_cycle = attotime::from_usec(25);

hle_ps2_mouse_device::hle_ps2_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HLE_PS2_MOUSE, tag, owner, clock)
	, device_pc_kbd_interface(mconfig, *this)
	, m_port_x_axis(*this, "mouse_x_axis")
	, m_port_y_axis(*this, "mouse_y_axis")
	, m_port_buttons(*this, "mouse_buttons")
{
}

ioport_constructor hle_ps2_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_ps2_mouse_device);
}

void hle_ps2_mouse_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_bit));

	save_item(NAME(m_mode));
	save_item(NAME(m_sample_rate));
	save_item(NAME(m_resolution));

	save_item(NAME(m_rx_len));
	save_item(NAME(m_rx_buf));
	save_item(NAME(m_tx_len));
	save_item(NAME(m_tx_pos));
	save_item(NAME(m_tx_buf));
	save_item(NAME(m_data));
	save_item(NAME(m_parity));

	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_mouse_b));

	set_pc_kbdc_device();

	m_serial = timer_alloc(FUNC(hle_ps2_mouse_device::serial), this);
	m_sample = timer_alloc(FUNC(hle_ps2_mouse_device::sample), this);
}

void hle_ps2_mouse_device::device_reset()
{
	// configure default settings
	defaults();

	// reset mouse state
	update();

	// clear tx/rx buffers
	m_rx_len = 0;
	m_tx_len = 0;
	m_tx_pos = 0;

	// enqueue bat result
	m_tx_buf[m_tx_len++] = 0xaa;
	m_tx_buf[m_tx_len++] = 0x00;

	m_state = IDLE;

	// release clock and data lines
	m_pc_kbdc->data_write_from_kb(1);
	m_pc_kbdc->clock_write_from_kb(1);
}

void hle_ps2_mouse_device::clock_write(int state)
{
	// record when the clock signal last changed state
	m_clock_changed = machine().time();

	// resume serial communication
	resume();
}

void hle_ps2_mouse_device::resume()
{
	if (clock_signal() && m_state == IDLE)
	{
		// check if receiving a start bit
		if (!data_signal())
			m_state = RX_START;

		// start serial communication
		if (!m_serial->enabled())
			m_serial->adjust(serial_cycle, 0, serial_cycle);
	}
}

void hle_ps2_mouse_device::serial(s32 param)
{
	// host may inhibit device communication by holding the clock low for 100µs
	if (!clock_signal() && clock_held(100))
	{
		m_state = IDLE;

		// release clock and data lines
		m_pc_kbdc->data_write_from_kb(1);
		m_pc_kbdc->clock_write_from_kb(1);

		// stop serial communication
		m_serial->enable(false);
		return;
	}

	LOGMASKED(LOG_STATE, "state %d clock %d data %d\n", m_state, clock_signal(), data_signal());

	switch (m_state)
	{
	case IDLE:
		if (clock_signal())
		{
			// stop serial communication if nothing to transmit
			if (m_tx_pos == m_tx_len)
				m_serial->enable(false);
			else
				// device may transmit after clock is high for 50µs
				if (clock_held(50))
					m_state = TX_START;
		}
		break;

	case RX_START:
		// check for start bit
		if (!data_signal())
		{
			m_state = RX_CLOCK_LO0;

			// prepare data
			m_data = 0;
			m_parity = 1;
			m_bit = 0;
		}
		break;

	case RX_CLOCK_LO0:
		// assert clock
		m_state = RX_CLOCK_LO1;
		m_pc_kbdc->clock_write_from_kb(0);
		break;

	case RX_CLOCK_LO1:
		// hold clock
		m_state = RX_CLOCK_HI0;
		break;

	case RX_CLOCK_HI0:
		// release clock
		m_state = RX_CLOCK_HI1;
		m_pc_kbdc->clock_write_from_kb(1);
		break;

	case RX_CLOCK_HI1:
		switch (m_bit)
		{
		case 8: // parity bit
			m_state = RX_CLOCK_LO0;

			if (data_signal() == (m_parity & 1))
			{
				LOGMASKED(LOG_RXTX, "rx data 0x%02x\n", m_data);
				m_rx_buf[m_rx_len++] = m_data;
			}
			else
				// TODO: transmit unbuffered resend command
				LOGMASKED(LOG_RXTX, "rx error data 0x%02x parity %d\n", m_data, data_signal());
			m_bit++;
			break;

		case 9: // acknowledge
			if (data_signal())
			{
				m_state = RX_CLOCK_LO0;
				m_pc_kbdc->data_write_from_kb(0);
				m_bit++;
			}
			break;

		case 10: // finished
			if (m_rx_len)
				m_state = COMMAND;
			else
				m_state = IDLE;
			m_pc_kbdc->data_write_from_kb(1);
			break;

		default: // data bit
			m_state = RX_CLOCK_LO0;
			m_parity += data_signal();
			m_data |= (data_signal() << m_bit++);
			break;
		}
		break;

	case COMMAND:
		m_state = IDLE;

		// execute the command
		command(m_rx_buf[0]);

		// reset mouse state
		update();
		break;

	case TX_START:
		// prepare data
		m_data = m_tx_buf[m_tx_pos++];
		m_parity = 1;
		m_bit = 0;

		// start bit
		m_state = TX_CLOCK_LO0;
		m_pc_kbdc->data_write_from_kb(0);
		break;

	case TX_CLOCK_LO0:
		// assert clock
		m_state = TX_CLOCK_LO1;
		m_pc_kbdc->clock_write_from_kb(0);
		break;

	case TX_CLOCK_LO1:
		// hold clock
		m_state = TX_CLOCK_HI0;
		break;

	case TX_CLOCK_HI0:
		// release clock
		m_state = TX_CLOCK_HI1;
		m_pc_kbdc->clock_write_from_kb(1);
		break;

	case TX_CLOCK_HI1:
		switch (m_bit)
		{
		case 8: // parity bit
			m_state = TX_CLOCK_LO0;
			m_pc_kbdc->data_write_from_kb(m_parity & 1);
			LOGMASKED(LOG_RXTX, "tx data 0x%02x\n", m_data);
			m_bit++;
			break;

		case 9: // stop bit
			m_state = TX_CLOCK_LO0;
			m_pc_kbdc->data_write_from_kb(1);
			m_bit++;
			break;

		case 10: // finished
			m_state = IDLE;
			break;

		default: // data
			m_state = TX_CLOCK_LO0;
			m_parity += BIT(m_data, m_bit);
			m_pc_kbdc->data_write_from_kb(BIT(m_data, m_bit));
			m_bit++;
			break;
		}
	}
}

void hle_ps2_mouse_device::command(u8 const command)
{
	// consume the command byte
	m_rx_len--;

	// reset the transmit position
	m_tx_pos = 0;

	// special case for resend
	if (command == 0xfe)
	{
		LOGMASKED(LOG_COMMAND, "resend\n");
		return;
	}
	else
		m_tx_len = 0;

	// special case for wrap mode
	if ((m_mode & WRAP) && command != 0xff && command != 0xec)
	{
		// echo the command
		m_tx_buf[m_tx_len++] = command;
		return;
	}

	// handle the command
	switch (command)
	{
	case 0xe6: // set scaling 1:1
		LOGMASKED(LOG_COMMAND, "set scaling 1:1\n");
		m_mode &= ~SCALE;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xe7: // set scaling 2:1
		LOGMASKED(LOG_COMMAND, "set scaling 2:1\n");
		m_mode |= SCALE;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xe8: // set resolution
		if (m_rx_len == 1)
		{
			m_resolution = m_rx_buf[m_rx_len--];
			LOGMASKED(LOG_COMMAND, "set resolution 0x%02x\n", m_resolution);
		}
		else
			// re-enqueue the command and wait for the parameter
			m_rx_len++;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xe9: // status request
		LOGMASKED(LOG_COMMAND, "status request\n");
		m_tx_buf[m_tx_len++] = 0xfa;
		m_tx_buf[m_tx_len++] = m_mode | m_port_buttons->read();
		m_tx_buf[m_tx_len++] = m_resolution;
		m_tx_buf[m_tx_len++] = m_sample_rate;
		break;

	case 0xea: // set stream mode
		LOGMASKED(LOG_COMMAND, "set stream mode\n");
		m_mode &= ~REMOTE;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xeb: // read data
		LOGMASKED(LOG_COMMAND, "read data\n");
		m_tx_buf[m_tx_len++] = 0xfa;

		// force data sample after acknowledge transmitted
		if (!m_sample->enabled())
			m_sample->adjust(serial_cycle * 48, 1);
		break;

	case 0xec: // reset wrap mode
		LOGMASKED(LOG_COMMAND, "reset wrap mode\n");
		m_mode &= ~WRAP;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xee: // set wrap mode
		LOGMASKED(LOG_COMMAND, "set wrap mode\n");
		m_mode |= WRAP;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xf0: // set remote mode
		LOGMASKED(LOG_COMMAND, "set remote mode\n");
		m_mode |= REMOTE;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xf2: // get device id
		LOGMASKED(LOG_COMMAND, "get device id\n");
		m_tx_buf[m_tx_len++] = 0xfa;
		m_tx_buf[m_tx_len++] = 0x00;
		break;

	case 0xf3: // set sample rate
		if (m_rx_len == 1)
		{
			m_sample_rate = m_rx_buf[m_rx_len--];
			LOGMASKED(LOG_COMMAND, "set sample rate %d\n", m_sample_rate);
		}
		else
			// re-enqueue the command and wait for the parameter
			m_rx_len++;
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xf4: // enable data reporting
		LOGMASKED(LOG_COMMAND, "enable data reporting\n");
		m_mode |= ENABLE;
		m_sample->adjust(attotime::from_hz(m_sample_rate), 0, attotime::from_hz(m_sample_rate));
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xf5: // disable data reporting
		LOGMASKED(LOG_COMMAND, "disable data reporting\n");
		m_mode &= ~ENABLE;
		m_sample->enable(false);
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xf6: // set defaults
		LOGMASKED(LOG_COMMAND, "set defaults\n");
		defaults();
		m_tx_buf[m_tx_len++] = 0xfa;
		break;

	case 0xff: // reset
		LOGMASKED(LOG_COMMAND, "reset\n");
		defaults();
		m_tx_buf[m_tx_len++] = 0xfa;
		m_tx_buf[m_tx_len++] = 0xaa; // bat successful
		m_tx_buf[m_tx_len++] = 0x00; // device id
		break;

	default:
		LOGMASKED(LOG_COMMAND, "unrecognized command 0x%02x\n", command);
		m_tx_buf[m_tx_len++] = 0xfc; // error
		break;
	}
}

void hle_ps2_mouse_device::sample(s32 param)
{
	// read mouse state
	s16 const x = m_port_x_axis->read();
	s16 const y = m_port_y_axis->read();
	u8 const b = m_port_buttons->read();

	// compute delta
	s16 dx = x - m_mouse_x;
	s16 dy = m_mouse_y - y;
	u8 const db = b ^ m_mouse_b;

	// data report if transmit buffer empty and position or buttons changed
	if ((m_tx_pos == m_tx_len) && (dx || dy || db || param))
	{
		LOGMASKED(LOG_REPORT, "data report dx %d dy %d db %d\n", dx, dy, db);

		// compute sign and overflow
		u8 const sx = (dx < 0) ? 0x10 : 0x00;
		u8 const sy = (dy < 0) ? 0x20 : 0x00;
		u8 const ox = ((dx < -256) || (dx > 255)) ? 0x40 : 0x00;
		u8 const oy = ((dy < -256) || (dy > 255)) ? 0x80 : 0x00;

		// apply scaling
		if (m_mode & SCALE)
		{
			static s32 const scale[] = { -9, -6, -3, -1, -1, 0, 1, 1, 3, 6, 9 };

			dx = (std::abs(dx) > 5) ? dx * 2 : scale[dx + 5];
			dy = (std::abs(dy) > 5) ? dy * 2 : scale[dy + 5];
		}

		// transmit data report
		m_tx_len = 0;
		m_tx_buf[m_tx_len++] = oy | ox | sy | sx | 0x08 | b;
		m_tx_buf[m_tx_len++] = u8(dx);
		m_tx_buf[m_tx_len++] = u8(dy);
		m_tx_pos = 0;

		// record mouse state
		m_mouse_x = x;
		m_mouse_y = y;
		m_mouse_b = b;

		// resume serial communication
		resume();
	}
}

void hle_ps2_mouse_device::defaults()
{
	m_mode = 0;
	m_sample_rate = 100;
	m_resolution = 2;
}

void hle_ps2_mouse_device::update()
{
	m_mouse_x = m_port_x_axis->read();
	m_mouse_y = m_port_y_axis->read();
	m_mouse_b = m_port_buttons->read();
}
