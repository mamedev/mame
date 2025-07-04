// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  pwrbkduo.cpp - Macintosh PowerBook Duo dock bus
  Emulation by R. Belmont

***************************************************************************/

#include "emu.h"
#include "pwrbkduo.h"

#include <algorithm>

pwrbkduo_slot_device::pwrbkduo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pwrbkduo_slot_device(mconfig, PWRBKDUO_SLOT, tag, owner, clock)
{
}

pwrbkduo_slot_device::pwrbkduo_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	nubus_slot_device(mconfig, type, tag, owner, clock),
	m_pwrbkduo(*this, finder_base::DUMMY_TAG),
	m_pwrbkduo_slottag(nullptr)
{
}

void pwrbkduo_slot_device::device_resolve_objects()
{
	device_nubus_card_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_nubus_tag(m_pwrbkduo.target(), m_pwrbkduo_slottag);
		m_pwrbkduo->add_nubus_card(*dev);
	}
}

void pwrbkduo_slot_device::device_start()
{
}

DEFINE_DEVICE_TYPE(PWRBKDUO_SLOT, pwrbkduo_slot_device, "pwrbkduo_slot", "PowerBook Duo dock connector")

device_pwrbkduo_card_interface::device_pwrbkduo_card_interface(const machine_config &mconfig, device_t &device) :
	device_nubus_card_interface(mconfig, device)
{
}

device_pwrbkduo_card_interface::~device_pwrbkduo_card_interface()
{
}

void device_pwrbkduo_card_interface::interface_pre_start()
{
	set_pds_slot(0xe);
}

DEFINE_DEVICE_TYPE(PWRBKDUO, pwrbkduo_device, "pwrbkduo", "PowerBook Duo bus")
