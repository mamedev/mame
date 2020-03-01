// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Research and Office Products Division Microprocessor (ROMP).
 *
 * Sources:
 *   - http://bitsavers.org/pdf/ibm/pc/rt/6489893_RT_PC_Technical_Reference_Volume_1_Nov85.pdf
 *
 * TODO:
 *   - unimplemented instructions
 *   - condition codes
 *   - memory management unit
 *   - interrupts and exceptions
 *   - timer/counter
 *   - assembler syntax
 *   - instruction clocks
 */

#include "emu.h"
#include "romp.h"
#include "rompdasm.h"
#include "debugger.h"

#define LOG_GENERAL (1U << 0)
//#define VERBOSE     (LOG_GENERAL)
#include "logmacro.h"

// instruction decode helpers
#define R2 ((op >> 4) & 15)
#define R3 (op & 15)

DEFINE_DEVICE_TYPE(ROMP, romp_device, "romp", "IBM ROMP")

ALLOW_SAVE_TYPE(romp_device::branch_state);

romp_device::romp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, ROMP, tag, owner, clock)
	, m_mem_config("memory", ENDIANNESS_BIG, 32, 32)
	, m_io_config("io", ENDIANNESS_BIG, 32, 24, -2)
	, m_icount(0)
{
}

void romp_device::device_start()
{
	// set instruction counter
	set_icountptr(m_icount);

	// register state for the debugger
	state_add(STATE_GENPC,     "GENPC", m_scr[IAR]).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_scr[IAR]).noshow();

	state_add(ROMP_SCR + IAR, "IAR", m_scr[IAR]);
	state_add(ROMP_SCR + COS, "COS", m_scr[COS]);
	state_add(ROMP_SCR + COU, "COU", m_scr[COU]);
	state_add(ROMP_SCR + TS,  "TS",  m_scr[TS]);
	state_add(ROMP_SCR + MQ,  "MQ",  m_scr[MQ]);
	state_add(ROMP_SCR + MCS, "MCS", m_scr[MCS]);
	state_add(ROMP_SCR + IRB, "IRB", m_scr[IRB]);
	state_add(ROMP_SCR + ICS, "ICS", m_scr[ICS]);
	state_add(ROMP_SCR + CS,  "CS",  m_scr[CS]);

	for (unsigned i = 0; i < ARRAY_LENGTH(m_gpr); i++)
		state_add(ROMP_GPR + i, util::string_format("R%d", i).c_str(), m_gpr[i]);

	// register state for saving
	save_item(NAME(m_scr));
	save_item(NAME(m_gpr));
	save_item(NAME(m_branch_state));
	save_item(NAME(m_branch_target));
}

void romp_device::device_reset()
{
	// initialize the state
	// FIXME: should fetch initial address from 0
	m_scr[IAR] = space(AS_PROGRAM).read_dword(0x80'0000);
	m_branch_state = NONE;
}

void romp_device::execute_run()
{
	// core execution loop
	while (m_icount-- > 0)
	{
		// debugging
		debugger_instruction_hook(m_scr[IAR]);

		// TODO: interrupts/exceptions

		// fetch instruction
		u16 const op = space(AS_PROGRAM).read_word(m_scr[IAR]);
		m_scr[IAR] += 2;

		// TODO: program check for illegal branch subject instructions

		switch (op >> 12)
		{
		case 0x0: // jb/jnb: jump on [not] condition bit
			if (BIT(m_scr[CS], ((op >> 8) & 7) ^ 7) == BIT(op, 11))
				m_scr[IAR] = m_scr[IAR] - 2 + ji(op);
			break;
		case 0x1: // stcs: store character short
			space(AS_PROGRAM).write_byte(r3_0(R3) + ((op >> 8) & 15), m_gpr[R2]);
			break;
		case 0x2: // sths: store half short
			space(AS_PROGRAM).write_word(r3_0(R3) + ((op >> 7) & 30), m_gpr[R2]);
			break;
		case 0x3: // sts: store short
			space(AS_PROGRAM).write_dword(r3_0(R3) + ((op >> 6) & 60), m_gpr[R2]);
			break;
		case 0x4: // lcs: load character short
			m_gpr[R2] = space(AS_PROGRAM).read_byte(r3_0(R3) + ((op >> 8) & 15));
			break;
		case 0x5: // lhas: load half algebraic short
			m_gpr[R2] = s32(s16(space(AS_PROGRAM).read_word(r3_0(R3) + ((op >> 7) & 30))));
			break;
		case 0x6: // cas: compute address short
			m_gpr[(op >> 8) & 15] = m_gpr[R2] + r3_0(R3);
			break;
		case 0x7: // ls: load short
			m_gpr[R2] = space(AS_PROGRAM).read_dword(r3_0(R3) + ((op >> 6) & 60));
			break;
		case 0x8: // BI, BA format
			{
				u16 const b = space(AS_PROGRAM).read_word(m_scr[IAR]);
				m_scr[IAR] += 2;

				switch (op >> 8)
				{
				case 0x88: // bnb: branch on not condition bit immediate
					if (!BIT(m_scr[CS], R2 ^ 15))
						m_scr[IAR] = m_scr[IAR] - 4 + bi(op, b);
					break;
				case 0x89: // bnbx: branch on not condition bit immediate with execute
					if (!BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_target = m_scr[IAR] - 4 + bi(op, b);
						m_branch_state = BRANCH;
					}
					break;
				case 0x8a: // bala: branch and link absolute
					m_gpr[15] = m_scr[IAR];
					m_scr[IAR] = ba(op, b);
					break;
				case 0x8b: // balax: branch and link absolute with execute
					m_gpr[15] = m_scr[IAR] + 4;
					m_branch_target = ba(op, b);
					m_branch_state = BRANCH;
					break;
				case 0x8c: // bali: branch and link immediate
					m_gpr[R2] = m_scr[IAR];
					m_scr[IAR] = m_scr[IAR] - 4 + bi(op, b);
					break;
				case 0x8d: // balix: branch and link immediate with execute
					m_gpr[R2] = m_scr[IAR] + 4;
					m_branch_target = m_scr[IAR] - 4 + bi(op, b);
					m_branch_state = BRANCH;
					break;
				case 0x8e: // bb: branch on condition bit immediate
					if (BIT(m_scr[CS], R2 ^ 15))
						m_scr[IAR] = m_scr[IAR] - 4 + bi(op, b);
					break;
				case 0x8f: // bbx: branch on condition bit immediate with execute
					if (BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_target = m_scr[IAR] - 4 + bi(op, b);
						m_branch_state = BRANCH;
					}
					break;
				}
			}
			break;

		case 0xc:
		case 0xd: // D format
			{
				u16 const i = space(AS_PROGRAM).read_word(m_scr[IAR]);
				m_scr[IAR] += 2;

				u32 const r3 = R3 ? m_gpr[R3] : 0;

				switch (op >> 8)
				{
				//case 0xc0: // svc: supervisor call
				case 0xc1: // ai: add immediate
					m_gpr[R2] = m_gpr[R3] + s16(i);
					// TODO: LT, EQ, GT, C0, OV
					break;
				case 0xc2: // cal16: compute address lower half 16-bit
					m_gpr[R2] = (r3 & 0xffff'0000U) | u16(r3 + i);
					break;
				case 0xc3: // oiu: or immediate upper half
					m_gpr[R2] = (u32(i) << 16) | m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xc4: // oil: or immediate lower half
					m_gpr[R2] = u32(i) | m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xc5: // nilz: and immediate lower half extended zeroes
					m_gpr[R2] = u32(i) & m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xc6: // nilo: and immediate lower half extended ones
					m_gpr[R2] = (i | 0xffff'0000U) & m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xc7: // xil: exclusive or immediate lower half
					m_gpr[R2] = u32(i) ^ m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xc8: // cal: compute address lower half
					m_gpr[R2] = r3 + s16(i);
					break;
				case 0xc9: // lm: load multiple
					for (unsigned reg = R2, offset = 0; reg < 16; reg++, offset += 4)
						m_gpr[reg] = space(AS_PROGRAM).read_dword(r3 + s16(i) + offset);
					break;
				case 0xca: // lha: load half algebraic
					m_gpr[R2] = s32(s16(space(AS_PROGRAM).read_word(r3 + s16(i))));
					break;
				case 0xcb: // ior: input/output read
					m_gpr[R2] = space(2).read_dword(r3 + i);
					break;
				//case 0xcc: // ti: trap on condition immediate
				case 0xcd: // l: load
					m_gpr[R2] = space(AS_PROGRAM).read_dword(r3 + s16(i));
					break;
				case 0xce: // lc: load character
					m_gpr[R2] = space(AS_PROGRAM).read_byte(r3 + s16(i));
					break;
				case 0xcf: // tsh: test and set half
					m_gpr[R2] = space(AS_PROGRAM).read_word(r3 + s16(i));
					space(AS_PROGRAM).write_byte(r3 + s16(i), 0xff);
					break;

				case 0xd0: // lps: load program status
					m_scr[IAR] = space(AS_PROGRAM).read_dword(r3 + s16(i) + 0);
					m_scr[ICS] = space(AS_PROGRAM).read_word(r3 + s16(i) + 4);
					m_scr[CS] = space(AS_PROGRAM).read_word(r3 + s16(i) + 6);
					// TODO: clear MCS/PCS
					// TODO: defer interrupt enable
					break;
				case 0xd1: // aei: add extended immediate
					m_gpr[R2] = s32(s16(i)) + m_gpr[R3] + BIT(m_scr[CS], C0);
					// TODO: LT, EQ, GT, C0, OV
					break;
				case 0xd2: // sfi: subtract from immediate
					m_gpr[R2] = s32(s16(i)) - m_gpr[R3];
					// TODO: LT, EQ, GT, C0, OV
					break;
				case 0xd3: // cli: compare logical immediate
					{
						// LT, EQ, GT
						m_scr[CS] &= ~(7U << GT);

						if (m_gpr[R2] == u32(s32(s16(i))))
							m_scr[CS] |= (1U << EQ);
						else
							if (m_gpr[R2] < u32(s32(s16(i))))
								m_scr[CS] |= (1U << LT);
							else
								m_scr[CS] |= (1U << GT);
					}
					break;
				case 0xd4: // ci: compare immediate
					{
						// LT, EQ, GT
						m_scr[CS] &= ~(7U << GT);

						if (m_gpr[R2] == s32(s16(i)))
							m_scr[CS] |= (1U << EQ);
						else
							if (s32(m_gpr[R2]) < s16(i))
								m_scr[CS] |= (1U << LT);
							else
								m_scr[CS] |= (1U << GT);
					}
					break;
				case 0xd5: // niuz: and immediate upper half extended zeroes
					m_gpr[R2] = (u32(i) << 16) & m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xd6: // niuo: and immediate upper half extended ones
					m_gpr[R2] = ((u32(i) << 16) | 0x0000'ffffU) & m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xd7: // xiu: exclusive or immediate upper half
					m_gpr[R2] = (u32(i) << 16) ^ m_gpr[R3];
					// TODO: LT, EQ, GT
					break;
				case 0xd8: // cau: compute address upper half
					m_gpr[R2] = r3 + (u32(i) << 16);
					break;
				case 0xd9: // stm: store multiple
					for (unsigned reg = R2, offset = 0; reg < 16; reg++, offset += 4)
						space(AS_PROGRAM).write_dword(r3 + s16(i) + offset, m_gpr[reg]);
					break;
				case 0xda: // lh: load half
					m_gpr[R2] = space(AS_PROGRAM).read_word(r3 + s16(i));
					break;
				case 0xdb: // iow: input/output write
					// TODO: program check on upper address byte set
					space(2).write_dword(r3 + i, m_gpr[R2]);
					break;
				case 0xdc: // sth: store half
					space(AS_PROGRAM).write_word(r3 + s16(i), m_gpr[R2]);
					break;
				case 0xdd: // st: store
					space(AS_PROGRAM).write_dword(r3 + s16(i), m_gpr[R2]);
					break;
				case 0xde: // stc: store character
					space(AS_PROGRAM).write_byte(r3 + s16(i), m_gpr[R2]);
					break;
				}
			}
			break;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xe:
		case 0xf: // R format
			switch (op >> 8)
			{
			case 0x90: // ais: add immediate short
				m_gpr[R2] += R3;
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0x91: // inc: increment
				m_gpr[R2] += R3;
				break;
			case 0x92: // sis: subtract immediate short
				m_gpr[R2] -= R3;
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0x93: // dec: decrement
				m_gpr[R2] -= R3;
				break;
			case 0x94: // cis: compare immediate short
				{
					// LT, EQ, GT
					m_scr[CS] &= ~(7U << GT);

					if (m_gpr[R2] == R3)
						m_scr[CS] |= (1U << EQ);
					else
						if (s32(m_gpr[R2]) < R3)
							m_scr[CS] |= (1U << LT);
						else
							m_scr[CS] |= (1U << GT);
				}
				break;
			case 0x95: // clrsb: clear scr bit
				m_scr[R2] &= ~(0x0000'8000U >> R3);
				// TODO: side effects?
				break;
			case 0x96: // mfs: move from scr
				m_gpr[R3] = m_scr[R2];
				break;
			case 0x97: // setsb: set scr bit
				m_scr[R2] |= (0x0000'8000U >> R3);
				// TODO: side effects, reserved bits
				break;
			case 0x98: // clrbu: clear bit upper half
				m_gpr[R2] &= ~(0x8000'0000U >> R3);
				// TODO: LT, EQ, GT
				break;
			case 0x99: // clrbl: clear bit lower half
				m_gpr[R2] &= ~(0x0000'8000U >> R3);
				// TODO: LT, EQ, GT
				break;
			case 0x9a: // setbu: set bit upper half
				m_gpr[R2] |= (0x8000'0000U >> R3);
				// TODO: LT, EQ, GT
				break;
			case 0x9b: // setbl: set bit lower half
				m_gpr[R2] |= (0x0000'8000U >> R3);
				// TODO: LT, EQ, GT
				break;
			case 0x9c: // mftbiu: move from test bit immediate upper half
				if (BIT(m_scr[CS], TB))
					m_gpr[R2] |= (0x8000'0000U >> R3);
				else
					m_gpr[R2] &= ~(0x8000'0000U >> R3);
				break;
			case 0x9d: // mftbil: move from test bit immediate lower half
				if (BIT(m_scr[CS], TB))
					m_gpr[R2] |= (0x0000'8000U >> R3);
				else
					m_gpr[R2] &= ~(0x0000'8000U >> R3);
				break;
			case 0x9e: // mttbiu: move to test bit immediate upper half
				if (m_gpr[R2] & (0x8000'0000U >> R3))
					m_scr[CS] |= (1U << TB);
				else
					m_scr[CS] &= ~(1U << TB);
				break;
			case 0x9f: // mttbil: move to test bit immediate lower half
				if (m_gpr[R2] & (0x0000'8000U >> R3))
					m_scr[CS] |= (1U << TB);
				else
					m_scr[CS] &= ~(1U << TB);
				break;

			case 0xa0: // sari: shift algebraic right immediate
				m_gpr[R2] = s32(m_gpr[R2]) >> R3;
				// TODO: LT, EQ, GT
				break;
			case 0xa1: // sari16: shift algebraic right immediate plus sixteen
				m_gpr[R2] = s32(m_gpr[R2]) >> (R3 + 16);
				// TODO: LT, EQ, GT
				break;

			case 0xa4: // lis: load immediate short
				m_gpr[R2] = R3;
				break;

			case 0xa8: // sri: shift right immediate
				m_gpr[R2] >>= R3;
				// TODO: LT, EQ, GT
				break;
			case 0xa9: // sri16: shift right immediate plus sixteen
				m_gpr[R2] >>= (R3 + 16);
				// TODO: LT, EQ, GT
				break;
			case 0xaa: // sli: shift left immediate
				m_gpr[R2] <<= R3;
				// TODO: LT, EQ, GT
				break;
			case 0xab: // sli16: shift left immediate plus sixteen
				m_gpr[R2] <<= (R3 + 16);
				// TODO: LT, EQ, GT
				break;
			case 0xac: // srpi: shift right paired immediate
				m_gpr[R2 ^ 1] = m_gpr[R2] >> R3;
				// TODO: LT, EQ, GT
				break;
			case 0xad: // srpi16: shift right paired immediate plus sixteen
				m_gpr[R2 ^ 1] = m_gpr[R2] >> (R3 + 16);
				// TODO: LT, EQ, GT
				break;
			case 0xae: // slpi: shift left paired immediate
				m_gpr[R2 ^ 1] = m_gpr[R2] << R3;
				// TODO: LT, EQ, GT
				break;
			case 0xaf: // slpi16: shift left paired immediate plus sixteen
				m_gpr[R2 ^ 1] = m_gpr[R2] << (R3 + 16);
				// TODO: LT, EQ, GT
				break;

			case 0xb0: // sar: shift algebraic right
				m_gpr[R2] = s32(m_gpr[R2]) >> (m_gpr[R3] & 63);
				// TODO: LT, EQ, GT
				break;
			case 0xb1: // exts: extend sign
				m_gpr[R2] = s16(m_gpr[R3]);
				// TODO: LT, EQ, GT
				break;
			case 0xb2: // sf: subtract from
				m_gpr[R2] = m_gpr[R3] - m_gpr[R2];
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xb3: // cl: compare logical
				{
					// LT, EQ, GT
					m_scr[CS] &= ~(7U << GT);

					if (m_gpr[R2] == m_gpr[R3])
						m_scr[CS] |= (1U << EQ);
					else
						if (m_gpr[R2] < m_gpr[R3])
							m_scr[CS] |= (1U << LT);
						else
							m_scr[CS] |= (1U << GT);
				}
				break;
			case 0xb4: // c: compare
				{
					// LT, EQ, GT
					m_scr[CS] &= ~(7U << GT);

					if (m_gpr[R2] == m_gpr[R3])
						m_scr[CS] |= (1U << EQ);
					else
						if (s32(m_gpr[R2]) < s32(m_gpr[R3]))
							m_scr[CS] |= (1U << LT);
						else
							m_scr[CS] |= (1U << GT);
				}
				break;
			case 0xb5: // mts: move to scr
				m_scr[R2] = m_gpr[R3];
				break;
			//case 0xb6: // d: divide step

			case 0xb8: // sr: shift right
				m_gpr[R2] >>= (m_gpr[R3] & 63);
				// TODO: LT, EQ, GT
				break;
			case 0xb9: // srp: shift right paired
				m_gpr[R2 ^ 1] = m_gpr[R2] >> (m_gpr[R3] & 63);
				// TODO: LT, EQ, GT
				break;
			case 0xba: // sl: shift left
				m_gpr[R2] <<= (m_gpr[R3] & 63);
				// TODO: LT, EQ, GT
				break;
			case 0xbb: // slp: shift left paired
				m_gpr[R2 ^ 1] = m_gpr[R2] << (m_gpr[R3] & 63);
				// TODO: LT, EQ, GT
				break;
			case 0xbc: // mftb: move from test bit
				if (BIT(m_scr[CS], TB))
					m_gpr[R2] |= (0x8000'0000U >> (m_gpr[R3] & 31));
				else
					m_gpr[R2] &= ~(0x8000'0000U >> (m_gpr[R3] & 31));
				break;
			//case 0xbd: // tgte: trap if register greater than or equal
			//case 0xbe: // tlt: trap if register less than
			case 0xbf: // mttb: move to test bit
				if (m_gpr[R2] & (0x8000'0000U >> (m_gpr[R3] & 31)))
					m_scr[CS] |= (1U << TB);
				else
					m_scr[CS] &= ~(1U << TB);
				break;

			case 0xe0: // abs: absolute
				if (s32(m_gpr[R3]) < 0)
					m_gpr[R2] = -s32(m_gpr[R3]);
				else
					m_gpr[R2] = m_gpr[R3];
				// TODO: test for maximum negative
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xe1: // a: add
				m_gpr[R2] += m_gpr[R3];
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xe2: // s: subtract
				m_gpr[R2] -= m_gpr[R3];
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xe3: // o: or
				m_gpr[R2] |= m_gpr[R3];
				// TODO: LT, EQ, GT
				break;
			case 0xe4: // twoc: twos complement
				m_gpr[R2] = -m_gpr[R3];
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xe5: // n: and
				m_gpr[R2] &= m_gpr[R3];
				// TODO: LT, EQ, GT
				break;
			//case 0xe6: // m: multiply step
			case 0xe7: // x: exclusive or
				m_gpr[R2] ^= m_gpr[R3];
				// TODO: LT, EQ, GT
				break;
			case 0xe8: // bnbr: branch on not condition bit
				if (!BIT(m_scr[CS], R2 ^ 15))
					m_scr[IAR] = m_gpr[R3] & ~1;
				break;
			case 0xe9: // bnbrx: branch on not condition bit with execute
				if (!BIT(m_scr[CS], R2 ^ 15))
				{
					m_branch_target = m_gpr[R3] & ~1;
					m_branch_state = BRANCH;
				}
				break;

			case 0xeb: // lhs: load half short
				m_gpr[R2] = space(AS_PROGRAM).read_word(m_gpr[R3]);
				break;
			case 0xec: // balr: branch and link
				m_gpr[R2] = m_scr[IAR];
				m_scr[IAR] = m_gpr[R3] & ~1;
				break;
			case 0xed: // balrx: branch and link with execute
				m_gpr[R2] = m_scr[IAR] + 4;
				m_branch_target = m_gpr[R3] & ~1;
				m_branch_state = BRANCH;
				break;
			case 0xee: // bbr: branch on condition bit
				if (BIT(m_scr[CS], R2 ^ 15))
					m_scr[IAR] = m_gpr[R3] & ~1;
				break;
			case 0xef: // bbrx: branch on condition bit with execute
				if (BIT(m_scr[CS], R2 ^ 15))
				{
					m_branch_target = m_gpr[R3] & ~1;
					m_branch_state = BRANCH;
				}
				break;

			//case 0xf0: // wait: wait
			case 0xf1: // ae: add extended
				m_gpr[R2] += m_gpr[R3] + BIT(m_scr[CS], C0);
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xf2: // se: subtract extended
				m_gpr[R2] -= m_gpr[R3] + BIT(m_scr[CS], C0);
				// TODO: LT, EQ, GT, C0, OV
				break;
			case 0xf3: // ca16: compute address 16-bit
				m_gpr[R2] = (m_gpr[R3] & 0xffff'0000U) | (u16(m_gpr[R2]) + u16(m_gpr[R3]));
				break;
			case 0xf4: // onec: ones complement
				m_gpr[R2] = ~m_gpr[R3];
				// TODO: LT, EQ, GT
				break;
			case 0xf5: // clz: count leading zeros
				m_gpr[R2] = count_leading_zeros(u16(m_gpr[R3])) - 16;
				break;

			case 0xf9: // mc03: move character zero from three
				m_gpr[R2] = (m_gpr[R2] & 0x00ff'ffffU) | ((m_gpr[R3] & 0x0000'000ffU) << 24);
				break;
			case 0xfa: // mc13: move character one from three
				m_gpr[R2] = (m_gpr[R2] & 0xff00'ffffU) | ((m_gpr[R3] & 0x0000'000ffU) << 16);
				break;
			case 0xfb: // mc23: move character two from three
				m_gpr[R2] = (m_gpr[R2] & 0xffff'00ffU) | ((m_gpr[R3] & 0x0000'000ffU) << 8);
				break;
			case 0xfc: // mc33: move character three from three
				m_gpr[R2] = (m_gpr[R2] & 0xffff'ff00U) | ((m_gpr[R3] & 0x0000'000ffU) << 0);
				break;
			case 0xfd: // mc30: move character three from zero
				m_gpr[R2] = (m_gpr[R2] & 0xffff'ff00U) | u8(m_gpr[R3] >> 24);
				break;
			case 0xfe: // mc31: move character three from one
				m_gpr[R2] = (m_gpr[R2] & 0xffff'ff00U) | u8(m_gpr[R3] >> 16);
				break;
			case 0xff: // mc32: move character three from two
				m_gpr[R2] = (m_gpr[R2] & 0xffff'ff00U) | u8(m_gpr[R3] >> 8);
				break;
			}
			break;
		}

		// update iar and branch state
		switch (m_branch_state)
		{
		case SUBJECT:
			m_scr[IAR] = m_branch_target;
			m_branch_state = NONE;
			break;

		case BRANCH:
			m_branch_state = SUBJECT;
			break;

		default:
			break;
		}
	}
}

void romp_device::execute_set_input(int irqline, int state)
{
	// enable debugger interrupt breakpoints
	if (state)
		standard_irq_callback(irqline);
}

device_memory_interface::space_config_vector romp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_mem_config),
		std::make_pair(AS_IO, &m_io_config)
	};
}

bool romp_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	return true;
}

std::unique_ptr<util::disasm_interface> romp_device::create_disassembler()
{
	return std::make_unique<romp_disassembler>();
}
