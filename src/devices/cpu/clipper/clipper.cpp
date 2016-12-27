// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "debugger.h"
#include "clipper.h"

#define SSP_HACK 0

#define VERBOSE 0
#if VERBOSE
#define LOG_INTERRUPT(...) logerror(__VA_ARGS__)
#else
#define LOG_INTERRUPT(...)
#endif


const device_type CLIPPER = &device_creator<clipper_device>;

clipper_device::clipper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, CLIPPER, "c400", tag, owner, clock, "c400", __FILE__),
	m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0),
	m_program(nullptr),
	m_direct(nullptr),
	m_pc(0),
	m_icount(0),
	m_interrupt_cycles(0)
{
}

void clipper_device::device_start()
{
	// get our address spaces
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// set our instruction counter
	m_icountptr = &m_icount;

	//save_item(NAME(m_pc));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	state_add(CLIPPER_PC, "pc", m_pc).formatstr("%08X");
	state_add(CLIPPER_PSW, "psw", m_psw.d).formatstr("%08X");
	state_add(CLIPPER_SSW, "ssw", m_ssw.d).formatstr("%08X");

	state_add(CLIPPER_R0, "r0", m_r[m_ssw.bits.u][0]).formatstr("%08X");
	state_add(CLIPPER_R1, "r1", m_r[m_ssw.bits.u][1]).formatstr("%08X");
	state_add(CLIPPER_R2, "r2", m_r[m_ssw.bits.u][2]).formatstr("%08X");
	state_add(CLIPPER_R3, "r3", m_r[m_ssw.bits.u][3]).formatstr("%08X");
	state_add(CLIPPER_R4, "r4", m_r[m_ssw.bits.u][4]).formatstr("%08X");
	state_add(CLIPPER_R5, "r5", m_r[m_ssw.bits.u][5]).formatstr("%08X");
	state_add(CLIPPER_R6, "r6", m_r[m_ssw.bits.u][6]).formatstr("%08X");
	state_add(CLIPPER_R7, "r7", m_r[m_ssw.bits.u][7]).formatstr("%08X");
	state_add(CLIPPER_R8, "r8", m_r[m_ssw.bits.u][8]).formatstr("%08X");
	state_add(CLIPPER_R9, "r9", m_r[m_ssw.bits.u][9]).formatstr("%08X");
	state_add(CLIPPER_R10, "r10", m_r[m_ssw.bits.u][10]).formatstr("%08X");
	state_add(CLIPPER_R11, "r11", m_r[m_ssw.bits.u][11]).formatstr("%08X");
	state_add(CLIPPER_R12, "r12", m_r[m_ssw.bits.u][12]).formatstr("%08X");
	state_add(CLIPPER_R13, "r13", m_r[m_ssw.bits.u][13]).formatstr("%08X");
	state_add(CLIPPER_R14, "r14", m_r[m_ssw.bits.u][14]).formatstr("%08X");
	state_add(CLIPPER_R15, "r15", m_r[m_ssw.bits.u][15]).formatstr("%08X");

	state_add(CLIPPER_F0, "f0", m_f[0]).formatstr("%016X");
	state_add(CLIPPER_F1, "f1", m_f[1]).formatstr("%016X");
	state_add(CLIPPER_F2, "f2", m_f[2]).formatstr("%016X");
	state_add(CLIPPER_F3, "f3", m_f[3]).formatstr("%016X");
	state_add(CLIPPER_F4, "f4", m_f[4]).formatstr("%016X");
	state_add(CLIPPER_F5, "f5", m_f[5]).formatstr("%016X");
	state_add(CLIPPER_F6, "f6", m_f[6]).formatstr("%016X");
	state_add(CLIPPER_F7, "f7", m_f[7]).formatstr("%016X");
	state_add(CLIPPER_F8, "f8", m_f[8]).formatstr("%016X");
	state_add(CLIPPER_F9, "f9", m_f[9]).formatstr("%016X");
	state_add(CLIPPER_F10, "f10", m_f[10]).formatstr("%016X");
	state_add(CLIPPER_F11, "f11", m_f[11]).formatstr("%016X");
	state_add(CLIPPER_F12, "f12", m_f[12]).formatstr("%016X");
	state_add(CLIPPER_F13, "f13", m_f[13]).formatstr("%016X");
	state_add(CLIPPER_F14, "f14", m_f[14]).formatstr("%016X");
	state_add(CLIPPER_F15, "f15", m_f[15]).formatstr("%016X");
}

void clipper_device::device_reset()
{
	m_ssw.bits.m = 0; // FIXME: turn off mapping
	m_ssw.bits.u = 0;

	m_pc = 0x7F100000; // FIXME: start executing boot rom
	m_immediate_irq = 0;
}

void clipper_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	default:
		break;
	}
}

void clipper_device::execute_run()
{
	uint16_t insn;
	
	if (m_immediate_irq)
	{
		LOG_INTERRUPT("taking interrupt - current pc = %08x\n", m_pc);

		m_pc = intrap(0x800 + m_immediate_vector * 8, m_pc);
		m_immediate_irq = 0;
	}

	while (m_icount > 0) {

		debugger_instruction_hook(this, m_pc);

		// fetch instruction word
		insn = m_direct->read_word(m_pc + 0);

		// decode and execute instruction, return next pc
		m_pc = execute_instruction(insn);

		m_icount--;
	}
}

void clipper_device::execute_set_input(int inputnum, int state)
{
	if (state)
	{
		// clock is vector  0e29 0000 1110
		// floppy is vector 0621 0000 0110

		uint16_t vector = standard_irq_callback(inputnum);

		uint8_t ivec = vector & 0xff;
		uint8_t il = ivec >> 4;
		uint8_t in = ivec & 0xf;

		LOG_INTERRUPT("received interrupt with vector %04x\n", vector);

		// allow NMI or equal/higher priority interrupts
		if (ivec == 0 || (m_ssw.bits.ei && (il <= m_ssw.bits.il)))
		{
			m_immediate_irq = 1;
			m_immediate_vector = ivec;
			
			LOG_INTERRUPT("accepting interrupt %x\n", m_immediate_vector);
		}
	}
}

const address_space_config * clipper_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr;
}

#define R1 ((insn & 0x00F0) >> 4)
#define R2 (insn & 0x000F)
int clipper_device::execute_instruction(uint16_t insn)
{
	uint32_t next_pc = 0;
	union {
		int32_t imm; // used by instructions with immediate operand values
		uint32_t r2; // used by instructions with complex addressing modes
		uint16_t macro; // used by macro instructions
	} op;
	op.imm = op.r2 = op.macro = 0;
	uint32_t addr_ea = 0; // used by instructions with complex addressing modes

	// test for immediate, address or macro instructions and decode additional operands
	if ((insn & 0xF800) == 0x3800 || (insn & 0xD300) == 0x8300)
	{
		// immediate instructions
		if (insn & 0x0080)
		{
			// fetch 16 bit immediate and sign extend
			op.imm = (int16_t)m_direct->read_word(m_pc + 2);
			next_pc = m_pc + 4;
		}
		else
		{
			// fetch 32 bit immediate and sign extend
			op.imm = (int32_t)m_direct->read_dword(m_pc + 2);
			next_pc = m_pc + 6;
		}
	}
	else if ((insn & 0xF100) == 0x4100 || (insn & 0xE100) == 0x6100)
	{
		// instructions with complex addressing modes (b*, bf*, call, load*, stor*)
		uint16_t temp;

		switch (insn & 0x00F0)
		{
		case ADDR_MODE_PC32:
			op.r2 = R2;
			addr_ea = m_pc + (int32_t)m_direct->read_dword(m_pc + 2);
			next_pc = m_pc + 6;
			break;

		case ADDR_MODE_ABS32:
			op.r2 = R2;
			addr_ea = m_direct->read_dword(m_pc + 2);
			next_pc = m_pc + 6;
			break;

		case ADDR_MODE_REL32:
			op.r2 = m_direct->read_word(m_pc + 2) & 0xf;
			addr_ea = m_r[m_ssw.bits.u][R2] + (int32_t)m_direct->read_dword(m_pc + 4);
			next_pc = m_pc + 8;
			break;

		case ADDR_MODE_PC16:
			op.r2 = R2;
			addr_ea = m_pc + (int16_t)m_direct->read_word(m_pc + 2);
			next_pc = m_pc + 4;
			break;

		case ADDR_MODE_REL12:
			temp = m_direct->read_word(m_pc + 2);

			op.r2 = temp & 0xf;
			addr_ea = m_r[m_ssw.bits.u][R2] + ((int16_t)temp >> 4);
			next_pc = m_pc + 4;
			break;

		case ADDR_MODE_ABS16:
			op.r2 = R2;
			addr_ea = (int16_t)m_direct->read_word(m_pc + 2);
			next_pc = m_pc + 4;
			break;

		case ADDR_MODE_PCX:
			temp = m_direct->read_word(m_pc + 2);

			op.r2 = temp & 0xf;
			addr_ea = m_pc + m_r[m_ssw.bits.u][(temp >> 4) & 0xf];
			next_pc = m_pc + 4;
			break;

		case ADDR_MODE_RELX:
			temp = m_direct->read_word(m_pc + 2);

			op.r2 = temp & 0xf;
			addr_ea = m_r[m_ssw.bits.u][R2] + m_r[m_ssw.bits.u][(temp >> 4) & 0xf];
			next_pc = m_pc + 4;
			break;

		default:
			logerror("illegal addressing mode pc = 0x%08x\n", m_pc);
			machine().debug_break();
			break;
		}
	}
	else if ((insn & 0xFD00) == 0xB400)
	{
		// macro instructions
		op.macro = m_direct->read_word(m_pc + 2);
		next_pc = m_pc + 4;
	}
	else
		next_pc = m_pc + 2;

	switch (insn >> 8)
	{
	case 0x00: // noop
		break;

	case 0x10: 
		// movwp (treated as a noop if target ssw in user mode)
		// R1 == 3 means "fast" mode - avoids pipeline flush
		if (R1 == 0)
			m_psw.d = m_r[m_ssw.bits.u][R2];
		else if (m_ssw.bits.u == 0 && (R1 == 1 || R1 == 3))
			m_ssw.d = m_r[m_ssw.bits.u][R2];
		break;

	case 0x11: // movpw
		switch (R1)
		{
		case 0: m_r[m_ssw.bits.u][R2] = m_psw.d; break;
		case 1: m_r[m_ssw.bits.u][R2] = m_ssw.d; break;
		}
		break;

	case 0x12: // calls
		next_pc = intrap(0x400 + (insn & 0x7F) * 8, next_pc);
		break;

	case 0x13: // ret
		next_pc = m_program->read_dword(m_r[m_ssw.bits.u][R2]);
		m_r[m_ssw.bits.u][R2] += 4;
		break;

	case 0x14: // pushw
		m_r[m_ssw.bits.u][R1] -= 4;
		m_program->write_dword(m_r[m_ssw.bits.u][R1], m_r[m_ssw.bits.u][R2]);
		break;

	case 0x16: // popw
		m_r[m_ssw.bits.u][R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		m_r[m_ssw.bits.u][R1] += 4;
		break;

	case 0x20: *((float *)&m_f[R2]) -= *((float *)&m_f[R1]); break; // subs
	case 0x21: *((float *)&m_f[R2]) += *((float *)&m_f[R1]); break; // adds
	case 0x22: m_f[R2] += m_f[R1]; break; // addd
	case 0x23: m_f[R2] -= m_f[R1]; break; // subd
	case 0x24: *((float *)&m_f[R2]) = *((float *)&m_f[R1]); break; // movs
	case 0x25: evaluate_cc2f(*((float *)&m_f[R2]), *((float *)&m_f[R1])); break; // cmps
	case 0x26: m_f[R2] = m_f[R1]; break; // movd
	case 0x27: evaluate_cc2f(m_f[R2], m_f[R1]); break; // cmpd
	case 0x2A: m_f[R2] *= m_f[R1]; break; // muld
	case 0x2B: m_f[R2] /= m_f[R1]; break; // divd
	case 0x2C: m_r[m_ssw.bits.u][R2] = *((int32_t *)&m_f[R1]); break; // movsw
	case 0x2D: *((int32_t *)&m_f[R2]) = m_r[m_ssw.bits.u][R1]; break; // movws
	case 0x2E: ((double *)m_r[m_ssw.bits.u])[R2 >> 1] = m_f[R1]; break; // movdl
	case 0x2F: m_f[R2] = ((double *)m_r[m_ssw.bits.u])[R1 >> 1]; break; // movld

	case 0x30: // shaw
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] << m_r[m_ssw.bits.u][R1];
		else
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] >> -(int32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x31: // shal
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= m_r[m_ssw.bits.u][R1];
		else
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -(int32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x32: // shlw
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			m_r[m_ssw.bits.u][R2] <<= m_r[m_ssw.bits.u][R1];
		else
			m_r[m_ssw.bits.u][R2] >>= -(int32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x33: // shll
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= m_r[m_ssw.bits.u][R1];
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -(int32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x34: // rotw
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			m_r[m_ssw.bits.u][R2] = _rotl(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1]);
		else
			m_r[m_ssw.bits.u][R2] = _rotr(m_r[m_ssw.bits.u][R2], -(int32_t)m_r[m_ssw.bits.u][R1]);
		break;
	case 0x35: // rotl
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotl64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], m_r[m_ssw.bits.u][R1]);
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotr64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], -(int32_t)m_r[m_ssw.bits.u][R1]);
		break;
	case 0x38: // shai
		if (op.imm > 0)
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] << op.imm;
		else
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] >> -op.imm;
		break;
	case 0x39: // shali
		if (op.imm > 0)
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= op.imm;
		else
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -op.imm;
		break;
	case 0x3A: // shli
		if (op.imm > 0)
			m_r[m_ssw.bits.u][R2] <<= op.imm;
		else
			m_r[m_ssw.bits.u][R2] >>= -op.imm;
		break;
	case 0x3B: // shlli
		if (op.imm > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= op.imm;
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -op.imm;
		break;
	case 0x3C: // roti
		if (op.imm > 0)
			m_r[m_ssw.bits.u][R2] = _rotl(m_r[m_ssw.bits.u][R2], op.imm);
		else
			m_r[m_ssw.bits.u][R2] = _rotr(m_r[m_ssw.bits.u][R2], -op.imm);
		break;
	case 0x3D: // rotli
		if (op.imm > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotl64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], op.imm);
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotr64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], -op.imm);
		break;

	case 0x44: // call
		m_r[m_ssw.bits.u][R2] -= 4;				// decrement sp
		m_program->write_dword(m_r[m_ssw.bits.u][R2], next_pc);	// push return address
		next_pc = m_r[m_ssw.bits.u][R1];
		break;
	case 0x45: // call
		m_r[m_ssw.bits.u][op.r2] -= 4;				// decrement sp
		m_program->write_dword(m_r[m_ssw.bits.u][op.r2], next_pc);	// push return address
		next_pc = addr_ea;
		break;

	case 0x48: // b*
		if (evaluate_branch(R2))
			next_pc = m_r[m_ssw.bits.u][R1];
		break;
	case 0x49: // b*
		if (evaluate_branch(op.r2))
			next_pc = addr_ea;
		break;

	case 0x60: // loadw
		m_r[m_ssw.bits.u][R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x61: // loadw
		m_r[m_ssw.bits.u][op.r2] = m_program->read_dword(addr_ea);
		break;

	case 0x62: // loada
		m_r[m_ssw.bits.u][R2] = m_r[m_ssw.bits.u][R1];
		break;
	case 0x63:
		m_r[m_ssw.bits.u][op.r2] = addr_ea;
		break;

	case 0x64: // loads
		((uint64_t *)&m_f)[R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x65: // loads
		((uint64_t *)&m_f)[op.r2] = m_program->read_dword(addr_ea);
		break;

	case 0x66: // loadd
		((uint64_t *)&m_f)[R2] = m_program->read_qword(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x67: // loadd
		((uint64_t *)&m_f)[op.r2] = m_program->read_qword(addr_ea);
		break;

	case 0x68: // loadb
		m_r[m_ssw.bits.u][R2] = (int8_t)m_program->read_byte(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x69: // loadb
		m_r[m_ssw.bits.u][op.r2] = (int8_t)m_program->read_byte(addr_ea);
		break;

	case 0x6A: // loadbu
		m_r[m_ssw.bits.u][R2] = (uint8_t)m_program->read_byte(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x6B:
		m_r[m_ssw.bits.u][op.r2] = m_program->read_byte(addr_ea);
		break;

	case 0x6C: // loadh
		m_r[m_ssw.bits.u][R2] = (int16_t)m_program->read_word(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x6D:
		m_r[m_ssw.bits.u][op.r2] = (int16_t)m_program->read_word(addr_ea);
		break;

	case 0x6E: // loadhu
		m_r[m_ssw.bits.u][R2] = (uint16_t)m_program->read_word(m_r[m_ssw.bits.u][R1]);
		break;
	case 0x6F: // loadhu
		m_r[m_ssw.bits.u][op.r2] = m_program->read_word(addr_ea);
		break;

	case 0x70: // storw
		m_program->write_dword(m_r[m_ssw.bits.u][R1], m_r[m_ssw.bits.u][R2]);
		break;
	case 0x71:
		m_program->write_dword(addr_ea, m_r[m_ssw.bits.u][op.r2]);
		break;

	case 0x74: // stors
		m_program->write_dword(m_r[m_ssw.bits.u][R1], *((uint32_t *)&m_f[R2]));
		break;
	case 0x75: // stors
		m_program->write_dword(addr_ea, *((uint32_t *)&m_f[op.r2]));
		break;

	case 0x76: // stord
		m_program->write_qword(m_r[m_ssw.bits.u][R1], *((uint64_t *)&m_f[R2]));
		break;
	case 0x77: // stord
		m_program->write_qword(addr_ea, *((uint64_t *)&m_f[op.r2]));
		break;

	case 0x78: // storb
		m_program->write_byte(m_r[m_ssw.bits.u][R1], (uint8_t)m_r[m_ssw.bits.u][R2]);
		break;
	case 0x79:
		m_program->write_byte(addr_ea, (uint8_t)m_r[m_ssw.bits.u][op.r2]);
		break;

	case 0x7C: // storh
		m_program->write_word(m_r[m_ssw.bits.u][R1], (uint16_t)m_r[m_ssw.bits.u][R2]);
		break;
	case 0x7D:
		m_program->write_word(addr_ea, (uint16_t)m_r[m_ssw.bits.u][op.r2]);
		break;

	case 0x80: // addw
		m_r[m_ssw.bits.u][R2] += m_r[m_ssw.bits.u][R1];
		break;

	case 0x82: // addq
		m_r[m_ssw.bits.u][R2] += R1;
		break;
	case 0x83: // addi
		m_r[m_ssw.bits.u][R2] += op.imm;
		break;
	case 0x84: // movw
		m_r[m_ssw.bits.u][R2] = m_r[m_ssw.bits.u][R1];
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

	case 0x86: // loadq
		m_r[m_ssw.bits.u][R2] = R1;
		break;
	case 0x87: // loadi
		m_r[m_ssw.bits.u][R2] = op.imm;
		break;
	case 0x88: // andw
		m_r[m_ssw.bits.u][R2] &= m_r[m_ssw.bits.u][R1];
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

	case 0x8B: // andi
		m_r[m_ssw.bits.u][R2] &= op.imm;
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;
	case 0x8C: // orw
		m_r[m_ssw.bits.u][R2] |= m_r[m_ssw.bits.u][R1];
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

	case 0x8F: // ori
		m_r[m_ssw.bits.u][R2] |= op.imm;
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

#if 0
	case 0x90: // addwc
		m_r[m_ssw.bits.u][r2] += m_r[m_ssw.bits.u][r1] + psw.b.c;
		break;

	case 0x91: // subwc
		m_r[m_ssw.bits.u][r2] -= m_r[m_ssw.bits.u][r1] + psw.b.c;
		break;
#endif

	case 0x93: // negw
		m_r[m_ssw.bits.u][R2] = -(int32_t)m_r[m_ssw.bits.u][R1];
		break;

	case 0x98: // mulw
		m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] * (int32_t)m_r[m_ssw.bits.u][R1];
		break;
	// TODO: mulwx
	case 0x9A: // mulwu
		m_r[m_ssw.bits.u][R2] = (uint32_t)m_r[m_ssw.bits.u][R2] * (uint32_t)m_r[m_ssw.bits.u][R1];
		break;
	// TODO: mulwux
	case 0x9C: // divw
		m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] / (int32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x9D: // modw
		m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] % (int32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x9E: // divwu
		m_r[m_ssw.bits.u][R2] = (uint32_t)m_r[m_ssw.bits.u][R2] / (uint32_t)m_r[m_ssw.bits.u][R1];
		break;
	case 0x9F: // modwu
		m_r[m_ssw.bits.u][R2] = (uint32_t)m_r[m_ssw.bits.u][R2] % (uint32_t)m_r[m_ssw.bits.u][R1];
		break;

	case 0xA0: // subw
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1]);
		m_r[m_ssw.bits.u][R2] -= m_r[m_ssw.bits.u][R1];
		break;

	case 0xA2: // subq
		evaluate_cc2(m_r[m_ssw.bits.u][R2], R1);
		m_r[m_ssw.bits.u][R2] -= R1;
		break;
	case 0xA3: // subi
		evaluate_cc2(m_r[m_ssw.bits.u][R2], op.imm);
		m_r[m_ssw.bits.u][R2] -= op.imm;
		break;
	case 0xA4: // cmpw
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1]);
		break;

	case 0xA6: // cmpq
		evaluate_cc2(m_r[m_ssw.bits.u][R2], R1);
		break;
	case 0xA7: // cmpi
		evaluate_cc2(m_r[m_ssw.bits.u][R2], op.imm);
		break;
	case 0xA8: // xorw
		m_r[m_ssw.bits.u][R2] ^= m_r[m_ssw.bits.u][R1];
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

	case 0xAB: // xori
		m_r[m_ssw.bits.u][R2] ^= op.imm;
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;
	case 0xAC: // notw
		m_r[m_ssw.bits.u][R2] = ~m_r[m_ssw.bits.u][R1];
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

	case 0xAE: // notq
		m_r[m_ssw.bits.u][R2] = ~R1;
		evaluate_cc1(m_r[m_ssw.bits.u][R2]);
		break;

	case 0xB4:
		switch (insn & 0xFF)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C:
			// savew0..savew12: push registers from rN through r14

			// store ri at sp - 4 * (15 - i)
			for (int reg = R2; reg < 15; reg++)
				m_program->write_dword(m_r[m_ssw.bits.u][15] - 4 * (15 - reg), m_r[m_ssw.bits.u][reg]);

			// decrement sp after push to allow restart on exceptions
			m_r[m_ssw.bits.u][15] -= 4 * (15 - R2);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C:
			// restwN..restw12: pop registers from rN:r14

			// load ri from sp + 4 * (i - N)
			for (int reg = R2; reg < 15; reg++)
				m_r[m_ssw.bits.u][reg] = m_program->read_dword(m_r[m_ssw.bits.u][15] + 4 * (reg - R2));

			// increment sp after pop to allow restart on exceptions
			m_r[m_ssw.bits.u][15] += 4 * (15 - R2);
			break;

		case 0x36: // cnvtdw
			m_r[m_ssw.bits.u][op.macro & 0xf] = (int32_t)m_f[(op.macro >> 4) & 0xf];
			break;

		case 0x37: // cnvwd
			m_f[op.macro & 0xf] = (double)m_r[m_ssw.bits.u][(op.macro >> 4) & 0xf];
			break;

		default:
			logerror("illegal macro opcode at 0x%08x\n", m_pc);
			break;
		}

		break;

	case 0xB6:
		if (m_ssw.bits.u == 0)
		{
			switch (insn & 0xFF)
			{
			case 0x00: // movus
				m_r[0][op.macro & 0xf] = m_r[1][(op.macro >> 4) & 0xf];
				// setcc1(m_r[m_ssw.bits.u][r2]);
				break;

			case 0x01: // movsu
				m_r[1][op.macro & 0xf] = m_r[0][(op.macro >> 4) & 0xf];
				// setcc1(m_r[!m_ssw.bits.u][r2]);
				break;

			case 0x04: 
				// reti: restore psw, ssw and pc from supervisor stack
#if SSP_HACK
				// some user code increments sp before reti
				// HACK: if previous instruction is addq $12,sp then correct sp
				// FIXME: figure out the real cause of this stack adjustment
				if (m_program->read_word(m_pc - 2) == 0x82cf)
				{
					LOG_INTERRUPT("correcting ssp for reti at 0x%08x\n", m_pc);

					m_r[0][(op.macro >> 4) & 0xf] -= 12;
				}
#endif
				LOG_INTERRUPT("reti r%d, ssp = %08x, pc = %08x, next_pc = %08x\n",
					(op.macro >> 4) & 0xf, m_r[0][(op.macro >> 4) & 0xf], m_pc, m_program->read_dword(m_r[0][(op.macro >> 4) & 0xf] + 8));
				m_psw.d = m_program->read_dword(m_r[0][(op.macro >> 4) & 0xf] + 0);
				m_ssw.d = m_program->read_dword(m_r[0][(op.macro >> 4) & 0xf] + 4);
				next_pc = m_program->read_dword(m_r[0][(op.macro >> 4) & 0xf] + 8);

				m_r[0][(op.macro >> 4) & 0xf] += 12;
				break;

			default:
				// TODO: set psw.b.cts
				logerror("illegal opcode at 0x%08x\n", m_pc);
				next_pc = intrap(0x300, next_pc);	// illegal operation, illegal operation
				machine().debug_break();
				break;
			}
		}
		else
			// TODO: set psw.b.cts
			next_pc = intrap(0x308, next_pc);	// illegal operation, privileged instruction
		break;

	default:
		logerror("illegal opcode at 0x%08x\n", m_pc);
		next_pc = intrap(0x300, next_pc);	// illegal operation, illegal operation
		machine().debug_break();
		break;

	}

	return next_pc;
}

/*
* Sets up the PC, SSW and PSW to handle an exception.
*/
uint32_t clipper_device::intrap(uint32_t vector, uint32_t pc)
{
	uint32_t next_pc, pmode;

	LOG_INTERRUPT("intrap - vector %x, pc = 0x%08x, next_pc = 0x%08x, ssp = 0x%08x\n", vector, pc, m_program->read_dword(vector + 4), m_r[0][15]);

	// push pc, psw and ssw onto supervisor stack
	m_program->write_dword(m_r[0][15] - 4, pc);
	m_program->write_dword(m_r[0][15] - 12, m_psw.d); // TODO: set mts or cts bits in saved psw
	m_program->write_dword(m_r[0][15] - 8, m_ssw.d);

	// decrement supervisor stack pointer
#if SSP_HACK
	m_r[0][15] -= 12;									
#else
	m_r[0][15] -= 24;
#endif

	// load SSW from trap vector and set previous mode flag
	pmode = m_ssw.bits.u;
	m_ssw.d = m_program->read_dword(vector + 0);
	m_ssw.bits.p = pmode;

	// clear psw
	m_psw.d = 0;										

	// fetch target pc
	next_pc = m_program->read_dword(vector + 4);

#if SSP_HACK
	// some code immediately increments sp by 12 on entry into isr (causing overwrite of exception frame)
	// HACK: if target instruction is addq $12,sp then adjust sp
	// FIXME: figure out the real cause of this stack adjustment
	if (m_program->read_word(next_pc) == 0x82cf)
	{
		LOG_INTERRUPT("correcting ssp for trap handler at 0x%08x\n", next_pc);

		m_r[0][15] -= 12;
	}
#endif

	// return new pc from trap vector
	return next_pc;
}

void clipper_device::evaluate_cc1(int32_t v0)
{
	m_psw.bits.n = v0 < 0;
	m_psw.bits.z = v0 == 0;
}

/*
evaluation of overflow:
both +:  v0 - v1 > v0
v0- v1+: v0 - v1 > v0 = 0

both -:  v0 - v1 < v0
v0+ v1-: v0 - v1 < v0 = 0

overflow (addition): inputs have same sign and output has opposite
s1 == s2 && != s3
*/
void clipper_device::evaluate_cc2(int32_t v0, int32_t v1)
{
	m_psw.bits.n = (v0 - v1) >> 31;
	m_psw.bits.z = v0 == v1;
	m_psw.bits.v = ((v1 < 0) ? (v0 - v1 < v0) : (v0 - v1 > v0));
	m_psw.bits.c = ((uint32_t)v0 < (uint32_t)v1);
}

void clipper_device::evaluate_cc3(int32_t v0, int32_t v1, int32_t v2)
{
	m_psw.bits.n = v2 >> 31;
	m_psw.bits.z = v2 == 0;
	m_psw.bits.v = ((v0 > 0 && v1 > 0 && v2 < 0) || (v0 < 0 && v1 < 0 && v2 > 0));
	m_psw.bits.v = ((v1 < 0) ? (v0 - v1 < v0) : (v0 - v1 > v0));
	m_psw.bits.c = ((uint32_t)v0 < (uint32_t)v1);
}

void clipper_device::evaluate_cc2f(double v0, double v1)
{
	m_psw.bits.n = v0 < v1;
	m_psw.bits.z = v0 == v1;
	//	psw.b.v = ((v1 < 0) ? (v0 - v1 < v0) : (v0 - v1 > v0));
	//	psw.b.c = (fabs(v0) < fabs(v1));
	m_psw.bits.v = 0;
	m_psw.bits.c = 0;
}

bool clipper_device::evaluate_branch(uint32_t r2)
{
	switch (r2)
	{
	case BRANCH_T:
		return true;

	case BRANCH_GT:
		return !((m_psw.bits.n ^ m_psw.bits.v) | m_psw.bits.z);
	case BRANCH_GE:
		return !(m_psw.bits.n ^ m_psw.bits.v);
	case BRANCH_EQ:
		return m_psw.bits.z;

	case BRANCH_LT:
		return m_psw.bits.n ^ m_psw.bits.v;
	case BRANCH_LE:
		return (m_psw.bits.n ^ m_psw.bits.v) | m_psw.bits.z;
	case BRANCH_NE:
		return !m_psw.bits.z;

	case BRANCH_GTU:
		return !(m_psw.bits.c | m_psw.bits.z);
	case BRANCH_GEU:
		return !m_psw.bits.c;
	case BRANCH_LTU:
		return m_psw.bits.c;
	case BRANCH_LEU:
		return m_psw.bits.c | m_psw.bits.z;

	case BRANCH_V:
		return m_psw.bits.v;
	case BRANCH_NV:
		return !m_psw.bits.v;
	case BRANCH_N:
		return m_psw.bits.n;
	case BRANCH_NN:
		return !m_psw.bits.n;

	case BRANCH_FV:
		return false;
	}

	return false;
}

offs_t clipper_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE(clipper);
	return CPU_DISASSEMBLE_NAME(clipper)(this, stream, pc, oprom, opram, options);
}
