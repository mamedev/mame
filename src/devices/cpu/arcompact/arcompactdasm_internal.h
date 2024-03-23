// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#ifndef MAME_CPU_ARCOMPACT_ARCOMPACTDASM_INTERNAL_H
#define MAME_CPU_ARCOMPACT_ARCOMPACTDASM_INTERNAL_H

#pragma once

#include "arcompactdasm.h"


class arcompact_disassembler::handle
{
public:
	static int dasm32_B_cc_D_s21(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_B_D_s25(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BL_cc_d_s21(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BL_d_s25(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BREQ_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRNE_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRLT_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRGE_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRLO_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRHS_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BBIT0_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BBIT1_reg_reg(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BREQ_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRNE_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRLT_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRGE_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRLO_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRHS_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BBIT0_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BBIT1_reg_imm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_r_o(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ST_r_o(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);

	// ALU Operations, 0x04, [0x00-0x1F]
	static int dasm32_ADD(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ADC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SUB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SBC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_AND(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_OR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BIC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_XOR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MAX(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MIN(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MOV(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_TST(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_CMP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_RCMP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_RSUB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BSET(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BCLR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BTST(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BXOR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BMSK(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ADD1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ADD2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ADD3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SUB1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SUB2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SUB3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MPY(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MPYH(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MPYHU(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MPYU(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	//
	static int dasm32_Jcc(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_Jcc_D(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_JLcc(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_JLcc_D(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_FLAG(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SR(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ASL_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ASR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LSR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ROR_single(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_RRC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SEXB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SEXW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_EXTB(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_EXTW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ABS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_NOT(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_RLC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_EX(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SLEEP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SWI(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SYNC(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_RTIE(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_BRK(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_0(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_1(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_2(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_3(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_4(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_5(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_6(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LD_7(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ASL_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_LSR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ASR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ROR_multiple(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MUL64(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MULU64(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ADDS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SUBS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_DIVAW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ASLS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ASRS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ADDSDW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SUBSDW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SWAP(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_NORM(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_SAT16(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_RND16(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ABSSW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ABSS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_NEGSW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_NEGS(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_NORMW(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_ARC_EXT06(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_USER_EXT07(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_USER_EXT08(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MARKET_EXT09(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MARKET_EXT0a(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int dasm32_MARKET_EXT0b(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);

	static int dasm_LD_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDB_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDW_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_a_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SUB_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASL_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASR_S_c_b_u3(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_b_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_MOV_S_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_CMP_S_b_h_or_limm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_MOV_S_h_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_J_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_J_S_D_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_JL_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_JL_S_D_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SUB_S_NE_b_b_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_NOP_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_UNIMP_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_JEQ_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_JNE_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_J_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_J_S_D_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SUB_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_AND_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_OR_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BIC_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_XOR_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_TST_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_MUL64_S_0_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SEXB_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SEXW_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_EXTB_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_EXTW_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ABS_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_NOT_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_NEG_S_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD1_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD2_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD3_S_b_b_c(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASL_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LSR_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASR_S_b_b_c_multiple(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASL_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASR_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LSR_S_b_c_single(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_TRAP_S_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BRK_S(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LD_S_c_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDB_S_c_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDW_S_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDW_S_X_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ST_S_c_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_STB_S_c_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_STW_S_c_b_u6(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASL_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LSR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ASR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SUB_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BSET_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BCLR_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BMSK_S_b_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BTST_S_b_u5(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LD_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDB_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ST_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_STB_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_b_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_sp_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_SUB_S_sp_sp_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_POP_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_POP_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_PUSH_S_b(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_PUSH_S_blink(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LD_S_r0_gp_s11(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDB_S_r0_gp_s9(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LDW_S_r0_gp_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_r0_gp_s11(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_LD_S_b_pcl_u10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_MOV_S_b_u8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_ADD_S_b_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_CMP_S_b_u7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BREQ_S_b_0_s8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BRNE_S_b_0_s8(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_B_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BEQ_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BNE_S_s10(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BGT_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BGE_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BLT_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BLE_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BHI_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BHS_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BLO_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BLS_S_s7(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);
	static int dasm_BL_S_s13(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes);

	/************************************************************************************************************************************
	 *                                                                                                                                   *
	 * illegal opcode handlers (disassembly)                                                                                             *
	 *                                                                                                                                   *
	 ************************************************************************************************************************************/

	static int dasm_illegal(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint16_t op, const data_buffer &opcodes);
	static int dasm_illegal(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op, const data_buffer &opcodes);
	static int dasm_illegal(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op, const data_buffer &opcodes);

	static int dasm_illegal(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint32_t op, const data_buffer &opcodes);
	static int dasm_illegal(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op, const data_buffer &opcodes);
	static int dasm_illegal(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op, const data_buffer &opcodes);

	static int dasm_reserved(std::ostream &stream, offs_t pc, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op, const data_buffer &opcodes);
};

#endif // MAME_CPU_ARCOMPACT_ARCOMPACTDASM_INTERNAL_H
