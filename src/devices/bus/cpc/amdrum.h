// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * amdrum.h
 *
 *  Created on: 23/08/2014
 *
 *  Cheetah Marketing Amdrum
 *
 *  I/O FFxx - 8-bit unsigned DAC, write only  (Ferranti ZN428E-8)
 *  Lower 8 address bits are not decoded.
 *
 */

#ifndef MAME_BUS_CPC_AMDRUM_H
#define MAME_BUS_CPC_AMDRUM_H

#pragma once

#include "cpcexp.h"
#include "sound/dac.h"

class cpc_amdrum_device  : public device_t,
							public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_amdrum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void dac_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<dac_byte_interface> m_dac;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_AMDRUM, cpc_amdrum_device)


#endif // MAME_BUS_CPC_AMDRUM_H
