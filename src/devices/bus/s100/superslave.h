// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Advanced Digital Corporation Super Slave card emulation

**********************************************************************/

#ifndef MAME_BUS_S100_SUPERSLAVE_H
#define MAME_BUS_S100_SUPERSLAVE_H

#pragma once

#include "s100.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/ram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_superslave_device

class s100_superslave_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	s100_superslave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	void memctrl_w(uint8_t data);
	uint8_t status_r();
	void cmd_w(uint8_t data);

	required_device<z80_device> m_maincpu;
	required_device<com8116_device> m_dbrg;
	required_device<ram_device> m_ram;
	required_device_array<rs232_port_device, 4> m_rs232;
	required_memory_region m_rom;

	uint8_t m_memctrl;
	uint8_t m_cmd;
};


// device type definition
DECLARE_DEVICE_TYPE(S100_SUPERSLAVE, s100_superslave_device)

#endif // MAME_BUS_S100_SUPERSLAVE_H
