// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The PLG100 series is a bunch of proprietary plugins for the Yamaha
// MU series of expanders.  It mostly provides two midi-rate (and midi
// protocol) serial lines (in and out) and two stereo serial sample
// streams (in and out too).

// Known existing cards:
//  PLG100-DX: DX7 as a plugin
//  PLG100-SG: Singing speech synthesis, e.g. a Vocaloid before the Vocaloid existed
//  PLG100-VH: Voice Harmonizer, harmony effects on the A/D inputs
//  PLG100-VL: Virtual Acoustic Synthesis, physical-modelling synthesis, a VL70-m on a plugin card
//  PLG100-XG: MU50 as a plugin

#ifndef MAME_BUS_PLG100_PLG100_H
#define MAME_BUS_PLG100_PLG100_H

#pragma once

class device_plg100_interface;

class plg100_connector: public device_t, public device_single_card_slot_interface<device_plg100_interface>, public device_mixer_interface
{
public:
	template <typename T>
	plg100_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: plg100_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	plg100_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void midi_rx(int state);
	auto midi_tx() { return m_midi_tx.bind(); }

	void do_midi_tx(int state) { m_midi_tx(state); }

protected:
	bool m_state_system_is_annoying = true;
	devcb_write_line m_midi_tx;

	virtual void device_start() override;
};

class device_plg100_interface: public device_interface
{
public:
	virtual ~device_plg100_interface();

	virtual void midi_rx(int state) = 0;

protected:
	plg100_connector *m_connector;

	device_plg100_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
};

DECLARE_DEVICE_TYPE(PLG100_CONNECTOR, plg100_connector)

void plg100_intf(device_slot_interface &device);

#endif // MAME_BUS_PLG100_PLG100_H

