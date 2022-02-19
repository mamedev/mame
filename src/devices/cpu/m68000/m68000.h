// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68000_H
#define MAME_CPU_M68000_M68000_H

#pragma once

// SoftFloat 2 lacks an include guard
#ifndef softfloat_h
#define softfloat_h 1
#include "softfloat/milieu.h"
#include "softfloat/softfloat.h"
#endif

extern flag floatx80_is_nan(floatx80 a);


/* MMU constants */
constexpr int MMU_ATC_ENTRIES = (22);    // 68851 has 64, 030 has 22

/* instruction cache constants */
constexpr int M68K_IC_SIZE = 128;

/* There are 7 levels of interrupt to the 68K.
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 *
 * If disable_interrupt_mixer() has been called, the 3 interrupt lines
 * are modeled instead, as numbers 0-2.
 */
constexpr int M68K_IRQ_NONE = 0;
constexpr int M68K_IRQ_1    = 1;
constexpr int M68K_IRQ_2    = 2;
constexpr int M68K_IRQ_3    = 3;
constexpr int M68K_IRQ_4    = 4;
constexpr int M68K_IRQ_5    = 5;
constexpr int M68K_IRQ_6    = 6;
constexpr int M68K_IRQ_7    = 7;

constexpr int M68K_IRQ_IPL0 = 0;
constexpr int M68K_IRQ_IPL1 = 1;
constexpr int M68K_IRQ_IPL2 = 2;

constexpr int M68K_SZ_LONG = 0;
constexpr int M68K_SZ_BYTE = 1;
constexpr int M68K_SZ_WORD = 2;

// special input lines
constexpr int M68K_LINE_BUSERROR = 16;

/* CPU types for use in m68k_set_cpu_type() */
enum
{
	M68K_CPU_TYPE_INVALID,
	M68K_CPU_TYPE_68000,
	M68K_CPU_TYPE_68008,
	M68K_CPU_TYPE_68010,
	M68K_CPU_TYPE_68EC020,
	M68K_CPU_TYPE_68020,
	M68K_CPU_TYPE_68EC030,
	M68K_CPU_TYPE_68030,
	M68K_CPU_TYPE_68EC040,
	M68K_CPU_TYPE_68LC040,
	M68K_CPU_TYPE_68040,
	M68K_CPU_TYPE_SCC68070,
	M68K_CPU_TYPE_FSCPU32,
	M68K_CPU_TYPE_COLDFIRE
};

// function codes
enum
{
	M68K_FC_USER_DATA = 1,
	M68K_FC_USER_PROGRAM = 2,
	M68K_FC_SUPERVISOR_DATA = 5,
	M68K_FC_SUPERVISOR_PROGRAM = 6,
	M68K_FC_INTERRUPT = 7
};

/* HMMU enable types for use with m68k_set_hmmu_enable() */
constexpr int M68K_HMMU_DISABLE   = 0;   /* no translation */
constexpr int M68K_HMMU_ENABLE_II = 1;   /* Mac II style fixed translation */
constexpr int M68K_HMMU_ENABLE_LC = 2;   /* Mac LC style fixed translation */

enum
{
	/* NOTE: M68K_SP fetches the current SP, be it USP, ISP, or MSP */
	M68K_PC = STATE_GENPC, M68K_SP = 1, M68K_ISP, M68K_USP, M68K_MSP, M68K_SR, M68K_VBR,
	M68K_SFC, M68K_DFC, M68K_CACR, M68K_CAAR, M68K_IR, M68K_PREF_ADDR, M68K_PREF_DATA,
	M68K_D0, M68K_D1, M68K_D2, M68K_D3, M68K_D4, M68K_D5, M68K_D6, M68K_D7,
	M68K_A0, M68K_A1, M68K_A2, M68K_A3, M68K_A4, M68K_A5, M68K_A6, M68K_A7,
	M68K_FP0, M68K_FP1, M68K_FP2, M68K_FP3, M68K_FP4, M68K_FP5, M68K_FP6, M68K_FP7,
	M68K_FPSR, M68K_FPCR, M68K_CRP_LIMIT, M68K_CRP_APTR, M68K_SRP_LIMIT, M68K_SRP_APTR,
	M68K_MMU_TC, M68K_TT0, M68K_TT1, M68K_MMU_SR, M68K_ITT0, M68K_ITT1,
	M68K_DTT0, M68K_DTT1, M68K_URP_APTR
};

class m68000_base_device : public cpu_device
{
public:
	enum {
		AS_CPU_SPACE = 4
	};

	// construction/destruction
	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr u8 autovector(int level) { return 0x18 + level; }
	void autovectors_map(address_map &map);

protected:
	static constexpr int NUM_CPU_TYPES = 8;

	typedef void (m68000_base_device::*opcode_handler_ptr)();
	static u16 m68ki_instruction_state_table[NUM_CPU_TYPES][0x10000]; /* opcode handler state numbers */
	static unsigned char m68ki_cycles[NUM_CPU_TYPES][0x10000]; /* Cycles used by CPU type */

	/* This is used to generate the opcode handler state table */
	struct opcode_handler_struct
	{
		unsigned int  match;                 /* what to match after masking */
		unsigned int  mask;                  /* mask on opcode */
		unsigned char cycles[NUM_CPU_TYPES]; /* cycles each cpu type takes */
	};

	static const opcode_handler_ptr m68k_handler_table[];
	static const opcode_handler_struct m68k_opcode_table[];
	static const u16 m68k_state_illegal;

	static void m68ki_set_one(unsigned short opcode, u16 state, const opcode_handler_struct &s);
	static void m68ki_build_opcode_table(void);

	void clear_all(void);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }
	virtual u32 execute_input_lines() const noexcept override { return m_interrupt_mixer ? 8 : 3; } // number of input lines
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return m_interrupt_mixer ? inputnum == M68K_IRQ_7 : false; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// address spaces
	const address_space_config m_program_config, m_oprogram_config;
	address_space_config m_cpu_space_config;

	void define_state(void);

public:
	template <typename... T> void set_reset_callback(T &&... args) { m_reset_instr_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_cmpild_callback(T &&... args) { m_cmpild_instr_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_rte_callback(T &&... args) { m_rte_instr_callback.set(std::forward<T>(args)...); }
	template <typename... T> void set_tas_write_callback(T &&... args) { m_tas_write_callback.set(std::forward<T>(args)...); }
	u16 get_fc();
	void set_hmmu_enable(int enable);
	int get_pmmu_enable() const {return m_pmmu_enabled;}
	void set_fpu_enable(int enable);
	void set_buserror_details(u32 fault_addr, u8 rw, u8 fc);
	void disable_interrupt_mixer() { m_interrupt_mixer = false; }
	void set_cpu_space(int space_id) { m_cpu_space_id = space_id; }

protected:
	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits);

	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map);

	int    m_has_fpu;      /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */

	bool   m_interrupt_mixer; /* Indicates whether to put a virtual 8->3 priority mixer on the input lines (default true) */
	int    m_cpu_space_id;    /* CPU space address space id (default AS_CPU_SPACE) */

	u32 m_cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
//
	u32 m_dar[16];      /* Data and Address Registers */
	u32 m_ppc;        /* Previous program counter */
	u32 m_pc;           /* Program Counter */
	u32 m_sp[7];        /* User, Interrupt, and Master Stack Pointers */
	u32 m_vbr;          /* Vector Base Register (m68010+) */
	u32 m_sfc;          /* Source Function Code Register (m68010+) */
	u32 m_dfc;          /* Destination Function Code Register (m68010+) */
	u32 m_cacr;         /* Cache Control Register (m68020, unemulated) */
	u32 m_caar;         /* Cache Address Register (m68020, unemulated) */
	u32 m_ir;           /* Instruction Register */
	floatx80 m_fpr[8];     /* FPU Data Register (m68030/040) */
	u32 m_fpiar;        /* FPU Instruction Address Register (m68040) */
	u32 m_fpsr;         /* FPU Status Register (m68040) */
	u32 m_fpcr;         /* FPU Control Register (m68040) */
	u32 m_t1_flag;      /* Trace 1 */
	u32 m_t0_flag;      /* Trace 0 */
	u32 m_s_flag;       /* Supervisor */
	u32 m_m_flag;       /* Master/Interrupt state */
	u32 m_x_flag;       /* Extend */
	u32 m_n_flag;       /* Negative */
	u32 m_not_z_flag;   /* Zero, inverted for speedups */
	u32 m_v_flag;       /* Overflow */
	u32 m_c_flag;       /* Carry */
	u32 m_int_mask;     /* I0-I2 */
	u32 m_int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	u32 m_stopped;      /* Stopped state */
	u32 m_pref_addr;    /* Last prefetch address */
	u32 m_pref_data;    /* Data in the prefetch queue */
	u32 m_sr_mask;      /* Implemented status register bits */
	u32 m_instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	u32 m_run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */
	int    m_has_pmmu;     /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	int    m_has_hmmu;     /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	int    m_pmmu_enabled; /* Indicates if the PMMU is enabled */
	int    m_hmmu_enabled; /* Indicates if the HMMU is enabled */
	int    m_fpu_just_reset; /* Indicates the FPU was just reset */

	/* Clocks required for instructions / exceptions */
	u32 m_cyc_bcc_notake_b;
	u32 m_cyc_bcc_notake_w;
	u32 m_cyc_dbcc_f_noexp;
	u32 m_cyc_dbcc_f_exp;
	u32 m_cyc_scc_r_true;
	u32 m_cyc_movem_w;
	u32 m_cyc_movem_l;
	u32 m_cyc_shift;
	u32 m_cyc_reset;

	int  m_initial_cycles;
	int  m_icount;                     /* Number of clocks remaining */
	int  m_reset_cycles;
	u32 m_tracing;

	int m_address_error;

	u32    m_aerr_address;
	u32    m_aerr_write_mode;
	u32    m_aerr_fc;

	/* Virtual IRQ lines state */
	u32 m_virq_state;
	u32 m_nmi_pending;

	const u16 *m_state_table;
	const u8* m_cyc_instruction;
	const u8* m_cyc_exception;

	/* Callbacks to host */
	write_line_delegate m_reset_instr_callback;           /* Called when a RESET instruction is encountered */
	write32sm_delegate m_cmpild_instr_callback;           /* Called when a CMPI.L #v, Dn instruction is encountered */
	write_line_delegate m_rte_instr_callback;             /* Called when a RTE instruction is encountered */
	write8sm_delegate m_tas_write_callback;               /* Called instead of normal write8 by the TAS instruction,
	                                                        allowing writeback to be disabled globally or selectively
	                                                        or other side effects to be implemented */

	address_space *m_program, *m_oprogram, *m_cpu_space;

	memory_access<24, 0, 0, ENDIANNESS_BIG>::cache m_oprogram8;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::cache m_oprogram16;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_oprogram32;
	memory_access<24, 0, 0, ENDIANNESS_BIG>::specific m_program8;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_program16;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_program32;

	/* Redirect memory calls */

	void init8(address_space &space, address_space &ospace);
	void init16(address_space &space, address_space &ospace);
	void init32(address_space &space, address_space &ospace);
	void init32mmu(address_space &space, address_space &ospace);
	void init32hmmu(address_space &space, address_space &ospace);

	std::function<u16 (offs_t)> m_readimm16;      // Immediate read 16 bit
	std::function<u8  (offs_t)> m_read8;
	std::function<u16 (offs_t)> m_read16;
	std::function<u32 (offs_t)> m_read32;
	std::function<void (offs_t, u8 )> m_write8;
	std::function<void (offs_t, u16)> m_write16;
	std::function<void (offs_t, u32)> m_write32;

	address_space *m_space, *m_ospace;

	u32      m_iotemp;

	/* save state data */
	u16 m_save_sr;
	u8 m_save_stopped;
	u8 m_save_halted;

	/* PMMU registers */
	u32 m_mmu_crp_aptr, m_mmu_crp_limit;
	u32 m_mmu_srp_aptr, m_mmu_srp_limit;
	u32 m_mmu_urp_aptr;    /* 040 only */
	u32 m_mmu_tc;
	u16 m_mmu_sr;
	u32 m_mmu_sr_040;
	u32 m_mmu_atc_tag[MMU_ATC_ENTRIES], m_mmu_atc_data[MMU_ATC_ENTRIES];
	u32 m_mmu_atc_rr;
	u32 m_mmu_tt0, m_mmu_tt1;
	u32 m_mmu_itt0, m_mmu_itt1, m_mmu_dtt0, m_mmu_dtt1;
	u32 m_mmu_acr0, m_mmu_acr1, m_mmu_acr2, m_mmu_acr3;
	u32 m_mmu_last_page_entry, m_mmu_last_page_entry_addr;

	u16 m_mmu_tmp_sr;      /* temporary hack: status code for ptest and to handle write protection */
	u16 m_mmu_tmp_fc;      /* temporary hack: function code for the mmu (moves) */
	u16 m_mmu_tmp_rw;      /* temporary hack: read/write (1/0) for the mmu */
	u8 m_mmu_tmp_sz;       /* temporary hack: size for mmu */

	u32 m_mmu_tmp_buserror_address;   /* temporary hack: (first) bus error address */
	u16 m_mmu_tmp_buserror_occurred;  /* temporary hack: flag that bus error has occurred from mmu */
	u16 m_mmu_tmp_buserror_fc;   /* temporary hack: (first) bus error fc */
	u16 m_mmu_tmp_buserror_rw;   /* temporary hack: (first) bus error rw */
	u16 m_mmu_tmp_buserror_sz;   /* temporary hack: (first) bus error size` */

	bool m_mmu_tablewalk;             /* set when MMU walks page tables */
	u32 m_mmu_last_logical_addr;
	u32 m_ic_address[M68K_IC_SIZE];   /* instruction cache address data */
	u32 m_ic_data[M68K_IC_SIZE];      /* instruction cache content data */
	bool   m_ic_valid[M68K_IC_SIZE];     /* instruction cache valid flags */



	/* 68307 / 68340 internal address map */
	address_space *m_internal;



	void init_cpu_common(void);
	void init_cpu_m68000(void);
	void init_cpu_m68008(void);
	void init_cpu_m68010(void);
	void init_cpu_m68020(void);
	void init_cpu_m68020fpu(void);
	void init_cpu_m68020pmmu(void);
	void init_cpu_m68020hmmu(void);
	void init_cpu_m68ec020(void);
	void init_cpu_m68030(void);
	void init_cpu_m68ec030(void);
	void init_cpu_m68040(void);
	void init_cpu_m68ec040(void);
	void init_cpu_m68lc040(void);
	void init_cpu_fscpu32(void);
	void init_cpu_scc68070(void);
	void init_cpu_coldfire(void);

	void default_autovectors_map(address_map &map);

	void m68ki_exception_interrupt(u32 int_level);

	inline void m68ki_check_address_error(u32 ADDR, u32 WRITE_MODE, u32 FC)
	{
		if((ADDR)&1)
		{
			m_aerr_address = ADDR;
			m_aerr_write_mode = WRITE_MODE;
			m_aerr_fc = FC;
			throw 10;
		}
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual bool memory_translate(int space, int intention, offs_t &address) override;

#include "m68kcpu.h"
#include "m68kops.h"
#include "m68kmmu.h"

	virtual void m68k_reset_peripherals() { }

	static double fx80_to_double(floatx80 fx)
	{
		u64 d;
		double *foo;

		foo = (double *)&d;

		d = floatx80_to_float64(fx);

		return *foo;
	}

	static floatx80 double_to_fx80(double in)
	{
		u64 *d;

		d = (u64 *)&in;

		return float64_to_floatx80(*d);
	}

	// defined in m68kfpu.cpp
	static const u32 pkmask2[18];
	static const u32 pkmask3[18];
	inline floatx80 load_extended_float80(u32 ea);
	inline void store_extended_float80(u32 ea, floatx80 fpr);
	inline floatx80 load_pack_float80(u32 ea);
	inline void store_pack_float80(u32 ea, int k, floatx80 fpr);
	inline void SET_CONDITION_CODES(floatx80 reg);
	inline int TEST_CONDITION(int condition);
	u8 READ_EA_8(int ea);
	u16 READ_EA_16(int ea);
	u32 READ_EA_32(int ea);
	u64 READ_EA_64(int ea);
	floatx80 READ_EA_FPE(int mode, int reg, uint32 di_mode_ea);
	floatx80 READ_EA_PACK(int ea);
	void WRITE_EA_8(int ea, u8 data);
	void WRITE_EA_16(int ea, u16 data);
	void WRITE_EA_32(int ea, u32 data);
	void WRITE_EA_64(int ea, u64 data);
	void WRITE_EA_FPE(int mode, int reg, floatx80 fpr, uint32 di_mode_ea);
	void WRITE_EA_PACK(int ea, int k, floatx80 fpr);
	void fpgen_rm_reg(u16 w2);
	void fmove_reg_mem(u16 w2);
	void fmove_fpcr(u16 w2);
	void fmovem(u16 w2);
	void fscc();
	void fbcc16();
	void fbcc32();
	void m68040_fpu_op0();
	int perform_fsave(u32 addr, int inc);
	void do_frestore_null();
	void m68040_do_fsave(u32 addr, int reg, int inc);
	void m68040_do_frestore(u32 addr, int reg);
	void m68040_fpu_op1();
	void m68881_ftrap();
};



class m68000_device : public m68000_base_device
{
public:
	// construction/destruction
	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);


	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;

protected:
	m68000_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock);

	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map);
};




class m68008_device : public m68000_base_device
{
public:
	// construction/destruction
	m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68008fn_device : public m68000_base_device
{
public:
	// construction/destruction
	m68008fn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68010_device : public m68000_base_device
{
public:
	// construction/destruction
	m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68ec020_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68020_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68020fpu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68020pmmu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68020hmmu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	virtual bool memory_translate(int space, int intention, offs_t &address) override;

	// device-level overrides
	virtual void device_start() override;
};

class m68ec030_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68030_device : public m68000_base_device
{
public:
	// construction/destruction
	m68030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68ec040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68lc040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class m68040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;
};

class scc68070_base_device : public m68000_base_device
{
protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;

	scc68070_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, address_map_constructor internal_map);
};




class fscpu32_device : public m68000_base_device
{
public:
	// construction/destruction
	fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override;

protected:
	fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map);
};



class mcf5206e_device : public m68000_base_device
{
public:
	// construction/destruction
	mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }


	// device-level overrides
	virtual void device_start() override;
};


DECLARE_DEVICE_TYPE(M68000, m68000_device)
DECLARE_DEVICE_TYPE(M68008, m68008_device)
DECLARE_DEVICE_TYPE(M68008FN, m68008fn_device)
DECLARE_DEVICE_TYPE(M68010, m68010_device)
DECLARE_DEVICE_TYPE(M68EC020, m68ec020_device)
DECLARE_DEVICE_TYPE(M68020, m68020_device)
DECLARE_DEVICE_TYPE(M68020FPU, m68020fpu_device)
DECLARE_DEVICE_TYPE(M68020PMMU, m68020pmmu_device)
DECLARE_DEVICE_TYPE(M68020HMMU, m68020hmmu_device)
DECLARE_DEVICE_TYPE(M68EC030, m68ec030_device)
DECLARE_DEVICE_TYPE(M68030, m68030_device)
DECLARE_DEVICE_TYPE(M68EC040, m68ec040_device)
DECLARE_DEVICE_TYPE(M68LC040, m68lc040_device)
DECLARE_DEVICE_TYPE(M68040, m68040_device)
DECLARE_DEVICE_TYPE(FSCPU32, fscpu32_device)
DECLARE_DEVICE_TYPE(MCF5206E, mcf5206e_device)


#endif // MAME_CPU_M68000_M68000_H
