// license:BSD-3-Clause
// copyright-holders:hap
/*

  The ChessMachine SR by Tasc

*/

#ifndef MAME_BUS_ISA_CHESSMSR_H
#define MAME_BUS_ISA_CHESSMSR_H

#pragma once

#include "isa.h"
#include "cpu/arm/arm.h"
#include "machine/gen_latch.h"


class isa8_chessmsr_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_chessmsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<arm_cpu_device> m_maincpu;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<generic_latch_8_device> m_sublatch;
	std::unique_ptr<u32[]> m_ram;

	u8 m_ram_offset;
	bool m_suspended;
	bool m_installed;

	uint8_t chessmsr_r(offs_t offset);
	void chessmsr_w(offs_t offset, uint8_t data);

	void chessmsr_mem(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(ISA8_CHESSMSR, isa8_chessmsr_device)

#endif // MAME_BUS_ISA_CHESSMSR_H
