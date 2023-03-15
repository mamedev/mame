// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, Ryan Holtz, David Haywood

#include "emu.h"
#include "unsp.h"
#include "unspfe.h"

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

		const uint8_t offset = (m_core->m_r[rs] & 0xf);

		m_core->m_r[REG_SR] &= ~UNSP_Z;
		m_core->m_r[REG_SR] |= BIT(m_core->m_r[rd], offset) ? 0 : UNSP_Z;

		switch (bitop)
		{
		case 0x00: // tstb
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
	else if (((op & 0xf1c0) == 0xe040))
	{
		// Register BITOP  BITOP Rd,offset
		const uint8_t bitop =  (op & 0x0030) >> 4;
		const uint8_t rd =     (op & 0x0e00) >> 9;
		const uint8_t offset = (op & 0x000f) >> 0;
		m_core->m_r[REG_SR] &= ~UNSP_Z;
		m_core->m_r[REG_SR] |= BIT(m_core->m_r[rd], offset) ? 0 : UNSP_Z;

		switch (bitop)
		{
		case 0x00: // tstb
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
		const uint8_t bitop =  (op & 0x0030) >> 4;
		const uint8_t rd =     (op & 0x0e00) >> 9;
		const uint8_t offset = (op & 0x000f) >> 0;
		const uint16_t addr =  m_core->m_r[rd];
		const uint16_t orig =  read16(addr);
		m_core->m_r[REG_SR] &= ~UNSP_Z;
		m_core->m_r[REG_SR] |= BIT(orig, offset) ? 0 : UNSP_Z;

		switch (bitop)
		{
		case 0x00: // tstb
			return;

		case 0x01: // setb
			write16(addr, orig | (1 << offset));
			return;

		case 0x02: // clrb
			write16(addr, orig & ~(1 << offset));
			return;

		case 0x03:
			write16(addr, orig ^ (1 << offset));
			return;
		}
		return;
	}
	else if (((op & 0xf1c0) == 0xe1c0))
	{
		// Memory BITOP    BITOP ds:[Rd], offset
		const uint8_t bitop =  (op & 0x0030) >> 4;
		const uint8_t rd =     (op & 0x0e00) >> 9;
		const uint8_t offset = (op & 0x000f) >> 0;
		const uint32_t addr =  m_core->m_r[rd] | (get_ds() << 16);
		const uint16_t orig =  read16(addr);
		m_core->m_r[REG_SR] &= ~UNSP_Z;
		m_core->m_r[REG_SR] |= BIT(orig, offset) ? 0 : UNSP_Z;

		switch (bitop)
		{
		case 0x00: // tstb
			return;

		case 0x01: // setb
			write16(addr, orig | (1 << offset));
			return;

		case 0x02: // clrb
			write16(addr, orig & ~(1 << offset));
			return;

		case 0x03:
			write16(addr, orig ^ (1 << offset));
			return;
		}
		return;
	}
	else if (((op & 0xf1c8) == 0xe100))
	{
		// Memory BITOP    BITOP [Rd], Rs
		const uint8_t bitop =  (op & 0x0030) >> 4;
		const uint8_t rd =     (op & 0x0e00) >> 9;
		const uint8_t rs =     (op & 0x0007) >> 0;
		const uint8_t offset = (m_core->m_r[rs] & 0xf);
		const uint16_t addr =  m_core->m_r[rd];
		const uint16_t orig =  read16(addr);
		m_core->m_r[REG_SR] &= ~UNSP_Z;
		m_core->m_r[REG_SR] |= BIT(orig, offset) ? 0 : UNSP_Z;

		switch (bitop)
		{
		case 0x00: // tstb
			return;

		case 0x01: // setb
			write16(addr, orig | (1 << offset));
			return;

		case 0x02: // clrb
			write16(addr, orig & ~(1 << offset));
			return;

		case 0x03:
			write16(addr, orig ^ (1 << offset));
			return;
		}
		return;
	}
	else if (((op & 0xf1c8) == 0xe140))
	{
		// Memory BITOP    BITOP ds:[Rd], Rs
		const uint8_t bitop =  (op & 0x0030) >> 4;
		const uint8_t rd =     (op & 0x0e00) >> 9;
		const uint8_t rs =     (op & 0x0007) >> 0;
		const uint8_t offset = (m_core->m_r[rs] & 0xf);
		const uint32_t addr =  m_core->m_r[rd] | (get_ds() << 16);
		const uint16_t orig =  read16(addr);
		m_core->m_r[REG_SR] &= ~UNSP_Z;
		m_core->m_r[REG_SR] |= BIT(orig, offset) ? 0 : UNSP_Z;

		switch (bitop)
		{
		case 0x00: // tstb
			return;

		case 0x01: // setb
			write16(addr, orig | (1 << offset));
			return;

		case 0x02: // clrb
			write16(addr, orig & ~(1 << offset));
			return;

		case 0x03:
			write16(addr, orig ^ (1 << offset));
			return;
		}
		return;
	}
	else if (((op & 0xf1f8) == 0xe008))
	{
		// MUL operations
		// MUL      1 1 1 0*  r r r S*  0 0 0 0   1 r r r     (* = sign bit, fixed here)
		/*
		print_mul(stream, op); // MUL uu
		*/
		// only valid if S is 1, otherwise falls through to shifter

		// MUL uu (unsigned * unsigned)
		uint32_t lres = 0;
		const uint16_t opa = (op >> 9) & 7;
		const uint16_t opb = op & 7;

		m_core->m_icount -= 12; // unknown

		LOGMASKED(LOG_UNSP_MULS, "%s: MUL uu with %04x (unsigned) * %04x (unsigned) (fra:%d) :\n", machine().describe_context(), m_core->m_r[opa], m_core->m_r[opb], m_core->m_fra);

		lres = m_core->m_r[opa] * m_core->m_r[opb];
		m_core->m_r[REG_R4] = lres >> 16;
		m_core->m_r[REG_R3] = (uint16_t)lres;

		LOGMASKED(LOG_UNSP_MULS, "result was : %08x\n", lres);

		return;
	}
	else if (((op & 0xf180) == 0xe080))
	{
		// MULS     1 1 1 0*  r r r S*  1 s s s   s r r r    (* = sign bit, fixed here)
		/*
		// MULS uu
		// only valid if S is 1? otherwise falls through to shifter

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
		case 0x00: // jak_smwm train movement
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s asr %s\n", regs[rd], regs[rd], regs[rs]);
			int16_t rdval = (int16_t)((uint16_t)m_core->m_r[rd]);
			int shift = (m_core->m_r[rs] & 0x1f);
			uint32_t res;
			res = rdval >> shift;
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %08x\n", res);
			m_core->m_r[rd] = res;
			return;
		}

		case 0x01: // jak_car2 on starting a game, myac220 'piggy golf' ball movement
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "%s = %s asror %s (%04x %04x)\n", regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			const int32_t rdval = (int32_t)(m_core->m_r[rd] << 16);
			const int shift = (m_core->m_r[rs] & 0x1f);
			const uint32_t res = rdval >> shift;
			m_core->m_r[REG_R3] |= (uint16_t)res;
			m_core->m_r[REG_R4] = (uint16_t)(res >> 16);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %04x%04x\n", m_core->m_r[REG_R4], m_core->m_r[REG_R3]);
			return;
		}

		case 0x02:
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsl %s (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);
			const uint32_t rdval = (uint16_t)(m_core->m_r[rd]);
			const int shift = (m_core->m_r[rs] & 0x1f);
			const uint32_t res = (uint16_t)(rdval << shift);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %08x\n", res);
			m_core->m_r[rd] = res;
			return;
		}

		case 0x03:
		{
			// wrlshunt uses this
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lslor %s  (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);

			const uint32_t rdval = (uint16_t)m_core->m_r[rd];
			const int shift = (m_core->m_r[rs] & 0x1f);
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
			const int shift = (m_core->m_r[rs] & 0x1f);
			const uint32_t res = (uint16_t)(rdval >> shift);
			LOGMASKED(LOG_UNSP_SHIFTS, "result: %08x\n", res);
			m_core->m_r[rd] = res;
			return;
		}

		case 0x05:
		{
			LOGMASKED(LOG_UNSP_SHIFTS, "pc:%06x: %s = %s lsror %s  (%04x %04x)\n", UNSP_LPC, regs[rd], regs[rd], regs[rs], m_core->m_r[rd], m_core->m_r[rs]);
			const uint32_t rdval = m_core->m_r[rd] << 16;
			const int shift = (m_core->m_r[rs] & 0x1f);
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
