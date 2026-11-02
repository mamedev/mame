// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7.h
 *   Portable CPU Emulator for 26/32-bit ARM v1/v2/v3/v4/v5 (including v5TE and v5TDMI)
 *   Emulation by R. Belmont, Ryan Holtz, and Steve Ellenoff
 *
 *****************************************************************************

 This file contains everything related to the arm7 cpu specific implementation.
 Anything related to the arm7 core itself is defined in arm7core.h instead.

 ******************************************************************************/

#ifndef MAME_CPU_ARM7_ARM7_H
#define MAME_CPU_ARM7_ARM7_H

#pragma once

#include "arm7dasm.h"

#include "cpu/drcuml.h"

#include "divtlb.h"

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>

/****************************************************************************************************
 *  PUBLIC FUNCTIONS
 ***************************************************************************************************/

class arm7_cpu_device : public cpu_device, public arm7_disassembler::config, public device_vtlb_interface
{
public:
	enum
	{
		ARM7_IRQ_LINE=0, ARM7_FIRQ_LINE,
		ARM7_ABORT_EXCEPTION, ARM7_ABORT_PREFETCH_EXCEPTION, ARM7_UNDEFINE_EXCEPTION,
		ARM7_NUM_LINES
	};
	// Really there's only 1 ABORT Line.. and cpu decides whether it's during data fetch or prefetch, but we let the user specify

	enum
	{
		ARM7_PC = 0,
		ARM7_R0, ARM7_R1, ARM7_R2, ARM7_R3, ARM7_R4, ARM7_R5, ARM7_R6, ARM7_R7,
		ARM7_R8, ARM7_R9, ARM7_R10, ARM7_R11, ARM7_R12, ARM7_R13, ARM7_R14, ARM7_R15,
		ARM7_FR8, ARM7_FR9, ARM7_FR10, ARM7_FR11, ARM7_FR12, ARM7_FR13, ARM7_FR14,
		ARM7_IR13, ARM7_IR14, ARM7_SR13, ARM7_SR14, ARM7_FSPSR, ARM7_ISPSR, ARM7_SSPSR,
		ARM7_CPSR, ARM7_AR13, ARM7_AR14, ARM7_ASPSR, ARM7_UR13, ARM7_UR14, ARM7_USPSR, ARM7_LOGTLB
	};

	// recompiler options (arm7drc_set_options())
	static constexpr uint32_t DRC_STRICT_VERIFY = 0x0001;   // verify every instruction's word as it runs, not only a sequence's first
	static constexpr uint32_t DRC_COMPATIBLE_OPTIONS = DRC_STRICT_VERIFY;
	static constexpr uint32_t DRC_FASTEST_OPTIONS = 0;

	// construction/destruction
	arm7_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~arm7_cpu_device();

	void set_high_vectors() { m_vectorbase = 0xffff0000; }

	// 26-bit data space (CP15 DATA32 clear on the parts with the 26-bit modes): a data address with bits 31:26 set takes
	// the address exception (vector 0x14, SVC mode).  Disabling it gives the old cpu/arm behaviour of silently masking
	// the address to 26 bits instead.
	void set_address_exception(bool enable) { m_address_exception = enable; }

	// GBA BIOS protection: fetch_shadow() is the word the prefetch last fetched at PC+8 for an instruction in
	// [start, end] (with the MMU off)
	void set_fetch_shadow(offs_t start, offs_t end);
	uint32_t fetch_shadow() const { return m_core->fetch_shadow; }

	virtual uint32_t vector_base() const;

	// recompiler configuration
	void arm7drc_set_options(uint32_t options) ATTR_COLD;
	void arm7drc_add_fastram(offs_t start, offs_t end, bool readonly, void *base) ATTR_COLD;
	void arm7drc_add_hotspot(offs_t pc, uint32_t opcode, uint32_t cycles) ATTR_COLD;
	void set_drc_cache_size(size_t bytes) { m_drcuml.set_cache_size(bytes); }

	// the recompiler's C callbacks
	void func_execute();
	void func_execute_volatile();
	void func_privilege_hook();
	void func_check_irq();
	void func_fetch_fault();
	void func_fetch_refill();
	void func_tlb_check();
	void func_read8();
	void func_read16();
	void func_read32();
	void func_write8();
	void func_write16();
	void func_write32();

protected:
	class opcode_desc;
	class frontend;

	enum arm_arch_flag : uint32_t;
	enum arm_copro_id : uint32_t;

	arm7_cpu_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			uint8_t archRev,
			uint32_t archFlags,
			endianness_t endianness,
			address_map_constructor internal_map = address_map_constructor());

	void postload();

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 3; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	void execute_run_interpreter();
	void execute_run_drc();
	void execute_arm_insn(uint32_t insn);
	void execute_thumb_insn(uint32_t pc, uint32_t insn);
	void interpret_slice();
	void drc_r15_to_interp();
	void drc_r15_from_interp();
	void drc_prefetch_abort(offs_t pc);

	// the cycles the handlers charge for an instruction (the recompiler frontend's static count), less the
	// operand-dependent steps of the multipliers
	uint32_t arm_insn_cycles(uint32_t insn) const;
	static uint32_t thumb_insn_cycles() { return 3; }

	// device_memory_interface  implementation
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface  implementation
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface  implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual bool get_t_flag() const override;
	virtual u8 get_arch_rev() const override;

	void translate_insn_command(const std::vector<std::string_view> &params);
	void translate_data_command(const std::vector<std::string_view> &params);
	void translate_command(const std::vector<std::string_view> &params, int intention);

	void update_insn_prefetch(uint32_t curr_pc);
	bool insn_fetch_thumb(uint32_t pc, uint32_t &out_insn);
	bool insn_fetch_arm(uint32_t pc, uint32_t &out_insn);

	void add_ce_kernel_addr(offs_t addr, std::string value);
	void init_ce_kernel_addrs();
	void print_ce_kernel_address(const offs_t addr);

	// for SoCs with onboard interrupt controllers - only call from scheduler or CPU's own context
	void set_irq(int state);
	void set_fiq(int state);

	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cachele;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cachebe;

	// The register file and the recompiler's working variables, in the near area of the recompiler cache, where
	// generated code reaches them with one host instruction (the cache is allocated whether the recompiler runs or not)
	struct core_state
	{
		uint32_t r[37];             // NUM_REGS: R0-R15, the CPSR and the banked copies
		alignas(8) int icount;      // with the stop word above it, one 64-bit word for the test at the end of a memory instruction
		uint32_t stop;              // why generated code must leave after a memory access (STOP_*)
		uint32_t mode;              // recompiler hash mode: register bank, Thumb and 26-bit state of the CPSR (update_reg_ptr())
		uint32_t arg0;              // the parameters and results of the C callbacks
		uint32_t arg1;
		uint32_t result;
		uint32_t exact_miss;        // the hash miss was in the exact copies
		uint32_t literal_miss;      // a folded PC-relative word changed: that load is not folded again
		uint32_t verify_miss;       // a word check failed: the address counts towards volatile code
		uint32_t resync_next;       // the last resync was from the end of an ARM memory instruction, to the next one
		uint32_t fetch_shadow;      // the word the prefetch last fetched at PC+8 inside the shadow range (set_fetch_shadow())
		uint64_t tlb_stats[4];      // DRC_TLB_CHECK: hits checked, hits the vtlb could not check, fills, misses
	};

	enum : uint32_t
	{
		STOP_ABORT = 1,             // the access aborted (m_pendingAbtD): the instruction ends there
		STOP_REMAP = 2              // the address map changed: the instruction completes, then the code cache is flushed
	};

	// The hash modes: m_core->mode (register bank in bits 4:2, T, 26-bit state) with bit 1 of the address folded
	// in, so the tables are word-granular, and a bit for the exact copies of the code (cycles taken after every
	// instruction)
	enum : uint8_t
	{
		DRC_MODE_HALF = 0x20,
		DRC_MODE_EXACT = 0x40
	};
	static uint8_t drc_hash_key(uint8_t mode, offs_t pc) { return mode | ((pc & 2) ? DRC_MODE_HALF : 0); }

	// the instruction classes with generators, each switchable for bisecting (DRC_GENERATE_DEFAULT)
	enum : uint32_t
	{
		DRC_GENERATE_ALU = 1U << 0,             // data processing
		DRC_GENERATE_PSR = 1U << 1,             // MRS, MSR
		DRC_GENERATE_BRANCH = 1U << 2,          // B, BL, BX, BLX
		DRC_GENERATE_MEM = 1U << 3,             // LDR, STR, LDRB, STRB
		DRC_GENERATE_MEMHALF = 1U << 4,         // halfword and signed transfers, LDRD, STRD
		DRC_GENERATE_MEMBLOCK = 1U << 5,        // LDM, STM
		DRC_GENERATE_MUL = 1U << 6,             // the multiplies
		DRC_GENERATE_THUMB = 1U << 7,           // Thumb
		DRC_GENERATE_ALU_S = 1U << 8,           // data processing that sets the flags
		DRC_GENERATE_ALU_COND = 1U << 9,        // conditional data processing
		DRC_GENERATE_ALU_REGSHIFT = 1U << 10,   // shifts by a register
		DRC_GENERATE_ALU_IMMSHIFT = 1U << 11,   // shifts by an immediate
		DRC_GENERATE_ALU_R15 = 1U << 12,        // R15 as an operand
		DRC_GENERATE_ALL = 0x1fff
	};

	// the optimisations, each switchable for bisecting (DRC_OPT_DEFAULT)
	enum : uint32_t
	{
		DRC_OPT_LITERAL = 1,        // PC-relative word loads folded to the word, checked in place
		DRC_OPT_RUNS = 2,           // one condition test for a run of instructions with the same condition
		DRC_OPT_BLFUSE = 4,         // a Thumb BL whose halves follow each other branches statically
		DRC_OPT_FLAGS = 8,          // condition flags nobody reads are not computed
		DRC_OPT_FASTRAM = 16,       // the fast RAM regions the driver declared (off: a harness tapping RAM sees every access)
		DRC_OPT_SUMS = 32,          // a sequence's words are verified as one sum up to its first store (off: a word at a time)
		DRC_OPT_CONTROL = 64,       // the MMU, DATA32 and late-abort bits are compiled in (a change flushes the cache)
		DRC_OPT_TLB = 128,          // the data TLB in front of the accessors' translation and region checks
		DRC_OPT_TLB_INLINE = 256,   // the data TLB probed in the block for the single transfers
		DRC_OPT_ALL = 511
	};

	// what the recompiler runs with; the switches are for bisecting it
	static constexpr uint32_t DRC_GENERATE_DEFAULT = DRC_GENERATE_ALL;
	static constexpr uint32_t DRC_OPT_DEFAULT = DRC_OPT_ALL;
	static constexpr uint32_t DRC_TLB_SIZE = 1024;          // data TLB entries per table, a power of two from 2 to 1024
	static constexpr bool DRC_TLB_CHECK = false;            // every data TLB hit is translated again in C++
	static constexpr bool DRC_TLB_FLUSH_SLICES = false;     // the data TLB is emptied at the start of every slice, for stress

	core_state *m_core;
	uint32_t *m_r;                  // m_core->r

	std::string m_ce_kernel_addrs[0x10400];
	bool m_ce_kernel_addr_present[0x10400];

	uint32_t m_insn_prefetch_depth;
	uint32_t m_insn_prefetch_count;
	uint32_t m_insn_prefetch_index;
	uint32_t m_insn_prefetch_buffer[3];
	uint32_t m_insn_prefetch_address[3];
	bool m_insn_prefetch_valid[3];
	const uint32_t m_prefetch_word0_shift;
	const uint32_t m_prefetch_word1_shift;
	int m_tlb_log;
	int m_actual_log;

	bool m_pendingIrq;
	bool m_pendingFiq;
	bool m_pendingAbtD;
	bool m_pendingAddrExc;      // the pending data abort is a 26-bit data space address exception (vector 0x14)
	bool m_pendingAbtP;
	bool m_pendingUnd;
	bool m_pendingSwi;
	bool m_pending_interrupt;
	endianness_t m_endian;
	address_space *m_program;
	std::function<u32 (offs_t)> m_pr32;
	std::function<const void * (offs_t)> m_prptr;

	/* Coprocessor Registers */
	uint32_t m_control;
	uint32_t m_tlbBase;
	uint32_t m_tlb_base_mask;
	uint32_t m_faultStatus[2];
	uint32_t m_faultAddress;
	uint32_t m_fcsePID;
	uint32_t m_pid_offset;
	uint32_t m_domainAccessControl;
	uint8_t m_decoded_access_control[16];

	uint8_t m_archRev;          // ARM architecture revision (3, 4, 5, and 6 are valid)
	uint32_t m_archFlags;        // architecture flags

	uint32_t m_vectorbase;
	bool m_address_exception = true;
	bool m_ldr_pc_round_up = false; // DE156: LDR PC with an unaligned address resumes at the next word boundary
	uint32_t m_reset_control = 0;   // CP15 control register value at reset (PROG32/DATA32 can be set by grounding/Vcc-ing pins)
	offs_t m_fetch_shadow_start;    // set_fetch_shadow(); empty when start > end
	offs_t m_fetch_shadow_end;

//#if ARM7_MMU_ENABLE_HACK
//  uint32_t mmu_enable_addr; // workaround for "MMU is enabled when PA != VA" problem
//#endif

	uint32_t m_copro_id;

	// For debugger
	uint32_t m_pc;

	bool m_isdrc;              // executing through the recompiler

	// recompiler
	struct compiler_state
	{
		compiler_state &operator=(compiler_state const &) = delete;

		uint32_t cycles = 0;            // cycles accumulated since the last update
		uml::code_label labelnum = 1;   // the next local label
		offs_t verified = ~offs_t(0);   // the word last verified against memory (DRC_OPT_SUMS off)
		const opcode_desc *verify_next = nullptr;   // the instruction whose words are verified next, after a store (DRC_OPT_SUMS)
		bool exact = false;             // an exact copy: cycles are taken and checked after every instruction
		const opcode_desc *seqhead = nullptr;   // the first instruction of the sequence being generated

		// a run of instructions sharing one condition test
		const opcode_desc *run_end = nullptr;   // the last instruction of the run, nullptr outside one
		uint32_t run_cond = 0;          // its condition
		uml::code_label run_skip = 0;   // where a failed condition goes
		uint32_t run_back = 0;          // the cycles a skipped run gives back
		uint32_t run_flushed = 0;       // the cycles flushed inside the run, which a skipped run still owes
	};

	struct fast_ram_info
	{
		offs_t start = 0;
		offs_t end = 0;
		bool readonly = false;
		void *base = nullptr;
		bool direct = false;        // the address map still maps the region straight to its memory (drc_check_fastram())
	};

	struct hotspot_info
	{
		uint32_t pc = 0;
		uint32_t opcode = 0;
		uint32_t cycles = 0;
	};

	struct tcm_info                 // a tightly coupled memory (ARM946E-S), accessed in place within [*base, *end]
	{
		const uint32_t *base = nullptr;
		const uint32_t *end = nullptr;
		void *data = nullptr;
		uint32_t mask = 0;
	};

	void drc_initialize() ATTR_COLD;
	void drc_start() ATTR_COLD;
	virtual bool drc_memory_fast_path() const { return true; }  // false when the device's memory accessors must see every access
	virtual bool drc_memory_rotates_halfwords() const { return true; }  // an unaligned LDRH rotates the halfword (ARMv4); the ARM946E-S takes it as it is
	uint32_t drc_lane_xor(int size) const;     // the address XOR for a byte or halfword in host memory when guest and host endianness differ
	void drc_remapped();
	void drc_check_fastram();
	bool drc_prefetch_after_remap();
	void drc_tlb_choose_base();
	bool drc_tlb_fits(const void *host, uint32_t bytes) const;
	uint64_t *drc_tlb_table(bool iswrite, bool user) const { return m_drc_tlb + ((iswrite ? 2 : 0) + (user ? 1 : 0)) * m_drc_tlb_size; }
	void drc_tlb_flush();
	void drc_tlb_flush_page(offs_t va);
	void drc_tlb_flush_mva(offs_t mva);
	void generate_tlb_fill(drcuml_block &block, uint64_t *table, uml::parameter offset, uint32_t hostdelta, uml::parameter temp);
	void generate_tlb_probe(drcuml_block &block, int size, bool iswrite, bool user, uml::code_label miss);
	void generate_tlb_lookup(drcuml_block &block, uml::parameter addr, bool iswrite, bool user, uml::code_label miss);
	void generate_tlb_check_call(drcuml_block &block, uml::parameter addr, uml::parameter offset, bool iswrite, bool user);
	uml::code_label generate_block_probe(drcuml_block &block, compiler_state &compiler, uint32_t count, bool iswrite, bool user);
	void generate_block_word(drcuml_block &block, uint32_t index, bool iswrite, uml::parameter value);
	bool drc_tlb_inline() const { return (m_drc_tlb_base != nullptr) && (m_drc_opt & DRC_OPT_TLB_INLINE); }
	uml::code_label generate_access(drcuml_block &block, compiler_state &compiler, int size, bool iswrite, bool user);
	static void generate_access_end(drcuml_block &block, uml::code_label done);
	bool drc_generates(const opcode_desc &desc) const;
	static bool alu_is_logical(uint32_t opcode);
	void code_flush_cache();
	static uint32_t estimate_uml_ops(const opcode_desc &desc);
	bool code_compile_block(uint8_t mode, offs_t pc);
	void static_generate_entry_point(drcuml_block &block);
	void static_generate_nocode_handler(drcuml_block &block, int &label);
	void static_generate_out_of_cycles(drcuml_block &block);
	void static_generate_resync(drcuml_block &block, int &label);
	void static_generate_slice_exact(drcuml_block &block);
	void static_generate_shift_reg(drcuml_block &block, int &label, uint32_t type, const char *name, uml::code_handle *&handleptr);
	void static_generate_memory_accessor(drcuml_block &block, int &label, bool user, int size, bool iswrite, const char *name, uml::code_handle *&handleptr);
	void generate_hashjmp_dynamic(drcuml_block &block);
	void generate_r15_psr26(drcuml_block &block, uml::parameter dst, uml::parameter addend, uml::parameter temp);
	uint32_t instruction_cycles(const opcode_desc *desc) const;
	uint32_t instruction_cycles_max(const opcode_desc *desc) const;
	void generate_slice_check(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_flush_cycles(drcuml_block &block, compiler_state &compiler);
	void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param);
	void generate_validate_fetch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_verify_chunk(drcuml_block &block, compiler_state &compiler, const opcode_desc *first, const opcode_desc *seqlast);
	void generate_verify_word(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void drc_control_written(uint32_t data);
	bool drc_static_control() const { return (m_drc_opt & DRC_OPT_CONTROL) && !debugger_enabled(); }
	void generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_debug_guard(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_fetch_fault(drcuml_block &block, const opcode_desc *desc);
	void generate_fallback(drcuml_block &block, const opcode_desc *desc);
	bool generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	// the generators (arm7drcops.cpp)
	uml::parameter drc_reg(uint32_t regnum) const;
	uml::code_label generate_condition(drcuml_block &block, compiler_state &compiler, uint32_t cond);
	void generate_condition_end(drcuml_block &block, compiler_state &compiler, uml::code_label skip, uint32_t cycles);
	void generate_condition_run(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, const opcode_desc *seqlast);
	void generate_condition_run_end(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool drc_literal(const opcode_desc &desc, offs_t address, const void *&host, uint32_t &word);
	void generate_literal_check(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, const void *host, uint32_t word);
	void generate_flags_arith(drcuml_block &block, bool subtract);
	void generate_flags_logical(drcuml_block &block, uml::parameter carry);
	void generate_flags_nz(drcuml_block &block);
	uml::parameter generate_operand2(drcuml_block &block, const opcode_desc *desc, uint32_t op, bool want_carry, uml::parameter &carry);
	uml::parameter generate_shift_imm(drcuml_block &block, const opcode_desc *desc, uint32_t op, bool want_carry, uml::parameter &carry);
	void generate_abort_check(drcuml_block &block, const opcode_desc *desc);
	void generate_memory_end(drcuml_block &block, const opcode_desc *desc);
	bool generate_mem_address(drcuml_block &block, const opcode_desc *desc, uint32_t op, uml::parameter offset, bool load);
	void generate_branch_end(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::code_label skip, uint32_t skip_cost, bool dynamic = false);
	void generate_loaded_pc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::code_label skip, bool interwork, bool accessed);
	void generate_loaded_pc_end(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uml::code_label skip);
	bool generate_alu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_clz(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_psr_transfer(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_branch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_bx(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_mem_single(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_mem_half(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_mem_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_multiplier_cycles(drcuml_block &block, uint32_t op, uml::parameter rsval);
	bool generate_mul(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_mul_long(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool thumb_generates(uint16_t op) const;
	void generate_thumb_access(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t kind, uint32_t rd);
	void generate_thumb_bx(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	bool generate_thumb(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op);
	void log_opcode_desc(const opcode_desc *desclist);
	static std::string log_register_list(std::bitset<24> const &reglist);

	drcuml_state m_drcuml;      // its cache's near area holds the register file too (device_start())
	std::unique_ptr<frontend> m_drcfe;
	uint32_t m_drcoptions;
	uint32_t m_drc_generate;    // the instruction classes with generators (DRC_GENERATE_*)
	uint32_t m_drc_opt;         // the optimisations in use (DRC_OPT_*)
	uint64_t *m_drc_tlb;        // the data TLB: four tables of m_drc_tlb_size entries, (addend << 32) | virtual page
	uint32_t m_drc_tlb_size;    // entries per table, a power of two (DRC_TLB_SIZE)
	uint8_t *m_drc_tlb_base;    // the host address the addends count from; nullptr when no region fits (the TLB is off)
	std::unordered_set<offs_t> m_drc_nofold;    // PC-relative loads whose word changed under them
	std::unordered_map<offs_t, uint32_t> m_drc_verify_misses;  // word check failures per address
	std::unordered_set<offs_t> m_drc_volatile;  // instructions whose word keeps changing: the interpreter's, fetched as they run
	std::unordered_map<offs_t, std::pair<uint32_t, uint32_t>> m_drc_prefetch;   // per ARM memory instruction (by the next address): the next two words as compiled
	bool m_drc_prefetched;      // the interpreter's prefetch buffer holds words from before a change to the map
	bool m_drc_cache_dirty;
	bool m_drc_fastram_stale;   // the address map changed since drc_check_fastram()
	util::notifier_subscription m_drc_map_subscription;   // the address map changed: flush
	uint32_t m_fastram_select;
	fast_ram_info m_fastram[4];
	uint32_t m_drc_hotspot_select;
	hotspot_info m_drc_hotspot[16];
	tcm_info m_drc_tcm[2];
	uml::code_handle *m_entry;
	uml::code_handle *m_entry_exact;
	uml::code_handle *m_nocode;
	uml::code_handle *m_nocode_verify;
	uml::code_handle *m_nocode_miss;
	uml::code_handle *m_nocode_exact;
	uml::code_handle *m_nocode_literal;
	uml::code_handle *m_out_of_cycles;
	uml::code_handle *m_resync;
	uml::code_handle *m_resync_next;
	uml::code_handle *m_slice_exact;
	uml::code_handle *m_shift_reg[4];   // LSL, LSR, ASR, ROR by a register
	uml::code_handle *m_read8[2];       // [0] privileged, [1] user mode
	uml::code_handle *m_read16[2];
	uml::code_handle *m_read32[2];
	uml::code_handle *m_write8[2];
	uml::code_handle *m_write16[2];
	uml::code_handle *m_write32[2];
	uml::code_handle *m_access_miss[2][3][2];   // [write][byte, halfword, word][user]: the accessors' entries after the data TLB probe

	int64_t saturate_qbit_overflow(int64_t res);
	void SwitchMode(uint32_t cpsr_mode_val);
	uint32_t decodeShift(uint32_t insn, uint32_t *pCarry);
	int loadInc(uint32_t pat, uint32_t rbv, uint32_t s, int mode);
	int loadDec(uint32_t pat, uint32_t rbv, uint32_t s, int mode);
	int storeInc(uint32_t pat, uint32_t rbv, int mode);
	int storeDec(uint32_t pat, uint32_t rbv, int mode);
	virtual bool handle_coprocessor(uint32_t insn) { return false; }
	virtual void prefetch_abort(uint32_t pc) { }
	void HandleCoProcDO(uint32_t insn);
	void HandleCoProcRT(uint32_t insn);
	void HandleCoProcDT(uint32_t insn);
	void HandleBranch(uint32_t insn, bool h_bit);
	void HandleMemSingle(uint32_t insn);
	void HandleHalfWordDT(uint32_t insn);
	void HandleSwap(uint32_t insn);
	void HandlePSRTransfer(uint32_t insn);
	void HandleALU(uint32_t insn);
	void HandleMul(uint32_t insn);
	void HandleSMulLong(uint32_t insn);
	void HandleUMulLong(uint32_t insn);
	void HandleMemBlock(uint32_t insn);

	void armv6_media(uint32_t insn);
	void armv6_exclusive(uint32_t insn);
	void armv6_cps(uint32_t insn);
	uint32_t m_exclusive_address = 0;
	uint8_t m_exclusive_size = 0;
	bool m_exclusive_valid = false;
	memory_passthrough_handler m_exclusive_tap;

	void arm7ops_0123(uint32_t insn);
	void arm7ops_0123_v4(uint32_t insn);
	void arm7ops_undef_conditional(uint32_t insn);
	void arm7ops_4567(uint32_t insn);
	void arm7ops_89(uint32_t insn);
	void arm7ops_ab(uint32_t insn);
	void arm7ops_cd(uint32_t insn);
	void arm7ops_e(uint32_t insn);
	void arm7ops_f(uint32_t insn);

	void arm9ops_undef(uint32_t insn);
	void arm9ops_1(uint32_t insn);
	void arm9ops_57(uint32_t insn);
	void arm9ops_89(uint32_t insn);
	void arm9ops_ab(uint32_t insn);
	void arm9ops_c(uint32_t insn);
	void arm9ops_e(uint32_t insn);

	void set_cpsr(uint32_t val);
	void write_r15_psr26(uint32_t value, bool write_pc);
	bool ldm_loads_base(uint32_t insn, uint32_t rb) const;
	bool check_data_address(offs_t &addr);
	virtual bool translate_vaddr_to_paddr(offs_t &addr, const int flags);
	bool page_table_finish_translation(offs_t &vaddr, const uint8_t type, const uint32_t lvl1, const uint32_t lvl2, const int flags, const uint32_t lvl1a, const uint32_t lvl2a);
	bool page_table_translate(offs_t &vaddr, const int flags);
	uint32_t get_fault_from_permissions(const uint8_t access, const uint8_t domain, const uint8_t type, const int flags, bool user);
	vtlb_entry vtlb_permission(int flags) const;
	vtlb_entry page_permissions(uint8_t access, uint8_t domain, uint8_t type);
	void vtlb_flush_mva(offs_t mva);
	void flush_translations();
	uint32_t get_lvl2_desc_from_page_table(uint32_t granularity, uint32_t first_desc, uint32_t vaddr);
	int detect_fault(int desc_lvl1, int ap, int flags);
	void arm7_check_irq_state();
	void update_irq_state();
	virtual void arm7_cpu_write32(offs_t addr, uint32_t data);
	virtual void arm7_cpu_write16(offs_t addr, uint16_t data);
	virtual void arm7_cpu_write8(offs_t addr, uint8_t data);
	virtual uint32_t arm7_cpu_read32(offs_t addr);
	virtual uint32_t arm7_cpu_read16(offs_t addr);
	virtual uint8_t arm7_cpu_read8(offs_t addr);

	virtual uint32_t insn_fetch_word(offs_t addr) { return m_pr32(addr); }
	virtual const void *insn_fetch_ptr(offs_t addr) { return m_prptr(addr); }

	// Coprocessor support
	virtual void arm7_do_callback(uint32_t insn);
	virtual uint32_t arm7_rt_r_callback(offs_t offset);
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data);
	void arm7_dt_r_callback(uint32_t insn, uint32_t *prn);
	void arm7_dt_w_callback(uint32_t insn, uint32_t *prn);

	void tg00_0(uint32_t pc, uint32_t insn);
	void tg00_1(uint32_t pc, uint32_t insn);
	void tg01_0(uint32_t pc, uint32_t insn);
	void tg01_10(uint32_t pc, uint32_t insn);
	void tg01_11(uint32_t pc, uint32_t insn);
	void tg01_12(uint32_t pc, uint32_t insn);
	void tg01_13(uint32_t pc, uint32_t insn);
	void tg02_0(uint32_t pc, uint32_t insn);
	void tg02_1(uint32_t pc, uint32_t insn);
	void tg03_0(uint32_t pc, uint32_t insn);
	void tg03_1(uint32_t pc, uint32_t insn);
	void tg04_00_00(uint32_t pc, uint32_t insn);
	void tg04_00_01(uint32_t pc, uint32_t insn);
	void tg04_00_02(uint32_t pc, uint32_t insn);
	void tg04_00_03(uint32_t pc, uint32_t insn);
	void tg04_00_04(uint32_t pc, uint32_t insn);
	void tg04_00_05(uint32_t pc, uint32_t insn);
	void tg04_00_06(uint32_t pc, uint32_t insn);
	void tg04_00_07(uint32_t pc, uint32_t insn);
	void tg04_00_08(uint32_t pc, uint32_t insn);
	void tg04_00_09(uint32_t pc, uint32_t insn);
	void tg04_00_0a(uint32_t pc, uint32_t insn);
	void tg04_00_0b(uint32_t pc, uint32_t insn);
	void tg04_00_0c(uint32_t pc, uint32_t insn);
	void tg04_00_0d(uint32_t pc, uint32_t insn);
	void tg04_00_0e(uint32_t pc, uint32_t insn);
	void tg04_00_0f(uint32_t pc, uint32_t insn);
	void tg04_01_00(uint32_t pc, uint32_t insn);
	void tg04_01_01(uint32_t pc, uint32_t insn);
	void tg04_01_02(uint32_t pc, uint32_t insn);
	void tg04_01_03(uint32_t pc, uint32_t insn);
	void tg04_01_10(uint32_t pc, uint32_t insn);
	void tg04_01_11(uint32_t pc, uint32_t insn);
	void tg04_01_12(uint32_t pc, uint32_t insn);
	void tg04_01_13(uint32_t pc, uint32_t insn);
	void tg04_01_20(uint32_t pc, uint32_t insn);
	void tg04_01_21(uint32_t pc, uint32_t insn);
	void tg04_01_22(uint32_t pc, uint32_t insn);
	void tg04_01_23(uint32_t pc, uint32_t insn);
	void tg04_01_30(uint32_t pc, uint32_t insn);
	void tg04_01_31(uint32_t pc, uint32_t insn);
	void tg04_01_32(uint32_t pc, uint32_t insn);
	void tg04_01_33(uint32_t pc, uint32_t insn);
	void tg04_0203(uint32_t pc, uint32_t insn);
	void tg05_0(uint32_t pc, uint32_t insn);
	void tg05_1(uint32_t pc, uint32_t insn);
	void tg05_2(uint32_t pc, uint32_t insn);
	void tg05_3(uint32_t pc, uint32_t insn);
	void tg05_4(uint32_t pc, uint32_t insn);
	void tg05_5(uint32_t pc, uint32_t insn);
	void tg05_6(uint32_t pc, uint32_t insn);
	void tg05_7(uint32_t pc, uint32_t insn);
	void tg06_0(uint32_t pc, uint32_t insn);
	void tg06_1(uint32_t pc, uint32_t insn);
	void tg07_0(uint32_t pc, uint32_t insn);
	void tg07_1(uint32_t pc, uint32_t insn);
	void tg08_0(uint32_t pc, uint32_t insn);
	void tg08_1(uint32_t pc, uint32_t insn);
	void tg09_0(uint32_t pc, uint32_t insn);
	void tg09_1(uint32_t pc, uint32_t insn);
	void tg0a_0(uint32_t pc, uint32_t insn);
	void tg0a_1(uint32_t pc, uint32_t insn);
	void tg0b_0(uint32_t pc, uint32_t insn);
	void tg0b_1(uint32_t pc, uint32_t insn);
	void tg0b_2(uint32_t pc, uint32_t insn);
	void tg0b_3(uint32_t pc, uint32_t insn);
	void tg0b_4(uint32_t pc, uint32_t insn);
	void tg0b_5(uint32_t pc, uint32_t insn);
	void tg0b_6(uint32_t pc, uint32_t insn);
	void tg0b_7(uint32_t pc, uint32_t insn);
	void tg0b_8(uint32_t pc, uint32_t insn);
	void tg0b_9(uint32_t pc, uint32_t insn);
	void tg0b_a(uint32_t pc, uint32_t insn);
	void tg0b_b(uint32_t pc, uint32_t insn);
	void tg0b_c(uint32_t pc, uint32_t insn);
	void tg0b_d(uint32_t pc, uint32_t insn);
	void tg0b_e(uint32_t pc, uint32_t insn);
	void tg0b_f(uint32_t pc, uint32_t insn);
	void tg0c_0(uint32_t pc, uint32_t insn);
	void tg0c_1(uint32_t pc, uint32_t insn);
	void tg0d_0(uint32_t pc, uint32_t insn);
	void tg0d_1(uint32_t pc, uint32_t insn);
	void tg0d_2(uint32_t pc, uint32_t insn);
	void tg0d_3(uint32_t pc, uint32_t insn);
	void tg0d_4(uint32_t pc, uint32_t insn);
	void tg0d_5(uint32_t pc, uint32_t insn);
	void tg0d_6(uint32_t pc, uint32_t insn);
	void tg0d_7(uint32_t pc, uint32_t insn);
	void tg0d_8(uint32_t pc, uint32_t insn);
	void tg0d_9(uint32_t pc, uint32_t insn);
	void tg0d_a(uint32_t pc, uint32_t insn);
	void tg0d_b(uint32_t pc, uint32_t insn);
	void tg0d_c(uint32_t pc, uint32_t insn);
	void tg0d_d(uint32_t pc, uint32_t insn);
	void tg0d_e(uint32_t pc, uint32_t insn);
	void thumb_undefined(uint32_t pc, uint32_t insn);
	void thumb_empty_rlist(uint32_t rb, bool load, bool ascending);
	void tg0d_f(uint32_t pc, uint32_t insn);
	void tg0e_0(uint32_t pc, uint32_t insn);
	void tg0e_1(uint32_t pc, uint32_t insn);
	void tg0f_0(uint32_t pc, uint32_t insn);
	void tg0f_1(uint32_t pc, uint32_t insn);

	typedef void ( arm7_cpu_device::*arm7thumb_ophandler ) (uint32_t, uint32_t);
	static const arm7thumb_ophandler thumb_handler[0x40*0x10];

	typedef void ( arm7_cpu_device::*arm7ops_ophandler )(uint32_t);
	static const arm7ops_ophandler ops_handler[0x20];

	void update_reg_ptr();
	const int* m_reg_group;
};


class arm7_be_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm7_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class arm7500_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm7500_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm710a_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm710a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm710a_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);
};

// ARM710a with the BIGEND pin tied high
class arm710a_be_cpu_device : public arm710a_cpu_device
{
public:
	// construction/destruction
	arm710a_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm610_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm610_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm610_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);
};

// ARM610 with the BIGEND pin tied high (Apple Newton)
class arm610_be_cpu_device : public arm610_cpu_device
{
public:
	// construction/destruction
	arm610_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm710t_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm710t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm720t_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm720t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class arm9_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm9_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm9_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness);
};


class arm920t_cpu_device : public arm9_cpu_device
{
public:
	// construction/destruction
	arm920t_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class arm946es_cpu_device : public arm9_cpu_device
{
public:
	// construction/destruction
	arm946es_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// 946E-S has Protection Unit instead of ARM MMU so CP15 is quite different
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;

	virtual void arm7_cpu_write32(offs_t addr, uint32_t data) override;
	virtual void arm7_cpu_write16(offs_t addr, uint16_t data) override;
	virtual void arm7_cpu_write8(offs_t addr, uint8_t data) override;
	virtual uint32_t arm7_cpu_read32(offs_t addr) override;
	virtual uint32_t arm7_cpu_read16(offs_t addr) override;
	virtual uint8_t arm7_cpu_read8(offs_t addr) override;

	virtual uint32_t insn_fetch_word(offs_t addr) override;

protected:
	arm946es_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual const void *insn_fetch_ptr(offs_t addr) override;
	virtual bool drc_memory_rotates_halfwords() const override { return false; }   // LDRH aligns the address and takes the halfword as it is

	virtual void device_start() override ATTR_COLD;

private:
	uint32_t cp15_control, cp15_itcm_base, cp15_dtcm_base, cp15_itcm_size, cp15_dtcm_size;
	uint32_t cp15_itcm_end, cp15_dtcm_end, cp15_itcm_reg, cp15_dtcm_reg;
	uint8_t ITCM[0x8000], DTCM[0x4000];

	void RefreshITCM();
	void RefreshDTCM();
};

class arm11_cpu_device : public arm9_cpu_device
{
public:
	// construction/destruction
	arm11_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm11_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags, endianness_t endianness);
};

class arm1176jzf_s_cpu_device : public arm11_cpu_device
{
public:
	arm1176jzf_s_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual bool get_vfp_flag() const override { return true; }
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
	virtual bool translate_vaddr_to_paddr(offs_t &addr, const int flags) override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;
	virtual void prefetch_abort(uint32_t pc) override;
	virtual uint32_t arm7_cpu_read32(offs_t addr) override;
	virtual uint32_t arm7_cpu_read16(offs_t addr) override;
	virtual uint8_t arm7_cpu_read8(offs_t addr) override;
	virtual void arm7_cpu_write32(offs_t addr, uint32_t data) override;
	virtual void arm7_cpu_write16(offs_t addr, uint16_t data) override;
	virtual void arm7_cpu_write8(offs_t addr, uint8_t data) override;
	virtual bool handle_coprocessor(uint32_t insn) override;
	virtual bool drc_memory_fast_path() const override { return false; }   // CPSR.E, the U bit, its own walker

	bool walk_page_table(offs_t &addr, int flags, bool side_effects);
	bool data_address(offs_t &addr, unsigned size, bool write);
	bool translation_fault(uint32_t addr, uint32_t status, int flags, bool side_effects);
	uint32_t read_data(uint32_t addr, unsigned size);
	void write_data(uint32_t addr, uint32_t data, unsigned size);
	void undefined_coprocessor();
	bool vfp_enabled(bool system_register = false);
	void vfp_execute(uint32_t insn);

	virtual uint32_t vector_base() const override;
	uint32_t m_vbar = 0;
	uint32_t m_aux_control = 0;
	uint32_t m_cpacr = 0;
	uint32_t m_ttbr1 = 0;
	uint32_t m_ttbcr = 0;
	uint32_t m_context_id = 0;
	uint32_t m_thread_id[3] = {};
	uint32_t m_ifar = 0;
	uint32_t m_prrr = 0;
	uint32_t m_nmrr = 0;
	uint32_t m_vfp_regs[32] = {};
	uint32_t m_fpscr = 0;
	uint32_t m_fpexc = 0;
};

class igs036_cpu_device : public arm946es_cpu_device
{
public:
	// construction/destruction
	igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pxa250_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa250_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pxa255_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa255_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pxa270_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	pxa270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sa110_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa110_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	sa110_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);
};

// SA-110 with the CP15 control register B bit set at reset (Apple Newton MessagePad 2x00)
class sa110_be_cpu_device : public sa110_cpu_device
{
public:
	// construction/destruction
	sa110_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sa1100_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class sa1110_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	sa1110_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ARM2 (VLSI VL86C010 / VY86C010): ARMv2 - the 26-bit modes only, no coprocessor 15
class arm2_cpu_device : public arm7_cpu_device
{
public:
	// construction/destruction
	arm2_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	arm2_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t archRev, uint32_t archFlags);

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }

	// no coprocessor 15: every MRC/MCR is an undefined instruction (CDP/LDC/STC already are in the base class)
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
};

// ARM1 (Acorn ARM Evaluation System): ARMv1 - no multiply, no swap
class arm1_cpu_device : public arm2_cpu_device
{
public:
	arm1_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ARM250 (ARM2aS core with MEMC, IOC and VIDC on the same die): ARMv2a - adds SWP
class arm250_cpu_device : public arm2_cpu_device
{
public:
	arm250_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ARM3 (VLSI VL86C020): ARMv2a core with a 4 KB cache that is controlled through coprocessor 15
class arm3_cpu_device : public arm2_cpu_device
{
public:
	arm3_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;

private:
	uint32_t m_cache_control;   // CP15 c2: bit 0 = cache on
	uint32_t m_cacheable;       // CP15 c3: one bit per 2 MB of address space
	uint32_t m_updateable;      // CP15 c4
	uint32_t m_disruptive;      // CP15 c5
};

// ARM60 / ARM61 (VLSI VY86C060, GEC Plessey P60ARM): ARMv3 core without cache or MMU.  PROG32 and DATA32 are input
// pins - set_config_pins(); the default is both low, the 26-bit configuration.  (BIGEND is the device's endianness.)
class arm60_cpu_device : public arm7_cpu_device
{
public:
	arm60_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_config_pins(bool prog32, bool data32);

protected:
	arm60_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness);

	// CP15 has only the ID register and the pin-strapped, read-only control register
	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
};

// ARM60 with the BIGEND pin tied high
class arm60_be_cpu_device : public arm60_cpu_device
{
public:
	arm60_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// Data East DE101 / DE156: an ARM2 core with a BCD/divide coprocessor 0 that apparently allows unaligned LDR PC
class de156_cpu_device : public arm2_cpu_device
{
public:
	de156_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	de156_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint32_t arm7_rt_r_callback(offs_t offset) override;
	virtual void arm7_rt_w_callback(offs_t offset, uint32_t data) override;
	virtual void arm7_do_callback(uint32_t insn) override;

private:
	uint32_t m_copro[16];       // coprocessor 0 register file
};

class de101_cpu_device : public de156_cpu_device
{
public:
	de101_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(ARM1,         arm1_cpu_device)
DECLARE_DEVICE_TYPE(ARM2,         arm2_cpu_device)
DECLARE_DEVICE_TYPE(ARM250,       arm250_cpu_device)
DECLARE_DEVICE_TYPE(ARM3,         arm3_cpu_device)
DECLARE_DEVICE_TYPE(ARM60,        arm60_cpu_device)
DECLARE_DEVICE_TYPE(ARM60_BE,     arm60_be_cpu_device)
DECLARE_DEVICE_TYPE(DE156,        de156_cpu_device)
DECLARE_DEVICE_TYPE(DE101,        de101_cpu_device)
DECLARE_DEVICE_TYPE(ARM7,         arm7_cpu_device)
DECLARE_DEVICE_TYPE(ARM7_BE,      arm7_be_cpu_device)
DECLARE_DEVICE_TYPE(ARM610,       arm610_cpu_device)
DECLARE_DEVICE_TYPE(ARM610_BE,    arm610_be_cpu_device)
DECLARE_DEVICE_TYPE(ARM710A,      arm710a_cpu_device)
DECLARE_DEVICE_TYPE(ARM710A_BE,   arm710a_be_cpu_device)
DECLARE_DEVICE_TYPE(ARM710T,      arm710t_cpu_device)
DECLARE_DEVICE_TYPE(ARM720T,      arm720t_cpu_device)
DECLARE_DEVICE_TYPE(ARM7500,      arm7500_cpu_device)
DECLARE_DEVICE_TYPE(ARM9,         arm9_cpu_device)
DECLARE_DEVICE_TYPE(ARM920T,      arm920t_cpu_device)
DECLARE_DEVICE_TYPE(ARM946ES,     arm946es_cpu_device)
DECLARE_DEVICE_TYPE(ARM11,        arm11_cpu_device)
DECLARE_DEVICE_TYPE(ARM1176JZF_S, arm1176jzf_s_cpu_device)
DECLARE_DEVICE_TYPE(PXA250,       pxa250_cpu_device)
DECLARE_DEVICE_TYPE(PXA255,       pxa255_cpu_device)
DECLARE_DEVICE_TYPE(PXA270,       pxa270_cpu_device)
DECLARE_DEVICE_TYPE(SA110,        sa110_cpu_device)
DECLARE_DEVICE_TYPE(SA110_BE,     sa110_be_cpu_device)
DECLARE_DEVICE_TYPE(SA1100,       sa1100_cpu_device)
DECLARE_DEVICE_TYPE(SA1110,       sa1110_cpu_device)
DECLARE_DEVICE_TYPE(IGS036,       igs036_cpu_device)

#endif // MAME_CPU_ARM7_ARM7_H
