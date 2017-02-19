// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once

#ifndef TLCS870_H
#define TLCS870_H

class tlcs870_device : public cpu_device
{
public:
	// construction/destruction
	tlcs870_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source, address_map_constructor program_map);


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

	uint32_t m_debugger_temp;

#define IS16BIT 0x80
#define BITPOS 0x40
#define BITPOS_INDIRECT 0x20

#define ABSOLUTE_VAL_8 0x01
#define REG_8BIT 0x02

// special
#define CONDITIONAL 0x03
#define STACKPOINTER (0x04 | IS16BIT) // this is a 16-bit reg
#define CARRYFLAG (0x5 | BITPOS) // also flag as BITPOS since it's a bit operation?
#define MEMVECTOR_16BIT 0x6
#define REGISTERBANK 0x7
#define PROGRAMSTATUSWORD 0x8

#define ABSOLUTE_VAL_16 (ABSOLUTE_VAL_8|IS16BIT)
#define REG_16BIT (REG_8BIT|IS16BIT)

#define ADDR_IN_BASE 0x10
#define ADDR_IN_IMM_X (ADDR_IN_BASE+0x0)
#define ADDR_IN_PC_PLUS_REG_A (ADDR_IN_BASE+0x1)
#define ADDR_IN_DE (ADDR_IN_BASE+0x2)
#define ADDR_IN_HL (ADDR_IN_BASE+0x3)
#define ADDR_IN_HL_PLUS_IMM_D (ADDR_IN_BASE+0x4)
#define ADDR_IN_HL_PLUS_REG_C (ADDR_IN_BASE+0x5)
#define ADDR_IN_HLINC (ADDR_IN_BASE+0x6)
#define ADDR_IN_DECHL (ADDR_IN_BASE+0x7)

#define MODE_MASK 0x1f


#define FLAG_J (0x80)
#define FLAG_Z (0x40)
#define FLAG_C (0x20)
#define FLAG_H (0x10)


#define IS_JF ((m_F & FLAG_J) ? 1 : 0)
#define IS_ZF ((m_F & FLAG_Z) ? 1 : 0)
#define IS_CF ((m_F & FLAG_C) ? 1 : 0)
#define IS_HF ((m_F & FLAG_H) ? 1 : 0)

#define SET_JF (m_F |= FLAG_J)
#define SET_ZF (m_F |= FLAG_Z)
#define SET_CF (m_F |= FLAG_C)
#define SET_HF (m_F |= FLAG_H)

#define CLEAR_JF (m_F &= ~FLAG_J)
#define CLEAR_ZF (m_F &= ~FLAG_Z)
#define CLEAR_CF (m_F &= ~FLAG_C)
#define CLEAR_HF (m_F &= ~FLAG_H)

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
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 6; }
	virtual void disasm_disassemble_param(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options, int type, uint16_t val);
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

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



extern const device_type TMP87PH40AN;

#endif /* TLCS870_H */
