// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B6100 MCU

*/

#ifndef MAME_CPU_B5000_B6100_H
#define MAME_CPU_B5000_B6100_H

#pragma once

#include "b6000.h"

// pinout reference

/*

*/

class b6100_cpu_device : public b6000_cpu_device
{
public:
	b6100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	b6100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	virtual bool op_canskip(u8 op) override;
	virtual bool op_is_lb(u8 op) override;
	virtual void reset_pc() override { set_pc(0, 0); }
	virtual u8 sr_page() override { return 15; }
	virtual u16 decode_digit(u8 data) override;

	void program_896x8(address_map &map);
	void data_48x4(address_map &map);

	// opcode handlers
	virtual void op_read() override;
};


DECLARE_DEVICE_TYPE(B6100, b6100_cpu_device)

#endif // MAME_CPU_B5000_B6100_H
