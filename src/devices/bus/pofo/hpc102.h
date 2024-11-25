// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio HPC-102 serial interface emulation

**********************************************************************/

#ifndef MAME_BUS_POFO_HPC102_H
#define MAME_BUS_POFO_HPC102_H

#pragma once

#include "exp.h"
#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pofo_hpc102_device

class pofo_hpc102_device : public device_t, public device_portfolio_expansion_slot_interface
{
public:
	// construction/destruction
	pofo_hpc102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_portfolio_expansion_slot_interface overrides
	bool pdet() override { return 1; }

	virtual uint8_t eack_r() override;

	virtual uint8_t nrdi_r(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1) override;
	virtual void nwri_w(offs_t offset, uint8_t data, bool iom, bool bcom, bool ncc1) override;

private:
	required_device<ins8250_device> m_uart;

	uint8_t m_vector;
};


// device type definition
DECLARE_DEVICE_TYPE(POFO_HPC102, pofo_hpc102_device)

#endif // MAME_BUS_POFO_HPC102_H
