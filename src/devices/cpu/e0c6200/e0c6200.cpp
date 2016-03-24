// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

  References:
  - 1998 MF297-06a E0C6200/E0C6200A Core CPU Manual
  - 1998 MF1049-01a E0C6S46 Technical Manual

  E0C6200 is a CPU core used as the basis of many chips, it is not standalone.
  Seiko Epson often changed prefixes of their device names. Depending on when,
  the E0C6200 is known as SMC6200, E0C6200, S1C6200.

  TODO:
  - RLC is part of the r,q opcodes and requires that r == q, what happens otherwise?
  - documentation is conflicting on whether or not the zero flag is set on RLC/RRC

*/

#include "e0c6200.h"
#include "debugger.h"

#include "e0c6200op.inc"


// disasm
void e0c6200_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c",
				(m_f & I_FLAG) ? 'I':'i',
				(m_f & D_FLAG) ? 'D':'d',
				(m_f & Z_FLAG) ? 'Z':'z',
				(m_f & C_FLAG) ? 'C':'c'
			);
			break;

		default: break;
	}
}

offs_t e0c6200_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(e0c6200);
	return CPU_DISASSEMBLE_NAME(e0c6200)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	E0C6200_PC=1, E0C6200_A, E0C6200_B,
	E0C6200_XP, E0C6200_XH, E0C6200_XL,
	E0C6200_YP, E0C6200_YH, E0C6200_YL,
	E0C6200_SP
};

void e0c6200_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// zerofill
	m_op = 0;
	m_prev_op = 0;
	m_irq_vector = 0;
	m_irq_id = 0;
	m_possible_irq = false;
	m_halt = m_sleep = false;
	m_pc = 0;
	m_prev_pc = 0;
	m_npc = 0;
	m_jpc = 0;

	m_a = 0;
	m_b = 0;
	m_xp = m_xh = m_xl = 0;
	m_yp = m_yh = m_yl = 0;
	m_sp = 0;
	m_f = 0;

	// register for savestates
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_id));
	save_item(NAME(m_possible_irq));
	save_item(NAME(m_halt));
	save_item(NAME(m_sleep));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_npc));
	save_item(NAME(m_jpc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_xp));
	save_item(NAME(m_xh));
	save_item(NAME(m_xl));
	save_item(NAME(m_yp));
	save_item(NAME(m_yh));
	save_item(NAME(m_yl));
	save_item(NAME(m_sp));
	save_item(NAME(m_f));

	// register state for debugger
	state_add(E0C6200_PC, "PC", m_pc).formatstr("%04X");
	state_add(E0C6200_A,  "A",  m_a).formatstr("%01X");
	state_add(E0C6200_B,  "B",  m_b).formatstr("%01X");
	state_add(E0C6200_XP, "XP", m_xp).formatstr("%01X");
	state_add(E0C6200_XH, "XH", m_xh).formatstr("%01X");
	state_add(E0C6200_XL, "XL", m_xl).formatstr("%01X");
	state_add(E0C6200_YP, "YP", m_yp).formatstr("%01X");
	state_add(E0C6200_YH, "YH", m_yh).formatstr("%01X");
	state_add(E0C6200_YL, "YL", m_yl).formatstr("%01X");
	state_add(E0C6200_SP, "SP", m_sp).formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_f).formatstr("%4s").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e0c6200_cpu_device::device_reset()
{
	m_halt = m_sleep = false;
	m_op = 0xfff; // nop
	m_pc = 0x100;
	m_f &= 3; // decimal flag is 0 on 6200A, undefined on 6200
}



//-------------------------------------------------
//  execute loop
//-------------------------------------------------

void e0c6200_cpu_device::do_interrupt()
{
	// interrupt handling takes 13* cycles, plus 1 extra if cpu was halted
	// *: 12.5 on E0C6200A, does the cpu osc source change polarity or something?
	m_icount -= 13;
	if (m_halt)
		m_icount--;

	m_halt = m_sleep = false;
	push_pc();
	m_f &= ~I_FLAG;

	// page 1 of the current bank
	m_pc = (m_pc & 0x1000) | 0x100 | m_irq_vector;

	standard_irq_callback(m_irq_id);
}

void e0c6200_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// check/handle interrupt, but not right after EI or in the middle of a longjump
		if (m_possible_irq && (m_op & 0xfe0) != 0xe40 && (m_op & 0xff8) != 0xf48)
		{
			m_possible_irq = false;
			if (m_f & I_FLAG && check_interrupt())
			{
				do_interrupt();
				if (m_icount <= 0)
					break;
			}
		}

		// core cpu not running (peripherals still work)
		if (m_halt || m_sleep)
		{
			m_icount = 0;
			break;
		}

		// remember previous state, prepare pset-longjump
		m_prev_op = m_op;
		m_prev_pc = m_pc;
		m_jpc = ((m_prev_op & 0xfe0) == 0xe40) ? m_npc : (m_prev_pc & 0x1f00);

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_op = m_program->read_word(m_pc << 1) & 0xfff;
		m_pc = (m_pc & 0x1000) | ((m_pc + 1) & 0x0fff);

		// minimal opcode time is 5 clock cycles, opcodes take 5, 7, or 12 clock cycles
		m_icount -= 5;

		// handle opcode
		execute_one();
	}
}



//-------------------------------------------------
//  execute one
//-------------------------------------------------

void e0c6200_cpu_device::execute_one()
{
	// legend:
	// X   = --.XH.XL 8-bit
	// Y   = --.YH.YL 8-bit
	// IX  = XP.XH.XL 12-bit index register
	// IY  = YP.YH.YL 12-bit index register
	// MX  = data memory at IX
	// MY  = data memory at IY
	// Mn  = data memory at 0-F, via 4-bit immediate param
	// r/q = 2-bit param directing to A/B/MX/MY
	// i   = 4-bit immediate param
	// e   = 8-bit immediate param
	// s   = 8-bit immediate branch destination

	switch (m_op & 0xf00)
	{
		// JP s: jump unconditional
		case 0x000:
			do_branch();
			break;

		// JP C,s: jump if carry
		case 0x200:
			do_branch(m_f & C_FLAG);
			break;

		// JP NC,s: jump if no carry
		case 0x300:
			do_branch(~m_f & C_FLAG);
			break;

		// JP Z,s: jump if zero
		case 0x600:
			do_branch(m_f & Z_FLAG);
			break;

		// JP NZ,s: jump if not zero
		case 0x700:
			do_branch(~m_f & Z_FLAG);
			break;

		// CALL s: call unconditional (on current bank)
		case 0x400:
			m_icount -= 2;
			push_pc();
			m_pc = (m_pc & 0x1000) | (m_jpc & 0x0f00) | (m_op & 0xff);
			break;

		// CALZ s: call zero page (on current bank)
		case 0x500:
			m_icount -= 2;
			push_pc();
			m_pc = (m_pc & 0x1000) | (m_op & 0xff);
			break;

		// RETD e: return from subroutine, then LBPX MX,e
		case 0x100:
			m_icount -= 7;
			pop_pc();
			// fall through!

		// LBPX MX,e: load memory with 8-bit immediate data, increment X by 2
		case 0x900:
			write_mx(m_op & 0xf); inc_x();
			write_mx(m_op >> 4 & 0xf); inc_x();
			break;

		// LD X,e: load X with 8-bit immediate data
		case 0xb00:
			m_xh = m_op >> 4 & 0xf;
			m_xl = m_op & 0xf;
			break;

		// LD Y,e: load Y with 8-bit immediate data
		case 0x800:
			m_yh = m_op >> 4 & 0xf;
			m_yl = m_op & 0xf;
			break;


		default:
			switch (m_op)
			{
		// LD r,q: load register with register
		case 0xec0: /* m_a = m_a; */ break;
		case 0xec1: m_a = m_b; break;
		case 0xec2: m_a = read_mx(); break;
		case 0xec3: m_a = read_my(); break;
		case 0xec4: m_b = m_a; break;
		case 0xec5: /* m_b = m_b; */ break;
		case 0xec6: m_b = read_mx(); break;
		case 0xec7: m_b = read_my(); break;
		case 0xec8: write_mx(m_a); break;
		case 0xec9: write_mx(m_b); break;
		case 0xeca: write_mx(read_mx()); break;
		case 0xecb: write_mx(read_my()); break;
		case 0xecc: write_my(m_a); break;
		case 0xecd: write_my(m_b); break;
		case 0xece: write_my(read_mx()); break;
		case 0xecf: write_my(read_my()); break;

		// LDPX r,q: LD r,q, then increment X
		case 0xee0: /* m_a = m_a; */ inc_x(); break;
		case 0xee1: m_a = m_b; inc_x(); break;
		case 0xee2: m_a = read_mx(); inc_x(); break;
		case 0xee3: m_a = read_my(); inc_x(); break;
		case 0xee4: m_b = m_a; inc_x(); break;
		case 0xee5: /* m_b = m_b; */ inc_x(); break;
		case 0xee6: m_b = read_mx(); inc_x(); break;
		case 0xee7: m_b = read_my(); inc_x(); break;
		case 0xee8: write_mx(m_a); inc_x(); break;
		case 0xee9: write_mx(m_b); inc_x(); break;
		case 0xeea: write_mx(read_mx()); inc_x(); break;
		case 0xeeb: write_mx(read_my()); inc_x(); break;
		case 0xeec: write_my(m_a); inc_x(); break;
		case 0xeed: write_my(m_b); inc_x(); break;
		case 0xeee: write_my(read_mx()); inc_x(); break;
		case 0xeef: write_my(read_my()); inc_x(); break;

		// LDPY r,q: LD r,q, then increment Y
		case 0xef0: /* m_a = m_a; */ inc_y(); break;
		case 0xef1: m_a = m_b; inc_y(); break;
		case 0xef2: m_a = read_mx(); inc_y(); break;
		case 0xef3: m_a = read_my(); inc_y(); break;
		case 0xef4: m_b = m_a; inc_y(); break;
		case 0xef5: /* m_b = m_b; */ inc_y(); break;
		case 0xef6: m_b = read_mx(); inc_y(); break;
		case 0xef7: m_b = read_my(); inc_y(); break;
		case 0xef8: write_mx(m_a); inc_y(); break;
		case 0xef9: write_mx(m_b); inc_y(); break;
		case 0xefa: write_mx(read_mx()); inc_y(); break;
		case 0xefb: write_mx(read_my()); inc_y(); break;
		case 0xefc: write_my(m_a); inc_y(); break;
		case 0xefd: write_my(m_b); inc_y(); break;
		case 0xefe: write_my(read_mx()); inc_y(); break;
		case 0xeff: write_my(read_my()); inc_y(); break;

		// LD Xphl/Yphl,r: load IX/IY with register
		case 0xe80: m_xp = m_a; break;
		case 0xe81: m_xp = m_b; break;
		case 0xe82: m_xp = read_mx(); break;
		case 0xe83: m_xp = read_my(); break;
		case 0xe84: m_xh = m_a; break;
		case 0xe85: m_xh = m_b; break;
		case 0xe86: m_xh = read_mx(); break;
		case 0xe87: m_xh = read_my(); break;
		case 0xe88: m_xl = m_a; break;
		case 0xe89: m_xl = m_b; break;
		case 0xe8a: m_xl = read_mx(); break;
		case 0xe8b: m_xl = read_my(); break;
		case 0xe90: m_yp = m_a; break;
		case 0xe91: m_yp = m_b; break;
		case 0xe92: m_yp = read_mx(); break;
		case 0xe93: m_yp = read_my(); break;
		case 0xe94: m_yh = m_a; break;
		case 0xe95: m_yh = m_b; break;
		case 0xe96: m_yh = read_mx(); break;
		case 0xe97: m_yh = read_my(); break;
		case 0xe98: m_yl = m_a; break;
		case 0xe99: m_yl = m_b; break;
		case 0xe9a: m_yl = read_mx(); break;
		case 0xe9b: m_yl = read_my(); break;

		// LD r,Xphl/Yphl: load register with IX/IY
		case 0xea0: m_a = m_xp; break;
		case 0xea1: m_b = m_xp; break;
		case 0xea2: write_mx(m_xp); break;
		case 0xea3: write_my(m_xp); break;
		case 0xea4: m_a = m_xh; break;
		case 0xea5: m_b = m_xh; break;
		case 0xea6: write_mx(m_xh); break;
		case 0xea7: write_my(m_xh); break;
		case 0xea8: m_a = m_xl; break;
		case 0xea9: m_b = m_xl; break;
		case 0xeaa: write_mx(m_xl); break;
		case 0xeab: write_my(m_xl); break;
		case 0xeb0: m_a = m_yp; break;
		case 0xeb1: m_b = m_yp; break;
		case 0xeb2: write_mx(m_yp); break;
		case 0xeb3: write_my(m_yp); break;
		case 0xeb4: m_a = m_yh; break;
		case 0xeb5: m_b = m_yh; break;
		case 0xeb6: write_mx(m_yh); break;
		case 0xeb7: write_my(m_yh); break;
		case 0xeb8: m_a = m_yl; break;
		case 0xeb9: m_b = m_yl; break;
		case 0xeba: write_mx(m_yl); break;
		case 0xebb: write_my(m_yl); break;

		// LD SPhl,r: load stackpointer with register
		case 0xfe0: m_sp = (m_sp & 0xf0) | m_a; break;
		case 0xfe1: m_sp = (m_sp & 0xf0) | m_b; break;
		case 0xfe2: m_sp = (m_sp & 0xf0) | read_mx(); break;
		case 0xfe3: m_sp = (m_sp & 0xf0) | read_my(); break;
		case 0xff0: m_sp = (m_sp & 0x0f) | m_a << 4; break;
		case 0xff1: m_sp = (m_sp & 0x0f) | m_b << 4; break;
		case 0xff2: m_sp = (m_sp & 0x0f) | read_mx() << 4; break;
		case 0xff3: m_sp = (m_sp & 0x0f) | read_my() << 4; break;

		// LD r,SPhl: load register with stackpointer
		case 0xfe4: m_a = m_sp >> 4 & 0xf; break;
		case 0xfe5: m_b = m_sp >> 4 & 0xf; break;
		case 0xfe6: write_mx(m_sp >> 4 & 0xf); break;
		case 0xfe7: write_my(m_sp >> 4 & 0xf); break;
		case 0xff4: m_a = m_sp & 0xf; break;
		case 0xff5: m_b = m_sp & 0xf; break;
		case 0xff6: write_mx(m_sp & 0xf); break;
		case 0xff7: write_my(m_sp & 0xf); break;

		// ADD r,q: add register to register (flags: C, Z)
		case 0xa80: m_a = op_add(m_a, m_a, D_FLAG); break;
		case 0xa81: m_a = op_add(m_a, m_b, D_FLAG); break;
		case 0xa82: m_a = op_add(m_a, read_mx(), D_FLAG); break;
		case 0xa83: m_a = op_add(m_a, read_my(), D_FLAG); break;
		case 0xa84: m_b = op_add(m_b, m_a, D_FLAG); break;
		case 0xa85: m_b = op_add(m_b, m_b, D_FLAG); break;
		case 0xa86: m_b = op_add(m_b, read_mx(), D_FLAG); break;
		case 0xa87: m_b = op_add(m_b, read_my(), D_FLAG); break;
		case 0xa88: write_mx(op_add(read_mx(), m_a, D_FLAG)); break;
		case 0xa89: write_mx(op_add(read_mx(), m_b, D_FLAG)); break;
		case 0xa8a: write_mx(op_add(read_mx(), read_mx(), D_FLAG)); break;
		case 0xa8b: write_mx(op_add(read_mx(), read_my(), D_FLAG)); break;
		case 0xa8c: write_my(op_add(read_my(), m_a, D_FLAG)); break;
		case 0xa8d: write_my(op_add(read_my(), m_b, D_FLAG)); break;
		case 0xa8e: write_my(op_add(read_my(), read_mx(), D_FLAG)); break;
		case 0xa8f: write_my(op_add(read_my(), read_my(), D_FLAG)); break;

		// ADC r,q: add with carry register to register (flags: C, Z)
		case 0xa90: m_a = op_adc(m_a, m_a, D_FLAG); break;
		case 0xa91: m_a = op_adc(m_a, m_b, D_FLAG); break;
		case 0xa92: m_a = op_adc(m_a, read_mx(), D_FLAG); break;
		case 0xa93: m_a = op_adc(m_a, read_my(), D_FLAG); break;
		case 0xa94: m_b = op_adc(m_b, m_a, D_FLAG); break;
		case 0xa95: m_b = op_adc(m_b, m_b, D_FLAG); break;
		case 0xa96: m_b = op_adc(m_b, read_mx(), D_FLAG); break;
		case 0xa97: m_b = op_adc(m_b, read_my(), D_FLAG); break;
		case 0xa98: write_mx(op_adc(read_mx(), m_a, D_FLAG)); break;
		case 0xa99: write_mx(op_adc(read_mx(), m_b, D_FLAG)); break;
		case 0xa9a: write_mx(op_adc(read_mx(), read_mx(), D_FLAG)); break;
		case 0xa9b: write_mx(op_adc(read_mx(), read_my(), D_FLAG)); break;
		case 0xa9c: write_my(op_adc(read_my(), m_a, D_FLAG)); break;
		case 0xa9d: write_my(op_adc(read_my(), m_b, D_FLAG)); break;
		case 0xa9e: write_my(op_adc(read_my(), read_mx(), D_FLAG)); break;
		case 0xa9f: write_my(op_adc(read_my(), read_my(), D_FLAG)); break;

		// ACPX MX,r: ADC MX,r, then increment X (flags: C, Z)
		case 0xf28: write_mx(op_adc(read_mx(), m_a, D_FLAG)); inc_x(); break;
		case 0xf29: write_mx(op_adc(read_mx(), m_b, D_FLAG)); inc_x(); break;
		case 0xf2a: write_mx(op_adc(read_mx(), read_mx(), D_FLAG)); inc_x(); break;
		case 0xf2b: write_mx(op_adc(read_mx(), read_my(), D_FLAG)); inc_x(); break;

		// ACPY MY,r: ADC MY,r, then increment Y (flags: C, Z)
		case 0xf2c: write_my(op_adc(read_my(), m_a, D_FLAG)); inc_y(); break;
		case 0xf2d: write_my(op_adc(read_my(), m_b, D_FLAG)); inc_y(); break;
		case 0xf2e: write_my(op_adc(read_my(), read_mx(), D_FLAG)); inc_y(); break;
		case 0xf2f: write_my(op_adc(read_my(), read_my(), D_FLAG)); inc_y(); break;

		// SUB r,q: subtract register from register (flags: C, Z)
		case 0xaa0: m_a = op_sub(m_a, m_a, D_FLAG); break;
		case 0xaa1: m_a = op_sub(m_a, m_b, D_FLAG); break;
		case 0xaa2: m_a = op_sub(m_a, read_mx(), D_FLAG); break;
		case 0xaa3: m_a = op_sub(m_a, read_my(), D_FLAG); break;
		case 0xaa4: m_b = op_sub(m_b, m_a, D_FLAG); break;
		case 0xaa5: m_b = op_sub(m_b, m_b, D_FLAG); break;
		case 0xaa6: m_b = op_sub(m_b, read_mx(), D_FLAG); break;
		case 0xaa7: m_b = op_sub(m_b, read_my(), D_FLAG); break;
		case 0xaa8: write_mx(op_sub(read_mx(), m_a, D_FLAG)); break;
		case 0xaa9: write_mx(op_sub(read_mx(), m_b, D_FLAG)); break;
		case 0xaaa: write_mx(op_sub(read_mx(), read_mx(), D_FLAG)); break;
		case 0xaab: write_mx(op_sub(read_mx(), read_my(), D_FLAG)); break;
		case 0xaac: write_my(op_sub(read_my(), m_a, D_FLAG)); break;
		case 0xaad: write_my(op_sub(read_my(), m_b, D_FLAG)); break;
		case 0xaae: write_my(op_sub(read_my(), read_mx(), D_FLAG)); break;
		case 0xaaf: write_my(op_sub(read_my(), read_my(), D_FLAG)); break;

		// SBC r,q: subtract with carry register from register (flags: C, Z)
		case 0xab0: m_a = op_sbc(m_a, m_a, D_FLAG); break;
		case 0xab1: m_a = op_sbc(m_a, m_b, D_FLAG); break;
		case 0xab2: m_a = op_sbc(m_a, read_mx(), D_FLAG); break;
		case 0xab3: m_a = op_sbc(m_a, read_my(), D_FLAG); break;
		case 0xab4: m_b = op_sbc(m_b, m_a, D_FLAG); break;
		case 0xab5: m_b = op_sbc(m_b, m_b, D_FLAG); break;
		case 0xab6: m_b = op_sbc(m_b, read_mx(), D_FLAG); break;
		case 0xab7: m_b = op_sbc(m_b, read_my(), D_FLAG); break;
		case 0xab8: write_mx(op_sbc(read_mx(), m_a, D_FLAG)); break;
		case 0xab9: write_mx(op_sbc(read_mx(), m_b, D_FLAG)); break;
		case 0xaba: write_mx(op_sbc(read_mx(), read_mx(), D_FLAG)); break;
		case 0xabb: write_mx(op_sbc(read_mx(), read_my(), D_FLAG)); break;
		case 0xabc: write_my(op_sbc(read_my(), m_a, D_FLAG)); break;
		case 0xabd: write_my(op_sbc(read_my(), m_b, D_FLAG)); break;
		case 0xabe: write_my(op_sbc(read_my(), read_mx(), D_FLAG)); break;
		case 0xabf: write_my(op_sbc(read_my(), read_my(), D_FLAG)); break;

		// SCPX MX,r: SBC MX,r, then increment X (flags: C, Z)
		case 0xf38: write_mx(op_sbc(read_mx(), m_a, D_FLAG)); inc_x(); break;
		case 0xf39: write_mx(op_sbc(read_mx(), m_b, D_FLAG)); inc_x(); break;
		case 0xf3a: write_mx(op_sbc(read_mx(), read_mx(), D_FLAG)); inc_x(); break;
		case 0xf3b: write_mx(op_sbc(read_mx(), read_my(), D_FLAG)); inc_x(); break;

		// SCPY MY,r: SBC MY,r, then increment Y (flags: C, Z)
		case 0xf3c: write_my(op_sbc(read_my(), m_a, D_FLAG)); inc_y(); break;
		case 0xf3d: write_my(op_sbc(read_my(), m_b, D_FLAG)); inc_y(); break;
		case 0xf3e: write_my(op_sbc(read_my(), read_mx(), D_FLAG)); inc_y(); break;
		case 0xf3f: write_my(op_sbc(read_my(), read_my(), D_FLAG)); inc_y(); break;

		// CP r,q: compare: SUB r,q, but discard result (flags: C, Z, no D flag)
		case 0xf00: op_sub(m_a, m_a); break;
		case 0xf01: op_sub(m_a, m_b); break;
		case 0xf02: op_sub(m_a, read_mx()); break;
		case 0xf03: op_sub(m_a, read_my()); break;
		case 0xf04: op_sub(m_b, m_a); break;
		case 0xf05: op_sub(m_b, m_b); break;
		case 0xf06: op_sub(m_b, read_mx()); break;
		case 0xf07: op_sub(m_b, read_my()); break;
		case 0xf08: op_sub(read_mx(), m_a); break;
		case 0xf09: op_sub(read_mx(), m_b); break;
		case 0xf0a: op_sub(read_mx(), read_mx()); break;
		case 0xf0b: op_sub(read_mx(), read_my()); break;
		case 0xf0c: op_sub(read_my(), m_a); break;
		case 0xf0d: op_sub(read_my(), m_b); break;
		case 0xf0e: op_sub(read_my(), read_mx()); break;
		case 0xf0f: op_sub(read_my(), read_my()); break;

		// AND r,q: logical AND register with register (flags: Z)
		case 0xac0: m_a = op_and(m_a, m_a); break;
		case 0xac1: m_a = op_and(m_a, m_b); break;
		case 0xac2: m_a = op_and(m_a, read_mx()); break;
		case 0xac3: m_a = op_and(m_a, read_my()); break;
		case 0xac4: m_b = op_and(m_b, m_a); break;
		case 0xac5: m_b = op_and(m_b, m_b); break;
		case 0xac6: m_b = op_and(m_b, read_mx()); break;
		case 0xac7: m_b = op_and(m_b, read_my()); break;
		case 0xac8: write_mx(op_and(read_mx(), m_a)); break;
		case 0xac9: write_mx(op_and(read_mx(), m_b)); break;
		case 0xaca: write_mx(op_and(read_mx(), read_mx())); break;
		case 0xacb: write_mx(op_and(read_mx(), read_my())); break;
		case 0xacc: write_my(op_and(read_my(), m_a)); break;
		case 0xacd: write_my(op_and(read_my(), m_b)); break;
		case 0xace: write_my(op_and(read_my(), read_mx())); break;
		case 0xacf: write_my(op_and(read_my(), read_my())); break;

		// FAN r,q: flag-check: AND r,q, but discard result (flags: Z)
		case 0xf10: op_and(m_a, m_a); break;
		case 0xf11: op_and(m_a, m_b); break;
		case 0xf12: op_and(m_a, read_mx()); break;
		case 0xf13: op_and(m_a, read_my()); break;
		case 0xf14: op_and(m_b, m_a); break;
		case 0xf15: op_and(m_b, m_b); break;
		case 0xf16: op_and(m_b, read_mx()); break;
		case 0xf17: op_and(m_b, read_my()); break;
		case 0xf18: op_and(read_mx(), m_a); break;
		case 0xf19: op_and(read_mx(), m_b); break;
		case 0xf1a: op_and(read_mx(), read_mx()); break;
		case 0xf1b: op_and(read_mx(), read_my()); break;
		case 0xf1c: op_and(read_my(), m_a); break;
		case 0xf1d: op_and(read_my(), m_b); break;
		case 0xf1e: op_and(read_my(), read_mx()); break;
		case 0xf1f: op_and(read_my(), read_my()); break;

		// OR r,q: logical OR register with register (flags: Z)
		case 0xad0: m_a = op_or(m_a, m_a); break;
		case 0xad1: m_a = op_or(m_a, m_b); break;
		case 0xad2: m_a = op_or(m_a, read_mx()); break;
		case 0xad3: m_a = op_or(m_a, read_my()); break;
		case 0xad4: m_b = op_or(m_b, m_a); break;
		case 0xad5: m_b = op_or(m_b, m_b); break;
		case 0xad6: m_b = op_or(m_b, read_mx()); break;
		case 0xad7: m_b = op_or(m_b, read_my()); break;
		case 0xad8: write_mx(op_or(read_mx(), m_a)); break;
		case 0xad9: write_mx(op_or(read_mx(), m_b)); break;
		case 0xada: write_mx(op_or(read_mx(), read_mx())); break;
		case 0xadb: write_mx(op_or(read_mx(), read_my())); break;
		case 0xadc: write_my(op_or(read_my(), m_a)); break;
		case 0xadd: write_my(op_or(read_my(), m_b)); break;
		case 0xade: write_my(op_or(read_my(), read_mx())); break;
		case 0xadf: write_my(op_or(read_my(), read_my())); break;

		// XOR r,q: exclusive-OR register with register (flags: Z)
		case 0xae0: m_a = op_xor(m_a, m_a); break;
		case 0xae1: m_a = op_xor(m_a, m_b); break;
		case 0xae2: m_a = op_xor(m_a, read_mx()); break;
		case 0xae3: m_a = op_xor(m_a, read_my()); break;
		case 0xae4: m_b = op_xor(m_b, m_a); break;
		case 0xae5: m_b = op_xor(m_b, m_b); break;
		case 0xae6: m_b = op_xor(m_b, read_mx()); break;
		case 0xae7: m_b = op_xor(m_b, read_my()); break;
		case 0xae8: write_mx(op_xor(read_mx(), m_a)); break;
		case 0xae9: write_mx(op_xor(read_mx(), m_b)); break;
		case 0xaea: write_mx(op_xor(read_mx(), read_mx())); break;
		case 0xaeb: write_mx(op_xor(read_mx(), read_my())); break;
		case 0xaec: write_my(op_xor(read_my(), m_a)); break;
		case 0xaed: write_my(op_xor(read_my(), m_b)); break;
		case 0xaee: write_my(op_xor(read_my(), read_mx())); break;
		case 0xaef: write_my(op_xor(read_my(), read_my())); break;

		// RLC r(,r): rotate register left through carry (flags: C, Z)
		case 0xaf0: m_a = op_rlc(m_a); break;
		case 0xaf5: m_b = op_rlc(m_b); break;
		case 0xafa: read_mx(); write_mx(op_rlc(read_mx())); break;
		case 0xaff: read_my(); write_my(op_rlc(read_my())); break;

		// RRC r: rotate register right through carry (flags: C, Z)
		case 0xe8c: m_a = op_rrc(m_a); break;
		case 0xe8d: m_b = op_rrc(m_b); break;
		case 0xe8e: write_mx(op_rrc(read_mx())); break;
		case 0xe8f: write_my(op_rrc(read_my())); break;

		// INC SP: increment stackpointer
		case 0xfdb:
			m_sp++;
			break;

		// DEC SP: decrement stackpointer
		case 0xfcb:
			m_sp--;
			break;

		// PUSH r/Xphl/Yphl/F: push register to stack
		case 0xfc0: push(m_a); break;
		case 0xfc1: push(m_b); break;
		case 0xfc2: push(read_mx()); break;
		case 0xfc3: push(read_my()); break;
		case 0xfc4: push(m_xp); break;
		case 0xfc5: push(m_xh); break;
		case 0xfc6: push(m_xl); break;
		case 0xfc7: push(m_yp); break;
		case 0xfc8: push(m_yh); break;
		case 0xfc9: push(m_yl); break;
		case 0xfca: push(m_f); break;

		// POP r/Xphl/Yphl/F: pop value from stack
		case 0xfd0: m_a = pop(); break;
		case 0xfd1: m_b = pop(); break;
		case 0xfd2: write_mx(pop()); break;
		case 0xfd3: write_my(pop()); break;
		case 0xfd4: m_xp = pop(); break;
		case 0xfd5: m_xh = pop(); break;
		case 0xfd6: m_xl = pop(); break;
		case 0xfd7: m_yp = pop(); break;
		case 0xfd8: m_yh = pop(); break;
		case 0xfd9: m_yl = pop(); break;
		case 0xfda: m_f = pop(); m_possible_irq = true; break;

		// RETS: return from subroutine, then skip next instruction
		case 0xfde:
			m_icount -= 7;
			pop_pc();
			m_pc = (m_pc & 0x1000) | ((m_pc + 1) & 0x0fff);
			break;

		// RET: return from subroutine
		case 0xfdf:
			m_icount -= 2;
			pop_pc();
			break;

		// JPBA: jump indirect using registers A and B
		case 0xfe8:
			m_pc = m_jpc | m_b << 4 | m_a;
			break;

		// HALT: halt (stop cpu core clock)
		case 0xff8:
			m_halt = true;
			break;

		// SLP: sleep (stop source oscillation)
		case 0xff9:
			m_sleep = true;
			break;

		// NOP5: no operation (5 clock cycles)
		case 0xffb:
			break;

		// NOP7: no operation (7 clock cycles)
		case 0xfff:
			m_icount -= 2;
			break;


		default:
			switch (m_op & 0xff0)
			{
		// LD r,i: load register with 4-bit immediate data
		case 0xe00: m_a = m_op & 0xf; break;
		case 0xe10: m_b = m_op & 0xf; break;
		case 0xe20: write_mx(m_op & 0xf); break;
		case 0xe30: write_my(m_op & 0xf); break;

		// LDPX MX,i: LD MX,i, then increment X
		case 0xe60:
			write_mx(m_op & 0xf); inc_x();
			break;

		// LDPY MY,i: LD MY,i, then increment Y
		case 0xe70:
			write_my(m_op & 0xf); inc_y();
			break;

		// LD A,Mn: load A with memory
		case 0xfa0:
			m_a = read_mn();
			break;

		// LD B,Mn: load B with memory
		case 0xfb0:
			m_b = read_mn();
			break;

		// LD Mn,A: load memory with A
		case 0xf80:
			write_mn(m_a);
			break;

		// LD Mn,B: load memory with B
		case 0xf90:
			write_mn(m_b);
			break;

		// INC Mn: increment memory (flags: C, Z)
		case 0xf60:
			write_mn(op_inc(read_mn()));
			break;

		// DEC Mn: decrement memory (flags: C, Z)
		case 0xf70:
			write_mn(op_dec(read_mn()));
			break;

		// ADD r,i: add 4-bit immediate data to register (flags: C, Z)
		case 0xc00: m_a = op_add(m_a, m_op & 0xf, D_FLAG); break;
		case 0xc10: m_b = op_add(m_b, m_op & 0xf, D_FLAG); break;
		case 0xc20: write_mx(op_add(read_mx(), m_op & 0xf, D_FLAG)); break;
		case 0xc30: write_my(op_add(read_my(), m_op & 0xf, D_FLAG)); break;

		// ADC r,i: add with carry 4-bit immediate data to register (flags: C, Z)
		case 0xc40: m_a = op_adc(m_a, m_op & 0xf, D_FLAG); break;
		case 0xc50: m_b = op_adc(m_b, m_op & 0xf, D_FLAG); break;
		case 0xc60: write_mx(op_adc(read_mx(), m_op & 0xf, D_FLAG)); break;
		case 0xc70: write_my(op_adc(read_my(), m_op & 0xf, D_FLAG)); break;

		// ADC Xhl/Yhl,i: add with carry 4-bit immediate data to X/Y (flags: C, Z, no D flag)
		case 0xa00: m_xh = op_adc(m_xh, m_op & 0xf); break;
		case 0xa10: m_xl = op_adc(m_xl, m_op & 0xf); break;
		case 0xa20: m_yh = op_adc(m_yh, m_op & 0xf); break;
		case 0xa30: m_yl = op_adc(m_yl, m_op & 0xf); break;

		// SBC r,i: subtract with carry 4-bit immediate data from register (flags: C, Z)
		case 0xd40: m_a = op_sbc(m_a, m_op & 0xf, D_FLAG); break;
		case 0xd50: m_b = op_sbc(m_b, m_op & 0xf, D_FLAG); break;
		case 0xd60: write_mx(op_sbc(read_mx(), m_op & 0xf, D_FLAG)); break;
		case 0xd70: write_my(op_sbc(read_my(), m_op & 0xf, D_FLAG)); break;

		// CP r,i: compare: SUB r,i, but discard result (flags: C, Z, no D flag)
		case 0xdc0: op_sub(m_a, m_op & 0xf); break;
		case 0xdd0: op_sub(m_b, m_op & 0xf); break;
		case 0xde0: op_sub(read_mx(), m_op & 0xf); break;
		case 0xdf0: op_sub(read_my(), m_op & 0xf); break;

		// CP XH,i: compare: SUB Xhl/Yhl,i, but discard result (flags: C, Z, no D flag)
		case 0xa40: op_sub(m_xh, m_op & 0xf); break;
		case 0xa50: op_sub(m_xl, m_op & 0xf); break;
		case 0xa60: op_sub(m_yh, m_op & 0xf); break;
		case 0xa70: op_sub(m_yl, m_op & 0xf); break;

		// AND r,i: logical AND register with 4-bit immediate data (flags: Z)
		case 0xc80: m_a = op_and(m_a, m_op & 0xf); break;
		case 0xc90: m_b = op_and(m_b, m_op & 0xf); break;
		case 0xca0: write_mx(op_and(read_mx(), m_op & 0xf)); break;
		case 0xcb0: write_my(op_and(read_my(), m_op & 0xf)); break;

		// FAN r,i: flag-check: AND r,i, but discard result (flags: Z)
		case 0xd80: op_and(m_a, m_op & 0xf); break;
		case 0xd90: op_and(m_b, m_op & 0xf); break;
		case 0xda0: op_and(read_mx(), m_op & 0xf); break;
		case 0xdb0: op_and(read_my(), m_op & 0xf); break;

		// OR r,i: logical OR register with 4-bit immediate data (flags: Z)
		case 0xcc0: m_a = op_or(m_a, m_op & 0xf); break;
		case 0xcd0: m_b = op_or(m_b, m_op & 0xf); break;
		case 0xce0: write_mx(op_or(read_mx(), m_op & 0xf)); break;
		case 0xcf0: write_my(op_or(read_my(), m_op & 0xf)); break;

		// XOR r,i: exclusive-OR register with 4-bit immediate data (flags: Z)
		case 0xd00: m_a = op_xor(m_a, m_op & 0xf); break;
		case 0xd10: m_b = op_xor(m_b, m_op & 0xf); break;
		case 0xd20: write_mx(op_xor(read_mx(), m_op & 0xf)); break;
		case 0xd30: write_my(op_xor(read_my(), m_op & 0xf)); break;

		// SET F,i: set flag(s), this includes opcodes SCF, SZF, SDF, EI
		case 0xf40:
			m_icount -= 2;
			m_f |= (m_op & 0xf);
			m_possible_irq = true;
			break;

		// RST F,i: reset flag(s), this includes opcodes RCF, RZF, RDF, DI
		case 0xf50:
			m_icount -= 2;
			m_f &= (m_op & 0xf);
			break;

		// PSET p: page set, used to set page/bank before a jump instruction
		case 0xe40: case 0xe50:
			m_npc = m_op << 8 & 0x1f00;
			break;


		// illegal opcode
		default:
			logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc);
			break;

			} // 0xff0
			break;

			} // 0xfff
			break;

	} // 0xf00 (big switch)
}
