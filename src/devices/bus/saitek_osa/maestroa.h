// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Saitek OSA Maestro A

***************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_MAESTROA_H
#define MAME_BUS_SAITEKOSA_MAESTROA_H

#pragma once

#include "expansion.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saitekosa_maestroa_device

class saitekosa_maestroa_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_maestroa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// from host
	virtual void nmi_w(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
};

// device type definition
DECLARE_DEVICE_TYPE(OSA_MAESTROA, saitekosa_maestroa_device)

#endif // MAME_BUS_SAITEKOSA_MAESTROA_H
