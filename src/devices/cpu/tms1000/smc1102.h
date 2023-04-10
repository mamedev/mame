// license:BSD-3-Clause
// copyright-holders:hap
/*

  Suwa Seikosha (now Seiko Epson) SMC1102, SMC1112

*/

#ifndef MAME_CPU_TMS1000_SMC1102_H
#define MAME_CPU_TMS1000_SMC1102_H

#pragma once

#include "tms1100.h"


// pinout reference (brief)

/*

SMC1102F (60-pin QFP):              SMC1102C (24-pin DIP):
- R0-R7: pin 25-21,19-17            - R0-R5: pin 41-34
- K1-K8: pin 6-9                    - K1-K8: pin 25-28
- HLT: pin 13, INIT: pin 14         - HLT: pin 32, INIT: pin 33
- COM1-COM4: pin 1-4                - COM1-COM4: pin 20,22-24
- D0-D31: pin 27,28,30-44,46-60     - D0-D18: pin 1-19

SMC1112F (60-pin QFP):
- R0-R7: pin 26-19
- K1-K8: pin 6-9
- HLT: pin 15, INIT: pin 16
- COM1-COM4: pin 1-4
- D0-D31: pin 28,30-60

*/


class smc1102_cpu_device : public tms1100_cpu_device
{
public:
	smc1102_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	smc1102_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override { }
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 decode_micro(offs_t offset) override;

	virtual void write_o_reg(u8 index) override { } // no O pins

	virtual void op_setr() override { tms1k_base_device::op_setr(); } // no anomaly with MSB of X register
	virtual void op_rstr() override { tms1k_base_device::op_rstr(); } // "

	virtual void op_extra() override;

private:
	void op_halt();
	void op_intdis();
	void op_inten();
	void op_selin();
	void op_tasr();
	void op_tmset();
	void op_tsg();
};

class smc1112_cpu_device : public smc1102_cpu_device
{
public:
	smc1112_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(SMC1102, smc1102_cpu_device)
DECLARE_DEVICE_TYPE(SMC1112, smc1112_cpu_device)

#endif // MAME_CPU_TMS1000_SMC1102_H
