// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_Z80_NSC800_H
#define MAME_CPU_Z80_NSC800_H

#pragma once

#include "z80.h"

enum
{
	NSC800_RSTA = Z80_INPUT_LINE_MAX,
	NSC800_RSTB,
	NSC800_RSTC
};


class nsc800_device : public z80_device
{
public:
	nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_set_input(int inputnum, int state) override;

	virtual void do_op() override;
	u8 m_nsc800_irq_state[3]; // state of NSC800 restart interrupts A, B, C
};

DECLARE_DEVICE_TYPE(NSC800, nsc800_device)


#endif // MAME_CPU_Z80_NSC800_H
