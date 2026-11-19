// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    IDE64 ShortBus

***************************************************************************/

#include "emu.h"
#include "shortbus.h"


DEFINE_DEVICE_TYPE(IDE64_SHORTBUS, ide64_shortbus_device, "ide64_shortbus", "IDE64 ShortBus")

template class device_finder<device_ide64_shortbus_interface, false>;
template class device_finder<device_ide64_shortbus_interface, true>;


ide64_shortbus_device::ide64_shortbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IDE64_SHORTBUS, tag, owner, clock),
	device_single_card_slot_interface<device_ide64_shortbus_interface>(mconfig, *this),
	m_card(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this)
{
}

ide64_shortbus_device::~ide64_shortbus_device()
{
}

void ide64_shortbus_device::device_start()
{
	m_card = get_card_device();
}

uint8_t ide64_shortbus_device::cd_r(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	return m_card ? m_card->shortbus_r(offset & 0xff, data, io1, io2, csel) : data;
}

void ide64_shortbus_device::cd_w(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (m_card)
		m_card->shortbus_w(offset & 0xff, data, io1, io2, csel);
}


device_ide64_shortbus_interface::device_ide64_shortbus_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "ide64shortbus"),
	m_host(dynamic_cast<ide64_shortbus_device *>(device.owner()))
{
}

device_ide64_shortbus_interface::~device_ide64_shortbus_interface()
{
}

uint8_t device_ide64_shortbus_interface::shortbus_r(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	return data;
}

void device_ide64_shortbus_interface::shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
}
