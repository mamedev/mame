// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-102 serial interface emulation

**********************************************************************/

#pragma once

#ifndef __HPC102__
#define __HPC102__

#include "exp.h"
#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hpc102_t

class hpc102_t :  public device_t,
					public device_portfolio_expansion_slot_interface
{
public:
	// construction/destruction
	hpc102_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_portfolio_expansion_slot_interface overrides
	bool pdet() override { return 1; }

	virtual uint8_t eack_r() override;

	virtual uint8_t nrdi_r(address_space &space, offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1) override;
	virtual void nwri_w(address_space &space, offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1) override;

private:
	required_device<ins8250_device> m_uart;

	uint8_t m_vector;
};


// device type definition
extern const device_type HPC102;



#endif
/*


*/
