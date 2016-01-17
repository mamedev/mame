// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#pragma once

#ifndef __SPC700_H__
#define __SPC700_H__


class spc700_device :  public cpu_device
{
public:
	// construction/destruction
	spc700_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 2; }
	virtual UINT32 execute_max_cycles() const override { return 8; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	UINT32 m_a;     /* Accumulator */
	UINT32 m_x;     /* Index Register X */
	UINT32 m_y;     /* Index Register Y */
	UINT32 m_s;     /* Stack Pointer */
	UINT32 m_pc;    /* Program Counter */
	UINT32 m_ppc;   /* Previous Program Counter */
	UINT32 m_flag_n;    /* Negative Flag */
	UINT32 m_flag_z;    /* Zero flag */
	UINT32 m_flag_v;    /* Overflow Flag */
	UINT32 m_flag_p;    /* Direct Page Flag */
	UINT32 m_flag_b;    /* BRK Instruction Flag */
	UINT32 m_flag_h;    /* Half-carry Flag */
	UINT32 m_flag_i;    /* Interrupt Mask Flag */
	UINT32 m_flag_c;    /* Carry Flag */
	UINT32 m_line_irq;  /* Status of the IRQ line */
	UINT32 m_line_nmi;  /* Status of the NMI line */
	UINT32 m_line_rst;  /* Status of the RESET line */
	UINT32 m_ir;        /* Instruction Register */
	address_space *m_program;
	UINT32 m_stopped;   /* stopped status */
	int m_ICount;
	UINT32 m_source;
	UINT32 m_destination;
	UINT32 m_temp1;
	UINT32 m_temp2;
	UINT32 m_temp3;
	short m_spc_int16;
	int m_spc_int32;

	UINT32 m_debugger_temp;

	inline UINT32 read_8_normal(UINT32 address);
	inline UINT32 read_8_immediate(UINT32 address);
	inline UINT32 read_8_instruction(UINT32 address);
	inline UINT32 read_8_direct(UINT32 address);
	inline void write_8_normal(UINT32 address, UINT32 value);
	inline void write_8_direct(UINT32 address, UINT32 value);
	inline UINT32 read_16_normal(UINT32 address);
	inline UINT32 read_16_immediate(UINT32 address);
	inline UINT32 read_16_direct(UINT32 address);
	inline void write_16_direct(UINT32 address, UINT32 value);
	inline UINT32 EA_IMM();
	inline UINT32 EA_IMM16();
	inline UINT32 EA_ABS();
	inline UINT32 EA_ABX();
	inline UINT32 EA_ABY();
	inline UINT32 EA_AXI();
	inline UINT32 EA_DP();
	inline UINT32 EA_DPX();
	inline UINT32 EA_DPY();
	inline UINT32 EA_DXI();
	inline UINT32 EA_DIY();
	inline UINT32 EA_XI();
	inline UINT32 EA_XII();
	inline UINT32 EA_YI();
	inline void JUMP(UINT32 address);
	inline void BRANCH(UINT32 offset);
	inline void SET_REG_YA(UINT32 value);
	inline void SET_REG_P(UINT32 value);
	inline void PUSH_8(UINT32 value);
	inline UINT32 PULL_8();
	inline void PUSH_16(UINT32 value);
	inline UINT32 PULL_16();
	inline void CHECK_IRQ();
	inline void SET_FLAG_I(UINT32 value);
	void SERVICE_IRQ();
};


extern const device_type SPC700;


/* ======================================================================== */
/* ============================= Configuration ============================ */
/* ======================================================================== */

/* Turn on optimizations for SNES since it doesn't hook up the interrupt lines */
#define SPC700_OPTIMIZE_SNES 1


/* ======================================================================== */
/* ============================== PROTOTYPES ============================== */
/* ======================================================================== */

enum
{
	SPC700_PC=1, SPC700_S, SPC700_P, SPC700_A, SPC700_X, SPC700_Y
};

#define SPC700_INT_NONE         0
#define SPC700_INT_IRQ          1
#define SPC700_INT_NMI          2


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __SPC700_H__ */
