// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "waveblaster.h"

#include "omniwave.h"
#include "db50xg.h"
#include "db60xg.h"
#include "wg130.h"


DEFINE_DEVICE_TYPE(WAVEBLASTER_CONNECTOR, waveblaster_connector, "waveblaster_connector", "Waveblaster extension connector")

waveblaster_connector::waveblaster_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WAVEBLASTER_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<device_waveblaster_interface>(mconfig, *this),
	device_mixer_interface(mconfig, *this),
	m_midi_tx(*this)
{
}

void waveblaster_connector::device_start()
{
	save_item(NAME(m_state_system_is_annoying));
}

void waveblaster_connector::midi_rx(int state)
{
	auto card = get_card_device();
	if(card)
		card->midi_rx(state);
}

void waveblaster_intf(device_slot_interface &device)
{
	device.option_add("omniwave", OMNIWAVE);
	device.option_add("db50xg", DB50XG);
	device.option_add("db60xg", DB60XG);
	device.option_add("wg130", WG130);
}

device_waveblaster_interface::device_waveblaster_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "waveblaster"),
	m_connector(nullptr)
{
}

device_waveblaster_interface::~device_waveblaster_interface()
{
}

void device_waveblaster_interface::interface_pre_start()
{
	m_connector = downcast<waveblaster_connector *>(device().owner());
}
