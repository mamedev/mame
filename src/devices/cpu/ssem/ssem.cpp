// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Manchester Small-Scale Experimental Machine (SSEM) emulator

    Written by Ryan Holtz
*/

#include "emu.h"
#include "ssem.h"
#include "ssemdasm.h"


#define SSEM_DISASM_ON_UNIMPL           0
#define SSEM_DUMP_MEM_ON_UNIMPL         0

#define INSTR       ((op >> 13) & 7)
#define ADDR        (op & 0x1f)

/*****************************************************************************/

// The SSEM stores its data, visually, with the leftmost bit corresponding to the least significant bit.
// The de facto snapshot format for other SSEM simulators stores the data physically in that format as well.
// Therefore, in MESS, every 32-bit word has its bits reversed, too, and as a result the values must be
// un-reversed before being used.
static inline uint32_t reverse(uint32_t v)
{
	// Taken from http://www-graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ...
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = ( v >> 16             ) | ( v               << 16);

	return v;
}

inline uint32_t ssem_device::program_read32(uint32_t address)
{
	uint32_t v = 0;
	// The MAME core does not have a good way of specifying a minimum datum size that is more than
	// 8 bits in width.  The minimum datum width on the SSEM is 32 bits, so we need to quadruple
	// the address value to get the appropriate byte index.
	address <<= 2;

	v |= m_program->read_byte(address + 0) << 24;
	v |= m_program->read_byte(address + 1) << 16;
	v |= m_program->read_byte(address + 2) <<  8;
	v |= m_program->read_byte(address + 3) <<  0;

	return reverse(v);
}

inline void ssem_device::program_write32(uint32_t address, uint32_t data)
{
	uint32_t v = reverse(data);

	// The MAME core does not have a good way of specifying a minimum datum size that is more than
	// 8 bits in width.  The minimum datum width on the SSEM is 32 bits, so we need to quadruple
	// the address value to get the appropriate byte index.
	address <<= 2;

	m_program->write_byte(address + 0, (v >> 24) & 0x000000ff);
	m_program->write_byte(address + 1, (v >> 16) & 0x000000ff);
	m_program->write_byte(address + 2, (v >>  8) & 0x000000ff);
	m_program->write_byte(address + 3, (v >>  0) & 0x000000ff);
	return;
}

/*****************************************************************************/

DEFINE_DEVICE_TYPE(SSEMCPU, ssem_device, "ssem_cpu", "Manchester SSEM")

//-------------------------------------------------
//  ssem_device - constructor
//-------------------------------------------------

ssem_device::ssem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SSEMCPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16)
	, m_pc(1)
	, m_shifted_pc(1<<2)
	, m_a(0)
	, m_halt(0)
	, m_icount(0)
{
	// Allocate & setup
}


void ssem_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_halt).callimport().callexport().formatstr("%1s").noshow();
	state_add(SSEM_PC,         "PC",        m_shifted_pc).mask(0xffff);
	state_add(SSEM_A,          "A",         m_a).mask(0xffffffff);
	state_add(SSEM_HALT,       "HALT",     m_halt).mask(0xf);

	/* setup regtable */
	save_item(NAME(m_pc));
	save_item(NAME(m_a));
	save_item(NAME(m_halt));

	// set our instruction counter
	set_icountptr(m_icount);
}

void ssem_device::device_stop()
{
}

void ssem_device::device_reset()
{
	m_pc = 1;
	m_shifted_pc = m_pc << 2;
	m_a = 0;
	m_halt = 0;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector ssem_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void ssem_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c", m_halt ? 'H' : '.');
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> ssem_device::create_disassembler()
{
	return std::make_unique<ssem_disassembler>();
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t ssem_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t ssem_device::execute_max_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void ssem_device::execute_set_input(int inputnum, int state)
{
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void ssem_device::execute_run()
{
	uint32_t op;

	m_pc &= 0x1f;
	m_shifted_pc = m_pc << 2;

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		op = program_read32(m_pc);

		if( !m_halt )
		{
			m_pc++;
			m_shifted_pc = m_pc << 2;
		}
		else
		{
			op = 0x0000e000;
		}

		switch (INSTR)
		{
			case 0:
				// JMP: Move the value at the specified address into the Program Counter.
				m_pc = program_read32(ADDR) + 1;
				m_shifted_pc = m_pc << 2;
				break;
			case 1:
				// JRP: Add the value at the specified address to the Program Counter.
				m_pc += (int32_t)program_read32(ADDR);
				m_shifted_pc = m_pc << 2;
				break;
			case 2:
				// LDN: Load the accumulator with the two's-complement negation of the value at the specified address.
				m_a = (uint32_t)(0 - (int32_t)program_read32(ADDR));
				break;
			case 3:
				// STO: Store the value in the accumulator at the specified address.
				program_write32(ADDR, m_a);
				break;
			case 4:
			case 5:
				// SUB: Subtract the value at the specified address from the accumulator.
				m_a -= program_read32(ADDR);
				break;
			case 6:
				// CMP: If the accumulator is less than zero, skip the next opcode.
				if((int32_t)(m_a) < 0)
				{
					m_pc++;
					m_shifted_pc = m_pc << 2;
				}
				break;
			case 7:
				// STP: Halt the computer.
				m_halt = 1;
				break;
			default:
				break;
		}

		--m_icount;
	}
}
