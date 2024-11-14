// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
   PlayCity expansion device

    I/O ports:
    * F880 - Z80CTC channel 0 (input is system clock (4MHz), output to YMZ294 clock)
    * F881 - Z80CTC channel 1 (input from CRTC CURSOR, output to /NMI)
    * F882 - Z80CTC channel 2 (input is system clock (4MHz), output to channel 3 input)
    * F883 - Z80CTC channel 3 (input is channel 2 output)
    * F884 - YMZ294 #1 (right) data
    * F888 - YMZ294 #2 (left) data
    * F984 - YMZ294 #1 (right) register select
    * F988 - YMZ294 #2 (left) register select
*/

#ifndef MAME_BUS_CPC_PLAYCITY_H
#define MAME_BUS_CPC_PLAYCITY_H

#pragma once


#include "cpcexp.h"
#include "sound/ay8910.h"
#include "machine/z80ctc.h"

class cpc_playcity_device : public device_t, public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_playcity_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ctc_r(offs_t offset);
	void ctc_w(offs_t offset, uint8_t data);
	void ymz1_address_w(uint8_t data);
	void ymz2_address_w(uint8_t data);
	void ymz1_data_w(uint8_t data);
	void ymz2_data_w(uint8_t data);
	uint8_t ymz1_data_r();
	uint8_t ymz2_data_r();

	virtual void cursor_w(int state) override { m_ctc->trg1(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void ctc_zc1_cb(int state) { if(state) { m_slot->nmi_w(1); m_slot->nmi_w(0); } }
	void ctc_intr_cb(int state) { m_slot->irq_w(state); }

	cpc_expansion_slot_device *m_slot;

	required_device<z80ctc_device> m_ctc;
	required_device<ymz294_device> m_ymz1;
	required_device<ymz294_device> m_ymz2;

	void update_ymz_clock();
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_PLAYCITY, cpc_playcity_device)


#endif // MAME_BUS_CPC_PLAYCITY_H
