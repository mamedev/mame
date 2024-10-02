// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * The Music Machine - MIDI and sampling expansion
 * by Ram Electronics
 *
 * Contains a 6850 AICA, Ferranti ZN429E8 DAC and ZN449 ADC
 */

#ifndef MAME_BUS_SPECTRUM_MUSICMACHINE_H
#define MAME_BUS_SPECTRUM_MUSICMACHINE_H

#pragma once

#include "exp.h"
#include "machine/6850acia.h"
#include "sound/dac.h"

class spectrum_musicmachine_device  : public device_t
	, public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_musicmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void write_acia_clock(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u8 iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, u8 data) override;

private:
	required_device<acia6850_device> m_acia;
	required_device<dac_byte_interface> m_dac;

	bool m_irq_select;
};

// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MUSICMACHINE, spectrum_musicmachine_device)

#endif  // MAME_BUS_SPECTRUM_MUSICMACHINE_H
