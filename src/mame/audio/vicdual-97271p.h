// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#pragma once

#ifndef __S97271P_H__
#define __S97271P_H__

#include "sound/samples.h"

#define MCFG_S97271P_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, S97271P, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s97271p_device : public device_t
{
public:
	// construction/destruction
	s97271p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<samples_device> m_samples;

	// daughterboard logic
	void port_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_state;
};

// device type definition
extern const device_type S97271P;

MACHINE_CONFIG_EXTERN( nsub_audio ); 

#endif  /* __S97271P_H__ */
