// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "unspdasm.h"

#include <climits>

#define LOG_UNSP_MULS            (1U << 1)

#define VERBOSE             (0)

#include "logmacro.h"

inline void unsp_device::execute_fxxx_000_group(uint16_t op)
{
	//                          |   | |
	// DS16     1 1 1 1   1 1 1 0   0 0 i i   i i i i
	// DS Reg   1 1 1 1   - - - 0   0 0 1 0   w r r r
	// FR Reg   1 1 1 1   - - - 0   0 0 1 1   w r r r

	// FR = 'inner flag register' on ISA1.2+
	// does this mean 1.2+ do not store the registers in the upper bits of SR, or is this something else?
	// smartfp IRQ4 reads this and puts in on the stack

	if (((op & 0xffc0) == 0xfe00) && m_iso >= 12)
	{
		// ds = imm6
		set_ds(op & 0x003f);
		return;
	}
	else if (((op & 0xf1f8) == 0xf020) && m_iso >= 12)
	{
		// rx = ds
		m_core->m_r[op & 0x7] = get_ds();
		return;
	}
	else if (((op & 0xf1f8) == 0xf028) && m_iso >= 12)
	{
		// ds = rx
		set_ds(m_core->m_r[op & 0x7]);
		return;
	}
	else if (((op & 0xf1f8) == 0xf030) && m_iso >= 12)
	{
		m_core->m_r[op & 0x7] = get_fr();
		return;
	}
	else if (((op & 0xf1f8) == 0xf038) && m_iso >= 12)
	{
		set_fr(m_core->m_r[op & 0x7]);
		return;
	}

	// everything else falls through to the multiply

	// MUL us ( signed * unsigned )
	// MUL      1 1 1 1*  r r r 0*  0 0 0 0   1 r r r     (** = sign bits, fixed here)

	const uint16_t opa = (op >> 9) & 7;
	const uint16_t opb = op & 7;
	m_core->m_icount -= 12;

	LOGMASKED(LOG_UNSP_MULS, "%s: MUL us with %04x (unsigned) * %04x (signed) (fra:%d) :\n", machine().describe_context(), m_core->m_r[opa], m_core->m_r[opb], m_core->m_fra);

	uint32_t lres = m_core->m_r[opa] * m_core->m_r[opb];
	if (m_core->m_r[opb] & 0x8000)
	{
		lres -= m_core->m_r[opa] << 16;
	}
	m_core->m_r[REG_R4] = lres >> 16;
	m_core->m_r[REG_R3] = (uint16_t)lres;

	LOGMASKED(LOG_UNSP_MULS, "result was : %08x\n", lres);

	return;
}

inline void unsp_device::execute_fxxx_001_group(uint16_t op)
{
	//                             |   | |
	// CALL16      1 1 1 1   - - 0 0   0 1 a a   a a a a   (+imm16)

	if ((op & 0xf3c0) == 0xf040)
	{
		m_core->m_icount -= 9;
		uint16_t r1 = read16(UNSP_LPC);
		add_lpc(1);
		push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
		push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
		m_core->m_r[REG_PC] = r1;
		m_core->m_r[REG_SR] &= 0xffc0;
		m_core->m_r[REG_SR] |= op & 0x3f;
		return;
	}
	// MEM bitop   1 1 1 1   - D 1 0   0 1 b b   o o o o   (+imm16)   ( BITOP {ds:}[A16],offset )
	else if (((op & 0xf3c0) == 0xf240) && m_iso >= 20)
	{
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t offset = (op & 0x000f) >> 0;
		uint8_t d =      (op & 0x0400) >> 10;
		uint16_t imm16 = read16(UNSP_LPC);
		add_lpc(1);

		if (d)
		{
			const uint32_t addr = imm16 | (get_ds() << 16);
			const uint16_t orig = read16(addr);

			// manual seems to indicate that the zero flag always gets changed based on original value
			// even for opcodes other than tstb
			m_core->m_r[REG_SR] &= ~UNSP_Z;
			m_core->m_r[REG_SR] |= BIT(orig, offset) ? 0 : UNSP_Z;

			switch (bitop)
			{
			case 0x0: // tstb
			{
				// handled above
				break;
			}
			case 0x1: // setb    (bkrankp uses this)
			{
				write16(addr, orig | (1 << offset));
				break;
			}
			case 0x2: // clrb
			{
				write16(addr, orig & ~(1 << offset));
				break;
			}

			case 0x3: // invb
			{
				write16(addr, orig ^ (1 << offset));
				break;
			}
			}
		}
		else
		{
			const uint16_t orig =  read16(imm16);

			// manual seems to indicate that the zero flag always gets changed based on original value
			// even for opcodes other than tstb
			m_core->m_r[REG_SR] &= ~UNSP_Z;
			m_core->m_r[REG_SR] |= BIT(orig, offset) ? 0 : UNSP_Z;

			switch (bitop)
			{
			case 0x0: // tstb
			{
				// handled above
				break;
			}
			case 0x1: // setb    (bkrankp uses this)
			{
				write16(imm16, orig | (1 << offset));
				break;
			}
			case 0x2: // clrb
			{
				write16(imm16, orig & ~(1 << offset));
				break;
			}

			case 0x3: // invb
			{
				write16(imm16, orig ^ (1 << offset));
				break;
			}
			}
		}

		return;
	}

	logerror("<DUNNO>\n");
	unimplemented_opcode(op);
	return;
}

inline void unsp_device::execute_fxxx_010_group(uint16_t op)
{
	//                         |   | |
	// JMPF    1 1 1 1   1 1 1 0   1 0 a a   a a a a    (+imm16)

	if ((op & 0xffc0) == 0xfe80) // apparently 1.2 and above, but jak_capb needs it and otherwise seems 1.0 / 1.1?
	{
		m_core->m_icount -= 5;
		m_core->m_r[REG_PC] = read16(UNSP_LPC);
		m_core->m_r[REG_SR] &= 0xffc0;
		m_core->m_r[REG_SR] |= op & 0x3f;
		return;
	}

	// signed * unsigned  (size 16,1,2,3,4,5,6,7)
	// MULS    1 1 1 1*  r r r 0*  1 0*s s   s r r r    (1* = sign bit, 0* = sign bit 0* = upper size bit)

	// MULS us with upper size bit not set
	logerror("MULS us\n");
	unimplemented_opcode(op);
	return;
}

inline void unsp_device::execute_fxxx_011_group(uint16_t op)
{
	//                         |   | |
	// JMPR    1 1 1 1   1 1 1 0   1 1 - -   - - - -
	if (((op & 0xffc0) == 0xfec0) && m_iso >= 12)
	{
		m_core->m_icount -= 5;
		m_core->m_r[REG_PC] = m_core->m_r[REG_R3];
		m_core->m_r[REG_SR] &= 0xffc0;
		m_core->m_r[REG_SR] |= m_core->m_r[REG_R4] & 0x3f;
		return;
	}

	// signed * unsigned  (size 8,9,10,11,12,13,14,15)
	// MULS    1 1 1 1*  r r r 0*  1 1*s s   s r r r    (1* = sign bit, 0* = sign bit 1* = upper size bit)

	// MULS us with upper size bit set
	logerror("MULS us\n");
	unimplemented_opcode(op);

	return;
}

inline void unsp_device::execute_fxxx_100_group(uint16_t op)
{
	//                          |   | |
	// signed * signed
	// MUL      1 1 1 1*  r r r 1*  0 0 0 0   1 r r r     (** = sign bits, fixed here)

	if ((op & 0xf1f8) == 0xf108)
	{
		const uint16_t opa = (op >> 9) & 7;
		const uint16_t opb = op & 7;

		LOGMASKED(LOG_UNSP_MULS, "%s: MUL ss with %04x (signed) * %04x (signed) (fra:%d) :\n", machine().describe_context(), m_core->m_r[opa], m_core->m_r[opb], m_core->m_fra);

		m_core->m_icount -= 12;
		uint32_t lres = m_core->m_r[opa] * m_core->m_r[opb];
		if (m_core->m_r[opb] & 0x8000)
		{
			lres -= m_core->m_r[opa] << 16;
		}
		if (m_core->m_r[opa] & 0x8000)
		{
			lres -= m_core->m_r[opb] << 16;
		}
		m_core->m_r[REG_R4] = lres >> 16;
		m_core->m_r[REG_R3] = (uint16_t)lres;

		LOGMASKED(LOG_UNSP_MULS, "result was : %08x\n", lres);

		return;
	}

	logerror("<DUNNO>\n");
	unimplemented_opcode(op);
	return;
}

void unsp_12_device::execute_divq(uint16_t op)
{
	if (m_core->m_divq_bit == UINT_MAX)
	{
		m_core->m_divq_bit = 15;
		m_core->m_divq_dividend = (m_core->m_r[REG_R4] << 16) | m_core->m_r[REG_R3];
		m_core->m_divq_divisor = m_core->m_r[REG_R2];
		m_core->m_divq_a = 0;
	}
	m_core->m_aq = BIT(m_core->m_divq_a, 31);
	if (m_core->m_aq)
	{
		m_core->m_divq_a += m_core->m_divq_a + BIT(m_core->m_divq_dividend, 15) + m_core->m_divq_divisor;
	}
	else
	{
		m_core->m_divq_a += m_core->m_divq_a + BIT(m_core->m_divq_dividend, 15) - m_core->m_divq_divisor;
	}
	m_core->m_divq_dividend <<= 1;
	m_core->m_divq_dividend++;
	m_core->m_divq_dividend ^= BIT(m_core->m_divq_a, 31);
	m_core->m_r[REG_R3] = (uint16_t)m_core->m_divq_dividend;
	m_core->m_divq_bit--;
}

bool unsp_12_device::op_is_divq(const uint16_t op)
{
	return (op & 0xf163) == 0xf163;
}

void unsp_12_device::execute_fxxx_101_group(uint16_t op)
{
	// FIR_MOV   1 1 1 1   - - - 1   0 1 0 0   0 1 0 f
	// Fraction  1 1 1 1   - - - 1   0 1 0 0   0 1 1 f
	// SECBANK   1 1 1 1   - - - 1   0 1 0 0   1 0 1 S
	// NESTMODE  1 1 1 1   - - - 1   0 1 0 0   1 1 N 1
	// CALLR     1 1 1 1   - - - 1   0 1 1 -   - 0 0 1
	// DIVS      1 1 1 1   - - - 1   0 1 1 -   - 0 1 0
	// DIVQ      1 1 1 1   - - - 1   0 1 1 -   - 0 1 1
	// EXP       1 1 1 1   - - - 1   0 1 1 -   - 1 0 0

	m_core->m_icount -= 1; // Unknown count

	switch (op)
	{
	case 0xf146: case 0xf346: case 0xf546: case 0xf746: case 0xf946: case 0xfb46: case 0xfd46: case 0xff46:
		m_core->m_fra = 0;
		return;

	case 0xf147: case 0xf347: case 0xf547: case 0xf747: case 0xf947: case 0xfb47: case 0xfd47: case 0xff47:
		m_core->m_fra = 1;
		return;

	case 0xf14a: case 0xf34a: case 0xf54a: case 0xf74a: case 0xf94a: case 0xfb4a: case 0xfd4a: case 0xff4a:
		if (m_core->m_bnk)
		{
			std::swap<uint32_t>(m_core->m_r[REG_R1], m_core->m_secbank[REG_SR1]);
			std::swap<uint32_t>(m_core->m_r[REG_R2], m_core->m_secbank[REG_SR2]);
			std::swap<uint32_t>(m_core->m_r[REG_R3], m_core->m_secbank[REG_SR3]);
			std::swap<uint32_t>(m_core->m_r[REG_R4], m_core->m_secbank[REG_SR4]);
		}
		m_core->m_bnk = 0;
		return;

	case 0xf14b: case 0xf34b: case 0xf54b: case 0xf74b: case 0xf94b: case 0xfb4b: case 0xfd4b: case 0xff4b:
		if (m_core->m_bnk == 0)
		{
			std::swap<uint32_t>(m_core->m_r[REG_R1], m_core->m_secbank[REG_SR1]);
			std::swap<uint32_t>(m_core->m_r[REG_R2], m_core->m_secbank[REG_SR2]);
			std::swap<uint32_t>(m_core->m_r[REG_R3], m_core->m_secbank[REG_SR3]);
			std::swap<uint32_t>(m_core->m_r[REG_R4], m_core->m_secbank[REG_SR4]);
		}
		m_core->m_bnk = 1;
		return;

	case 0xf14d: case 0xf34d: case 0xf54d: case 0xf74d: case 0xf94d: case 0xfb4d: case 0xfd4d: case 0xff4d:
		m_core->m_ine = 0;
		return;

	case 0xf14f: case 0xf34f: case 0xf54f: case 0xf74f: case 0xf94f: case 0xfb4f: case 0xfd4f: case 0xff4f:
		m_core->m_ine = 1;
		return;

	case 0xf144: case 0xf344: case 0xf544: case 0xf744: case 0xf944: case 0xfb44: case 0xfd44: case 0xff44:
		m_core->m_fir_move = 1;
		return;

	case 0xf145: case 0xf345: case 0xf545: case 0xf745: case 0xf945: case 0xfb45: case 0xfd45: case 0xff45:
		m_core->m_fir_move = 0;
		return;

	case 0xf161: case 0xf361: case 0xf561: case 0xf761: case 0xf961: case 0xfb61: case 0xfd61: case 0xff61:
	case 0xf169: case 0xf369: case 0xf569: case 0xf769: case 0xf969: case 0xfb69: case 0xfd69: case 0xff69:
	case 0xf171: case 0xf371: case 0xf571: case 0xf771: case 0xf971: case 0xfb71: case 0xfd71: case 0xff71:
	case 0xf179: case 0xf379: case 0xf579: case 0xf779: case 0xf979: case 0xfb79: case 0xfd79: case 0xff79:
	{
		uint32_t addr = m_core->m_r[REG_R3] | ((m_core->m_r[REG_R4] & 0x3f) << 16);
		push(m_core->m_r[REG_PC], &m_core->m_r[REG_SP]);
		push(m_core->m_r[REG_SR], &m_core->m_r[REG_SP]);
		m_core->m_r[REG_PC] = addr & 0x0000ffff;
		m_core->m_r[REG_SR] &= 0xffc0;
		m_core->m_r[REG_SR] |= ((addr & 0x003f0000) >> 16);
		return;
	}

	case 0xf162: case 0xf362: case 0xf562: case 0xf762: case 0xf962: case 0xfb62: case 0xfd62: case 0xff62:
	case 0xf16a: case 0xf36a: case 0xf56a: case 0xf76a: case 0xf96a: case 0xfb6a: case 0xfd6a: case 0xff6a:
	case 0xf172: case 0xf372: case 0xf572: case 0xf772: case 0xf972: case 0xfb72: case 0xfd72: case 0xff72:
	case 0xf17a: case 0xf37a: case 0xf57a: case 0xf77a: case 0xf97a: case 0xfb7a: case 0xfd7a: case 0xff7a:
		logerror("divs mr, r2\n");
		unimplemented_opcode(op);
		return;

	case 0xf163: case 0xf363: case 0xf563: case 0xf763: case 0xf963: case 0xfb63: case 0xfd63: case 0xff63:
	case 0xf16b: case 0xf36b: case 0xf56b: case 0xf76b: case 0xf96b: case 0xfb6b: case 0xfd6b: case 0xff6b:
	case 0xf173: case 0xf373: case 0xf573: case 0xf773: case 0xf973: case 0xfb73: case 0xfd73: case 0xff73:
	case 0xf17b: case 0xf37b: case 0xf57b: case 0xf77b: case 0xf97b: case 0xfb7b: case 0xfd7b: case 0xff7b:
		execute_divq(op);
		return;

	case 0xf164: case 0xf364: case 0xf564: case 0xf764: case 0xf964: case 0xfb64: case 0xfd64: case 0xff64:
	case 0xf16c: case 0xf36c: case 0xf56c: case 0xf76c: case 0xf96c: case 0xfb6c: case 0xfd6c: case 0xff6c:
	case 0xf174: case 0xf374: case 0xf574: case 0xf774: case 0xf974: case 0xfb74: case 0xfd74: case 0xff74:
	case 0xf17c: case 0xf37c: case 0xf57c: case 0xf77c: case 0xf97c: case 0xfb7c: case 0xfd7c: case 0xff7c:
	{
		uint16_t r4 = m_core->m_r[REG_R4];

		if (r4 & 0x8000)
		{
			m_core->m_r[REG_R2] = count_leading_ones_32(0xffff0000 | r4) - 17;
		}
		else
		{
			m_core->m_r[REG_R2] = count_leading_zeros_32(r4) - 17; // -17 because count_leading_zeros_32 works with 32-bit values
		}

		return;
	}

	default:
		return unsp_device::execute_fxxx_101_group(op);
	}
}


void unsp_device::execute_fxxx_101_group(uint16_t op)
{
	//                           |   | |
	// INT SET   1 1 1 1   - - - 1   0 1 0 0   0 0 F I
	// BREAK     1 1 1 1   - - - 1   0 1 1 -   - 0 0 0
	// NOP       1 1 1 1   - - - 1   0 1 1 -   - 1 0 1

	// IRQ       1 1 1 1   - - - 1   0 1 0 0   1 0 0 I
	// FIRQ      1 1 1 1   - - - 1   0 1 0 0   1 1 F 0

	m_core->m_icount -= 2;

	switch (op)
	{
	case 0xf140: case 0xf340: case 0xf540: case 0xf740: case 0xf940: case 0xfb40: case 0xfd40: case 0xff40:
		m_core->m_enable_irq = 0;
		m_core->m_enable_fiq = 0;
		return;

	case 0xf141: case 0xf341: case 0xf541: case 0xf741: case 0xf941: case 0xfb41: case 0xfd41: case 0xff41:
		m_core->m_enable_irq = 1;
		m_core->m_enable_fiq = 0;
		return;

	case 0xf142: case 0xf342: case 0xf542: case 0xf742: case 0xf942: case 0xfb42: case 0xfd42: case 0xff42:
		m_core->m_enable_irq = 0;
		m_core->m_enable_fiq = 1;
		return;

	case 0xf143: case 0xf343: case 0xf543: case 0xf743: case 0xf943: case 0xfb43: case 0xfd43: case 0xff43:
		m_core->m_enable_irq = 1;
		m_core->m_enable_fiq = 1;
		return;

	case 0xf144: case 0xf344: case 0xf544: case 0xf744: case 0xf944: case 0xfb44: case 0xfd44: case 0xff44:
		m_core->m_fir_move = 1;
		return;

	case 0xf145: case 0xf345: case 0xf545: case 0xf745: case 0xf945: case 0xfb45: case 0xfd45: case 0xff45:
		m_core->m_fir_move = 0;
		return;

	case 0xf160: case 0xf360: case 0xf560: case 0xf760: case 0xf960: case 0xfb60: case 0xfd60: case 0xff60:
	case 0xf168: case 0xf368: case 0xf568: case 0xf768: case 0xf968: case 0xfb68: case 0xfd68: case 0xff68:
	case 0xf170: case 0xf370: case 0xf570: case 0xf770: case 0xf970: case 0xfb70: case 0xfd70: case 0xff70:
	case 0xf178: case 0xf378: case 0xf578: case 0xf778: case 0xf978: case 0xfb78: case 0xfd78: case 0xff78:
		// break (call vector fff5?)
		unimplemented_opcode(op);
		return;

	case 0xf165: case 0xf365: case 0xf565: case 0xf765: case 0xf965: case 0xfb65: case 0xfd65: case 0xff65:
	case 0xf16d: case 0xf36d: case 0xf56d: case 0xf76d: case 0xf96d: case 0xfb6d: case 0xfd6d: case 0xff6d:
	case 0xf175: case 0xf375: case 0xf575: case 0xf775: case 0xf975: case 0xfb75: case 0xfd75: case 0xff75:
	case 0xf17d: case 0xf37d: case 0xf57d: case 0xf77d: case 0xf97d: case 0xfb7d: case 0xfd7d: case 0xff7d:
		// nothing
		return;

	// these are in code that otherwise seems like ISA1.0 / 1.1 (jak_disf for example) but spec says it's only in ISA 1.2?
	case 0xf148: case 0xf348: case 0xf548: case 0xf748: case 0xf948: case 0xfb48: case 0xfd48: case 0xff48:
		m_core->m_enable_irq = 0;
		return;

	case 0xf149: case 0xf349: case 0xf549: case 0xf749: case 0xf949: case 0xfb49: case 0xfd49: case 0xff49:
		m_core->m_enable_irq = 1;
		return;

	case 0xf14c: case 0xf34c: case 0xf54c: case 0xf74c: case 0xf94c: case 0xfb4c: case 0xfd4c: case 0xff4c:
		m_core->m_enable_fiq = 0;
		return;

	case 0xf14e: case 0xf34e: case 0xf54e: case 0xf74e: case 0xf94e: case 0xfb4e: case 0xfd4e: case 0xff4e:
		m_core->m_enable_fiq = 1;
		return;

	default:
		unimplemented_opcode(op);
		return;
	}

	return;
}


inline void unsp_device::execute_fxxx_110_group(uint16_t op)
{
	//uint32_t len = 1;

	// some sources say this is FFc0, but smartfp clearly uses ff80
	// EXTOP   1 1 1 1   1 1 1 1   1 0 0 0   0 0 0 0    (+16 bit imm)
	if ((op == 0xff80) && m_iso >= 20)
	{
		return execute_extended_group(op);
	}

	//                         |   | |
	// signed * signed  (size 16,1,2,3,4,5,6,7)
	// MULS    1 1 1 1*  r r r 1*  1 0*s s   s r r r    (1* = sign bit, 1* = sign bit 0* = upper size bit)

	// MULS ss with upper size bit not set
	const uint16_t size = ((op >> 3) & 7) ? ((op >> 3) & 7) : 16;
	const uint16_t rd = (op >> 9) & 7;
	const uint16_t rs = op & 7;
	execute_muls_ss(rd, rs, size);
}

void unsp_device::execute_muls_ss(const uint16_t rd, const uint16_t rs, const uint16_t size)
{
	const uint32_t rdv = m_core->m_r[rd];
	const uint32_t rsv = m_core->m_r[rs];
	int64_t lres = 0;
	uint16_t values[16];
	for (uint16_t i = 0; i < size; i++)
	{
		values[i] = read16(rdv + i);
		const uint16_t rhs = read16(rsv + i);
		uint32_t tres = values[i] * rhs;
		if (values[i] & 0x8000)
		{
			tres -= rhs << 16;
		}
		if (rhs & 0x8000)
		{
			tres -= values[i] << 16;
		}
		lres += (int64_t)(int32_t)tres;
	}
	m_core->m_sb = 0;
	if (m_core->m_fir_move)
	{
		for (uint16_t i = size - 1; i > 0; i--)
		{
			write16(rdv + i, values[i - 1]);
		}
	}
	m_core->m_r[rd] += size;
	m_core->m_r[rs] += size;
	m_core->m_r[REG_R4] = (uint16_t)(lres >> 16);
	m_core->m_r[REG_R3] = (uint16_t)lres;
}

inline void unsp_device::execute_fxxx_111_group(uint16_t op)
{
	//uint32_t len = 1;
	//                         |   | |


	// signed * signed  (size 8,9,10,11,12,13,14,15)
	// MULS    1 1 1 1*  r r r 1*  1 1*s s   s r r r    (1* = sign bit, 1* = sign bit 1* = upper size bit)

	// MULS ss with upper size bit set.
	const uint16_t rd = (op >> 9) & 7;
	const uint16_t rs = op & 7;
	execute_muls_ss(rd, rs, ((op >> 3) & 7) + 8);
}

void unsp_device::execute_fxxx_group(uint16_t op)
{
	switch ((op & 0x01c0) >> 6)
	{
	case 0x0:
		return execute_fxxx_000_group(op);

	case 0x1:
		return execute_fxxx_001_group(op);

	case 0x2:
		return execute_fxxx_010_group(op);

	case 0x3:
		return execute_fxxx_011_group(op);

	case 0x4:
		return execute_fxxx_100_group(op);

	case 0x5:
		return execute_fxxx_101_group(op);

	case 0x6:
		return execute_fxxx_110_group(op);

	case 0x7:
		return execute_fxxx_111_group(op);

	}

	return;
}
