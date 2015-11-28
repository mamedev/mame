// license:GPL2+
// copyright-holders:Felipe Sanches
#include "emu.h"
#include "cpu/patinhofeio/patinho_feio.h"

CPU_DISASSEMBLE( patinho_feio )
{
//	int md;
//	int x;
//	md = oprom[1] << 8 | oprom[0];
//	x = md & 0xFFF;
    
    //single-opcode instructions
	switch (oprom[0])
	{
    case 0x80:
        sprintf (buffer, "LIMPO");
        break;
    case 0x85:
        sprintf (buffer, "INC");
        break;
    case 0x96:
        sprintf (buffer, "SV 1");
        break;
    case 0x99:
        sprintf (buffer, "TRE");
        break;
    case 0x9D:
        sprintf (buffer, "PARE");
        break;
    default:
		sprintf (buffer, "illegal");
		break;
	}
	return 1;
}
