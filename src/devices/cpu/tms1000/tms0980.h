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
	tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, uint8_t o_pins, uint8_t r_pins, uint8_t pc_bits, uint8_t byte_bits, uint8_t x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual uint32_t decode_fixed(uint16_t op);
	virtual uint32_t decode_micro(uint8_t sel);
	virtual void device_reset() override;

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual uint32_t disasm_min_opcode_bytes() const override { return 2; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	virtual uint8_t read_k_input() override;
	virtual void set_cki_bus() override;
	virtual uint32_t read_micro();
	virtual void read_opcode() override;

	virtual void op_comx() override;
};

class tms1980_cpu_device : public tms0980_cpu_device
{
public:
	tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void write_o_output(uint8_t index) override { tms1k_base_device::write_o_output(index); }
	virtual uint8_t read_k_input() override { return tms1k_base_device::read_k_input(); }

	virtual void op_setr() override { tms1k_base_device::op_setr(); }
	virtual void op_tdo() override;
};


extern const device_type TMS0980;
extern const device_type TMS1980;

#endif /* _TMS0980_H_ */
