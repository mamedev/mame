// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_BUS_ISA_SVGA_CIRRUS_H
#define MAME_BUS_ISA_SVGA_CIRRUS_H

#pragma once

#include "isa.h"
#include "video/pc_vga_cirrus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa16_svga_cirrus_gd5401_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_cirrus_gd5401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<cirrus_gd5401_vga_device> m_vga;
};

class isa16_svga_cirrus_gd542x_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_cirrus_gd542x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t input_port_0_r();
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<cirrus_gd5428_vga_device> m_vga;
};

class isa16_svga_cirrus_gd5430_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_cirrus_gd5430_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t input_port_0_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<cirrus_gd5430_vga_device> m_vga;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA16_AVGA1_CIRRUS_GD5401, isa16_svga_cirrus_gd5401_device)
DECLARE_DEVICE_TYPE(ISA16_SVGA_CIRRUS_GD5430,  isa16_svga_cirrus_gd5430_device)
DECLARE_DEVICE_TYPE(ISA16_SVGA_CIRRUS_GD542X,  isa16_svga_cirrus_gd542x_device)

#endif // MAME_BUS_ISA_SVGA_CIRRUS_H
