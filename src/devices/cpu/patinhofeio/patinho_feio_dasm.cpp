// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#include "emu.h"
#include "patinho_feio_dasm.h"

u32 patinho_feio_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t patinho_feio_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int addr, value, n, f;

	switch (opcodes.r8(pc) & 0xF0) {
		case 0x00:
			//PLA = "Pula": Unconditionally JUMP to effective address
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "PLA     /%03X", addr);
			return 2;
		case 0x10:
			//PLAX = "Pulo indexado": Unconditionally JUMP to indexed address
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "PLAX    (IDX) + /%03X", addr);
			return 2;
		case 0x20:
			//ARM = "Armazena": Stores the contents of the
			//                  accumulator in the given 12bit address
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			if (addr==0){
				util::stream_format(stream, "ARM     (IDX)");
			}else{
				util::stream_format(stream, "ARM     /%03X", addr);
			}
			return 2;
		case 0x30:
			//ARMX = "Armazenamento indexado": Stores the contents of the accumulator in the
			//                                 given 12bit address (indexed by IDX)
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "ARMX    (IDX) + /%03X", addr);
			return 2;
		case 0x40:
			//CAR = "Carrega": Loads the contents of the given 12bit address
			//                 into the accumulator
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			if (addr==0){
				util::stream_format(stream, "CAR     (IDX)");
			}else{
				util::stream_format(stream, "CAR     /%03X", addr);
			}
			return 2;
		case 0x50:
			//CARX = "Carga indexada": Loads the contents of the given 12bit address
			//                         (indexed by IDX) into the accumulator
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "CARX    (IDX) + /%03X", addr);
			return 2;
		case 0x60:
			//SOM = "Soma": Adds the contents of the given 12bit address
			//              into the accumulator
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "SOM     /%03X", addr);
			return 2;
		case 0x70:
			//SOMX = "Soma indexada": Adds the contents of the given 12bit address
			//                        (indexed by IDX) into the accumulator
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "SOMX    (IDX) + /%03X", addr);
			return 2;
		case 0xA0:
			//PLAN = "Pula se ACC for negativo": Jumps to the 12bit address
			//                                   if the accumulator is negative
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "PLAN    /%03X", addr);
			return 2;
		case 0xB0:
			//PLAZ = "Pula se ACC for zero": Jumps to the 12bit address
			//                               if the accumulator is zero
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "PLAZ    /%03X", addr);
			return 2;
		case 0xC0:
			n = (opcodes.r8(pc) & 0x0F);
			f = (opcodes.r8(pc+1) & 0x0F);
			n+= (n < 10) ? '0' : 'A'-10;
			f+= (f < 10) ? '0' : 'A'-10;
			switch(opcodes.r8(pc+1) & 0xF0)
			{
				case 0x10: util::stream_format(stream, "FNC     /%c%c", n, f); return 2;
				case 0x20: util::stream_format(stream, "SAL     /%c%c", n, f); return 2;
				case 0x40: util::stream_format(stream, "ENTR    /%c0", n); return 2;
				case 0x80: util::stream_format(stream, "SAI     /%c0", n); return 2;
			}
			break;
		case 0xD0:
			value = opcodes.r8(pc+1) & 0x0F;
			switch (opcodes.r8(pc) & 0x0F)
			{
				case 0x01:
					switch (opcodes.r8(pc+1) & 0xF0)
					{
						case 0x00: util::stream_format(stream, "DD      /%01X", value); return 2; //DD = "Deslocamento para a direita": Shift right
						case 0x10: util::stream_format(stream, "DDV     /%01X", value); return 2; //DDV = "Deslocamento para a direita c/ V": Shift right with carry
						case 0x20: util::stream_format(stream, "GD      /%01X", value); return 2; //GD = "Giro para a direita": Rotate right
						case 0x30: util::stream_format(stream, "GDV     /%01X", value); return 2; //GDV = "Giro para a direita c/ V": Rotate right with carry
						case 0x40: util::stream_format(stream, "DE      /%01X", value); return 2; //DE = "Deslocamento para a esquerda": Shift right
						case 0x50: util::stream_format(stream, "DEV     /%01X", value); return 2; //DEV = "Deslocamento para a esquerda c/ V": Shift right with carry
						case 0x60: util::stream_format(stream, "GE      /%01X", value); return 2; //GE = "Giro para a esquerda": Rotate right
						case 0x70: util::stream_format(stream, "GEV     /%01X", value); return 2; //GEV = "Giro para a esquerda c/ V": Rotate right with carry
						case 0x80: util::stream_format(stream, "DDS     /%01X", value); return 2; //DDS = "Deslocamento para a direita com duplicacao de sinal": Shift right with sign duplication
					}
					break;
				case 0x02: util::stream_format(stream, "XOR     /%02X", opcodes.r8(pc+1)); return 2; //Logical XOR
				case 0x04: util::stream_format(stream, "NAND    /%02X", opcodes.r8(pc+1)); return 2; //Logical NAND
				case 0x08: util::stream_format(stream, "SOMI    /%02X", opcodes.r8(pc+1)); return 2; //SOMI = "Soma imediata": Add immediate value into accumulator
				case 0x0A: util::stream_format(stream, "CARI    /%02X", opcodes.r8(pc+1)); return 2; //CARI = "Carrega imediato": Loads an immediate value into the accumulator
			}
			break;
		case 0xE0:
			//SUS = "Subtrai um ou salta"
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "SUS     /%03X", addr);
			return 2;
		case 0xF0:
			//PUG = "Pula e guarda"
			addr = (opcodes.r8(pc) & 0x0F) << 8 | opcodes.r8(pc+1);
			util::stream_format(stream, "PUG     /%03X", addr);
			return 2;
	}

	switch (opcodes.r8(pc)) {
		case 0x80: util::stream_format(stream, "LIMPO");       return 1;
		case 0x81: util::stream_format(stream, "UM");          return 1;
		case 0x82: util::stream_format(stream, "CMP1");        return 1;
		case 0x83: util::stream_format(stream, "CMP2");        return 1;
		case 0x84: util::stream_format(stream, "LIN");         return 1;
		case 0x85: util::stream_format(stream, "INC");         return 1;
		case 0x86: util::stream_format(stream, "UNEG");        return 1;
		case 0x87: util::stream_format(stream, "LIMP1");       return 1;
		case 0x88: util::stream_format(stream, "PNL     0");   return 1;
		case 0x89: util::stream_format(stream, "PNL     1");   return 1;
		case 0x8A: util::stream_format(stream, "PNL     2");   return 1;
		case 0x8B: util::stream_format(stream, "PNL     3");   return 1;
		case 0x8C: util::stream_format(stream, "PNL     4");   return 1;
		case 0x8D: util::stream_format(stream, "PNL     5");   return 1;
		case 0x8E: util::stream_format(stream, "PNL     6");   return 1;
		case 0x8F: util::stream_format(stream, "PNL     7");   return 1;
		case 0x90: util::stream_format(stream, "ST      0");   return 1;
		case 0x91: util::stream_format(stream, "STM     0");   return 1;
		case 0x92: util::stream_format(stream, "ST      1");   return 1;
		case 0x93: util::stream_format(stream, "STM     1");   return 1;
		case 0x94: util::stream_format(stream, "SV      0");   return 1;
		case 0x95: util::stream_format(stream, "SVM     0");   return 1;
		case 0x96: util::stream_format(stream, "SV      1");   return 1;
		case 0x97: util::stream_format(stream, "SVM     1");   return 1;
		case 0x98: util::stream_format(stream, "PUL");         return 1;
		case 0x99: util::stream_format(stream, "TRE");         return 1;
		case 0x9A: util::stream_format(stream, "INIB");        return 1;
		case 0x9B: util::stream_format(stream, "PERM");        return 1;
		case 0x9C: util::stream_format(stream, "ESP");         return 1;
		case 0x9D: util::stream_format(stream, "PARE");        return 1;
		case 0x9E: util::stream_format(stream, "TRI");         return 1;
		case 0x9F: util::stream_format(stream, "IND");         return 1;
	}

	util::stream_format(stream, "illegal instruction");
	return 1;
}
