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
	e0c6200_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, 16, 13, -1, program)
		, m_data_config("data", ENDIANNESS_BIG, 8, 12, 0, data), m_program(nullptr), m_data(nullptr), m_op(0), m_prev_op(0), m_irq_vector(0), m_irq_id(0), m_possible_irq(false), m_halt(false),
		m_sleep(false), m_icount(0), m_pc(0), m_prev_pc(0), m_npc(0), m_jpc(0), m_a(0), m_b(0), m_xp(0), m_xh(0), m_xl(0), m_yp(0), m_yh(0), m_yl(0), m_sp(0), m_f(0)
	{ }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 5; }
	virtual uint32_t execute_max_cycles() const override { return 14; } // longest opcode is 12 cycles, but interrupt service takes up to 14
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_one();
	virtual bool check_interrupt() { return false; } // nothing to do by default
	virtual void do_interrupt();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : nullptr); }

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 2; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	uint16_t m_op;
	uint16_t m_prev_op;
	uint8_t m_irq_vector;     // low 4 bits for new programcounter after interrupt
	int m_irq_id;           // for standard_irq_callback(id)
	bool m_possible_irq;    // indicates interrupts need to be re-checked
	bool m_halt;            // cpu halt state
	bool m_sleep;           // cpu sleep state
	int m_icount;

	uint16_t m_pc;            // 13-bit programcounter: 1-bit bank, 4-bit page, 8-bit 'step'
	uint16_t m_prev_pc;
	uint16_t m_npc;           // new bank/page prepared by pset
	uint16_t m_jpc;           // actual bank/page destination for jumps

	// all work registers are 4-bit
	uint8_t m_a;              // accumulator
	uint8_t m_b;              // generic
	uint8_t m_xp, m_xh, m_xl; // 12-bit index register when combined
	uint8_t m_yp, m_yh, m_yl; // "
	uint8_t m_sp;             // stackpointer (SPH, SPL)
	uint8_t m_f;              // flags
	enum
	{
		C_FLAG = 1,
		Z_FLAG = 2,
		D_FLAG = 4,
		I_FLAG = 8
	};

	// internal data memory read/write
	uint8_t read_mx();
	uint8_t read_my();
	uint8_t read_mn();
	void write_mx(uint8_t data);
	void write_my(uint8_t data);
	void write_mn(uint8_t data);

	// common stack ops
	void push(uint8_t data);
	uint8_t pop();
	void push_pc();
	void pop_pc();

	// misc internal helpers
	void set_cf(uint8_t data);
	void set_zf(uint8_t data);
	void inc_x();
	void inc_y();
	void do_branch(int condition = 1);

	// opcode handlers
	uint8_t op_inc(uint8_t x);
	uint8_t op_dec(uint8_t x);
	uint8_t op_add(uint8_t x, uint8_t y, int decimal = 0);
	uint8_t op_adc(uint8_t x, uint8_t y, int decimal = 0);
	uint8_t op_sub(uint8_t x, uint8_t y, int decimal = 0);
	uint8_t op_sbc(uint8_t x, uint8_t y, int decimal = 0);

	uint8_t op_and(uint8_t x, uint8_t y);
	uint8_t op_or(uint8_t x, uint8_t y);
	uint8_t op_xor(uint8_t x, uint8_t y);
	uint8_t op_rlc(uint8_t x);
	uint8_t op_rrc(uint8_t x);
};



#endif /* _E0C6200_H_ */
