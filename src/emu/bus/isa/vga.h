// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#pragma once

#ifndef __ISA_VGA_H__
#define __ISA_VGA_H__

#include "emu.h"
#include "isa.h"
#include "video/pc_vga.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_vga_device

class isa8_vga_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
private:
		vga_device *m_vga;
};


// device type definition
extern const device_type ISA8_VGA;

#endif  /* __ISA_VGA_H__ */
