// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

    mcs48.c

    Intel MCS-48/UPI-41 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator

***************************************************************************/

#ifndef MAME_CPU_MCS48_MCS48_H
#define MAME_CPU_MCS48_MCS48_H

#pragma once



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
	MCS48_P0,   // 8021/8022 only
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
	MCS48_STS,  // UPI-41 only
	MCS48_DBBO, // UPI-41 only
	MCS48_DBBI  // UPI-41 only
};


/* I/O port access indexes */
enum
{
	MCS48_INPUT_IRQ = 0,
	UPI41_INPUT_IBF = 0,
	MCS48_INPUT_EA
};


/***************************************************************************
    MACROS
***************************************************************************/

#define MCS48_LC_CLOCK(_L, _C) \
	(1 / (2 * 3.14159265358979323846 * sqrt(_L * _C)))

#define MCS48_ALE_CLOCK(_clock) \
	attotime::from_hz(_clock/(3*5))


/***************************************************************************
    TYPES
***************************************************************************/

/* Official Intel MCS-48 parts */
DECLARE_DEVICE_TYPE(I8021, i8021_device)    /* 1k internal ROM,      64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8022, i8022_device)    /* 2k internal ROM,     128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8035, i8035_device)    /* external ROM,         64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8048, i8048_device)    /* 1k internal ROM,      64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8648, i8648_device)    /* 1k internal OTP ROM,  64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8748, i8748_device)    /* 1k internal EEPROM,   64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8039, i8039_device)    /* external ROM,        128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8049, i8049_device)    /* 2k internal ROM,     128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8749, i8749_device)    /* 2k internal EEPROM,  128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8040, i8040_device)    /* external ROM,        256 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8050, i8050_device)    /* 4k internal ROM,     256 bytes internal RAM */

/* Official Intel UPI-41 parts */
DECLARE_DEVICE_TYPE(I8041A,  i8041a_device)   /* 1k internal ROM,      64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8741A,  i8741a_device)   /* 1k internal EEPROM,   64 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8041AH, i8041ah_device)  /* 1k internal ROM,     128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8741AH, i8741ah_device)  /* 1k internal EEPROM,  128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8042,   i8042_device)    /* 2k internal ROM,     128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8742,   i8742_device)    /* 2k internal EEPROM,  128 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8042AH, i8042ah_device)  /* 2k internal ROM,     256 bytes internal RAM */
DECLARE_DEVICE_TYPE(I8742AH, i8742ah_device)  /* 2k internal EEPROM,  256 bytes internal RAM */

/* Clones */
DECLARE_DEVICE_TYPE(MB8884, mb8884_device)  /* 8035 clone */
DECLARE_DEVICE_TYPE(N7751, n7751_device)    /* 8048 clone */
DECLARE_DEVICE_TYPE(M58715, m58715_device)  /* 8049 clone */



class mcs48_cpu_device : public cpu_device
{
public:
	// 8243 expander operations
	enum expander_op
	{
		EXPANDER_OP_READ = 0,
		EXPANDER_OP_WRITE = 1,
		EXPANDER_OP_OR = 2,
		EXPANDER_OP_AND = 3
	};

	// configuration
	auto p1_in_cb() { return m_port_in_cb[0].bind(); }
	auto p2_in_cb() { return m_port_in_cb[1].bind(); }
	auto p1_out_cb() { return m_port_out_cb[0].bind(); }
	auto p2_out_cb() { return m_port_out_cb[1].bind(); }
	auto bus_in_cb() { return m_bus_in_cb.bind(); }
	auto bus_out_cb() { return m_bus_out_cb.bind(); }
	auto t0_in_cb() { return m_test_in_cb[0].bind(); }
	auto t1_in_cb() { return m_test_in_cb[1].bind(); }

	// PROG line to 8243 expander
	auto prog_out_cb() { return m_prog_out_cb.bind(); }

	uint8_t p1_r() { return m_p1; }
	uint8_t p2_r() { return m_p2; }

	void data_6bit(address_map &map);
	void data_7bit(address_map &map);
	void data_8bit(address_map &map);
	void program_10bit(address_map &map);
	void program_11bit(address_map &map);
	void program_12bit(address_map &map);

	template <typename... T> void set_t0_clk_cb(T &&... args) { m_t0_clk_func.set(std::forward<T>(args)...); }

protected:
	typedef int (mcs48_cpu_device::*mcs48_ophandler)();

	// construction/destruction
	mcs48_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rom_size, int ram_size, uint8_t feature_mask, const mcs48_ophandler *opcode_table);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 15 - 1) / 15; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 15); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 3; }
	virtual uint32_t execute_input_lines() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

protected:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	devcb_read8   m_port_in_cb[2];
	devcb_write8  m_port_out_cb[2];
	devcb_read8   m_bus_in_cb;
	devcb_write8  m_bus_out_cb;

	devcb_read_line m_test_in_cb[2];
	clock_update_delegate m_t0_clk_func;
	devcb_write_line m_prog_out_cb;

	uint16_t      m_prevpc;             /* 16-bit previous program counter */
	uint16_t      m_pc;                 /* 16-bit program counter */

	uint8_t       m_a;                  /* 8-bit accumulator */
	uint8_t *     m_regptr;             /* pointer to r0-r7 */
	uint8_t       m_psw;                /* 8-bit psw */
	uint8_t       m_p1;                 /* 8-bit latched port 1 */
	uint8_t       m_p2;                 /* 8-bit latched port 2 */
	uint8_t       m_ea;                 /* 1-bit latched ea input */
	uint8_t       m_timer;              /* 8-bit timer */
	uint8_t       m_prescaler;          /* 5-bit timer prescaler */
	uint8_t       m_t1_history;         /* 8-bit history of the T1 input */
	uint8_t       m_sts;                /* 8-bit status register (UPI-41 only, except for F1) */
	uint8_t       m_dbbi;               /* 8-bit input data buffer (UPI-41 only) */
	uint8_t       m_dbbo;               /* 8-bit output data buffer (UPI-41 only) */

	bool          m_irq_state;          /* true if the IRQ line is active */
	bool          m_irq_polled;         /* true if last instruction was JNI (and not taken) */
	bool          m_irq_in_progress;    /* true if an IRQ is in progress */
	bool          m_timer_overflow;     /* true on a timer overflow; cleared by taking interrupt */
	bool          m_timer_flag;         /* true on a timer overflow; cleared on JTF */
	bool          m_tirq_enabled;       /* true if the timer IRQ is enabled */
	bool          m_xirq_enabled;       /* true if the external IRQ is enabled */
	uint8_t       m_timecount_enabled;  /* bitmask of timer/counter enabled */
	bool          m_flags_enabled;      /* true if I/O flags have been enabled (UPI-41 only) */
	bool          m_dma_enabled;        /* true if DMA has been enabled (UPI-41 only) */

	uint16_t      m_a11;                /* A11 value, either 0x000 or 0x800 */

	int         m_icount;

	/* Memory spaces */
	address_space *m_program;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_cache;
	address_space *m_data;
	address_space *m_io;

	required_shared_ptr<uint8_t> m_dataptr;

	uint8_t       m_feature_mask;       /* processor feature flags */
	uint16_t      m_int_rom_size;       /* internal rom size */

	uint8_t       m_rtemp;              /* temporary for import/export */

	static const mcs48_ophandler s_mcs48_opcodes[256];
	static const mcs48_ophandler s_upi41_opcodes[256];
	static const mcs48_ophandler s_i8021_opcodes[256];
	static const mcs48_ophandler s_i8022_opcodes[256];
	const mcs48_ophandler *const m_opcode_table;

	/* ROM is mapped to AS_PROGRAM */
	uint8_t program_r(offs_t a)         { return m_program->read_byte(a); }

	/* RAM is mapped to AS_DATA */
	uint8_t ram_r(offs_t a)             { return m_data->read_byte(a); }
	void    ram_w(offs_t a, uint8_t v)  { m_data->write_byte(a, v); }

	/* ports are mapped to AS_IO and callbacks */
	uint8_t ext_r(offs_t a)             { return m_io->read_byte(a); }
	void    ext_w(offs_t a, uint8_t v)  { m_io->write_byte(a, v); }
	uint8_t port_r(offs_t a)            { return m_port_in_cb[a - 1](); }
	void    port_w(offs_t a, uint8_t v) { m_port_out_cb[a - 1](v); }
	int     test_r(offs_t a)            { return m_test_in_cb[a](); }
	uint8_t bus_r()                     { return m_bus_in_cb(); }
	void    bus_w(uint8_t v)            { m_bus_out_cb(v); }
	void    prog_w(int v)               { m_prog_out_cb(v); }

	uint8_t opcode_fetch();
	uint8_t argument_fetch();
	void update_regptr();
	void push_pc_psw();
	void pull_pc_psw();
	void pull_pc();
	void execute_add(uint8_t dat);
	void execute_addc(uint8_t dat);
	void execute_jmp(uint16_t address);
	void execute_call(uint16_t address);
	void execute_jcc(uint8_t result);
	uint8_t p2_mask();
	void expander_operation(expander_op operation, uint8_t port);
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
	int in_a_p0();
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
	int outl_p0_a();
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
};

class i8021_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 30 - 1) / 30; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 30); }
};

class i8022_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 30 - 1) / 30; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 30); }
};

class i8035_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8035_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8048_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8048_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8648_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8648_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8748_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8748_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8039_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8039_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8049_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8049_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8749_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8749_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8040_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8050_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mb8884_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	mb8884_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class n7751_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	n7751_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class m58715_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	m58715_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class upi41_cpu_device : public mcs48_cpu_device
{
public:
	/* functions for talking to the input/output buffers on the UPI41-class chips */
	DECLARE_READ8_MEMBER(upi41_master_r);
	DECLARE_WRITE8_MEMBER(upi41_master_w);

protected:
	// construction/destruction
	upi41_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rom_size, int ram_size);

	TIMER_CALLBACK_MEMBER( master_callback );
};

class i8041a_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8041a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8741a_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8741a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8041ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8041ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8741ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8741ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8042_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8742_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8742_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8042ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8042ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class i8742ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8742ah_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif  // MAME_CPU_MCS48_MCS48_H
