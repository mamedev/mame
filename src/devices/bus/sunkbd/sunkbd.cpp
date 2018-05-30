// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "sunkbd.h"


DEFINE_DEVICE_TYPE(SUNKBD_PORT, sun_keyboard_port_device, "sunkbd", "Sun Keyboard Port")


int const device_sun_keyboard_port_interface::START_BIT_COUNT;
int const device_sun_keyboard_port_interface::DATA_BIT_COUNT;
device_serial_interface::parity_t const device_sun_keyboard_port_interface::PARITY;
device_serial_interface::stop_bits_t const device_sun_keyboard_port_interface::STOP_BITS;
int const device_sun_keyboard_port_interface::BAUD;



sun_keyboard_port_device::sun_keyboard_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: sun_keyboard_port_device(mconfig, SUNKBD_PORT, tag, owner, clock)
{
}


sun_keyboard_port_device::sun_keyboard_port_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
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


void sun_keyboard_port_device::device_resolve_objects()
{
	m_rxd_handler.resolve_safe();
}


void sun_keyboard_port_device::device_start()
{
	save_item(NAME(m_rxd));

	m_rxd = 1;

	m_rxd_handler(m_rxd);
}


WRITE_LINE_MEMBER( sun_keyboard_port_device::write_txd )
{
	if (m_dev)
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



#include "hlekbd.h"

void default_sun_keyboard_devices(device_slot_interface &device)
{
	device.option_add("type3hle",   SUN_TYPE3_HLE_KEYBOARD);
	device.option_add("type4hle",   SUN_TYPE4_HLE_KEYBOARD);
	device.option_add("type5hle",   SUN_TYPE5_HLE_KEYBOARD);
	device.option_add("type5gbhle", SUN_TYPE5_GB_HLE_KEYBOARD);
	device.option_add("type5sehle", SUN_TYPE5_SE_HLE_KEYBOARD);
	device.option_add("type5jphle", SUN_TYPE5_JP_HLE_KEYBOARD);
}
