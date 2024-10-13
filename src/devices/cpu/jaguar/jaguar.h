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


class jaguar_cpu_device : public cpu_device
{
public:
	// construction/destruction
	~jaguar_cpu_device();

	// configuration helpers
	auto irq() { return m_cpu_interrupt.bind(); }

	// TODO: add which device triggered the I/O
	void iobus_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iobus_r(offs_t offset, u32 mem_mask = ~0);
	void go_w(int state);

protected:
	jaguar_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 version, bool isdsp, address_map_constructor io_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// I/Os (common)
	u32 flags_r();
	void flags_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void matrix_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void matrix_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void end_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void pc_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 status_r();
	void control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 div_remainder_r();
	void div_control_w(u32 data);

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
	address_space_config m_io_config;

	/* core registers */
	u32      m_r[32];
	u32      m_a[32];
	u32 *    m_b0;
	u32 *    m_b1;

	/* control registers */
	u32      m_ppc;
	u64      m_accum;

	/* internal stuff */
	u8          m_version;
	bool        m_isdsp;
	int         m_icount;
	int         m_bankswitch_icount;
	devcb_write_line m_cpu_interrupt;
	memory_access<24, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<24, 2, 0, ENDIANNESS_BIG>::specific m_program;
	memory_access< 8, 2, 0, ENDIANNESS_BIG>::specific m_io;

	u32      m_internal_ram_start;
	u32      m_internal_ram_end;

	typedef void (jaguar_cpu_device::*op_func)(u16 op);

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
	void resmac_rn(u16 op);
	void ror_rn_rn(u16 op);
	void rorq_n_rn(u16 op);

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
	void sub_rn_rn(u16 op);
	void subc_rn_rn(u16 op);
	void subq_n_rn(u16 op);
	void subqt_n_rn(u16 op);
	void xor_rn_rn(u16 op);
	// GPU only opcodes
	void loadp_rn_rn(u16 op);
	void pack_rn(u16 op);
	void sat8_rn(u16 op);
	void sat16_rn(u16 op);
	void sat24_rn(u16 op);
	void storep_rn_rn(u16 op);
	// DSP only opcodes
	void addqmod_n_rn(u16 op);
	void mirror_rn(u16 op);
	void sat16s_rn(u16 op);
	void sat32s_rn(u16 op);
	void subqmod_n_rn(u16 op);

	void update_register_banks();
	void check_irqs();
	void init_tables();

	// I/O internal regs
	void io_common_map(address_map &map) ATTR_COLD;
	// TODO: the m_io* stubs are conventionally given for allowing a correct register setup from vanilla 68k.
	// This is yet another reason about needing a bus device dispatcher for this system.
	u32 m_io_end;
	u32 m_io_pc;
	u32 m_io_status;
	u32 m_io_mtxc;
	u32 m_io_mtxa;

	u32 m_pc;
	u32 m_flags;
	bool m_imask;
	bool m_maddw;
	u8 m_mwidth;
	u32 m_mtxaddr;
	bool m_go;
	u8 m_int_latch;
	u8 m_int_mask;
	bool m_bus_hog;
	u32 m_div_remainder;
	bool m_div_offset;

	// GPU specific
	u32 m_hidata;
	static const op_func gpu_op_table[64];
	// DSP specific
	u32 m_modulo;
	static const op_func dsp_op_table[64];
};


class jaguargpu_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguargpu_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void io_map(address_map &map) ATTR_COLD;

protected:
	virtual void execute_run() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void hidata_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 hidata_r();
};


class jaguardsp_cpu_device : public jaguar_cpu_device
{
public:
	// construction/destruction
	jaguardsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void io_map(address_map &map) ATTR_COLD;

protected:
	virtual void execute_run() override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	void modulo_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void dsp_end_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 high_accum_r();
};


#endif // MAME_CPU_JAGUAR_JAGUAR_H
