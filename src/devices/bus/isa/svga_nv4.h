// license:BSD-3-Clause
// copyright-holders:Darius Goad

/*
 * TODO: THIS IS A HACK
 * NO ACCELERATION WILL WORK UNTIL THIS IS CONVERTED TO PCI
 * AS THAT IS WHAT OSES USE TO IDENTIFY THIS CARD.
 */

#pragma once

#ifndef __ISA_NV4_H__
#define __ISA_NV4_H__

#include "emu.h"
#include "isa.h"
#include "video/pc_vga.h"
#include "nv4.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa16_svga_nv4_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_svga_nv4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		nv4_vga_device *m_vga;
};


// device type definition
extern const device_type ISA16_SVGA_NV4;

#endif  /* __ISA_NV4_H__ */
