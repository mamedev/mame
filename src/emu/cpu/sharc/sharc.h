// license:BSD-3-Clause
// copyright-holders:Ville Linde
#pragma once

#ifndef __SHARC_H__
#define __SHARC_H__


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


struct SHARC_DAG
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


#define MCFG_SHARC_BOOT_MODE(boot_mode) \
	adsp21062_device::set_boot_mode(*device, boot_mode);


class adsp21062_device : public cpu_device
{
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

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 8; }
	virtual UINT32 execute_max_cycles() const { return 8; }
	virtual UINT32 execute_input_lines() const { return 32; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : NULL ); }
	virtual bool memory_read(address_spacenum spacenum, offs_t offset, int size, UINT64 &value);
	virtual bool memory_readop(offs_t offset, int size, UINT64 &value);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 40; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

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

	UINT32 m_pc;
	SHARC_REG m_r[16];
	SHARC_REG m_reg_alt[16];
	UINT64 m_mrf;
	UINT64 m_mrb;

	UINT32 m_pcstack[32];
	UINT32 m_lcstack[6];
	UINT32 m_lastack[6];
	UINT32 m_lstkp;

	UINT32 m_faddr;
	UINT32 m_daddr;
	UINT32 m_pcstk;
	UINT32 m_pcstkp;
	SHARC_LADDR m_laddr;
	UINT32 m_curlcntr;
	UINT32 m_lcntr;
	UINT8 m_extdma_shift;

	/* Data Address Generator (DAG) */
	SHARC_DAG m_dag1;     // (DM bus)
	SHARC_DAG m_dag2;     // (PM bus)
	SHARC_DAG m_dag1_alt;
	SHARC_DAG m_dag2_alt;

	SHARC_DMA_REGS m_dma[12];

	/* System registers */
	UINT32 m_mode1;
	UINT32 m_mode2;
	UINT32 m_astat;
	UINT32 m_stky;
	UINT32 m_irptl;
	UINT32 m_imask;
	UINT32 m_imaskp;
	UINT32 m_ustat1;
	UINT32 m_ustat2;

	UINT32 m_flag[4];

	UINT32 m_syscon;
	UINT32 m_sysstat;

	struct
	{
		UINT32 mode1;
		UINT32 astat;
	} m_status_stack[5];
	INT32 m_status_stkp;

	UINT64 m_px;

	UINT16 *m_internal_ram_block0, *m_internal_ram_block1;

	address_space *m_program;
	address_space *m_data;
	opcode_func m_sharc_op[512];
	int m_icount;
	UINT64 m_opcode;

	UINT32 m_nfaddr;

	INT32 m_idle;
	INT32 m_irq_active;
	INT32 m_active_irq_num;

	SHARC_BOOT_MODE m_boot_mode;

	SHARC_DMA_OP m_dma_op[12];
	UINT32 m_dma_status;

	INT32 m_interrupt_active;

	UINT32 m_iop_delayed_reg;
	UINT32 m_iop_delayed_data;
	emu_timer *m_delayed_iop_timer;

	UINT32 m_delay_slot1, m_delay_slot2;

	INT32 m_systemreg_latency_cycles;
	INT32 m_systemreg_latency_reg;
	UINT32 m_systemreg_latency_data;
	UINT32 m_systemreg_previous_data;

	UINT32 m_astat_old;
	UINT32 m_astat_old_old;
	UINT32 m_astat_old_old_old;

	UINT16 m_internal_ram[2 * 0x10000]; // 2x 128KB

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

};


extern const device_type ADSP21062;


#endif /* __SHARC_H__ */
