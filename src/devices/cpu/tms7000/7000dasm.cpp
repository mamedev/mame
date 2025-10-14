// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/*

  TMS7000 disassembler

*/

#include "emu.h"
#include "7000dasm.h"

const tms7000_disassembler::oprandinfo tms7000_disassembler::of[] = {
/* 00 */ { {" B,A",     "",         "",         ""},        {NONE, DONE, DONE, DONE} },
/* 01 */ { {" R%u",     ",A",       "",         ""},        {UI8, NONE, DONE, DONE} },
/* 02 */ { {" R%u",     ",B",       "",         ""},        {UI8, NONE, DONE, DONE} },
/* 03 */ { {" R%u",     ",R%u",     "",         ""},        {UI8, UI8, DONE, DONE} },
/* 04 */ { {" %%>%X",   ",A",       "",         ""},        {UI8, NONE, DONE, DONE} },
/* 05 */ { {" %%>%X",   ",B",       "",         ""},        {UI8, NONE, DONE, DONE} },
/* 06 */ { {" %%>%X",   ",R%u",     "",         ""},        {UI8, UI8, DONE, DONE} },

/* 07 */ { {" A",       ",P%u",     "",         ""},        {NONE,UI8,DONE,DONE} },
/* 08 */ { {" B",       ",P%u",     "",         ""},        {NONE,UI8,DONE,DONE} },
/* 09 */ { {" %%>%02X", ",P%u",     "",         ""},        {UI8,UI8,DONE,DONE} },

/* 10 */ { {" @>%04X",  "",         "",         ""},        {UI16,DONE,DONE,DONE} },
/* 11 */ { {" R%u",     "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 12 */ { {" @>%04X(B)","",        "",         ""},        {UI16, DONE, DONE, DONE} },

/* 13 */ { {" B,A,%s",  "",         "",         ""},        {PCREL, DONE, DONE, DONE} },
/* 14 */ { {" R%u,A",   ",%s",      "",         ""},        {UI8, PCREL, DONE, DONE} },
/* 15 */ { {" R%u,B",   ",%s",      "",         ""},        {UI8, PCREL, DONE, DONE} },
/* 16 */ { {" R%u",     ",R%u",     ",%s",      ""},        {UI8, UI8, PCREL, DONE} },
/* 17 */ { {" %%>%X",   ",A,%s",    "",         ""},        {UI8, PCREL, DONE, DONE} },
/* 18 */ { {" %%>%X",   ",B,%s",    "",         ""},        {UI8, PCREL, DONE, DONE} },
/* 19 */ { {" %%>%X",   ",R%u",     ",%s",      ""},        {UI8, UI8, PCREL, DONE} },

/* 20 */ { {" A,P%u",   ",%s",      "",         ""},        {UI8, PCREL, DONE, DONE} },
/* 21 */ { {" B,P%u",   ",%s",      "",         ""},        {UI8, PCREL, DONE, DONE} },
/* 22 */ { {" %%>%02X", ",P%u",     ",%s"       ""},        {UI8, UI8, PCREL, DONE} },

/* 23 */ { {"",         "",         "",         ""},        {DONE, DONE, DONE, DONE} },
/* 24 */ { {" R%u",     "",         "",         ""},        {UI8, DONE, DONE, DONE} },

/* 25 */ { {" A,%s",    "",         "",         ""},        {PCREL, DONE, DONE, DONE} },
/* 26 */ { {" B,%s",    "",         "",         ""},        {PCREL, DONE, DONE, DONE} },
/* 27 */ { {" R%u",     ",%s",      "",         ""},        {UI8, PCREL, DONE, DONE} },

/* 28 */ { {" %s",      "",         "",         ""},        {PCREL, DONE, DONE, DONE} },

/* 29 */ { {" A,B",     "",         "",         ""},        {NONE, DONE, DONE, DONE} },
/* 30 */ { {" B,A",     "",         "",         ""},        {NONE, DONE, DONE, DONE} },
/* 31 */ { {" A,R%u",   "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 32 */ { {" B,R%u",   "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 33 */ { {" R%u,A",   "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 34 */ { {" R%u,B",   "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 35 */ { {" R%u",     ",R%u",     "",         ""},        {UI8, UI8, DONE, DONE} },
/* 36 */ { {" %%>%X,A", "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 37 */ { {" %%>%X,B", "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 38 */ { {" %%>%X",   ",R%u",     "",         ""},        {UI8, UI8, DONE, DONE} },

/* 39 */ { {" %%>%04X",",R%u",      "",         ""},        {UI16, UI8, DONE, DONE} },
/* 40 */ { {" %%>%04X(B)",",R%u",   "",         ""},        {UI16, UI8, DONE, DONE} },

/* 41 */ { {" P%u,A",   "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 42 */ { {" P%u,B",   "",         "",         ""},        {UI8, DONE, DONE, DONE} },
/* 43 */ { {" %s",      "",         "",         ""},        {PCABS, DONE, DONE, DONE} },
/* 44 */ { {"",         "",         "",         ""},        {TRAP, DONE, DONE, DONE} }, /* Only For TRAP */

/* 45 */ { {" *R%u",    "",         "",         ""},        {UI8, DONE, DONE, DONE} }
};

const tms7000_disassembler::tms7000_opcodeinfo tms7000_disassembler::opcs[] = {
	{0x69, "ADC", 0, 0 },
	{0x19, "ADC", 1, 0 },
	{0x39, "ADC", 2, 0 },
	{0x49, "ADC", 3, 0 },
	{0x29, "ADC", 4, 0 },
	{0x59, "ADC", 5, 0 },
	{0x79, "ADC", 6, 0 },

	{0x68, "ADD", 0, 0 },
	{0x18, "ADD", 1, 0 },
	{0x38, "ADD", 2, 0 },
	{0x48, "ADD", 3, 0 },
	{0x28, "ADD", 4, 0 },
	{0x58, "ADD", 5, 0 },
	{0x78, "ADD", 6, 0 },

	{0x63, "AND", 0, 0 },
	{0x13, "AND", 1, 0 },
	{0x33, "AND", 2, 0 },
	{0x43, "AND", 3, 0 },
	{0x23, "AND", 4, 0 },
	{0x53, "AND", 5, 0 },
	{0x73, "AND", 6, 0 },

	{0x83, "ANDP", 7, 0 },
	{0x93, "ANDP", 8, 0 },
	{0xA3, "ANDP", 9, 0 },

	{0x8C, "BR", 43, 0 },
	{0x9C, "BR", 45, 0 },
	{0xAC, "BR", 12, 0 },

	{0x66, "BTJO", 13, STEP_COND },
	{0x16, "BTJO", 14, STEP_COND },
	{0x36, "BTJO", 15, STEP_COND },
	{0x46, "BTJO", 16, STEP_COND },
	{0x26, "BTJO", 17, STEP_COND },
	{0x56, "BTJO", 18, STEP_COND },
	{0x76, "BTJO", 19, STEP_COND },

	{0x86, "BTJOP", 20, STEP_COND },
	{0x96, "BTJOP", 21, STEP_COND },
	{0xA6, "BTJOP", 22, STEP_COND },

	{0x67, "BTJZ", 13, STEP_COND },
	{0x17, "BTJZ", 14, STEP_COND },
	{0x37, "BTJZ", 15, STEP_COND },
	{0x47, "BTJZ", 16, STEP_COND },
	{0x27, "BTJZ", 17, STEP_COND },
	{0x57, "BTJZ", 18, STEP_COND },
	{0x77, "BTJZ", 19, STEP_COND },

	{0x87, "BTJZP", 20, STEP_COND },
	{0x97, "BTJZP", 21, STEP_COND },
	{0xA7, "BTJZP", 22, STEP_COND },

	{0x8E, "CALL", 43, STEP_OVER },
	{0x9E, "CALL", 45, STEP_OVER },
	{0xAE, "CALL", 12, STEP_OVER },

	{0xB5, "CLR A", 23, 0 },
	{0xC5, "CLR B", 23, 0 },
	{0xD5, "CLR", 24, 0 },

	{0xB0, "CLRC", 23, 0 },

	{0x6D, "CMP", 0, 0 },
	{0x1D, "CMP", 1, 0 },
	{0x3D, "CMP", 2, 0 },
	{0x4D, "CMP", 3, 0 },
	{0x2D, "CMP", 4, 0 },
	{0x5D, "CMP", 5, 0 },
	{0x7D, "CMP", 6, 0 },

	{0x8D, "CMPA", 10, 0 },
	{0x9D, "CMPA", 45, 0 },
	{0xAD, "CMPA", 12, 0 },

	{0x6E, "DAC", 0, 0 },
	{0x1E, "DAC", 1, 0 },
	{0x3E, "DAC", 2, 0 },
	{0x4E, "DAC", 3, 0 },
	{0x2E, "DAC", 4, 0 },
	{0x5E, "DAC", 5, 0 },
	{0x7E, "DAC", 6, 0 },

	{0xB2, "DEC A", 23, 0 },
	{0xC2, "DEC B", 23, 0 },
	{0xD2, "DEC", 24, 0 },

	{0xBB, "DECD A", 23, 0 },
	{0xCB, "DECD B", 23, 0 },
	{0xDB, "DECD", 24, 0 },

	{0x06, "DINT", 23, 0 },

	{0xBA, "DJNZ", 25, STEP_COND },
	{0xCA, "DJNZ", 26, STEP_COND },
	{0xDA, "DJNZ", 27, STEP_COND },

	{0x6F, "DSB", 0, 0 },
	{0x1F, "DSB", 1, 0 },
	{0x3F, "DSB", 2, 0 },
	{0x4F, "DSB", 3, 0 },
	{0x2F, "DSB", 4, 0 },
	{0x5F, "DSB", 5, 0 },
	{0x7F, "DSB", 6, 0 },

	{0x05, "EINT", 23, 0 },

	{0x01, "IDLE", 23, 0 },

	{0xB3, "INC A", 23, 0 },
	{0xC3, "INC B", 23, 0 },
	{0xD3, "INC", 24, 0 },

	{0xB4, "INV A", 23, 0 },
	{0xC4, "INV B", 23, 0 },
	{0xD4, "INV", 24, 0 },

	{0xE2, "JEQ", 28, STEP_COND },
	{0xE3, "JHS", 28, STEP_COND },
	{0xE7, "JL", 28, STEP_COND },
	{0xE0, "JMP", 28, 0 },
	{0xE1, "JN", 28, STEP_COND },
	{0xE6, "JNZ", 28, STEP_COND },
	{0xE4, "JP", 28, STEP_COND },
	{0xE5, "JPZ", 28, STEP_COND },

	{0x8A, "LDA", 10, 0 },
	{0x9A, "LDA", 45, 0 },
	{0xAA, "LDA", 12, 0 },

	{0x0D, "LDSP", 23, 0 },

	{0xC0, "MOV", 29, 0 },
	{0x62, "MOV", 30, 0 },
	{0xD0, "MOV", 31, 0 },
	{0xD1, "MOV", 32, 0 },
	{0x12, "MOV", 33, 0 },
	{0x32, "MOV", 34, 0 },
	{0x42, "MOV", 35, 0 },
	{0x22, "MOV", 36, 0 },
	{0x52, "MOV", 37, 0 },
	{0x72, "MOV", 38, 0 },

	{0x88, "MOVD", 39, 0 },
	{0x98, "MOVD", 35, 0 },
	{0xA8, "MOVD", 40, 0 },

	{0x82, "MOVP", 7, 0 },
	{0x92, "MOVP", 8, 0 },
	{0xA2, "MOVP", 9, 0 },
	{0x80, "MOVP", 41, 0 },
	{0x91, "MOVP", 42, 0 },

	{0x6C, "MPY", 00, 0 },
	{0x1C, "MPY", 01, 0 },
	{0x3C, "MPY", 02, 0 },
	{0x4C, "MPY", 03, 0 },
	{0x2C, "MPY", 04, 0 },
	{0x5C, "MPY", 05, 0 },
	{0x7C, "MPY", 06, 0 },

	{0x00, "NOP", 23, 0 },

	{0x64, "OR", 00, 0 },
	{0x14, "OR", 01, 0 },
	{0x34, "OR", 02, 0 },
	{0x44, "OR", 03, 0 },
	{0x24, "OR", 04, 0 },
	{0x54, "OR", 05, 0 },
	{0x74, "OR", 06, 0 },

	{0x84, "ORP", 7, 0 },
	{0x94, "ORP", 8, 0 },
	{0xA4, "ORP", 9, 0 },

	{0xB9, "POP A", 23, 0 },
	{0xC9, "POP B", 23, 0 },
	{0xD9, "POP", 24, 0 },
	{0x08, "POP ST", 23, 0 },

	{0xB8, "PUSH A", 23, 0 },
	{0xC8, "PUSH B", 23, 0 },
	{0xD8, "PUSH", 24, 0 },
	{0x0E, "PUSH ST", 23, 0 },

	{0x0B, "RETI", 23, STEP_OUT },
	{0x0A, "RETS", 23, STEP_OUT },

	{0xBE, "RL A", 23, 0 },
	{0xCE, "RL B", 23, 0 },
	{0xDE, "RL", 11, 0 },

	{0xBF, "RLC A", 23, 0 },
	{0xCF, "RLC B", 23, 0 },
	{0xDF, "RLC", 11, 0 },

	{0xBC, "RR A", 23, 0 },
	{0xCC, "RR B", 23, 0 },
	{0xDC, "RR", 11, 0 },

	{0xBD, "RRC A", 23, 0 },
	{0xCD, "RRC B", 23, 0 },
	{0xDD, "RRC", 11, 0 },

	{0x6B, "SBB", 0, 0 },
	{0x1B, "SBB", 1, 0 },
	{0x3B, "SBB", 2, 0 },
	{0x4B, "SBB", 3, 0 },
	{0x2B, "SBB", 4, 0 },
	{0x5B, "SBB", 5, 0 },
	{0x7B, "SBB", 6, 0 },

	{0x07, "SETC", 23, 0 },

	{0x8B, "STA", 10, 0 },
	{0x9B, "STA", 45, 0 },
	{0xAB, "STA", 12, 0 },

	{0x09, "STSP", 23, 0 },

	{0x6A, "SUB", 0, 0 },
	{0x1A, "SUB", 1, 0 },
	{0x3A, "SUB", 2, 0 },
	{0x4A, "SUB", 3, 0 },
	{0x2A, "SUB", 4, 0 },
	{0x5A, "SUB", 5, 0 },
	{0x7A, "SUB", 6, 0 },

	{0xFF, "TRAP 0", 44, STEP_OVER },
	{0xFE, "TRAP 1", 44, STEP_OVER },
	{0xFD, "TRAP 2", 44, STEP_OVER },
	{0xFC, "TRAP 3", 44, STEP_OVER },
	{0xFB, "TRAP 4", 44, STEP_OVER },
	{0xFA, "TRAP 5", 44, STEP_OVER },
	{0xF9, "TRAP 6", 44, STEP_OVER },
	{0xF8, "TRAP 7", 44, STEP_OVER },
	{0xF7, "TRAP 8", 44, STEP_OVER },
	{0xF6, "TRAP 9", 44, STEP_OVER },
	{0xF5, "TRAP 10", 44, STEP_OVER },
	{0xF4, "TRAP 11", 44, STEP_OVER },
	{0xF3, "TRAP 12", 44, STEP_OVER },
	{0xF2, "TRAP 13", 44, STEP_OVER },
	{0xF1, "TRAP 14", 44, STEP_OVER },
	{0xF0, "TRAP 15", 44, STEP_OVER },
	{0xEF, "TRAP 16", 44, STEP_OVER },
	{0xEE, "TRAP 17", 44, STEP_OVER },
	{0xED, "TRAP 18", 44, STEP_OVER },
	{0xEC, "TRAP 19", 44, STEP_OVER },
	{0xEB, "TRAP 20", 44, STEP_OVER },
	{0xEA, "TRAP 21", 44, STEP_OVER },
	{0xE9, "TRAP 22", 44, STEP_OVER },
	{0xE8, "TRAP 23", 44, STEP_OVER },

	{0xB7, "SWAP A", 23, 0 },
	{0xC7, "SWAP B", 23, 0 },
	{0xD7, "SWAP", 11, 0 },

	{0xB0, "TSTA", 23, 0 },
	{0xC1, "TSTB", 23, 0 },

	{0xB6, "XCHB A", 23, 0 },
	{0xC6, "XCHB B", 23, 0 },
	{0xD6, "XCHB", 11, 0 },

	{0x65, "XOR", 0, 0 },
	{0x15, "XOR", 1, 0 },
	{0x35, "XOR", 2, 0 },
	{0x45, "XOR", 3, 0 },
	{0x25, "XOR", 4, 0 },
	{0x55, "XOR", 5, 0 },
	{0x75, "XOR", 6, 0 },

	{0x85, "XORP", 7, 0 },
	{0x95, "XORP", 8, 0 },
	{0xA5, "XORP", 9, 0 },

	{0x00, "NOP", 23, 0 }
};

u32 tms7000_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t tms7000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int opcode, i/*, size = 1*/;
	offs_t pos = pc;

	opcode = opcodes.r8(pos++);

	for( i=0; i<sizeof(opcs) / sizeof(tms7000_opcodeinfo); i++ )
	{
		if( opcode == opcs[i].opcode )
		{
			/* We found a match */

			int j,k,vector;
			uint8_t   a;
			int8_t    b;
			uint16_t  c;
			int16_t   d;

			util::stream_format(stream, "%s", opcs[i].name);

			j=opcs[i].operand;

			for( k=0; k<4; k++ )
			{
				switch( of[j].decode[k] )
				{
					case DONE:
						break;
					case NONE:
						util::stream_format(stream, "%s", of[j].opstr[k]);
						break;
					case UI8:
						a = (uint8_t)params.r8(pos++);
						util::stream_format(stream, of[j].opstr[k], (unsigned int)a);
						break;
					case I8:
						b = (int8_t)params.r8(pos++);
						util::stream_format(stream, of[j].opstr[k], (int8_t)b);
						break;
					case UI16:
						c = (uint16_t)params.r16(pos);
						pos += 2;
						util::stream_format(stream, of[j].opstr[k], (unsigned int)c);
						break;
					case I16:
						d = (int16_t)params.r16(pos);
						pos += 2;
						util::stream_format(stream, of[j].opstr[k], (signed int)d);
						break;
					case PCREL:
						b = (int8_t)params.r8(pos++);
						util::stream_format(stream, of[j].opstr[k], util::string_format(">%04X", pc+2+k+b));
						break;
					case PCABS:
						c = (uint16_t)params.r16(pos);
						pos += 2;
						util::stream_format(stream, of[j].opstr[k], util::string_format(">%04X", c));
						break;
					case TRAP:
						vector = 0xffff - ((0xff - opcode) * 2);
						c = vector;
						break;
				}
			}
			return (pos - pc) | opcs[i].s_flag | SUPPORTED;
		}
	}

	/* No Match */
	stream << "Illegal Opcode";
	return (pos - pc) | SUPPORTED;
}
