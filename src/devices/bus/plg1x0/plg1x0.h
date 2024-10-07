// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// The PLG100 series is a bunch of proprietary plugins for the Yamaha
// MU series of expanders.  It mostly provides two midi-rate (and midi
// protocol) serial lines (in and out) and two stereo serial sample
// streams (in and out too).
//
// The PLG150 series, the successor, seems essentially compatible.
// The main difference is (in some cases) nvram to save settings.

// Known existing cards:
//  PLG100-DX: DX7 as a plugin
//  PLG100-SG: Singing speech synthesis, e.g. a Vocaloid before the Vocaloid existed
//  PLG100-VH: Voice Harmonizer, harmony effects on the A/D inputs
//  PLG100-VL: Virtual Acoustic Synthesis, physical-modelling synthesis, a VL70-m on a plugin card
//  PLG100-XG: MU50 as a plugin
//
//  PLG150-AN: Analog Physical Modeling
//  PLG150-AP: Acoustic Piano
//  PLG150-DR: Drums
//  PLG150-DX: DX7 as a plugin
//  PLG150-PC: Latin drums
//  PLG150-PF: Piano
//  PLG150-VL: Virtual Acoustic Synthesis


#ifndef MAME_BUS_PLG1X0_PLG1X0_H
#define MAME_BUS_PLG1X0_PLG1X0_H

#pragma once

class device_plg1x0_interface;

class plg1x0_connector: public device_t, public device_single_card_slot_interface<device_plg1x0_interface>, public device_mixer_interface
{
public:
	template <typename T>
	plg1x0_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: plg1x0_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	plg1x0_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void midi_rx(int state);
	auto midi_tx() { return m_midi_tx.bind(); }

	void do_midi_tx(int state) { m_midi_tx(state); }

protected:
	bool m_state_system_is_annoying = true;
	devcb_write_line m_midi_tx;

	virtual void device_start() override ATTR_COLD;
};

class device_plg1x0_interface: public device_interface
{
public:
	virtual ~device_plg1x0_interface();

	virtual void midi_rx(int state) = 0;

protected:
	plg1x0_connector *m_connector;

	device_plg1x0_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;
};

DECLARE_DEVICE_TYPE(PLG1X0_CONNECTOR, plg1x0_connector)

void plg1x0_intf(device_slot_interface &device);

#endif // MAME_BUS_PLG1X0_PLG1X0_H

