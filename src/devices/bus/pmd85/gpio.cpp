// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*******************************************************************************

    Tesla PMD 85 GPIO/0 connector

*******************************************************************************/

#include "emu.h"
#include "gpio.h"

#include "pmd32.h"


DEFINE_DEVICE_TYPE(PMD85_GPIO_SLOT, pmd85_gpio_slot_device, "pmd85_gpio", "Tesla PMD 85 GPIO/0 connector")


pmd85_gpio_slot_device::pmd85_gpio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PMD85_GPIO_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_pmd85_gpio_interface>(mconfig, *this)
	, m_stb_cb(*this)
	, m_ack_cb(*this)
	, m_card(nullptr)
{
}

void pmd85_gpio_slot_device::device_start()
{
	m_card = get_card_device();
}

uint8_t pmd85_gpio_slot_device::pa_r()
{
	return m_card ? m_card->pa_r() : 0xff;
}

void pmd85_gpio_slot_device::pa_w(uint8_t data)
{
	if (m_card)
		m_card->pa_w(data);
}

void pmd85_gpio_slot_device::pc_w(uint8_t data)
{
	if (m_card)
		m_card->pc_w(data);
}


device_pmd85_gpio_interface::device_pmd85_gpio_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pmd85_gpio")
	, m_slot(dynamic_cast<pmd85_gpio_slot_device *>(device.owner()))
{
}

device_pmd85_gpio_interface::~device_pmd85_gpio_interface()
{
}

void device_pmd85_gpio_interface::strobe_host()
{
	m_slot->m_stb_cb(0);
	m_slot->m_stb_cb(1);
}

void device_pmd85_gpio_interface::acknowledge_host()
{
	m_slot->m_ack_cb(0);
	m_slot->m_ack_cb(1);
}


void pmd85_gpio_devices(device_slot_interface &device)
{
	device.option_add("pmd32", PMD32);
}
