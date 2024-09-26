// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midioutport.h

    MIDI Out port - glues the image device to the pluggable midi port

*********************************************************************/

#ifndef MAME_BUS_MIDI_MIDIOUTPORT_H
#define MAME_BUS_MIDI_MIDIOUTPORT_H

#pragma once

#include "midi.h"
#include "imagedev/midiout.h"

class midiout_port_device : public device_t,
	public device_midi_port_interface
{
public:
	midiout_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override { if (started()) m_midiout->tx(state); }

protected:
	virtual void device_start() override { }
	virtual void device_reset() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<midiout_device> m_midiout;
};

DECLARE_DEVICE_TYPE(MIDIOUT_PORT, midiout_port_device)

#endif // MAME_BUS_MIDI_MIDIOUTPORT_H
