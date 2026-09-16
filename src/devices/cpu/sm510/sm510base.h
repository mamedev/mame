// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family - base/shared

  Don't include this file, include the specific device header instead,
  for example sm510.h

*/

#ifndef MAME_CPU_SM510_SM510BASE_H
#define MAME_CPU_SM510_SM510BASE_H

#pragma once

// I/O ports setup

// when in halt state, any active K input can wake up the CPU,
// driver is required to use set_input_line(SM510_EXT_WAKEUP_LINE, state)
#define SM510_EXT_WAKEUP_LINE 0

// ACL input pin
#define SM510_INPUT_LINE_ACL INPUT_LINE_RESET

// LCD commons
enum
{
	SM510_PORT_SEGA = 0x00,
	SM510_PORT_SEGB = 0x04,
	SM510_PORT_SEGBS = 0x08,
	SM510_PORT_SEGC = 0x0c
};


class sm510_base_device : public cpu_device
{
public:
	// For SM510, SM500, SM5A, R port output is selected with a mask option,
	// either from the divider or direct contol. Documented options are:
	// SM510/SM5A: direct control, 2(4096Hz meant for alarm sound)
	// SM500: 14, 11, 3 (divider f1, f4, f12)
	void set_r_mask_option(int bit) { m_r_mask_option = bit; }
	static constexpr int RMASK_DIRECT = -1;

	// 4/8-bit K input port (pull-down)
	auto read_k() { return m_read_k.bind(); }

	// 1-bit BA(aka alpha) input pin (pull-up)
	auto read_ba() { return m_read_ba.bind(); }

	// 1-bit B(beta) input pin (pull-up)
	auto read_b() { return m_read_b.bind(); }

	// 4/8-bit S strobe output port
	auto write_s() { return m_write_s.bind(); }

	// 1/2-bit R (buzzer/melody) output port
	// may also be called F(frequency?) or SO(sound out)
	// SM590 has 4 R ports, don't use this one, see sm590.h
	auto write_r() { return m_write_r.bind(); }

	// LCD segment outputs, SM51x: H1-4 as offset(low), a/b/c 1-16 as data d0-d15,
	// bs output is same as above, but only up to 2 bits used.
	// SM500/SM5A/SM530: H1/2 as a0, O group as a1-a4, O data as d0-d3
	auto write_segs() { return m_write_segs.bind(); }

protected:
	// construction/destruction
	sm510_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + m_clk_div - 1) / m_clk_div; } // default 2 cycles per machine cycle
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * m_clk_div); } // "
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 3+1; }
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	virtual void execute_one() = 0;
	virtual bool op_argument() { return false; }

	virtual void reset_vector() { do_branch(3, 7, 0); }
	virtual void wakeup_vector() { do_branch(1, 0, 0); } // after halt

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;
	int m_pagemask;

	int m_icount;
	int m_state_count;

	u16 m_pc, m_prev_pc;
	u16 m_op, m_prev_op;
	u8 m_param;
	int m_stack_levels;
	u16 m_stack[4]; // max 4

	u8 m_acc;
	u8 m_bl;
	u8 m_bm;
	u8 m_bmask;
	u8 m_c;
	bool m_skip;
	u8 m_w;
	u8 m_r;
	u8 m_r_out;
	int m_r_mask_option;
	bool m_ext_wakeup;
	bool m_halt;
	int m_clk_div;

	// lcd driver
	optional_shared_ptr<u8> m_lcd_ram_a, m_lcd_ram_b, m_lcd_ram_c;
	devcb_write16 m_write_segs;
	emu_timer *m_lcd_timer;
	u8 m_l;
	u8 m_x;
	u8 m_y;
	u8 m_bp;
	bool m_bc;

	u16 get_lcd_row(int column, u8* ram);
	virtual void lcd_update();
	TIMER_CALLBACK_MEMBER(lcd_timer_cb);
	virtual void init_lcd_driver();

	// melody controller
	optional_region_ptr<u8> m_melody_rom;
	u8 m_melody_rd;
	u8 m_melody_step_count;
	u8 m_melody_duty_count;
	u8 m_melody_duty_index;
	u8 m_melody_address;

	virtual void clock_melody() { }
	virtual void init_melody() { }

	// divider
	emu_timer *m_div_timer;
	u16 m_div;
	u8 m_gamma;

	virtual void init_divider();
	virtual TIMER_CALLBACK_MEMBER(div_timer_cb);

	// other i/o handlers
	devcb_read8 m_read_k;
	devcb_read_line m_read_ba;
	devcb_read_line m_read_b;
	devcb_write8 m_write_s;
	devcb_write8 m_write_r;

	// misc internal helpers
	virtual void increment_pc();
	virtual void update_w_latch() { }
	void do_interrupt();

	virtual u8 ram_r();
	virtual void ram_w(u8 data);
	void pop_stack();
	void push_stack();
	virtual void do_branch(u8 pu, u8 pm, u8 pl);
	u8 bitmask(u16 param);

	// opcode handlers
	virtual void op_lb();
	virtual void op_lbl();
	virtual void op_sbm();
	virtual void op_exbla();
	virtual void op_incb();
	virtual void op_decb();

	virtual void op_atpl();
	virtual void op_rtn0();
	virtual void op_rtn1();
	virtual void op_tl();
	virtual void op_tml();
	virtual void op_tm();
	virtual void op_t();

	virtual void op_exc();
	virtual void op_bdc();
	virtual void op_exci();
	virtual void op_excd();
	virtual void op_lda();
	virtual void op_lax();
	virtual void op_ptw();
	virtual void op_wr();
	virtual void op_ws();

	virtual void op_kta();
	virtual void op_atbp();
	virtual void op_atx();
	virtual void op_atl();
	virtual void op_atfc();
	virtual void op_atr();

	virtual void op_add();
	virtual void op_add11();
	virtual void op_adx();
	virtual void op_coma();
	virtual void op_rot();
	virtual void op_rc();
	virtual void op_sc();

	virtual void op_tb();
	virtual void op_tc();
	virtual void op_tam();
	virtual void op_tmi();
	virtual void op_ta0();
	virtual void op_tabl();
	virtual void op_tis();
	virtual void op_tal();
	virtual void op_tf1();
	virtual void op_tf4();

	virtual void op_rm();
	virtual void op_sm();

	virtual void op_pre();
	virtual void op_sme();
	virtual void op_rme();
	virtual void op_tmel();

	virtual void op_skip();
	virtual void op_cend();
	virtual void op_idiv();
	virtual void op_dta();
	virtual void op_clklo();
	virtual void op_clkhi();

	void op_illegal();
};

#endif // MAME_CPU_SM510_SM510BASE_H
