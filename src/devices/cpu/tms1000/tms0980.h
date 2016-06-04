// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0980, TMS1980

*/

#ifndef _TMS0980_H_
#define _TMS0980_H_

#include "tms0970.h"


class tms0980_cpu_device : public tms0970_cpu_device
{
public:
	tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual UINT32 decode_fixed(UINT16 op);
	virtual UINT32 decode_micro(UINT8 sel);
	virtual void device_reset() override;

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT8 read_k_input() override;
	virtual void set_cki_bus() override;
	virtual UINT32 read_micro();
	virtual void read_opcode() override;

	virtual void op_comx() override;
};

class tms1980_cpu_device : public tms0980_cpu_device
{
public:
	tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void write_o_output(UINT8 index) override { tms1k_base_device::write_o_output(index); }
	virtual UINT8 read_k_input() override { return tms1k_base_device::read_k_input(); }

	virtual void op_setr() override { tms1k_base_device::op_setr(); }
	virtual void op_tdo() override;
};


extern const device_type TMS0980;
extern const device_type TMS1980;

#endif /* _TMS0980_H_ */
