/***************************************************************************

    asapdasm.c
    Disassembler for the portable ASAP emulator.
    Written by Aaron Giles

***************************************************************************/

#include "asap.h"


static const char *reg[32] =
{
	"0",	"r1",	"r2",	"r3",	"r4",	"r5",	"r6",	"r7",
	"r8",	"r9",	"r10",	"r11",	"r12",	"r13",	"r14",	"r15",
	"r16",	"r17",	"r18",	"r19",	"r20",	"r21",	"r22",	"r23",
	"r24",	"r25",	"r26",	"r27",	"r28",	"r29",	"r30",	"r31"
};

static const char *setcond[2] =
{
	"  ", ".c"
};

static const char *condition[16] =
{
	"sp", "mz", "gt", "le", "ge", "lt", "hi", "ls", "cc", "cs", "pl", "mi", "ne", "eq", "vc", "vs"
};


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(pc)		cpu_readop32(pc)


/***************************************************************************
    CODE CODE
***************************************************************************/

INLINE char *src2(UINT32 op, int scale)
{
	static char temp[20];
	if ((op & 0xffe0) == 0xffe0)
		sprintf(temp, "%s", reg[op & 31]);
	else
		sprintf(temp, "$%x", (op & 0xffff) << scale);
	return temp;
}

offs_t asap_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	int opcode = op >> 27;
	int cond = (op >> 21) & 1;
	int rdst = (op >> 22) & 31;
	int rsrc1 = (op >> 16) & 31;
	int rsrc2 = op & 0xffff;
	int rsrc2_iszero = (!rsrc2 || rsrc2 == 0xffe0);
	UINT32 flags = 0;

	switch (opcode)
	{
		case 0x00:	sprintf(buffer, "trap   $00"); flags = DASMFLAG_STEP_OVER;								break;
		case 0x01:	sprintf(buffer, "b%s    $%08x", condition[rdst & 15], pc + ((INT32)(op << 10) >> 8));	break;
		case 0x02:	if ((op & 0x003fffff) == 3)
					{
						UINT32 nextop = oprom[4] | (oprom[5] << 8) | (oprom[6] << 16) | (oprom[7] << 24);
						if ((nextop >> 27) == 0x10 && ((nextop >> 22) & 31) == rdst && (nextop & 0xffff) == 0)
						{
							UINT32 nextnextop = oprom[8] | (oprom[9] << 8) | (oprom[10] << 16) | (oprom[11] << 24);
							sprintf(buffer, "llit%s $%08x,%s", setcond[cond], nextnextop, reg[rdst]);
							return 12 | DASMFLAG_STEP_OVER | DASMFLAG_SUPPORTED;
						}
					}
					if (rdst)
					{
						flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
						sprintf(buffer, "bsr    %s,$%08x", reg[rdst], pc + ((INT32)(op << 10) >> 8));
					}
					else
						sprintf(buffer, "bra    $%08x", pc + ((INT32)(op << 10) >> 8));
					break;
		case 0x03:	sprintf(buffer, "lea%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,2), reg[rdst]);	break;
		case 0x04:	sprintf(buffer, "leah%s %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);	break;
		case 0x05:	sprintf(buffer, "subr%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x06:	sprintf(buffer, "xor%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x07:	sprintf(buffer, "xorn%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x08:	if (!rsrc1 && !rdst && rsrc2_iszero)
					sprintf(buffer, "nop");
					else if (!rsrc1)
					sprintf(buffer, "mov%s  %s,%s", setcond[cond], src2(op,0), reg[rdst]);
					else if (rsrc2_iszero)
					sprintf(buffer, "mov%s  %s,%s", setcond[cond], reg[rsrc1], reg[rdst]);
					else
					sprintf(buffer, "add%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x09:	sprintf(buffer, "sub%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x0a:	sprintf(buffer, "addc%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x0b:	sprintf(buffer, "subc%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x0c:	sprintf(buffer, "and%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x0d:	sprintf(buffer, "andn%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x0e:	if (!rsrc1 && !rdst && rsrc2_iszero)
					sprintf(buffer, "nop");
					else if (!rsrc1)
					sprintf(buffer, "mov%s  %s,%s", setcond[cond], src2(op,0), reg[rdst]);
					else if (rsrc2_iszero)
					sprintf(buffer, "mov%s  %s,%s", setcond[cond], reg[rsrc1], reg[rdst]);
					else
					sprintf(buffer, "or%s   %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x0f:	sprintf(buffer, "orn%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x10:	sprintf(buffer, "ld%s   %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,2), reg[rdst]);	break;
		case 0x11:	sprintf(buffer, "ldh%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);	break;
		case 0x12:	sprintf(buffer, "lduh%s %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);	break;
		case 0x13:	sprintf(buffer, "sth%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);	break;
		case 0x14:	sprintf(buffer, "st%s   %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,2), reg[rdst]);	break;
		case 0x15:	sprintf(buffer, "ldb%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x16:	sprintf(buffer, "ldub%s %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x17:	sprintf(buffer, "stb%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x18:	sprintf(buffer, "ashr%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x19:	sprintf(buffer, "lshr%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x1a:	sprintf(buffer, "ashl%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x1b:	sprintf(buffer, "rotl%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);	break;
		case 0x1c:	sprintf(buffer, "getps  %s", reg[rdst]);												break;
		case 0x1d:	sprintf(buffer, "putps  %s", src2(op,0));												break;
		case 0x1e:	if (rdst && rsrc2_iszero)
					{
						flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
						sprintf(buffer, "jsr%s  %s,%s", setcond[cond], reg[rdst], reg[rsrc1]);
					}
					else if (rdst)
					{
						flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
						sprintf(buffer, "jsr%s  %s,%s[%s]", setcond[cond], reg[rdst], reg[rsrc1], src2(op,2));
					}
					else if (rsrc2_iszero)
					{
						if (rsrc1 == 28)
							flags = DASMFLAG_STEP_OUT;
						sprintf(buffer, "jmp%s  %s", setcond[cond], reg[rsrc1]);
					}
					else
						sprintf(buffer, "jmp%s  %s[%s]", setcond[cond], reg[rsrc1], src2(op,2));
					break;
		case 0x1f:	sprintf(buffer, "trap   $1f"); flags = DASMFLAG_STEP_OVER;								break;
	}
	return 4 | flags | DASMFLAG_SUPPORTED;
}
