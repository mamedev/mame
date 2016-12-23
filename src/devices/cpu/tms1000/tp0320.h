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
	tp0320_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// overrides
	//virtual void device_start() override;

	virtual uint32_t decode_fixed(uint16_t op) override { return 0; } // not yet
	virtual uint32_t decode_micro(uint8_t sel) override;
	virtual void device_reset() override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	virtual machine_config_constructor device_mconfig_additions() const override;
};


extern const device_type TP0320;

#endif /* _TP0320_H_ */
