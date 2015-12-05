// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#pragma once

#ifndef __ISA_SVGA_CIRRUS_H__
#define __ISA_SVGA_CIRRUS_H__

#include "emu.h"
#include "isa.h"
#include "video/clgd542x.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa16_svga_cirrus_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_svga_cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		cirrus_gd5430_device *m_vga;
};

class isa16_svga_cirrus_gd542x_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_svga_cirrus_gd542x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		cirrus_gd5428_device *m_vga;
};


// device type definition
extern const device_type ISA16_SVGA_CIRRUS;
extern const device_type ISA16_SVGA_CIRRUS_GD542X;

#endif  /* __ISA_VGA_H__ */
