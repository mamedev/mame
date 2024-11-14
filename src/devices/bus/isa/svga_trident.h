// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * svga_trident.h
 */

#ifndef MAME_BUS_ISA_SVGA_TRIDENT_H
#define MAME_BUS_ISA_SVGA_TRIDENT_H

#include "isa.h"

#include "video/pc_vga_trident.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa16_svga_tvga9000_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_tvga9000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t input_port_0_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<tvga9000_device> m_vga;
};

class isa16_svga_tgui9680_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_tgui9680_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t input_port_0_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<trident_vga_device> m_vga;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_SVGA_TVGA9000, isa16_svga_tvga9000_device)
DECLARE_DEVICE_TYPE(ISA16_SVGA_TGUI9680, isa16_svga_tgui9680_device)


#endif // MAME_BUS_ISA_SVGA_TRIDENT_H
