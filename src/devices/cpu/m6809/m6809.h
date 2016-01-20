// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    m6809.h

    Portable Motorola 6809 emulator

**********************************************************************/

#pragma once

#ifndef __M6809_H__
#define __M6809_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m6809_device;


// device type definition
extern const device_type M6809;
extern const device_type M6809E;

// ======================> m6809_base_device

// Used by core CPU interface
class m6809_base_device : public cpu_device
{
public:
	// construction/destruction
	m6809_base_device(const machine_config &mconfig, std::string name, std::string tag, device_t *owner, UINT32 clock, const device_type type, int divider, std::string shortname, std::string source);

	DECLARE_WRITE_LINE_MEMBER( irq_line );
	DECLARE_WRITE_LINE_MEMBER( firq_line );
	DECLARE_WRITE_LINE_MEMBER( nmi_line );

protected:
	class memory_interface {
	public:
		address_space *m_program, *m_sprogram;
		direct_read_data *m_direct, *m_sdirect;

		virtual ~memory_interface() {}
		virtual UINT8 read(UINT16 adr) = 0;
		virtual UINT8 read_opcode(UINT16 adr) = 0;
		virtual UINT8 read_opcode_arg(UINT16 adr) = 0;
		virtual void write(UINT16 adr, UINT8 val) = 0;
	};

	class mi_default : public memory_interface {
	public:
		virtual ~mi_default() {}
		virtual UINT8 read(UINT16 adr) override;
		virtual UINT8 read_opcode(UINT16 adr) override;
		virtual UINT8 read_opcode_arg(UINT16 adr) override;
		virtual void write(UINT16 adr, UINT8 val) override;
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

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
		UINT8   byte_value;
		UINT16  word_value;
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

	// Memory interface
	memory_interface *          m_mintf;

	// CPU registers
	PAIR16                      m_pc;               // program counter
	PAIR16                      m_ppc;              // previous program counter
	PAIR16                      m_d;                // accumulator a and b
	PAIR16                      m_x, m_y;           // index registers
	PAIR16                      m_u, m_s;           // stack pointers
	UINT8                       m_dp;               // direct page register
	UINT8                       m_cc;
	PAIR16                      m_temp;
	UINT8                       m_opcode;

	// other internal state
	UINT8 *                     m_reg8;
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
	inline UINT8 read_memory(UINT16 address)             { eat(1); return m_mintf->read(address); }

	// write a byte to given memory location
	inline void write_memory(UINT16 address, UINT8 data) { eat(1); m_mintf->write(address, data); }

	// read_opcode() is like read_memory() except it is used for reading opcodes. In  the case of a system
	// with memory mapped I/O, this function can be used  to greatly speed up emulation.
	inline UINT8 read_opcode(UINT16 address)             { eat(1); return m_mintf->read_opcode(address); }

	// read_opcode_arg() is identical to read_opcode() except it is used for reading opcode  arguments. This
	// difference can be used to support systems that use different encoding mechanisms for opcodes
	// and opcode arguments.
	inline UINT8 read_opcode_arg(UINT16 address)         { eat(1); return m_mintf->read_opcode_arg(address); }

	// read_opcode() and bump the program counter
	inline UINT8 read_opcode()                           { return read_opcode(m_pc.w++); }
	inline UINT8 read_opcode_arg()                       { return read_opcode_arg(m_pc.w++); }

	// state stack - implemented as a UINT32
	void push_state(UINT8 state)                    { m_state = (m_state << 8) | state; }
	UINT8 pop_state()                               { UINT8 result = (UINT8) m_state; m_state >>= 8; return result; }
	void reset_state()                              { m_state = 0; }

	// effective address reading/writing
	UINT8 read_ea()                                 { return read_memory(m_ea.w); }
	void write_ea(UINT8 data)                       { write_memory(m_ea.w, data); }
	void set_ea(UINT16 ea)                          { m_ea.w = ea; m_addressing_mode = ADDRESSING_MODE_EA; }
	void set_ea_h(UINT8 ea_h)                       { m_ea.b.h = ea_h; }
	void set_ea_l(UINT8 ea_l)                       { m_ea.b.l = ea_l; m_addressing_mode = ADDRESSING_MODE_EA; }

	// operand reading/writing
	UINT8 read_operand();
	UINT8 read_operand(int ordinal);
	void write_operand(UINT8 data);
	void write_operand(int ordinal, UINT8 data);

	// instructions
	void daa();
	void mul();

	// miscellaneous
	void nop()                                      { }
	template<class T> T rotate_right(T value);
	template<class T> UINT32 rotate_left(T value);
	void set_a()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_A; }
	void set_b()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_B; }
	void set_d()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_D; }
	void set_imm()                                  { m_addressing_mode = ADDRESSING_MODE_IMMEDIATE; }
	void set_regop8(UINT8 &reg)                     { m_reg8 = &reg; m_reg16 = nullptr; }
	void set_regop16(PAIR16 &reg)                   { m_reg16 = &reg; m_reg8 = nullptr; }
	UINT8 &regop8()                                 { assert(m_reg8 != nullptr); return *m_reg8; }
	PAIR16 &regop16()                               { assert(m_reg16 != nullptr); return *m_reg16; }
	bool is_register_register_op_16_bit()           { return m_reg16 != nullptr; }
	bool add8_sets_h()                              { return true; }
	bool hd6309_native_mode()                       { return false; }

	// index reg
	UINT16 &ireg();

	// flags
	template<class T> T set_flags(UINT8 mask, T a, T b, UINT32 r);
	template<class T> T set_flags(UINT8 mask, T r);

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
	UINT16 partial_state_registers()    { return 0x81; }
	UINT16 entire_state_registers()     { return 0xFF; }

	// miscellaneous
	inline exgtfr_register read_exgtfr_register(UINT8 reg);
	inline void write_exgtfr_register(UINT8 reg, exgtfr_register value);
	bool is_register_addressing_mode();
	bool is_ea_addressing_mode() { return m_addressing_mode == ADDRESSING_MODE_EA; }
	UINT16 get_pending_interrupt();
	void log_illegal();

private:
	// address spaces
	const address_space_config  m_program_config;
	const address_space_config  m_sprogram_config;

	// other state
	UINT32                      m_state;
	bool                        m_cond;

	// incidentals
	int                         m_clock_divider;

	// functions
	inline void execute_one();
	const char *inputnum_string(int inputnum);
};

// ======================> m6809_device

class m6809_device : public m6809_base_device
{
public:
	// construction/destruction
	m6809_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

// ======================> m6809e_device

#define MCFG_M6809E_LIC_CB(_devcb) \
	m6809e_device::set_lic_cb(*device, DEVCB_##_devcb);


class m6809e_device : public m6809_base_device
{
public:
	// construction/destruction
	m6809e_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_lic_cb(device_t &device, _Object object) { return downcast<m6809e_device &>(device).m_lic_func.set_callback(object); }
};

enum
{
	M6809_PC=1, M6809_S, M6809_CC ,M6809_A, M6809_B, M6809_D, M6809_U, M6809_X, M6809_Y,
	M6809_DP
};

#define M6809_IRQ_LINE  0   /* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

/* M6809e has LIC line to indicate opcode/data fetch */

#endif /* __M6809_H__ */
