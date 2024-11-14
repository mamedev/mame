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
DECLARE_DEVICE_TYPE(M68HC05EG, m68hc05eg_device)
DECLARE_DEVICE_TYPE(HD6305V0,  hd6305v0_device)
DECLARE_DEVICE_TYPE(HD6305Y2,  hd6305y2_device)
DECLARE_DEVICE_TYPE(HD63705Z0, hd63705z0_device)

// ======================> m6805_base_device

// Used by core CPU interface
class m6805_base_device : public cpu_device
{
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
	typedef op_handler_func const op_handler_table[256];
	typedef u8 const cycle_count_table[256];

	struct configuration_params
	{
		configuration_params(
				op_handler_table &ops,
				cycle_count_table &cycles,
				u32 addr_width,
				u32 sp_mask,
				u32 sp_floor,
				u16 swi_vector)
			: m_ops(ops)
			, m_cycles(cycles)
			, m_addr_width(addr_width)
			, m_sp_mask(sp_mask)
			, m_sp_floor(sp_floor)
			, m_vector_mask((1U << addr_width) - 1)
			, m_swi_vector(swi_vector)
		{
		}

		configuration_params(
				op_handler_table &ops,
				cycle_count_table &cycles,
				u32 addr_width,
				u32 sp_mask,
				u32 sp_floor,
				u16 vector_mask,
				u16 swi_vector)
			: m_ops(ops)
			, m_cycles(cycles)
			, m_addr_width(addr_width)
			, m_sp_mask(sp_mask)
			, m_sp_floor(sp_floor)
			, m_vector_mask(vector_mask)
			, m_swi_vector(swi_vector)
		{
		}

		op_handler_table &m_ops;
		cycle_count_table &m_cycles;
		u32 m_addr_width;
		u32 m_sp_mask;
		u32 m_sp_floor;
		u16 m_vector_mask;
		u16 m_swi_vector;
	};

	// opcode tables
	static op_handler_table s_hmos_s_ops;
	static op_handler_table s_hmos_b_ops;
	static op_handler_table s_cmos_b_ops;
	static op_handler_table s_hc_s_ops;
	static op_handler_table s_hc_b_ops;
	static cycle_count_table s_hmos_cycles;
	static cycle_count_table s_cmos_cycles;
	static cycle_count_table s_hc_cycles;

	// construction/destruction
	m6805_base_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			device_type const type,
			configuration_params const &params);
	m6805_base_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			device_type const type,
			configuration_params const &params,
			address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return true; }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// for devices with timing-sensitive peripherals
	virtual void burn_cycles(unsigned count) { }

	void clr_nz()   { m_cc &= ~(NFLAG | ZFLAG); }
	void clr_nzc()  { m_cc &= ~(NFLAG | ZFLAG | CFLAG); }
	void clr_hc()   { m_cc &= ~(HFLAG | CFLAG); }
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

	template <bool big> unsigned    rdmem(u32 addr)             { return big ? m_program16.read_byte(addr) : m_program13.read_byte(addr); }
	template <bool big> void        wrmem(u32 addr, u8 value)   { if(big) m_program16.write_byte(addr, value); else m_program13.write_byte(addr, value); }
	template <bool big> unsigned    rdop(u32 addr)              { return big ? m_cprogram16.read_byte(addr) : m_cprogram13.read_byte(addr); }
	template <bool big> unsigned    rdop_arg(u32 addr)          { return big ? m_cprogram16.read_byte(addr) : m_cprogram13.read_byte(addr); }

	template <bool big> unsigned    rm(u32 addr)                { return rdmem<big>(addr); }
	template <bool big> void        rm16(u32 addr, PAIR &p);
	template <bool big> void        wm(u32 addr, u8 value)      { wrmem<big>(addr, value); }

	template <bool big> void        pushbyte(u8 b);
	template <bool big> void        pushword(PAIR const &p);
	template <bool big> void        pullbyte(u8 &b);
	template <bool big> void        pullword(PAIR &p);

	template <bool big, typename T> void immbyte(T &b);
	template <bool big> void immword(PAIR &w);
	template <bool big> void skipbyte();

	template <bool big, unsigned B> void brset();
	template <bool big, unsigned B> void brclr();
	template <bool big, unsigned B> void bset();
	template <bool big, unsigned B> void bclr();

	template <bool big, bool C> void bra();
	template <bool big, bool C> void bhi();
	template <bool big, bool C> void bcc();
	template <bool big, bool C> void bne();
	template <bool big, bool C> void bhcc();
	template <bool big, bool C> void bpl();
	template <bool big, bool C> void bmc();
	template <bool big, bool C> void bil();
	template <bool big> void bsr();

	template <bool big, addr_mode M> void neg();
	template <bool big, addr_mode M> void com();
	template <bool big, addr_mode M> void lsr();
	template <bool big, addr_mode M> void ror();
	template <bool big, addr_mode M> void asr();
	template <bool big, addr_mode M> void lsl();
	template <bool big, addr_mode M> void rol();
	template <bool big, addr_mode M> void dec();
	template <bool big, addr_mode M> void inc();
	template <bool big, addr_mode M> void tst();
	template <bool big, addr_mode M> void clr();

	template <bool big> void nega();
	template <bool big> void mul();
	template <bool big> void coma();
	template <bool big> void lsra();
	template <bool big> void rora();
	template <bool big> void asra();
	template <bool big> void lsla();
	template <bool big> void rola();
	template <bool big> void deca();
	template <bool big> void inca();
	template <bool big> void tsta();
	template <bool big> void clra();

	template <bool big> void negx();
	template <bool big> void comx();
	template <bool big> void lsrx();
	template <bool big> void rorx();
	template <bool big> void asrx();
	template <bool big> void lslx();
	template <bool big> void rolx();
	template <bool big> void decx();
	template <bool big> void incx();
	template <bool big> void tstx();
	template <bool big> void clrx();

	template <bool big> void rti();
	template <bool big> void rts();
	template <bool big> void swi();
	template <bool big> void stop();
	template <bool big> void wait();

	template <bool big> void tax();
	template <bool big> void txa();

	template <bool big> void clc();
	template <bool big> void sec();
	template <bool big> void cli();
	template <bool big> void sei();

	template <bool big> void rsp();
	template <bool big> void nop();

	template <bool big, addr_mode M> void suba();
	template <bool big, addr_mode M> void cmpa();
	template <bool big, addr_mode M> void sbca();
	template <bool big, addr_mode M> void cpx();
	template <bool big, addr_mode M> void anda();
	template <bool big, addr_mode M> void bita();
	template <bool big, addr_mode M> void lda();
	template <bool big, addr_mode M> void sta();
	template <bool big, addr_mode M> void eora();
	template <bool big, addr_mode M> void adca();
	template <bool big, addr_mode M> void ora();
	template <bool big, addr_mode M> void adda();
	template <bool big, addr_mode M> void jmp();
	template <bool big, addr_mode M> void jsr();
	template <bool big, addr_mode M> void ldx();
	template <bool big, addr_mode M> void stx();

	template <bool big> void illegal();

	virtual void interrupt();
	virtual void interrupt_vector();
	virtual bool test_il();

	configuration_params const m_params;
	u32 m_min_cycles;
	u32 m_max_cycles;

	// address spaces
	address_space_config const m_program_config;

	// CPU registers
	PAIR    m_ea;           // effective address (should really be a temporary in opcode handlers)

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
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cprogram16;
	memory_access<13, 0, 0, ENDIANNESS_BIG>::cache m_cprogram13;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program16;
	memory_access<13, 0, 0, ENDIANNESS_BIG>::specific m_program13;
};


// ======================> m68hc05eg_device

class m68hc05eg_device : public m6805_base_device
{
public:
	// construction/destruction
	m68hc05eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	virtual void interrupt_vector() override;

	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

// ======================> hd6305_device

class hd6305_device : public m6805_base_device
{
protected:
	// construction/destruction
	hd6305_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			device_type const type,
			configuration_params const &params,
			address_map_constructor internal_map);

	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }

	virtual void interrupt_vector() override;
	virtual bool test_il() override { return m_nmi_state != CLEAR_LINE; }
};

// ======================> hd6305v0_device

class hd6305v0_device : public hd6305_device
{
public:
	// construction/destruction
	hd6305v0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// ======================> hd6305y2_device

class hd6305y2_device : public hd6305_device
{
public:
	// construction/destruction
	hd6305y2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// ======================> hd63705z0_device

class hd63705z0_device : public hd6305_device
{
public:
	// construction/destruction
	hd63705z0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
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

#endif // MAME_CPU_M6805_M6805_H
