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
#include "bus/rs232/terminal.h"
#include "bus/rs232/null_modem.h"
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

	DECLARE_READ8_MEMBER(z80_control_r);
	DECLARE_WRITE8_MEMBER(z80_control_w);
	DECLARE_READ8_MEMBER(z80_ram_r) { return m_ram->read(offset); }
	DECLARE_WRITE8_MEMBER(z80_ram_w) { m_ram->write(offset,data); }
	
	DECLARE_WRITE_LINE_MEMBER(port1_irq) { m_isa->irq4_w(state); }
	DECLARE_WRITE_LINE_MEMBER(port2_irq) { m_isa->irq3_w(state); }
	DECLARE_WRITE_LINE_MEMBER(lpt_irq);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;


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
	
	void z80_program_map(address_map &map);
	void z80_io_map(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_BABYBLUE2, isa8_babyblue2_device)

#endif // MAME_BUS_ISA_BBLUE2_H
