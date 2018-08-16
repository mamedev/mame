// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "keyboard.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(INTERPRO_KEYBOARD_PORT, interpro_keyboard_port_device, "interpro_keyboard_port", "InterPro Keyboard Port")

interpro_keyboard_port_device::interpro_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_KEYBOARD_PORT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_rxd_handler(*this)
	, m_dev(nullptr)
{
}

void interpro_keyboard_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_interpro_keyboard_port_interface *>(get_card_device());
}

void interpro_keyboard_port_device::device_start()
{
	m_rxd_handler.resolve_safe();
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

#include "hle.h"
#include "lle.h"

void interpro_keyboard_devices(device_slot_interface &device)
{
	device.option_add("hle_en_us", INTERPRO_HLE_EN_US_KEYBOARD);
	device.option_add("lle_en_us", INTERPRO_LLE_EN_US_KEYBOARD);
}
