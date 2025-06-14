// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    AY Slot for ZX Spectrum

    Supports variates AY8910 sound chips
**********************************************************************/

#include "emu.h"
#include "slot.h"

DEFINE_DEVICE_TYPE(AY_SLOT, ay_slot_device, "ay_slot", "AY Slot")

ay_slot_device::ay_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ay_slot_device(mconfig, AY_SLOT, tag, owner, clock)
{
}

ay_slot_device::ay_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_ay_slot_interface>(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_dev(nullptr)
{
}

ay_slot_device::~ay_slot_device()
{
}

void ay_slot_device::device_start()
{
	m_dev = get_card_device();
}

u8 ay_slot_device::data_r()
{
	return m_dev ? m_dev->data_r() : 0xff;
};

void ay_slot_device::data_w(u8 data)
{
	if (m_dev) m_dev->data_w(data);
};

void ay_slot_device::address_w(u8 data)
{
	if (m_dev) m_dev->address_w(data);
};


device_ay_slot_interface::device_ay_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "ay")
{
	m_slot = dynamic_cast<ay_slot_device *>(device.owner());
}

device_ay_slot_interface::~device_ay_slot_interface()
{
}


spectrum_ay_device::spectrum_ay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_ay_slot_interface(mconfig, *this)
	, m_ay0(*this, "ay0")
	, m_ay1(*this, "ay1")
{
}

u8 spectrum_ay_device::data_r()
{
	if (m_ay_selected)
		return m_ay1->data_r();
	else
		return m_ay0->data_r();
}

void spectrum_ay_device::data_w(u8 data)
{
	if (m_ay_selected)
		return m_ay1->data_w(data);
	else
		return m_ay0->data_w(data);
}

void spectrum_ay_device::address_w(u8 data)
{
	if (m_ay1 &&  ((data & 0xfe) == 0xfe))
		m_ay_selected = data & 1;
	else if (m_ay_selected)
		m_ay1->address_w(data);
	else
		m_ay0->address_w(data);
}

void spectrum_ay_device::device_start()
{
	save_item(NAME(m_ay_selected));
}

void spectrum_ay_device::device_reset()
{
	m_ay_selected = 0;
}

void spectrum_ay_device::device_add_mconfig(machine_config &config)
{
	m_ay0->add_route(0, DEVICE_SELF_OWNER, 1.0, 0)
		.add_route(1, DEVICE_SELF_OWNER, 1.0, 1)
		.add_route(2, DEVICE_SELF_OWNER, 1.0, 2);
	if (m_ay1)
	{
		m_ay1->add_route(0, DEVICE_SELF_OWNER, 1.0, 0)
			.add_route(1, DEVICE_SELF_OWNER, 1.0, 1)
			.add_route(2, DEVICE_SELF_OWNER, 1.0, 2);
	}
}


#include "cards.h"

template class device_finder<device_ay_slot_interface, false>;
template class device_finder<device_ay_slot_interface, true>;

void default_ay_slot_devices(device_slot_interface &device)
{
	device.option_add("ay_ay8912",     AY_SLOT_AY8912);
	device.option_add("ay_turbosound", AY_SLOT_TURBOSOUND);
	device.option_add("ay_ym2149",     AY_SLOT_YM2149);
}
