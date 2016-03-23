// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000, TMS1070, TMS1040, TMS1200

*/

#ifndef _TMS1000_H_
#define _TMS1000_H_

#include "tms1k_base.h"


class tms1000_cpu_device : public tms1k_base_device
{
public:
	tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;


	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};

class tms1070_cpu_device : public tms1000_cpu_device
{
public:
	tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tms1040_cpu_device : public tms1000_cpu_device
{
public:
	tms1040_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1200_cpu_device : public tms1000_cpu_device
{
public:
	tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS1000;
extern const device_type TMS1070;
extern const device_type TMS1040;
extern const device_type TMS1200;

#endif /* _TMS1000_H_ */
