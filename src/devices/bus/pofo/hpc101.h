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

	virtual uint8_t nrdi_r(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1) override;
	virtual void nwri_w(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1) override;

private:
	required_device<i8255_device> m_ppi;
};


// device type definition
DECLARE_DEVICE_TYPE(POFO_HPC101, pofo_hpc101_device)

#endif // MAME_BUS_POFO_HPC101_H
