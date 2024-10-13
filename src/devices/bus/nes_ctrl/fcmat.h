// license:BSD-3-Clause
// copyright-holders:kmg, Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Bandai Family Trainer Mat
    Nintendo Family Computer - IGS Tap-tap Mat

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_FCMAT_H
#define MAME_BUS_NES_CTRL_FCMAT_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_fcmat_device

class nes_fcmat_device :
	public device_t,
	public device_nes_control_port_interface
{
public:
	virtual u8 read_exp(offs_t offset) override;
	virtual void write(u8 data) override;

protected:
	// construction/destruction
	nes_fcmat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport_array<4> m_mat;
	u8 m_row_scan;
};


// ======================> nes_ftrainer_device

class nes_ftrainer_device : public nes_fcmat_device
{
public:
	// construction/destruction
	nes_ftrainer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// ======================> nes_taptapmat_device

class nes_taptapmat_device : public nes_fcmat_device
{
public:
	// construction/destruction
	nes_taptapmat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_FTRAINER,  nes_ftrainer_device)
DECLARE_DEVICE_TYPE(NES_TAPTAPMAT, nes_taptapmat_device)

#endif // MAME_BUS_NES_CTRL_FCMAT_H
