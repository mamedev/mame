// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_FP1020FD_H
#define MAME_BUS_FP1020FD_H

#pragma once

#include "fp1060io_exp.h"
#include "imagedev/floppy.h"
#include "machine/timer.h"
#include "machine/upd765.h"

class fp1020fd_device : public fp1060io_exp_device
{
public:
	fp1020fd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void io_map(address_map &map) override ATTR_COLD;
	virtual u8 get_id() override { return 0x04; };


protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	TIMER_CALLBACK_MEMBER(motor_timeout_cb);
	emu_timer *m_motor_timer;

	void intrq_w(int state);
	void drq_w(int state);
};

DECLARE_DEVICE_TYPE(FP1020FD, fp1020fd_device)


#endif // MAME_BUS_FP1020FD_H
