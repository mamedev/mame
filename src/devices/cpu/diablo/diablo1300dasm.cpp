// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
// thanks-to: Jeff Laughton
/*
  Diablo 1300 series Printer TTL CPU disassembler
  The work is based on the RE done by Jeff Laughton http://laughtonelectronics.com/Arcana/Diablo%20CPU/DiabloCPU.html
*/

#include "emu.h"
#include "diablo1300dasm.h"

u32 diablo1300_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t diablo1300_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint16_t op = opcodes.r16(pc);
	uint32_t flags = 0;

	switch (op & 0x0007)
	{
	case 0: // OUTPUT Dport, Sreg
		util::stream_format(stream, "OUTPUT dv%d, r%02X",
					(op & 0x0070) >> 4,
					((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
		break;
	case 1: // JNC Addr
		util::stream_format(stream, "JNC    %03X", ((op & 0xff00) >> 8) + ((op & 0x0008) ? 0x100 : 0));
		flags = STEP_COND;
		break;
	case 2: // RST #Dport
		util::stream_format(stream, "RST    dv%d", (op & 0x0700) >> 8);
		break;
	case 3: // LDBBIT Sreg, #val
		util::stream_format(stream, "LDBBIT r%02X, %02X",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					(op & 0xff00) >> 8);
		break;
	case 4: //
		switch (op & 0xc000)
		{
		case 0x4000: // XLAT Dreg
			util::stream_format(stream, "XLAT   r%02X",
						((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0));
			break;
		case 0xc000: // MOVCPL Dreg, Sreg
			util::stream_format(stream, "MOVCPL r%02X, r%02X",
						((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
						((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
			break;
		case 0x8000: // INPUT Dreg, Sport
			util::stream_format(stream, "INPUT  r%02X, dv%X",
						((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
						((op & 0x0f00) >> 8));
			break;
		default:
			util::stream_format(stream, "???");
			break;
		}
		break;
	case 5: // LOAD# Dreg,#val
		util::stream_format(stream, "LOAD#  r%02X, %02X",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					(op & 0xff00) >> 8);
		break;
	case 6: // ADCCPL S/Dreg, Sreg
		util::stream_format(stream, "ADCCPL r%02X, r%02X",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
		break;
	case 7: // ADC# S/Dreg, val
		util::stream_format(stream, "ADC#   r%02X, %02X",
					((op & 0x00f0) >> 4) + ((op & 0x0008) ? 0x10 : 0),
					(op & 0xff00) >> 8);
		break;
	default:
		util::stream_format(stream, "???");
		break;
	}

	return 1 | flags | SUPPORTED;
}
