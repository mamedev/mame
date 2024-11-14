// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * The Music Machine - MIDI and sampling expansion
 * by Ram Electronics
 *
 * Contains a 6850 AICA, Ferranti ZN429E8 DAC and ZN449 ADC
 */

#ifndef MAME_BUS_CPC_MUSICMACHINE_H
#define MAME_BUS_CPC_MUSICMACHINE_H

#pragma once

#include "cpcexp.h"
#include "machine/6850acia.h"
#include "sound/dac.h"

class cpc_musicmachine_device  : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_musicmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void dac_w(uint8_t data);
	uint8_t acia_r(offs_t offset);
	void acia_w(offs_t offset, uint8_t data);
	void irqsel_w(uint8_t data);
	void irq_w(int state);

	void write_acia_clock(u8 data) { m_acia->write_txc(data); m_acia->write_rxc(data); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<acia6850_device> m_acia;
	required_device<dac_byte_interface> m_dac;

	bool m_irq_select;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_MUSICMACHINE, cpc_musicmachine_device)

#endif  // MAME_BUS_CPC_MUSICMACHINE_H
