// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * doubler.c  --  Draysoft Doubler - external cassette interface for the 464 (works on 664/6128 with external cassette?),
 *                intended for use in duplicating cassette software
 *
 * Uses only port F0E0 (may conflict with other peripherals, PPI port A is not usable while Doubler software is running)
 *
 */

#ifndef MAME_BUS_CPC_DOUBLER_H
#define MAME_BUS_CPC_DOUBLER_H

#pragma once

#include "cpcexp.h"
#include "imagedev/cassette.h"

class cpc_doubler_device  : public device_t,
						public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_doubler_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ext_tape_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;

	required_device<cassette_image_device> m_tape;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_DOUBLER, cpc_doubler_device)

#endif // MAME_BUS_CPC_DOUBLER_H
