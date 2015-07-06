// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family cores

*/

#ifndef _SM510_H_
#define _SM510_H_

#include "emu.h"


// I/O ports setup

// 4-bit K input port
#define MCFG_SM510_READ_K_CB(_devcb) \
	sm510_base_device::set_read_k_callback(*device, DEVCB_##_devcb);

// 8-bit S strobe output port
#define MCFG_SM510_WRITE_S_CB(_devcb) \
	sm510_base_device::set_write_s_callback(*device, DEVCB_##_devcb);



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
		, m_lcd_ram(*this, "lcd_ram")
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
		, m_stack_levels(stack_levels)
		, m_read_k(*this)
		, m_write_s(*this)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_k_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_read_k.set_callback(object); }
	template<class _Object> static devcb_base &set_write_s_callback(device_t &device, _Object object) { return downcast<sm510_base_device &>(device).m_write_s.set_callback(object); }

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
	//virtual void execute_set_input(int line, int state);
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
	required_shared_ptr<UINT8> m_lcd_ram;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;

	UINT16 m_pc, m_prev_pc;
	UINT8 m_op, m_prev_op;
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
	UINT16 m_div;
	bool m_bdc;
	bool m_cend;

	// i/o handlers
	devcb_read16 m_read_k;
	devcb_write8 m_write_s;

	// misc internal helpers
	void increment_pc();
	virtual void get_opcode_param() { } // -> child class

	UINT8 ram_r();
	void ram_w(UINT8 data);
	void pop_stack();
	void push_stack();
	void do_branch(UINT8 pu, UINT8 pm, UINT8 pl);
	UINT8 bitmask(UINT8 param);

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
	void op_wr();
	void op_ws();

	void op_kta();
	void op_atbp();
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
