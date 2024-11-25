// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_E132XS_E132XS_H
#define MAME_CPU_E132XS_E132XS_H

#pragma once

#include "32xsdasm.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

/*
    A note about clock multipliers and dividers:

    E1-16[T] and E1-32[T] accept a straight clock

    E1-16X[T|N] and E1-32X[T|N] accept a clock and multiply it
        internally by 4; in the emulator, you MUST specify 4 * XTAL
        to achieve the correct speed

    E1-16XS[R] and E1-32XS[R] accept a clock and multiply it
        internally by 8; in the emulator, you MUST specify 8 * XTAL
        to achieve the correct speed
*/



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC                       M0
#define MAPVAR_CYCLES                   M1

#define E132XS_STRICT_VERIFY            0x0001          /* verify all instructions */

#define SINGLE_INSTRUCTION_MODE         (1)

#define ENABLE_E132XS_DRC               (1)

#define E132XS_LOG_DRC_REGS             (0)
#define E132XS_LOG_INTERPRETER_REGS     (0)
#define E132XS_COUNT_INSTRUCTIONS       (0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class e132xs_frontend;

// ======================> hyperstone_device

enum
{
	E132XS_PC = 1,
	E132XS_SR,
	E132XS_FER,
	E132XS_G3,
	E132XS_G4,
	E132XS_G5,
	E132XS_G6,
	E132XS_G7,
	E132XS_G8,
	E132XS_G9,
	E132XS_G10,
	E132XS_G11,
	E132XS_G12,
	E132XS_G13,
	E132XS_G14,
	E132XS_G15,
	E132XS_G16,
	E132XS_G17,
	E132XS_SP,
	E132XS_UB,
	E132XS_BCR,
	E132XS_TPR,
	E132XS_TCR,
	E132XS_TR,
	E132XS_WCR,
	E132XS_ISR,
	E132XS_FCR,
	E132XS_MCR,
	E132XS_G28,
	E132XS_G29,
	E132XS_G30,
	E132XS_G31,
	E132XS_CL0, E132XS_CL1, E132XS_CL2, E132XS_CL3,
	E132XS_CL4, E132XS_CL5, E132XS_CL6, E132XS_CL7,
	E132XS_CL8, E132XS_CL9, E132XS_CL10, E132XS_CL11,
	E132XS_CL12, E132XS_CL13, E132XS_CL14, E132XS_CL15,
	E132XS_L0, E132XS_L1, E132XS_L2, E132XS_L3,
	E132XS_L4, E132XS_L5, E132XS_L6, E132XS_L7,
	E132XS_L8, E132XS_L9, E132XS_L10, E132XS_L11,
	E132XS_L12, E132XS_L13, E132XS_L14, E132XS_L15,
	E132XS_L16, E132XS_L17, E132XS_L18, E132XS_L19,
	E132XS_L20, E132XS_L21, E132XS_L22, E132XS_L23,
	E132XS_L24, E132XS_L25, E132XS_L26, E132XS_L27,
	E132XS_L28, E132XS_L29, E132XS_L30, E132XS_L31,
	E132XS_L32, E132XS_L33, E132XS_L34, E132XS_L35,
	E132XS_L36, E132XS_L37, E132XS_L38, E132XS_L39,
	E132XS_L40, E132XS_L41, E132XS_L42, E132XS_L43,
	E132XS_L44, E132XS_L45, E132XS_L46, E132XS_L47,
	E132XS_L48, E132XS_L49, E132XS_L50, E132XS_L51,
	E132XS_L52, E132XS_L53, E132XS_L54, E132XS_L55,
	E132XS_L56, E132XS_L57, E132XS_L58, E132XS_L59,
	E132XS_L60, E132XS_L61, E132XS_L62, E132XS_L63
};

// Used by core CPU interface
class hyperstone_device : public cpu_device, public hyperstone_disassembler::config
{
	friend class e132xs_frontend;

public:
	virtual ~hyperstone_device() override;

	inline void ccfunc_unimplemented();
	inline void ccfunc_print();
	inline void ccfunc_total_cycles();
	inline void ccfunc_standard_irq_callback();

#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	void dump_registers();
#endif
	void update_timer_prescale();
	void compute_tr();
	void adjust_timer_interrupt();

	void e116_16k_iram_map(address_map &map) ATTR_COLD;
	void e116_4k_iram_map(address_map &map) ATTR_COLD;
	void e116_8k_iram_map(address_map &map) ATTR_COLD;
	void e132_16k_iram_map(address_map &map) ATTR_COLD;
	void e132_4k_iram_map(address_map &map) ATTR_COLD;
	void e132_8k_iram_map(address_map &map) ATTR_COLD;

	static uint32_t imm_length(uint16_t op);

protected:
	// compilation boundaries -- how far back/forward does the analysis extend?
	enum : u32
	{
		COMPILE_BACKWARDS_BYTES     = 128,
		COMPILE_FORWARDS_BYTES      = 512,
		COMPILE_MAX_INSTRUCTIONS    = (COMPILE_BACKWARDS_BYTES / 4) + (COMPILE_FORWARDS_BYTES / 4),
		COMPILE_MAX_SEQUENCE        = 64
	};

	// exit codes
	enum : int
	{
		EXECUTE_OUT_OF_CYCLES       = 0,
		EXECUTE_MISSING_CODE        = 1,
		EXECUTE_UNMAPPED_CODE       = 2,
		EXECUTE_RESET_CACHE         = 3
	};

	struct internal_hyperstone_state
	{
		// CPU registers
		uint32_t  global_regs[32];
		uint32_t  local_regs[64];
		uint8_t   fl_lut[16];

		/* internal stuff */
		uint32_t  trap_entry;   // entry point to get trap address

		uint8_t   clock_scale_mask;
		uint8_t   clck_scale;
		uint32_t  clock_cycles_1;
		uint32_t  clock_cycles_2;
		uint32_t  clock_cycles_3;
		uint32_t  clock_cycles_4;
		uint32_t  clock_cycles_6;
		uint32_t  clock_cycles_36;

		uint64_t  tr_base_cycles;
		uint32_t  tr_base_value;
		uint32_t  tr_result;
		uint32_t  tr_clocks_per_tick;
		uint32_t  timer_int_pending;

		uint64_t  numcycles;

		uint32_t  delay_pc;
		uint32_t  delay_slot;
		uint32_t  delay_slot_taken;

		int32_t   intblock;

		uint32_t  arg0;
		uint32_t  arg1;

		// other internal state
		int       icount;
	};

	enum reg_bank
	{
		LOCAL = 0,
		GLOBAL = 1
	};

	enum imm_size
	{
		SIMM = 0,
		LIMM = 1
	};

	enum shift_type
	{
		N_LO = 0,
		N_HI = 1
	};

	enum sign_mode
	{
		IS_UNSIGNED = 0,
		IS_SIGNED = 1
	};

	enum branch_condition
	{
		COND_V = 0,
		COND_Z = 1,
		COND_C = 2,
		COND_CZ = 3,
		COND_N = 4,
		COND_NZ = 5
	};

	enum condition_set
	{
		IS_CLEAR = 0,
		IS_SET = 1
	};

	enum trap_exception_or_int
	{
		IS_TRAP = 0,
		IS_INT = 1,
		IS_EXCEPTION = 2
	};

	enum is_timer
	{
		NO_TIMER = 0,
		IS_TIMER = 1
	};

	enum
	{
		EXCEPTION_IO2                  = 48,
		EXCEPTION_IO1                  = 49,
		EXCEPTION_INT4                 = 50,
		EXCEPTION_INT3                 = 51,
		EXCEPTION_INT2                 = 52,
		EXCEPTION_INT1                 = 53,
		EXCEPTION_IO3                  = 54,
		EXCEPTION_TIMER                = 55,
		EXCEPTION_RESERVED1            = 56,
		EXCEPTION_TRACE                = 57,
		EXCEPTION_PARITY_ERROR         = 58,
		EXCEPTION_EXTENDED_OVERFLOW    = 59,
		EXCEPTION_RANGE_ERROR          = 60,
		EXCEPTION_PRIVILEGE_ERROR      = EXCEPTION_RANGE_ERROR,
		EXCEPTION_FRAME_ERROR          = EXCEPTION_RANGE_ERROR,
		EXCEPTION_RESERVED2            = 61,
		EXCEPTION_RESET                = 62,  // reserved if not mapped @ MEM3
		EXCEPTION_ERROR_ENTRY          = 63,  // for instruction code of all ones
		EXCEPTION_COUNT
	};

	// construction/destruction
	hyperstone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
						const device_type type, uint32_t prg_data_width, uint32_t io_data_width, address_map_constructor internal_map);

	void init(int scale_mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual u8 get_fp() const override;
	virtual bool get_h() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_io_config;
	address_space *m_program;
	memory_access<32, 1, 0, ENDIANNESS_BIG>::cache m_cache16;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache32;

	std::function<u16 (offs_t)> m_pr16;
	std::function<const void * (offs_t)> m_prptr;
	address_space *m_io;

	uint16_t  m_op;           // opcode

	/* core state */
	internal_hyperstone_state *m_core;

	int32_t m_instruction_length;
	bool m_instruction_length_valid;

	emu_timer *m_timer;

	static const uint32_t s_trap_entries[8];
	static const int32_t s_immediate_values[16];

	uint32_t m_op_counts[256];
	uint32_t get_trap_addr(uint8_t trapno);

private:
	// internal functions
	template <hyperstone_device::is_timer TIMER> void check_interrupts();

	void set_global_register(uint8_t code, uint32_t val);
	void set_local_register(uint8_t code, uint32_t val);

	uint32_t get_global_register(uint8_t code);

	// words interpreted as pairs of signed half-words (HS)
	static int get_lhs(uint32_t val) { return int16_t(val & 0xffff); }
	static int get_rhs(uint32_t val) { return int16_t(val >> 16); }

	uint32_t get_emu_code_addr(uint8_t num);
	int32_t get_instruction_length(uint16_t op);

	TIMER_CALLBACK_MEMBER(timer_callback);

	uint32_t decode_const();
	uint32_t decode_immediate_s();
	void ignore_immediate_s();
	int32_t decode_pcrel();
	void ignore_pcrel();

	void hyperstone_br();
	void execute_trap(uint32_t addr);
	void execute_int(uint32_t addr);
	void execute_exception(uint32_t addr);
	void execute_software();

	template <reg_bank DST_GLOBAL> uint64_t get_double_word(uint8_t dst_code, uint8_t dstf_code) const;
	template <reg_bank DST_GLOBAL> void set_double_word(uint8_t dst_code, uint8_t dstf_code, uint64_t val);

	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_chk();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_movd();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL, sign_mode SIGNED> void hyperstone_divsu();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_xm();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_mask();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_sum();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_sums();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_cmp();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_mov();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_add();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_adds();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_cmpb();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_subc();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_sub();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_subs();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_addc();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_neg();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_negs();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_and();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_andn();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_or();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_xor();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_not();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_cmpi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_movi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_addi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_addsi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_cmpbi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_andni();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_ori();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_xori();
	template <shift_type HI_N> void hyperstone_shrdi();
	void hyperstone_shrd();
	void hyperstone_shr();
	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_shri();
	template <shift_type HI_N> void hyperstone_sardi();
	void hyperstone_sard();
	void hyperstone_sar();
	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_sari();
	template <shift_type HI_N> void hyperstone_shldi();
	void hyperstone_shld();
	void hyperstone_shl();
	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_shli();
	void hyperstone_testlz();
	void hyperstone_rol();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_ldxx1();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_ldxx2();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_stxx1();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_stxx2();

	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL, sign_mode SIGNED> void hyperstone_mulsu();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_mul();

	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_set();

	template <reg_bank SRC_GLOBAL> void hyperstone_ldwr();
	template <reg_bank SRC_GLOBAL> void hyperstone_lddr();
	template <reg_bank SRC_GLOBAL> void hyperstone_ldwp();
	template <reg_bank SRC_GLOBAL> void hyperstone_lddp();

	template <reg_bank SRC_GLOBAL> void hyperstone_stwr();
	template <reg_bank SRC_GLOBAL> void hyperstone_stdr();
	template <reg_bank SRC_GLOBAL> void hyperstone_stwp();
	template <reg_bank SRC_GLOBAL> void hyperstone_stdp();

	template <branch_condition CONDITION, condition_set COND_SET> void hyperstone_b();
	template <branch_condition CONDITION, condition_set COND_SET> void hyperstone_db();
	void hyperstone_dbr();

	void hyperstone_frame();
	template <hyperstone_device::reg_bank SRC_GLOBAL> void hyperstone_call();

	void hyperstone_trap();
	void hyperstone_extend();

	void hyperstone_reserved();
	void hyperstone_do();

#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	FILE *m_trace_log;
#endif

	drc_cache m_cache;
	std::unique_ptr<drcuml_state> m_drcuml;
	std::unique_ptr<e132xs_frontend> m_drcfe;
	uint32_t m_drcoptions;
	uint8_t m_cache_dirty;

	uml::code_handle *m_entry;
	uml::code_handle *m_nocode;
	uml::code_handle *m_interrupt_checks;
	uml::code_handle *m_out_of_cycles;

	uml::code_handle *m_mem_read8;
	uml::code_handle *m_mem_write8;
	uml::code_handle *m_mem_read16;
	uml::code_handle *m_mem_write16;
	uml::code_handle *m_mem_read32;
	uml::code_handle *m_mem_write32;
	uml::code_handle *m_io_read32;
	uml::code_handle *m_io_write32;
	uml::code_handle *m_exception[EXCEPTION_COUNT];

	bool m_enable_drc;

	/* internal compiler state */
	struct compiler_state
	{
		compiler_state(compiler_state const &) = delete;
		compiler_state &operator=(compiler_state const &) = delete;

		uint32_t m_cycles;          /* accumulated cycles */
		uint8_t m_checkints;        /* need to check interrupts before next instruction */
		uml::code_label m_labelnum; /* index for local labels */
	};

	void execute_run_drc();
	void flush_drc_cache();
	void code_flush_cache();
	void code_compile_block(offs_t pc);
	//void load_fast_iregs(drcuml_block &block);
	//void save_fast_iregs(drcuml_block &block);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_exception(uint32_t exception, const char *name);
	void static_generate_memory_accessor(int size, int iswrite, bool isio, const char *name, uml::code_handle *&handleptr);
	void static_generate_interrupt_checks();
	void generate_interrupt_checks_no_timer(drcuml_block &block, uml::code_label &labelnum);
	void generate_interrupt_checks_with_timer(drcuml_block &block, uml::code_label &labelnum);
	void generate_branch(drcuml_block &block, uml::parameter targetpc, const opcode_desc *desc, bool update_cycles = true);
	void generate_update_cycles(drcuml_block &block, bool check_interrupts = true);
	void generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op);
	bool generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	void generate_get_trap_addr(drcuml_block &block, uml::code_label &label, uint32_t trapno);
	void generate_check_delay_pc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_decode_const(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_decode_immediate_s(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_ignore_immediate_s(drcuml_block &block, const opcode_desc *desc);
	void generate_decode_pcrel(drcuml_block &block, const opcode_desc *desc);
	void generate_ignore_pcrel(drcuml_block &block, const opcode_desc *desc);

	void generate_get_global_register(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_set_global_register(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <trap_exception_or_int TYPE> void generate_trap_exception_or_int(drcuml_block &block);
	void generate_int(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t addr);
	void generate_exception(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t addr);
	void generate_software(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_chk(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_movd(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL, sign_mode SIGNED> void generate_divsu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_xm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_mask(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_sum(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_sums(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_cmp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_mov(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_add(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_adds(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_cmpb(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_subc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_sub(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_subs(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_addc(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_neg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_negs(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_and(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_andn(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_or(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_xor(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_not(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_cmpi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_movi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_addi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_addsi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_cmpbi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_andni(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_ori(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void generate_xori(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <shift_type HI_N> void generate_shrdi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_shrd(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_shr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <shift_type HI_N, reg_bank DST_GLOBAL> void generate_shri(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <shift_type HI_N> void generate_sardi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_sard(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_sar(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <shift_type HI_N, reg_bank DST_GLOBAL> void generate_sari(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <shift_type HI_N> void generate_shldi(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_shld(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_shl(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <shift_type HI_N, reg_bank DST_GLOBAL> void generate_shli(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_testlz(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_rol(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_ldxx1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_ldxx2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_stxx1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_stxx2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL, sign_mode SIGNED> void generate_mulsu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void generate_mul(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <shift_type HI_N, reg_bank DST_GLOBAL> void generate_set(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <reg_bank SRC_GLOBAL> void generate_ldwr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank SRC_GLOBAL> void generate_lddr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank SRC_GLOBAL> void generate_ldwp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank SRC_GLOBAL> void generate_lddp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <reg_bank SRC_GLOBAL> void generate_stwr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank SRC_GLOBAL> void generate_stdr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank SRC_GLOBAL> void generate_stwp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <reg_bank SRC_GLOBAL> void generate_stdp(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	template <branch_condition CONDITION, condition_set COND_SET> void generate_b(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_br(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <branch_condition CONDITION, condition_set COND_SET> void generate_db(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_dbr(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	void generate_frame(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	template <hyperstone_device::reg_bank SRC_GLOBAL> void generate_call(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	void generate_trap_op(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_extend(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	void generate_reserved(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_do(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
};

// device type definition
DECLARE_DEVICE_TYPE(E116T,      e116t_device)
DECLARE_DEVICE_TYPE(E116XT,     e116xt_device)
DECLARE_DEVICE_TYPE(E116XS,     e116xs_device)
DECLARE_DEVICE_TYPE(E116XSR,    e116xsr_device)
DECLARE_DEVICE_TYPE(E132N,      e132n_device)
DECLARE_DEVICE_TYPE(E132T,      e132t_device)
DECLARE_DEVICE_TYPE(E132XN,     e132xn_device)
DECLARE_DEVICE_TYPE(E132XT,     e132xt_device)
DECLARE_DEVICE_TYPE(E132XS,     e132xs_device)
DECLARE_DEVICE_TYPE(E132XSR,    e132xsr_device)
DECLARE_DEVICE_TYPE(GMS30C2116, gms30c2116_device)
DECLARE_DEVICE_TYPE(GMS30C2132, gms30c2132_device)
DECLARE_DEVICE_TYPE(GMS30C2216, gms30c2216_device)
DECLARE_DEVICE_TYPE(GMS30C2232, gms30c2232_device)


// ======================> e116t_device

class e116t_device : public hyperstone_device
{
public:
	// construction/destruction
	e116t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e116xt_device

class e116xt_device : public hyperstone_device
{
public:
	// construction/destruction
	e116xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e116xs_device

class e116xs_device : public hyperstone_device
{
public:
	// construction/destruction
	e116xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e116xsr_device

class e116xsr_device : public hyperstone_device
{
public:
	// construction/destruction
	e116xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e132n_device

class e132n_device : public hyperstone_device
{
public:
	// construction/destruction
	e132n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e132t_device

class e132t_device : public hyperstone_device
{
public:
	// construction/destruction
	e132t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e132xn_device

class e132xn_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e132xt_device

class e132xt_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e132xs_device

class e132xs_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> e132xsr_device

class e132xsr_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> gms30c2116_device

class gms30c2116_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> gms30c2132_device

class gms30c2132_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> gms30c2216_device

class gms30c2216_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2216_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


// ======================> gms30c2232_device

class gms30c2232_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


#endif // MAME_CPU_E132XS_E132XS_H
