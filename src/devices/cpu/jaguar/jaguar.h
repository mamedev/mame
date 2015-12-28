// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jaguar.h
    Interface file for the portable Jaguar DSP emulator.
    Written by Aaron Giles

***************************************************************************/

#pragma once

#ifndef __JAGUAR_H__
#define __JAGUAR_H__



/***************************************************************************
    GLOBAL CONSTANTS
***************************************************************************/

#define JAGUAR_VARIANT_GPU      0
#define JAGUAR_VARIANT_DSP      1



/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	JAGUAR_PC=1,JAGUAR_FLAGS,
	JAGUAR_R0,JAGUAR_R1,JAGUAR_R2,JAGUAR_R3,JAGUAR_R4,JAGUAR_R5,JAGUAR_R6,JAGUAR_R7,
	JAGUAR_R8,JAGUAR_R9,JAGUAR_R10,JAGUAR_R11,JAGUAR_R12,JAGUAR_R13,JAGUAR_R14,JAGUAR_R15,
	JAGUAR_R16,JAGUAR_R17,JAGUAR_R18,JAGUAR_R19,JAGUAR_R20,JAGUAR_R21,JAGUAR_R22,JAGUAR_R23,
	JAGUAR_R24,JAGUAR_R25,JAGUAR_R26,JAGUAR_R27,JAGUAR_R28,JAGUAR_R29,JAGUAR_R30,JAGUAR_R31
};

enum
{
	G_FLAGS = 0,
	G_MTXC,
	G_MTXA,
	G_END,
	G_PC,
	G_CTRL,
	G_HIDATA,
	G_DIVCTRL,
	G_DUMMY,
	G_REMAINDER,
	G_CTRLMAX
};

enum
{
	D_FLAGS = 0,
	D_MTXC,
	D_MTXA,
	D_END,
	D_PC,
	D_CTRL,
	D_MOD,
	D_DIVCTRL,
	D_MACHI,
	D_REMAINDER,
	D_CTRLMAX
};



/***************************************************************************
    CONFIGURATION
***************************************************************************/

#define MCFG_JAGUAR_IRQ_HANDLER(_devcb) \
	devcb = &jaguar_cpu_device::set_int_func(*device, DEVCB_##_devcb);


/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define JAGUAR_IRQ0     0       /* IRQ0 */
#define JAGUAR_IRQ1     1       /* IRQ1 */
#define JAGUAR_IRQ2     2       /* IRQ2 */
#define JAGUAR_IRQ3     3       /* IRQ3 */
#define JAGUAR_IRQ4     4       /* IRQ4 */
#define JAGUAR_IRQ5     5       /* IRQ5 */



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/


extern const device_type JAGUARGPU;
extern const device_type JAGUARDSP;


class jaguar_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	jaguar_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, bool isdsp);
	~jaguar_cpu_device();

	// static configuration helpers
	template<class _Object> static devcb_base &set_int_func(device_t &device, _Object object) { return downcast<jaguar_cpu_device &>(device).m_cpu_interrupt.set_callback(object); }

	virtual DECLARE_WRITE32_MEMBER(ctrl_w) { assert(0); }
	virtual DECLARE_READ32_MEMBER(ctrl_r) { assert(0); return 0; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 5; }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 6; }

	address_space_config m_program_config;

	/* core registers */
	UINT32      m_r[32];
	UINT32      m_a[32];
	UINT32 *    m_b0;
	UINT32 *    m_b1;

	/* control registers */
	UINT32      m_ctrl[G_CTRLMAX];
	UINT32      m_ppc;
	UINT64      m_accum;

	/* internal stuff */
	bool        m_isdsp;
	int         m_icount;
	int         m_bankswitch_icount;
	devcb_write_line m_cpu_interrupt;
	address_space *m_program;
	direct_read_data *m_direct;

	UINT32      m_internal_ram_start;
	UINT32      m_internal_ram_end;

	typedef void (jaguar_cpu_device::*op_func)(UINT16 op);

	static const op_func gpu_op_table[64];
	static const op_func dsp_op_table[64];
	static const UINT32 convert_zero[32];
	bool m_tables_referenced;

	UINT32       table_refcount;
	std::unique_ptr<UINT16[]>  mirror_table;
	std::unique_ptr<UINT8[]>   condition_table;

	const op_func *m_table;

	void abs_rn(UINT16 op);
	void add_rn_rn(UINT16 op);
	void addc_rn_rn(UINT16 op);
	void addq_n_rn(UINT16 op);
	void addqmod_n_rn(UINT16 op);  /* DSP only */
	void addqt_n_rn(UINT16 op);
	void and_rn_rn(UINT16 op);
	void bclr_n_rn(UINT16 op);
	void bset_n_rn(UINT16 op);
	void btst_n_rn(UINT16 op);
	void cmp_rn_rn(UINT16 op);
	void cmpq_n_rn(UINT16 op);
	void div_rn_rn(UINT16 op);
	void illegal(UINT16 op);
	void imacn_rn_rn(UINT16 op);
	void imult_rn_rn(UINT16 op);
	void imultn_rn_rn(UINT16 op);
	void jr_cc_n(UINT16 op);
	void jump_cc_rn(UINT16 op);
	void load_rn_rn(UINT16 op);
	void load_r14n_rn(UINT16 op);
	void load_r15n_rn(UINT16 op);
	void load_r14rn_rn(UINT16 op);
	void load_r15rn_rn(UINT16 op);
	void loadb_rn_rn(UINT16 op);
	void loadw_rn_rn(UINT16 op);
	void loadp_rn_rn(UINT16 op);   /* GPU only */
	void mirror_rn(UINT16 op); /* DSP only */
	void mmult_rn_rn(UINT16 op);
	void move_rn_rn(UINT16 op);
	void move_pc_rn(UINT16 op);
	void movefa_rn_rn(UINT16 op);
	void movei_n_rn(UINT16 op);
	void moveq_n_rn(UINT16 op);
	void moveta_rn_rn(UINT16 op);
	void mtoi_rn_rn(UINT16 op);
	void mult_rn_rn(UINT16 op);
	void neg_rn(UINT16 op);
	void nop(UINT16 op);
	void normi_rn_rn(UINT16 op);
	void not_rn(UINT16 op);
	void or_rn_rn(UINT16 op);
	void pack_rn(UINT16 op);       /* GPU only */
	void resmac_rn(UINT16 op);
	void ror_rn_rn(UINT16 op);
	void rorq_n_rn(UINT16 op);
	void sat8_rn(UINT16 op);       /* GPU only */
	void sat16_rn(UINT16 op);      /* GPU only */
	void sat16s_rn(UINT16 op);     /* DSP only */
	void sat24_rn(UINT16 op);          /* GPU only */
	void sat32s_rn(UINT16 op);     /* DSP only */
	void sh_rn_rn(UINT16 op);
	void sha_rn_rn(UINT16 op);
	void sharq_n_rn(UINT16 op);
	void shlq_n_rn(UINT16 op);
	void shrq_n_rn(UINT16 op);
	void store_rn_rn(UINT16 op);
	void store_rn_r14n(UINT16 op);
	void store_rn_r15n(UINT16 op);
	void store_rn_r14rn(UINT16 op);
	void store_rn_r15rn(UINT16 op);
	void storeb_rn_rn(UINT16 op);
	void storew_rn_rn(UINT16 op);
	void storep_rn_rn(UINT16 op);  /* GPU only */
	void sub_rn_rn(UINT16 op);
	void subc_rn_rn(UINT16 op);
	void subq_n_rn(UINT16 op);
	void subqmod_n_rn(UINT16 op);  /* DSP only */
	void subqt_n_rn(UINT16 op);
	void xor_rn_rn(UINT16 op);
	void update_register_banks();
	void check_irqs();
	void init_tables();
	void jaguar_postload();
};


class jaguargpu_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguargpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE32_MEMBER(ctrl_w) override;
	DECLARE_READ32_MEMBER(ctrl_r) override;

protected:
	virtual void execute_run() override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};


class jaguardsp_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguardsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE32_MEMBER(ctrl_w) override;
	DECLARE_READ32_MEMBER(ctrl_r) override;

protected:
	virtual UINT32 execute_input_lines() const override { return 6; }
	virtual void execute_run() override;

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};


#endif /* __JAGUAR_H__ */
