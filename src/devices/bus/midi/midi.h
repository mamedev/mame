// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_MIDI_MIDI_H
#define MAME_BUS_MIDI_MIDI_H

#pragma once


class device_midi_port_interface;

class midi_port_device : public device_t, public device_single_card_slot_interface<device_midi_port_interface>
{
	friend class device_midi_port_interface;

public:
	template <typename T>
	midi_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: midi_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	midi_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~midi_port_device();

	// static configuration helpers
	auto rxd_handler() { return m_rxd_handler.bind(); }

	void write_txd(int state);

	int rx_r() { return m_rxd; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	void common(machine_config &config);

	int m_rxd;

	devcb_write_line m_rxd_handler;

private:
	device_midi_port_interface *m_dev;
};

class device_midi_port_interface : public device_interface
{
	friend class midi_port_device;

public:
	virtual ~device_midi_port_interface();

	virtual void input_txd(int state) { }
	void output_rxd(int state) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

protected:
	device_midi_port_interface(const machine_config &mconfig, device_t &device);

	midi_port_device *m_port;
};

DECLARE_DEVICE_TYPE(MIDI_PORT, midi_port_device)

device_slot_interface &midiin_slot(device_slot_interface &device);
device_slot_interface &midiout_slot(device_slot_interface &device);

#endif // MAME_BUS_MIDI_MIDI_H
