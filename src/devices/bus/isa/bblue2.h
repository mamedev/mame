// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Microlog Baby Blue II CPU Plus ISA card
 */

#ifndef MAME_BUS_ISA_BBLUE2_H
#define MAME_BUS_ISA_BBLUE2_H

#pragma once

#include "machine/ins8250.h"
#include "bus/rs232/rs232.h"
#include "machine/pc_lpt.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"

#include "isa.h"

class isa8_babyblue2_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_babyblue2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t z80_control_r(offs_t offset);
	void z80_control_w(offs_t offset, uint8_t data);
	uint8_t z80_ram_r(offs_t offset) { return m_ram->read(offset); }
	void z80_ram_w(offs_t offset, uint8_t data) { m_ram->write(offset,data); }

	void port1_irq(int state) { m_isa->irq4_w(state); }
	void port2_irq(int state) { m_isa->irq3_w(state); }
	void lpt_irq(int state);
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;


private:
	required_device<z80_device> m_z80;
	required_device<ins8250_device> m_serial1;
	required_device<ins8250_device> m_serial2;
	required_device<rs232_port_device> m_rs232_1;
	required_device<rs232_port_device> m_rs232_2;
	required_device<pc_lpt_device> m_parallel;
	required_ioport m_dsw1;
	required_ioport m_dsw2;
	required_ioport m_dsw3;
	required_ioport m_h2;
	required_device<ram_device> m_ram;

	void z80_program_map(address_map &map) ATTR_COLD;
	void z80_io_map(address_map &map) ATTR_COLD;

	bool m_devices_installed;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_BABYBLUE2, isa8_babyblue2_device)

#endif // MAME_BUS_ISA_BBLUE2_H
