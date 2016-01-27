// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

    mcs48.c

    Intel MCS-48/UPI-41 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator
    You are not allowed to distribute this software commercially

***************************************************************************/

#pragma once

#ifndef __MCS48_H__
#define __MCS48_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register access indexes */
enum
{
	MCS48_PC,
	MCS48_PSW,
	MCS48_A,
	MCS48_TC,
	MCS48_TPRE,
	MCS48_P1,
	MCS48_P2,
	MCS48_R0,
	MCS48_R1,
	MCS48_R2,
	MCS48_R3,
	MCS48_R4,
	MCS48_R5,
	MCS48_R6,
	MCS48_R7,
	MCS48_EA,
	MCS48_STS,  /* UPI-41 systems only */
	MCS48_DBBO, /* UPI-41 systems only */
	MCS48_DBBI, /* UPI-41 systems only */

	MCS48_GENPC = STATE_GENPC,
	MCS48_GENSP = STATE_GENSP,
	MCS48_GENPCBASE = STATE_GENPCBASE
};


/* I/O port access indexes */
enum
{
	MCS48_INPUT_IRQ = 0,
	UPI41_INPUT_IBF = 0,
	MCS48_INPUT_EA
};


/* special I/O space ports */
enum
{
	MCS48_PORT_P0   = 0x100,    /* Not used */
	MCS48_PORT_P1   = 0x101,
	MCS48_PORT_P2   = 0x102,
	MCS48_PORT_T0   = 0x110,
	MCS48_PORT_T1   = 0x111,
	MCS48_PORT_BUS  = 0x120,
	MCS48_PORT_PROG = 0x121     /* PROG line to 8243 expander */
};


/* 8243 expander operations */
enum
{
	MCS48_EXPANDER_OP_READ = 0,
	MCS48_EXPANDER_OP_WRITE = 1,
	MCS48_EXPANDER_OP_OR = 2,
	MCS48_EXPANDER_OP_AND = 3
};



/***************************************************************************
    MACROS
***************************************************************************/

#define MCS48_LC_CLOCK(_L, _C) \
	(1 / (2 * 3.14159265358979323846 * sqrt(_L * _C)))

#define MCS48_ALE_CLOCK(_clock) \
	attotime::from_hz(_clock/(3*5))

/* Official Intel MCS-48 parts */
extern const device_type I8021;            /* 1k internal ROM,      64 bytes internal RAM */
extern const device_type I8022;            /* 2k internal ROM,     128 bytes internal RAM */
extern const device_type I8035;            /* external ROM,         64 bytes internal RAM */
extern const device_type I8048;            /* 1k internal ROM,      64 bytes internal RAM */
extern const device_type I8648;            /* 1k internal OTP ROM,  64 bytes internal RAM */
extern const device_type I8748;            /* 1k internal EEPROM,   64 bytes internal RAM */
extern const device_type I8039;            /* external ROM,        128 bytes internal RAM */
extern const device_type I8049;            /* 2k internal ROM,     128 bytes internal RAM */
extern const device_type I8749;            /* 2k internal EEPROM,  128 bytes internal RAM */
extern const device_type I8040;            /* external ROM,        256 bytes internal RAM */
extern const device_type I8050;            /* 4k internal ROM,     256 bytes internal RAM */

/* Official Intel UPI-41 parts */
extern const device_type I8041;            /* 1k internal ROM,     128 bytes internal RAM */
extern const device_type I8741;            /* 1k internal EEPROM,  128 bytes internal RAM */
extern const device_type I8042;            /* 2k internal ROM,     256 bytes internal RAM */
extern const device_type I8242;            /* 2k internal ROM,     256 bytes internal RAM */
extern const device_type I8742;            /* 2k internal EEPROM,  256 bytes internal RAM */

/* Clones */
extern const device_type MB8884;           /* 8035 clone */
extern const device_type N7751;            /* 8048 clone */
extern const device_type M58715;           /* 8049 clone */



class mcs48_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mcs48_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int rom_size, int ram_size, UINT8 feature_mask = 0);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 15 - 1) / 15; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 15); }
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 3; }
	virtual UINT32 execute_input_lines() const override { return 2; }
	virtual UINT32 execute_default_irq_vector() const override { return MCS48_INPUT_IRQ; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ) );
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

protected:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	UINT16      m_prevpc;             /* 16-bit previous program counter */
	UINT16      m_pc;                 /* 16-bit program counter */

	UINT8       m_a;                  /* 8-bit accumulator */
	UINT8 *     m_regptr;             /* pointer to r0-r7 */
	UINT8       m_psw;                /* 8-bit psw */
	UINT8       m_p1;                 /* 8-bit latched port 1 */
	UINT8       m_p2;                 /* 8-bit latched port 2 */
	UINT8       m_ea;                 /* 1-bit latched ea input */
	UINT8       m_timer;              /* 8-bit timer */
	UINT8       m_prescaler;          /* 5-bit timer prescaler */
	UINT8       m_t1_history;         /* 8-bit history of the T1 input */
	UINT8       m_sts;                /* 8-bit status register (UPI-41 only, except for F1) */
	UINT8       m_dbbi;               /* 8-bit input data buffer (UPI-41 only) */
	UINT8       m_dbbo;               /* 8-bit output data buffer (UPI-41 only) */

	UINT8       m_irq_state;          /* TRUE if an IRQ is pending */
	UINT8       m_irq_in_progress;    /* TRUE if an IRQ is in progress */
	UINT8       m_timer_overflow;     /* TRUE on a timer overflow; cleared by taking interrupt */
	UINT8       m_timer_flag;         /* TRUE on a timer overflow; cleared on JTF */
	UINT8       m_tirq_enabled;       /* TRUE if the timer IRQ is enabled */
	UINT8       m_xirq_enabled;       /* TRUE if the external IRQ is enabled */
	UINT8       m_timecount_enabled;  /* bitmask of timer/counter enabled */
	UINT8       m_flags_enabled;      /* TRUE if I/O flags have been enabled (UPI-41 only) */
	UINT8       m_dma_enabled;        /* TRUE if DMA has been enabled (UPI-41 only) */

	UINT16      m_a11;                /* A11 value, either 0x000 or 0x800 */

	int         m_icount;

	/* Memory spaces */
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	UINT8       m_feature_mask;       /* processor feature flags */
	UINT16      m_int_rom_size;       /* internal rom size */

	UINT8       m_rtemp;              /* temporary for import/export */

	typedef int (mcs48_cpu_device::*mcs48_ophandler)();
	static const mcs48_ophandler s_opcode_table[256];

	UINT8 opcode_fetch();
	UINT8 argument_fetch();
	void update_regptr();
	void push_pc_psw();
	void pull_pc_psw();
	void pull_pc();
	void execute_add(UINT8 dat);
	void execute_addc(UINT8 dat);
	void execute_jmp(UINT16 address);
	void execute_call(UINT16 address);
	void execute_jcc(UINT8 result);
	UINT8 p2_mask();
	void expander_operation(UINT8 operation, UINT8 port);
	int check_irqs();
	void burn_cycles(int count);

	int illegal();
	int add_a_r0();
	int add_a_r1();
	int add_a_r2();
	int add_a_r3();
	int add_a_r4();
	int add_a_r5();
	int add_a_r6();
	int add_a_r7();
	int add_a_xr0();
	int add_a_xr1();
	int add_a_n();
	int adc_a_r0();
	int adc_a_r1();
	int adc_a_r2();
	int adc_a_r3();
	int adc_a_r4();
	int adc_a_r5();
	int adc_a_r6();
	int adc_a_r7();
	int adc_a_xr0();
	int adc_a_xr1();
	int adc_a_n();
	int anl_a_r0();
	int anl_a_r1();
	int anl_a_r2();
	int anl_a_r3();
	int anl_a_r4();
	int anl_a_r5();
	int anl_a_r6();
	int anl_a_r7();
	int anl_a_xr0();
	int anl_a_xr1();
	int anl_a_n();
	int anl_bus_n();
	int anl_p1_n();
	int anl_p2_n();
	int anld_p4_a();
	int anld_p5_a();
	int anld_p6_a();
	int anld_p7_a();
	int call_0();
	int call_1();
	int call_2();
	int call_3();
	int call_4();
	int call_5();
	int call_6();
	int call_7();
	int clr_a();
	int clr_c();
	int clr_f0();
	int clr_f1();
	int cpl_a();
	int cpl_c();
	int cpl_f0();
	int cpl_f1();
	int da_a();
	int dec_a();
	int dec_r0();
	int dec_r1();
	int dec_r2();
	int dec_r3();
	int dec_r4();
	int dec_r5();
	int dec_r6();
	int dec_r7();
	int dis_i();
	int dis_tcnti();
	int djnz_r0();
	int djnz_r1();
	int djnz_r2();
	int djnz_r3();
	int djnz_r4();
	int djnz_r5();
	int djnz_r6();
	int djnz_r7();
	int en_i();
	int en_tcnti();
	int en_dma();
	int en_flags();
	int ent0_clk();
	int in_a_p1();
	int in_a_p2();
	int ins_a_bus();
	int in_a_dbb();
	int inc_a();
	int inc_r0();
	int inc_r1();
	int inc_r2();
	int inc_r3();
	int inc_r4();
	int inc_r5();
	int inc_r6();
	int inc_r7();
	int inc_xr0();
	int inc_xr1();
	int jb_0();
	int jb_1();
	int jb_2();
	int jb_3();
	int jb_4();
	int jb_5();
	int jb_6();
	int jb_7();
	int jc();
	int jf0();
	int jf1();
	int jnc();
	int jni();
	int jnibf();
	int jnt_0();
	int jnt_1();
	int jnz();
	int jobf();
	int jtf();
	int jt_0();
	int jt_1();
	int jz();
	int jmp_0();
	int jmp_1();
	int jmp_2();
	int jmp_3();
	int jmp_4();
	int jmp_5();
	int jmp_6();
	int jmp_7();
	int jmpp_xa();
	int mov_a_n();
	int mov_a_psw();
	int mov_a_r0();
	int mov_a_r1();
	int mov_a_r2();
	int mov_a_r3();
	int mov_a_r4();
	int mov_a_r5();
	int mov_a_r6();
	int mov_a_r7();
	int mov_a_xr0();
	int mov_a_xr1();
	int mov_a_t();
	int mov_psw_a();
	int mov_sts_a();
	int mov_r0_a();
	int mov_r1_a();
	int mov_r2_a();
	int mov_r3_a();
	int mov_r4_a();
	int mov_r5_a();
	int mov_r6_a();
	int mov_r7_a();
	int mov_r0_n();
	int mov_r1_n();
	int mov_r2_n();
	int mov_r3_n();
	int mov_r4_n();
	int mov_r5_n();
	int mov_r6_n();
	int mov_r7_n();
	int mov_t_a();
	int mov_xr0_a();
	int mov_xr1_a();
	int mov_xr0_n();
	int mov_xr1_n();
	int movd_a_p4();
	int movd_a_p5();
	int movd_a_p6();
	int movd_a_p7();
	int movd_p4_a();
	int movd_p5_a();
	int movd_p6_a();
	int movd_p7_a();
	int movp_a_xa();
	int movp3_a_xa();
	int movx_a_xr0();
	int movx_a_xr1();
	int movx_xr0_a();
	int movx_xr1_a();
	int nop();
	int orl_a_r0();
	int orl_a_r1();
	int orl_a_r2();
	int orl_a_r3();
	int orl_a_r4();
	int orl_a_r5();
	int orl_a_r6();
	int orl_a_r7();
	int orl_a_xr0();
	int orl_a_xr1();
	int orl_a_n();
	int orl_bus_n();
	int orl_p1_n();
	int orl_p2_n();
	int orld_p4_a();
	int orld_p5_a();
	int orld_p6_a();
	int orld_p7_a();
	int outl_bus_a();
	int outl_p1_a();
	int outl_p2_a();
	int out_dbb_a();
	int ret();
	int retr();
	int rl_a();
	int rlc_a();
	int rr_a();
	int rrc_a();
	int sel_mb0();
	int sel_mb1();
	int sel_rb0();
	int sel_rb1();
	int stop_tcnt();
	int strt_cnt();
	int strt_t();
	int swap_a();
	int xch_a_r0();
	int xch_a_r1();
	int xch_a_r2();
	int xch_a_r3();
	int xch_a_r4();
	int xch_a_r5();
	int xch_a_r6();
	int xch_a_r7();
	int xch_a_xr0();
	int xch_a_xr1();
	int xchd_a_xr0();
	int xchd_a_xr1();
	int xrl_a_r0();
	int xrl_a_r1();
	int xrl_a_r2();
	int xrl_a_r3();
	int xrl_a_r4();
	int xrl_a_r5();
	int xrl_a_r6();
	int xrl_a_r7();
	int xrl_a_xr0();
	int xrl_a_xr1();
	int xrl_a_n();
	int split_02();
	int split_08();
	int split_22();
	int split_75();
	int split_80();
	int split_81();
	int split_86();
	int split_88();
	int split_90();
	int split_91();
	int split_98();
	int split_d6();
	int split_e5();
	int split_f5();

};

class i8021_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8021_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 30 - 1) / 30; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 30); }
};

class i8022_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8022_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 30 - 1) / 30; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 30); }
};

class i8035_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8035_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8048_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8048_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8648_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8648_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8748_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8748_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8039_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8039_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8049_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8049_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8749_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8749_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8040_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8050_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mb8884_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	mb8884_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class n7751_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	n7751_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class m58715_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	m58715_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class upi41_cpu_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	upi41_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int rom_size, int ram_size);

	/* functions for talking to the input/output buffers on the UPI41-class chips */
	DECLARE_READ8_MEMBER(upi41_master_r);
	DECLARE_WRITE8_MEMBER(upi41_master_w);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	TIMER_CALLBACK_MEMBER( master_callback );
};

class i8041_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8041_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8741_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8741_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8042_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8242_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class i8742_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8742_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


#endif  /* __MCS48_H__ */
