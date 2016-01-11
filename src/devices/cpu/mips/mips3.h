// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3.h

    Interface file for the universal machine language-based
    MIPS III/IV emulator.

***************************************************************************/

#pragma once

#ifndef __MIPS3_H__
#define __MIPS3_H__


#include "cpu/vtlb.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"


// NEC VR4300 series is MIPS III with 32-bit address bus and slightly custom COP0/TLB
extern const device_type VR4300BE;
extern const device_type VR4300LE;
// VR4310 = VR4300 with different speed bin
extern const device_type VR4310BE;
extern const device_type VR4310LE;
extern const device_type R4600BE;
extern const device_type R4600LE;
extern const device_type R4650BE;
extern const device_type R4650LE;
extern const device_type R4700BE;
extern const device_type R4700LE;
extern const device_type R5000BE;
extern const device_type R5000LE;
extern const device_type QED5271BE;
extern const device_type QED5271LE;
extern const device_type RM7000BE;
extern const device_type RM7000LE;


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MIPS3_PC = 1,
	MIPS3_R0,
	MIPS3_R1,
	MIPS3_R2,
	MIPS3_R3,
	MIPS3_R4,
	MIPS3_R5,
	MIPS3_R6,
	MIPS3_R7,
	MIPS3_R8,
	MIPS3_R9,
	MIPS3_R10,
	MIPS3_R11,
	MIPS3_R12,
	MIPS3_R13,
	MIPS3_R14,
	MIPS3_R15,
	MIPS3_R16,
	MIPS3_R17,
	MIPS3_R18,
	MIPS3_R19,
	MIPS3_R20,
	MIPS3_R21,
	MIPS3_R22,
	MIPS3_R23,
	MIPS3_R24,
	MIPS3_R25,
	MIPS3_R26,
	MIPS3_R27,
	MIPS3_R28,
	MIPS3_R29,
	MIPS3_R30,
	MIPS3_R31,
	MIPS3_HI,
	MIPS3_LO,
	MIPS3_FPR0,
	MIPS3_FPS0,
	MIPS3_FPD0,
	MIPS3_FPR1,
	MIPS3_FPS1,
	MIPS3_FPD1,
	MIPS3_FPR2,
	MIPS3_FPS2,
	MIPS3_FPD2,
	MIPS3_FPR3,
	MIPS3_FPS3,
	MIPS3_FPD3,
	MIPS3_FPR4,
	MIPS3_FPS4,
	MIPS3_FPD4,
	MIPS3_FPR5,
	MIPS3_FPS5,
	MIPS3_FPD5,
	MIPS3_FPR6,
	MIPS3_FPS6,
	MIPS3_FPD6,
	MIPS3_FPR7,
	MIPS3_FPS7,
	MIPS3_FPD7,
	MIPS3_FPR8,
	MIPS3_FPS8,
	MIPS3_FPD8,
	MIPS3_FPR9,
	MIPS3_FPS9,
	MIPS3_FPD9,
	MIPS3_FPR10,
	MIPS3_FPS10,
	MIPS3_FPD10,
	MIPS3_FPR11,
	MIPS3_FPS11,
	MIPS3_FPD11,
	MIPS3_FPR12,
	MIPS3_FPS12,
	MIPS3_FPD12,
	MIPS3_FPR13,
	MIPS3_FPS13,
	MIPS3_FPD13,
	MIPS3_FPR14,
	MIPS3_FPS14,
	MIPS3_FPD14,
	MIPS3_FPR15,
	MIPS3_FPS15,
	MIPS3_FPD15,
	MIPS3_FPR16,
	MIPS3_FPS16,
	MIPS3_FPD16,
	MIPS3_FPR17,
	MIPS3_FPS17,
	MIPS3_FPD17,
	MIPS3_FPR18,
	MIPS3_FPS18,
	MIPS3_FPD18,
	MIPS3_FPR19,
	MIPS3_FPS19,
	MIPS3_FPD19,
	MIPS3_FPR20,
	MIPS3_FPS20,
	MIPS3_FPD20,
	MIPS3_FPR21,
	MIPS3_FPS21,
	MIPS3_FPD21,
	MIPS3_FPR22,
	MIPS3_FPS22,
	MIPS3_FPD22,
	MIPS3_FPR23,
	MIPS3_FPS23,
	MIPS3_FPD23,
	MIPS3_FPR24,
	MIPS3_FPS24,
	MIPS3_FPD24,
	MIPS3_FPR25,
	MIPS3_FPS25,
	MIPS3_FPD25,
	MIPS3_FPR26,
	MIPS3_FPS26,
	MIPS3_FPD26,
	MIPS3_FPR27,
	MIPS3_FPS27,
	MIPS3_FPD27,
	MIPS3_FPR28,
	MIPS3_FPS28,
	MIPS3_FPD28,
	MIPS3_FPR29,
	MIPS3_FPS29,
	MIPS3_FPD29,
	MIPS3_FPR30,
	MIPS3_FPS30,
	MIPS3_FPD30,
	MIPS3_FPR31,
	MIPS3_FPS31,
	MIPS3_FPD31,
	MIPS3_CCR1_31,
	MIPS3_SR,
	MIPS3_EPC,
	MIPS3_CAUSE,
	MIPS3_COUNT,
	MIPS3_COMPARE,
	MIPS3_INDEX,
	MIPS3_RANDOM,
	MIPS3_ENTRYHI,
	MIPS3_ENTRYLO0,
	MIPS3_ENTRYLO1,
	MIPS3_PAGEMASK,
	MIPS3_WIRED,
	MIPS3_BADVADDR
};

#define MIPS3_MAX_FASTRAM       3
#define MIPS3_MAX_HOTSPOTS      16

/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define MIPS3_IRQ0      0       /* IRQ0 */
#define MIPS3_IRQ1      1       /* IRQ1 */
#define MIPS3_IRQ2      2       /* IRQ2 */
#define MIPS3_IRQ3      3       /* IRQ3 */
#define MIPS3_IRQ4      4       /* IRQ4 */
#define MIPS3_IRQ5      5       /* IRQ5 */



/***************************************************************************
    STRUCTURES
***************************************************************************/

/* MIPS3 TLB entry */
struct mips3_tlb_entry
{
	UINT64          page_mask;
	UINT64          entry_hi;
	UINT64          entry_lo[2];
};

/* internal compiler state */
struct compiler_state
{
	UINT32              cycles;                     /* accumulated cycles */
	UINT8               checkints;                  /* need to check interrupts before next instruction */
	UINT8               checksoftints;              /* need to check software interrupts before next instruction */
	uml::code_label  labelnum;                   /* index for local labels */
};

#define MIPS3_MAX_TLB_ENTRIES       48

#define MCFG_MIPS3_ICACHE_SIZE(_size) \
	mips3_device::set_icache_size(*device, _size);

#define MCFG_MIPS3_DCACHE_SIZE(_size) \
	mips3_device::set_dcache_size(*device, _size);

#define MCFG_MIPS3_SYSTEM_CLOCK(_clock) \
	mips3_device::set_system_clock(*device, _clock);


class mips3_frontend;

class mips3_device : public cpu_device
{
	friend class mips3_frontend;

protected:
	/* MIPS flavors */
	enum mips3_flavor
	{
		/* MIPS III variants */
		MIPS3_TYPE_MIPS_III,
		MIPS3_TYPE_VR4300,
		MIPS3_TYPE_R4600,
		MIPS3_TYPE_R4650,
		MIPS3_TYPE_R4700,

		/* MIPS IV variants */
		MIPS3_TYPE_MIPS_IV,
		MIPS3_TYPE_R5000,
		MIPS3_TYPE_QED5271,
		MIPS3_TYPE_RM7000
	};

public:
	// construction/destruction
	mips3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, mips3_flavor flavor, endianness_t endiannes);

	static void set_icache_size(device_t &device, size_t icache_size) { downcast<mips3_device &>(device).c_icache_size = icache_size; }
	static void set_dcache_size(device_t &device, size_t dcache_size) { downcast<mips3_device &>(device).c_dcache_size = dcache_size; }
	static void set_system_clock(device_t &device, UINT32 system_clock) { downcast<mips3_device &>(device).c_system_clock = system_clock; }

	TIMER_CALLBACK_MEMBER(compare_int_callback);

	void add_fastram(offs_t start, offs_t end, UINT8 readonly, void *base);
	void clear_fastram(UINT32 select_start);
	void mips3drc_set_options(UINT32 options);
	void mips3drc_add_hotspot(offs_t pc, UINT32 opcode, UINT32 cycles);
	void burn_cycles(INT32 cycles);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 40; }
	virtual UINT32 execute_input_lines() const override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual void execute_burn(INT32 cycles) override { m_totalcycles += cycles; }

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }
	virtual bool memory_translate(address_spacenum spacenum, int intention, offs_t &address) override;

	// device_state_interface overrides
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;


private:
	struct internal_mips3_state
	{
		/* core registers */
		UINT32          pc;
		int             icount;
		UINT64          r[35];

		/* COP registers */
		UINT64          cpr[3][32];
		UINT64          ccr[3][32];
		UINT32          llbit;

		UINT32          mode;                       /* current global mode */

		/* parameters for subroutines */
		UINT64          numcycles;                  /* return value from gettotalcycles */
		const char *    format;                     /* format string for print_debug */
		UINT32          arg0;                       /* print_debug argument 1 */
		UINT32          arg1;                       /* print_debug argument 2 */

		UINT64          count_zero_time;
		UINT32          compare_armed;
		UINT32          jmpdest;                    /* destination jump target */

	};

	address_space_config m_program_config;
	mips3_flavor m_flavor;

	/* core state */
	internal_mips3_state *m_core;

	/* internal stuff */
	UINT32      m_ppc;
	UINT32      m_nextpc;
	UINT32      m_pcbase;
	UINT8       m_cf[4][8];
	bool        m_delayslot;
	int         m_op;
	int         m_interrupt_cycles;
	UINT32      m_ll_value;
	UINT64      m_lld_value;
	UINT32      m_badcop_value;
	const vtlb_entry *m_tlb_table;

	/* endian-dependent load/store */
	typedef void (mips3_device::*loadstore_func)(UINT32 op);
	loadstore_func m_lwl;
	loadstore_func m_lwr;
	loadstore_func m_swl;
	loadstore_func m_swr;
	loadstore_func m_ldl;
	loadstore_func m_ldr;
	loadstore_func m_sdl;
	loadstore_func m_sdr;

	address_space *m_program;
	direct_read_data *m_direct;
	UINT32          c_system_clock;
	UINT32          m_cpu_clock;
	emu_timer *     m_compare_int_timer;

	/* derived info based on flavor */
	UINT32          m_pfnmask;
	UINT8           m_tlbentries;

	/* memory accesses */
	bool            m_bigendian;
	UINT32          m_byte_xor;
	UINT32          m_word_xor;
	data_accessors  m_memory;

	/* cache memory */
	size_t          c_icache_size;
	size_t          c_dcache_size;

	/* MMU */
	vtlb_state *    m_vtlb;
	mips3_tlb_entry m_tlb[MIPS3_MAX_TLB_ENTRIES];

	/* fast RAM */
	UINT32              m_fastram_select;
	struct
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		UINT8               readonly;                   /* TRUE if read-only */
		void *              base;                       /* base in memory where the RAM lives */
		UINT8 *             offset_base8;               /* base in memory where the RAM lives, 8-bit pointer, with the start offset pre-applied */
		UINT16 *            offset_base16;              /* base in memory where the RAM lives, 16-bit pointer, with the start offset pre-applied  */
		UINT32 *            offset_base32;              /* base in memory where the RAM lives, 32-bit pointer, with the start offset pre-applied  */
	}       m_fastram[MIPS3_MAX_FASTRAM];

	UINT32 m_debugger_temp;

	/* core state */
	drc_cache           m_cache;                      /* pointer to the DRC code cache */
	std::unique_ptr<drcuml_state>      m_drcuml;                     /* DRC UML generator state */
	std::unique_ptr<mips3_frontend>    m_drcfe;                      /* pointer to the DRC front-end state */
	UINT32              m_drcoptions;                 /* configurable DRC options */

	/* internal stuff */
	UINT8               m_cache_dirty;                /* true if we need to flush the cache */

	/* tables */
	UINT8               m_fpmode[4];                  /* FPU mode table */

	/* register mappings */
	uml::parameter   m_regmap[34];                 /* parameter to register mappings for all 32 integer registers */
	uml::parameter   m_regmaplo[34];               /* parameter to register mappings for all 32 integer registers */

	/* subroutines */
	uml::code_handle *   m_entry;                      /* entry point */
	uml::code_handle *   m_nocode;                     /* nocode exception handler */
	uml::code_handle *   m_out_of_cycles;              /* out of cycles exception handler */
	uml::code_handle *   m_tlb_mismatch;               /* tlb mismatch handler */
	uml::code_handle *   m_read8[3];                   /* read byte */
	uml::code_handle *   m_write8[3];                  /* write byte */
	uml::code_handle *   m_read16[3];                  /* read half */
	uml::code_handle *   m_write16[3];                 /* write half */
	uml::code_handle *   m_read32[3];                  /* read word */
	uml::code_handle *   m_read32mask[3];              /* read word masked */
	uml::code_handle *   m_write32[3];                 /* write word */
	uml::code_handle *   m_write32mask[3];             /* write word masked */
	uml::code_handle *   m_read64[3];                  /* read double */
	uml::code_handle *   m_read64mask[3];              /* read double masked */
	uml::code_handle *   m_write64[3];                 /* write double */
	uml::code_handle *   m_write64mask[3];             /* write double masked */
	uml::code_handle *   m_exception[18/*EXCEPTION_COUNT*/]; /* array of exception handlers */
	uml::code_handle *   m_exception_norecover[18/*EXCEPTION_COUNT*/];   /* array of no-recover exception handlers */

	/* hotspots */
	UINT32              m_hotspot_select;
	struct
	{
		offs_t              pc;                         /* PC to consider */
		UINT32              opcode;                     /* required opcode at that PC */
		UINT32              cycles;                     /* number of cycles to eat when hit */
	}       m_hotspot[MIPS3_MAX_HOTSPOTS];
	bool m_isdrc;


	void generate_exception(int exception, int backup);
	void generate_tlb_exception(int exception, offs_t address);
	void invalid_instruction(UINT32 op);
	void check_irqs();
public:
	void mips3com_update_cycle_counting();
	void mips3com_asid_changed();
	void mips3com_tlbr();
	void mips3com_tlbwi();
	void mips3com_tlbwr();
	void mips3com_tlbp();
private:
	UINT32 compute_config_register();
	UINT32 compute_prid_register();

	void tlb_map_entry(int tlbindex);
	void tlb_write_common(int tlbindex);

	bool RBYTE(offs_t address, UINT32 *result);
	bool RHALF(offs_t address, UINT32 *result);
	bool RWORD(offs_t address, UINT32 *result);
	bool RWORD_MASKED(offs_t address, UINT32 *result, UINT32 mem_mask);
	bool RDOUBLE(offs_t address, UINT64 *result);
	bool RDOUBLE_MASKED(offs_t address, UINT64 *result, UINT64 mem_mask);
	void WBYTE(offs_t address, UINT8 data);
	void WHALF(offs_t address, UINT16 data);
	void WWORD(offs_t address, UINT32 data);
	void WWORD_MASKED(offs_t address, UINT32 data, UINT32 mem_mask);
	void WDOUBLE(offs_t address, UINT64 data);
	void WDOUBLE_MASKED(offs_t address, UINT64 data, UINT64 mem_mask);

	UINT64 get_cop0_reg(int idx);
	void set_cop0_reg(int idx, UINT64 val);
	UINT64 get_cop0_creg(int idx);
	void set_cop0_creg(int idx, UINT64 val);
	void handle_cop0(UINT32 op);

	UINT32 get_cop1_reg32(int idx);
	UINT64 get_cop1_reg64(int idx);
	void set_cop1_reg32(int idx, UINT32 val);
	void set_cop1_reg64(int idx, UINT64 val);
	UINT64 get_cop1_creg(int idx);
	void set_cop1_creg(int idx, UINT64 val);
	void handle_cop1_fr0(UINT32 op);
	void handle_cop1_fr1(UINT32 op);
	void handle_cop1x_fr0(UINT32 op);
	void handle_cop1x_fr1(UINT32 op);

	UINT64 get_cop2_reg(int idx);
	void set_cop2_reg(int idx, UINT64 val);
	UINT64 get_cop2_creg(int idx);
	void set_cop2_creg(int idx, UINT64 val);
	void handle_cop2(UINT32 op);

	void handle_special(UINT32 op);
	void handle_regimm(UINT32 op);

	void lwl_be(UINT32 op);
	void lwr_be(UINT32 op);
	void ldl_be(UINT32 op);
	void ldr_be(UINT32 op);
	void swl_be(UINT32 op);
	void swr_be(UINT32 op);
	void sdl_be(UINT32 op);
	void sdr_be(UINT32 op);
	void lwl_le(UINT32 op);
	void lwr_le(UINT32 op);
	void ldl_le(UINT32 op);
	void ldr_le(UINT32 op);
	void swl_le(UINT32 op);
	void swr_le(UINT32 op);
	void sdl_le(UINT32 op);
	void sdr_le(UINT32 op);
	void load_fast_iregs(drcuml_block *block);
	void save_fast_iregs(drcuml_block *block);
	void code_flush_cache();
	void code_compile_block(UINT8 mode, offs_t pc);
public:
	void func_get_cycles();
	void func_printf_exception();
	void func_printf_debug();
	void func_printf_probe();
	void func_unimplemented();
private:
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_tlb_mismatch();
	void static_generate_exception(UINT8 exception, int recover, const char *name);
	void static_generate_memory_accessor(int mode, int size, int iswrite, int ismasked, const char *name, uml::code_handle **handleptr);

	void generate_update_mode(drcuml_block *block);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, int allow_exception);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg);

	int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_special(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_regimm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_idt(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

	int generate_set_cop0_reg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 reg);
	int generate_get_cop0_reg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 reg);
	int generate_cop0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_cop1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	int generate_cop1x(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

	void check_cop0_access(drcuml_block *block);
	void check_cop1_access(drcuml_block *block);
	void generate_badcop(drcuml_block *block, const int cop);

	void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op);
	const char *log_desc_flags_to_string(UINT32 flags);
	void log_register_list(drcuml_state *drcuml, const char *string, const UINT32 *reglist, const UINT32 *regnostarlist);
	void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);

};


class vr4300be_device : public mips3_device
{
public:
	// construction/destruction
	vr4300be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, VR4300BE, "VR4300 (big)", tag, owner, clock, "vr4300be", MIPS3_TYPE_VR4300, ENDIANNESS_BIG)
	{ }
};

class vr4300le_device : public mips3_device
{
public:
	// construction/destruction
	vr4300le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, VR4300LE, "VR4300 (little)", tag, owner, clock, "vr4300le", MIPS3_TYPE_VR4300, ENDIANNESS_LITTLE)
	{ }
};

class vr4310be_device : public mips3_device
{
public:
	// construction/destruction
	vr4310be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, VR4310BE, "VR4310 (big)", tag, owner, clock, "vr4310be", MIPS3_TYPE_VR4300, ENDIANNESS_BIG)
	{ }
};

class vr4310le_device : public mips3_device
{
public:
	// construction/destruction
	vr4310le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, VR4310LE, "VR4310 (little)", tag, owner, clock, "vr4310le", MIPS3_TYPE_VR4300, ENDIANNESS_LITTLE)
	{ }
};

class r4600be_device : public mips3_device
{
public:
	// construction/destruction
	r4600be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R4600BE, "R4600 (big)", tag, owner, clock, "r4600be", MIPS3_TYPE_R4600, ENDIANNESS_BIG)
	{ }
};

class r4600le_device : public mips3_device
{
public:
	// construction/destruction
	r4600le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R4600LE, "R4600 (little)", tag, owner, clock, "r4600le", MIPS3_TYPE_R4600, ENDIANNESS_LITTLE)
	{ }
};

class r4650be_device : public mips3_device
{
public:
	// construction/destruction
	r4650be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R4650BE, "IDT R4650 (big)", tag, owner, clock, "r4650be", MIPS3_TYPE_R4650, ENDIANNESS_BIG)
	{ }
};

class r4650le_device : public mips3_device
{
public:
	// construction/destruction
	r4650le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R4650LE, "IDT R4650 (little)", tag, owner, clock, "r4650le", MIPS3_TYPE_R4650, ENDIANNESS_LITTLE)
	{ }
};

class r4700be_device : public mips3_device
{
public:
	// construction/destruction
	r4700be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R4700BE, "R4700 (big)", tag, owner, clock, "r4700be", MIPS3_TYPE_R4700, ENDIANNESS_BIG)
	{ }
};

class r4700le_device : public mips3_device
{
public:
	// construction/destruction
	r4700le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R4700LE, "R4700 (little)", tag, owner, clock, "r4700le", MIPS3_TYPE_R4700, ENDIANNESS_LITTLE)
	{ }
};

class r5000be_device : public mips3_device
{
public:
	// construction/destruction
	r5000be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R5000BE, "R5000 (big)", tag, owner, clock, "r5000be", MIPS3_TYPE_R5000, ENDIANNESS_BIG)
	{ }
};

class r5000le_device : public mips3_device
{
public:
	// construction/destruction
	r5000le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, R5000LE, "R5000 (little)", tag, owner, clock, "r5000le", MIPS3_TYPE_R5000, ENDIANNESS_LITTLE)
	{ }
};

class qed5271be_device : public mips3_device
{
public:
	// construction/destruction
	qed5271be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, QED5271BE, "QED5271 (big)", tag, owner, clock, "qed5271be", MIPS3_TYPE_QED5271, ENDIANNESS_BIG)
	{ }
};

class qed5271le_device : public mips3_device
{
public:
	// construction/destruction
	qed5271le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, QED5271LE, "QED5271 (little)", tag, owner, clock, "qed5271le", MIPS3_TYPE_QED5271, ENDIANNESS_LITTLE)
	{ }
};

class rm7000be_device : public mips3_device
{
public:
	// construction/destruction
	rm7000be_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, RM7000BE, "RM7000 (big)", tag, owner, clock, "rm7000be", MIPS3_TYPE_RM7000, ENDIANNESS_BIG)
	{ }
};

class rm7000le_device : public mips3_device
{
public:
	// construction/destruction
	rm7000le_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: mips3_device(mconfig, RM7000LE, "RM7000 (little)", tag, owner, clock, "rm7000le", MIPS3_TYPE_RM7000, ENDIANNESS_LITTLE)
	{ }
};



class mips3_frontend : public drc_frontend
{
public:
	// construction/destruction
	mips3_frontend(mips3_device *mips3, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	// internal helpers
	bool describe_special(UINT32 op, opcode_desc &desc);
	bool describe_regimm(UINT32 op, opcode_desc &desc);
	bool describe_idt(UINT32 op, opcode_desc &desc);
	bool describe_cop0(UINT32 op, opcode_desc &desc);
	bool describe_cop1(UINT32 op, opcode_desc &desc);
	bool describe_cop1x(UINT32 op, opcode_desc &desc);
	bool describe_cop2(UINT32 op, opcode_desc &desc);

	// internal state
	mips3_device *m_mips3;
};


/***************************************************************************
    COMPILER-SPECIFIC OPTIONS
***************************************************************************/

/* fix me -- how do we make this work?? */
#define MIPS3DRC_STRICT_VERIFY      0x0001          /* verify all instructions */
#define MIPS3DRC_STRICT_COP0        0x0002          /* validate all COP0 instructions */
#define MIPS3DRC_STRICT_COP1        0x0004          /* validate all COP1 instructions */
#define MIPS3DRC_STRICT_COP2        0x0008          /* validate all COP2 instructions */
#define MIPS3DRC_FLUSH_PC           0x0010          /* flush the PC value before each memory access */
#define MIPS3DRC_CHECK_OVERFLOWS    0x0020          /* actually check overflows on add/sub instructions */
#define MIPS3DRC_ACCURATE_DIVZERO   0x0040          /* load correct values into HI/LO on integer divide-by-zero */

#define MIPS3DRC_COMPATIBLE_OPTIONS (MIPS3DRC_STRICT_VERIFY | MIPS3DRC_STRICT_COP1 | MIPS3DRC_STRICT_COP0 | MIPS3DRC_STRICT_COP2 | MIPS3DRC_FLUSH_PC)
#define MIPS3DRC_FASTEST_OPTIONS    (0)


#endif /* __MIPS3_H__ */
