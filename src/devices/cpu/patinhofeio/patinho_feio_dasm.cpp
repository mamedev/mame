// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#include "emu.h"
#include "cpu/patinhofeio/patinho_feio.h"

CPU_DISASSEMBLE( patinho_feio )
{
	int addr, value, n, f;

	switch (oprom[0] & 0xF0)
	{
		case 0x00:
			//PLA = "Pula": Unconditionally JUMP to effective address
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "PLA     /%03X", addr);
			return 2;
		case 0x10:
			//PLAX = "Pulo indexado": Unconditionally JUMP to indexed address
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "PLAX    (IDX) + /%03X", addr);
			return 2;
		case 0x20:
			//ARM = "Armazena": Stores the contents of the
			//                  accumulator in the given 12bit address
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			if (addr==0){
				sprintf (buffer, "ARM     (IDX)");
			}else{
				sprintf (buffer, "ARM     /%03X", addr);
			}
			return 2;
		case 0x30:
			//ARMX = "Armazenamento indexado": Stores the contents of the accumulator in the
			//                                 given 12bit address (indexed by IDX)
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "ARMX    (IDX) + /%03X", addr);
			return 2;
		case 0x40:
			//CAR = "Carrega": Loads the contents of the given 12bit address
			//                 into the accumulator
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			if (addr==0){
				sprintf (buffer, "CAR     (IDX)");
			}else{
				sprintf (buffer, "CAR     /%03X", addr);
			}
			return 2;
		case 0x50:
			//CARX = "Carga indexada": Loads the contents of the given 12bit address
			//                         (indexed by IDX) into the accumulator
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "CARX    (IDX) + /%03X", addr);
			return 2;
		case 0x60:
			//SOM = "Soma": Adds the contents of the given 12bit address
			//              into the accumulator
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "SOM     /%03X", addr);
			return 2;
		case 0x70:
			//SOMX = "Soma indexada": Adds the contents of the given 12bit address
			//                        (indexed by IDX) into the accumulator
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "SOMX    (IDX) + /%03X", addr);
			return 2;
		case 0xA0:
			//PLAN = "Pula se ACC for negativo": Jumps to the 12bit address
			//                                   if the accumulator is negative
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "PLAN    /%03X", addr);
			return 2;
		case 0xB0:
			//PLAZ = "Pula se ACC for zero": Jumps to the 12bit address
			//                               if the accumulator is zero
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "PLAZ    /%03X", addr);
			return 2;
		case 0xC0:
			n = (oprom[0] & 0x0F);
			f = (oprom[1] & 0x0F);
			n+= (n < 10) ? '0' : 'A'-10;
			f+= (f < 10) ? '0' : 'A'-10;
			switch(oprom[1] & 0xF0)
			{
				case 0x10: sprintf (buffer, "FNC     /%c%c", n, f); return 2;
				case 0x20: sprintf (buffer, "SAL     /%c%c", n, f); return 2;
				case 0x40: sprintf (buffer, "ENTR    /%c0", n); return 2;
				case 0x80: sprintf (buffer, "SAI     /%c0", n); return 2;
			}
			break;
		case 0xD0:
			value = oprom[1] & 0x0F;
			switch (oprom[0] & 0x0F)
			{
				case 0x01:
					switch (oprom[1] & 0xF0)
					{
						case 0x00: sprintf (buffer, "DD      /%01X", value); return 2; //DD = "Deslocamento para a direita": Shift right
						case 0x10: sprintf (buffer, "DDV     /%01X", value); return 2; //DDV = "Deslocamento para a direita c/ V": Shift right with carry
						case 0x20: sprintf (buffer, "GD      /%01X", value); return 2; //GD = "Giro para a direita": Rotate right
						case 0x30: sprintf (buffer, "GDV     /%01X", value); return 2; //GDV = "Giro para a direita c/ V": Rotate right with carry
						case 0x40: sprintf (buffer, "DE      /%01X", value); return 2; //DE = "Deslocamento para a esquerda": Shift right
						case 0x50: sprintf (buffer, "DEV     /%01X", value); return 2; //DEV = "Deslocamento para a esquerda c/ V": Shift right with carry
						case 0x60: sprintf (buffer, "GE      /%01X", value); return 2; //GE = "Giro para a esquerda": Rotate right
						case 0x70: sprintf (buffer, "GEV     /%01X", value); return 2; //GEV = "Giro para a esquerda c/ V": Rotate right with carry
						case 0x80: sprintf (buffer, "DDS     /%01X", value); return 2; //DDS = "Deslocamento para a direita com duplicacao de sinal": Shift right with sign duplication
					}
					break;
				case 0x02: sprintf (buffer, "XOR     /%02X", oprom[1]); return 2; //Logical XOR
				case 0x04: sprintf (buffer, "NAND    /%02X", oprom[1]); return 2; //Logical NAND
				case 0x08: sprintf (buffer, "SOMI    /%02X", oprom[1]); return 2; //SOMI = "Soma imediata": Add immediate value into accumulator
				case 0x0A: sprintf (buffer, "CARI    /%02X", oprom[1]); return 2; //CARI = "Carrega imediato": Loads an immediate value into the accumulator
			}
			break;
		case 0xE0:
			//SUS = "Subtrai um ou salta"
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "SUS     /%03X", addr);
			return 2;
		case 0xF0:
			//PUG = "Pula e guarda"
			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			sprintf (buffer, "PUG     /%03X", addr);
			return 2;
	}

	switch (oprom[0])
	{
		case 0x80: sprintf (buffer, "LIMPO");       return 1;
		case 0x81: sprintf (buffer, "UM");          return 1;
		case 0x82: sprintf (buffer, "CMP1");        return 1;
		case 0x83: sprintf (buffer, "CMP2");        return 1;
		case 0x84: sprintf (buffer, "LIN");         return 1;
		case 0x85: sprintf (buffer, "INC");         return 1;
		case 0x86: sprintf (buffer, "UNEG");        return 1;
		case 0x87: sprintf (buffer, "LIMP1");       return 1;
		case 0x88: sprintf (buffer, "PNL     0");   return 1;
		case 0x89: sprintf (buffer, "PNL     1");   return 1;
		case 0x8A: sprintf (buffer, "PNL     2");   return 1;
		case 0x8B: sprintf (buffer, "PNL     3");   return 1;
		case 0x8C: sprintf (buffer, "PNL     4");   return 1;
		case 0x8D: sprintf (buffer, "PNL     5");   return 1;
		case 0x8E: sprintf (buffer, "PNL     6");   return 1;
		case 0x8F: sprintf (buffer, "PNL     7");   return 1;
		case 0x90: sprintf (buffer, "ST      0");   return 1;
		case 0x91: sprintf (buffer, "STM     0");   return 1;
		case 0x92: sprintf (buffer, "ST      1");   return 1;
		case 0x93: sprintf (buffer, "STM     1");   return 1;
		case 0x94: sprintf (buffer, "SV      0");   return 1;
		case 0x95: sprintf (buffer, "SVM     0");   return 1;
		case 0x96: sprintf (buffer, "SV      1");   return 1;
		case 0x97: sprintf (buffer, "SVM     1");   return 1;
		case 0x98: sprintf (buffer, "PUL");         return 1;
		case 0x99: sprintf (buffer, "TRE");         return 1;
		case 0x9A: sprintf (buffer, "INIB");        return 1;
		case 0x9B: sprintf (buffer, "PERM");        return 1;
		case 0x9C: sprintf (buffer, "ESP");         return 1;
		case 0x9D: sprintf (buffer, "PARE");        return 1;
		case 0x9E: sprintf (buffer, "TRI");         return 1;
		case 0x9F: sprintf (buffer, "IND");         return 1;
	}

	sprintf (buffer, "illegal instruction");
	return 1;
}
