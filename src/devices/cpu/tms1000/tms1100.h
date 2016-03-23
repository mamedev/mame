// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1100, TMS1170, TMS1300, TMS1370

*/

#ifndef _TMS1100_H_
#define _TMS1100_H_

#include "tms1000.h"


class tms1100_cpu_device : public tms1000_cpu_device
{
public:
	tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset() override;

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual void op_setr() override;
	virtual void op_rstr() override;
};

class tms1170_cpu_device : public tms1100_cpu_device
{
public:
	tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tms1300_cpu_device : public tms1100_cpu_device
{
public:
	tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tms1370_cpu_device : public tms1100_cpu_device
{
public:
	tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS1100;
extern const device_type TMS1170;
extern const device_type TMS1300;
extern const device_type TMS1370;


#endif /* _TMS1100_H_ */
