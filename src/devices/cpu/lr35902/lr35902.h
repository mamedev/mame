// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#pragma once

#ifndef __LR35902_H__
#define __LR35902_H__


#define MCFG_LR35902_TIMER_CB(_devcb) \
	lr35902_cpu_device::set_timer_cb(*device, DEVCB_##_devcb);

// The first release of this CPU has a bug where the programcounter
// is not incremented properly after an interrupt after the halt opcode.
// This was fixed in a newer revision.
#define MCFG_LR35902_HALT_BUG \
	lr35902_cpu_device::set_halt_bug(*device);

// The GameBoy has a bug where OAM data gets corrupted if you inc/dec
// a 16-bit register in the $fe** region.
// note: oldval is in hiword, newval is in loword
#define MCFG_LR35902_INCDEC16_CB(_devcb) \
	lr35902_cpu_device::set_incdec16_cb(*device, DEVCB_##_devcb);


enum
{
	LR35902_PC=1, LR35902_SP, LR35902_A, LR35902_F, LR35902_B, LR35902_C, LR35902_D, LR35902_E, LR35902_H, LR35902_L,
	LR35902_IRQ_STATE,
	/* Pseudo registers to keep track of the interrupt statuses */
	LR35902_IE, LR35902_IF,
	/* Pseudo register to change and check the cpu operating speed */
	LR35902_SPEED
};


class lr35902_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	lr35902_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_timer_cb(device_t &device, _Object object) { return downcast<lr35902_cpu_device &>(device).m_timer_func.set_callback(object); }
	template<class _Object> static devcb_base &set_incdec16_cb(device_t &device, _Object object) { return downcast<lr35902_cpu_device &>(device).m_incdec16_func.set_callback(object); }
	static void set_halt_bug(device_t &device) { downcast<lr35902_cpu_device &>(device).m_has_halt_bug = true; }

	UINT8 get_speed();
	void set_speed( UINT8 speed_request );

	UINT8 get_ie() { return m_IE; }
	void set_ie( UINT8 data ) { m_IE = data; }

	UINT8 get_if() { return m_IF; }
	void set_if( UINT8 data ) { m_IF = data; }

protected:

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 16; }
	virtual UINT32 execute_input_lines() const { return 5; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	inline void cycles_passed(UINT8 cycles);
	inline UINT8 mem_read_byte(UINT16 addr);
	inline void mem_write_byte(UINT16 addr, UINT8 data);
	inline UINT16 mem_read_word(UINT16 addr);
	inline void mem_write_word(UINT16 addr, UINT16 data);
	inline void check_interrupts();

	address_space_config m_program_config;

	UINT8 m_A;
	UINT8 m_F;
	UINT8 m_B;
	UINT8 m_C;
	UINT8 m_D;
	UINT8 m_E;
	UINT8 m_H;
	UINT8 m_L;

	UINT16 m_SP;
	UINT16 m_PC;

	/* Interrupt related */
	UINT8 m_IE;
	UINT8 m_IF;
	int m_irq_state;
	bool m_handle_ei_delay;
	lr35902_cpu_device *m_device;
	address_space *m_program;
	int m_icount;

	/* Fetch & execute related */
	int m_execution_state;
	UINT8 m_op;

	/* Others */
	int m_gb_speed;
	int m_gb_speed_change_pending;
	int m_enable;
	bool m_handle_halt_bug;
	bool m_has_halt_bug;

	/* Callbacks */
	devcb_write8 m_timer_func;
	devcb_write32 m_incdec16_func;
};

extern const device_type LR35902;

#endif /* __LR35902_H__ */
