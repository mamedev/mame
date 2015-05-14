// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    MAME conversion by R. Belmont

*/

#pragma once

#ifndef MN10200_H
#define MN10200_H

enum
{
	MN10200_PC = 0,
	MN10200_PSW,
	MN10200_MDR,
	MN10200_D0,
	MN10200_D1,
	MN10200_D2,
	MN10200_D3,
	MN10200_A0,
	MN10200_A1,
	MN10200_A2,
	MN10200_A3,
	MN10200_NMICR,
	MN10200_IAGR
};

enum
{
	MN10200_PORT0 = 0,
	MN10200_PORT1,
	MN10200_PORT2,
	MN10200_PORT3,
	MN10200_PORT4
};

enum
{
	MN10200_IRQ0 = 0,
	MN10200_IRQ1,
	MN10200_IRQ2,
	MN10200_IRQ3,

	MN10200_MAX_EXT_IRQ
};


extern const device_type MN1020012A;


#define MN10200_NUM_PRESCALERS (2)
#define MN10200_NUM_TIMERS_8BIT (10)
#define MN10200_NUM_IRQ_GROUPS (31)


class mn10200_device : public cpu_device
{
public:
	// construction/destruction
	mn10200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(io_control_r);
	DECLARE_WRITE8_MEMBER(io_control_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + 2 - 1) / 2; } // internal /2 divider
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * 2); } // internal /2 divider
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 13; }
	virtual UINT32 execute_input_lines() const { return 4; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : NULL );
	}

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 7; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	address_space *m_program;
	address_space *m_io;

	int m_cycles;

	// The UINT32s are really UINT24
	UINT32 m_pc;
	UINT32 m_d[4];
	UINT32 m_a[4];
	UINT16 m_psw;
	UINT16 m_mdr;

	// interrupts
	UINT8 m_icrl[MN10200_NUM_IRQ_GROUPS];
	UINT8 m_icrh[MN10200_NUM_IRQ_GROUPS];

	UINT8 m_nmicr;
	UINT8 m_iagr;
	UINT8 m_extmdl;
	UINT8 m_extmdh;
	bool m_possible_irq;

	// timers
	attotime m_sysclock_base;
	emu_timer *m_timer_timers[MN10200_NUM_TIMERS_8BIT];

	struct
	{
		UINT8 mode;
		UINT8 base;
		UINT8 cur;
	} m_simple_timer[MN10200_NUM_TIMERS_8BIT];

	struct
	{
		UINT8 mode;
		UINT8 base;
		UINT8 cur;
	} m_prescaler[MN10200_NUM_PRESCALERS];

	// dma
	struct
	{
		UINT32 adr;
		UINT32 count;
		UINT16 iadr;
		UINT8 ctrll;
		UINT8 ctrlh;
		UINT8 irq;
	} m_dma[8];

	// serial
	struct
	{
		UINT8 ctrll;
		UINT8 ctrlh;
		UINT8 buf;
	} m_serial[2];

	// ports
	UINT8 m_pplul;
	UINT8 m_ppluh;
	UINT8 m_p3md;
	UINT8 m_p4;

	struct
	{
		UINT8 out;
		UINT8 dir;
	} m_port[4];

	// internal read/write
	inline UINT8 read_arg8(UINT32 address) { return m_program->read_byte(address); }
	inline UINT16 read_arg16(UINT32 address) { return m_program->read_byte(address) | m_program->read_byte(address + 1) << 8; }
	inline UINT32 read_arg24(UINT32 address) { return m_program->read_byte(address) | m_program->read_byte(address + 1) << 8 | m_program->read_byte(address + 2) << 16; }

	inline UINT8 read_mem8(UINT32 address) { return m_program->read_byte(address); }
	inline UINT16 read_mem16(UINT32 address) { return m_program->read_word(address & ~1); }
	inline UINT32 read_mem24(UINT32 address) { return m_program->read_word(address & ~1) | m_program->read_byte((address & ~1) + 2) << 16; }

	inline void write_mem8(UINT32 address, UINT8 data) { m_program->write_byte(address, data); }
	inline void write_mem16(UINT32 address, UINT16 data) { m_program->write_word(address & ~1, data); }
	inline void write_mem24(UINT32 address, UINT32 data) { m_program->write_word(address & ~1, data); m_program->write_byte((address & ~1) + 2, data >> 16); }

	inline void change_pc(UINT32 pc) { m_pc = pc & 0xffffff; }

	void take_irq(int level, int group);
	void check_irq();
	void check_ext_irq();
	void refresh_timer(int tmr);
	void refresh_all_timers();
	int timer_tick_simple(int tmr);
	TIMER_CALLBACK_MEMBER( simple_timer_cb );
	void illegal(UINT8 prefix, UINT8 op);
	UINT32 do_add(UINT32 a, UINT32 b, UINT32 c);
	UINT32 do_sub(UINT32 a, UINT32 b, UINT32 c);
	void test_nz16(UINT16 v);
	void do_jsr(UINT32 to, UINT32 ret);
	void do_branch(bool state);
};


#endif
