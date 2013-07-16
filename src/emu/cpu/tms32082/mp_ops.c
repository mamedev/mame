// TMS320C82 Master Processor core execution

#include "emu.h"
#include "tms32082.h"


#define OP_LINK() ((m_ir >> 27) & 0x1f)
#define OP_RD() ((m_ir >> 27) & 0x1f)
#define OP_RS() ((m_ir >> 22) & 0x1f)
#define OP_BASE() ((m_ir >> 22) & 0x1f)
#define OP_SIMM15() ((m_ir & 0x4000) ? (0xffffe000 | (m_ir & 0x7fff)) : (m_ir & 0x7fff))
#define OP_UIMM15() (m_ir & 0x7fff)
#define OP_BITNUM() ((m_ir >> 27) & 0x1f)



void tms32082_mp_device::execute_short_imm()
{
	switch ((m_ir >> 15) & 0x7f)
	{
		case 0x17:			// or
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] | imm;
			break;
		}

		case 0x49:			// bbz.a
		{
			int bitnum = OP_BITNUM();
			INT32 offset = OP_SIMM15();
			int rs = OP_RS();

			if ((m_reg[rs] & (1 << bitnum)) == 0)
			{
				m_fetchpc = m_pc + (offset * 4);
			}
			break;
		}

		default:
			fatalerror("tms32082_mp_device::execute_short_imm(): opcode %08X (%02X)", m_ir, (m_ir >> 15) & 0x7f);
	}
}

void tms32082_mp_device::execute_reg_long_imm()
{
	UINT32 imm32 = 0;

	if (m_ir & (1 << 12))
		imm32 = fetch();

	switch ((m_ir >> 12) & 0xff)
	{
		case 0x2f:			// or
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] | imm32;
			break;
		}

		case 0x43:			// ld.h
		case 0x4b:
		{
			int shift = (m_ir & (1 << 11)) ? 1 : 0;
			int m = m_ir & (1 << 15);

			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + (imm32 << shift);
			if (rd)
			{
				m_reg[rd] = m_program->read_word(address);
				if (m_reg[rd] & 0x8000)
					m_reg[rd] |= 0xffff0000;
			}

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x8b:			// jsr.a
		{
			int link = OP_LINK();
			int base = OP_BASE();

			if (link)
				m_reg[link] = m_fetchpc;

			m_fetchpc = m_reg[base] + (INT32)(imm32);
			break;
		}

		default:
			fatalerror("tms32082_mp_device::execute_reg_long_imm(): opcode %08X (%02X)", m_ir, (m_ir >> 12) & 0xff);
	}
}


void tms32082_mp_device::execute()
{
	switch ((m_ir >> 20) & 3)
	{
		case 0:
		case 1:
		case 2:
			execute_short_imm();
			break;

		case 3:
			execute_reg_long_imm();
			break;
	}
}