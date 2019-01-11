// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "sgikbd.h"


DEFINE_DEVICE_TYPE(SGIKBD_PORT, sgi_keyboard_port_device, "sgikbd", "SGI Keyboard Port")


int const device_sgi_keyboard_port_interface::START_BIT_COUNT;
int const device_sgi_keyboard_port_interface::DATA_BIT_COUNT;
device_serial_interface::parity_t const device_sgi_keyboard_port_interface::PARITY;
device_serial_interface::stop_bits_t const device_sgi_keyboard_port_interface::STOP_BITS;
int const device_sgi_keyboard_port_interface::BAUD;

sgi_keyboard_port_device::sgi_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: sgi_keyboard_port_device(mconfig, SGIKBD_PORT, tag, owner, clock)
{
}

sgi_keyboard_port_device::sgi_keyboard_port_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_rxd(0)
	, m_rxd_handler(*this)
	, m_dev(nullptr)
{
}

sgi_keyboard_port_device::~sgi_keyboard_port_device()
{
}

void sgi_keyboard_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_sgi_keyboard_port_interface *>(get_card_device());
}

void sgi_keyboard_port_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_sgi_keyboard_port_interface *>(card))
	{
		logerror("Card device %s (%s) does not implement device_sgi_keyboard_port_interface\n", card->tag(), card->name());
	}
}

void sgi_keyboard_port_device::device_resolve_objects()
{
	m_rxd_handler.resolve_safe();
}

void sgi_keyboard_port_device::device_start()
{
	if (get_card_device() && !m_dev)
	{
		throw emu_fatalerror("Card device %s (%s) does not implement device_sgi_keyboard_port_interface\n", get_card_device()->tag(), get_card_device()->name());
	}

	save_item(NAME(m_rxd));

	m_rxd = 1;
	m_rxd_handler(m_rxd);
}

WRITE_LINE_MEMBER(sgi_keyboard_port_device::write_txd)
{
	if (m_dev)
		m_dev->input_txd(state);
}

device_sgi_keyboard_port_interface::device_sgi_keyboard_port_interface(machine_config const &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<sgi_keyboard_port_device *>(device.owner()))
{
}

device_sgi_keyboard_port_interface::~device_sgi_keyboard_port_interface()
{
}


#include "hlekbd.h"

void default_sgi_keyboard_devices(device_slot_interface &device)
{
	device.option_add("hlekbd",   SGI_HLE_KEYBOARD);
}
