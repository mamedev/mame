// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family cores

*/

#ifndef _SM510_H_
#define _SM510_H_

#include "emu.h"


// I/O ports setup

// 4-bit K input port (pull-down)
#define MCFG_SM510_READ_K_CB(_devcb) \
	sm510_base_device::set_read_k_callback(*device, DEVCB_##_devcb);
// when in halt state, any K input going High can wake up the CPU,
// driver is required to use execute_set_input(SM510_INPUT_LINE_K, state)
#define SM510_INPUT_LINE_K 0

// 1-bit BA input pin (pull-up)
#define MCFG_SM510_READ_BA_CB(_devcb) \
	sm510_base_device::set_read_ba_callback(*device, DEVCB_##_devcb);

// 1-bit B(beta) input pin (pull-up)
#define MCFG_SM510_READ_B_CB(_devcb) \
	sm510_base_device::set_read_b_callback(*device, DEVCB_##_devcb);

// 8-bit S strobe output port
#define MCFG_SM510_WRITE_S_CB(_devcb) \
	sm510_base_device::set_write_s_callback(*device, DEVCB_##_devcb);

// 2-bit R melody output port
#define MCFG_SM510_WRITE_R_CB(_devcb) \
	sm510_base_device::set_write_r_callback(*device, DEVCB_##_devcb);

// LCD segment outputs: H1-4 as offset(low), a/b/c 1-16 as data d0-d15
#define MCFG_SM510_WRITE_SEGA_CB(_devcb) \
	sm510_base_device::set_write_sega_callback(*device, DEVCB_##_devcb);
#define MCFG_SM510_WRITE_SEGB_CB(_devcb) \
	sm510_base_device::set_write_segb_callback(*device, DEVCB_##_devcb);
#define MCFG_SM510_WRITE_SEGC_CB(_devcb) \
	sm510_base_device::set_write_segc_callback(*device, DEVCB_##_devcb);

// LCD bs output: same as above, but only up to 2 bits used
#define MCFG_SM510_WRITE_SEGBS_CB(_devcb) \
	sm510_base_device::set_write_segbs_callback(*device, DEVCB_##_devcb);

enum
{
	SM510_PORT_SEGA = 0x00,
	SM510_PORT_SEGB = 0x04,
	SM510_PORT_SEGBS = 0x08,
	SM510_PORT_SEGC = 0x0c
};


// pinout reference

/*

*/

class sm510_base_device : public cpu_device
{
public:
	// construction/destruction
	sm510_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_LITTLE, 8, prgwidth, 0, program)
		, m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data)
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
		, m_stack_levels(stack_levels)
		, m_lcd_ram_a(*this, "lcd_ram_a"), m_lcd_ram_b(*this, "lcd_ram_b"), m_lcd_ram_c(*this, "lcd_ram_c")
		, m_write_sega(*this), m_write_segb(*this), m_write_segc(*this), m_write_segbs(*this)
		, m_melody_rom(*this, "music")
		, m_read_k(*this)
		, m_read_ba(*this), m_read_b(*this)
		, m_write_s(*this)
		, m_write_r(*this)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_k_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_read_k.set_callback(object); }
	template<class _Object> static devcb_base &set_read_ba_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_read_ba.set_callback(object); }
	template<class _Object> static devcb_base &set_read_b_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_read_b.set_callback(object); }
	template<class _Object> static devcb_base &set_write_s_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_s.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_r.set_callback(object); }

	template<class _Object> static devcb_base &set_write_sega_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_sega.set_callback(object); }
	template<class _Object> static devcb_base &set_write_segb_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_segb.set_callback(object); }
	template<class _Object> static devcb_base &set_write_segc_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_segc.set_callback(object); }
	template<class _Object> static devcb_base &set_write_segbs_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_segbs.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + 2 - 1) / 2; } // default 2 cycles per machine cycle
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * 2); } // "
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 2; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_set_input(int line, int state);
	virtual void execute_run();
	virtual void execute_one() { } // -> child class

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 0x40; } // actually 2, but debugger doesn't like non-linear pc

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;

	UINT16 m_pc, m_prev_pc;
	UINT16 m_op, m_prev_op;
	UINT8 m_param;
	int m_stack_levels;
	UINT16 m_stack[2];
	int m_icount;
	
	UINT8 m_acc;
	UINT8 m_bl;
	UINT8 m_bm;
	UINT8 m_c;
	bool m_skip;
	UINT8 m_w;
	UINT8 m_r;
	bool m_k_active;
	bool m_halt;

	// lcd driver
	optional_shared_ptr<UINT8> m_lcd_ram_a, m_lcd_ram_b, m_lcd_ram_c;
	devcb_write16 m_write_sega, m_write_segb, m_write_segc, m_write_segbs;
	emu_timer *m_lcd_timer;
	UINT8 m_l, m_x;
	UINT8 m_y;
	bool m_bp;
	bool m_bc;

	UINT16 get_lcd_row(int column, UINT8* ram);
	TIMER_CALLBACK_MEMBER(lcd_timer_cb);
	void init_lcd_driver();

	// melody controller
	optional_region_ptr<UINT8> m_melody_rom;
	UINT8 m_melody_rd;
	UINT8 m_melody_step_count;
	UINT8 m_melody_duty_count;
	UINT8 m_melody_duty_index;
	UINT8 m_melody_address;

	void clock_melody();
	void init_melody();

	// interrupt/divider
	emu_timer *m_div_timer;
	UINT16 m_div;
	bool m_1s;

	bool wake_me_up();
	void init_divider();
	TIMER_CALLBACK_MEMBER(div_timer_cb);
	
	// other i/o handlers
	devcb_read8 m_read_k;
	devcb_read_line m_read_ba;
	devcb_read_line m_read_b;
	devcb_write8 m_write_s;
	devcb_write8 m_write_r;

	// misc internal helpers
	void increment_pc();
	virtual void get_opcode_param() { }
	virtual void update_w_latch() { }

	UINT8 ram_r();
	void ram_w(UINT8 data);
	void pop_stack();
	void push_stack();
	void do_branch(UINT8 pu, UINT8 pm, UINT8 pl);
	UINT8 bitmask(UINT16 param);

	// opcode handlers
	void op_lb();
	void op_lbl();
	void op_sbm();
	void op_exbla();
	void op_incb();
	void op_decb();

	void op_atpl();
	void op_rtn0();
	void op_rtn1();
	void op_tl();
	void op_tml();
	void op_tm();
	void op_t();

	void op_exc();
	void op_bdc();
	void op_exci();
	void op_excd();
	void op_lda();
	void op_lax();
	void op_ptw();
	void op_wr();
	void op_ws();

	void op_kta();
	void op_atbp();
	void op_atx();
	void op_atl();
	void op_atfc();
	void op_atr();

	void op_add();
	void op_add11();
	void op_adx();
	void op_coma();
	void op_rot();
	void op_rc();
	void op_sc();

	void op_tb();
	void op_tc();
	void op_tam();
	void op_tmi();
	void op_ta0();
	void op_tabl();
	void op_tis();
	void op_tal();
	void op_tf1();
	void op_tf4();

	void op_rm();
	void op_sm();
	
	void op_pre();
	void op_sme();
	void op_rme();
	void op_tmel();
	
	void op_skip();
	void op_cend();
	void op_idiv();

	void op_illegal();
};


class sm510_device : public sm510_base_device
{
public:
	sm510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void execute_one();
	virtual void get_opcode_param();
	
	virtual void update_w_latch() { m_write_s(0, m_w, 0xff); } // W is connected directly to S
};


class sm511_device : public sm510_base_device
{
public:
	sm511_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	sm511_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void execute_one();
	virtual void get_opcode_param();
};

class sm512_device : public sm511_device
{
public:
	sm512_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type SM510;
extern const device_type SM511;
extern const device_type SM512;


#endif /* _SM510_H_ */
