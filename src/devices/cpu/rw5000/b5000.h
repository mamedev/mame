// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 MCU

*/

#ifndef MAME_CPU_RW5000_B5000_H
#define MAME_CPU_RW5000_B5000_H

#pragma once

#include "rw5000base.h"

// pinout reference (preliminary, other A/B5xxx pinouts are same or very similar)

/*
            _____   _____
    VSS  1 |*    \_/     | 42 NC
    CLK? 2 |             | 41 NC
     VC? 3 |             | 40 NC
     NC  4 |             | 39 VDD
     NC  5 |             | 38 STR8
     NC  6 |             | 37 STR7
     NC  7 |             | 36 STR6
     NC  8 |             | 35 STR5
     NC  9 |             | 34 STR4
     NC 10 |    B5000    | 33 STR3
    KB3 11 |             | 32 STR2
  DIN1? 12 |             | 31 STR1
    KB1 13 |             | 30 STR0
    KB2 14 |             | 29 SEG7
    KB4 15 |             | 28 SEG6
     NC 16 |             | 27 SEG5
     NC 17 |             | 26 SEG4
     NC 18 |             | 25 SEG3
     NC 19 |             | 24 SEG2
     NC 20 |      _      | 23 SEG1
     NC 21 |_____/ \_____| 22 SEG0

*/

class b5000_cpu_device : public rw5000_base_device
{
public:
	b5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	b5000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	virtual bool op_canskip(u8 op) override { return !op_is_tl(op); }
	virtual bool op_is_tl(u8 op);
	virtual bool op_is_lb(u8 op);
	virtual bool op_is_atb(u8 op);
	virtual void reset_pc() override { set_pc(7, 0); }
	virtual u8 sr_page() { return 0; }
	virtual u16 decode_digit(u8 data);

	void program_448x8(address_map &map) ATTR_COLD;
	void data_45x4(address_map &map) ATTR_COLD;

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void set_pc(u8 pu, u8 pl);
	void set_bu(u8 bu);
	void seg_w(u16 seg);
	void op_illegal();

	// opcode handlers
	virtual void op_tl();
	virtual void op_tra_step() override;
	virtual void op_ret_step() override;
	virtual void op_nop();

	virtual void op_lb(u8 bl);
	virtual void op_atb();
	virtual void op_lda();
	virtual void op_exc0();
	virtual void op_excp();
	virtual void op_excm();
	virtual void op_sm();
	virtual void op_rsm();
	virtual void op_tm();
	virtual void op_tam();

	virtual void op_lax();
	virtual void op_comp();
	virtual void op_adx();
	virtual void op_add();
	virtual void op_sc();
	virtual void op_rsc();
	virtual void op_tc();

	virtual void op_kseg();
	virtual void op_atb_step() override;
	virtual void op_tkb();
	virtual void op_tkbs();
	virtual void op_read();
	virtual void op_tdin();
};


DECLARE_DEVICE_TYPE(B5000, b5000_cpu_device)

#endif // MAME_CPU_RW5000_B5000_H
