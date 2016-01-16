// license:BSD-3-Clause
// copyright-holders:R. Belmont,Kevin Horton
#pragma once

#ifndef __ISA_MPU401_H__
#define __ISA_MPU401_H__

#include "emu.h"
#include "isa.h"
#include "machine/mpu401.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_mpu401_device

class isa8_mpu401_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_mpu401_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

		required_device<mpu401_device> m_mpu401;

		// called back by the MPU401 core to set the IRQ line state
		DECLARE_WRITE_LINE_MEMBER(mpu_irq_out);

		// optional information overrides
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual machine_config_constructor device_mconfig_additions() const override;
};


// device type definition
extern const device_type ISA8_MPU401;

#endif  /* __ISA_MPU401_H__ */
