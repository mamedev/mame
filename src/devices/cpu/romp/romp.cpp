// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Research and Office Products Division Microprocessor (ROMP).
 *
 * Sources:
 *   - IBM RT PC Hardware Technical Reference Volume I, 75X0232, March 1987
 *
 * TODO:
 *   - configurable storage channel
 *   - multiple exceptions
 *   - check stop mask
 *   - advanced/enhanced variants
 */

#include "emu.h"
#include "romp.h"
#include "rompdasm.h"

#define LOG_INTERRUPT (1U << 1)

//#define VERBOSE     (LOG_INTERRUPT)
#include "logmacro.h"

// instruction decode helpers
#define R2 ((op >> 4) & 15)
#define R3 (op & 15)

DEFINE_DEVICE_TYPE(ROMP, romp_device, "romp", "IBM ROMP")

ALLOW_SAVE_TYPE(romp_device::branch_state);

romp_device::romp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, ROMP, tag, owner, clock)
	, m_mem_config("memory", ENDIANNESS_BIG, 32, 32)
	, m_mmu(*this, finder_base::DUMMY_TAG)
	, m_iou(*this, finder_base::DUMMY_TAG)
	, m_icount(0)
	, m_reqi(0)
{
}

void romp_device::device_start()
{
	// set instruction counter
	set_icountptr(m_icount);

	// register state for the debugger
	state_add(STATE_GENPC,     "GENPC", m_scr[IAR]).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_scr[IAR]).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_scr[CS]).formatstr("%6s").noshow();

	state_add(ROMP_SCR + IAR,  "IAR",  m_scr[IAR]);
	state_add(ROMP_SCR + COUS, "COUS", m_scr[COUS]);
	state_add(ROMP_SCR + COU,  "COU",  m_scr[COU]);
	state_add(ROMP_SCR + TS,   "TS",   m_scr[TS]);
	state_add(ROMP_SCR + MQ,   "MQ",   m_scr[MQ]);
	state_add(ROMP_SCR + MPCS, "MPCS", m_scr[MPCS]);
	state_add(ROMP_SCR + IRB,  "IRB",  m_scr[IRB]);
	state_add(ROMP_SCR + ICS,  "ICS",  m_scr[ICS]);
	state_add(ROMP_SCR + CS,   "CS",   m_scr[CS]);

	for (unsigned i = 0; i < std::size(m_gpr); i++)
		state_add(ROMP_GPR + i, util::string_format("R%d", i).c_str(), m_gpr[i]);

	// register state for saving
	save_item(NAME(m_scr));
	save_item(NAME(m_gpr));

	save_item(NAME(m_reqi));
	save_item(NAME(m_trap));

	save_item(NAME(m_branch_state));
	save_item(NAME(m_branch_source));
	save_item(NAME(m_branch_target));
	save_item(NAME(m_defer_int));
}

void romp_device::state_string_export(device_state_entry const &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c",
			(m_scr[CS] & CS_L) ? 'L' : '.',
			(m_scr[CS] & CS_E) ? 'E' : '.',
			(m_scr[CS] & CS_G) ? 'G' : '.',
			(m_scr[CS] & CS_C) ? 'C' : '.',
			(m_scr[CS] & CS_O) ? 'O' : '.',
			(m_scr[CS] & CS_T) ? 'T' : '.');
		break;
	}
}

void romp_device::device_reset()
{
	for (u32 &scr : m_scr)
		scr = 0;

	for (u32 &gpr : m_gpr)
		gpr = 0;

	// initialize the state
	m_trap = false;
	m_branch_state = DEFAULT;
	m_defer_int = false;

	// fetch initial iar
	load<u32>(0, [this](u32 data) { m_scr[IAR] = data; });
}

void romp_device::execute_run()
{
	// core execution loop
	while (m_icount-- > 0)
	{
		if (m_defer_int)
			m_defer_int = false;
		else
			interrupt_check();

		if (m_branch_state == WAIT)
		{
			debugger_wait_hook();
			m_icount = 0;
			return;
		}
		else
			debugger_instruction_hook(m_scr[IAR]);

		// fetch instruction
		u32 updated_iar = m_scr[IAR];
		fetch(m_scr[IAR], [this, &updated_iar](u16 op)
			{
				updated_iar += 2;

		switch (op >> 12)
		{
		case 0x0: // jb/jnb: jump on [not] condition bit
			if (m_branch_state != BRANCH)
			{
				if (BIT(m_scr[CS], ((op >> 8) & 7) ^ 7) == BIT(op, 11))
				{
					m_branch_target = m_scr[IAR] + ji(op);
					m_branch_state = BRANCH;
					m_icount -= 4;
				}
			}
			else
				program_check(PCS_PCK | PCS_IOC);
			break;
		case 0x1: // stcs: store character short
			store<u8>(r3_0(R3) + ((op >> 8) & 15), m_gpr[R2]);
			m_icount -= 4;
			break;
		case 0x2: // sths: store half short
			store<u16>(r3_0(R3) + ((op >> 7) & 30), m_gpr[R2]);
			m_icount -= 4;
			break;
		case 0x3: // sts: store short
			store<u32>(r3_0(R3) + ((op >> 6) & 60), m_gpr[R2]);
			m_icount -= 4;
			break;
		case 0x4: // lcs: load character short
			load<u8>(r3_0(R3) + ((op >> 8) & 15), [this, op](u8 data) { m_gpr[R2] = data; });
			m_icount -= 4;
			break;
		case 0x5: // lhas: load half algebraic short
			load<u16>(r3_0(R3) + ((op >> 7) & 30), [this, op](u16 data) { m_gpr[R2] = s32(s16(data)); });
			m_icount -= 4;
			break;
		case 0x6: // cas: compute address short
			m_gpr[(op >> 8) & 15] = m_gpr[R2] + r3_0(R3);
			break;
		case 0x7: // ls: load short
			load<u32>(r3_0(R3) + ((op >> 6) & 60), [this, op](u32 data) { m_gpr[R2] = data; });
			m_icount -= 4;
			break;
		case 0x8: // BI, BA format
			fetch(updated_iar, [this, &updated_iar, op](u16 b)
			{
				updated_iar += 2;

				if (m_branch_state == BRANCH)
				{
					program_check(PCS_PCK | PCS_IOC);
					return;
				}

				switch (op >> 8)
				{
				case 0x88: // bnb: branch on not condition bit immediate
					if (!BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_target = m_scr[IAR] + bi(op, b);
						m_branch_state = BRANCH;
						m_icount -= 4;
					}
					break;
				case 0x89: // bnbx: branch on not condition bit immediate with execute
					if (!BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_source = m_scr[IAR];
						m_branch_target = m_scr[IAR] + bi(op, b);
						m_branch_state = DELAY;
						m_icount -= 4;
					}
					break;
				case 0x8a: // bala: branch and link absolute
					m_gpr[15] = updated_iar;
					m_branch_target = ba(op, b);
					m_branch_state = BRANCH;
					m_icount -= 4;
					break;
				case 0x8b: // balax: branch and link absolute with execute
					m_gpr[15] = updated_iar + 4;
					m_branch_source = m_scr[IAR];
					m_branch_target = ba(op, b);
					m_branch_state = DELAY;
					m_icount -= 4;
					break;
				case 0x8c: // bali: branch and link immediate
					m_gpr[R2] = updated_iar;
					m_branch_target = m_scr[IAR] + bi(op, b);
					m_branch_state = BRANCH;
					m_icount -= 4;
					break;
				case 0x8d: // balix: branch and link immediate with execute
					m_gpr[R2] = updated_iar + 4;
					m_branch_source = m_scr[IAR];
					m_branch_target = m_scr[IAR] + bi(op, b);
					m_branch_state = DELAY;
					m_icount -= 4;
					break;
				case 0x8e: // bb: branch on condition bit immediate
					if (BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_target = m_scr[IAR] + bi(op, b);
						m_branch_state = BRANCH;
						m_icount -= 4;
					}
					break;
				case 0x8f: // bbx: branch on condition bit immediate with execute
					if (BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_source = m_scr[IAR];
						m_branch_target = m_scr[IAR] + bi(op, b);
						m_branch_state = DELAY;
						m_icount -= 4;
					}
					break;

				default:
					program_check(PCS_PCK | PCS_IOC);
					break;
				}
			});
			break;

		case 0xc:
		case 0xd: // D format
			fetch(updated_iar, [this, &updated_iar, op](u16 i)
			{
				updated_iar += 2;

				u32 const r3 = R3 ? m_gpr[R3] : 0;

				switch (op >> 8)
				{
				case 0xc0: // svc: supervisor call
					if (m_branch_state != BRANCH)
					{
						interrupt_enter(9, updated_iar, r3 + i);
						m_branch_state = EXCEPTION;

						m_icount -= 15;
					}
					else
						program_check(PCS_PCK | PCS_IOC);
					break;
				case 0xc1: // ai: add immediate
					flags_add(m_gpr[R3], s32(s16(i)));
					m_gpr[R2] = m_gpr[R3] + s32(s16(i));
					break;
				case 0xc2: // cal16: compute address lower half 16-bit
					m_gpr[R2] = (r3 & 0xffff'0000U) | u16(r3 + i);
					break;
				case 0xc3: // oiu: or immediate upper half
					m_gpr[R2] = (u32(i) << 16) | m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xc4: // oil: or immediate lower half
					m_gpr[R2] = u32(i) | m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xc5: // nilz: and immediate lower half extended zeroes
					m_gpr[R2] = u32(i) & m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xc6: // nilo: and immediate lower half extended ones
					m_gpr[R2] = (i | 0xffff'0000U) & m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xc7: // xil: exclusive or immediate lower half
					m_gpr[R2] = u32(i) ^ m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xc8: // cal: compute address lower half
					m_gpr[R2] = r3 + s16(i);
					break;
				case 0xc9: // lm: load multiple
					for (unsigned reg = R2, offset = r3 + s16(i); (reg < 16) && m_branch_state != EXCEPTION; reg++, offset += 4)
					{
						load<u32>(offset, [this, reg](u32 data) { m_gpr[reg] = data; });
						m_icount -= 2;
					}
					m_icount -= (m_scr[ICS] & ICS_TM) ? 3 : 1;
					break;
				case 0xca: // lha: load half algebraic
					load<u16>(r3 + s16(i), [this, op](u16 data) { m_gpr[R2] = s32(s16(data)); });
					m_icount -= 4;
					break;
				case 0xcb: // ior: input/output read
					if (((r3 + i) & 0xff00'0000U) || !m_mmu->pio_load(r3 + i, m_gpr[R2]))
						program_check(PCS_PCK | PCS_DAE);
					break;
				case 0xcc: // ti: trap on condition immediate
					if (m_branch_state != BRANCH)
					{
						if ((BIT(op, 6) && (m_gpr[R3] < u32(s32(s16(i)))))
							|| (BIT(op, 5) && (m_gpr[R3] == u32(s32(s16(i)))))
							|| (BIT(op, 4) && (m_gpr[R3] > u32(s32(s16(i))))))
							program_check(PCS_PCK | PCS_PT);
					}
					else
						program_check(PCS_PCK | PCS_IOC);
					break;
				case 0xcd: // l: load
					load<u32>(r3 + s16(i), [this, op](u32 data) { m_gpr[R2] = data; });
					m_icount -= 4;
					break;
				case 0xce: // lc: load character
					load<u8>(r3 + s16(i), [this, op](u8 data) { m_gpr[R2] = data; });
					m_icount -= 4;
					break;
				case 0xcf: // tsh: test and set half
					modify<u16>(r3 + s16(i), [this, op](u16 data)
						{
							m_gpr[R2] = data;

							return 0xff00 | data;
						});
					m_icount -= 4;
					break;

				case 0xd0: // lps: load program status
					if (!(m_scr[ICS] & ICS_US))
					{
						if (m_branch_state != BRANCH)
						{
							load<u32>(r3 + s16(i) + 0, [this](u32 data) { m_branch_target = data; });
							m_branch_state = BRANCH;
							load<u16>(r3 + s16(i) + 6, [this](u16 data) { m_scr[CS] = data; });
							load<u16>(r3 + s16(i) + 4, [this](u16 data) { m_scr[ICS] = data; });
							if (m_scr[MPCS] & MCS_ALL)
								m_scr[MPCS] &= ~MCS_ALL;
							else
								m_scr[MPCS] &= ~PCS_ALL;

							// optionally defer interrupts
							m_defer_int = BIT(op, 4);

							m_icount -= 15;
						}
						else
							program_check(PCS_PCK | PCS_IOC);
					}
					else
						program_check(PCS_PCK | PCS_PIE);
					break;
				case 0xd1: // aei: add extended immediate
					flags_add(m_gpr[R3], s32(s16(i)) + bool(m_scr[CS] & CS_C));
					m_gpr[R2] = m_gpr[R3] + s32(s16(i)) + bool(m_scr[CS] & CS_C);
					break;
				case 0xd2: // sfi: subtract from immediate
					flags_sub(s32(s16(i)), m_gpr[R3]);
					m_gpr[R2] = s32(s16(i)) - m_gpr[R3];
					break;
				case 0xd3: // cli: compare logical immediate
					m_scr[CS] &= ~(CS_L | CS_E | CS_G);
					if (m_gpr[R3] == u32(s32(s16(i))))
						m_scr[CS] |= CS_E;
					else if (m_gpr[R3] < u32(s32(s16(i))))
						m_scr[CS] |= CS_L;
					else
						m_scr[CS] |= CS_G;
					break;
				case 0xd4: // ci: compare immediate
					m_scr[CS] &= ~(CS_L | CS_E | CS_G);
					if (s32(m_gpr[R3]) == s32(s16(i)))
						m_scr[CS] |= CS_E;
					else if (s32(m_gpr[R3]) < s32(s16(i)))
						m_scr[CS] |= CS_L;
					else
						m_scr[CS] |= CS_G;
					break;
				case 0xd5: // niuz: and immediate upper half extended zeroes
					m_gpr[R2] = (u32(i) << 16) & m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xd6: // niuo: and immediate upper half extended ones
					m_gpr[R2] = ((u32(i) << 16) | 0x0000'ffffU) & m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xd7: // xiu: exclusive or immediate upper half
					m_gpr[R2] = (u32(i) << 16) ^ m_gpr[R3];
					flags_log(m_gpr[R2]);
					break;
				case 0xd8: // cau: compute address upper half
					m_gpr[R2] = r3 + (u32(i) << 16);
					break;
				case 0xd9: // stm: store multiple
					for (unsigned reg = R2, offset = r3 + s16(i); (reg < 16) && m_branch_state != EXCEPTION; reg++, offset += 4)
					{
						store<u32>(offset, m_gpr[reg]);
						m_icount -= (m_scr[ICS] & ICS_TM) ? 3 : 2;
					}
					m_icount -= (m_scr[ICS] & ICS_TM) ? 3 : 2;
					break;
				case 0xda: // lh: load half
					load<u16>(r3 + s16(i), [this, op](u16 data) { m_gpr[R2] = data; });
					m_icount -= 4;
					break;
				case 0xdb: // iow: input/output write
					if (((r3 + i) & 0xff00'0000U) || !m_mmu->pio_store(r3 + i, m_gpr[R2]))
						program_check(PCS_PCK | PCS_DAE);
					m_icount--;
					break;
				case 0xdc: // sth: store half
					store<u16>(r3 + s16(i), m_gpr[R2]);
					m_icount -= 4;
					break;
				case 0xdd: // st: store
					store<u32>(r3 + s16(i), m_gpr[R2]);
					m_icount -= 4;
					break;
				case 0xde: // stc: store character
					store<u8>(r3 + s16(i), m_gpr[R2]);
					m_icount -= 4;
					break;

				default:
					program_check(PCS_PCK | PCS_IOC);
					break;
				}
			});
			break;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xe:
		case 0xf: // R format
			switch (op >> 8)
			{
			case 0x90: // ais: add immediate short
				flags_add(m_gpr[R2], R3);
				m_gpr[R2] += R3;
				break;
			case 0x91: // inc: increment
				m_gpr[R2] += R3;
				break;
			case 0x92: // sis: subtract immediate short
				flags_sub(m_gpr[R2], R3);
				m_gpr[R2] -= R3;
				break;
			case 0x93: // dec: decrement
				m_gpr[R2] -= R3;
				break;
			case 0x94: // cis: compare immediate short
				m_scr[CS] &= ~(CS_L | CS_E | CS_G);
				if (m_gpr[R2] == R3)
					m_scr[CS] |= CS_E;
				else if (s32(m_gpr[R2]) < s32(R3))
					m_scr[CS] |= CS_L;
				else
					m_scr[CS] |= CS_G;
				break;
			case 0x95: // clrsb: clear scr bit
				set_scr(R2, m_scr[R2] & ~(0x0000'8000U >> R3));
				m_icount -= 3;
				break;
			case 0x96: // mfs: move from scr
				if (!(m_scr[ICS] & ICS_US) || R2 == MQ || R2 == CS)
				{
					if (R2 == IAR)
						m_gpr[R3] = updated_iar;
					else
						m_gpr[R3] = m_scr[R2];
				}
				else
					program_check(PCS_PCK | PCS_PIE);
				m_icount--;
				break;
			case 0x97: // setsb: set scr bit
				set_scr(R2, m_scr[R2] | (0x0000'8000U >> R3));
				m_icount -= 3;
				break;
			case 0x98: // clrbu: clear bit upper half
				m_gpr[R2] &= ~(0x8000'0000U >> R3);
				flags_log(m_gpr[R2]);
				break;
			case 0x99: // clrbl: clear bit lower half
				m_gpr[R2] &= ~(0x0000'8000U >> R3);
				flags_log(m_gpr[R2]);
				break;
			case 0x9a: // setbu: set bit upper half
				m_gpr[R2] |= (0x8000'0000U >> R3);
				flags_log(m_gpr[R2]);
				break;
			case 0x9b: // setbl: set bit lower half
				m_gpr[R2] |= (0x0000'8000U >> R3);
				flags_log(m_gpr[R2]);
				break;
			case 0x9c: // mftbiu: move from test bit immediate upper half
				if (m_scr[CS] & CS_T)
					m_gpr[R2] |= (0x8000'0000U >> R3);
				else
					m_gpr[R2] &= ~(0x8000'0000U >> R3);
				break;
			case 0x9d: // mftbil: move from test bit immediate lower half
				if (m_scr[CS] & CS_T)
					m_gpr[R2] |= (0x0000'8000U >> R3);
				else
					m_gpr[R2] &= ~(0x0000'8000U >> R3);
				break;
			case 0x9e: // mttbiu: move to test bit immediate upper half
				if (m_gpr[R2] & (0x8000'0000U >> R3))
					m_scr[CS] |= CS_T;
				else
					m_scr[CS] &= ~CS_T;
				break;
			case 0x9f: // mttbil: move to test bit immediate lower half
				if (m_gpr[R2] & (0x0000'8000U >> R3))
					m_scr[CS] |= CS_T;
				else
					m_scr[CS] &= ~CS_T;
				break;

			case 0xa0: // sari: shift algebraic right immediate
				m_gpr[R2] = s32(m_gpr[R2]) >> R3;
				flags_log(m_gpr[R2]);
				break;
			case 0xa1: // sari16: shift algebraic right immediate plus sixteen
				m_gpr[R2] = s32(m_gpr[R2]) >> (R3 + 16);
				flags_log(m_gpr[R2]);
				break;

			case 0xa4: // lis: load immediate short
				m_gpr[R2] = R3;
				break;

			case 0xa8: // sri: shift right immediate
				m_gpr[R2] >>= R3;
				flags_log(m_gpr[R2]);
				break;
			case 0xa9: // sri16: shift right immediate plus sixteen
				m_gpr[R2] >>= (R3 + 16);
				flags_log(m_gpr[R2]);
				break;
			case 0xaa: // sli: shift left immediate
				m_gpr[R2] <<= R3;
				flags_log(m_gpr[R2]);
				break;
			case 0xab: // sli16: shift left immediate plus sixteen
				m_gpr[R2] <<= (R3 + 16);
				flags_log(m_gpr[R2]);
				break;
			case 0xac: // srpi: shift right paired immediate
				m_gpr[R2 ^ 1] = m_gpr[R2] >> R3;
				flags_log(m_gpr[R2 ^ 1]);
				break;
			case 0xad: // srpi16: shift right paired immediate plus sixteen
				m_gpr[R2 ^ 1] = m_gpr[R2] >> (R3 + 16);
				flags_log(m_gpr[R2 ^ 1]);
				break;
			case 0xae: // slpi: shift left paired immediate
				m_gpr[R2 ^ 1] = m_gpr[R2] << R3;
				flags_log(m_gpr[R2 ^ 1]);
				break;
			case 0xaf: // slpi16: shift left paired immediate plus sixteen
				m_gpr[R2 ^ 1] = m_gpr[R2] << (R3 + 16);
				flags_log(m_gpr[R2 ^ 1]);
				break;

			case 0xb0: // sar: shift algebraic right
				m_gpr[R2] = s32(m_gpr[R2]) >> (m_gpr[R3] & 63);
				flags_log(m_gpr[R2]);
				break;
			case 0xb1: // exts: extend sign
				m_gpr[R2] = s16(m_gpr[R3]);
				flags_log(m_gpr[R2]);
				break;
			case 0xb2: // sf: subtract from
				flags_sub(m_gpr[R3], m_gpr[R2]);
				m_gpr[R2] = m_gpr[R3] - m_gpr[R2];
				break;
			case 0xb3: // cl: compare logical
				m_scr[CS] &= ~(CS_L | CS_E | CS_G);
				if (m_gpr[R2] == m_gpr[R3])
					m_scr[CS] |= CS_E;
				else if (m_gpr[R2] < m_gpr[R3])
					m_scr[CS] |= CS_L;
				else
					m_scr[CS] |= CS_G;
				break;
			case 0xb4: // c: compare
				m_scr[CS] &= ~(CS_L | CS_E | CS_G);
				if (s32(m_gpr[R2]) == s32(m_gpr[R3]))
					m_scr[CS] |= CS_E;
				else if (s32(m_gpr[R2]) < s32(m_gpr[R3]))
					m_scr[CS] |= CS_L;
				else
					m_scr[CS] |= CS_G;
				break;
			case 0xb5: // mts: move to scr
				set_scr(R2, m_gpr[R3]);
				m_icount -= 2;
				break;
			case 0xb6: // d: divide step
				{
					m_scr[CS] &= ~(CS_C | CS_O);

					s64 sum = (s64(s32(m_gpr[R2])) << 1) | (m_scr[MQ] >> 31);

					if (BIT(m_gpr[R2], 31) == BIT(m_gpr[R3], 31))
						sum -= s32(m_gpr[R3]);
					else
						sum += s32(m_gpr[R3]);

					// update remainder
					m_gpr[R2] = sum;

					// update quotient
					m_scr[MQ] <<= 1;
					if (BIT(sum, 32) == BIT(m_gpr[R3], 31))
					{
						m_scr[MQ] |= 1;
						m_scr[CS] |= CS_C;
					}

					// overflow test
					if (BIT(sum, 32) == BIT(m_gpr[R2], 31))
						m_scr[CS] |= CS_O;
				}
				m_icount -= 2;
				break;
			case 0xb8: // sr: shift right
				m_gpr[R2] >>= (m_gpr[R3] & 63);
				flags_log(m_gpr[R2]);
				break;
			case 0xb9: // srp: shift right paired
				m_gpr[R2 ^ 1] = m_gpr[R2] >> (m_gpr[R3] & 63);
				flags_log(m_gpr[R2 ^ 1]);
				break;
			case 0xba: // sl: shift left
				m_gpr[R2] <<= (m_gpr[R3] & 63);
				flags_log(m_gpr[R2]);
				break;
			case 0xbb: // slp: shift left paired
				m_gpr[R2 ^ 1] = m_gpr[R2] << (m_gpr[R3] & 63);
				flags_log(m_gpr[R2 ^ 1]);
				break;
			case 0xbc: // mftb: move from test bit
				if (m_scr[CS] & CS_T)
					m_gpr[R2] |= (0x8000'0000U >> (m_gpr[R3] & 31));
				else
					m_gpr[R2] &= ~(0x8000'0000U >> (m_gpr[R3] & 31));
				break;
			case 0xbd: // tgte: trap if register greater than or equal
				if (m_branch_state != BRANCH)
				{
					if (m_gpr[R2] >= m_gpr[R3])
					{
						program_check(PCS_PCK | PCS_PT);
						m_icount -= 14;
					}

					m_icount--;
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;
			case 0xbe: // tlt: trap if register less than
				if (m_branch_state != BRANCH)
				{
					if (m_gpr[R2] < m_gpr[R3])
					{
						program_check(PCS_PCK | PCS_PT);
						m_icount -= 14;
					}

					m_icount--;
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;
			case 0xbf: // mttb: move to test bit
				if (m_gpr[R2] & (0x8000'0000U >> (m_gpr[R3] & 31)))
					m_scr[CS] |= CS_T;
				else
					m_scr[CS] &= ~CS_T;
				break;

			case 0xe0: // abs: absolute
				m_scr[CS] &= ~(CS_C | CS_O);
				if (m_gpr[R3] == 0x8000'0000U)
				{
					m_gpr[R2] = m_gpr[R3];
					m_scr[CS] |= CS_O;
				}
				else if (s32(m_gpr[R3]) < 0)
					m_gpr[R2] = -s32(m_gpr[R3]);
				else
					m_gpr[R2] = m_gpr[R3];
				flags_log(m_gpr[R2]);
				m_icount--;
				break;
			case 0xe1: // a: add
				flags_add(m_gpr[R2], m_gpr[R3]);
				m_gpr[R2] += m_gpr[R3];
				break;
			case 0xe2: // s: subtract
				flags_sub(m_gpr[R2], m_gpr[R3]);
				m_gpr[R2] -= m_gpr[R3];
				break;
			case 0xe3: // o: or
				m_gpr[R2] |= m_gpr[R3];
				flags_log(m_gpr[R2]);
				break;
			case 0xe4: // twoc: twos complement
				flags_sub(0, m_gpr[R3]);
				m_gpr[R2] = -m_gpr[R3];
				break;
			case 0xe5: // n: and
				m_gpr[R2] &= m_gpr[R3];
				flags_log(m_gpr[R2]);
				break;
			case 0xe6: // m: multiply step
				{
					s64 sum = s32(m_gpr[R2]);

					if (m_scr[CS] & CS_C)
					{
						// no carry
						switch (m_scr[MQ] & 3)
						{
						case 1: sum += s32(m_gpr[R3]); break;
						case 2: sum -= s64(s32(m_gpr[R3])) * 2; break;
						case 3: sum -= s32(m_gpr[R3]); break;
						}
					}
					else
					{
						// carry
						switch (m_scr[MQ] & 3)
						{
						case 0: sum += s32(m_gpr[R3]); break;
						case 1: sum += s64(s32(m_gpr[R3])) * 2; break;
						case 2: sum -= s32(m_gpr[R3]); break;
						}
					}

					// update carry flag
					if (m_scr[MQ] & 2)
						m_scr[CS] &= ~CS_C;
					else
						m_scr[CS] |= CS_C;

					m_scr[MQ] = (sum << 30) | (m_scr[MQ] >> 2);
					m_gpr[R2] = sum >> 2;
				}
				m_icount -= 3;
				break;
			case 0xe7: // x: exclusive or
				m_gpr[R2] ^= m_gpr[R3];
				flags_log(m_gpr[R2]);
				break;
			case 0xe8: // bnbr: branch on not condition bit
				if (m_branch_state != BRANCH)
				{
					if (!BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_target = m_gpr[R3] & ~1;
						m_branch_state = BRANCH;
					}
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;
			case 0xe9: // bnbrx: branch on not condition bit with execute
				if (m_branch_state != BRANCH)
				{
					if (!BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_source = m_scr[IAR];
						m_branch_target = m_gpr[R3] & ~1;
						m_branch_state = DELAY;
					}
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;

			case 0xeb: // lhs: load half short
				load<u16>(m_gpr[R3], [this, op](u16 data) { m_gpr[R2] = data; });
				m_icount -= 4;
				break;
			case 0xec: // balr: branch and link
				if (m_branch_state != BRANCH)
				{
					m_branch_target = m_gpr[R3] & ~1;
					m_branch_state = BRANCH;
					m_gpr[R2] = updated_iar;
					m_icount -= 4;
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;
			case 0xed: // balrx: branch and link with execute
				if (m_branch_state != BRANCH)
				{
					m_branch_source = m_scr[IAR];
					m_branch_target = m_gpr[R3] & ~1;
					m_branch_state = DELAY;
					m_gpr[R2] = updated_iar + 4;
					m_icount -= 4;
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;
			case 0xee: // bbr: branch on condition bit
				if (m_branch_state != BRANCH)
				{
					if (BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_target = m_gpr[R3] & ~1;
						m_branch_state = BRANCH;
						m_icount -= 4;
					}
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;
			case 0xef: // bbrx: branch on condition bit with execute
				if (m_branch_state != BRANCH)
				{
					if (BIT(m_scr[CS], R2 ^ 15))
					{
						m_branch_source = m_scr[IAR];
						m_branch_target = m_gpr[R3] & ~1;
						m_branch_state = DELAY;
						m_icount -= 4;
					}
				}
				else
					program_check(PCS_PCK | PCS_IOC);
				break;

			case 0xf0: // wait: wait
				if (!(m_scr[ICS] & ICS_US))
				{
					if (m_branch_state != BRANCH)
						m_branch_state = WAIT;
					else
						program_check(PCS_PCK | PCS_IOC);
				}
				else
					program_check(PCS_PCK | PCS_PIE);
				break;
			case 0xf1: // ae: add extended
				flags_add(m_gpr[R2], m_gpr[R3] + bool(m_scr[CS] & CS_C));
				m_gpr[R2] += m_gpr[R3] + bool(m_scr[CS] & CS_C);
				break;
			case 0xf2: // se: subtract extended
				flags_add(m_gpr[R2], ~m_gpr[R3] + bool(m_scr[CS] & CS_C));
				m_gpr[R2] += ~m_gpr[R3] + bool(m_scr[CS] & CS_C);
				break;
			case 0xf3: // ca16: compute address 16-bit
				m_gpr[R2] = (m_gpr[R3] & 0xffff'0000U) | (u16(m_gpr[R2]) + u16(m_gpr[R3]));
				break;
			case 0xf4: // onec: ones complement
				m_gpr[R2] = ~m_gpr[R3];
				flags_log(m_gpr[R2]);
				break;
			case 0xf5: // clz: count leading zeros
				m_gpr[R2] = count_leading_zeros_32(u16(m_gpr[R3])) - 16;
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

			default:
				program_check(PCS_PCK | PCS_IOC);
				break;
			}
			break;
		}

			});
		// update iar and branch state
		switch (m_branch_state)
		{
		case DEFAULT:
			m_scr[IAR] = updated_iar;
			break;

		case BRANCH:
			m_scr[IAR] = m_branch_target;
			m_branch_state = DEFAULT;
			break;

		case DELAY:
			m_scr[IAR] = updated_iar;
			m_branch_state = BRANCH;
			m_defer_int = true;
			break;

		case EXCEPTION:
			m_branch_state = DEFAULT;
			break;

		case WAIT:
			// TODO: assume iar is updated
			m_scr[IAR] = updated_iar;
			break;
		}
	}
}

void romp_device::set_scr(unsigned scr, u32 data)
{
	if (VERBOSE & LOG_GENERAL)
	{
		static char const *const scr_names[16] =
		{
			"scr0", "scr1", "scr2", "scr3", "scr4", "scr5", "cous", "cou",
			"ts",   "ecr",  "mq",   "mpcs", "irb",  "iar",  "ics",  "cs",
		};

		LOG("set_scr %s data 0x%08x (%s)\n", scr_names[scr], data, machine().describe_context());
	}

	static u32 const scr_mask[16] =
	{
		0, 0, 0, 0, 0, 0,                                               // reserved
		0xffff'ffffU, 0xffff'ffffU,                                     // counter source and counter
		TS_E | TS_I | TS_O | TS_P,
		0,                                                              // reserved
		0xffff'ffffU,                                                   // multiplier quotient
		MCS_ALL | PCS_ALL,
		IRB_ALL,
		0xffff'fffeU,                                                   // instruction address
		ICS_PE | ICS_MP | ICS_US | ICS_TM | ICS_IM | ICS_CS | ICS_PP,
		CS_L | CS_E | CS_G | CS_C | CS_O | CS_T,
	};

	if (!(m_scr[ICS] & ICS_US) || scr == MQ || scr == CS)
		m_scr[scr] = data & scr_mask[scr];
	else
		program_check(PCS_PCK | PCS_PIE);
}

void romp_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_NMI:
		if (!state)
			m_trap = true;
		break;

	default:
		// interrupt lines are active low
		if (!state)
			m_reqi |= 1U << irqline;
		else
			m_reqi &= ~(1U << irqline);
		break;
	}
}

device_memory_interface::space_config_vector romp_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_mem_config) };
}

bool romp_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);

	if (m_scr[ICS] & ICS_TM)
		return m_mmu->translate(address);
	else
		return true;
}

std::unique_ptr<util::disasm_interface> romp_device::create_disassembler()
{
	return std::make_unique<romp_disassembler>();
}

void romp_device::flags_log(u32 const data)
{
	m_scr[CS] &= ~(CS_L | CS_E | CS_G);

	if (data == 0)
		m_scr[CS] |= CS_E;
	else
		if (BIT(data, 31))
			m_scr[CS] |= CS_L;
		else
			m_scr[CS] |= CS_G;
}

void romp_device::flags_add(u32 const op1, u32 const op2)
{
	u32 const result = op1 + op2;

	m_scr[CS] &= ~(CS_L | CS_E | CS_G | CS_C | CS_O);

	if (result == 0)
		m_scr[CS] |= CS_E;
	else
		if (BIT(result, 31))
			m_scr[CS] |= CS_L;
		else
			m_scr[CS] |= CS_G;

	// carry
	if ((BIT(op2, 31) && BIT(op1, 31)) || (!BIT(result, 31) && (BIT(op2, 31) || BIT(op1, 31))))
		m_scr[CS] |= CS_C;

	// overflow
	if ((BIT(op2, 31) == BIT(op1, 31)) && (BIT(result, 31) != BIT(op2, 31)))
		m_scr[CS] |= CS_O;
}

void romp_device::flags_sub(u32 const op1, u32 const op2)
{
	u32 const result = op1 - op2;

	m_scr[CS] &= ~(CS_L | CS_E | CS_G | CS_O);

	if (result == 0)
		m_scr[CS] |= CS_E;
	else
		if (BIT(result, 31))
			m_scr[CS] |= CS_L;
		else
			m_scr[CS] |= CS_G;

	// borrow
	if ((!BIT(op2, 31) && BIT(op1, 31)) || (BIT(result, 31) && (!BIT(op2, 31) || BIT(op1, 31))))
		m_scr[CS] &= ~CS_C;
	else
		m_scr[CS] |= CS_C;

	// overflow
	if ((BIT(op2, 31) != BIT(op1, 31)) && (BIT(result, 31) != BIT(op2, 31)))
		m_scr[CS] |= CS_O;
}

void romp_device::interrupt_check()
{
	if (m_trap)
	{
		// TODO: traps with check-stop mask 0
		machine_check(MCS_IOT);
		m_trap = false;
		return;
	}

	// interrupts masked, machine/program check, or no interrupts
	if ((m_scr[ICS] & ICS_IM) || m_scr[MPCS] || !(m_reqi || (m_scr[IRB] & IRB_ALL)))
		return;

	unsigned const priority = m_scr[ICS] & ICS_PP;
	for (unsigned irl = 0; irl < priority; irl++)
	{
		if (BIT(m_reqi, irl) || BIT(m_scr[IRB], 15 - irl))
		{
			// enable debugger interrupt breakpoints
			standard_irq_callback(irl, m_scr[IAR]);

			LOGMASKED(LOG_INTERRUPT, "interrupt_check taking interrupt request level %d\n", irl);
			interrupt_enter(irl, m_scr[IAR]);

			return;
		}
	}
}

void romp_device::machine_check(u32 mcs)
{
	debugger_exception_hook(7);

	LOGMASKED(LOG_INTERRUPT, "machine_check mcs 0x%08x\n", mcs);

	m_scr[MPCS] &= ~MCS_ALL;
	m_scr[MPCS] |= (mcs & MCS_ALL);

	interrupt_enter(7, m_scr[IAR]);
}

void romp_device::program_check(u32 pcs)
{
	debugger_exception_hook(8);

	LOGMASKED(LOG_INTERRUPT, "program_check pcs 0x%08x\n", pcs);

	m_scr[MPCS] &= ~PCS_ALL;
	m_scr[MPCS] |= (pcs & PCS_ALL);

	/*
	 * Old program status reflects the address of a branch with execute
	 * instruction when a program-check error occurs due to either a invalid
	 * subject instruction or a data address exception caused by a subject
	 * instruction.
	 */
	// TODO: what about program check caused by subject instruction fetch?
	interrupt_enter(8, m_branch_state == BRANCH ? m_branch_source : m_scr[IAR]);

	m_branch_state = EXCEPTION;
}

void romp_device::interrupt_enter(unsigned vector, u32 iar, u16 svc)
{
	// compute program status pair address
	u32 const address = 0x100 + vector * 16;

	// save old program status
	// TODO: error handling
	store<u32>(address + 0, iar, false);
	store<u16>(address + 4, u16(m_scr[ICS]), false);
	store<u16>(address + 6, u16(m_scr[CS]), false);
	if (vector == 9)
		store<u16>(address + 14, svc, false);

	// load new program status
	load<u32>(address + 8, [this](u32 data) { m_scr[IAR] = data; }, false);
	load<u16>(address + 12, [this](u16 data) { m_scr[ICS] = data; }, false);
	if (vector < 7)
		load<u16>(address + 14, [this](u16 data) { m_scr[CS] = data; }, false);

	m_branch_state = DEFAULT;
}

void romp_device::clk_w(int state)
{
	if (state)
	{
		// decrement counter
		if (m_scr[COU])
			m_scr[COU]--;

		// check counter expiry
		if (!m_scr[COU])
		{
			// check alarm enabled
			if (m_scr[TS] & TS_E)
			{
				// overflow check
				if (m_scr[TS] & TS_I)
					m_scr[TS] |= TS_O;

				// set status
				m_scr[TS] |= TS_I;

				// raise interrupt
				if ((m_scr[TS] & TS_P) < 7)
					m_scr[IRB] |= IRB_L0 >> (m_scr[TS] & TS_P);
			}

			// reload counter
			m_scr[COU] = m_scr[COUS];
		}
	}
}
