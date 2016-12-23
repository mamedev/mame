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
	tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint8_t o_pins, uint8_t r_pins, uint8_t pc_bits, uint8_t byte_bits, uint8_t x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset() override;

	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	virtual void op_setr() override;
	virtual void op_rstr() override;
};

class tms1170_cpu_device : public tms1100_cpu_device
{
public:
	tms1170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tms1300_cpu_device : public tms1100_cpu_device
{
public:
	tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tms1370_cpu_device : public tms1100_cpu_device
{
public:
	tms1370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


extern const device_type TMS1100;
extern const device_type TMS1170;
extern const device_type TMS1300;
extern const device_type TMS1370;


#endif /* _TMS1100_H_ */
