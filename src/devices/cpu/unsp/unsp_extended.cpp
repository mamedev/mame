// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "debugger.h"

#include "unspdasm.h"

void unsp_device::execute_extended_group(uint16_t op)
{
	// shouldn't get here anyway
	logerror("<UNKNOWN EXTENDED>\n");
	unimplemented_opcode(op);
	return;
}

void unsp_20_device::execute_extended_group(uint16_t op)
{
	uint16_t ximm = read16(UNSP_LPC);
	add_lpc(1);

	switch ((ximm & 0x01f0) >> 4)
	{
	case 0x00: case 0x10:
	{
		// Ext Register Ra = Ra op Rb
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		switch (aluop)
		{
		case 0x00: // add
		{
			// A += B
			//logerror( "(Extended group 0) %s += %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
			//                                         , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			m_core->m_icount -= 1; // TODO

			uint16_t r0 = (ra & 0x8) ? m_secondary_r[ra & 0x7] : m_core->m_r[ra & 0x7];
			uint16_t r1 = (rb & 0x8) ? m_secondary_r[rb & 0x7] : m_core->m_r[rb & 0x7];

			uint32_t lres = r0 + r1;

			update_nzsc(lres, r0, r1);

			if (ra & 0x8)
			{
				m_secondary_r[ra & 0x7] = (uint16_t)lres;
			}
			else
			{
				m_core->m_r[ra & 0x7] = (uint16_t)lres;
			}

			//unimplemented_opcode(op, ximm);
			return;
			break;
		}

		case 0x01: // adc
			// A += B, Carry
			logerror( "(Extended group 0) %s += %s, carry\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x02: // sub
			// A -= B
			logerror( "(Extended group 0) %s -= %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
													 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x03: // sbc
			// A -= B, Carry
			logerror( "(Extended group 0) %s -= %s, carry\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x04: // cmp
		{
			// CMP A,B
			//logerror( "(Extended group 0) cmp %s, %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
			//                                         , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			m_core->m_icount -= 1; // TODO

			uint16_t r0 = (ra & 0x8) ? m_secondary_r[ra & 0x7] : m_core->m_r[ra & 0x7];
			uint16_t r1 = (rb & 0x8) ? m_secondary_r[rb & 0x7] : m_core->m_r[rb & 0x7];

			uint32_t lres = r0 + (uint16_t)(~r1) + uint32_t(1);
			update_nzsc(lres, r0 , ~r1);

			//unimplemented_opcode(op, ximm);
			return;
			break;
		}

		case 0x06: // neg
			// A = -B
			logerror( "(Extended group 0) %s = -%s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
													 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x08: // xor
			// A ^= B
			logerror( "(Extended group 0) %s ^= %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
													 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x09: // load
		{
			// A = B
			//logerror("(Extended group 0) %s = %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
			//                                       , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			m_core->m_icount -= 1; // TODO

			uint32_t lres = (rb & 0x8) ? m_secondary_r[rb & 0x7] : m_core->m_r[rb & 0x7];
			update_nz(lres);
			if (ra & 0x8)
			{
				m_secondary_r[ra & 0x7] = (uint16_t)lres;
			}
			else
			{
				m_core->m_r[ra & 0x7] = (uint16_t)lres;
			}

			return;
			break;
		}
		case 0x0a: // or
			// A |= B
			logerror( "(Extended group 0) %s |= %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
													 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0b: // and
			// A &= B
			logerror( "(Extended group 0) %s &= %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
													 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0c: // test
			// TEST A,B
			logerror( "(Extended group 0) test %s, %s\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0d: // store
			// B = A
			logerror( "(Extended group 0) %s = %s\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
													, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 0) <INVALID Ra = Ra op Rb form>\n");
			unimplemented_opcode(op, ximm);
			return;
			break;
		}


		unimplemented_opcode(op, ximm);
		return;
	}
	case 0x02:
	{
		// register decoding could be incorrect here

		// Ext Push/Pop
		if (ximm & 0x8000)
		{
			// just skip for now, as nothing really even uses the extended registers yet

			uint8_t rb = (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx = (ximm & 0x0e00) >> 9;

			if (size == 0) size = 8;

			if ((rx - (size - 1)) >= 0)
			{
				//logerror("(Ext) push %s, %s to [%s]\n",
				//  extregs[rx - size], extregs[rx], (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);

				while (size--)
				{
					if (rb & 0x8)
						push(m_secondary_r[rx--], &m_secondary_r[rb & 0x7]);
					else
						push(m_secondary_r[rx--], &m_core->m_r[rb & 0x7]);
				}
			}
			else
			{
				logerror("(Ext) push <BAD>\n");
				unimplemented_opcode(op, ximm);
			}

			return;
		}
		else
		{
			uint8_t rb = (ximm & 0x000f) >> 0;
			uint8_t size = (ximm & 0x7000) >> 12;
			uint8_t rx = (ximm & 0x0e00) >> 9;


			if (size == 0) size = 8;

			if ((rx - (size - 1)) >= 0)
			{
				//logerror("(Ext) pop %s, %s from [%s]\n",
				//  extregs[rx - size], extregs[rx], (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
				int realrx = 7 - rx;

				while (size--)
				{
					if (rb & 0x8)
						m_secondary_r[realrx++] = pop(&m_secondary_r[rb & 0x07]);
					else
						m_secondary_r[realrx++] = pop(&m_core->m_r[rb & 0x07]);
				}
			}
			else
			{
				logerror("(Ext) pop <BAD>\n");
				unimplemented_opcode(op, ximm);
			}


			return;
		}
		return;
	}
	case 0x04:  case 0x14:
	{
		uint16_t imm16_2 = read16(UNSP_LPC);
		add_lpc(1);

		// Ra=Rb op IMM16
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;


		switch (aluop)
		{
		case 0x00: // add
			// A = B + C
			logerror( "(Extended group 1) %s = %s + %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x01: // adc
			// A = B + C, Carry
			logerror( "(Extended group 1) %s = %s + %04x, carry\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																  , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																  , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x02: // sub
			// A = B - C
			logerror( "(Extended group 1) %s = %s - %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x03: // sbc
			// A = B - C, Carry
			logerror( "(Extended group 1) %s = %s - %04x, carry\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																  , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																  , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x04: // cmp
			// CMP B,C
			logerror( "(Extended group 1) cmp %s, %04x\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x06: // neg
			// A = -C
			logerror( "(Extended group 1) %s = -%04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
													   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x08: // xor
			// A = B ^ C
			logerror( "(Extended group 1) %s = %s ^ %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x09: // load
		{
			// A = C
			//logerror("(Extended group 1) %s = %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
			//                                       , imm16_2);
			m_core->m_icount -= 1; // TODO

			uint32_t lres = imm16_2;
			update_nz(lres);
			if (ra & 0x8)
			{
				m_secondary_r[ra & 0x7] = (uint16_t)lres;
			}
			else
			{
				m_core->m_r[ra & 0x7] = (uint16_t)lres;
			}

			return;
			break;
		}
		case 0x0a: // or
			// A = B | C
			logerror( "(Extended group 1) %s = %s | %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0b: // and
			// A = B & C
			logerror( "(Extended group 1) %s = %s & %04x\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														   , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0c: // test
			// TEST B,C
			logerror( "(Extended group 1) test %s, %04x\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														  , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0d: // store, C = B, invalid for this mode (would store to immediate)
		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 1) <INVALID Ra=Rb op IMM16 form>\n");
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;
		}

		unimplemented_opcode(op, ximm, imm16_2);
		return;
	}

	case 0x06:
	case 0x16:
	{
		uint16_t imm16_2 = read16(UNSP_LPC);
		add_lpc(1);

		// Ra=Rb op [A16]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		switch (aluop)
		{
		case 0x00: // add
			// A = B + C
			logerror( "(Extended group 2) %s = %s + [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x01: // adc
			// A = B + C, Carry
			logerror( "(Extended group 2) %s = %s + [%04x], carry\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																	, imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x02: // sub
			// A = B - C
			logerror( "(Extended group 2) %s = %s - [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x03: // sbc
			// A = B - C, Carry
			logerror( "(Extended group 2) %s = %s - [%04x], carry\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
																	, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																	, imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x04: // cmp
			// CMP B,C
			logerror( "(Extended group 2) cmp %s, [%04x]\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														   , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x06: // neg
			// A = -C
			logerror( "(Extended group 2) %s = -[%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
														 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x08: // xor
			// A = B ^ C
			logerror( "(Extended group 2) %s = %s ^ [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x09: // load
		{
			// A = C
			//logerror("(Extended group 2) %s = [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
			//                                           , imm16_2);
			//unimplemented_opcode(op, ximm, imm16_2);

			m_core->m_icount -= 1; // TODO

			//uin32_t lreg = UNSP_LREG_I(imm16_2);

			uint32_t lres = read16(imm16_2);
			update_nz(lres);
			if (ra & 0x8)
			{
				m_secondary_r[ra & 0x7] = (uint16_t)lres;
			}
			else
			{
				m_core->m_r[ra & 0x7] = (uint16_t)lres;
			}

			return;
			break;
		}

		case 0x0a: // or
			// A = B | C
			logerror( "(Extended group 2) %s = %s | [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0b: // and
			// A = B & C
			logerror( "(Extended group 2) %s = %s & [%04x]\n", (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0c: // test
			// TEST B,C
			logerror( "(Extended group 2) test %s, [%04x]\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															, imm16_2);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0d: // store
			// C = B
			logerror( "(Extended group 2) [%04x] = %s\n", imm16_2
														, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 2) <INVALID Ra = Rb op [A16] form>\n");
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;
		}

		unimplemented_opcode(op, ximm, imm16_2);
		return;
	}

	case 0x07:
	case 0x17:
	{
		uint16_t imm16_2 = read16(UNSP_LPC);
		add_lpc(1);

		//[A16] = Ra op Rb
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rb = (ximm & 0x000f) >> 0;
		uint8_t ra = (ximm & 0x0e00) >> 9;
		ra |= (ximm & 0x0100) >> 5;

		switch (aluop)
		{
		case 0x00: // add
			// A = B + C
			logerror( "(Extended group 3) [%04x] = %s + %s\n", imm16_2
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x01: // adc
			// A = B + C, Carry
			logerror( "(Extended group 3) [%04x] = %s + %s, carry\n", imm16_2
																	, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																	, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x02: // sub
			// A = B - C
			logerror( "(Extended group 3) [%04x] = %s - %s\n", imm16_2
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x03: // sbc
			// A = B - C, Carry
			logerror( "(Extended group 3) [%04x] = %s - %s, carry\n", imm16_2
																	, (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
																	, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x04: // cmp  (is this even a valid form? [A16] not even used)
			// CMP B,C
			logerror( "(Extended group 3) cmp %s, %s\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
													   , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x06: // neg
			// A = -C
			logerror( "(Extended group 3) [%04x] = -%s\n", imm16_2
														 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x08: // xor
			// A = B ^ C
			logerror( "(Extended group 3) [%04x] = %s ^ %s\n", imm16_2
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x09: // load
			// A = C
			logerror( "(Extended group 3) [%04x] = %s\n", imm16_2
														, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0a: // or
			// A = B | C
			logerror( "(Extended group 3) [%04x] = %s | %s\n", imm16_2
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0b: // and
			// A = B & C
			logerror( "(Extended group 3) [%04x] = %s & %s\n", imm16_2
															 , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
															 , (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0c: // test  (is this even a valid form? [A16] not even used)
			// TEST B,C
			logerror( "(Extended group 3) test %s, %s\n", (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]
														, (ra & 0x8) ? extregs[ra & 0x7] : regs[ra & 0x7]);
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;

		case 0x0d: // store  (is this even a valid form? [A16] not even used if C = B form below is used, code needs to write to A)
		{
			// C = B
			//logerror( "(Extended group 3) [%04x] = %s\n", imm16_2
			//                                          , (rb & 0x8) ? extregs[rb & 0x7] : regs[rb & 0x7]);
			m_core->m_icount -= 1; // TODO

			if (ra != rb)
			{
				logerror("(Extended group 3) store ra != rb");
				unimplemented_opcode(op, ximm, imm16_2);
			}

			if (rb & 0x8)
			{
				write16(imm16_2, m_secondary_r[rb & 0x07]);
			}
			else
			{
				write16(imm16_2, m_core->m_r[rb & 0x07]);
			}

			return;
			break;
		}

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 3) <INVALID [A16] = Ra op Rb form>\n");
			unimplemented_opcode(op, ximm, imm16_2);
			return;
			break;
		}

		unimplemented_opcode(op, ximm, imm16_2);
		return;
	}

	case 0x08: case 0x09:
	{
		// Ext Indirect Rx=Rx op [Ry@]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			logerror( "(Extended group 4) %s += ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x01: // adc
			// A += B, Carry
			logerror( "(Extended group 4) %s += ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror( ", carry");
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x02: // sub
			// A -= B
			logerror( "(Extended group 4) %s -= ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x03: // sbc
			// A -= B, Carry
			logerror( "(Extended group 4) %s -= ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror( ", carry");
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x04: // cmp
			// CMP A,B
			logerror( "(Extended group 4) cmp %s, ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x06: // neg
			// A = -B
			logerror( "(Extended group 4) %s = -", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x08: // xor
			// A ^= B
			logerror( "(Extended group 4) %s ^= ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x09: // load
			// A = B
			logerror( "(Extended group 4) %s = ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0a: // or
			// A |= B
			logerror( "(Extended group 4) %s |= ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0b: // and
			// A &= B
			logerror( "(Extended group 4) %s &= ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0c: // test
			// TEST A,B
			logerror( "(Extended group 4) test %s, ", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0d: // store
			// B = A
			logerror( "(Extended group 4) ");
			logerror( forms[form], extregs[ry]);
			logerror( " = %s", extregs[rx]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 4) <INVALID Indirect Rx=Rx op [Ry@] form>\n");
			unimplemented_opcode(op, ximm);
			return;
			break;
		}

		unimplemented_opcode(op, ximm);
		return;
	}
	case 0x0a: case 0x0b:
	{
		// Ext DS_Indirect Rx=Rx op ds:[Ry@]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t ry = (ximm & 0x0007) >> 0;
		uint8_t form = (ximm & 0x0018) >> 3;
		uint8_t rx = (ximm & 0x0e00) >> 9;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			logerror( "(Extended group 5) %s += ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x01: // adc
			// A += B, Carry
			logerror( "(Extended group 5) %s += ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror( ", carry");
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x02: // sub
			// A -= B
			logerror( "(Extended group 5) %s -= ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x03: // sbc
			// A -= B, Carry
			logerror( "(Extended group 5) %s -= ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror( ", carry");
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x04: // cmp
			// CMP A,B
			logerror( "(Extended group 5) cmp %s, ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x06: // neg
			// A = -B
			logerror( "(Extended group 5) %s = -ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x08: // xor
			// A ^= B
			logerror( "(Extended group 5) %s ^= ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x09: // load
			// A = B
			logerror( "(Extended group 5) %s = ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0a: // or
			// A |= B
			logerror( "(Extended group 5) %s |= ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0b: // and
			// A &= B
			logerror( "(Extended group 5) %s &= ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0c: // test
			// TEST A,B
			logerror( "(Extended group 5) test %s, ds:", extregs[rx]);
			logerror( forms[form], extregs[ry]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0d: // store
			// B = A
			logerror( "(Extended group 5) ds:");
			logerror( forms[form], extregs[ry]);
			logerror( " = %s", extregs[rx]);
			logerror("\n");
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 5) <INVALID DS_Indirect Rx=Rx op ds:[Ry@] form>\n");
			unimplemented_opcode(op, ximm);
			return;
			break;
		}

		unimplemented_opcode(op, ximm);
		return;
	}
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	{
		// Ext IM6 Rx=Rx op IM6
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			logerror("(Extended group 6) %s += %02x\n", extregs[rx], imm6);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x01: // adc
			// A += B, Carry
			logerror("(Extended group 6) %s += %02x, carry\n", extregs[rx], imm6);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x02: // sub
			// A -= B
			logerror("(Extended group 6) %s -= %02x\n", extregs[rx], imm6);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x03: // sbc
			// A -= B, Carry
			logerror("(Extended group 6) %s -= %02x, carry\n", extregs[rx], imm6);
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x04: // cmp
		{
			// CMP A,B
			//logerror("(Extended group 6) cmp %s, %02x\n", extregs[rx], imm6);

			m_core->m_icount -= 1; // TODO

			uint32_t lres = m_secondary_r[rx] + (uint16_t)(~imm6) + uint32_t(1);
			update_nzsc(lres, m_secondary_r[rx] , ~imm6);

			//unimplemented_opcode(op, ximm);
			return;
			break;
		}
		case 0x06: // neg
			// A = -B
			logerror( "(Extended group 6) %s = -%02x\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x08: // xor
			// A ^= B
			logerror( "(Extended group 6) %s ^= %02x\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x09: // load
		{
			// A = B
			//logerror( "(Extended group 6) %s = %02x\n", extregs[rx], imm6 );

			m_core->m_icount -= 1; // TODO

			uint32_t lres = imm6;
			update_nz(lres);
			m_secondary_r[rx] = (uint16_t)lres;

			return;
			break;
		}

		case 0x0a: // or
			// A |= B
			logerror( "(Extended group 6) %s |= %02x\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0b: // and
			// A &= B
			logerror( "(Extended group 6) %s &= %02x\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0c: // test
			// TEST A,B
			logerror( "(Extended group 6) test %s, %02x\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0d: // store,  B = A (invalid for this type)
		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 6) <INVALID IM6 Rx=Rx op IM6 form>\n");
			unimplemented_opcode(op, ximm);
			return;
			break;
		}

		unimplemented_opcode(op, ximm);
		return;
	}

	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	{
		// Ext Base+Disp6 Rx=Rx op [BP+IM6]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t imm6 = (ximm & 0x003f) >> 0;

		switch (aluop)
		{
		case 0x00: // add
		{
			// A += B
			// logerror( "(Extended group 7) %s += [BP+%02x]\n", extregs[rx], imm6 );
			m_core->m_icount -= 1; // TODO

			uint16_t addr = (uint16_t)(m_core->m_r[REG_BP] + (imm6 & 0x3f));
			uint16_t r0 = m_secondary_r[rx];
			uint16_t r1 = read16(addr);

			uint32_t lres = r0 + r1;

			update_nzsc(lres, r0, r1);

			m_secondary_r[rx] = lres;

			//unimplemented_opcode(op, ximm);
			return;
			break;
		}
		case 0x01: // adc
		{
			// A += B, Carry
			//logerror( "(Extended group 7) %s += [BP+%02x], carry\n", extregs[rx], imm6 );
			m_core->m_icount -= 1; // TODO

			uint32_t c = (m_core->m_r[REG_SR] & UNSP_C) ? 1 : 0;
			uint16_t addr = (uint16_t)(m_core->m_r[REG_BP] + (imm6 & 0x3f));
			uint16_t r0 = m_secondary_r[rx];
			uint16_t r1 = read16(addr);

			uint32_t lres = r0 + r1 + c;

			update_nzsc(lres, r0, r1);

			m_secondary_r[rx] = lres;

			//unimplemented_opcode(op, ximm);
			return;
			break;
		}

		case 0x02: // sub
			// A -= B
			logerror( "(Extended group 7) %s -= [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x03: // sbc
			// A -= B, Carry
			logerror( "(Extended group 7) %s -= [BP+%02x], carry\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x04: // cmp
			// CMP A,B
			logerror( "(Extended group 7) cmp %s, [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x06: // neg
			// A = -B
			logerror( "(Extended group 7) %s = -[BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x08: // xor
			// A ^= B
			logerror( "(Extended group 7) %s ^= [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x09: // load
			// A = B
			logerror( "(Extended group 7) %s = [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0a: // or
			// A |= B
			logerror( "(Extended group 7) %s |= [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0b: // and
			// A &= B
			logerror( "(Extended group 7) %s &= [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0c: // test
			// TEST A,B
			logerror( "(Extended group 7) test %s, [BP+%02x]\n", extregs[rx], imm6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0d: // store
			// B = A
			logerror( "(Extended group 7) [BP+%02x] = %s\n", imm6, extregs[rx] );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 7) <INVALID Base+Disp6 Rx=Rx op [BP+IM6] form>\n");
			unimplemented_opcode(op, ximm);
			return;
			break;
		}

		unimplemented_opcode(op, ximm);
		return;
	}

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	{
		// Ext A6 Rx=Rx op [A6]
		uint8_t aluop = (ximm & 0xf000) >> 12;
		uint8_t rx = (ximm & 0x0e00) >> 9;
		uint8_t a6 = (ximm & 0x003f) >> 0;

		switch (aluop)
		{
		case 0x00: // add
			// A += B
			logerror( "(Extended group 8) %s += [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x01: // adc
			// A += B, Carry
			logerror( "(Extended group 8) %s += [%02x], carry\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x02: // sub
			// A -= B
			logerror( "(Extended group 8) %s -= [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x03: // sbc
			// A -= B, Carry
			logerror( "(Extended group 8) %s -= [%02x], carry\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x04: // cmp
			// CMP A,B
			logerror( "(Extended group 8) cmp %s, [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x06: // neg
			// A = -B
			logerror( "(Extended group 8) %s = -[%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x08: // xor
			// A ^= B
			logerror( "(Extended group 8) %s ^= [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x09: // load
			// A = B
			logerror( "(Extended group 8) %s = [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0a: // or
			// A |= B
			logerror( "(Extended group 8) %s |= [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0b: // and
			// A &= B
			logerror( "(Extended group 8) %s &= [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0c: // test
			// TEST A,B
			logerror( "(Extended group 8) test %s, [%02x]\n", extregs[rx], a6 );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x0d: // store
			// B = A
			logerror( "(Extended group 8) [%02x] = %s\n", a6, extregs[rx] );
			unimplemented_opcode(op, ximm);
			return;
			break;

		case 0x05: // invalid
		case 0x07: // invalid
		case 0x0e: // invalid
		case 0x0f: // invalid
			logerror( "(Extended group 8) <INVALID A6 Rx=Rx op [A6] form>\n");
			unimplemented_opcode(op, ximm);
			return;
			break;
		}


		unimplemented_opcode(op, ximm);
		return;
	}
	}

	// illegal?
	unimplemented_opcode(op, ximm);
	return;
}
