// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "plg100.h"

#include "vl.h"

DEFINE_DEVICE_TYPE(PLG100_CONNECTOR, plg100_connector, "plg100_connector", "PLG100 extension connector")

plg100_connector::plg100_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLG100_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_plg100_interface>(mconfig, *this),
	device_mixer_interface(mconfig, *this, 2),
	m_midi_tx(*this)
{
}

void plg100_connector::device_start()
{
	save_item(NAME(m_state_system_is_annoying));
}

void plg100_connector::midi_rx(int state)
{
	auto card = get_card_device();
	if(card)
		card->midi_rx(state);
}

void plg100_intf(device_slot_interface &device)
{
	device.option_add("vl", PLG100_VL);
}

device_plg100_interface::device_plg100_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "plg100"),
	m_connector(nullptr)
{
}

device_plg100_interface::~device_plg100_interface()
{
}

void device_plg100_interface::interface_pre_start()
{
	m_connector = downcast<plg100_connector *>(device().owner());
}
