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

	virtual void ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0) = 0;
	virtual u32 ctrl_r(offs_t offset) = 0;

protected:
	jaguar_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 version, bool isdsp);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 1; }
	virtual u32 execute_max_cycles() const override { return 1; }
	virtual u32 execute_input_lines() const override { return 5; }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// defines
	inline void CLR_Z();
	inline void CLR_ZN();
	inline void CLR_ZNC();
	inline void SET_Z(u32 r);
	inline void SET_C_ADD(u32 a, u32 b);
	inline void SET_C_SUB(u32 a, u32 b);
	inline void SET_N(u32 r);
	inline void SET_ZN(u32 r);
	inline void SET_ZNC_ADD(u32 a, u32 b, u32 r);
	inline void SET_ZNC_SUB(u32 a, u32 b, u32 r);

	inline u8 CONDITION(u8 x);

	inline u8 READBYTE(offs_t a);
	inline u16 READWORD(offs_t a);
	inline u32 READLONG(offs_t a);

	inline void WRITEBYTE(offs_t a, u8 v);
	inline void WRITEWORD(offs_t a, u16 v);
	inline void WRITELONG(offs_t a, u32 v);

	inline u16 ROPCODE(offs_t pc);

	address_space_config m_program_config;

	/* core registers */
	u32      m_r[32];
	u32      m_a[32];
	u32 *    m_b0;
	u32 *    m_b1;

	/* control registers */
	u32      m_ctrl[G_CTRLMAX];
	u32      m_ppc;
	u64      m_accum;

	/* internal stuff */
	u8          m_version;
	bool        m_isdsp;
	int         m_icount;
	int         m_bankswitch_icount;
	devcb_write_line m_cpu_interrupt;
	address_space *m_program;
	memory_access_cache<2, 0, ENDIANNESS_BIG> *m_cache;

	u32      m_internal_ram_start;
	u32      m_internal_ram_end;

	typedef void (jaguar_cpu_device::*op_func)(u16 op);

	static const op_func gpu_op_table[64];
	static const op_func dsp_op_table[64];
	static const u32 convert_zero[32];
	bool m_tables_referenced;

	u32       table_refcount;
	std::unique_ptr<u16[]>  mirror_table;
	std::unique_ptr<u8[]>   condition_table;

	const op_func *m_table;

	void abs_rn(u16 op);
	void add_rn_rn(u16 op);
	void addc_rn_rn(u16 op);
	void addq_n_rn(u16 op);
	void addqmod_n_rn(u16 op);  /* DSP only */
	void addqt_n_rn(u16 op);
	void and_rn_rn(u16 op);
	void bclr_n_rn(u16 op);
	void bset_n_rn(u16 op);
	void btst_n_rn(u16 op);
	void cmp_rn_rn(u16 op);
	void cmpq_n_rn(u16 op);
	void div_rn_rn(u16 op);
	void illegal(u16 op);
	void imacn_rn_rn(u16 op);
	void imult_rn_rn(u16 op);
	void imultn_rn_rn(u16 op);
	void jr_cc_n(u16 op);
	void jump_cc_rn(u16 op);
	void load_rn_rn(u16 op);
	void load_r14n_rn(u16 op);
	void load_r15n_rn(u16 op);
	void load_r14rn_rn(u16 op);
	void load_r15rn_rn(u16 op);
	void loadb_rn_rn(u16 op);
	void loadw_rn_rn(u16 op);
	void loadp_rn_rn(u16 op);   /* GPU only */
	void mirror_rn(u16 op); /* DSP only */
	void mmult_rn_rn(u16 op);
	void move_rn_rn(u16 op);
	void move_pc_rn(u16 op);
	void movefa_rn_rn(u16 op);
	void movei_n_rn(u16 op);
	void moveq_n_rn(u16 op);
	void moveta_rn_rn(u16 op);
	void mtoi_rn_rn(u16 op);
	void mult_rn_rn(u16 op);
	void neg_rn(u16 op);
	void nop(u16 op);
	void normi_rn_rn(u16 op);
	void not_rn(u16 op);
	void or_rn_rn(u16 op);
	void pack_rn(u16 op);       /* GPU only */
	void resmac_rn(u16 op);
	void ror_rn_rn(u16 op);
	void rorq_n_rn(u16 op);
	void sat8_rn(u16 op);       /* GPU only */
	void sat16_rn(u16 op);      /* GPU only */
	void sat16s_rn(u16 op);     /* DSP only */
	void sat24_rn(u16 op);          /* GPU only */
	void sat32s_rn(u16 op);     /* DSP only */
	void sh_rn_rn(u16 op);
	void sha_rn_rn(u16 op);
	void sharq_n_rn(u16 op);
	void shlq_n_rn(u16 op);
	void shrq_n_rn(u16 op);
	void store_rn_rn(u16 op);
	void store_rn_r14n(u16 op);
	void store_rn_r15n(u16 op);
	void store_rn_r14rn(u16 op);
	void store_rn_r15rn(u16 op);
	void storeb_rn_rn(u16 op);
	void storew_rn_rn(u16 op);
	void storep_rn_rn(u16 op);  /* GPU only */
	void sub_rn_rn(u16 op);
	void subc_rn_rn(u16 op);
	void subq_n_rn(u16 op);
	void subqmod_n_rn(u16 op);  /* DSP only */
	void subqt_n_rn(u16 op);
	void xor_rn_rn(u16 op);
	void update_register_banks();
	void check_irqs();
	void init_tables();
};


class jaguargpu_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguargpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0) override;
	u32 ctrl_r(offs_t offset) override;

protected:
	virtual void execute_run() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


class jaguardsp_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguardsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0) override;
	u32 ctrl_r(offs_t offset) override;

protected:
	virtual u32 execute_input_lines() const override { return 6; }
	virtual void execute_run() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


#endif // MAME_CPU_JAGUAR_JAGUAR_H
