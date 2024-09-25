// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS400 MCU family cores

*/

#ifndef MAME_CPU_HMCS400_HMCS400_H
#define MAME_CPU_HMCS400_HMCS400_H

#pragma once


// input lines

enum
{
	HMCS400_INPUT_LINE_INT0 = 0,
	HMCS400_INPUT_LINE_INT1
};


// pinout reference

/*
                _________________
        D11  1 |*                | 64 D10
        D12  2 |                 | 63 D9
        D13  3 |                 | 62 D8
        D14  4 |                 | 61 D7
        D15  5 |                 | 60 D6
        R00  6 |                 | 59 D5
        R01  7 |                 | 58 D4
        R02  8 |                 | 57 D3
        R03  9 |                 | 56 D2
        R10 10 |                 | 55 D1
        R11 11 |                 | 54 D0
        R12 12 |                 | 53 GND
        R13 13 |                 | 52 OSC2
        R20 14 |    HD61402x     | 51 OSC1
        R21 15 |    HD61404x     | 50 _TEST
        R22 16 |    HD61408x     | 49 RESET
        R23 17 |                 | 48 R93
        RA0 18 |     DP-64S      | 47 R92
  RA1/Vdisp 19 |     DC-64S      | 46 R91
        R30 20 |                 | 45 R90
        R31 21 |                 | 44 R83
  R32/_INT0 22 |                 | 43 R82
  R33/_INT1 23 |                 | 42 R81
        R50 24 |                 | 41 R80
        R51 25 |                 | 40 R73
        R52 26 |                 | 39 R72
        R53 27 |                 | 38 R71
        R60 28 |                 | 37 R70
        R61 29 |                 | 36 R43
        R62 30 |                 | 35 R42/SO
        R63 31 |                 | 34 R41/SI
        Vcc 32 |_________________| 33 R40/_SCK

        (see datasheets for FP-64 pinouts)

*/


class hmcs400_cpu_device : public cpu_device, public device_nvram_interface
{
public:
	virtual ~hmcs400_cpu_device();

	// configuration helpers

	// 10 4-bit R ports (port A is 2-bit)
	template <std::size_t N> auto read_r() { return m_read_r[N].bind(); }
	template <std::size_t N> auto write_r() { return m_write_r[N].bind(); }

	// 16-bit discrete
	auto read_d() { return m_read_d.bind(); }
	auto write_d() { return m_write_d.bind(); }

	// system clock divider mask option (only for HMCS408, HMCS414, HMCS424)
	// valid options: 4, 8, 16, default to 8
	auto &set_divider(u8 div) { assert(m_has_div); m_divider = div; return *this; }

	// nvram
	void nvram_set_battery(int state) { m_nvram_battery = bool(state); } // default is 1 (nvram_enable_backup needs to be true)
	void nvram_set_default_value(u8 val) { m_nvram_defval = val & 0xf; } // default is 0
	auto stop_cb() { return m_stop_cb.bind(); } // notifier (not an output pin)
	int stop_mode() { return m_stop ? 1 : 0; }

protected:
	// construction
	hmcs400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + m_divider - 1) / m_divider; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * m_divider); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 3+2; } // max 3 + interrupt
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// memory maps
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	required_shared_ptr_array<u8, 2> m_ram;
	u8 m_nvram_defval;
	bool m_nvram_battery; // keeps internal RAM intact after soft power-off

	int m_icount;
	int m_state_count;

	const u32 m_rom_size; // ROM size in 16-bit words
	const u32 m_ram_size; // RAM size minus the 64-byte stack
	bool m_has_div;       // MCU supports divider mask option
	u8 m_divider;         // system clock divider

	u16 m_pc;             // program counter
	u16 m_prev_pc;
	u16 m_sp;             // stack pointer
	u16 m_op;             // current opcode
	u16 m_param;          // 2-byte opcode param or RAM address
	u8 m_i;               // 4-bit immediate opcode param

	u8 m_a;               // 4-bit accumulator
	u8 m_b;               // 4-bit B register
	u8 m_w;               // 2-bit W register
	u8 m_x;               // 4-bit X register
	u8 m_spx;             // 4-bit SPX register
	u8 m_y;               // 4-bit Y register
	u8 m_spy;             // 4-bit SPY register
	u8 m_st;              // status flag
	u8 m_ca;              // carry flag
	bool m_standby;       // standby mode (SBY opcode)
	bool m_stop;          // stop mode (STOP opcode)

	u8 m_r[11];           // R outputs state
	u8 m_r_mask[11];
	u16 m_d;              // D pins state
	u16 m_d_mask;

	u8 m_int_line[2];     // INT0/INT1 pin state
	u16 m_irq_flags;      // interrupt control bits
	u8 m_pmr;             // port mode register
	u16 m_prescaler;      // 11-bit clock prescaler
	u8 m_timer_mode[2];   // TMA/TMB: timer mode registers
	u16 m_timer_div[2];   // timer prescaler divide ratio masks from TMA/TMB
	u8 m_timer_count[2];  // TCA/TCA: timer counters
	u8 m_timer_load;      // timer B reload register
	u8 m_timer_b_low;     // timer B counter low latch

	// I/O handlers
	devcb_read8::array<11> m_read_r;
	devcb_write8::array<11> m_write_r;
	devcb_read16 m_read_d;
	devcb_write16 m_write_d;
	devcb_write_line m_stop_cb;

	// misc internal helpers
	u8 ram_r(u8 mem_mask = 0xf);
	void ram_w(u8 data, u8 mem_mask = 0xf);
	void pop_stack();
	void push_stack();

	void reset_io();
	u8 read_r(u8 index);
	void write_r(u8 index, u8 data);
	int read_d(u8 index);
	void write_d(u8 index, int state);

	bool access_mode(u8 mem_mask, bool bit_mode = false);
	u8 irq_control_r(offs_t offset, u8 mem_mask);
	void irq_control_w(offs_t offset, u8 data, u8 mem_mask);
	void pmr_w(offs_t offset, u8 data, u8 mem_mask);
	void tm_w(offs_t offset, u8 data, u8 mem_mask);
	void tlrl_w(offs_t offset, u8 data, u8 mem_mask);
	void tlru_w(offs_t offset, u8 data, u8 mem_mask);
	u8 tcbl_r(offs_t offset, u8 mem_mask);
	u8 tcbu_r(offs_t offset, u8 mem_mask);

	void ext_int_edge(int line);
	void take_interrupt(int irq);
	void check_interrupts();
	void clock_timer(int timer);
	void clock_prescaler();
	void cycle();
	u16 fetch();

	// opcode handlers
	void op_illegal();
	void op_todo();

	void op_lai();
	void op_lbi();
	void op_lmi();
	void op_lmiiy();

	void op_lab();
	void op_lba();
	void op_law();
	void op_lay();
	void op_laspx();
	void op_laspy();
	void op_lamr();
	void op_xmra();

	void op_lwi();
	void op_lxi();
	void op_lyi();
	void op_lwa();
	void op_lxa();
	void op_lya();
	void op_iy();
	void op_dy();
	void op_ayy();
	void op_syy();
	void op_xsp();

	void op_lam();
	void op_lbm();
	void op_lma();
	void op_lmaiy();
	void op_lmady();
	void op_xma();
	void op_xmb();

	void op_ai();
	void op_ib();
	void op_db();
	void op_daa();
	void op_das();
	void op_nega();
	void op_comb();
	void op_rotr();
	void op_rotl();
	void op_sec();
	void op_rec();
	void op_tc();
	void op_am();
	void op_amc();
	void op_smc();
	void op_or();
	void op_anm();
	void op_orm();
	void op_eorm();

	void op_inem();
	void op_anem();
	void op_bnem();
	void op_ynei();
	void op_ilem();
	void op_alem();
	void op_blem();
	void op_alei();

	void op_sem();
	void op_rem();
	void op_tm();

	void op_br();
	void op_brl();
	void op_jmpl();
	void op_cal();
	void op_call();
	void op_tbr();
	void op_rtn();
	void op_rtni();

	void op_sed();
	void op_sedd();
	void op_red();
	void op_redd();
	void op_td();
	void op_tdd();
	void op_lar();
	void op_lbr();
	void op_lra();
	void op_lrb();
	void op_p();

	void op_sts();
	void op_sby();
	void op_stop();
};


class hmcs402_cpu_device : public hmcs400_cpu_device
{
protected:
	hmcs402_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class hd614022_device : public hmcs402_cpu_device
{
public:
	hd614022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614023_device : public hmcs402_cpu_device
{
public:
	hd614023_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614025_device : public hmcs402_cpu_device
{
public:
	hd614025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614026_device : public hmcs402_cpu_device
{
public:
	hd614026_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614028_device : public hmcs402_cpu_device
{
public:
	hd614028_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614029_device : public hmcs402_cpu_device
{
public:
	hd614029_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs404_cpu_device : public hmcs400_cpu_device
{
protected:
	hmcs404_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class hd614042_device : public hmcs404_cpu_device
{
public:
	hd614042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614043_device : public hmcs404_cpu_device
{
public:
	hd614043_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614045_device : public hmcs404_cpu_device
{
public:
	hd614045_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614046_device : public hmcs404_cpu_device
{
public:
	hd614046_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614048_device : public hmcs404_cpu_device
{
public:
	hd614048_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614049_device : public hmcs404_cpu_device
{
public:
	hd614049_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs408_cpu_device : public hmcs400_cpu_device
{
protected:
	hmcs408_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

class hd614080_device : public hmcs408_cpu_device
{
public:
	hd614080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614081_device : public hmcs408_cpu_device
{
public:
	hd614081_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614085_device : public hmcs408_cpu_device
{
public:
	hd614085_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614086_device : public hmcs408_cpu_device
{
public:
	hd614086_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614088_device : public hmcs408_cpu_device
{
public:
	hd614088_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd614089_device : public hmcs408_cpu_device
{
public:
	hd614089_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(HD614022, hd614022_device)
DECLARE_DEVICE_TYPE(HD614023, hd614023_device)
DECLARE_DEVICE_TYPE(HD614025, hd614025_device)
DECLARE_DEVICE_TYPE(HD614026, hd614026_device)
DECLARE_DEVICE_TYPE(HD614028, hd614028_device)
DECLARE_DEVICE_TYPE(HD614029, hd614029_device)

DECLARE_DEVICE_TYPE(HD614042, hd614042_device)
DECLARE_DEVICE_TYPE(HD614043, hd614043_device)
DECLARE_DEVICE_TYPE(HD614045, hd614045_device)
DECLARE_DEVICE_TYPE(HD614046, hd614046_device)
DECLARE_DEVICE_TYPE(HD614048, hd614048_device)
DECLARE_DEVICE_TYPE(HD614049, hd614049_device)

DECLARE_DEVICE_TYPE(HD614080, hd614080_device)
DECLARE_DEVICE_TYPE(HD614081, hd614081_device)
DECLARE_DEVICE_TYPE(HD614085, hd614085_device)
DECLARE_DEVICE_TYPE(HD614086, hd614086_device)
DECLARE_DEVICE_TYPE(HD614088, hd614088_device)
DECLARE_DEVICE_TYPE(HD614089, hd614089_device)

#endif // MAME_CPU_HMCS400_HMCS400_H
