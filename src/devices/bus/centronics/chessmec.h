// license:BSD-3-Clause
// copyright-holders:hap
/*

  The ChessMachine EC by Tasc

*/

#ifndef MAME_BUS_CENTRONICS_CHESSMEC_H
#define MAME_BUS_CENTRONICS_CHESSMEC_H

#pragma once

#include "ctronics.h"
#include "machine/chessmachine.h"


class centronics_chessmec_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_chessmec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void input_data0(int state) override { if (started()) m_chessm->data0_w(state); }
	virtual void input_data1(int state) override { if (started()) m_chessm->reset_w(state); }
	virtual void input_data7(int state) override { if (started()) m_chessm->data1_w(state); }

private:
	required_device<chessmachine_device> m_chessm;
};


DECLARE_DEVICE_TYPE(CENTRONICS_CHESSMEC, centronics_chessmec_device)

#endif // MAME_BUS_CENTRONICS_CHESSMEC_H
