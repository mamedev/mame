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


// STKY flags
#define AUS     0x1         /* ALU floating-point underflow */
#define AVS     0x2         /* ALU floating-point overflow */
#define AOS     0x4         /* ALU fixed-point overflow */
#define AIS     0x20        /* ALU floating-point invalid operation */

// MODE1 flags
#define MODE1_BR8           0x1         /* Bit-reverse for I8 */
#define MODE1_BR0           0x2         /* Bit-reverse for I0 */
#define MODE1_SRCU          0x4         /* Alternate register select for computational units */
#define MODE1_SRD1H         0x8         /* DAG alternate register select (7-4) */
#define MODE1_SRD1L         0x10        /* DAG alternate register select (3-0) */
#define MODE1_SRD2H         0x20        /* DAG alternate register select (15-12) */
#define MODE1_SRD2L         0x40        /* DAG alternate register select (11-8) */
#define MODE1_SRRFH         0x80        /* Register file alternate select for R(15-8) */
#define MODE1_SRRFL         0x400       /* Register file alternate select for R(7-0) */
#define MODE1_NESTM         0x800       /* Interrupt nesting enable */
#define MODE1_IRPTEN        0x1000      /* Global interrupt enable */
#define MODE1_ALUSAT        0x2000      /* Enable ALU fixed-point saturation */
#define MODE1_SSE           0x4000      /* Enable short word sign extension */
#define MODE1_TRUNCATE      0x8000      /* (1) Floating-point truncation / (0) round to nearest */
#define MODE1_RND32         0x10000     /* (1) 32-bit floating-point rounding / (0) 40-bit rounding */
#define MODE1_CSEL          0x60000     /* CSelect */

// MODE2 flags
#define MODE2_IRQ0E         0x1         /* IRQ0 (1) Edge sens. / (0) Level sens. */
#define MODE2_IRQ1E         0x2         /* IRQ1 (1) Edge sens. / (0) Level sens. */
#define MODE2_IRQ2E         0x4         /* IRQ2 (1) Edge sens. / (0) Level sens. */
#define MODE2_CADIS         0x10        /* Cache disable */
#define MODE2_TIMEN         0x20        /* Timer enable */
#define MODE2_BUSLK         0x40        /* External bus lock */
#define MODE2_FLG0O         0x8000      /* FLAG0 (1) Output / (0) Input */
#define MODE2_FLG1O         0x10000     /* FLAG1 (1) Output / (0) Input */
#define MODE2_FLG2O         0x20000     /* FLAG2 (1) Output / (0) Input */
#define MODE2_FLG3O         0x40000     /* FLAG3 (1) Output / (0) Input */
#define MODE2_CAFRZ         0x80000     /* Cache freeze */


#define OP_USERFLAG_COUNTER_LOOP            0x00000001
#define OP_USERFLAG_COND_LOOP               0x00000002
#define OP_USERFLAG_COND_FIELD              0x000003fc
#define OP_USERFLAG_COND_FIELD_SHIFT        2
#define OP_USERFLAG_ASTAT_DELAY_COPY_AZ     0x00001000
#define OP_USERFLAG_ASTAT_DELAY_COPY_AN     0x00002000
#define OP_USERFLAG_ASTAT_DELAY_COPY_AC     0x00004000
#define OP_USERFLAG_ASTAT_DELAY_COPY_AV     0x00008000
#define OP_USERFLAG_ASTAT_DELAY_COPY_MV     0x00010000
#define OP_USERFLAG_ASTAT_DELAY_COPY_MN     0x00020000
#define OP_USERFLAG_ASTAT_DELAY_COPY_SV     0x00040000
#define OP_USERFLAG_ASTAT_DELAY_COPY_SZ     0x00080000
#define OP_USERFLAG_ASTAT_DELAY_COPY_BTF    0x00100000
#define OP_USERFLAG_ASTAT_DELAY_COPY        0x001ff000
#define OP_USERFLAG_CALL                    0x10000000


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
	adsp21062_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);
	virtual ~adsp21062_device() override;

	// configuration helpers
	void set_boot_mode(const sharc_boot_mode boot_mode) { m_boot_mode = boot_mode; }

	void set_flag_input(int flag_num, int state);
	void external_iop_write(uint32_t address, uint32_t data);
	void external_dma_write(uint32_t address, uint64_t data);

	TIMER_CALLBACK_MEMBER(sharc_iop_delayed_write_callback);
	TIMER_CALLBACK_MEMBER(sharc_dma_callback);

	void write_stall(int state);

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

	void enable_recompiler();

	uint64_t pm0_r(offs_t offset);
	void pm0_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t pm1_r(offs_t offset);
	void pm1_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint32_t dmw0_r(offs_t offset);
	void dmw0_w(offs_t offset, uint32_t data);
	uint32_t dmw1_r(offs_t offset);
	void dmw1_w(offs_t offset, uint32_t data);
	uint32_t iop_r(offs_t offset);
	void iop_w(offs_t offset, uint32_t data);

	enum ASTAT_FLAGS
	{
		// ASTAT flags
		AZ =    0x1,         /* ALU result zero */
		AV =    0x2,         /* ALU overflow */
		AN =    0x4,         /* ALU result negative */
		AC =    0x8,         /* ALU fixed-point carry */
		AS =    0x10,        /* ALU X input sign */
		AI =    0x20,        /* ALU floating-point invalid operation */
		MN =    0x40,        /* Multiplier result negative */
		MV =    0x80,        /* Multiplier overflow */
		MU =    0x100,       /* Multiplier underflow */
		MI =    0x200,       /* Multiplier floating-point invalid operation */
		AF =    0x400,
		SV =    0x800,      /* Shifter overflow */
		SZ =    0x1000,     /* Shifter result zero */
		SS =    0x2000,     /* Shifter input sign */
		BTF =   0x40000,    /* Bit Test Flag */
		FLG0 =  0x80000,    /* FLAG0 */
		FLG1 =  0x100000,   /* FLAG1 */
		FLG2 =  0x200000,   /* FLAG2 */
		FLG3 =  0x400000    /* FLAG3 */
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

	enum MEM_ACCESSOR_TYPE
	{
		MEM_ACCESSOR_PM_READ48,
		MEM_ACCESSOR_PM_WRITE48,
		MEM_ACCESSOR_PM_READ32,
		MEM_ACCESSOR_PM_WRITE32,
		MEM_ACCESSOR_DM_READ32,
		MEM_ACCESSOR_DM_WRITE32
	};

	enum
	{
		EXCEPTION_INTERRUPT = 0,
		EXCEPTION_COUNT
	};

	void internal_data(address_map &map) ATTR_COLD;
	void internal_pgm(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 8; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 8; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	struct alignas(16) SHARC_DAG
	{
		uint32_t i[8];
		uint32_t m[8];
		uint32_t b[8];
		uint32_t l[8];
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

	struct SHARC_LADDR
	{
		uint32_t addr;
		uint32_t code;
		uint32_t loop_type;
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


	address_space_config m_program_config;
	address_space_config m_data_config;

	typedef void (adsp21062_device::*opcode_func)();
	struct SHARC_OP
	{
		uint32_t op_mask;
		uint32_t op_bits;
		opcode_func handler;
	};
	static const SHARC_OP s_sharc_opcode_table[];

	struct ASTAT_DRC
	{
		union
		{
			struct
			{
				uint32_t az;
				uint32_t av;
				uint32_t an;
				uint32_t ac;
				uint32_t as;
				uint32_t ai;
				uint32_t mn;
				uint32_t mv;
				uint32_t mu;
				uint32_t mi;
				uint32_t sv;
				uint32_t sz;
				uint32_t ss;
				uint32_t btf;
				uint32_t af;
				uint32_t cacc;
			};
			uint64_t flags64[8];
		};
	};

	struct alignas(16) sharc_internal_state
	{
		SHARC_REG r[16];
		SHARC_REG reg_alt[16];

		uint32_t pc;
		uint64_t mrf;
		uint64_t mrb;

		uint32_t pcstack[32];
		uint32_t lcstack[6];
		uint32_t lastack[6];
		uint32_t lstkp;

		uint32_t faddr;
		uint32_t daddr;
		uint32_t pcstk;
		uint32_t pcstkp;
		SHARC_LADDR laddr;
		uint32_t curlcntr;
		uint32_t lcntr;
		uint8_t extdma_shift;

		/* Data Address Generator (DAG) */
		SHARC_DAG dag1;     // (DM bus)
		SHARC_DAG dag2;     // (PM bus)
		SHARC_DAG dag1_alt;
		SHARC_DAG dag2_alt;

		SHARC_DMA_REGS dma[12];

		/* System registers */
		uint32_t mode1;
		uint32_t mode2;
		uint32_t astat;
		uint32_t stky;
		uint32_t irptl;
		uint32_t imask;
		uint32_t imaskp;
		uint32_t ustat1;
		uint32_t ustat2;

		uint32_t flag[4];

		uint32_t syscon;
		uint32_t sysstat;

		struct
		{
			uint32_t mode1;
			uint32_t astat;
		} status_stack[5];
		int32_t status_stkp;

		uint64_t px;

		int icount;
		uint64_t opcode;

		uint32_t nfaddr;

		int32_t idle;
		int32_t irq_pending;
		int32_t active_irq_num;

		SHARC_DMA_OP dma_op[12];
		uint32_t dma_status;
		bool write_stalled;

		int32_t interrupt_active;

		uint32_t iop_delayed_reg;
		uint32_t iop_delayed_data;
		emu_timer *delayed_iop_timer;

		uint32_t delay_slot1, delay_slot2;

		int32_t systemreg_latency_cycles;
		int32_t systemreg_latency_reg;
		uint32_t systemreg_latency_data;
		uint32_t systemreg_previous_data;

		uint32_t astat_old;
		uint32_t astat_old_old;
		uint32_t astat_old_old_old;

		uint32_t arg0;
		uint32_t arg1;
		uint32_t arg2;
		uint32_t arg3;

		uint64_t arg64;
		uint32_t mode1_delay_data;

		ASTAT_DRC astat_drc;
		ASTAT_DRC astat_drc_copy;
		ASTAT_DRC astat_delay_copy;
		uint32_t dreg_temp;
		uint32_t dreg_temp2;
		uint32_t jmpdest;
		uint32_t temp_return;

		float fp0;
		float fp1;

		uint32_t force_recompile;
		uint32_t cache_dirty;
	};

	sharc_internal_state* m_core;

	sharc_boot_mode m_boot_mode;

	// UML stuff
	drc_cache m_cache;
	std::unique_ptr<drcuml_state> m_drcuml;
	std::unique_ptr<sharc_frontend> m_drcfe;
	uml::parameter   m_regmap[16];

	uml::code_handle *m_entry;                      /* entry point */
	uml::code_handle *m_nocode;                     /* nocode exception handler */
	uml::code_handle *m_out_of_cycles;              /* out of cycles exception handler */
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
	uml::code_handle *m_exception[EXCEPTION_COUNT];     // exception handlers
	uml::code_handle *m_swap_dag1_0_3;
	uml::code_handle *m_swap_dag1_4_7;
	uml::code_handle *m_swap_dag2_0_3;
	uml::code_handle *m_swap_dag2_4_7;
	uml::code_handle *m_swap_r0_7;
	uml::code_handle *m_swap_r8_15;

	address_space *m_program;
	address_space *m_data;

	required_shared_ptr<uint32_t> m_block0, m_block1;

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

	/* internal compiler state */
	struct compiler_state
	{
		uint32_t cycles;                             /* accumulated cycles */
		uint8_t  checkints;                          /* need to check interrupts before next instruction */
		uml::code_label  labelnum;                 /* index for local labels */
		struct
		{
			int counter;
			int mode;
			uint32_t data;
		} mode1_delay;
	};

	void execute_run_drc();
	void flush_cache();
	void compile_block(offs_t pc);
	void alloc_handle(uml::code_handle *&handleptr, const char *name);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(MEM_ACCESSOR_TYPE type, const char *name, uml::code_handle *&handleptr);
	void static_generate_exception(uint8_t exception, const char *name);
	void static_generate_push_pc();
	void static_generate_pop_pc();
	void static_generate_push_loop();
	void static_generate_pop_loop();
	void static_generate_push_status();
	void static_generate_pop_status();
	void static_generate_mode1_ops();
	void load_fast_iregs(drcuml_block &block);
	void save_fast_iregs(drcuml_block &block);
	void generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, bool last_delayslot);
	void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception);
	bool generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_unimplemented_compute(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_compute(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_if_condition(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int condition, int skip_label);
	void generate_do_condition(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int condition, int skip_label, ASTAT_DRC &astat);
	void generate_shift_imm(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int data, int shiftop, int rn, int rx);
	void generate_call(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, bool delayslot);
	void generate_jump(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, bool delayslot, bool loopabort, bool clearint);
	void generate_loop_jump(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
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
};


DECLARE_DEVICE_TYPE(ADSP21062, adsp21062_device)

#endif // MAME_CPU_SHARC_SHARC_H
