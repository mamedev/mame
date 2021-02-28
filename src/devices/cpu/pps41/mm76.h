// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM76 MCU

*/

#ifndef MAME_CPU_PPS41_MM76_H
#define MAME_CPU_PPS41_MM76_H

#pragma once

#include "pps41base.h"

// pinout reference

/*
...

*/

class mm76_device : public pps41_base_device
{
public:
	mm76_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	void data_map(address_map &map);
	void program_map(address_map &map);

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void pop_pc();
	void push_pc();
	void op_illegal();

	virtual bool op_is_prefix(u8 op) override { return (op & 0xf0) == 0x30; };

	// opcode handlers
	void op_nop();
};


DECLARE_DEVICE_TYPE(MM76, mm76_device)

#endif // MAME_CPU_PPS41_MM76_H
