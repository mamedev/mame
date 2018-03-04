// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "ie15dasm.h"

u32 ie15_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t ie15_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	uint8_t op;
	unsigned PC = pc;

	op = opcodes.r8(pc++);
	switch (op & 0xf0)
	{
		case 0x00:
			util::stream_format(stream, "add  r%d", op & 0x0f);
			break;
		case 0x10:
			util::stream_format(stream, "jmp  $%04x", (((op & 0x0f) << 8) | params.r8(pc)) + 1);
			pc+=1;
			break;
		case 0x20:
			util::stream_format(stream, "ldc  r%d, #$%02x", (op & 0x0f), params.r8(pc));
			pc+=1;
			break;
		case 0x30: switch (op)
		{
			case 0x30:
				util::stream_format(stream, "lca  #$%02x", params.r8(pc));
				pc+=1;
				break;
			case 0x33:
				util::stream_format(stream, "ral");
				break;
			case 0x35:
				util::stream_format(stream, "rar");
				break;
			default:
				util::stream_format(stream, "illegal");
				break;
		};
		break;
		case 0x40:
			util::stream_format(stream, "dsr  r%d", op & 0x0f);
			break;
		case 0x50: switch (op)
		{
			case 0x50:
				util::stream_format(stream, "isn");
				break;
			case 0x51:
				util::stream_format(stream, "inc");
				break;
			case 0x52:
				util::stream_format(stream, "dsn");
				break;
			case 0x58:
				util::stream_format(stream, "ise");
				break;
			case 0x5a:
				util::stream_format(stream, "dse");
				break;
			case 0x5b:
				util::stream_format(stream, "dec");
				break;
			case 0x5d:
				util::stream_format(stream, "com");
				break;
			case 0x5f:
				util::stream_format(stream, "clr");
				break;
			default:
				util::stream_format(stream, "illegal");
				break;
		};
		break;
		case 0x60:
			util::stream_format(stream, "lla  #$%02x", op & 0x0f);
			break;
		case 0x70:
			util::stream_format(stream, "jmi  r%d", op & 0x0f);
			break;
		case 0x80: switch (op)
		{
			case 0x80: case 0x81: case 0x82: case 0x83:
			case 0x84: case 0x85: case 0x86:
				util::stream_format(stream, "sfc  #%d", op & 0x07);
				break;
			case 0x87:
				util::stream_format(stream, "skp");
				break;
			case 0x88: case 0x89: case 0x8a: case 0x8b:
			case 0x8c: case 0x8d: case 0x8e:
				util::stream_format(stream, "sfs  #%d", op & 0x07);
				break;
			case 0x8f:
				util::stream_format(stream, "nop");
				break;
		};
		break;
		case 0x90:
			util::stream_format(stream, "and  r%d", op & 0x0f);
			break;
		case 0xa0:
			util::stream_format(stream, "xor  r%d", op & 0x0f);
			break;
		case 0xb0:
			util::stream_format(stream, "cs   r%d", op & 0x0f);
			break;
		case 0xc0:
			util::stream_format(stream, "%s  #%d", BIT(op, 3) ? "sfl" : "cfl", op & 0x07);
			break;
		case 0xd0:
			util::stream_format(stream, "lda  r%d", op & 0x0f);
			break;
		case 0xe0:
			util::stream_format(stream, "sta  r%d", op & 0x0f);
			break;
		case 0xf0:
			util::stream_format(stream, "ota  #$%02x", op & 0x0f);
			break;
	}

	return (pc - PC) | flags | SUPPORTED;
}
