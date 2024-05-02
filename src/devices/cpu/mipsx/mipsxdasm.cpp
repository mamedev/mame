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

/*


AJR: I took a look and think it's definitely MIPS-X of the big-endian variety. There might be some undocumented instruction set extensions, though.
AJR: The giveaway is the prevalence of the word 0x60000019, which performs the operation r0 + r0 -> r0, in other words a no-op.


*/


offs_t mipsx_disassembler::disassemble(std::ostream& stream, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	u32 opcode = opcodes.r32(pc);

	u8 ty = get_ty(opcode);

	switch (ty)
	{
	case 0:
	{
		u8 op = get_op(opcode);

		switch (op)
		{
		case 1:
		{
			util::stream_format(stream, "beq (%02x, %02x, %01x %04x)", get_src1(opcode), get_src2_dest(opcode), get_sq(opcode), get_disp(opcode));
			break;
		}
		case 2:
		{
			util::stream_format(stream, "bhs (%02x, %02x, %01x %04x)", get_src1(opcode), get_src2_dest(opcode), get_sq(opcode), get_disp(opcode));
			break;
		}
		case 3:
		{
			util::stream_format(stream, "blt (%02x, %02x, %01x %04x)", get_src1(opcode), get_src2_dest(opcode), get_sq(opcode), get_disp(opcode));
			break;
		}
		case 5:
		{
			util::stream_format(stream, "bne (%02x, %02x, %01x %04x)", get_src1(opcode), get_src2_dest(opcode), get_sq(opcode), get_disp(opcode));
			break;
		}
		case 6:
		{
			util::stream_format(stream, "blo (%02x, %02x, %01x %04x)", get_src1(opcode), get_src2_dest(opcode), get_sq(opcode), get_disp(opcode));
			break;
		}
		case 7:
		{
			util::stream_format(stream, "bge (%02x, %02x, %01x %04x)", get_src1(opcode), get_src2_dest(opcode), get_sq(opcode), get_disp(opcode));
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
					util::stream_format(stream, "mstart (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
				}
				else
				{
					util::stream_format(stream, "illegal mstart (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
				}
			}
			else if (comp == 0x099)
			{
				util::stream_format(stream, "mstep (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x166)
			{
				util::stream_format(stream, "dstep (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
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
				util::stream_format(stream, "rotlcb (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x0c0)
			{
				util::stream_format(stream, "rotlb (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if ((comp & 0xf80) == 0x200)
			{
				util::stream_format(stream, "sh (%02x, %02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode) & 0x7f);
			}
			else if ((comp & 0xf80) == 0x100)
			{
				u8 src2 = get_src2_dest(opcode);

				if (src2 == 0x0)
				{
					util::stream_format(stream, "asr (%02x, %02x, %02x)", get_src1(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode) & 0x7f);
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
				util::stream_format(stream, "blc (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x00f)
			{
				util::stream_format(stream, "not (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
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
						util::stream_format(stream, "mov (%02x, %02x)", get_src1(opcode), get_compute_dest(opcode));
					}
				}
				else
				{
					util::stream_format(stream, "add (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
				}
			}
			else if (comp == 0x01b)
			{
				util::stream_format(stream, "xor (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x023)
			{
				util::stream_format(stream, "and (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x026)
			{
				util::stream_format(stream, "subnc (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x03b)
			{
				util::stream_format(stream, "or (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
			}
			else if (comp == 0x066)
			{
				util::stream_format(stream, "sub (%02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
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

		switch (op)
		{
		case 0:
		{
			util::stream_format(stream, "ld (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 1:
		{
			util::stream_format(stream, "ldt (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 2:
		{
			util::stream_format(stream, "st (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 3:
		{
			util::stream_format(stream, "stt (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 4:
		{
			util::stream_format(stream, "ldf (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
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
				util::stream_format(stream, "movfrc (%02x, %02x, %02x, %02x, %02x)", dest, op, func, c1, c2);
			}
			else
			{
				util::stream_format(stream, "illegal movfrc form");
			}
			break;

		}
		case 6:
		{
			util::stream_format(stream, "stf (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 7: // movtoc
		{
			u8 c2 =   (opcode & 0x0000000f);
			u8 c1 =   (opcode & 0x000000f0) >> 4;
			u8 func = (opcode & 0x00003f00) >> 8;
			u8 op =   (opcode & 0x0001c000) >> 14;
			u8 dest = get_src2_dest(opcode);
			u8 src = get_src1(opcode);

			if (src == 0)
			{
				util::stream_format(stream, "movtoc (%02x, %02x, %02x, %02x, %02x)", dest, op, func, c1, c2);
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
			util::stream_format(stream, "jspci (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_imm17(opcode));
			break;
		}

		case 1:
		{
			// Halt and Spontaneously Combust
			util::stream_format(stream, "hsc");
			break;
		}

		case 2:
		{
			u16 comp = get_compute_compfunc(opcode);
			if ((comp & 0xffc) == 0x000)
			{
				u8 dest = get_src2_dest(opcode);

				if (dest == 0x00)
				{
					u8 src = get_src1(opcode);
					util::stream_format(stream, "movtos (%02x, %04x)", src, comp & 0x3);
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
			u16 comp = get_compute_compfunc(opcode);
			if ((comp & 0xffc) == 0x000)
			{
				u8 src = get_src1(opcode);
				if (src == 0x00)
				{
					u8 dest = get_src2_dest(opcode);
					util::stream_format(stream, "movfrs (%02x, %04x)", dest, comp & 0x3);
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
			util::stream_format(stream, "addi (%02x, %02x, %04x)", get_src1(opcode), get_src2_dest(opcode), get_imm17(opcode));
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
					util::stream_format(stream, "jpc (%02x, %02x, %04x)");
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
			util::stream_format(stream, "trap");
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
					util::stream_format(stream, "jpcrs (%02x, %02x, %04x)");
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
