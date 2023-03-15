// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_NS32382_H
#define MAME_MACHINE_NS32382_H

#pragma once

#include "cpu/ns32000/slave.h"

class ns32382_device
	: public device_t
	, public ns32000_fast_slave_interface
	, public ns32000_mmu_interface
{
public:
	ns32382_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// ns32000_slave_interface overrides
	virtual void state_add(device_state_interface &parent, int &index) override;

	// ns32000_fast_slave_interface overrides
	virtual u32 read_st(int *icount = nullptr) override;
	virtual void write(u32 data) override;
	virtual u32 read() override;

	// ns32000_mmu_interface overrides
	virtual translate_result translate(address_space &space, unsigned st, u32 &address, bool user, bool write, bool pfs = false, bool debug = false) override;

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void execute();

private:
	// registers
	u32 m_bar;    // breakpoint address register
	u32 m_bdr;    // breakpoint data register
	u32 m_bear;   // bus error address register
	u32 m_bmr;    // breakpoint mask register
	u32 m_mcr;    // memory management control register
	u32 m_msr;    // memory management status register
	u32 m_ptb[2]; // page table base registers
	u32 m_tear;   // translation exception address register

	// operating state
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

DECLARE_DEVICE_TYPE(NS32382, ns32382_device)

#endif // MAME_MACHINE_NS32382_H
