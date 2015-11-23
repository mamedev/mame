// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

*/

#ifndef _E0C6200_H_
#define _E0C6200_H_

#include "emu.h"


class e0c6200_cpu_device : public cpu_device
{
public:
	// construction/destruction
	e0c6200_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, address_map_constructor program, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, 16, 13, -1, program)
		, m_data_config("data", ENDIANNESS_BIG, 8, 12, 0, data), m_program(nullptr), m_data(nullptr), m_op(0), m_prev_op(0), m_irq_vector(0), m_irq_id(0), m_possible_irq(false), m_halt(false), 
		m_sleep(false), m_icount(0), m_pc(0), m_prev_pc(0), m_npc(0), m_jpc(0), m_a(0), m_b(0), m_xp(0), m_xh(0), m_xl(0), m_yp(0), m_yh(0), m_yl(0), m_sp(0), m_f(0)
	{ }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 14; } // longest opcode is 12 cycles, but interrupt service takes up to 14
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();
	virtual void execute_one();
	virtual bool check_interrupt() { return false; } // nothing to do by default
	virtual void do_interrupt();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	UINT16 m_op;
	UINT16 m_prev_op;
	UINT8 m_irq_vector;     // low 4 bits for new programcounter after interrupt
	int m_irq_id;           // for standard_irq_callback(id)
	bool m_possible_irq;    // indicates interrupts need to be re-checked
	bool m_halt;            // cpu halt state
	bool m_sleep;           // cpu sleep state
	int m_icount;

	UINT16 m_pc;            // 13-bit programcounter: 1-bit bank, 4-bit page, 8-bit 'step'
	UINT16 m_prev_pc;
	UINT16 m_npc;           // new bank/page prepared by pset
	UINT16 m_jpc;           // actual bank/page destination for jumps

	// all work registers are 4-bit
	UINT8 m_a;              // accumulator
	UINT8 m_b;              // generic
	UINT8 m_xp, m_xh, m_xl; // 12-bit index register when combined
	UINT8 m_yp, m_yh, m_yl; // "
	UINT8 m_sp;             // stackpointer (SPH, SPL)
	UINT8 m_f;              // flags

	// internal data memory read/write
	inline UINT8 read_mx();
	inline UINT8 read_my();
	inline UINT8 read_mn();
	inline void write_mx(UINT8 data);
	inline void write_my(UINT8 data);
	inline void write_mn(UINT8 data);

	// common stack ops
	inline void push(UINT8 data);
	inline UINT8 pop();
	inline void push_pc();
	inline void pop_pc();

	// misc internal helpers
	inline void set_cf(UINT8 data);
	inline void set_zf(UINT8 data);
	inline void inc_x();
	inline void inc_y();
	void do_branch(int condition = 1);

	// opcode handlers
	UINT8 op_inc(UINT8 x);
	UINT8 op_dec(UINT8 x);
	UINT8 op_add(UINT8 x, UINT8 y, int decimal = 0);
	UINT8 op_adc(UINT8 x, UINT8 y, int decimal = 0);
	UINT8 op_sub(UINT8 x, UINT8 y, int decimal = 0);
	UINT8 op_sbc(UINT8 x, UINT8 y, int decimal = 0);

	UINT8 op_and(UINT8 x, UINT8 y);
	UINT8 op_or(UINT8 x, UINT8 y);
	UINT8 op_xor(UINT8 x, UINT8 y);
	UINT8 op_rlc(UINT8 x);
	UINT8 op_rrc(UINT8 x);
};



#endif /* _E0C6200_H_ */
