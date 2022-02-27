// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NS32081_H
#define MAME_MACHINE_NS32081_H

#pragma once

#include "cpu/ns32000/slave.h"

class ns32081_device
	: public device_t
	, public ns32000_slow_slave_interface
{
public:
	ns32081_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void state_add(device_state_interface &parent, int &index) override;

	virtual u16 read_st(int *icount = nullptr) override;
	virtual u16 read_op() override;

	virtual void write_id(u16 data) override;
	virtual void write_op(u16 data) override;

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void execute();
	void complete(s32 param);

private:
	emu_timer *m_complete;

	// registers
	u32 m_fsr;
	u32 m_f[8];

	// operating state
	u8 m_idbyte;
	u16 m_opword;
	struct operand
	{
		unsigned expected;
		unsigned issued;
		u64 value;
	}
	m_op[3];
	u16 m_status;

	// implementation state
	unsigned m_state;
	unsigned m_tcy;
};

DECLARE_DEVICE_TYPE(NS32081, ns32081_device)

#endif // MAME_MACHINE_NS32081_H
