// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    m6809.h

    Portable Motorola 6809 emulator

**********************************************************************/

#ifndef MAME_CPU_M6809_M6809_H
#define MAME_CPU_M6809_M6809_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(MC6809, mc6809_device)
DECLARE_DEVICE_TYPE(MC6809E, mc6809e_device)
DECLARE_DEVICE_TYPE(M6809, m6809_device)

// ======================> m6809_base_device

// Used by core CPU interface
class m6809_base_device : public cpu_device
{
protected:
	// construction/destruction
	m6809_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, int divider);

	class memory_interface {
	public:
		memory_access<16, 0, 0, ENDIANNESS_BIG>::cache cprogram, csprogram;
		memory_access<16, 0, 0, ENDIANNESS_BIG>::specific program;

		virtual ~memory_interface() {}
		virtual uint8_t read(uint16_t adr) = 0;
		virtual uint8_t read_opcode(uint16_t adr) = 0;
		virtual uint8_t read_opcode_arg(uint16_t adr) = 0;
		virtual void write(uint16_t adr, uint8_t val) = 0;
	};

	class mi_default : public memory_interface {
	public:
		virtual ~mi_default() {}
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_opcode(uint16_t adr) override;
		virtual uint8_t read_opcode_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint32_t execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual bool is_6809() { return true; }

	// addressing modes
	enum
	{
		ADDRESSING_MODE_IMMEDIATE   = 0,
		ADDRESSING_MODE_EA          = 1,
		ADDRESSING_MODE_REGISTER_A  = 2,
		ADDRESSING_MODE_REGISTER_B  = 3,
		ADDRESSING_MODE_REGISTER_D = 4
	};

	// register transfer
	struct exgtfr_register
	{
		uint8_t   byte_value;
		uint16_t  word_value;
	};

	// flag bits in the cc register
	enum
	{
		CC_C        = 0x01,         // Carry
		CC_V        = 0x02,         // Overflow
		CC_Z        = 0x04,         // Zero
		CC_N        = 0x08,         // Negative
		CC_I        = 0x10,         // Inhibit IRQ
		CC_H        = 0x20,         // Half (auxiliary) carry
		CC_F        = 0x40,         // Inhibit FIRQ
		CC_E        = 0x80          // Entire state pushed
	};

	// flag combinations
	enum
	{
		CC_VC   = CC_V | CC_C,
		CC_ZC   = CC_Z | CC_C,
		CC_NZ   = CC_N | CC_Z,
		CC_NZC  = CC_N | CC_Z | CC_C,
		CC_NZV  = CC_N | CC_Z | CC_V,
		CC_NZVC = CC_N | CC_Z | CC_V | CC_C,
		CC_HNZVC = CC_H | CC_N | CC_Z | CC_V | CC_C
	};

	// interrupt vectors
	enum
	{
		VECTOR_SWI3         = 0xFFF2,
		VECTOR_SWI2         = 0xFFF4,
		VECTOR_FIRQ         = 0xFFF6,
		VECTOR_IRQ          = 0xFFF8,
		VECTOR_SWI          = 0xFFFA,
		VECTOR_NMI          = 0xFFFC,
		VECTOR_RESET_FFFE   = 0xFFFE
	};

	union M6809Q
	{
		#ifdef LSB_FIRST
			union
			{
				struct { uint8_t f, e, b, a; };
				struct { uint16_t w, d; };
			} r;
			struct { PAIR16 w, d; } p;
		#else
			union
			{
				struct { uint8_t a, b, e, f; };
				struct { uint16_t d, w; };
			} r;
			struct { PAIR16 d, w; } p;
		#endif
		uint32_t q;
	};

	// Memory interface
	std::unique_ptr<memory_interface> m_mintf;

	// CPU registers
	PAIR16                      m_pc;               // program counter
	PAIR16                      m_ppc;              // previous program counter
	M6809Q                      m_q;                // accumulator a and b (plus e and f on 6309)
	PAIR16                      m_x, m_y;           // index registers
	PAIR16                      m_u, m_s;           // stack pointers
	uint8_t                       m_dp;               // direct page register
	uint8_t                       m_cc;
	PAIR16                      m_temp;
	uint8_t                       m_opcode;

	// other internal state
	uint8_t *                     m_reg8;
	PAIR16 *                    m_reg16;
	int                         m_reg;
	bool                        m_nmi_line;
	bool                        m_nmi_asserted;
	bool                        m_firq_line;
	bool                        m_irq_line;
	bool                        m_lds_encountered;
	int                         m_icount;
	int                         m_addressing_mode;
	PAIR16                      m_ea;               // effective address

	// Callbacks
	devcb_write_line           m_lic_func;         // LIC pin on the 6809E

	// eat cycles
	inline void eat(int cycles)                          { m_icount -= cycles; }
	void eat_remaining();

	// read a byte from given memory location
	inline uint8_t read_memory(uint16_t address)             { eat(1); return m_mintf->read(address); }

	// write a byte to given memory location
	inline void write_memory(uint16_t address, uint8_t data) { eat(1); m_mintf->write(address, data); }

	// read_opcode() is like read_memory() except it is used for reading opcodes. In  the case of a system
	// with memory mapped I/O, this function can be used  to greatly speed up emulation.
	inline uint8_t read_opcode(uint16_t address)             { eat(1); return m_mintf->read_opcode(address); }

	// read_opcode_arg() is identical to read_opcode() except it is used for reading opcode  arguments. This
	// difference can be used to support systems that use different encoding mechanisms for opcodes
	// and opcode arguments.
	inline uint8_t read_opcode_arg(uint16_t address)         { eat(1); return m_mintf->read_opcode_arg(address); }

	// read_opcode() and bump the program counter
	inline uint8_t read_opcode()                           { return read_opcode(m_pc.w++); }
	inline uint8_t read_opcode_arg()                       { return read_opcode_arg(m_pc.w++); }
	inline void dummy_read_opcode_arg(uint16_t delta)      { read_opcode_arg(m_pc.w + delta); }
	inline void dummy_vma(int count)                       { for(int i=0; i != count; i++) { read_opcode_arg(0xffff); } }

	// state stack - implemented as a uint32_t
	void push_state(uint16_t state)                    { m_state = (m_state << 9) | state; }
	uint16_t pop_state()                               { uint16_t result = m_state & 0x1ff; m_state >>= 9; return result; }
	void reset_state()                              { m_state = 0; }

	// effective address reading/writing
	uint8_t read_ea()                                 { return read_memory(m_ea.w); }
	void write_ea(uint8_t data)                       { write_memory(m_ea.w, data); }
	void set_ea(uint16_t ea)                          { m_ea.w = ea; m_addressing_mode = ADDRESSING_MODE_EA; }
	void set_ea_h(uint8_t ea_h)                       { m_ea.b.h = ea_h; }
	void set_ea_l(uint8_t ea_l)                       { m_ea.b.l = ea_l; m_addressing_mode = ADDRESSING_MODE_EA; }

	// operand reading/writing
	uint8_t read_operand();
	uint8_t read_operand(int ordinal);
	void write_operand(uint8_t data);
	void write_operand(int ordinal, uint8_t data);

	// instructions
	void daa();
	void mul();

	// miscellaneous
	void nop()                                      { }
	template<class T> T rotate_right(T value);
	template<class T> uint32_t rotate_left(T value);
	void set_a()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_A; }
	void set_b()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_B; }
	void set_d()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_D; }
	void set_imm()                                  { m_addressing_mode = ADDRESSING_MODE_IMMEDIATE; }
	void set_regop8(uint8_t &reg)                     { m_reg8 = &reg; m_reg16 = nullptr; }
	void set_regop16(PAIR16 &reg)                   { m_reg16 = &reg; m_reg8 = nullptr; }
	uint8_t &regop8()                                 { assert(m_reg8 != nullptr); return *m_reg8; }
	PAIR16 &regop16()                               { assert(m_reg16 != nullptr); return *m_reg16; }
	bool is_register_register_op_16_bit()           { return m_reg16 != nullptr; }
	bool add8_sets_h()                              { return true; }
	bool hd6309_native_mode()                       { return false; }

	// index reg
	uint16_t &ireg();

	// flags
	template<class T> T set_flags(uint8_t mask, T a, T b, uint32_t r);
	template<class T> T set_flags(uint8_t mask, T r);

	// branch conditions
	inline bool cond_hi() { return !(m_cc & CC_ZC); }                                                // BHI/BLS
	inline bool cond_cc() { return !(m_cc & CC_C);   }                                               // BCC/BCS
	inline bool cond_ne() { return !(m_cc & CC_Z);   }                                               // BNE/BEQ
	inline bool cond_vc() { return !(m_cc & CC_V);   }                                               // BVC/BVS
	inline bool cond_pl() { return !(m_cc & CC_N);   }                                               // BPL/BMI
	inline bool cond_ge() { return (m_cc & CC_N ? true : false) == (m_cc & CC_V ? true : false); }   // BGE/BLT
	inline bool cond_gt() { return cond_ge() && !(m_cc & CC_Z); }                                    // BGT/BLE
	inline void set_cond(bool cond)  { m_cond = cond; }
	inline bool branch_taken()       { return m_cond; }

	// interrupt registers
	bool firq_saves_entire_state()      { return false; }
	uint16_t partial_state_registers()    { return 0x81; }
	uint16_t entire_state_registers()     { return 0xFF; }

	// miscellaneous
	inline exgtfr_register read_exgtfr_register(uint8_t reg);
	inline void write_exgtfr_register(uint8_t reg, exgtfr_register value);
	bool is_register_addressing_mode();
	bool is_ea_addressing_mode() { return m_addressing_mode == ADDRESSING_MODE_EA; }
	uint16_t get_pending_interrupt();
	void log_illegal();

private:
	// address spaces
	const address_space_config  m_program_config;
	const address_space_config  m_sprogram_config;

	// other state
	uint32_t                      m_state;
	bool                        m_cond;

	// incidentals
	int                         m_clock_divider;

	// functions
	inline void execute_one();
	const char *inputnum_string(int inputnum);
};

// ======================> mc6809_device

class mc6809_device : public m6809_base_device
{
public:
	// construction/destruction
	mc6809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> mc6809e_device

class mc6809e_device : public m6809_base_device
{
public:
	// construction/destruction
	mc6809e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// MC6809E has LIC line to indicate opcode/data fetch
	auto lic() { return m_lic_func.bind(); }
};

// ======================> m6809_device (LEGACY)

class m6809_device : public m6809_base_device
{
public:
	// construction/destruction
	m6809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

enum
{
	M6809_PC = STATE_GENPC, M6809_S = 0, M6809_CC ,M6809_A, M6809_B, M6809_D, M6809_U, M6809_X, M6809_Y,
	M6809_DP
};

#define M6809_IRQ_LINE  0   /* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */
#define M6809_SWI       2   /* Virtual SWI line to be used during SWI acknowledge cycle */

#endif // MAME_CPU_M6809_M6809_H
