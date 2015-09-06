// license:BSD-3-Clause
// copyright-holders:Ville Linde
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
#define OP_PD() ((m_ir >> 9) & 0x3)
#define OP_P1() ((m_ir >> 5) & 0x3)
#define OP_P2() ((m_ir >> 7) & 0x3)
#define OP_ACC() ((m_ir >> 15) & 0x2) | ((m_ir >> 11) & 1)

#define ROTATE_L(x, r) ((x << r) | (x >> (32-r)))
#define ROTATE_R(x, r) ((x >> r) | (x << (32-r)))

#define CMP_OVERFLOW32(r, s, d)     ((((d) ^ (s)) & ((d) ^ (r)) & 0x80000000) ? 1 : 0)
#define CMP_OVERFLOW16(r, s, d)     ((((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? 1 : 0)
#define CMP_OVERFLOW8(r, s, d)      ((((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? 1 : 0)
#define CARRY32(x)                  (((x) & (((UINT64)1) << 32)) ? 1 : 0)
#define CARRY16(x)                  (((x) & 0x10000) ? 1 : 0)
#define CARRY8(x)                   (((x) & 0x100) ? 1 : 0)
#define SIGN32(x)                   (((x) & 0x80000000) ? 1 : 0)
#define SIGN16(x)                   (((x) & 0x8000) ? 1 : 0)
#define SIGN8(x)                    (((x) & 0x80) ? 1 : 0)

#define SIGN_EXTEND(x, r)           ((x) | (((x) & (0x80000000 >> r)) ? ((INT32)(0x80000000) >> r) : 0))



bool tms32082_mp_device::test_condition(int condition, UINT32 value)
{
	switch (condition)
	{
		case 0x00:  return false;                   // never, byte
		case 0x01:  return (INT8)(value) > 0;       // greater than zero, byte
		case 0x02:  return (INT8)(value) == 0;      // equals zero, byte
		case 0x03:  return (INT8)(value) >= 0;      // greater than or equal to zero, byte
		case 0x04:  return (INT8)(value) < 0;       // less than zero, byte
		case 0x05:  return (INT8)(value) != 0;      // not equal to zero, byte
		case 0x06:  return (INT8)(value) <= 0;      // less than or equal to zero, byte
		case 0x07:  return true;                    // always, byte
		case 0x08:  return false;                   // never, word
		case 0x09:  return (INT16)(value) > 0;      // greater than zero, word
		case 0x0a:  return (INT16)(value) == 0;     // equals zero, word
		case 0x0b:  return (INT16)(value) >= 0;     // greater than or equal to zero, word
		case 0x0c:  return (INT16)(value) < 0;      // less than zero, word
		case 0x0d:  return (INT16)(value) != 0;     // not equal to zero, word
		case 0x0e:  return (INT16)(value) <= 0;     // less than or equal to zero, word
		case 0x0f:  return true;                    // always, word
		case 0x10:  return false;                   // never, dword
		case 0x11:  return (INT32)(value) > 0;      // greater than zero, dword
		case 0x12:  return (INT32)(value) == 0;     // equals zero, dword
		case 0x13:  return (INT32)(value) >= 0;     // greater than or equal to zero, dword
		case 0x14:  return (INT32)(value) < 0;      // less than zero, dword
		case 0x15:  return (INT32)(value) != 0;     // not equal to zero, dword
		case 0x16:  return (INT32)(value) <= 0;     // less than or equal to zero, dword
		case 0x17:  return true;                    // always, dword
		default:    return false;                   // reserved
	}
}

UINT32 tms32082_mp_device::calculate_cmp(UINT32 src1, UINT32 src2)
{
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
	flags |= ((~c32) & 1) << 29;                                // higher than or same (C)
	flags |= ((c32) & 1) << 28;                                 // lower than (~C)
	flags |= ((c32|z32) & 1) << 27;                             // lower than or same (~C|Z)
	flags |= ((~c32&~z32) & 1) << 26;                           // higher than (C&~Z)
	flags |= (((n32&v32)|(~n32&~v32)) & 1) << 25;               // greater than or equal (N&V)|(~N&~V)
	flags |= (((n32&~v32)|(~n32&v32)) & 1) << 24;               // less than (N&~V)|(~N&V)
	flags |= (((n32&~v32)|(~n32&v32)|(z32)) & 1) << 23;         // less than or equal (N&~V)|(~N&V)|Z
	flags |= (((n32&v32&~z32)|(~n32&~v32&~z32)) & 1) << 22;     // greater than (N&V&~Z)|(~N&~V&~Z)
	flags |= ((~z32) & 1) << 21;                                // not equal (~Z)
	flags |= ((z32) & 1) << 20;                                 // equal (Z)
	// 16-bits (bits 10-19)
	flags |= ((~c16) & 1) << 19;                                // higher than or same (C)
	flags |= ((c16) & 1) << 18;                                 // lower than (~C)
	flags |= ((c16|z16) & 1) << 17;                             // lower than or same (~C|Z)
	flags |= ((~c16&~z16) & 1) << 16;                           // higher than (C&~Z)
	flags |= (((n16&v16)|(~n16&~v16)) & 1) << 15;               // greater than or equal (N&V)|(~N&~V)
	flags |= (((n16&~v16)|(~n16&v16)) & 1) << 14;               // less than (N&~V)|(~N&V)
	flags |= (((n16&~v16)|(~n16&v16)|(z16)) & 1) << 13;         // less than or equal (N&~V)|(~N&V)|Z
	flags |= (((n16&v16&~z16)|(~n16&~v16&~z16)) & 1) << 12;     // greater than (N&V&~Z)|(~N&~V&~Z)
	flags |= ((~z16) & 1) << 11;                                // not equal (~Z)
	flags |= ((z16) & 1) << 10;                                 // equal (Z)
	// 8-bits (bits 0-9)
	flags |= ((~c8) & 1) << 9;                                  // higher than or same (C)
	flags |= ((c8) & 1) << 8;                                   // lower than (~C)
	flags |= ((c8|z8) & 1) << 7;                                // lower than or same (~C|Z)
	flags |= ((~c8&~z8) & 1) << 6;                              // higher than (C&~Z)
	flags |= (((n8&v8)|(~n8&~v8)) & 1) << 5;                    // greater than or equal (N&V)|(~N&~V)
	flags |= (((n8&~v8)|(~n8&v8)) & 1) << 4;                    // less than (N&~V)|(~N&V)
	flags |= (((n8&~v8)|(~n8&v8)|(z8)) & 1) << 3;               // less than or equal (N&~V)|(~N&V)|Z
	flags |= (((n8&v8&~z8)|(~n8&~v8&~z8)) & 1) << 2;            // greater than (N&V&~Z)|(~N&~V&~Z)
	flags |= ((~z8) & 1) << 1;                                  // not equal (~Z)
	flags |= ((z8) & 1) << 0;                                   // equal (Z)

	return flags;
}

void tms32082_mp_device::vector_loadstore()
{
	int rd = OP_RD();
	int vector_ls_bits = (((m_ir >> 9) & 0x3) << 1) | ((m_ir >> 6) & 1);

	switch (vector_ls_bits)
	{
		case 0x01:          // vst.s
		{
			m_program->write_dword(m_outp, m_reg[rd]);
			m_outp += 4;
			break;
		}
		case 0x03:          // vst.d
		{
			UINT64 data = m_fpair[rd >> 1];
			m_program->write_qword(m_outp, data);
			m_outp += 8;
			break;
		}
		case 0x04:          // vld0.s
		{
			m_reg[rd] = m_program->read_dword(m_in0p);
			m_in0p += 4;
			break;
		}
		case 0x05:          // vld1.s
		{
			m_reg[rd] = m_program->read_dword(m_in1p);
			m_in1p += 4;
			break;
		}
		case 0x06:          // vld0.d
		{
			m_fpair[rd >> 1] = m_program->read_qword(m_in0p);
			m_in0p += 8;
			break;
		}
		case 0x07:          // vld1.d
		{
			m_fpair[rd >> 1] = m_program->read_qword(m_in1p);
			m_in1p += 8;
			break;
		}

		default:
			fatalerror("vector_loadstore(): ls bits = %02X\n", vector_ls_bits);
	}
}

void tms32082_mp_device::execute_short_imm()
{
	switch ((m_ir >> 15) & 0x7f)
	{
		case 0x02:          // cmnd
		{
			UINT32 data = OP_UIMM15();

			processor_command(data);
			break;
		}

		case 0x04:          // rdcr
		{
			int rd = OP_RD();
			UINT32 imm = OP_UIMM15();

			UINT32 r = read_creg(imm);

			if (rd)
				m_reg[rd] = r;
			break;
		}

		case 0x05:          // swcr
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			UINT32 r = read_creg(imm);
			if (rd)
				m_reg[rd] = r;

			write_creg(imm, m_reg[rs]);
			break;
		}

		case 0x06:          // brcr
		{
			int cr = OP_UIMM15();

			if (cr == 0x0001)
			{
				// ignore jump to EIP because of how we emulate the pipeline
			}
			else
			{
				UINT32 data = read_creg(cr);

				m_fetchpc = data & ~3;
				m_ie = (m_ie & ~1) | (data & 1);        // global interrupt mask from creg
				// TODO: user/supervisor latch from creg
			}
			break;
		}

		case 0x08:          // shift.dz
		{
			int r = (m_ir & (1 << 10));
			int inv = (m_ir & (1 << 11));
			int rot = OP_ROTATE();
			int end = OP_ENDMASK();
			UINT32 source = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT32 endmask = SHIFT_MASK[end ? end : 32];
			if (inv) endmask = ~endmask;

			UINT32 compmask = endmask;          // shiftmask == 0xffffffff

			UINT32 res = 0;
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0a:          // shift.ds
		{
			int r = (m_ir & (1 << 10));
			int inv = (m_ir & (1 << 11));
			int rot = OP_ROTATE();
			int end = OP_ENDMASK();
			UINT32 source = m_reg[OP_RS()];
			int rd = OP_RD();

			UINT32 endmask = SHIFT_MASK[end ? end : 32];
			if (inv) endmask = ~endmask;

			UINT32 compmask = endmask;          // shiftmask == 0xffffffff

			UINT32 res = 0;
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
				res = SIGN_EXTEND(res, rot);
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
				// sign extend makes no sense to left..
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0b:          // shift.ez
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
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0c:          // shift.em
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
			UINT32 compmask = endmask & shiftmask;

			UINT32 res = 0;
			if (r)      // right
			{
				res = (ROTATE_R(source, rot) & compmask) | (m_reg[rd] & ~compmask);
			}
			else        // left
			{
				res = (ROTATE_L(source, rot) & compmask) | (m_reg[rd] & ~compmask);
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0d:          // shift.es
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
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
				res = SIGN_EXTEND(res, rot);
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
				// sign extend makes no sense to left..
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0e:          // shift.iz
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
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x0f:          // shift.im
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
			if (r)      // right
			{
				res = (ROTATE_R(source, rot) & compmask) | (m_reg[rd] & ~compmask);
			}
			else        // left
			{
				res = (ROTATE_L(source, rot) & compmask) | (m_reg[rd] & ~compmask);
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x11:          // and
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] & imm;
			break;
		}

		case 0x12:          // and.tf
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = ~m_reg[rs] & imm;
			break;
		}

		case 0x14:          // and.ft
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] & ~imm;
			break;
		}

		case 0x17:          // or
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] | imm;
			break;
		}

		case 0x1d:          // or.ft
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 imm = OP_UIMM15();

			if (rd)
				m_reg[rd] = m_reg[rs] | ~imm;
			break;
		}

		case 0x24:
		case 0x20:          // ld.b
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;
			UINT32 data = (UINT8)m_program->read_byte(address);
			if (data & 0x80) data |= 0xffffff00;
			if (rd)
				m_reg[rd] = data;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x25:
		case 0x21:          // ld.h
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;
			UINT32 data = (UINT16)m_program->read_word(address);
			if (data & 0x8000) data |= 0xffff0000;
			if (rd)
				m_reg[rd] = data;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x26:
		case 0x22:          // ld
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;
			UINT32 data = m_program->read_dword(address);
			if (rd)
				m_reg[rd] = data;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x27:
		case 0x23:          // ld.d
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;
			UINT32 data1 = m_program->read_dword(address);
			UINT32 data2 = m_program->read_dword(address+4);
			if (rd)
			{
				m_reg[(rd & ~1)+1] = data1;
				m_reg[(rd & ~1)] = data2;
			}

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x28:
		case 0x2c:          // ld.ub
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;
			UINT32 data = (UINT8)(m_program->read_byte(address));
			if (rd)
				m_reg[rd] = data;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x2d:
		case 0x29:          // ld.uh
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;
			UINT32 data = (UINT16)(m_program->read_word(address));
			if (rd)
				m_reg[rd] = data;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x34:
		case 0x30:          // st.b
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;

			m_program->write_byte(address, (UINT8)(m_reg[rd]));

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x35:
		case 0x31:          // st.h
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;

			m_program->write_word(address, (UINT16)(m_reg[rd]));

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x36:
		case 0x32:          // st
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

		case 0x37:
		case 0x33:          // st.d
		{
			int rd = OP_RD();
			int base = OP_BASE();
			int m = m_ir & (1 << 17);
			INT32 offset = OP_SIMM15();

			UINT32 address = m_reg[base] + offset;

			m_program->write_dword(address+0, m_reg[(rd & ~1) + 1]);
			m_program->write_dword(address+4, m_reg[rd & ~1]);

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x45:          // jsr.a
		{
			int link = OP_LINK();
			int base = OP_BASE();
			INT32 offset = OP_SIMM15();

			if (link)
				m_reg[link] = m_fetchpc;

			m_fetchpc = m_reg[base] + offset;
			break;
		}

		case 0x48:          // bbz
		{
			int bitnum = OP_BITNUM() ^ 0x1f;
			INT32 offset = OP_SIMM15();
			int rs = OP_RS();

			if ((m_reg[rs] & (1 << bitnum)) == 0)
			{
				UINT32 address = m_pc + (offset * 4);

				m_pc = m_fetchpc;
				delay_slot();

				m_fetchpc = address;
			}
			break;
		}

		case 0x49:          // bbz.a
		{
			int bitnum = OP_BITNUM() ^ 0x1f;
			INT32 offset = OP_SIMM15();
			int rs = OP_RS();

			if ((m_reg[rs] & (1 << bitnum)) == 0)
			{
				m_fetchpc = m_pc + (offset * 4);
			}
			break;
		}

		case 0x4a:          // bbo
		{
			int bitnum = OP_BITNUM() ^ 0x1f;
			INT32 offset = OP_SIMM15();
			int rs = OP_RS();

			if ((m_reg[rs] & (1 << bitnum)) != 0)
			{
				UINT32 address = m_pc + (offset * 4);

				m_pc = m_fetchpc;
				delay_slot();

				m_fetchpc = address;
			}
			break;
		}

		case 0x4b:          // bbo.a
		{
			int bitnum = OP_BITNUM() ^ 0x1f;
			INT32 offset = OP_SIMM15();
			int rs = OP_RS();

			if ((m_reg[rs] & (1 << bitnum)) != 0)
			{
				m_fetchpc = m_pc + (offset * 4);
			}
			break;
		}

		case 0x4c:          // bcnd
		{
			INT32 offset = OP_SIMM15();
			int code = OP_RD();
			int rs = OP_RS();

			if (test_condition(code, m_reg[rs]))
			{
				UINT32 address = m_pc + (offset * 4);

				m_pc = m_fetchpc;
				delay_slot();

				m_fetchpc = address;
			}
			break;
		}

		case 0x4d:          // bcnd.a
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

		case 0x50:          // cmp
		{
			UINT32 src1 = OP_SIMM15();
			UINT32 src2 = m_reg[OP_RS()];
			int rd = OP_RD();

			if (rd)
				m_reg[rd] = calculate_cmp(src1, src2);
			break;
		}

		case 0x58:          // add
		{
			INT32 imm = OP_SIMM15();
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] + imm;

			// TODO: integer overflow exception
			break;
		}

		case 0x59:          // addu
		{
			INT32 imm = OP_SIMM15();
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] + imm;
			break;
		}

		case 0x5a:          // sub
		{
			INT32 imm = OP_SIMM15();
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = imm - m_reg[rs];

			// TODO: integer overflow exception
			break;
		}

		case 0x5b:          // subu
		{
			INT32 imm = OP_SIMM15();
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = imm - m_reg[rs];
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
		case 0x04:          // cmnd
		{
			UINT32 data = has_imm ? imm32 : m_reg[OP_SRC1()];

			processor_command(data);
			break;
		}

		case 0x16:          // shift.ez
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
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x1a:          // shift.es
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
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
				res = SIGN_EXTEND(res, rot);
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x1c:          // shift.iz
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
			UINT32 compmask = endmask & ~shiftmask;

			UINT32 res = 0;
			if (r)      // right
			{
				res = ROTATE_R(source, rot) & compmask;
			}
			else        // left
			{
				res = ROTATE_L(source, rot) & compmask;
			}

			if (rd)
				m_reg[rd] = res;
			break;
		}

		case 0x22:
		case 0x23:          // and
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 src1 = has_imm ? imm32 : m_reg[OP_SRC1()];

			if (rd)
				m_reg[rd] = src1 & m_reg[rs];
			break;
		}

		case 0x24:
		case 0x25:          // and.tf
		{
			int rd = OP_RD();
			int rs = OP_RS();
			UINT32 src1 = has_imm ? imm32 : m_reg[OP_SRC1()];

			if (rd)
				m_reg[rd] = src1 & ~(m_reg[rs]);
			break;
		}

		case 0x2c:
		case 0x2d:          // xor
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] ^ (has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		case 0x2e:
		case 0x2f:          // or
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] | (has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		case 0x3a:
		case 0x3b:          // or.ft
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] | ~(has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		case 0x40:
		case 0x41:
		case 0x48:
		case 0x49:          // ld.b
		{
			int m = m_ir & (1 << 15);

			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
			UINT32 r = m_program->read_byte(address);
			if (r & 0x80) r |= 0xffffff00;

			if (rd)
				m_reg[rd] = r;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x42:
		case 0x4a:
		case 0x43:
		case 0x4b:          // ld.h
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

		case 0x4c:
		case 0x44:
		case 0x4d:
		case 0x45:          // ld
		{
			int shift = (m_ir & (1 << 11)) ? 2 : 0;
			int m = m_ir & (1 << 15);
			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + ((has_imm ? imm32 : m_reg[OP_SRC1()]) << shift);
			UINT32 r = m_program->read_dword(address);

			if (rd)
				m_reg[rd] = r;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x4e:
		case 0x4f:
		case 0x46:
		case 0x47:          // ld.d
		{
			int shift = (m_ir & (1 << 11)) ? 3 : 0;
			int m = m_ir & (1 << 15);
			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + ((has_imm ? imm32 : m_reg[OP_SRC1()]) << shift);
			UINT64 r = m_program->read_qword(address);

			if (rd)
				m_fpair[rd >> 1] = r;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x58:
		case 0x59:
		case 0x50:
		case 0x51:          // ld.ub
		{
			int m = m_ir & (1 << 15);
			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
			UINT32 r = (UINT8)(m_program->read_byte(address));

			if (rd)
				m_reg[rd] = r;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x5a:
		case 0x5b:
		case 0x52:
		case 0x53:          // ld.uh
		{
			int shift = (m_ir & (1 << 11)) ? 1 : 0;
			int m = m_ir & (1 << 15);
			int base = OP_BASE();
			int rd = OP_RD();

			UINT32 address = m_reg[base] + ((has_imm ? imm32 : m_reg[OP_SRC1()]) << shift);
			UINT32 r = (UINT16)(m_program->read_word(address));

			if (rd)
				m_reg[rd] = r;

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x60:
		case 0x61:
		case 0x68:
		case 0x69:          // st.b
		{
			int m = m_ir & (1 << 15);

			int base = OP_BASE();

			UINT32 address = m_reg[base] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
			m_program->write_byte(address, (UINT8)(m_reg[OP_RD()]));

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x62:
		case 0x63:
		case 0x6a:
		case 0x6b:          // st.h
		{
			int shift = (m_ir & (1 << 11)) ? 1 : 0;
			int m = m_ir & (1 << 15);

			int base = OP_BASE();

			UINT32 address = m_reg[base] + ((has_imm ? imm32 : m_reg[OP_SRC1()]) << shift);
			m_program->write_word(address, (UINT16)(m_reg[OP_RD()]));

			if (m && base)
				m_reg[base] = address;
			break;
		}

		case 0x6c:
		case 0x6d:
		case 0x64:
		case 0x65:          // st
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
		case 0x89:          // jsr
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
		case 0x8b:          // jsr.a
		{
			int link = OP_LINK();
			int base = OP_BASE();

			if (link)
				m_reg[link] = m_fetchpc;

			m_fetchpc = m_reg[base] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		case 0xa0:
		case 0xa1:          // cmp
		{
			int rd = OP_RD();
			UINT32 src1 = has_imm ? imm32 : m_reg[OP_SRC1()];
			UINT32 src2 = m_reg[OP_RS()];

			if (rd)
				m_reg[rd] = calculate_cmp(src1, src2);
			break;
		}

		case 0xb2:
		case 0xb3:          // addu
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = m_reg[rs] + (has_imm ? imm32 : m_reg[OP_SRC1()]);
			break;
		}

		case 0xb4:
		case 0xb5:          // sub
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = (has_imm ? imm32 : m_reg[OP_SRC1()]) - m_reg[rs];

			// TODO: overflow interrupt
			break;
		}

		case 0xb6:
		case 0xb7:          // subu
		{
			int rd = OP_RD();
			int rs = OP_RS();

			if (rd)
				m_reg[rd] = (has_imm ? imm32 : m_reg[OP_SRC1()]) - m_reg[rs];
			break;
		}

		case 0xc4:
		case 0xd4:
		case 0xc5:
		case 0xd5:          // vmpy
		{
			int p1 = m_ir & (1 << 5);
			int pd = m_ir & (1 << 7);
			int ls_bit1 = m_ir & (1 << 10);
			int ls_bit2 = m_ir & (1 << 6);
			int rd = OP_RS();
			int src1 OP_SRC1();

			double source = has_imm ? (double)u2f(imm32) : (p1 ? u2d(m_fpair[src1 >> 1]) : (double)u2f(m_reg[src1]));

			if (rd)
			{
				if (pd)
				{
					double res = source * u2d(m_fpair[rd >> 1]);
					m_fpair[rd >> 1] = d2u(res);
				}
				else
				{
					float res = (float)(source) * u2f(m_reg[rd]);
					m_reg[rd] = f2u(res);
				}
			}

			// parallel load/store op
			if (!(ls_bit1 == 0 && ls_bit2 == 0))
			{
				vector_loadstore();
			}
			break;
		}

		case 0xc8:
		case 0xd8:
		case 0xc9:
		case 0xd9:          // vrnd
		{
			int acc = OP_ACC();
			int p1 = m_ir & (1 << 5);
			int pd = (m_ir >> 7) & 3;
			int ls_bit1 = m_ir & (1 << 10);
			int ls_bit2 = m_ir & (1 << 6);
			int rd = OP_RS();
			int rs1 = OP_SRC1();

			double source = has_imm ? (double)u2f(imm32) : (p1 ? u2d(m_fpair[rs1 >> 1]) : (double)u2f(m_reg[rs1]));

			if (rd)
			{
				// destination register
				switch (pd)
				{
					case 0:
						m_reg[rd] = f2u((float)source);
						break;
					case 1:
						m_fpair[rd >> 1] = d2u(source);
						break;
					case 2:
						m_reg[rd] = (INT32)(source);
						break;
					case 3:
						m_reg[rd] = (UINT32)(source);
						break;
				}
			}
			else
			{
				// destination accumulator
				if (pd != 1)
					fatalerror("vrnd pd = %d at %08X\n", pd, m_pc);

				m_facc[acc] = source;
			}

			// parallel load/store op
			if (!(ls_bit1 == 0 && ls_bit2 == 0))
			{
				vector_loadstore();
			}
			break;
		}

		case 0xcc:
		case 0xdc:
		case 0xcd:
		case 0xdd:          // vmac
		{
			int acc = OP_ACC();
			int z = m_ir & (1 << 8);
			int pd = m_ir & (1 << 9);
			int ls_bit1 = m_ir & (1 << 10);
			int ls_bit2 = m_ir & (1 << 6);
			int rd = OP_RD();

			float src1 = u2f(m_reg[OP_SRC1()]);
			float src2 = u2f(m_reg[OP_RS()]);

			float res = (src1 * src2) + (z ? 0.0f : m_acc[acc]);

			// parallel load/store op
			if (!(ls_bit1 == 0 && ls_bit2 == 0))
			{
				vector_loadstore();

				// if the opcode has load/store, dest is always accumulator
				m_facc[acc] = (double)res;
			}
			else
			{
				if (rd)
				{
					if (pd)
						m_fpair[rd >> 1] = d2u(res);
					else
						m_reg[rd] = f2u((float)res);
				}
				else
				{
					// write to accumulator
					m_facc[acc] = (double)res;
				}
			}
			break;
		}

		case 0xce:
		case 0xde:
		case 0xcf:
		case 0xdf:          // vmsc
		{
			int acc = OP_ACC();
			int z = m_ir & (1 << 8);
			int pd = m_ir & (1 << 9);
			int ls_bit1 = m_ir & (1 << 10);
			int ls_bit2 = m_ir & (1 << 6);
			int rd = OP_RD();

			float src1 = u2f(m_reg[OP_SRC1()]);
			float src2 = u2f(m_reg[OP_RS()]);

			float res = (z ? 0.0f : m_acc[acc]) - (src1 * src2);

			// parallel load/store op
			if (!(ls_bit1 == 0 && ls_bit2 == 0))
			{
				vector_loadstore();

				// if the opcode has load/store, dest is always accumulator
				m_facc[acc] = (double)res;
			}
			else
			{
				if (rd)
				{
					if (pd)
						m_fpair[rd >> 1] = d2u(res);
					else
						m_reg[rd] = f2u((float)res);
				}
				else
				{
					// write to accumulator
					m_facc[acc] = (double)res;
				}
			}
			break;
		}

		case 0xe0:
		case 0xe1:          // fadd
		{
			int rd = OP_RD();
			int rs = OP_RS();
			int src1 = OP_SRC1();
			int precision = (m_ir >> 5) & 0x3f;

			if (rd)     // only calculate if destination register is valid
			{
				switch (precision)
				{
					case 0x00:          // SP - SP -> SP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						float s2 = u2f(m_reg[rs]);
						m_reg[rd] = f2u(s1 + s2);
						break;
					}
					case 0x10:          // SP - SP -> DP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						float s2 = u2f(m_reg[rs]);
						UINT64 res = d2u((double)(s1 + s2));
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x14:          // SP - DP -> DP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						double s2 = u2d(m_fpair[rs >> 1]);
						UINT64 res = d2u((double) s1 + s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x11:          // DP - SP -> DP
					{
						double s1 = u2d(m_fpair[src1 >> 1]);
						float s2 = u2f(m_reg[rs]);
						UINT64 res = d2u(s1 + (double) s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x15:          // DP - DP -> DP
					{
						double s1 = u2d(m_fpair[src1 >> 1]);
						double s2 = u2d(m_fpair[rs >> 1]);
						UINT64 res = d2u((double)(s1 + s2));
						m_fpair[rd >> 1] = res;
						break;
					}
					default:
						fatalerror("fadd: invalid precision combination %02X\n", precision);
				}
			}
			break;
		}

		case 0xe2:
		case 0xe3:          // fsub
		{
			int rd = OP_RD();
			int rs = OP_RS();
			int src1 = OP_SRC1();
			int precision = (m_ir >> 5) & 0x3f;

			if (rd)     // only calculate if destination register is valid
			{
				switch (precision)
				{
					case 0x00:          // SP - SP -> SP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						float s2 = u2f(m_reg[rs]);
						m_reg[rd] = f2u(s1 - s2);
						break;
					}
					case 0x10:          // SP - SP -> DP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						float s2 = u2f(m_reg[rs]);
						UINT64 res = d2u((double)(s1 - s2));
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x14:          // SP - DP -> DP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						double s2 = u2d(m_fpair[rs >> 1]);
						UINT64 res = d2u((double) s1 - s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x11:          // DP - SP -> DP
					{
						double s1 = u2d(m_fpair[src1 >> 1]);
						float s2 = u2f(m_reg[rs]);
						UINT64 res = d2u(s1 - (double) s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x15:          // DP - DP -> DP
					{
						double s1 = u2d(m_fpair[src1 >> 1]);
						double s2 = u2d(m_fpair[rs >> 1]);
						UINT64 res = d2u((double)(s1 - s2));
						m_fpair[rd >> 1] = res;
						break;
					}
					default:
						fatalerror("fsub: invalid precision combination %02X\n", precision);
				}
			}
			break;
		}

		case 0xe4:
		case 0xe5:          // fmpy
		{
			int rd = OP_RD();
			int rs = OP_RS();
			int src1 = OP_SRC1();
			int precision = (m_ir >> 5) & 0x3f;

			if (rd)     // only calculate if destination register is valid
			{
				switch (precision)
				{
					case 0x00:          // SP x SP -> SP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						float s2 = u2f(m_reg[rs]);
						m_reg[rd] = f2u(s1 * s2);
						break;
					}
					case 0x10:          // SP x SP -> DP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						float s2 = u2f(m_reg[rs]);
						UINT64 res = d2u((double)(s1 * s2));
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x14:          // SP x DP -> DP
					{
						float s1 = u2f(has_imm ? imm32 : m_reg[src1]);
						double s2 = u2d(m_fpair[rs >> 1]);
						UINT64 res = d2u((double)s1 * s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x11:          // DP x SP -> DP
					{
						double s1 = u2d(m_fpair[src1 >> 1]);
						float s2 = u2f(m_reg[rs]);
						UINT64 res = d2u(s1 * (double) s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x15:          // DP x DP -> DP
					{
						double s1 = u2d(m_fpair[src1 >> 1]);
						double s2 = u2d(m_fpair[rs >> 1]);
						UINT64 res = d2u(s1 * s2);
						m_fpair[rd >> 1] = res;
						break;
					}
					case 0x2a:          // I x I -> I
					{
						m_reg[rd] = (INT32)(m_reg[rs]) * (INT32)(has_imm ? imm32 : m_reg[OP_SRC1()]);
						break;
					}
					case 0x3f:          // U x U -> U
					{
						m_reg[rd] = (UINT32)(m_reg[rs]) * (UINT32)(has_imm ? imm32 : m_reg[OP_SRC1()]);
						break;
					}
					default:
						fatalerror("fmpy: invalid precision combination %02X\n", precision);
				}
			}
			break;
		}

		case 0xe6:
		case 0xe7:          // fdiv
		{
			int rd = OP_RD();
			int p1 = m_ir & (1 << 5);
			int p2 = m_ir & (1 << 7);
			int pd = m_ir & (1 << 9);
			int rs1 = OP_SRC1();
			int rs2 = OP_RS();

			if (rd)
			{
				double src1 = has_imm ? (double)u2f(imm32) : (p1 ? u2d(m_fpair[rs1 >> 1]) : (double)u2f(m_reg[rs1]));
				double src2 = p2 ? u2d(m_fpair[rs2 >> 1]) : (double)u2f(m_reg[rs2]);

				double res = src1 / src2;

				if (pd)
					m_fpair[rd >> 1] = d2u(res);
				else
					m_reg[rd] = f2u((float)res);
			}
			break;
		}

		case 0xe8:
		case 0xe9:          // frnd
		{
			//int mode = (m_ir >> 7) & 3;
			int p1 = (m_ir >> 5) & 3;
			int pd = (m_ir >> 9) & 3;
			int src1 = OP_SRC1();
			int rd = OP_RD();

			double s = 0.0;

			switch (p1)
			{
				case 0:
					s = has_imm ? (double)(u2f(imm32)) : (double)u2f(m_reg[src1]);
					break;
				case 1:
					s = u2d(m_fpair[src1 >> 1]);
					break;
				case 2:
					s = has_imm ? (double)((INT32)(imm32)) : (double)(INT32)(m_reg[src1]);
					break;
				case 3:
					s = has_imm ? (double)((UINT32)(imm32)) : (double)(UINT32)(m_reg[src1]);
					break;
			}

			// TODO: round

			if (rd)
			{
				switch (pd)
				{
					case 0:
						m_reg[rd] = f2u((float)(s));
						break;
					case 1:
						m_fpair[rd] = d2u(s);
						break;
					case 2:
						m_reg[rd] = (INT32)(s);
						break;
					case 3:
						m_reg[rd] = (UINT32)(s);
						break;
				}
			}
			break;
		}

		case 0xea:
		case 0xeb:          // fcmp
		{
			int rd = OP_RD();
			int p1 = m_ir & (1 << 5);
			int p2 = m_ir & (1 << 7);
			int rs1 = OP_SRC1();
			int rs2 = OP_RS();

			double src1 = has_imm ? (double)(u2f(imm32)) : (p1 ? u2d(m_fpair[rs1 >> 1]) : (double)u2f(m_reg[rs1]));
			double src2 = p2 ? u2d(m_fpair[rs2 >> 1]) : (double)u2f(m_reg[rs2]);

			if (rd)
			{
				UINT32 flags = 0;
				flags |= (src1 == src2) ? (1 << 20) : 0;
				flags |= (src1 != src2) ? (1 << 21) : 0;
				flags |= (src1 >  src2) ? (1 << 22) : 0;
				flags |= (src1 <= src2) ? (1 << 23) : 0;
				flags |= (src1 <  src2) ? (1 << 24) : 0;
				flags |= (src1 >= src2) ? (1 << 25) : 0;
				flags |= (src1 < 0 || src1 > src2) ? (1 << 26) : 0;
				flags |= (src1 > 0 && src1 < src2) ? (1 << 27) : 0;
				flags |= (src1 >= 0 && src1 <= src2) ? (1 << 28) : 0;
				flags |= (src1 <= 0 || src1 >= src2) ? (1 << 29) : 0;
				// TODO: src1 or src2 unordered
				// TODO: src1 and src2 ordered

				m_reg[rd] = flags;
			}
			break;
		}

		case 0xee:
		case 0xef:          // fsqrt
		{
			int rd = OP_RD();
			int src1 = OP_SRC1();
			int p1 = m_ir & (1 << 5);
			int pd = m_ir & (1 << 9);
			double source = has_imm ? (double)u2f(imm32) : (p1 ? u2d(m_fpair[src1 >> 1]) : (double)u2f(m_reg[src1]));

			if (rd)
			{
				double res = sqrt(source);

				if (pd)
					m_fpair[rd >> 1] = d2u(res);
				else
					m_reg[rd] = f2u((float)res);
			}
			break;
		}

		case 0xf2:          // rmo
		{
			UINT32 source = m_reg[OP_RS()];
			int rd = OP_RD();

			int bit = 32;

			for (int i=0; i < 32; i++)
			{
				if (source & (1 << (31-i)))
				{
					bit = i;
					break;
				}
			}

			if (rd)
				m_reg[rd] = bit;
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
