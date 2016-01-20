// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#pragma once

#ifndef __M68000_H__
#define __M68000_H__




#include "softfloat/milieu.h"
#include "softfloat/softfloat.h"


/* MMU constants */
#define MMU_ATC_ENTRIES (22)    // 68851 has 64, 030 has 22

/* instruction cache constants */
#define M68K_IC_SIZE 128




#define m68ki_check_address_error(m68k, ADDR, WRITE_MODE, FC) \
	if((ADDR)&1) \
	{ \
		m68k->aerr_address = ADDR; \
		m68k->aerr_write_mode = WRITE_MODE; \
		m68k->aerr_fc = FC; \
		throw 10; \
	}



/* There are 7 levels of interrupt to the 68K.
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 */
#define M68K_IRQ_NONE 0
#define M68K_IRQ_1    1
#define M68K_IRQ_2    2
#define M68K_IRQ_3    3
#define M68K_IRQ_4    4
#define M68K_IRQ_5    5
#define M68K_IRQ_6    6
#define M68K_IRQ_7    7

// special input lines
#define M68K_LINE_BUSERROR 16

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
#define M68K_HMMU_DISABLE   0   /* no translation */
#define M68K_HMMU_ENABLE_II 1   /* Mac II style fixed translation */
#define M68K_HMMU_ENABLE_LC 2   /* Mac LC style fixed translation */

/* Special interrupt acknowledge values.
 * Use these as special returns from the interrupt acknowledge callback
 * (specified later in this header).
 */

/* Causes an interrupt autovector (0x18 + interrupt level) to be taken.
 * This happens in a real 68K if VPA or AVEC is asserted during an interrupt
 * acknowledge cycle instead of DTACK.
 */
#define M68K_INT_ACK_AUTOVECTOR    0xffffffff

/* Causes the spurious interrupt vector (0x18) to be taken
 * This happens in a real 68K if BERR is asserted during the interrupt
 * acknowledge cycle (i.e. no devices responded to the acknowledge).
 */
#define M68K_INT_ACK_SPURIOUS      0xfffffffe

enum
{
	/* NOTE: M68K_SP fetches the current SP, be it USP, ISP, or MSP */
	M68K_PC, M68K_SP, M68K_ISP, M68K_USP, M68K_MSP, M68K_SR, M68K_VBR,
	M68K_SFC, M68K_DFC, M68K_CACR, M68K_CAAR, M68K_PREF_ADDR, M68K_PREF_DATA,
	M68K_D0, M68K_D1, M68K_D2, M68K_D3, M68K_D4, M68K_D5, M68K_D6, M68K_D7,
	M68K_A0, M68K_A1, M68K_A2, M68K_A3, M68K_A4, M68K_A5, M68K_A6, M68K_A7,
	M68K_FP0, M68K_FP1, M68K_FP2, M68K_FP3, M68K_FP4, M68K_FP5, M68K_FP6, M68K_FP7,
	M68K_FPSR, M68K_FPCR,

	M68K_GENPC = STATE_GENPC,
	M68K_GENSP = STATE_GENSP,
	M68K_GENPCBASE = STATE_GENPCBASE
};

unsigned int m68k_disassemble_raw(char* str_buff, unsigned int pc, const unsigned char* opdata, const unsigned char* argdata, unsigned int cpu_type);

class m68000_base_device;


extern const device_type M68K;

class m68000_base_device : public cpu_device
{
public:

	// construction/destruction
	m68000_base_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
						const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, const char *shortname, const char *source);

	m68000_base_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
						const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, address_map_constructor internal_map, const char *shortname, const char *source);

	m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( write_irq1 );
	DECLARE_WRITE_LINE_MEMBER( write_irq2 );
	DECLARE_WRITE_LINE_MEMBER( write_irq3 );
	DECLARE_WRITE_LINE_MEMBER( write_irq4 );
	DECLARE_WRITE_LINE_MEMBER( write_irq5 );
	DECLARE_WRITE_LINE_MEMBER( write_irq6 );
	DECLARE_WRITE_LINE_MEMBER( write_irq7 );

	void clear_all(void);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;




	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };
	virtual UINT32 execute_input_lines() const override { return 8; }; // number of input lines
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// address spaces
	const address_space_config m_program_config, m_oprogram_config;

	void define_state(void);

	void set_reset_callback(write_line_delegate callback);
	void set_cmpild_callback(write32_delegate callback);
	void set_rte_callback(write_line_delegate callback);
	void set_tas_write_callback(write8_delegate callback);
	UINT16 get_fc();
	void set_hmmu_enable(int enable);
	void set_instruction_hook(read32_delegate ihook);
	void set_buserror_details(UINT32 fault_addr, UINT8 rw, UINT8 fc);

public:


	UINT32 cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, 68020, 68EC030, 68030, 68EC040, or 68040 */
//  UINT32 dasm_type;    /* disassembly type */
	UINT32 dar[16];      /* Data and Address Registers */
	UINT32 ppc;        /* Previous program counter */
	UINT32 pc;           /* Program Counter */
	UINT32 sp[7];        /* User, Interrupt, and Master Stack Pointers */
	UINT32 vbr;          /* Vector Base Register (m68010+) */
	UINT32 sfc;          /* Source Function Code Register (m68010+) */
	UINT32 dfc;          /* Destination Function Code Register (m68010+) */
	UINT32 cacr;         /* Cache Control Register (m68020, unemulated) */
	UINT32 caar;         /* Cache Address Register (m68020, unemulated) */
	UINT32 ir;           /* Instruction Register */
	floatx80 fpr[8];     /* FPU Data Register (m68030/040) */
	UINT32 fpiar;        /* FPU Instruction Address Register (m68040) */
	UINT32 fpsr;         /* FPU Status Register (m68040) */
	UINT32 fpcr;         /* FPU Control Register (m68040) */
	UINT32 t1_flag;      /* Trace 1 */
	UINT32 t0_flag;      /* Trace 0 */
	UINT32 s_flag;       /* Supervisor */
	UINT32 m_flag;       /* Master/Interrupt state */
	UINT32 x_flag;       /* Extend */
	UINT32 n_flag;       /* Negative */
	UINT32 not_z_flag;   /* Zero, inverted for speedups */
	UINT32 v_flag;       /* Overflow */
	UINT32 c_flag;       /* Carry */
	UINT32 int_mask;     /* I0-I2 */
	UINT32 int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	UINT32 stopped;      /* Stopped state */
	UINT32 pref_addr;    /* Last prefetch address */
	UINT32 pref_data;    /* Data in the prefetch queue */
	UINT32 sr_mask;      /* Implemented status register bits */
	UINT32 instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	UINT32 run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */
	int    has_pmmu;     /* Indicates if a PMMU available (yes on 030, 040, no on EC030) */
	int    has_hmmu;     /* Indicates if an Apple HMMU is available in place of the 68851 (020 only) */
	int    pmmu_enabled; /* Indicates if the PMMU is enabled */
	int    hmmu_enabled; /* Indicates if the HMMU is enabled */
	int    has_fpu;      /* Indicates if a FPU is available (yes on 030, 040, may be on 020) */
	int    fpu_just_reset; /* Indicates the FPU was just reset */

	/* Clocks required for instructions / exceptions */
	UINT32 cyc_bcc_notake_b;
	UINT32 cyc_bcc_notake_w;
	UINT32 cyc_dbcc_f_noexp;
	UINT32 cyc_dbcc_f_exp;
	UINT32 cyc_scc_r_true;
	UINT32 cyc_movem_w;
	UINT32 cyc_movem_l;
	UINT32 cyc_shift;
	UINT32 cyc_reset;

	int  initial_cycles;
	int  remaining_cycles;                     /* Number of clocks remaining */
	int  reset_cycles;
	UINT32 tracing;

	int m_address_error;

	UINT32    aerr_address;
	UINT32    aerr_write_mode;
	UINT32    aerr_fc;

	/* Virtual IRQ lines state */
	UINT32 virq_state;
	UINT32 nmi_pending;

	void (**jump_table)(m68000_base_device *m68k);
	const UINT8* cyc_instruction;
	const UINT8* cyc_exception;

	/* Callbacks to host */
	device_irq_acknowledge_delegate int_ack_callback;   /* Interrupt Acknowledge */
	write32_delegate bkpt_ack_callback;                 /* Breakpoint Acknowledge */
	write_line_delegate reset_instr_callback;           /* Called when a RESET instruction is encountered */
	write32_delegate cmpild_instr_callback;             /* Called when a CMPI.L #v, Dn instruction is encountered */
	write_line_delegate rte_instr_callback;             /* Called when a RTE instruction is encountered */
	write8_delegate tas_write_callback;                 /* Called instead of normal write8 by the TAS instruction,
                                                            allowing writeback to be disabled globally or selectively
                                                            or other side effects to be implemented */

	address_space *program, *oprogram;

	/* Redirect memory calls */

	typedef delegate<UINT8 (offs_t)> m68k_read8_delegate;
	typedef delegate<UINT16 (offs_t)> m68k_readimm16_delegate;
	typedef delegate<UINT16 (offs_t)> m68k_read16_delegate;
	typedef delegate<UINT32 (offs_t)> m68k_read32_delegate;
	typedef delegate<void (offs_t, UINT8)> m68k_write8_delegate;
	typedef delegate<void (offs_t, UINT16)> m68k_write16_delegate;
	typedef delegate<void (offs_t, UINT32)> m68k_write32_delegate;

//  class m68k_memory_interface
//  {
	public:
		void init8(address_space &space, address_space &ospace);
		void init16(address_space &space, address_space &ospace);
		void init32(address_space &space, address_space &ospace);
		void init32mmu(address_space &space, address_space &ospace);
		void init32hmmu(address_space &space, address_space &ospace);

		offs_t  opcode_xor;                     // Address Calculation
		m68k_readimm16_delegate readimm16;      // Immediate read 16 bit
		m68k_read8_delegate read8;
		m68k_read16_delegate read16;
		m68k_read32_delegate read32;
		m68k_write8_delegate write8;
		m68k_write16_delegate write16;
		m68k_write32_delegate write32;

	private:
		UINT16 m68008_read_immediate_16(offs_t address);
		UINT16 read_immediate_16(offs_t address);
		UINT16 simple_read_immediate_16(offs_t address);

		void m68000_write_byte(offs_t address, UINT8 data);

		UINT8 read_byte_32_mmu(offs_t address);
		void write_byte_32_mmu(offs_t address, UINT8 data);
		UINT16 read_immediate_16_mmu(offs_t address);
		UINT16 readword_d32_mmu(offs_t address);
		void writeword_d32_mmu(offs_t address, UINT16 data);
		UINT32 readlong_d32_mmu(offs_t address);
		void writelong_d32_mmu(offs_t address, UINT32 data);

		UINT8 read_byte_32_hmmu(offs_t address);
		void write_byte_32_hmmu(offs_t address, UINT8 data);
		UINT16 read_immediate_16_hmmu(offs_t address);
		UINT16 readword_d32_hmmu(offs_t address);
		void writeword_d32_hmmu(offs_t address, UINT16 data);
		UINT32 readlong_d32_hmmu(offs_t address);
		void writelong_d32_hmmu(offs_t address, UINT32 data);

//      m68000_base_device *m_cpustate;
//  };

	public:
//  m68k_memory_interface memory;

	address_space *m_space, *m_ospace;
	direct_read_data *m_direct, *m_odirect;

	UINT32      iotemp;

	/* save state data */
	UINT16 save_sr;
	UINT8 save_stopped;
	UINT8 save_halted;

	/* PMMU registers */
	UINT32 mmu_crp_aptr, mmu_crp_limit;
	UINT32 mmu_srp_aptr, mmu_srp_limit;
	UINT32 mmu_urp_aptr;    /* 040 only */
	UINT32 mmu_tc;
	UINT16 mmu_sr;
	UINT32 mmu_sr_040;
	UINT32 mmu_atc_tag[MMU_ATC_ENTRIES], mmu_atc_data[MMU_ATC_ENTRIES];
	UINT32 mmu_atc_rr;
	UINT32 mmu_tt0, mmu_tt1;
	UINT32 mmu_itt0, mmu_itt1, mmu_dtt0, mmu_dtt1;
	UINT32 mmu_acr0, mmu_acr1, mmu_acr2, mmu_acr3;
	UINT32 mmu_last_page_entry, mmu_last_page_entry_addr;

	UINT16 mmu_tmp_sr;      /* temporary hack: status code for ptest and to handle write protection */
	UINT16 mmu_tmp_fc;      /* temporary hack: function code for the mmu (moves) */
	UINT16 mmu_tmp_rw;      /* temporary hack: read/write (1/0) for the mmu */
	UINT32 mmu_tmp_buserror_address;   /* temporary hack: (first) bus error address */
	UINT16 mmu_tmp_buserror_occurred;  /* temporary hack: flag that bus error has occurred from mmu */
	UINT16 mmu_tmp_buserror_fc;   /* temporary hack: (first) bus error fc */
	UINT16 mmu_tmp_buserror_rw;   /* temporary hack: (first) bus error rw */

	UINT32 ic_address[M68K_IC_SIZE];   /* instruction cache address data */
	UINT32 ic_data[M68K_IC_SIZE];      /* instruction cache content data */
	bool   ic_valid[M68K_IC_SIZE];     /* instruction cache valid flags */



	/* 68307 / 68340 internal address map */
	address_space *internal;



	/* external instruction hook (does not depend on debug mode) */
	read32_delegate instruction_hook;



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


	void m68ki_exception_interrupt(m68000_base_device *m68k, UINT32 int_level);

	void reset_cpu(void);
	inline void cpu_execute(void);

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual bool memory_translate(address_spacenum space, int intention, offs_t &address) override;
};



class m68000_device : public m68000_base_device
{
public:
	// construction/destruction
	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	m68000_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
						const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, address_map_constructor internal_map, const char *shortname, const char *source);



	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68301_device : public m68000_base_device
{
public:
	// construction/destruction
	m68301_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};




class m68008_device : public m68000_base_device
{
public:
	// construction/destruction
	m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68008plcc_device : public m68000_base_device
{
public:
	// construction/destruction
	m68008plcc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68010_device : public m68000_base_device
{
public:
	// construction/destruction
	m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68ec020_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020fpu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020pmmu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68020hmmu_device : public m68000_base_device
{
public:
	// construction/destruction
	m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	virtual bool memory_translate(address_spacenum space, int intention, offs_t &address) override;

	// device-level overrides
	virtual void device_start() override;
};

class m68ec030_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68030_device : public m68000_base_device
{
public:
	// construction/destruction
	m68030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68ec040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68lc040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class m68040_device : public m68000_base_device
{
public:
	// construction/destruction
	m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};

class scc68070_device : public m68000_base_device
{
public:
	// construction/destruction
	scc68070_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 10; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 4; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};




class fscpu32_device : public m68000_base_device
{
public:
	// construction/destruction
	fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	fscpu32_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
						const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, address_map_constructor internal_map, const char *shortname, const char *source);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };

	// device-level overrides
	virtual void device_start() override;
};



class mcf5206e_device : public m68000_base_device
{
public:
	// construction/destruction
	mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; };
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; };
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual UINT32 execute_min_cycles() const override { return 2; };
	virtual UINT32 execute_max_cycles() const override { return 158; };

	virtual UINT32 execute_default_irq_vector() const override { return -1; };


	// device-level overrides
	virtual void device_start() override;
};


extern const device_type M68000;
extern const device_type M68301;
extern const device_type M68008;
extern const device_type M68008PLCC;
extern const device_type M68010;
extern const device_type M68EC020;
extern const device_type M68020;
extern const device_type M68020FPU;
extern const device_type M68020PMMU;
extern const device_type M68020HMMU;
extern const device_type M68EC030;
extern const device_type M68030;
extern const device_type M68EC040;
extern const device_type M68LC040;
extern const device_type M68040;
extern const device_type SCC68070;
extern const device_type FSCPU32;
extern const device_type MCF5206E;


#endif /* __M68000_H__ */
