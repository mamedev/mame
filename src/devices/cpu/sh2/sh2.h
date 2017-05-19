// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   sh2.h
 *   Portable Hitachi SH-2 (SH7600 family) emulator interface
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was heavily changed to the MAME CPU requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH2_SH2_H
#define MAME_CPU_SH2_SH2_H

#pragma once

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"


#define SH2_INT_NONE    -1
#define SH2_INT_VBLIN   0
#define SH2_INT_VBLOUT  1
#define SH2_INT_HBLIN   2
#define SH2_INT_TIMER0  3
#define SH2_INT_TIMER1  4
#define SH2_INT_DSP     5
#define SH2_INT_SOUND   6
#define SH2_INT_SMPC    7
#define SH2_INT_PAD     8
#define SH2_INT_DMA2    9
#define SH2_INT_DMA1    10
#define SH2_INT_DMA0    11
#define SH2_INT_DMAILL  12
#define SH2_INT_SPRITE  13
#define SH2_INT_14      14
#define SH2_INT_15      15
#define SH2_INT_ABUS    16

enum
{
	SH2_PC = STATE_GENPC, SH2_SR=1, SH2_PR, SH2_GBR, SH2_VBR, SH2_MACH, SH2_MACL,
	SH2_R0, SH2_R1, SH2_R2, SH2_R3, SH2_R4, SH2_R5, SH2_R6, SH2_R7,
	SH2_R8, SH2_R9, SH2_R10, SH2_R11, SH2_R12, SH2_R13, SH2_R14, SH2_R15, SH2_EA
};


#define SH2_DMA_KLUDGE_CB(name)  int name(uint32_t src, uint32_t dst, uint32_t data, int size)

#define SH2_DMA_FIFO_DATA_AVAILABLE_CB(name)  int name(uint32_t src, uint32_t dst, uint32_t data, int size)

#define SH2_FTCSR_READ_CB(name)  void name(uint32_t data)


#define MCFG_SH2_IS_SLAVE(_slave) \
	sh2_device::set_is_slave(*device, _slave);

#define MCFG_SH2_DMA_KLUDGE_CB(_class, _method) \
	sh2_device::set_dma_kludge_callback(*device, sh2_device::dma_kludge_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SH2_FIFO_DATA_AVAIL_CB(_class, _method) \
	sh2_device::set_dma_fifo_data_available_callback(*device, sh2_device::dma_fifo_data_available_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SH2_FTCSR_READ_CB(_class, _method) \
	sh2_device::set_ftcsr_read_callback(*device, sh2_device::ftcsr_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

#define SH2DRC_STRICT_VERIFY        0x0001          /* verify all instructions */
#define SH2DRC_FLUSH_PC         0x0002          /* flush the PC value before each memory access */
#define SH2DRC_STRICT_PCREL     0x0004          /* do actual loads on MOVLI/MOVWI instead of collapsing to immediates */

#define SH2DRC_COMPATIBLE_OPTIONS   (SH2DRC_STRICT_VERIFY | SH2DRC_FLUSH_PC | SH2DRC_STRICT_PCREL)
#define SH2DRC_FASTEST_OPTIONS  (0)

#define SH2_MAX_FASTRAM       4

class sh2_frontend;

class sh2_device : public cpu_device
{
	friend class sh2_frontend;

public:
	typedef device_delegate<int (uint32_t src, uint32_t dst, uint32_t data, int size)> dma_kludge_delegate;
	typedef device_delegate<int (uint32_t src, uint32_t dst, uint32_t data, int size)> dma_fifo_data_available_delegate;
	typedef device_delegate<void (uint32_t data)> ftcsr_read_delegate;

	// construction/destruction
	sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_is_slave(device_t &device, int slave) { downcast<sh2_device &>(device).m_is_slave = slave; }
	static void set_dma_kludge_callback(device_t &device, dma_kludge_delegate callback) { downcast<sh2_device &>(device).m_dma_kludge_cb = callback; }
	static void set_dma_fifo_data_available_callback(device_t &device, dma_fifo_data_available_delegate callback) { downcast<sh2_device &>(device).m_dma_fifo_data_available_cb = callback; }
	static void set_ftcsr_read_callback(device_t &device, ftcsr_read_delegate callback) { downcast<sh2_device &>(device).m_ftcsr_read_cb = callback; }

	DECLARE_WRITE32_MEMBER( sh7604_w );
	DECLARE_READ32_MEMBER( sh7604_r );
	DECLARE_READ32_MEMBER(sh2_internal_a5);

	void sh2_set_frt_input(int state);
	void sh2drc_set_options(uint32_t options);
	void sh2drc_add_pcflush(offs_t address);
	void sh2drc_add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base);

	void sh2_notify_dma_data_available();

protected:
	sh2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cpu_type,address_map_constructor internal_map, int addrlines);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 4; }
	virtual uint32_t execute_input_lines() const override { return 16; }
	virtual uint32_t execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 2; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;
	address_space *m_program, *m_decrypted_program;

private:
	address_space_config m_program_config, m_decrypted_program_config;

	// Data that needs to be stored close to the generated DRC code
	struct internal_sh2_state
	{
		uint32_t  pc;
		uint32_t  pr;
		uint32_t  sr;
		uint32_t  gbr;
		uint32_t  vbr;
		uint32_t  mach;
		uint32_t  macl;
		uint32_t  r[16];
		uint32_t  ea;
		uint32_t  pending_irq;
		uint32_t  pending_nmi;
		int32_t   irqline;
		uint32_t  evec;               // exception vector for DRC
		uint32_t  irqsr;              // IRQ-time old SR for DRC
		uint32_t  target;             // target for jmp/jsr/etc so the delay slot can't kill it
		int     internal_irq_level;
		int     icount;
		uint8_t   sleep_mode;
		uint32_t  arg0;              /* print_debug argument 1 */
	};

	uint32_t  m_delay;
	uint32_t  m_cpu_off;
	uint32_t  m_dvsr, m_dvdnth, m_dvdntl, m_dvcr;
	uint32_t  m_test_irq;
	struct
	{
		int irq_vector;
		int irq_priority;
	} m_irq_queue[16];

	bool m_isdrc;

	int m_pcfsel;                 // last pcflush entry set
	int m_maxpcfsel;              // highest valid pcflush entry
	uint32_t m_pcflushes[16];           // pcflush entries

	int8_t    m_irq_line_state[17];
protected:
	direct_read_data *m_direct;
private:
	address_space *m_internal;
	uint32_t m_m[0x200/4];
	int8_t  m_nmi_line_state;

	uint16_t  m_frc;
	uint16_t  m_ocra, m_ocrb, m_icr;
	uint64_t  m_frc_base;

	int     m_frt_input;
	int     m_internal_irq_vector;

	emu_timer *m_timer;
	emu_timer *m_dma_current_active_timer[2];
	int     m_dma_timer_active[2];
	uint8_t  m_dma_irq[2];

	int m_active_dma_incs[2];
	int m_active_dma_incd[2];
	int m_active_dma_size[2];
	int m_active_dma_steal[2];
	uint32_t m_active_dma_src[2];
	uint32_t m_active_dma_dst[2];
	uint32_t m_active_dma_count[2];
	uint16_t m_wtcnt;
	uint8_t m_wtcsr;

	int     m_is_slave, m_cpu_type;
	dma_kludge_delegate              m_dma_kludge_cb;
	dma_fifo_data_available_delegate m_dma_fifo_data_available_cb;
	ftcsr_read_delegate              m_ftcsr_read_cb;

	drc_cache           m_cache;                  /* pointer to the DRC code cache */
	std::unique_ptr<drcuml_state>      m_drcuml;                 /* DRC UML generator state */
	std::unique_ptr<sh2_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	uint32_t              m_drcoptions;         /* configurable DRC options */

	internal_sh2_state *m_sh2_state;

	/* internal stuff */
	uint8_t               m_cache_dirty;                /* true if we need to flush the cache */

	/* parameters for subroutines */
	uint64_t              m_numcycles;              /* return value from gettotalcycles */
	uint32_t              m_arg1;                   /* print_debug argument 2 */
	uint32_t              m_irq;                /* irq we're taking */

	/* register mappings */
	uml::parameter      m_regmap[16];                 /* parameter to register mappings for all 16 integer registers */

	uml::code_handle *  m_entry;                      /* entry point */
	uml::code_handle *  m_read8;                  /* read byte */
	uml::code_handle *  m_write8;                 /* write byte */
	uml::code_handle *  m_read16;                 /* read half */
	uml::code_handle *  m_write16;                    /* write half */
	uml::code_handle *  m_read32;                 /* read word */
	uml::code_handle *  m_write32;                    /* write word */

	uml::code_handle *  m_interrupt;              /* interrupt */
	uml::code_handle *  m_nocode;                 /* nocode */
	uml::code_handle *  m_out_of_cycles;              /* out of cycles exception handler */

	/* fast RAM */
	uint32_t              m_fastram_select;
	struct
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		bool                readonly;                   /* true if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	} m_fastram[SH2_MAX_FASTRAM];

	uint32_t m_debugger_temp;

	inline uint8_t RB(offs_t A);
	inline uint16_t RW(offs_t A);
	inline uint32_t RL(offs_t A);
	inline void WB(offs_t A, uint8_t V);
	inline void WW(offs_t A, uint16_t V);
	inline void WL(offs_t A, uint32_t V);
	inline void ADD(uint32_t m, uint32_t n);
	inline void ADDI(uint32_t i, uint32_t n);
	inline void ADDC(uint32_t m, uint32_t n);
	inline void ADDV(uint32_t m, uint32_t n);
	inline void AND(uint32_t m, uint32_t n);
	inline void ANDI(uint32_t i);
	inline void ANDM(uint32_t i);
	inline void BF(uint32_t d);
	inline void BFS(uint32_t d);
	inline void BRA(uint32_t d);
	inline void BRAF(uint32_t m);
	inline void BSR(uint32_t d);
	inline void BSRF(uint32_t m);
	inline void BT(uint32_t d);
	inline void BTS(uint32_t d);
	inline void CLRMAC();
	inline void CLRT();
	inline void CMPEQ(uint32_t m, uint32_t n);
	inline void CMPGE(uint32_t m, uint32_t n);
	inline void CMPGT(uint32_t m, uint32_t n);
	inline void CMPHI(uint32_t m, uint32_t n);
	inline void CMPHS(uint32_t m, uint32_t n);
	inline void CMPPL(uint32_t n);
	inline void CMPPZ(uint32_t n);
	inline void CMPSTR(uint32_t m, uint32_t n);
	inline void CMPIM(uint32_t i);
	inline void DIV0S(uint32_t m, uint32_t n);
	inline void DIV0U();
	inline void DIV1(uint32_t m, uint32_t n);
	inline void DMULS(uint32_t m, uint32_t n);
	inline void DMULU(uint32_t m, uint32_t n);
	inline void DT(uint32_t n);
	inline void EXTSB(uint32_t m, uint32_t n);
	inline void EXTSW(uint32_t m, uint32_t n);
	inline void EXTUB(uint32_t m, uint32_t n);
	inline void EXTUW(uint32_t m, uint32_t n);
	inline void ILLEGAL();
	inline void JMP(uint32_t m);
	inline void JSR(uint32_t m);
	inline void LDCSR(uint32_t m);
	inline void LDCGBR(uint32_t m);
	inline void LDCVBR(uint32_t m);
	inline void LDCMSR(uint32_t m);
	inline void LDCMGBR(uint32_t m);
	inline void LDCMVBR(uint32_t m);
	inline void LDSMACH(uint32_t m);
	inline void LDSMACL(uint32_t m);
	inline void LDSPR(uint32_t m);
	inline void LDSMMACH(uint32_t m);
	inline void LDSMMACL(uint32_t m);
	inline void LDSMPR(uint32_t m);
	inline void MAC_L(uint32_t m, uint32_t n);
	inline void MAC_W(uint32_t m, uint32_t n);
	inline void MOV(uint32_t m, uint32_t n);
	inline void MOVBS(uint32_t m, uint32_t n);
	inline void MOVWS(uint32_t m, uint32_t n);
	inline void MOVLS(uint32_t m, uint32_t n);
	inline void MOVBL(uint32_t m, uint32_t n);
	inline void MOVWL(uint32_t m, uint32_t n);
	inline void MOVLL(uint32_t m, uint32_t n);
	inline void MOVBM(uint32_t m, uint32_t n);
	inline void MOVWM(uint32_t m, uint32_t n);
	inline void MOVLM(uint32_t m, uint32_t n);
	inline void MOVBP(uint32_t m, uint32_t n);
	inline void MOVWP(uint32_t m, uint32_t n);
	inline void MOVLP(uint32_t m, uint32_t n);
	inline void MOVBS0(uint32_t m, uint32_t n);
	inline void MOVWS0(uint32_t m, uint32_t n);
	inline void MOVLS0(uint32_t m, uint32_t n);
	inline void MOVBL0(uint32_t m, uint32_t n);
	inline void MOVWL0(uint32_t m, uint32_t n);
	inline void MOVLL0(uint32_t m, uint32_t n);
	inline void MOVI(uint32_t i, uint32_t n);
	inline void MOVWI(uint32_t d, uint32_t n);
	inline void MOVLI(uint32_t d, uint32_t n);
	inline void MOVBLG(uint32_t d);
	inline void MOVWLG(uint32_t d);
	inline void MOVLLG(uint32_t d);
	inline void MOVBSG(uint32_t d);
	inline void MOVWSG(uint32_t d);
	inline void MOVLSG(uint32_t d);
	inline void MOVBS4(uint32_t d, uint32_t n);
	inline void MOVWS4(uint32_t d, uint32_t n);
	inline void MOVLS4(uint32_t m, uint32_t d, uint32_t n);
	inline void MOVBL4(uint32_t m, uint32_t d);
	inline void MOVWL4(uint32_t m, uint32_t d);
	inline void MOVLL4(uint32_t m, uint32_t d, uint32_t n);
	inline void MOVA(uint32_t d);
	inline void MOVT(uint32_t n);
	inline void MULL(uint32_t m, uint32_t n);
	inline void MULS(uint32_t m, uint32_t n);
	inline void MULU(uint32_t m, uint32_t n);
	inline void NEG(uint32_t m, uint32_t n);
	inline void NEGC(uint32_t m, uint32_t n);
	inline void NOP(void);
	inline void NOT(uint32_t m, uint32_t n);
	inline void OR(uint32_t m, uint32_t n);
	inline void ORI(uint32_t i);
	inline void ORM(uint32_t i);
	inline void ROTCL(uint32_t n);
	inline void ROTCR(uint32_t n);
	inline void ROTL(uint32_t n);
	inline void ROTR(uint32_t n);
	inline void RTE();
	inline void RTS();
	inline void SETT();
	inline void SHAL(uint32_t n);
	inline void SHAR(uint32_t n);
	inline void SHLL(uint32_t n);
	inline void SHLL2(uint32_t n);
	inline void SHLL8(uint32_t n);
	inline void SHLL16(uint32_t n);
	inline void SHLR(uint32_t n);
	inline void SHLR2(uint32_t n);
	inline void SHLR8(uint32_t n);
	inline void SHLR16(uint32_t n);
	inline void SLEEP();
	inline void STCSR(uint32_t n);
	inline void STCGBR(uint32_t n);
	inline void STCVBR(uint32_t n);
	inline void STCMSR(uint32_t n);
	inline void STCMGBR(uint32_t n);
	inline void STCMVBR(uint32_t n);
	inline void STSMACH(uint32_t n);
	inline void STSMACL(uint32_t n);
	inline void STSPR(uint32_t n);
	inline void STSMMACH(uint32_t n);
	inline void STSMMACL(uint32_t n);
	inline void STSMPR(uint32_t n);
	inline void SUB(uint32_t m, uint32_t n);
	inline void SUBC(uint32_t m, uint32_t n);
	inline void SUBV(uint32_t m, uint32_t n);
	inline void SWAPB(uint32_t m, uint32_t n);
	inline void SWAPW(uint32_t m, uint32_t n);
	inline void TAS(uint32_t n);
	inline void TRAPA(uint32_t i);
	inline void TST(uint32_t m, uint32_t n);
	inline void TSTI(uint32_t i);
	inline void TSTM(uint32_t i);
	inline void XOR(uint32_t m, uint32_t n);
	inline void XORI(uint32_t i);
	inline void XORM(uint32_t i);
	inline void XTRCT(uint32_t m, uint32_t n);
	inline void op0000(uint16_t opcode);
	inline void op0001(uint16_t opcode);
	inline void op0010(uint16_t opcode);
	inline void op0011(uint16_t opcode);
	inline void op0100(uint16_t opcode);
	inline void op0101(uint16_t opcode);
	inline void op0110(uint16_t opcode);
	inline void op0111(uint16_t opcode);
	inline void op1000(uint16_t opcode);
	inline void op1001(uint16_t opcode);
	inline void op1010(uint16_t opcode);
	inline void op1011(uint16_t opcode);
	inline void op1100(uint16_t opcode);
	inline void op1101(uint16_t opcode);
	inline void op1110(uint16_t opcode);
	inline void op1111(uint16_t opcode);
	TIMER_CALLBACK_MEMBER( sh2_timer_callback );
	TIMER_CALLBACK_MEMBER( sh2_dma_current_active_callback );
	void sh2_timer_resync();
	void sh2_timer_activate();
	void sh2_do_dma(int dma);
	void sh2_exception(const char *message, int irqline);
	void sh2_dmac_check(int dma);
	void sh2_recalc_irq();

	/* internal compiler state */
	struct compiler_state
	{
		uint32_t          cycles;                     /* accumulated cycles */
		uint8_t           checkints;                  /* need to check interrupts before next instruction */
		uml::code_label  labelnum;                   /* index for local labels */
	};

	inline uint32_t epc(const opcode_desc *desc);
	inline void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	inline void load_fast_iregs(drcuml_block *block);
	inline void save_fast_iregs(drcuml_block *block);

	void code_flush_cache();
	void execute_run_drc();
	void code_compile_block(uint8_t mode, offs_t pc);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle **handleptr);
	const char *log_desc_flags_to_string(uint32_t flags);
	void log_register_list(drcuml_state *drcuml, const char *string, const uint32_t *reglist, const uint32_t *regnostarlist);
	void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);
	void log_add_disasm_comment(drcuml_block *block, uint32_t pc, uint32_t op);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, bool allow_exception);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t ovrpc);
	void generate_delay_slot(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t ovrpc);
	bool generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint32_t ovrpc);
	bool generate_group_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, uint32_t ovrpc);
	bool generate_group_4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);
	bool generate_group_12(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc);

public:
	void func_printf_probe();
	void func_unimplemented();
	void func_fastirq();
	void func_MAC_W();
	void func_MAC_L();
	void func_DIV1();
	void func_ADDV();
	void func_SUBV();
};

class sh2a_device : public sh2_device
{
public:
	// construction/destruction
	sh2a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(dma_sar0_r);
	DECLARE_WRITE32_MEMBER(dma_sar0_w);
	DECLARE_READ32_MEMBER(dma_dar0_r);
	DECLARE_WRITE32_MEMBER(dma_dar0_w);
	DECLARE_READ16_MEMBER(dmaor_r);
	DECLARE_WRITE16_MEMBER(dmaor_w);
	DECLARE_READ16_MEMBER(dma_tcr0_r);
	DECLARE_WRITE16_MEMBER(dma_tcr0_w);
	DECLARE_READ16_MEMBER(dma_chcr0_r);
	DECLARE_WRITE16_MEMBER(dma_chcr0_w);
	DECLARE_READ16_MEMBER(sh7021_r);
	DECLARE_WRITE16_MEMBER(sh7021_w);
	void sh7032_dma_exec(int ch);

private:
	uint16_t m_sh7021_regs[0x200];
	struct
	{
		uint32_t              sar;    /**< Source Address Register */
		uint32_t              dar;    /**< Destination Address Register */
		uint16_t              tcr;    /**< Transfer Count Register */
		uint16_t              chcr;   /**< Channel Control Register */
	} m_dma[4];
	uint16_t m_dmaor;                 /**< DMA Operation Register (status flags) */

};

class sh1_device : public sh2_device
{
public:
	// construction/destruction
	sh1_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	DECLARE_READ16_MEMBER(sh7032_r);
	DECLARE_WRITE16_MEMBER(sh7032_w);
private:
	uint16_t m_sh7032_regs[0x200];
};


class sh2_frontend : public drc_frontend
{
public:
	sh2_frontend(sh2_device *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_2(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_3(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_6(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_8(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_12(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);

	sh2_device *m_sh2;
};


DECLARE_DEVICE_TYPE(SH1,  sh1_device)
DECLARE_DEVICE_TYPE(SH2,  sh2_device)
DECLARE_DEVICE_TYPE(SH2A, sh2a_device)


#endif // MAME_CPU_SH2_SH2_H
