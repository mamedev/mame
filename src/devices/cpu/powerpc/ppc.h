// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppc.h

    Interface file for the universal machine language-based
    PowerPC emulator.

***************************************************************************/

#pragma once

#ifndef __PPC_H__
#define __PPC_H__

#include "cpu/vtlb.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* general constants */
#define PPC_MAX_FASTRAM         4
#define PPC_MAX_HOTSPOTS        16


/* interrupt types */
#define PPC_IRQ                 0       /* external IRQ */
#define PPC_IRQ_LINE_0          0       /* (4XX) external IRQ0 */
#define PPC_IRQ_LINE_1          1       /* (4XX) external IRQ1 */
#define PPC_IRQ_LINE_2          2       /* (4XX) external IRQ2 */
#define PPC_IRQ_LINE_3          3       /* (4XX) external IRQ3 */
#define PPC_IRQ_LINE_4          4       /* (4XX) external IRQ4 */


/* register enumeration */
enum
{
	PPC_PC = 1,
	PPC_R0,
	PPC_R1,
	PPC_R2,
	PPC_R3,
	PPC_R4,
	PPC_R5,
	PPC_R6,
	PPC_R7,
	PPC_R8,
	PPC_R9,
	PPC_R10,
	PPC_R11,
	PPC_R12,
	PPC_R13,
	PPC_R14,
	PPC_R15,
	PPC_R16,
	PPC_R17,
	PPC_R18,
	PPC_R19,
	PPC_R20,
	PPC_R21,
	PPC_R22,
	PPC_R23,
	PPC_R24,
	PPC_R25,
	PPC_R26,
	PPC_R27,
	PPC_R28,
	PPC_R29,
	PPC_R30,
	PPC_R31,
	PPC_CR,
	PPC_LR,
	PPC_CTR,
	PPC_XER,

	PPC_F0,
	PPC_F1,
	PPC_F2,
	PPC_F3,
	PPC_F4,
	PPC_F5,
	PPC_F6,
	PPC_F7,
	PPC_F8,
	PPC_F9,
	PPC_F10,
	PPC_F11,
	PPC_F12,
	PPC_F13,
	PPC_F14,
	PPC_F15,
	PPC_F16,
	PPC_F17,
	PPC_F18,
	PPC_F19,
	PPC_F20,
	PPC_F21,
	PPC_F22,
	PPC_F23,
	PPC_F24,
	PPC_F25,
	PPC_F26,
	PPC_F27,
	PPC_F28,
	PPC_F29,
	PPC_F30,
	PPC_F31,
	PPC_FPSCR,

	PPC_MSR,
	PPC_SRR0,
	PPC_SRR1,
	PPC_SPRG0,
	PPC_SPRG1,
	PPC_SPRG2,
	PPC_SPRG3,
	PPC_SDR1,
	PPC_EXIER,
	PPC_EXISR,
	PPC_EVPR,
	PPC_IOCR,
	PPC_TBL,
	PPC_TBH,
	PPC_DEC,

	PPC_SR0,
	PPC_SR1,
	PPC_SR2,
	PPC_SR3,
	PPC_SR4,
	PPC_SR5,
	PPC_SR6,
	PPC_SR7,
	PPC_SR8,
	PPC_SR9,
	PPC_SR10,
	PPC_SR11,
	PPC_SR12,
	PPC_SR13,
	PPC_SR14,
	PPC_SR15
};


/* compiler-specific options */
#define PPCDRC_STRICT_VERIFY        0x0001          /* verify all instructions */
#define PPCDRC_FLUSH_PC             0x0002          /* flush the PC value before each memory access */
#define PPCDRC_ACCURATE_SINGLES     0x0004          /* do excessive rounding to make single-precision results "accurate" */


/* common sets of options */
#define PPCDRC_COMPATIBLE_OPTIONS   (PPCDRC_STRICT_VERIFY | PPCDRC_FLUSH_PC | PPCDRC_ACCURATE_SINGLES)
#define PPCDRC_FASTEST_OPTIONS      (0)



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

#define MCFG_PPC_BUS_FREQUENCY(_frequency) \
	ppc_device::set_bus_frequency(*device, _frequency);


class ppc_frontend;


class ppc_device : public cpu_device
{
	friend class ppc_frontend;

protected:
	/* PowerPC flavors */
	enum powerpc_flavor
	{
		PPC_MODEL_403GA             = 0x00200000,
		PPC_MODEL_403GB             = 0x00200100,
		PPC_MODEL_403GC             = 0x00200200,
		PPC_MODEL_403GCX            = 0x00201400,
		PPC_MODEL_405GP             = 0x40110000,
		PPC_MODEL_601               = 0x00010000,
		PPC_MODEL_603               = 0x00030000,   /* "Wart" */
		PPC_MODEL_604               = 0x00040000,   /* "Zephyr" */
		PPC_MODEL_602               = 0x00050200,   /* "Galahad" */
		PPC_MODEL_603E              = 0x00060103,   /* "Stretch", version 1.3 */
		PPC_MODEL_603EV             = 0x00070000,   /* "Valiant" */
		PPC_MODEL_603R              = 0x00071202,   /* "Goldeneye", version 2.1 */
		PPC_MODEL_740               = 0x00080301,   /* "Arthur", version 3.1 */
		PPC_MODEL_750               = PPC_MODEL_740,
		PPC_MODEL_740P              = 0x00080202,   /* "Conan Doyle", version 1.2 */
		PPC_MODEL_750P              = PPC_MODEL_740P,
		PPC_MODEL_755               = 0x00083203,   /* "Goldfinger", version 2.3 */
		PPC_MODEL_7400              = 0x000c0209,   /* "Max", version 2.9 */
		PPC_MODEL_7410              = 0x800c1104,   /* "Nitro", version 3.4 */
		PPC_MODEL_7450              = 0x80000201,   /* "Vger", version 2.1 */
		PPC_MODEL_7451              = 0x80000203,   /* "Vger", version 2.3 */
		PPC_MODEL_7441              = PPC_MODEL_7451,
		PPC_MODEL_7455              = 0x80010303,   /* "Apollo 6", version 3.3 */
		PPC_MODEL_7445              = PPC_MODEL_7455,
		PPC_MODEL_7457              = 0x80020101,   /* "Apollo 7", version 1.1 */
		PPC_MODEL_MPC8240           = 0x00810101,   /* "Kahlua" */
		PPC_MODEL_MPC8241           = 0x80811014,   /* "Kahlua Lt" */
		PPC_MODEL_MPC8245           = 0x80811014    /* "Kahlua II" */
	};

public:
	// construction/destruction
	ppc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int address_bits, int data_bits, powerpc_flavor flavor, UINT32 cap, UINT32 tb_divisor, address_map_constructor internal_map);

	static void set_bus_frequency(device_t &device, UINT32 bus_frequency) { downcast<ppc_device &>(device).c_bus_frequency = bus_frequency; }

	void ppc_set_dcstore_callback(write32_delegate callback);

	void ppcdrc_set_options(UINT32 options);
	void ppcdrc_add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base);
	void ppcdrc_add_hotspot(offs_t pc, UINT32 opcode, UINT32 cycles);

	TIMER_CALLBACK_MEMBER(decrementer_int_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_buffered_dma_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_fit_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_pit_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_spu_callback);

	void ppc_cfunc_printf_exception();
	void ppc_cfunc_printf_debug();
	void ppc_cfunc_printf_probe();
	void ppc_cfunc_unimplemented();
	void ppccom_tlb_fill();
	void ppccom_update_fprf();
	void ppccom_dcstore_callback();
	void ppccom_execute_tlbie();
	void ppccom_execute_tlbia();
	void ppccom_execute_tlbl();
	void ppccom_execute_mfspr();
	void ppccom_execute_mftb();
	void ppccom_execute_mtspr();
	void ppccom_tlb_flush();
	void ppccom_execute_mfdcr();
	void ppccom_execute_mtdcr();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 40; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }
	virtual bool memory_translate(address_spacenum spacenum, int intention, offs_t &address) override;

	// device_state_interface overrides
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	/* exception types */
	enum
	{
		EXCEPTION_RESET         = 1,
		EXCEPTION_MACHCHECK     = 2,
		EXCEPTION_DSI           = 3,        /* PPCCAP_OEA */
		EXCEPTION_PROTECTION    = 3,        /* PPCCAP_4XX */
		EXCEPTION_ISI           = 4,
		EXCEPTION_EI            = 5,
		EXCEPTION_ALIGN         = 6,
		EXCEPTION_PROGRAM       = 7,
		EXCEPTION_NOFPU         = 8,
		EXCEPTION_DECREMENT     = 9,
		EXCEPTION_SYSCALL       = 12,
		EXCEPTION_TRACE         = 13,
		EXCEPTION_FPASSIST      = 14,
		EXCEPTION_ITLBMISS      = 16,       /* PPCCAP_603_MMU */
		EXCEPTION_DTLBMISSL     = 17,       /* PPCCAP_603_MMU */
		EXCEPTION_DTLBMISSS     = 18,       /* PPCCAP_603_MMU */
		EXCEPTION_COUNT
	};

	address_space_config m_program_config;
	address_space *m_program;
	UINT32 c_bus_frequency;

	struct internal_ppc_state
	{
		UINT32 pc;
		UINT32 r[32];
		double f[32];
		UINT32 cr[8];
		UINT32 fpscr;
		UINT32 msr;
		UINT32 xerso;
		UINT32 sr[16];
		UINT32 spr[1024];
		int icount;
		UINT32 mode;                       /* current global mode */
		UINT32 irq_pending;
		/* parameters for calls */
		UINT32 param0;
		UINT32 param1;
		/* PowerPC 603-specific state */
		UINT32 mmu603_cmp;
		UINT32 mmu603_hash[2];
		UINT32 mmu603_r[4];
		/* parameters for subroutines */
		UINT32       tempaddr;                   /* temporary address storage */
		drcuml_ireg  tempdata;                   /* temporary data storage */
		UINT32       updateaddr;                 /* update address storage */
		UINT32       swcount;                    /* counter for sw instructions */
		const char * format;                     /* format string for printing */
		UINT32       arg0;                       /* print_debug argument 1 */
		double       fp0;                        /* floating point 0 */
	};

	internal_ppc_state *m_core;

	int m_ppc_tb_base_icount;
	int m_ppc_dec_base_icount;
	int m_ppc_dec_trigger_cycle;
	int m_bus_freq_multiplier;

	UINT32 m_npc;
	UINT32 m_dcr[256];

	UINT32 m_lr;
	UINT32 m_ctr;
	UINT32 m_xer;
	UINT32 m_pvr;
	UINT32 m_srr0;
	UINT32 m_srr1;
	UINT32 m_srr2;
	UINT32 m_srr3;
	UINT32 m_hid0;
	UINT32 m_hid1;
	UINT32 m_hid2;
	UINT32 m_sdr1;
	UINT32 m_sprg[4];

	UINT32 m_dsisr;
	UINT32 m_dar;
	UINT32 m_ear;
	UINT32 m_dmiss;
	UINT32 m_dcmp;
	UINT32 m_hash1;
	UINT32 m_hash2;
	UINT32 m_imiss;
	UINT32 m_icmp;
	UINT32 m_rpa;

	struct BATENT {
		UINT32 u;
		UINT32 l;
	};

	BATENT m_ibat[4];
	BATENT m_dbat[4];

	UINT32 m_evpr;
	UINT32 m_exier;
	UINT32 m_exisr;
	UINT32 m_bear;
	UINT32 m_besr;
	UINT32 m_iocr;
	UINT32 m_br[8];
	UINT32 m_iabr;
	UINT32 m_esr;
	UINT32 m_iccr;
	UINT32 m_dccr;
	UINT32 m_pit;
	UINT32 m_pit_counter;
	UINT32 m_pit_int_enable;
	UINT32 m_tsr;
	UINT32 m_dbsr;
	UINT32 m_sgr;
	UINT32 m_pid;
	UINT32 m_pbl1;
	UINT32 m_pbl2;
	UINT32 m_pbu1;
	UINT32 m_pbu2;
	UINT32 m_fit_bit;
	UINT32 m_fit_int_enable;
	UINT32 m_wdt_bit;
	UINT32 m_wdt_int_enable;
	UINT32 m_dac1;
	UINT32 m_dac2;
	UINT32 m_iac1;
	UINT32 m_iac2;

	struct SPU_REGS {
		UINT8 spls;
		UINT8 sphs;
		UINT16 brd;
		UINT8 spctl;
		UINT8 sprc;
		UINT8 sptc;
		UINT8 sprb;
		UINT8 sptb;
		emu_timer *rx_timer;
		emu_timer *tx_timer;
	};

	SPU_REGS m_spu_old;

	struct DMA_REGS {
		UINT32 cr;
		UINT32 da;
		UINT32 sa;
		UINT32 ct;
		UINT32 cc;
	};

	DMA_REGS m_dma[4];
	UINT32 m_dmasr;

	int m_reserved;
	UINT32 m_reserved_address;

	int m_interrupt_pending;

	UINT64 m_tb;          /* 56-bit timebase register */

	// STUFF added for the 6xx series
	UINT32 m_dec, m_dec_frac;

	union FPR {
		UINT64  id;
		double  fd;
	};

	union FPR32 {
		UINT32 i;
		float f;
	};

	FPR m_fpr[32];

	int m_is603;
	int m_is602;

	/* PowerPC 602 specific registers */
	UINT32 m_lt;
	UINT32 m_sp;
	UINT32 m_tcr;
	UINT32 m_ibr;
	UINT32 m_esasrr;
	UINT32 m_sebr;
	UINT32 m_ser;

	/* MMU */
	vtlb_state *m_vtlb;

	/* architectural distinctions */
	powerpc_flavor  m_flavor;
	UINT32          m_cap;
	UINT8           m_cache_line_size;
	UINT32          m_tb_divisor;

	/* PowerPC 4xx-specific state */
	/* PowerPC 4XX-specific serial port state */
	struct ppc4xx_spu_state
	{
		UINT8           regs[9];
		UINT8           txbuf;
		UINT8           rxbuf;
		emu_timer *     timer;
		UINT8           rxbuffer[256];
		UINT32          rxin, rxout;
		write8_delegate tx_cb;
	};

	ppc4xx_spu_state m_spu;
	emu_timer *     m_fit_timer;
	emu_timer *     m_pit_timer;
	emu_timer *     m_wdog_timer;
	UINT32          m_pit_reload;
	UINT32          m_irqstate;
	emu_timer *     m_buffered_dma_timer[4];
	int             m_buffered_dma_rate[4];

	/* internal stuff */
	direct_read_data *m_direct;
	offs_t          m_codexor;
	UINT32          m_system_clock;
	UINT32          m_cpu_clock;
	UINT64          m_tb_zero_cycles;
	UINT64          m_dec_zero_cycles;
	emu_timer *     m_decrementer_int_timer;

	read32_delegate  m_dcr_read_func;
	write32_delegate m_dcr_write_func;

	write32_delegate m_dcstore_cb;

	read32_delegate m_ext_dma_read_cb[4];
	write32_delegate m_ext_dma_write_cb[4];

	/* PowerPC function pointers for memory accesses/exceptions */
#ifdef PPC_H_INCLUDED_FROM_PPC_C
	jmp_buf m_exception_jmpbuf;
#endif
	UINT8 (*m_ppcread8)(address_space &space, offs_t address);
	UINT16 (*m_ppcread16)(address_space &space, offs_t address);
	UINT32 (*m_ppcread32)(address_space &space, offs_t address);
	UINT64 (*m_ppcread64)(address_space &space, offs_t address);
	void (*m_ppcwrite8)(address_space &space, offs_t address, UINT8 data);
	void (*m_ppcwrite16)(address_space &space, offs_t address, UINT16 data);
	void (*m_ppcwrite32)(address_space &space, offs_t address, UINT32 data);
	void (*m_ppcwrite64)(address_space &space, offs_t address, UINT64 data);
	UINT16 (*m_ppcread16_unaligned)(address_space &space, offs_t address);
	UINT32 (*m_ppcread32_unaligned)(address_space &space, offs_t address);
	UINT64 (*m_ppcread64_unaligned)(address_space &space, offs_t address);
	void (*m_ppcwrite16_unaligned)(address_space &space, offs_t address, UINT16 data);
	void (*m_ppcwrite32_unaligned)(address_space &space, offs_t address, UINT32 data);
	void (*m_ppcwrite64_unaligned)(address_space &space, offs_t address, UINT64 data);

	void (*m_optable19[1024])(UINT32);
	void (*m_optable31[1024])(UINT32);
	void (*m_optable59[1024])(UINT32);
	void (*m_optable63[1024])(UINT32);
	void (*m_optable[64])(UINT32);

	/* core state */
	drc_cache           m_cache;                      /* pointer to the DRC code cache */
	std::unique_ptr<drcuml_state>      m_drcuml;                     /* DRC UML generator state */
	std::unique_ptr<ppc_frontend>      m_drcfe;                      /* pointer to the DRC front-end state */
	UINT32              m_drcoptions;                 /* configurable DRC options */

	/* parameters for subroutines */
	UINT32              m_arg1;                       /* print_debug argument 2 */

	/* tables */
	UINT8               m_fpmode[4];                  /* FPU mode table */
	UINT8               m_sz_cr_table[32];            /* SZ CR table */
	UINT8               m_cmp_cr_table[32];           /* CMP CR table */
	UINT8               m_cmpl_cr_table[32];          /* CMPL CR table */
	UINT8               m_fcmp_cr_table[32];          /* FCMP CR table */

	/* internal stuff */
	UINT8               m_cache_dirty;                /* true if we need to flush the cache */

	/* register mappings */
	uml::parameter   m_regmap[32];                 /* parameter to register mappings for all 32 integer registers */
	uml::parameter   m_fdregmap[32];               /* parameter to register mappings for all 32 floating point registers */

	/* subroutines */
	uml::code_handle *   m_entry;                      /* entry point */
	uml::code_handle *   m_nocode;                     /* nocode exception handler */
	uml::code_handle *   m_out_of_cycles;              /* out of cycles exception handler */
	uml::code_handle *   m_tlb_mismatch;               /* tlb mismatch handler */
	uml::code_handle *   m_swap_tgpr;                  /* swap TGPR handler */
	uml::code_handle *   m_lsw[8][32];                 /* lsw entries */
	uml::code_handle *   m_stsw[8][32];                /* stsw entries */
	uml::code_handle *   m_read8[8];                   /* read byte */
	uml::code_handle *   m_write8[8];                  /* write byte */
	uml::code_handle *   m_read16[8];                  /* read half */
	uml::code_handle *   m_read16mask[8];              /* read half */
	uml::code_handle *   m_write16[8];                 /* write half */
	uml::code_handle *   m_write16mask[8];             /* write half */
	uml::code_handle *   m_read32[8];                  /* read word */
	uml::code_handle *   m_read32align[8];             /* read word aligned */
	uml::code_handle *   m_read32mask[8];              /* read word */
	uml::code_handle *   m_write32[8];                 /* write word */
	uml::code_handle *   m_write32align[8];            /* write word aligned */
	uml::code_handle *   m_write32mask[8];             /* write word */
	uml::code_handle *   m_read64[8];                  /* read double */
	uml::code_handle *   m_read64mask[8];              /* read double */
	uml::code_handle *   m_write64[8];                 /* write double */
	uml::code_handle *   m_write64mask[8];             /* write double */
	uml::code_handle *   m_exception[EXCEPTION_COUNT]; /* array of exception handlers */
	uml::code_handle *   m_exception_norecover[EXCEPTION_COUNT];   /* array of exception handlers */

	/* fast RAM */
	/* fast RAM info */
	struct fast_ram_info
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		UINT8               readonly;                   /* TRUE if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	};

	UINT32              m_fastram_select;
	fast_ram_info       m_fastram[PPC_MAX_FASTRAM];

	/* hotspots */
	/* hotspot info */
	struct hotspot_info
	{
		offs_t              pc;                         /* PC to consider */
		UINT32              opcode;                     /* required opcode at that PC */
		UINT32              cycles;                     /* number of cycles to eat when hit */
	};
	UINT32              m_hotspot_select;
	hotspot_info        m_hotspot[PPC_MAX_HOTSPOTS];

	UINT64 m_debugger_temp;

	/* internal compiler state */
	struct compiler_state
	{
		UINT32              cycles;                     /* accumulated cycles */
		UINT8               checkints;                  /* need to check interrupts before next instruction */
		UINT8               checksoftints;              /* need to check software interrupts before next instruction */
		uml::code_label  labelnum;                   /* index for local labels */
	};

	UINT32 get_cr();
	void set_cr(UINT32 value);
	UINT32 get_xer();
	void set_xer(UINT32 value);
	UINT64 get_timebase();
	void set_timebase(UINT64 newtb);
	UINT32 get_decrementer();
	void set_decrementer(UINT32 newdec);
	UINT32 ppccom_translate_address_internal(int intention, offs_t &address);
	void ppc4xx_set_irq_line(UINT32 bitmask, int state);
	int ppc4xx_get_irq_line(UINT32 bitmask);
	void ppc4xx_dma_update_irq_states();
	int ppc4xx_dma_decrement_count(int dmachan);
	int ppc4xx_dma_fetch_transmit_byte(int dmachan, UINT8 *byte);
	int ppc4xx_dma_handle_receive_byte(int dmachan, UINT8 byte);
	void ppc4xx_dma_exec(int dmachan);
	void ppc4xx_spu_update_irq_states();
	void ppc4xx_spu_rx_data(UINT8 data);
	void ppc4xx_spu_timer_reset();
	void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	void load_fast_iregs(drcuml_block *block);
	void save_fast_iregs(drcuml_block *block);
	UINT32 compute_rlw_mask(UINT8 mb, UINT8 me);
	UINT32 compute_crf_mask(UINT8 crm);
	UINT32 compute_spr(UINT32 spr);
	void code_flush_cache();
	void code_compile_block(UINT8 mode, offs_t pc);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_tlb_mismatch();
	void static_generate_exception(UINT8 exception, int recover, const char *name);
	void static_generate_memory_accessor(int mode, int size, int iswrite, int ismasked, const char *name, uml::code_handle *&handleptr, uml::code_handle *masked);
	void static_generate_swap_tgpr();
	void static_generate_lsw_entries(int mode);
	void static_generate_stsw_entries(int mode);
	void generate_update_mode(drcuml_block *block);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, int allow_exception);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_compute_flags(drcuml_block *block, const opcode_desc *desc, int updatecr, UINT32 xermask, int invertcarry);
	void generate_shift_flags(drcuml_block *block, const opcode_desc *desc, UINT32 op);
	void generate_fp_flags(drcuml_block *block, const opcode_desc *desc, int updatefprf);
	void generate_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int source, UINT8 link);
	void generate_branch_bo(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 bo, UINT32 bi, int source, int link);
	int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_instruction_13(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_instruction_1f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_instruction_3b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_instruction_3f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op);
	const char *log_desc_flags_to_string(UINT32 flags);
	void log_register_list(drcuml_state *drcuml, const char *string, const UINT32 *reglist, const UINT32 *regnostarlist);
	void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);

};


//class ppc403_device : public ppc_device
//{
//public:
//  ppc403_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
//
//protected:
//  virtual UINT32 execute_input_lines() const { return 8; }
//};
//
//
//class ppc405_device : public ppc_device
//{
//public:
//  ppc405_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
//
//protected:
//  virtual UINT32 execute_input_lines() const { return 8; }
//};


class ppc603_device : public ppc_device
{
public:
	ppc603_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc603e_device : public ppc_device
{
public:
	ppc603e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc603r_device : public ppc_device
{
public:
	ppc603r_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc602_device : public ppc_device
{
public:
	ppc602_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class mpc8240_device : public ppc_device
{
public:
	mpc8240_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc601_device : public ppc_device
{
public:
	ppc601_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc604_device : public ppc_device
{
public:
	ppc604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc4xx_device : public ppc_device
{
public:
	ppc4xx_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, powerpc_flavor flavor, UINT32 cap, UINT32 tb_divisor);

	void ppc4xx_spu_set_tx_handler(write8_delegate callback);
	void ppc4xx_spu_receive_byte(UINT8 byteval);

	void ppc4xx_set_dma_read_handler(int channel, read32_delegate callback, int rate);
	void ppc4xx_set_dma_write_handler(int channel, write32_delegate callback, int rate);
	void ppc4xx_set_dcr_read_handler(read32_delegate dcr_read_func);
	void ppc4xx_set_dcr_write_handler(write32_delegate dcr_write_func);

	DECLARE_READ8_MEMBER( ppc4xx_spu_r );
	DECLARE_WRITE8_MEMBER( ppc4xx_spu_w );

protected:
	virtual UINT32 execute_input_lines() const override { return 5; }
	virtual void execute_set_input(int inputnum, int state) override;
};


class ppc403ga_device : public ppc4xx_device
{
public:
	ppc403ga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc403gcx_device : public ppc4xx_device
{
public:
	ppc403gcx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class ppc405gp_device : public ppc4xx_device
{
public:
	ppc405gp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type PPC601;
extern const device_type PPC602;
extern const device_type PPC603;
extern const device_type PPC603E;
extern const device_type PPC603R;
extern const device_type PPC604;
extern const device_type MPC8240;
extern const device_type PPC403GA;
extern const device_type PPC403GCX;
extern const device_type PPC405GP;


#endif  /* __PPC_H__ */
