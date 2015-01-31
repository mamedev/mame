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
	amis2000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 bu_bits, UINT8 callstack_bits, UINT8 callstack_depth, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

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
	UINT8 m_callstack_bits;     // number of program counter bits held in callstack
	UINT16 m_callstack_mask;
	UINT8 m_callstack_depth;    // callstack levels: 3 on 2000/2150, 5 on 2200/2400
	UINT16 m_callstack[5];      // max 5

	UINT16 m_pc;                // 13-bit program counter
	UINT8 m_ppr;                // prepared page register (PP 1)
	UINT8 m_pbr;                // prepared bank register (PP 2)
	bool m_skip;                // skip next opcode, including PP prefixes
	UINT8 m_op;
	UINT8 m_prev_op;            // previous opcode, needed for PP, LAI, LB*
	UINT8 m_f;                  // generic flags: 2 on 2000/2150, 6 on 2200/2400
	UINT8 m_carry;              // carry flag
	UINT8 m_bl;                 // 4-bit ram index x
	UINT8 m_bu;                 // 2/3-bit ram index y
	UINT8 m_acc;                // 4-bit accumulator
	UINT8 m_e;                  // 4-bit generic register
	UINT8 m_i;                  // 4-bit i-pins latch
	UINT8 m_k;                  // 4-bit k-pins latch
	UINT8 m_d;                  // 8-bit d-pins latch
	UINT16 m_a;                 // 13-bit a-pins latch (master strobe latch)

	devcb_read8 m_read_k;
	devcb_read8 m_read_i;
	devcb_read8 m_read_d;
	devcb_write8 m_write_d;
	devcb_write16 m_write_a;

	int m_icount;
	
	UINT8 ram_r();
	void ram_w(UINT8 data);
	void pop_callstack();
	void push_callstack();
	void op_illegal();
	
	void op_lai();
	void op_lab();
	void op_lae();
	void op_xab();
	void op_xabu();
	void op_xae();
	void op_lbe();
	void op_lbep();
	void op_lbz();
	void op_lbf();

	void op_lam();
	void op_xc();
	void op_xci();
	void op_xcd();
	void op_stm();
	void op_rsm();

	void op_inp();
	void op_out();
	void op_disb();
	void op_disn();
	void op_mvs();
	void op_psh();
	void op_psl();
	void op_eur();

	void op_pp();
	void op_jmp();
	void op_jms();
	void op_rt();
	void op_rts();
	void op_nop();

	void op_szc();
	void op_szm();
	void op_szi();
	void op_szk();
	void op_sbe();
	void op_sam();
	void op_sos();
	void op_tf1();
	void op_tf2();

	void op_adcs();
	void op_adis();
	void op_add();
	void op_and();
	void op_xor();
	void op_stc();
	void op_rsc();
	void op_cma();
	void op_sf1();
	void op_rf1();
	void op_sf2();
	void op_rf2();
};


class amis2150_device : public amis2000_device
{
public:
	amis2150_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type AMI_S2000;
extern const device_type AMI_S2150;


#endif /* _AMIS2000_H_ */
