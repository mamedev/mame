// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    The Serial Port MIDI Interface

**********************************************************************/


#ifndef MAME_BUS_ARCHIMEDES_ECONET_MIDI_H
#define MAME_BUS_ARCHIMEDES_ECONET_MIDI_H

#pragma once


#include "slot.h"
#include "machine/mc6854.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> arc_serial_midi_device

class arc_serial_midi_device:
	public device_t,
	public device_archimedes_econet_interface
{
public:
	// construction/destruction
	arc_serial_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	arc_serial_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

private:
	required_device<mc6854_device> m_adlc;
};


// ======================> arc_serial_sampler_device

class arc_serial_sampler_device: public arc_serial_midi_device
{
public:
	// construction/destruction
	arc_serial_sampler_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};



// device type definition
DECLARE_DEVICE_TYPE(ARC_SERIAL_MIDI, arc_serial_midi_device);
DECLARE_DEVICE_TYPE(ARC_SERIAL_SAMPLER, arc_serial_sampler_device);


#endif /* MAME_BUS_ARCHIMEDES_ECONET_MIDI_H */
