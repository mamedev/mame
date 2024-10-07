// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family cores

*/

#ifndef MAME_CPU_HMCS40_HMCS40_H
#define MAME_CPU_HMCS40_HMCS40_H

#pragma once


// input lines

enum
{
	HMCS40_INPUT_LINE_INT0 = 0,
	HMCS40_INPUT_LINE_INT1,
	HMCS40_INPUT_LINE_HLT
};


// pinout reference

/*
            _________________
    D3   1 |*                | 42 D2
    D4   2 |                 | 41 D1
    D5   3 |                 | 40 D0
    D6   4 |                 | 39 R33
    D7   5 |                 | 38 R32
    D8   6 |                 | 37 R31
    D9   7 |                 | 36 R30
    D10  8 |                 | 35 R23          .......................................
    D11  9 |                 | 34 R22         :
    D12 10 |     HD38750     | 33 R21         :
    D13 11 |     HD38800     | 32 R20         :
    D14 12 |                 | 31 INT1        :
    D15 13 |                 | 30 INT0        :                     _________________
  Vdisp 14 |                 | 29 R13         :             D4   1 |*                | 64 D3
  RESET 15 |                 | 28 R12         :             D5   2 |                 | 63 D2
    Vbb 16 |                 | 27 R11         :             D6   3 |                 | 62 D1
    Vdd 17 |                 | 26 R10         :             D7   4 |                 | 61 D0
    OSC 18 |                 | 25 R03         :             D8   5 |                 | 60 R63
   <NC> 19 |                 | 24 R02         :             D9   6 |                 | 59 R62
  /TEST 20 |                 | 23 R01         :             <NC> 7 |                 | 58 <NC>
    Vss 21 |_________________| 22 R00         :             <NC> 8 |                 | 57 <NC>
                                                            <NC> 9 |                 | 56 <NC>
                                                            D10 10 |                 | 55 R61
            D8 D7 D6 D5 D4    <NC>D3 D2 D1 D0               D11 11 |                 | 54 R60
             5  4  3  2  1     54 53 52 51 50               D12 12 |                 | 53 R33
            __________________________________              D13 13 |                 | 52 R32
           /                                  |             D14 14 |                 | 51 R31
    D9   6 |                                  | 49 R63      D15 15 |                 | 50 R30
    D10  7 |                                  | 48 R62      R40 16 |                 | 49 R23
    D11  8 |                                  | 47 R61      R41 17 |                 | 48 R22
    D12  9 |                                  | 46 R60      R42 18 |                 | 47 R21
    D13 10 |                                  | 45 R33      R43 19 |                 | 46 R20
    D14 11 |                                  | 44 R32      R50 20 |                 | 45 INT1
    D15 12 |                                  | 43 R31      R51 21 |                 | 44 INT0
    R40 13 |             HD38820              | 42 R30      R52 22 |    HD38820      | 43 R13
    R41 14 |             (FP-54 pkg)          | 41 R23      R53 23 |    (DP-64S pkg) | 42 R12
    R42 15 |                                  | 40 R22    Vdisp 24 |                 | 41 <NC>
    R43 16 |                                  | 39 R21     <NC> 25 |                 | 40 <NC>
    R50 17 |                                  | 38 R20    RESET 26 |                 | 39 <NC>
    R51 18 |                                  | 37 INT1     Vbb 27 |                 | 38 R11
    R52 19 |                                  | 36 INT0     Vdd 28 |                 | 37 R10
    R53 20 |                                  | 35 R13      OSC 29 |                 | 36 R03
  Vdisp 21 |                                  | 34 R12     <NC> 30 |                 | 35 R02
  RESET 22 |                                  | 33 R11    /TEST 31 |                 | 34 R01
           |__________________________________|             Vss 32 |_________________| 33 R00

            23 24 25 26 27     28 29 30 31 32
            Vbb | OSC | Vss    R00 | R02 | R10
               Vdd   /TEST        R01   R03
*/


class hmcs40_cpu_device : public cpu_device
{
public:
	virtual ~hmcs40_cpu_device();

	// max 8 4-bit R ports
	template <std::size_t N> auto read_r() { return m_read_r[N].bind(); }
	template <std::size_t N> auto write_r() { return m_write_r[N].bind(); }

	// 16-bit discrete
	auto read_d() { return m_read_d.bind(); }
	auto write_d() { return m_write_d.bind(); }

protected:
	enum
	{
		HMCS42_FAMILY = 0,
		HMCS43_FAMILY,
		HMCS44_FAMILY,
		HMCS45_FAMILY,
		HMCS46_FAMILY,
		HMCS47_FAMILY
	};

	// construction
	hmcs40_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int family, u16 polarity, int stack_levels, int pcwidth, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; } // 4 cycles per machine cycle
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); } // "
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2+1; } // max 2 + interrupt
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// memory maps
	void program_1k(address_map &map) ATTR_COLD;
	void program_2k(address_map &map) ATTR_COLD;
	void data_80x4(address_map &map) ATTR_COLD;
	void data_160x4(address_map &map) ATTR_COLD;
	void data_256x4(address_map &map) ATTR_COLD;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_icount;
	int m_state_count;

	const int m_pcwidth;      // program counter bit-width
	const int m_prgwidth;
	const int m_datawidth;
	const int m_family;       // MCU family (42-47)
	const u16 m_polarity;     // i/o polarity (pmos vs cmos)
	const int m_stack_levels; // number of callstack levels
	int m_pcmask;
	int m_prgmask;
	int m_datamask;

	u16 m_stack[4];           // max 4
	u16 m_op;                 // current opcode
	u16 m_prev_op;
	u8 m_i;                   // 4-bit immediate opcode param
	int m_eint_line;          // which input_line caused an interrupt
	int m_halt;               // internal HLT state
	u8 m_prescaler;           // internal timer prescaler
	bool m_block_int;         // block interrupt on next cycle

	u16 m_pc;                 // program counter
	u16 m_prev_pc;
	u8 m_pc_upper;            // LPU prepared upper bits of PC
	u8 m_a;                   // 4-bit accumulator
	u8 m_b;                   // 4-bit B register
	u8 m_x;                   // 1/3/4-bit X register
	u8 m_spx;                 // 1/3/4-bit SPX register
	u8 m_y;                   // 4-bit Y register
	u8 m_spy;                 // 4-bit SPY register
	u8 m_s;                   // status F/F (F/F = flip-flop)
	u8 m_c;                   // carry F/F

	u8 m_tc;                  // timer/counter
	u8 m_cf;                  // CF F/F (timer mode or counter mode)
	u8 m_ie;                  // I/E (interrupt enable) F/F
	u8 m_iri;                 // external interrupt pending I/RI F/F
	u8 m_irt;                 // timer interrupt pending I/RT F/F
	u8 m_if[2];               // external interrupt mask IF0,1 F/F
	u8 m_tf;                  // timer interrupt mask TF F/F
	u8 m_int[2];              // INT0/1 pins state
	u8 m_r[8];                // R outputs state
	u16 m_d;                  // D pins state

	// I/O handlers
	devcb_read8::array<8> m_read_r;
	devcb_write8::array<8> m_write_r;
	devcb_read16 m_read_d;
	devcb_write16 m_write_d;

	// misc internal helpers
	void increment_pc();
	void cycle();

	u8 ram_r();
	void ram_w(u8 data);
	void pop_stack();
	void push_stack();

	virtual void reset_io();
	virtual u8 read_r(u8 index);
	virtual void write_r(u8 index, u8 data);
	virtual int read_d(u8 index);
	virtual void write_d(u8 index, int state);

	void take_interrupt();
	void clock_timer();
	void clock_prescaler();

	// opcode handlers
	void op_illegal();

	void op_lab();
	void op_lba();
	void op_lay();
	void op_laspx();
	void op_laspy();
	void op_xamr();

	void op_lxa();
	void op_lya();
	void op_lxi();
	void op_lyi();
	void op_iy();
	void op_dy();
	void op_ayy();
	void op_syy();
	void op_xsp();

	void op_lam();
	void op_lbm();
	void op_xma();
	void op_xmb();
	void op_lmaiy();
	void op_lmady();

	void op_lmiiy();
	void op_lai();
	void op_lbi();

	void op_ai();
	void op_ib();
	void op_db();
	void op_amc();
	void op_smc();
	void op_am();
	void op_daa();
	void op_das();
	void op_nega();
	void op_comb();
	void op_sec();
	void op_rec();
	void op_tc();
	void op_rotl();
	void op_rotr();
	void op_or();

	void op_mnei();
	void op_ynei();
	void op_anem();
	void op_bnem();
	void op_alei();
	void op_alem();
	void op_blem();

	void op_sem();
	void op_rem();
	void op_tm();

	void op_br();
	void op_cal();
	void op_lpu();
	void op_tbr();
	void op_rtn();

	void op_seie();
	void op_seif0();
	void op_seif1();
	void op_setf();
	void op_secf();
	void op_reie();
	void op_reif0();
	void op_reif1();
	void op_retf();
	void op_recf();
	void op_ti0();
	void op_ti1();
	void op_tif0();
	void op_tif1();
	void op_ttf();
	void op_lti();
	void op_lta();
	void op_lat();
	void op_rtni();

	void op_sed();
	void op_red();
	void op_td();
	void op_sedd();
	void op_redd();
	void op_lar();
	void op_lbr();
	void op_lra();
	void op_lrb();
	void op_p();
};


class hmcs43_cpu_device : public hmcs40_cpu_device
{
protected:
	hmcs43_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity);

	// overrides
	virtual u8 read_r(u8 index) override;
	virtual void write_r(u8 index, u8 data) override;
	virtual int read_d(u8 index) override;
};

class hd38750_device : public hmcs43_cpu_device
{
public:
	hd38750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd38755_device : public hmcs43_cpu_device
{
public:
	hd38755_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44750_device : public hmcs43_cpu_device
{
public:
	hd44750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44758_device : public hmcs43_cpu_device
{
public:
	hd44758_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs44_cpu_device : public hmcs40_cpu_device
{
protected:
	hmcs44_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity);

	// overrides
	virtual u8 read_r(u8 index) override;
	virtual void write_r(u8 index, u8 data) override;
};

class hd38800_device : public hmcs44_cpu_device
{
public:
	hd38800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd38805_device : public hmcs44_cpu_device
{
public:
	hd38805_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44801_device : public hmcs44_cpu_device
{
public:
	hd44801_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44808_device : public hmcs44_cpu_device
{
public:
	hd44808_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs45_cpu_device : public hmcs40_cpu_device
{
protected:
	hmcs45_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity);

	// overrides
	virtual u8 read_r(u8 index) override;
	virtual void write_r(u8 index, u8 data) override;
};

class hd38820_device : public hmcs45_cpu_device
{
public:
	hd38820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd38825_device : public hmcs45_cpu_device
{
public:
	hd38825_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44820_device : public hmcs45_cpu_device
{
public:
	hd44820_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44828_device : public hmcs45_cpu_device
{
public:
	hd44828_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs46_cpu_device : public hmcs40_cpu_device
{
protected:
	hmcs46_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity);

	// overrides
	virtual u8 read_r(u8 index) override;
	virtual void write_r(u8 index, u8 data) override;
};

class hd44840_device : public hmcs46_cpu_device
{
public:
	hd44840_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44848_device : public hmcs46_cpu_device
{
public:
	hd44848_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class hmcs47_cpu_device : public hmcs40_cpu_device
{
protected:
	hmcs47_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 polarity);

	// overrides
	virtual u8 read_r(u8 index) override;
};

class hd38870_device : public hmcs47_cpu_device
{
public:
	hd38870_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44860_device : public hmcs47_cpu_device
{
public:
	hd44860_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd44868_device : public hmcs47_cpu_device
{
public:
	hd44868_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(HD38750, hd38750_device)
DECLARE_DEVICE_TYPE(HD38755, hd38755_device)
DECLARE_DEVICE_TYPE(HD44750, hd44750_device)
DECLARE_DEVICE_TYPE(HD44758, hd44758_device)

DECLARE_DEVICE_TYPE(HD38800, hd38800_device)
DECLARE_DEVICE_TYPE(HD38805, hd38805_device)
DECLARE_DEVICE_TYPE(HD44801, hd44801_device)
DECLARE_DEVICE_TYPE(HD44808, hd44808_device)

DECLARE_DEVICE_TYPE(HD38820, hd38820_device)
DECLARE_DEVICE_TYPE(HD38825, hd38825_device)
DECLARE_DEVICE_TYPE(HD44820, hd44820_device)
DECLARE_DEVICE_TYPE(HD44828, hd44828_device)

DECLARE_DEVICE_TYPE(HD44840, hd44840_device)
DECLARE_DEVICE_TYPE(HD44848, hd44848_device)

DECLARE_DEVICE_TYPE(HD38870, hd38870_device)
DECLARE_DEVICE_TYPE(HD44860, hd44860_device)
DECLARE_DEVICE_TYPE(HD44868, hd44868_device)


#endif // MAME_CPU_HMCS40_HMCS40_H
