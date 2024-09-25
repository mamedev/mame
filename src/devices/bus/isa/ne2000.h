// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_NE2000_H
#define MAME_BUS_ISA_NE2000_H

#pragma once

#include "isa.h"
#include "machine/dp8390.h"

class ne2000_device: public device_t,
						public device_isa16_card_interface
{
public:
	ne2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t ne2000_port_r(offs_t offset, uint16_t mem_mask = ~0);
	void ne2000_port_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void ne2000_irq_w(int state);
	uint8_t ne2000_mem_read(offs_t offset);
	void ne2000_mem_write(offs_t offset, uint8_t data);

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_irq;
	uint8_t m_board_ram[16*1024];
	uint8_t m_prom[16];
};

DECLARE_DEVICE_TYPE(NE2000, ne2000_device)

#endif // MAME_BUS_ISA_NE2000_H
