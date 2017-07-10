// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Fairchild/Intergraph CLIPPER CPU family.
 *
 * Primary source: http://bitsavers.trailing-edge.com/pdf/fairchild/clipper/Clipper_Instruction_Set_Oct85.pdf
 *
 * TODO:
 *   - save/restore state
 *   - unimplemented instructions
 *   - C100, C300, C400 variants
 *   - correct boot logic
 *   - condition codes for multiply instructions
 *   - most cpu traps/faults
 *   - instruction timing
 *   - big endian support (not present in the wild)
 */

#include "emu.h"
#include "debugger.h"
#include "clipper.h"

#define VERBOSE 0
#define LOG_INTERRUPT(...) do { if (VERBOSE) logerror(__VA_ARGS__); } while (false)

// convenience macros for frequently used instruction fields
#define R1 (m_info.r1)
#define R2 (m_info.r2)

// macros for setting psw condition codes
#define FLAGS(C,V,Z,N) \
	m_psw = (m_psw & ~(PSW_C | PSW_V | PSW_Z | PSW_N)) | (((C) << 3) | ((V) << 2) | ((Z) << 1) | ((N) << 0));
#define FLAGS_CV(C,V) \
	m_psw = (m_psw & ~(PSW_C | PSW_V)) | (((C) << 3) | ((V) << 2));
#define FLAGS_ZN(Z,N) \
	m_psw = (m_psw & ~(PSW_Z | PSW_N)) | (((Z) << 1) | ((N) << 0));

// over/underflow for addition/subtraction from here: http://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
#define OF_ADD(a, b) ((b > 0) && (a > INT_MAX - b))
#define UF_ADD(a, b) ((b < 0) && (a < INT_MIN - b))
#define OF_SUB(a, b) ((b < 0) && (a > INT_MAX + b))
#define UF_SUB(a, b) ((b > 0) && (a < INT_MIN + b))

// CLIPPER logic for carry and overflow flags
#define C_ADD(a, b) ((u32)a + (u32)b < (u32)a)
#define V_ADD(a, b) (OF_ADD((s32)a, (s32)b) || UF_ADD((s32)a, (s32)b))
#define C_SUB(a, b) ((u32)a < (u32)b)
#define V_SUB(a, b) (OF_SUB((s32)a, (s32)b) || UF_SUB((s32)a, (s32)b))

DEFINE_DEVICE_TYPE(CLIPPER_C100, clipper_c100_device, "clipper_c100", "C100 CLIPPER")
DEFINE_DEVICE_TYPE(CLIPPER_C300, clipper_c300_device, "clipper_c300", "C300 CLIPPER")
DEFINE_DEVICE_TYPE(CLIPPER_C400, clipper_c400_device, "clipper_c400", "C400 CLIPPER")

clipper_c100_device::clipper_c100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: clipper_device(mconfig, CLIPPER_C100, tag, owner, clock, 0) { }

clipper_c300_device::clipper_c300_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: clipper_device(mconfig, CLIPPER_C300, tag, owner, clock, 0) { }

clipper_c400_device::clipper_c400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: clipper_device(mconfig, CLIPPER_C400, tag, owner, clock, SSW_ID_C400R4) { }

clipper_device::clipper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const u32 cpuid)
	: cpu_device(mconfig, type, tag, owner, clock),
	m_pc(0),
	m_psw(0),
	m_ssw(cpuid),
	m_r(m_rs),
	m_insn_config("insn", ENDIANNESS_LITTLE, 32, 32, 0),
	m_data_config("data", ENDIANNESS_LITTLE, 32, 32, 0),
	m_insn(nullptr),
	m_data(nullptr),
	m_icount(0)
{
}

// rotate helpers to replace MSVC intrinsics
inline u32 rotl32(u32 x, u8 shift)
{
  shift &= 31;
  return (x << shift) | (x >> ((32 - shift) & 31));
}

inline u32 rotr32(u32 x, u8 shift)
{
  shift &= 31;
  return (x >> shift) | (x << ((32 - shift) & 31));
}

inline u64 rotl64(u64 x, u8 shift)
{
  shift &= 63;
  return (x << shift) | (x >> ((64 - shift) & 63));
}

inline u64 rotr64(u64 x, u8 shift)
{
  shift &= 63;
  return (x >> shift) | (x << ((64 - shift) & 63));
}

void clipper_device::device_start()
{
	// get our address spaces
	m_insn = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// set our instruction counter
	m_icountptr = &m_icount;

	//save_item(NAME(m_pc));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENSP, "GENSP", m_r[15]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_psw).mask(0xf).formatstr("%4s").noshow();

	state_add(CLIPPER_PC, "pc", m_pc);
	state_add(CLIPPER_PSW, "psw", m_psw);
	state_add(CLIPPER_SSW, "ssw", m_ssw);

	state_add(CLIPPER_R0, "r0", m_r[0]);
	state_add(CLIPPER_R1, "r1", m_r[1]);
	state_add(CLIPPER_R2, "r2", m_r[2]);
	state_add(CLIPPER_R3, "r3", m_r[3]);
	state_add(CLIPPER_R4, "r4", m_r[4]);
	state_add(CLIPPER_R5, "r5", m_r[5]);
	state_add(CLIPPER_R6, "r6", m_r[6]);
	state_add(CLIPPER_R7, "r7", m_r[7]);
	state_add(CLIPPER_R8, "r8", m_r[8]);
	state_add(CLIPPER_R9, "r9", m_r[9]);
	state_add(CLIPPER_R10, "r10", m_r[10]);
	state_add(CLIPPER_R11, "r11", m_r[11]);
	state_add(CLIPPER_R12, "r12", m_r[12]);
	state_add(CLIPPER_R13, "r13", m_r[13]);
	state_add(CLIPPER_R14, "r14", m_r[14]);
	state_add(CLIPPER_R15, "r15", m_r[15]);

	state_add(CLIPPER_F0, "f0", m_f[0]);
	state_add(CLIPPER_F1, "f1", m_f[1]);
	state_add(CLIPPER_F2, "f2", m_f[2]);
	state_add(CLIPPER_F3, "f3", m_f[3]);
	state_add(CLIPPER_F4, "f4", m_f[4]);
	state_add(CLIPPER_F5, "f5", m_f[5]);
	state_add(CLIPPER_F6, "f6", m_f[6]);
	state_add(CLIPPER_F7, "f7", m_f[7]);

	// C400 has 8 additional floating point registers
	if (type() == CLIPPER_C400)
	{
		state_add(CLIPPER_F8, "f8", m_f[8]);
		state_add(CLIPPER_F9, "f9", m_f[9]);
		state_add(CLIPPER_F10, "f10", m_f[10]);
		state_add(CLIPPER_F11, "f11", m_f[11]);
		state_add(CLIPPER_F12, "f12", m_f[12]);
		state_add(CLIPPER_F13, "f13", m_f[13]);
		state_add(CLIPPER_F14, "f14", m_f[14]);
		state_add(CLIPPER_F15, "f15", m_f[15]);
	}
}

void clipper_device::device_reset()
{
	/*
	 * From C300 documentation, on reset:
	 *   psw: T cleared, BIG set from hardware, others undefined
	 *   ssw: EI, TP, M, U, K, KU, UU, P cleared, ID set from hardware, others undefined
	 */
	m_psw = 0;
	set_ssw(0);

	m_r = SSW(U) ? m_ru : m_rs;

	// we'll opt to clear the integer and floating point registers too
	memset(m_r, 0, sizeof(s32)*16);
	memset(m_f, 0, sizeof(m_f));

	// FIXME: figure out how to branch to the boot code properly
	m_pc = 0x7f100000;
	m_irq = 0;
	m_nmi = 0;
}

void clipper_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c",
			PSW(C) ? 'C' : '.',
			PSW(V) ? 'V' : '.',
			PSW(Z) ? 'Z' : '.',
			PSW(N) ? 'N' : '.');
		break;
	}
}

void clipper_device::execute_run()
{
	u16 insn;

	// check for non-maskable and prioritised interrupts
	if (m_nmi)
	{
		// acknowledge non-maskable interrupt
		standard_irq_callback(INPUT_LINE_NMI);

		LOG_INTERRUPT("non-maskable interrupt - current pc = 0x%08x\n", m_pc);
		m_pc = intrap(EXCEPTION_INTERRUPT_BASE, m_pc);
	}
	else if (SSW(EI) && m_irq)
	{
		// FIXME: sample interrupt vector from the bus without acknowledging the interrupt
		u8 ivec = standard_irq_callback(-1);
		LOG_INTERRUPT("received prioritised interrupt with vector 0x%04x\n", ivec);

		// allow equal/higher priority interrupts
		if ((ivec >> 4) <= SSW(IL))
		{
			// acknowledge interrupt
			standard_irq_callback(INPUT_LINE_IRQ0);

			LOG_INTERRUPT("accepting interrupt vector 0x%04x - current pc = %08x\n", ivec, m_pc);
			m_pc = intrap(EXCEPTION_INTERRUPT_BASE + ivec * 8, m_pc);
		}
	}

	while (m_icount > 0) {

		debugger_instruction_hook(this, m_pc);

		// fetch instruction word
		insn = m_insn->read_word(m_pc + 0);

		decode_instruction(insn);

		// decode and execute instruction, return next pc
		m_pc = execute_instruction();

		// FIXME: some instructions take longer (significantly) than one cycle
		// and also the timings are often slower for the C100 and C300
		m_icount--;
	}
}

void clipper_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case INPUT_LINE_IRQ0:
		m_irq = state;
		break;

	case INPUT_LINE_NMI:
		m_nmi = state;
		break;
	}
}

/*
 * The CLIPPER has a true Harvard architecture. In the InterPro, these are tied back together
 * again by the MMU, which then directs the access to one of 3 address spaces: main, i/o or boot.
 */
device_memory_interface::space_config_vector clipper_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_insn_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

/*
 * This function decodes instruction operands and computes effective addresses (for
 * instructions with addressing modes). The results are contained in the m_info
 * structure to simplify passing between here and execute_instruction().
 */
void clipper_device::decode_instruction (u16 insn)
{
	// decode the primary parcel
	m_info.opcode = insn >> 8;
	m_info.subopcode = insn & 0xff;
	m_info.r1 = (insn & 0x00f0) >> 4;
	m_info.r2 = insn & 0x000f;

	// initialise the other fields
	m_info.imm = 0;
	m_info.macro = 0;
	m_info.size = 0;
	m_info.address = 0;

	if ((insn & 0xf800) == 0x3800)
	{
		// instruction has a 16 bit immediate operand

		// fetch 16 bit immediate and sign extend
		m_info.imm = (s16)m_insn->read_word(m_pc + 2);
		m_info.size = 4;
	}
	else if ((insn & 0xd300) == 0x8300)
	{
		// instruction has an immediate operand, either 16 or 32 bit
		if (insn & 0x0080)
		{
			// fetch 16 bit immediate and sign extend
			m_info.imm = (s16)m_insn->read_word(m_pc + 2);
			m_info.size = 4;
		}
		else
		{
			// fetch 32 bit immediate and sign extend
			m_info.imm = (s32)m_insn->read_dword_unaligned(m_pc + 2);
			m_info.size = 6;
		}
	}
	else if ((insn & 0xc000) == 0x4000)
	{
		// instructions with addresses
		if (insn & 0x0100)
		{
			// instructions with complex modes
			u16 temp;

			switch (insn & 0x00f0)
			{
			case ADDR_MODE_PC32:
				m_info.address = m_pc + (s32)m_insn->read_dword_unaligned(m_pc + 2);
				m_info.size = 6;
				break;

			case ADDR_MODE_ABS32:
				m_info.address = m_insn->read_dword_unaligned(m_pc + 2);
				m_info.size = 6;
				break;

			case ADDR_MODE_REL32:
				m_info.r2 = m_insn->read_word(m_pc + 2) & 0xf;
				m_info.address = m_r[insn & 0xf] + (s32)m_insn->read_dword_unaligned(m_pc + 4);
				m_info.size = 8;
				break;

			case ADDR_MODE_PC16:
				m_info.address = m_pc + (s16)m_insn->read_word(m_pc + 2);
				m_info.size = 4;
				break;

			case ADDR_MODE_REL12:
				temp = m_insn->read_word(m_pc + 2);

				m_info.r2 = temp & 0xf;
				m_info.address = m_r[insn & 0xf] + ((s16)temp >> 4);
				m_info.size = 4;
				break;

			case ADDR_MODE_ABS16:
				m_info.address = (s16)m_insn->read_word(m_pc + 2);
				m_info.size = 4;
				break;

			case ADDR_MODE_PCX:
				temp = m_insn->read_word(m_pc + 2);

				m_info.r2 = temp & 0xf;
				m_info.address = m_pc + m_r[(temp >> 4) & 0xf];
				m_info.size = 4;
				break;

			case ADDR_MODE_RELX:
				temp = m_insn->read_word(m_pc + 2);

				m_info.r2 = temp & 0xf;
				m_info.address = m_r[insn & 0xf] + m_r[(temp >> 4) & 0xf];
				m_info.size = 4;
				break;

			default:
				logerror("illegal addressing mode pc = 0x%08x\n", m_pc);
				machine().debug_break();
				break;
			}
		}
		else
		{
			// relative addressing mode
			m_info.address = m_r[m_info.r1];
			m_info.size = 2;
		}
	}
	else if ((insn & 0xfd00) == 0xb400)
	{
		// macro instructions
		m_info.macro = m_insn->read_word(m_pc + 2);
		m_info.size = 4;
	}
	else
		// all other instruction formats are 16 bits
		m_info.size = 2;
}

int clipper_device::execute_instruction ()
{
	// the address of the next instruction
	u32 next_pc;

	// next instruction follows the current one by default, but
	// may be changed for branch, call or trap instructions
	next_pc = m_pc + m_info.size;

	switch (m_info.opcode)
	{
	case 0x00: // noop
		break;

	case 0x10:
		// movwp: move word to processor register
		// treated as a noop if target ssw in user mode
		// R1 == 3 means "fast" mode - avoids pipeline flush
		if (R1 == 0)
			m_psw = m_r[R2];
		else if (!SSW(U) && (R1 == 1 || R1 == 3))
		{
			set_ssw(m_r[R2]);
			m_r = SSW(U) ? m_ru : m_rs;
		}
		// FLAGS: CVZN
		break;
	case 0x11:
		// movpw: move processor register to word
		switch (R1)
		{
		case 0: m_r[R2] = m_psw; break;
		case 1: m_r[R2] = m_ssw; break;
		}
		break;
	case 0x12:
		// calls: call supervisor
		next_pc = intrap(EXCEPTION_SUPERVISOR_CALL_BASE + (m_info.subopcode & 0x7f) * 8, next_pc);
		break;
	case 0x13:
		// ret: return from subroutine
		next_pc = m_data->read_dword(m_r[R2]);
		m_r[R2] += 4;
		// TRAPS: C,U,A,P,R
		break;
	case 0x14:
		// pushw: push word
		m_r[R1] -= 4;
		m_data->write_dword(m_r[R1], m_r[R2]);
		// TRAPS: A,P,W
		break;

	case 0x16:
		// popw: pop word
		m_r[R2] = m_data->read_dword(m_r[R1]);
		m_r[R1] += 4;
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
		FLAGS(0, 0, *((float *)&m_f[R2]) == *((float *)&m_f[R1]), *((float *)&m_f[R2]) < *((float *)&m_f[R1]))
		break;
	case 0x26:
		// movd: move double floating
		m_f[R2] = m_f[R1];
		break;
	case 0x27:
		// cmpd: compare double floating
		FLAGS(0, 0, m_f[R2] == m_f[R1], m_f[R2] < m_f[R1])
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
		m_r[R2] = *((s32 *)&m_f[R1]);
		break;
	case 0x2d:
		// movws: move word to single floating
		*((s32 *)&m_f[R2]) = m_r[R1];
		break;
	case 0x2e:
		// movdl: move double floating to longword
		((double *)m_r)[R2 >> 1] = m_f[R1];
		break;
	case 0x2f:
		// movld: move longword to double floating
		m_f[R2] = ((double *)m_r)[R1 >> 1];
		break;
	case 0x30:
		// shaw: shift arithmetic word
		if (m_r[R1] > 0)
		{
			// save the bits that will be shifted out plus new sign bit
			s32 v = m_r[R2] >> (31 - m_r[R1]);

			m_r[R2] <<= m_r[R1];

			// overflow is set if sign changes during shift
			FLAGS(0, v != 0 && v != -1, m_r[R2] == 0, m_r[R2] < 0)
		}
		else
		{
			m_r[R2] >>= -m_r[R1];
			FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		}
		// FLAGS: 0VZN
		break;
	case 0x31:
		// shal: shift arithmetic longword
		if (m_r[R1] > 0)
		{
			// save the bits that will be shifted out plus new sign bit
			s64 v = ((s64 *)m_r)[R2 >> 1] >> (63 - m_r[R1]);

			((s64 *)m_r)[R2 >> 1] <<= m_r[R1];

			// overflow is set if sign changes during shift
			FLAGS(0, v != 0 && v != -1, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0)
		}
		else
		{
			((s64 *)m_r)[R2 >> 1] >>= -m_r[R1];
			FLAGS(0, 0, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0)
		}
		// FLAGS: 0VZN
		break;
	case 0x32:
		// shlw: shift logical word
		if (m_r[R1] > 0)
			m_r[R2] <<= m_r[R1];
		else
			((u32 *)m_r)[R2] >>= -m_r[R1];
		// FLAGS: 00ZN
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0);
		break;
	case 0x33:
		// shll: shift logical longword
		if (m_r[R1] > 0)
			((u64 *)m_r)[R2 >> 1] <<= m_r[R1];
		else
			((u64 *)m_r)[R2 >> 1] >>= -m_r[R1];
		// FLAGS: 00ZN
		FLAGS(0, 0, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0);
		break;
	case 0x34:
		// rotw: rotate word
		if (m_r[R1] > 0)
			m_r[R2] = rotl32(m_r[R2], m_r[R1]);
		else
			m_r[R2] = rotr32(m_r[R2], -m_r[R1]);
		// FLAGS: 00ZN
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0);
		break;
	case 0x35:
		// rotl: rotate longword
		if (m_r[R1] > 0)
			((u64 *)m_r)[R2 >> 1] = rotl64(((u64 *)m_r)[R2 >> 1], m_r[R1]);
		else
			((u64 *)m_r)[R2 >> 1] = rotr64(((u64 *)m_r)[R2 >> 1], -m_r[R1]);
		// FLAGS: 00ZN
		FLAGS(0, 0, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0);
		break;

	case 0x38:
		// shai: shift arithmetic immediate
		if (m_info.imm > 0)
		{
			// save the bits that will be shifted out plus new sign bit
			s32 v = m_r[R2] >> (31 - m_info.imm);

			m_r[R2] <<= m_info.imm;

			// overflow is set if sign changes during shift
			FLAGS(0, v != 0 && v != -1, m_r[R2] == 0, m_r[R2] < 0)
		}
		else
		{
			m_r[R2] >>= -m_info.imm;
			FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		}
		// FLAGS: 0VZN
		// TRAPS: I
		break;
	case 0x39:
		// shali: shift arithmetic longword immediate
		if (m_info.imm > 0)
		{
			// save the bits that will be shifted out plus new sign bit
			s64 v = ((s64 *)m_r)[R2 >> 1] >> (63 - m_info.imm);

			((s64 *)m_r)[R2 >> 1] <<= m_info.imm;

			// overflow is set if sign changes during shift
			FLAGS(0, v != 0 && v != -1, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0)
		}
		else
		{
			((s64 *)m_r)[R2 >> 1] >>= -m_info.imm;
			FLAGS(0, 0, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0)
		}
		// FLAGS: 0VZN
		// TRAPS: I
		break;
	case 0x3a:
		// shli: shift logical immediate
		if (m_info.imm > 0)
			m_r[R2] <<= m_info.imm;
		else
			((u32 *)m_r)[R2] >>= -m_info.imm;
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0);
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x3b:
		// shlli: shift logical longword immediate
		if (m_info.imm > 0)
			((u64 *)m_r)[R2 >> 1] <<= m_info.imm;
		else
			((u64 *)m_r)[R2 >> 1] >>= -m_info.imm;
		FLAGS(0, 0, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0);
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x3c:
		// roti: rotate immediate
		if (m_info.imm > 0)
			m_r[R2] = rotl32(m_r[R2], m_info.imm);
		else
			m_r[R2] = rotr32(m_r[R2], -m_info.imm);
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0);
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x3d:
		// rotli: rotate longword immediate
		if (m_info.imm > 0)
			((u64 *)m_r)[R2 >> 1] = rotl64(((u64 *)m_r)[R2 >> 1], m_info.imm);
		else
			((u64 *)m_r)[R2 >> 1] = rotr64(((u64 *)m_r)[R2 >> 1], -m_info.imm);
		FLAGS(0, 0, ((s64 *)m_r)[R2 >> 1] == 0, ((s64 *)m_r)[R2 >> 1] < 0);
		// FLAGS: 00ZN
		// TRAPS: I
		break;

	case 0x44:
	case 0x45:
		// call: call subroutine
		m_r[R2] -= 4;
		m_data->write_dword(m_r[R2], next_pc);
		next_pc = m_info.address;
		// TRAPS: A,P,W
		break;
#ifdef UNIMPLEMENTED_C400
	case 0x46:
	case 0x47:
		// loadd2:
		break;
#endif
	case 0x48:
	case 0x49:
		// b*: branch on condition
		if (evaluate_branch())
			next_pc = m_info.address;
		// TRAPS: A,I
		break;
#ifdef UNIMPLEMENTED_C400
	case 0x4a:
	case 0x4b:
		// cdb:
		break;
	case 0x4c:
	case 0x4d:
		// cdbeq:
		break;
	case 0x4e:
	case 0x4f:
		// cdbne:
		break;
	case 0x50:
	case 0x51:
		// db*:
		break;
#endif
#ifdef UNIMPLEMENTED
	case 0x4c:
	case 0x4d:
		// bf*:
		break;
#endif

	case 0x60:
	case 0x61:
		// loadw: load word
		m_r[R2] = m_data->read_dword(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x62:
	case 0x63:
		// loada: load address
		m_r[R2] = m_info.address;
		// TRAPS: I
		break;
	case 0x64:
	case 0x65:
		// loads: load single floating
		((u64 *)&m_f)[R2] = m_data->read_dword(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x66:
	case 0x67:
		// loadd: load double floating
		((u64 *)&m_f)[R2] = m_data->read_qword(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x68:
	case 0x69:
		// loadb: load byte
		m_r[R2] = (s8)m_data->read_byte(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6a:
	case 0x6b:
		// loadbu: load byte unsigned
		m_r[R2] = m_data->read_byte(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6c:
	case 0x6d:
		// loadh: load halfword
		m_r[R2] = (s16)m_data->read_word(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x6e:
	case 0x6f:
		// loadhu: load halfword unsigned
		m_r[R2] = m_data->read_word(m_info.address);
		// TRAPS: C,U,A,P,R,I
		break;
	case 0x70:
	case 0x71:
		// storw: store word
		m_data->write_dword(m_info.address, m_r[R2]);
		// TRAPS: A,P,W,I
		break;
	case 0x72:
	case 0x73:
		// tsts: test and set
		m_r[R2] = m_data->read_dword(m_info.address);
		m_data->write_dword(m_info.address, m_r[R2] | 0x80000000);
		// TRAPS: C,U,A,P,R,W,I
		break;
	case 0x74:
	case 0x75:
		// stors: store single floating
		m_data->write_dword(m_info.address, *((u32 *)&m_f[R2]));
		// TRAPS: A,P,W,I
		break;
	case 0x76:
	case 0x77:
		// stord: store double floating
		m_data->write_qword(m_info.address, *((u64 *)&m_f[R2]));
		// TRAPS: A,P,W,I
		break;
	case 0x78:
	case 0x79:
		// storb: store byte
		m_data->write_byte(m_info.address, (u8)m_r[R2]);
		// TRAPS: A,P,W,I
		break;

	case 0x7c:
	case 0x7d:
		// storh: store halfword
		m_data->write_word(m_info.address, (u16)m_r[R2]);
		// TRAPS: A,P,W,I
		break;

	case 0x80:
		// addw: add word
		FLAGS_CV(C_ADD(m_r[R2], m_r[R1]), V_ADD(m_r[R2], m_r[R1]))
		m_r[R2] += m_r[R1];
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;

	case 0x82:
		// addq: add quick
		FLAGS_CV(C_ADD(m_r[R2], R1), V_ADD(m_r[R2], R1))
		m_r[R2] += R1;
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;
	case 0x83:
		// addi: add immediate
		FLAGS_CV(C_ADD(m_r[R2], m_info.imm), V_ADD(m_r[R2], m_info.imm))
		m_r[R2] += m_info.imm;
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		// TRAPS: I
		break;
	case 0x84:
		// movw: move word
		m_r[R2] = m_r[R1];
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		break;

	case 0x86:
		// loadq: load quick
		m_r[R2] = R1;
		FLAGS(0, 0, m_r[R2] == 0, 0)
		// FLAGS: 00Z0
		break;
	case 0x87:
		// loadi: load immediate
		m_r[R2] = m_info.imm;
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x88:
		// andw: and word
		m_r[R2] &= m_r[R1];
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		break;

	case 0x8b:
		// andi: and immediate
		m_r[R2] &= m_info.imm;
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x8c:
		// orw: or word
		m_r[R2] |= m_r[R1];
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		break;

	case 0x8f:
		// ori: or immediate
		m_r[R2] |= m_info.imm;
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0x90:
		// addwc: add word with carry
		FLAGS_CV(C_ADD(m_r[R2], (m_r[R1] + (PSW(C) ? 1 : 0))), V_ADD(m_r[R2], (m_r[R1] + (PSW(C) ? 1 : 0))))
		m_r[R2] += m_r[R1] + (PSW(C) ? 1 : 0);
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;
	case 0x91:
		// subwc: subtract word with carry
		FLAGS_CV(C_SUB(m_r[R2], (m_r[R1] + (PSW(C) ? 1 : 0))), V_SUB(m_r[R2], (m_r[R1] + (PSW(C) ? 1 : 0))))
		m_r[R2] -= m_r[R1] + (PSW(C) ? 1 : 0);
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;

	case 0x93:
		// negw: negate word
		FLAGS_CV(m_r[R1] != 0, m_r[R1] == INT32_MIN)
		m_r[R2] = -m_r[R1];
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;

	case 0x98:
		// mulw: multiply word
		m_r[R2] = m_r[R2] * m_r[R1];
		// FLAGS: 0V00
		break;
	case 0x99:
		// mulwx: multiply word extended
		((s64 *)m_r)[R2 >> 1] = (s64)m_r[R2] * (s64)m_r[R1];
		// FLAGS: 0V00
		break;
	case 0x9a:
		// mulwu: multiply word unsigned
		m_r[R2] = (u32)m_r[R2] * (u32)m_r[R1];
		// FLAGS: 0V00
		break;
	case 0x9b:
		// mulwux: multiply word unsigned extended
		((u64 *)m_r)[R2 >> 1] = (u64)m_r[R2] * (u64)m_r[R1];
		// FLAGS: 0V00
		break;
	case 0x9c:
		// divw: divide word
		if (m_r[R1] != 0)
		{
			FLAGS(0, m_r[R2] == INT32_MIN && m_r[R1] == -1, 0, 0)
			m_r[R2] = m_r[R2] / m_r[R1];
		}
		else
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		// FLAGS: 0V00
		// TRAPS: D
		break;
	case 0x9d:
		// modw: modulus word
		if (m_r[R1] != 0)
		{
			FLAGS(0, m_r[R2] == INT32_MIN && m_r[R1] == -1, 0, 0)
			m_r[R2] = m_r[R2] % m_r[R1];
		}
		else
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		// FLAGS: 0V00
		// TRAPS: D
		break;
	case 0x9e:
		// divwu: divide word unsigned
		if ((u32)m_r[R1] != 0)
			m_r[R2] = (u32)m_r[R2] / (u32)m_r[R1];
		else
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		FLAGS(0, 0, 0, 0)
		// FLAGS: 0000
		// TRAPS: D
		break;
	case 0x9f:
		// modwu: modulus word unsigned
		if ((u32)m_r[R1] != 0)
			m_r[R2] = (u32)m_r[R2] % (u32)m_r[R1];
		else
			next_pc = intrap(EXCEPTION_INTEGER_DIVIDE_BY_ZERO, next_pc, CTS_DIVIDE_BY_ZERO);
		FLAGS(0, 0, 0, 0)
		// FLAGS: 0000
		// TRAPS: D
		break;
	case 0xa0:
		// subw: subtract word
		FLAGS_CV(C_SUB(m_r[R2], m_r[R1]), V_SUB(m_r[R2], m_r[R1]))
		m_r[R2] -= m_r[R1];
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;

	case 0xa2:
		// subq: subtract quick
		FLAGS_CV(C_SUB(m_r[R2], R1), V_SUB(m_r[R2], R1))
		m_r[R2] -= R1;
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		break;
	case 0xa3:
		// subi: subtract immediate
		FLAGS_CV(C_SUB(m_r[R2], m_info.imm), V_SUB(m_r[R2], m_info.imm))
		m_r[R2] -= m_info.imm;
		FLAGS_ZN(m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: CVZN
		// TRAPS: I
		break;
	case 0xa4:
		// cmpw: compare word
		FLAGS(C_SUB(m_r[R2], m_r[R1]), V_SUB(m_r[R2], m_r[R1]), m_r[R2] == m_r[R1], m_r[R2] < m_r[R1])
		// FLAGS: CVZN
		break;

	case 0xa6:
		// cmpq: compare quick
		FLAGS(C_SUB(m_r[R2], R1), V_SUB(m_r[R2], R1), m_r[R2] == (s32)R1, m_r[R2] < (s32)R1)
		// FLAGS: CVZN
		break;
	case 0xa7:
		// cmpi: compare immediate
		FLAGS(C_SUB(m_r[R2], m_info.imm), V_SUB(m_r[R2], m_info.imm), m_r[R2] == m_info.imm, m_r[R2] < m_info.imm)
		// FLAGS: CVZN
		// TRAPS: I
		break;
	case 0xa8:
		// xorw: exclusive or word
		m_r[R2] ^= m_r[R1];
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		break;

	case 0xab:
		// xori: exclusive or immediate
		m_r[R2] ^= m_info.imm;
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		// TRAPS: I
		break;
	case 0xac:
		// notw: not word
		m_r[R2] = ~m_r[R1];
		FLAGS(0, 0, m_r[R2] == 0, m_r[R2] < 0)
		// FLAGS: 00ZN
		break;

	case 0xae:
		// notq: not quick
		m_r[R2] = ~R1;
		FLAGS(0, 0, 0, 1)
		// FLAGS: 0001
		break;

#ifdef UNIMPLEMENTED_C400
	case 0xb0:
		// abss: absolute value single floating?
		break;

	case 0xb2:
		// absd: absolute value double floating?
		break;
#endif

	case 0xb4:
		// unprivileged macro instructions
		switch (m_info.subopcode)
		{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c:
			// savew0..savew12: push registers rN:r14

			// store ri at sp - 4 * (15 - i)
			for (int i = R2; i < 15; i++)
				m_data->write_dword(m_r[15] - 4 * (15 - i), m_r[i]);

			// decrement sp after push to allow restart on exceptions
			m_r[15] -= 4 * (15 - R2);
			// TRAPS: A,P,W
			break;
		// NOTE: the movc, initc and cmpc macro instructions are implemented in a very basic way because
		// at some point they will need to be improved to deal with possible exceptions (e.g. page faults)
		// that may occur during execution. The implementation here is intended to allow the instructions
		// to be "continued" after such exceptions.
		case 0x0d:
			// movc: copy r0 bytes from r1 to r2

			while (m_r[0])
			{
				m_data->write_byte(m_r[2], m_data->read_byte(m_r[1]));

				m_r[0]--;
				m_r[1]++;
				m_r[2]++;
			}
			// TRAPS: C,U,P,R,W
			break;
		case 0x0e:
			// initc: initialise r0 bytes at r1 with value in r2
			while (m_r[0])
			{
				m_data->write_byte(m_r[1], m_r[2] & 0xff);

				m_r[0]--;
				m_r[1]++;
				m_r[2] = rotr32(m_r[2], 8);
			}
			// TRAPS: P,W
			break;
		case 0x0f:
			// cmpc: compare r0 bytes at r1 with r2

			// set condition codes assuming strings match
			FLAGS(0, 0, 1, 0);

			while (m_r[0])
			{
				// set condition codes and abort the loop if the current byte does not match
				s32 byte1 = (s8)m_data->read_byte(m_r[1]);
				s32 byte2 = (s8)m_data->read_byte(m_r[2]);
				if (byte1 != byte2)
				{
					FLAGS(C_SUB(byte2, byte1), V_SUB(byte2, byte1), byte2 == byte1, byte2 < byte1)
					break;
				}

				m_r[0]--;
				m_r[1]++;
				m_r[2]++;
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
				m_r[i] = m_data->read_dword(m_r[15] + 4 * (i - R2));

			// increment sp after pop to allow restart on exceptions
			m_r[15] += 4 * (15 - R2);
			// TRAPS: C,U,A,P,R
			break;

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
			// saved0..saved7: push registers fN:f7

			// store fi at sp - 8 * (8 - i)
			for (int i = R2; i < 8; i++)
				m_data->write_qword(m_r[15] - 8 * (8 - i), m_f[i]);

			// decrement sp after push to allow restart on exceptions
			m_r[15] -= 8 * (8 - R2);
			// TRAPS: A,P,W
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			// restd0..restd7: pop registers fN:f7

			// load fi from sp + 8 * (i - N)
			for (int i = R2; i < 8; i++)
				m_f[i] = m_data->read_qword(m_r[15] + 8 * (i - R2));

			// increment sp after pop to allow restart on exceptions
			m_r[15] += 8 * (8 - R2);
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
			m_r[m_info.macro & 0xf] = (s32)m_f[(m_info.macro >> 4) & 0xf];
			// TRAPS: F_IX
			break;
		case 0x37: // cnvwd
			m_f[m_info.macro & 0xf] = (double)m_r[(m_info.macro >> 4) & 0xf];
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
		if (!SSW(U))
		{
			switch (m_info.subopcode)
			{
			case 0x00:
				// movus: move user to supervisor
				m_rs[m_info.macro & 0xf] = m_ru[(m_info.macro >> 4) & 0xf];
				FLAGS(0, 0, m_rs[m_info.macro & 0xf] == 0, m_rs[m_info.macro & 0xf] < 0)
				// FLAGS: 00ZN
				// TRAPS: S
				break;
			case 0x01:
				// movsu: move supervisor to user
				m_ru[m_info.macro & 0xf] = m_rs[(m_info.macro >> 4) & 0xf];
				FLAGS(0, 0, m_ru[m_info.macro & 0xf] == 0, m_ru[m_info.macro & 0xf] < 0)
				// FLAGS: 00ZN
				// TRAPS: S
				break;
			case 0x02:
				// saveur: save user registers
				for (int i = 0; i < 16; i++)
					m_data->write_dword(m_rs[(m_info.macro >> 4) & 0xf] - 4 * (i + 1), m_ru[15 - i]);

				m_rs[(m_info.macro >> 4) & 0xf] -= 64;
				// TRAPS: A,P,W,S
				break;
			case 0x03:
				// restur: restore user registers
				for (int i = 0; i < 16; i++)
					m_ru[i] = m_data->read_dword(m_rs[(m_info.macro >> 4) & 0xf] + 4 * i);

				m_rs[(m_info.macro >> 4) & 0xf] += 64;
				// TRAPS: C,U,A,P,R,S
				break;
			case 0x04:
				// reti: restore psw, ssw and pc from supervisor stack
				LOG_INTERRUPT("reti r%d, ssp = %08x, pc = %08x, next_pc = %08x\n",
					(m_info.macro >> 4) & 0xf, m_rs[(m_info.macro >> 4) & 0xf], m_pc, m_data->read_dword(m_rs[(m_info.macro >> 4) & 0xf] + 8));

				m_psw = m_data->read_dword(m_rs[(m_info.macro >> 4) & 0xf] + 0);
				set_ssw(m_data->read_dword(m_rs[(m_info.macro >> 4) & 0xf] + 4));
				next_pc = m_data->read_dword(m_rs[(m_info.macro >> 4) & 0xf] + 8);

				m_rs[(m_info.macro >> 4) & 0xf] += 12;

				m_r = SSW(U) ? m_ru : m_rs;
				// TRAPS: S
				break;
			case 0x05:
				// wait: wait for interrupt
				next_pc = m_pc;
				// TRAPS: S
				break;
#ifdef UNIMPLEMENTED_C400
			case 0x07:
				// loadts: unknown?
				break;
#endif

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

#ifdef UNIMPLEMENTED_C400
	case 0xbc:
		// waitd:
		break;

	case 0xc0:
		// s*:
		break;
#endif

	default:
		logerror("illegal opcode at 0x%08x\n", m_pc);
		next_pc = intrap(EXCEPTION_ILLEGAL_OPERATION, next_pc, CTS_ILLEGAL_OPERATION);
		break;
	}

	return next_pc;
}

/*
* Common entry point for transferring control in the event of an interrupt or exception.
*/
u32 clipper_device::intrap(u32 vector, u32 pc, u32 cts, u32 mts)
{
	LOG_INTERRUPT("intrap - vector %x, pc = 0x%08x, next_pc = 0x%08x, ssp = 0x%08x\n", vector, pc, m_data->read_dword(vector + 4), m_rs[15]);

	// set cts and mts to indicate source of exception
	m_psw = (m_psw & ~(PSW_CTS | PSW_MTS)) | mts | cts;

	// push pc, psw and ssw onto supervisor stack
	m_data->write_dword(m_rs[15] - 4, pc);
	m_data->write_dword(m_rs[15] - 12, m_psw);
	m_data->write_dword(m_rs[15] - 8, m_ssw);

	// decrement supervisor stack pointer

	// NOTE: while not explicitly stated anywhere, it seems the InterPro boot code has been
	// developed with the assumption that the SSP is decremented by 24 bytes during an exception,
	// rather than the 12 bytes that might otherwise be expected. This means the exception handler
	// code must explicitly increment the SSP by 12 prior to executing the RETI instruction,
	// as otherwise the SSP will not be pointing at a valid return frame. It's possible this
	// behaviour might vary with some other version of the CPU, but this is all we know for now.
	m_rs[15] -= 24;

	// load ssw from trap vector and set previous mode
	set_ssw((m_data->read_dword(vector + 0) & ~(SSW(P))) | (SSW(U) << 1));

	// clear psw
	m_psw = 0;

	m_r = SSW(U) ? m_ru : m_rs;

	// return new pc from trap vector
	return m_data->read_dword(vector + 4);
}

bool clipper_device::evaluate_branch ()
{
	switch (m_info.r2)
	{
	case BRANCH_T:
		return true;

	case BRANCH_LT:
		return (!PSW(V) && !PSW(Z) && !PSW(N))
			|| (PSW(V) && !PSW(Z) && PSW(N));

	case BRANCH_LE:
		return (!PSW(V) && !PSW(N))
			|| (PSW(V) && !PSW(Z) && PSW(N));

	case BRANCH_EQ:
		return PSW(Z) && !PSW(N);

	case BRANCH_GT:
		return (!PSW(V) && !PSW(Z) && PSW(N))
			|| (PSW(V) && !PSW(N));

	case BRANCH_GE:
		return (PSW(V) && !PSW(N))
			|| (!PSW(V) && !PSW(Z) && PSW(N))
			|| (PSW(Z) && !PSW(N));

	case BRANCH_NE:
		return (!PSW(Z))
			|| (PSW(Z) && PSW(N));

	case BRANCH_LTU:
		return (!PSW(C) && !PSW(Z));

	case BRANCH_LEU:
		return !PSW(C);

	case BRANCH_GTU:
		return PSW(C);

	case BRANCH_GEU:
		return PSW(C) || PSW(Z);

	case BRANCH_V:
		return PSW(V);
	case BRANCH_NV:
		return !PSW(V);

	case BRANCH_N:
		return !PSW(Z) && PSW(N);
	case BRANCH_NN:
		return !PSW(N);

	case BRANCH_FN:
		return PSW(Z) && PSW(N);
	}

	return false;
}

offs_t clipper_device::disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options)
{
	return CPU_DISASSEMBLE_NAME(clipper)(this, stream, pc, oprom, opram, options);
}
