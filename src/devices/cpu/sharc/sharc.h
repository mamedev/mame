// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_CPU_SHARC_SHARC_H
#define MAME_CPU_SHARC_SHARC_H

#pragma once

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"

#define SHARC_INPUT_FLAG0       3
#define SHARC_INPUT_FLAG1       4
#define SHARC_INPUT_FLAG2       5
#define SHARC_INPUT_FLAG3       6


class sharc_frontend;

class adsp21062_device : public cpu_device
{
	friend class sharc_frontend;

public:
	enum sharc_boot_mode
	{
		BOOT_MODE_EPROM,
		BOOT_MODE_HOST,
		BOOT_MODE_LINK,
		BOOT_MODE_NOBOOT
	};


	// construction/destruction
	adsp21062_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~adsp21062_device() override;

	// configuration helpers
	void set_boot_mode(const sharc_boot_mode boot_mode) { m_boot_mode = boot_mode; }

	void set_flag_input(int flag_num, int state);
	void external_iop_write(uint32_t address, uint32_t data);
	void external_dma_write(uint32_t address, uint64_t data);

	TIMER_CALLBACK_MEMBER(sharc_iop_delayed_write_callback);
	TIMER_CALLBACK_MEMBER(sharc_dma_callback);

	void write_stall(int state);

	void enable_recompiler();

	template <unsigned N> uint64_t pm_r(offs_t offset);
	template <unsigned N> void pm_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	template <unsigned N> uint32_t dmw_r(offs_t offset);
	template <unsigned N> void dmw_w(offs_t offset, uint32_t data);
	uint32_t iop_r(offs_t offset);
	void iop_w(offs_t offset, uint32_t data);

	enum ASTAT_FLAGS : uint32_t
	{
		// ASTAT flags
		AZ =    0x000001,   // ALU result zero
		AV =    0x000002,   // ALU overflow
		AN =    0x000004,   // ALU result negative
		AC =    0x000008,   // ALU fixed-point carry
		AS =    0x000010,   // ALU X input sign
		AI =    0x000020,   // ALU floating-point invalid operation
		MN =    0x000040,   // Multiplier result negative
		MV =    0x000080,   // Multiplier overflow
		MU =    0x000100,   // Multiplier underflow
		MI =    0x000200,   // Multiplier floating-point invalid operation
		AF =    0x000400,
		SV =    0x000800,   // Shifter overflow
		SZ =    0x001000,   // Shifter result zero
		SS =    0x002000,   // Shifter input sign
		BTF =   0x040000,   // Bit Test Flag
		FLG0 =  0x080000,   // FLAG0
		FLG1 =  0x100000,   // FLAG1
		FLG2 =  0x200000,   // FLAG2
		FLG3 =  0x400000    // FLAG3
	};

	enum ASTAT_SHIFT
	{
		AZ_SHIFT = 0,
		AV_SHIFT = 1,
		AN_SHIFT = 2,
		AC_SHIFT = 3,
		AS_SHIFT = 4,
		AI_SHIFT = 5,
		MN_SHIFT = 6,
		MV_SHIFT = 7,
		MU_SHIFT = 8,
		MI_SHIFT = 9,
		AF_SHIFT = 10,
		SV_SHIFT = 11,
		SZ_SHIFT = 12,
		SS_SHIFT = 13,
		BTF_SHIFT = 18,
		FLG0_SHIFT = 19,
		FLG1_SHIFT = 20,
		FLG2_SHIFT = 21,
		FLG3_SHIFT = 22
	};

	enum MODE1_DELAY_MODE
	{
		MODE1_WRITE_IMM,
		MODE1_WRITE_REG,
		MODE1_SET,
		MODE1_CLEAR,
		MODE1_TOGGLE,
	};

	enum
	{
		EXCEPTION_INTERRUPT = 0,
		EXCEPTION_COUNT
	};

protected:
	adsp21062_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			address_map_constructor internal_pgm,
			address_map_constructor internal_data);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 8; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 8; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void pgm_2m(address_map &map) ATTR_COLD;
	void pgm_4m(address_map &map) ATTR_COLD;
	void data_2m(address_map &map) ATTR_COLD;
	void data_4m(address_map &map) ATTR_COLD;

private:
	// STKY flags
	static constexpr uint32_t AUS =             0x0000'0001;    // ALU floating-point underflow
	static constexpr uint32_t AVS =             0x0000'0002;    // ALU floating-point overflow
	static constexpr uint32_t AOS =             0x0000'0004;    // ALU fixed-point overflow
	static constexpr uint32_t AIS =             0x0000'0020;    // ALU floating-point invalid operation

	// MODE1 flags
	static constexpr uint32_t MODE1_BR8 =       0x0000'0001;    // Bit-reverse for I8
	static constexpr uint32_t MODE1_BR0 =       0x0000'0002;    // Bit-reverse for I0
	static constexpr uint32_t MODE1_SRCU =      0x0000'0004;    // Alternate register select for computational units
	static constexpr uint32_t MODE1_SRD1H =     0x0000'0008;    // DAG alternate register select (7-4)
	static constexpr uint32_t MODE1_SRD1L =     0x0000'0010;    // DAG alternate register select (3-0)
	static constexpr uint32_t MODE1_SRD2H =     0x0000'0020;    // DAG alternate register select (15-12)
	static constexpr uint32_t MODE1_SRD2L =     0x0000'0040;    // DAG alternate register select (11-8)
	static constexpr uint32_t MODE1_SRRFH =     0x0000'0080;    // Register file alternate select for R(15-8)
	static constexpr uint32_t MODE1_SRRFL =     0x0000'0400;    // Register file alternate select for R(7-0)
	static constexpr uint32_t MODE1_NESTM =     0x0000'0800;    // Interrupt nesting enable
	static constexpr uint32_t MODE1_IRPTEN =    0x0000'1000;    // Global interrupt enable
	static constexpr uint32_t MODE1_ALUSAT =    0x0000'2000;    // Enable ALU fixed-point saturation
	static constexpr uint32_t MODE1_SSE =       0x0000'4000;    // Enable short word sign extension
	static constexpr uint32_t MODE1_TRUNCATE =  0x0000'8000;    // (1) Floating-point truncation / (0) round to nearest
	static constexpr uint32_t MODE1_RND32 =     0x0001'0000;    // (1) 32-bit floating-point rounding / (0) 40-bit rounding
	static constexpr uint32_t MODE1_CSEL =      0x0006'0000;    // CSelect

	// MODE2 flags
	static constexpr uint32_t MODE2_IRQ0E =     0x0000'0001;    // IRQ0 (1) Edge sens. / (0) Level sens.
	static constexpr uint32_t MODE2_IRQ1E =     0x0000'0002;    // IRQ1 (1) Edge sens. / (0) Level sens.
	static constexpr uint32_t MODE2_IRQ2E =     0x0000'0004;    // IRQ2 (1) Edge sens. / (0) Level sens.
	static constexpr uint32_t MODE2_CADIS =     0x0000'0010;    // Cache disable
	static constexpr uint32_t MODE2_TIMEN =     0x0000'0020;    // Timer enable
	static constexpr uint32_t MODE2_BUSLK =     0x0000'0040;    // External bus lock
	static constexpr uint32_t MODE2_FLG0O =     0x0000'8000;    // FLAG0 (1) Output / (0) Input
	static constexpr uint32_t MODE2_FLG1O =     0x0001'0000;    // FLAG1 (1) Output / (0) Input
	static constexpr uint32_t MODE2_FLG2O =     0x0002'0000;    // FLAG2 (1) Output / (0) Input
	static constexpr uint32_t MODE2_FLG3O =     0x0004'0000;    // FLAG3 (1) Output / (0) Input
	static constexpr uint32_t MODE2_CAFRZ =     0x0008'0000;    // Cache freeze


	using opcode_func = void (adsp21062_device::*)();
	struct SHARC_OP
	{
		uint32_t op_mask;
		uint32_t op_bits;
		opcode_func handler;
	};


	union SHARC_REG
	{
		int32_t r;
		float f;
	};

	struct SHARC_DMA_REGS
	{
		uint32_t control;
		uint32_t int_index;
		uint32_t int_modifier;
		uint32_t int_count;
		uint32_t chain_ptr;
		uint32_t gen_purpose;
		uint32_t ext_index;
		uint32_t ext_modifier;
		uint32_t ext_count;
	};

	struct SHARC_DMA_OP
	{
		uint32_t src;
		uint32_t dst;
		uint32_t chain_ptr;
		int32_t src_modifier;
		int32_t dst_modifier;
		int32_t src_count;
		int32_t dst_count;
		int32_t pmode;
		int32_t chained_direction;
		emu_timer *timer;
		bool active;
		bool chained;
	};

	struct alignas(16) sharc_internal_state;

	static const SHARC_OP s_sharc_opcode_table[];
	static const size_t s_num_ops;

	static const uint32_t recips_mantissa_lookup[128];
	static const uint32_t rsqrts_mantissa_lookup[128];


	const address_space_config m_program_config;
	const address_space_config m_data_config;

	sharc_internal_state *m_core;

	sharc_boot_mode m_boot_mode;

	// UML stuff
	drc_cache m_cache;
	std::unique_ptr<drcuml_state> m_drcuml;
	std::unique_ptr<sharc_frontend> m_drcfe;
	uml::parameter   m_regmap[16];

	uml::code_handle *m_entry;
	uml::code_handle *m_nocode;
	uml::code_handle *m_out_of_cycles;
	uml::code_handle *m_reset_cache;
	uml::code_handle *m_pm_read48;
	uml::code_handle *m_pm_write48;
	uml::code_handle *m_pm_read32;
	uml::code_handle *m_pm_write32;
	uml::code_handle *m_dm_read32;
	uml::code_handle *m_dm_write32;
	uml::code_handle *m_push_pc;
	uml::code_handle *m_pop_pc;
	uml::code_handle *m_push_loop;
	uml::code_handle *m_pop_loop;
	uml::code_handle *m_push_status;
	uml::code_handle *m_pop_status;
	uml::code_handle *m_loop_check;
	uml::code_handle *m_call_loop_check;
	uml::code_handle *m_exception[EXCEPTION_COUNT];
	uml::code_handle *m_swap_dag1_0_3;
	uml::code_handle *m_swap_dag1_4_7;
	uml::code_handle *m_swap_dag2_0_3;
	uml::code_handle *m_swap_dag2_4_7;
	uml::code_handle *m_swap_r0_7;
	uml::code_handle *m_swap_r8_15;

	memory_access<24, 3, -3, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<32, 2, -2, ENDIANNESS_LITTLE>::specific m_data;

	required_shared_ptr_array<uint32_t, 2> m_blocks;

	opcode_func m_sharc_op[512];

	bool m_enable_drc;

	inline void CHANGE_PC(uint32_t newpc);
	inline void CHANGE_PC_DELAYED(uint32_t newpc);
	void sharc_iop_delayed_w(uint32_t reg, uint32_t data, int cycles);
	uint32_t pm_read32(uint32_t address);
	void pm_write32(uint32_t address, uint32_t data);
	uint64_t pm_read48(uint32_t address);
	void pm_write48(uint32_t address, uint64_t data);
	uint32_t dm_read32(uint32_t address);
	void dm_write32(uint32_t address, uint32_t data);
	void schedule_chained_dma_op(int channel, uint32_t dma_chain_ptr, int chained_direction);
	void schedule_dma_op(int channel, uint32_t src, uint32_t dst, int src_modifier, int dst_modifier, int src_count, int dst_count, int pmode);
	void dma_op(int channel);
	void sharc_dma_exec(int channel);
	void dma_run_cycle(int channel);
	void add_systemreg_write_latency_effect(int sysreg, uint32_t data, uint32_t prev_data);
	inline void swap_register(uint32_t *a, uint32_t *b);
	void systemreg_write_latency_effect();
	uint32_t GET_UREG(int ureg);
	void SET_UREG(int ureg, uint32_t data);
	void SHIFT_OPERATION_IMM(int shiftop, int data, int rn, int rx);
	void COMPUTE(uint32_t opcode);
	void check_interrupts();
	inline void PUSH_PC(uint32_t pc);
	inline uint32_t POP_PC();
	inline uint32_t TOP_PC();
	inline void PUSH_LOOP(uint32_t addr, uint32_t code, uint32_t type, uint32_t count);
	inline void POP_LOOP();
	inline void PUSH_STATUS_STACK();
	inline void POP_STATUS_STACK();
	inline int IF_CONDITION_CODE(int cond);
	inline int DO_CONDITION_CODE(int cond);
	void sharcop_compute_dreg_dm_dreg_pm();
	void sharcop_compute();
	void sharcop_compute_ureg_dmpm_premod();
	void sharcop_compute_ureg_dmpm_postmod();
	void sharcop_compute_dm_to_dreg_immmod();
	void sharcop_compute_dreg_to_dm_immmod();
	void sharcop_compute_pm_to_dreg_immmod();
	void sharcop_compute_dreg_to_pm_immmod();
	void sharcop_compute_ureg_to_ureg();
	void sharcop_imm_shift_dreg_dmpm();
	void sharcop_imm_shift();
	void sharcop_compute_modify();
	void sharcop_direct_call();
	void sharcop_direct_jump();
	void sharcop_relative_call();
	void sharcop_relative_jump();
	void sharcop_indirect_jump();
	void sharcop_indirect_call();
	void sharcop_relative_jump_compute();
	void sharcop_relative_call_compute();
	void sharcop_indirect_jump_compute_dreg_dm();
	void sharcop_relative_jump_compute_dreg_dm();
	void sharcop_rts();
	void sharcop_rti();
	void sharcop_do_until_counter_imm();
	void sharcop_do_until_counter_ureg();
	void sharcop_do_until();
	void sharcop_dm_to_ureg_direct();
	void sharcop_ureg_to_dm_direct();
	void sharcop_pm_to_ureg_direct();
	void sharcop_ureg_to_pm_direct();
	void sharcop_dm_to_ureg_indirect();
	void sharcop_ureg_to_dm_indirect();
	void sharcop_pm_to_ureg_indirect();
	void sharcop_ureg_to_pm_indirect();
	void sharcop_imm_to_dmpm();
	void sharcop_imm_to_ureg();
	void sharcop_sysreg_bitop();
	void sharcop_modify();
	void sharcop_bit_reverse();
	void sharcop_push_pop_stacks();
	void sharcop_nop();
	void sharcop_idle();
	void sharcop_unimplemented();
	inline void compute_add(int rn, int rx, int ry);
	inline void compute_sub(int rn, int rx, int ry);
	inline void compute_add_ci(int rn, int rx, int ry);
	inline void compute_sub_ci(int rn, int rx, int ry);
	inline void compute_and(int rn, int rx, int ry);
	inline void compute_comp(int rx, int ry);
	inline void compute_pass(int rn, int rx);
	inline void compute_xor(int rn, int rx, int ry);
	inline void compute_or(int rn, int rx, int ry);
	inline void compute_inc(int rn, int rx);
	inline void compute_dec(int rn, int rx);
	inline void compute_min(int rn, int rx, int ry);
	inline void compute_max(int rn, int rx, int ry);
	inline void compute_neg(int rn, int rx);
	inline void compute_not(int rn, int rx);
	inline uint32_t SCALB(SHARC_REG rx, int ry);
	inline void compute_float(int rn, int rx);
	inline void compute_fix(int rn, int rx);
	inline void compute_fix_scaled(int rn, int rx, int ry);
	inline void compute_float_scaled(int rn, int rx, int ry);
	inline void compute_logb(int rn, int rx);
	inline void compute_scalb(int rn, int rx, int ry);
	inline void compute_fadd(int rn, int rx, int ry);
	inline void compute_fsub(int rn, int rx, int ry);
	inline void compute_favg(int rn, int rx, int ry);
	inline void compute_fneg(int rn, int rx);
	inline void compute_fcomp(int rx, int ry);
	inline void compute_fabs_plus(int rn, int rx, int ry);
	inline void compute_fmax(int rn, int rx, int ry);
	inline void compute_fmin(int rn, int rx, int ry);
	inline void compute_fcopysign(int rn, int rx, int ry);
	inline void compute_fclip(int rn, int rx, int ry);
	inline void compute_recips(int rn, int rx);
	inline void compute_rsqrts(int rn, int rx);
	inline void compute_fpass(int rn, int rx);
	inline void compute_fabs(int rn, int rx);
	inline void compute_mul_uuin(int rn, int rx, int ry);
	inline void compute_mul_ssin(int rn, int rx, int ry);
	inline uint32_t compute_mrf_plus_mul_ssin(int rx, int ry);
	inline uint32_t compute_mrb_plus_mul_ssin(int rx, int ry);
	inline void compute_fmul(int rn, int rx, int ry);
	inline void compute_multi_mr_to_reg(int ai, int rk);
	inline void compute_multi_reg_to_mr(int ai, int rk);
	inline void compute_dual_add_sub(int ra, int rs, int rx, int ry);
	inline void compute_mul_ssfr_add(int rm, int rxm, int rym, int ra, int rxa, int rya);
	inline void compute_mul_ssfr_sub(int rm, int rxm, int rym, int ra, int rxa, int rya);
	inline void compute_dual_fadd_fsub(int ra, int rs, int rx, int ry);
	inline void compute_fmul_fadd(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_fsub(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_float_scaled(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_fix_scaled(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_avg(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_abs(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_fmax(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_fmin(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_dual_fadd_fsub(int fm, int fxm, int fym, int fa, int fs, int fxa, int fya);
	void build_opcode_table();

	// internal compiler state
	struct compiler_state
	{
		uint32_t        cycles = 0;     // accumulated cycles
		uint8_t         checkints = 0;  // need to check interrupts before next instruction
		uml::code_label labelnum = 1;   // index for local labels
		struct
		{
			int         counter = 0;
			int         mode = 0;
			uint32_t    data = 0;
		} mode1_delay;
	};

	void execute_run_drc();
	void generate_invariant();
	void flush_drc_cache();
	void compile_block(offs_t pc);
	void alloc_handle(uml::code_handle *&handleptr, const char *name);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_reset_cache();
	void static_generate_memory_accessors();
	void static_generate_exception(uint8_t exception, const char *name);
	void static_generate_push_pc();
	void static_generate_pop_pc();
	void static_generate_push_loop();
	void static_generate_pop_loop();
	void static_generate_push_status();
	void static_generate_pop_status();
	void static_generate_loop_check();
	void static_generate_call_loop_check();
	void static_generate_loop_check_body(drcuml_block &block, bool is_call);
	void static_generate_mode1_ops();
	void load_fast_iregs(drcuml_block &block);
	void save_fast_iregs(drcuml_block &block);
	void generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, bool last_delayslot);
	void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception);
	bool generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_unimplemented_compute(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_compute(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_if_condition(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int condition, int skip_label);
	void generate_shift_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int data, int shiftop, int rn, int rx);
	void generate_call(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, bool delayslot);
	void generate_jump(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, bool delayslot, bool loopabort, bool clearint);
	void generate_write_mode1_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t data);
	void generate_set_mode1_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t data);
	void generate_clear_mode1_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t data);
	void generate_toggle_mode1_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t data);
	void generate_read_ureg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int ureg, bool has_compute);
	void generate_write_ureg(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int ureg, bool imm, uint32_t data);
	void generate_update_circular_buffer(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int g, int i);
	void generate_astat_copy(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);

	bool if_condition_always_true(int condition);
	uint32_t do_condition_astat_bits(int condition);

	void sharc_cfunc_unimplemented();
	void sharc_cfunc_read_iop();
	void sharc_cfunc_write_iop();
	void sharc_cfunc_pcstack_overflow();
	void sharc_cfunc_pcstack_underflow();
	void sharc_cfunc_loopstack_overflow();
	void sharc_cfunc_loopstack_underflow();
	void sharc_cfunc_statusstack_overflow();
	void sharc_cfunc_statusstack_underflow();

	void sharc_cfunc_unimplemented_compute();
	void sharc_cfunc_unimplemented_shiftimm();
	void sharc_cfunc_write_snoop();

	static void cfunc_unimplemented(void *param);
	static void cfunc_pcstack_overflow(void *param);
	static void cfunc_pcstack_underflow(void *param);
	static void cfunc_loopstack_overflow(void *param);
	static void cfunc_loopstack_underflow(void *param);
	static void cfunc_statusstack_overflow(void *param);
	static void cfunc_statusstack_underflow(void *param);
	static void cfunc_unimplemented_compute(void *param);
	static void cfunc_unimplemented_shiftimm(void *param);
};


class adsp21060_device : public adsp21062_device
{
public:
	// construction/destruction
	adsp21060_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~adsp21060_device() override;
};


DECLARE_DEVICE_TYPE(ADSP21062, adsp21062_device)
DECLARE_DEVICE_TYPE(ADSP21060, adsp21060_device)

#endif // MAME_CPU_SHARC_SHARC_H
