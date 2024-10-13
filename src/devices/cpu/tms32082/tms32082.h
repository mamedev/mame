// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_CPU_TMS32082_TMS32082_H
#define MAME_CPU_TMS32082_TMS32082_H

#pragma once

// Master Processor class
class tms32082_mp_device : public cpu_device
{
public:
	// construction/destruction
	tms32082_mp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		MP_PC=1,
		MP_R0,
		MP_R1,
		MP_R2,
		MP_R3,
		MP_R4,
		MP_R5,
		MP_R6,
		MP_R7,
		MP_R8,
		MP_R9,
		MP_R10,
		MP_R11,
		MP_R12,
		MP_R13,
		MP_R14,
		MP_R15,
		MP_R16,
		MP_R17,
		MP_R18,
		MP_R19,
		MP_R20,
		MP_R21,
		MP_R22,
		MP_R23,
		MP_R24,
		MP_R25,
		MP_R26,
		MP_R27,
		MP_R28,
		MP_R29,
		MP_R30,
		MP_R31,
		MP_ACC0,
		MP_ACC1,
		MP_ACC2,
		MP_ACC3,
		MP_IN0P,
		MP_IN1P,
		MP_OUTP,
		MP_IE,
		MP_INTPEN,
		MP_TCOUNT,
		MP_TSCALE
	};

	enum
	{
		INPUT_X1        = 1,
		INPUT_X2        = 2,
		INPUT_X3        = 3,
		INPUT_X4        = 4
	};

	uint32_t mp_param_r(offs_t offset, uint32_t mem_mask = ~0);
	void mp_param_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	template <typename... T> void set_command_callback(T &&... args) { m_cmd_callback.set(std::forward<T>(args)...); }

	void mp_internal_map(address_map &map) ATTR_COLD;
protected:
	// device level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;

	static const uint32_t SHIFT_MASK[33];


	uint32_t m_pc;
	uint32_t m_fetchpc;
	union
	{
		uint32_t m_reg[32];
		uint64_t m_fpair[16];
	};
	union
	{
		uint64_t m_acc[4];
		double m_facc[4];
	};
	uint32_t m_ir;

	uint32_t m_in0p;
	uint32_t m_in1p;
	uint32_t m_outp;
	uint32_t m_ie;
	uint32_t m_intpen;
	uint32_t m_epc;
	uint32_t m_eip;

	uint32_t m_tcount;
	uint32_t m_tscale;

	uint32_t m_param_ram[0x800];

	int m_icount;

	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_program;

	write32mo_delegate m_cmd_callback;

	uint32_t m_pp_status;

	void check_interrupts();
	void processor_command(uint32_t command);
	uint32_t fetch();
	void delay_slot();
	void execute();
	void execute_short_imm();
	void execute_reg_long_imm();
	uint32_t read_creg(int reg);
	void write_creg(int reg, uint32_t data);
	bool test_condition(int condition, uint32_t value);
	uint32_t calculate_cmp(uint32_t src1, uint32_t src2);
	void vector_loadstore();
	void tc_command_execute(int channel, uint32_t entrypoint);
};


// Parallel Processor class
class tms32082_pp_device : public cpu_device
{
public:
	// construction/destruction
	tms32082_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		PP_PC = 1
	};

	void pp_internal_map(address_map &map) ATTR_COLD;
protected:
	// device level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;

	uint32_t m_pc;
	uint32_t m_fetchpc;

	int m_icount;

	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_program;
};


DECLARE_DEVICE_TYPE(TMS32082_MP, tms32082_mp_device)
DECLARE_DEVICE_TYPE(TMS32082_PP, tms32082_pp_device)


#endif // MAME_CPU_TMS32082_TMS32082_H
