// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** m6805: Portable 6805 emulator ******************************************/
#ifndef MAME_CPU_M6805_M6805_H
#define MAME_CPU_M6805_M6805_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
extern const device_type M6805;
extern const device_type M68HC05EG;
extern const device_type HD63705;

// ======================> m6805_base_device

// Used by core CPU interface
class m6805_base_device : public cpu_device
{
public:
	// construction/destruction
	m6805_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, const char *name, uint32_t addr_width, const char *shortname, const char *source);

protected:
	// addressing mode selector for opcode handler templates
	enum class addr_mode { IM, DI, EX, IX, IX1, IX2 };

	// state index constants
	enum
	{
		M6805_PC = 1,
		M6805_S,
		M6805_CC,
		M6805_A,
		M6805_X,
		M6805_IRQ_STATE
	};

	// CC masks      H INZC
	//            7654 3210
	enum
	{
		CFLAG = 0x01,
		ZFLAG = 0x02,
		NFLAG = 0x04,
		IFLAG = 0x08,
		HFLAG = 0x10
	};

	typedef void (m6805_base_device::*op_handler_func)();

	// opcode tables
	static op_handler_func const m_hmos_ops[256];
	static u8 const m_hmos_cycles[256];
	static u8 const m_cmos_cycles[256];

	m6805_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, const char *name, uint32_t addr_width, address_map_delegate internal_map, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override = 0;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override;
	virtual uint32_t disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// for devices with timing-sensitive peripherals
	virtual void burn_cycles(unsigned count) { }

	void clr_nz()   { m_cc &= ~(NFLAG | ZFLAG); }
	void clr_nzc()  { m_cc &= ~(NFLAG | ZFLAG | CFLAG); }
	void clr_hnzc() { m_cc &= ~(HFLAG | NFLAG | ZFLAG | CFLAG); }

	// macros for CC -- CC bits affected should be reset before calling
	void set_z8(u8 a)                       { if (!a) m_cc |= ZFLAG; }
	void set_n8(u8 a)                       { m_cc |= (a & 0x80) >> 5; }
	void set_h(u8 a, u8 b, u8 r)            { m_cc |= (a ^ b ^ r) & 0x10; }
	void set_c8(u16 a)                      { m_cc |= BIT(a, 8); }

	// combos
	void set_nz8(u8 a)                      { set_n8(a); set_z8(a); }
	void set_nzc8(u16 a)                    { set_nz8(a); set_c8(a); }
	void set_hnzc8(u8 a, u8 b, u16 r)       { set_h(a, b, r); set_nzc8(r); }

	unsigned    rdmem(u32 addr)             { return unsigned(m_program->read_byte(addr)); }
	void        wrmem(u32 addr, u8 value)   { m_program->write_byte(addr, value); }
	unsigned    rdop(u32 addr)              { return unsigned(m_direct->read_byte(addr)); }
	unsigned    rdop_arg(u32 addr)          { return unsigned(m_direct->read_byte(addr)); }

	unsigned    rm(u32 addr)                { return rdmem(addr); }
	void        rm16(u32 addr, PAIR &p);
	void        wm(u32 addr, u8 value)      { wrmem(addr, value); }

	void        pushbyte(u8 b);
	void        pushword(PAIR const &p);
	void        pullbyte(u8 &b);
	void        pullword(PAIR &p);

	template <typename T> void immbyte(T &b);
	void immword(PAIR &w);
	void skipbyte();

	template <unsigned B> void brset();
	template <unsigned B> void brclr();
	template <unsigned B> void bset();
	template <unsigned B> void bclr();

	template <bool C> void bra();
	template <bool C> void bhi();
	template <bool C> void bcc();
	template <bool C> void bne();
	template <bool C> void bhcc();
	template <bool C> void bpl();
	template <bool C> void bmc();
	virtual void bil();
	virtual void bih();
	void bsr();

	template <addr_mode M> void neg();
	template <addr_mode M> void com();
	template <addr_mode M> void lsr();
	template <addr_mode M> void ror();
	template <addr_mode M> void asr();
	template <addr_mode M> void lsl();
	template <addr_mode M> void rol();
	template <addr_mode M> void dec();
	template <addr_mode M> void inc();
	template <addr_mode M> void tst();
	template <addr_mode M> void clr();

	void nega();
	void coma();
	void lsra();
	void rora();
	void asra();
	void lsla();
	void rola();
	void deca();
	void inca();
	void tsta();
	void clra();

	void negx();
	void comx();
	void lsrx();
	void rorx();
	void asrx();
	void lslx();
	void rolx();
	void decx();
	void incx();
	void tstx();
	void clrx();

	void rti();
	void rts();
	virtual void swi();

	void tax();
	void txa();

	void clc();
	void sec();
	void cli();
	void sei();

	void rsp();
	void nop();

	template <addr_mode M> void suba();
	template <addr_mode M> void cmpa();
	template <addr_mode M> void sbca();
	template <addr_mode M> void cpx();
	template <addr_mode M> void anda();
	template <addr_mode M> void bita();
	template <addr_mode M> void lda();
	template <addr_mode M> void sta();
	template <addr_mode M> void eora();
	template <addr_mode M> void adca();
	template <addr_mode M> void ora();
	template <addr_mode M> void adda();
	template <addr_mode M> void jmp();
	template <addr_mode M> void jsr();
	template <addr_mode M> void ldx();
	template <addr_mode M> void stx();

	void illegal();

	virtual void interrupt();
	virtual void interrupt_vector();

	const char *m_tag;

	// address spaces
	const address_space_config m_program_config;

	// CPU registers
	PAIR    m_ea;           // effective address (should really be a temporary in opcode handlers)

	u32     m_sp_mask;      // Stack pointer address mask
	u32     m_sp_low;       // Stack pointer low water mark (or floor)
	PAIR    m_pc;           // Program counter
	PAIR    m_s;            // Stack pointer
	u8      m_a;            // Accumulator
	u8      m_x;            // Index register
	u8      m_cc;           // Condition codes

	uint16_t  m_pending_interrupts; /* MB */

	int     m_irq_state[9]; /* KW Additional lines for HD63705 */
	int     m_nmi_state;

	// other internal states
	int     m_icount;

	// address spaces
	address_space *m_program;
	direct_read_data *m_direct;
};


// ======================> m6805_device

class m6805_device : public m6805_base_device
{
public:
	// construction/destruction
	m6805_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: m6805_base_device(mconfig, tag, owner, clock, M6805, "M6805", 12, "m6805", __FILE__) { }

protected:
	virtual void execute_set_input(int inputnum, int state) override;
};


// ======================> m68hc05eg_device

class m68hc05eg_device : public m6805_base_device
{
public:
	// construction/destruction
	m68hc05eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: m6805_base_device(mconfig, tag, owner, clock, M68HC05EG, "M68HC05EG", 13, "m68hc05eg", __FILE__) { }

protected:
	// device-level overrides
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void interrupt_vector() override;
};

// ======================> hd63705_device

class hd63705_device : public m6805_base_device
{
public:
	// construction/destruction
	hd63705_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: m6805_base_device(mconfig, tag, owner, clock, HD63705, "HD63705", 16, "hd63705", __FILE__) { }

protected:
	// device-level overrides
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void interrupt_vector() override;

	// opcodes
	virtual void bil() override;
	virtual void bih() override;
	virtual void swi() override;
};

#define M6805_IRQ_LINE      0

/****************************************************************************
 * 68HC05EG section
 ****************************************************************************/

#define M68HC05EG_INT_IRQ   (M6805_IRQ_LINE)
#define M68HC05EG_INT_TIMER (M6805_IRQ_LINE+1)
#define M68HC05EG_INT_CPI   (M6805_IRQ_LINE+2)

/****************************************************************************
 * HD63705 section
 ****************************************************************************/

#define HD63705_A                   M6805_A
#define HD63705_PC                  M6805_PC
#define HD63705_S                   M6805_S
#define HD63705_X                   M6805_X
#define HD63705_CC                  M6805_CC
#define HD63705_NMI_STATE           M6805_IRQ_STATE
#define HD63705_IRQ1_STATE          M6805_IRQ_STATE+1
#define HD63705_IRQ2_STATE          M6805_IRQ_STATE+2
#define HD63705_ADCONV_STATE        M6805_IRQ_STATE+3

#define HD63705_INT_MASK            0x1ff

#define HD63705_INT_IRQ1            0x00
#define HD63705_INT_IRQ2            0x01
#define HD63705_INT_TIMER1          0x02
#define HD63705_INT_TIMER2          0x03
#define HD63705_INT_TIMER3          0x04
#define HD63705_INT_PCI             0x05
#define HD63705_INT_SCI             0x06
#define HD63705_INT_ADCONV          0x07
#define HD63705_INT_NMI             0x08

CPU_DISASSEMBLE( m6805 );

#endif // MAME_CPU_M6805_M6805_H
