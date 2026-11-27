// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    VideoLogic PMX1 primary core
*/

#ifndef MAME_CPU_PMX1_PMX1CORE_H
#define MAME_CPU_PMX1_PMX1CORE_H

#pragma once

#include "pmx1dasm.h"


class pmx1core_device : public cpu_device, protected pmx1core_disassembler::config
{
public:
	static constexpr u32 THREADS = 4;

	// debugger state: a program counter and flags for each thread, then the registers of units 1 to 4
	enum
	{
		PMX1_PC = 1,
		PMX1_FLAGS = PMX1_PC + THREADS,
		PMX1_REG = PMX1_FLAGS + THREADS
	};

	pmx1core_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto implicit_read() { return m_implicit_read_cb.bind(); }
	auto implicit_write() { return m_implicit_write_cb.bind(); }
	auto ta_instruction() { return m_ta_instruction_cb.bind(); }
	auto ta_status() { return m_ta_status_cb.bind(); }
	auto dac_feed() { return m_dac_feed_cb.bind(); }
	auto dac_busy() { return m_dac_busy_cb.bind(); }
	auto port_read() { return m_port_r_cb.bind(); }
	auto port_write() { return m_port_w_cb.bind(); }

	// signals from the rest of the chip
	void core_reset_w(int state);
	void set_reset_vector(u32 vector);
	void set_thread_enable(u32 thread, bool enable);
	void set_fb_offset(u8 offset);
	void kick(u32 thread, u16 data);
	void dac_ready();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override;
	virtual u32 execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// pmx1core_disassembler::config implementation
	virtual bool jumps_delayed() const override;

private:
	// the registers of units 1 to 4 follow one another
	static constexpr u32 REGISTERS = 32 + 32 + 16 + 16;
	static constexpr u8 UNIT_FIRST[5] = { 0, 0, 32, 64, 80 };
	static constexpr u8 UNIT_SIZE[5] = { 0, 32, 32, 16, 16 };

	// the reads a thread can have waiting to be taken
	static constexpr u32 READ_QUEUE_SIZE = 16;

	static constexpr u8 FLAG_V = 0x01;
	static constexpr u8 FLAG_C = 0x02;
	static constexpr u8 FLAG_Z = 0x04;
	static constexpr u8 FLAG_N = 0x08;

	// what keeps a thread from running
	enum : u8
	{
		HOLD_NONE,
		HOLD_KICK,      // until a kicker is written
		HOLD_FEED,      // until the display feed takes another word
		HOLD_STOPPED    // until the device is reset, or for thread 0 also the core
	};

	// what became of the instruction a thread was given
	enum class outcome : u8
	{
		DONE,
		REPEAT,         // done for this cycle, and the thread stays on it
		WAIT_KICK,
		WAIT_FEED,
		UNKNOWN
	};

	struct thread_state
	{
		u32 pc = 0;
		u32 jump_target = 0;
		u8 jump_delay = 0;              // instructions to complete before a jump that was made is taken
		u32 repeat = 0;                 // stores or feeds that a repeated instruction has still to make
		u64 reads[READ_QUEUE_SIZE] = { };
		u8 read_first = 0;              // where the oldest read is
		u8 read_count = 0;
		u16 kick_count = 0;
		bool kick_async = false;
		u32 answer = 0;                 // from the last read of an implicit register
		u32 port_answer = 0;            // from the last read of a port register
		u8 flags = 0;
		u8 hold = HOLD_NONE;
	};

	static bool has_register(u32 unit, u32 number) { return (unit >= 1) && (unit <= 4) && (number < UNIT_SIZE[unit]); }
	static bool can_take(u32 unit, u32 number);
	static bool condition_known(u32 code);
	static bool condition(u32 code, u8 flags);
	static u32 operand16(u32 op);
	static u32 compute(u32 operation, u32 a, u32 b);
	static u8 flags_after(u32 operation, u32 a, u32 b, u32 result, u8 flags);

	static u32 fetch_address(u32 pc) { return pc & 0xffff'fffc; }
	static u32 jump_origin(u32 thread, u32 pc) { return thread ? pc : (pc + 16); }
	static u64 take_read(thread_state &ts);

	u32 &reg(u32 unit, u32 number) { return m_reg[UNIT_FIRST[unit] + number]; }
	bool in_memory(u32 address) const { return (address >> 26) == m_fb_offset; }
	bool jump(u32 thread, u32 target);
	bool send(u32 thread, u32 unit, u32 number, u32 value);
	bool queue_read(thread_state &ts, u32 address);

	int runnable_thread() const;
	void step(u32 thread);
	outcome arithmetic(u32 thread, u32 op);
	outcome compare(thread_state &ts, u32 op);
	outcome branch(u32 thread, u32 pc, u32 op);
	outcome move(u32 thread, u32 pc, u32 op);
	outcome implicit(thread_state &ts, u32 op);
	outcome move64(thread_state &ts, u32 op);
	outcome memory(thread_state &ts, u32 op);
	bool start_repeat(thread_state &ts, u32 pc);
	outcome repeated_store(thread_state &ts, u32 pc, u32 op);
	outcome port(thread_state &ts, u32 op);

	const address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;

	devcb_read16 m_implicit_read_cb;
	devcb_write16 m_implicit_write_cb;
	devcb_write64 m_ta_instruction_cb;
	devcb_read64 m_ta_status_cb;
	devcb_write64 m_dac_feed_cb;
	devcb_read_line m_dac_busy_cb;
	devcb_read32 m_port_r_cb;
	devcb_write32 m_port_w_cb;

	// what the chip tells the core
	u32 m_reset_vector;
	u8 m_fb_offset;
	bool m_in_reset;
	bool m_thread_enable[THREADS];

	u32 m_reg[REGISTERS];
	thread_state m_thread[THREADS];
	u32 m_latch;                        // set and stored through the port registers
	u32 m_cycles;
	u8 m_current;                       // the thread that was last given an instruction
	int m_icount;
};

DECLARE_DEVICE_TYPE(PMX1CORE, pmx1core_device)

#endif // MAME_CPU_PMX1_PMX1CORE_H
