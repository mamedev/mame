// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** m6805: Portable 6805 emulator ******************************************/

#pragma once

#ifndef __M6805_H__
#define __M6805_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m6805_device;

// device type definition
extern const device_type M6805;
extern const device_type M68HC05EG;
extern const device_type M68705;
extern const device_type HD63705;

// ======================> m6805_base_device

// Used by core CPU interface
class m6805_base_device : public cpu_device
{
public:
	// construction/destruction
	m6805_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const device_type type, const char *name, UINT32 addr_width, const char *shortname, const char *source);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override = 0;
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

private:
	// opcode/condition tables
	static const UINT8 m_flags8i[256];
	static const UINT8 m_flags8d[256];
	static const UINT8 m_cycles1[256];

protected:
	void rd_s_handler_b(UINT8 *b);
	void rd_s_handler_w(PAIR *p);
	void wr_s_handler_b(UINT8 *b);
	void wr_s_handler_w(PAIR *p);
	void RM16(UINT32 addr, PAIR *p);

	void brset(UINT8 bit);
	void brclr(UINT8 bit);
	void bset(UINT8 bit);
	void bclr(UINT8 bit);

	void bra();
	void brn();
	void bhi();
	void bls();
	void bcc();
	void bcs();
	void bne();
	void beq();
	void bhcc();
	void bhcs();
	void bpl();
	void bmi();
	void bmc();
	void bms();
	virtual void bil();
	virtual void bih();
	void bsr();

	void neg_di();
	void com_di();
	void lsr_di();
	void ror_di();
	void asr_di();
	void lsl_di();
	void rol_di();
	void dec_di();
	void inc_di();
	void tst_di();
	void clr_di();

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
	void aslx();
//  void lslx();
	void rolx();
	void decx();
	void incx();
	void tstx();
	void clrx();

	void neg_ix1();
	void com_ix1();
	void lsr_ix1();
	void ror_ix1();
	void asr_ix1();
	void lsl_ix1();
	void rol_ix1();
	void dec_ix1();
	void inc_ix1();
	void tst_ix1();
	void clr_ix1();

	void neg_ix();
	void com_ix();
	void lsr_ix();
	void ror_ix();
	void asr_ix();
	void lsl_ix();
	void rol_ix();
	void dec_ix();
	void inc_ix();
	void tst_ix();
	void clr_ix();

	void rti();
	void rts();
	virtual void swi();

	void tax();
	void txa();

	void rsp();
	void nop();

	void suba_im();
	void cmpa_im();
	void sbca_im();
	void cpx_im();
	void anda_im();
	void bita_im();
	void lda_im();
	void eora_im();
	void adca_im();
	void ora_im();
	void adda_im();

	void ldx_im();
	void suba_di();
	void cmpa_di();
	void sbca_di();
	void cpx_di();
	void anda_di();
	void bita_di();
	void lda_di();
	void sta_di();
	void eora_di();
	void adca_di();
	void ora_di();
	void adda_di();
	void jmp_di();
	void jsr_di();
	void ldx_di();
	void stx_di();
	void suba_ex();
	void cmpa_ex();
	void sbca_ex();
	void cpx_ex();
	void anda_ex();
	void bita_ex();
	void lda_ex();
	void sta_ex();
	void eora_ex();
	void adca_ex();
	void ora_ex();
	void adda_ex();
	void jmp_ex();
	void jsr_ex();
	void ldx_ex();
	void stx_ex();
	void suba_ix2();
	void cmpa_ix2();
	void sbca_ix2();
	void cpx_ix2();
	void anda_ix2();
	void bita_ix2();
	void lda_ix2();
	void sta_ix2();
	void eora_ix2();
	void adca_ix2();
	void ora_ix2();
	void adda_ix2();
	void jmp_ix2();
	void jsr_ix2();
	void ldx_ix2();
	void stx_ix2();
	void suba_ix1();
	void cmpa_ix1();
	void sbca_ix1();
	void cpx_ix1();
	void anda_ix1();
	void bita_ix1();
	void lda_ix1();
	void sta_ix1();
	void eora_ix1();
	void adca_ix1();
	void ora_ix1();
	void adda_ix1();
	void jmp_ix1();
	void jsr_ix1();
	void ldx_ix1();
	void stx_ix1();
	void suba_ix();
	void cmpa_ix();
	void sbca_ix();
	void cpx_ix();
	void anda_ix();
	void bita_ix();
	void lda_ix();
	void sta_ix();
	void eora_ix();
	void adca_ix();
	void ora_ix();
	void adda_ix();
	void jmp_ix();
	void jsr_ix();
	void ldx_ix();
	void stx_ix();

	void illegal();

	virtual void interrupt();
	virtual void interrupt_vector();

	const char *m_tag;

	// address spaces
	const address_space_config m_program_config;

	// CPU registers
	PAIR    m_ea;           /* effective address */

	UINT32  m_sp_mask;      /* Stack pointer address mask */
	UINT32  m_sp_low;       /* Stack pointer low water mark (or floor) */
	PAIR    m_pc;           /* Program counter */
	PAIR    m_s;            /* Stack pointer */
	UINT8   m_a;            /* Accumulator */
	UINT8   m_x;            /* Index register */
	UINT8   m_cc;           /* Condition codes */

	UINT16  m_pending_interrupts; /* MB */

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
	m6805_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: m6805_base_device(mconfig, tag, owner, clock, M6805, "M6805", 12, "m6805", __FILE__) { }

protected:
	virtual void execute_set_input(int inputnum, int state) override;
};

// ======================> m68hc05eg_device

class m68hc05eg_device : public m6805_base_device
{
public:
	// construction/destruction
	m68hc05eg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: m6805_base_device(mconfig, tag, owner, clock, M68HC05EG, "M68HC05EG", 13, "m68hc05eg", __FILE__) { }

protected:
	// device-level overrides
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void interrupt_vector() override;
};

// ======================> m68705_device

class m68705_device : public m6805_base_device
{
public:
	// construction/destruction
	m68705_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: m6805_base_device(mconfig, tag, owner, clock, M68705, "M68705", 12, "m68705", __FILE__) { }

protected:
	// device-level overrides
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;

	virtual void interrupt() override;
};

// ======================> hd63705_device

class hd63705_device : public m6805_base_device
{
public:
	// construction/destruction
	hd63705_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
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

enum { M6805_PC=1, M6805_S, M6805_CC, M6805_A, M6805_X, M6805_IRQ_STATE };

#define M6805_IRQ_LINE      0

/****************************************************************************
 * 68HC05EG section
 ****************************************************************************/

#define M68HC05EG_INT_IRQ   (M6805_IRQ_LINE)
#define M68HC05EG_INT_TIMER (M6805_IRQ_LINE+1)
#define M68HC05EG_INT_CPI   (M6805_IRQ_LINE+2)

/****************************************************************************
 * 68705 section
 ****************************************************************************/

#define M68705_A                    M6805_A
#define M68705_PC                   M6805_PC
#define M68705_S                    M6805_S
#define M68705_X                    M6805_X
#define M68705_CC                   M6805_CC
#define M68705_IRQ_STATE            M6805_IRQ_STATE

#define M68705_INT_MASK             0x03
#define M68705_IRQ_LINE             M6805_IRQ_LINE
#define M68705_INT_TIMER            0x01

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

#endif /* __M6805_H__ */
