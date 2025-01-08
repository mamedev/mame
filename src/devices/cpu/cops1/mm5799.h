// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor MM5799 MCU

*/

#ifndef MAME_CPU_COPS1_MM5799_H
#define MAME_CPU_COPS1_MM5799_H

#pragma once

#include "cops1base.h"

// pinout reference

/*
            ____   ____
     K1  1 |*   \_/    | 28 DO1
     K2  2 |           | 27 DO2
     K3  3 |           | 26 DO3
     K4  4 |           | 25 DO4
    INB  5 |           | 24 SI
   SYNC  6 |           | 23 SO
    OSC  7 |  MM5799N  | 22 BLK
     F3  8 |           | 21 Vdd
     F2  9 |           | 20 Sa
     F1 10 |           | 19 Sb
 PWR ON 11 |           | 18 Sc
     Sp 12 |           | 17 Sd
     Sg 13 |           | 16 Se
     Sf 14 |___________| 15 Vss

*/

class mm5799_device : public cops1_base_device
{
public:
	mm5799_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;
	virtual bool op_argument() override;

	void data_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void pop_pc();
	void push_pc();

	// opcode handlers
	void op_ad();
	void op_add();
	void op_sub();
	void op_comp();
	void op_0ta();
	void op_adx();
	void op_hxa();
	void op_tam();
	void op_sc();
	void op_rsc();
	void op_tc();

	void op_tin();
	void op_tf();
	void op_tkb();
	void op_tir();

	void op_btd();
	void op_dspa();
	void op_dsps();
	void op_axo();
	void op_ldf();
	void op_read();

	void op_go();
	void op_call();
	void op_ret();
	void op_rets();
	void op_lg();
	void op_nop();

	void op_exc();
	void op_excm();
	void op_excp();
	void op_mta();
	void op_lm();

	void op_sm();
	void op_rsm();
	void op_tm();

	void op_lb();
	void op_lbl();
	void op_atb();
	void op_bta();
	void op_hxbr();
};


DECLARE_DEVICE_TYPE(MM5799, mm5799_device)

#endif // MAME_CPU_COPS1_MM5799_H
