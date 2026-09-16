// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A5500 MCU

*/

#ifndef MAME_CPU_RW5000_A5500_H
#define MAME_CPU_RW5000_A5500_H

#pragma once

#include "a5000.h"


class a5500_cpu_device : public a5000_cpu_device
{
public:
	a5500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	a5500_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	virtual bool op_is_tl(u8 op) override;
	virtual bool op_is_lb(u8 op) override;
	virtual void reset_pc() override { set_pc(0, 0); }
	virtual u8 sr_page() override { return 15; }

	void program_768x8(address_map &map) ATTR_COLD;
	void data_48x4(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(A5500, a5500_cpu_device)

#endif // MAME_CPU_RW5000_A5500_H
