// license:BSD-3-Clause
// copyright-holders:Dan Boris, Mirko Buffoni, Aaron Giles, Couriersud
/***************************************************************************

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

// register access indexes
enum
{
	MCS48_PC,
	MCS48_PSW,
	MCS48_SP,
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


// I/O port access indexes
enum
{
	MCS48_INPUT_IRQ = 0,
	MCS48_INPUT_EA
};


/***************************************************************************
    MACROS
***************************************************************************/

#define MCS48_LC_CLOCK(_L, _C) \
	(1 / (2 * M_PI * sqrt(_L * _C)))

#define MCS48_ALE_CLOCK(_clock) \
	attotime::from_hz(_clock/(3*5))


/***************************************************************************
    TYPES
***************************************************************************/

// Official Intel MCS-48 parts
DECLARE_DEVICE_TYPE(I8021,   i8021_device)    // 1k internal ROM,       64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8022,   i8022_device)    // 2k internal ROM,      128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8035,   i8035_device)    // external ROM,          64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8048,   i8048_device)    // 1k internal ROM,       64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8648,   i8648_device)    // 1k internal OTP ROM,   64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8748,   i8748_device)    // 1k internal UV EPROM,  64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8039,   i8039_device)    // external ROM,         128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8049,   i8049_device)    // 2k internal ROM,      128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8749,   i8749_device)    // 2k internal UV EPROM, 128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8040,   i8040_device)    // external ROM,         256 bytes internal RAM
DECLARE_DEVICE_TYPE(I8050,   i8050_device)    // 4k internal ROM,      256 bytes internal RAM

// Official Intel UPI-41 parts
DECLARE_DEVICE_TYPE(I8041A,  i8041a_device)   // 1k internal ROM,       64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8741A,  i8741a_device)   // 1k internal UV EPROM,  64 bytes internal RAM
DECLARE_DEVICE_TYPE(I8041AH, i8041ah_device)  // 1k internal ROM,      128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8741AH, i8741ah_device)  // 1k internal UV EPROM, 128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8042,   i8042_device)    // 2k internal ROM,      128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8742,   i8742_device)    // 2k internal UV EPROM, 128 bytes internal RAM
DECLARE_DEVICE_TYPE(I8042AH, i8042ah_device)  // 2k internal ROM,      256 bytes internal RAM
DECLARE_DEVICE_TYPE(I8742AH, i8742ah_device)  // 2k internal UV EPROM, 256 bytes internal RAM

// Clones
DECLARE_DEVICE_TYPE(MB8884,  mb8884_device)   // 8035 clone
DECLARE_DEVICE_TYPE(UPD7751, upd7751_device)  // 8048 clone
DECLARE_DEVICE_TYPE(M58715,  m58715_device)   // 8049 clone


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

	u8 p1_r() { return m_p1; }
	u8 p2_r() { return m_p2; }

	template <typename... T> void set_t0_clk_cb(T &&... args) { m_t0_clk_func.set(std::forward<T>(args)...); }

	u32 get_ale_clock() { return m_clock / 3 / 5; }
	u32 get_t0_clock() { return m_clock / 3; }

protected:
	typedef void (mcs48_cpu_device::*mcs48_ophandler)();

	// construction/destruction
	mcs48_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int rom_size, int ram_size, u8 feature_mask, const mcs48_ophandler *opcode_table);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { update_regptr(); }

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 15 - 1) / 15; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 15); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2+2; } // opcode+irq
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

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;
	memory_view m_rom_view;

	devcb_read8::array<2> m_port_in_cb;
	devcb_write8::array<2> m_port_out_cb;
	devcb_read8 m_bus_in_cb;
	devcb_write8 m_bus_out_cb;

	devcb_read_line::array<2> m_test_in_cb;
	clock_update_delegate m_t0_clk_func;
	devcb_write_line m_prog_out_cb;

	u16 m_prevpc;           // 16-bit previous program counter
	u16 m_pc;               // 16-bit program counter

	u8 m_a;                 // 8-bit accumulator
	u8 *m_regptr;           // pointer to r0-r7
	u8 m_psw;               // 8-bit PSW
	bool m_f1;              // F1 flag (F0 is in PSW)
	u16 m_a11;              // A11 value, either 0x000 or 0x800
	u8 m_p1;                // 8-bit latched port 1
	u8 m_p2;                // 8-bit latched port 2
	u8 m_ea;                // 1-bit latched ea input
	u8 m_timer;             // 8-bit timer
	u8 m_prescaler;         // 5-bit timer prescaler
	u8 m_t1_history;        // 8-bit history of the T1 input
	u8 m_sts;               // 4-bit status register + OBF/IBF flags (UPI-41 only)
	u8 m_dbbi;              // 8-bit input data buffer (UPI-41 only)
	u8 m_dbbo;              // 8-bit output data buffer (UPI-41 only)

	bool m_irq_state;       // true if the IRQ line is active
	bool m_irq_polled;      // true if last instruction was JNI (and not taken)
	bool m_irq_in_progress; // true if an IRQ is in progress
	bool m_timer_overflow;  // true on a timer overflow; cleared by taking interrupt
	bool m_timer_flag;      // true on a timer overflow; cleared on JTF
	bool m_tirq_enabled;    // true if the timer IRQ is enabled
	bool m_xirq_enabled;    // true if the external IRQ is enabled
	u8 m_timecount_enabled; // bitmask of timer/counter enabled
	bool m_flags_enabled;   // true if I/O flags have been enabled (UPI-41 only)
	bool m_dma_enabled;     // true if DMA has been enabled (UPI-41 only)

	int  m_icount;

	// Memory spaces
	memory_access<12, 0, 0, ENDIANNESS_LITTLE>::cache m_program;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<8, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	required_shared_ptr<u8> m_dataptr;

	u8 m_feature_mask;      // processor feature flags
	u16 m_rom_size;         // internal rom size
	u16 m_ram_size;         // internal ram size

	u8 m_rtemp;             // temporary for import/export

	static const mcs48_ophandler s_mcs48_opcodes[256];
	static const mcs48_ophandler s_upi41_opcodes[256];
	static const mcs48_ophandler s_i8021_opcodes[256];
	static const mcs48_ophandler s_i8022_opcodes[256];
	const mcs48_ophandler *const m_opcode_table;

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	// ROM is mapped to AS_PROGRAM
	u8 program_r(offs_t a)      { return m_program.read_byte(a); }

	// RAM is mapped to AS_DATA
	u8 ram_r(offs_t a)          { return m_data.read_byte(a); }
	void ram_w(offs_t a, u8 v)  { m_data.write_byte(a, v); }

	// ports are mapped to AS_IO and callbacks
	u8 ext_r(offs_t a)          { return m_io.read_byte(a); }
	void ext_w(offs_t a, u8 v)  { m_io.write_byte(a, v); }
	u8 port_r(offs_t a)         { return m_port_in_cb[a - 1](); }
	void port_w(offs_t a, u8 v) { m_port_out_cb[a - 1](v); }
	int test_r(offs_t a)        { return m_test_in_cb[a](); }
	u8 bus_r()                  { return m_bus_in_cb(); }
	void bus_w(u8 v)            { m_bus_out_cb(v); }
	void prog_w(int v)          { m_prog_out_cb(v); }

	u8 opcode_fetch();
	u8 argument_fetch();
	void update_regptr();
	void update_ea();
	void push_pc_psw();
	void pull_pc_psw();
	void pull_pc();
	void execute_add(u8 dat);
	void execute_addc(u8 dat);
	void execute_jmp(u16 address);
	void execute_call(u16 address);
	void execute_jcc(bool result);
	u8 p2_mask();
	void expander_operation(expander_op operation, u8 port);
	void check_irqs();
	void burn_cycles(int count);

	void illegal();
	void add_a_r0();
	void add_a_r1();
	void add_a_r2();
	void add_a_r3();
	void add_a_r4();
	void add_a_r5();
	void add_a_r6();
	void add_a_r7();
	void add_a_xr0();
	void add_a_xr1();
	void add_a_n();
	void adc_a_r0();
	void adc_a_r1();
	void adc_a_r2();
	void adc_a_r3();
	void adc_a_r4();
	void adc_a_r5();
	void adc_a_r6();
	void adc_a_r7();
	void adc_a_xr0();
	void adc_a_xr1();
	void adc_a_n();
	void anl_a_r0();
	void anl_a_r1();
	void anl_a_r2();
	void anl_a_r3();
	void anl_a_r4();
	void anl_a_r5();
	void anl_a_r6();
	void anl_a_r7();
	void anl_a_xr0();
	void anl_a_xr1();
	void anl_a_n();
	void anl_bus_n();
	void anl_p1_n();
	void anl_p2_n();
	void anld_p4_a();
	void anld_p5_a();
	void anld_p6_a();
	void anld_p7_a();
	void call_0();
	void call_1();
	void call_2();
	void call_3();
	void call_4();
	void call_5();
	void call_6();
	void call_7();
	void clr_a();
	void clr_c();
	void clr_f0();
	void clr_f1();
	void cpl_a();
	void cpl_c();
	void cpl_f0();
	void cpl_f1();
	void da_a();
	void dec_a();
	void dec_r0();
	void dec_r1();
	void dec_r2();
	void dec_r3();
	void dec_r4();
	void dec_r5();
	void dec_r6();
	void dec_r7();
	void dis_i();
	void dis_tcnti();
	void djnz_r0();
	void djnz_r1();
	void djnz_r2();
	void djnz_r3();
	void djnz_r4();
	void djnz_r5();
	void djnz_r6();
	void djnz_r7();
	void en_i();
	void en_tcnti();
	void en_dma();
	void en_flags();
	void ent0_clk();
	void in_a_p0();
	void in_a_p1();
	void in_a_p2();
	void ins_a_bus();
	void in_a_dbb();
	void inc_a();
	void inc_r0();
	void inc_r1();
	void inc_r2();
	void inc_r3();
	void inc_r4();
	void inc_r5();
	void inc_r6();
	void inc_r7();
	void inc_xr0();
	void inc_xr1();
	void jb_0();
	void jb_1();
	void jb_2();
	void jb_3();
	void jb_4();
	void jb_5();
	void jb_6();
	void jb_7();
	void jc();
	void jf0();
	void jf1();
	void jnc();
	void jni();
	void jnibf();
	void jnt_0();
	void jnt_1();
	void jnz();
	void jobf();
	void jtf();
	void jt_0();
	void jt_1();
	void jz();
	void jmp_0();
	void jmp_1();
	void jmp_2();
	void jmp_3();
	void jmp_4();
	void jmp_5();
	void jmp_6();
	void jmp_7();
	void jmpp_xa();
	void mov_a_n();
	void mov_a_psw();
	void mov_a_r0();
	void mov_a_r1();
	void mov_a_r2();
	void mov_a_r3();
	void mov_a_r4();
	void mov_a_r5();
	void mov_a_r6();
	void mov_a_r7();
	void mov_a_xr0();
	void mov_a_xr1();
	void mov_a_t();
	void mov_psw_a();
	void mov_sts_a();
	void mov_r0_a();
	void mov_r1_a();
	void mov_r2_a();
	void mov_r3_a();
	void mov_r4_a();
	void mov_r5_a();
	void mov_r6_a();
	void mov_r7_a();
	void mov_r0_n();
	void mov_r1_n();
	void mov_r2_n();
	void mov_r3_n();
	void mov_r4_n();
	void mov_r5_n();
	void mov_r6_n();
	void mov_r7_n();
	void mov_t_a();
	void mov_xr0_a();
	void mov_xr1_a();
	void mov_xr0_n();
	void mov_xr1_n();
	void movd_a_p4();
	void movd_a_p5();
	void movd_a_p6();
	void movd_a_p7();
	void movd_p4_a();
	void movd_p5_a();
	void movd_p6_a();
	void movd_p7_a();
	void movp_a_xa();
	void movp3_a_xa();
	void movx_a_xr0();
	void movx_a_xr1();
	void movx_xr0_a();
	void movx_xr1_a();
	void nop();
	void orl_a_r0();
	void orl_a_r1();
	void orl_a_r2();
	void orl_a_r3();
	void orl_a_r4();
	void orl_a_r5();
	void orl_a_r6();
	void orl_a_r7();
	void orl_a_xr0();
	void orl_a_xr1();
	void orl_a_n();
	void orl_bus_n();
	void orl_p1_n();
	void orl_p2_n();
	void orld_p4_a();
	void orld_p5_a();
	void orld_p6_a();
	void orld_p7_a();
	void outl_bus_a();
	void outl_p0_a();
	void outl_p1_a();
	void outl_p2_a();
	void out_dbb_a();
	void ret();
	void retr();
	void rl_a();
	void rlc_a();
	void rr_a();
	void rrc_a();
	void sel_mb0();
	void sel_mb1();
	void sel_rb0();
	void sel_rb1();
	void stop_tcnt();
	void strt_cnt();
	void strt_t();
	void swap_a();
	void xch_a_r0();
	void xch_a_r1();
	void xch_a_r2();
	void xch_a_r3();
	void xch_a_r4();
	void xch_a_r5();
	void xch_a_r6();
	void xch_a_r7();
	void xch_a_xr0();
	void xch_a_xr1();
	void xchd_a_xr0();
	void xchd_a_xr1();
	void xrl_a_r0();
	void xrl_a_r1();
	void xrl_a_r2();
	void xrl_a_r3();
	void xrl_a_r4();
	void xrl_a_r5();
	void xrl_a_r6();
	void xrl_a_r7();
	void xrl_a_xr0();
	void xrl_a_xr1();
	void xrl_a_n();
};

class i8021_device : public mcs48_cpu_device
{
public:
	// configuration
	auto p0_in_cb() { return bus_in_cb(); }
	auto p0_out_cb() { return bus_out_cb(); }

	// construction/destruction
	i8021_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 30 - 1) / 30; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 30); }
};

class i8022_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 30 - 1) / 30; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 30); }
};

class i8035_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8035_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8048_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8048_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8648_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8648_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8748_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8748_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8039_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8039_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8049_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8049_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8749_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8749_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8040_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8050_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	i8050_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mb8884_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	mb8884_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class upd7751_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	upd7751_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class m58715_device : public mcs48_cpu_device
{
public:
	// construction/destruction
	m58715_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class upi41_cpu_device : public mcs48_cpu_device
{
public:
	// functions for talking to the input/output buffers on the UPI41-class chips
	u8 upi41_master_r(offs_t offset);
	void upi41_master_w(offs_t offset, u8 data);

protected:
	// construction/destruction
	upi41_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int rom_size, int ram_size);

	TIMER_CALLBACK_MEMBER(master_callback);
};

class i8041a_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8041a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8741a_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8741a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8041ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8041ah_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8741ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8741ah_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8042_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8742_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8742_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8042ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8042ah_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class i8742ah_device : public upi41_cpu_device
{
public:
	// construction/destruction
	i8742ah_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


#endif // MAME_CPU_MCS48_MCS48_H
