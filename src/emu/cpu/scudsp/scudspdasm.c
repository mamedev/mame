#include "emu.h"
#include "debugger.h"
#include "scudsp.h"

CPU_DISASSEMBLE( scudsp )
{
	UINT32 op = oprom[0]<<24|oprom[1]<<16|oprom[2]<<8|oprom[3]<<0;
	unsigned size = 1;
//  const char *sym, *sym2;

	switch( op >> 30 )
	{
		case 0:
			sprintf(buffer, "ALU OP");
			break;
		case 2:
			sprintf(buffer, "MVI");
			break;
		case 3:
			switch((op >> 28) & 3)
			{
				case 0:
					sprintf(buffer, "DMA");
					break;
				case 1:
					sprintf(buffer, "JMP");
					break;
				case 2:
					sprintf(buffer, op & 0x8000000 ? "LPS" : "BTM");
					break;
				case 3:
					sprintf(buffer, op & 0x8000000 ? "ENDI" : "END");
					break;
			}
			break;

		default:
			sprintf(buffer, "???");
			break;
	}

	return size;
}
