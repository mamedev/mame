// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "plg1x0.h"

#include "plg100-vl.h"
#include "plg150-ap.h"

DEFINE_DEVICE_TYPE(PLG1X0_CONNECTOR, plg1x0_connector, "plg1x0_connector", "PLG1x0 extension connector")

plg1x0_connector::plg1x0_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLG1X0_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_plg1x0_interface>(mconfig, *this),
	device_mixer_interface(mconfig, *this),
	m_midi_tx(*this)
{
}

void plg1x0_connector::device_start()
{
	save_item(NAME(m_state_system_is_annoying));
}

void plg1x0_connector::midi_rx(int state)
{
	auto card = get_card_device();
	if(card)
		card->midi_rx(state);
}

void plg1x0_intf(device_slot_interface &device)
{
	device.option_add("plg100vl", PLG100_VL);
	device.option_add("plg150ap", PLG150_AP);
}

device_plg1x0_interface::device_plg1x0_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "plg1x0"),
	m_connector(nullptr)
{
}

device_plg1x0_interface::~device_plg1x0_interface()
{
}

void device_plg1x0_interface::interface_pre_start()
{
	m_connector = downcast<plg1x0_connector *>(device().owner());
}
