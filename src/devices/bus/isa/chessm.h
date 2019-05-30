// license:BSD-3-Clause
// copyright-holders:hap
/*

  The ChessMachine by Tasc

*/

#ifndef MAME_BUS_ISA_CHESSM_H
#define MAME_BUS_ISA_CHESSM_H

#pragma once

#include "isa.h"
#include "cpu/arm/arm.h"
#include "machine/gen_latch.h"


class isa8_chessm_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_chessm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<arm_cpu_device> m_maincpu;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<generic_latch_8_device> m_sublatch;

	u8 m_ram_offset;
	bool m_suspended;
	bool m_installed;

	DECLARE_READ8_MEMBER(chessm_r);
	DECLARE_WRITE8_MEMBER(chessm_w);

	void chessm_mem(address_map &map);
};


DECLARE_DEVICE_TYPE(ISA8_CHESSM, isa8_chessm_device)


#endif // MAME_BUS_ISA_CHESSM_H
