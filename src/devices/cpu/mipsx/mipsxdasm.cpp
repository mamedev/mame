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


offs_t mipsx_disassembler::disassemble(std::ostream& stream, offs_t pc, const data_buffer& opcodes, const data_buffer& params)
{
	u32 opcode = opcodes.r32(pc);

	u8 ty = get_ty(opcode);

	switch (ty)
	{
	case 0:
	{
		uint8_t op = get_op(opcode);

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
	case 2:
	{
		uint8_t op = get_op(opcode);

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

	default:
	{
		util::stream_format(stream, "Unhandled (%08x)", opcode);
		break;
	}
	}

	return 4 | SUPPORTED;
}
