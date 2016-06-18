// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-101 parallel interface emulation

**********************************************************************/

#pragma once

#ifndef __HPC101__
#define __HPC101__

#include "emu.h"
#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hpc101_t

class hpc101_t :  public device_t,
				  public device_portfolio_expansion_slot_interface
{
public:
	// construction/destruction
	hpc101_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_portfolio_expansion_slot_interface overrides
	bool pdet() override { return 1; }

    virtual UINT8 nrdi_r(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1) override;
    virtual void nwri_w(address_space &space, offs_t offset, UINT8 data, bool iom, bool bcom, bool ncc1) override;

private:
	required_device<i8255_device> m_ppi;
};


// device type definition
extern const device_type HPC101;



#endif
