// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Bandai Family Trainer Mat

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_FTRAINER_H
#define MAME_BUS_NES_CTRL_FTRAINER_H

#pragma once

#include "ctrl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_ftrainer_device

class nes_ftrainer_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_ftrainer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_exp(offs_t offset) override;
	virtual void write(uint8_t data) override;

private:
	required_ioport_array<4> m_trainer;
	uint8_t m_row_scan;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_FTRAINER, nes_ftrainer_device)


#endif // MAME_BUS_NES_CTRL_FTRAINER_H
