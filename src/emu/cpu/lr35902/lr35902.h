#pragma once

#ifndef __LR35902_H__
#define __LR35902_H__


#define MCFG_LR35902_CONFIG(_config) \
	lr35902_cpu_device::static_set_config(*device, _config);

class lr35902_cpu_device;

// Perhaps replace this with a standard device callback
typedef void (*lr35902_timer_fired_func)(lr35902_cpu_device *device, int cycles);

struct lr35902_config
{
	const UINT16    *c_regs;
	UINT8           c_features;
	lr35902_timer_fired_func c_timer_expired_func;
};

enum
{
	LR35902_PC=1, LR35902_SP, LR35902_A, LR35902_F, LR35902_B, LR35902_C, LR35902_D, LR35902_E, LR35902_H, LR35902_L,
	LR35902_IRQ_STATE,
	/* Pseudo registers to keep track of the interrupt statuses */
	LR35902_IE, LR35902_IF,
	/* Pseudo register to change and check the cpu operating speed */
	LR35902_SPEED,
};

// This and the features configuration could be removed if we introduce proper subclasses
#define LR35902_FEATURE_HALT_BUG    0x01


class lr35902_cpu_device :  public cpu_device,
							public lr35902_config
{
public:
	// construction/destruction
	lr35902_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	static void static_set_config(device_t &device, const lr35902_config &config);

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
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : NULL; }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

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

protected:
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
	UINT8   m_IE;
	UINT8   m_IF;
	int m_irq_state;
	int m_ei_delay;
	lr35902_cpu_device *m_device;
	address_space *m_program;
	int m_icount;
	/* Timer stuff */
	lr35902_timer_fired_func m_timer_expired_func;
	/* Fetch & execute related */
	int     m_execution_state;
	UINT8   m_op;
	/* Others */
	int m_gb_speed;
	int m_gb_speed_change_pending;
	int m_enable;
	int m_doHALTbug;
	UINT8   m_features;
	const struct lr35902_config *m_config;

	/* Flag bit definitions */
	static const UINT8 FLAG_Z = 0x80;
	static const UINT8 FLAG_N = 0x40;
	static const UINT8 FLAG_H = 0x20;
	static const UINT8 FLAG_C = 0x10;
};

extern const device_type LR35902;

#endif /* __LR35902_H__ */
