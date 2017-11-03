// license:GPL-2.0+
// copyright-holders:Felipe Sanches
#include "emu.h"
#include "cpu/zezinho/zezinho_cpu.h"

CPU_DISASSEMBLE(zezinho)
{
	//int addr;
        //int value;

	switch (oprom[0] & 0xF0) {
		case 0x20: // Saída
			util::stream_format(stream, "SAIDA");
			return 2;
		case 0x30: // Armazena
//			addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "ARMAZENAR");
			return 2;
		case 0x40: // l - Transfira se Acc < 0
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "TRANSFIRA_SE_ACC_<_0");
			return 2;
		case 0x50: // k - Transfira se Acc > 0
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "TRANSFIRA_SE_ACC_>_0");
			return 2;
		case 0x60: // j - Transfira se Acc = 0
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "TRANSFIRA_SE_ACC_ZERO");
			return 2;
		case 0x70: // i - Transfira
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "TRANSFIRA");
			return 2;
		case 0x80: // h - Subtração inversa
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "SUBTRAÇÂO_INVERSA");
			return 2;
		case 0x90: // g - Divisão inversa
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "DIVISÃO_INVERSA");
			return 2;
		case 0xa0: // f - Divida
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "DIVIDA");
			return 2;
		case 0xb0: // e - Multiplique
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "MULTIPLIQUE");
			return 2;
		case 0xc0: // d - Subtrai
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "SUBTRAI");
			return 2;
		case 0xd0: // c - Limpa acumulador e subtrai
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "LIMPA_ACC_E_SUBTRAI");
			return 2;
		case 0xe0: // b - Soma
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "SOMA");
			return 2;
		case 0xf0: // a - Limpa acumulador e soma
			//
			//addr = (oprom[0] & 0x0F) << 8 | oprom[1];
			util::stream_format(stream, "LIMPA_ACC_E_SOMA");
			return 2;
	}

	util::stream_format(stream, "illegal instruction");
	return 1;
}
