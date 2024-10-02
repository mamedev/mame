// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

*/

#ifndef MAME_CPU_E0C6200_E0C6200_H
#define MAME_CPU_E0C6200_E0C6200_H


class e0c6200_cpu_device : public cpu_device
{
public:
	// construction/destruction
	e0c6200_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program, address_map_constructor data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 5; }
	virtual u32 execute_max_cycles() const noexcept override { return 14; } // longest opcode is 12 cycles, but interrupt service takes up to 14
	virtual void execute_run() override;
	virtual void execute_one();
	virtual bool check_interrupt() { return false; } // nothing to do by default
	virtual void do_interrupt();

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	u16 m_op;
	u16 m_prev_op;
	u8 m_irq_vector;     // low 4 bits for new programcounter after interrupt
	int m_irq_id;        // for standard_irq_callback(id)
	bool m_possible_irq; // indicates interrupts need to be re-checked
	bool m_halt;         // cpu halt state
	bool m_sleep;        // cpu sleep state
	int m_icount;

	u16 m_pc;            // 13-bit programcounter: 1-bit bank, 4-bit page, 8-bit 'step'
	u16 m_prev_pc;
	u16 m_npc;           // new bank/page prepared by pset
	u16 m_jpc;           // actual bank/page destination for jumps

	// all work registers are 4-bit
	u8 m_a;              // accumulator
	u8 m_b;              // generic
	u8 m_xp, m_xh, m_xl; // 12-bit index register when combined
	u8 m_yp, m_yh, m_yl; // "
	u8 m_sp;             // stackpointer (SPH, SPL)
	u8 m_f;              // flags
	enum
	{
		C_FLAG = 1,
		Z_FLAG = 2,
		D_FLAG = 4,
		I_FLAG = 8
	};

	// internal data memory read/write
	u8 read_mx();
	u8 read_my();
	u8 read_mn();
	void write_mx(u8 data);
	void write_my(u8 data);
	void write_mn(u8 data);

	// common stack ops
	void push(u8 data);
	u8 pop();
	void push_pc();
	void pop_pc();

	// misc internal helpers
	void set_cf(u8 data);
	void set_zf(u8 data);
	void inc_x();
	void inc_y();
	void do_branch(int condition = 1);

	// opcode handlers
	u8 op_inc(u8 x);
	u8 op_dec(u8 x);
	u8 op_add(u8 x, u8 y, int decimal = 0);
	u8 op_adc(u8 x, u8 y, int decimal = 0);
	u8 op_sub(u8 x, u8 y, int decimal = 0);
	u8 op_sbc(u8 x, u8 y, int decimal = 0);

	u8 op_and(u8 x, u8 y);
	u8 op_or(u8 x, u8 y);
	u8 op_xor(u8 x, u8 y);
	u8 op_rlc(u8 x);
	u8 op_rrc(u8 x);
};

#endif // MAME_CPU_E0C6200_E0C6200_H
