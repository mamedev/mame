// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "e132xs.h"

void hyperstone_device::generate_check_delay_pc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	/* if PC is used in a delay instruction, the delayed PC should be used */
	UML_TEST(block, mem(&m_delay_slot), 1);
	UML_MOVc(block, COND_E, mem(m_global_regs), mem(&m_delay_pc));
	UML_MOVc(block, COND_E, mem(&m_delay_slot), 0);
}

void hyperstone_device::generate_decode_const(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
}

int hyperstone_device::generate_decode_immediate_s(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	static int32_t immediate_values[16] =
	{
		16, 0, 0, 0, 32, 64, 128, int32_t(0x80000000),
		-8, -7, -6, -5, -4, -3, -2, -1
	};

	int label = 1;
	int nolut, done, zero_or_one, three;

	UML_AND(block, I1, I0, 0x0f);
	UML_CMP(block, I1, 4);
	UML_JMPc(block, COND_L, nolut = label++);

	// 4..f, immediate lookup
	UML_LOAD(block, I1, (void *)immediate_values, I1, SIZE_DWORD, SCALE_x4);
	UML_JMP(block, done = label++);

	UML_LABEL(block, nolut);
	UML_CMP(block, I1, 2);
	UML_JMPc(block, COND_L, zero_or_one = label++);
	UML_JMPc(block, COND_G, three = label++);

	// 2
	UML_MOV(block, mem(&m_instruction_length), (2<<19));
	UML_READ(block, I1, mem(m_global_regs), SIZE_WORD, SPACE_PROGRAM);
	UML_JMP(block, done);

	// 0..1
	UML_LABEL(block, zero_or_one);
	UML_TEST(block, I1, 1);
	UML_MOVc(block, COND_E, I1, 16); // 0
	UML_JMPc(block, COND_E, done); // if 0, we exit here. if 1, we fall through.

	// 1
	UML_MOV(block, mem(&m_instruction_length), (3<<19));
	UML_READ(block, I1, mem(m_global_regs), SIZE_WORD, SPACE_PROGRAM);
	UML_SHL(block, I1, I1, 16);
	UML_ADD(block, mem(m_global_regs), mem(m_global_regs), 2);
	UML_READ(block, I2, mem(m_global_regs), SIZE_WORD, SPACE_PROGRAM);
	UML_ADD(block, mem(m_global_regs), mem(m_global_regs), 2);
	UML_JMP(block, done);

	// 3
	UML_LABEL(block, three);
	UML_MOV(block, mem(&m_instruction_length), (2<<19));
	UML_READ(block, I1, mem(m_global_regs), SIZE_WORD, SPACE_PROGRAM);
	UML_SEXT(block, I1, I1, SIZE_WORD);
	// fall through to done

	UML_LABEL(block, done);
	return label;
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

int hyperstone_device::generate_set_global_register(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int label)
{
	// Expects register index in I0, value in I1
	return label;
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
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::reg_bank SRC_GLOBAL>
void hyperstone_device::generate_sum(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
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
}


template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
int hyperstone_device::generate_movi(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int label = 1;
	if (IMM_LONG)
	{
		label = generate_decode_immediate_s(block, compiler, desc); // I1 <-- imm32
	}
	else
	{
		UML_AND(block, I1, I0, 0xf);
	}

	int done = label++;
	if (DST_GLOBAL)
	{
		UML_AND(block, I2, mem(&m_global_regs[1]), H_MASK);
		UML_TEST(block, mem(&m_global_regs[1]), S_MASK);
		UML_EXHc(block, uml::COND_Z, *m_exception[EXCEPTION_PRIVILEGE_ERROR], 0);
		UML_JMPc(block, uml::COND_Z, done);
	}

	UML_AND(block, DRC_SR, DRC_SR, ~(Z_MASK | N_MASK));

	int no_z;
	UML_TEST(block, I1, ~0);
	UML_JMPc(block, uml::COND_NZ, no_z = label++);
	UML_OR(block, DRC_SR, DRC_SR, Z_MASK);
	UML_LABEL(block, no_z);

	int no_n;
	UML_TEST(block, I1, 0x80000000);
	UML_JMPc(block, uml::COND_Z, no_n = label++);
	UML_OR(block, DRC_SR, DRC_SR, N_MASK);
	UML_LABEL(block, no_n);

#if MISSIONCRAFT_FLAGS
	UML_AND(block, DRC_SR, DRC_SR, ~V_MASK);
#endif

	UML_ROLAND(block, I0, I0, 30, 0x3c);

	if (DST_GLOBAL)
	{
		int no_pc;
		UML_TEST(block, I0, 0xf0);
		UML_JMPc(block, uml::COND_NZ, no_pc = label++);
		UML_AND(block, mem(m_global_regs), I1, ~1);
		UML_AND(block, DRC_SR, DRC_SR, ~M_MASK);
		generate_delay_slot_and_branch(block, compiler, desc);
		UML_JMP(block, done);

		UML_LABEL(block, no_pc);

		// TODO
		UML_JMP(block, done);
	}
	else
	{
		UML_ROLAND(block, I2, DRC_SR, 7, 0x7f);
		UML_ADD(block, I0, I0, I2);
		UML_AND(block, I0, I0, 0x3f);
		UML_STORE(block, (void *)m_local_regs, I0, I1, SIZE_DWORD, SCALE_x4);
	}

	UML_LABEL(block, done);
	return label;
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

