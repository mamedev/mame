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

#pragma once

#ifndef __SH2_H__
#define __SH2_H__

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
	SH2_PC=1, SH2_SR, SH2_PR, SH2_GBR, SH2_VBR, SH2_MACH, SH2_MACL,
	SH2_R0, SH2_R1, SH2_R2, SH2_R3, SH2_R4, SH2_R5, SH2_R6, SH2_R7,
	SH2_R8, SH2_R9, SH2_R10, SH2_R11, SH2_R12, SH2_R13, SH2_R14, SH2_R15, SH2_EA
};


typedef device_delegate<int (UINT32 src, UINT32 dst, UINT32 data, int size)> sh2_dma_kludge_delegate;
#define SH2_DMA_KLUDGE_CB(name)  int name(UINT32 src, UINT32 dst, UINT32 data, int size)

typedef device_delegate<int (UINT32 src, UINT32 dst, UINT32 data, int size)> sh2_dma_fifo_data_available_delegate;
#define SH2_DMA_FIFO_DATA_AVAILABLE_CB(name)  int name(UINT32 src, UINT32 dst, UINT32 data, int size)

typedef device_delegate<void (UINT32 data)> sh2_ftcsr_read_delegate;
#define SH2_FTCSR_READ_CB(name)  void name(UINT32 data)


#define MCFG_SH2_IS_SLAVE(_slave) \
	sh2_device::set_is_slave(*device, _slave);

#define MCFG_SH2_DMA_KLUDGE_CB(_class, _method) \
	sh2_device::set_dma_kludge_callback(*device, sh2_dma_kludge_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SH2_FIFO_DATA_AVAIL_CB(_class, _method) \
	sh2_device::set_dma_fifo_data_available_callback(*device, sh2_dma_fifo_data_available_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SH2_FTCSR_READ_CB(_class, _method) \
	sh2_device::set_ftcsr_read_callback(*device, sh2_ftcsr_read_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


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
	// construction/destruction
	sh2_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);
	sh2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int cpu_type,address_map_constructor internal_map, int addrlines);

	static void set_is_slave(device_t &device, int slave) { downcast<sh2_device &>(device).m_is_slave = slave; }
	static void set_dma_kludge_callback(device_t &device, sh2_dma_kludge_delegate callback) { downcast<sh2_device &>(device).m_dma_kludge_cb = callback; }
	static void set_dma_fifo_data_available_callback(device_t &device, sh2_dma_fifo_data_available_delegate callback) { downcast<sh2_device &>(device).m_dma_fifo_data_available_cb = callback; }
	static void set_ftcsr_read_callback(device_t &device, sh2_ftcsr_read_delegate callback) { downcast<sh2_device &>(device).m_ftcsr_read_cb = callback; }

	DECLARE_WRITE32_MEMBER( sh7604_w );
	DECLARE_READ32_MEMBER( sh7604_r );
	DECLARE_READ32_MEMBER(sh2_internal_a5);

	void sh2_set_frt_input(int state);
	void sh2drc_set_options(UINT32 options);
	void sh2drc_add_pcflush(offs_t address);
	void sh2drc_add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base);

	void sh2_notify_dma_data_available();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 4; }
	virtual UINT32 execute_input_lines() const override { return 16; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	address_space *m_program, *m_decrypted_program;

private:
	address_space_config m_program_config, m_decrypted_program_config;

	// Data that needs to be stored close to the generated DRC code
	struct internal_sh2_state
	{
		UINT32  ppc;
		UINT32  pc;
		UINT32  pr;
		UINT32  sr;
		UINT32  gbr;
		UINT32  vbr;
		UINT32  mach;
		UINT32  macl;
		UINT32  r[16];
		UINT32  ea;
		UINT32  pending_irq;
		UINT32  pending_nmi;
		INT32   irqline;
		UINT32  evec;               // exception vector for DRC
		UINT32  irqsr;              // IRQ-time old SR for DRC
		UINT32  target;             // target for jmp/jsr/etc so the delay slot can't kill it
		int     internal_irq_level;
		int     icount;
		UINT8   sleep_mode;
		UINT32  arg0;              /* print_debug argument 1 */
	};

	UINT32  m_delay;
	UINT32  m_cpu_off;
	UINT32  m_dvsr, m_dvdnth, m_dvdntl, m_dvcr;
	UINT32  m_test_irq;
	struct
	{
		int irq_vector;
		int irq_priority;
	} m_irq_queue[16];

	bool m_isdrc;

	int m_pcfsel;                 // last pcflush entry set
	int m_maxpcfsel;              // highest valid pcflush entry
	UINT32 m_pcflushes[16];           // pcflush entries

	INT8    m_irq_line_state[17];
protected:
	direct_read_data *m_direct;
private:
	address_space *m_internal;
	UINT32 m_m[0x200/4];
	INT8  m_nmi_line_state;

	UINT16  m_frc;
	UINT16  m_ocra, m_ocrb, m_icr;
	UINT64  m_frc_base;

	int     m_frt_input;
	int     m_internal_irq_vector;

	emu_timer *m_timer;
	emu_timer *m_dma_current_active_timer[2];
	int     m_dma_timer_active[2];
	UINT8  m_dma_irq[2];

	int m_active_dma_incs[2];
	int m_active_dma_incd[2];
	int m_active_dma_size[2];
	int m_active_dma_steal[2];
	UINT32 m_active_dma_src[2];
	UINT32 m_active_dma_dst[2];
	UINT32 m_active_dma_count[2];
	UINT16 m_wtcnt;
	UINT8 m_wtcsr;

	int     m_is_slave, m_cpu_type;
	sh2_dma_kludge_delegate              m_dma_kludge_cb;
	sh2_dma_fifo_data_available_delegate m_dma_fifo_data_available_cb;
	sh2_ftcsr_read_delegate              m_ftcsr_read_cb;

	drc_cache           m_cache;                  /* pointer to the DRC code cache */
	std::unique_ptr<drcuml_state>      m_drcuml;                 /* DRC UML generator state */
	std::unique_ptr<sh2_frontend>      m_drcfe;                  /* pointer to the DRC front-end state */
	UINT32              m_drcoptions;         /* configurable DRC options */

	internal_sh2_state *m_sh2_state;

	/* internal stuff */
	UINT8               m_cache_dirty;                /* true if we need to flush the cache */

	/* parameters for subroutines */
	UINT64              m_numcycles;              /* return value from gettotalcycles */
	UINT32              m_arg1;                   /* print_debug argument 2 */
	UINT32              m_irq;                /* irq we're taking */

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
	UINT32              m_fastram_select;
	struct
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		UINT8               readonly;                   /* TRUE if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	} m_fastram[SH2_MAX_FASTRAM];

	UINT32 m_debugger_temp;

	inline UINT8 RB(offs_t A);
	inline UINT16 RW(offs_t A);
	inline UINT32 RL(offs_t A);
	inline void WB(offs_t A, UINT8 V);
	inline void WW(offs_t A, UINT16 V);
	inline void WL(offs_t A, UINT32 V);
	inline void ADD(UINT32 m, UINT32 n);
	inline void ADDI(UINT32 i, UINT32 n);
	inline void ADDC(UINT32 m, UINT32 n);
	inline void ADDV(UINT32 m, UINT32 n);
	inline void AND(UINT32 m, UINT32 n);
	inline void ANDI(UINT32 i);
	inline void ANDM(UINT32 i);
	inline void BF(UINT32 d);
	inline void BFS(UINT32 d);
	inline void BRA(UINT32 d);
	inline void BRAF(UINT32 m);
	inline void BSR(UINT32 d);
	inline void BSRF(UINT32 m);
	inline void BT(UINT32 d);
	inline void BTS(UINT32 d);
	inline void CLRMAC();
	inline void CLRT();
	inline void CMPEQ(UINT32 m, UINT32 n);
	inline void CMPGE(UINT32 m, UINT32 n);
	inline void CMPGT(UINT32 m, UINT32 n);
	inline void CMPHI(UINT32 m, UINT32 n);
	inline void CMPHS(UINT32 m, UINT32 n);
	inline void CMPPL(UINT32 n);
	inline void CMPPZ(UINT32 n);
	inline void CMPSTR(UINT32 m, UINT32 n);
	inline void CMPIM(UINT32 i);
	inline void DIV0S(UINT32 m, UINT32 n);
	inline void DIV0U();
	inline void DIV1(UINT32 m, UINT32 n);
	inline void DMULS(UINT32 m, UINT32 n);
	inline void DMULU(UINT32 m, UINT32 n);
	inline void DT(UINT32 n);
	inline void EXTSB(UINT32 m, UINT32 n);
	inline void EXTSW(UINT32 m, UINT32 n);
	inline void EXTUB(UINT32 m, UINT32 n);
	inline void EXTUW(UINT32 m, UINT32 n);
	inline void ILLEGAL();
	inline void JMP(UINT32 m);
	inline void JSR(UINT32 m);
	inline void LDCSR(UINT32 m);
	inline void LDCGBR(UINT32 m);
	inline void LDCVBR(UINT32 m);
	inline void LDCMSR(UINT32 m);
	inline void LDCMGBR(UINT32 m);
	inline void LDCMVBR(UINT32 m);
	inline void LDSMACH(UINT32 m);
	inline void LDSMACL(UINT32 m);
	inline void LDSPR(UINT32 m);
	inline void LDSMMACH(UINT32 m);
	inline void LDSMMACL(UINT32 m);
	inline void LDSMPR(UINT32 m);
	inline void MAC_L(UINT32 m, UINT32 n);
	inline void MAC_W(UINT32 m, UINT32 n);
	inline void MOV(UINT32 m, UINT32 n);
	inline void MOVBS(UINT32 m, UINT32 n);
	inline void MOVWS(UINT32 m, UINT32 n);
	inline void MOVLS(UINT32 m, UINT32 n);
	inline void MOVBL(UINT32 m, UINT32 n);
	inline void MOVWL(UINT32 m, UINT32 n);
	inline void MOVLL(UINT32 m, UINT32 n);
	inline void MOVBM(UINT32 m, UINT32 n);
	inline void MOVWM(UINT32 m, UINT32 n);
	inline void MOVLM(UINT32 m, UINT32 n);
	inline void MOVBP(UINT32 m, UINT32 n);
	inline void MOVWP(UINT32 m, UINT32 n);
	inline void MOVLP(UINT32 m, UINT32 n);
	inline void MOVBS0(UINT32 m, UINT32 n);
	inline void MOVWS0(UINT32 m, UINT32 n);
	inline void MOVLS0(UINT32 m, UINT32 n);
	inline void MOVBL0(UINT32 m, UINT32 n);
	inline void MOVWL0(UINT32 m, UINT32 n);
	inline void MOVLL0(UINT32 m, UINT32 n);
	inline void MOVI(UINT32 i, UINT32 n);
	inline void MOVWI(UINT32 d, UINT32 n);
	inline void MOVLI(UINT32 d, UINT32 n);
	inline void MOVBLG(UINT32 d);
	inline void MOVWLG(UINT32 d);
	inline void MOVLLG(UINT32 d);
	inline void MOVBSG(UINT32 d);
	inline void MOVWSG(UINT32 d);
	inline void MOVLSG(UINT32 d);
	inline void MOVBS4(UINT32 d, UINT32 n);
	inline void MOVWS4(UINT32 d, UINT32 n);
	inline void MOVLS4(UINT32 m, UINT32 d, UINT32 n);
	inline void MOVBL4(UINT32 m, UINT32 d);
	inline void MOVWL4(UINT32 m, UINT32 d);
	inline void MOVLL4(UINT32 m, UINT32 d, UINT32 n);
	inline void MOVA(UINT32 d);
	inline void MOVT(UINT32 n);
	inline void MULL(UINT32 m, UINT32 n);
	inline void MULS(UINT32 m, UINT32 n);
	inline void MULU(UINT32 m, UINT32 n);
	inline void NEG(UINT32 m, UINT32 n);
	inline void NEGC(UINT32 m, UINT32 n);
	inline void NOP(void);
	inline void NOT(UINT32 m, UINT32 n);
	inline void OR(UINT32 m, UINT32 n);
	inline void ORI(UINT32 i);
	inline void ORM(UINT32 i);
	inline void ROTCL(UINT32 n);
	inline void ROTCR(UINT32 n);
	inline void ROTL(UINT32 n);
	inline void ROTR(UINT32 n);
	inline void RTE();
	inline void RTS();
	inline void SETT();
	inline void SHAL(UINT32 n);
	inline void SHAR(UINT32 n);
	inline void SHLL(UINT32 n);
	inline void SHLL2(UINT32 n);
	inline void SHLL8(UINT32 n);
	inline void SHLL16(UINT32 n);
	inline void SHLR(UINT32 n);
	inline void SHLR2(UINT32 n);
	inline void SHLR8(UINT32 n);
	inline void SHLR16(UINT32 n);
	inline void SLEEP();
	inline void STCSR(UINT32 n);
	inline void STCGBR(UINT32 n);
	inline void STCVBR(UINT32 n);
	inline void STCMSR(UINT32 n);
	inline void STCMGBR(UINT32 n);
	inline void STCMVBR(UINT32 n);
	inline void STSMACH(UINT32 n);
	inline void STSMACL(UINT32 n);
	inline void STSPR(UINT32 n);
	inline void STSMMACH(UINT32 n);
	inline void STSMMACL(UINT32 n);
	inline void STSMPR(UINT32 n);
	inline void SUB(UINT32 m, UINT32 n);
	inline void SUBC(UINT32 m, UINT32 n);
	inline void SUBV(UINT32 m, UINT32 n);
	inline void SWAPB(UINT32 m, UINT32 n);
	inline void SWAPW(UINT32 m, UINT32 n);
	inline void TAS(UINT32 n);
	inline void TRAPA(UINT32 i);
	inline void TST(UINT32 m, UINT32 n);
	inline void TSTI(UINT32 i);
	inline void TSTM(UINT32 i);
	inline void XOR(UINT32 m, UINT32 n);
	inline void XORI(UINT32 i);
	inline void XORM(UINT32 i);
	inline void XTRCT(UINT32 m, UINT32 n);
	inline void op0000(UINT16 opcode);
	inline void op0001(UINT16 opcode);
	inline void op0010(UINT16 opcode);
	inline void op0011(UINT16 opcode);
	inline void op0100(UINT16 opcode);
	inline void op0101(UINT16 opcode);
	inline void op0110(UINT16 opcode);
	inline void op0111(UINT16 opcode);
	inline void op1000(UINT16 opcode);
	inline void op1001(UINT16 opcode);
	inline void op1010(UINT16 opcode);
	inline void op1011(UINT16 opcode);
	inline void op1100(UINT16 opcode);
	inline void op1101(UINT16 opcode);
	inline void op1110(UINT16 opcode);
	inline void op1111(UINT16 opcode);
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
		UINT32          cycles;                     /* accumulated cycles */
		UINT8           checkints;                  /* need to check interrupts before next instruction */
		uml::code_label  labelnum;                   /* index for local labels */
	};

	inline UINT32 epc(const opcode_desc *desc);
	inline void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	inline void load_fast_iregs(drcuml_block *block);
	inline void save_fast_iregs(drcuml_block *block);

	void code_flush_cache();
	void execute_run_drc();
	void code_compile_block(UINT8 mode, offs_t pc);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(int size, int iswrite, const char *name, uml::code_handle **handleptr);
	const char *log_desc_flags_to_string(UINT32 flags);
	void log_register_list(drcuml_state *drcuml, const char *string, const UINT32 *reglist, const UINT32 *regnostarlist);
	void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);
	void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, int allow_exception);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc);
	void generate_delay_slot(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc);
	int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 ovrpc);
	int generate_group_0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
	int generate_group_2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
	int generate_group_3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, UINT32 ovrpc);
	int generate_group_4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
	int generate_group_6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
	int generate_group_8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);
	int generate_group_12(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT16 opcode, int in_delay_slot, UINT32 ovrpc);

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
	sh2a_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

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
	UINT16 m_sh7021_regs[0x200];
	struct
	{
		UINT32              sar;    /**< Source Address Register */
		UINT32              dar;    /**< Destination Address Register */
		UINT16              tcr;    /**< Transfer Count Register */
		UINT16              chcr;   /**< Channel Control Register */
	} m_dma[4];
	UINT16 m_dmaor;                 /**< DMA Operation Register (status flags) */

};

class sh1_device : public sh2_device
{
public:
	// construction/destruction
	sh1_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	DECLARE_READ16_MEMBER(sh7032_r);
	DECLARE_WRITE16_MEMBER(sh7032_w);
private:
	UINT16 m_sh7032_regs[0x200];
};


class sh2_frontend : public drc_frontend
{
public:
	sh2_frontend(sh2_device *device, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

protected:
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_2(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_3(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_6(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_8(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_12(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);

	sh2_device *m_sh2;
};


extern const device_type SH1;
extern const device_type SH2;
extern const device_type SH2A;


#endif /* __SH2_H__ */
