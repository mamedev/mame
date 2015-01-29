// license:BSD-3-Clause
// copyright-holders:hap
/*

  AMI S2000-family MCU cores

*/

#ifndef _AMIS2000_H_
#define _AMIS2000_H_

#include "emu.h"


// generic input pins (4 bits each)
#define MCFG_AMI_S2000_READ_K_CB(_devcb) \
	amis2000_device::set_read_k_callback(*device, DEVCB_##_devcb);

#define MCFG_AMI_S2000_READ_I_CB(_devcb) \
	amis2000_device::set_read_i_callback(*device, DEVCB_##_devcb);

// 8-bit external databus coupled as input/output pins
#define MCFG_AMI_S2000_READ_D_CB(_devcb) \
	amis2000_device::set_read_d_callback(*device, DEVCB_##_devcb);

#define MCFG_AMI_S2000_WRITE_D_CB(_devcb) \
	amis2000_device::set_write_d_callback(*device, DEVCB_##_devcb);

// 13-bit external addressbus coupled as output pins
#define MCFG_AMI_S2000_WRITE_A_CB(_devcb) \
	amis2000_device::set_write_a_callback(*device, DEVCB_##_devcb);


class amis2000_device : public cpu_device
{
public:
	// construction/destruction
	amis2000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	amis2000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 bu_bits, UINT8 stack_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_k_callback(device_t &device, _Object object) { return downcast<amis2000_device &>(device).m_read_k.set_callback(object); }
	template<class _Object> static devcb_base &set_read_i_callback(device_t &device, _Object object) { return downcast<amis2000_device &>(device).m_read_i.set_callback(object); }
	template<class _Object> static devcb_base &set_read_d_callback(device_t &device, _Object object) { return downcast<amis2000_device &>(device).m_read_d.set_callback(object); }
	template<class _Object> static devcb_base &set_write_d_callback(device_t &device, _Object object) { return downcast<amis2000_device &>(device).m_write_d.set_callback(object); }
	template<class _Object> static devcb_base &set_write_a_callback(device_t &device, _Object object) { return downcast<amis2000_device &>(device).m_write_a.set_callback(object); }

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
	virtual UINT32 disasm_max_opcode_bytes() const { return 1; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	void state_string_export(const device_state_entry &entry, astring &string);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;
	
	UINT8 m_bu_bits;
	UINT16 m_bu_mask;
	UINT8 m_stack_bits;
	UINT16 m_stack_mask;
	UINT16 m_stack[3];

	UINT16 m_pc;
	UINT8 m_op;
	UINT8 m_f;
	UINT8 m_bl;
	UINT8 m_bu;
	UINT8 m_a;
	UINT8 m_e;

	devcb_read8 m_read_k;
	devcb_read8 m_read_i;
	devcb_read8 m_read_d;
	devcb_write8 m_write_d;
	devcb_write16 m_write_a;

	int m_icount;
};


class amis2150_device : public amis2000_device
{
public:
	amis2150_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type AMI_S2000;
extern const device_type AMI_S2150;


#endif /* _AMIS2000_H_ */
