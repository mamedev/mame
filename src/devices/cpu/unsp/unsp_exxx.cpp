// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "debugger.h"

#include "unspdasm.h"

#define LOG_UNSP_MULS            (1U << 1)
#define LOG_UNSP_SHIFTS          (1U << 2)

#define VERBOSE             (LOG_UNSP_SHIFTS)

#include "logmacro.h"


void unsp_device::execute_exxx_group(uint16_t op)
{
	// several exxx opcodes have already been decoded as jumps by the time we get here
	//logerror("<DUNNO>\n");
	unimplemented_opcode(op);
	return;
}

void unsp_12_device::execute_exxx_group(uint16_t op)
{
	// several exxx opcodes have already been decoded as jumps by the time we get here

	//   Register BITOP  BITOP Rd,Rs             1 1 1 0   r r r 0   0 0 b b   0 r r r
	//   Register BITOP  BITOP Rd,offset         1 1 1 0   r r r 0   0 1 b b   o o o o
	//   Memory BITOP    BITOP [Rd], offset      1 1 1 0   r r r 1   1 0 b b   o o o o
	//   Memory BITOP    BITOP ds:[Rd], offset   1 1 1 0   r r r 1   1 1 b b   o o o o
	//   Memory BITOP    BITOP [Rd], Rs          1 1 1 0   r r r 1   0 0 b b   0 r r r
	//   Memory BITOP    BITOP ds:[Rd], Rs       1 1 1 0   r r r 1   0 1 b b   0 r r r

	if (((op & 0xf1c8) == 0xe000))
	{
		// Register BITOP  BITOP Rd,Rs
		uint8_t bitop = (op & 0x0030) >> 4;
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t rs =    (op & 0x0007) >> 0;
		logerror("%s %s,%s\n", bitops[bitop], regs[rd], regs[rs]);
		unimplemented_opcode(op);
		return;
	}
	else if (((op & 0xf1c0) == 0xe040))
	{
		// Register BITOP  BITOP Rd,offset
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t rd =     (op & 0x0e00) >> 9;
		uint8_t offset = (op & 0x000f) >> 0;

		switch (bitop)
		{
		case 0x00:
			fatalerror("UNSP: unknown opcode tstb Rd,offset (%04x) at %04x\n", op, UNSP_LPC);
			return;

		case 0x01: // setb
			m_core->m_r[rd] |= (1 << offset);
			return;

		case 0x02: // clrb
			m_core->m_r[rd] &= ~(1 << offset);
			return;

		case 0x03:
			m_core->m_r[rd] ^= (1 << offset);
			return;
		}
		return;
	}
	else if (((op & 0xf1c0) == 0xe180))
	{
		// Memory BITOP    BITOP [Rd], offset
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t rd =     (op & 0x0e00) >> 9;
		uint8_t offset = (op & 0x000f) >> 0;
		logerror("%s [%s],%d\n", bitops[bitop], regs[rd], offset);
		unimplemented_opcode(op);
		return;
	}
	else if (((op & 0xf1c0) == 0xe1c0))
	{
		// Memory BITOP    BITOP ds:[Rd], offset
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t rd =     (op & 0x0e00) >> 9;
		uint8_t offset = (op & 0x000f) >> 0;
		logerror("%s ds:[%s],%d\n", bitops[bitop], regs[rd], offset);
		unimplemented_opcode(op);
		return;
	}
	else if (((op & 0xf1c8) == 0xe100))
	{
		// Memory BITOP    BITOP [Rd], Rs
		uint8_t bitop = (op & 0x0030) >> 4;
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t rs =    (op & 0x0007) >> 0;
		logerror("%s [%s],%s\n", bitops[bitop], regs[rd], regs[rs]);
		unimplemented_opcode(op);
		return;
	}
	else if (((op & 0xf1c8) == 0xe140))
	{
		// Memory BITOP    BITOP ds:[Rd], Rs
		uint8_t bitop = (op & 0x0030) >> 4;
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t rs =    (op & 0x0007) >> 0;
		logerror("%s ds:[%s],%s\n", bitops[bitop], regs[rd], regs[rs]);
		unimplemented_opcode(op);
		return;
	}
	else if (((op & 0xf0f8) == 0xe008))
	{
		// MUL operations
		// MUL      1 1 1 0*  r r r S*  0 0 0 0   1 r r r     (* = sign bit, fixed here)
		/*
		print_mul(stream, op); // MUL uu or MUL su
		*/

		if (op & 0x0100)
		{
			// MUL su ( unsigned * signed )
			const uint16_t opa = (op >> 9) & 7;
			const uint16_t opb = op & 7;
			m_core->m_icount -= 12;

			LOGMASKED(LOG_UNSP_MULS, "%s: MUL su with %04x (signed) * %04x (unsigned) (fra:%d) :\n", machine().describe_context(), m_core->m_r[opa], m_core->m_r[opb], m_core->m_fra);

			uint32_t lres = m_core->m_r[opa] * m_core->m_r[opb];
			if (m_core->m_r[opa] & 0x8000)
			{
				lres -= m_core->m_r[opb] << 16;
			}
			m_core->m_r[REG_R4] = lres >> 16;
			m_core->m_r[REG_R3] = (uint16_t)lres;

			LOGMASKED(LOG_UNSP_MULS, "result was : %08x\n", lres);

			return;
		}
		else
		{
			// MUL uu (unsigned * unsigned)
			uint32_t lres = 0;
			const uint16_t opa = (op >> 9) & 7;
			const uint16_t opb = op & 7;

			m_core->m_icount -= 12; // unknown
			lres = m_core->m_r[opa] * m_core->m_r[opb];
			m_core->m_r[REG_R4] = lres >> 16;
			m_core->m_r[REG_R3] = (uint16_t)lres;
			return;
		}
		return;
	}
	else if (((op & 0xf080) == 0xe080))
	{
		// MULS     1 1 1 0*  r r r S*  1 s s s   s r r r    (* = sign bit, fixed here)
		/*
		// MULS uu or MULS su (invalid?)
		print_muls(stream, op);
		*/
		LOGMASKED(LOG_UNSP_MULS, "MULS uu or su\n");
		unimplemented_opcode(op);
		return;
	}
	else if (((op & 0xf188) == 0xe108))
	{
		// 16 bit Shift    1 1 1 0   r r r 1   0 l l l   1 r r r
		uint8_t rd =    (op & 0x0e00) >> 9;
		uint8_t shift = (op & 0x0070) >> 4;
		uint8_t rs =    (op & 0x0007) >> 0;

		switch (shift)
		{
		case 0x00:
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s asr %s\n", regs[rd], regs[rd], regs[rs]);

			uint32_t rdval = (uint16_t)(m_core->m_r[rd]);
			int shift = (m_core->m_r[rs] & 0x01f);
			if (shift == 0)
				return;

			uint32_t res;
			if (BIT(rd, (32 - shift)))
			{
				if (shift >= 16)
				{
					res = 0x0000ffff;
				}
				else
				{
					res = (rdval >> shift) | (uint16_t)(0xffff0000 >> shift);
				}
			}
			else
			{
				if (shift >= 16)
				{
					res = 0;
				}
				else
				{
					res = rdval >> shift;
				}
			}

			LOGMASKED(LOG_UNSP_SHIFTS, "result: %08x\n", res);
			m_core->m_r[rd] = res;
			return;
		}

		case 0x01: // jak_car2 on starting a game
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s asror %s (%04x %04x)\n", regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			const int32_t rdval = (int32_t)(m_core->m_r[rd] << 16);
			const int shift = (m_core->m_r[rs] & 0x01f);
			const uint32_t res = rdval >> shift;
			m_core->m_r[REG_R3] |= (uint16_t)res;
			m_core->m_r[REG_R4] |= (uint16_t)(res >> 16);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %04x%04x\n", m_core->m_r[REG_R4], m_core->m_r[REG_R3]);
			return;
		}

		case 0x02:
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsl %s (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);
			const uint32_t rdval = (uint16_t)(m_core->m_r[rd]);
			const int shift = (m_core->m_r[rs] & 0x01f);
			const uint32_t res = (uint16_t)(rdval << shift);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %08x\n", res);
			m_core->m_r[rd] = res;
			return;
		}

		case 0x03:
		{
			// wrlshunt uses this
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lslor %s  (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			const uint32_t rdval = m_core->m_r[rd];
			const int shift = (m_core->m_r[rs] & 0x01f);
			const uint32_t res = rdval << shift;
			m_core->m_r[REG_R3] = (uint16_t)res;
			m_core->m_r[REG_R4] |= (uint16_t)(res >> 16);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %04x%04x\n", m_core->m_r[REG_R4], m_core->m_r[REG_R3]);
			return;
		}

		case 0x04:
		{
			// smartfp loops increasing shift by 4 up to values of 28
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsr %s  (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			const uint32_t rdval = (uint16_t)(m_core->m_r[rd]);
			const int shift = (m_core->m_r[rs] & 0x01f);
			const uint32_t res = (uint16_t)(rdval >> shift);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %08x\n", res);
			m_core->m_r[rd] = res;
			return;
		}

		case 0x05:
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsror %s  (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);
			const uint32_t rdval = m_core->m_r[rd] << 16;
			const int shift = (m_core->m_r[rs] & 0x01f);
			const uint32_t res = rdval >> shift;
			m_core->m_r[REG_R3] |= (uint16_t)res;
			m_core->m_r[REG_R4] = (uint16_t)(res >> 16);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %04x%04x\n", m_core->m_r[REG_R4], m_core->m_r[REG_R3]);
			return;
		}

		case 0x06:
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s rol %s\n", regs[rd], regs[rd], regs[rs]);
			unimplemented_opcode(op);
			return;

		case 0x07:
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s ror %s\n", regs[rd], regs[rd], regs[rs]);
			unimplemented_opcode(op);
			return;
		}
	}

	logerror("<DUNNO>\n");
	unimplemented_opcode(op);
	return;
}
