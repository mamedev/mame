// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Memotech ROMPAK

**********************************************************************/


#ifndef MAME_BUS_MTX_ROMPAK_H
#define MAME_BUS_MTX_ROMPAK_H

#pragma once

#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mtx_rompak_device

class mtx_rompak_device : public device_t, public device_mtx_exp_interface
{
public:
	// construction/destruction
	mtx_rompak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definitions
DECLARE_DEVICE_TYPE(MTX_ROMPAK, mtx_rompak_device)


#endif // MAME_BUS_MTX_ROMPAK_H
