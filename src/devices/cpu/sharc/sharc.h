// license:BSD-3-Clause
// copyright-holders:Ville Linde
#pragma once

#ifndef __SHARC_H__
#define __SHARC_H__

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"

#define SHARC_INPUT_FLAG0       3
#define SHARC_INPUT_FLAG1       4
#define SHARC_INPUT_FLAG2       5
#define SHARC_INPUT_FLAG3       6


enum SHARC_BOOT_MODE
{
	BOOT_MODE_EPROM,
	BOOT_MODE_HOST,
	BOOT_MODE_LINK,
	BOOT_MODE_NOBOOT
};


struct alignas(16) SHARC_DAG
{
	UINT32 i[8];
	UINT32 m[8];
	UINT32 b[8];
	UINT32 l[8];
};

union SHARC_REG
{
	INT32 r;
	float f;
};

struct SHARC_DMA_REGS
{
	UINT32 control;
	UINT32 int_index;
	UINT32 int_modifier;
	UINT32 int_count;
	UINT32 chain_ptr;
	UINT32 gen_purpose;
	UINT32 ext_index;
	UINT32 ext_modifier;
	UINT32 ext_count;
};

struct SHARC_LADDR
{
	UINT32 addr;
	UINT32 code;
	UINT32 loop_type;
};

struct SHARC_DMA_OP
{
	UINT32 src;
	UINT32 dst;
	UINT32 chain_ptr;
	INT32 src_modifier;
	INT32 dst_modifier;
	INT32 src_count;
	INT32 dst_count;
	INT32 pmode;
	INT32 chained_direction;
	emu_timer *timer;
	bool active;
};


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


#define SIGN_EXTEND6(x)				(((x) & 0x20) ? (0xffffffc0 | (x)) : (x))
#define SIGN_EXTEND24(x)			(((x) & 0x800000) ? (0xff000000 | (x)) : (x))
#define MAKE_EXTRACT_MASK(start_bit, length)    ((0xffffffff << start_bit) & (((UINT32)0xffffffff) >> (32 - (start_bit + length))))

#define OP_USERFLAG_COUNTER_LOOP			0x00000001
#define OP_USERFLAG_COND_LOOP				0x00000002
#define OP_USERFLAG_COND_FIELD				0x000003fc
#define OP_USERFLAG_COND_FIELD_SHIFT		2
#define OP_USERFLAG_ASTAT_DELAY_COPY_AZ		0x00001000
#define OP_USERFLAG_ASTAT_DELAY_COPY_AN		0x00002000
#define OP_USERFLAG_ASTAT_DELAY_COPY_AC		0x00004000
#define OP_USERFLAG_ASTAT_DELAY_COPY_AV		0x00008000
#define OP_USERFLAG_ASTAT_DELAY_COPY_MV		0x00010000
#define OP_USERFLAG_ASTAT_DELAY_COPY_MN		0x00020000
#define OP_USERFLAG_ASTAT_DELAY_COPY_SV		0x00040000
#define OP_USERFLAG_ASTAT_DELAY_COPY_SZ		0x00080000
#define OP_USERFLAG_ASTAT_DELAY_COPY_BTF	0x00100000
#define OP_USERFLAG_ASTAT_DELAY_COPY		0x001ff000


#define MCFG_SHARC_BOOT_MODE(boot_mode) \
	adsp21062_device::set_boot_mode(*device, boot_mode);

class sharc_frontend;

class adsp21062_device : public cpu_device
{
	friend class sharc_frontend;

public:
	// construction/destruction
	adsp21062_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	// static configuration helpers
	static void set_boot_mode(device_t &device, const SHARC_BOOT_MODE boot_mode) { downcast<adsp21062_device &>(device).m_boot_mode = boot_mode; }

	void set_flag_input(int flag_num, int state);
	void external_iop_write(UINT32 address, UINT32 data);
	void external_dma_write(UINT32 address, UINT64 data);

	TIMER_CALLBACK_MEMBER(sharc_iop_delayed_write_callback);
	TIMER_CALLBACK_MEMBER(sharc_dma_callback);

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

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 8; }
	virtual UINT32 execute_max_cycles() const override { return 8; }
	virtual UINT32 execute_input_lines() const override { return 32; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ); }
	virtual bool memory_read(address_spacenum spacenum, offs_t offset, int size, UINT64 &value) override;
	virtual bool memory_readop(offs_t offset, int size, UINT64 &value) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 6; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 6; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	direct_read_data *m_direct;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	typedef void (adsp21062_device::*opcode_func)();
	struct SHARC_OP
	{
		UINT32 op_mask;
		UINT32 op_bits;
		opcode_func handler;
	};
	static const SHARC_OP s_sharc_opcode_table[];

	struct ASTAT_DRC
	{
		union
		{
			struct
			{
				UINT32 az;
				UINT32 av;
				UINT32 an;
				UINT32 ac;
				UINT32 as;
				UINT32 ai;
				UINT32 mn;
				UINT32 mv;
				UINT32 mu;
				UINT32 mi;
				UINT32 sv;
				UINT32 sz;
				UINT32 ss;
				UINT32 btf;
				UINT32 af;
				UINT32 cacc;
			};
			UINT64 flags64[8];
		};
	};

	struct alignas(16) sharc_internal_state
	{
		SHARC_REG r[16];
		SHARC_REG reg_alt[16];

		UINT32 pc;
		UINT64 mrf;
		UINT64 mrb;

		UINT32 pcstack[32];
		UINT32 lcstack[6];
		UINT32 lastack[6];
		UINT32 lstkp;

		UINT32 faddr;
		UINT32 daddr;
		UINT32 pcstk;
		UINT32 pcstkp;
		SHARC_LADDR laddr;
		UINT32 curlcntr;
		UINT32 lcntr;
		UINT8 extdma_shift;

		/* Data Address Generator (DAG) */
		SHARC_DAG dag1;     // (DM bus)
		SHARC_DAG dag2;     // (PM bus)
		SHARC_DAG dag1_alt;
		SHARC_DAG dag2_alt;

		SHARC_DMA_REGS dma[12];

		/* System registers */
		UINT32 mode1;
		UINT32 mode2;
		UINT32 astat;
		UINT32 stky;
		UINT32 irptl;
		UINT32 imask;
		UINT32 imaskp;
		UINT32 ustat1;
		UINT32 ustat2;

		UINT32 flag[4];

		UINT32 syscon;
		UINT32 sysstat;

		struct
		{
			UINT32 mode1;
			UINT32 astat;
		} status_stack[5];
		INT32 status_stkp;

		UINT64 px;

		int icount;
		UINT64 opcode;

		UINT32 nfaddr;

		INT32 idle;
		INT32 irq_pending;
		INT32 active_irq_num;

		SHARC_DMA_OP dma_op[12];
		UINT32 dma_status;

		INT32 interrupt_active;

		UINT32 iop_delayed_reg;
		UINT32 iop_delayed_data;
		emu_timer *delayed_iop_timer;

		UINT32 delay_slot1, delay_slot2;

		INT32 systemreg_latency_cycles;
		INT32 systemreg_latency_reg;
		UINT32 systemreg_latency_data;
		UINT32 systemreg_previous_data;

		UINT32 astat_old;
		UINT32 astat_old_old;
		UINT32 astat_old_old_old;

		UINT32 arg0;
		UINT32 arg1;
		UINT32 arg2;
		UINT32 arg3;

		UINT64 arg64;
		UINT32 mode1_delay_data;

		ASTAT_DRC astat_drc;
		ASTAT_DRC astat_drc_copy;
		ASTAT_DRC astat_delay_copy;
		UINT32 dreg_temp;
		UINT32 jmpdest;

		float fp0;
		float fp1;

		UINT32 force_recompile;
	};

	sharc_internal_state* m_core;

	SHARC_BOOT_MODE m_boot_mode;

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
	uml::code_handle *m_exception[EXCEPTION_COUNT];		// exception handlers
	uml::code_handle *m_swap_dag1_0_3;
	uml::code_handle *m_swap_dag1_4_7;
	uml::code_handle *m_swap_dag2_0_3;
	uml::code_handle *m_swap_dag2_4_7;
	uml::code_handle *m_swap_r0_7;
	uml::code_handle *m_swap_r8_15;

	bool m_cache_dirty;

	UINT16 *m_internal_ram_block0, *m_internal_ram_block1;

	address_space *m_program;
	address_space *m_data;
	opcode_func m_sharc_op[512];

	UINT16 m_internal_ram[2 * 0x10000]; // 2x 128KB

	bool m_enable_drc;

	inline void CHANGE_PC(UINT32 newpc);
	inline void CHANGE_PC_DELAYED(UINT32 newpc);
	void sharc_iop_delayed_w(UINT32 reg, UINT32 data, int cycles);
	UINT32 sharc_iop_r(UINT32 address);
	void sharc_iop_w(UINT32 address, UINT32 data);
	UINT32 pm_read32(UINT32 address);
	void pm_write32(UINT32 address, UINT32 data);
	UINT64 pm_read48(UINT32 address);
	void pm_write48(UINT32 address, UINT64 data);
	UINT32 dm_read32(UINT32 address);
	void dm_write32(UINT32 address, UINT32 data);
	void schedule_chained_dma_op(int channel, UINT32 dma_chain_ptr, int chained_direction);
	void schedule_dma_op(int channel, UINT32 src, UINT32 dst, int src_modifier, int dst_modifier, int src_count, int dst_count, int pmode);
	void dma_op(int channel);
	void sharc_dma_exec(int channel);
	void add_systemreg_write_latency_effect(int sysreg, UINT32 data, UINT32 prev_data);
	inline void swap_register(UINT32 *a, UINT32 *b);
	void systemreg_write_latency_effect();
	UINT32 GET_UREG(int ureg);
	void SET_UREG(int ureg, UINT32 data);
	void SHIFT_OPERATION_IMM(int shiftop, int data, int rn, int rx);
	void COMPUTE(UINT32 opcode);
	void check_interrupts();
	inline void PUSH_PC(UINT32 pc);
	inline UINT32 POP_PC();
	inline UINT32 TOP_PC();
	inline void PUSH_LOOP(UINT32 addr, UINT32 code, UINT32 type, UINT32 count);
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
	inline UINT32 SCALB(SHARC_REG rx, int ry);
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
	inline void compute_fclip(int rn, int rx, int ry);
	inline void compute_recips(int rn, int rx);
	inline void compute_rsqrts(int rn, int rx);
	inline void compute_fpass(int rn, int rx);
	inline void compute_fabs(int rn, int rx);
	inline void compute_mul_uuin(int rn, int rx, int ry);
	inline void compute_mul_ssin(int rn, int rx, int ry);
	inline UINT32 compute_mrf_plus_mul_ssin(int rx, int ry);
	inline UINT32 compute_mrb_plus_mul_ssin(int rx, int ry);
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
	inline void compute_fmul_fmax(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_fmin(int fm, int fxm, int fym, int fa, int fxa, int fya);
	inline void compute_fmul_dual_fadd_fsub(int fm, int fxm, int fym, int fa, int fs, int fxa, int fya);
	void build_opcode_table();

	/* internal compiler state */
	struct compiler_state
	{
		UINT32 cycles;                             /* accumulated cycles */
		UINT8  checkints;                          /* need to check interrupts before next instruction */
		uml::code_label  labelnum;				   /* index for local labels */
		struct
		{
			int counter;
			int mode;
			UINT32 data;
		} mode1_delay;
	};

	void execute_run_drc();
	void flush_cache();
	void compile_block(offs_t pc);
	void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(MEM_ACCESSOR_TYPE type, const char *name, uml::code_handle *&handleptr);
	void static_generate_exception(UINT8 exception, const char *name);
	void static_generate_push_pc();
	void static_generate_pop_pc();
	void static_generate_push_loop();
	void static_generate_pop_loop();
	void static_generate_push_status();
	void static_generate_pop_status();
	void static_generate_mode1_ops();
	void load_fast_iregs(drcuml_block *block);
	void save_fast_iregs(drcuml_block *block);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, bool last_delayslot);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, int allow_exception);
	int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_unimplemented_compute(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_compute(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_if_condition(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int condition, int skip_label);
	void generate_do_condition(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int condition, int skip_label, ASTAT_DRC &astat);
	void generate_shift_imm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int data, int shiftop, int rn, int rx);
	void generate_call(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, bool delayslot);
	void generate_jump(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, bool delayslot, bool loopabort, bool clearint);
	void generate_write_mode1_imm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 data);
	void generate_set_mode1_imm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 data);
	void generate_clear_mode1_imm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 data);
	void generate_toggle_mode1_imm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 data);
	void generate_read_ureg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int ureg, bool has_compute);
	void generate_write_ureg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int ureg, bool imm, UINT32 data);
	void generate_update_circular_buffer(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int g, int i);
	void generate_astat_copy(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

	bool if_condition_always_true(int condition);
	UINT32 do_condition_astat_bits(int condition);
};


extern const device_type ADSP21062;


#endif /* __SHARC_H__ */
