// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_VICDUAL_97271P_H
#define MAME_SEGA_VICDUAL_97271P_H

#pragma once

#include "sound/samples.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s97271p_device : public device_t
{
public:
	// construction/destruction
	s97271p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// daughterboard logic
	void port_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<samples_device> m_samples;

	uint8_t m_state = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(S97271P, s97271p_device)

#endif // MAME_SEGA_VICDUAL_97271P_H
