#pragma once

#ifndef __ISA_FINALCHS_H__
#define __ISA_FINALCHS_H__

#include "emu.h"
#include "machine/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_finalchs_device

class isa8_finalchs_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
        isa8_finalchs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

        DECLARE_READ8_MEMBER(finalchs_r);
        DECLARE_WRITE8_MEMBER(finalchs_w);
		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();

private:
        // internal state
};


// device type definition
extern const device_type ISA8_FINALCHS;

#endif  /* __ISA_FINALCHS_H__ */
