// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_BUS_ISA_SVGA_TSENG_H
#define MAME_BUS_ISA_SVGA_TSENG_H

#pragma once

#include "isa.h"

#include "video/pc_vga_tseng.h"

#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa16_svga_et4k_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t input_port_0_r();

	virtual void remap(int space_id, offs_t start, offs_t end) override;

protected:
	isa16_svga_et4k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<tseng_vga_device> m_vga;
};

class isa16_svga_et4k_kasan16_device :
		public isa16_svga_et4k_device
{
public:
	isa16_svga_et4k_kasan16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_hangul_rom;
};

class isa16_svga_et4k_w32i_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	isa16_svga_et4k_w32i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void remap(int space_id, offs_t start, offs_t end) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<et4kw32i_vga_device> m_vga;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_SVGA_ET4K,         isa16_svga_et4k_device)
DECLARE_DEVICE_TYPE(ISA16_SVGA_ET4K_KASAN16, isa16_svga_et4k_kasan16_device)
DECLARE_DEVICE_TYPE(ISA16_SVGA_ET4K_W32I,    isa16_svga_et4k_w32i_device)

#endif // MAME_BUS_ISA_SVGA_TSENG_H
