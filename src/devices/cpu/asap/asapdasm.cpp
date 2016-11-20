// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    asapdasm.c
    Disassembler for the portable ASAP emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "asap.h"


static const char *const reg[32] =
{
	"0",    "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
	"r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
	"r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
	"r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};

static const char *const setcond[2] =
{
	"  ", ".c"
};

static const char *const condition[16] =
{
	"sp", "mz", "gt", "le", "ge", "lt", "hi", "ls", "cc", "cs", "pl", "mi", "ne", "eq", "vc", "vs"
};


/***************************************************************************
    CODE CODE
***************************************************************************/

static inline char *src2(uint32_t op, int scale)
{
	static char temp[20];
	if ((op & 0xffe0) == 0xffe0)
		sprintf(temp, "%s", reg[op & 31]);
	else
		sprintf(temp, "$%x", (op & 0xffff) << scale);
	return temp;
}

CPU_DISASSEMBLE(asap)
{
	uint32_t op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	int opcode = op >> 27;
	int cond = (op >> 21) & 1;
	int rdst = (op >> 22) & 31;
	int rsrc1 = (op >> 16) & 31;
	int rsrc2 = op & 0xffff;
	int rsrc2_iszero = (!rsrc2 || rsrc2 == 0xffe0);
	uint32_t flags = 0;

	switch (opcode)
	{
		case 0x00:  util::stream_format(stream, "trap   $00"); flags = DASMFLAG_STEP_OVER;                              break;
		case 0x01:  util::stream_format(stream, "b%s    $%08x", condition[rdst & 15], pc + ((int32_t)(op << 10) >> 8));   break;
		case 0x02:  if ((op & 0x003fffff) == 3)
					{
						uint32_t nextop = oprom[4] | (oprom[5] << 8) | (oprom[6] << 16) | (oprom[7] << 24);
						if ((nextop >> 27) == 0x10 && ((nextop >> 22) & 31) == rdst && (nextop & 0xffff) == 0)
						{
							uint32_t nextnextop = oprom[8] | (oprom[9] << 8) | (oprom[10] << 16) | (oprom[11] << 24);
							util::stream_format(stream, "llit%s $%08x,%s", setcond[cond], nextnextop, reg[rdst]);
							return 12 | DASMFLAG_STEP_OVER | DASMFLAG_SUPPORTED;
						}
					}
					if (rdst)
					{
						flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
						util::stream_format(stream, "bsr    %s,$%08x", reg[rdst], pc + ((int32_t)(op << 10) >> 8));
					}
					else
						util::stream_format(stream, "bra    $%08x", pc + ((int32_t)(op << 10) >> 8));
					break;
		case 0x03:  util::stream_format(stream, "lea%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,2), reg[rdst]);  break;
		case 0x04:  util::stream_format(stream, "leah%s %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);  break;
		case 0x05:  util::stream_format(stream, "subr%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x06:  util::stream_format(stream, "xor%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x07:  util::stream_format(stream, "xorn%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x08:  if (!rsrc1 && !rdst && rsrc2_iszero)
					util::stream_format(stream, "nop");
					else if (!rsrc1)
					util::stream_format(stream, "mov%s  %s,%s", setcond[cond], src2(op,0), reg[rdst]);
					else if (rsrc2_iszero)
					util::stream_format(stream, "mov%s  %s,%s", setcond[cond], reg[rsrc1], reg[rdst]);
					else
					util::stream_format(stream, "add%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);
			break;
		case 0x09:  util::stream_format(stream, "sub%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x0a:  util::stream_format(stream, "addc%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x0b:  util::stream_format(stream, "subc%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x0c:  util::stream_format(stream, "and%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x0d:  util::stream_format(stream, "andn%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x0e:  if (!rsrc1 && !rdst && rsrc2_iszero)
					util::stream_format(stream, "nop");
					else if (!rsrc1)
					util::stream_format(stream, "mov%s  %s,%s", setcond[cond], src2(op,0), reg[rdst]);
					else if (rsrc2_iszero)
					util::stream_format(stream, "mov%s  %s,%s", setcond[cond], reg[rsrc1], reg[rdst]);
					else
					util::stream_format(stream, "or%s   %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);
			break;
		case 0x0f:  util::stream_format(stream, "orn%s  %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x10:  util::stream_format(stream, "ld%s   %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,2), reg[rdst]);  break;
		case 0x11:  util::stream_format(stream, "ldh%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);  break;
		case 0x12:  util::stream_format(stream, "lduh%s %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);  break;
		case 0x13:  util::stream_format(stream, "sth%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,1), reg[rdst]);  break;
		case 0x14:  util::stream_format(stream, "st%s   %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,2), reg[rdst]);  break;
		case 0x15:  util::stream_format(stream, "ldb%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);  break;
		case 0x16:  util::stream_format(stream, "ldub%s %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);  break;
		case 0x17:  util::stream_format(stream, "stb%s  %s[%s],%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);  break;
		case 0x18:  util::stream_format(stream, "ashr%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x19:  util::stream_format(stream, "lshr%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x1a:  util::stream_format(stream, "ashl%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x1b:  util::stream_format(stream, "rotl%s %s,%s,%s", setcond[cond], reg[rsrc1], src2(op,0), reg[rdst]);   break;
		case 0x1c:  util::stream_format(stream, "getps  %s", reg[rdst]);                                                break;
		case 0x1d:  util::stream_format(stream, "putps  %s", src2(op,0));                                               break;
		case 0x1e:  if (rdst && rsrc2_iszero)
					{
						flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
						util::stream_format(stream, "jsr%s  %s,%s", setcond[cond], reg[rdst], reg[rsrc1]);
					}
					else if (rdst)
					{
						flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
						util::stream_format(stream, "jsr%s  %s,%s[%s]", setcond[cond], reg[rdst], reg[rsrc1], src2(op,2));
					}
					else if (rsrc2_iszero)
					{
						if (rsrc1 == 28)
							flags = DASMFLAG_STEP_OUT;
						util::stream_format(stream, "jmp%s  %s", setcond[cond], reg[rsrc1]);
					}
					else
						util::stream_format(stream, "jmp%s  %s[%s]", setcond[cond], reg[rsrc1], src2(op,2));
			break;
		case 0x1f:  util::stream_format(stream, "trap   $1f"); flags = DASMFLAG_STEP_OVER;                              break;
	}	
	return 4 | flags | DASMFLAG_SUPPORTED;
}
