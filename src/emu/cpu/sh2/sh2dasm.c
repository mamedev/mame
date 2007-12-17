#include "debugger.h"
#include "sh2.h"

#define SIGNX8(x)	(((INT32)(x) << 24) >> 24)
#define SIGNX12(x)	(((INT32)(x) << 20) >> 20)

#define Rn ((opcode >> 8) & 15)
#define Rm ((opcode >> 4) & 15)

static const char *regname[16] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"R8", "R9", "R10","R11","R12","R13","R14","SP"
};

static UINT32 op0000(char *buffer, UINT32 pc, UINT16 opcode)
{
	UINT32  flags = 0;
	switch(opcode & 0x3f)
	{
	case 0x02:
		sprintf(buffer,"STC     SR,%s", regname[Rn]);
		break;
	case 0x03:
		sprintf(buffer,"BSRF    %s", regname[Rn]);
		break;
	case 0x08:
		sprintf(buffer,"CLRT");
		break;
	case 0x09:
		sprintf(buffer,"NOP");
		break;
	case 0x0A:
		sprintf(buffer,"STS     MACH,%s", regname[Rn]);
		break;
	case 0x0B:
		sprintf(buffer,"RTS");
		flags = DASMFLAG_STEP_OUT;
		break;
	case 0x12:
		sprintf(buffer,"STS     GBR,%s", regname[Rn]);
		break;
	case 0x18:
		sprintf(buffer,"SETT");
		break;
	case 0x19:
		sprintf(buffer,"DIV0U");
		break;
	case 0x1A:
		sprintf(buffer,"STS     MACL,%s", regname[Rn]);
		break;
	case 0x1B:
		sprintf(buffer,"SLEEP");
		break;
	case 0x22:
		sprintf(buffer,"STC     VBR,%s", regname[Rn]);
		break;
	case 0x23:
		sprintf(buffer,"BRAF    %s", regname[Rn]);
		break;
	case 0x28:
		sprintf(buffer,"CLRMAC");
		break;
	case 0x29:
		sprintf(buffer,"MOVT    %s", regname[Rn]);
		break;
	case 0x2A:
		sprintf(buffer,"STS     PR,%s", regname[Rn]);
		break;
	case 0x2B:
		sprintf(buffer,"RTE");
		flags = DASMFLAG_STEP_OUT;
		break;
	default:
		switch(opcode & 15)
		{
		case  0:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case  1:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case  2:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case  3:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case  4:
			sprintf(buffer, "MOV.B   %s,@(R0,%s)", regname[Rm], regname[Rn]);
			break;
		case  5:
			sprintf(buffer, "MOV.W   %s,@(R0,%s)", regname[Rm], regname[Rn]);
			break;
		case  6:
			sprintf(buffer, "MOV.L   %s,@(R0,%s)", regname[Rm], regname[Rn]);
			break;
		case  7:
			sprintf(buffer, "MUL.L   %s,%s\n", regname[Rm], regname[Rn]);
			break;
		case  8:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case  9:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case 10:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case 11:
			sprintf(buffer, "??????  $%04X", opcode);
			break;
		case 12:
			sprintf(buffer, "MOV.B   @(R0,%s),%s", regname[Rm], regname[Rn]);
			break;
		case 13:
			sprintf(buffer, "MOV.W   @(R0,%s),%s", regname[Rm], regname[Rn]);
			break;
		case 14:
			sprintf(buffer, "MOV.L   @(R0,%s),%s", regname[Rm], regname[Rn]);
			break;
		case 15:
			sprintf(buffer, "MAC.L   @%s+,@%s+", regname[Rn], regname[Rm]);
			break;
		}
	}
	return flags;
}

static UINT32 op0001(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "MOV.L   %s,@($%02X,%s)\n", regname[Rm], (opcode & 15) * 4, regname[Rn]);
	return 0;
}

static UINT32 op0010(char *buffer, UINT32 pc, UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0:
		sprintf(buffer, "MOV.B   %s,@%s", regname[Rm], regname[Rn]);
		break;
	case  1:
		sprintf(buffer, "MOV.W   %s,@%s", regname[Rm], regname[Rn]);
		break;
	case  2:
		sprintf(buffer, "MOV.L   %s,@%s", regname[Rm], regname[Rn]);
		break;
	case  3:
		sprintf(buffer, "??????  $%04X", opcode);
		break;
	case  4:
		sprintf(buffer, "MOV.B   %s,@-%s", regname[Rm], regname[Rn]);
		break;
	case  5:
		sprintf(buffer, "MOV.W   %s,@-%s", regname[Rm], regname[Rn]);
		break;
	case  6:
		sprintf(buffer, "MOV.L   %s,@-%s", regname[Rm], regname[Rn]);
		break;
	case  7:
		sprintf(buffer, "DIV0S   %s,%s", regname[Rm], regname[Rn]);
		break;
	case  8:
		sprintf(buffer, "TST     %s,%s", regname[Rm], regname[Rn]);
		break;
	case  9:
		sprintf(buffer, "AND     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 10:
		sprintf(buffer, "XOR     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 11:
		sprintf(buffer, "OR      %s,%s", regname[Rm], regname[Rn]);
		break;
	case 12:
		sprintf(buffer, "CMP/STR %s,%s", regname[Rm], regname[Rn]);
		break;
	case 13:
		sprintf(buffer, "XTRCT   %s,%s", regname[Rm], regname[Rn]);
		break;
	case 14:
		sprintf(buffer, "MULU.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 15:
		sprintf(buffer, "MULS.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	}
	return 0;
}

static UINT32 op0011(char *buffer, UINT32 pc, UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0:
		sprintf(buffer, "CMP/EQ  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  1:
		sprintf(buffer, "??????  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  2:
		sprintf(buffer, "CMP/HS  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  3:
		sprintf(buffer, "CMP/GE  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  4:
		sprintf(buffer, "DIV1    %s,%s", regname[Rm], regname[Rn]);
		break;
	case  5:
		sprintf(buffer, "DMULU.L %s,%s", regname[Rm], regname[Rn]);
		break;
	case  6:
		sprintf(buffer, "CMP/HI  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  7:
		sprintf(buffer, "CMP/GT  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  8:
		sprintf(buffer, "SUB     %s,%s", regname[Rm], regname[Rn]);
		break;
	case  9:
		sprintf(buffer, "??????  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 10:
		sprintf(buffer, "SUBC    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 11:
		sprintf(buffer, "SUBV    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 12:
		sprintf(buffer, "ADD     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 13:
		sprintf(buffer, "DMULS.L %s,%s", regname[Rm], regname[Rn]);
		break;
	case 14:
		sprintf(buffer, "ADDC    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 15:
		sprintf(buffer, "ADDV    %s,%s", regname[Rm], regname[Rn]);
		break;
	}
	return 0;
}

static UINT32 op0100(char *buffer, UINT32 pc, UINT16 opcode)
{
	UINT32 flags = 0;
	switch(opcode & 0x3F)
	{
	case 0x00:
		sprintf(buffer, "SHLL    %s", regname[Rn]);
		break;
	case 0x01:
		sprintf(buffer, "SHLR    %s", regname[Rn]);
		break;
	case 0x02:
		sprintf(buffer, "STS.L   MACH,@-%s", regname[Rn]);
		break;
	case 0x03:
		sprintf(buffer, "STC.L   SR,@-%s", regname[Rn]);
		break;
	case 0x04:
		sprintf(buffer, "ROTL    %s", regname[Rn]);
		break;
	case 0x05:
		sprintf(buffer, "ROTR    %s", regname[Rn]);
		break;
	case 0x06:
		sprintf(buffer, "LDS.L   @%s+,MACH", regname[Rn]);
		break;
	case 0x07:
		sprintf(buffer, "LDC.L   @%s+,SR", regname[Rn]);
		break;
	case 0x08:
		sprintf(buffer, "SHLL2   %s", regname[Rn]);
		break;
	case 0x09:
		sprintf(buffer, "SHLR2   %s", regname[Rn]);
		break;
	case 0x0a:
		sprintf(buffer, "LDS     %s,MACH", regname[Rn]);
		break;
	case 0x0b:
		sprintf(buffer, "JSR     %s", regname[Rn]);
		flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
		break;
	case 0x0e:
		sprintf(buffer, "LDC     %s,SR", regname[Rn]);
		break;
	case 0x10:
		sprintf(buffer, "DT      %s", regname[Rn]);
		break;
	case 0x11:
		sprintf(buffer, "CMP/PZ  %s", regname[Rn]);
		break;
	case 0x12:
		sprintf(buffer, "STS.L   MACL,@-%s", regname[Rn]);
		break;
	case 0x13:
		sprintf(buffer, "STC.L   GBR,@-%s", regname[Rn]);
		break;
	case 0x15:
		sprintf(buffer, "CMP/PL  %s", regname[Rn]);
		break;
	case 0x16:
		sprintf(buffer, "LDS.L   @%s+,MACL", regname[Rn]);
		break;
	case 0x17:
		sprintf(buffer, "LDC.L   @%s+,GBR", regname[Rn]);
		break;
	case 0x18:
		sprintf(buffer, "SHLL8   %s", regname[Rn]);
		break;
	case 0x19:
		sprintf(buffer, "SHLR8   %s", regname[Rn]);
		break;
	case 0x1a:
		sprintf(buffer, "LDS     %s,MACL", regname[Rn]);
		break;
	case 0x1b:
		sprintf(buffer, "TAS     %s", regname[Rn]);
		break;
	case 0x1e:
		sprintf(buffer, "LDC     %s,GBR", regname[Rn]);
		break;
	case 0x20:
		sprintf(buffer, "SHAL    %s", regname[Rn]);
		break;
	case 0x21:
		sprintf(buffer, "SHAR    %s", regname[Rn]);
		break;
	case 0x22:
		sprintf(buffer, "STS.L   PR,@-%s", regname[Rn]);
		break;
	case 0x23:
		sprintf(buffer, "STC.L   VBR,@-%s", regname[Rn]);
		break;
	case 0x24:
		sprintf(buffer, "ROTCL   %s", regname[Rn]);
		break;
	case 0x25:
		sprintf(buffer, "ROTCR   %s", regname[Rn]);
		break;
	case 0x26:
		sprintf(buffer, "LDS.L   @%s+,PR", regname[Rn]);
		break;
	case 0x27:
		sprintf(buffer, "LDC.L   @%s+,VBR", regname[Rn]);
		break;
	case 0x28:
		sprintf(buffer, "SHLL16  %s", regname[Rn]);
		break;
	case 0x29:
		sprintf(buffer, "SHLR16  %s", regname[Rn]);
		break;
	case 0x2a:
		sprintf(buffer, "LDS     %s,PR", regname[Rn]);
		break;
	case 0x2b:
		sprintf(buffer, "JMP     %s", regname[Rn]);
		break;
	case 0x2e:
		sprintf(buffer, "LDC     %s,VBR", regname[Rn]);
		break;
	default:
		if ((opcode & 15) == 15)
			sprintf(buffer, "MAC.W   @%s+,@%s+", regname[Rm], regname[Rn]);
		else
			sprintf(buffer, "??????  $%04X", opcode);
	}
	return flags;
}

static UINT32 op0101(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "MOV.L   @($%02X,%s),%s\n", (opcode & 15) * 4, regname[Rm], regname[Rn]);
	return 0;
}

static UINT32 op0110(char *buffer, UINT32 pc, UINT16 opcode)

{
	switch(opcode & 0xF)
	{
	case 0x00:
		sprintf(buffer, "MOV.B   @%s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x01:
		sprintf(buffer, "MOV.W   @%s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x02:
		sprintf(buffer, "MOV.L   @%s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x03:
		sprintf(buffer, "MOV     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x04:
		sprintf(buffer, "MOV.B   @%s+,%s", regname[Rm], regname[Rn]);
		break;
	case 0x05:
		sprintf(buffer, "MOV.W   @%s+,%s", regname[Rm], regname[Rn]);
		break;
	case 0x06:
		sprintf(buffer, "MOV.L   @%s+,%s", regname[Rm], regname[Rn]);
		break;
	case 0x07:
		sprintf(buffer, "NOT     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x08:
		sprintf(buffer, "SWAP.B  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x09:
		sprintf(buffer, "SWAP.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0a:
		sprintf(buffer, "NEGC    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0b:
		sprintf(buffer, "NEG     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0c:
		sprintf(buffer, "EXTU.B  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0d:
		sprintf(buffer, "EXTU.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0e:
		sprintf(buffer, "EXTS.B  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0f:
		sprintf(buffer, "EXTS.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	}
	return 0;
}

static UINT32 op0111(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "ADD     #$%02X,%s\n", opcode & 0xff, regname[Rn]);
	return 0;
}

static UINT32 op1000(char *buffer, UINT32 pc, UINT16 opcode)
{
	switch((opcode >> 8) & 15)
	{
	case  0:
		sprintf(buffer, "MOV.B   R0,@($%02X,%s)", (opcode & 15), regname[Rm]);
		break;
	case  1:
		sprintf(buffer, "MOV.W   R0,@($%02X,%s)", (opcode & 15) * 2, regname[Rm]);
		break;
	case  4:
		sprintf(buffer, "MOV.B   @($%02X,%s),R0", (opcode & 15), regname[Rm]);
		break;
	case  5:
		sprintf(buffer, "MOV.W   @($%02X,%s),R0", (opcode & 15), regname[Rm]);
		break;
	case  8:
		sprintf(buffer, "CMP/EQ  #$%02X,R0", (opcode & 0xff));
		break;
	case  9:
		sprintf(buffer, "BT      $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	case 11:
		sprintf(buffer, "BF      $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	case 13:
		sprintf(buffer, "BTS     $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	case 15:
		sprintf(buffer, "BFS     $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	default :
		sprintf(buffer, "invalid $%04X\n", opcode);
	}
	return 0;
}

static UINT32 op1001(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "MOV.W   @($%04X,PC),%s", (opcode & 0xff) * 2, regname[Rn]);
	return 0;
}

static UINT32 op1010(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "BRA     $%08X", SIGNX12(opcode & 0xfff) * 2 + pc + 2);
	return 0;
}

static UINT32 op1011(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "BSR     $%08X", SIGNX12(opcode & 0xfff) * 2 + pc + 2);
	return DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
}

static UINT32 op1100(char *buffer, UINT32 pc, UINT16 opcode)
{
	UINT32 flags = 0;
	switch((opcode >> 8) & 15)
	{
	case  0:
		sprintf(buffer, "MOV.B   R0,@($%02X,GBR)", opcode & 0xff);
		break;
	case  1:
		sprintf(buffer, "MOV.W   R0,@($%04X,GBR)", (opcode & 0xff) * 2);
		break;
	case  2:
		sprintf(buffer, "MOV.L   R0,@($%04X,GBR)", (opcode & 0xff) * 4);
		break;
	case  3:
		sprintf(buffer, "TRAPA   #$%02X", opcode & 0xff);
		flags = DASMFLAG_STEP_OVER;
		break;
	case  4:
		sprintf(buffer, "MOV.B   @($%02X,GBR),R0", opcode & 0xff);
		break;
	case  5:
		sprintf(buffer, "MOV.W   @($%04X,GBR),R0", (opcode & 0xff) * 2);
		break;
	case  6:
		sprintf(buffer, "MOV.L   @($%04X,GBR),R0", (opcode & 0xff) * 4);
		break;
	case  7:
		sprintf(buffer, "MOVA    @($%04X,PC),R0", (opcode & 0xff) * 4);
		break;
	case  8:
		sprintf(buffer, "TST     #$%02X,R0", opcode & 0xff);
		break;
	case  9:
		sprintf(buffer, "AND     #$%02X,R0", opcode & 0xff);
		break;
	case 10:
		sprintf(buffer, "XOR     #$%02X,R0", opcode & 0xff);
		break;
	case 11:
		sprintf(buffer, "OR      #$%02X,R0", opcode & 0xff);
		break;
	case 12:
		sprintf(buffer, "TST.B   #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	case 13:
		sprintf(buffer, "AND.B   #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	case 14:
		sprintf(buffer, "XOR.B   #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	case 15:
		sprintf(buffer, "OR.B    #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	}
	return flags;
}

static UINT32 op1101(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "MOV.L   @($%02X,PC),%s", (opcode * 4) & 0xff, regname[Rn]);
	return 0;
}

static UINT32 op1110(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "MOV     #$%02X,%s", (opcode & 0xff), regname[Rn]);
	return 0;
}

static UINT32 op1111(char *buffer, UINT32 pc, UINT16 opcode)
{
	sprintf(buffer, "unknown $%04X", opcode);
	return 0;
}

unsigned DasmSH2(char *buffer, unsigned pc, UINT16 opcode)
{
	UINT32 flags;

	pc += 2;

	switch((opcode >> 12) & 15)
	{
	case  0: flags = op0000(buffer,pc,opcode);	  break;
	case  1: flags = op0001(buffer,pc,opcode);	  break;
	case  2: flags = op0010(buffer,pc,opcode);	  break;
	case  3: flags = op0011(buffer,pc,opcode);	  break;
	case  4: flags = op0100(buffer,pc,opcode);	  break;
	case  5: flags = op0101(buffer,pc,opcode);	  break;
	case  6: flags = op0110(buffer,pc,opcode);	  break;
	case  7: flags = op0111(buffer,pc,opcode);	  break;
	case  8: flags = op1000(buffer,pc,opcode);	  break;
	case  9: flags = op1001(buffer,pc,opcode);	  break;
	case 10: flags = op1010(buffer,pc,opcode);	  break;
	case 11: flags = op1011(buffer,pc,opcode);	  break;
	case 12: flags = op1100(buffer,pc,opcode);	  break;
	case 13: flags = op1101(buffer,pc,opcode);	  break;
	case 14: flags = op1110(buffer,pc,opcode);	  break;
	default: flags = op1111(buffer,pc,opcode);	  break;
	}
	return 2 | flags | DASMFLAG_SUPPORTED;
}
