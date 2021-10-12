// license:BSD-3-Clause
// copyright-holders:Ville Linde
// TMS32082 PP Disassembler

#include "emu.h"
#include "dis_pp.h"

#define ROTATE_L(x, r) ((x << r) | (x >> (32-r)))

std::string tms32082_pp_disassembler::get_reg_name(int reg, bool read)
{
	switch (reg)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
			return util::string_format("a%d", reg & 0xf);

		case 0x07:
		case 0x0f:
			return util::string_format("0");		// a7 and a15 read as zero

		case 0x06:
		case 0x0e:
			return util::string_format("sp");

		case 0x10:
		case 0x11:
		case 0x12:
		case 0x18:
		case 0x19:
		case 0x1a:
			return util::string_format("x%d", reg & 0xf);

		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			return util::string_format("d%d", reg & 0x7);

		case 0x29: return util::string_format("sr");
		case 0x2a: return util::string_format("mf");
		case 0x38: return read ? util::string_format("pc") : util::string_format("call");
		case 0x39: return read ? util::string_format("ipa") : util::string_format("br");
		case 0x3a: return util::string_format("ipe");
		case 0x3b: return util::string_format("iprs");
		case 0x3c: return util::string_format("inten");
		case 0x3d: return util::string_format("intflg");
		case 0x3e: return util::string_format("comm");
		case 0x3f: return util::string_format("lctl");
		case 0x60: return util::string_format("lc0");
		case 0x61: return util::string_format("lc1");
		case 0x62: return util::string_format("lc2");
		case 0x64: return util::string_format("lr0");
		case 0x65: return util::string_format("lr1");
		case 0x66: return util::string_format("lr2");
		case 0x68: return util::string_format("lrse0");
		case 0x69: return util::string_format("lrse1");
		case 0x6a: return util::string_format("lrse2");
		case 0x6c: return util::string_format("lrs0");
		case 0x6d: return util::string_format("lrs1");
		case 0x6e: return util::string_format("lrs2");
		case 0x70: return util::string_format("ls0");
		case 0x71: return util::string_format("ls1");
		case 0x72: return util::string_format("ls2");
		case 0x74: return util::string_format("le0");
		case 0x75: return util::string_format("le1");
		case 0x76: return util::string_format("le2");
		case 0x7c: return util::string_format("tag0");
		case 0x7d: return util::string_format("tag1");
		case 0x7e: return util::string_format("tag2");
		case 0x7f: return util::string_format("tag3");
		default: return util::string_format("???");
	}
}

char const *const tms32082_pp_disassembler::CONDITION_CODES[16] =
{
	"u",     "p",     "ls",    "hi",
	"lt",    "le",    "ge",    "gt",
	"hs",    "lo",    "eq",    "ne",
	"v",     "nv",    "n",     "nn"
};

char const *const tms32082_pp_disassembler::TRANSFER_SIZE[4] =
{
	"b:", "h:", "w:", ""
};

std::string tms32082_pp_disassembler::make_ea(int mode, int areg, bool scale, int size, int offset, int xreg)
{
	std::string offset_text;
	if (scale)
	{
		if (size == 1)
			offset *= 2;
		else if (size == 2)
			offset *= 4;
	}

	offset_text = util::string_format("0x%X", offset);

	switch (mode)
	{
		case 0x4: return util::string_format("%s++=x%d", get_reg_name(areg, true), xreg);
		case 0x5: return util::string_format("%s--=x%d", get_reg_name(areg, true), xreg);
		case 0x6: return util::string_format("%s++%s", get_reg_name(areg, true), offset_text);
		case 0x7: return util::string_format("%s--%s", get_reg_name(areg, true), offset_text);
		case 0x8: return util::string_format("%s+x%d", get_reg_name(areg, true), xreg);
		case 0x9: return util::string_format("%s-x%d", get_reg_name(areg, true), xreg);
		case 0xa: return util::string_format("%s+%s", get_reg_name(areg, true), offset_text);
		case 0xb: return util::string_format("%s-%s", get_reg_name(areg, true), offset_text);
		case 0xc: return util::string_format("%s+=x%d", get_reg_name(areg, true), xreg);
		case 0xd: return util::string_format("%s-=x%d", get_reg_name(areg, true), xreg);
		case 0xe: return util::string_format("%s+=%s", get_reg_name(areg, true), offset_text);
		case 0xf: return util::string_format("%s-=%s", get_reg_name(areg, true), offset_text);
	}

	return "";
}

std::string tms32082_pp_disassembler::make_mem_transfer(int mode, int dst, int a, bool scale, int size, int le, int imm, int x)
{
	std::string transfer_text;

	switch (le)
	{
		case 0:		// store
			transfer_text = util::string_format("&%s*(%s) = %s", TRANSFER_SIZE[size], make_ea(mode, a, scale, size, imm, x), get_reg_name(dst, true));
			break;
		case 1:		// address unit arithmetic
			transfer_text = util::string_format("%s = %s", get_reg_name(dst, false), make_ea(mode, a, scale, size, imm, x));
			break;
		case 2:		// zero-extended load
			transfer_text = util::string_format("%s = &%s*(%s)", get_reg_name(dst, false), TRANSFER_SIZE[size], make_ea(mode, a, scale, size, imm, x));
			break;
		case 3:		// sign-extended load
			transfer_text = util::string_format("%s = &s%s*(%s)", get_reg_name(dst, false), TRANSFER_SIZE[size], make_ea(mode, a, scale, size, imm, x));
			break;
	}

	return transfer_text;
}

std::string tms32082_pp_disassembler::make_condition(int cond, int ncvz)
{
	std::string condition;

	if (cond || ncvz)
	{
		condition.append("[");
	}
	if (cond)
	{
		condition.append(util::string_format("%s", CONDITION_CODES[cond]));
	}
	if (ncvz)
	{
		condition.append(".");
		if (ncvz & 8)
			condition.append("n");
		if (ncvz & 4)
			condition.append("c");
		if (ncvz & 2)
			condition.append("v");
		if (ncvz & 1)
			condition.append("z");
	}
	if (cond || ncvz)
	{
		condition.append("]");
	}
	return condition;
}


std::string tms32082_pp_disassembler::make_field_move(bool d, bool e, int size, int itm)
{
	std::string field;
	if (d)
	{
		field.append("r");
	}
	else
	{
		field.append(util::string_format("%s", e ? "s" : ""));
	}

	switch (size)
	{
		case 0: field.append("b"); break;
		case 1: field.append("w"); break;
	}

	if (!d)
	{
		field.append(util::string_format("%d", itm));
	}

	return field;
}


void tms32082_pp_disassembler::parallel_transfer(uint64_t op)
{
	int lbits = (op >> 37) & 3;
	int gbits = (op >> 15) & 3;

	if (lbits == 0 && gbits == 0)
	{		
		switch ((op >> 13) & 3)
		{
			case 0:
			{
				// 7. Conditional DU||Conditional Move
				int cond = (op >> 32) & 0xf;				
				int ncvz = (op >> 25) & 0xf;

				m_alu_condition = make_condition(cond, ncvz);

				m_src1bank = (op >> 6) & 0xf;
				m_dstbank = (op >> 18) & 0xf;

				int dst = ((op >> 3) & 7) | (m_dstbank << 3);
				int src = ((op >> 10) & 7) | (m_src1bank << 3);

				bool r = (op & (1 << 29)) ? true : false;

				if (r)
					m_parallel_condition = util::string_format("%s", make_condition(cond, ncvz));

				m_parallel_transfer = util::string_format("%s = %s", get_reg_name(dst, false), get_reg_name(src, true));

				break;
			}
			case 1:
			{
				// 8. Conditional DU||Conditional Field Move
				m_parallel_transfer = util::string_format("cond du||cond field move");

				m_dstbank = (op >> 18) & 0xf;

				int itm = (op >> 22) & 3;
				bool e = (op & (1 << 9)) ? true : false;
				bool d = (op & (1 << 6)) ? true : false;
				int size = (op >> 7) & 3;
				int dst = ((op >> 3) & 7) | (m_dstbank << 3);
				int src = ((op >> 10) & 7) | (4 << 3);

				int cond = (op >> 32) & 0xf;				
				int ncvz = (op >> 25) & 0xf;

				bool r = (op & (1 << 29)) ? true : false;

				if (r)
					m_parallel_condition = util::string_format("%s", make_condition(cond, ncvz));

				m_alu_condition = make_condition(cond, ncvz);

				m_parallel_transfer = util::string_format("%s = %s %s", get_reg_name(dst, false), make_field_move(d, e, size, itm), get_reg_name(src, true));
				break;
			}
			case 2:
			case 3:
			{
				// 10. Conditional Non-D Data Unit
			
				// no transfer, only modifies ALU operation
				m_src1bank = (op >> 6) & 0xf;
				m_dstbank = (op >> 18) & 0xf;

				int cond = (op >> 32) & 0xf;
		//		bool c = (op & (1 << 31)) ? true : false;
		//		bool r = (op & (1 << 30)) ? true : false;
				int ncvz = (op >> 25) & 0xf;

				m_alu_condition = make_condition(cond, ncvz);
				break;
			}
		}

	}
	else if (lbits == 0 && gbits != 0)
	{		
		if (op & 0x4)
		{
			// 9. Conditional DU||Conditional Global
			int global_im = (op >> 22) & 7;
			int global_x = ((op >> 22) & 7) | 8;		// global x-registers: x8-x10
			int bank = (op >> 18) & 7;
			int global_le = ((op >> 9) & 1) | ((op >> 16) & 2);
			int gmode = (op >> 13) & 0xf;
			int reg = (op >> 10) & 7;
			int global_size = (op >> 7) & 3;
			bool global_s = (op & (1 << 6)) ? true : false;
			int global_a = ((op >> 3) & 7) | 8;		// global a-registers: a8-a12
			int cond = (op >> 32) & 0xf;			
			int ncvz = (op >> 25) & 0xf;

			m_alu_condition = make_condition(cond, ncvz);

			bool r = (op & (1 << 29)) ? true : false;

			if (r)
				m_parallel_condition = util::string_format("%s", make_condition(cond, ncvz));

			m_parallel_transfer = make_mem_transfer(gmode, (bank << 3) | reg, global_a, global_s, global_size, global_le, global_im, global_x);
		}
		else
		{
			// 5. Global(Long Offset)			
			int bank = (op >> 18) & 0xf;
			int global_le = ((op >> 9) & 1) | ((op >> 16) & 2);
			int gmode = (op >> 13) & 0xf;
			int reg = (op >> 10) & 7;
			int global_size = (op >> 7) & 3;
			bool global_s = (op & (1 << 6)) ? true : false;
			int global_a = ((op >> 3) & 7) | 8;		// global a-registers: a8-a12

			int offset = (op >> 22) & 0x7fff;
			int x = (offset & 7) | 8;		// global x-registers: x8-x10

			// s-bit is MSB for immediate value when transfer size is 8 bits
			if (global_size == 0)
				offset |= global_s ? 0x8000 : 0;

			m_parallel_transfer.append(make_mem_transfer(gmode, (bank << 3) | reg, global_a, global_s, global_size, global_le, offset, x));
		}
	}
	else if (lbits != 0 && gbits == 0)
	{
		switch (((op >> 22) & 4) | ((op >> 13) & 3))
		{
			case 0:
			{
				// 2. Move||Local
				int lmode = (op >> 35) & 0xf;
				int d = ((op >> 32) & 7) | (0x4 << 3);		// d-registers are in bank 4
				int local_le = ((op >> 31) & 1) | ((op >> 16) & 2);
				int local_size = (op >> 29) & 3;
				bool local_s = (op & (1 << 28)) ? true : false;
				int local_a = (op >> 25) & 7;
				int local_im = op & 7;
				int local_x = op & 7;

				if (local_size == 0)
					local_im |= local_s ? 0x8 : 0;

				int srcbank = (op >> 6) & 0xf;
				int dstbank = (op >> 18) & 0xf;
				int src = ((op >> 10) & 7) | (srcbank << 3);
				int dst = ((op >> 3) & 7) | (dstbank << 3);

				m_parallel_transfer = make_mem_transfer(lmode, d, local_a, local_s, local_size, local_le, local_im, local_x);
				m_parallel_transfer.append(" || ");
				m_parallel_transfer.append(util::string_format("%s = %s", get_reg_name(dst, false), get_reg_name(src, true)));
				break;
			}

			case 1:
			{
				// 3. Field Move||Local
				// TODO: not used by any program so far
				m_parallel_transfer = util::string_format("field move || local");
				break;
			}

			case 2:
			case 3:
			{
				// 6. Non-D DU||Local
				int lmode = (op >> 35) & 0xf;
				int d = ((op >> 32) & 7) | (0x4 << 3);		// d-registers are in bank 4
				int le = ((op >> 31) & 1) | ((op >> 16) & 2);
				int size = (op >> 29) & 3;
				bool s = (op & (1 << 28)) ? true : false;
				int a = (op >> 25) & 7;
				int im = (op >> 22) & 7;
				int x = ((op >> 22) & 7);

				m_dstbank = (op >> 18) & 0xf;
				m_src1bank = (op >> 6) & 0xf;

				m_parallel_transfer = make_mem_transfer(lmode, d, a, s, size, le, im, x);
				break;
			}

			case 4:
			case 5:
			case 6:
			case 7:
			{
				// 4. Local (Long Offset)
				int lmode = (op >> 35) & 0xf;
				int d = (op >> 32) & 7;
				int local_le = ((op >> 31) & 1) | ((op >> 16) & 2);
				int local_size = (op >> 29) & 3;
				bool local_s = (op & (1 << 28)) ? true : false;
				int local_a = (op >> 25) & 7;
				int bank = (op >> 18) & 0xf;
				int offset = op & 0x7fff;

				// s-bit is MSB for immediate value when transfer size is 8 bits
				if (local_size == 0)
					offset |= local_s ? 0x8000 : 0;

				m_parallel_transfer.append(make_mem_transfer(lmode, (bank << 3) | d, local_a, local_s, local_size, local_le, offset, offset & 7));
				break;
			}
		}
	}
	else
	{
		// (gbits != 0 && lbits != 0)
		// 1. Double Parallel
		int lmode = (op >> 35) & 0xf;
		int d = ((op >> 32) & 7) | (0x4 << 3);		// d-registers are in bank 4
		int local_le = ((op >> 31) & 1) | ((op >> 20) & 2);
		int local_size = (op >> 29) & 3;
		bool local_s = (op & (1 << 28)) ? true : false;
		int local_a = (op >> 25) & 7;
		int global_im = (op >> 22) & 7;		
		int global_x = ((op >> 22) & 7) | 8;		// global x-registers: x8-x10
		int bank = (op >> 18) & 7;
		int global_le = ((op >> 9) & 1) | ((op >> 16) & 2);		
		int gmode = (op >> 13) & 0xf;
		int reg = (op >> 10) & 7;		
		int global_size = (op >> 7) & 3;
		bool global_s = (op & (1 << 6)) ? true : false;
		int global_a = ((op >> 3) & 7) | 8;		// global a-registers: a8-a12
		int local_im = op & 7;
		int local_x = op & 7;

		// s-bit is MSB for immediate value when transfer size is 8 bits
		if (global_size == 0)
			global_im |= local_s ? 0x8 : 0;
		if (local_size == 0)
			local_im |= local_s ? 0x8 : 0;

		m_parallel_transfer.append(make_mem_transfer(lmode, d, local_a, local_s, local_size, local_le, local_im, local_x));
		m_parallel_transfer.append(" || ");
		m_parallel_transfer.append(make_mem_transfer(gmode, (bank << 3) | reg, global_a, global_s, global_size, global_le, global_im, global_x));
	}
}

std::string tms32082_pp_disassembler::format_alu_op(int aluop, int a, const std::string& dst_text, std::string& a_text, std::string& b_text, std::string& c_text)
{
	if (a)      // arithmetic
	{
		int modifier = ((aluop >> 1) & 1) | ((aluop >> 2) & 2) | ((aluop >> 3) & 4) | ((aluop >> 4) & 8);

		switch (modifier)
		{
			case 0:		break;											// normal operation
			case 1:		break;											// cin (TODO)
			case 4:		a_text = util::string_format("0"); break;		// A port = 0
			case 5:		a_text = util::string_format("0"); break;		// A port = 0, cin (TODO)
			case 6:		a_text = util::string_format("0"); break;		// A port = 0 and %! if maskgen instruction. lmbc if not maskgen instruction. (TODO)
			case 7:		a_text = util::string_format("0"); break;		// A port = 0, %!, and cin if maskgen instruction. rmbc if not maskgen instruction. (TODO)
		}

		int bits = (aluop & 1) | ((aluop >> 1) & 2) | ((aluop >> 2) & 4) | ((aluop >> 3) & 8);
		switch (bits)
		{
			case 1:     return util::string_format("%s = %s - %s<1<", dst_text, a_text, b_text); break;
			case 2:     return util::string_format("%s = %s + %s<0<", dst_text, a_text, b_text); break;
			case 3:     return util::string_format("%s = %s - %s", dst_text, a_text, c_text); break;
			case 4:     return util::string_format("%s = %s - %s>1>", dst_text, a_text, b_text); break;
			case 5:     return util::string_format("%s = %s - %s", dst_text, a_text, b_text); break;
			case 6:     return util::string_format("?"); break;
			case 7:     return util::string_format("%s = %s - %s>0>", dst_text, a_text, b_text); break;
			case 8:     return util::string_format("%s = %s + %s>0>", dst_text, a_text, b_text); break;
			case 9:     return util::string_format("?"); break;
			case 10:    return util::string_format("%s = %s + %s", dst_text, a_text, b_text); break;
			case 11:    return util::string_format("%s = %s + %s>1>", dst_text, a_text, b_text); break;
			case 12:    return util::string_format("%s = %s + %s", dst_text, a_text, c_text); break;
			case 13:    return util::string_format("%s = %s - %s<0<", dst_text, a_text, b_text); break;
			case 14:    return util::string_format("%s = %s + %s<1<", dst_text, a_text, b_text); break;
			case 15:    return util::string_format("%s = field %s + %s", dst_text, a_text, b_text); break;
		}
	}
	else        // boolean
	{
		switch (aluop)
		{
			case 0xaa:      // A & B & C | A & ~B & C | A & B & ~C | A & ~B & ~C       = A
				return util::string_format("%s = %s", dst_text, a_text);

			case 0x55:      // ~A & B & C | ~A & ~B & C | ~A & B & ~C | ~A & ~B & ~C   = ~A
				return util::string_format("%s = ~%s", dst_text, a_text);

			case 0xcc:      // A & B & C | ~A & B & C | A & B & ~C | ~A & B & ~C       = B
				return util::string_format("%s = %s", dst_text, b_text);

			case 0x33:      // A & ~B & C | ~A & ~B & C | A & ~B & ~C | ~A & ~B & ~C   = ~B
				return util::string_format("%s = %s", dst_text, b_text);

			case 0xf0:      // A & B & C | ~A & B & C | A & ~B & C | ~A & ~B & C       = C
				return util::string_format("%s = %s", dst_text, c_text);;

			case 0x0f:      // A & B & ~C | ~A & B & ~C | A & ~B & ~C | ~A & ~B & ~C   = ~C
				return util::string_format("%s = ~%s", dst_text, c_text);

			case 0x80:      // A & B & C
				return util::string_format("%s = %s & %s & %s", dst_text, a_text, b_text, c_text);

			case 0x88:      // A & B & C | A & B & ~C                                  = A & B
				return util::string_format("%s = %s & %s", dst_text, a_text, b_text);

			case 0xa0:      // A & B & C | A & ~B & C                                  = A & C
				return util::string_format("%s = %s & %s", dst_text, a_text, c_text);

			case 0xc0:      // A & B & C | ~A & B & C                                  = B & C
				return util::string_format("%s = %s & %s", dst_text, b_text, c_text);

			case 0xea:      //  A &  B &  C | ~A &  B &  C |  A & ~B &  C |
							//  A &  B & ~C |  A & ~B & ~C                             = A | C
				return util::string_format("%s = %s | %s", dst_text, a_text, c_text);

			case 0xee:      //  A &  B &  C | ~A &  B &  C |  A & ~B &  C |
							//  A &  B & ~C | ~A &  B & ~C |  A & ~B & ~C              = A | B
				return util::string_format("%s = %s | %s", dst_text, a_text, b_text);

			case 0x44:      // ~A &  B &  C | ~A &  B & ~C                             = ~A & B
				return util::string_format("%s = ~%s & %s", dst_text, a_text, b_text);

			case 0x22:		//  A & ~B & ~C |  A & ~B & C                              = A & ~B
				return util::string_format("%s = %s & ~%s", dst_text, a_text, b_text);

			case 0xaf:		//  A  &  B &  C |  A & ~B &  C |  A &  B & ~C |
							// ~A  &  B & ~C |  A & ~B & ~C | ~A & ~B & ~C             = A | ~C
				return util::string_format("%s = %s | ~%s", dst_text, a_text, c_text);

			case 0xfa:		//  A  &  B &  C | ~A &  B &  C |  A & ~B &  C |
							// ~A  & ~B &  C |  A &  B & ~C |  A & ~B & ~C             = A | C
				return util::string_format("%s = %s | %s", dst_text, a_text, c_text);


			default:
				return util::string_format("%s = b%02X(%s, %s, %s)", dst_text, aluop, a_text, b_text, c_text);
		}
	}
	return std::string("");
}

u32 tms32082_pp_disassembler::opcode_alignment() const
{
	return 8;
}

offs_t tms32082_pp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;

	m_alu_condition.clear();
	m_alu_operation.clear();
	m_parallel_condition.clear();
	m_parallel_transfer.clear();

	// by default src/dst bank is 4, but can be overridden by parallel transfer types 6 & 10
	m_src1bank = 4;
	m_dstbank = 4;

	uint64_t op = opcodes.r64(pc);
	switch (op >> 60)
	{
		case 0x6:
		case 0x7:           // Six-operand
		{
			int sixop = (op >> 57) & 0xf;

			int src3 = ((op >> 54) & 7) | (4 << 3);
			int dst2 = ((op >> 51) & 7) | (4 << 3);
			int dst1 = ((op >> 48) & 7) | (m_dstbank << 3);
			int src1 = ((op >> 45) & 7) | (m_src1bank << 3);
			int src4 = ((op >> 42) & 7) | (4 << 3);
			int src2 = ((op >> 39) & 7) | (4 << 3);

			parallel_transfer(op);

			switch (sixop)
			{
				case 0x0:
				case 0x1:
				case 0x4:
				case 0x5:
					// MPY||ADD
					m_alu_operation = util::string_format("%s = %c(%s * %s), %s = %s %c %s",
						get_reg_name(dst2, false),
						(sixop & 4) ? 'u' : 's',
						get_reg_name(src3, true),
						get_reg_name(src4, true),
						get_reg_name(dst1, false),
						get_reg_name(src2, true),
						(sixop & 1) ? '-' : '+',
						get_reg_name(src1, true));
					break;

				case 0x2:
				case 0x3:
				case 0x6:
				case 0x7:
					m_alu_operation = util::string_format("MPY||EALU");
					break;

				case 0x8:
				case 0x9:
				case 0xb:
					m_alu_operation = util::string_format("EALU||ROTATE");
					break;

				case 0xa:
					m_alu_operation = util::string_format("divi");
					break;

				case 0xc:
				case 0xd:
				case 0xe:
				case 0xf:
					m_alu_operation = util::string_format("MPY||SADD");
					break;
			}
			break;
		}

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
		{
			if ((op & 0xfaa8100000000000U) == 0x8800000000000000U)
			{
				int operation = (op >> 39) & 0x1f;
				
				parallel_transfer(op);

				switch (operation)
				{
					case 0x00: break;
					case 0x02: m_alu_operation = util::string_format("eint"); break;
					case 0x03: m_alu_operation = util::string_format("dint"); break;
					default:   m_alu_operation = util::string_format("<reserved>"); break;
				}
			}
			else
			{
				std::string dst_text;
				std::string a_text;
				std::string b_text;
				std::string c_text;

				switch ((op >> 43) & 3)
				{
					case 0:
					case 1:     // Base set ALU (5-bit immediate)
					{
						parallel_transfer(op);						

						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int src2imm = (op >> 39) & 0x1f;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;

						int s1reg = src1 | (m_src1bank << 3);
						int dreg = dst | (m_dstbank << 3);
						int dstcreg = dst | (0x4 << 3);

						dst_text = util::string_format("%s", get_reg_name(dreg, false));
						switch (cl)
						{
							case 0:
								a_text = util::string_format("0x%02X", src2imm);
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("@mf");
								break;
							case 1:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("0x%02X", src2imm);
								break;
							case 2:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("%%0x%02X", src2imm);
								break;
							case 3:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s\\\\0x%02X", get_reg_name(s1reg, true), src2imm);
								c_text = util::string_format("%%0x%02X", src2imm);
								break;
							case 4:
								a_text = util::string_format("0x%02X", src2imm);
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("%%d0");
								break;
							case 5:
								a_text = util::string_format("0x%02X", src2imm);
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("@mf");
								break;
							case 6:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("0x%02X", src2imm);
								break;
							case 7:
								a_text = util::string_format("%s", get_reg_name(s1reg, true));
								//b_text = util::string_format("1\\\\0x%02X", src2imm);
								b_text = util::string_format("0x%08X", ROTATE_L(1, src2imm));
								c_text = util::string_format("0x%02X", src2imm);
								break;
						}

						m_alu_operation = format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);					
						break;
					}

					case 2:     // Base set ALU (reg src2)
					{
						parallel_transfer(op);

						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int src2 = (op >> 39) & 7;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;

						int s1reg = src1 | (m_src1bank << 3);
						int s2reg = src2 | (0x4 << 3);
						int dstcreg = dst | (0x4 << 3);
						int dreg = dst | (m_dstbank << 3);

						dst_text = util::string_format("%s", get_reg_name(dreg, false));
						switch (cl)
						{
							case 0:
								a_text = util::string_format("%s", get_reg_name(s2reg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("@mf");
								break;
							case 1:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("%s", get_reg_name(s2reg, true));
								break;
							case 2:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("%%%s", get_reg_name(s2reg, true));
								break;
							case 3:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s\\\\%s", get_reg_name(s1reg, true), get_reg_name(s2reg, true));
								c_text = util::string_format("%%%s", get_reg_name(s2reg, true));
								break;
							case 4:
								a_text = util::string_format("%s", get_reg_name(s2reg, true));
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("%%d0");
								break;
							case 5:
								a_text = util::string_format("%s", get_reg_name(s2reg, true));
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("@mf");
								break;
							case 6:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("%s", get_reg_name(s2reg, true));
								break;
							case 7:
								a_text = util::string_format("%s", get_reg_name(s1reg, true));
								b_text = util::string_format("1\\\\%s", get_reg_name(s2reg, true));
								c_text = util::string_format("%s", get_reg_name(s2reg, true));
								break;
						}

						m_alu_operation = format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);						
						break;
					}

					case 3:     // Base set ALU (32-bit immediate)
					{
						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int dstbank = (op >> 39) & 0xf;
						int s1bank = (op >> 36) & 7;
						int cond = (op >> 32) & 0xf;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;
						uint32_t imm32 = (uint32_t)(op);

						int dreg = dst | (dstbank << 3);
						int s1reg = src1 | (s1bank << 3);
						int dstcreg = dst | (0x4 << 3);

						dst_text = util::string_format("%s", get_reg_name(dreg, false));
						switch (cl)
						{
							case 0:
								a_text = util::string_format("0x%08X", imm32);
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("@mf");
								break;
							case 1:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("0x%08X", imm32);
								break;
							case 2:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("%%0x%08X", imm32);
								break;
							case 3:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s\\\\0x%08X", get_reg_name(s1reg, true), imm32);
								c_text = util::string_format("%%0x%08X", imm32);
								break;
							case 4:
								a_text = util::string_format("0x%08X", imm32);
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("%%d0");
								break;
							case 5:
								a_text = util::string_format("0x%08X", imm32);
								b_text = util::string_format("%s\\\\d0", get_reg_name(s1reg, true));
								c_text = util::string_format("@mf");
								break;
							case 6:
								a_text = util::string_format("%s", get_reg_name(dstcreg, true));
								b_text = util::string_format("%s", get_reg_name(s1reg, true));
								c_text = util::string_format("0x%08X", imm32);
								break;
							case 7:
								a_text = util::string_format("%s", get_reg_name(s1reg, true));
								//b_text = util::string_format("1\\\\0x%08X", imm32);
								b_text = util::string_format("%08X", ROTATE_L(1, imm32));
								c_text = util::string_format("0x%08X", imm32);
								break;
						}

						m_alu_condition = make_condition(cond, 0);

						m_alu_operation = format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);
						break;
					}
				}
			}
			break;
		}

		default:
			m_alu_operation = util::string_format("??? (%02X)", (uint32_t)(op >> 60));
			break;
	}


	if (m_alu_operation.length() == 0 && m_parallel_transfer.length() == 0)
	{
		stream << "nop";
	}
	else
	{
		if (m_alu_operation.length() > 0)
		{
			stream << m_alu_condition;
			stream << m_alu_operation;

			if (m_parallel_transfer.length() > 0)
			{
				stream << " || ";
			}
		}		
		if (m_parallel_transfer.length() > 0)
		{
			stream << m_parallel_condition;
			stream << m_parallel_transfer;
		}
	}

	return 8 | flags | SUPPORTED;
}
