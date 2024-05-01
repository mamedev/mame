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
					util::stream_format(stream, "??? (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode));
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
				util::stream_format(stream, "??? (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode));
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
			else if ((comp & 0x0f8) == 0x020)
			{
				util::stream_format(stream, "sh (%02x, %02x, %02x, %02x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode) & 0x7);
			}
			else if ((comp & 0x0f8) == 0x010)
			{
				u8 src2 = get_src2_dest(opcode);

				if (src2 == 0x0)
				{
					util::stream_format(stream, "asr (%02x, %02x, %02x)", get_src1(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode) & 0x7);
				}
			}
			else
			{
				util::stream_format(stream, "??? (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode));
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
				util::stream_format(stream, "??? (%02x, %02x, %02x %04x)", get_src1(opcode), get_src2_dest(opcode), get_compute_dest(opcode), get_compute_compfunc(opcode));
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
			util::stream_format(stream, "ld (%02x, %02x, %05x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 1:
		{
			util::stream_format(stream, "ldt (%02x, %02x, %05x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 2:
		{
			util::stream_format(stream, "st (%02x, %02x, %05x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 3:
		{
			util::stream_format(stream, "stt (%02x, %02x, %05x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		case 4:
		{
			util::stream_format(stream, "ldf (%02x, %02x, %05x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		//case 5: // movfrc or aluc
		case 6:
		{
			util::stream_format(stream, "stf (%02x, %02x, %05x)", get_src1(opcode), get_src2_dest(opcode), get_offset(opcode));
			break;
		}
		//case 7: // movtoc
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
