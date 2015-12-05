// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
/*

Notice: please do not modify this file, except in case of compile- or critical emulation error
A more accurate implementation is in mame/alpha8201.*

cpu/alph8201/ will be removed soon




*/

	/**************************************************************************\
	*                      Alpha8201 Emulator                                  *
	*                                                                          *
	*                    Copyright Tatsuyuki Satoh                             *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	\**************************************************************************/

#pragma once

#ifndef __ALPH8201_H__
#define __ALPH8201_H__

enum
{
	ALPHA8201_PC=1,
	ALPHA8201_SP,
	ALPHA8201_RB,
	ALPHA8201_MB,
//
	ALPHA8201_CF,
	ALPHA8201_ZF,
//
	ALPHA8201_IX0,
	ALPHA8201_IX1,
	ALPHA8201_IX2,
	ALPHA8201_LP0,
	ALPHA8201_LP1,
	ALPHA8201_LP2,
	ALPHA8201_A,
	ALPHA8201_B,
//
	ALPHA8201_R0,ALPHA8201_R1,ALPHA8201_R2,ALPHA8201_R3,
	ALPHA8201_R4,ALPHA8201_R5,ALPHA8201_R6,ALPHA8201_R7
};


class alpha8201_cpu_device : public cpu_device
{
public:
	// construction/destruction
	alpha8201_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	alpha8201_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 16; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	UINT8 M_RDMEM(UINT16 A) { return m_program->read_byte(A); }
	void M_WRMEM(UINT16 A,UINT8 V) { m_program->write_byte(A, V); }
	UINT8 M_RDOP(UINT16 A) { return m_direct->read_byte(A); }
	UINT8 M_RDOP_ARG(UINT16 A) { return m_direct->read_byte(A); }
	UINT8 RD_REG(UINT8 x) { return m_RAM[(m_regPtr<<3)+(x)]; }
	void WR_REG(UINT8 x, UINT8 d) { m_RAM[(m_regPtr<<3)+(x)]=(d); }

	unsigned M_RDMEM_OPCODE();
	void M_ADD(UINT8 dat);
	void M_ADDB(UINT8 dat);
	void M_SUB(UINT8 dat);
	void M_AND(UINT8 dat);
	void M_OR(UINT8 dat);
	void M_XOR(UINT8 dat);
	void M_JMP(UINT8 dat);
	void M_UNDEFINED();
	void M_UNDEFINED2();

	void undefined()    { M_UNDEFINED(); }
	void undefined2()   { M_UNDEFINED2(); }

	void nop()       { }
	void rora()      { m_cf = m_A &1;     m_A = (m_A>>1) | (m_A<<7); }
	void rola()      { m_cf = (m_A>>7)&1; m_A = (m_A<<1) | (m_A>>7); }
	void inc_b()         { M_ADDB(0x02); }
	void dec_b()         { M_ADDB(0xfe); }
	void inc_a()         { M_ADD(0x01); }
	void dec_a()         { M_ADD(0xff); }
	void cpl()       { m_A ^= 0xff; };

	void ld_a_ix0_0() { m_A = M_RDMEM(m_ix0.w.l+0); }
	void ld_a_ix0_1() { m_A = M_RDMEM(m_ix0.w.l+1); }
	void ld_a_ix0_2() { m_A = M_RDMEM(m_ix0.w.l+2); }
	void ld_a_ix0_3() { m_A = M_RDMEM(m_ix0.w.l+3); }
	void ld_a_ix0_4() { m_A = M_RDMEM(m_ix0.w.l+4); }
	void ld_a_ix0_5() { m_A = M_RDMEM(m_ix0.w.l+5); }
	void ld_a_ix0_6() { m_A = M_RDMEM(m_ix0.w.l+6); }
	void ld_a_ix0_7() { m_A = M_RDMEM(m_ix0.w.l+7); }

	void ld_a_ix1_0() { m_A = M_RDMEM(m_ix1.w.l+0); }
	void ld_a_ix1_1() { m_A = M_RDMEM(m_ix1.w.l+1); }
	void ld_a_ix1_2() { m_A = M_RDMEM(m_ix1.w.l+2); }
	void ld_a_ix1_3() { m_A = M_RDMEM(m_ix1.w.l+3); }
	void ld_a_ix1_4() { m_A = M_RDMEM(m_ix1.w.l+4); }
	void ld_a_ix1_5() { m_A = M_RDMEM(m_ix1.w.l+5); }
	void ld_a_ix1_6() { m_A = M_RDMEM(m_ix1.w.l+6); }
	void ld_a_ix1_7() { m_A = M_RDMEM(m_ix1.w.l+7); }

	void ld_ix2_0_a() { M_WRMEM(m_ix2.w.l+0,m_A); }
	void ld_ix2_1_a() { M_WRMEM(m_ix2.w.l+1,m_A); }
	void ld_ix2_2_a() { M_WRMEM(m_ix2.w.l+2,m_A); }
	void ld_ix2_3_a() { M_WRMEM(m_ix2.w.l+3,m_A); }
	void ld_ix2_4_a() { M_WRMEM(m_ix2.w.l+4,m_A); }
	void ld_ix2_5_a() { M_WRMEM(m_ix2.w.l+5,m_A); }
	void ld_ix2_6_a() { M_WRMEM(m_ix2.w.l+6,m_A); }
	void ld_ix2_7_a() { M_WRMEM(m_ix2.w.l+7,m_A); }

	void ld_ix0_0_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+0); }
	void ld_ix0_1_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+1); }
	void ld_ix0_2_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+2); }
	void ld_ix0_3_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+3); }
	void ld_ix0_4_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+4); }
	void ld_ix0_5_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+5); }
	void ld_ix0_6_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+6); }
	void ld_ix0_7_b() { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l+7); }

	void bit_r0_0()  { m_zf = RD_REG(0)&(1<<0)?0:1; }
	void bit_r0_1()  { m_zf = RD_REG(0)&(1<<1)?0:1; }
	void bit_r0_2()  { m_zf = RD_REG(0)&(1<<2)?0:1; }
	void bit_r0_3()  { m_zf = RD_REG(0)&(1<<3)?0:1; }
	void bit_r0_4()  { m_zf = RD_REG(0)&(1<<4)?0:1; }
	void bit_r0_5()  { m_zf = RD_REG(0)&(1<<5)?0:1; }
	void bit_r0_6()  { m_zf = RD_REG(0)&(1<<6)?0:1; }
	void bit_r0_7()  { m_zf = RD_REG(0)&(1<<7)?0:1; }

	void ld_a_n()    { m_A = M_RDMEM_OPCODE(); }

	void ld_a_r0()   { m_A = RD_REG(0); m_zf = (m_A==0); }
	void ld_a_r1()   { m_A = RD_REG(1); m_zf = (m_A==0); }
	void ld_a_r2()   { m_A = RD_REG(2); m_zf = (m_A==0); }
	void ld_a_r3()   { m_A = RD_REG(3); m_zf = (m_A==0); }
	void ld_a_r4()   { m_A = RD_REG(4); m_zf = (m_A==0); }
	void ld_a_r5()   { m_A = RD_REG(5); m_zf = (m_A==0); }
	void ld_a_r6()   { m_A = RD_REG(6); m_zf = (m_A==0); }
	void ld_a_r7()   { m_A = RD_REG(7); m_zf = (m_A==0); }

	void ld_r0_a()   { WR_REG(0,m_A); }
	void ld_r1_a()   { WR_REG(1,m_A); }
	void ld_r2_a()   { WR_REG(2,m_A); }
	void ld_r3_a()   { WR_REG(3,m_A); }
	void ld_r4_a()   { WR_REG(4,m_A); }
	void ld_r5_a()   { WR_REG(5,m_A); }
	void ld_r6_a()   { WR_REG(6,m_A); }
	void ld_r7_a()   { WR_REG(7,m_A); }

	void add_a_n()   { M_ADD(M_RDMEM_OPCODE()); }

	void add_a_r0()  { M_ADD(RD_REG(0)); }
	void add_a_r1()  { M_ADD(RD_REG(1)); }
	void add_a_r2()  { M_ADD(RD_REG(2)); }
	void add_a_r3()  { M_ADD(RD_REG(3)); }
	void add_a_r4()  { M_ADD(RD_REG(4)); }
	void add_a_r5()  { M_ADD(RD_REG(5)); }
	void add_a_r6()  { M_ADD(RD_REG(6)); }
	void add_a_r7()  { M_ADD(RD_REG(7)); }

	void sub_a_n()   { M_SUB(M_RDMEM_OPCODE()); }

	void sub_a_r0()  { M_SUB(RD_REG(0)); }
	void sub_a_r1()  { M_SUB(RD_REG(1)); }
	void sub_a_r2()  { M_SUB(RD_REG(2)); }
	void sub_a_r3()  { M_SUB(RD_REG(3)); }
	void sub_a_r4()  { M_SUB(RD_REG(4)); }
	void sub_a_r5()  { M_SUB(RD_REG(5)); }
	void sub_a_r6()  { M_SUB(RD_REG(6)); }
	void sub_a_r7()  { M_SUB(RD_REG(7)); }

	void and_a_n()   { M_AND(M_RDMEM_OPCODE()); }

	void and_a_r0()  { M_AND(RD_REG(0)); }
	void and_a_r1()  { M_AND(RD_REG(1)); }
	void and_a_r2()  { M_AND(RD_REG(2)); }
	void and_a_r3()  { M_AND(RD_REG(3)); }
	void and_a_r4()  { M_AND(RD_REG(4)); }
	void and_a_r5()  { M_AND(RD_REG(5)); }
	void and_a_r6()  { M_AND(RD_REG(6)); }
	void and_a_r7()  { M_AND(RD_REG(7)); }

	void or_a_n()    { M_OR(M_RDMEM_OPCODE()); }

	void or_a_r0()   { M_OR(RD_REG(0)); }
	void or_a_r1()   { M_OR(RD_REG(1)); }
	void or_a_r2()   { M_OR(RD_REG(2)); }
	void or_a_r3()   { M_OR(RD_REG(3)); }
	void or_a_r4()   { M_OR(RD_REG(4)); }
	void or_a_r5()   { M_OR(RD_REG(5)); }
	void or_a_r6()   { M_OR(RD_REG(6)); }
	void or_a_r7()   { M_OR(RD_REG(7)); }

	void add_ix0_0()     { }
	void add_ix0_1()     { m_ix0.b.l += 1; }
	void add_ix0_2()     { m_ix0.b.l += 2; }
	void add_ix0_3()     { m_ix0.b.l += 3; }
	void add_ix0_4()     { m_ix0.b.l += 4; }
	void add_ix0_5()     { m_ix0.b.l += 5; }
	void add_ix0_6()     { m_ix0.b.l += 6; }
	void add_ix0_7()     { m_ix0.b.l += 7; }
	void add_ix0_8()     { m_ix0.b.l += 8; }
	void add_ix0_9()     { m_ix0.b.l += 9; }
	void add_ix0_a()     { m_ix0.b.l += 10; }
	void add_ix0_b()     { m_ix0.b.l += 11; }
	void add_ix0_c()     { m_ix0.b.l += 12; }
	void add_ix0_d()     { m_ix0.b.l += 13; }
	void add_ix0_e()     { m_ix0.b.l += 14; }
	void add_ix0_f()     { m_ix0.b.l += 15; }

	void add_ix1_0()     { }
	void add_ix1_1()     { m_ix1.b.l += 1; }
	void add_ix1_2()     { m_ix1.b.l += 2; }
	void add_ix1_3()     { m_ix1.b.l += 3; }
	void add_ix1_4()     { m_ix1.b.l += 4; }
	void add_ix1_5()     { m_ix1.b.l += 5; }
	void add_ix1_6()     { m_ix1.b.l += 6; }
	void add_ix1_7()     { m_ix1.b.l += 7; }
	void add_ix1_8()     { m_ix1.b.l += 8; }
	void add_ix1_9()     { m_ix1.b.l += 9; }
	void add_ix1_a()     { m_ix1.b.l += 10; }
	void add_ix1_b()     { m_ix1.b.l += 11; }
	void add_ix1_c()     { m_ix1.b.l += 12; }
	void add_ix1_d()     { m_ix1.b.l += 13; }
	void add_ix1_e()     { m_ix1.b.l += 14; }
	void add_ix1_f()     { m_ix1.b.l += 15; }

	void add_ix2_0()     { }
	void add_ix2_1()     { m_ix2.b.l += 1; }
	void add_ix2_2()     { m_ix2.b.l += 2; }
	void add_ix2_3()     { m_ix2.b.l += 3; }
	void add_ix2_4()     { m_ix2.b.l += 4; }
	void add_ix2_5()     { m_ix2.b.l += 5; }
	void add_ix2_6()     { m_ix2.b.l += 6; }
	void add_ix2_7()     { m_ix2.b.l += 7; }
	void add_ix2_8()     { m_ix2.b.l += 8; }
	void add_ix2_9()     { m_ix2.b.l += 9; }
	void add_ix2_a()     { m_ix2.b.l += 10; }
	void add_ix2_b()     { m_ix2.b.l += 11; }
	void add_ix2_c()     { m_ix2.b.l += 12; }
	void add_ix2_d()     { m_ix2.b.l += 13; }
	void add_ix2_e()     { m_ix2.b.l += 14; }
	void add_ix2_f()     { m_ix2.b.l += 15; }

	void ld_base_0()     { m_regPtr = 0; }
	void ld_base_1()     { m_regPtr = 1; }
	void ld_base_2()     { m_regPtr = 2; }
	void ld_base_3()     { m_regPtr = 3; }
	void ld_base_4()     { m_regPtr = 4; }
	void ld_base_5()     { m_regPtr = 5; }
	void ld_base_6()     { m_regPtr = 6; }
	void ld_base_7()     { m_regPtr = 7; }

	void ld_bank_0()     { m_mb = 0; }
	void ld_bank_1()     { m_mb = 1; }
	void ld_bank_2()     { m_mb = 2; }
	void ld_bank_3()     { m_mb = 3; }

	void ld_ix0_n()  { m_ix0.b.l = M_RDMEM_OPCODE(); }
	void ld_ix1_n()  { m_ix1.b.l = M_RDMEM_OPCODE(); }
	void ld_ix2_n()  { m_ix2.b.l = M_RDMEM_OPCODE(); }
	void ld_lp0_n()  { m_lp0 = M_RDMEM_OPCODE(); }
	void ld_lp1_n()  { m_lp1 = M_RDMEM_OPCODE(); }
	void ld_lp2_n()  { m_lp2 = M_RDMEM_OPCODE(); }
	void ld_b_n()    { m_B = M_RDMEM_OPCODE(); }

	void djnz_lp0() { UINT8 i=M_RDMEM_OPCODE(); m_lp0--; if (m_lp0 != 0) M_JMP(i); }
	void djnz_lp1() { UINT8 i=M_RDMEM_OPCODE(); m_lp1--; if (m_lp1 != 0) M_JMP(i); }
	void djnz_lp2() { UINT8 i=M_RDMEM_OPCODE(); m_lp2--; if (m_lp2 != 0) M_JMP(i); }
	void jnz()  { UINT8 i=M_RDMEM_OPCODE(); if (!m_zf) M_JMP(i); }
	void jnc()  { UINT8 i=M_RDMEM_OPCODE(); if (!m_cf) M_JMP(i);}
	void jz()   { UINT8 i=M_RDMEM_OPCODE(); if ( m_zf) M_JMP(i); }
	void jc()   { UINT8 i=M_RDMEM_OPCODE(); if ( m_cf) M_JMP(i);}
	void jmp()  { M_JMP(M_RDMEM_OPCODE() ); }

	void stop();

	/* ALPHA 8301 : added instruction */
	void exg_a_ix0()  { UINT8 t=m_A; m_A = m_ix0.b.l; m_ix0.b.l = t; }
	void exg_a_ix1()  { UINT8 t=m_A; m_A = m_ix1.b.l; m_ix1.b.l = t; }
	void exg_a_ix2()  { UINT8 t=m_A; m_A = m_ix2.b.l; m_ix2.b.l = t; }
	void exg_a_lp0()  { UINT8 t=m_A; m_A = m_lp0; m_lp0 = t; }
	void exg_a_lp1()  { UINT8 t=m_A; m_A = m_lp1; m_lp1 = t; }
	void exg_a_lp2()  { UINT8 t=m_A; m_A = m_lp2; m_lp2 = t; }
	void exg_a_b()    { UINT8 t=m_A; m_A = m_B; m_B = t; }
	void exg_a_rb()   { UINT8 t=m_A; m_A = m_regPtr; m_regPtr = t; }

	void ld_ix0_a()    { m_ix0.b.l = m_A; }
	void ld_ix1_a()    { m_ix1.b.l = m_A; }
	void ld_ix2_a()    { m_ix2.b.l = m_A; }
	void ld_lp0_a()    { m_lp0 = m_A; }
	void ld_lp1_a()    { m_lp1 = m_A; }
	void ld_lp2_a()    { m_lp2 = m_A; }
	void ld_b_a()      { m_B = m_A; }
	void ld_rb_a()     { m_regPtr = m_A; }

	void exg_ix0_ix1()  { UINT8 t=m_ix1.b.l; m_ix1.b.l = m_ix0.b.l; m_ix0.b.l = t; }
	void exg_ix0_ix2()  { UINT8 t=m_ix2.b.l; m_ix2.b.l = m_ix0.b.l; m_ix0.b.l = t; }

	void op_d4() { m_A = M_RDMEM( ((m_RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE() ); }
	void op_d5() { M_WRMEM( ((m_RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(), m_A ); }
	void op_d6() { m_lp0 = M_RDMEM( ((m_RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE() ); }
	void op_d7() { M_WRMEM( ((m_RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(), m_lp0 ); }

	void ld_a_abs() { m_A = M_RDMEM( ((m_mb & 3) << 8) | M_RDMEM_OPCODE() ); }
	void ld_abs_a() { M_WRMEM( ((m_mb & 3) << 8) | M_RDMEM_OPCODE(), m_A ); }

	void ld_a_r() { m_A = m_RAM[(M_RDMEM_OPCODE()>>1)&0x3f]; }
	void ld_r_a() { m_RAM[(M_RDMEM_OPCODE()>>1)&0x3f] = m_A; }
	void op_rep_ld_ix2_b() { do { M_WRMEM(m_ix2.w.l, m_RAM[(m_B>>1)&0x3f]); m_ix2.b.l++; m_B+=2; m_lp0--; } while (m_lp0 != 0); }
	void op_rep_ld_b_ix0() { do { m_RAM[(m_B>>1)&0x3f] = M_RDMEM(m_ix0.w.l); m_ix0.b.l++; m_B+=2; m_lp0--; } while (m_lp0 != 0); }
	void ld_rxb_a() { m_RAM[(m_B>>1)&0x3f] = m_A; }
	void ld_a_rxb() { m_A = m_RAM[(m_B>>1)&0x3f]; }
	void cmp_a_rxb() { UINT8 i=m_RAM[(m_B>>1)&0x3f];  m_zf = (m_A==i); m_cf = (m_A>=i); }
	void xor_a_rxb() { M_XOR(m_RAM[(m_B>>1)&0x3f] ); }

	void add_a_cf() { if (m_cf) inc_a(); }
	void sub_a_cf() { if (m_cf) dec_a(); }
	void tst_a()     { m_zf = (m_A==0); }
	void clr_a()     { m_A = 0; m_zf = (m_A==0); }
	void cmp_a_n()  { UINT8 i=M_RDMEM_OPCODE();  m_zf = (m_A==i); m_cf = (m_A>=i); }
	void xor_a_n()  { M_XOR(M_RDMEM_OPCODE() ); }
	void call() { UINT8 i=M_RDMEM_OPCODE(); m_retptr.w.l = m_pc.w.l; M_JMP(i); };
	void ld_a_ix0_a() { m_A = M_RDMEM(m_ix0.w.l+m_A); }
	void ret() { m_mb = m_retptr.b.h; M_JMP( m_retptr.b.l ); };
	void save_zc() { m_savez = m_zf; m_savec = m_cf; };
	void rest_zc() { m_zf = m_savez; m_cf = m_savec; };

	typedef void ( alpha8201_cpu_device::*opcode_fun ) ();

	/* The opcode table now is a combination of cycle counts and function pointers */
	struct s_opcode {
		unsigned cycles;
		opcode_fun opcode_func;
	};

	static const s_opcode opcode_8201[256];
	static const s_opcode opcode_8301[256];

	address_space_config m_program_config;
	address_space_config m_io_config;

	UINT8 m_RAM[8*8];  /* internal GP register 8 * 8bank       */
	unsigned m_PREVPC;
	PAIR  m_retptr;   /* for 8301, return address of CALL       */
	PAIR  m_pc;       /* 2bit+8bit program counter              */
	UINT8 m_regPtr;   /* RB register base                       */
	UINT8 m_mb;       /* MB memory bank reg. latch after Branch */
	UINT8 m_cf;       /* C flag                                 */
	UINT8 m_zf;       /* Z flag                                 */
	UINT8 m_savec;    /* for 8301, save flags                   */
	UINT8 m_savez;    /* for 8301, save flags                   */
//
	PAIR m_ix0;       /* 8bit memory read index reg. */
	PAIR m_ix1;       /* 8bitmemory read index reg.  */
	PAIR m_ix2;       /* 8bitmemory write index reg. */
	UINT8 m_lp0;       /* 8bit loop reg.             */
	UINT8 m_lp1;       /* 8bit loop reg.             */
	UINT8 m_lp2;       /* 8bit loop reg.             */
	UINT8 m_A;         /* 8bit accumerator           */
	UINT8 m_B;         /* 8bit regiser               */
//
	UINT8 m_halt;     /* halt input line                        */

	address_space *m_program;
	direct_read_data *m_direct;
	int m_icount;
	int m_inst_cycles;

	const s_opcode *m_opmap;

	// Used for import/export only
	UINT8 m_sp;
	UINT8 m_R[8];
};


class alpha8301_cpu_device : public alpha8201_cpu_device
{
public:
	// construction/destruction
	alpha8301_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type ALPHA8201L;
extern const device_type ALPHA8301L;


#endif  /* __ALPH8201_H__ */
