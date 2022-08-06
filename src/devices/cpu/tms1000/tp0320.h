// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TP0320

*/

#ifndef MAME_CPU_TMS1000_TP0320_H
#define MAME_CPU_TMS1000_TP0320_H

#pragma once

#include "tms0980.h"


class tp0320_cpu_device : public tms0980_cpu_device
{
public:
	tp0320_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	void ram_192x4(address_map &map);

	// overrides
	//virtual void device_start() override;

	virtual u32 decode_fixed(u16 op) override { return 0; } // not yet
	virtual u32 decode_micro(u8 sel) override;
	virtual void device_reset() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void device_add_mconfig(machine_config &config) override;
};


DECLARE_DEVICE_TYPE(TP0320, tp0320_cpu_device)

#endif // MAME_CPU_TMS1000_TP0320_H
