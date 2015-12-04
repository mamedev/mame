// license:BSD-3-Clause
// copyright-holders:Ville Linde
#pragma once

#ifndef __TMS32082_H__
#define __TMS32082_H__

// Master Processor class
class tms32082_mp_device : public cpu_device
{
public:
	// construction/destruction
	tms32082_mp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

	DECLARE_READ32_MEMBER(mp_param_r);
	DECLARE_WRITE32_MEMBER(mp_param_w);


protected:
	// device level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 1; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			default:         return nullptr;
		}
	}

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	address_space_config m_program_config;

	static const UINT32 SHIFT_MASK[33];


	UINT32 m_pc;
	UINT32 m_fetchpc;
	union
	{
		UINT32 m_reg[32];
		UINT64 m_fpair[16];
	};
	union
	{
		UINT64 m_acc[4];
		double m_facc[4];
	};
	UINT32 m_ir;

	UINT32 m_in0p;
	UINT32 m_in1p;
	UINT32 m_outp;
	UINT32 m_ie;
	UINT32 m_intpen;
	UINT32 m_epc;
	UINT32 m_eip;

	UINT32 m_tcount;
	UINT32 m_tscale;

	UINT32 m_param_ram[0x800];

	int m_icount;

	address_space *m_program;
	direct_read_data* m_direct;

	void check_interrupts();
	void processor_command(UINT32 command);
	UINT32 fetch();
	void delay_slot();
	void execute();
	void execute_short_imm();
	void execute_reg_long_imm();
	UINT32 read_creg(int reg);
	void write_creg(int reg, UINT32 data);
	bool test_condition(int condition, UINT32 value);
	UINT32 calculate_cmp(UINT32 src1, UINT32 src2);
	void vector_loadstore();
};


// Parallel Processor class
class tms32082_pp_device : public cpu_device
{
public:
	// construction/destruction
	tms32082_pp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	enum
	{
		PP_PC = 1
	};

protected:
	// device level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 1; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			default:         return nullptr;
		}
	}

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 8; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	address_space_config m_program_config;

	UINT32 m_pc;
	UINT32 m_fetchpc;

	int m_icount;

	address_space *m_program;
	direct_read_data* m_direct;
};


extern const device_type TMS32082_MP;
extern const device_type TMS32082_PP;


#endif /* __TMS32082_H__ */
