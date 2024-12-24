// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_CPU_E132XS_E132XSDRC_OPS_HXX
#define MAME_CPU_E132XS_E132XSDRC_OPS_HXX

#pragma once

#include "e132xs.h"

constexpr uint32_t WRITE_ONLY_REGMASK = (1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER);

void hyperstone_device::generate_check_delay_pc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	/* if PC is used in a delay instruction, the delayed PC should be used */
	UML_TEST(block, mem(&m_core->delay_slot), 1);
	UML_MOVc(block, uml::COND_NZ, DRC_PC, mem(&m_core->delay_pc));
	UML_MOVc(block, uml::COND_NZ, mem(&m_core->delay_slot), 0);
	UML_SETc(block, uml::COND_NZ, mem(&m_core->delay_slot_taken));
}

void hyperstone_device::generate_decode_const(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t imm_1 = m_pr16(desc->pc + 2);

	if (imm_1 & 0x8000)
	{
		const uint16_t imm_2 = m_pr16(desc->pc + 4);

		uint32_t imm = imm_2;
		imm |= ((imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			imm |= 0xc0000000;

		UML_ADD(block, DRC_PC, DRC_PC, 4);
		UML_MOV(block, I1, imm);
	}
	else
	{
		uint32_t imm = imm_1 & 0x3fff;

		if (imm_1 & 0x4000)
			imm |= 0xffffc000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
		UML_MOV(block, I1, imm);
	}
}

void hyperstone_device::generate_decode_immediate_s(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	switch (op & 0xf)
	{
		case 0:
			UML_MOV(block, I1, 16);
			return;
		case 1:
		{
			uint32_t extra_u = (m_pr16(desc->pc + 2) << 16) | m_pr16(desc->pc + 4);
			UML_ADD(block, DRC_PC, DRC_PC, 4);
			UML_MOV(block, I1, extra_u);
			return;
		}
		case 2:
		{
			uint32_t extra_u = m_pr16(desc->pc + 2);
			UML_ADD(block, DRC_PC, DRC_PC, 2);
			UML_MOV(block, I1, extra_u);
			return;
		}
		case 3:
		{
			uint32_t extra_u = 0xffff0000 | m_pr16(desc->pc + 2);
			UML_ADD(block, DRC_PC, DRC_PC, 2);
			UML_MOV(block, I1, extra_u);
			return;
		}
		default:
			UML_MOV(block, I1, s_immediate_values[op & 0xf]);
			return;
	}
}

void hyperstone_device::generate_ignore_immediate_s(drcuml_block &block, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	static const uint32_t offsets[16] = { 0, 4, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const uint8_t nybble = op & 0x0f;

	UML_ADD(block, DRC_PC, DRC_PC, offsets[nybble]);
}

void hyperstone_device::generate_decode_pcrel(drcuml_block &block, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	int32_t offset;
	if (op & 0x80)
	{
		uint16_t next = m_pr16(desc->pc + 2);

		offset = (op & 0x7f) << 16;
		offset |= (next & 0xfffe);

		if (next & 1)
			offset |= 0xff800000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}
	else
	{
		offset = op & 0x7e;

		if (op & 1)
			offset |= 0xffffff80;
	}

	UML_MOV(block, I1, offset);
}

void hyperstone_device::generate_ignore_pcrel(drcuml_block &block, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];

	if (op & 0x80)
	{
		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}
}

void hyperstone_device::generate_set_global_register(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	// Expects register index in I4, value in I5, clobbers I6
	int extended;
	UML_CMP(block, I4, 16);
	UML_JMPc(block, uml::COND_AE, extended = compiler.m_labelnum++);

	int generic_store, set_sr, done;
	UML_CMP(block, I4, 1);
	UML_JMPc(block, uml::COND_A, generic_store = compiler.m_labelnum++);
	UML_JMPc(block, uml::COND_E, set_sr = compiler.m_labelnum++);
	UML_AND(block, DRC_PC, I5, ~1);
	UML_JMP(block, done = compiler.m_labelnum++);

	UML_LABEL(block, set_sr);
	UML_ROLINS(block, DRC_SR, I5, 0, 0x0000ffff);
	UML_AND(block, DRC_SR, DRC_SR, ~0x40);
	UML_TEST(block, mem(&m_core->intblock), ~0);
	UML_MOVc(block, uml::COND_Z, mem(&m_core->intblock), 1);
	UML_JMP(block, done);

	UML_LABEL(block, generic_store);
	UML_STORE(block, (void *)m_core->global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_JMP(block, done);

	int above_bcr;
	UML_LABEL(block, extended);
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I7);
	UML_MOV(block, I7, 0);
	UML_CMP(block, I4, 17);
	UML_JMPc(block, uml::COND_BE, generic_store);
	UML_CMP(block, I4, BCR_REGISTER);
	UML_JMPc(block, uml::COND_A, above_bcr = compiler.m_labelnum++);
	UML_JMPc(block, uml::COND_E, generic_store);

	// SP or UB
	UML_AND(block, I5, I5, ~3);
	UML_JMP(block, generic_store);

	int set_tpr, set_tcr, set_tr, set_fcr;
	UML_LABEL(block, above_bcr);
	UML_CMP(block, I4, TCR_REGISTER);
	UML_JMPc(block, uml::COND_B, set_tpr = compiler.m_labelnum++);
	UML_JMPc(block, uml::COND_E, set_tcr = compiler.m_labelnum++);
	// Above TCR
	UML_CMP(block, I4, WCR_REGISTER);
	UML_JMPc(block, uml::COND_B, set_tr = compiler.m_labelnum++);
	UML_JMPc(block, uml::COND_E, generic_store); // WCR
	// Above WCR
	UML_CMP(block, I4, FCR_REGISTER);
	UML_JMPc(block, uml::COND_B, done); // ISR - read only
	UML_JMPc(block, uml::COND_E, set_fcr = compiler.m_labelnum++);
	UML_CMP(block, I4, MCR_REGISTER);
	UML_JMPc(block, uml::COND_A, generic_store); // regs 28..31
	// Set MCR
	UML_ROLAND(block, I6, I5, 20, 0x7);
	UML_LOAD(block, I6, (void *)s_trap_entries, I6, SIZE_DWORD, SCALE_x4);
	UML_MOV(block, mem(&m_core->trap_entry), I6);
	UML_JMP(block, generic_store);

	int skip_compute_tr;
	UML_LABEL(block, set_tpr);
	UML_STORE(block, (void *)m_core->global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_TEST(block, I5, 0x80000000);
	UML_JMPc(block, uml::COND_NZ, skip_compute_tr = compiler.m_labelnum++);
	UML_CALLC(block, cfunc_compute_tr, this);
	UML_CALLC(block, cfunc_update_timer_prescale, this);
	UML_LABEL(block, skip_compute_tr);
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_JMP(block, done);

	UML_LABEL(block, set_tcr);
	UML_LOAD(block, I6, (void *)m_core->global_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_CMP(block, I6, I5);
	UML_JMPc(block, uml::COND_E, done);
	UML_STORE(block, (void *)m_core->global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_CMP(block, mem(&m_core->intblock), 1);
	UML_MOVc(block, uml::COND_L, mem(&m_core->intblock), 1);
	UML_JMP(block, done);

	UML_LABEL(block, set_tr);
	UML_STORE(block, (void *)m_core->global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_MOV(block, mem(&m_core->tr_base_value), I5);
	UML_CALLC(block, cfunc_total_cycles, this);
	UML_DMOV(block, mem(&m_core->tr_base_cycles), mem(&m_core->numcycles));
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_JMP(block, done);

	int skip_adjust_timer;
	UML_LABEL(block, set_fcr);
	UML_LOAD(block, I6, (void *)m_core->global_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_XOR(block, I6, I6, I5);
	UML_TEST(block, I6, 0x80000000);
	UML_JMPc(block, uml::COND_Z, skip_adjust_timer = compiler.m_labelnum++);
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_LABEL(block, skip_adjust_timer);
	UML_STORE(block, (void *)m_core->global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_CMP(block, mem(&m_core->intblock), 1);
	UML_MOVc(block, uml::COND_L, mem(&m_core->intblock), 1);
	// Fall through to done

	UML_LABEL(block, done);
}

template <hyperstone_device::trap_exception_or_int TYPE>
void hyperstone_device::generate_trap_exception_or_int(drcuml_block &block)
{
	UML_ADD(block, I7, I7, mem(&m_core->clock_cycles_2));

	UML_MOV(block, I4, DRC_SR);

	UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
	UML_ROLAND(block, I2, DRC_SR, 11, 0xf);
	UML_TEST(block, I2, 0xf);
	UML_MOVc(block, uml::COND_Z, I2, 16);
	UML_ADD(block, I3, I1, I2);

	if (TYPE != IS_TRAP)
		UML_ROLINS(block, DRC_SR,  2, 21, 0x01e00000);
	else
		UML_ROLINS(block, DRC_SR,  6, 21, 0x01e00000);
	UML_ROLINS(block, DRC_SR, I3, 25, 0xfe000000);

	UML_AND(block, I1, I3, 0x3f);
	UML_AND(block, I2, DRC_PC, ~1);
	UML_ROLINS(block, I2, DRC_SR, 32-S_SHIFT, 1);
	UML_STORE(block, (void *)m_core->local_regs, I1, I2, SIZE_DWORD, SCALE_x4);
	UML_ADD(block, I2, I1, 1);
	UML_AND(block, I3, I2, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I3, I4, SIZE_DWORD, SCALE_x4);

	UML_AND(block, DRC_SR, DRC_SR, ~(M_MASK | T_MASK));
	if (TYPE == IS_INT)
		UML_OR(block, DRC_SR, DRC_SR, (L_MASK | S_MASK | I_MASK));
	else
		UML_OR(block, DRC_SR, DRC_SR, (L_MASK | S_MASK));

	UML_MOV(block, DRC_PC, I0);
	generate_branch(block, DRC_PC, nullptr, true);
}

void hyperstone_device::generate_int(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t addr)
{
	printf("Unimplemented: generate_int (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}

void hyperstone_device::generate_exception(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t addr)
{
	printf("Unimplemented: generate_exception (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}

void hyperstone_device::generate_software(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_6));

	uint16_t op = desc->opptr.w[0];

	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f); // I3 = FP

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4); // I0 = sreg
	UML_ADD(block, I2, I3, srcf_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4); // I1 = sregf

	UML_ROLINS(block, DRC_SR, 1, 19, 0x00180000);

	uint32_t num = op >> 8;
	int mem3 = compiler.m_labelnum++;
	int have_code_addr = compiler.m_labelnum++;
	UML_MOV(block, I4, mem(&m_core->trap_entry));
	UML_CMP(block, I4, 0xffffff00);
	UML_JMPc(block, uml::COND_E, mem3);
	UML_OR(block, I5, I4, (0x10c | ((0xcf - num) << 4)));
	UML_JMP(block, have_code_addr);

	UML_LABEL(block, mem3);
	UML_SUB(block, I5, I4, 0x100);
	UML_OR(block, I5, I5, ((num & 0xf) << 4)); // I5 = addr

	UML_LABEL(block, have_code_addr);

	UML_ROLAND(block, I2, DRC_SR, 11, 0xf);
	UML_TEST(block, I2, 0xf);
	UML_MOVc(block, uml::COND_Z, I2, 16);
	UML_ADD(block, I4, I2, I3); // I4 = reg

	UML_AND(block, I2, mem(&SP), 0xffffff00);
	UML_ADD(block, I6, I2, 0x100); // I6 = (SP & ~0xff) + 0x100
	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_SHL(block, I2, I2, 2); // I2 = (((fp + DST_CODE) & 0x3f) << 2)
	UML_ADD(block, I6, I6, I2); // I6 = (SP & ~0xff) + 0x100 + (((fp + DST_CODE) & 0x3f) << 2)

	UML_AND(block, I2, I4, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I6, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 0) & 0x3f] = stack_of_dst;
	UML_ADD(block, I6, I2, 1);
	UML_AND(block, I2, I6, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 1) & 0x3f] = sreg;
	UML_ADD(block, I6, I2, 1);
	UML_AND(block, I2, I6, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 2) & 0x3f] = sregf;

	UML_AND(block, I0, DRC_PC, ~1);
	UML_ROLINS(block, I0, DRC_SR, 32-S_SHIFT, 1);
	UML_ADD(block, I6, I2, 1);
	UML_AND(block, I2, I6, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 3) & 0x3f] = (PC & ~1) | GET_S;

	UML_ADD(block, I6, I2, 1);
	UML_AND(block, I2, I6, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I2, DRC_SR, SIZE_DWORD, SCALE_x4); // m_core->local_regs[(reg + 4) & 0x3f] = oldSR;

	UML_MOV(block, DRC_PC, I5); // PC = addr

	UML_MOV(block, I0, DRC_SR);
	UML_ROLINS(block, I0, 0x00c08000, 0, 0x01e08000); // SET_FL(6), SR |= L_MASK
	UML_ROLINS(block, I0, I4, 25, 0xfe000000); // SET_FP(reg)
	UML_AND(block, DRC_SR, I0, ~(M_MASK | T_MASK));

	generate_branch(block, desc->targetpc, desc);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_chk(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	if (!DST_GLOBAL || !SRC_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
		{
			UML_TEST(block, I1, ~0);
			UML_EXHc(block, uml::COND_Z, *m_exception[EXCEPTION_RANGE_ERROR], 0);
		}
		else
		{
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
			UML_CMP(block, I1, I0);
			if (src_code == PC_REGISTER)
				UML_EXHc(block, uml::COND_AE, *m_exception[EXCEPTION_RANGE_ERROR], 0);
			else
				UML_EXHc(block, uml::COND_A, *m_exception[EXCEPTION_RANGE_ERROR], 0);
		}
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);

		UML_CMP(block, I1, I0);
		UML_EXHc(block, uml::COND_A, *m_exception[EXCEPTION_RANGE_ERROR], 0);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_movd(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	if (DST_GLOBAL && (dst_code == PC_REGISTER))
	{
		if (SRC_GLOBAL && src_code < 2)
		{
			printf("Denoted PC or SR in RET instruction. PC = %08X\n", desc->pc);
			return;
		}

		UML_AND(block, I1, DRC_SR, (S_MASK | L_MASK));
		if (SRC_GLOBAL)
		{
			UML_LOAD(block, I2, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
			UML_LOAD(block, I3, (void *)m_core->global_regs, srcf_code, SIZE_DWORD, SCALE_x4);
		}
		else
		{
			UML_ROLAND(block, I5, DRC_SR, 7, 0x7f);
			UML_ADD(block, I3, I5, src_code);
			UML_AND(block, I4, I3, 0x3f);
			UML_LOAD(block, I2, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);

			UML_ADD(block, I6, I5, srcf_code);
			UML_AND(block, I5, I6, 0x3f);
			UML_LOAD(block, I3, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
		}

		UML_AND(block, DRC_PC, I2, ~1);

		UML_AND(block, DRC_SR, I3, 0xffe3ffff);
		UML_ROLINS(block, DRC_SR, I2, S_SHIFT, S_MASK);

		UML_TEST(block, mem(&m_core->intblock), ~0);
		UML_MOVc(block, uml::COND_Z, mem(&m_core->intblock), 1);

		int no_exception;
		UML_AND(block, I2, DRC_SR, (S_MASK | L_MASK));
		UML_AND(block, I3, I1, I2);
		UML_TEST(block, I3, S_MASK);
		UML_JMPc(block, uml::COND_NZ, no_exception = compiler.m_labelnum++); // If S is set and unchanged, there won't be an exception.

		UML_XOR(block, I3, I1, I2);
		UML_AND(block, I4, I3, I2);
		UML_TEST(block, I4, S_MASK);
		UML_EXHc(block, uml::COND_NZ, *m_exception[EXCEPTION_PRIVILEGE_ERROR], 0); // If S is newly set, it's a privilege error.

		UML_TEST(block, I3, L_MASK);
		UML_JMPc(block, uml::COND_Z, no_exception); // If L is unchanged, there won't be an exception.
		UML_TEST(block, I1, L_MASK);
		UML_JMPc(block, uml::COND_NZ, no_exception); // If L was previously set, there won't be an exception.
		UML_TEST(block, I2, S_MASK);
		UML_EXHc(block, uml::COND_Z, *m_exception[EXCEPTION_PRIVILEGE_ERROR], 0); // If L is newly set and we are not in Supervisor mode, it's a privilege error.

		int diff_in_range, done_ret;
		UML_LABEL(block, no_exception);
		UML_MOV(block, I0, mem(&SP));
		UML_ROLAND(block, I1, I0, 30, 0x7f);
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);
		UML_SUB(block, I3, I2, I1);
		UML_CMP(block, I3, -64);
		UML_JMPc(block, uml::COND_L, done_ret = compiler.m_labelnum++);
		UML_CMP(block, I3, 64);
		UML_JMPc(block, uml::COND_L, diff_in_range = compiler.m_labelnum++);
		UML_OR(block, I3, I3, 0x80);
		UML_SEXT(block, I3, I3, SIZE_BYTE);
		UML_LABEL(block, diff_in_range);

		int pop_next;
		UML_LABEL(block, pop_next = compiler.m_labelnum++);
		UML_CMP(block, I3, 0);
		UML_JMPc(block, uml::COND_GE, done_ret);
		UML_SUB(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);
		UML_ROLAND(block, I2, I0, 30, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I3, I3, 1);
		UML_TEST(block, I3, ~0);
		UML_JMP(block, pop_next);

		UML_LABEL(block, done_ret);
		UML_MOV(block, mem(&SP), I0);
		generate_branch(block, desc->targetpc, desc);
		return;
	}
	else if (SRC_GLOBAL && (src_code == SR_REGISTER)) // Rd doesn't denote PC and Rs denotes SR
	{
		UML_OR(block, DRC_SR, DRC_SR, Z_MASK);
		UML_AND(block, DRC_SR, DRC_SR, ~N_MASK);
		if (DST_GLOBAL)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, 0);
			generate_set_global_register(block, compiler, desc);
			UML_MOV(block, I4, dstf_code);
			UML_MOV(block, I5, 0);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER || dstf_code == PC_REGISTER)
				generate_branch(block, desc->targetpc, desc);
		}
		else
		{
			UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
			UML_ADD(block, I0, I0, dst_code);
			UML_AND(block, I0, I0, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I0, 0, SIZE_DWORD, SCALE_x4);
			UML_ADD(block, I0, I0, 1);
			UML_AND(block, I0, I0, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I0, 0, SIZE_DWORD, SCALE_x4);
		}
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		if (!SRC_GLOBAL || !DST_GLOBAL)
		{
			UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		}

		if (SRC_GLOBAL)
		{
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
			UML_LOAD(block, I1, (void *)m_core->global_regs, srcf_code, SIZE_DWORD, SCALE_x4);
		}
		else
		{
			UML_ADD(block, I0, I3, src_code);
			UML_AND(block, I0, I0, 0x3f);
			UML_LOAD(block, I0, (void *)m_core->local_regs, I0, SIZE_DWORD, SCALE_x4);
			UML_ADD(block, I1, I3, srcf_code);
			UML_AND(block, I1, I1, 0x3f);
			UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
		}

		UML_AND(block, DRC_SR, DRC_SR, ~(Z_MASK | N_MASK));

		UML_OR(block, I2, I0, I1);
		UML_TEST(block, I2, ~0);
		UML_SETc(block, uml::COND_Z, I2);
		UML_ROLINS(block, DRC_SR, I2, Z_SHIFT, Z_MASK);

		UML_TEST(block, I0, 0x80000000);
		UML_SETc(block, uml::COND_NZ, I2);
		UML_ROLINS(block, DRC_SR, I2, N_SHIFT, N_MASK);

		if (DST_GLOBAL)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I0);
			generate_set_global_register(block, compiler, desc);
			UML_MOV(block, I4, dstf_code);
			UML_MOV(block, I5, I1);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER || dstf_code == PC_REGISTER)
				generate_branch(block, desc->targetpc, desc);
		}
		else
		{
			UML_ADD(block, I2, I3, dst_code);
			UML_AND(block, I2, I2, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4);
			UML_ADD(block, I2, I3, dstf_code);
			UML_AND(block, I2, I2, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
		}
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL, hyperstone_device::sign_mode SIGNED>
void hyperstone_device::generate_divsu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_36));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;
	const uint32_t src_code = op & 0xf;

	if ((SRC_GLOBAL == DST_GLOBAL && (src_code == dst_code || src_code == dstf_code)) || (SRC_GLOBAL && src_code < 2))
	{
		printf("Denoted the same register code or PC/SR as source in generate_divsu. PC = %08X\n", desc->pc);
		return;
	}

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
#endif

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
		UML_LOAD(block, I2, (void *)m_core->global_regs, dstf_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I4, I3, dstf_code);
		UML_AND(block, I6, I4, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I2, I2, 0x00000000ffffffffULL);
#endif

	UML_DSHL(block, I1, I1, 32);
	UML_DOR(block, I1, I1, I2);

	int no_result = compiler.m_labelnum++;
	int done = compiler.m_labelnum++;
	UML_TEST(block, I0, ~0);
	UML_JMPc(block, uml::COND_Z, no_result);
	if (SIGNED)
	{
		UML_DTEST(block, I1, 0x8000000000000000LL);
		UML_JMPc(block, uml::COND_NZ, no_result);
		UML_DSEXT(block, I0, I0, SIZE_DWORD);
	}

	if (SIGNED)
		UML_DDIVS(block, I2, I4, I1, I0);
	else
		UML_DDIVU(block, I2, I4, I1, I0);

	UML_AND(block, I3, DRC_SR, ~(V_MASK | Z_MASK | N_MASK));
	UML_TEST(block, I2, ~0);
	UML_MOVc(block, uml::COND_Z, I0, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I0, 0);
	UML_ROLINS(block, I0, I2, 3, N_MASK);
	UML_OR(block, DRC_SR, I3, I0);

	if (DST_GLOBAL)
	{
		UML_STORE(block, (void *)m_core->global_regs, dst_code, I4, SIZE_DWORD, SCALE_x4);
		UML_STORE(block, (void *)m_core->global_regs, dstf_code, I2, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
		UML_STORE(block, (void *)m_core->local_regs, I6, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_JMP(block, done);

	UML_LABEL(block, no_result);
	UML_OR(block, DRC_SR, DRC_SR, V_MASK);
	UML_EXH(block, *m_exception[EXCEPTION_RANGE_ERROR], 0);

	UML_LABEL(block, done);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_xm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	const uint32_t next = m_pr16(desc->pc + 2);
	const uint8_t sub_type = (next & 0x7000) >> 12;

	uint32_t extra_u = next & 0xfff;
	if (next & 0x8000)
	{
		extra_u = ((extra_u & 0xfff) << 16) | m_pr16(desc->pc + 4);
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else
	{
		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}

	UML_MOV(block, I1, extra_u);

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_AND(block, I0, DRC_SR, C_MASK);
		else
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	if ((SRC_GLOBAL && (src_code == SR_REGISTER)) || (DST_GLOBAL && (dst_code < 2)))
	{
		return;
	}

	if (sub_type < 4)
	{
		UML_CMP(block, I0, extra_u);
		int skip, done;
		if (SRC_GLOBAL && (src_code == PC_REGISTER))
		{
			UML_JMPc(block, uml::COND_B, skip = compiler.m_labelnum++);
			UML_EXH(block, *m_exception[EXCEPTION_RANGE_ERROR], 0);
			UML_JMP(block, done = compiler.m_labelnum++);
		}
		else
		{
			UML_JMPc(block, uml::COND_BE, skip = compiler.m_labelnum++);
			UML_EXH(block, *m_exception[EXCEPTION_RANGE_ERROR], 0);
			UML_JMP(block, done = compiler.m_labelnum++);
		}

		UML_LABEL(block, skip);
		UML_SHL(block, I5, I0, sub_type);

		UML_LABEL(block, done);
	}
	else
	{
		UML_SHL(block, I5, I0, sub_type - 4);
	}

	if (DST_GLOBAL)
	{
		UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I6, I3, dst_code);
		UML_AND(block, I4, I6, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_mask(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_decode_const(block, compiler, desc);
	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I1, I1, I2);

	int skip_mask;
	UML_AND(block, DRC_SR, DRC_SR, ~Z_MASK);
	UML_TEST(block, I1, ~0);
	UML_JMPc(block, uml::COND_NZ, skip_mask = compiler.m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, Z_MASK);
	UML_LABEL(block, skip_mask);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I1);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER)
				generate_branch(block, desc->targetpc, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I1, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I0, I3, dst_code);
		UML_AND(block, I0, I0, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I0, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sum(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_decode_const(block, compiler, desc);
	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	}

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
	UML_DAND(block, I2, I2, 0x00000000ffffffffULL);
#endif

	UML_DADD(block, I5, I1, I2);

	UML_AND(block, DRC_SR, DRC_SR, ~(C_MASK | V_MASK | Z_MASK | N_MASK));
	UML_DTEST(block, I5, 0x100000000ULL);
	UML_SETc(block, uml::COND_NZ, I6);
	UML_ROLINS(block, DRC_SR, I6, C_SHIFT, C_MASK);

	UML_XOR(block, I6, I5, I1);
	UML_XOR(block, I1, I5, I2);
	UML_AND(block, I1, I1, I6);
	UML_AND(block, I1, I1, 0x80000000);
	UML_ROLINS(block, DRC_SR, I1, 4, V_MASK);

	UML_TEST(block, I5, ~0);
	UML_SETc(block, uml::COND_Z, I6);
	UML_ROLINS(block, DRC_SR, I6, Z_SHIFT, Z_MASK);

	UML_ROLINS(block, DRC_SR, I5, 3, N_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER)
				generate_branch(block, desc->targetpc, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I0, I3, dst_code);
		UML_AND(block, I0, I0, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I0, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sums(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	printf("Unimplemented: generate_sums (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_cmp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_AND(block, I0, DRC_SR, C_MASK);
		else
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I1, I2, src_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I2, dst_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif

	UML_DSUB(block, I2, I1, I0); // tmp

	UML_XOR(block, I2, I2, I1);
	UML_XOR(block, I3, I1, I0);
	UML_AND(block, I3, I3, I2);
	UML_ROLINS(block, I3, I3, 4, V_MASK);

	UML_CMP(block, I1, I0);
	UML_SETc(block, uml::COND_B, I2);
	UML_ROLINS(block, I3, I2, C_SHIFT, C_MASK);

	UML_CMP(block, I1, I0);
	UML_SETc(block, uml::COND_E, I2);
	UML_ROLINS(block, I3, I2, Z_SHIFT, Z_MASK);

	UML_CMP(block, I1, I0);
	UML_SETc(block, uml::COND_L, I2);
	UML_ROLINS(block, I3, I2, N_SHIFT, N_MASK);

	UML_ROLINS(block, DRC_SR, I3, 0, (V_MASK | N_MASK | Z_MASK | C_MASK));
}

void hyperstone_device::generate_get_global_register(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;

	int regular_load = compiler.m_labelnum++;
	int done = compiler.m_labelnum++;
	UML_TEST(block, DRC_SR, H_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 16 + src_code);
	UML_MOVc(block, uml::COND_Z, I1, src_code);
	UML_CMP(block, I1, TR_REGISTER);
	UML_JMPc(block, uml::COND_NE, regular_load);

	UML_SHR(block, I2, mem(&m_core->tr_clocks_per_tick), 1);
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I7);
	UML_MOV(block, I7, 0);
	UML_CMP(block, mem(&m_core->icount), I2);
	UML_MOVc(block, uml::COND_BE, I2, 0);
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I2);
	UML_CALLC(block, cfunc_compute_tr, this);
	UML_MOV(block, I5, mem(&m_core->tr_result));
	UML_JMP(block, done);

	UML_LABEL(block, regular_load);
	UML_LOAD(block, I5, (void *)m_core->global_regs, I1, SIZE_DWORD, SCALE_x4);
	UML_SHL(block, I2, 1, I1);
	UML_TEST(block, I2, WRITE_ONLY_REGMASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);

	UML_LABEL(block, done);
}

template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_mov(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		int no_exception;
		UML_TEST(block, DRC_SR, H_MASK);
		UML_JMPc(block, uml::COND_Z, no_exception = compiler.m_labelnum++);
		UML_TEST(block, DRC_SR, S_MASK);
		UML_JMPc(block, uml::COND_NZ, no_exception);
		UML_EXH(block, *m_exception[EXCEPTION_PRIVILEGE_ERROR], 0);
		UML_LABEL(block, no_exception);
	}

	if (SRC_GLOBAL)
	{
		generate_get_global_register(block, compiler, desc);
		if (!DST_GLOBAL)
			UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
	}
	else
	{
		UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I1, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I5, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~(Z_MASK | N_MASK));
	UML_TEST(block, I5, ~0);
	UML_SETc(block, uml::COND_Z, I2);
	UML_ROLINS(block, DRC_SR, I2, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, DRC_SR, I5, 3, N_MASK);

	int done = compiler.m_labelnum++;
	if (DST_GLOBAL)
	{
		UML_TEST(block, DRC_SR, H_MASK);
		UML_MOVc(block, uml::COND_NZ, I4, 16 + dst_code);
		UML_MOVc(block, uml::COND_Z, I4, dst_code);
		UML_AND(block, DRC_SR, DRC_SR, ~H_MASK);
		UML_MOV(block, I3, I4);
		generate_set_global_register(block, compiler, desc);
		UML_CMP(block, I3, 0);
		UML_JMPc(block, uml::COND_NE, done);
		generate_branch(block, desc->targetpc, desc);
	}
	else
	{
		UML_AND(block, DRC_SR, DRC_SR, ~H_MASK);
		UML_ADD(block, I2, I1, dst_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I2, I5, SIZE_DWORD, SCALE_x4);
	}

	UML_LABEL(block, done);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_add(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_AND(block, I0, DRC_SR, 1);
		else
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I3, I3, dst_code);
		UML_AND(block, I3, I3, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
	}

	UML_ADD(block, I2, I0, I1);

	UML_SETc(block, uml::COND_C, I6);

	UML_XOR(block, I4, I0, I2);
	UML_XOR(block, I5, I1, I2);
	UML_AND(block, I4, I4, I5);
	UML_ROLINS(block, I6, I4, 4, V_MASK);

	UML_TEST(block, I2, ~0);
	UML_SETc(block, uml::COND_Z, I4);
	UML_ROLINS(block, I6, I4, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, I6, I2, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I6, 0, (V_MASK | N_MASK | Z_MASK | C_MASK));

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I2);
			generate_set_global_register(block, compiler, desc);

			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I2, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I3, I2, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_adds(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	printf("Unimplemented: generate_adds (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_cmpb(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I1, I2, src_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I2, dst_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_TEST(block, I1, I0);
	UML_SETc(block, uml::COND_Z, I0);
	UML_ROLINS(block, DRC_SR, I0, Z_SHIFT, Z_MASK);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_subc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_AND(block, I0, DRC_SR, C_MASK);
#ifndef PTR64
	UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
#endif

	if (SRC_GLOBAL)
	{
		if (src_code != SR_REGISTER)
		{
			UML_LOAD(block, I2, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
#ifndef PTR64
			UML_DAND(block, I2, I2, 0x00000000ffffffffULL);
#endif
			UML_DADD(block, I0, I2, I0);
		}
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
#ifndef PTR64
		UML_DAND(block, I2, I2, 0x00000000ffffffffULL);
#endif
		UML_DADD(block, I0, I2, I0);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif

	UML_AND(block, I6, DRC_SR, Z_MASK);
	UML_AND(block, I5, DRC_SR, ~(C_MASK | V_MASK | Z_MASK | N_MASK));

	UML_DSUB(block, I2, I1, I0);
	UML_DROLINS(block, I5, I2, 32, C_MASK);
	UML_XOR(block, I2, I2, I1); // tmp ^ dreg
	UML_XOR(block, I3, I1, I0); // dreg ^ sreg_c
	UML_AND(block, I3, I2, I3); // (tmp ^ dreg) ^ (dreg ^ sreg_c)
	UML_ROLINS(block, I5, I3, 4, V_MASK);

	UML_SUB(block, I1, I1, I0);
	UML_TEST(block, I1, ~0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_AND(block, I2, I2, I6); // old_z && dreg == 0
	UML_OR(block, I5, I5, I2);
	UML_ROLINS(block, I5, I1, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I5, 0, (C_MASK | V_MASK | Z_MASK | N_MASK));

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I1);
			generate_set_global_register(block, compiler, desc);

			if (dst_code == PC_REGISTER)
				generate_branch(block, desc->targetpc, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I1, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sub(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_AND(block, I0, DRC_SR, 1);
		else
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif

	UML_DSUB(block, I2, I1, I0);

	UML_AND(block, DRC_SR, DRC_SR, ~(C_MASK | V_MASK | Z_MASK | N_MASK));

	UML_DTEST(block, I2, 0x100000000ULL);
	UML_SETc(block, uml::COND_NZ, I4);
	UML_ROLINS(block, DRC_SR, I4, 0, C_MASK);

	UML_XOR(block, I4, I1, I2);
	UML_XOR(block, I5, I0, I1);
	UML_AND(block, I6, I4, I5);
	UML_ROLINS(block, DRC_SR, I6, 4, V_MASK);

	UML_SUB(block, I2, I1, I0);

	UML_TEST(block, I2, ~0);
	UML_SETc(block, uml::COND_Z, I4);
	UML_ROLINS(block, DRC_SR, I4, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, DRC_SR, I2, 3, N_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I2);
			generate_set_global_register(block, compiler, desc);

			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I2, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I4, I3, dst_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I2, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_subs(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_AND(block, I0, DRC_SR, 1);
		else
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_DSEXT(block, I0, I0, SIZE_DWORD);
	UML_DSEXT(block, I1, I1, SIZE_DWORD);
	UML_DSUB(block, I2, I1, I0);

	UML_AND(block, DRC_SR, DRC_SR, ~(V_MASK | Z_MASK | N_MASK));

	UML_XOR(block, I4, I1, I2);
	UML_XOR(block, I5, I0, I1);
	UML_AND(block, I4, I4, I5);
	UML_ROLINS(block, DRC_SR, I4, 4, V_MASK);

	UML_TEST(block, I2, ~0);
	UML_SETc(block, uml::COND_Z, I4);
	UML_ROLINS(block, DRC_SR, I4, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, DRC_SR, I2, 3, N_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I2);
			generate_set_global_register(block, compiler, desc);

			if (dst_code == PC_REGISTER)
			{
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I2, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I4, I3, dst_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I2, SIZE_DWORD, SCALE_x4);
	}

	int no_exception = compiler.m_labelnum++;
	UML_TEST(block, DRC_SR, V_MASK);
	UML_JMPc(block, uml::COND_Z, no_exception);
	UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT), 0, ILC_MASK);
	generate_trap_exception_or_int<IS_EXCEPTION>(block);
	UML_LABEL(block, no_exception);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_addc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
		{
			UML_AND(block, I0, DRC_SR, 1);
		}
		else
		{
			UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
			UML_AND(block, I1, DRC_SR, 1);
#ifndef PTR64
			UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
			UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif
			UML_DADD(block, I0, I0, I1);
		}
	}
	else
	{
		UML_ADD(block, I1, I3, src_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
		UML_AND(block, I1, DRC_SR, 1);
#ifndef PTR64
		UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
		UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif
		UML_DADD(block, I0, I0, I1);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I3, I3, dst_code);
		UML_AND(block, I3, I3, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif

	UML_DADD(block, I2, I0, I1);

	UML_XOR(block, I4, I0, I2);
	UML_XOR(block, I5, I1, I2);
	UML_AND(block, I4, I4, I5);
	UML_AND(block, I4, I4, I2);
	UML_ROLAND(block, I4, I4, 4, V_MASK);
	UML_DROLINS(block, I4, I2, 32, C_MASK);

	UML_ADD(block, I0, I0, I1);
	UML_SHR(block, I1, DRC_SR, Z_SHIFT);
	UML_AND(block, I1, I1, 1);
	UML_TEST(block, I0, ~0);
	UML_SETc(block, uml::COND_Z, I2);
	UML_AND(block, I1, I1, I2);
	UML_ROLINS(block, I4, I1, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, I4, I0, 3, N_MASK);

	UML_ROLINS(block, DRC_SR, I4, 0, (C_MASK | V_MASK | Z_MASK | N_MASK));

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I0);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I0, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_neg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

#ifndef PTR64
	UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
#endif

	UML_DSUB(block, I4, 0, I0);
	UML_SUB(block, I2, 0, I0);

	UML_TEST(block, I2, ~0);
	UML_MOVc(block, uml::COND_Z, I6, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I6, 0);

	UML_AND(block, I5, DRC_SR, ~(C_MASK | V_MASK | Z_MASK | N_MASK));
	UML_DROLINS(block, I6, I4, 32, C_MASK);
	UML_AND(block, I1, I4, I0);
	UML_ROLINS(block, I6, I1, 4, V_MASK);
	UML_ROLINS(block, I6, I2, 3, N_MASK);
	UML_OR(block, DRC_SR, I5, I6);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I2);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I2, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I1, I3, dst_code);
		UML_AND(block, I4, I1, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I2, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_negs(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	printf("Unimplemented: generate_negs (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_and(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I5, I1, I0);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_ROLINS(block, DRC_SR, I1, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_andn(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_XOR(block, I2, I0, ~0);
	UML_AND(block, I5, I1, I2);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_ROLINS(block, DRC_SR, I1, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_or(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_OR(block, I5, I1, I0);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_ROLINS(block, DRC_SR, I1, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_xor(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_XOR(block, I5, I1, I0);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_ROLINS(block, DRC_SR, I1, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_not(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	UML_XOR(block, I5, I0, ~0);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_ROLINS(block, DRC_SR, I1, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_cmpi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_MOV(block, I1, op & 0xf);
	}

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~(V_MASK | Z_MASK | N_MASK | C_MASK));

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
	UML_DAND(block, I2, I2, 0x00000000ffffffffULL);
#endif

	UML_DSUB(block, I0, I2, I1);

	int no_v;
	UML_XOR(block, I0, I0, I2);
	UML_XOR(block, I3, I1, I2);
	UML_AND(block, I0, I0, I3);
	UML_TEST(block, I0, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_v = compiler.m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, V_MASK);
	UML_LABEL(block, no_v);

	int no_n;
	UML_MOV(block, I3, 0);
	UML_CMP(block, I2, I1);
	UML_MOVc(block, uml::COND_E, I3, Z_MASK);
	UML_MOVc(block, uml::COND_B, I3, C_MASK);
	UML_JMPc(block, uml::COND_GE, no_n = compiler.m_labelnum++);
	UML_OR(block, I3, I3, N_MASK);
	UML_LABEL(block, no_n);

	UML_OR(block, DRC_SR, DRC_SR, I3);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_movi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (IMM_LONG)
		generate_decode_immediate_s(block, compiler, desc);
	else
		UML_MOV(block, I1, src_code);

	generate_check_delay_pc(block, compiler, desc);

	int done, no_exception;
	if (DST_GLOBAL)
	{
		UML_TEST(block, DRC_SR, H_MASK);
		UML_JMPc(block, uml::COND_Z, no_exception = compiler.m_labelnum++);
		UML_TEST(block, DRC_SR, S_MASK);
		UML_JMPc(block, uml::COND_NZ, no_exception);
		UML_EXH(block, *m_exception[EXCEPTION_PRIVILEGE_ERROR], 0);
		UML_JMP(block, done = compiler.m_labelnum++);
		UML_LABEL(block, no_exception);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~(Z_MASK | N_MASK));

	int no_z;
	UML_TEST(block, I1, ~0);
	UML_JMPc(block, uml::COND_NZ, no_z = compiler.m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, Z_MASK);
	UML_LABEL(block, no_z);

	int no_n;
	UML_TEST(block, I1, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_n = compiler.m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, N_MASK);
	UML_LABEL(block, no_n);

#if MISSIONCRAFT_FLAGS
	UML_AND(block, DRC_SR, DRC_SR, ~V_MASK);
#endif

	if (DST_GLOBAL)
	{
		UML_TEST(block, DRC_SR, H_MASK);
		UML_MOVc(block, uml::COND_NZ, I4, dst_code + 16);
		UML_MOVc(block, uml::COND_Z, I4, dst_code);
		UML_AND(block, DRC_SR, DRC_SR, ~H_MASK);
		UML_MOV(block, I5, I1);
		generate_set_global_register(block, compiler, desc);

		UML_TEST(block, op, 0xf0);
		UML_JMPc(block, uml::COND_NZ, done);
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		generate_branch(block, desc->targetpc, desc);

		UML_LABEL(block, done);
	}
	else
	{
		UML_AND(block, DRC_SR, DRC_SR, ~H_MASK);
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);
		UML_ADD(block, I0, I2, dst_code);
		UML_AND(block, I0, I0, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I0, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_addi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_MOV(block, I1, src_code);
	}

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I2, dst_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	if (!(op & 0x10f))
	{
		UML_ROLAND(block, I3, DRC_SR, 32-Z_SHIFT, 1);
		UML_XOR(block, I3, I3, 1);
		UML_AND(block, I4, I0, 1);
		UML_OR(block, I3, I3, I4);
		UML_AND(block, I1, DRC_SR, I3);
	}

#ifndef PTR64
	UML_DAND(block, I0, I0, 0x00000000ffffffffULL);
	UML_DAND(block, I1, I1, 0x00000000ffffffffULL);
#endif

	UML_DADD(block, I3, I0, I1);

	UML_DROLAND(block, I6, I3, 32, C_MASK);

	UML_XOR(block, I4, I0, I3);
	UML_XOR(block, I5, I1, I3);
	UML_AND(block, I4, I4, I5);
	UML_ROLINS(block, I6, I4, 4, V_MASK);

	UML_TEST(block, I3, ~0);
	UML_SETc(block, uml::COND_Z, I4);
	UML_ROLINS(block, I6, I4, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, I6, I3, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I6, 0, (V_MASK | N_MASK | Z_MASK | C_MASK));

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I3);
			generate_set_global_register(block, compiler, desc);

			if (dst_code == PC_REGISTER)
			{
				UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I3, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I2, I3, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_addsi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	printf("Unimplemented: generate_addsi (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_cmpbi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (!IMM_LONG)
		generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I4, I3, dst_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	const uint32_t n = ((op & 0x100) >> 4) | (op & 0x0f);
	if (n)
	{
		if (n == 31)
		{
			if (IMM_LONG)
			{
				generate_ignore_immediate_s(block, desc);
				generate_check_delay_pc(block, compiler, desc);
			}
			UML_MOV(block, I1, 0x7fffffff);
		}
		else
		{
			if (IMM_LONG)
			{
				generate_decode_immediate_s(block, compiler, desc);
				generate_check_delay_pc(block, compiler, desc);
			}
			else
			{
				UML_MOV(block, I1, op & 0xf);
			}
		}

		UML_AND(block, DRC_SR, DRC_SR, ~Z_MASK);
		UML_TEST(block, I2, I1);
		UML_SETc(block, uml::COND_Z, I3);
		UML_ROLINS(block, DRC_SR, I3, Z_SHIFT, Z_MASK);
	}
	else
	{
		if (IMM_LONG)
		{
			generate_ignore_immediate_s(block, desc);
			generate_check_delay_pc(block, compiler, desc);
		}

		int or_mask, done;
		UML_TEST(block, I2, 0xff000000);
		UML_JMPc(block, uml::COND_Z, or_mask = compiler.m_labelnum++);
		UML_TEST(block, I2, 0x00ff0000);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_TEST(block, I2, 0x0000ff00);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_TEST(block, I2, 0x000000ff);
		UML_JMPc(block, uml::COND_Z, or_mask);
		UML_AND(block, DRC_SR, DRC_SR, ~Z_MASK);
		UML_JMP(block, done = compiler.m_labelnum++);

		UML_LABEL(block, or_mask);
		UML_OR(block, DRC_SR, DRC_SR, Z_MASK);

		UML_LABEL(block, done);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_andni(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (DRC_N_OP_MASK == 0x10f)
		UML_MOV(block, I1, 0x7fffffff);
	else if (IMM_LONG)
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	else
		UML_MOV(block, I1, op & 0xf);

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_XOR(block, I1, I1, ~0);
	UML_AND(block, I5, I2, I1);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_ROLINS(block, DRC_SR, I2, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER)
			{
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I4, I3, dst_code);
		UML_AND(block, I4, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_ori(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_MOV(block, I1, op & 0xf);
	}

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_OR(block, I5, I2, I1);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_ROLINS(block, DRC_SR, I2, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER)
			{
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I4, I3, dst_code);
		UML_AND(block, I4, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_xori(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_MOV(block, I1, op & 0xf);
	}

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	UML_XOR(block, I5, I2, I1);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_ROLINS(block, DRC_SR, I2, 0, Z_MASK);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
			if (dst_code == PC_REGISTER)
			{
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I4, I3, dst_code);
		UML_AND(block, I4, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::shift_type HI_N>
void hyperstone_device::generate_shrdi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4); // I0 = sreg

	UML_ADD(block, I2, I3, dst_code + 1);
	UML_AND(block, I6, I2, 0x3f);
	UML_LOAD(block, I2, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4); // I1 = sregf

#ifndef PTR64
	UML_DAND(block, I2, I2, 0x00000000ffffffff);
#endif

	UML_DSHL(block, I0, I0, 32);
	UML_DOR(block, I2, I2, I0);

	UML_AND(block, I4, DRC_SR, ~(C_MASK | Z_MASK | N_MASK));

	const uint32_t n = HI_N ? (0x10 | (op & 0xf)) : (op & 0xf);
	if (HI_N || n)
	{
		int no_carry = compiler.m_labelnum++;
		UML_DTEST(block, I2, (1 << (n - 1)));
		UML_JMPc(block, uml::COND_Z, no_carry);
		UML_OR(block, I4, I4, 1);
		UML_LABEL(block, no_carry);

		UML_DSHR(block, I2, I2, n);
	}

	UML_DTEST(block, I2, ~0ULL);
	UML_MOVc(block, uml::COND_Z, I5, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);
	UML_DROLINS(block, I5, I2, 3, N_MASK);
	UML_OR(block, DRC_SR, I4, I5);

	UML_STORE(block, (void *)m_core->local_regs, I6, I2, SIZE_DWORD, SCALE_x4);
	UML_DSHR(block, I0, I2, 32);
	UML_STORE(block, (void *)m_core->local_regs, I1, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shrd(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	generate_check_delay_pc(block, compiler, desc);

	if (src_code == dst_code || src_code == dstf_code)
	{
		return;
	}

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I2, I3, dstf_code);
	UML_AND(block, I5, I2, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffff);
#endif

	UML_DSHL(block, I2, I0, 32);
	UML_DOR(block, I0, I1, I2);

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I6, I2, 0x3f);
	UML_LOAD(block, I2, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I6, I2, 0x1f);

	int no_shift = compiler.m_labelnum++;
	UML_TEST(block, I6, ~0);
	UML_JMPc(block, uml::COND_Z, no_shift);

	UML_SUB(block, I2, I6, 1);
	UML_DSHR(block, I3, I0, I2);
	UML_AND(block, I2, I3, 1);
	UML_DSHR(block, I0, I0, I6);

	UML_LABEL(block, no_shift);
	UML_DCMP(block, I0, 0ULL);
	UML_MOVc(block, uml::COND_E, I3, Z_MASK);
	UML_MOVc(block, uml::COND_NE, I3, 0);
	UML_OR(block, I1, I2, I3);
	UML_DROLINS(block, I1, I0, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I1, 0, (C_MASK | Z_MASK | N_MASK));

	UML_STORE(block, (void *)m_core->local_regs, I5, I0, SIZE_DWORD, SCALE_x4);
	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4); // I0 = dreg

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_LOAD(block, I5, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I1, I5, 0x1f); // I1 = sreg & 0x1f

	UML_AND(block, I6, DRC_SR, ~(C_MASK | Z_MASK | N_MASK));

	int no_shift = compiler.m_labelnum++;
	int no_carry = compiler.m_labelnum++;
	UML_CMP(block, I1, 0);
	UML_JMPc(block, uml::COND_E, no_shift);
	UML_SUB(block, I2, I1, 1);
	UML_SHL(block, I2, 1, I2);
	UML_TEST(block, I0, I2);
	UML_JMPc(block, uml::COND_Z, no_carry);
	UML_OR(block, I6, I6, C_MASK);

	UML_LABEL(block, no_carry);
	UML_SHR(block, I0, I0, I1);

	UML_LABEL(block, no_shift);
	UML_TEST(block, I0, ~0);
	UML_MOVc(block, uml::COND_Z, I5, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);
	UML_ROLINS(block, I5, I0, 3, N_MASK);

	UML_OR(block, DRC_SR, I5, I6);
	UML_STORE(block, (void *)m_core->local_regs, I4, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_shri(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I4, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I6, I2, 0x3f);
		UML_LOAD(block, I4, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I1, DRC_SR, ~(C_MASK | Z_MASK | N_MASK));
	const uint32_t n = HI_N ? (0x10 | (op & 0xf)) : (op & 0xf);
	if (HI_N || n)
		UML_ROLINS(block, I1, I4, 32 - (n - 1), 1);

	UML_SHR(block, I5, I4, n);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_ROLINS(block, I2, I5, 3, N_MASK);
	UML_OR(block, DRC_SR, I1, I2);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::shift_type HI_N>
void hyperstone_device::generate_sardi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I2, I3, dstf_code);
	UML_AND(block, I5, I2, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffff);
#endif

	UML_DSHL(block, I2, I0, 32);
	UML_DOR(block, I0, I1, I2);

	UML_AND(block, I4, DRC_SR, ~(C_MASK | Z_MASK | N_MASK));

	const uint32_t n = HI_N ? (0x10 | (op & 0xf)) : (op & 0xf);
	if (HI_N || n)
	{
		int no_carry = compiler.m_labelnum++;
		UML_DTEST(block, I2, (1 << (n - 1)));
		UML_JMPc(block, uml::COND_Z, no_carry);
		UML_OR(block, I4, I4, 1);
		UML_LABEL(block, no_carry);

		UML_DSAR(block, I2, I2, n);
	}

	UML_DTEST(block, I2, ~0ULL);
	UML_MOVc(block, uml::COND_Z, I5, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);
	UML_DROLINS(block, I5, I2, 3, N_MASK);
	UML_OR(block, DRC_SR, I4, I5);

	UML_STORE(block, (void *)m_core->local_regs, I6, I2, SIZE_DWORD, SCALE_x4);
	UML_DSHR(block, I0, I2, 32);
	UML_STORE(block, (void *)m_core->local_regs, I1, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_sard(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	generate_check_delay_pc(block, compiler, desc);

	if (src_code == dst_code || src_code == dstf_code)
	{
		return;
	}

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I2, I3, dstf_code);
	UML_AND(block, I5, I2, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffff);
#endif

	UML_DSHL(block, I2, I0, 32);
	UML_DOR(block, I0, I1, I2);

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I6, I2, 0x3f);
	UML_LOAD(block, I2, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I6, I2, 0x1f);

	int no_shift = compiler.m_labelnum++;
	UML_TEST(block, I6, ~0);
	UML_JMPc(block, uml::COND_Z, no_shift);

	UML_SUB(block, I2, I6, 1);
	UML_DSAR(block, I3, I0, I2);
	UML_AND(block, I2, I3, 1);
	UML_DSAR(block, I0, I0, I6);

	UML_LABEL(block, no_shift);
	UML_DCMP(block, I0, 0);
	UML_MOVc(block, uml::COND_E, I3, Z_MASK);
	UML_MOVc(block, uml::COND_NE, I3, 0);
	UML_OR(block, I1, I2, I3);
	UML_DROLINS(block, I1, I0, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I1, 0, (C_MASK | Z_MASK | N_MASK));

	UML_STORE(block, (void *)m_core->local_regs, I5, I0, SIZE_DWORD, SCALE_x4);
	UML_DSHR(block, I1, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_sar(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
	UML_ADD(block, I2, I1, dst_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I1, I1, src_code);
	UML_AND(block, I1, I1, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I1, I1, 0x1f);

	int no_shift = compiler.m_labelnum++;
	UML_MOV(block, I3, 0);
	UML_CMP(block, I1, 0);
	UML_JMPc(block, uml::COND_E, no_shift);
	UML_SUB(block, I4, I1, 1);
	UML_SHR(block, I4, I0, I4);
	UML_AND(block, I3, I4, 1);
	UML_SAR(block, I0, I0, I1);
	UML_LABEL(block, no_shift);

	UML_TEST(block, I0, ~0);
	UML_SETc(block, uml::COND_Z, I1);
	UML_ROLINS(block, I3, I1, Z_SHIFT, Z_MASK);
	UML_ROLINS(block, I3, I0, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I3, 0, (N_MASK | Z_MASK | C_MASK));

	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_sari(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
		UML_ADD(block, I1, I1, dst_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	const uint32_t n = HI_N ? DRC_HI_N_VALUE : DRC_LO_N_VALUE;

	UML_MOV(block, I2, 0);
	if (HI_N || n)
	{

		UML_ROLINS(block, I2, I0, 32 - (n - 1), 1);
		UML_SAR(block, I0, I0, n);
	}

	UML_TEST(block, I0, ~0);
	UML_MOVc(block, uml::COND_Z, I3, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I3, 0);
	UML_OR(block, I2, I2, I3);
	UML_ROLINS(block, I2, I0, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I2, 0, (C_MASK | Z_MASK | N_MASK));

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			UML_MOV(block, I5, I0);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I0, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I1, I0, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::shift_type HI_N>
void hyperstone_device::generate_shldi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I4, DRC_SR, 7, 0x7f); // I4: FP

	UML_ADD(block, I2, I4, dst_code);
	UML_AND(block, I2, I2, 0x3f); // I2: dst_code
	UML_LOAD(block, I6, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4); // I0: high_order

	UML_ADD(block, I3, I4, dstf_code);
	UML_AND(block, I3, I3, 0x3f); // I3: dstf_code
	UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4); // I1: low_order

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffff);
#endif

	UML_DSHL(block, I0, I6, 32);
	UML_DOR(block, I0, I0, I1); // I0: val, I1 free after this point

	UML_MOV(block, I4, HI_N ? (0x10 | (op & 0xf)) : (op & 0xf));

	UML_DSHR(block, I1, 0xffffffff00000000ULL, I4); // I1: mask

	UML_AND(block, DRC_SR, DRC_SR, ~C_MASK);

	int no_carry = compiler.m_labelnum++;
	UML_TEST(block, I4, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_carry);
	UML_DROLINS(block, DRC_SR, I0, I4, 1); // Insert carry flag
	UML_LABEL(block, no_carry);

	int no_hi_bit = compiler.m_labelnum++;
	int no_overflow = compiler.m_labelnum++;
	UML_AND(block, I5, I6, I1); // I5: high_order & mask
	UML_DSHL(block, I0, I0, I4); // I0: val << n

	UML_MOV(block, I4, 0);
	UML_DTEST(block, I0, 0x8000000000000000ULL);
	UML_JMPc(block, uml::COND_Z, no_hi_bit);
	UML_XOR(block, I5, I5, I1); // I5: (high_order & mask) ^ mask
	UML_LABEL(block, no_hi_bit);
	UML_TEST(block, I5, ~0);
	UML_JMPc(block, uml::COND_Z, no_overflow);
	UML_OR(block, I4, I4, V_MASK);
	UML_LABEL(block, no_overflow);

	UML_DTEST(block, I0, ~0ULL);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_DROLINS(block, I1, I0, 3, N_MASK);
	UML_OR(block, I1, I1, I4);
	UML_ROLINS(block, DRC_SR, I1, 0, (N_MASK | Z_MASK | V_MASK));

	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
	UML_DSHR(block, I0, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shld(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;

	generate_check_delay_pc(block, compiler, desc);

	if (src_code == dst_code || src_code == dstf_code)
	{
		return;
	}

	UML_ROLAND(block, I4, DRC_SR, 7, 0x7f); // I4: FP

	UML_ADD(block, I2, I4, dst_code);
	UML_AND(block, I2, I2, 0x3f); // I2: dst_code
	UML_LOAD(block, I6, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4); // I0: high_order

	UML_ADD(block, I3, I4, dstf_code);
	UML_AND(block, I3, I3, 0x3f); // I3: dstf_code
	UML_LOAD(block, I1, (void *)m_core->local_regs, I3, SIZE_DWORD, SCALE_x4); // I1: low_order

#ifndef PTR64
	UML_DAND(block, I1, I1, 0x00000000ffffffff);
#endif

	UML_DSHL(block, I0, I6, 32);
	UML_DOR(block, I0, I0, I1); // I0: val, I1 free after this point

	UML_ADD(block, I4, I4, src_code);
	UML_AND(block, I4, I4, 0x3f);
	UML_LOAD(block, I4, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I4, I4, 0x1f); // I4: n

	UML_DSHR(block, I1, 0xffffffff00000000ULL, I4); // I1: mask

	UML_AND(block, DRC_SR, DRC_SR, ~C_MASK);

	int no_carry = compiler.m_labelnum++;
	UML_TEST(block, I4, 0x1f);
	UML_JMPc(block, uml::COND_Z, no_carry);
	UML_DROLINS(block, DRC_SR, I0, I4, 1); // Insert carry flag
	UML_LABEL(block, no_carry);

	int no_hi_bit = compiler.m_labelnum++;
	int no_overflow = compiler.m_labelnum++;
	UML_AND(block, I5, I6, I1); // I5: high_order & mask
	UML_DSHL(block, I0, I0, I4); // I0: val << n

	UML_MOV(block, I4, 0);
	UML_DTEST(block, I0, 0x8000000000000000ULL);
	UML_JMPc(block, uml::COND_Z, no_hi_bit);
	UML_XOR(block, I5, I5, I1); // I5: (high_order & mask) ^ mask
	UML_LABEL(block, no_hi_bit);
	UML_TEST(block, I5, ~0);
	UML_JMPc(block, uml::COND_Z, no_overflow);
	UML_OR(block, I4, I4, V_MASK);
	UML_LABEL(block, no_overflow);

	UML_DTEST(block, I0, ~0ULL);
	UML_MOVc(block, uml::COND_Z, I1, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I1, 0);
	UML_DROLINS(block, I1, I0, 3, N_MASK);
	UML_OR(block, I1, I1, I4);
	UML_ROLINS(block, DRC_SR, I1, 0, (N_MASK | Z_MASK | V_MASK));

	UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
	UML_DSHR(block, I0, I0, 32);
	UML_STORE(block, (void *)m_core->local_regs, I2, I0, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_shl(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4); // I0 = dreg

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_LOAD(block, I5, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I1, I5, 0x1f); // I1 = sreg & 0x1f

	UML_AND(block, I6, DRC_SR, ~(C_MASK | Z_MASK | N_MASK | V_MASK));

	int done_shift = compiler.m_labelnum++;
	int no_carry = compiler.m_labelnum++;
	UML_CMP(block, I1, 0);
	UML_JMPc(block, uml::COND_E, done_shift);
	UML_SUB(block, I2, I1, 1);
	UML_SHL(block, I2, I0, I2);
	UML_TEST(block, I2, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_carry);
	UML_OR(block, I6, I6, C_MASK);

	UML_LABEL(block, no_carry);
	UML_DSHR(block, I5, 0xffffffff00000000ULL, I1);
	UML_AND(block, I3, I0, I5);

	UML_SHL(block, I0, I0, I1);

	int no_hi_bit = compiler.m_labelnum++;
	UML_TEST(block, I0, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_hi_bit);

	UML_XOR(block, I3, I3, I5);

	UML_LABEL(block, no_hi_bit);
	UML_TEST(block, I3, ~0);
	UML_JMPc(block, uml::COND_Z, done_shift);
	UML_OR(block, I6, I6, V_MASK);

	UML_LABEL(block, done_shift);
	UML_TEST(block, I0, ~0);
	UML_MOVc(block, uml::COND_Z, I5, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);
	UML_ROLINS(block, I5, I0, 3, N_MASK);

	UML_OR(block, DRC_SR, I5, I6);
	UML_STORE(block, (void *)m_core->local_regs, I4, I0, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_shli(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;

	generate_check_delay_pc(block, compiler, desc);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I4, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I6, I2, 0x3f);
		UML_LOAD(block, I4, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I1, DRC_SR, ~(C_MASK | Z_MASK | N_MASK | V_MASK));
	const uint32_t n = HI_N ? (0x10 | (op & 0xf)) : (op & 0xf);

	if (HI_N || n)
	{
		int skip_c = compiler.m_labelnum++;
		UML_TEST(block, I4, (0x80000000 >> (n - 1)));
		UML_JMPc(block, uml::COND_Z, skip_c);
		UML_OR(block, I1, I1, 1);
		UML_LABEL(block, skip_c);
	}

	UML_SHL(block, I5, I4, n);

	int done_v = compiler.m_labelnum++;
	uint32_t mask = (uint32_t)(0xffffffff00000000ULL >> n);

	int no_high_bit = compiler.m_labelnum++;
	UML_TEST(block, I5, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_high_bit);

	UML_AND(block, I4, I4, mask);
	UML_XOR(block, I4, I4, mask);
	UML_TEST(block, I4, ~0);
	UML_JMPc(block, uml::COND_Z, done_v);
	UML_OR(block, I1, I1, V_MASK);
	UML_JMP(block, done_v);

	UML_LABEL(block, no_high_bit);
	UML_TEST(block, I4, mask);
	UML_JMPc(block, uml::COND_Z, done_v);
	UML_OR(block, I1, I1, V_MASK);
	UML_LABEL(block, done_v);

	UML_TEST(block, I5, ~0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_ROLINS(block, I2, I5, 3, N_MASK);
	UML_OR(block, DRC_SR, I1, I2);

	if (DST_GLOBAL)
	{
		if (dst_code < 2)
		{
			UML_MOV(block, I4, dst_code);
			generate_set_global_register(block, compiler, desc);
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
	}
}


void hyperstone_device::generate_testlz(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);

	UML_LZCNT(block, I4, I0);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I1, I4, SIZE_DWORD, SCALE_x4);
}


void hyperstone_device::generate_rol(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t src_code = op & 0xf;

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4); // I0 = dreg

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_LOAD(block, I5, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I1, I5, 0x1f); // I1 = sreg & 0x1f

	int no_shift = compiler.m_labelnum++;
	UML_CMP(block, I1, 0);
	UML_JMPc(block, uml::COND_E, no_shift);
	UML_ROL(block, I2, I0, I1);
	UML_LABEL(block, no_shift);

	UML_DSHR(block, I5, 0xffffffff00000000ULL, I1);
	UML_AND(block, I3, I0, I5);

	UML_MOV(block, I6, 0);

	int no_hi_bit = compiler.m_labelnum++;
	UML_TEST(block, I0, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_hi_bit);

	UML_XOR(block, I3, I3, I5);

	int done_shift = compiler.m_labelnum++;
	UML_LABEL(block, no_hi_bit);
	UML_TEST(block, I3, ~0);
	UML_JMPc(block, uml::COND_Z, done_shift);
	UML_OR(block, I6, I6, V_MASK);

	UML_LABEL(block, done_shift);
	UML_TEST(block, I2, ~0);
	UML_MOVc(block, uml::COND_Z, I5, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);
	UML_ROLINS(block, I5, I2, 3, N_MASK);

	UML_OR(block, I5, I5, I6);
	UML_ROLINS(block, DRC_SR, I5, 0, (V_MASK | N_MASK | Z_MASK | C_MASK));
	UML_STORE(block, (void *)m_core->local_regs, I4, I2, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldxx1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	uint16_t next_1 = m_pr16(desc->pc + 2);
	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = m_pr16(desc->pc + 4);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else
	{
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (!DST_GLOBAL || !SRC_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (DST_GLOBAL)
	{
		if (dst_code == SR_REGISTER)
			UML_MOV(block, I4, 0);
		else
			UML_LOAD(block, I4, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_LOAD(block, I4, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	switch (sub_type)
	{
		case 0: // LDBS.A
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_ADD(block, I0, I4, extra_s);
			UML_CALLH(block, *m_mem_read8);
			UML_SEXT(block, I5, I1, SIZE_BYTE);

			if (SRC_GLOBAL)
			{
				UML_MOV(block, I4, src_code);
				generate_set_global_register(block, compiler, desc);
				if (src_code == PC_REGISTER)
					generate_branch(block, desc->targetpc, desc);
			}
			else
			{
				UML_ADD(block, I2, I3, src_code);
				UML_AND(block, I2, I2, 0x3f);
				UML_STORE(block, (void *)m_core->local_regs, I2, I5, SIZE_DWORD, SCALE_x4);
			}
			break;

		case 1: // LDBU.A
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_ADD(block, I0, I4, extra_s);
			UML_CALLH(block, *m_mem_read8);

			if (SRC_GLOBAL)
			{
				UML_MOV(block, I4, src_code);
				UML_MOV(block, I5, I1);
				generate_set_global_register(block, compiler, desc);
				if (src_code == PC_REGISTER)
					generate_branch(block, desc->targetpc, desc);
			}
			else
			{
				UML_ADD(block, I2, I3, src_code);
				UML_AND(block, I2, I2, 0x3f);
				UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
			}
			break;

		case 2:
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_ADD(block, I0, I4, extra_s & ~1);
			UML_AND(block, I0, I0, ~1);
			UML_CALLH(block, *m_mem_read16);

			if (SRC_GLOBAL)
			{
				UML_MOV(block, I4, src_code);
				if (extra_s & 1) // LDHS.A
					UML_SEXT(block, I5, I1, SIZE_WORD);
				else // LDHU.A
					UML_MOV(block, I5, I1);

				generate_set_global_register(block, compiler, desc);

				if (src_code == 0)
					generate_branch(block, desc->targetpc, desc);
			}
			else
			{
				UML_ADD(block, I2, I3, src_code);
				UML_AND(block, I2, I2, 0x3f);
				if (extra_s & 1)
				{
					UML_SEXT(block, I5, I1, SIZE_WORD);
					UML_STORE(block, (void *)m_core->local_regs, I2, I5, SIZE_DWORD, SCALE_x4);
				}
				else
				{
					UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
				}
			}
			break;

		case 3:
		{
			uint32_t switch_val = extra_s & 3;
			extra_s &= ~3;
			UML_ADD(block, I0, I4, extra_s);
			UML_AND(block, I0, I0, ~3);
			switch (switch_val)
			{
				case 0: // LDW.A/D
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_CALLH(block, *m_mem_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I5, I1);
						UML_MOV(block, I4, src_code);
						generate_set_global_register(block, compiler, desc);
						if (src_code == PC_REGISTER)
							generate_branch(block, desc->targetpc, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I2, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
					}
					break;
				case 1: // LDD.A
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_CALLH(block, *m_mem_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I5, I1);
						UML_MOV(block, I4, src_code);
						generate_set_global_register(block, compiler, desc);
						if (src_code == PC_REGISTER)
							generate_branch(block, desc->targetpc, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I2, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_mem_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I5, I1);
						UML_MOV(block, I4, srcf_code);
						generate_set_global_register(block, compiler, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, srcf_code);
						UML_AND(block, I2, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
					}
					break;
				case 2: // LDW.IOD
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_ROLAND(block, I0, I0, 21, 0x7ffc);
					UML_CALLH(block, *m_io_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I5, I1);
						UML_MOV(block, I4, src_code);
						generate_set_global_register(block, compiler, desc);
						if (src_code == 0)
							generate_branch(block, desc->targetpc, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I2, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
					}
					break;

				case 3: // LDD.IODs
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_ROLAND(block, I0, I0, 21, 0x7ffc);
					UML_CALLH(block, *m_io_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I5, I1);
						UML_MOV(block, I4, src_code);
						generate_set_global_register(block, compiler, desc);
						if (src_code == PC_REGISTER)
							generate_branch(block, desc->targetpc, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I2, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_io_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I5, I1);
						UML_MOV(block, I4, srcf_code);
						generate_set_global_register(block, compiler, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, srcf_code);
						UML_AND(block, I2, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I2, I1, SIZE_DWORD, SCALE_x4);
					}
					break;
			}
			break;
		}
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldxx2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];
	uint16_t next_1 = m_pr16(desc->pc + 2);
	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = m_pr16(desc->pc + 4);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else
	{
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t srcf_code = src_code + 1;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (DST_GLOBAL && dst_code < 2)
	{
		printf("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", desc->pc);
		return;
	}

	if (!DST_GLOBAL || !SRC_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I6, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_LOAD(block, I6, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	switch (sub_type)
	{
		case 0: // LDBS.N
		case 1: // LDBU.N
		case 2: // LDHS.N, LDHU.N
		{
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			if (sub_type == 0)
			{
				UML_MOV(block, I0, I6);
				UML_CALLH(block, *m_mem_read8);
				UML_SEXT(block, I5, I1, SIZE_BYTE);
			}
			else if (sub_type == 2)
			{
				UML_AND(block, I0, I6, ~1);
				UML_CALLH(block, *m_mem_read16);
				if (extra_s & 1)
					UML_SEXT(block, I5, I1, SIZE_WORD);
				else
					UML_MOV(block, I5, I1);
			}
			else
			{
				UML_MOV(block, I0, I6);
				UML_CALLH(block, *m_mem_read8);
				UML_MOV(block, I5, I1);
			}

			if (SRC_GLOBAL)
			{
				UML_MOV(block, I4, src_code);
				generate_set_global_register(block, compiler, desc);
			}
			else
			{
				UML_ADD(block, I2, I3, src_code);
				UML_AND(block, I4, I2, 0x3f);
				UML_STORE(block, (void *)m_core->local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
			}

			if (DST_GLOBAL != SRC_GLOBAL || src_code != dst_code)
			{
				if (sub_type == 2)
					UML_ADD(block, I4, I6, extra_s & ~1);
				else
					UML_ADD(block, I4, I6, extra_s);

				if (DST_GLOBAL)
				{
					UML_STORE(block, (void *)m_core->global_regs, dst_code, I4, SIZE_DWORD, SCALE_x4);
				}
				else
				{
					UML_ADD(block, I2, I3, dst_code);
					UML_AND(block, I5, I2, 0x3f);
					UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
				}
			}
			break;
		}
		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.N
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_AND(block, I0, I6, ~3);
					UML_CALLH(block, *m_mem_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I4, src_code);
						UML_MOV(block, I5, I1);
						generate_set_global_register(block, compiler, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I4, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
					}

					if (DST_GLOBAL != SRC_GLOBAL || src_code != dst_code)
					{
						UML_ADD(block, I4, I6, extra_s);

						if (DST_GLOBAL)
						{
							UML_STORE(block, (void *)m_core->global_regs, dst_code, I4, SIZE_DWORD, SCALE_x4);
						}
						else
						{
							UML_ADD(block, I2, I3, dst_code);
							UML_AND(block, I5, I2, 0x3f);
							UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
						}
					}
					break;
				}
				case 1: // LDD.N
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_AND(block, I0, I6, ~3);
					UML_CALLH(block, *m_mem_read32);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I4, src_code);
						UML_MOV(block, I5, I1);
						generate_set_global_register(block, compiler, desc);

						UML_ADD(block, I0, I0, 4);
						UML_CALLH(block, *m_mem_read32);

						UML_MOV(block, I4, src_code);
						UML_MOV(block, I5, I1);
						generate_set_global_register(block, compiler, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I4, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);

						UML_ADD(block, I0, I0, 4);
						UML_CALLH(block, *m_mem_read32);

						UML_ADD(block, I2, I3, srcf_code);
						UML_AND(block, I4, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
					}

					if (DST_GLOBAL != SRC_GLOBAL || src_code != dst_code)
					{
						UML_ADD(block, I4, I6, extra_s & ~1);

						if (DST_GLOBAL)
						{
							UML_STORE(block, (void *)m_core->global_regs, dst_code, I4, SIZE_DWORD, SCALE_x4);
						}
						else
						{
							UML_ADD(block, I2, I3, dst_code);
							UML_AND(block, I5, I2, 0x3f);
							UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
						}
					}
					break;
				}
				case 2: // Reserved
					printf("Reserved instruction in generate_ldxx2. PC = %08X\n", desc->pc);
					break;
				case 3: // LDW.S
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_3));

					int below_sp = compiler.m_labelnum++;
					int done = compiler.m_labelnum++;

					UML_MOV(block, I2, mem(&m_core->global_regs[SP_REGISTER]));
					UML_CMP(block, I6, I2);
					UML_JMPc(block, uml::COND_B, below_sp);

					UML_ROLAND(block, I0, I6, 30, 0x3f);
					UML_LOAD(block, I1, (void *)m_core->local_regs, I0, SIZE_DWORD, SCALE_x4);
					UML_JMP(block, done);

					UML_LABEL(block, below_sp);
					UML_AND(block, I0, I6, ~3);
					UML_CALLH(block, *m_mem_read32);

					UML_LABEL(block, done);

					if (SRC_GLOBAL)
					{
						UML_MOV(block, I4, src_code);
						UML_MOV(block, I5, I1);
						generate_set_global_register(block, compiler, desc);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code);
						UML_AND(block, I4, I2, 0x3f);
						UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
					}

					if (DST_GLOBAL != SRC_GLOBAL || src_code != dst_code)
					{
						UML_ADD(block, I4, I6, extra_s & ~3);

						if (DST_GLOBAL)
						{
							UML_STORE(block, (void *)m_core->global_regs, dst_code, I4, SIZE_DWORD, SCALE_x4);
						}
						else
						{
							UML_ADD(block, I2, I3, dst_code);
							UML_AND(block, I5, I2, 0x3f);
							UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
						}
					}
					break;
				}
			}
			break;
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stxx1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];

	uint16_t next_1 = m_pr16(desc->pc + 2);
	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = m_pr16(desc->pc + 4);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else
	{
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (DST_GLOBAL)
	{
		if (dst_code == SR_REGISTER)
			UML_MOV(block, I0, 0);
		else
			UML_LOAD(block, I0, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I1, I3, dst_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I1, I3, src_code);
		UML_AND(block, I1, I1, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	switch (sub_type)
	{
		case 0: // STBS.D
		case 1: // STBU.D
			// TODO: missing trap on range error for STBS.D
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_ADD(block, I0, I0, extra_s);
			UML_CALLH(block, *m_mem_write8);
			break;

		case 2: // STHS.D, STHU.D
			// TODO: missing trap on range error with STHS.D
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_ADD(block, I0, I0, extra_s);
			UML_AND(block, I0, I0, ~1);
			UML_CALLH(block, *m_mem_write16);
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.D
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_ADD(block, I0, I0, extra_s & ~1);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);
					break;
				case 1: // STD.D
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_ADD(block, I0, I0, extra_s & ~1);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);

					if (SRC_GLOBAL)
					{
						if (src_code == SR_REGISTER)
							UML_MOV(block, I1, 0);
						else
							UML_LOAD(block, I1, (void *)m_core->global_regs, src_code + 1, SIZE_DWORD, SCALE_x4);
					}
					else
					{
						UML_ADD(block, I1, I3, src_code + 1);
						UML_AND(block, I1, I1, 0x3f);
						UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_mem_write32);
					break;
				}
				case 2: // STW.IOD
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_ADD(block, I0, I0, extra_s & ~3);
					UML_ROLAND(block, I0, I0, 21, 0x7ffc);
					UML_CALLH(block, *m_io_write32);
					break;
				case 3: // STD.IOD
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_ADD(block, I0, I0, extra_s & ~1); // Is this correct?
					UML_ROLAND(block, I0, I0, 21, 0x7ffc);
					UML_CALLH(block, *m_io_write32);

					if (SRC_GLOBAL)
					{
						if (src_code == SR_REGISTER)
							UML_MOV(block, I1, 0);
						else
							UML_LOAD(block, I1, (void *)m_core->global_regs, src_code + 1, SIZE_DWORD, SCALE_x4);
					}
					else
					{
						UML_ADD(block, I1, I3, src_code + 1);
						UML_AND(block, I1, I1, 0x3f);
						UML_LOAD(block, I1, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_io_write32);
					break;
				}
			}
			break;
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stxx2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];

	uint16_t next_1 = m_pr16(desc->pc + 2);
	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = m_pr16(desc->pc + 4);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else
	{
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if (DST_GLOBAL && dst_code < 2)
	{
		printf("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", desc->pc);
		UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
		return;
	}

	if (!DST_GLOBAL || !SRC_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I0, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I6, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	switch (sub_type)
	{
		case 0: // STBS.N
		case 1: // STBU.N
			// TODO: missing trap on range error with STBS.N
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_CALLH(block, *m_mem_write8);
			UML_ADD(block, I0, I0, extra_s);

			if (DST_GLOBAL)
				UML_STORE(block, (void *)m_core->global_regs, dst_code, I0, SIZE_DWORD, SCALE_x4);
			else
				UML_STORE(block, (void *)m_core->local_regs, I6, I0, SIZE_DWORD, SCALE_x4);
			break;
		case 2: // STHS.N, STHU.N
			// TODO: missing trap on range error with STHS.N
			UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
			UML_MOV(block, I5, I0);
			UML_AND(block, I0, I0, ~1);
			UML_CALLH(block, *m_mem_write16);
			UML_ADD(block, I5, I5, extra_s & ~1);

			if (DST_GLOBAL)
				UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
			else
				UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
			break;
		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.N
					UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
					UML_MOV(block, I5, I0);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);
					UML_ADD(block, I5, I5, extra_s);

					if (DST_GLOBAL)
						UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
					else
						UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
					break;
				case 1: // STD.N
					UML_MOV(block, I7, mem(&m_core->clock_cycles_2));
					UML_MOV(block, I5, I0);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);

					UML_ADD(block, I5, I5, extra_s & ~1);
					if (DST_GLOBAL)
						UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
					else
						UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);

					if (SRC_GLOBAL)
					{
						UML_LOAD(block, I1, (void *)m_core->global_regs, src_code + 1, SIZE_DWORD, SCALE_x4);
					}
					else
					{
						UML_ADD(block, I2, I3, src_code + 1);
						UML_AND(block, I4, I2, 0x3f);
						UML_LOAD(block, I1, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
					}

					UML_ADD(block, I0, I0, 4);
					UML_CALLH(block, *m_mem_write32);
					break;
				case 2: // Reserved
					printf("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", desc->pc);
					break;
				case 3: // STW.S
				{
					UML_MOV(block, I7, mem(&m_core->clock_cycles_3));

					int less_than_sp = compiler.m_labelnum++;
					int store_done = compiler.m_labelnum++;

					UML_MOV(block, I5, I0);
					UML_CMP(block, I5, mem(&SP));
					UML_JMPc(block, uml::COND_B, less_than_sp);

					UML_ROLAND(block, I4, I0, 30, 0x3f);
					UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
					UML_JMP(block, store_done);

					UML_LABEL(block, less_than_sp);
					UML_AND(block, I0, I0, ~3);
					UML_CALLH(block, *m_mem_write32);

					UML_LABEL(block, store_done);
					UML_ADD(block, I5, I5, extra_s & ~3);
					if (DST_GLOBAL)
						UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
					else
						UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
					break;
				}
			}
			break;
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL, hyperstone_device::sign_mode SIGNED>
void hyperstone_device::generate_mulsu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_36));

	uint16_t op = desc->opptr.w[0];

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t dstf_code = dst_code + 1;
	const uint32_t src_code = op & 0xf;

	if ((SRC_GLOBAL && src_code < 2) || (DST_GLOBAL && dst_code < 2))
	{
		printf("Denoted PC or SR in hyperstone_muls/u instruction. PC = %08X\n", desc->pc);
		return;
	}

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
	{
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I6, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

	if (SIGNED == IS_SIGNED)
		UML_MULS(block, I4, I5, I0, I1);
	else
		UML_MULU(block, I4, I5, I0, I1);

	UML_OR(block, I2, I4, I5);
	UML_TEST(block, I2, ~0);
	UML_MOVc(block, uml::COND_NZ, I2, 0);
	UML_MOVc(block, uml::COND_Z, I2, Z_MASK);
	UML_ROLINS(block, I2, I5, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I2, 0, (N_MASK | Z_MASK));

	if (DST_GLOBAL)
	{
		UML_STORE(block, (void *)m_core->global_regs, dst_code, I5, SIZE_DWORD, SCALE_x4);
		UML_STORE(block, (void *)m_core->global_regs, dstf_code, I4, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_STORE(block, (void *)m_core->local_regs, I6, I5, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I2, I3, dstf_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I4, SIZE_DWORD, SCALE_x4);
	}

	int done = compiler.m_labelnum++;
	UML_MOV(block, I7, mem(&m_core->clock_cycles_6));
	if (SIGNED == IS_SIGNED)
	{
		UML_CMP(block, I0, 0xffff8000);
		UML_JMPc(block, uml::COND_B, done);
		UML_CMP(block, I0, 0x00008000);
		UML_JMPc(block, uml::COND_AE, done);
		UML_CMP(block, I1, 0xffff8000);
		UML_JMPc(block, uml::COND_B, done);
		UML_CMP(block, I1, 0x00008000);
		UML_JMPc(block, uml::COND_AE, done);
	}
	else
	{
		UML_CMP(block, I0, 0x0000ffff);
		UML_JMPc(block, uml::COND_A, done);
		UML_CMP(block, I1, 0x0000ffff);
		UML_JMPc(block, uml::COND_A, done);
	}
	UML_SUB(block, I7, I7, mem(&m_core->clock_cycles_2));

	UML_LABEL(block, done);
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_mul(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	if ((SRC_GLOBAL && src_code < 2) || (DST_GLOBAL && dst_code < 2))
	{
		printf("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", desc->pc);
		return;
	}

	if (!SRC_GLOBAL || !DST_GLOBAL)
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
		UML_LOAD(block, I0, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I1, I2, 0x3f);
		UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	}

	if (DST_GLOBAL)
		UML_LOAD(block, I1, (void *)m_core->global_regs, dst_code, SIZE_DWORD, SCALE_x4);
	else
	{
		UML_ADD(block, I2, I3, dst_code);
		UML_AND(block, I6, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I6, SIZE_DWORD, SCALE_x4);
	}

	UML_MULU(block, I2, I3, I0, I1);

	UML_AND(block, I4, DRC_SR, ~(Z_MASK | N_MASK));
	UML_TEST(block, I2, ~0);
	UML_MOVc(block, uml::COND_Z, I5, Z_MASK);
	UML_MOVc(block, uml::COND_NZ, I5, 0);
	UML_ROLINS(block, I5, I2, 3, N_MASK);
	UML_ROLINS(block, DRC_SR, I5, 0, (Z_MASK | N_MASK));

	if (DST_GLOBAL)
		UML_STORE(block, (void *)m_core->global_regs, dst_code, I2, SIZE_DWORD, SCALE_x4);
	else
		UML_STORE(block, (void *)m_core->local_regs, I6, I2, SIZE_DWORD, SCALE_x4);

	UML_MOV(block, I7, mem(&m_core->clock_cycles_3));
	int add_cycles = compiler.m_labelnum++;
	int done = compiler.m_labelnum++;
	UML_CMP(block, I0, 0xffff8000);
	UML_JMPc(block, uml::COND_B, add_cycles);
	UML_CMP(block, I0, 0x8000);
	UML_JMPc(block, uml::COND_AE, add_cycles);
	UML_CMP(block, I1, 0xffff8000);
	UML_JMPc(block, uml::COND_B, add_cycles);
	UML_CMP(block, I1, 0x8000);
	UML_JMPc(block, uml::COND_AE, add_cycles);
	UML_JMP(block, done);

	UML_LABEL(block, add_cycles);
	UML_ADD(block, I7, I7, mem(&m_core->clock_cycles_2));

	UML_LABEL(block, done);
	// TODO: proper cycle counts
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_set(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t dst_code = (op & 0xf0) >> 4;
	const uint32_t n = op & 0xf;

	if (DST_GLOBAL && dst_code < 2)
	{
		return;
	}

	if (HI_N)
	{
		if (n >= 4 || n == 2)
		{
			static const uint32_t   set_result[16] = { 0, 0, 0,          0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0 };
			static const uint32_t unset_result[16] = { 0, 0, 0xffffffff, 0,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff,  0, 0xffffffff };
			static const uint32_t mask[16] = { 0, 0, 0, 0, (N_MASK | Z_MASK), (N_MASK | Z_MASK), N_MASK, N_MASK,
				(C_MASK | Z_MASK), (C_MASK | Z_MASK), C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, V_MASK };

			UML_TEST(block, DRC_SR, mask[n]);
			UML_MOVc(block, uml::COND_NZ, I0, set_result[n]);
			UML_MOVc(block, uml::COND_Z, I0, unset_result[n]);
		}
		else
		{
			printf("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, desc->pc);
			return;
		}
	}
	else
	{
		if (n == 0)
		{
			int no_low_bit = compiler.m_labelnum++;
			UML_MOV(block, I1, mem(&m_core->global_regs[SP_REGISTER]));
			UML_AND(block, I0, I1, 0xfffffe00);
			UML_ROLINS(block, I0, DRC_SR, 9, 0x000001fc);
			UML_TEST(block, I1, 0x100);
			UML_JMPc(block, uml::COND_Z, no_low_bit);
			UML_TEST(block, DRC_SR, 0x80000000);
			UML_JMPc(block, uml::COND_NZ, no_low_bit);
			UML_OR(block, I0, I0, 1);
			UML_LABEL(block, no_low_bit);
		}
		else if (n >= 2)
		{
			static const uint32_t   set_result[16] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };
			static const uint32_t unset_result[16] = { 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
			static const uint32_t mask[16] = { 0, 0, 0, 0, (N_MASK | Z_MASK), (N_MASK | Z_MASK), N_MASK, N_MASK,
				(C_MASK | Z_MASK), (C_MASK | Z_MASK), C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, V_MASK };

			UML_TEST(block, DRC_SR, mask[n]);
			UML_MOVc(block, uml::COND_NZ, I0, set_result[n]);
			UML_MOVc(block, uml::COND_Z, I0, unset_result[n]);
		}
		else
		{
			printf("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, desc->pc);
			return;
		}
	}

	if (DST_GLOBAL)
	{
		UML_STORE(block, (void *)m_core->global_regs, dst_code, I0, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I1, dst_code);
		UML_AND(block, I3, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I3, I0, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldwr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I0, I0, ~3);
	UML_CALLH(block, *m_mem_read32);

	if (SRC_GLOBAL)
	{
		if (src_code < 2)
		{
			UML_MOV(block, I4, src_code);
			UML_MOV(block, I5, I1);
			generate_set_global_register(block, compiler, desc);
			if (src_code == PC_REGISTER)
			{
				generate_branch(block, desc->targetpc, desc);
			}
		}
		else
		{
			UML_STORE(block, (void *)m_core->global_regs, src_code, I1, SIZE_DWORD, SCALE_x4);
		}
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_lddr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I1, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I1, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I0, I0, ~3);
	UML_CALLH(block, *m_mem_read32);

	if (SRC_GLOBAL)
	{
		UML_MOV(block, I4, src_code);
		UML_MOV(block, I5, I1);
		generate_set_global_register(block, compiler, desc);

		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);

		UML_MOV(block, I4, src_code + 1);
		UML_MOV(block, I5, I1);
		generate_set_global_register(block, compiler, desc);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I4, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);

		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_read32);

		UML_ADD(block, I2, I3, src_code + 1);
		UML_AND(block, I4, I2, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldwp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
	UML_ADD(block, I1, I0, dst_code);
	UML_AND(block, I2, I1, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I3, I0, 4);
	UML_AND(block, I0, I0, ~3);
	UML_CALLH(block, *m_mem_read32);

	if (SRC_GLOBAL)
	{
		UML_MOV(block, I4, src_code);
		UML_MOV(block, I5, I1);
		generate_set_global_register(block, compiler, desc);

		UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
		UML_ADD(block, I1, I0, dst_code);
		UML_AND(block, I2, I1, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I2, I3, SIZE_DWORD, SCALE_x4);

		if (src_code == PC_REGISTER)
			generate_branch(block, desc->targetpc, desc);
	}
	else
	{
		UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
		UML_ADD(block, I4, I0, src_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I1, SIZE_DWORD, SCALE_x4);

		if (src_code != dst_code)
		{
			UML_ADD(block, I4, I0, dst_code);
			UML_AND(block, I5, I4, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I5, I3, SIZE_DWORD, SCALE_x4);
		}
	}
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_lddp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
	UML_ADD(block, I1, I0, dst_code);
	UML_AND(block, I2, I1, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I3, I0, 8);
	UML_AND(block, I0, I0, ~3);
	UML_CALLH(block, *m_mem_read32);
	UML_MOV(block, I2, I1);             // I2: dreg[0]
	UML_ADD(block, I0, I0, 4);
	UML_CALLH(block, *m_mem_read32);    // I1: dreg[4]

	if (SRC_GLOBAL)
	{
		UML_MOV(block, I4, src_code);
		UML_MOV(block, I5, I2);
		generate_set_global_register(block, compiler, desc);
		UML_MOV(block, I4, src_code + 1);
		UML_MOV(block, I5, I1);
		generate_set_global_register(block, compiler, desc);

		UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
		UML_ADD(block, I1, I0, dst_code);
		UML_AND(block, I2, I1, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I2, I3, SIZE_DWORD, SCALE_x4);

		if (src_code == PC_REGISTER || (src_code + 1) == PC_REGISTER)
			generate_branch(block, desc->targetpc, desc);
	}
	else
	{
		UML_ROLAND(block, I0, DRC_SR, 7, 0x7f);
		UML_ADD(block, I4, I0, src_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I2, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I4, I0, src_code + 1);
		UML_AND(block, I5, I4, 0x3f);
		UML_STORE(block, (void *)m_core->local_regs, I5, I1, SIZE_DWORD, SCALE_x4);

		if (src_code != dst_code && (src_code + 1) != dst_code)
		{
			UML_ADD(block, I4, I0, dst_code);
			UML_AND(block, I5, I4, 0x3f);
			UML_STORE(block, (void *)m_core->local_regs, I5, I3, SIZE_DWORD, SCALE_x4);
		}
	}
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stwr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I0, I0, ~3);
	UML_CALLH(block, *m_mem_write32);
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stdr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);

		if ((src_code + 1) == SR_REGISTER)
			UML_MOV(block, I2, 0);
		else
			UML_LOAD(block, I2, (void *)m_core->global_regs, src_code + 1, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I4, I3, src_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
		UML_ADD(block, I4, I3, src_code + 1);
		UML_AND(block, I5, I4, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	UML_ADD(block, I4, I3, dst_code);
	UML_AND(block, I5, I4, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I0, I0, ~3);

	UML_CALLH(block, *m_mem_write32);
	UML_ADD(block, I0, I0, 4);
	UML_MOV(block, I1, I2);
	UML_CALLH(block, *m_mem_write32);
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stwp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I0, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I0, SIZE_DWORD, SCALE_x4);
	}

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f);
	UML_LOAD(block, I5, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_AND(block, I0, I5, ~3);
	UML_CALLH(block, *m_mem_write32);
	UML_ADD(block, I2, I5, 4);
	UML_STORE(block, (void *)m_core->local_regs, I4, I2, SIZE_DWORD, SCALE_x4);
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stdp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I4, I2, 0x3f); // I4 = dst_code
	UML_LOAD(block, I0, (void *)m_core->local_regs, I4, SIZE_DWORD, SCALE_x4); // I0 = dreg

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);

		UML_CALLH(block, *m_mem_write32);

		if ((src_code + 1) == SR_REGISTER)
			UML_MOV(block, I1, 0);
		else
			UML_LOAD(block, I1, (void *)m_core->global_regs, src_code + 1, SIZE_DWORD, SCALE_x4);

		UML_ADD(block, I2, I0, 4);
		UML_AND(block, I0, I2, ~3);
		UML_CALLH(block, *m_mem_write32);
		UML_ADD(block, I2, I2, 4);
		UML_STORE(block, (void *)m_core->local_regs, I4, I2, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		int srcf_dst_equal = compiler.m_labelnum++;
		int done = compiler.m_labelnum++;

		UML_MOV(block, I6, I0);
		UML_AND(block, I0, I0, ~3);
		UML_ADD(block, I2, I3, src_code);
		UML_AND(block, I5, I2, 0x3f);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
		UML_CALLH(block, *m_mem_write32);

		UML_ADD(block, I2, I5, 1);
		UML_AND(block, I5, I2, 0x3f);
		UML_ADD(block, I1, I6, 8);
		UML_CMP(block, I4, I5);
		UML_JMPc(block, uml::COND_E, srcf_dst_equal);

		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);
		UML_LOAD(block, I1, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
		UML_JMP(block, done);

		UML_LABEL(block, srcf_dst_equal);
		UML_STORE(block, (void *)m_core->local_regs, I4, I1, SIZE_DWORD, SCALE_x4);

		UML_LABEL(block, done);
		UML_ADD(block, I0, I0, 4);
		UML_CALLH(block, *m_mem_write32);
	}
}


template <hyperstone_device::branch_condition CONDITION, hyperstone_device::condition_set COND_SET>
void hyperstone_device::generate_b(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	static const uint32_t condition_masks[6] = { V_MASK, Z_MASK, C_MASK, C_MASK | Z_MASK, N_MASK, N_MASK | Z_MASK };

	int done = compiler.m_labelnum++;
	uml::condition_t condition = COND_SET ? uml::COND_Z : uml::COND_NZ;

	int skip;
	UML_TEST(block, DRC_SR, condition_masks[CONDITION]);
	UML_JMPc(block, condition, skip = compiler.m_labelnum++);
	generate_br(block, compiler, desc);

	UML_JMP(block, done);

	UML_LABEL(block, skip);
	generate_ignore_pcrel(block, desc);
	generate_check_delay_pc(block, compiler, desc);
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	UML_LABEL(block, done);
}


void hyperstone_device::generate_br(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	generate_decode_pcrel(block, desc);
	generate_check_delay_pc(block, compiler, desc);

	UML_ADD(block, DRC_PC, DRC_PC, I1);
	UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);

	generate_branch(block, desc->targetpc, desc);
	// TODO: correct cycle count
}


template <hyperstone_device::branch_condition CONDITION, hyperstone_device::condition_set COND_SET>
void hyperstone_device::generate_db(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	static const uint32_t condition_masks[6] = { V_MASK, Z_MASK, C_MASK, C_MASK | Z_MASK, N_MASK, N_MASK | Z_MASK };
	int skip_jump = compiler.m_labelnum++;
	int done = compiler.m_labelnum++;

	UML_TEST(block, DRC_SR, condition_masks[CONDITION]);
	if (COND_SET)
		UML_JMPc(block, uml::COND_Z, skip_jump);
	else
		UML_JMPc(block, uml::COND_NZ, skip_jump);

	generate_dbr(block, compiler, desc);
	UML_JMP(block, done);

	UML_LABEL(block, skip_jump);
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
	generate_ignore_pcrel(block, desc);
	generate_check_delay_pc(block, compiler, desc);

	UML_LABEL(block, done);
}


void hyperstone_device::generate_dbr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_2));

	generate_decode_pcrel(block, desc);
	generate_check_delay_pc(block, compiler, desc);

	UML_MOV(block, mem(&m_core->delay_slot), 1);
	UML_ADD(block, mem(&m_core->delay_pc), DRC_PC, I1);
	UML_MOV(block, mem(&m_core->intblock), 3);
}


void hyperstone_device::generate_frame(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	const uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	UML_ROLAND(block, I1, DRC_SR, 7, 0x7f);
	UML_SUB(block, I1, I1, op & 0xf);
	UML_ROLINS(block, DRC_SR, I1, 25, 0xfe000000);  // SET_FP(GET_FP - SRC_CODE)
	UML_ROLINS(block, DRC_SR, op, 17, 0x01e00000);  // SET_FL(DST_CODE)
	UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);        // SET_M(0)

	UML_MOV(block, I0, mem(&SP));
	UML_MOV(block, I6, I0);
	UML_AND(block, I0, I0, ~3);
	const uint32_t dst_code = (op & 0xf0) >> 4;
	UML_ADD(block, I1, I1, dst_code ? dst_code : 16);
	UML_ROLAND(block, I2, I0, 30, 0x7f);
	UML_ADD(block, I2, I2, (64 - 10));
	UML_SUB(block, I3, I2, I1);
	UML_SEXT(block, I3, I3, SIZE_BYTE);             // difference = ((SP & 0x1fc) >> 2) + (64 - 10) - ((GET_FP - SRC_CODE) + GET_FL)

	int diff_in_range, done;
	UML_CMP(block, I3, -64);
	UML_JMPc(block, uml::COND_L, done = compiler.m_labelnum++);
	UML_CMP(block, I3, 64);
	UML_JMPc(block, uml::COND_L, diff_in_range = compiler.m_labelnum++);
	UML_OR(block, I3, I3, 0xffffff80);
	UML_LABEL(block, diff_in_range);

	UML_CMP(block, I0, mem(&UB));
	UML_SETc(block, uml::COND_AE, I4);
	UML_CMP(block, I3, 0);
	UML_JMPc(block, uml::COND_GE, done);

	int push_next;
	UML_LABEL(block, push_next = compiler.m_labelnum++);
	UML_ROLAND(block, I2, I0, 30, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4);
	UML_CALLH(block, *m_mem_write32);
	UML_ADD(block, I0, I0, 4);
	UML_ADD(block, I6, I6, 4);
	UML_ADD(block, I3, I3, 1);

	UML_TEST(block, I3, ~0);
	UML_JMPc(block, uml::COND_NZ, push_next);

	UML_MOV(block, mem(&SP), I6);

	UML_TEST(block, I4, ~0);
	UML_EXHc(block, uml::COND_NZ, *m_exception[EXCEPTION_FRAME_ERROR], 0);

	UML_LABEL(block, done);
}

template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_call(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));
	UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT), 0, ILC_MASK);

	uint16_t op = desc->opptr.w[0];
	uint16_t imm_1 = m_pr16(desc->pc + 2);

	int32_t extra_s = 0;

	if (imm_1 & 0x8000)
	{
		uint16_t imm_2 = m_pr16(desc->pc + 4);

		extra_s = imm_2;
		extra_s |= ((imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			extra_s |= 0xc0000000;

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else
	{
		extra_s = imm_1 & 0x3fff;

		if (imm_1 & 0x4000)
			extra_s |= 0xffffc000;

		UML_ADD(block, DRC_PC, DRC_PC, 2);
	}

	UML_MOV(block, I1, extra_s);

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	uint32_t dst_code = (op & 0xf0) >> 4;

	if (!dst_code)
		dst_code = 16;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	if (SRC_GLOBAL)
	{
		if (src_code == SR_REGISTER)
			UML_MOV(block, I2, 0);
		else
			UML_LOAD(block, I2, (void *)m_core->global_regs, src_code, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I4, I3, src_code);
		UML_AND(block, I5, I4, 0x3f);
		UML_LOAD(block, I2, (void *)m_core->local_regs, I5, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I4, DRC_PC, ~1);
	UML_ROLINS(block, I4, DRC_SR, 32-S_SHIFT, 1);

	UML_ADD(block, I1, I3, dst_code);
	UML_AND(block, I6, I1, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I6, I4, SIZE_DWORD, SCALE_x4);

	UML_ADD(block, I4, I6, 1);
	UML_AND(block, I5, I4, 0x3f);
	UML_STORE(block, (void *)m_core->local_regs, I5, DRC_SR, SIZE_DWORD, SCALE_x4);

	UML_ROLINS(block, DRC_SR, I1, 25, 0xfe000000);
	UML_ROLINS(block, DRC_SR, 6, 21, 0x01e00000);
	UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);

	UML_ADD(block, DRC_PC, I2, extra_s & ~1);

	UML_MOV(block, mem(&m_core->intblock), 2);

	generate_branch(block, desc->targetpc, nullptr);
	//TODO: add interrupt locks, errors, ....
}



void hyperstone_device::generate_trap_op(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1)); // TODO: with the latency it can change

	static const uint32_t conditions[16] = {
		0, 0, 0, 0, N_MASK | Z_MASK, N_MASK | Z_MASK, N_MASK, N_MASK, C_MASK | Z_MASK, C_MASK | Z_MASK, C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, 0
	};
	static const bool trap_if_set[16] = {
		false, false, false, false, true, false, true, false, true, false, true, false, true, false, true, false
	};

	uint16_t op = desc->opptr.w[0];

	generate_check_delay_pc(block, compiler, desc);

	const uint8_t trapno = (op & 0xfc) >> 2;
	const uint8_t code = ((op & 0x300) >> 6) | (op & 0x03);

	UML_TEST(block, DRC_SR, conditions[code]);

	int skip_trap = compiler.m_labelnum++;
	if (trap_if_set[code])
		UML_JMPc(block, uml::COND_Z, skip_trap);
	else
		UML_JMPc(block, uml::COND_NZ, skip_trap);

	UML_ROLINS(block, DRC_SR, ((desc->length >> 1) << ILC_SHIFT), 0, ILC_MASK);
	generate_get_trap_addr(block, compiler.m_labelnum, trapno);
	generate_trap_exception_or_int<IS_TRAP>(block);

	UML_LABEL(block, skip_trap);
}

void hyperstone_device::generate_extend(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	UML_MOV(block, I7, mem(&m_core->clock_cycles_1));

	uint16_t op = desc->opptr.w[0];

	uint16_t func = m_pr16(desc->pc + 2);
	UML_ADD(block, DRC_PC, DRC_PC, 2);

	generate_check_delay_pc(block, compiler, desc);

	const uint32_t src_code = op & 0xf;
	const uint32_t dst_code = (op & 0xf0) >> 4;

	UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);

	UML_ADD(block, I2, I3, src_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_LOAD(block, I0, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4); // I0: vals

	UML_ADD(block, I2, I3, dst_code);
	UML_AND(block, I2, I2, 0x3f);
	UML_LOAD(block, I1, (void *)m_core->local_regs, I2, SIZE_DWORD, SCALE_x4); // I1: vald

	switch (func)
	{
		// signed or unsigned multiplication, single word product
		case EMUL:
		case EMUL_N: // used in "N" type cpu
		{
			UML_MULU(block, I2, I3, I0, I1);
			UML_STORE(block, (void *)m_core->global_regs, 15, I2, SIZE_DWORD, SCALE_x4);
			break;
		}

		case EMULU: // unsigned multiplication, double word product
		case EMULS: // signed multiplication, double word product
		{
			if (func == EMULU)
				UML_MULU(block, I2, I3, I0, I1);
			else
				UML_MULS(block, I2, I3, I0, I1);
			UML_STORE(block, (void *)m_core->global_regs, 14, I3, SIZE_DWORD, SCALE_x4);
			UML_STORE(block, (void *)m_core->global_regs, 15, I2, SIZE_DWORD, SCALE_x4);
			break;
		}

		case EMAC:  // signed multiply/add, single word product sum
		case EMSUB: // signed multiply/substract, single word product difference
		{
			UML_MULS(block, I2, I3, I0, I1);
			UML_LOAD(block, I3, (void *)m_core->global_regs, 15, SIZE_DWORD, SCALE_x4);
			if (func == EMAC)
				UML_ADD(block, I3, I3, I2);
			else
				UML_SUB(block, I3, I3, I2);
			UML_STORE(block, (void *)m_core->global_regs, 15, I3, SIZE_DWORD, SCALE_x4);
			break;
		}

		case EMACD:  // signed multiply/add, double word product sum
		case EMSUBD: // signed multiply/substract, double word product difference
		{
			UML_DSEXT(block, I0, I0, SIZE_DWORD);
			UML_DSEXT(block, I1, I1, SIZE_DWORD);
			UML_DMULS(block, I2, I3, I0, I1);
			UML_LOAD(block, I3, (void *)m_core->global_regs, 14, SIZE_DWORD, SCALE_x4);
			UML_LOAD(block, I4, (void *)m_core->global_regs, 15, SIZE_DWORD, SCALE_x4);
			UML_DSHL(block, I3, I3, 32);
			UML_DOR(block, I3, I3, I4);
			if (func == EMACD)
				UML_DADD(block, I3, I3, I2);
			else
				UML_DSUB(block, I3, I3, I2);
			UML_STORE(block, (void *)m_core->global_regs, 15, I3, SIZE_DWORD, SCALE_x4);
			UML_DSHR(block, I3, I3, 32);
			UML_STORE(block, (void *)m_core->global_regs, 14, I3, SIZE_DWORD, SCALE_x4);
			break;
		}

		// signed half-word multiply/add, single word product sum
		case EHMAC:
		{
			UML_AND(block, I2, I0, 0x0000ffff);
			UML_AND(block, I3, I1, 0x0000ffff);
			UML_MULS(block, I2, I3, I2, I3);
			UML_SHR(block, I0, I0, 16);
			UML_SHR(block, I1, I1, 16);
			UML_MULS(block, I0, I1, I0, I1);
			UML_ADD(block, I0, I0, I2);
			UML_LOAD(block, I1, (void *)m_core->global_regs, 15, SIZE_DWORD, SCALE_x4);
			UML_ADD(block, I0, I0, I1);
			UML_STORE(block, (void *)m_core->global_regs, 15, I0, SIZE_DWORD, SCALE_x4);
			break;
		}

		// signed half-word multiply/add, double word product sum
		case EHMACD:
		{
			printf("Unimplemented extended opcode, EHMACD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;
		}

		// half-word complex multiply
		case EHCMULD:
		{
			printf("Unimplemented extended opcode, EHCMULD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;
		}

		// half-word complex multiply/add
		case EHCMACD:
		{
			printf("Unimplemented extended opcode, EHCMACD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;
		}

		// half-word (complex) add/substract
		// Ls is not used and should denote the same register as Ld
		case EHCSUMD:
		{
			printf("Unimplemented extended opcode, EHCSUMD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;
		}

		// half-word (complex) add/substract with fixed point adjustment
		// Ls is not used and should denote the same register as Ld
		case EHCFFTD:
		{
			printf("Unimplemented extended opcode, EHCFFTD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;
		}

		// half-word (complex) add/substract with fixed point adjustment and shift
		// Ls is not used and should denote the same register as Ld
		case EHCFFTSD:
		{
			printf("Unimplemented extended opcode, EHCFFTSD, PC = %08x\n", desc->pc);
			fatalerror(" ");
			break;
		}

		default:
			printf("Unknown extended opcode (%04x), PC = %08x\n", func, desc->pc);
			fatalerror(" ");
			break;
	}
}


void hyperstone_device::generate_reserved(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	printf("Unimplemented: generate_reserved (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}

void hyperstone_device::generate_do(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	printf("Unimplemented: generate_do (%08x)\n", desc->pc);
	fflush(stdout);
	fatalerror(" ");
}

#endif // MAME_CPU_E132XS_E132XSDRC_OPS_HXX
