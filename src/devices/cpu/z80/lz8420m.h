// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Sharp LZ8420M Z80 with Built-in RAM

    (TODO: everything;
        8 Bit IO, 4 Bit Input, 4 Bit Output, 512 Byte RAM)

***************************************************************************/

#ifndef MAME_CPU_Z80_LZ8420M_H
#define MAME_CPU_Z80_LZ8420M_H

#pragma once

#include "z80.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class lz8420m_device : public z80_device
{
public:
	lz8420m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(LZ8420M, lz8420m_device)

#endif // MAME_CPU_Z80_LZ8420M_H
