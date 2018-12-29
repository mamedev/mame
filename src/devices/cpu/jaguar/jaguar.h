// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jaguar.h
    Interface file for the portable Jaguar DSP emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_JAGUAR_JAGUAR_H
#define MAME_CPU_JAGUAR_JAGUAR_H

#pragma once



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

DECLARE_DEVICE_TYPE(JAGUARGPU, jaguargpu_cpu_device)
DECLARE_DEVICE_TYPE(JAGUARDSP, jaguardsp_cpu_device)


class jaguar_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	~jaguar_cpu_device();

	// configuration helpers
	auto irq() { return m_cpu_interrupt.bind(); }

	virtual DECLARE_WRITE32_MEMBER(ctrl_w) = 0;
	virtual DECLARE_READ32_MEMBER(ctrl_r) = 0;

protected:
	jaguar_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool isdsp);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 1; }
	virtual uint32_t execute_input_lines() const override { return 5; }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config m_program_config;

	/* core registers */
	uint32_t      m_r[32];
	uint32_t      m_a[32];
	uint32_t *    m_b0;
	uint32_t *    m_b1;

	/* control registers */
	uint32_t      m_ctrl[G_CTRLMAX];
	uint32_t      m_ppc;
	uint64_t      m_accum;

	/* internal stuff */
	bool        m_isdsp;
	int         m_icount;
	int         m_bankswitch_icount;
	devcb_write_line m_cpu_interrupt;
	address_space *m_program;
	memory_access_cache<2, 0, ENDIANNESS_BIG> *m_cache;

	uint32_t      m_internal_ram_start;
	uint32_t      m_internal_ram_end;

	typedef void (jaguar_cpu_device::*op_func)(uint16_t op);

	static const op_func gpu_op_table[64];
	static const op_func dsp_op_table[64];
	static const uint32_t convert_zero[32];
	bool m_tables_referenced;

	uint32_t       table_refcount;
	std::unique_ptr<uint16_t[]>  mirror_table;
	std::unique_ptr<uint8_t[]>   condition_table;

	const op_func *m_table;

	void abs_rn(uint16_t op);
	void add_rn_rn(uint16_t op);
	void addc_rn_rn(uint16_t op);
	void addq_n_rn(uint16_t op);
	void addqmod_n_rn(uint16_t op);  /* DSP only */
	void addqt_n_rn(uint16_t op);
	void and_rn_rn(uint16_t op);
	void bclr_n_rn(uint16_t op);
	void bset_n_rn(uint16_t op);
	void btst_n_rn(uint16_t op);
	void cmp_rn_rn(uint16_t op);
	void cmpq_n_rn(uint16_t op);
	void div_rn_rn(uint16_t op);
	void illegal(uint16_t op);
	void imacn_rn_rn(uint16_t op);
	void imult_rn_rn(uint16_t op);
	void imultn_rn_rn(uint16_t op);
	void jr_cc_n(uint16_t op);
	void jump_cc_rn(uint16_t op);
	void load_rn_rn(uint16_t op);
	void load_r14n_rn(uint16_t op);
	void load_r15n_rn(uint16_t op);
	void load_r14rn_rn(uint16_t op);
	void load_r15rn_rn(uint16_t op);
	void loadb_rn_rn(uint16_t op);
	void loadw_rn_rn(uint16_t op);
	void loadp_rn_rn(uint16_t op);   /* GPU only */
	void mirror_rn(uint16_t op); /* DSP only */
	void mmult_rn_rn(uint16_t op);
	void move_rn_rn(uint16_t op);
	void move_pc_rn(uint16_t op);
	void movefa_rn_rn(uint16_t op);
	void movei_n_rn(uint16_t op);
	void moveq_n_rn(uint16_t op);
	void moveta_rn_rn(uint16_t op);
	void mtoi_rn_rn(uint16_t op);
	void mult_rn_rn(uint16_t op);
	void neg_rn(uint16_t op);
	void nop(uint16_t op);
	void normi_rn_rn(uint16_t op);
	void not_rn(uint16_t op);
	void or_rn_rn(uint16_t op);
	void pack_rn(uint16_t op);       /* GPU only */
	void resmac_rn(uint16_t op);
	void ror_rn_rn(uint16_t op);
	void rorq_n_rn(uint16_t op);
	void sat8_rn(uint16_t op);       /* GPU only */
	void sat16_rn(uint16_t op);      /* GPU only */
	void sat16s_rn(uint16_t op);     /* DSP only */
	void sat24_rn(uint16_t op);          /* GPU only */
	void sat32s_rn(uint16_t op);     /* DSP only */
	void sh_rn_rn(uint16_t op);
	void sha_rn_rn(uint16_t op);
	void sharq_n_rn(uint16_t op);
	void shlq_n_rn(uint16_t op);
	void shrq_n_rn(uint16_t op);
	void store_rn_rn(uint16_t op);
	void store_rn_r14n(uint16_t op);
	void store_rn_r15n(uint16_t op);
	void store_rn_r14rn(uint16_t op);
	void store_rn_r15rn(uint16_t op);
	void storeb_rn_rn(uint16_t op);
	void storew_rn_rn(uint16_t op);
	void storep_rn_rn(uint16_t op);  /* GPU only */
	void sub_rn_rn(uint16_t op);
	void subc_rn_rn(uint16_t op);
	void subq_n_rn(uint16_t op);
	void subqmod_n_rn(uint16_t op);  /* DSP only */
	void subqt_n_rn(uint16_t op);
	void xor_rn_rn(uint16_t op);
	void update_register_banks();
	void check_irqs();
	void init_tables();
	void jaguar_postload();
};


class jaguargpu_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguargpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE32_MEMBER(ctrl_w) override;
	DECLARE_READ32_MEMBER(ctrl_r) override;

protected:
	virtual void execute_run() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


class jaguardsp_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguardsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE32_MEMBER(ctrl_w) override;
	DECLARE_READ32_MEMBER(ctrl_r) override;

protected:
	virtual uint32_t execute_input_lines() const override { return 6; }
	virtual void execute_run() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


#endif // MAME_CPU_JAGUAR_JAGUAR_H
