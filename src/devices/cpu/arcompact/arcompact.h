// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact Core
\*********************************/

#ifndef MAME_CPU_ARCOMPACT_ARCOMPACT_H
#define MAME_CPU_ARCOMPACT_ARCOMPACT_H

#pragma once

#include "arcompact_common.h"


class arcompact_device : public cpu_device, protected arcompact_common
{
public:
	// construction/destruction
	arcompact_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_default_vector_base(uint32_t address) { m_default_vector_base = address & 0xfffffc00; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void arcompact_auxreg_map(address_map &map) ATTR_COLD;


	uint32_t arcompact_auxreg002_LPSTART_r();
	void arcompact_auxreg002_LPSTART_w(uint32_t data);
	uint32_t arcompact_auxreg003_LPEND_r();
	void arcompact_auxreg003_LPEND_w(uint32_t data);

	uint32_t arcompact_auxreg00a_STATUS32_r();

	uint32_t arcompact_auxreg00b_STATUS32_L1_r();
	uint32_t arcompact_auxreg00c_STATUS32_L2_r();
	void arcompact_auxreg00b_STATUS32_L1_w(uint32_t data);
	void arcompact_auxreg00c_STATUS32_L2_w(uint32_t data);

	void arcompact_auxreg012_MULHI_w(uint32_t data);

	uint32_t arcompact_auxreg012_TIMER0_r(offs_t offset);
	uint32_t arcompact_auxreg100_TIMER1_r(offs_t offset);
	void arcompact_auxreg012_TIMER0_w(offs_t offset, uint32_t data);
	void arcompact_auxreg100_TIMER1_w(offs_t offset, uint32_t data);

	uint32_t arcompact_auxreg025_INTVECTORBASE_r();
	void arcompact_auxreg025_INTVECTORBASE_w(uint32_t data);

	uint32_t arcompact_auxreg043_AUX_IRQ_LV12_r();
	void arcompact_auxreg043_AUX_IRQ_LV12_w(uint32_t data);

	uint32_t arcompact_auxreg200_AUX_IRQ_LVL_r();
	void arcompact_auxreg200_AUX_IRQ_LVL_w(uint32_t data);


	const static int REG_BLINK    = 0x1f; // r31
	const static int REG_GP       = 0x1a; // r26
	const static int REG_FP       = 0x1b; // r27
	const static int REG_SP       = 0x1c; // r28
	const static int REG_ILINK1   = 0x1d; // r29
	const static int REG_ILINK2   = 0x1e; // r30
	const static int REG_MLO      = 0x39; // r57 - multiply low 32-bits (of 64-bit result)
	const static int REG_MMID     = 0x3a; // r58 - multiply mid 32-bits (of 64-bit result - overlaps)
	const static int REG_MHI      = 0x3b; // r59 - multiply high 32-bits (of 64-bit result)
	const static int REG_LP_COUNT = 0x3c; // r60
	const static int REG_LIMM     = 0x3e; // r62 - used to indicate long immedaite
	const static int REG_PCL      = 0x3f; // r63

	const static uint32_t E1_FLAG = 0x00000002;
	const static uint32_t E2_FLAG = 0x00000004;

	const static uint32_t V_OVERFLOW_FLAG = 0x00000100;
	const static uint32_t C_CARRY_FLAG = 0x00000200;
	const static uint32_t N_NEGATIVE_FLAG = 0x00000400;
	const static uint32_t Z_ZERO_FLAG = 0x00000800;

	enum
	{
		ARCOMPACT_PC = STATE_GENPC,
		ARCOMPACT_STATUS32 = 0x10,
		ARCOMPACT_LP_START,
		ARCOMPACT_LP_END
	};

	void get_limm_32bit_opcode()
	{
		m_regs[REG_LIMM] = (READ16((m_pc + 4)) << 16);
		m_regs[REG_LIMM] |= READ16((m_pc + 6));
	}

	void get_limm_16bit_opcode()
	{
		m_regs[REG_LIMM] = (READ16((m_pc + 2)) << 16);
		m_regs[REG_LIMM] |= READ16((m_pc + 4));
	}

	int check_limm16(uint8_t hreg)
	{
		if (hreg == REG_LIMM)
		{
			get_limm_16bit_opcode();
			return 6;
		}
		return 2;
	}

	int check_limm(uint8_t reg)
	{
		if (reg == REG_LIMM)
		{
			get_limm_32bit_opcode();
			return 8;
		}
		return 4;
	}

	int check_limm(uint8_t breg, uint8_t creg)
	{
		if ((breg == REG_LIMM) || (creg == REG_LIMM))
		{
			get_limm_32bit_opcode();
			return 8;
		}
		return 4;
	}

	static uint8_t group_0e_get_h(uint16_t op)
	{
		uint8_t h = ((op & 0x0007) << 3);
		h |= ((op & 0x00e0) >> 5);
		return h;
	}

	void status32_set_e1() { m_status32 |= E1_FLAG; }
	void status32_clear_e1() { m_status32 &= ~E1_FLAG; }
	bool status32_check_e1() { return (m_status32 & E1_FLAG ? true : false); }

	void status32_set_e2() { m_status32 |= E2_FLAG; }
	void status32_clear_e2() { m_status32 &= ~E2_FLAG; }
	bool status32_check_e2() { return (m_status32 & E2_FLAG ? true : false); }

	// V = overflow (set if signed operation would overflow)
	void status32_set_v() { m_status32 |= V_OVERFLOW_FLAG; }
	void status32_clear_v() { m_status32 &= ~V_OVERFLOW_FLAG; }
	bool status32_check_v() { return (m_status32 & V_OVERFLOW_FLAG ? true : false); }

	// C = carry (unsigned op, carry set is same condition as LO Lower Than, carry clear is same condition as HS Higher Same)
	void status32_set_c() { m_status32 |=  C_CARRY_FLAG; }
	void status32_clear_c() { m_status32 &= ~C_CARRY_FLAG; }
	bool status32_check_c() { return (m_status32 &   C_CARRY_FLAG ? true : false); }

	// N = negative (set if most significant bit of result is set)
	void status32_set_n()  { m_status32 |=  N_NEGATIVE_FLAG; }
	void status32_clear_n() { m_status32 &= ~N_NEGATIVE_FLAG; }
	bool status32_check_n() { return (m_status32 &   N_NEGATIVE_FLAG ? true : false); }

	// Z = zero (set if result is zero, ie both values the same for CMP)
	void status32_set_z() { m_status32 |=  Z_ZERO_FLAG; }
	void status32_clear_z() { m_status32 &= ~Z_ZERO_FLAG; }
	bool status32_check_z() { return (m_status32 &   Z_ZERO_FLAG ? true : false); }

	// debug is the name of the register, this is not a debug function
	void debugreg_set_ZZ() { m_debug |= (1<<23); }
	void debugreg_clear_ZZ() { m_debug &= ~(1<<23); }
	bool debugreg_check_ZZ() { return (m_debug & (1<<23)) ? true : false; }

	// 0x00 - AL / RA - Always
	static bool condition_AL() { return (true); }
	// 0x01 - EQ / Z - Zero
	bool condition_EQ() { return (status32_check_z()); }
	// 0x02 - NE / NZ - Non-Zero
	bool condition_NE() { return (!status32_check_z()); }
	// 0x03 - PL / P - Positive
	bool condition_PL() { return (!status32_check_n()); }
	// 0x04 - MI / N - Negative
	bool condition_MI() { return (status32_check_n()); }
	// 0x05 - CS / C / LO - Carry Set
	bool condition_CS() { return (status32_check_c()); }
	// 0x06 - CC / NC / HS - Carry Clear
	bool condition_HS() { return (!status32_check_c()); }
	// 0x07 - VS / V - Overflow set
	bool condition_VS() { return (status32_check_v()); }
	// 0x08 - VC / NV - Overflow clear
	bool condition_VC() { return (!status32_check_v()); }
	// 0x09 GT - Greater than (signed)
	bool condition_GT() { return ((status32_check_n() && status32_check_v() && !status32_check_z()) || (!status32_check_n() && !status32_check_v() && !status32_check_z())); }
	// 0x0a - GE - Greater than or equal to (signed)
	bool condition_GE() { return ((status32_check_n() && status32_check_v()) || (!status32_check_n() && !status32_check_v())); }
	// 0x0b - LT - Less than (signed)
	bool condition_LT() { return ((status32_check_n() && !status32_check_v()) || (!status32_check_n() && status32_check_v())); };
	// 0x0c - LE - Less than or equal (signed)
	bool condition_LE() { return ((status32_check_z()) || (status32_check_n() && !status32_check_v()) ||  (!status32_check_n() && status32_check_v())) ; }
	// 0x0d - HI - Higher than (unsigned)
	bool condition_HI() { return ((!status32_check_c()) && (!status32_check_z())); }
	// 0x0e - LS - Lower than or same (unsigned)
	bool condition_LS() { return (status32_check_c() || status32_check_z()); }
	// 0x0f - PNZ - Positive Non Zero
	bool condition_PNZ() { return ((!status32_check_n()) && (!status32_check_z())); }

	void check_interrupts();

	/************************************************************************************************************************************
	*                                                                                                                                   *
	* 32-bit opcode handlers                                                                                                            *
	*                                                                                                                                   *
	************************************************************************************************************************************/

	// arcompact_execute_ops_00to01.cpp
	uint32_t handleop32_B_cc_D_s21(uint32_t op);
	uint32_t handleop32_B_D_s25(uint32_t op);
	uint32_t handleop32_BL_cc_d_s21(uint32_t op);
	uint32_t handleop32_BL_d_s25(uint32_t op);
	uint32_t get_01_01_01_address_offset(uint32_t op);
	uint32_t BRxx_takejump(uint32_t address, uint8_t n, int size, bool link);
	static bool BRxx_condition(uint8_t condition, uint32_t b, uint32_t c);
	uint32_t handleop32_BRxx_reg_reg(uint32_t op, uint8_t condition);
	uint32_t handleop32_BRxx_reg_imm(uint32_t op, uint8_t condition);

	// arcompact_execute_ops_02to03.cpp
	uint32_t handleop32_LD_r_o(uint32_t op);
	uint32_t handleop32_ST_r_o(uint32_t op);

	// arcompact_execute_ops_04.cpp
	static uint32_t handleop32_ADD_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ADC_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SUB_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_AND_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_OR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_BIC_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_XOR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);

	void handleop32_MOV_do_op(uint32_t breg, uint32_t src2, bool set_flags);
	uint32_t handleop32_MOV(uint32_t op);

	static uint32_t handleop32_RSUB_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_BSET_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_BCLR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_BMSK_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ADD1_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ADD2_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ADD3_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SUB1_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SUB2_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SUB3_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SBC_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_MAX_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_MIN_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_BXOR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_MPY_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_MPYH_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_MPYHU_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_MPYU_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);

	static void handleop32_TST_do_op(arcompact_device &o, uint32_t src1, uint32_t src2);
	static void handleop32_CMP_do_op(arcompact_device &o, uint32_t src1, uint32_t src2);
	static void handleop32_RCMP_do_op(arcompact_device &o, uint32_t src1, uint32_t src2);
	static void handleop32_BTST_do_op(arcompact_device &o, uint32_t src1, uint32_t src2);

	void handleop32_FLAG_do_op(uint32_t source);
	uint32_t handleop32_FLAG(uint32_t op);

	// arcompact_execute_ops_04_jumps.cpp
	uint32_t handle_jump_to_addr(bool delay, bool link, uint32_t address, uint32_t next_addr);
	uint32_t handle_jump_to_register(bool delay, bool link, uint32_t reg, uint32_t next_addr, int flag);
	uint32_t handleop32_Jcc_f_a_b_c_helper(uint32_t op, bool delay, bool link);
	uint32_t handleop32_Jcc_cc_f_b_b_c_helper(uint32_t op, bool delay, bool link);
	uint32_t handleop32_J(uint32_t op, bool delay, bool link);

	// arcompact_execute_ops_04_aux.cpp
	uint32_t handleop32_LR(uint32_t op);
	uint32_t handleop32_SR(uint32_t op);

	// arcompact_execute_ops_04_loop.cpp
	uint32_t handleop32_LP(uint32_t op);

	// arcompact_execute_ops_04_2f_sop.cpp
	static uint32_t handleop32_ASR_single_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_ASL_single_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_LSR_single_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_ROR_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_EXTB_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_EXTW_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_RLC_do_op(arcompact_device &o, uint32_t src, bool set_flags);

	static uint32_t handleop32_RRC_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_SEXB_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_SEXW_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_ABS_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_NOT_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	uint32_t handleop32_EX(uint32_t op);

	// arcompact_execute_ops_04_2f_3f_zop.cpp
	uint32_t handleop32_SLEEP(uint32_t op);
	uint32_t handleop32_SWI(uint32_t op);
	uint32_t handleop32_SYNC(uint32_t op);
	uint32_t handleop32_RTIE(uint32_t op);
	uint32_t handleop32_BRK(uint32_t op);

	// arcompact_execute_ops_04_3x.cpp
	uint32_t handleop32_LDrr(uint32_t op, int dsize, int extend);

	// arcompact_execute_ops_05.cpp
	static uint32_t handleop32_ASL_multiple_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_LSR_multiple_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ASR_multiple_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ROR_multiple_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static void handleop32_MUL64_do_op(arcompact_device &o, uint32_t src1, uint32_t src2);
	static void handleop32_MULU64_do_op(arcompact_device &o, uint32_t src1, uint32_t src2);
	static uint32_t handleop32_ADDS_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SUBS_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_DIVAW_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ASLS_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ASRS_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_ADDSDW_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_SUBSDW_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_UNKNOWN_05_0c_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_UNKNOWN_05_10_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);
	static uint32_t handleop32_UNKNOWN_05_14_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags);

	// arcompact_execute_ops_05_2f_sop.cpp
	static uint32_t handleop32_NORM_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_SWAP_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_SAT16_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_RND16_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_ABSSW_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_ABSS_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_NEGSW_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_NEGS_do_op(arcompact_device &o, uint32_t src, bool set_flags);
	static uint32_t handleop32_NORMW_do_op(arcompact_device &o, uint32_t src, bool set_flags);

	// arcompact_execute_ops_06to0b.cpp
	uint32_t handleop32_ARC_EXT06(uint32_t op);
	uint32_t handleop32_USER_EXT07(uint32_t op);
	uint32_t handleop32_USER_EXT08(uint32_t op);
	uint32_t handleop32_MARKET_EXT09(uint32_t op);
	uint32_t handleop32_MARKET_EXT0a(uint32_t op);
	uint32_t handleop32_MARKET_EXT0b(uint32_t op);

	/************************************************************************************************************************************
	*                                                                                                                                   *
	* 16-bit opcode handlers                                                                                                            *
	*                                                                                                                                   *
	************************************************************************************************************************************/

	uint32_t handleop_LD_S_a_b_c(uint16_t op);
	uint32_t handleop_LDB_S_a_b_c(uint16_t op);
	uint32_t handleop_LDW_S_a_b_c(uint16_t op);
	uint32_t handleop_ADD_S_a_b_c(uint16_t op);
	uint32_t handleop_ADD_S_c_b_u3(uint16_t op);
	uint32_t handleop_SUB_S_c_b_u3(uint16_t op);
	uint32_t handleop_ASL_S_c_b_u3(uint16_t op);
	uint32_t handleop_ASR_S_c_b_u3(uint16_t op);
	uint32_t handleop_ADD_S_b_b_h_or_limm(uint16_t op);
	uint32_t handleop_MOV_S_b_h_or_limm(uint16_t op);
	uint32_t handleop_CMP_S_b_h_or_limm(uint16_t op);
	uint32_t handleop_MOV_S_h_b(uint16_t op);
	uint32_t handleop_J_S_b(uint16_t op);
	uint32_t handleop_J_S_D_b(uint16_t op);
	uint32_t handleop_JL_S_b(uint16_t op);
	uint32_t handleop_JL_S_D_b(uint16_t op);
	uint32_t handleop_SUB_S_NE_b_b_b(uint16_t op);
	uint32_t handleop_NOP_S(uint16_t op);
	uint32_t handleop_UNIMP_S(uint16_t op);
	uint32_t handleop_JEQ_S_blink(uint16_t op);
	uint32_t handleop_JNE_S_blink(uint16_t op);
	uint32_t handleop_J_S_blink(uint16_t op);
	uint32_t handleop_J_S_D_blink(uint16_t op);
	uint32_t handleop_SUB_S_b_b_c(uint16_t op);
	uint32_t handleop_AND_S_b_b_c(uint16_t op);
	uint32_t handleop_OR_S_b_b_c(uint16_t op);
	uint32_t handleop_BIC_S_b_b_c(uint16_t op);
	uint32_t handleop_XOR_S_b_b_c(uint16_t op);
	uint32_t handleop_TST_S_b_c(uint16_t op);
	uint32_t handleop_MUL64_S_0_b_c(uint16_t op);
	uint32_t handleop_SEXB_S_b_c(uint16_t op);
	uint32_t handleop_SEXW_S_b_c(uint16_t op);
	uint32_t handleop_EXTB_S_b_c(uint16_t op);
	uint32_t handleop_EXTW_S_b_c(uint16_t op);
	uint32_t handleop_ABS_S_b_c(uint16_t op);
	uint32_t handleop_NOT_S_b_c(uint16_t op);
	uint32_t handleop_NEG_S_b_c(uint16_t op);
	uint32_t handleop_ADD1_S_b_b_c(uint16_t op);
	uint32_t handleop_ADD2_S_b_b_c(uint16_t op);
	uint32_t handleop_ADD3_S_b_b_c(uint16_t op);
	uint32_t handleop_ASL_S_b_b_c_multiple(uint16_t op);
	uint32_t handleop_LSR_S_b_b_c_multiple(uint16_t op);
	uint32_t handleop_ASR_S_b_b_c_multiple(uint16_t op);
	uint32_t handleop_ASL_S_b_c_single(uint16_t op);
	uint32_t handleop_ASR_S_b_c_single(uint16_t op);
	uint32_t handleop_LSR_S_b_c_single(uint16_t op);
	uint32_t handleop_TRAP_S_u6(uint16_t op);
	uint32_t handleop_BRK_S(uint16_t op);
	uint32_t handleop_LD_S_c_b_u7(uint16_t op);
	uint32_t handleop_LDB_S_c_b_u5(uint16_t op);
	uint32_t handleop_LDW_S_c_b_u6(uint16_t op);
	uint32_t handleop_LDW_S_X_c_b_u6(uint16_t op);
	uint32_t handleop_ST_S_c_b_u7(uint16_t op);
	uint32_t handleop_STB_S_c_b_u5(uint16_t op);
	uint32_t handleop_STW_S_c_b_u6(uint16_t op);
	uint32_t handleop_ASL_S_b_b_u5(uint16_t op);
	uint32_t handleop_LSR_S_b_b_u5(uint16_t op);
	uint32_t handleop_ASR_S_b_b_u5(uint16_t op);
	uint32_t handleop_SUB_S_b_b_u5(uint16_t op);
	uint32_t handleop_BSET_S_b_b_u5(uint16_t op);
	uint32_t handleop_BCLR_S_b_b_u5(uint16_t op);
	uint32_t handleop_BMSK_S_b_b_u5(uint16_t op);
	uint32_t handleop_BTST_S_b_u5(uint16_t op);
	uint32_t handleop_LD_S_b_sp_u7(uint16_t op);
	uint32_t handleop_LDB_S_b_sp_u7(uint16_t op);
	uint32_t handleop_ST_S_b_sp_u7(uint16_t op);
	uint32_t handleop_STB_S_b_sp_u7(uint16_t op);
	uint32_t handleop_ADD_S_b_sp_u7(uint16_t op);
	uint32_t handleop_ADD_S_sp_sp_u7(uint16_t op);
	uint32_t handleop_SUB_S_sp_sp_u7(uint16_t op);
	uint32_t handleop_POP_S_b(uint16_t op);
	uint32_t handleop_POP_S_blink(uint16_t op);
	uint32_t handleop_PUSH_S_b(uint16_t op);
	uint32_t handleop_PUSH_S_blink(uint16_t op);
	uint32_t handleop_LD_S_r0_gp_s11(uint16_t op);
	uint32_t handleop_LDB_S_r0_gp_s9(uint16_t op);
	uint32_t handleop_LDW_S_r0_gp_s10(uint16_t op);
	uint32_t handleop_ADD_S_r0_gp_s11(uint16_t op);
	uint32_t handleop_LD_S_b_pcl_u10(uint16_t op);
	uint32_t handleop_MOV_S_b_u8(uint16_t op);
	uint32_t handleop_ADD_S_b_b_u7(uint16_t op);
	uint32_t handleop_CMP_S_b_u7(uint16_t op);
	uint32_t branch_common(uint16_t op, bool cond, unsigned width);
	uint32_t handleop_BREQ_S_b_0_s8(uint16_t op);
	uint32_t handleop_BRNE_S_b_0_s8(uint16_t op);
	uint32_t handleop_B_S_s10(uint16_t op);
	uint32_t handleop_BEQ_S_s10(uint16_t op);
	uint32_t handleop_BNE_S_s10(uint16_t op);
	uint32_t handleop_BGT_S_s7(uint16_t op);
	uint32_t handleop_BGE_S_s7(uint16_t op);
	uint32_t handleop_BLT_S_s7(uint16_t op);
	uint32_t handleop_BLE_S_s7(uint16_t op);
	uint32_t handleop_BHI_S_s7(uint16_t op);
	uint32_t handleop_BHS_S_s7(uint16_t op);
	uint32_t handleop_BLO_S_s7(uint16_t op);
	uint32_t handleop_BLS_S_s7(uint16_t op);
	uint32_t handleop_BL_S_s13(uint16_t op);

	/************************************************************************************************************************************
	*                                                                                                                                   *
	* illegal opcode handlers                                                                                                           *
	*                                                                                                                                   *
	************************************************************************************************************************************/

	uint32_t arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint16_t op);
	uint32_t arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint16_t op);
	uint32_t arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint16_t op);

	uint32_t arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint32_t op);
	uint32_t arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint32_t op);
	uint32_t arcompact_handle_illegal(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op);

	uint32_t arcompact_handle_reserved(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint32_t op);

	/************************************************************************************************************************************
	*                                                                                                                                   *
	* helpers                                                                                                                           *
	*                                                                                                                                   *
	************************************************************************************************************************************/

	uint32_t get_instruction(uint32_t op);

	const address_space_config m_program_config;
	const address_space_config m_io_config;

	address_space *m_program;
	address_space *m_io;

	int m_icount;

	uint32_t m_debugger_temp;

	void unimplemented_opcode(uint16_t op);

	void set_pc(uint32_t pc)
	{
		m_pc = pc; // can be 16-bit aligned
		m_regs[REG_PCL] = m_pc & 0xfffffffc; // always 32-bit aligned
	}

	uint32_t READ32(uint32_t address)
	{
		if (address & 0x3)
			fatalerror("%08x: attempted unaligned READ32 on address %08x", m_pc, address);

		return m_program->read_dword(address);
	}

	void WRITE32(uint32_t address, uint32_t data)
	{
		if (address & 0x3)
			fatalerror("%08x: attempted unaligned WRITE32 on address %08x", m_pc, address);

		m_program->write_dword(address, data);
	}
	uint16_t READ16(uint32_t address)
	{
		if (address & 0x1)
			fatalerror("%08x: attempted unaligned READ16 on address %08x", m_pc, address);

		return m_program->read_word(address);
	}
	void WRITE16(uint32_t address, uint16_t data)
	{
		if (address & 0x1)
			fatalerror("%08x: attempted unaligned WRITE16 on address %08x", m_pc, address);

		m_program->write_word(address, data);
	}
	uint8_t READ8(uint32_t address)
	{
		return m_program->read_byte(address);
	}
	void WRITE8(uint32_t address, uint8_t data)
	{
		m_program->write_byte(address, data);
	}

	uint64_t READAUX(uint64_t address) { return m_io->read_dword(address); }
	void WRITEAUX(uint64_t address, uint32_t data) { m_io->write_dword(address, data); }

	// arcompact_helper.ipp
	bool check_condition(uint8_t condition);
	void do_flags_overflow(uint32_t result, uint32_t b, uint32_t c);
	void do_flags_add(uint32_t result, uint32_t b, uint32_t c);
	void do_flags_sub(uint32_t result, uint32_t b, uint32_t c);
	void do_flags_nz(uint32_t result);
	using ophandler32 = uint32_t (*)(arcompact_device &obj, uint32_t src1, uint32_t src2, bool set_flags);
	using ophandler32_ff = void (*)(arcompact_device &obj, uint32_t src1, uint32_t src2);
	using ophandler32_mul = void (*)(arcompact_device &obj, uint32_t src1, uint32_t src2);
	using ophandler32_sop = uint32_t (*)(arcompact_device &obj, uint32_t src1, bool set_flags);
	uint32_t handleop32_general(uint32_t op, ophandler32 ophandler);
	uint32_t handleop32_general_MULx64(uint32_t op, ophandler32_mul ophandler);
	uint32_t handleop32_general_nowriteback_forced_flag(uint32_t op, ophandler32_ff ophandler);
	uint32_t handleop32_general_SOP_group(uint32_t op, ophandler32_sop ophandler);
	void arcompact_handle_ld_helper(uint32_t op, uint8_t areg, uint8_t breg, uint32_t s, uint8_t X, uint8_t Z, uint8_t a);

	// config
	uint32_t m_default_vector_base;

	// internal state
	uint32_t m_pc;
	uint32_t m_regs[0x40];

	bool m_delayactive;
	bool m_delaylinks;
	uint32_t m_delayjump;
	bool m_allow_loop_check;
	bool m_irq_pending;

//  f  e  d  c| b  a  9  8| 7  6  5  4| 3  2  1  0
//  -  -  -  L| Z  N  C  V| U DE AE A2|A1 E2 E1  H
	uint32_t m_status32;
	uint32_t m_status32_l1;
	uint32_t m_status32_l2;

// 31   30   29   28   27   26   25   24   23   22   21   20   19   18   17   16   15   14   13   12   11   10   09   08   07   06   05   04   03   02   01   00
// LD | SH | BH | UB | xx | xx | xx | xx | ZZ | RA | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | IS | xx | xx | xx | xx | xx | xx | xx | xx | xx | FH | SS
	uint32_t m_debug;

	uint32_t m_timer[2][3];

	uint32_t m_LP_START;
	uint32_t m_LP_END;
	uint32_t m_INTVECTORBASE;
	uint32_t m_AUX_IRQ_LV12;
	uint32_t m_AUX_IRQ_LEV;
};

DECLARE_DEVICE_TYPE(ARCA5, arcompact_device)

#endif // MAME_CPU_ARCOMPACT_ARCOMPACT_H
