// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

#include "debugger.h"

#include "unspdasm.h"

#define LOG_UNSP_SHIFTS          (1U << 2)
#define LOG_UNSP_MULS            (1U << 1)

#define VERBOSE             (0)

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

			LOGMASKED(LOG_UNSP_MULS, "%s: MUL su with %04x (signed) * %04x (unsigned) : ", machine().describe_context(), m_core->m_r[opa], m_core->m_r[opb]);

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
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s asr %s\n", regs[rd], regs[rd], regs[rs]);
			unimplemented_opcode(op);
			return;

		case 0x01: // jak_car2 on starting a game
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s asror %s\n", regs[rd], regs[rd], regs[rs]);
			if (rd != 4) // 4 = R4 3 = R3 - the 'register bleeding' is only verified as needed between r3/r4, so bail for other regs until verified
				unimplemented_opcode(op);

			uint32_t res = (uint16_t)(m_core->m_r[rd]);
			int shift = (m_core->m_r[rs] & 0x01f);
			res <<= 16;

			if (res & 0x80000000)
			{
				res = res >> shift;
				res |= (0xffffffff << (32 - shift));

				m_core->m_r[rd] = (res >> 16);
				m_core->m_r[rd - 1] |= (res & 0xffff); // register bleeding?
			}
			else
			{
				res = res >> shift;
				m_core->m_r[rd] = (res >> 16);
				m_core->m_r[rd - 1] |= (res & 0xffff); // register bleeding?
			}

			LOGMASKED(LOG_UNSP_SHIFTS, "result %04x\n", m_core->m_r[rd]);
			return;
		}

		case 0x02:
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsl %s (%04x %04x) : ", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			m_core->m_r[rd] = (uint16_t)((m_core->m_r[rd]&0xffff) << (m_core->m_r[rs] & 0x01f));

			LOGMASKED(LOG_UNSP_SHIFTS, "result %04x\n", m_core->m_r[rd]);

			return;

		case 0x03:
		{
			// wrlshunt uses this
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lslor %s  (%04x %04x) : ", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			if (rd != 3) // 4 = R4 3 = R3 - the 'register bleeding' is only verified as needed between r3/r4, so bail for other regs until verified
				unimplemented_opcode(op);

			uint32_t res = (uint16_t)(m_core->m_r[rd]);

			res <<= (m_core->m_r[rs] & 0x01f);

			m_core->m_r[rd] = (res & 0xffff);
			m_core->m_r[rd + 1] |= (res >> 16); // register bleeding?

			LOGMASKED(LOG_UNSP_SHIFTS, "result %04x\n", m_core->m_r[rd]);
			return;
		}

		case 0x04:
			// smartfp loops increasing shift by 4 up to values of 28? (but regs are 16-bit?)
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsr %s  (%04x %04x) : ", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);
			m_core->m_r[rd] = (uint16_t)((m_core->m_r[rd]&0xffff) >> (m_core->m_r[rs] & 0x1f));
			LOGMASKED(LOG_UNSP_SHIFTS, "result %04x\n", m_core->m_r[rd]);
			return;

		case 0x05:
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsror %s  (%04x %04x) : ", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			if (rd != 4) // 4 = R4 3 = R3 - the 'register bleeding' is only verified as needed between r3/r4, so bail for other regs until verified
				unimplemented_opcode(op);

			uint32_t res = (uint16_t)(m_core->m_r[rd]);

			res <<= 16;

			res = res >> (m_core->m_r[rs] & 0x01f);

			// register bleeding behavior needed (for example) in jak_cars2 nand browser when increasing upper digits of address
			// TODO: check if I'm missing something becaus this doesn't really seem logical, maybe other code is meant to take care of those bits?
			//  (although R3/R4 are the 'MR' register used for multiply stuff, so maybe there is some logic to this?)

			// code attempts to put a 32-bit value in r4/r3 then shift it like this?
			//3c990: e92a            r4 = r4 lsl r2
			//3c991: e73a            r3 = r3 lslor r2

			m_core->m_r[rd] = (res >> 16);
			m_core->m_r[rd - 1] |= (res & 0xffff); // register bleeding?


			LOGMASKED(LOG_UNSP_SHIFTS, "result %04x\n", m_core->m_r[rd]);
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
