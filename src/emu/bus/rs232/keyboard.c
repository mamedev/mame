/***************************************************************************
Generic ASCII Keyboard

Use MCFG_SERIAL_KEYBOARD_ADD to attach this as a serial device to a terminal
or computer.

Use MCFG_ASCII_KEYBOARD_ADD to attach as a generic ascii input device in
cases where either the driver isn't developed enough yet; or for testing;
or for the case of a computer with an inbuilt (not serial) ascii keyboard.

Example of usage in a driver.

In MACHINE_CONFIG
    MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)

In the code:

WRITE8_MEMBER( xxx_state::kbd_put )
{
    (code to capture the key as it is pressed)
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
    DEVCB_DRIVER_MEMBER(xxx_state, kbd_put)
};

***************************************************************************/

#include "keyboard.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static INPUT_PORTS_START(serial_keyboard)
	PORT_INCLUDE(generic_keyboard)
	PORT_START("TERM_FRAME")
	PORT_CONFNAME(0x0f, 0x06, "Baud") PORT_CHANGED_MEMBER(DEVICE_SELF, serial_keyboard_device, update_frame, 0)
	PORT_CONFSETTING( 0x0d, "110")
	PORT_CONFSETTING( 0x00, "150")
	PORT_CONFSETTING( 0x01, "300")
	PORT_CONFSETTING( 0x02, "600")
	PORT_CONFSETTING( 0x03, "1200")
	PORT_CONFSETTING( 0x04, "2400")
	PORT_CONFSETTING( 0x05, "4800")
	PORT_CONFSETTING( 0x06, "9600")
	PORT_CONFSETTING( 0x07, "14400")
	PORT_CONFSETTING( 0x08, "19200")
	PORT_CONFSETTING( 0x09, "28800")
	PORT_CONFSETTING( 0x0a, "38400")
	PORT_CONFSETTING( 0x0b, "57600")
	PORT_CONFSETTING( 0x0c, "115200")
	PORT_CONFNAME(0x30, 0x00, "Format") PORT_CHANGED_MEMBER(DEVICE_SELF, serial_keyboard_device, update_frame, 0)
	PORT_CONFSETTING( 0x00, "8N1")
	PORT_CONFSETTING( 0x10, "7E1")
	PORT_CONFSETTING( 0x20, "8N2")
	PORT_CONFSETTING( 0x30, "8E1")
INPUT_PORTS_END

ioport_constructor serial_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serial_keyboard);
}

serial_keyboard_device::serial_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: generic_keyboard_device(mconfig, SERIAL_KEYBOARD, "Serial Keyboard", tag, owner, clock, "serial_keyboard", __FILE__),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_io_term_frame(*this, "TERM_FRAME")
{
}

serial_keyboard_device::serial_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: generic_keyboard_device(mconfig, type, name, tag, owner, clock, shortname, source),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_io_term_frame(*this, "TERM_FRAME")
{
}

void serial_keyboard_device::device_config_complete()
{
	const serial_keyboard_interface *intf = reinterpret_cast<const serial_keyboard_interface *>(static_config());
	if(intf != NULL)
	{
		*static_cast<serial_keyboard_interface *>(this) = *intf;
	}
	else
	{
		memset(&m_out_tx_cb, 0, sizeof(m_out_tx_cb));
	}
}

static int rates[] = {150, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 110};

void serial_keyboard_device::device_start()
{
	int baud = clock();
	if(!baud) baud = 9600;
	m_out_tx_func.resolve(m_out_tx_cb, *this);
	m_timer = timer_alloc();
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_tra_rate(baud);
}

INPUT_CHANGED_MEMBER(serial_keyboard_device::update_frame)
{
	reset();
}

void serial_keyboard_device::device_reset()
{
	generic_keyboard_device::device_reset();
	m_rbit = 1;
	if (m_port)
		output_rxd(m_rbit);
	else
		m_out_tx_func(m_rbit);

	UINT8 val = m_io_term_frame->read();
	set_tra_rate(rates[val & 0x0f]);

	switch(val & 0x30)
	{
	case 0x10:
		set_data_frame(1, 7, PARITY_EVEN, STOP_BITS_1);
		break;
	case 0x00:
	default:
		set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
		break;
	case 0x20:
		set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
		break;
	case 0x30:
		set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_1);
		break;
	}
}

void serial_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id)
		device_serial_interface::device_timer(timer, id, param, ptr);
	else
		generic_keyboard_device::device_timer(timer, id, param, ptr);
}

void serial_keyboard_device::send_key(UINT8 code)
{
	if(is_transmit_register_empty())
	{
		transmit_register_setup(code);
		return;
	}
	m_key_valid = true;
	m_curr_key = code;
}

void serial_keyboard_device::tra_callback()
{
	m_rbit = transmit_register_get_data_bit();
	if(m_port)
		output_rxd(m_rbit);
	else
		m_out_tx_func(m_rbit);
}

void serial_keyboard_device::tra_complete()
{
	if(m_key_valid)
	{
		transmit_register_setup(m_curr_key);
		m_key_valid = false;
	}
}

READ_LINE_MEMBER(serial_keyboard_device::tx_r)
{
	return m_rbit;
}

const device_type SERIAL_KEYBOARD = &device_creator<serial_keyboard_device>;
