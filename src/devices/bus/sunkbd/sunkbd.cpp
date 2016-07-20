// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "sunkbd.h"


device_type const SUNKBD_PORT = &device_creator<sun_keyboard_port_device>;


int const device_sun_keyboard_port_interface::START_BIT_COUNT;
int const device_sun_keyboard_port_interface::DATA_BIT_COUNT;
device_serial_interface::parity_t const device_sun_keyboard_port_interface::PARITY;
device_serial_interface::stop_bits_t const device_sun_keyboard_port_interface::STOP_BITS;
int const device_sun_keyboard_port_interface::BAUD;



sun_keyboard_port_device::sun_keyboard_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		UINT32 clock)
	: sun_keyboard_port_device(mconfig, SUNKBD_PORT, "Sun Keyboard Port", tag, owner, clock, "sunkbd", __FILE__)
{
}


sun_keyboard_port_device::sun_keyboard_port_device(
		machine_config const &mconfig,
		device_type type,
		char const *name,
		char const *tag,
		device_t *owner,
		UINT32 clock,
		char const *shortname,
		char const *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_slot_interface(mconfig, *this)
	, m_rxd(0)
	, m_rxd_handler(*this)
	, m_dev(nullptr)
{
}


sun_keyboard_port_device::~sun_keyboard_port_device()
{
}


void sun_keyboard_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_sun_keyboard_port_interface *>(get_card_device());
}


void sun_keyboard_port_device::device_start()
{
	m_rxd_handler.resolve_safe();

	save_item(NAME(m_rxd));

	m_rxd = 1;

	m_rxd_handler(m_rxd);
}


WRITE_LINE_MEMBER( sun_keyboard_port_device::write_txd )
{
	if(m_dev)
		m_dev->input_txd(state);
}



device_sun_keyboard_port_interface::device_sun_keyboard_port_interface(machine_config const &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<sun_keyboard_port_device *>(device.owner()))
{
}


device_sun_keyboard_port_interface::~device_sun_keyboard_port_interface()
{
}



#include "sparckbd.h"


SLOT_INTERFACE_START( default_sun_keyboard_devices )
	SLOT_INTERFACE("sparckbd", SPARC_KEYBOARD)
SLOT_INTERFACE_END
