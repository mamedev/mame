// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-102 serial interface emulation

**********************************************************************/

#pragma once

#ifndef __HPC102__
#define __HPC102__

#include "emu.h"
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
	hpc102_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_portfolio_expansion_slot_interface overrides

private:
	required_device<ins8250_device> m_uart;
};


// device type definition
extern const device_type HPC102;



#endif
/*


*/