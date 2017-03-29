// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Bandai Family Trainer Mat

**********************************************************************/

#pragma once

#ifndef __NES_FTRAINER__
#define __NES_FTRAINER__


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
extern const device_type NES_FTRAINER;


#endif
