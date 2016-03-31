// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0950, TMS0970, TMS1990

*/

#ifndef _TMS0970_H_
#define _TMS0970_H_

#include "tms1000.h"


class tms0970_cpu_device : public tms1000_cpu_device
{
public:
	tms0970_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms0970_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void write_o_output(UINT8 index) override;

	virtual void op_setr() override;
	virtual void op_tdo() override;
};

class tms0950_cpu_device : public tms0970_cpu_device
{
public:
	tms0950_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// overrides
	virtual void device_reset() override { tms1000_cpu_device::device_reset(); }
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void op_rstr() override { ; } // assume it has no RSTR or CLO
	virtual void op_clo() override { ; } // "
};

class tms1990_cpu_device : public tms0970_cpu_device
{
public:
	tms1990_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS0950;
extern const device_type TMS0970;
extern const device_type TMS1990;

#endif /* _TMS0970_H_ */
