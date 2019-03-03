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
extern flag floatx80_is_nan(floatx80 a);
#endif


/* MMU constants */
constexpr int MMU_ATC_ENTRIES = (22);    // 68851 has 64, 030 has 22

/* instruction cache constants */
constexpr int M68K_IC_SIZE = 128;

/* There are 7 levels of interrupt to the 68K.
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 */
constexpr int M68K_IRQ_NONE = 0;
constexpr int M68K_IRQ_1    = 1;
constexpr int M68K_IRQ_2    = 2;
constexpr int M68K_IRQ_3    = 3;
constexpr int M68K_IRQ_4    = 4;
constexpr int M68K_IRQ_5    = 5;
constexpr int M68K_IRQ_6    = 6;
constexpr int M68K_IRQ_7    = 7;

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

/* Special interrupt acknowledge values.
 * Use these as special returns from the interrupt acknowledge callback
 * (specified later in this header).
 */

/* Causes an interrupt autovector (0x18 + interrupt level) to be taken.
 * This happens in a real 68K if VPA or AVEC is asserted during an interrupt
 * acknowledge cycle instead of DTACK.
 */
constexpr uint32_t M68K_INT_ACK_AUTOVECTOR    = 0xffffffff;

/* Causes the spurious interrupt vector (0x18) to be taken
 * This happens in a real 68K if BERR is asserted during the interrupt
 * acknowledge cycle (i.e. no devices responded to the acknowledge).
 */
constexpr uint32_t M68K_INT_ACK_SPURIOUS      = 0xfffffffe;

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

	// construction/destruction
	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void presave();
	void postload();

	void clear_all(void);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };
	virtual uint32_t execute_input_lines() const override { return 8; }; // number of input lines
	virtual uint32_t execute_default_irq_vector(int inputnum) const override { return M68K_INT_ACK_AUTOVECTOR; }
	virtual bool execute_input_edge_triggered(int inputnum) const override { return inputnum == M68K_IRQ_7; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// address spaces
	const address_space_config m_program_config, m_oprogram_config;

	void define_state(void);

public:
	void set_reset_callback(write_line_delegate callback);
	void set_cmpild_callback(write32_delegate callback);
	void set_rte_callback(write_line_delegate callback);
	void set_tas_write_callback(write8_delegate callback);
	uint16_t get_fc();
	void set_hmmu_enable(int enable);
	void set_fpu_enable(int enable);
	void set_buserror_details(uint32_t fault_addr, uint8_t rw, uint8_t fc);

protected:
	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
						const device_type type, uint32_t prg_data_width, uint32_t prg_address_bits);

	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
						const device_type type, uint32_t prg_data_width, uint32_t prg_address_bits, address_map_constructor internal_map);

	int    m_has_fpu;      /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */


	uint32_t m_cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
//
	uint32_t m_dar[16];      /* Data and Address Registers */
	uint32_t m_ppc;        /* Previous program counter */
	uint32_t m_pc;           /* Program Counter */
	uint32_t m_sp[7];        /* User, Interrupt, and Master Stack Pointers */
	uint32_t m_vbr;          /* Vector Base Register (m68010+) */
	uint32_t m_sfc;          /* Source Function Code Register (m68010+) */
	uint32_t m_dfc;          /* Destination Function Code Register (m68010+) */
	uint32_t m_cacr;         /* Cache Control Register (m68020, unemulated) */
	uint32_t m_caar;         /* Cache Address Register (m68020, unemulated) */
	uint32_t m_ir;           /* Instruction Register */
	floatx80 m_fpr[8];     /* FPU Data Register (m68030/040) */
	uint32_t m_fpiar;        /* FPU Instruction Address Register (m68040) */
	uint32_t m_fpsr;         /* FPU Status Register (m68040) */
	uint32_t m_fpcr;         /* FPU Control Register (m68040) */
	uint32_t m_t1_flag;      /* Trace 1 */
	uint32_t m_t0_flag;      /* Trace 0 */
	uint32_t m_s_flag;       /* Supervisor */
	uint32_t m_m_flag;       /* Master/Interrupt state */
	uint32_t m_x_flag;       /* Extend */
	uint32_t m_n_flag;       /* Negative */
	uint32_t m_not_z_flag;   /* Zero, inverted for speedups */
	uint32_t m_v_flag;       /* Overflow */
	uint32_t m_c_flag;       /* Carry */
	uint32_t m_int_mask;     /* I0-I2 */
	uint32_t m_int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	uint32_t m_stopped;      /* Stopped state */
	uint32_t m_pref_addr;    /* Last prefetch address */
	uint32_t m_pref_data;    /* Data in the prefetch queue */
	uint32_t m_sr_mask;      /* Implemented status register bits */
	uint32_t m_instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	uint32_t m_run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */
	int    m_has_pmmu;     /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	int    m_has_hmmu;     /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	int    m_pmmu_enabled; /* Indicates if the PMMU is enabled */
	int    m_hmmu_enabled; /* Indicates if the HMMU is enabled */
	int    m_fpu_just_reset; /* Indicates the FPU was just reset */

	/* Clocks required for instructions / exceptions */
	uint32_t m_cyc_bcc_notake_b;
	uint32_t m_cyc_bcc_notake_w;
	uint32_t m_cyc_dbcc_f_noexp;
	uint32_t m_cyc_dbcc_f_exp;
	uint32_t m_cyc_scc_r_true;
	uint32_t m_cyc_movem_w;
	uint32_t m_cyc_movem_l;
	uint32_t m_cyc_shift;
	uint32_t m_cyc_reset;

	int  m_initial_cycles;
	int  m_remaining_cycles;                     /* Number of clocks remaining */
	int  m_reset_cycles;
	uint32_t m_tracing;

	int m_address_error;

	uint32_t    m_aerr_address;
	uint32_t    m_aerr_write_mode;
	uint32_t    m_aerr_fc;

	/* Virtual IRQ lines state */
	uint32_t m_virq_state;
	uint32_t m_nmi_pending;

	void (m68000_base_device::**m_jump_table)();
	const uint8_t* m_cyc_instruction;
	const uint8_t* m_cyc_exception;

	/* Callbacks to host */
	device_irq_acknowledge_delegate m_int_ack_callback;   /* Interrupt Acknowledge */
	write32_delegate m_bkpt_ack_callback;                 /* Breakpoint Acknowledge */
	write_line_delegate m_reset_instr_callback;           /* Called when a RESET instruction is encountered */
	write32_delegate m_cmpild_instr_callback;             /* Called when a CMPI.L #v, Dn instruction is encountered */
	write_line_delegate m_rte_instr_callback;             /* Called when a RTE instruction is encountered */
	write8_delegate m_tas_write_callback;                 /* Called instead of normal write8 by the TAS instruction,
	                                                        allowing writeback to be disabled globally or selectively
	                                                        or other side effects to be implemented */

	address_space *m_program, *m_oprogram;

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

	uint32_t      m_iotemp;

	/* save state data */
	uint16_t m_save_sr;
	uint8_t m_save_stopped;
	uint8_t m_save_halted;

	/* PMMU registers */
	uint32_t m_mmu_crp_aptr, m_mmu_crp_limit;
	uint32_t m_mmu_srp_aptr, m_mmu_srp_limit;
	uint32_t m_mmu_urp_aptr;    /* 040 only */
	uint32_t m_mmu_tc;
	uint16_t m_mmu_sr;
	uint32_t m_mmu_sr_040;
	uint32_t m_mmu_atc_tag[MMU_ATC_ENTRIES], m_mmu_atc_data[MMU_ATC_ENTRIES];
	uint32_t m_mmu_atc_rr;
	uint32_t m_mmu_tt0, m_mmu_tt1;
	uint32_t m_mmu_itt0, m_mmu_itt1, m_mmu_dtt0, m_mmu_dtt1;
	uint32_t m_mmu_acr0, m_mmu_acr1, m_mmu_acr2, m_mmu_acr3;
	uint32_t m_mmu_last_page_entry, m_mmu_last_page_entry_addr;

	uint16_t m_mmu_tmp_sr;      /* temporary hack: status code for ptest and to handle write protection */
	uint16_t m_mmu_tmp_fc;      /* temporary hack: function code for the mmu (moves) */
	uint16_t m_mmu_tmp_rw;      /* temporary hack: read/write (1/0) for the mmu */
	uint8_t m_mmu_tmp_sz;       /* temporary hack: size for mmu */

	uint32_t m_mmu_tmp_buserror_address;   /* temporary hack: (first) bus error address */
	uint16_t m_mmu_tmp_buserror_occurred;  /* temporary hack: flag that bus error has occurred from mmu */
	uint16_t m_mmu_tmp_buserror_fc;   /* temporary hack: (first) bus error fc */
	uint16_t m_mmu_tmp_buserror_rw;   /* temporary hack: (first) bus error rw */
	uint16_t m_mmu_tmp_buserror_sz;   /* temporary hack: (first) bus error size` */

	bool m_mmu_tablewalk;             /* set when MMU walks page tables */
	uint32_t m_mmu_last_logical_addr;
	uint32_t m_ic_address[M68K_IC_SIZE];   /* instruction cache address data */
	uint32_t m_ic_data[M68K_IC_SIZE];      /* instruction cache content data */
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


	void m68ki_exception_interrupt(uint32_t int_level);

	inline void m68ki_check_address_error(uint32_t ADDR, uint32_t WRITE_MODE, uint32_t FC)
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
#include "m68kfpu.hxx"
#include "m68kmmu.h"

	virtual void m68k_reset_peripherals() { }
};



class m68000_device : public m68000_base_device
{
public:
	// construction/destruction
	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;

protected:
	m68000_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock);

	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
						const device_type type, uint32_t prg_data_width, uint32_t prg_address_bits, address_map_constructor internal_map);
};

class m68301_device : public m68000_base_device
{
public:
	// construction/destruction
	m68301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};




class m68008_device : public m68000_base_device
{
public:
	// construction/destruction
	m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68008plcc_device : public m68000_base_device
{
public:
	// construction/destruction
	m68008plcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68010_device : public m68000_base_device
{
public:
	// construction/destruction
	m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68ec020_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020fpu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020pmmu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020hmmu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	virtual bool memory_translate(int space, int intention, offs_t &address) override;

	// device-level overrides
	virtual void device_start() override;
};

class m68ec030_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68030_device : public m68000_base_device
{
public:
	// construction/destruction
	m68030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68ec040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68lc040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class m68040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};

class scc68070_device : public m68000_base_device
{
public:
	// construction/destruction
	scc68070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 4; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;
};




class fscpu32_device : public m68000_base_device
{
public:
	// construction/destruction
	fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };

	// device-level overrides
	virtual void device_start() override;

protected:
	fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
						const device_type type, uint32_t prg_data_width, uint32_t prg_address_bits, address_map_constructor internal_map);
};



class mcf5206e_device : public m68000_base_device
{
public:
	// construction/destruction
	mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual uint32_t execute_min_cycles() const override { return 2; };
	virtual uint32_t execute_max_cycles() const override { return 158; };


	// device-level overrides
	virtual void device_start() override;
};


DECLARE_DEVICE_TYPE(M68000, m68000_device)
DECLARE_DEVICE_TYPE(M68301, m68301_device)
DECLARE_DEVICE_TYPE(M68008, m68008_device)
DECLARE_DEVICE_TYPE(M68008PLCC, m68008plcc_device)
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
DECLARE_DEVICE_TYPE(SCC68070, scc68070_device)
DECLARE_DEVICE_TYPE(FSCPU32, fscpu32_device)
DECLARE_DEVICE_TYPE(MCF5206E, mcf5206e_device)


#endif // MAME_CPU_M68000_M68000_H
