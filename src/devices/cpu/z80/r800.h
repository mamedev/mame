// license:BSD-3-Clause
// copyright-holders:AJR,Juergen Buchmueller,Wilbert Pol
/***************************************************************************

    ASCII R800 CPU

***************************************************************************/

#ifndef MAME_CPU_Z80_R800_H
#define MAME_CPU_Z80_R800_H

#pragma once

#include "z80.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class r800_device : public z80_device
{
public:
	// device type constructor
	r800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }

	virtual u8 data_read(u16 addr) override;
	virtual void data_write(u16 addr, u8 value) override;
	virtual u8 opcode_read() override;
	virtual u8 arg_read() override;
	void execute_cycles(u8 icount);

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void halt();
	void leave_halt();
	u8 in(u16 port);
	void out(u16 port, u8 value);
	u8 rm(u16 addr);
	u8 rm_reg(u16 addr);
	void rm16(u16 addr, PAIR &r);
	void wm(u16 addr, u8 value);
	void wm16(u16 addr, PAIR &r);
	void wm16_sp(PAIR &r);
	u8 rop();
	u8 arg();
	u16 arg16();
	void eax();
	void eay();
	void pop(PAIR &r);
	void push(PAIR &r);
	void jp(void);
	void jp_cond(bool cond);
	void jr();
	void jr_cond(bool cond, u8 opcode);
	void call();
	void call_cond(bool cond, u8 opcode);
	void ret_cond(bool cond, u8 opcode);
	void retn();
	void reti();
	void ld_r_a();
	void ld_a_r();
	void ld_i_a();
	void ld_a_i();
	void rst(u16 addr);
	u8 inc(u8 value);
	u8 dec(u8 value);
	void rlca();
	void rrca();
	void rla();
	void rra();
	void rrd();
	void rld();
	void add_a(u8 value);
	void adc_a(u8 value);
	void sub(u8 value);
	void sbc_a(u8 value);
	void neg();
	void daa();
	void and_a(u8 value);
	void or_a(u8 value);
	void xor_a(u8 value);
	void cp(u8 value);
	void exx();
	void ex_sp(PAIR &r);
	void add16(PAIR &dr, PAIR &sr);
	void adc_hl(PAIR &r);
	void sbc_hl(PAIR &r);
	u8 rlc(u8 value);
	u8 rrc(u8 value);
	u8 rl(u8 value);
	u8 rr(u8 value);
	u8 sla(u8 value);
	u8 sra(u8 value);
	u8 sll(u8 value);
	u8 srl(u8 value);
	void bit(int bit, u8 value);
	void bit_hl(int bit, u8 value);
	void bit_xy(int bit, u8 value);
	u8 res(int bit, u8 value);
	u8 set(int bit, u8 value);
	void ldi();
	void cpi();
	u8 ini(bool take_extra_cycle);
	u8 outi(bool take_extra_cycle);
	void ldd();
	void cpd();
	u8 ind(bool take_extra_cycle);
	u8 outd(bool take_extra_cycle);
	void ldir();
	void cpir();
	void inir();
	void otir();
	void lddr();
	void cpdr();
	void indr();
	void otdr();
	void ei();
	void mulub(u8 value);
	void muluw(u16 value);
	void set_f(u8 f);
	void block_io_interrupted_flags(); // TODO Remove
	void block_io_interrupted_flags(u8 data);
	void illegal_1();
	void illegal_2();

	virtual void do_rop() override;
	virtual void do_op() override;
};

// device type declaration
DECLARE_DEVICE_TYPE(R800, r800_device)

#endif // MAME_CPU_Z80_R800_H
