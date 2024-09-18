// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS400 MCU family cores

*/

#ifndef MAME_CPU_HMCS400_HMCS400_H
#define MAME_CPU_HMCS400_HMCS400_H

#pragma once


class hmcs400_cpu_device : public cpu_device
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

protected:
	// construction
	hmcs400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + m_divider - 1) / m_divider; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * m_divider); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 3+2; } // max 3 + interrupt
	//virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// memory maps
	void program_map(address_map &map);
	void data_map(address_map &map);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

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

	u8 m_r[10];           // R outputs state
	u8 m_r_mask[10];
	u16 m_d;              // D pins state
	u16 m_d_mask;

	// I/O handlers
	devcb_read8::array<8> m_read_r;
	devcb_write8::array<8> m_write_r;
	devcb_read16 m_read_d;
	devcb_write16 m_write_d;

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
