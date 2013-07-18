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
#define OP_ROTATE() (m_ir & 0x1f)
#define OP_ENDMASK() ((m_ir >> 5) & 0x1f)
#define OP_SRC1() (m_ir & 0x1f)

#define ROTATE_L(x, r) ((x << r) | (x >> (32-r)))
#define ROTATE_R(x, r) ((x >> r) | (x << (32-r)))


void tms32082_mp_device::execute_short_imm()
{
	switch ((m_ir >> 15) & 0x7f)
	{
		case 0x04:			// rdcr
		{
			int rd = OP_RD();
			INT32 imm = OP_SIMM15();

			UINT32 r = read_creg(imm);

			if (rd)
				m_reg[rd] = r;
			break;
		}

		case 0x05:			// swcr
		{
			int rd = OP_RD();
			int rs = OP_RS();
			INT32 imm = OP_SIMM15();

			UINT32 r = read_creg(imm);
			if (rd)
				m_reg[rd] = r;

			write_creg(imm, m_reg[rs]);
			break;
		}

		case 0x0e:			// shift.iz
		{
			int r = (m_ir & (1 << 10));
			int inv = (m_ir & (1 << 11));
			int rot = OP_ROTATE();
			int source = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT32 endmask = SHIFT_MASK[OP_ENDMASK()];
			if (inv) endmask = ~endmask;

			UINT32 shiftmask = SHIFT_MASK[rot];
			UINT32 compmask = endmask & ~shiftmask;

			UINT32 res = 0;
			if (r)		// right
			{
				res = (ROTATE_R(source, rot) & compmask) | (m_reg[rd] & ~compmask);
			}
			else		// left
			{
				res = (ROTATE_L(source, rot) & compmask) | (m_reg[rd] & ~compmask);
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x17:			// or
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] | imm;
			break;
		}

		case 0x36:
		case 0x32:			// st
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;

			m_program->write_dword(address, m_reg[rd]);

			if (m && base)
				m_reg[base] = address;
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

		case 0x4b:			// bbo.a
		{
			int bitnum = OP_BITNUM();
			INT32 offset = OP_SIMM15();
			int rs = OP_RS();

			if ((m_reg[rs] & (1 << bitnum)) != 0)
			{
				m_fetchpc = m_pc + (offset * 4);
			}
			break;
		}

		case 0x59:			// addu
		{
			INT32 imm = OP_SIMM15();
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] + imm;
			break;
		}

		default:
			fatalerror("tms32082_mp_device::execute_short_imm(): opcode %08X (%02X)", m_ir, (m_ir >> 15) & 0x7f);
	}
}

void tms32082_mp_device::execute_reg_long_imm()
{
	UINT32 imm32 = 0;

	int has_imm = (m_ir & (1 << 12));

	if (has_imm)
		imm32 = fetch();

	switch ((m_ir >> 12) & 0xff)
	{
		case 0x2e:
		case 0x2f:			// or
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] | (has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		case 0x43:			// ld.h
		case 0x4b:
		{
			int shift = (m_ir & (1 << 11)) ? 1 : 0;
			int m = m_ir & (1 << 15);

			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + ((has_imm ? imm32 : m_reg[OP_SRC1()]) << shift);
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

		case 0x6d:
		case 0x65:			// st
		{
			int shift = (m_ir & (1 << 11)) ? 2 : 0;
			int m = m_ir & (1 << 15);

			int base = OP_BASE();

			UINT32 address = m_reg[base] + ((has_imm ? imm32 : m_reg[OP_SRC1()]) << shift);
			m_program->write_dword(address, m_reg[OP_RD()]);

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x8a:
		case 0x8b:			// jsr.a
		{
			int link = OP_LINK();
			int base = OP_BASE();

			if (link)
				m_reg[link] = m_fetchpc;

			m_fetchpc = m_reg[base] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
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