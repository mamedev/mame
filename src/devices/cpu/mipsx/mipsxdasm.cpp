// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "mipsxdasm.h"

u32 mipsx_disassembler::opcode_alignment() const
{
	return 4;
}

int mipsx_disassembler::get_ty(u32 opcode)
{
	return (opcode & 0xc0000000) >> 30;
}

int mipsx_disassembler::get_op(u32 opcode)
{
	return (opcode & 0x38000000) >> 27;
}

int mipsx_disassembler::get_src1(u32 opcode)
{
	return (opcode & 0x07c00000) >> 22;
}

int mipsx_disassembler::get_src2_dest(u32 opcode)
{
	return (opcode & 0x003e0000) >> 17;
}

int mipsx_disassembler::get_compute_dest(u32 opcode)
{
	return (opcode & 0x0001f000) >> 12;
}

int mipsx_disassembler::get_compute_compfunc(u32 opcode)
{
	return (opcode & 0x00000fff) >> 0;
}

int mipsx_disassembler::get_offset(u32 opcode)
{
	return (opcode & 0x0001ffff) >> 0;
}

int mipsx_disassembler::get_imm17(u32 opcode)
{
	return (opcode & 0x0001ffff) >> 0;
}

int mipsx_disassembler::get_sq(u32 opcode)
{
	return (opcode & 0x00010000) >> 16;
}

int mipsx_disassembler::get_disp(u32 opcode)
{
	return (opcode & 0x0000ffff) >> 0;
}

int mipsx_disassembler::get_asr_amount(int shift)
{
	// TODO
	return shift;
}

int mipsx_disassembler::get_sh_amount(int shift)
{
	// TODO
	return shift;
}


std::string mipsx_disassembler::get_regname(u8 reg)
{
	// general purpose register 0 just acts as a constant 0, it can't be changed.
	if (reg == 0)
		return "#0";
	else
		return "r" + std::to_string(reg);
}

offs_t mipsx_disassembler::disassemble(std::ostream& stream, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	u32 opcode = opcodes.r32(pc);

	u8 ty = get_ty(opcode);

	switch (ty)
	{
	case 0:
	{
		u8 op = get_op(opcode);
		int disp = get_disp(opcode) << 2;
		disp = util::sext(disp & 0x3fffc, 18);
		u32 basepc = pc;

		switch (op)
		{
		case 1:
		{
			if (get_sq(opcode))
			{
				util::stream_format(stream, "beqsq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			else
			{
				util::stream_format(stream, "beq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			break;
		}
		case 2:
		{
			if (get_sq(opcode))
			{
				util::stream_format(stream, "bhssq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			else
			{
				util::stream_format(stream, "bhs %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			break;
		}
		case 3:
		{
			if (get_sq(opcode))
			{
				util::stream_format(stream, "bltsq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			else
			{
				util::stream_format(stream, "blt %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			break;
		}
		case 5:
		{
			if (get_sq(opcode))
			{
				util::stream_format(stream, "bnesq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			else
			{
				util::stream_format(stream, "bne %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			break;
		}
		case 6:
		{
			if (get_sq(opcode))
			{
				util::stream_format(stream, "blosq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			else
			{
				util::stream_format(stream, "blo %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			break;
		}
		case 7:
		{
			if (get_sq(opcode))
			{
				util::stream_format(stream, "bgesq %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			else
			{
				util::stream_format(stream, "bge %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), basepc + disp);
			}
			break;
		}

		default:
		{
			util::stream_format(stream, "Unhandled TY0 (%08x)", opcode);
			break;
		}
		}
		break;
	}

	case 1:
	{
		u8 op = get_op(opcode);

		switch (op)
		{
		case 0:
		{
			u16 comp = get_compute_compfunc(opcode);
			if (comp == 0x0e6)
			{
				u8 src1 = get_src1(opcode);

				if (src1 == 0)
				{
					util::stream_format(stream, "mstart %s, %s", get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
				}
				else
				{
					util::stream_format(stream, "illegal mstart form");
				}
			}
			else if (comp == 0x099)
			{
				util::stream_format(stream, "mstep %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x166)
			{
				util::stream_format(stream, "dstep %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else
			{
				util::stream_format(stream, "unkonwn TY1 subcase 0 (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode));
			}
			break;
		}

		case 1:
		{
			u16 comp = get_compute_compfunc(opcode);

			if (comp == 0x080)
			{
				util::stream_format(stream, "rotlcb %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x0c0)
			{
				util::stream_format(stream, "rotlb %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if ((comp & 0xf80) == 0x200)
			{
				int shift = get_sh_amount(get_compute_compfunc(opcode) & 0x7f);
				util::stream_format(stream, "sh %s, %s, %s, #%d", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)), shift);
			}
			else if ((comp & 0xf80) == 0x100)
			{
				u8 src2 = get_src2_dest(opcode);

				if (src2 == 0x0)
				{
					int shift = get_asr_amount((get_compute_compfunc(opcode) & 0x7f));
					util::stream_format(stream, "asr %s, %s, #%d", get_regname(get_src1(opcode)), get_regname(get_compute_dest(opcode)), shift);
				}
				else
				{
					util::stream_format(stream, "illegal asr form");
				}
			}
			else
			{
				util::stream_format(stream, "unknown TY1 subcase 1 (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode));
			}
			break;
		}

		case 4:
		{
			u16 comp = get_compute_compfunc(opcode);

			if (comp == 0x00b)
			{
				util::stream_format(stream, "blc %s, %s,%s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x00f)
			{
				if (get_src2_dest(opcode) == 0x00)
				{
					util::stream_format(stream, "not %s, %s", get_regname(get_src1(opcode)), get_regname(get_compute_dest(opcode)));
				}
				else
				{
					util::stream_format(stream, "illegal not form");
				}
			}
			else if (comp == 0x019)
			{
				u8 src2 = get_src2_dest(opcode);
				if (src2 == 0)
				{
					u8 src1 = get_src1(opcode);
					u8 dest = get_compute_dest(opcode);

					if ((src1 == 0) && (dest == 0))
					{
						util::stream_format(stream, "nop");
					}
					else
					{
						util::stream_format(stream, "mov %s, %s", get_regname(get_src1(opcode)), get_regname(get_compute_dest(opcode)));
					}
				}
				else
				{
					util::stream_format(stream, "add %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
				}
			}
			else if (comp == 0x01b)
			{
				util::stream_format(stream, "xor %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x023)
			{
				util::stream_format(stream, "and %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x026)
			{
				util::stream_format(stream, "subnc %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x03b)
			{
				util::stream_format(stream, "or %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else if (comp == 0x066)
			{
				util::stream_format(stream, "sub %s, %s, %s", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), get_regname(get_compute_dest(opcode)));
			}
			else
			{
				util::stream_format(stream, "unknown compute (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode));
			}
			break;
		}

		default:
		{
			util::stream_format(stream, "Unhandled TY1 (%08x)", opcode);
			break;
		}
		}
		break;

	}

	case 2:
	{
		u8 op = get_op(opcode);
		int imm17 = get_offset(opcode);
		imm17 = util::sext(imm17 & 0x1ffff, 17);

		switch (op)
		{
		case 0:
		{
			// ld - Load
			// ld Offset[rSrc1], rDest
			util::stream_format(stream, "ld %08x[%s], %s", imm17, get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)));
			break;
		}
		case 1:
		{
			// ldt - Load Through
			// ldt Offset[rSrc1], rDest
			util::stream_format(stream, "ldt %08x[%s], %s", imm17, get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)));
			break;
		}
		case 2:
		{
			// st - Store
			// st Offset[rSrc1], rDest
			util::stream_format(stream, "st %08x[%s], %s", imm17, get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)));
			break;
		}
		case 3:
		{
			// stt - Store Through
			// stt Offset[rSrc1], rDest
			util::stream_format(stream, "stt %08x[%s], %s", imm17, get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)));
			break;
		}
		case 4:
		{
			// ldf - Load Floating Point
			// ldf Offset[rSrc1], rDest
			util::stream_format(stream, "ldf %08x[%s], %s", imm17, get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)));
			break;

		}
		case 5: // movfrc or aluc
		{
			u8 c2 =   (opcode & 0x0000000f);
			u8 c1 =   (opcode & 0x000000f0) >> 4;
			u8 func = (opcode & 0x00003f00) >> 8;
			u8 op =   (opcode & 0x0001c000) >> 14;
			u8 dest = get_src2_dest(opcode);
			u8 src = get_src1(opcode);

			if (src == 0)
			{
				util::stream_format(stream, "movfrc %s, (%02x, %02x, %02x, %02x)", get_regname(get_src1(dest)), op, func, c1, c2);
			}
			else
			{
				util::stream_format(stream, "illegal movfrc form");
			}
			break;

		}
		case 6:
		{
			// stf - Store Floating Point
			// stf Offset[rSrc1], rDest
			util::stream_format(stream, "stf %08x[%s], %s", imm17, get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)));
			break;
		}
		case 7: // movtoc
		{
			// this is just a suggested form
			u8 c2 =   (opcode & 0x0000000f);
			u8 c1 =   (opcode & 0x000000f0) >> 4;
			u8 func = (opcode & 0x00003f00) >> 8;
			u8 op =   (opcode & 0x0001c000) >> 14;
			u8 dest = get_src2_dest(opcode);
			u8 src = get_src1(opcode);

			if (src == 0)
			{
				util::stream_format(stream, "movtoc %s, (%02x, %02x, %02x, %02x)", get_regname(get_src1(dest)), op, func, c1, c2);
			}
			else
			{
				util::stream_format(stream, "illegal movtoc form");
			}
			break;

		}
		default:
		{
			util::stream_format(stream, "Unhandled TY2 (%08x)", opcode);
			break;
		}
		}
		break;
	}

	case 3:
	{
		u8 op = get_op(opcode);

		switch (op)
		{
		case 0:
		{
			// jspci - Jump Indexed and Store PC
			// jspci rSrc1,#Immed,Rdest
			int imm17 = get_imm17(opcode);
			imm17 = util::sext(imm17 & 0x1ffff, 17);
			// should imm17 be shifted?

			util::stream_format(stream, "jspci %s, #%02x, %s", get_regname(get_src1(opcode)), imm17, get_regname(get_src2_dest(opcode)));
			break;
		}

		case 1:
		{
			// Halt and Spontaneously Combust
			if ((opcode & 0x07ffffff) == 0x07c00000)
			{
				util::stream_format(stream, "hsc");
			}
			else
			{
				util::stream_format(stream, "illegal hsc form");
			}
			break;
		}

		case 2:
		{
			// movtos - Move to Special Register
			// movtos rSrc1, SpecialReg

			// psw  001
			// md   010
			// pcm1 100
			u16 comp = get_compute_compfunc(opcode);
			if ((comp & 0xffe) == 0x000)
			{
				u8 dest = get_src2_dest(opcode);

				if (dest == 0x00)
				{
					u8 src = get_src1(opcode);
					util::stream_format(stream, "movtos %s, %01x", get_regname(src), comp & 0x7);
				}
				else
				{
					util::stream_format(stream, "illegal movtos form");
				}
			}
			else
			{
				util::stream_format(stream, "illegal movtos form");
			}
			break;

		}

		case 3:
		{
			// movfrs - Move from Special Register
			// movfrs SpecialReg, rDest

			// psw  001
			// md   010
			// pcm4 100
			u16 comp = get_compute_compfunc(opcode);
			if ((comp & 0xffe) == 0x000)
			{
				u8 src = get_src1(opcode);
				if (src == 0x00)
				{
					u8 dest = get_src2_dest(opcode);
					util::stream_format(stream, "movfrs %01x, %s", comp & 0x7, get_regname(dest));
				}
				else
				{
					util::stream_format(stream, "illegal movfrs form");
				}
			}
			else
			{
				util::stream_format(stream, "illegal movfrs form");
			}
			break;

		}

		case 4:
		{
			int imm17 = get_imm17(opcode);
			imm17 = util::sext(imm17 & 0x1ffff, 17);
			util::stream_format(stream, "addi %s, %s, %08x", get_regname(get_src1(opcode)), get_regname(get_src2_dest(opcode)), imm17);
			break;
		}

		case 5:
		{
			u16 comp = get_compute_compfunc(opcode);
			if (comp == 0x03)
			{
				u8 dest = get_src2_dest(opcode);
				u8 src = get_src1(opcode);

				if ((src == 0x00) && (dest == 0x00))
				{
					util::stream_format(stream, "jpc");
				}
				else
				{
					util::stream_format(stream, "illegal jpc form");
				}
			}
			else
			{
				util::stream_format(stream, "illegal jpc form");
			}
			break;
		}

		case 6:
		{
			if ((opcode & 0x07fff807) == 0x00000003)
			{
				int vector = (opcode & 0x000007f8) >> 3;
				util::stream_format(stream, "trap %02x", vector);
			}
			else
			{
				util::stream_format(stream, "illegal trap form");
			}
			break;
		}


		case 7:
		{
			u16 comp = get_compute_compfunc(opcode);
			if (comp == 0x03)
			{
				u8 dest = get_src2_dest(opcode);
				u8 src = get_src1(opcode);

				if ((src == 0x00) && (dest == 0x00))
				{
					util::stream_format(stream, "jpcrs");
				}
				else
				{
					util::stream_format(stream, "illegal jpcrs form");
				}
			}
			else
			{
				util::stream_format(stream, "illegal jpcrs form");
			}
			break;
		}

		default:
		{
			util::stream_format(stream, "Unhandled TY3 (%08x)", opcode);
			break;
		}
		}
		break;
	}

	default:
	{
		util::stream_format(stream, "Unhandled (%08x)", opcode);
		break;
	}
	}

	return 4 | SUPPORTED;
}
