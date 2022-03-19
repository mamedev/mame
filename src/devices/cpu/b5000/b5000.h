// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 MCU core implementation

*/

#ifndef MAME_CPU_B5000_B5000_H
#define MAME_CPU_B5000_B5000_H

#pragma once

#include "b5000base.h"

// pinout reference

/*

*/

class b5000_cpu_device : public b5000_base_device
{
public:
	b5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	virtual bool op_canskip(u8 op) override;
	virtual bool op_is_lb(u8 op);
	virtual void reset_pc() override { set_pc(7, 0); }
	virtual u8 sr_page() { return 0; }
	virtual u16 decode_digit(u8 data);

	void data_map(address_map &map);
	void program_map(address_map &map);

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void set_pc(u8 pu, u8 pl);
	void set_bu(u8 bu);
	void op_illegal();

	// opcode handlers
	virtual void op_tl();
	virtual void op_tra() override;
	virtual void op_ret() override;
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
	virtual void op_atbz() override;
	virtual void op_tkb();
	virtual void op_tkbs() override;
	virtual void op_read();
	virtual void op_tdin();
};


DECLARE_DEVICE_TYPE(B5000, b5000_cpu_device)

#endif // MAME_CPU_B5000_B5000_H
