// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiinport.h

    MIDI In port - glues the image device to the pluggable midi port

*********************************************************************/

#ifndef MAME_BUS_MIDI_MIDIINPORT_H
#define MAME_BUS_MIDI_MIDIINPORT_H

#pragma once

#include "midi.h"
#include "imagedev/midiin.h"

class midiin_port_device : public device_t,
	public device_midi_port_interface
{
public:
	midiin_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override { m_owner = dynamic_cast<midi_port_device *>(owner()); }
	virtual void device_reset() override { }

private:
	void read(int state) { output_rxd(state); }

	required_device<midiin_device> m_midiin;
};

DECLARE_DEVICE_TYPE(MIDIIN_PORT, midiin_port_device)

#endif // MAME_BUS_MIDI_MIDIINPORT_H
