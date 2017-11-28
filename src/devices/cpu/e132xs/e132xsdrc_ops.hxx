// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "e132xs.h"

void hyperstone_device::generate_check_delay_pc(drcuml_block *block)
{
	/* if PC is used in a delay instruction, the delayed PC should be used */
	UML_TEST(block, mem(&m_delay_slot), 1);
	UML_MOVc(block, uml::COND_NZ, DRC_PC, mem(&m_delay_pc));
	UML_MOVc(block, uml::COND_NZ, mem(&m_delay_slot), 0);
}

void hyperstone_device::generate_decode_const(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int half_read;
	UML_READ(block, I1, DRC_PC, SIZE_WORD, SPACE_PROGRAM);
	UML_ADD(block, DRC_PC, DRC_PC, 2);
	UML_TEST(block, I1, 0x8000);
	UML_JMPc(block, uml::COND_Z, half_read = compiler->m_labelnum++);

	UML_MOV(block, mem(&m_instruction_length), (3<<19));

	int skip;
	UML_SHL(block, I1, I1, 16);
	UML_AND(block, I2, I1, 0x3fff0000);
	UML_TEST(block, I1, 0x40000000);
	UML_MOVc(block, uml::COND_Z, I1, I2);
	UML_JMPc(block, uml::COND_Z, skip = compiler->m_labelnum++);
	UML_OR(block, I1, I2, 0xc0000000);
	UML_LABEL(block, skip);
	UML_READ(block, I2, DRC_PC, SIZE_WORD, SPACE_PROGRAM);
	UML_ADD(block, DRC_PC, DRC_PC, 2);
	UML_OR(block, I1, I1, I2);

	int done;
	UML_JMP(block, done = compiler->m_labelnum++);

	UML_LABEL(block, half_read);
	UML_MOV(block, mem(&m_instruction_length), (2<<19));
	UML_TEST(block, I1, 0x4000);
	UML_MOVc(block, uml::COND_NZ, I2, 0xffffc000);
	UML_MOVc(block, uml::COND_Z, I2, 0);
	UML_AND(block, I1, I1, 0x3fff);
	UML_OR(block, I1, I1, I2);

	UML_LABEL(block, done);
}

void hyperstone_device::generate_decode_immediate_s(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	static int32_t immediate_values[16] =
	{
		16, 0, 0, 0, 32, 64, 128, int32_t(0x80000000),
		-8, -7, -6, -5, -4, -3, -2, -1
	};

	int nolut, done, zero_or_one, three;

	UML_AND(block, I1, I0, 0x0f);
	UML_CMP(block, I1, 4);
	UML_JMPc(block, COND_L, nolut = compiler->m_labelnum++);

	// 4..f, immediate lookup
	UML_LOAD(block, I1, (void *)immediate_values, I1, SIZE_DWORD, SCALE_x4);
	UML_JMP(block, done = compiler->m_labelnum++);

	UML_LABEL(block, nolut);
	UML_CMP(block, I1, 2);
	UML_JMPc(block, COND_L, zero_or_one = compiler->m_labelnum++);
	UML_JMPc(block, COND_G, three = compiler->m_labelnum++);

	// 2
	UML_MOV(block, mem(&m_instruction_length), (2<<19));
	UML_READ(block, I1, DRC_PC, SIZE_WORD, SPACE_PROGRAM);
	UML_JMP(block, done);

	// 0..1
	UML_LABEL(block, zero_or_one);
	UML_TEST(block, I1, 1);
	UML_MOVc(block, COND_E, I1, 16); // 0
	UML_JMPc(block, COND_E, done); // if 0, we exit here. if 1, we fall through.

	// 1
	UML_MOV(block, mem(&m_instruction_length), (3<<19));
	UML_READ(block, I1, DRC_PC, SIZE_WORD, SPACE_PROGRAM);
	UML_SHL(block, I1, I1, 16);
	UML_ADD(block, DRC_PC, DRC_PC, 2);
	UML_READ(block, I2, DRC_PC, SIZE_WORD, SPACE_PROGRAM);
	UML_ADD(block, DRC_PC, DRC_PC, 2);
	UML_JMP(block, done);

	// 3
	UML_LABEL(block, three);
	UML_MOV(block, mem(&m_instruction_length), (2<<19));
	UML_READ(block, I1, DRC_PC, SIZE_WORD, SPACE_PROGRAM);
	UML_SEXT(block, I1, I1, SIZE_WORD);
	// fall through to done

	UML_LABEL(block, done);
}

void hyperstone_device::generate_ignore_immediate_s(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_decode_pcrel(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_ignore_pcrel(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_set_global_register(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	// Expects register index in I4, value in I5, clobbers I6
	int extended;
	UML_CMP(block, I4, 16);
	UML_JMPc(block, uml::COND_AE, extended = compiler->m_labelnum++);

	int generic_store, set_sr, done;
	UML_CMP(block, I4, 1);
	UML_JMPc(block, uml::COND_A, generic_store = compiler->m_labelnum++);
	UML_JMPc(block, uml::COND_E, set_sr = compiler->m_labelnum++);
	UML_AND(block, DRC_PC, I5, ~1);
	generate_delay_slot_and_branch(block, compiler, desc);
	UML_JMP(block, done = compiler->m_labelnum++);

	UML_LABEL(block, set_sr);
	UML_ROLINS(block, DRC_SR, I5, 0, 0x0000ffff);
	UML_AND(block, DRC_SR, DRC_SR, ~0x40);
	UML_TEST(block, mem(&m_intblock), ~0);
	UML_MOVc(block, uml::COND_Z, mem(&m_intblock), 1);
	UML_JMP(block, done);

	UML_LABEL(block, generic_store);
	UML_STORE(block, (void *)m_global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_JMP(block, done);

	int above_bcr;
	UML_LABEL(block, extended);
	UML_CMP(block, I4, 17);
	UML_JMPc(block, uml::COND_BE, generic_store);
	UML_CMP(block, I4, BCR_REGISTER);
	UML_JMPc(block, uml::COND_A, above_bcr = compiler->m_labelnum++);
	UML_JMPc(block, uml::COND_E, generic_store);

	// SP or UB
	UML_AND(block, I5, I5, ~3);
	UML_JMP(block, generic_store);

	int set_tpr, set_tcr, set_tr, set_fcr;
	UML_LABEL(block, above_bcr);
	UML_CMP(block, I4, TCR_REGISTER);
	UML_JMPc(block, uml::COND_B, set_tpr = compiler->m_labelnum++);
	UML_JMPc(block, uml::COND_E, set_tcr = compiler->m_labelnum++);
	// Above TCR
	UML_CMP(block, I4, WCR_REGISTER);
	UML_JMPc(block, uml::COND_B, set_tr = compiler->m_labelnum++);
	UML_JMPc(block, uml::COND_E, generic_store); // WCR
	// Above WCR
	UML_CMP(block, I4, FCR_REGISTER);
	UML_JMPc(block, uml::COND_B, done); // ISR - read only
	UML_JMPc(block, uml::COND_E, set_fcr = compiler->m_labelnum++);
	UML_CMP(block, I4, MCR_REGISTER);
	UML_JMPc(block, uml::COND_A, generic_store); // regs 28..31
	// Set MCR
	UML_ROLAND(block, I6, I5, 20, 0x7);
	UML_LOAD(block, I6, (void *)s_trap_entries, I6, SIZE_DWORD, SCALE_x4);
	UML_MOV(block, mem(&m_trap_entry), I6);
	UML_JMP(block, generic_store);

	int skip_compute_tr;
	UML_LABEL(block, set_tpr);
	UML_STORE(block, (void *)m_global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_TEST(block, I5, 0x80000000);
	UML_JMPc(block, uml::COND_NZ, skip_compute_tr = compiler->m_labelnum++);
	UML_CALLC(block, cfunc_compute_tr, this);
	UML_CALLC(block, cfunc_update_timer_prescale, this);
	UML_LABEL(block, skip_compute_tr);
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_JMP(block, done);

	UML_LABEL(block, set_tcr);
	UML_LOAD(block, I6, (void *)m_global_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_CMP(block, I6, I5);
	UML_JMPc(block, uml::COND_E, done);
	UML_STORE(block, (void *)m_global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_CMP(block, mem(&m_intblock), 1);
	UML_MOVc(block, uml::COND_L, mem(&m_intblock), 1);
	UML_JMP(block, done);

	UML_LABEL(block, set_tr);
	UML_STORE(block, (void *)m_global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_MOV(block, mem(&m_tr_base_value), I5);
	UML_CALLC(block, cfunc_total_cycles, this);
	UML_DMOV(block, mem(&m_tr_base_cycles), mem(&m_numcycles));
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_JMP(block, done);

	int skip_adjust_timer;
	UML_LABEL(block, set_fcr);
	UML_LOAD(block, I6, (void *)m_global_regs, I4, SIZE_DWORD, SCALE_x4);
	UML_XOR(block, I6, I6, I5);
	UML_TEST(block, I6, 0x80000000);
	UML_JMPc(block, uml::COND_Z, skip_adjust_timer = compiler->m_labelnum++);
	UML_CALLC(block, cfunc_adjust_timer_interrupt, this);
	UML_LABEL(block, skip_adjust_timer);
	UML_STORE(block, (void *)m_global_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	UML_CMP(block, mem(&m_intblock), 1);
	UML_MOVc(block, uml::COND_L, mem(&m_intblock), 1);
	// Fall through to done

	UML_LABEL(block, done);
}

void hyperstone_device::generate_trap(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t addr)
{
}

void hyperstone_device::generate_int(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t addr)
{
}

void hyperstone_device::generate_exception(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t addr)
{
}

void hyperstone_device::generate_software(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_chk(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_movd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL, hyperstone_device::sign_mode SIGNED>
void hyperstone_device::generate_divsu(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_xm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_mask(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	generate_decode_const(block, compiler, desc);
	generate_check_delay_pc(block);
	UML_AND(block, I2, I0, 0x000f);
	if (!SRC_GLOBAL || !DST_GLOBAL)
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	}

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_global_regs, I2, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I2, I3);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, I1, I2, I1);

	int skip_mask;
	UML_AND(block, DRC_SR, DRC_SR, ~Z_MASK);
	UML_TEST(block, I1, ~0);
	UML_JMPc(block, uml::COND_NZ, skip_mask = compiler->m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, Z_MASK);
	UML_LABEL(block, skip_mask);

	if (DST_GLOBAL)
	{
		UML_ROLAND(block, I4, I0, 28, 0xf);
		UML_MOV(block, I5, I2);
		generate_set_global_register(block, compiler, desc);
	}
	else
	{
		UML_ROLAND(block, I0, I0, 28, 0xf);
		UML_ADD(block, I0, I0, I3);
		UML_AND(block, I0, I0, 0x3f);
		UML_STORE(block, (void *)m_local_regs, I0, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sum(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	generate_decode_const(block, compiler, desc);
	generate_check_delay_pc(block);
	UML_AND(block, I2, I0, 0x000f);
	if (!SRC_GLOBAL || !DST_GLOBAL)
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
	}

	if (SRC_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_global_regs, I2, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ADD(block, I2, I2, I3);
		UML_AND(block, I2, I2, 0x3f);
		UML_LOAD(block, I2, (void *)m_local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_MOV(block, mem(&m_drc_arg0), (uint32_t)'Q');
	UML_MOV(block, mem(&m_drc_arg1), I1);
	UML_CALLC(block, cfunc_print, this);
	UML_MOV(block, mem(&m_drc_arg0), (uint32_t)'R');
	UML_MOV(block, mem(&m_drc_arg1), I2);
	UML_CALLC(block, cfunc_print, this);
	UML_DADD(block, I5, I1, I2);

	UML_AND(block, DRC_SR, DRC_SR, ~(C_MASK | V_MASK | Z_MASK | N_MASK));
	UML_DTEST(block, I5, 0x100000000U);
	UML_SETc(block, uml::COND_NZ, I6);
	UML_ROLINS(block, DRC_SR, I6, 0, C_MASK);

	UML_XOR(block, I1, I5, I2);
	UML_XOR(block, I6, I5, I1);
	UML_AND(block, I1, I1, I6);
	UML_TEST(block, I1, 0x80000000);
	UML_SETc(block, uml::COND_NZ, I6);
	UML_ROLINS(block, DRC_SR, I6, 0, V_MASK);

	UML_TEST(block, I5, ~0);
	UML_SETc(block, uml::COND_Z, I6);
	UML_ROLINS(block, DRC_SR, I6, 0, Z_MASK);

	UML_TEST(block, I5, 0x80000000);
	UML_SETc(block, uml::COND_NZ, I6);
	UML_ROLINS(block, DRC_SR, I6, 0, N_MASK);

	UML_ROLAND(block, I4, I0, 28, 0xf);
	if (DST_GLOBAL)
	{
		generate_set_global_register(block, compiler, desc);
	}
	else
	{
		UML_ADD(block, I0, I4, I3);
		UML_AND(block, I2, I0, 0x3f);
		UML_STORE(block, (void *)m_local_regs, I0, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sums(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_cmp(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_mov(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_add(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_adds(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_cmpb(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_subc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sub(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_subs(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_addc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_neg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_negs(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_and(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_andn(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_or(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_xor(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_not(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_cmpi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_AND(block, I1, I0, 0xf);
	}

	generate_check_delay_pc(block);

	UML_ROLAND(block, I2, I0, 28, 0xf);
	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_global_regs, I2, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I2, I3);
		UML_LOAD(block, I2, (void *)m_local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~(V_MASK | Z_MASK | N_MASK | C_MASK));
	UML_DSUB(block, I0, I2, I1);

	int no_v;
	UML_XOR(block, I0, I0, I2);
	UML_XOR(block, I3, I1, I2);
	UML_AND(block, I0, I0, I3);
	UML_TEST(block, I0, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_v = compiler->m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, V_MASK);
	UML_LABEL(block, no_v);

	int no_n;
	UML_MOV(block, I3, 0);
	UML_CMP(block, I2, I1);
	UML_MOVc(block, uml::COND_E, I3, Z_MASK);
	UML_MOVc(block, uml::COND_B, I3, C_MASK);
	UML_JMPc(block, uml::COND_L, no_n = compiler->m_labelnum++);
	UML_OR(block, I3, I3, N_MASK);
	UML_LABEL(block, no_n);
	UML_OR(block, DRC_SR, DRC_SR, I3);

	UML_CMP(block, I2, I1);

}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_movi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_AND(block, I1, I0, 0xf);
	}

	generate_check_delay_pc(block);

	int done;
	if (DST_GLOBAL)
	{
		UML_AND(block, I2, mem(&m_global_regs[1]), H_MASK);
		UML_TEST(block, mem(&m_global_regs[1]), S_MASK);
		UML_EXHc(block, uml::COND_Z, *m_exception[EXCEPTION_PRIVILEGE_ERROR], 0);
		UML_JMPc(block, uml::COND_Z, done = compiler->m_labelnum++);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~(Z_MASK | N_MASK));

	int no_z;
	UML_TEST(block, I1, ~0);
	UML_JMPc(block, uml::COND_NZ, no_z = compiler->m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, Z_MASK);
	UML_LABEL(block, no_z);

	int no_n;
	UML_TEST(block, I1, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_n = compiler->m_labelnum++);
	UML_OR(block, DRC_SR, DRC_SR, N_MASK);
	UML_LABEL(block, no_n);

#if MISSIONCRAFT_FLAGS
	UML_AND(block, DRC_SR, DRC_SR, ~V_MASK);
#endif

	if (DST_GLOBAL)
	{
		UML_ROLAND(block, I4, I0, 28, 0xf);
		UML_TEST(block, I2, ~0);
		UML_MOVc(block, uml::COND_NZ, I2, 16);
		UML_MOVc(block, uml::COND_Z, I2, 0);
		UML_ADD(block, I4, I4, I2);
		UML_MOV(block, I5, I1);
		generate_set_global_register(block, compiler, desc);

		UML_TEST(block, I0, 0xf0);
		UML_JMPc(block, uml::COND_NZ, done);
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		generate_delay_slot_and_branch(block, compiler, desc);

		UML_LABEL(block, done);
	}
	else
	{
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);
		UML_ADD(block, I0, I0, I2);
		UML_AND(block, I0, I0, 0x3f);
		UML_STORE(block, (void *)m_local_regs, I0, I1, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_addi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_addsi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_cmpbi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_andni(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_ori(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	if (IMM_LONG)
	{
		generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_AND(block, I1, I0, 0xf);
	}

	generate_check_delay_pc(block);

	UML_ROLAND(block, I2, I0, 28, 0xf);
	if (DST_GLOBAL)
	{
		UML_LOAD(block, I2, (void *)m_global_regs, I2, SIZE_DWORD, SCALE_x4);
	}
	else
	{
		UML_ROLAND(block, I3, DRC_SR, 7, 0x7f);
		UML_ADD(block, I2, I2, I3);
		UML_LOAD(block, I2, (void *)m_local_regs, I2, SIZE_DWORD, SCALE_x4);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~Z_MASK);
	UML_OR(block, I5, I2, I1);
	UML_ROLAND(block, I4, I0, 28, 0xf);

	if (DST_GLOBAL)
	{
		generate_set_global_register(block, compiler, desc);
	}
	else
	{
		UML_ROLAND(block, I4, I0, 28, 0xf);
		UML_ADD(block, I4, I4, I2);
		UML_AND(block, I4, I4, 0x3f);
		UML_STORE(block, (void *)m_local_regs, I4, I5, SIZE_DWORD, SCALE_x4);
	}
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::generate_xori(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N>
void hyperstone_device::generate_shrdi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_shrd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_shr(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_shri(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N>
void hyperstone_device::generate_sardi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_sard(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_sar(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_sari(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N>
void hyperstone_device::generate_shldi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_shld(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_shl(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_shli(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_testlz(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_rol(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldxx1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldxx2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stxx1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stxx2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL, hyperstone_device::sign_mode SIGNED>
void hyperstone_device::generate_mulsu(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_mul(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::shift_type HI_N, hyperstone_device::reg_bank DST_GLOBAL>
void hyperstone_device::generate_set(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldwr(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_lddr(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_ldwp(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_lddp(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stwr(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stdr(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stwp(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_stdp(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::branch_condition CONDITION, hyperstone_device::condition_set COND_SET>
void hyperstone_device::generate_b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_br(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


template <hyperstone_device::branch_condition CONDITION, hyperstone_device::condition_set COND_SET>
void hyperstone_device::generate_db(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_dbr(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_frame(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_call_global(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_call_local(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_trap_op(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_extend(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}


void hyperstone_device::generate_reserved(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

void hyperstone_device::generate_do(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

