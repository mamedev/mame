// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#pragma once

#ifndef __ISA_SVGA_S3_H__
#define __ISA_SVGA_S3_H__

#include "emu.h"
#include "isa.h"
#include "video/pc_vga.h"
#include "s3virge.h"

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
		isa16_svga_s3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		s3_vga_device *m_vga;
		ibm8514a_device *m_8514;
};

class isa16_s3virge_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_s3virge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		s3virge_vga_device *m_vga;
};

class isa16_s3virgedx_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_s3virgedx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		s3virgedx_vga_device *m_vga;
};

class isa16_stealth3d2kpro_device :
		public device_t,
		public device_isa16_card_interface
{
public:
		// construction/destruction
		isa16_stealth3d2kpro_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		DECLARE_READ8_MEMBER(input_port_0_r);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
private:
		s3virgedx_vga_device *m_vga;
};


// device type definition
extern const device_type ISA16_SVGA_S3;
extern const device_type ISA16_S3VIRGE;
extern const device_type ISA16_S3VIRGEDX;
extern const device_type ISA16_DMS3D2KPRO;

#endif  /* __ISA_VGA_H__ */
