// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1400, TMS1470, TMS1600, TMS1670

*/

#ifndef _TMS1400_H_
#define _TMS1400_H_

#include "tms1100.h"


class tms1400_cpu_device : public tms1100_cpu_device
{
public:
	tms1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1400_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void op_br() override;
	virtual void op_call() override;
	virtual void op_retn() override;

	virtual void op_setr() override { tms1k_base_device::op_setr(); } // no anomaly with MSB of X register
	virtual void op_rstr() override { tms1k_base_device::op_rstr(); } // "
};

class tms1470_cpu_device : public tms1400_cpu_device
{
public:
	tms1470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1600_cpu_device : public tms1400_cpu_device
{
public:
	tms1600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1600_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);
};

class tms1670_cpu_device : public tms1600_cpu_device
{
public:
	tms1670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS1400;
extern const device_type TMS1470;
extern const device_type TMS1600;
extern const device_type TMS1670;


#endif /* _TMS1400_H_ */
