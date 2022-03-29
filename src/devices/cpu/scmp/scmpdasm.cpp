// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   scmpdasm.c
 *
 *   National Semiconductor SC/MP CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"
#include "scmpdasm.h"

u32 scmp_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t scmp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned PC = pc;
	uint8_t op = opcodes.r8(pc++);
	uint8_t ptr = op & 3;

	if (BIT(op,7)) {
		// two-byte instructions
		char as[10];
		uint8_t arg = params.r8(pc++);
		if (arg==0x80) {
			sprintf(as,"E");
		} else if (arg & 0x80) {
			sprintf(as,"-$%02x",0x100-arg);
		} else {
			sprintf(as,"+$%02x",arg);
		}

		switch (op)
		{
			// Memory Reference Instructions
			case 0xc0 : util::stream_format(stream, "ld %s",as); break;
			case 0xc1 : case 0xc2 : case 0xc3 :
						util::stream_format(stream, "ld %s(%d)",as,ptr);break;
			case 0xc5 : case 0xc6 : case 0xc7 :
						util::stream_format(stream, "ld @%s(%d)",as,ptr); break;
			case 0xc8 : util::stream_format(stream, "st %s",as); break;
			case 0xc9 : case 0xca : case 0xcb :
						util::stream_format(stream, "st %s(%d)",as,ptr);break;
			case 0xcd : case 0xce : case 0xcf :
						util::stream_format(stream, "st @%s(%d)",as,ptr); break;
			case 0xd0 : util::stream_format(stream, "and %s",as); break;
			case 0xd1 : case 0xd2 : case 0xd3 :
						util::stream_format(stream, "and %s(%d)",as,ptr);break;
			case 0xd5 : case 0xd6 : case 0xd7 :
						util::stream_format(stream, "and @%s(%d)",as,ptr); break;
			case 0xd8 : util::stream_format(stream, "or %s",as); break;
			case 0xd9 : case 0xda : case 0xdb :
						util::stream_format(stream, "or %s(%d)",as,ptr);break;
			case 0xdd : case 0xde : case 0xdf :
						util::stream_format(stream, "or @%s(%d)",as,ptr); break;
			case 0xe0 : util::stream_format(stream, "xor %s",as); break;
			case 0xe1 : case 0xe2 : case 0xe3 :
						util::stream_format(stream, "xor %s(%d)",as,ptr);break;
			case 0xe5 : case 0xe6 : case 0xe7 :
						util::stream_format(stream, "xor @%s(%d)",as,ptr); break;
			case 0xe8 : util::stream_format(stream, "dad %s",as); break;
			case 0xe9 : case 0xea : case 0xeb :
						util::stream_format(stream, "dad %s(%d)",as,ptr);break;
			case 0xed : case 0xee : case 0xef :
						util::stream_format(stream, "dad @%s(%d)",as,ptr); break;
			case 0xf0 : util::stream_format(stream, "add %s",as); break;
			case 0xf1 : case 0xf2 : case 0xf3 :
						util::stream_format(stream, "add %s(%d)",as,ptr);break;
			case 0xf5 : case 0xf6 : case 0xf7 :
						util::stream_format(stream, "add @%s(%d)",as,ptr); break;
			case 0xf8 : util::stream_format(stream, "cad %s",as); break;
			case 0xf9 : case 0xfa : case 0xfb :
						util::stream_format(stream, "cad %s(%d)",as,ptr);break;
			case 0xfd : case 0xfe : case 0xff :
						util::stream_format(stream, "cad @%s(%d)",as,ptr); break;
			// Memory Increment/Decrement Instructions
			case 0xa8 : case 0xa9 : case 0xaa : case 0xab :
						util::stream_format(stream, "ild %s(%d)",as,ptr); break;
			case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
						util::stream_format(stream, "dld %s(%d)",as,ptr); break;
			// Immediate Instructions
			case 0xc4 : util::stream_format(stream, "ldi $%02x",arg); break;
			case 0xd4 : util::stream_format(stream, "ani $%02x",arg); break;
			case 0xdc : util::stream_format(stream, "ori $%02x",arg); break;
			case 0xe4 : util::stream_format(stream, "xri $%02x",arg); break;
			case 0xec : util::stream_format(stream, "dai $%02x",arg); break;
			case 0xf4 : util::stream_format(stream, "adi $%02x",arg); break;
			case 0xfc : util::stream_format(stream, "cai $%02x",arg); break;
			// Transfer Instructions
			case 0x90 : util::stream_format(stream, "jmp %s",as);break;
			case 0x91 : case 0x92 : case 0x93 :
						util::stream_format(stream, "jmp %s(%d)",as,ptr);break;
			case 0x94 : util::stream_format(stream, "jp %s",as); break;
			case 0x95 : case 0x96 : case 0x97 :
						util::stream_format(stream, "jp %s(%d)",as,ptr); break;
			case 0x98 : util::stream_format(stream, "jz %s",as); break;
			case 0x99 : case 0x9a : case 0x9b :
						util::stream_format(stream, "jz %s(%d)",as,ptr); break;
			case 0x9c : util::stream_format(stream, "jnz %s",as); break;
			case 0x9d : case 0x9e : case 0x9f :
						util::stream_format(stream, "jnz %s(%d)",as,ptr); break;
			// Double-Byte Miscellaneous Instructions
			case 0x8f:  util::stream_format(stream, "dly $%02x",arg); break;
			// Others are illegal
			default : util::stream_format(stream, "illegal"); pc--; break; // Illegal we consider without param
		}
	} else {
		// one byte instructions
		switch (op)
		{
			// Extension Register Instructions
			case 0x40:  util::stream_format(stream, "lde"); break;
			case 0x01:  util::stream_format(stream, "xae"); break;
			case 0x50:  util::stream_format(stream, "ane"); break;
			case 0x58:  util::stream_format(stream, "ore"); break;
			case 0x60:  util::stream_format(stream, "xre"); break;
			case 0x68:  util::stream_format(stream, "dae"); break;
			case 0x70:  util::stream_format(stream, "ade"); break;
			case 0x78:  util::stream_format(stream, "cae"); break;
			// Pointer Register Move Instructions
			case 0x30:  case 0x31 :case 0x32: case 0x33:
						util::stream_format(stream, "xpal %d",ptr); break;
			case 0x34:  case 0x35 :case 0x36: case 0x37:
						util::stream_format(stream, "xpah %d",ptr); break;
			case 0x3c:  case 0x3d :case 0x3e: case 0x3f:
						util::stream_format(stream, "xppc %d",ptr); break;
			// Shift, Rotate, Serial I/O Instructions
			case 0x19:  util::stream_format(stream, "sio"); break;
			case 0x1c:  util::stream_format(stream, "sr"); break;
			case 0x1d:  util::stream_format(stream, "srl"); break;
			case 0x1e:  util::stream_format(stream, "rr"); break;
			case 0x1f:  util::stream_format(stream, "rrl"); break;
			// Single Byte Miscellaneous Instructions
			case 0x00:  util::stream_format(stream, "halt"); break;
			case 0x02:  util::stream_format(stream, "ccl"); break;
			case 0x03:  util::stream_format(stream, "scl"); break;
			case 0x04:  util::stream_format(stream, "dint"); break;
			case 0x05:  util::stream_format(stream, "ien"); break;
			case 0x06:  util::stream_format(stream, "csa"); break;
			case 0x07:  util::stream_format(stream, "cas"); break;
			case 0x08:  util::stream_format(stream, "nop"); break;
			// Others are illegal
			default : util::stream_format(stream, "illegal"); break;
		}
	}

	return (pc - PC);
}
