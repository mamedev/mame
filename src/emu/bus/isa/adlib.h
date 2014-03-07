#pragma once

#ifndef __ISA_ADLIB_H__
#define __ISA_ADLIB_H__

#include "emu.h"
#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_adlib_device

class isa8_adlib_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
private:
		// internal state
};


// device type definition
extern const device_type ISA8_ADLIB;

#endif  /* __ISA_ADLIB_H__ */
