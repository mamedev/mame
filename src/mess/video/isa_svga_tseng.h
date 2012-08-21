#pragma once

#ifndef __ISA_SVGA_ET4K_H__
#define __ISA_SVGA_ET4K_H__

#include "emu.h"
#include "machine/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_vga_device

class isa8_svga_et4k_device :
		public device_t,
		public device_isa8_card_interface
{
public:
        // construction/destruction
        isa8_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();
};


// device type definition
extern const device_type ISA8_SVGA_ET4K;

#endif  /* __ISA_SVGA_ET4K_H__ */
