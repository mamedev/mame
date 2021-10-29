// license:BSD-3-Clause
// copyright-holders:kmg, Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - IGS Tap-tap Mat

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_TAPTAPMAT_H
#define MAME_BUS_NES_CTRL_TAPTAPMAT_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_taptapmat_device

class nes_taptapmat_device :
	public device_t,
	public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_taptapmat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

private:
	required_ioport_array<4> m_mat;
	u8 m_row_scan;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TAPTAPMAT, nes_taptapmat_device)


#endif // MAME_BUS_NES_CTRL_TAPTAPMAT_H
