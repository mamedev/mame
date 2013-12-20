#include "emu.h"
#include "serial.h"

const device_type SERIAL_PORT = &device_creator<serial_port_device>;

device_serial_port_interface::device_serial_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_rbit = FALSE;
	m_tbit = FALSE;
}

device_serial_port_interface::~device_serial_port_interface()
{
}

serial_port_device::serial_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SERIAL_PORT, "Serial Port", tag, owner, clock, "serial_port", __FILE__),
	device_slot_interface(mconfig, *this),
	m_dev(NULL),
	m_out_rx_handler(*this)
{
}

serial_port_device::serial_port_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_slot_interface(mconfig, *this),
	m_dev(NULL),
	m_out_rx_handler(*this)
{
}

serial_port_device::~serial_port_device()
{
}

void serial_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_serial_port_interface *>(get_card_device());
}

void serial_port_device::device_start()
{
	m_out_rx_handler.resolve_safe();
}

const device_type RS232_PORT = &device_creator<rs232_port_device>;

device_rs232_port_interface::device_rs232_port_interface(const machine_config &mconfig, device_t &device)
	: device_serial_port_interface(mconfig, device)
{
	m_dtr = FALSE;
	m_rts = FALSE;
	m_dcd = FALSE;
	m_dsr = FALSE;
	m_ri  = FALSE;
	m_cts = FALSE;
}

device_rs232_port_interface::~device_rs232_port_interface()
{
}

rs232_port_device::rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: serial_port_device(mconfig, RS232_PORT, "RS232 Port", tag, owner, clock, "rs232", __FILE__),
	m_out_dcd_handler(*this),
	m_out_dsr_handler(*this),
	m_out_ri_handler(*this),
	m_out_cts_handler(*this)
{
}

rs232_port_device::~rs232_port_device()
{
}

void rs232_port_device::device_config_complete()
{
	serial_port_device::device_config_complete();
	m_dev = dynamic_cast<device_rs232_port_interface *>(get_card_device());
	loopdtr = 0;
	looprts = 0;
}

void rs232_port_device::device_start()
{
	serial_port_device::device_start();
	m_out_dcd_handler.resolve_safe();
	m_out_dsr_handler.resolve_safe();
	m_out_ri_handler.resolve_safe();
	m_out_cts_handler.resolve_safe();
}

// XXX:make loopback handshaking optional if needed
WRITE_LINE_MEMBER( rs232_port_device::dtr_w )
{
	if(m_dev)
		return m_dev->dtr_w(state);

	if(serial_port_device::m_dev)
	{
		loopdtr = state;
		out_dcd(state);
		out_dsr(state);
	}
}

WRITE_LINE_MEMBER( rs232_port_device::rts_w )
{
	if(m_dev)
		return m_dev->rts_w(state);

	if(serial_port_device::m_dev)
	{
		looprts = state;
		out_cts(state);
	}
}

#include "machine/null_modem.h"
#include "machine/terminal.h"

SLOT_INTERFACE_START( default_rs232_devices )
	SLOT_INTERFACE("serial_terminal", SERIAL_TERMINAL)
	SLOT_INTERFACE("null_modem", NULL_MODEM)
SLOT_INTERFACE_END
