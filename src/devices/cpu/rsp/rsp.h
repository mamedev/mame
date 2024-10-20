// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
/***************************************************************************

    rsp.h

    Interface file for the universal machine language-based
    Reality Signal Processor (RSP) emulator.

***************************************************************************/

#ifndef MAME_CPU_RSP_RSP_H
#define MAME_CPU_RSP_RSP_H

#pragma once

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	RSP_PC = 1,
	RSP_R0,
	RSP_R1,
	RSP_R2,
	RSP_R3,
	RSP_R4,
	RSP_R5,
	RSP_R6,
	RSP_R7,
	RSP_R8,
	RSP_R9,
	RSP_R10,
	RSP_R11,
	RSP_R12,
	RSP_R13,
	RSP_R14,
	RSP_R15,
	RSP_R16,
	RSP_R17,
	RSP_R18,
	RSP_R19,
	RSP_R20,
	RSP_R21,
	RSP_R22,
	RSP_R23,
	RSP_R24,
	RSP_R25,
	RSP_R26,
	RSP_R27,
	RSP_R28,
	RSP_R29,
	RSP_R30,
	RSP_R31,
	RSP_SR,
	RSP_NEXTPC,
	RSP_STEPCNT,
	RSP_V0,  RSP_V1,  RSP_V2,  RSP_V3,  RSP_V4,  RSP_V5,  RSP_V6,  RSP_V7,
	RSP_V8,  RSP_V9,  RSP_V10, RSP_V11, RSP_V12, RSP_V13, RSP_V14, RSP_V15,
	RSP_V16, RSP_V17, RSP_V18, RSP_V19, RSP_V20, RSP_V21, RSP_V22, RSP_V23,
	RSP_V24, RSP_V25, RSP_V26, RSP_V27, RSP_V28, RSP_V29, RSP_V30, RSP_V31
};

#define RSP_STATUS_HALT          0x0001
#define RSP_STATUS_BROKE         0x0002
#define RSP_STATUS_DMABUSY       0x0004
#define RSP_STATUS_DMAFULL       0x0008
#define RSP_STATUS_IOFULL        0x0010
#define RSP_STATUS_SSTEP         0x0020
#define RSP_STATUS_INTR_BREAK    0x0040
#define RSP_STATUS_SIGNAL0       0x0080
#define RSP_STATUS_SIGNAL1       0x0100
#define RSP_STATUS_SIGNAL2       0x0200
#define RSP_STATUS_SIGNAL3       0x0400
#define RSP_STATUS_SIGNAL4       0x0800
#define RSP_STATUS_SIGNAL5       0x1000
#define RSP_STATUS_SIGNAL6       0x2000
#define RSP_STATUS_SIGNAL7       0x4000

class rsp_device : public cpu_device
{
	class cop2;

public:
	// construction/destruction
	rsp_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);
	virtual ~rsp_device() override;

	auto dp_reg_r() { return m_dp_reg_r_func.bind(); }
	auto dp_reg_w() { return m_dp_reg_w_func.bind(); }
	auto sp_reg_r() { return m_sp_reg_r_func.bind(); }
	auto sp_reg_w() { return m_sp_reg_w_func.bind(); }
	auto status_set() { return m_sp_set_status_func.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override { }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void unimplemented_opcode(uint32_t op);

private:
	address_space_config m_imem_config;
	address_space_config m_dmem_config;

	uint16_t m_pc;
	uint32_t m_r[35];
	int m_icount;
	int m_ideduct;
	bool m_scalar_busy;
	bool m_vector_busy;
	bool m_paired_busy;

	void update_scalar_op_deduction();
	void update_vector_op_deduction();

	FILE *m_exec_output;

	uint32_t m_sr;
	uint32_t m_step_count;

	uint16_t m_ppc;
	uint16_t m_nextpc;

protected:
	memory_access<12, 2, 0, ENDIANNESS_BIG>::cache m_icache;
	memory_access<12, 2, 0, ENDIANNESS_BIG>::specific m_imem;
	memory_access<12, 2, 0, ENDIANNESS_BIG>::cache m_dcache;
	memory_access<12, 2, 0, ENDIANNESS_BIG>::specific m_dmem;

private:
	union VECTOR_REG
	{
		uint64_t d[2];
		uint32_t l[4];
		uint16_t w[8];
		int16_t  s[8];
		uint8_t  b[16];
	};

	union ACCUMULATOR_REG
	{
		uint64_t q;
		uint32_t l[2];
		uint16_t w[4];
	};

	uint32_t m_debugger_temp;
	uint16_t m_pc_temp;
	uint16_t m_ppc_temp;
	uint16_t m_nextpc_temp;

	devcb_read32 m_dp_reg_r_func;
	devcb_write32 m_dp_reg_w_func;
	devcb_read32 m_sp_reg_r_func;
	devcb_write32 m_sp_reg_w_func;
	devcb_write32 m_sp_set_status_func;

	uint8_t read_dmem_byte(uint32_t address);
	uint16_t read_dmem_word(uint32_t address);
	uint32_t read_dmem_dword(uint32_t address);
	void write_dmem_byte(uint32_t address, uint8_t data);
	void write_dmem_word(uint32_t address, uint16_t data);
	void write_dmem_dword(uint32_t address, uint32_t data);
	uint32_t get_cop0_reg(int reg);
	void set_cop0_reg(int reg, uint32_t data);
	void rspcom_init();

	// COP2 (vectors)
	uint16_t SATURATE_ACCUM(int accum, int slice, uint16_t negative, uint16_t positive);

	uint16_t          m_vres[8];
	VECTOR_REG        m_v[32];
	ACCUMULATOR_REG   m_accum[8];
	uint8_t           m_vcarry;
	uint8_t           m_vcompare;
	uint8_t           m_vclip1;
	uint8_t           m_vzero;
	uint8_t           m_vclip2;

	int32_t           m_reciprocal_res;
	uint32_t          m_reciprocal_high;
	int32_t           m_dp_allowed;

	void              handle_cop2(uint32_t op);
	void              handle_lwc2(uint32_t op);
	void              handle_swc2(uint32_t op);
	void              handle_vector_ops(uint32_t op);

	uint32_t          m_div_in;
	uint32_t          m_div_out;
};

DECLARE_DEVICE_TYPE(RSP, rsp_device)

#endif // MAME_CPU_RSP_RSP_H
