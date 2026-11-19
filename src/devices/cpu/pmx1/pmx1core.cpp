// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    VideoLogic PMX1 primary core

    The processor inside the PMX1 (PowerVR 250) graphics chip.  It serves
    the host's requests, starts 2D operations, passes instructions to the
    tile accelerator and feeds the DAC, all with code that the host
    software loads into the chip's memory.

    Four threads share two banks of 32 data registers and two banks of 16
    address registers.  Thread 0 is the background thread that the host
    talks to; threads 1 to 3 are real-time threads, which the chip's
    documentation gives to video in, the TV encoder and the DAC feed.  Each
    thread has a program counter, flags N Z C V, a kicker and a queue of
    memory reads of its own.  Every cycle of the core clock the
    highest-numbered thread that is able to run executes one instruction.

    Only the core itself can set the program counters of threads 1 to 3,
    so thread 0 gives each of them one and then enables it through the
    chip's registers (Programming Guide, "Primary Core").  The known code
    does that for thread 2 as well, and sends it to address zero.

    A kicker is a counter that the host, or a thread, adds to by writing
    one of the chip's implicit registers.  A thread that takes a kick from
    a kicker that has none sleeps until one is written, which is how the
    threads wait for work.

    Memory is not read by the instruction that wants the data.  A read is
    queued, which captures 64 bits, and a later instruction takes the
    oldest read from the queue.

    Thread 0 takes a jump after the four instructions that follow it, and
    the addresses of its relative jumps and of its returns are counted from
    there.  Threads 1 to 3 jump at once.

    Instruction fields that select a unit use these numbers:
        0       memory (only as the destination of a move)
        1, 2    data registers
        3, 4    address registers
        5       program counters of the four threads
        6       read queue
        8       answer to the last read of an implicit register
        10      kickers
        11      cycle counter

    The condition codes are those of the ARM plus one: 0 is always, 1 EQ,
    2 NE, 3 CS, 4 CC, 5 MI, 6 PL, 7 VS, 8 VC, 9 HI, 10 LS, 11 GE, 12 LT,
    13 GT and 14 LE.  A shift leaves the last bit shifted out in C.

    VideoLogic did not publish the instruction set.  What is known of the
    encoding was worked out from the microcode of the Neon 250 drivers, the
    Gaelco games and the Aristocrat MK6, so that only what this code uses
    is known. A thread that meets anything else is stopped and the
    instruction is logged.

    TODO:
    - How the processor is told the number of stores of a repeated store,
      or of feeds of a repeated feed, is not known.  The known code always
      has three comparisons of the same register in front of it, with -2,
      -1 and 0, and that register holds three less than the number.
    - What bit 0 does in the conditional forms of the arithmetic is not
      known.  The MK6 code has it set in a few places where the use of the
      result shows it makes no difference.
    - What a jump does when thread 0 has not yet taken the one before is
      not known.
    - Instructions are fetched from the chip's own memory only.  A thread
      whose program counter is outside the 64MB that the memory is in does
      not run: neither the boot ROM, which thread 0 is sent to when the
      chip is reset, nor memory of the host is reached.  The known boards
      have no such ROM, and their hosts hold the core in reset until they
      have loaded its code.
    - The cycle counter is taken to count the cycles of the core clock
      whether a thread runs or not.  The known code only stores it as a
      time stamp.
*/

#include "emu.h"
#include "pmx1core.h"

#include <algorithm>
#include <iterator>
#include <utility>


DEFINE_DEVICE_TYPE(PMX1CORE, pmx1core_device, "pmx1core", "VideoLogic PMX1 primary core")


pmx1core_device::pmx1core_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	cpu_device(mconfig, PMX1CORE, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0),
	m_implicit_read_cb(*this, 0),
	m_implicit_write_cb(*this),
	m_ta_instruction_cb(*this),
	m_ta_status_cb(*this, 0),
	m_dac_feed_cb(*this),
	m_dac_busy_cb(*this, 0),
	m_reset_vector(0),
	m_fb_offset(0),
	m_in_reset(false),
	m_thread_enable{ false, false, false, false }
{
}

void pmx1core_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	m_current = 0;

	save_item(NAME(m_reset_vector));
	save_item(NAME(m_fb_offset));
	save_item(NAME(m_in_reset));
	save_item(NAME(m_thread_enable));
	save_item(NAME(m_reg));
	save_item(STRUCT_MEMBER(m_thread, pc));
	save_item(STRUCT_MEMBER(m_thread, jump_target));
	save_item(STRUCT_MEMBER(m_thread, jump_delay));
	save_item(STRUCT_MEMBER(m_thread, repeat));
	save_item(STRUCT_MEMBER(m_thread, reads));
	save_item(STRUCT_MEMBER(m_thread, read_first));
	save_item(STRUCT_MEMBER(m_thread, read_count));
	save_item(STRUCT_MEMBER(m_thread, kick_count));
	save_item(STRUCT_MEMBER(m_thread, kick_async));
	save_item(STRUCT_MEMBER(m_thread, answer));
	save_item(STRUCT_MEMBER(m_thread, flags));
	save_item(STRUCT_MEMBER(m_thread, hold));
	save_item(NAME(m_latch));
	save_item(NAME(m_cycles));
	save_item(NAME(m_current));

	// the debugger is shown the thread that was last given an instruction, at the address its instruction is fetched from
	state_add<u32>(STATE_GENPC, "GENPC", [this] () { return fetch_address(m_thread[m_current].pc); }).noshow();
	state_add<u32>(STATE_GENPCBASE, "CURPC", [this] () { return fetch_address(m_thread[m_current].pc); }).noshow();
	state_add<u8>(STATE_GENFLAGS, "CURFLAGS", [this] () { return m_thread[m_current].flags; }).formatstr("%4s").noshow();
	for (u32 thread = 0; thread < THREADS; thread++)
	{
		state_add(PMX1_PC + thread, util::string_format("PC%u", thread), m_thread[thread].pc);
		state_add(PMX1_FLAGS + thread, util::string_format("F%u", thread), m_thread[thread].flags).mask(0x0f).formatstr("%4s");
	}
	for (u32 number = 0; number < 32; number++)
		state_add(PMX1_REG + UNIT_FIRST[1] + number, util::string_format("DA%u", number), reg(1, number));
	for (u32 number = 0; number < 32; number++)
		state_add(PMX1_REG + UNIT_FIRST[2] + number, util::string_format("DB%u", number), reg(2, number));
	for (u32 number = 0; number < 16; number++)
		state_add(PMX1_REG + UNIT_FIRST[3] + number, util::string_format("AA%u", number), reg(3, number));
	for (u32 number = 0; number < 16; number++)
		state_add(PMX1_REG + UNIT_FIRST[4] + number, util::string_format("AB%u", number), reg(4, number));

	set_icountptr(m_icount);
}

void pmx1core_device::device_reset()
{
	for (thread_state &ts : m_thread)
		ts = thread_state();

	// thread 0 is sent to the reset vector, where the boot ROM is
	m_thread[0].pc = m_reset_vector + 4;

	m_latch = 0;
	m_cycles = 0;
}

u32 pmx1core_device::execute_min_cycles() const noexcept
{
	return 1;
}

u32 pmx1core_device::execute_max_cycles() const noexcept
{
	return 1;
}

device_memory_interface::space_config_vector pmx1core_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_program_config) };
}

void pmx1core_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	const int index = entry.index();
	if ((index == STATE_GENFLAGS) || ((index >= PMX1_FLAGS) && (index < PMX1_REG)))
	{
		const u8 flags = entry.value();
		str = util::string_format("%c%c%c%c",
				(flags & FLAG_N) ? 'N' : '.',
				(flags & FLAG_Z) ? 'Z' : '.',
				(flags & FLAG_C) ? 'C' : '.',
				(flags & FLAG_V) ? 'V' : '.');
	}
}

std::unique_ptr<util::disasm_interface> pmx1core_device::create_disassembler()
{
	const pmx1core_disassembler::config *const conf = this;
	return std::make_unique<pmx1core_disassembler>(conf);
}

bool pmx1core_device::jumps_delayed() const
{
	return m_current == 0;
}


//**************************************************************************
//  signals from the rest of the chip
//**************************************************************************

void pmx1core_device::core_reset_w(int state)
{
	if (state && m_in_reset)
	{
		// thread 0 starts afresh at the instruction after the reset vector; the other threads are as they were
		thread_state &ts = m_thread[0];
		ts.pc = m_reset_vector + 4;
		ts.jump_delay = 0;
		ts.repeat = 0;
		ts.read_first = 0;
		ts.read_count = 0;
		ts.hold = HOLD_NONE;
	}
	m_in_reset = !state;
}

void pmx1core_device::set_reset_vector(u32 vector)
{
	m_reset_vector = vector;
}

void pmx1core_device::set_thread_enable(u32 thread, bool enable)
{
	if (thread < THREADS)
		m_thread_enable[thread] = enable;
}

void pmx1core_device::set_fb_offset(u8 offset)
{
	m_fb_offset = offset & 0x3f;
}

void pmx1core_device::kick(u32 thread, u16 data)
{
	if (thread >= THREADS)
		return;

	// 15 bits are counted and the top bit sticks
	thread_state &kicked = m_thread[thread];
	kicked.kick_count = (kicked.kick_count + data) & 0x7fff;
	if (BIT(data, 15))
		kicked.kick_async = true;

	// every thread that waits for a kick looks at its kicker again
	for (thread_state &ts : m_thread)
	{
		if (ts.hold == HOLD_KICK)
			ts.hold = HOLD_NONE;
	}
}

void pmx1core_device::dac_ready()
{
	for (thread_state &ts : m_thread)
	{
		if (ts.hold == HOLD_FEED)
			ts.hold = HOLD_NONE;
	}
}


//**************************************************************************
//  parts of the instructions
//**************************************************************************

bool pmx1core_device::can_take(u32 unit, u32 number)
{
	// besides a register, a value can be sent to a program counter or to the read queue as the address of a read
	switch (unit)
	{
	case 5:
		return number < THREADS;
	case 6:
		return number == 0;
	default:
		return has_register(unit, number);
	}
}

bool pmx1core_device::condition_known(u32 code)
{
	// 15 is not used by the known code
	return code != 15;
}

bool pmx1core_device::condition(u32 code, u8 flags)
{
	const bool zero = flags & FLAG_Z;
	const bool less = bool(flags & FLAG_N) != bool(flags & FLAG_V);
	switch (code)
	{
	case 1:
		return zero;
	case 2:
		return !zero;
	case 9:
		return (flags & FLAG_C) && !zero;
	case 11:
		return !less;
	case 12:
		return less;
	case 3:
		return flags & FLAG_C;
	case 4:
		return !(flags & FLAG_C);
	case 5:
		return flags & FLAG_N;
	case 6:
		return !(flags & FLAG_N);
	case 7:
		return flags & FLAG_V;
	case 8:
		return !(flags & FLAG_V);
	case 10:
		return !(flags & FLAG_C) || zero;
	case 13:
		return !zero && !less;
	case 14:
		return zero || less;
	default:
		return true;
	}
}

u32 pmx1core_device::operand16(u32 op)
{
	// bit 1 extends the sign of the 16 bits, bit 0 then moves them to the upper half
	const u32 value = BIT(op, 1) ? u32(util::sext(BIT(op, 3, 16), 16)) : BIT(op, 3, 16);
	return BIT(op, 0) ? (value << 16) : value;
}

u32 pmx1core_device::compute(u32 operation, u32 a, u32 b)
{
	switch (operation)
	{
	case 0:
		return a + b;
	case 1:
		return a - b;
	case 2:
		return a & b;
	case 3:
		return a | b;
	case 4:
		return a ^ b;
	case 5:
		return a << (b & 31);
	default:
		return a >> (b & 31);
	}
}

u8 pmx1core_device::flags_after(u32 operation, u32 a, u32 b, u32 result, u8 flags)
{
	// the logical operations leave C and V alone, a subtraction has C set when there is no borrow, and a shift has the last
	// bit shifted out in C and clears V: the MK6 code multiplies by a number of bytes per pixel with shr.f by 1 and add.cs
	bool carry = flags & FLAG_C, overflow = flags & FLAG_V;
	switch (operation)
	{
	case 0:
		carry = result < a;
		overflow = BIT(~(a ^ b) & (a ^ result), 31);
		break;
	case 1:
		carry = a >= b;
		overflow = BIT((a ^ b) & (a ^ result), 31);
		break;
	case 5:
		carry = b && (b <= 32) && BIT(a, 32 - b);
		overflow = false;
		break;
	case 6:
		carry = b && (b <= 32) && BIT(a, b - 1);
		overflow = false;
		break;
	default:
		break;
	}
	return (BIT(result, 31) ? FLAG_N : 0) | (result ? 0 : FLAG_Z) | (carry ? FLAG_C : 0) | (overflow ? FLAG_V : 0);
}

bool pmx1core_device::jump(u32 thread, u32 target)
{
	// thread 0 first executes the four instructions that follow
	thread_state &ts = m_thread[thread];
	if (ts.jump_delay)
		return false;
	ts.jump_target = target;
	ts.jump_delay = thread ? 1 : 5;
	return true;
}

bool pmx1core_device::send(u32 thread, u32 unit, u32 number, u32 value)
{
	// the caller has seen that the unit can take a value; what is returned is whether it is known what happens
	switch (unit)
	{
	case 5:
		// the value is the address before the one to go to
		if (number == thread)
			return jump(thread, value + 4);
		m_thread[number].pc = value + 4;
		m_thread[number].repeat = 0;
		if (!in_memory(value + 4))
			logerror("thread %u is sent to %08x, outside the chip's memory: it does not run from there\n", number, value + 4);
		return true;
	case 6:
		return queue_read(m_thread[thread], value);
	default:
		reg(unit, number) = value;
		return true;
	}
}

bool pmx1core_device::queue_read(thread_state &ts, u32 address)
{
	// the data is that of this moment, whenever it is taken
	if (ts.read_count >= READ_QUEUE_SIZE)
		return false;
	const u32 low = m_program.read_dword(address);
	const u32 high = m_program.read_dword(address + 4);
	ts.reads[(ts.read_first + ts.read_count) % READ_QUEUE_SIZE] = (u64(high) << 32) | low;
	ts.read_count++;
	return true;
}

u64 pmx1core_device::take_read(thread_state &ts)
{
	const u64 data = ts.reads[ts.read_first];
	ts.read_first = (ts.read_first + 1) % READ_QUEUE_SIZE;
	ts.read_count--;
	return data;
}


//**************************************************************************
//  execution
//**************************************************************************

int pmx1core_device::runnable_thread() const
{
	if (!m_in_reset)
	{
		// FIXME: a thread that has its next instruction outside the 64MB of the chip's memory is passed over, as one that
		// waits for memory which never answers.  What the chip fetches from there is not emulated.
		for (int thread = THREADS - 1; thread >= 0; thread--)
		{
			if (m_thread_enable[thread] && (m_thread[thread].hold == HOLD_NONE) && in_memory(m_thread[thread].pc))
				return thread;
		}
	}
	return -1;
}

void pmx1core_device::execute_run()
{
	do
	{
		const int thread = runnable_thread();
		if (thread < 0)
		{
			// nothing happens until the chip lets a thread run, but the clock goes on
			m_cycles += m_icount;
			m_icount = 0;
			break;
		}
		m_current = thread;
		step(thread);
		m_cycles++;
		m_icount--;
	} while (m_icount > 0);
}

void pmx1core_device::step(u32 thread)
{
	thread_state &ts = m_thread[thread];
	debugger_instruction_hook(fetch_address(ts.pc));
	const u32 pc = ts.pc;
	const u32 op = m_cache.read_dword(fetch_address(pc));

	outcome result = outcome::UNKNOWN;
	switch (BIT(op, 28, 4))
	{
	case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 9:
		result = arithmetic(thread, op);
		break;

	case 8:
		result = compare(ts, op);
		break;

	case 10:
		switch (BIT(op, 24, 4))
		{
		case 4:
			result = move(thread, pc, op);
			break;
		case 5:
			result = implicit(ts, op);
			break;
		case 14:
			result = move64(ts, op);
			break;
		}
		break;

	case 11: case 12:
		if (BIT(op, 24, 8) == 0xbc)
			result = repeated_store(ts, pc, op);
		else
			result = memory(ts, op);
		break;

	case 14:
		result = port(ts, op);
		break;

	case 15:
		// two words that do nothing; one of them starts each fragment of the code, with any number in bits 23-19
		if ((op == 0xf100'0000) || ((op & 0xff07'ffff) == 0xf100'0002))
			result = outcome::DONE;
		else if (!BIT(op, 24, 4))
			result = branch(thread, pc, op);
		break;
	}

	switch (result)
	{
	case outcome::DONE:
		// a jump is taken once the instructions that it lets through are done
		if (ts.jump_delay == 1)
		{
			ts.pc = ts.jump_target;
			if (!in_memory(ts.pc))
				logerror("thread %u jumps from %08x to %08x, outside the chip's memory: it does not run from there\n", thread, pc, ts.pc);
		}
		else
		{
			ts.pc += 4;
		}
		if (ts.jump_delay)
			ts.jump_delay--;
		break;

	case outcome::REPEAT:
		break;

	case outcome::WAIT_KICK:
		ts.hold = HOLD_KICK;
		break;

	case outcome::WAIT_FEED:
		ts.hold = HOLD_FEED;
		break;

	case outcome::UNKNOWN:
		ts.hold = HOLD_STOPPED;
		logerror("thread %u stopped at %08x: instruction %08x is not known\n", thread, pc, op);
		break;
	}
}

pmx1core_device::outcome pmx1core_device::arithmetic(u32 thread, u32 op)
{
	// majors 0 to 6 are the operations on the data registers; major 9 adds address registers
	thread_state &ts = m_thread[thread];
	const u32 major = BIT(op, 28, 4);
	const u32 operation = (major == 9) ? 0 : major;
	const u32 unit = ((major == 9) ? 3 : 1) + BIT(op, 24);
	const bool flags = BIT(op, 27), conditional = BIT(op, 26), immediate = BIT(op, 25);
	const bool shift = operation >= 5;
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5), s2 = BIT(op, 9, 5);
	const bool to_unit = conditional && immediate && !shift && BIT(op, 5);
	if (flags && ((major == 9) || (conditional && !to_unit)))
		return outcome::UNKNOWN;

	if (!conditional)
	{
		// the result goes to register D of the same unit
		if (!has_register(unit, d))
			return outcome::UNKNOWN;
		u32 a, b;
		if (!immediate)
		{
			// two registers; with bit 2 the second is only copied
			if ((op & 0x1fb) || !has_register(unit, s2))
				return outcome::UNKNOWN;
			if (BIT(op, 2))
			{
				if (flags || shift)
					return outcome::UNKNOWN;
				reg(unit, d) = reg(unit, s2);
				return outcome::DONE;
			}
			if (!has_register(unit, s1) || (shift && (reg(unit, s2) > 31)))
				return outcome::UNKNOWN;
			a = reg(unit, s1);
			b = reg(unit, s2);
		}
		else if (shift)
		{
			// the count is the S2 field itself
			if (BIT(op, 0, 9))
				return outcome::UNKNOWN;
			a = reg(unit, s1);
			b = s2;
		}
		else if (BIT(op, 2))
		{
			// the 16-bit operand, or what is left of zero when it is taken away, is loaded
			if ((operation > 1) || flags)
				return outcome::UNKNOWN;
			reg(unit, d) = compute(operation, 0, operand16(op));
			return outcome::DONE;
		}
		else
		{
			a = reg(unit, d);
			b = operand16(op);
		}
		const u32 result = compute(operation, a, b);
		reg(unit, d) = result;
		if (flags)
			ts.flags = flags_after(operation, a, b, result, ts.flags);
		return outcome::DONE;
	}

	if (!has_register(unit, s1))
		return outcome::UNKNOWN;
	if (immediate && !shift && BIT(op, 5))
	{
		// an 8-bit operand, and the result can go to another unit
		const u32 to = BIT(op, 1, 4);
		if ((operation > 2) || !can_take(to, d))
			return outcome::UNKNOWN;
		const u32 a = reg(unit, s1), b = BIT(op, 6, 8);
		const u32 result = compute(operation, a, b);
		if (flags)
			ts.flags = flags_after(operation, a, b, result, ts.flags);
		return send(thread, to, d, result) ? outcome::DONE : outcome::UNKNOWN;
	}

	// the other forms have a condition
	const u32 code = BIT(op, 1, 4);
	if (!condition_known(code))
		return outcome::UNKNOWN;
	const bool met = condition(code, ts.flags);
	if (immediate && shift)
	{
		// shift by the S2 field into a register of any unit
		const u32 to = BIT(op, 5, 4);
		if (!has_register(to, d))
			return outcome::UNKNOWN;
		if (met)
			reg(to, d) = compute(operation, reg(unit, s1), s2);
	}
	else if (immediate)
	{
		// FIXME: what bit 0 does is not known
		if (!has_register(unit, d))
			return outcome::UNKNOWN;
		if (met)
			reg(unit, d) = compute(operation, reg(unit, s1), BIT(op, 6, 8));
	}
	else
	{
		// two registers, and the result can go to another unit
		// FIXME: what bit 0 does is not known
		const u32 to = BIT(op, 5, 4);
		if (shift || !has_register(unit, s2) || !can_take(to, d))
			return outcome::UNKNOWN;
		if (met && !send(thread, to, d, compute(operation, reg(unit, s1), reg(unit, s2))))
			return outcome::UNKNOWN;
	}
	return outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::compare(thread_state &ts, u32 op)
{
	// a subtraction or an AND of data registers of which only the flags are kept
	const u32 operation = BIT(op, 27) ? 2 : 1;
	const u32 unit = 1 + BIT(op, 24);
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5), s2 = BIT(op, 9, 5);
	u32 a, b;
	switch (BIT(op, 25, 2))
	{
	case 0:
		if (d || BIT(op, 0, 9))
			return outcome::UNKNOWN;
		a = reg(unit, s1);
		b = reg(unit, s2);
		break;

	case 1:
		if (BIT(op, 2))
			return outcome::UNKNOWN;
		a = reg(unit, d);
		b = operand16(op);
		break;

	case 3:
		if (d || BIT(op, 5) || BIT(op, 0) || !condition_known(BIT(op, 1, 4)))
			return outcome::UNKNOWN;
		if (!condition(BIT(op, 1, 4), ts.flags))
			return outcome::DONE;
		a = reg(unit, s1);
		b = BIT(op, 6, 8);
		break;

	default:
		return outcome::UNKNOWN;
	}
	ts.flags = flags_after(operation, a, b, compute(operation, a, b), ts.flags);
	return outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::branch(u32 thread, u32 pc, u32 op)
{
	const u32 code = BIT(op, 1, 4);
	if (BIT(op, 19, 5) || BIT(op, 0) || !condition_known(code))
		return outcome::UNKNOWN;

	// the distance is in instructions
	if (condition(code, m_thread[thread].flags) && !jump(thread, jump_origin(thread, pc) + (u32(util::sext(BIT(op, 5, 14), 14)) << 2)))
		return outcome::UNKNOWN;
	return outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::move(u32 thread, u32 pc, u32 op)
{
	thread_state &ts = m_thread[thread];
	const u32 code = BIT(op, 1, 4);
	const u32 from = BIT(op, 5, 4), to = BIT(op, 10, 4);
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5);
	if (!condition_known(code))
		return outcome::UNKNOWN;

	if (BIT(op, 9))
	{
		// a register changes places with another, or with the program counter to call a subroutine
		if (!has_register(from, s1))
			return outcome::UNKNOWN;
		if (to == 5)
		{
			if (d != thread)
				return outcome::UNKNOWN;
			if (condition(code, ts.flags))
			{
				// like any value sent to a program counter, the register holds the address before the one meant
				if (!jump(thread, reg(from, s1) + 4))
					return outcome::UNKNOWN;
				reg(from, s1) = jump_origin(thread, pc);
			}
		}
		else if (has_register(to, d))
		{
			if (condition(code, ts.flags))
				std::swap(reg(from, s1), reg(to, d));
		}
		else
		{
			return outcome::UNKNOWN;
		}
		return outcome::DONE;
	}

	switch (from)
	{
	case 1: case 2: case 3: case 4:
		if (!has_register(from, s1))
			return outcome::UNKNOWN;
		break;
	case 6:
		break;
	case 8:
		if (s1 != 2)
			return outcome::UNKNOWN;
		break;
	case 10:
		if ((s1 >= THREADS) && (s1 != 7))
			return outcome::UNKNOWN;
		break;
	case 11:
		if (s1)
			return outcome::UNKNOWN;
		break;
	default:
		return outcome::UNKNOWN;
	}
	if (to && !can_take(to, d))
		return outcome::UNKNOWN;

	// a read is taken from the queue whether the condition is met or not
	u32 value = 0;
	if (from == 6)
	{
		if (!ts.read_count)
			return outcome::UNKNOWN;
		value = u32(take_read(ts));
	}
	if (!condition(code, ts.flags))
		return outcome::DONE;

	switch (from)
	{
	case 6:
		break;

	case 8:
		value = ts.answer;
		break;

	case 10:
		{
			// 7 is the thread's own kicker as it is; with a thread's number a kick is taken from its kicker, when it has one
			thread_state &kicked = (s1 == 7) ? ts : m_thread[s1];
			value = (kicked.kick_async ? 0x8000 : 0) | kicked.kick_count;
			if (s1 != 7)
			{
				if (!value)
					return outcome::WAIT_KICK;
				if (kicked.kick_count)
					kicked.kick_count--;
				kicked.kick_async = false;
			}
		}
		break;

	case 11:
		value = m_cycles;
		break;

	default:
		value = reg(from, s1);
		break;
	}

	// unit 0 as the destination is memory, at the address in any of the address registers
	if (!to)
		m_program.write_dword(reg(3 + BIT(d, 4), d & 15), value);
	else if (!send(thread, to, d, value))
		return outcome::UNKNOWN;
	return outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::implicit(thread_state &ts, u32 op)
{
	const u32 unit = BIT(op, 5, 4);
	const u32 s1 = BIT(op, 14, 5);
	const u32 code = BIT(op, 1, 4);
	if (BIT(op, 19, 5) || BIT(op, 9, 5) || BIT(op, 0) || !has_register(unit, s1) || !condition_known(code))
		return outcome::UNKNOWN;

	if (condition(code, ts.flags))
	{
		// the register holds what the host would put in the chip's write access register
		const u32 command = reg(unit, s1);
		const offs_t number = BIT(command, 16, 10);
		if (BIT(command, 30, 2) == 1)
			m_implicit_write_cb(number, u16(command));
		else if (!BIT(command, 26, 6))
			ts.answer = (command & 0xffff'0000) | m_implicit_read_cb(number);
		else
			return outcome::UNKNOWN;
	}
	return outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::move64(thread_state &ts, u32 op)
{
	// 64 bits to a register of each bank of data registers
	const u32 d = BIT(op, 19, 5);
	u64 data;
	switch (BIT(op, 5, 4))
	{
	case 6:
		if (d || !ts.read_count)
			return outcome::UNKNOWN;
		data = take_read(ts);
		break;

	case 8:
		if (d != 4)
			return outcome::UNKNOWN;
		data = m_ta_status_cb();
		break;

	default:
		return outcome::UNKNOWN;
	}
	reg(1, BIT(op, 14, 5)) = u32(data);
	reg(2, BIT(op, 9, 5)) = u32(data >> 32);
	return outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::memory(thread_state &ts, u32 op)
{
	// the address is an address register and an offset; with bit 26 the register is left with that address
	const bool load = BIT(op, 28, 4) == 12;
	const u32 minor = BIT(op, 24, 4);
	const u32 bank = 3 + BIT(op, 18);
	const u32 unit = BIT(op, 1, 4);
	const u32 d = BIT(op, 19, 5);
	u32 &pointer = reg(bank, BIT(op, 14, 4));

	if (!load && !(minor & 0xb))
	{
		// store with the offset in a register of the same bank
		if (BIT(op, 13) || BIT(op, 6, 3) || !BIT(op, 5) || BIT(op, 0) || !has_register(unit, d))
			return outcome::UNKNOWN;
		const u32 address = pointer + reg(bank, BIT(op, 9, 4));
		m_program.write_dword(address, reg(unit, d));
		if (BIT(op, 26))
			pointer = address;
		return outcome::DONE;
	}

	if (load && !(minor & 0xb))
	{
		// load with the offset in a register of the same bank
		if (BIT(op, 13) || BIT(op, 6, 3) || !BIT(op, 5) || BIT(op, 0) || !has_register(unit, d))
			return outcome::UNKNOWN;
		const u32 address = pointer + reg(bank, BIT(op, 9, 4));
		if (!queue_read(ts, address))
			return outcome::UNKNOWN;
		if (BIT(op, 26))
			pointer = address;
		reg(unit, d) = u32(take_read(ts));
		return outcome::DONE;
	}

	// the other forms have the offset in the instruction, in words
	if (BIT(op, 6, 2) || !BIT(op, 5) || BIT(op, 0))
		return outcome::UNKNOWN;
	const u32 address = pointer + (BIT(op, 8, 6) << 2);
	if (load)
	{
		// the read joins the queue, and what the register gets is the oldest read of the queue
		if (((minor & 0xb) != 2) || !has_register(unit, d) || !queue_read(ts, address))
			return outcome::UNKNOWN;
		if (BIT(op, 26))
			pointer = address;
		reg(unit, d) = u32(take_read(ts));
	}
	else if ((minor & 0xb) == 2)
	{
		// store a register or a read from the queue
		if (unit == 6)
		{
			if (!ts.read_count)
				return outcome::UNKNOWN;
			m_program.write_dword(address, u32(take_read(ts)));
		}
		else if (has_register(unit, d))
		{
			m_program.write_dword(address, reg(unit, d));
		}
		else
		{
			return outcome::UNKNOWN;
		}
		if (BIT(op, 26))
			pointer = address;
	}
	else if (minor == 7)
	{
		// store all 64 bits of a read from the queue
		if ((unit != 6) || !ts.read_count)
			return outcome::UNKNOWN;
		const u64 data = take_read(ts);
		m_program.write_dword(address, u32(data));
		m_program.write_dword(address + 4, u32(data >> 32));
		pointer = address;
	}
	else
	{
		return outcome::UNKNOWN;
	}
	return outcome::DONE;
}

bool pmx1core_device::start_repeat(thread_state &ts, u32 pc)
{
	// FIXME: the number is taken from the data register that the three instructions in front compare with -2, -1
	// and 0, which is three less.  How the processor comes by the number is not known.
	const u32 last = m_cache.read_dword(fetch_address(pc - 4));
	const u32 unit = 1 + BIT(last, 24), number = BIT(last, 19, 5);
	for (u32 n = 0; n < 3; n++)
	{
		const u32 before = m_cache.read_dword(fetch_address(pc - 12 + (4 * n)));
		if (((before & 0xfef8'0004) != (0x8200'0000 | (number << 19))) || (BIT(before, 24) != BIT(last, 24)) || (operand16(before) != (n - 2)))
			return false;
	}
	const u32 count = reg(unit, number) + 3;
	if (ts.jump_delay || (s32(count) < 1))
		return false;
	ts.repeat = count;
	return true;
}

pmx1core_device::outcome pmx1core_device::repeated_store(thread_state &ts, u32 pc, u32 op)
{
	if (BIT(op, 13) || (BIT(op, 0, 9) != 0x059))
		return outcome::UNKNOWN;

	if (!ts.repeat && !start_repeat(ts, pc))
		return outcome::UNKNOWN;

	// one store in each cycle, each time after moving the address register on by another register of its bank
	const u32 bank = 3 + BIT(op, 18);
	u32 &pointer = reg(bank, BIT(op, 14, 4));
	pointer += reg(bank, BIT(op, 9, 4));
	m_program.write_dword(pointer, reg(2, BIT(op, 19, 5)));
	ts.repeat--;
	return ts.repeat ? outcome::REPEAT : outcome::DONE;
}

pmx1core_device::outcome pmx1core_device::port(thread_state &ts, u32 op)
{
	const u32 d = BIT(op, 19, 5), s1 = BIT(op, 14, 5);
	switch (BIT(op, 24, 4))
	{
	case 2:
		if (BIT(op, 0, 14) == 0x0004)
		{
			// an instruction for the tile accelerator, made of two address registers
			m_ta_instruction_cb((u64(reg(4, s1 & 15)) << 32) | reg(3, d & 15));
		}
		else
		{
			// an address register or a data register is written to a port register: one port register keeps
			// the value, and the other stores what was kept at the address written to it
			if (BIT(op, 0, 3) || !BIT(op, 5) || !BIT(op, 4))
				return outcome::UNKNOWN;
			const u32 value = BIT(op, 3) ? reg(1, d) : reg(3, d & 15);
			switch (BIT(op, 6, 8))
			{
			case 0x64:
				m_program.write_dword(value, m_latch);
				break;
			case 0x65:
				m_latch = value;
				break;
			default:
				return outcome::UNKNOWN;
			}
		}
		return outcome::DONE;

	case 4:
		// the oldest read goes to the display feed, and the next one is queued
		if (BIT(op, 0, 14) != 0x000b)
			return outcome::UNKNOWN;
		if (m_dac_busy_cb())
			return outcome::WAIT_FEED;
		if (!ts.read_count)
			return outcome::UNKNOWN;
		m_dac_feed_cb(take_read(ts));
		return queue_read(ts, reg(3 + BIT(d, 4), d & 15)) ? outcome::DONE : outcome::UNKNOWN;

	case 13:
		{
			// repeated feed: each time the oldest read goes to the display feed, address register D is moved on by
			// register S1 of its bank, and a read from there is queued
			if (BIT(op, 0, 14) != 0x1c03)
				return outcome::UNKNOWN;
			if (!ts.repeat && !start_repeat(ts, ts.pc))
				return outcome::UNKNOWN;
			if (m_dac_busy_cb())
				return outcome::WAIT_FEED;
			if (!ts.read_count)
				return outcome::UNKNOWN;
			m_dac_feed_cb(take_read(ts));
			u32 &pointer = reg(3 + BIT(d, 4), d & 15);
			pointer += reg(3 + BIT(d, 4), s1 & 15);
			if (!queue_read(ts, pointer))
				return outcome::UNKNOWN;
			ts.repeat--;
			return ts.repeat ? outcome::REPEAT : outcome::DONE;
		}

	default:
		return outcome::UNKNOWN;
	}
}
