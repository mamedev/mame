// license:BSD-3-Clause
// copyright-holders:hap
/*

  Tasc SmartBoard SB30 (LPT interface)

*/

#ifndef MAME_BUS_CENTRONICS_SMARTBOARD_H
#define MAME_BUS_CENTRONICS_SMARTBOARD_H

#pragma once

#include "ctronics.h"
#include "machine/smartboard.h"


class centronics_smartboard_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_smartboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void input_data0(int state) override { if (started()) m_smartboard->data0_w(state); }
	virtual void input_data7(int state) override { if (started()) m_smartboard->data1_w(state); }

private:
	required_device<tasc_sb30_device> m_smartboard;
};


DECLARE_DEVICE_TYPE(CENTRONICS_SMARTBOARD, centronics_smartboard_device)

#endif // MAME_BUS_CENTRONICS_SMARTBOARD_H
