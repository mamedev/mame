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
	: device_t(mconfig, SERIAL_PORT, "Serial Port", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_dev(NULL)
{
}

serial_port_device::serial_port_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_dev(NULL)
{
}

serial_port_device::~serial_port_device()
{
}

void serial_port_device::device_config_complete()
{
	const serial_port_interface *intf = reinterpret_cast<const serial_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<serial_port_interface *>(this) = *intf;
	}
	else
	{
		memset(&m_out_rx_cb, 0, sizeof(m_out_rx_cb));
	}
	m_dev = dynamic_cast<device_serial_port_interface *>(get_card_device());
}

void serial_port_device::device_start()
{
	m_out_rx_func.resolve(m_out_rx_cb, *this);
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
	: serial_port_device(mconfig, RS232_PORT, "RS232 Port", tag, owner, clock)
{
}

rs232_port_device::~rs232_port_device()
{
}

void rs232_port_device::device_config_complete()
{
	const rs232_port_interface *intf = reinterpret_cast<const rs232_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<rs232_port_interface *>(this) = *intf;
		memcpy(&(serial_port_interface::m_out_rx_cb), &(rs232_port_interface::m_out_rx_cb), sizeof(rs232_port_interface::m_out_rx_cb));
	}
	else
	{
		memset(&(serial_port_interface::m_out_rx_cb), 0, sizeof(serial_port_interface::m_out_rx_cb));
		memset(&m_out_dcd_cb, 0, sizeof(m_out_dcd_cb));
		memset(&m_out_dsr_cb, 0, sizeof(m_out_dsr_cb));
		memset(&m_out_ri_cb, 0, sizeof(m_out_ri_cb));
		memset(&m_out_cts_cb, 0, sizeof(m_out_cts_cb));
	}
	m_dev = dynamic_cast<device_rs232_port_interface *>(get_card_device());
	serial_port_device::m_dev = dynamic_cast<device_serial_port_interface *>(get_card_device());
	loopdtr = 0;
	looprts = 0;
}

void rs232_port_device::device_start()
{
	serial_port_device::device_start();
	m_out_dcd_func.resolve(m_out_dcd_cb, *this);
	m_out_dsr_func.resolve(m_out_dsr_cb, *this);
	m_out_ri_func.resolve(m_out_ri_cb, *this);
	m_out_cts_func.resolve(m_out_cts_cb, *this);
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
