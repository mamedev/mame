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
	// overrides
	//virtual void device_start() override;

	virtual u32 decode_fixed(u16 op) override { return 0; } // not yet
	virtual u32 decode_micro(u8 sel) override;
	virtual void device_reset() override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options) override;

	virtual machine_config_constructor device_mconfig_additions() const override;
};


DECLARE_DEVICE_TYPE(TP0320, tp0320_cpu_device)

#endif // MAME_CPU_TMS1000_TP0320_H
