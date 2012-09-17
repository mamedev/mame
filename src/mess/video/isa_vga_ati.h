/*
 * isa_vga_ati.h
 *
 *  Header for ATi Graphics Ultra ISA video card
 *
 *  Created on: 9/09/2012
 */
#pragma once

#ifndef ISA_VGA_ATI_H_
#define ISA_VGA_ATI_H_

#include "emu.h"
#include "machine/isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa16_vga_gfxultra_device :
		public device_t,
		public device_isa16_card_interface
{
public:
        // construction/destruction
        isa16_vga_gfxultra_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
protected:
        // device-level overrides
        virtual void device_start();
        virtual void device_reset();
};


// device type definition
extern const device_type ISA16_VGA_GFXULTRA;


#endif /* ISA_VGA_ATI_H_ */
