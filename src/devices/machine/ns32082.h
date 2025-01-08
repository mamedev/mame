// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NS32082_H
#define MAME_MACHINE_NS32082_H

#pragma once

#include "cpu/ns32000/common.h"

class ns32082_device
	: public device_t
	, public ns32000_mmu_interface
	, public ns32000_slow_slave_interface
{
public:
	ns32082_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// ns32000_slave_interface implementation
	virtual void state_add(device_state_interface &parent, int &index) override;

	// ns32000_slow_slave_interface implementation
	virtual u16 slow_status(int *icount = nullptr) override;
	virtual u16 slow_read() override;
	virtual void slow_write(u16 data) override;

	// ns32000_mmu_interface implementation
	virtual translate_result translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool pfs = false, bool suppress = false) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void execute();

	void set_msr(u32 data);
	void set_eia(u32 data) {}

private:
	// registers
	u32 m_bpr[2];
	u32 m_pf[2];
	u32 m_sc;
	u32 m_msr;
	u32 m_bcnt;
	u32 m_ptb[2];
	u32 m_eia;

	// operating state
	u8 m_idbyte;
	u16 m_opword;
	struct operand
	{
		u32 expected;
		u32 issued;
		u64 value;
	}
	m_op[3];
	u16 m_status;

	// implementation state
	u32 m_state;
	u32 m_tcy;
};

DECLARE_DEVICE_TYPE(NS32082, ns32082_device)

#endif // MAME_MACHINE_NS32082_H
