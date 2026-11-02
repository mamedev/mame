// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit ET-3400 Microprocessor Trainer 40-pin accessory connector

****************************************************************************/

#include "emu.h"
#include "exp.h"

#include "eta3400.h"


DEFINE_DEVICE_TYPE(ET3400_EXPANSION_SLOT, et3400_expansion_slot_device, "et3400_exp", "Heathkit ET-3400 accessory connector")


device_et3400_expansion_card_interface::device_et3400_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "et3400exp")
	, m_slot(dynamic_cast<et3400_expansion_slot_device *>(device.owner()))
{
}

address_space &device_et3400_expansion_card_interface::space() const
{
	return m_slot->space();
}


et3400_expansion_slot_device::et3400_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ET3400_EXPANSION_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_et3400_expansion_card_interface>(mconfig, *this)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_card(nullptr)
{
}

void et3400_expansion_slot_device::device_config_complete()
{
	m_card = get_card_device();
}

void et3400_expansion_slot_device::device_start()
{
}


void et3400_expansion_cards(device_slot_interface &device)
{
	device.option_add("eta3400", ETA3400);
}
