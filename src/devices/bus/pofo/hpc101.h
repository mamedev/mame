// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-101 parallel interface emulation

**********************************************************************/

#ifndef MAME_BUS_POFO_HPC101_H
#define MAME_BUS_POFO_HPC101_H

#pragma once

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pofo_hpc101_device

class pofo_hpc101_device : public device_t, public device_portfolio_expansion_slot_interface
{
public:
	// construction/destruction
	pofo_hpc101_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_portfolio_expansion_slot_interface overrides
	bool pdet() override { return 1; }

private:
	required_device<i8255_device> m_ppi;
	memory_passthrough_handler m_ppi_tap;
};


// device type definition
DECLARE_DEVICE_TYPE(POFO_HPC101, pofo_hpc101_device)

#endif // MAME_BUS_POFO_HPC101_H
