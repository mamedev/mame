// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Primary source: http://bitsavers.trailing-edge.com/pdf/fairchild/clipper/Clipper_Instruction_Set_Oct85.pdf
 *
 * TODO:
 *   - save/restore state
 *   - tlb, mmu and cache
 *   - unimplemented instructions
 *   - c100, c300, c400 variants
 *   - boot logic
 */

#include "emu.h"
#include "debugger.h"
#include "clipper.h"

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
	state_add(STATE_GENSP, "GENSP", m_r[m_ssw.bits.u][15]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_psw.d).mask(0xf).formatstr("%4s").noshow();

	state_add(CLIPPER_PC, "pc", m_pc);
	state_add(CLIPPER_PSW, "psw", m_psw.d);
	state_add(CLIPPER_SSW, "ssw", m_ssw.d);

	state_add(CLIPPER_R0, "r0", m_r[m_ssw.bits.u][0]);
	state_add(CLIPPER_R1, "r1", m_r[m_ssw.bits.u][1]);
	state_add(CLIPPER_R2, "r2", m_r[m_ssw.bits.u][2]);
	state_add(CLIPPER_R3, "r3", m_r[m_ssw.bits.u][3]);
	state_add(CLIPPER_R4, "r4", m_r[m_ssw.bits.u][4]);
	state_add(CLIPPER_R5, "r5", m_r[m_ssw.bits.u][5]);
	state_add(CLIPPER_R6, "r6", m_r[m_ssw.bits.u][6]);
	state_add(CLIPPER_R7, "r7", m_r[m_ssw.bits.u][7]);
	state_add(CLIPPER_R8, "r8", m_r[m_ssw.bits.u][8]);
	state_add(CLIPPER_R9, "r9", m_r[m_ssw.bits.u][9]);
	state_add(CLIPPER_R10, "r10", m_r[m_ssw.bits.u][10]);
	state_add(CLIPPER_R11, "r11", m_r[m_ssw.bits.u][11]);
	state_add(CLIPPER_R12, "r12", m_r[m_ssw.bits.u][12]);
	state_add(CLIPPER_R13, "r13", m_r[m_ssw.bits.u][13]);
	state_add(CLIPPER_R14, "r14", m_r[m_ssw.bits.u][14]);
	state_add(CLIPPER_R15, "r15", m_r[m_ssw.bits.u][15]);

	state_add(CLIPPER_F0, "f0", m_f[0]);
	state_add(CLIPPER_F1, "f1", m_f[1]);
	state_add(CLIPPER_F2, "f2", m_f[2]);
	state_add(CLIPPER_F3, "f3", m_f[3]);
	state_add(CLIPPER_F4, "f4", m_f[4]);
	state_add(CLIPPER_F5, "f5", m_f[5]);
	state_add(CLIPPER_F6, "f6", m_f[6]);
	state_add(CLIPPER_F7, "f7", m_f[7]);
	state_add(CLIPPER_F8, "f8", m_f[8]);
	state_add(CLIPPER_F9, "f9", m_f[9]);
	state_add(CLIPPER_F10, "f10", m_f[10]);
	state_add(CLIPPER_F11, "f11", m_f[11]);
	state_add(CLIPPER_F12, "f12", m_f[12]);
	state_add(CLIPPER_F13, "f13", m_f[13]);
	state_add(CLIPPER_F14, "f14", m_f[14]);
	state_add(CLIPPER_F15, "f15", m_f[15]);
}

void clipper_device::device_reset()
{
	m_ssw.bits.m = 0; // FIXME: turn off mapping
	m_ssw.bits.u = 0;

	m_pc = 0x7f100000; // FIXME: start executing boot rom
	m_immediate_irq = 0;
}

void clipper_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c", 
			m_psw.bits.c ? 'C' : '.',
			m_psw.bits.v ? 'V' : '.',
			m_psw.bits.z ? 'Z' : '.',
			m_psw.bits.n ? 'N' : '.');
		break;
	}
}

void clipper_device::execute_run()
{
	uint16_t insn;
	
	if (m_immediate_irq)
	{
		LOG_INTERRUPT("taking interrupt - current pc = %08x\n", m_pc);

		m_pc = intrap(EXCEPTION_INTERRUPT_BASE + m_immediate_vector * 8, m_pc);
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

#define R1 ((insn & 0x00f0) >> 4)
#define R2 (insn & 0x000f)

void clipper_device::decode_instruction (uint16_t insn)
{
	// initialise the decoding results
	m_info.op.imm = m_info.op.r2 = m_info.op.macro = 0;
	m_info.size = 0;
	m_info.address = 0;

	if ((insn & 0xf800) == 0x3800 || (insn & 0xd300) == 0x8300)
	{
		// instruction has an immediate operand, either 16 or 32 bit
		if (insn & 0x0080)
		{
			// fetch 16 bit immediate and sign extend
			m_info.op.imm = (int16_t)m_direct->read_word(m_pc + 2);
			m_info.size = 4;
		}
		else
		{
			// fetch 32 bit immediate and sign extend
			m_info.op.imm = (int32_t)m_direct->read_dword(m_pc + 2);
			m_info.size = 6;
		}
	}
	else if ((insn & 0xf100) == 0x4100 || (insn & 0xe100) == 0x6100)
	{
		// instruction has a complex addressing mode (b*, bf*, call, load*, stor*)
		// decode the operands and compute the effective address
		uint16_t temp;

		switch (insn & 0x00f0)
		{
		case ADDR_MODE_PC32:
			m_info.op.r2 = R2;
			m_info.address = m_pc + (int32_t)m_direct->read_dword(m_pc + 2);
			m_info.size = 6;
			break;

		case ADDR_MODE_ABS32:
			m_info.op.r2 = R2;
			m_info.address = m_direct->read_dword(m_pc + 2);
			m_info.size = 6;
			break;

		case ADDR_MODE_REL32:
			m_info.op.r2 = m_direct->read_word(m_pc + 2) & 0xf;
			m_info.address = m_r[m_ssw.bits.u][R2] + (int32_t)m_direct->read_dword(m_pc + 4);
			m_info.size = 8;
			break;

		case ADDR_MODE_PC16:
			m_info.op.r2 = R2;
			m_info.address = m_pc + (int16_t)m_direct->read_word(m_pc + 2);
			m_info.size = 4;
			break;

		case ADDR_MODE_REL12:
			temp = m_direct->read_word(m_pc + 2);

			m_info.op.r2 = temp & 0xf;
			m_info.address = m_r[m_ssw.bits.u][R2] + ((int16_t)temp >> 4);
			m_info.size = 4;
			break;

		case ADDR_MODE_ABS16:
			m_info.op.r2 = R2;
			m_info.address = (int16_t)m_direct->read_word(m_pc + 2);
			m_info.size = 4;
			break;

		case ADDR_MODE_PCX:
			temp = m_direct->read_word(m_pc + 2);

			m_info.op.r2 = temp & 0xf;
			m_info.address = m_pc + m_r[m_ssw.bits.u][(temp >> 4) & 0xf];
			m_info.size = 4;
			break;

		case ADDR_MODE_RELX:
			temp = m_direct->read_word(m_pc + 2);

			m_info.op.r2 = temp & 0xf;
			m_info.address = m_r[m_ssw.bits.u][R2] + m_r[m_ssw.bits.u][(temp >> 4) & 0xf];
			m_info.size = 4;
			break;

		default:
			logerror("illegal addressing mode pc = 0x%08x\n", m_pc);
			machine().debug_break();
			break;
		}
	}
	else if ((insn & 0xfd00) == 0xb400)
	{
		// macro instructions
		m_info.op.macro = m_direct->read_word(m_pc + 2);
		m_info.size = 4;
	}
	else
		// all other instruction formats are 16 bits
		m_info.size = 2;
}

int clipper_device::execute_instruction (uint16_t insn)
{
	// the address of the next instruction
	uint32_t next_pc;

	// handle complex instruction formats and addressing modes
	decode_instruction(insn);

	// next instruction follows the current one by default, but
	// may be changed for branch, call or trap instructions
	next_pc = m_pc + m_info.size;

	switch (insn >> 8)
	{
	case 0x00: // noop
		break;

	case 0x10: 
		// movwp: move word to processor register
		// treated as a noop if target ssw in user mode
		// R1 == 3 means "fast" mode - avoids pipeline flush
		if (R1 == 0)
			m_psw.d = m_r[m_ssw.bits.u][R2];
		else if (m_ssw.bits.u == 0 && (R1 == 1 || R1 == 3))
			m_ssw.d = m_r[m_ssw.bits.u][R2];
		// FLAGS: CVZN
		break;
	case 0x11: 
		// movpw: move processor register to word
		switch (R1)
		{
		case 0: m_r[m_ssw.bits.u][R2] = m_psw.d; break;
		case 1: m_r[m_ssw.bits.u][R2] = m_ssw.d; break;
		}
		break;
	case 0x12: 
		// calls: call supervisor
		next_pc = intrap(EXCEPTION_SUPERVISOR_CALL_BASE + (insn & 0x7f) * 8, next_pc);
		break;
	case 0x13: 
		// ret: return from subroutine
		next_pc = m_program->read_dword(m_r[m_ssw.bits.u][R2]);
		m_r[m_ssw.bits.u][R2] += 4;
		// TRAPS: C,U,A,P,R
		break;
	case 0x14: 
		// pushw: push word
		m_r[m_ssw.bits.u][R1] -= 4;
		m_program->write_dword(m_r[m_ssw.bits.u][R1], m_r[m_ssw.bits.u][R2]);
		// TRAPS: A,P,W
		break;

	case 0x16: 
		// popw: pop word
		m_r[m_ssw.bits.u][R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		m_r[m_ssw.bits.u][R1] += 4;
		// TRAPS: C,U,A,P,R
		break;

	case 0x20:
		// adds: add single floating
		*((float *)&m_f[R2]) += *((float *)&m_f[R1]);
		// TRAPS: F_IVUX
		break;
	case 0x21:
		// subs: subtract single floating
		*((float *)&m_f[R2]) -= *((float *)&m_f[R1]); 
		// TRAPS: F_IVUX
		break;
	case 0x22:
		// addd: add double floating
		m_f[R2] += m_f[R1]; 
		// TRAPS: F_IVUX
		break;
	case 0x23:
		// subd: subtract double floating
		m_f[R2] -= m_f[R1]; 
		// TRAPS: F_IVUX
		break;
	case 0x24:
		// movs: move single floating
		*((float *)&m_f[R2]) = *((float *)&m_f[R1]); 
		break;
	case 0x25:
		// cmps: compare single floating
		evaluate_cc2f(*((float *)&m_f[R2]), *((float *)&m_f[R1])); 
		break;
	case 0x26:
		// movd: move double floating
		m_f[R2] = m_f[R1]; 
		break;
	case 0x27:
		// cmpd: compare double floating
		evaluate_cc2f(m_f[R2], m_f[R1]); 
		// FLAGS: 00ZN
		break;
	case 0x28:
		// muls: multiply single floating
		*((float *)&m_f[R2]) *= *((float *)&m_f[R1]);
		// TRAPS: F_IVUX
		break;
	case 0x29:
		// divs: divide single floating
		*((float *)&m_f[R2]) /= *((float *)&m_f[R1]);
		// TRAPS: F_IVDUX
		break;
	case 0x2a:
		// muld: multiply double floating
		m_f[R2] *= m_f[R1]; 
		// TRAPS: F_IVUX
		break;
	case 0x2b:
		// divd: divide double floating
		m_f[R2] /= m_f[R1];
		// TRAPS: F_IVDUX
		break;
	case 0x2c:
		// movsw: move single floating to word
		m_r[m_ssw.bits.u][R2] = *((int32_t *)&m_f[R1]); 
		break;
	case 0x2d:
		// movws: move word to single floating
		*((int32_t *)&m_f[R2]) = m_r[m_ssw.bits.u][R1]; 
		break;
	case 0x2e:
		// movdl: move double floating to longword
		((double *)m_r[m_ssw.bits.u])[R2 >> 1] = m_f[R1]; 
		break;
	case 0x2f:
		// movld: move longword to double floating
		m_f[R2] = ((double *)m_r[m_ssw.bits.u])[R1 >> 1]; 
		break;

	case 0x30: 
		// shaw: shift arithmetic word
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] << m_r[m_ssw.bits.u][R1];
		else
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] >> -(int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0VZN
		break;
	case 0x31: 
		// shal: shift arithmetic longword
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= m_r[m_ssw.bits.u][R1];
		else
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -(int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0VZN
		break;
	case 0x32: 
		// shlw: shift logical word
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			m_r[m_ssw.bits.u][R2] <<= m_r[m_ssw.bits.u][R1];
		else
			m_r[m_ssw.bits.u][R2] >>= -(int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 00ZN
		break;
	case 0x33: 
		// shll: shift logical longword
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= m_r[m_ssw.bits.u][R1];
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -(int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 00ZN
		break;
	case 0x34: 
		// rotw: rotate word
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			m_r[m_ssw.bits.u][R2] = _rotl(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1]);
		else
			m_r[m_ssw.bits.u][R2] = _rotr(m_r[m_ssw.bits.u][R2], -(int32_t)m_r[m_ssw.bits.u][R1]);
		// FLAGS: 00ZN
		break;
	case 0x35: 
		// rotl: rotate longword
		if ((int32_t)m_r[m_ssw.bits.u][R1] > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotl64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], m_r[m_ssw.bits.u][R1]);
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotr64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], -(int32_t)m_r[m_ssw.bits.u][R1]);
		// FLAGS: 00ZN
		break;

	case 0x38: 
		// shai: shift arithmetic immediate
		if (m_info.op.imm > 0)
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] << m_info.op.imm;
		else
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] >> -m_info.op.imm;
		// FLAGS: 0VZN
		// TRAPS: I
		break;
	case 0x39: 
		// shali: shift arithmetic longword immediate
		if (m_info.op.imm > 0)
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= m_info.op.imm;
		else
			((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -m_info.op.imm;
		// FLAGS: 0VZN
		// TRAPS: I
		break;
	case 0x3a: 
		// shli: shift logical immediate
		if (m_info.op.imm > 0)
			m_r[m_ssw.bits.u][R2] <<= m_info.op.imm;
		else
			m_r[m_ssw.bits.u][R2] >>= -m_info.op.imm;
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x3b: 
		// shlli: shift logical longword immediate
		if (m_info.op.imm > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] <<= m_info.op.imm;
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] >>= -m_info.op.imm;
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x3c: 
		// roti: rotate immediate
		if (m_info.op.imm > 0)
			m_r[m_ssw.bits.u][R2] = _rotl(m_r[m_ssw.bits.u][R2], m_info.op.imm);
		else
			m_r[m_ssw.bits.u][R2] = _rotr(m_r[m_ssw.bits.u][R2], -m_info.op.imm);
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x3d: 
		// rotli: rotate longword immediate
		if (m_info.op.imm > 0)
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotl64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], m_info.op.imm);
		else
			((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = _rotr64(((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1], -m_info.op.imm);
		// FLAGS: 00ZN
		// TRAPS: I
		break;

	case 0x44: 
		// call: call subroutine (relative)
		m_r[m_ssw.bits.u][R2] -= 4;
		m_program->write_dword(m_r[m_ssw.bits.u][R2], next_pc);
		next_pc = m_r[m_ssw.bits.u][R1];
		// TRAPS: A,P,W
		break;
	case 0x45: 
		// call: call subroutine (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] -= 4;
		m_program->write_dword(m_r[m_ssw.bits.u][m_info.op.r2], next_pc);
		next_pc = m_info.address;
		// TRAPS: A,P,W
		break;
#ifdef UNIMPLEMENTED_C400
	case 0x46:
		// loadd2:
		break;
	case 0x47:
		// loadd2:
		break;
#endif
	case 0x48: 
		// b*: branch on condition (relative)
		if (evaluate_branch(R2))
			next_pc = m_r[m_ssw.bits.u][R1];
		// TRAPS: A,I
		break;
	case 0x49:
		// b*: branch on condition (other modes)
		if (evaluate_branch(m_info.op.r2))
			next_pc = m_info.address;
		// TRAPS: A,I
		break;
#ifdef UNIMPLEMENTED_C400
	case 0x4a:
		// cdb:
		break;
	case 0x4b:
		// cdb:
		break;
	case 0x4c:
		// bf*/cdbeq:
		break;
	case 0x4d:
		// bf*/cdbeq:
		break;
	case 0x4e:
		// cdbne:
		break;
	case 0x4f:
		// cdbne:
		break;

	case 0x50:
		// db*:
		break;
	case 0x51:
		// db*:
		break;
#endif

	case 0x60: 
		// loadw: load word (relative)
		m_r[m_ssw.bits.u][R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x61: 
		// loadw: load word (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] = m_program->read_dword(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x62:
		// loada: load address (relative)
		m_r[m_ssw.bits.u][R2] = m_r[m_ssw.bits.u][R1];
		// TRAPS: I
		break;
	case 0x63:
		// loada: load address (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] = m_info.address;
		// TRAPS: I
		break;
	case 0x64: 
		// loads: load single floating (relative)
		((uint64_t *)&m_f)[R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x65:
		// loads: load single floating (other modes)
		((uint64_t *)&m_f)[m_info.op.r2] = m_program->read_dword(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x66: 
		// loadd: load double floating (relative)
		((uint64_t *)&m_f)[R2] = m_program->read_qword(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x67:
		// loadd: load double floating (other modes)
		((uint64_t *)&m_f)[m_info.op.r2] = m_program->read_qword(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x68:
		// loadb: load byte (relative)
		m_r[m_ssw.bits.u][R2] = (int8_t)m_program->read_byte(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x69:
		// loadb: load byte (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] = (int8_t)m_program->read_byte(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6a: 
		// loadbu: load byte unsigned (relative)
		m_r[m_ssw.bits.u][R2] = (uint8_t)m_program->read_byte(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6b:
		// loadbu: load byte unsigned (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] = m_program->read_byte(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6c: 
		// loadh: load halfword (relative)
		m_r[m_ssw.bits.u][R2] = (int16_t)m_program->read_word(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6d:
		// loadh: load halfword (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] = (int16_t)m_program->read_word(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6e:
		// loadhu: load halfword unsigned (relative)
		m_r[m_ssw.bits.u][R2] = (uint16_t)m_program->read_word(m_r[m_ssw.bits.u][R1]);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6f:
		// loadhu: load halfword unsigned (other modes)
		m_r[m_ssw.bits.u][m_info.op.r2] = m_program->read_word(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x70: 
		// storw: store word (relative)
		m_program->write_dword(m_r[m_ssw.bits.u][R1], m_r[m_ssw.bits.u][R2]);
		// TRAPS: A,P,W,I
		break;
	case 0x71:
		// storw: store word (other modes)
		m_program->write_dword(m_info.address, m_r[m_ssw.bits.u][m_info.op.r2]);
		// TRAPS: A,P,W,I
		break;
	case 0x72:
		// tsts: test and set (relative)
		m_r[m_ssw.bits.u][R2] = m_program->read_dword(m_r[m_ssw.bits.u][R1]);
		m_program->write_dword(m_r[m_ssw.bits.u][R1], m_r[m_ssw.bits.u][R2] | 0x80000000);
		// TRAPS: C,U,A,P,R,W,I
		break;
	case 0x73:
		// tsts: test and set (other modes)
		m_r[m_ssw.bits.u][R2] = m_program->read_dword(m_info.address);
		m_program->write_dword(m_info.address, m_r[m_ssw.bits.u][R2] | 0x80000000);
		// TRAPS: C,U,A,P,R,W,I
		break;
	case 0x74:
		// stors: store single floating (relative)
		m_program->write_dword(m_r[m_ssw.bits.u][R1], *((uint32_t *)&m_f[R2]));
		// TRAPS: A,P,W,I
		break;
	case 0x75:
		// stors: store single floating (other modes)
		m_program->write_dword(m_info.address, *((uint32_t *)&m_f[m_info.op.r2]));
		// TRAPS: A,P,W,I
		break;
	case 0x76:
		// stord: store double floating (relative)
		m_program->write_qword(m_r[m_ssw.bits.u][R1], *((uint64_t *)&m_f[R2]));
		// TRAPS: A,P,W,I
		break;
	case 0x77:
		// stord: store double floating (other modes)
		m_program->write_qword(m_info.address, *((uint64_t *)&m_f[m_info.op.r2]));
		// TRAPS: A,P,W,I
		break;
	case 0x78:
		// storb: store byte (relative)
		m_program->write_byte(m_r[m_ssw.bits.u][R1], (uint8_t)m_r[m_ssw.bits.u][R2]);
		// TRAPS: A,P,W,I
		break;
	case 0x79:
		// storb: store byte (other modes)
		m_program->write_byte(m_info.address, (uint8_t)m_r[m_ssw.bits.u][m_info.op.r2]);
		// TRAPS: A,P,W,I
		break;

	case 0x7c:
		// storh: store halfword (relative)
		m_program->write_word(m_r[m_ssw.bits.u][R1], (uint16_t)m_r[m_ssw.bits.u][R2]);
		// TRAPS: A,P,W,I
		break;
	case 0x7d:
		// storh: store halfword (other modes)
		m_program->write_word(m_info.address, (uint16_t)m_r[m_ssw.bits.u][m_info.op.r2]);
		// TRAPS: A,P,W,I
		break;

	case 0x80: 
		// addw: add word
		m_r[m_ssw.bits.u][R2] += m_r[m_ssw.bits.u][R1];
		// FLAGS: CVZN
		break;

	case 0x82:
		// addq: add quick
		m_r[m_ssw.bits.u][R2] += R1;
		// FLAGS: CVZN
		break;
	case 0x83:
		// addi: add immediate
		m_r[m_ssw.bits.u][R2] += m_info.op.imm;
		// FLAGS: CVZN
		// TRAPS: I
		break;
	case 0x84:
		// movw: move word
		m_r[m_ssw.bits.u][R2] = m_r[m_ssw.bits.u][R1];
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		break;

	case 0x86:
		// loadq: load quick
		m_r[m_ssw.bits.u][R2] = R1;
		// FLAGS: 00Z0
		break;
	case 0x87:
		// loadi: load immediate
		m_r[m_ssw.bits.u][R2] = m_info.op.imm;
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x88:
		// andw: and word
		m_r[m_ssw.bits.u][R2] &= m_r[m_ssw.bits.u][R1];
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		break;

	case 0x8b:
		// andi: and immediate
		m_r[m_ssw.bits.u][R2] &= m_info.op.imm;
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		// TRAPS: I
		break;
	case 0x8c:
		// orw: or word
		m_r[m_ssw.bits.u][R2] |= m_r[m_ssw.bits.u][R1];
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		break;

	case 0x8f: 
		// ori: or immediate
		m_r[m_ssw.bits.u][R2] |= m_info.op.imm;
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		// TRAPS: I
		break;
	case 0x90:
		// addwc: add word with carry
		m_r[m_ssw.bits.u][R2] += m_r[m_ssw.bits.u][R1] + m_psw.bits.c;
		// FLAGS: CVZN
		break;
	case 0x91:
		// subwc: subtract word with carry
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1] + m_psw.bits.c, FLAGS_CVZN);
		m_r[m_ssw.bits.u][R2] -= m_r[m_ssw.bits.u][R1] + m_psw.bits.c;
		// FLAGS: CVZN
		break;

	case 0x93: 
		// negw: negate word
		m_r[m_ssw.bits.u][R2] = -(int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: CVZN
		break;

	case 0x98: 
		// mulw: multiply word
		m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] * (int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0V00
		break;
	case 0x99:
		// mulwx: multiply word extended
		((int64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = (int64_t)m_r[m_ssw.bits.u][R2] * (int64_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0V00
		break;
	case 0x9a: 
		// mulwu: multiply word unsigned
		m_r[m_ssw.bits.u][R2] = (uint32_t)m_r[m_ssw.bits.u][R2] * (uint32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0V00
		break;
	case 0x9b:
		// mulwux: multiply word unsigned extended
		((uint64_t *)m_r[m_ssw.bits.u])[R2 >> 1] = (uint64_t)m_r[m_ssw.bits.u][R2] * (uint64_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0V00
		break;
	case 0x9c: 
		// divw: divide word
		if ((int32_t)m_r[m_ssw.bits.u][R1] == 0)
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		else
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] / (int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0V00
		// TRAPS: D
		break;
	case 0x9d: 
		// modw: modulus word
		if ((int32_t)m_r[m_ssw.bits.u][R1] == 0)
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		else
			m_r[m_ssw.bits.u][R2] = (int32_t)m_r[m_ssw.bits.u][R2] % (int32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0V00
		// TRAPS: D
		break;
	case 0x9e: 
		// divwu: divide word unsigned
		if ((uint32_t)m_r[m_ssw.bits.u][R1] == 0)
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		else
			m_r[m_ssw.bits.u][R2] = (uint32_t)m_r[m_ssw.bits.u][R2] / (uint32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0000
		// TRAPS: D
		break;
	case 0x9f: 
		// modwu: modulus word unsigned
		if ((uint32_t)m_r[m_ssw.bits.u][R1] == 0)
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		else
			m_r[m_ssw.bits.u][R2] = (uint32_t)m_r[m_ssw.bits.u][R2] % (uint32_t)m_r[m_ssw.bits.u][R1];
		// FLAGS: 0000
		// TRAPS: D
		break;
	case 0xa0: 
		// subw: subtract word
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1], FLAGS_CVZN);
		m_r[m_ssw.bits.u][R2] -= m_r[m_ssw.bits.u][R1];
		break;

	case 0xa2:
		// subq: subtract quick
		evaluate_cc2(m_r[m_ssw.bits.u][R2], R1, FLAGS_CVZN);
		m_r[m_ssw.bits.u][R2] -= R1;
		break;
	case 0xa3:
		// subi: subtract immediate
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_info.op.imm, FLAGS_CVZN);
		m_r[m_ssw.bits.u][R2] -= m_info.op.imm;
		// TRAPS: I
		break;
	case 0xa4:
		// cmpw: compare word
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_r[m_ssw.bits.u][R1], FLAGS_CVZN);
		break;

	case 0xa6: 
		// cmpq: compare quick
		evaluate_cc2(m_r[m_ssw.bits.u][R2], R1, FLAGS_CVZN);
		break;
	case 0xa7: 
		// cmpi: compare immediate
		evaluate_cc2(m_r[m_ssw.bits.u][R2], m_info.op.imm, FLAGS_CVZN);
		// TRAPS: I
		break;
	case 0xa8: 
		// xorw: exclusive or word
		m_r[m_ssw.bits.u][R2] ^= m_r[m_ssw.bits.u][R1];
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		break;

	case 0xab: 
		// xori: exclusive or immediate
		m_r[m_ssw.bits.u][R2] ^= m_info.op.imm;
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		// TRAPS: I
		break;
	case 0xac: 
		// notw: not word
		m_r[m_ssw.bits.u][R2] = ~m_r[m_ssw.bits.u][R1];
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAGS_ZN);
		break;

	case 0xae: 
		// notq: not quick
		m_r[m_ssw.bits.u][R2] = ~R1;
		evaluate_cc2(m_r[m_ssw.bits.u][R2], 0, FLAG_N);
		break;

#ifdef UNIMPLEMENTED
	case 0xb0:
		// abss: absolute value single floating?
		break;

	case 0xb2:
		// absd: absolute value double floating?
		break;
#endif

	case 0xb4:
		// unprivileged macro instructions
		switch (insn & 0xff)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c:
			// savew0..savew12: push registers rN:r14

			// store ri at sp - 4 * (15 - i)
			for (int i = R2; i < 15; i++)
				m_program->write_dword(m_r[m_ssw.bits.u][15] - 4 * (15 - i), m_r[m_ssw.bits.u][i]);

			// decrement sp after push to allow restart on exceptions
			m_r[m_ssw.bits.u][15] -= 4 * (15 - R2);
			// TRAPS: A,P,W
			break;

		// NOTE: the movc, initc and cmpc macro instructions are implemented in a very basic way because
		// at some point they will need to be improved to deal with possible exceptions (e.g. page faults)
		// that may occur during execution. The implementation here is intended to allow the instructions
		// to be "continued" after such exceptions.
		case 0x0d:
			// movc: copy r0 bytes from r1 to r2

			while (m_r[m_ssw.bits.u][0])
			{
				m_program->write_byte(m_r[m_ssw.bits.u][2], m_program->read_byte(m_r[m_ssw.bits.u][1]));

				m_r[m_ssw.bits.u][0]--;
				m_r[m_ssw.bits.u][1]++;
				m_r[m_ssw.bits.u][2]++;
			}
			// TRAPS: C,U,P,R,W
			break;

		case 0x0e:
			// initc: initialise r0 bytes at r1 with value in r2
			while (m_r[m_ssw.bits.u][0])
			{
				m_program->write_byte(m_r[m_ssw.bits.u][1], m_r[m_ssw.bits.u][2] & 0xff);

				m_r[m_ssw.bits.u][0]--;
				m_r[m_ssw.bits.u][1]++;
				m_r[m_ssw.bits.u][2] = _rotr(m_r[m_ssw.bits.u][2], 8);
			}
			// TRAPS: P,W
			break;

		case 0x0f:
			// cmpc: compare r0 bytes at r1 with r2

			// set condition codes assuming strings match
			m_psw.bits.n = 0;
			m_psw.bits.z = 1;
			m_psw.bits.v = 0;
			m_psw.bits.c = 0;

			while (m_r[m_ssw.bits.u][0])
			{
				// set condition codes and abort the loop if the current byte does not match
				uint8_t byte1 = m_program->read_byte(m_r[m_ssw.bits.u][1]);
				uint8_t byte2 = m_program->read_byte(m_r[m_ssw.bits.u][2]);
				if (byte1 != byte2)
				{
					evaluate_cc2(byte1, byte2, FLAGS_CVZN);
					break;
				}

				m_r[m_ssw.bits.u][0]--;
				m_r[m_ssw.bits.u][1]++;
				m_r[m_ssw.bits.u][2]++;
			}
			// TRAPS: C,U,P,R
			break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c:
			// restwN..restw12: pop registers rN:r14

			// load ri from sp + 4 * (i - N)
			for (int i = R2; i < 15; i++)
				m_r[m_ssw.bits.u][i] = m_program->read_dword(m_r[m_ssw.bits.u][15] + 4 * (i - R2));

			// increment sp after pop to allow restart on exceptions
			m_r[m_ssw.bits.u][15] += 4 * (15 - R2);
			// TRAPS: C,U,A,P,R
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: 
		case 0x24: case 0x25: case 0x26: case 0x27:
			// saved0..saved7: push registers fN:f7

			// store fi at sp - 8 * (8 - i)
			for (int i = R2; i < 8; i++)
				m_program->write_qword(m_r[m_ssw.bits.u][15] - 8 * (8 - i), m_f[i]);

			// decrement sp after push to allow restart on exceptions
			m_r[m_ssw.bits.u][15] -= 8 * (8 - R2);
			// TRAPS: A,P,W
			break;

		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			// restd0..restd7: pop registers fN:f7

			// load fi from sp + 8 * (i - N)
			for (int i = R2; i < 8; i++)
				m_f[i] = m_program->read_qword(m_r[m_ssw.bits.u][15] + 8 * (i - R2));

			// increment sp after pop to allow restart on exceptions
			m_r[m_ssw.bits.u][15] += 8 * (8 - R2);
			// TRAPS: C,U,A,P,R
			break;
#ifdef UNIMPLEMENTED
		case 0x30:
			// cnvsw
		case 0x31:
			// cnvrsw
			// TRAPS: F_IX
		case 0x32:
			// cnvtsw
			// TRAPS: F_IX
		case 0x33:
			// cnvws
			// TRAPS: F_X
		case 0x34:
			// cnvdw
			// TRAPS: F_IX
		case 0x35:
			// cnvrdw
			// TRAPS: F_IX
			break;
#endif
		case 0x36: // cnvtdw
			m_r[m_ssw.bits.u][m_info.op.macro & 0xf] = (int32_t)m_f[(m_info.op.macro >> 4) & 0xf];
			// TRAPS: F_IX
			break;

		case 0x37: // cnvwd
			m_f[m_info.op.macro & 0xf] = (double)m_r[m_ssw.bits.u][(m_info.op.macro >> 4) & 0xf];
			break;
#ifdef UNIMPLEMENTED
		case 0x38:
			// cnvsd
			// TRAPS: F_I
		case 0x39:
			// cnvds
			// TRAPS: F_IVUX
		case 0x3a:
			// negs
		case 0x3b:
			// negds
		case 0x3c:
			// scalbs
			// TRAPS: F_IVUX
		case 0x3d:
			// scalbd
			// FLAGS: N
			// TRAPS: F_IVUX
		case 0x3e:
			// trapfn
			// TRAPS: I
		case 0x3f:
			// loadfs
			break;
#endif
		default:
			logerror("illegal unprivileged macro opcode at 0x%08x\n", m_pc);
			next_pc = intrap(EXCEPTION_ILLEGAL_OPERATION, next_pc, CTS_ILLEGAL_OPERATION);
			machine().debug_break();
			break;
		}

		break;

	case 0xb6:
		// privileged macro instructions
		if (m_ssw.bits.u == 0)
		{
			switch (insn & 0xff)
			{
			case 0x00: 
				// movus: move user to supervisor
				m_r[0][m_info.op.macro & 0xf] = m_r[1][(m_info.op.macro >> 4) & 0xf];
				// setcc1(m_r[m_ssw.bits.u][r2]);
				// FLAGS: 00ZN
				// TRAPS: S
				break;

			case 0x01: 
				// movsu: move supervisor to user
				m_r[1][m_info.op.macro & 0xf] = m_r[0][(m_info.op.macro >> 4) & 0xf];
				// setcc1(m_r[!m_ssw.bits.u][r2]);
				// FLAGS: 00ZN
				// TRAPS: S
				break;

			case 0x02:
				// saveur: save user registers
				for (int i = 0; i < 16; i++)
					m_program->write_dword(m_r[0][(m_info.op.macro >> 4) & 0xf] - 4 * (i + 1), m_r[1][15 - i]);

				m_r[0][(m_info.op.macro >> 4) & 0xf] -= 64;
				// TRAPS: A,P,W,S
				break;

			case 0x03:
				// restur: restore user registers
				for (int i = 0; i < 16; i++)
					m_r[1][i] = m_program->read_dword(m_r[0][(m_info.op.macro >> 4) & 0xf] + 4 * i);

				m_r[0][(m_info.op.macro >> 4) & 0xf] += 64;
				// TRAPS: C,U,A,P,R,S
				break;

			case 0x04: 
				// reti: restore psw, ssw and pc from supervisor stack
				LOG_INTERRUPT("reti r%d, ssp = %08x, pc = %08x, next_pc = %08x\n",
					(op.macro >> 4) & 0xf, m_r[0][(m_info.op.macro >> 4) & 0xf], m_pc, m_program->read_dword(m_r[0][(m_info.op.macro >> 4) & 0xf] + 8));

				m_psw.d = m_program->read_dword(m_r[0][(m_info.op.macro >> 4) & 0xf] + 0);
				m_ssw.d = m_program->read_dword(m_r[0][(m_info.op.macro >> 4) & 0xf] + 4);
				next_pc = m_program->read_dword(m_r[0][(m_info.op.macro >> 4) & 0xf] + 8);

				m_r[0][(m_info.op.macro >> 4) & 0xf] += 12;
				// TRAPS: S
				break;

			case 0x05:
				// wait: wait for interrupt
				next_pc = m_pc;
				// TRAPS: S
				break;

			default:
				// illegal operation
				logerror("illegal privileged macro opcode at 0x%08x\n", m_pc);
				next_pc = intrap(EXCEPTION_ILLEGAL_OPERATION, next_pc, CTS_ILLEGAL_OPERATION);
				machine().debug_break();
				break;
			}
		}
		else
			next_pc = intrap(EXCEPTION_PRIVILEGED_INSTRUCTION, next_pc, CTS_PRIVILEGED_INSTRUCTION);
		break;

#ifdef UNIMPLEMENTED
	case 0xbc:
		// waitd:
		break;

	case 0xc0:
		// sregc:
		break;
#endif

	default:
		logerror("illegal opcode at 0x%08x\n", m_pc);
		next_pc = intrap(EXCEPTION_ILLEGAL_OPERATION, next_pc, CTS_ILLEGAL_OPERATION);
		machine().debug_break();
		break;

	}

	return next_pc;
}

/*
* Sets up the PC, SSW and PSW to handle an exception.
*/
uint32_t clipper_device::intrap(uint32_t vector, uint32_t pc, uint32_t cts = CTS_NO_CPU_TRAP, uint32_t mts = MTS_NO_MEMORY_TRAP)
{
	uint32_t previous_mode;

	LOG_INTERRUPT("intrap - vector %x, pc = 0x%08x, next_pc = 0x%08x, ssp = 0x%08x\n", vector, pc, m_program->read_dword(vector + 4), m_r[0][15]);

	// set cts and mts to indicate source of exception
	m_psw.bits.cts = cts;
	m_psw.bits.mts = mts;

	// push pc, psw and ssw onto supervisor stack
	m_program->write_dword(m_r[0][15] - 4, pc);
	m_program->write_dword(m_r[0][15] - 12, m_psw.d);
	m_program->write_dword(m_r[0][15] - 8, m_ssw.d);

	// decrement supervisor stack pointer

	// NOTE: while not explicitly stated anywhere, it seems the InterPro boot code has been
	// developed with the assumption that the SSP is decremented by 24 bytes during an exception, 
	// rather than the 12 bytes that might otherwise be expected. This means the exception handler
	// code must explicitly increment the SSP by 12 prior to executing the RETI instruction,
	// as otherwise the SSP will not be pointing at a valid return frame. It's possible this 
	// behaviour might vary with some other version of the CPU, but this is all we know for now.
	m_r[0][15] -= 24;

	// load SSW from trap vector and set previous mode flag
	previous_mode = m_ssw.bits.u;
	m_ssw.d = m_program->read_dword(vector + 0);
	m_ssw.bits.p = previous_mode;

	// clear psw
	m_psw.d = 0;

	// return new pc from trap vector
	return m_program->read_dword(vector + 4);
}

/*
void clipper_device::evaluate_cc1(int32_t v0)
{
	m_psw.bits.n = v0 < 0;
	m_psw.bits.z = v0 == 0;
}
*/

/*
evaluation of overflow:
both +:  v0 - v1 > v0
v0- v1+: v0 - v1 > v0 = 0

both -:  v0 - v1 < v0
v0+ v1-: v0 - v1 < v0 = 0

overflow (addition): inputs have same sign and output has opposite
s1 == s2 && != s3

THEORY:
  for compare, call evaluate_cc2(r1, r2)
  for move/logical, call evaluate_cc2(r2, 0)
  these instructions should only set N or Z
*/
void clipper_device::evaluate_cc2 (int32_t v0, int32_t v1, uint32_t flags)
{
	m_psw.bits.n = flags & FLAG_N ? (v0 - v1) >> 31 : 0;
	m_psw.bits.z = flags & FLAG_Z ? v0 == v1 : 0;
	m_psw.bits.v = flags & FLAG_V ? ((v1 < 0) ? (v0 - v1 < v0) : (v0 - v1 > v0)) : 0;
	m_psw.bits.c = flags & FLAG_C ? ((uint32_t)v0 < (uint32_t)v1) : 0;
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

bool clipper_device::evaluate_branch (uint32_t condition)
{
	switch (condition)
	{
	case BRANCH_T:
		return true;

	case BRANCH_LT:
		return (!m_psw.bits.v && !m_psw.bits.z && !m_psw.bits.n)
			|| (m_psw.bits.v && !m_psw.bits.z && m_psw.bits.n);

	case BRANCH_LE:
		return (!m_psw.bits.v && !m_psw.bits.n)
			|| (m_psw.bits.v && !m_psw.bits.z && m_psw.bits.n);

	case BRANCH_EQ:
		return m_psw.bits.z && !m_psw.bits.n;

	case BRANCH_GT:
		return (!m_psw.bits.v && !m_psw.bits.z && m_psw.bits.n) 
			|| (m_psw.bits.v && !m_psw.bits.n);

	case BRANCH_GE:
		return (m_psw.bits.v && !m_psw.bits.n)
			|| (!m_psw.bits.v && !m_psw.bits.z && m_psw.bits.n)
			|| (m_psw.bits.z && !m_psw.bits.n);

	case BRANCH_NE:
		return (!m_psw.bits.z) 
			|| (m_psw.bits.z && m_psw.bits.n);

	case BRANCH_LTU:
		return (!m_psw.bits.c && !m_psw.bits.z);

	case BRANCH_LEU:
		return !m_psw.bits.c;

	case BRANCH_GTU:
		return m_psw.bits.c;

	case BRANCH_GEU:
		return m_psw.bits.c || m_psw.bits.z;

	case BRANCH_V:
		return m_psw.bits.v;
	case BRANCH_NV:
		return !m_psw.bits.v;

	case BRANCH_N:
		return !m_psw.bits.z && m_psw.bits.n;
	case BRANCH_NN:
		return !m_psw.bits.n;

	case BRANCH_FN:
		return m_psw.bits.z && m_psw.bits.n;
	}

	return false;
}

offs_t clipper_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE(clipper);

	return CPU_DISASSEMBLE_NAME(clipper)(this, stream, pc, oprom, opram, options);
}
