// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VT61 CPU

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "vt61.h"
#include "vt61dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(VT61_CPU, vt61_cpu_device, "vt61_cpu", "DEC VT61 CPU")

ALLOW_SAVE_TYPE(vt61_cpu_device::state);
ALLOW_SAVE_TYPE(vt61_cpu_device::pc_mode);

vt61_cpu_device::vt61_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, VT61_CPU, tag, owner, clock)
	, m_program_config("microprogram", ENDIANNESS_LITTLE, 16, 10, -1)
	, m_memory_config("memory", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_idr_config("idr", ENDIANNESS_LITTLE, 8, 6, 0)
	, m_time_state(state::T1)
	, m_pc(0)
	, m_pc_mode(pc_mode::CLR)
	, m_pv_flags(0)
	, m_ac(0)
	, m_mar(0)
	, m_md(0)
	, m_mdr(0)
	, m_ir(0)
	, m_sp{0}
	, m_icount(0)
	, m_misc_flags(0)
	, m_modem_flags(0)
	, m_intrpt_control(0)
{
	m_program_config.m_is_octal = true;
	m_memory_config.m_is_octal = true;
	m_idr_config.m_is_octal = true;
}

std::unique_ptr<util::disasm_interface> vt61_cpu_device::create_disassembler()
{
	return std::make_unique<vt61_disassembler>();
}

device_memory_interface::space_config_vector vt61_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_memory_config),
		std::make_pair(AS_IDR, &m_idr_config)
	};
}

void vt61_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_DATA).cache(m_memory_cache);
	space(AS_IDR).cache(m_idr_cache);

	set_icountptr(m_icount);

	state_add(VT61_PC, "PC", m_pc).mask(01777).formatstr("%5s");
	state_add(STATE_GENPC, "GENPC", m_pc).mask(01777).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(01777).noshow();
	state_add(STATE_GENFLAGS, "FLAGS", m_pv_flags).mask(7).noshow().formatstr("%3s");
	state_add(VT61_AC, "AC", m_ac).formatstr("%03O");
	state_add(VT61_MAR, "MAR", m_mar).formatstr("%06O");
	state_add<u8>(VT61_MALO, "MALO",
		[this]() { return m_mar & 000377; },
		[this](u8 data) { m_mar = (m_mar & 0177400) | data; }
	).noshow();
	state_add<u8>(VT61_MAHI, "MAHI",
		[this]() { return (m_mar & 0177400) >> 8; },
		[this](u8 data) { m_mar = (m_mar & 000377) | (data << 8); }
	).noshow();
	state_add(VT61_MD, "MD", m_md).formatstr("%03O");
	state_add(VT61_MDR, "MDR", m_mdr).formatstr("%03O");
	state_add(VT61_IR, "IR", m_ir).mask(6);
	for (int i = 0; i < 16; i++)
		state_add(VT61_R0 + i, string_format("R%d", i).c_str(), m_sp[i]).formatstr("%03O");
	state_add(VT61_MISC, "MISC", m_misc_flags).formatstr("%03O");
	state_add(VT61_MOD, "MOD", m_modem_flags).formatstr("%02O").mask(017);
	state_add(VT61_INTRC, "INTRC", m_intrpt_control).formatstr("%02O").mask(017);

	save_item(NAME(m_time_state));
	save_item(NAME(m_pc));
	save_item(NAME(m_pc_mode));
	save_item(NAME(m_pv_flags));
	save_item(NAME(m_ac));
	save_item(NAME(m_mar));
	save_item(NAME(m_md));
	save_item(NAME(m_mdr));
	save_item(NAME(m_ir));
	save_item(NAME(m_sp));
	save_item(NAME(m_misc_flags));
	save_item(NAME(m_modem_flags));
	save_item(NAME(m_intrpt_control));
}

void vt61_cpu_device::device_reset()
{
	m_time_state = state::T1;
	m_pc_mode = pc_mode::CLR;
	m_pc = 0;

	m_misc_flags = 0;
	m_modem_flags = 0;
	m_intrpt_control = 0;
}

u8 vt61_cpu_device::get_db(u16 i)
{
	// Source decode
	if ((i >= 0140000) || (i & 03000) == 03000)
	{
		// SEL ROM
		return i & 0377;
	}
	else if ((i & 03400) == 02400)
	{
		// SEL SPM
		u8 addr = bitswap<4>(i, 5, 4, 3, 7);
		if (BIT(i, 6))
			addr = (addr & 011) | (m_ir << 1);
		return m_sp[addr];
	}
	else switch (BIT(i, 7, 4))
	{
	case 0:
		// SEL AC
		return m_ac;

	case 1:
		// SEL STATUS 1 (TODO)
		return 0xff;

	case 2:
		// SEL SWITCHES (TODO)
		return 0xff;

	case 3:
		// CAS 1 (TODO)
		return 0xff;

	case 4:
		// CAS 2 (TODO)
		return 0xff;

	case 5:
		// SEL RAM
		return m_md;

	case 6:
		// SEL IDR
		return m_idr_cache.read_byte(m_md & 077);

	case 7:
		// SEL UART (TODO)
		return 0xff;

	case 9:
		// SEL TB SW (TODO)
		return 0xff;

	default:
		return 0xff;
	}
}

bool vt61_cpu_device::branch_mux(u8 cond)
{
	// "A" (0-15) and "B" (16-23) branch multiplexers
	switch (cond)
	{
	case 0:
		// INTR 1 (TODO: other conditions)
		return !BIT(m_misc_flags, 7) || BIT(m_modem_flags, 1);

	case 1:
		// NBX (TODO)
		return true;

	case 2:
		// NBR (TODO)
		return true;

	case 3:
		// F1
		return !BIT(m_intrpt_control, 2);

	case 4:
		// F2
		return !BIT(m_intrpt_control, 3);

	case 5:
		// C (TODO)
		return true;

	case 6:
		// UART R FLAG (TODO)
		return true;

	case 7:
		// UART T BUFF (TODO)
		return true;

	case 8:
		// DO X OFF (TODO)
		return true;

	case 9:
		// GOUT SER RQ (TODO)
		return true;

	case 10:
		// F3
		return !BIT(m_modem_flags, 1);

	case 11:
		// INTR 2 (TODO)
		return true;

	case 12:
		// KEY DOWN (TODO)
		return (m_ac != 050); // HACK: this forces what should be the C key down, which is somehow necessary at power-up

	case 13:
		// PWR UP
		return BIT(m_misc_flags, 7);

	case 14:
		// SYNC ENA (TODO)
		return true;

	case 15:
		// PARITY ERROR (TODO)
		return false;

	case 16:
		// MEM BZY (TODO)
		return false;

	case 17:
		// EQUAL
		return (m_pv_flags & EQUAL) != 0;

	case 18:
		// CARRY OUT
		return (m_pv_flags & CARRY_OUT) == 0;

	case 19:
		// MDR O.F. OUT
		return (m_pv_flags & MDR_OF_OUT) != 0;

	case 20:
		// COPIER SER REQ (TODO)
		return true;

	case 21:
		// C-PCF (TODO)
		return true;

	case 22:
		// TRUE
		return true;

	case 23:
		// VID SERV REQ (TODO)
		return true;

	default:
		logerror("%04o: Undefined branch condition\n", m_pc);
		return false;
	}
}

void vt61_cpu_device::operate_t5(u16 i)
{
	if (i >= 0140000)
	{
		if (branch_mux(BIT(i, 8, 5)) != BIT(i, 13))
			m_pc_mode = pc_mode::LD;
	}
	else if ((i & 0140000) == 040000)
	{
		// Destination decode
		switch (BIT(i, 11, 3))
		{
		case 0:
			// LD MA LO (MA HI is cleared)
			m_mar = get_db(i);
			break;

		case 1:
			// LD MA HI
			m_mar = u16(get_db(i)) << 8 | (m_mar & 0377);
			break;

		case 2:
			// LD PC
			m_pc_mode = pc_mode::LD;
			break;

		case 3:
			// LD UART (TODO)
			break;

		case 4:
			// CAS3 (TODO)
			break;

		case 5:
			// LD MDR
			m_mdr = get_db(i);
			break;

		case 6: case 7:
			// LD SP
			if ((i & 02400) == 02400)
				logerror("%04o: SP load from SP!?\n", m_pc);
			else
			{
				u8 addr = bitswap<4>(i, 5, 4, 3, 11);
				if (BIT(i, 6))
					addr = (addr & 011) | (m_ir << 1);
				m_sp[addr] = get_db(i);
			}
			break;
		}
	}
	else if ((i & 060000) == 0 && (BIT(i, 15) || !BIT(i, 12)))
	{
		u8 b = get_db(i);
		if (BIT(i, 15))
		{
			bool cin = BIT(i, 11) && !BIT(i, 12) && BIT(i, 2) && (m_pv_flags & CARRY_OUT) != 0;
			bool cout = false;
			u8 f = m_ac;
			switch (BIT(i, 12) ? 6 : i & 7)
			{
			case 0:
				// 74LS181: M = 1, Sn = 0000
				f = ~m_ac;
				break;

			case 1:
				// 74LS181: M = 1, Sn = 1110
				f = m_ac | b;
				break;

			case 2:
				// 74LS181: M = 1, Sn = 0110
				f = m_ac ^ b;
				break;

			case 3:
				// 74LS181: M = 1, Sn = 1011
				f = m_ac & b;
				break;

			case 4:
				// 74LS181: M = 0, Sn = 1111
				cout = !cin && m_ac == 0;
				f = m_ac - (cin ? 0 : 1);
				break;

			case 5:
				// 74LS181: M = 0, Sn = 1001
				cout = (m_ac + b + (cin ? 1 : 0)) >= 0400;
				f = m_ac + b + (cin ? 1 : 0);
				break;

			case 6:
				// 74LS181: M = 0, Sn = 0110
				cout = m_ac >= (b + (cin ? 0 : 1));
				f = m_ac - b - (cin ? 0 : 1);
				break;

			case 7:
				// 74LS181: M = 0, Sn = 1100
				cout = m_ac >= 0200;
				f = m_ac + m_ac + (cin ? 1 : 0);
				break;
			}
			m_pv_flags &= ~(CARRY_OUT | EQUAL);
			if (cout)
				m_pv_flags |= CARRY_OUT;
			if (f == 0377)
				m_pv_flags |= EQUAL;
			if (!BIT(i, 12))
				m_ac = f;
		}
		else
			m_ac = get_db(i);
	}
}

void vt61_cpu_device::operate_t8(u16 i)
{
	// Note that only dynamic RAM outputs are latched like this; static RAM and macrocode PROMs are not latched
	u8 next_md = m_memory_cache.read_byte(m_mar);

	bool op_ins = (i & 0160000) == 0120000;
	if (op_ins)
	{
		// "B" micro-operations on I/O board
		switch (BIT(i, 8, 4))
		{
		case 0:
			// No-op
			break;

		case 2:
			// RESET C (TODO)
			break;

		case 5:
			// LOAD MISC F
			m_misc_flags = m_ac;
			break;

		case 6:
			// LD LED (TODO)
			break;

		case 7:
			// LOAD MOD F
			m_modem_flags = m_ac & 017;
			break;

		case 8:
			// SET KEY (TODO)
			break;

		case 9:
			// LD INTRPT CONT
			m_intrpt_control = m_ac & 017;
			break;

		case 11:
			// CLR NBR (TODO)
			break;

		case 12:
			// LD SYNC + CLR NBX (TODO)
			break;

		case 13:
			// PLD (TODO)
			break;

		case 14:
			// CLR VID SER REQ (TODO)
			break;

		case 15:
			// ENABLE VID LD (TODO)
			break;
		}
	}

	if (op_ins || ((!BIT(i, 15) || (i & 070000) == 010000) && (i & 03000) != 03000))
	{
		// "A" micro-operations
		switch (i & 7)
		{
		case 0:
			// No-op
			break;

		case 1:
			// INC AC
			++m_ac;
			break;

		case 2:
			// LD IR
			m_ir = m_md >> 6;
			m_pc = (m_pc + 0400) & 01777;
			break;

		case 3:
			// INC MA
			++m_mar;
			break;

		case 4:
			// SHIFT MDR (TODO: serial input)
			if (BIT(m_mdr, 0))
				m_pv_flags |= MDR_OF_OUT;
			else
				m_pv_flags &= ~MDR_OF_OUT;
			m_mdr >>= 1;
			break;

		case 5:
			// CLR PC
			m_pc_mode = pc_mode::CLR;
			break;

		case 6:
			// COMP PC 8
			m_pc = (m_pc + 0400) & 01777;
			break;

		case 7:
			// LD RAM
			m_memory_cache.write_byte(m_mar, m_mdr);
			break;
		}
	}

	m_md = next_md;
	switch (m_pc_mode)
	{
	case pc_mode::CLR:
		m_pc = 0;
		break;

	case pc_mode::INC:
		m_pc = (m_pc & 01400) | ((m_pc + 1) & 0377);
		break;

	case pc_mode::LD:
		m_pc = (m_pc & 01400) | get_db(i);
		break;
	}
}

void vt61_cpu_device::execute_run()
{
	switch (m_time_state)
	{
		while (true)
		{
			[[fallthrough]];

		case state::T1:
			debugger_instruction_hook(m_pc);
			if (--m_icount <= 0)
			{
				m_time_state = state::T2;
				return;
			}
			[[fallthrough]];

		case state::T2:
			m_pc_mode = pc_mode::INC;
			if (--m_icount <= 0)
			{
				m_time_state = state::T3;
				return;
			}
			[[fallthrough]];

		case state::T3:
			if (--m_icount <= 0)
			{
				m_time_state = state::T4;
				return;
			}
			[[fallthrough]];

		case state::T4:
			if (--m_icount <= 0)
			{
				m_time_state = state::T5;
				return;
			}
			[[fallthrough]];

		case state::T5:
			operate_t5(m_program_cache.read_word(m_pc));
			if (--m_icount <= 0)
			{
				m_time_state = state::T6;
				return;
			}
			[[fallthrough]];

		case state::T6:
			if (--m_icount <= 0)
			{
				m_time_state = state::T7;
				return;
			}
			[[fallthrough]];

		case state::T7:
			if (--m_icount <= 0)
			{
				m_time_state = state::T8;
				return;
			}
			[[fallthrough]];

		case state::T8:
			operate_t8(m_program_cache.read_word(m_pc));
			if (--m_icount <= 0)
			{
				m_time_state = state::T9;
				return;
			}
			[[fallthrough]];

		case state::T9:
			if (--m_icount <= 0)
			{
				m_time_state = state::T10;
				return;
			}
			[[fallthrough]];

		case state::T10:
			if (--m_icount <= 0)
			{
				m_time_state = state::T1;
				return;
			}
		}
	}
}

void vt61_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case VT61_PC:
		str = string_format("%o:%03o", m_pc >> 8, m_pc & 0377);
		break;

	case STATE_GENFLAGS:
		str = string_format("%c%c%c",
							(m_pv_flags & MDR_OF_OUT) != 0 ? 'O' : '.',
							(m_pv_flags & EQUAL) != 0 ? '=' : '.',
							(m_pv_flags & CARRY_OUT) != 0 ? 'C' : '.');
		break;
	}
}
