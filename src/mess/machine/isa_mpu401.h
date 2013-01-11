#pragma once

#ifndef __ISA_MPU401_H__
#define __ISA_MPU401_H__

#include "emu.h"
#include "machine/isa.h"

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
		isa8_mpu401_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		DECLARE_READ8_MEMBER(mpu401_r);
		DECLARE_WRITE8_MEMBER(mpu401_w);
		// optional information overrides
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
		virtual void device_config_complete() { m_shortname = "isa_mpu401"; }
private:
		// internal state
};


// device type definition
extern const device_type ISA8_MPU401;

#endif  /* __ISA_MPU401_H__ */
