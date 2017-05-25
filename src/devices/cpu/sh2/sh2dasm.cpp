// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#include "emu.h"
#include "debugger.h"
#include "sh2.h"

#define SIGNX8(x)   (((int32_t)(x) << 24) >> 24)
#define SIGNX12(x)  (((int32_t)(x) << 20) >> 20)

#define Rn ((opcode >> 8) & 15)
#define Rm ((opcode >> 4) & 15)

static const char *const regname[16] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"R8", "R9", "R10","R11","R12","R13","R14","SP"
};

static uint32_t op0000(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	uint32_t  flags = 0;
	switch(opcode & 0x3f)
	{
	case 0x02:
		util::stream_format(stream, "STC     SR,%s", regname[Rn]);
		break;
	case 0x03:
		util::stream_format(stream, "BSRF    %s", regname[Rn]);
		break;
	case 0x08:
		stream << "CLRT";
		break;
	case 0x09:
		stream << "NOP";
		break;
	case 0x0A:
		util::stream_format(stream, "STS     MACH,%s", regname[Rn]);
		break;
	case 0x0B:
		stream << "RTS";
		flags = DASMFLAG_STEP_OUT;
		break;
	case 0x12:
		util::stream_format(stream, "STS     GBR,%s", regname[Rn]);
		break;
	case 0x18:
		stream << "SETT";
		break;
	case 0x19:
		stream << "DIV0U";
		break;
	case 0x1A:
		util::stream_format(stream, "STS     MACL,%s", regname[Rn]);
		break;
	case 0x1B:
		stream << "SLEEP";
		break;
	case 0x22:
		util::stream_format(stream, "STC     VBR,%s", regname[Rn]);
		break;
	case 0x23:
		util::stream_format(stream, "BRAF    %s", regname[Rn]);
		break;
	case 0x28:
		stream << "CLRMAC";
		break;
	case 0x29:
		util::stream_format(stream, "MOVT    %s", regname[Rn]);
		break;
	case 0x2A:
		util::stream_format(stream, "STS     PR,%s", regname[Rn]);
		break;
	case 0x2B:
		stream << "RTE";
		flags = DASMFLAG_STEP_OUT;
		break;
	default:
		switch(opcode & 15)
		{
		case  0:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case  1:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case  2:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case  3:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case  4:
			util::stream_format(stream, "MOV.B   %s,@(R0,%s)", regname[Rm], regname[Rn]);
			break;
		case  5:
			util::stream_format(stream, "MOV.W   %s,@(R0,%s)", regname[Rm], regname[Rn]);
			break;
		case  6:
			util::stream_format(stream, "MOV.L   %s,@(R0,%s)", regname[Rm], regname[Rn]);
			break;
		case  7:
			util::stream_format(stream, "MUL.L   %s,%s", regname[Rm], regname[Rn]);
			break;
		case  8:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case  9:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case 10:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case 11:
			util::stream_format(stream, "??????  $%04X", opcode);
			break;
		case 12:
			util::stream_format(stream, "MOV.B   @(R0,%s),%s", regname[Rm], regname[Rn]);
			break;
		case 13:
			util::stream_format(stream, "MOV.W   @(R0,%s),%s", regname[Rm], regname[Rn]);
			break;
		case 14:
			util::stream_format(stream, "MOV.L   @(R0,%s),%s", regname[Rm], regname[Rn]);
			break;
		case 15:
			util::stream_format(stream, "MAC.L   @%s+,@%s+", regname[Rn], regname[Rm]);
			break;
		}
	}
	return flags;
}

static uint32_t op0001(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "MOV.L   %s,@($%02X,%s)", regname[Rm], (opcode & 15) * 4, regname[Rn]);
	return 0;
}

static uint32_t op0010(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0:
		util::stream_format(stream, "MOV.B   %s,@%s", regname[Rm], regname[Rn]);
		break;
	case  1:
		util::stream_format(stream, "MOV.W   %s,@%s", regname[Rm], regname[Rn]);
		break;
	case  2:
		util::stream_format(stream, "MOV.L   %s,@%s", regname[Rm], regname[Rn]);
		break;
	case  3:
		util::stream_format(stream, "??????  $%04X", opcode);
		break;
	case  4:
		util::stream_format(stream, "MOV.B   %s,@-%s", regname[Rm], regname[Rn]);
		break;
	case  5:
		util::stream_format(stream, "MOV.W   %s,@-%s", regname[Rm], regname[Rn]);
		break;
	case  6:
		util::stream_format(stream, "MOV.L   %s,@-%s", regname[Rm], regname[Rn]);
		break;
	case  7:
		util::stream_format(stream, "DIV0S   %s,%s", regname[Rm], regname[Rn]);
		break;
	case  8:
		util::stream_format(stream, "TST     %s,%s", regname[Rm], regname[Rn]);
		break;
	case  9:
		util::stream_format(stream, "AND     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 10:
		util::stream_format(stream, "XOR     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 11:
		util::stream_format(stream, "OR      %s,%s", regname[Rm], regname[Rn]);
		break;
	case 12:
		util::stream_format(stream, "CMP/STR %s,%s", regname[Rm], regname[Rn]);
		break;
	case 13:
		util::stream_format(stream, "XTRCT   %s,%s", regname[Rm], regname[Rn]);
		break;
	case 14:
		util::stream_format(stream, "MULU.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 15:
		util::stream_format(stream, "MULS.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	}
	return 0;
}

static uint32_t op0011(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0:
		util::stream_format(stream, "CMP/EQ  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  1:
		util::stream_format(stream, "??????  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  2:
		util::stream_format(stream, "CMP/HS  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  3:
		util::stream_format(stream, "CMP/GE  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  4:
		util::stream_format(stream, "DIV1    %s,%s", regname[Rm], regname[Rn]);
		break;
	case  5:
		util::stream_format(stream, "DMULU.L %s,%s", regname[Rm], regname[Rn]);
		break;
	case  6:
		util::stream_format(stream, "CMP/HI  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  7:
		util::stream_format(stream, "CMP/GT  %s,%s", regname[Rm], regname[Rn]);
		break;
	case  8:
		util::stream_format(stream, "SUB     %s,%s", regname[Rm], regname[Rn]);
		break;
	case  9:
		util::stream_format(stream, "??????  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 10:
		util::stream_format(stream, "SUBC    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 11:
		util::stream_format(stream, "SUBV    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 12:
		util::stream_format(stream, "ADD     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 13:
		util::stream_format(stream, "DMULS.L %s,%s", regname[Rm], regname[Rn]);
		break;
	case 14:
		util::stream_format(stream, "ADDC    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 15:
		util::stream_format(stream, "ADDV    %s,%s", regname[Rm], regname[Rn]);
		break;
	}
	return 0;
}

static uint32_t op0100(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	uint32_t flags = 0;
	switch(opcode & 0x3F)
	{
	case 0x00:
		util::stream_format(stream, "SHLL    %s", regname[Rn]);
		break;
	case 0x01:
		util::stream_format(stream, "SHLR    %s", regname[Rn]);
		break;
	case 0x02:
		util::stream_format(stream, "STS.L   MACH,@-%s", regname[Rn]);
		break;
	case 0x03:
		util::stream_format(stream, "STC.L   SR,@-%s", regname[Rn]);
		break;
	case 0x04:
		util::stream_format(stream, "ROTL    %s", regname[Rn]);
		break;
	case 0x05:
		util::stream_format(stream, "ROTR    %s", regname[Rn]);
		break;
	case 0x06:
		util::stream_format(stream, "LDS.L   @%s+,MACH", regname[Rn]);
		break;
	case 0x07:
		util::stream_format(stream, "LDC.L   @%s+,SR", regname[Rn]);
		break;
	case 0x08:
		util::stream_format(stream, "SHLL2   %s", regname[Rn]);
		break;
	case 0x09:
		util::stream_format(stream, "SHLR2   %s", regname[Rn]);
		break;
	case 0x0a:
		util::stream_format(stream, "LDS     %s,MACH", regname[Rn]);
		break;
	case 0x0b:
		util::stream_format(stream, "JSR     %s", regname[Rn]);
		flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
		break;
	case 0x0e:
		util::stream_format(stream, "LDC     %s,SR", regname[Rn]);
		break;
	case 0x10:
		util::stream_format(stream, "DT      %s", regname[Rn]);
		break;
	case 0x11:
		util::stream_format(stream, "CMP/PZ  %s", regname[Rn]);
		break;
	case 0x12:
		util::stream_format(stream, "STS.L   MACL,@-%s", regname[Rn]);
		break;
	case 0x13:
		util::stream_format(stream, "STC.L   GBR,@-%s", regname[Rn]);
		break;
	case 0x15:
		util::stream_format(stream, "CMP/PL  %s", regname[Rn]);
		break;
	case 0x16:
		util::stream_format(stream, "LDS.L   @%s+,MACL", regname[Rn]);
		break;
	case 0x17:
		util::stream_format(stream, "LDC.L   @%s+,GBR", regname[Rn]);
		break;
	case 0x18:
		util::stream_format(stream, "SHLL8   %s", regname[Rn]);
		break;
	case 0x19:
		util::stream_format(stream, "SHLR8   %s", regname[Rn]);
		break;
	case 0x1a:
		util::stream_format(stream, "LDS     %s,MACL", regname[Rn]);
		break;
	case 0x1b:
		util::stream_format(stream, "TAS     %s", regname[Rn]);
		break;
	case 0x1e:
		util::stream_format(stream, "LDC     %s,GBR", regname[Rn]);
		break;
	case 0x20:
		util::stream_format(stream, "SHAL    %s", regname[Rn]);
		break;
	case 0x21:
		util::stream_format(stream, "SHAR    %s", regname[Rn]);
		break;
	case 0x22:
		util::stream_format(stream, "STS.L   PR,@-%s", regname[Rn]);
		break;
	case 0x23:
		util::stream_format(stream, "STC.L   VBR,@-%s", regname[Rn]);
		break;
	case 0x24:
		util::stream_format(stream, "ROTCL   %s", regname[Rn]);
		break;
	case 0x25:
		util::stream_format(stream, "ROTCR   %s", regname[Rn]);
		break;
	case 0x26:
		util::stream_format(stream, "LDS.L   @%s+,PR", regname[Rn]);
		break;
	case 0x27:
		util::stream_format(stream, "LDC.L   @%s+,VBR", regname[Rn]);
		break;
	case 0x28:
		util::stream_format(stream, "SHLL16  %s", regname[Rn]);
		break;
	case 0x29:
		util::stream_format(stream, "SHLR16  %s", regname[Rn]);
		break;
	case 0x2a:
		util::stream_format(stream, "LDS     %s,PR", regname[Rn]);
		break;
	case 0x2b:
		util::stream_format(stream, "JMP     %s", regname[Rn]);
		break;
	case 0x2e:
		util::stream_format(stream, "LDC     %s,VBR", regname[Rn]);
		break;
	default:
		if ((opcode & 15) == 15)
			util::stream_format(stream, "MAC.W   @%s+,@%s+", regname[Rm], regname[Rn]);
		else
			util::stream_format(stream, "??????  $%04X", opcode);
	}
	return flags;
}

static uint32_t op0101(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "MOV.L   @($%02X,%s),%s", (opcode & 15) * 4, regname[Rm], regname[Rn]);
	return 0;
}

static uint32_t op0110(std::ostream &stream, uint32_t pc, uint16_t opcode)

{
	switch(opcode & 0xF)
	{
	case 0x00:
		util::stream_format(stream, "MOV.B   @%s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x01:
		util::stream_format(stream, "MOV.W   @%s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x02:
		util::stream_format(stream, "MOV.L   @%s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x03:
		util::stream_format(stream, "MOV     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x04:
		util::stream_format(stream, "MOV.B   @%s+,%s", regname[Rm], regname[Rn]);
		break;
	case 0x05:
		util::stream_format(stream, "MOV.W   @%s+,%s", regname[Rm], regname[Rn]);
		break;
	case 0x06:
		util::stream_format(stream, "MOV.L   @%s+,%s", regname[Rm], regname[Rn]);
		break;
	case 0x07:
		util::stream_format(stream, "NOT     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x08:
		util::stream_format(stream, "SWAP.B  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x09:
		util::stream_format(stream, "SWAP.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0a:
		util::stream_format(stream, "NEGC    %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0b:
		util::stream_format(stream, "NEG     %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0c:
		util::stream_format(stream, "EXTU.B  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0d:
		util::stream_format(stream, "EXTU.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0e:
		util::stream_format(stream, "EXTS.B  %s,%s", regname[Rm], regname[Rn]);
		break;
	case 0x0f:
		util::stream_format(stream, "EXTS.W  %s,%s", regname[Rm], regname[Rn]);
		break;
	}
	return 0;
}

static uint32_t op0111(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "ADD     #$%02X,%s", opcode & 0xff, regname[Rn]);
	return 0;
}

static uint32_t op1000(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	switch((opcode >> 8) & 15)
	{
	case  0:
		util::stream_format(stream, "MOV.B   R0,@($%02X,%s)", (opcode & 15), regname[Rm]);
		break;
	case  1:
		util::stream_format(stream, "MOV.W   R0,@($%02X,%s)", (opcode & 15) * 2, regname[Rm]);
		break;
	case  4:
		util::stream_format(stream, "MOV.B   @($%02X,%s),R0", (opcode & 15), regname[Rm]);
		break;
	case  5:
		util::stream_format(stream, "MOV.W   @($%02X,%s),R0", (opcode & 15), regname[Rm]);
		break;
	case  8:
		util::stream_format(stream, "CMP/EQ  #$%02X,R0", (opcode & 0xff));
		break;
	case  9:
		util::stream_format(stream, "BT      $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	case 11:
		util::stream_format(stream, "BF      $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	case 13:
		util::stream_format(stream, "BTS     $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	case 15:
		util::stream_format(stream, "BFS     $%08X", pc + SIGNX8(opcode & 0xff) * 2 + 2);
		break;
	default :
		util::stream_format(stream, "invalid $%04X", opcode);
	}
	return 0;
}

static uint32_t op1001(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "MOV.W   @($%04X,PC),%s [%08X]", (opcode & 0xff) * 2, regname[Rn], pc+((opcode & 0xff) * 2)+2);
	return 0;
}

static uint32_t op1010(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "BRA     $%08X", SIGNX12(opcode & 0xfff) * 2 + pc + 2);
	return 0;
}

static uint32_t op1011(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "BSR     $%08X", SIGNX12(opcode & 0xfff) * 2 + pc + 2);
	return DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1);
}

static uint32_t op1100(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	uint32_t flags = 0;
	switch((opcode >> 8) & 15)
	{
	case  0:
		util::stream_format(stream, "MOV.B   R0,@($%02X,GBR)", opcode & 0xff);
		break;
	case  1:
		util::stream_format(stream, "MOV.W   R0,@($%04X,GBR)", (opcode & 0xff) * 2);
		break;
	case  2:
		util::stream_format(stream, "MOV.L   R0,@($%04X,GBR)", (opcode & 0xff) * 4);
		break;
	case  3:
		util::stream_format(stream, "TRAPA   #$%02X", opcode & 0xff);
		flags = DASMFLAG_STEP_OVER;
		break;
	case  4:
		util::stream_format(stream, "MOV.B   @($%02X,GBR),R0", opcode & 0xff);
		break;
	case  5:
		util::stream_format(stream, "MOV.W   @($%04X,GBR),R0", (opcode & 0xff) * 2);
		break;
	case  6:
		util::stream_format(stream, "MOV.L   @($%04X,GBR),R0", (opcode & 0xff) * 4);
		break;
	case  7:
		util::stream_format(stream, "MOVA    @($%04X,PC),R0 [%08X]", (opcode & 0xff) * 4, ((pc + 2) & ~3) + (opcode & 0xff) * 4);
		break;
	case  8:
		util::stream_format(stream, "TST     #$%02X,R0", opcode & 0xff);
		break;
	case  9:
		util::stream_format(stream, "AND     #$%02X,R0", opcode & 0xff);
		break;
	case 10:
		util::stream_format(stream, "XOR     #$%02X,R0", opcode & 0xff);
		break;
	case 11:
		util::stream_format(stream, "OR      #$%02X,R0", opcode & 0xff);
		break;
	case 12:
		util::stream_format(stream, "TST.B   #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	case 13:
		util::stream_format(stream, "AND.B   #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	case 14:
		util::stream_format(stream, "XOR.B   #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	case 15:
		util::stream_format(stream, "OR.B    #$%02X,@(R0,GBR)", opcode & 0xff);
		break;
	}
	return flags;
}

static uint32_t op1101(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "MOV.L   @($%02X,PC),%s [%08X]", (opcode * 4) & 0xff, regname[Rn], ((pc + 2) & ~3) + (opcode & 0xff) * 4);
	return 0;
}

static uint32_t op1110(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "MOV     #$%02X,%s", (opcode & 0xff), regname[Rn]);
	return 0;
}

static uint32_t op1111(std::ostream &stream, uint32_t pc, uint16_t opcode)
{
	util::stream_format(stream, "unknown $%04X", opcode);
	return 0;
}

unsigned DasmSH2(std::ostream &stream, unsigned pc, uint16_t opcode)
{
	uint32_t flags;

	pc += 2;

	switch ((opcode >> 12) & 15)
	{
	case  0: flags = op0000(stream, pc, opcode); break;
	case  1: flags = op0001(stream, pc, opcode); break;
	case  2: flags = op0010(stream, pc, opcode); break;
	case  3: flags = op0011(stream, pc, opcode); break;
	case  4: flags = op0100(stream, pc, opcode); break;
	case  5: flags = op0101(stream, pc, opcode); break;
	case  6: flags = op0110(stream, pc, opcode); break;
	case  7: flags = op0111(stream, pc, opcode); break;
	case  8: flags = op1000(stream, pc, opcode); break;
	case  9: flags = op1001(stream, pc, opcode); break;
	case 10: flags = op1010(stream, pc, opcode); break;
	case 11: flags = op1011(stream, pc, opcode); break;
	case 12: flags = op1100(stream, pc, opcode); break;
	case 13: flags = op1101(stream, pc, opcode); break;
	case 14: flags = op1110(stream, pc, opcode); break;
	default: flags = op1111(stream, pc, opcode); break;
	}
	return 2 | flags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE(sh2)
{
	return DasmSH2(stream, pc, (oprom[0] << 8) | oprom[1]);
}
