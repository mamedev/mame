// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_M68000_M68KMUSASHI_H
#define MAME_CPU_M68000_M68KMUSASHI_H

#pragma once

#include "m68kcommon.h"

#include "softfloat3/source/include/softfloat.h"
#include "softfloat3/bochs_ext/softfloat3_ext.h"

/* MMU constants */
constexpr int MMU_ATC_ENTRIES = (22);    // 68851 has 64, 030 has 22

/* instruction cache constants */
constexpr int M68K_IC_SIZE = 128;

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

class m68000_musashi_device : public m68000_base_device
{
public:
	// construction/destruction
	m68000_musashi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual bool supervisor_mode() const noexcept override;

protected:
	static constexpr int NUM_CPU_TYPES = 8;

	typedef void (m68000_musashi_device::*opcode_handler_ptr)();
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
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == M68K_LINE_BUSERROR || (m_interrupt_mixer ? inputnum == M68K_IRQ_7 : false); }

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
	virtual u16 get_fc() const noexcept override;
	void set_hmmu_enable(int enable);
	void set_emmu_enable(bool enable);
	bool get_pmmu_enable() const {return m_pmmu_enabled;}
	void set_fpu_enable(bool enable);
	void set_buserror_details(u32 fault_addr, u8 rw, u8 fc, bool rerun = false);
	void restart_this_instruction();

protected:
	m68000_musashi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits);

	m68000_musashi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map);

	bool m_has_fpu;     /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */

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
	extFloat80_t m_fpr[8]; /* FPU Data Register (m68030/040) */
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
	bool m_has_pmmu;     /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	bool m_has_hmmu;     /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	bool m_pmmu_enabled; /* Indicates if the PMMU is enabled */
	int m_hmmu_enabled;  /* Indicates if the HMMU is enabled */
	bool m_emmu_enabled; /* Indicates if external MMU is enabled */
	bool m_instruction_restart; /* Save DA regs for potential instruction restart */
	bool m_fpu_just_reset; /* Indicates the FPU was just reset */
	bool m_restart_instruction; /* Indicates the instruction should be restarted */

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
	virtual bool memory_translate(int space, int intention, offs_t &address, address_space *&target_space) override;

#include "m68kcpu.h"
#include "m68kops.h"
#include "m68kmmu.h"

	static double fx80_to_double(extFloat80_t fx)
	{
		const float64_t d = extF80_to_f64(fx);
		const double *foo = (double *)&d;

		return *foo;
	}

	static extFloat80_t double_to_fx80(double in)
	{
		float64_t *d = (float64_t *)&in;

		return f64_to_extF80(*d);
	}

	// defined in m68kfpu.cpp
	static const u32 pkmask2[18];
	static const u32 pkmask3[18];
	inline extFloat80_t load_extended_float80(u32 ea);
	inline void store_extended_float80(u32 ea, extFloat80_t fpr);
	inline extFloat80_t load_pack_float80(u32 ea);
	inline void store_pack_float80(u32 ea, int k, extFloat80_t fpr);
	void set_condition_codes(extFloat80_t reg);
	int test_condition(int condition);
	void clear_exception_flags();
	void sync_exception_flags(extFloat80_t op1, extFloat80_t op2, u32 enables);
	s32 convert_to_int(extFloat80_t source, s32 lowerLimit, s32 upperLimit);
	u8 READ_EA_8(int ea);
	u16 READ_EA_16(int ea);
	u32 READ_EA_32(int ea);
	u64 READ_EA_64(int ea);
	extFloat80_t READ_EA_FPE(int mode, int reg, uint32_t di_mode_ea);
	extFloat80_t READ_EA_PACK(int ea);
	void WRITE_EA_8(int ea, u8 data);
	void WRITE_EA_16(int ea, u16 data);
	void WRITE_EA_32(int ea, u32 data);
	void WRITE_EA_64(int ea, u64 data);
	void WRITE_EA_FPE(int mode, int reg, extFloat80_t fpr, uint32_t di_mode_ea);
	void WRITE_EA_PACK(int ea, int k, extFloat80_t fpr);
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

#endif // MAME_CPU_M68000_M68KMUSASHI_H
