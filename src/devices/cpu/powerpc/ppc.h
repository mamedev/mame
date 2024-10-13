// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppc.h

    Interface file for the universal machine language-based
    PowerPC emulator.

***************************************************************************/
#ifndef MAME_CPU_POWERPC_PPC_H
#define MAME_CPU_POWERPC_PPC_H

#pragma once

#include "ppc_dasm.h"

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

#include "divtlb.h"


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

class ppc_device : public cpu_device, public device_vtlb_interface
{
protected:
	class frontend;

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

	// construction/destruction
	ppc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int address_bits, int data_bits, powerpc_flavor flavor, uint32_t cap, uint32_t tb_divisor, address_map_constructor internal_map);

public:
	virtual ~ppc_device() override;

	void set_cache_dirty() { m_cache_dirty = true; }
	void set_bus_frequency(uint32_t bus_frequency) { c_bus_frequency = bus_frequency; }
	void set_bus_frequency(const XTAL &xtal) { set_bus_frequency(xtal.value()); }

	void set_serial_clock(uint32_t serial_clock) { c_serial_clock = serial_clock; }
	void set_serial_clock(const XTAL &xtal) { set_serial_clock(xtal.value()); }

	void ppc_set_dcstore_callback(write32sm_delegate callback);

	void ppcdrc_set_options(uint32_t options);
	void ppcdrc_add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base);
	void ppcdrc_add_hotspot(offs_t pc, uint32_t opcode, uint32_t cycles);

	TIMER_CALLBACK_MEMBER(decrementer_int_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_buffered_dma_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_fit_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_pit_callback);
	TIMER_CALLBACK_MEMBER(ppc4xx_spu_callback);

	void ppc_cfunc_printf_exception();
	void ppc_cfunc_printf_debug();
	void ppc_cfunc_printf_probe();
	void ppc_cfunc_unimplemented();
	void ppc_cfunc_ppccom_mismatch();
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
	void ppccom_get_dsisr();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 40; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface overrides
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

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
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache32;
	memory_access<32, 3, 0, ENDIANNESS_BIG>::cache m_cache64;
	uint32_t c_bus_frequency;
	uint32_t c_serial_clock;

	struct internal_ppc_state
	{
		uint32_t pc;
		uint32_t r[32];
		double f[32];
		uint32_t cr[8];
		uint32_t fpscr;
		uint32_t msr;
		uint32_t xerso;
		uint32_t sr[16];
		uint32_t spr[1024];
		int icount;
		uint32_t mode;                       /* current global mode */
		uint32_t irq_pending;
		/* parameters for calls */
		uint32_t param0;
		uint32_t param1;
		/* PowerPC 603-specific state */
		uint32_t mmu603_cmp;
		uint32_t mmu603_hash[2];
		uint32_t mmu603_r[4];
		/* parameters for subroutines */
		uint32_t       tempaddr;                   /* temporary address storage */
		drcuml_ireg  tempdata;                   /* temporary data storage */
		uint32_t       updateaddr;                 /* update address storage */
		uint32_t       swcount;                    /* counter for sw instructions */
		const char * format;                     /* format string for printing */
		uint32_t       arg0;                       /* print_debug argument 1 */
		double       fp0;                        /* floating point 0 */
	};

	internal_ppc_state *m_core;

	int m_ppc_tb_base_icount;
	int m_ppc_dec_base_icount;
	int m_ppc_dec_trigger_cycle;
	int m_bus_freq_multiplier;

	uint32_t m_npc;
	uint32_t m_dcr[256];

	uint32_t m_lr;
	uint32_t m_ctr;
	uint32_t m_xer;
	uint32_t m_pvr;
	uint32_t m_srr0;
	uint32_t m_srr1;
	uint32_t m_srr2;
	uint32_t m_srr3;
	uint32_t m_hid0;
	uint32_t m_hid1;
	uint32_t m_hid2;
	uint32_t m_sdr1;
	uint32_t m_sprg[4];

	uint32_t m_dsisr;
	uint32_t m_dar;
	uint32_t m_ear;
	uint32_t m_dmiss;
	uint32_t m_dcmp;
	uint32_t m_hash1;
	uint32_t m_hash2;
	uint32_t m_imiss;
	uint32_t m_icmp;
	uint32_t m_rpa;

	struct BATENT {
		uint32_t u;
		uint32_t l;
	};

	BATENT m_ibat[4];
	BATENT m_dbat[4];

	uint32_t m_evpr;
	uint32_t m_exier;
	uint32_t m_exisr;
	uint32_t m_bear;
	uint32_t m_besr;
	uint32_t m_iocr;
	uint32_t m_br[8];
	uint32_t m_iabr;
	uint32_t m_esr;
	uint32_t m_iccr;
	uint32_t m_dccr;
	uint32_t m_pit;
	uint32_t m_pit_counter;
	uint32_t m_pit_int_enable;
	uint32_t m_tsr;
	uint32_t m_dbsr;
	uint32_t m_sgr;
	uint32_t m_pid;
	uint32_t m_pbl1;
	uint32_t m_pbl2;
	uint32_t m_pbu1;
	uint32_t m_pbu2;
	uint32_t m_fit_bit;
	uint32_t m_fit_int_enable;
	uint32_t m_wdt_bit;
	uint32_t m_wdt_int_enable;
	uint32_t m_dac1;
	uint32_t m_dac2;
	uint32_t m_iac1;
	uint32_t m_iac2;

	struct SPU_REGS {
		uint8_t spls;
		uint8_t sphs;
		uint16_t brd;
		uint8_t spctl;
		uint8_t sprc;
		uint8_t sptc;
		uint8_t sprb;
		uint8_t sptb;
		emu_timer *rx_timer;
		emu_timer *tx_timer;
	};

	SPU_REGS m_spu_old;

	struct DMA_REGS {
		uint32_t cr;
		uint32_t da;
		uint32_t sa;
		uint32_t ct;
		uint32_t cc;
	};

	DMA_REGS m_dma[4];
	uint32_t m_dmasr;

	int m_reserved;
	uint32_t m_reserved_address;

	int m_interrupt_pending;

	uint64_t m_tb;          /* 56-bit timebase register */

	// STUFF added for the 6xx series
	uint32_t m_dec, m_dec_frac;

	union FPR {
		uint64_t  id;
		double  fd;
	};

	union FPR32 {
		uint32_t i;
		float f;
	};

	FPR m_fpr[32];

	int m_is603;
	int m_is602;

	/* PowerPC 602 specific registers */
	uint32_t m_lt;
	uint32_t m_sp;
	uint32_t m_tcr;
	uint32_t m_ibr;
	uint32_t m_esasrr;
	uint32_t m_sebr;
	uint32_t m_ser;

	/* architectural distinctions */
	powerpc_flavor  m_flavor;
	uint32_t          m_cap;
	uint8_t           m_cache_line_size;
	uint32_t          m_tb_divisor;

	/* PowerPC 4xx-specific state */
	/* PowerPC 4XX-specific serial port state */
	struct ppc4xx_spu_state
	{
		ppc4xx_spu_state(device_t &owner) : tx_cb(owner) { }

		uint8_t           regs[9];
		uint8_t           txbuf;
		uint8_t           rxbuf;
		emu_timer *     timer;
		uint8_t           rxbuffer[256];
		uint32_t          rxin, rxout;
		write8smo_delegate tx_cb;
	};

	ppc4xx_spu_state m_spu;
	emu_timer *     m_fit_timer;
	emu_timer *     m_pit_timer;
	emu_timer *     m_wdog_timer;
	uint32_t          m_pit_reload;
	uint32_t          m_irqstate;
	emu_timer *     m_buffered_dma_timer[4];
	int             m_buffered_dma_rate[4];

	/* internal stuff */
	std::function<u32 (offs_t)> m_pr32;
	std::function<const void * (offs_t)> m_prptr;

	uint32_t          m_system_clock;
	uint32_t          m_cpu_clock;
	uint32_t          m_serial_clock;
	uint64_t          m_tb_zero_cycles;
	uint64_t          m_dec_zero_cycles;
	emu_timer *     m_decrementer_int_timer;


	read32sm_delegate  m_dcr_read_func;
	write32sm_delegate m_dcr_write_func;

	write32sm_delegate m_dcstore_cb;

	read32_delegate::array<4> m_ext_dma_read_cb;
	write32sm_delegate::array<4> m_ext_dma_write_cb;

	/* PowerPC function pointers for memory accesses/exceptions */
#ifdef PPC_H_INCLUDED_FROM_PPC_C
	jmp_buf m_exception_jmpbuf;
#endif
	uint8_t (*m_ppcread8)(address_space &space, offs_t address);
	uint16_t (*m_ppcread16)(address_space &space, offs_t address);
	uint32_t (*m_ppcread32)(address_space &space, offs_t address);
	uint64_t (*m_ppcread64)(address_space &space, offs_t address);
	void (*m_ppcwrite8)(address_space &space, offs_t address, uint8_t data);
	void (*m_ppcwrite16)(address_space &space, offs_t address, uint16_t data);
	void (*m_ppcwrite32)(address_space &space, offs_t address, uint32_t data);
	void (*m_ppcwrite64)(address_space &space, offs_t address, uint64_t data);
	uint16_t (*m_ppcread16_unaligned)(address_space &space, offs_t address);
	uint32_t (*m_ppcread32_unaligned)(address_space &space, offs_t address);
	uint64_t (*m_ppcread64_unaligned)(address_space &space, offs_t address);
	void (*m_ppcwrite16_unaligned)(address_space &space, offs_t address, uint16_t data);
	void (*m_ppcwrite32_unaligned)(address_space &space, offs_t address, uint32_t data);
	void (*m_ppcwrite64_unaligned)(address_space &space, offs_t address, uint64_t data);

	void (*m_optable19[1024])(uint32_t);
	void (*m_optable31[1024])(uint32_t);
	void (*m_optable59[1024])(uint32_t);
	void (*m_optable63[1024])(uint32_t);
	void (*m_optable[64])(uint32_t);

	/* core state */
	drc_cache           m_cache;                      /* pointer to the DRC code cache */
	std::unique_ptr<drcuml_state>      m_drcuml;                     /* DRC UML generator state */
	std::unique_ptr<frontend>          m_drcfe;                      /* pointer to the DRC front-end state */
	uint32_t              m_drcoptions;                 /* configurable DRC options */

	/* parameters for subroutines */
	uint32_t              m_arg1;                       /* print_debug argument 2 */

	/* tables */
	uint8_t               m_fpmode[4];                  /* FPU mode table */
	uint8_t               m_sz_cr_table[32];            /* SZ CR table */
	uint8_t               m_cmp_cr_table[32];           /* CMP CR table */
	uint8_t               m_cmpl_cr_table[32];          /* CMPL CR table */
	uint8_t               m_fcmp_cr_table[32];          /* FCMP CR table */

	/* internal stuff */
	uint8_t               m_cache_dirty;                /* true if we need to flush the cache */

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
		bool                readonly;                   /* true if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	};

	uint32_t              m_fastram_select;
	fast_ram_info       m_fastram[PPC_MAX_FASTRAM];

	/* hotspots */
	/* hotspot info */
	struct hotspot_info
	{
		offs_t              pc;                         /* PC to consider */
		uint32_t              opcode;                     /* required opcode at that PC */
		uint32_t              cycles;                     /* number of cycles to eat when hit */
	};
	uint32_t              m_hotspot_select;
	hotspot_info        m_hotspot[PPC_MAX_HOTSPOTS];

	uint64_t m_debugger_temp;

	/* internal compiler state */
	struct compiler_state
	{
		uint32_t              cycles;                     /* accumulated cycles */
		uint8_t               checkints;                  /* need to check interrupts before next instruction */
		uint8_t               checksoftints;              /* need to check software interrupts before next instruction */
		uml::code_label  labelnum;                   /* index for local labels */
	};

	uint32_t get_cr();
	void set_cr(uint32_t value);
	uint32_t get_xer();
	void set_xer(uint32_t value);
	uint64_t get_timebase();
	void set_timebase(uint64_t newtb);
	uint32_t get_decrementer();
	void set_decrementer(uint32_t newdec);
	uint32_t ppccom_translate_address_internal(int intention, bool debug, offs_t &address);
	void ppc4xx_set_irq_line(uint32_t bitmask, int state);
	int ppc4xx_get_irq_line(uint32_t bitmask);
	void ppc4xx_dma_update_irq_states();
	bool ppc4xx_dma_decrement_count(int dmachan);
	bool ppc4xx_dma_fetch_transmit_byte(int dmachan, uint8_t *byte);
	bool ppc4xx_dma_handle_receive_byte(int dmachan, uint8_t byte);
	void ppc4xx_dma_exec(int dmachan);
	void ppc4xx_spu_update_irq_states();
	void ppc4xx_spu_rx_data(uint8_t data);
	void ppc4xx_spu_timer_reset();
	void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	void load_fast_iregs(drcuml_block &block);
	void save_fast_iregs(drcuml_block &block);
	void load_fast_fregs(drcuml_block &block);
	void save_fast_fregs(drcuml_block &block);
	uint32_t compute_rlw_mask(uint8_t mb, uint8_t me);
	uint32_t compute_crf_mask(uint8_t crm);
	uint32_t compute_spr(uint32_t spr);
	void code_flush_cache();
	void code_compile_block(uint8_t mode, offs_t pc);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_tlb_mismatch();
	void static_generate_exception(uint8_t exception, int recover, const char *name);
	void static_generate_memory_accessor(int mode, int size, int iswrite, int ismasked, const char *name, uml::code_handle *&handleptr, uml::code_handle *masked);
	void static_generate_swap_tgpr();
	void static_generate_lsw_entries(int mode);
	void static_generate_stsw_entries(int mode);
	void generate_update_mode(drcuml_block &block);
	void generate_update_cycles(drcuml_block &block, compiler_state *compiler, uml::parameter param, bool allow_exception);
	void generate_checksum_block(drcuml_block &block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_compute_flags(drcuml_block &block, const opcode_desc *desc, int updatecr, uint32_t xermask, int invertcarry);
	void generate_shift_flags(drcuml_block &block, const opcode_desc *desc, uint32_t op);
	void generate_fp_flags(drcuml_block &block, const opcode_desc *desc, int updatefprf);
	void generate_branch(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc, int source, uint8_t link);
	void generate_branch_bo(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc, uint32_t bo, uint32_t bi, int source, int link);
	bool generate_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	bool generate_instruction_13(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	bool generate_instruction_1f(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	bool generate_instruction_3b(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	bool generate_instruction_3f(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op);
	const char *log_desc_flags_to_string(uint32_t flags);
	void log_register_list(const char *string, const uint32_t *reglist, const uint32_t *regnostarlist);
	void log_opcode_desc(const opcode_desc *desclist, int indent);

private:
	powerpc_disassembler m_dasm;
};


//class ppc403_device : public ppc_device
//{
//public:
//  ppc403_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
//};
//
//
//class ppc405_device : public ppc_device
//{
//public:
//  ppc405_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
//};


class ppc603_device : public ppc_device
{
public:
	ppc603_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc603e_device : public ppc_device
{
public:
	ppc603e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc603r_device : public ppc_device
{
public:
	ppc603r_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc602_device : public ppc_device
{
public:
	ppc602_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mpc8240_device : public ppc_device
{
public:
	mpc8240_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc601_device : public ppc_device
{
public:
	ppc601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc604_device : public ppc_device
{
public:
	ppc604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ppc740_device : public ppc_device
{
public:
	ppc740_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ppc750_device : public ppc_device
{
public:
	ppc750_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ppc4xx_device : public ppc_device
{
public:
	void ppc4xx_spu_set_tx_handler(write8smo_delegate callback);
	void ppc4xx_spu_receive_byte(uint8_t byteval);

	void ppc4xx_set_dma_read_handler(int channel, read32_delegate callback, int rate);
	void ppc4xx_set_dma_write_handler(int channel, write32sm_delegate callback, int rate);
	void ppc4xx_set_dcr_read_handler(read32sm_delegate dcr_read_func);
	void ppc4xx_set_dcr_write_handler(write32sm_delegate dcr_write_func);

	uint8_t ppc4xx_spu_r(offs_t offset);
	void ppc4xx_spu_w(offs_t offset, uint8_t data);

	void internal_ppc4xx(address_map &map) ATTR_COLD;
protected:
	ppc4xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, powerpc_flavor flavor, uint32_t cap, uint32_t tb_divisor);

	virtual void execute_set_input(int inputnum, int state) override;
};


class ppc403ga_device : public ppc4xx_device
{
public:
	ppc403ga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc403gcx_device : public ppc4xx_device
{
public:
	ppc403gcx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ppc405gp_device : public ppc4xx_device
{
public:
	ppc405gp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(PPC601,    ppc601_device)
DECLARE_DEVICE_TYPE(PPC602,    ppc602_device)
DECLARE_DEVICE_TYPE(PPC603,    ppc603_device)
DECLARE_DEVICE_TYPE(PPC603E,   ppc603e_device)
DECLARE_DEVICE_TYPE(PPC603R,   ppc603r_device)
DECLARE_DEVICE_TYPE(PPC604,    ppc604_device)
DECLARE_DEVICE_TYPE(MPC8240,   mpc8240_device)
DECLARE_DEVICE_TYPE(PPC403GA,  ppc403ga_device)
DECLARE_DEVICE_TYPE(PPC403GCX, ppc403gcx_device)
DECLARE_DEVICE_TYPE(PPC405GP,  ppc405gp_device)
DECLARE_DEVICE_TYPE(PPC740,    ppc740_device)
DECLARE_DEVICE_TYPE(PPC750,    ppc750_device)

#endif  // MAME_CPU_POWERPC_PPC_H
