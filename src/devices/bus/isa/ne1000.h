// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_NE1000_H
#define MAME_BUS_ISA_NE1000_H

#pragma once

// NE1000 is 8bit has 8KB ram; NE2000 is 16bit has 16KB ram

#include "isa.h"
#include "machine/dp8390.h"

class ne1000_device: public device_t,
						public device_isa8_card_interface
{
public:
	ne1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ne1000_port_r(offs_t offset);
	void ne1000_port_w(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void ne1000_irq_w(int state);
	uint8_t ne1000_mem_read(offs_t offset);
	void ne1000_mem_write(offs_t offset, uint8_t data);

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_irq;
	uint8_t m_board_ram[8*1024];
	uint8_t m_prom[16];
};

DECLARE_DEVICE_TYPE(NE1000, ne1000_device)

#endif // MAME_BUS_ISA_NE1000_H
