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

#define CMP_OVERFLOW32(r, s, d)		((((d) ^ (s)) & ((d) ^ (r)) & 0x80000000) ? 1 : 0)
#define CMP_OVERFLOW16(r, s, d)		((((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? 1 : 0)
#define CMP_OVERFLOW8(r, s, d)		((((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? 1 : 0)
#define CARRY32(x)					(((x) & (((UINT64)1) << 32)) ? 1 : 0)
#define CARRY16(x)					(((x) & 0x10000) ? 1 : 0)
#define CARRY8(x)					(((x) & 0x100) ? 1 : 0)
#define SIGN32(x)					(((x) & 0x80000000) ? 1 : 0)
#define SIGN16(x)					(((x) & 0x8000) ? 1 : 0)
#define SIGN8(x)					(((x) & 0x80) ? 1 : 0)



bool tms32082_mp_device::test_condition(int condition, UINT32 value)
{
	switch (condition)
	{
		case 0x00:	return false;					// never, byte
		case 0x01:	return (INT8)(value) > 0;		// greater than zero, byte
		case 0x02:	return (INT8)(value) == 0;		// equals zero, byte
		case 0x03:	return (INT8)(value) >= 0;		// greater than or equal to zero, byte
		case 0x04:	return (INT8)(value) < 0;		// less than zero, byte
		case 0x05:	return (INT8)(value) != 0;		// not equal to zero, byte
		case 0x06:	return (INT8)(value) <= 0;		// less than or equal to zero, byte
		case 0x07:	return true;					// always, byte
		case 0x08:	return false;					// never, word
		case 0x09:	return (INT16)(value) > 0;		// greater than zero, word
		case 0x0a:	return (INT16)(value) == 0;		// equals zero, word
		case 0x0b:	return (INT16)(value) >= 0;		// greater than or equal to zero, word
		case 0x0c:	return (INT16)(value) < 0;		// less than zero, word
		case 0x0d:	return (INT16)(value) != 0;		// not equal to zero, word
		case 0x0e:	return (INT16)(value) <= 0;		// less than or equal to zero, word
		case 0x0f:	return true;					// always, word
		case 0x10:	return false;					// never, dword
		case 0x11:	return (INT32)(value) > 0;		// greater than zero, dword
		case 0x12:	return (INT32)(value) == 0;		// equals zero, dword
		case 0x13:	return (INT32)(value) >= 0;		// greater than or equal to zero, dword
		case 0x14:	return (INT32)(value) < 0;		// less than zero, dword
		case 0x15:	return (INT32)(value) != 0;		// not equal to zero, dword
		case 0x16:	return (INT32)(value) <= 0;		// less than or equal to zero, dword
		case 0x17:	return true;					// always, dword
		default:	return false;					// reserved
	}
}

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

		case 0x0b:			// shift.ez
		{
			int r = (m_ir & (1 << 10));
			int inv = (m_ir & (1 << 11));
			int rot = OP_ROTATE();
			int end = OP_ENDMASK();
			UINT32 source = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT32 endmask = SHIFT_MASK[end ? end : 32];
			if (inv) endmask = ~endmask;

			int shift = r ? 32-rot : rot;
			UINT32 shiftmask = SHIFT_MASK[shift ? shift : 32];
			UINT32 compmask = endmask & shiftmask;

			UINT32 res = 0;
			if (r)		// right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else		// left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0e:			// shift.iz
		{
			int r = (m_ir & (1 << 10));
			int inv = (m_ir & (1 << 11));
			int rot = OP_ROTATE();
			int end = OP_ENDMASK();
			UINT32 source = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT32 endmask = SHIFT_MASK[end ? end : 32];
			if (inv) endmask = ~endmask;

			UINT32 shiftmask = SHIFT_MASK[r ? 32-rot : rot];
			UINT32 compmask = endmask & ~shiftmask;

			UINT32 res = 0;
			if (r)		// right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else		// left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x11:			// and
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] & imm;
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

		case 0x4d:			// bcnd.a
		{
			INT32 offset = OP_SIMM15();
			int code = OP_RD();
			int rs = OP_RS();

			if (test_condition(code, m_reg[rs]))
			{
				m_fetchpc = m_pc + (offset * 4);
			}
			break;
		}

		case 0x50:			// cmp
		{
			UINT32 src1 = OP_SIMM15();
			UINT32 src2 = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT16 src1_16 = (UINT16)(src1);
			UINT8 src1_8 = (UINT8)(src1);
			UINT16 src2_16 = (UINT16)(src2);
			UINT8 src2_8 = (UINT8)(src2);

			UINT64 res32 = (UINT64)src1 - (UINT64)src2;
			int z32 = (res32 == 0) ? 1 : 0;
			int n32 = SIGN32(res32);
			int v32 = CMP_OVERFLOW32(res32, src2, src1);
			int c32 = CARRY32(res32);

			UINT32 res16 = (UINT32)src1_16 - (UINT32)src2_16;
			int z16 = (res16 == 0) ? 1 : 0;
			int n16 = SIGN16(res16);
			int v16 = CMP_OVERFLOW16(res16, src2_16, src1_16);
			int c16 = CARRY16(res16);

			UINT16 res8 = (UINT16)src1_8 - (UINT16)src2_8;
			int z8 = (res8 == 0) ? 1 : 0;
			int n8 = SIGN8(res8);
			int v8 = CMP_OVERFLOW8(res8, src2_8, src1_8);
			int c8 = CARRY8(res8);

			UINT32 flags = 0;
			// 32-bits (bits 20-29)
			flags |= ((c32) & 1) << 29;									// higher than or same (C)
			flags |= ((~c32) & 1) << 28;								// lower than (~C)
			flags |= ((~c32|z32) & 1) << 27;							// lower than or same (~C|Z)
			flags |= ((c32&~z32) & 1) << 26;							// higher than (C&~Z)
			flags |= (((n32&v32)|(~n32&~v32)) & 1) << 25;				// greater than or equal (N&V)|(~N&~V)
			flags |= (((n32&~v32)|(~n32&v32)) & 1) << 24;				// less than (N&~V)|(~N&V)
			flags |= (((n32&~v32)|(~n32&v32)|(z32)) & 1) << 23;			// less than or equal (N&~V)|(~N&V)|Z
			flags |= (((n32&v32&~z32)|(~n32&~v32&~z32)) & 1) << 22;		// greater than (N&V&~Z)|(~N&~V&~Z)
			flags |= ((~z32) & 1) << 21;								// not equal (~Z)
			flags |= ((z32) & 1) << 20;									// equal (Z)				
			// 16-bits (bits 10-19)
			flags |= ((c16) & 1) << 19;									// higher than or same (C)
			flags |= ((~c16) & 1) << 18;								// lower than (~C)
			flags |= ((~c16|z16) & 1) << 17;							// lower than or same (~C|Z)
			flags |= ((c16&~z16) & 1) << 16;							// higher than (C&~Z)
			flags |= (((n16&v16)|(~n16&~v16)) & 1) << 15;				// greater than or equal (N&V)|(~N&~V)
			flags |= (((n16&~v16)|(~n16&v16)) & 1) << 14;				// less than (N&~V)|(~N&V)
			flags |= (((n16&~v16)|(~n16&v16)|(z16)) & 1) << 13;			// less than or equal (N&~V)|(~N&V)|Z
			flags |= (((n16&v16&~z16)|(~n16&~v16&~z16)) & 1) << 12;		// greater than (N&V&~Z)|(~N&~V&~Z)
			flags |= ((~z16) & 1) << 11;								// not equal (~Z)
			flags |= ((z16) & 1) << 10;									// equal (Z)	
			// 8-bits (bits 0-9)
			flags |= ((c8) & 1) << 9;									// higher than or same (C)
			flags |= ((~c8) & 1) << 8;									// lower than (~C)
			flags |= ((~c8|z8) & 1) << 7;								// lower than or same (~C|Z)
			flags |= ((c8&~z8) & 1) << 6;								// higher than (C&~Z)
			flags |= (((n8&v8)|(~n8&~v8)) & 1) << 5;					// greater than or equal (N&V)|(~N&~V)
			flags |= (((n8&~v8)|(~n8&v8)) & 1) << 4;					// less than (N&~V)|(~N&V)
			flags |= (((n8&~v8)|(~n8&v8)|(z8)) & 1) << 3;				// less than or equal (N&~V)|(~N&V)|Z
			flags |= (((n8&v8&~z8)|(~n8&~v8&~z8)) & 1) << 2;			// greater than (N&V&~Z)|(~N&~V&~Z)
			flags |= ((~z8) & 1) << 1;									// not equal (~Z)
			flags |= ((z8) & 1) << 0;									// equal (Z)	

			if (rd)
				m_reg[rd] = flags;
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
			fatalerror("execute_short_imm(): %08X: opcode %08X (%02X)", m_pc, m_ir, (m_ir >> 15) & 0x7f);
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
		case 0x04:			// cmnd
		{
			UINT32 data = has_imm ? imm32 : m_reg[OP_SRC1()];

			printf("CMND %08X\n", data);
			break;
		}

		case 0x16:			// shift.ez
		{
			int r = (m_ir & (1 << 10));
			int inv = (m_ir & (1 << 11));
			int rot = m_reg[OP_ROTATE()];
			int end = OP_ENDMASK();
			UINT32 source = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT32 endmask = end ? SHIFT_MASK[end ? end : 32] : m_reg[OP_ROTATE()+1];
			if (inv) endmask = ~endmask;

			int shift = r ? 32-rot : rot;
			UINT32 shiftmask = SHIFT_MASK[shift ? shift : 32];
			UINT32 compmask = endmask & shiftmask;

			UINT32 res = 0;
			if (r)		// right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else		// left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x24:
		case 0x25:			// and.tf
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 src1 = has_imm ? imm32 : m_reg[OP_SRC1()];

			if (rd)
				m_reg[rd] = src1 & ~(m_reg[rs]);
			break;
		}

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
			UINT32 r = m_program->read_word(address);
			if (r & 0x8000) r |= 0xffff0000;
			
			if (rd)
				m_reg[rd] = r;

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

		case 0x88:
		case 0x89:			// jsr
		{
			int link = OP_LINK();
			int base = OP_BASE();

			if (link)
				m_reg[link] = m_fetchpc + 4;

			UINT32 address = m_reg[base] + (has_imm ? imm32 : m_reg[OP_SRC1()]);

			m_pc = m_fetchpc;
			delay_slot();

			m_fetchpc = address;
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

		case 0xb2:
		case 0xb3:			// addu
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		default:
			fatalerror("execute_reg_long_imm(): %08X: opcode %08X (%02X)", m_pc, m_ir, (m_ir >> 12) & 0xff);
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