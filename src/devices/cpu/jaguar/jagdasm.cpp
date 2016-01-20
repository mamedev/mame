// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jagdasm.c
    Disassembler for the portable Jaguar DSP emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "jaguar.h"


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(offs)   ((oprom[offs] << 8) | oprom[(offs) + 1])


/***************************************************************************
    STATIC VARIABLES
***************************************************************************/

static const UINT8 convert_zero[32] =
{ 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

static const char *const condition[32] =
{
	"",
	"nz,",
	"z,",
	"???,",
	"nc,",
	"nc nz,",
	"nc z,",
	"???,",
	"c,",
	"c nz,",
	"c z,",
	"???,",
	"???,",
	"???,",
	"???,",
	"???,",

	"???,",
	"???,",
	"???,",
	"???,",
	"nn,",
	"nn nz,",
	"nn z,",
	"???,",
	"n,",
	"n nz,",
	"n z,",
	"???,",
	"???,",
	"???,",
	"???,",
	"never,"
};



/***************************************************************************
    CODE CODE
***************************************************************************/

static inline char *signed_16bit(INT16 val)
{
	static char temp[10];
	if (val < 0)
		sprintf(temp, "-$%x", -val);
	else
		sprintf(temp, "$%x", val);
	return temp;
}

static unsigned dasmjag(int variant, char *buffer, unsigned pc, const UINT8 *oprom)
{
	UINT32 flags = 0;
	int op = ROPCODE(0);
	int reg1 = (op >> 5) & 31;
	int reg2 = op & 31;
	int size = 2;

	pc += 2;
	switch (op >> 10)
	{
		case 0:     sprintf(buffer, "add     r%d,r%d", reg1, reg2);                 break;
		case 1:     sprintf(buffer, "addc    r%d,r%d", reg1, reg2);                 break;
		case 2:     sprintf(buffer, "addq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 3:     sprintf(buffer, "addqt   $%x,r%d", convert_zero[reg1], reg2);   break;
		case 4:     sprintf(buffer, "sub     r%d,r%d", reg1, reg2);                 break;
		case 5:     sprintf(buffer, "subc    r%d,r%d", reg1, reg2);                 break;
		case 6:     sprintf(buffer, "subq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 7:     sprintf(buffer, "subqt   $%x,r%d", convert_zero[reg1], reg2);   break;
		case 8:     sprintf(buffer, "neg     r%d", reg2);                           break;
		case 9:     sprintf(buffer, "and     r%d,r%d", reg1, reg2);                 break;
		case 10:    sprintf(buffer, "or      r%d,r%d", reg1, reg2);                 break;
		case 11:    sprintf(buffer, "xor     r%d,r%d", reg1, reg2);                 break;
		case 12:    sprintf(buffer, "not     r%d", reg2);                           break;
		case 13:    sprintf(buffer, "btst    $%x,r%d", reg1, reg2);                 break;
		case 14:    sprintf(buffer, "bset    $%x,r%d", reg1, reg2);                 break;
		case 15:    sprintf(buffer, "bclr    $%x,r%d", reg1, reg2);                 break;
		case 16:    sprintf(buffer, "mult    r%d,r%d", reg1, reg2);                 break;
		case 17:    sprintf(buffer, "imult   r%d,r%d", reg1, reg2);                 break;
		case 18:    sprintf(buffer, "imultn  r%d,r%d", reg1, reg2);                 break;
		case 19:    sprintf(buffer, "resmac  r%d", reg2);                           break;
		case 20:    sprintf(buffer, "imacn   r%d,r%d", reg1, reg2);                 break;
		case 21:    sprintf(buffer, "div     r%d,r%d", reg1, reg2);                 break;
		case 22:    sprintf(buffer, "abs     r%d", reg2);                           break;
		case 23:    sprintf(buffer, "sh      r%d,r%d", reg1, reg2);                 break;
		case 24:    sprintf(buffer, "shlq    $%x,r%d", 32 - convert_zero[reg1], reg2);  break;
		case 25:    sprintf(buffer, "shrq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 26:    sprintf(buffer, "sha     r%d,r%d", reg1, reg2);                 break;
		case 27:    sprintf(buffer, "sharq   $%x,r%d", convert_zero[reg1], reg2);   break;
		case 28:    sprintf(buffer, "ror     r%d,r%d", reg1, reg2);                 break;
		case 29:    sprintf(buffer, "rorq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 30:    sprintf(buffer, "cmp     r%d,r%d", reg1, reg2);                 break;
		case 31:    sprintf(buffer, "cmpq    %s,r%d", signed_16bit((INT16)(reg1 << 11) >> 11), reg2);break;
		case 32:    if (variant == JAGUAR_VARIANT_GPU)
					sprintf(buffer, "sat8    r%d", reg2);
					else
					sprintf(buffer, "subqmod $%x,r%d", convert_zero[reg1], reg2);
					break;
		case 33:    if (variant == JAGUAR_VARIANT_GPU)
					sprintf(buffer, "sat16   r%d", reg2);
					else
					sprintf(buffer, "sat16s  r%d", reg2);
					break;
		case 34:    sprintf(buffer, "move    r%d,r%d", reg1, reg2);                 break;
		case 35:    sprintf(buffer, "moveq   %d,r%d", reg1, reg2);                  break;
		case 36:    sprintf(buffer, "moveta  r%d,r%d", reg1, reg2);                 break;
		case 37:    sprintf(buffer, "movefa  r%d,r%d", reg1, reg2);                 break;
		case 38:    sprintf(buffer, "movei   $%x,r%d", ROPCODE(2) | (ROPCODE(4)<<16), reg2); size = 6; break;
		case 39:    sprintf(buffer, "loadb   (r%d),r%d", reg1, reg2);                   break;
		case 40:    sprintf(buffer, "loadw   (r%d),r%d", reg1, reg2);                   break;
		case 41:    sprintf(buffer, "load    (r%d),r%d", reg1, reg2);                   break;
		case 42:    if (variant == JAGUAR_VARIANT_GPU)
					sprintf(buffer, "loadp   (r%d),r%d", reg1, reg2);
					else
					sprintf(buffer, "sat32s  r%d", reg2);
					break;
		case 43:    sprintf(buffer, "load    (r14+$%x),r%d", convert_zero[reg1]*4, reg2);break;
		case 44:    sprintf(buffer, "load    (r15+$%x),r%d", convert_zero[reg1]*4, reg2);break;
		case 45:    sprintf(buffer, "storeb  r%d,(r%d)", reg2, reg1);               break;
		case 46:    sprintf(buffer, "storew  r%d,(r%d)", reg2, reg1);               break;
		case 47:    sprintf(buffer, "store   r%d,(r%d)", reg2, reg1);                   break;
		case 48:    if (variant == JAGUAR_VARIANT_GPU)
					sprintf(buffer, "storep  r%d,(r%d)", reg2, reg1);
					else
					sprintf(buffer, "mirror  r%d", reg2);
					break;
		case 49:    sprintf(buffer, "store   r%d,(r14+$%x)", reg2, convert_zero[reg1]*4);break;
		case 50:    sprintf(buffer, "store   r%d,(r15+$%x)", reg2, convert_zero[reg1]*4);break;
		case 51:    sprintf(buffer, "move    pc,r%d", reg2);                            break;
		case 52:    sprintf(buffer, "jump    %s(r%d)", condition[reg2], reg1);          break;
		case 53:    sprintf(buffer, "jr      %s%08X", condition[reg2], pc + ((INT8)(reg1 << 3) >> 2)); break;
		case 54:    sprintf(buffer, "mmult   r%d,r%d", reg1, reg2);                 break;
		case 55:    sprintf(buffer, "mtoi    r%d,r%d", reg1, reg2);                 break;
		case 56:    sprintf(buffer, "normi   r%d,r%d", reg1, reg2);                 break;
		case 57:    sprintf(buffer, "nop");                                         break;
		case 58:    sprintf(buffer, "load    (r14+r%d),r%d", reg1, reg2);               break;
		case 59:    sprintf(buffer, "load    (r15+r%d),r%d", reg1, reg2);               break;
		case 60:    sprintf(buffer, "store   r%d,(r14+r%d)", reg2, reg1);               break;
		case 61:    sprintf(buffer, "store   r%d,(r15+r%d)", reg2, reg1);               break;
		case 62:    if (variant == JAGUAR_VARIANT_GPU)
					sprintf(buffer, "sat24   r%d", reg2);
					else
					sprintf(buffer, "illegal");
					break;
		case 63:    if (variant == JAGUAR_VARIANT_GPU)
					sprintf(buffer, reg1 ?
									"unpack  r%d" :
									"pack    r%d", reg2);
					else
					sprintf(buffer, "addqmod $%x,r%d", convert_zero[reg1], reg2);
					break;
	}
	return size | flags | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE( jaguargpu )
{
	return dasmjag(JAGUAR_VARIANT_GPU, buffer, pc, oprom);
}

CPU_DISASSEMBLE( jaguardsp )
{
	return dasmjag(JAGUAR_VARIANT_DSP, buffer, pc, oprom);
}
