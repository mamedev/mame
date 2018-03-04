// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CPU_TLCS870_TLCS870_H
#define MAME_CPU_TLCS870_TLCS870_H

#pragma once

class tlcs870_device : public cpu_device
{
protected:
	enum _e_op {
		UNKNOWN = 0x00,
		CALL, CALLP, CALLV, CLR, CPL,
		DAA, DAS, DEC, /*DI,*/ DIV,
		/*EI,*/
		INC,
		/*J,*/ JP, JR, JRS,
		LD, LDW,
		MCMP, MUL,
		NOP,
		POP, PUSH,
		RET, RETI, RETN, ROLC, ROLD, RORC, RORD,
		SET, SHLC, SHRC, SWAP, SWI,
		/*TEST,*/ XCH,

		ALU_ADDC,
		ALU_ADD,
		ALU_SUBB,
		ALU_SUB,
		ALU_AND,
		ALU_XOR,
		ALU_OR,
		ALU_CMP
	};

	enum _regs8 {
		REG_A,
		REG_W,
		REG_C,
		REG_B,
		REG_E,
		REG_D,
		REG_L,
		REG_H
	};

	enum _regs16 {
		REG_WA,
		REG_BC,
		REG_DE,
		REG_HL
	};

	enum _regs16p {
		pREG_DE,
		pREG_HL
	};

	enum _regs_debugger {
		DEBUGGER_REG_A,
		DEBUGGER_REG_W,
		DEBUGGER_REG_C,
		DEBUGGER_REG_B,
		DEBUGGER_REG_E,
		DEBUGGER_REG_D,
		DEBUGGER_REG_L,
		DEBUGGER_REG_H,
		DEBUGGER_REG_WA,
		DEBUGGER_REG_BC,
		DEBUGGER_REG_DE,
		DEBUGGER_REG_HL
	};

	enum _conditions {
		COND_EQ_Z,
		COND_NE_NZ,
		COND_LT_CS,
		COND_GE_CC,
		COND_LE,
		COND_GT,
		COND_T,
		COND_F
	};

	// construction/destruction
	tlcs870_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 2; }
	virtual uint32_t execute_max_cycles() const override { return 26; }
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual uint32_t execute_default_irq_vector() const override { return 0xff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual util::disasm_interface *create_disassembler() override;

	void tmp87ph40an_mem(address_map &map);

	uint32_t m_debugger_temp;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;
	required_shared_ptr<uint8_t> m_intram;

	PAIR        m_prvpc, m_pc, m_sp;

	address_space *m_program;
	address_space *m_io;
	int     m_icount;

	// Work registers
	uint16_t m_op;
	int m_param2_type;
	uint16_t m_param2;

	int m_param1_type;
	uint16_t m_param1;
	uint16_t m_temppc; // this is just PPC? use generic reg?

	uint8_t m_bitpos;
	uint8_t m_flagsaffected;
	uint8_t m_cycles;

	uint32_t  m_addr;

	uint16_t m_F;

	/* CPU registers */
	uint8_t m_RBS; // register base (4-bits)

	inline uint8_t  RM8 (uint32_t a);
	inline uint16_t RM16(uint32_t a);
	inline void WM8 (uint32_t a, uint8_t  v);
	inline void WM16(uint32_t a, uint16_t v);
	inline uint8_t  RX8 (uint32_t a, uint32_t base);
	inline uint16_t RX16(uint32_t a, uint32_t base);
	inline void WX8 (uint32_t a, uint8_t  v, uint32_t base);
	inline void WX16(uint32_t a, uint16_t v, uint32_t base);
	inline uint8_t  READ8();
	inline uint16_t READ16();
	void decode();
	void decode_register_prefix(uint8_t b0);
	void decode_source(int type, uint16_t val);
	void decode_dest(uint8_t b0);

	void setbit_param(uint16_t param_type, uint16_t param, uint8_t bit, bool do_flag);
	uint8_t getbit_param(uint16_t param_type, uint16_t param);

	uint16_t get_addr(uint16_t param_type, uint16_t param_val);
	uint16_t get_source_val(uint16_t param_type, uint16_t param_val);
	void set_dest_val(uint16_t param_type, uint16_t param_val, uint16_t dest_val);

	uint8_t get_reg8(int reg);
	void set_reg8(int reg, uint8_t val);
	uint16_t get_reg16(int reg);
	void set_reg16(int reg, uint16_t val);

	bool stream_arg(std::ostream &stream, uint32_t pc, const char *pre, const uint16_t mode, const uint16_t r, const uint16_t rb);
};


class tmp87ph40an_device : public tlcs870_device
{
public:
	// construction/destruction
	tmp87ph40an_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMP87PH40AN, tmp87ph40an_device)

#endif // MAME_CPU_TLCS870_TLCS870_H
