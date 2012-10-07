#pragma once

#ifndef __ISA_SVGA_S3_H__
#define __ISA_SVGA_S3_H__

#include "emu.h"
#include "machine/isa.h"
#include "video/pc_vga.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

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

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();
private:
		s3_vga_device *m_vga;
		ibm8514a_device *m_8514;
};


// device type definition
extern const device_type ISA16_SVGA_S3;

#endif  /* __ISA_VGA_H__ */
