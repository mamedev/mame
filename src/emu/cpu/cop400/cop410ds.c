/**************************************************************************
 *               National Semiconductor COP410 Emulator                   *
 *                                                                        *
 *                   Copyright (C) 2006 MAME Team                         *
 **************************************************************************/

#include "cpuintrf.h"

offs_t cop410_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int op;
	int cnt = 1;
	UINT16 addr;
	UINT8 op2;
	UINT32 flags = 0;

	op = oprom[0];

	if ((op >= 0x80 && op <= 0xBE) || (op >= 0xC0 && op <= 0xFE))
	{
		if ((pc & 0x3E0) >= 0x80 && (pc & 0x3E0) < 0x100) //JP pages 2,3
		{
			addr = (UINT16)((pc & 0x380) | (op & 0x7F));
			sprintf(buffer,"JP %x",addr);
		}
		else
		{
			if ((op & 0xC0) == 0xC0) //JP other pages
			{
				addr = (UINT16)((pc & 0x3C0) | (op & 0x3F));
				sprintf(buffer,"JP %x",addr);
			}
			else					//JSRP
			{
				addr = (UINT16)(0x80 | (op & 0x3F));
				sprintf(buffer,"JSRP %x",addr);
				flags = DASMFLAG_STEP_OVER;
			}
		}
	}
	else if (op >= 0x08 && op <= 0x0F)
	{
		sprintf(buffer,"LBI 0,%u",((op & 0xF) + 1) & 0xF);
	}
	else if (op >= 0x18 && op <= 0x1F)
	{
		sprintf(buffer,"LBI 1,%u",((op & 0xF) + 1) & 0xF);
	}
	else if (op >= 0x28 && op <= 0x2F)
	{
		sprintf(buffer,"LBI 2,%u",((op & 0xF) + 1) & 0xF);
	}
	else if (op >= 0x38 && op <= 0x3F)
	{
		sprintf(buffer,"LBI 3,%u",((op & 0xF) + 1) & 0xF);
	}
	else if (op >= 0x51 && op <= 0x5F)
	{
		sprintf(buffer,"AISC %u",op & 0xF);
	}
	else if (op >= 0x60 && op <= 0x61)
	{
		addr = ((op & 0x01) << 8) | oprom[1];
		sprintf(buffer,"JMP %x",addr);
		cnt = 2;
	}
	else if (op >= 0x68 && op <= 0x69)
	{
		addr = ((op & 0x01) << 8) | oprom[1];
		sprintf(buffer,"JSR %x",addr);
		flags = DASMFLAG_STEP_OVER;
		cnt = 2;
	}
	else if (op >= 0x70 && op <= 0x7F)
	{
		sprintf(buffer,"STII %u",op & 0xF);
	}
	else
	{
		switch (op)
		{
		case 0:
			sprintf(buffer,"CLRA");
			break;

		case 1:
			sprintf(buffer,"SKMBZ 0");
			break;

		case 2:
			sprintf(buffer,"XOR");
			break;

		case 3:
			sprintf(buffer,"SKMBZ 2");
			break;

		case 4:
			sprintf(buffer,"XIS 0");
			break;

		case 5:
			sprintf(buffer,"LD 0");
			break;

		case 6:
			sprintf(buffer,"X 0");
			break;

		case 7:
			sprintf(buffer,"XDS 0");
			break;

		case 0x11:
			sprintf(buffer,"SKMBZ 1");
			break;

		case 0x13:
			sprintf(buffer,"SKMBZ 3");
			break;

		case 0x14:
			sprintf(buffer,"XIS 1");
			break;

		case 0x15:
			sprintf(buffer,"LD 1");
			break;

		case 0x16:
			sprintf(buffer,"X 1");
			break;

		case 0x17:
			sprintf(buffer,"XDS 1");
			break;

		case 0x20:
			sprintf(buffer,"SKC");
			break;

		case 0x21:
			sprintf(buffer,"SKE");
			break;

		case 0x22:
			sprintf(buffer,"SC");
			break;

		case 0x23:
			addr = (UINT16)(oprom[1] & 0x3F);
			sprintf(buffer,"XAD %x,%x",((addr & 0x30) >> 4),addr & 0x0F);
			cnt = 2;
			break;

		case 0x24:
			sprintf(buffer,"XIS 2");
			break;

		case 0x25:
			sprintf(buffer,"LD 2");
			break;

		case 0x26:
			sprintf(buffer,"X 2");
			break;

		case 0x27:
			sprintf(buffer,"XDS 2");
			break;

		case 0x30:
			sprintf(buffer,"ASC");
			break;

		case 0x31:
			sprintf(buffer,"ADD");
			break;

		case 0x32:
			sprintf(buffer,"RC");
			break;

		case 0x33:
			op2 = oprom[1];
			cnt = 2;

			if (op2 >= 0x60 || op2 <= 0x6F)
			{
				sprintf(buffer,"LEI %x",op2 & 0xF);
			}
			else
			{
				switch (op2)
				{
				case 0x01:
					sprintf(buffer,"SKGBZ 0");
					break;

				case 0x03:
					sprintf(buffer,"SKGBZ 2");
					break;

				case 0x11:
					sprintf(buffer,"SKGBZ 1");
					break;

				case 0x13:
					sprintf(buffer,"SKGBZ 3");
					break;

				case 0x21:
					sprintf(buffer,"SKGZ");
					break;

				case 0x2A:
					sprintf(buffer,"ING");
					break;

				case 0x2E:
					sprintf(buffer,"INL");
					break;

				case 0x3A:
					sprintf(buffer,"OMG");
					break;

				case 0x3C:
					sprintf(buffer,"CAMQ");
					break;

				case 0x3E:
					sprintf(buffer,"OBD");
					break;

				default:
					sprintf(buffer,"Invalid");
					break;
				}
				break;
			}

		case 0x34:
			sprintf(buffer,"XIS 3");
			break;

		case 0x35:
			sprintf(buffer,"LD 3");
			break;

		case 0x36:
			sprintf(buffer,"X 3");
			break;

		case 0x37:
			sprintf(buffer,"XDS 3");
			break;

		case 0x40:
			sprintf(buffer,"COMP");
			break;

		case 0x42:
			sprintf(buffer,"RMB 2");
			break;

		case 0x43:
			sprintf(buffer,"RMB 3");
			break;

		case 0x44:
			sprintf(buffer,"NOP");
			break;

		case 0x45:
			sprintf(buffer,"RMB 1");
			break;

		case 0x46:
			sprintf(buffer,"SMB 2");
			break;

		case 0x47:
			sprintf(buffer,"SMB 1");
			break;

		case 0x48:
			sprintf(buffer,"RET");
			flags = DASMFLAG_STEP_OUT;
			break;

		case 0x49:
			sprintf(buffer,"RETSK");
			flags = DASMFLAG_STEP_OUT;
			break;

		case 0x4B:
			sprintf(buffer,"SMB 3");
			break;

		case 0x4C:
			sprintf(buffer,"RMB 0");
			break;

		case 0x4D:
			sprintf(buffer,"SMB 0");
			break;

		case 0x4E:
			sprintf(buffer,"CBA");
			break;

		case 0x4F:
			sprintf(buffer,"XAS");
			break;

		case 0x50:
			sprintf(buffer,"CAB");
			break;

		case 0xBF:
			sprintf(buffer,"LQID");
			break;

		case 0xFF:
			sprintf(buffer,"JID");
			break;

		default:
			sprintf(buffer,"Invalid");
			break;
		}
	}

	return cnt | flags | DASMFLAG_SUPPORTED;
}
