// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact Core
\*********************************/

#ifndef MAME_CPU_ARCOMPACT_ARCOMPACT_H
#define MAME_CPU_ARCOMPACT_ARCOMPACT_H

#pragma once

#define ARCOMPACT_LOGGING 1

#define arcompact_fatal if (ARCOMPACT_LOGGING) fatalerror
#define arcompact_log if (ARCOMPACT_LOGGING) fatalerror

class arcompact_device : public cpu_device
{
public:
	// construction/destruction
	arcompact_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
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
	void arcompact_auxreg_map(address_map &map);

	void do_flags(uint32_t result, uint32_t b, uint32_t c);
	void do_flags_nz(uint32_t result);

	uint32_t arcompact_auxreg002_LPSTART_r();
	void arcompact_auxreg002_LPSTART_w(uint32_t data);
	uint32_t arcompact_auxreg003_LPEND_r();
	void arcompact_auxreg003_LPEND_w(uint32_t data);

	uint32_t arcompact_auxreg00a_STATUS32_r();
	uint32_t arcompact_auxreg025_INTVECTORBASE_r();
	void arcompact_auxreg025_INTVECTORBASE_w(uint32_t data);

	const static int LIMM_REG = 62;

	const static int REG_BLINK = 0x1f; // r31
	const static int REG_SP = 0x1c; // r28
	const static int REG_ILINK1 = 0x1d; // r29
	const static int REG_ILINK2 = 0x1e; // r30
	const static int REG_LP_COUNT = 0x3c; // r60

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

	void get_limm_32bit_opcode(void)
	{
		m_regs[LIMM_REG] = (READ16((m_pc + 4)) << 16);
		m_regs[LIMM_REG] |= READ16((m_pc + 6));
	}

	void get_limm_16bit_opcode(void)
	{
		m_regs[LIMM_REG] = (READ16((m_pc + 2)) << 16);
		m_regs[LIMM_REG] |= READ16((m_pc + 4));
	}

	// registers used in 16-bit opcodes have a limited range
	// and can only address registers r0-r3 and r12-r15
	uint8_t expand_reg(uint8_t reg)
	{
		if (reg>3) reg += 8;
		return reg;
	}

	uint8_t common16_get_breg(uint16_t op)
	{
		return ((op & 0x0700) >> 8);
	}

	uint8_t common16_get_creg(uint16_t op)
	{
		return ((op & 0x00e0) >> 5);
	}

	uint8_t common16_get_areg(uint16_t op)
	{
		return ((op & 0x0007) >> 0);
	}

	uint8_t common16_get_u3(uint16_t op)
	{
		return ((op & 0x0007) >> 0);
	}

	uint8_t common16_get_u5(uint16_t op)
	{
		return ((op & 0x001f) >> 0);
	}

	uint8_t common16_get_u8(uint16_t op)
	{
		return ((op & 0x00ff) >>0);
	}

	uint8_t common16_get_u7(uint16_t op)
	{
		return ((op & 0x007f) >>0);
	}

	uint8_t common16_get_s9(uint16_t op)
	{
		uint8_t s = ((op & 0x01ff) >> 0);
		if (s & 0x100)
			s |= 0xfe00;

		return s;
	}

	uint8_t common32_get_breg(uint32_t op)
	{
		int b_temp = (op & 0x07000000) >> 24;
		int B_temp = (op & 0x00007000) >> 12;
		return b_temp | (B_temp << 3);
	}

	uint8_t common32_get_F(uint32_t op)
	{
		return (op & 0x00008000) >> 15;
	}

	uint8_t common32_get_creg(uint32_t op)
	{
		return (op & 0x00000fc0) >> 6;
	}

	uint8_t common32_get_areg(uint32_t op)
	{
		return (op & 0x0000003f) >> 0;
	}

	uint8_t common32_get_p(uint32_t op)
	{
		return (op & 0x00c00000) >> 22;
	}

	uint8_t common32_get_areg_reserved(uint32_t op)
	{
		return (op & 0x0000003f) >> 0;
	}

	uint32_t common32_get_s12(uint32_t op)
	{
		int S_temp = (op & 0x0000003f) >> 0;
		int s_temp = (op & 0x00000fc0) >> 6;
		uint32_t S = s_temp | (S_temp<<6);

		if (S & 0x800) // sign extend
			S |= 0xfffff000;

		return S;
	}

	uint32_t common32_get_u6(uint32_t op)
	{
		return (op & 0x00000fc0) >> 6;
	}

	int check_c_limm(uint8_t creg)
	{
		if (creg == LIMM_REG)
		{
			get_limm_32bit_opcode();
			return 8;
		}
		return 4;
	}

	int check_b_limm(uint8_t breg)
	{
		if (breg == LIMM_REG)
		{
			get_limm_32bit_opcode();
			return 8;
		}
		return 4;
	}

	int check_b_c_limm(uint8_t breg, uint8_t creg)
	{
		if ((breg == LIMM_REG) || (creg == LIMM_REG))
		{
			get_limm_32bit_opcode();
			return 8;
		}
		return 4;
	}

	uint8_t group_0e_get_h(uint16_t op)
	{
		uint8_t h = ((op & 0x0007) << 3);
		h |= ((op & 0x00e0) >> 5);
		return h;
	}

	uint8_t common32_get_condition(uint32_t op)
	{
		return op & 0x0000001f;
	}

	/************************************************************************************************************************************
	*                                                                                                                                   *
	* 32-bit opcode handlers                                                                                                            *
	*                                                                                                                                   *
	************************************************************************************************************************************/

	uint32_t handleop32_B_cc_D_s21(uint32_t op);
	uint32_t handleop32_B_D_s25(uint32_t op);
	uint32_t handleop32_BL_cc_d_s21(uint32_t op);
	uint32_t handleop32_BL_d_s25(uint32_t op);
	uint32_t handleop32_BREQ_reg_reg(uint32_t op);
	uint32_t handleop32_BRNE_reg_reg(uint32_t op);
	uint32_t handleop32_BRLT_reg_reg(uint32_t op);
	uint32_t handleop32_BRGE_reg_reg(uint32_t op);
	uint32_t handleop32_BRLO_reg_reg(uint32_t op);
	uint32_t handleop32_BRHS_reg_reg(uint32_t op);
	uint32_t handleop32_BBIT0_reg_reg(uint32_t op);
	uint32_t handleop32_BBIT1_reg_reg(uint32_t op);
	uint32_t handleop32_BREQ_reg_imm(uint32_t op);
	uint32_t handleop32_BRNE_reg_imm(uint32_t op);
	uint32_t handleop32_BRLT_reg_imm(uint32_t op);
	uint32_t handleop32_BRGE_reg_imm(uint32_t op);
	uint32_t handleop32_BRLO_reg_imm(uint32_t op);
	uint32_t handleop32_BRHS_reg_imm(uint32_t op);
	uint32_t handleop32_BBIT0_reg_imm(uint32_t op);
	uint32_t handleop32_BBIT1_reg_imm(uint32_t op);
	uint32_t handleop32_LD_r_o(uint32_t op);
	uint32_t handleop32_ST_r_o(uint32_t op);

	// arcompact_execute_ops_04.cpp

	uint32_t handleop32_ADD_cc(uint32_t op);
	uint32_t handleop32_ADD(uint32_t op);
	uint32_t handleop32_ADD_f_a_b_c(uint32_t op);
	uint32_t handleop32_ADD_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ADD_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ADD_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ADD_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_SUB_cc(uint32_t op);
	uint32_t handleop32_SUB(uint32_t op);
	uint32_t handleop32_SUB_f_a_b_c(uint32_t op);
	uint32_t handleop32_SUB_f_a_b_u6(uint32_t op);
	uint32_t handleop32_SUB_f_b_b_s12(uint32_t op);
	uint32_t handleop32_SUB_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_SUB_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_AND_cc(uint32_t op);
	uint32_t handleop32_AND(uint32_t op);
	uint32_t handleop32_AND_f_a_b_c(uint32_t op);
	uint32_t handleop32_AND_f_a_b_u6(uint32_t op);
	uint32_t handleop32_AND_f_b_b_s12(uint32_t op);
	uint32_t handleop32_AND_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_AND_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_OR_cc(uint32_t op);
	uint32_t handleop32_OR(uint32_t op);
	uint32_t handleop32_OR_f_a_b_c(uint32_t op);
	uint32_t handleop32_OR_f_a_b_u6(uint32_t op);
	uint32_t handleop32_OR_f_b_b_s12(uint32_t op);
	uint32_t handleop32_OR_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_OR_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_BIC_cc(uint32_t op);
	uint32_t handleop32_BIC(uint32_t op);
	uint32_t handleop32_BIC_f_a_b_c(uint32_t op);
	uint32_t handleop32_BIC_f_a_b_u6(uint32_t op);
	uint32_t handleop32_BIC_f_b_b_s12(uint32_t op);
	uint32_t handleop32_BIC_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_BIC_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_XOR_cc(uint32_t op);
	uint32_t handleop32_XOR(uint32_t op);
	uint32_t handleop32_XOR_f_a_b_c(uint32_t op);
	uint32_t handleop32_XOR_f_a_b_u6(uint32_t op);
	uint32_t handleop32_XOR_f_b_b_s12(uint32_t op);
	uint32_t handleop32_XOR_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_XOR_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_MOV_cc(uint32_t op);
	uint32_t handleop32_MOV(uint32_t op);
	uint32_t handleop32_MOV_f_a_b_c(uint32_t op);
	uint32_t handleop32_MOV_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MOV_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MOV_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MOV_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_RSUB_cc(uint32_t op);
	uint32_t handleop32_RSUB(uint32_t op);
	uint32_t handleop32_RSUB_f_a_b_c(uint32_t op);
	uint32_t handleop32_RSUB_f_a_b_u6(uint32_t op);
	uint32_t handleop32_RSUB_f_b_b_s12(uint32_t op);
	uint32_t handleop32_RSUB_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_RSUB_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_BSET_cc(uint32_t op);
	uint32_t handleop32_BSET(uint32_t op);
	uint32_t handleop32_BSET_f_a_b_c(uint32_t op);
	uint32_t handleop32_BSET_f_a_b_u6(uint32_t op);
	uint32_t handleop32_BSET_f_b_b_s12(uint32_t op);
	uint32_t handleop32_BSET_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_BSET_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_BMSK_cc(uint32_t op);
	uint32_t handleop32_BMSK(uint32_t op);
	uint32_t handleop32_BMSK_f_a_b_c(uint32_t op);
	uint32_t handleop32_BMSK_f_a_b_u6(uint32_t op);
	uint32_t handleop32_BMSK_f_b_b_s12(uint32_t op);
	uint32_t handleop32_BMSK_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_BMSK_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_ADD1_cc(uint32_t op);
	uint32_t handleop32_ADD1(uint32_t op);
	uint32_t handleop32_ADD1_f_a_b_c(uint32_t op);
	uint32_t handleop32_ADD1_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ADD1_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ADD1_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ADD1_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_ADD2_cc(uint32_t op);
	uint32_t handleop32_ADD2(uint32_t op);
	uint32_t handleop32_ADD2_f_a_b_c(uint32_t op);
	uint32_t handleop32_ADD2_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ADD2_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ADD2_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ADD2_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_ADD3_cc(uint32_t op);
	uint32_t handleop32_ADD3(uint32_t op);
	uint32_t handleop32_ADD3_f_a_b_c(uint32_t op);
	uint32_t handleop32_ADD3_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ADD3_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ADD3_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ADD3_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_SUB1_cc(uint32_t op);
	uint32_t handleop32_SUB1(uint32_t op);
	uint32_t handleop32_SUB1_f_a_b_c(uint32_t op);
	uint32_t handleop32_SUB1_f_a_b_u6(uint32_t op);
	uint32_t handleop32_SUB1_f_b_b_s12(uint32_t op);
	uint32_t handleop32_SUB1_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_SUB1_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_SUB2_cc(uint32_t op);
	uint32_t handleop32_SUB2(uint32_t op);
	uint32_t handleop32_SUB2_f_a_b_c(uint32_t op);
	uint32_t handleop32_SUB2_f_a_b_u6(uint32_t op);
	uint32_t handleop32_SUB2_f_b_b_s12(uint32_t op);
	uint32_t handleop32_SUB2_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_SUB2_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_SUB3_cc(uint32_t op);
	uint32_t handleop32_SUB3(uint32_t op);
	uint32_t handleop32_SUB3_f_a_b_c(uint32_t op);
	uint32_t handleop32_SUB3_f_a_b_u6(uint32_t op);
	uint32_t handleop32_SUB3_f_b_b_s12(uint32_t op);
	uint32_t handleop32_SUB3_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_SUB3_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_Jcc_cc(uint32_t op);
	uint32_t handleop32_Jcc(uint32_t op);
	uint32_t handleop32_Jcc_f_a_b_c(uint32_t op);
	uint32_t handleop32_Jcc_f_a_b_u6(uint32_t op);
	uint32_t handleop32_Jcc_f_b_b_s12(uint32_t op);
	uint32_t handleop32_Jcc_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_Jcc_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_Jcc_D_cc(uint32_t op);
	uint32_t handleop32_Jcc_D(uint32_t op);
	uint32_t handleop32_Jcc_D_f_a_b_c(uint32_t op);
	uint32_t handleop32_Jcc_D_f_a_b_u6(uint32_t op);
	uint32_t handleop32_Jcc_D_f_b_b_s12(uint32_t op);
	uint32_t handleop32_Jcc_D_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_Jcc_D_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_LR_cc(uint32_t op);
	uint32_t handleop32_LR(uint32_t op);
	uint32_t handleop32_LR_f_a_b_c(uint32_t op);
	uint32_t handleop32_LR_f_a_b_u6(uint32_t op);
	uint32_t handleop32_LR_f_b_b_s12(uint32_t op);
	uint32_t handleop32_LR_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_LR_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_SR_cc(uint32_t op);
	uint32_t handleop32_SR(uint32_t op);
	uint32_t handleop32_SR_f_a_b_c(uint32_t op);
	uint32_t handleop32_SR_f_a_b_u6(uint32_t op);
	uint32_t handleop32_SR_f_b_b_s12(uint32_t op);
	uint32_t handleop32_SR_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_SR_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_ADC_f_a_b_c(uint32_t op);
	uint32_t handleop32_ADC_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ADC_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ADC_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ADC_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_ADC_cc(uint32_t op);
	uint32_t handleop32_ADC(uint32_t op);

	uint32_t handleop32_SBC_f_a_b_c(uint32_t op);
	uint32_t handleop32_SBC_f_a_b_u6(uint32_t op);
	uint32_t handleop32_SBC_f_b_b_s12(uint32_t op);
	uint32_t handleop32_SBC_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_SBC_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_SBC_cc(uint32_t op);
	uint32_t handleop32_SBC(uint32_t op);

	uint32_t handleop32_MAX_f_a_b_c(uint32_t op);
	uint32_t handleop32_MAX_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MAX_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MAX_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MAX_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_MAX_cc(uint32_t op);
	uint32_t handleop32_MAX(uint32_t op);

	uint32_t handleop32_MIN_f_a_b_c(uint32_t op);
	uint32_t handleop32_MIN_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MIN_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MIN_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MIN_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_MIN_cc(uint32_t op);
	uint32_t handleop32_MIN(uint32_t op);

	uint32_t handleop32_TST_f_a_b_c(uint32_t op);
	uint32_t handleop32_TST_f_a_b_u6(uint32_t op);
	uint32_t handleop32_TST_f_b_b_s12(uint32_t op);
	uint32_t handleop32_TST_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_TST_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_TST_cc(uint32_t op);
	uint32_t handleop32_TST(uint32_t op);

	uint32_t handleop32_CMP_f_a_b_c(uint32_t op);
	uint32_t handleop32_CMP_f_a_b_u6(uint32_t op);
	uint32_t handleop32_CMP_f_b_b_s12(uint32_t op);
	uint32_t handleop32_CMP_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_CMP_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_CMP_cc(uint32_t op);
	uint32_t handleop32_CMP(uint32_t op);

	uint32_t handleop32_RCMP_f_a_b_c(uint32_t op);
	uint32_t handleop32_RCMP_f_a_b_u6(uint32_t op);
	uint32_t handleop32_RCMP_f_b_b_s12(uint32_t op);
	uint32_t handleop32_RCMP_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_RCMP_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_RCMP_cc(uint32_t op);
	uint32_t handleop32_RCMP(uint32_t op);

	uint32_t handleop32_BCLR_f_a_b_c(uint32_t op);
	uint32_t handleop32_BCLR_f_a_b_u6(uint32_t op);
	uint32_t handleop32_BCLR_f_b_b_s12(uint32_t op);
	uint32_t handleop32_BCLR_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_BCLR_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_BCLR_cc(uint32_t op);
	uint32_t handleop32_BCLR(uint32_t op);

	uint32_t handleop32_BTST_f_a_b_c(uint32_t op);
	uint32_t handleop32_BTST_f_a_b_u6(uint32_t op);
	uint32_t handleop32_BTST_f_b_b_s12(uint32_t op);
	uint32_t handleop32_BTST_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_BTST_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_BTST_cc(uint32_t op);
	uint32_t handleop32_BTST(uint32_t op);

	uint32_t handleop32_BXOR_f_a_b_c(uint32_t op);
	uint32_t handleop32_BXOR_f_a_b_u6(uint32_t op);
	uint32_t handleop32_BXOR_f_b_b_s12(uint32_t op);
	uint32_t handleop32_BXOR_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_BXOR_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_BXOR_cc(uint32_t op);
	uint32_t handleop32_BXOR(uint32_t op);

	uint32_t handleop32_MPY_f_a_b_c(uint32_t op);
	uint32_t handleop32_MPY_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MPY_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MPY_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MPY_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_MPY_cc(uint32_t op);
	uint32_t handleop32_MPY(uint32_t op);

	uint32_t handleop32_MPYH_f_a_b_c(uint32_t op);
	uint32_t handleop32_MPYH_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MPYH_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MPYH_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MPYH_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_MPYH_cc(uint32_t op);
	uint32_t handleop32_MPYH(uint32_t op);

	uint32_t handleop32_MPYHU_f_a_b_c(uint32_t op);
	uint32_t handleop32_MPYHU_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MPYHU_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MPYHU_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MPYHU_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_MPYHU_cc(uint32_t op);
	uint32_t handleop32_MPYHU(uint32_t op);

	uint32_t handleop32_MPYU_f_a_b_c(uint32_t op);
	uint32_t handleop32_MPYU_f_a_b_u6(uint32_t op);
	uint32_t handleop32_MPYU_f_b_b_s12(uint32_t op);
	uint32_t handleop32_MPYU_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_MPYU_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_MPYU_cc(uint32_t op);
	uint32_t handleop32_MPYU(uint32_t op);

	uint32_t handleop32_JLcc_f_a_b_c(uint32_t op);
	uint32_t handleop32_JLcc_f_a_b_u6(uint32_t op);
	uint32_t handleop32_JLcc_f_b_b_s12(uint32_t op);
	uint32_t handleop32_JLcc_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_JLcc_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_JLcc_cc(uint32_t op);
	uint32_t handleop32_JLcc(uint32_t op);

	uint32_t handleop32_JLcc_D_f_a_b_c(uint32_t op);
	uint32_t handleop32_JLcc_D_f_a_b_u6(uint32_t op);
	uint32_t handleop32_JLcc_D_f_b_b_s12(uint32_t op);
	uint32_t handleop32_JLcc_D_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_JLcc_D_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_JLcc_D_cc(uint32_t op);
	uint32_t handleop32_JLcc_D(uint32_t op);

	uint32_t handleop32_LP(uint32_t op);

	uint32_t handleop32_FLAG_f_a_b_c(uint32_t op);
	uint32_t handleop32_FLAG_f_a_b_u6(uint32_t op);
	uint32_t handleop32_FLAG_f_b_b_s12(uint32_t op);
	uint32_t handleop32_FLAG_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_FLAG_cc_f_b_b_u6(uint32_t op);
	uint32_t handleop32_FLAG_cc(uint32_t op);
	uint32_t handleop32_FLAG(uint32_t op);

	// arcompact_execute_ops_04_2f_sop.cpp

	uint32_t handleop32_ASL_single(uint32_t op);
	uint32_t handleop32_ASR_single(uint32_t op);

	uint32_t handleop32_LSR_single(uint32_t op);
	uint32_t handleop32_LSR_single_f_b_c(uint32_t op);
	uint32_t handleop32_LSR_single_f_b_u6(uint32_t op);

	uint32_t handleop32_ROR_single_cc(uint32_t op);
	uint32_t handleop32_ROR_single(uint32_t op);
	uint32_t handleop32_ROR_single_f_a_b_c(uint32_t op);
	uint32_t handleop32_ROR_single_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ROR_single_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ROR_single_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ROR_single_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_EXTB_cc(uint32_t op);
	uint32_t handleop32_EXTB(uint32_t op);
	uint32_t handleop32_EXTB_f_a_b_c(uint32_t op);
	uint32_t handleop32_EXTB_f_a_b_u6(uint32_t op);
	uint32_t handleop32_EXTB_f_b_b_s12(uint32_t op);
	uint32_t handleop32_EXTB_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_EXTB_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_EXTW_cc(uint32_t op);
	uint32_t handleop32_EXTW(uint32_t op);
	uint32_t handleop32_EXTW_f_a_b_c(uint32_t op);
	uint32_t handleop32_EXTW_f_a_b_u6(uint32_t op);
	uint32_t handleop32_EXTW_f_b_b_s12(uint32_t op);
	uint32_t handleop32_EXTW_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_EXTW_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_RRC(uint32_t op);
	uint32_t handleop32_SEXB(uint32_t op);
	uint32_t handleop32_SEXW(uint32_t op);
	uint32_t handleop32_ABS(uint32_t op);
	uint32_t handleop32_NOT(uint32_t op);
	uint32_t handleop32_RLC(uint32_t op);
	uint32_t handleop32_EX(uint32_t op);

	// arcompact_execute_ops_04_2f_3f_zop.cpp

	uint32_t handleop32_SLEEP(uint32_t op);
	uint32_t handleop32_SWI(uint32_t op);
	uint32_t handleop32_SYNC(uint32_t op);
	uint32_t handleop32_RTIE(uint32_t op);
	uint32_t handleop32_BRK(uint32_t op);




	uint32_t handleop32_LD_0(uint32_t op);
	uint32_t handleop32_LD_1(uint32_t op);
	uint32_t handleop32_LD_2(uint32_t op);
	uint32_t handleop32_LD_3(uint32_t op);
	uint32_t handleop32_LD_4(uint32_t op);
	uint32_t handleop32_LD_5(uint32_t op);
	uint32_t handleop32_LD_6(uint32_t op);
	uint32_t handleop32_LD_7(uint32_t op);



	uint32_t handleop32_ASR_multiple(uint32_t op);
	uint32_t handleop32_ROR_multiple(uint32_t op);
	uint32_t handleop32_MUL64(uint32_t op);
	uint32_t handleop32_MULU64(uint32_t op);
	uint32_t handleop32_ADDS(uint32_t op);
	uint32_t handleop32_SUBS(uint32_t op);
	uint32_t handleop32_DIVAW(uint32_t op);
	uint32_t handleop32_ASLS(uint32_t op);
	uint32_t handleop32_ASRS(uint32_t op);
	uint32_t handleop32_ADDSDW(uint32_t op);
	uint32_t handleop32_SUBSDW(uint32_t op);
	uint32_t handleop32_SWAP(uint32_t op);
	uint32_t handleop32_NORM(uint32_t op);
	uint32_t handleop32_SAT16(uint32_t op);
	uint32_t handleop32_RND16(uint32_t op);
	uint32_t handleop32_ABSSW(uint32_t op);
	uint32_t handleop32_ABSS(uint32_t op);
	uint32_t handleop32_NEGSW(uint32_t op);
	uint32_t handleop32_NEGS(uint32_t op);
	uint32_t handleop32_NORMW(uint32_t op);

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
	uint32_t handleop_MOV_S_hob(uint16_t op);
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



	uint32_t handleop32_ASL_multiple_cc(uint32_t op);
	uint32_t handleop32_ASL_multiple(uint32_t op);
	uint32_t handleop32_ASL_multiple_f_a_b_c(uint32_t op);
	uint32_t handleop32_ASL_multiple_f_a_b_u6(uint32_t op);
	uint32_t handleop32_ASL_multiple_f_b_b_s12(uint32_t op);
	uint32_t handleop32_ASL_multiple_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_ASL_multiple_cc_f_b_b_u6(uint32_t op);

	uint32_t handleop32_LSR_multiple_cc(uint32_t op);
	uint32_t handleop32_LSR_multiple(uint32_t op);
	uint32_t handleop32_LSR_multiple_f_a_b_c(uint32_t op);
	uint32_t handleop32_LSR_multiple_f_a_b_u6(uint32_t op);
	uint32_t handleop32_LSR_multiple_f_b_b_s12(uint32_t op);
	uint32_t handleop32_LSR_multiple_cc_f_b_b_c(uint32_t op);
	uint32_t handleop32_LSR_multiple_cc_f_b_b_u6(uint32_t op);

	uint32_t arcompact_01_01_00_helper(uint32_t op, const char* optext);
	uint32_t arcompact_01_01_01_helper(uint32_t op, const char* optext);
	uint32_t arcompact_handle04_helper(uint32_t op, const char* optext, int ignore_dst, int b_reserved);
	uint32_t arcompact_handle04_2f_helper(uint32_t op, const char* optext);
	uint32_t arcompact_handle04_3x_helper(uint32_t op, int dsize, int extend);
	uint32_t arcompact_handle05_2f_0x_helper(uint32_t op, const char* optext);
	uint32_t arcompact_handle0c_helper(uint16_t op, const char* optext);
	uint32_t arcompact_handle0d_helper(uint16_t op, const char* optext);
	uint32_t arcompact_handle0e_0x_helper(uint16_t op, const char* optext, int revop);
	uint32_t arcompact_handle0f_00_0x_helper(uint16_t op, const char* optext);
	uint32_t arcompact_handle0f_0x_helper(uint16_t op, const char* optext, int nodst);
	uint32_t arcompact_handle_ld_helper(uint16_t op, const char* optext, int shift, int swap);
	uint32_t arcompact_handle_l7_0x_helper(uint16_t op, const char* optext);
	uint32_t arcompact_handle18_0x_helper(uint16_t op, const char* optext, int st);
	uint32_t arcompact_handle19_0x_helper(uint16_t op, const char* optext, int shift, int format);
	uint32_t arcompact_handle1e_0x_helper(uint16_t op, const char* optext);
	uint32_t arcompact_handle1e_03_0x_helper(uint16_t op, const char* optext);


	uint32_t handle_jump_to_addr(int delay, int link, uint32_t address, uint32_t next_addr);
	uint32_t handle_jump_to_register(int delay, int link, uint32_t reg, uint32_t next_addr, int flag);

	uint32_t get_instruction(uint32_t op);

	const address_space_config m_program_config;
	const address_space_config m_io_config;

	uint32_t m_pc;

	address_space *m_program;
	address_space  *m_io;

	int m_icount;

	uint32_t m_debugger_temp;

	void unimplemented_opcode(uint16_t op);

	inline  uint32_t READ32(uint32_t address)
	{
		if (address & 0x3)
			fatalerror("%08x: attempted unaligned READ32 on address %08x", pc, address);

		return m_program->read_dword(address);
	}

	inline void WRITE32(uint32_t address, uint32_t data)
	{
		if (address & 0x3)
			fatalerror("%08x: attempted unaligned WRITE32 on address %08x", pc, address);

		m_program->write_dword(address, data);
	}
	inline uint16_t READ16(uint32_t address)
	{
		if (address & 0x1)
			fatalerror("%08x: attempted unaligned READ16 on address %08x", pc, address);

		return m_program->read_word(address);
	}
	inline void WRITE16(uint32_t address, uint16_t data)
	{
		if (address & 0x1)
			fatalerror("%08x: attempted unaligned WRITE16 on address %08x", pc, address);

		m_program->write_word(address, data);
	}
	inline uint8_t READ8(uint32_t address)
	{
		return m_program->read_byte(address);
	}
	inline void WRITE8(uint32_t address, uint8_t data)
	{
		m_program->write_byte(address, data);
	}

	inline uint64_t READAUX(uint64_t address) { return m_io->read_dword(address); }
	inline void WRITEAUX(uint64_t address, uint32_t data) { m_io->write_dword(address, data); }


	int check_condition(uint8_t condition);

	uint32_t m_regs[0x40];

	int m_delayactive;
	int m_delaylinks;
	uint32_t m_delayjump;

//  f  e  d  c| b  a  9  8| 7  6  5  4| 3  2  1  0
//  -  -  -  L| Z  N  C  V| U DE AE A2|A1 E2 E1  H
	uint32_t m_status32;

	uint32_t m_LP_START;
	uint32_t m_LP_END;
	uint32_t m_INTVECTORBASE;

};


// V = overflow (set if signed operation would overflow)
#define STATUS32_SET_V   (m_status32 |=  V_OVERFLOW_FLAG)
#define STATUS32_CLEAR_V (m_status32 &= ~V_OVERFLOW_FLAG)
#define STATUS32_CHECK_V (m_status32 &   V_OVERFLOW_FLAG)

// C = carry (unsigned op, carry set is same condition as LO Lower Than, carry clear is same condition as HS Higher Same)
#define STATUS32_SET_C   (m_status32 |=  C_CARRY_FLAG)
#define STATUS32_CLEAR_C (m_status32 &= ~C_CARRY_FLAG)
#define STATUS32_CHECK_C (m_status32 &   C_CARRY_FLAG)

// N = negative (set if most significant bit of result is set)
#define STATUS32_SET_N   (m_status32 |=  N_NEGATIVE_FLAG)
#define STATUS32_CLEAR_N (m_status32 &= ~N_NEGATIVE_FLAG)
#define STATUS32_CHECK_N (m_status32 &   N_NEGATIVE_FLAG)

// Z = zero (set if result is zero, ie both values the same for CMP)
#define STATUS32_SET_Z   (m_status32 |=  Z_ZERO_FLAG)
#define STATUS32_CLEAR_Z (m_status32 &= ~Z_ZERO_FLAG)
#define STATUS32_CHECK_Z (m_status32 &   Z_ZERO_FLAG)


// 0x00 - AL / RA - Always
#define CONDITION_AL (1)
// 0x01 - EQ / Z - Zero
#define CONDITION_EQ (STATUS32_CHECK_Z)
// 0x02 - NE / NZ - Non-Zero
#define CONDITION_NE (!STATUS32_CHECK_Z)
// 0x03 - PL / P - Positive
#define CONDITION_PL (!STATUS32_CHECK_N)
// 0x04 - MI / N - Negative
#define CONDITION_MI (STATUS32_CHECK_N)
// 0x05 - CS / C / LO - Carry Set
#define CONDITION_CS (STATUS32_CHECK_C)
// 0x06 - CC / NC / HS - Carry Clear
#define CONDITION_HS (!STATUS32_CHECK_C)
// 0x07 - VS / V - Overflow set
#define CONDITION_VS (STATUS32_CHECK_V)
// 0x08 - VC / NV - Overflow clear
#define CONDITION_VC (!STATUS32_CHECK_V)
// 0x09 GT - Greater than (signed)
#define CONDITION_GT ((STATUS32_CHECK_N && STATUS32_CHECK_V && !STATUS32_CHECK_Z) || (!STATUS32_CHECK_N && !STATUS32_CHECK_V && !STATUS32_CHECK_Z))
// 0x0a - GE - Greater than or equal to (signed)
#define CONDITION_GE ((STATUS32_CHECK_N && STATUS32_CHECK_V) || (!STATUS32_CHECK_N && !STATUS32_CHECK_V))
// 0x0b - LT - Less than (signed)
#define CONDITION_LT ((STATUS32_CHECK_N && !STATUS32_CHECK_V) || (!STATUS32_CHECK_N && STATUS32_CHECK_V))
// 0x0c - LE - Less than or equal (signed)
#define CONDITION_LE ((STATUS32_CHECK_Z) || (STATUS32_CHECK_N && !STATUS32_CHECK_V) ||  (!STATUS32_CHECK_N && STATUS32_CHECK_V)) // Z or (N and /V) or (/N and V)
// 0x0d - HI - Higher than (unsigned)
#define CONDITION_HI ((!STATUS32_CHECK_C) && (!STATUS32_CHECK_Z))
// 0x0e - LS - Lower than or same (unsigned)
#define CONDITION_LS (STATUS32_CHECK_C || STATUS32_CHECK_Z)
// 0x0f - PNZ - Positive Non Zero
#define CONDITION_PNZ ((!STATUS32_CHECK_N) && (!STATUS32_CHECK_Z))



DECLARE_DEVICE_TYPE(ARCA5, arcompact_device)

#endif // MAME_CPU_ARCOMPACT_ARCOMPACT_H
