// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "keyboard.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(INTERPRO_KEYBOARD_PORT, interpro_keyboard_port_device, "interpro_keyboard_port", "InterPro Keyboard Port")

int const device_interpro_keyboard_port_interface::START_BIT_COUNT;
int const device_interpro_keyboard_port_interface::DATA_BIT_COUNT;
device_serial_interface::parity_t const device_interpro_keyboard_port_interface::PARITY;
device_serial_interface::stop_bits_t const device_interpro_keyboard_port_interface::STOP_BITS;
int const device_interpro_keyboard_port_interface::BAUD;

interpro_keyboard_port_device::interpro_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: interpro_keyboard_port_device(mconfig, INTERPRO_KEYBOARD_PORT, tag, owner, clock)
{
}

interpro_keyboard_port_device::interpro_keyboard_port_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_rxd_handler(*this)
	, m_dev(nullptr)
{
}

interpro_keyboard_port_device::~interpro_keyboard_port_device()
{
}

void interpro_keyboard_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_interpro_keyboard_port_interface *>(get_card_device());
}

void interpro_keyboard_port_device::device_start()
{
	m_rxd_handler.resolve_safe();

	save_item(NAME(m_rxd));
}

WRITE_LINE_MEMBER(interpro_keyboard_port_device::write_txd)
{
	if (m_dev)
		m_dev->input_txd(state);
}

device_interpro_keyboard_port_interface::device_interpro_keyboard_port_interface(machine_config const &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<interpro_keyboard_port_device *>(device.owner()))
{
}

device_interpro_keyboard_port_interface::~device_interpro_keyboard_port_interface()
{
}

#include "hle.h"

SLOT_INTERFACE_START(interpro_keyboard_devices)
	SLOT_INTERFACE("hle_en_us", INTERPRO_HLE_EN_US_KEYBOARD)
SLOT_INTERFACE_END
