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

	DECLARE_READ8_MEMBER(ne1000_port_r);
	DECLARE_WRITE8_MEMBER(ne1000_port_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	void ne1000_irq_w(int state);
	DECLARE_READ8_MEMBER(ne1000_mem_read);
	DECLARE_WRITE8_MEMBER(ne1000_mem_write);

	required_device<dp8390d_device> m_dp8390;
	uint8_t m_irq;
	uint8_t m_board_ram[8*1024];
	uint8_t m_prom[16];
};

DECLARE_DEVICE_TYPE(NE1000, ne1000_device)

#endif // MAME_BUS_ISA_NE1000_H
