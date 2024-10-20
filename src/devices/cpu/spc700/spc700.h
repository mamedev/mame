// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_SPC700_SPC700_H
#define MAME_CPU_SPC700_SPC700_H

#pragma once


class spc700_device : public cpu_device
{
public:
	// construction/destruction
	spc700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// construction/destruction
	spc700_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map = address_map_constructor());

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 8; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
private:
	uint32_t m_a;     /* Accumulator */
	uint32_t m_x;     /* Index Register X */
	uint32_t m_y;     /* Index Register Y */
	uint32_t m_s;     /* Stack Pointer */
	uint32_t m_pc;    /* Program Counter */
	uint32_t m_ppc;   /* Previous Program Counter */
	uint32_t m_flag_n;    /* Negative Flag */
	uint32_t m_flag_z;    /* Zero flag */
	uint32_t m_flag_v;    /* Overflow Flag */
	uint32_t m_flag_p;    /* Direct Page Flag */
	uint32_t m_flag_b;    /* BRK Instruction Flag */
	uint32_t m_flag_h;    /* Half-carry Flag */
	uint32_t m_flag_i;    /* Interrupt Mask Flag */
	uint32_t m_flag_c;    /* Carry Flag */
	uint32_t m_line_irq;  /* Status of the IRQ line */
	uint32_t m_line_nmi;  /* Status of the NMI line */
	uint32_t m_line_rst;  /* Status of the RESET line */
	uint32_t m_ir;        /* Instruction Register */
	address_space *m_program;
	uint32_t m_stopped;   /* stopped status */
	int m_ICount;
	uint32_t m_source;
	uint32_t m_destination;
	uint32_t m_temp1;
	uint32_t m_temp2;
	uint32_t m_temp3;
	short m_spc_int16;
	int m_spc_int32;

	uint32_t m_debugger_temp;

	inline uint32_t read_8_normal(uint32_t address);
	inline uint32_t read_8_immediate(uint32_t address);
	inline uint32_t read_8_instruction(uint32_t address);
	inline uint32_t read_8_direct(uint32_t address);
	inline void write_8_normal(uint32_t address, uint32_t value);
	inline void write_8_direct(uint32_t address, uint32_t value);
	inline uint32_t read_16_normal(uint32_t address);
	inline uint32_t read_16_immediate(uint32_t address);
	inline uint32_t read_16_direct(uint32_t address);
	inline void write_16_direct(uint32_t address, uint32_t value);
	inline uint32_t EA_IMM();
	inline uint32_t EA_IMM16();
	inline uint32_t EA_ABS();
	inline uint32_t EA_ABX();
	inline uint32_t EA_ABY();
	inline uint32_t EA_AXI();
	inline uint32_t EA_DP();
	inline uint32_t EA_DPX();
	inline uint32_t EA_DPY();
	inline uint32_t EA_DXI();
	inline uint32_t EA_DIY();
	inline uint32_t EA_XI();
	inline uint32_t EA_XII();
	inline uint32_t EA_YI();
	inline void JUMP(uint32_t address);
	inline void BRANCH(uint32_t offset);
	inline void SET_REG_YA(uint32_t value);
	inline void SET_REG_P(uint32_t value);
	inline void PUSH_8(uint32_t value);
	inline uint32_t PULL_8();
	inline void PUSH_16(uint32_t value);
	inline uint32_t PULL_16();
	inline void CHECK_IRQ();
	inline void SET_FLAG_I(uint32_t value);
	void SERVICE_IRQ();
};


DECLARE_DEVICE_TYPE(SPC700, spc700_device)


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

#endif // MAME_CPU_SPC700_SPC700_H
