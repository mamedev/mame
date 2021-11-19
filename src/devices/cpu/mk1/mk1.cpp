// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Mark 1 FORTH Computer TTL CPU

    This emulates the vertically microcoded CPU of Andrew Holme's Mark 1.
    Each instruction executes in one cycle of the quadrature clock. All
    data paths other than the address bus are 8 bits wide, even though
    16-bit words are the nominal basic data type.

    The instruction set is very rudimentary. ALU operations require the
    operands and function to be loaded in three separate steps. 0 is the
    only immediate operand that can be moved into a register. The only
    program transfer operations allowed by the microcode sequencer are
    conditional forward skips, direct jumps to within the first 16
    instructions and an indirect jump to one of 256 subroutines beginning
    on 16-word boundaries. The stack pointers can only be initialized by
    the hardware RESET signal.

    Though the CPU decodes microinstructions without the aid of any
    microprocessor, gate arrays, PLDs or PROMs, it does use a 7x16 diode
    matrix ROM to generate 74LS181 function codes for the ALU.

    The W and IP index registers are implemented on identical boards using
    four 74LS169 counters each. A jumper and LS157 selector are used to
    associate each board with the correct set of decode signals.

    The parameter and return stacks logically hold 256 16-bit words each,
    but the stack board actually implements them using a pair of dedicated
    byte-wide 6116 or 6264 static RAMs. This emulation uses a single
    address space for both stacks.

    The ALU's overflow checker and interrupt feature are not actually
    needed by the current microcode. They are emulated here for the sake of
    completeness.

***************************************************************************/

#include "emu.h"
#include "mk1.h"
#include "mk1dasm.h"


// device type definition
DEFINE_DEVICE_TYPE(MK1_CPU, mk1_cpu_device, "mk1_cpu", "Mark 1 CPU")

mk1_cpu_device::mk1_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, MK1_CPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_stack_config("stack", ENDIANNESS_LITTLE, 8, 10, 0)
	, m_pc(0)
	, m_inst(0)
	, m_op_latch(0)
	, m_index_reg{0, 0}
	, m_sp{0, 0}
	, m_alu_a(0)
	, m_alu_b(0)
	, m_alu_function(0b1111111)
	, m_alu_result(0)
	, m_cond_flags(0b1010)
	, m_irq_asserted(false)
	, m_irq_enabled(false)
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> mk1_cpu_device::create_disassembler()
{
	return std::make_unique<mk1_disassembler>();
}

device_memory_interface::space_config_vector mk1_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config),
		std::make_pair(AS_STACK, &m_stack_config)
	};
}

void mk1_cpu_device::device_start()
{
	// Hook address spaces
	space(AS_PROGRAM).cache(m_cache);
	space(AS_DATA).specific(m_data);
	space(AS_STACK).specific(m_stack);

	set_icountptr(m_icount);

	// Register debug state
	state_add(MK1_PC, "PC", m_pc).mask(0xfff);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow().mask(0xfff);
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow().mask(0xfff);
	state_add(STATE_GENFLAGS, "FLAGS", m_cond_flags).noshow().mask(0b1111).formatstr("%4s");
	state_add(MK1_OP, "OP", m_op_latch);
	state_add(MK1_W, "W", m_index_reg[0]);
	state_add(MK1_IP, "IP", m_index_reg[1]);
	state_add<u16>(MK1_TOS, "TOS",
		[this]() { auto dis = machine().disable_side_effects(); return m_stack.read_word(u16(m_sp[0]) << 1); },
		[this](u16 data) { auto dis = machine().disable_side_effects(); m_stack.write_word(u16(m_sp[0]) << 1, data); }
	);
	state_add<u16>(MK1_RS, "RS",
		[this]() { auto dis = machine().disable_side_effects(); return m_stack.read_word(0x200 | u16(m_sp[1]) << 1); },
		[this](u16 data) { auto dis = machine().disable_side_effects(); m_stack.write_word(0x200 | u16(m_sp[1]) << 1, data); }
	);
	state_add(MK1_PSP, "PSP", m_sp[0]);
	state_add(MK1_RSP, "RSP", m_sp[1]);
	state_add(MK1_A, "A", m_alu_a, [this](u8 data) { m_alu_a = data; alu_update(); });
	state_add(MK1_B, "B", m_alu_b, [this](u8 data) { m_alu_b = data; alu_update(); });
	state_add(MK1_ALU, "ALU", m_alu_function, [this](u8 data) { m_alu_function = data; alu_update(); }).mask(0b1111111).formatstr("%3s");
	state_add(MK1_F, "F", m_alu_result).readonly();
	state_add(MK1_IE, "IE", m_irq_enabled, [this](bool state) { set_irq_enable(state); });

	// Save internal state
	save_item(NAME(m_pc));
	save_item(NAME(m_inst));
	save_item(NAME(m_op_latch));
	save_item(NAME(m_index_reg));
	save_item(NAME(m_sp));
	save_item(NAME(m_alu_a));
	save_item(NAME(m_alu_b));
	save_item(NAME(m_alu_function));
	save_item(NAME(m_alu_result));
	save_item(NAME(m_cond_flags));
	save_item(NAME(m_irq_asserted));
	save_item(NAME(m_irq_enabled));
}

void mk1_cpu_device::device_reset()
{
	// Reset microprogram counter
	m_pc = 0;
	m_inst = 0;

	// Reset stack pointers
	m_sp[0] = m_sp[1] = 0;
}

// ALU functions decoded by 7x16 diode matrix ROM
const u8 mk1_cpu_device::s_alu_decode[16] =
{
	0b11101001, // ADD
	0b01101001, // ADC
	0b10100110, // SUB
	0b01100110, // SBB
	0b11101100, // ASL
	0b01101100, // ROL
	0b11101111,
	0b11101111,

	0b11111111, // A
	0b11111010, // B
	0b11111011, // AND
	0b11111110, // OR
	0b11010000, // NOT
	0b11110110, // XOR
	0b11111001, // A=B
	0b11111111
};

void mk1_cpu_device::alu_update()
{
	// Ultra-complete emulation of 74LS181 ALU functions (most not actually used here)
	bool carry = !BIT(m_alu_function, 6);
	switch (BIT(m_alu_function, 0, 4))
	{
	case 0b0000:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~m_alu_a;
		else
			m_alu_result = m_alu_a + (carry ? 1 : 0);
		carry = carry && m_alu_a == 0xff;
		break;

	case 0b0001:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~(m_alu_a | m_alu_b);
		else
			m_alu_result = (m_alu_a | m_alu_b) + (carry ? 1 : 0);
		carry = carry && (m_alu_a | m_alu_b) == 0xff;
		break;

	case 0b0010:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~m_alu_a & m_alu_b;
		else
			m_alu_result = (m_alu_a | ~m_alu_b) + (carry ? 1 : 0);
		carry = carry && (m_alu_a | ~m_alu_b) == 0xff;
		break;

	case 0b0011:
		m_alu_result = BIT(m_alu_function, 4) || carry ? 0 : -1;
		break;

	case 0b0100:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~(m_alu_a & m_alu_b);
		else
			m_alu_result = m_alu_a + (m_alu_a & ~m_alu_b) + (carry ? 1 : 0);
		carry = u16(m_alu_a) + u16(m_alu_a & ~m_alu_b) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b0101:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~m_alu_b;
		else
			m_alu_result = (m_alu_a | m_alu_b) + (m_alu_a & ~m_alu_b) + (carry ? 1 : 0);
		carry = u16(m_alu_a | m_alu_b) + u16(m_alu_a & ~m_alu_b) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b0110:
		if (BIT(m_alu_function, 4))
			m_alu_result = m_alu_a ^ m_alu_b;
		else
			m_alu_result = m_alu_a - m_alu_b - (carry ? 0 : 1);
		carry = m_alu_a >= m_alu_b + (carry ? 0 : 1);
		break;

	case 0b0111:
		m_alu_result = (m_alu_a & ~m_alu_b) - (BIT(m_alu_function, 4) || carry ? 0 : 1);
		carry = carry || (m_alu_a & ~m_alu_b) != 0;
		break;

	case 0b1000:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~m_alu_a | m_alu_b;
		else
			m_alu_result = m_alu_a + (m_alu_a & m_alu_b) + (carry ? 1 : 0);
		carry = u16(m_alu_a) + u16(m_alu_a & m_alu_b) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b1001:
		if (BIT(m_alu_function, 4))
			m_alu_result = ~(m_alu_a ^ m_alu_b);
		else
			m_alu_result = m_alu_a + m_alu_b + (carry ? 1 : 0);
		carry = u16(m_alu_a) + u16(m_alu_b) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b1010:
		if (BIT(m_alu_function, 4))
			m_alu_result = m_alu_b;
		else
			m_alu_result = (m_alu_a | ~m_alu_b) + (m_alu_a & m_alu_b) + (carry ? 1 : 0);
		carry = u16(m_alu_a | ~m_alu_b) + u16(m_alu_a & m_alu_b) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b1011:
		m_alu_result = (m_alu_a & m_alu_b) - (BIT(m_alu_function, 4) || carry ? 0 : 1);
		carry = carry || (m_alu_a & m_alu_b) != 0;
		break;

	case 0b1100:
		if (BIT(m_alu_function, 4))
			m_alu_result = 0xff;
		else
			m_alu_result = m_alu_a + m_alu_a + (carry ? 1 : 0);
		carry = u16(m_alu_a) + u16(m_alu_a) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b1101:
		if (BIT(m_alu_function, 4))
			m_alu_result = m_alu_a | ~m_alu_b;
		else
			m_alu_result = (m_alu_a | m_alu_b) + m_alu_a + (carry ? 1 : 0);
		carry = u16(m_alu_a | m_alu_b) + u16(m_alu_a) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b1110:
		if (BIT(m_alu_function, 4))
			m_alu_result = m_alu_a | m_alu_b;
		else
			m_alu_result = (m_alu_a | ~m_alu_b) + m_alu_a + (carry ? 1 : 0);
		carry = u16(m_alu_a | ~m_alu_b) + u16(m_alu_a) + (carry ? 1 : 0) >= 0x100;
		break;

	case 0b1111:
		m_alu_result = m_alu_a - (BIT(m_alu_function, 4) || carry ? 0 : 1);
		carry = carry || m_alu_a != 0;
		break;
	}

	// Update flags
	m_cond_flags = (m_cond_flags & 8) | (m_alu_result == 0xff ? 4 : 0) | (carry ? 0 : 2) | (BIT(m_alu_result, 7) ? 0 : 1);
	if (BIT(m_alu_function, 5) && BIT(m_alu_a ^ m_alu_b ^ m_alu_result, 7) == carry)
		m_cond_flags ^= 1;
}

void mk1_cpu_device::set_alu_function(u8 data)
{
	// D6 and D7 determine carry semantics
	// Previous carry flag is latched at this time; this may convert ADC to ADD, SBB to SUB or ROL to ASL
	if (data < 0x80)
		m_alu_function = (data & 0x3f) | (BIT(m_cond_flags, 1) ? 0x40 : 0x00);
	else
		m_alu_function = data & 0x7f;
	alu_update();
}

void mk1_cpu_device::set_irq_enable(bool state)
{
	// Clock bit into IRQ enable flip-flop
	m_irq_enabled = state;
	if (!state)
		m_cond_flags |= 8;
}

void mk1_cpu_device::execute_one()
{
	if (m_inst < 0x80)
	{
		// Fetch source for MOV
		u8 data;
		switch (BIT(m_inst, 3, 3))
		{
		case 0: case 1:
		{
			const u16 &index_reg = m_index_reg[BIT(m_inst, 3)];
			if (BIT(m_inst, 6))
				data = index_reg >> 8;
			else
				data = index_reg & 0x00ff;
			break;
		}

		case 2: case 3:
			data = m_stack.read_byte((BIT(m_inst, 3) ? 0x200 : 0) | u16(m_sp[BIT(m_inst, 3)]) << 1 | BIT(m_inst, 6));
			break;

		case 4: case 5:
			data = m_data.read_byte(m_index_reg[BIT(m_inst, 3)]);
			break;

		case 6: default:
			data = 0;
			break;

		case 7:
			data = m_alu_result;
			break;
		}

		// Move data to destination
		switch (BIT(m_inst, 0, 3))
		{
		case 0: case 1:
		{
			u16 &index_reg = m_index_reg[BIT(m_inst, 0)];
			if (BIT(m_inst, 6))
				index_reg = (index_reg & 0x00ff) | u16(data) << 8;
			else
				index_reg = (index_reg & 0xff00) | data;
			break;
		}

		case 2: case 3:
			m_stack.write_byte((BIT(m_inst, 0) ? 0x200 : 0) | u16(m_sp[BIT(m_inst, 0)]) << 1 | BIT(m_inst, 6), data);
			break;

		case 4:
			m_data.write_byte(m_index_reg[0], data);
			break;

		case 5:
			m_op_latch = data;
			break;

		case 6:
			m_alu_a = data;
			alu_update();
			break;

		case 7:
			m_alu_b = data;
			alu_update();
			break;
		}
	}
	else if (m_inst < 0x90)
	{
		if (BIT(m_inst, 2))
			set_irq_enable(BIT(m_inst, 3));
		else
		{
			// 8-bit or 16-bit increment or decrement
			if (BIT(m_inst, 1))
				m_sp[BIT(m_inst, 0)] += BIT(m_inst, 3) ? 1 : -1;
			else
				m_index_reg[BIT(m_inst, 0)] += BIT(m_inst, 3) ? 1 : -1;
		}
	}
	else if (m_inst < 0xa0)
	{
		// Jump direct to within first 16 bytes of microcode
		m_pc = BIT(m_inst, 0, 4);
	}
	else if (m_inst < 0xb0)
	{
		// ALU function specified by decode matrix
		set_alu_function(s_alu_decode[BIT(m_inst, 0, 4)]);
	}
	else if (m_inst < 0xc0)
	{
		// XOP clears the lower 4 bits of PC while loading the upper 8
		m_pc = u16(m_op_latch) << 4;
	}
}

void mk1_cpu_device::execute_run()
{
	do
	{
		if ((m_inst & 0xcf) > 0xc0 && !BIT(m_cond_flags, BIT(m_inst, 4, 2)))
		{
			// Lower half of microinstruction register becomes a synchronous down counter when a skip is taken
			--m_inst;
			m_pc = (m_pc + 1) & 0xfff;
		}
		else
		{
			debugger_instruction_hook(m_pc);

			// Fetch the next microinstruction and latch IRQ flag
			const bool was_skip = m_inst >= 0xc0;
			m_inst = m_cache.read_byte(m_pc);
			if (!was_skip && m_irq_enabled && m_inst >= 0xc0)
				m_cond_flags = (m_cond_flags & 7) | (m_irq_asserted ? 0 : 8);
			m_pc = (m_pc + 1) & 0xfff;

			execute_one();
		}
	} while (--m_icount > 0);
}

void mk1_cpu_device::execute_set_input(int linenum, int state)
{
	if (linenum == IRQ_LINE)
		m_irq_asserted = (state != CLEAR_LINE);
}

void mk1_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c",
			BIT(m_cond_flags, 3) ? '.' : 'I',
			BIT(m_cond_flags, 2) ? '=' : '.',
			BIT(m_cond_flags, 1) ? '.' : 'C',
			BIT(m_cond_flags, 0) ? '.' : (BIT(m_alu_function, 5) ? '<' : 'S')
		);
		break;

	case MK1_ALU:
		switch (m_alu_function)
		{
		case 0b1101001:
			str = "ADD";
			break;

		case 0b0101001:
			str = "ADC";
			break;

		case 0b0100110:
			str = "SUB";
			break;

		case 0b1100110:
			str = "SBB";
			break;

		case 0b1101100:
			str = "ASL";
			break;

		case 0b0101100:
			str = "ROL";
			break;

		case 0b1111111:
			str = "A  ";
			break;

		case 0b1011111:
			str = "0< ";
			break;

		case 0b1111010:
			str = "B  ";
			break;

		case 0b1111011:
			str = "AND";
			break;

		case 0b1111110:
			str = "OR ";
			break;

		case 0b1010000:
			str = "NOT";
			break;

		case 0b1110110:
			str = "XOR";
			break;

		case 0b1111001:
			str = "A=B";
			break;

		default:
			str = "???";
			break;
		}
		break;
	}
}
