// license:BSD-3-Clause
// copyright-holders:hap
/*

  The ChessMachine DR by Tasc

*/

#ifndef MAME_BUS_ISA_CHESSMDR_H
#define MAME_BUS_ISA_CHESSMDR_H

#pragma once

#include "isa.h"
#include "machine/chessmachine.h"


class isa8_chessmdr_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_chessmdr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<chessmachine_device> m_chessm;

	bool m_installed;

	DECLARE_READ8_MEMBER(chessmdr_r);
	DECLARE_WRITE8_MEMBER(chessmdr_w);
};


DECLARE_DEVICE_TYPE(ISA8_CHESSMDR, isa8_chessmdr_device)

#endif // MAME_BUS_ISA_CHESSMDR_H
