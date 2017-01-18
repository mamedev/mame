// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TP0320

*/

#ifndef _TP0320_H_
#define _TP0320_H_

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


extern const device_type TP0320;

#endif /* _TP0320_H_ */
