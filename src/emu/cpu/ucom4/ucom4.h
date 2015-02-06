// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family cores

*/

#ifndef _UCOM4_H_
#define _UCOM4_H_

#include "emu.h"


// I/O ports setup
#define MCFG_UCOM4_READ_A_CB(_devcb) \
	ucom4_cpu_device::set_read_a_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_READ_B_CB(_devcb) \
	ucom4_cpu_device::set_read_b_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_READ_C_CB(_devcb) \
	ucom4_cpu_device::set_read_c_callback(*device, DEVCB_##_devcb);
#define MCFG_UCOM4_WRITE_C_CB(_devcb) \
	ucom4_cpu_device::set_write_c_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_READ_D_CB(_devcb) \
	ucom4_cpu_device::set_read_d_callback(*device, DEVCB_##_devcb);
#define MCFG_UCOM4_WRITE_D_CB(_devcb) \
	ucom4_cpu_device::set_write_d_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_WRITE_E_CB(_devcb) \
	ucom4_cpu_device::set_write_e_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_WRITE_F_CB(_devcb) \
	ucom4_cpu_device::set_write_f_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_WRITE_G_CB(_devcb) \
	ucom4_cpu_device::set_write_g_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_WRITE_H_CB(_devcb) \
	ucom4_cpu_device::set_write_h_callback(*device, DEVCB_##_devcb);

#define MCFG_UCOM4_WRITE_I_CB(_devcb) \
	ucom4_cpu_device::set_write_i_callback(*device, DEVCB_##_devcb);



class ucom4_cpu_device : public cpu_device
{
public:
	// construction/destruction
	ucom4_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, 8, prgwidth, 0, program)
		, m_data_config("data", ENDIANNESS_BIG, 8, datawidth, 0, data)
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
		, m_stack_levels(stack_levels)
		, m_read_a(*this)
		, m_read_b(*this)
		, m_read_c(*this)
		, m_read_d(*this)
		, m_write_c(*this)
		, m_write_d(*this)
		, m_write_e(*this)
		, m_write_f(*this)
		, m_write_g(*this)
		, m_write_h(*this)
		, m_write_i(*this)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_a_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_read_a.set_callback(object); }
	template<class _Object> static devcb_base &set_read_b_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_read_b.set_callback(object); }
	template<class _Object> static devcb_base &set_read_c_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_read_c.set_callback(object); }
	template<class _Object> static devcb_base &set_read_d_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_read_d.set_callback(object); }

	template<class _Object> static devcb_base &set_write_c_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_c.set_callback(object); }
	template<class _Object> static devcb_base &set_write_d_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_d.set_callback(object); }
	template<class _Object> static devcb_base &set_write_e_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_e.set_callback(object); }
	template<class _Object> static devcb_base &set_write_f_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_f.set_callback(object); }
	template<class _Object> static devcb_base &set_write_g_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_g.set_callback(object); }
	template<class _Object> static devcb_base &set_write_h_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_h.set_callback(object); }
	template<class _Object> static devcb_base &set_write_i_callback(device_t &device, _Object object) { return downcast<ucom4_cpu_device &>(device).m_write_i.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 2; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config :((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	void state_string_export(const device_state_entry &entry, astring &string);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;
	int m_stack_levels; // number of callstack levels
	UINT16 m_stack[3];  // max 3
	
	UINT8 m_op;
	UINT8 m_prev_op;
	int m_icount;
	
	UINT16 m_pc;        // program counter
	UINT8 m_acc;        // 4-bit accumulator
	UINT8 m_dpl;        // 4-bit data pointer low (RAM x)
	UINT8 m_dph;        // 1/2/3-bit data pointer high (RAM y)
	UINT8 m_carry_f;    // carry flag
	UINT8 m_timer_f;    // timer out flag
	UINT8 m_int_f;      // interrupt flag
	UINT8 m_inte_f;     // interrupt enable flag

	// i/o handlers
	devcb_read8 m_read_a;
	devcb_read8 m_read_b;
	devcb_read8 m_read_c;
	devcb_read8 m_read_d;

	devcb_write8 m_write_c;
	devcb_write8 m_write_d;
	devcb_write8 m_write_e;
	devcb_write8 m_write_f;
	devcb_write8 m_write_g;
	devcb_write8 m_write_h;
	devcb_write8 m_write_i;
};


class upd553_cpu_device : public ucom4_cpu_device
{
public:
	upd553_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class upd650_cpu_device : public ucom4_cpu_device
{
public:
	upd650_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class upd552_cpu_device : public ucom4_cpu_device
{
public:
	upd552_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type NEC_D553;
extern const device_type NEC_D650;
extern const device_type NEC_D552;


#endif /* _UCOM4_H_ */
