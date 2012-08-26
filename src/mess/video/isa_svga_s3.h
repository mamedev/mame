#pragma once

#ifndef __ISA_SVGA_S3_H__
#define __ISA_SVGA_S3_H__

#include "emu.h"
#include "machine/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_vga_device

class isa16_svga_s3_device :
		public device_t,
		public device_isa16_card_interface
{
public:
        // construction/destruction
        isa16_svga_s3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();
};


// device type definition
extern const device_type ISA16_SVGA_S3;

#endif  /* __ISA_VGA_H__ */
