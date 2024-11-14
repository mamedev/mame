// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Waveblaster midi extension cards

#ifndef MAME_BUS_WAVEBLASTER_WAVEBLASTER_H
#define MAME_BUS_WAVEBLASTER_WAVEBLASTER_H

#pragma once

class device_waveblaster_interface;

class waveblaster_connector: public device_t, public device_single_card_slot_interface<device_waveblaster_interface>, public device_mixer_interface
{
public:
	template <typename T>
	waveblaster_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: waveblaster_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	waveblaster_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void midi_rx(int state);
	auto midi_tx() { return m_midi_tx.bind(); }

	void do_midi_tx(int state) { m_midi_tx(state); }

protected:
	bool m_state_system_is_annoying = true;
	devcb_write_line m_midi_tx;

	virtual void device_start() override ATTR_COLD;
};

class device_waveblaster_interface: public device_interface
{
public:
	virtual ~device_waveblaster_interface();

	virtual void midi_rx(int state) = 0;

protected:
	waveblaster_connector *m_connector;

	device_waveblaster_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
};

DECLARE_DEVICE_TYPE(WAVEBLASTER_CONNECTOR, waveblaster_connector)

void waveblaster_intf(device_slot_interface &device);

#endif // MAME_BUS_WAVEBLASTER_WAVEBLASTER_H

