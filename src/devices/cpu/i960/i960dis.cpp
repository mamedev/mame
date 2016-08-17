// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
/*
    Intel i960 disassembler

    By Farfetch'd and R. Belmont
*/

#include "emu.h"
#include "i960.h"
#include "i960dis.h"

struct mnemonic_t
{
	const char      *mnem;
	unsigned short  type;
};


static const mnemonic_t mnemonic[256] = {
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // 00
	{ "b", 8 }, { "call", 8 }, { "ret", 9 }, { "bal", 8 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "bno", 8 }, { "bg", 8 }, { "be", 8 }, { "bge", 8 }, { "bl", 8 }, { "bne", 8 }, { "ble", 8 }, { "bo", 8 }, // 10
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "testno", 10 }, { "testg", 10 }, { "teste", 10 }, { "testge", 10 }, { "testl", 10 }, { "testne", 10 }, { "testle", 10 }, { "testo", 10 }, // 20
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "bbc", 6 }, { "cmpobg", 7 }, { "cmpobe", 7 }, { "cmpobge", 7 }, { "cmpobl", 7 }, { "cmpobne", 7 }, { "cmpoble", 7 }, { "bbs", 6 }, // 30
	{ "cmpibno", 7 }, { "cmpibg", 7 }, { "cmpibe", 7 }, { "cmpibge", 7 }, { "cmpibl", 7 }, { "cmpibne", 7 }, { "cmpible", 7 }, { "cmpibo", 7 },

	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // 40
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // 50
	{ "58", 3 }, { "59", 3 }, { "5A", 3 }, { "5B", 3 }, { "5C", 2 }, { "5D", 3 }, { "?", 0 }, { "5F", 3 },

	{ "synmov", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "64", 3 }, { "65", 3 }, { "calls", 0 }, { "67", 3 }, // 60
	{ "68", 3 }, { "69", 3 }, { "?", 0 }, { "?", 0 }, { "6C", 3 }, { "6D", 3 }, { "6E", 3 }, { "?", 0 },

	{ "70", 3 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "74", 3 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // 70
	{ "78", 3 }, { "79", 3 }, { "7A", 3 }, { "7B", 3 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "ldob", 1 }, { "?", 0 }, { "stob", 1 }, { "?", 0 }, { "bx", 1 }, { "balx", 1 }, { "callx", 1 }, { "?", 0 }, // 80
	{ "ldos", 1 }, { "?", 0 }, { "stos", 1 }, { "?", 0 }, { "lda", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "ld", 1 }, { "?", 0 }, { "st", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // 90
	{ "ldl", 1 }, { "?", 0 }, { "stl", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "ldt", 1 }, { "?", 0 }, { "stt", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // a0
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "ldq", 1 }, { "?", 0 }, { "stq", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // b0
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "ldib", 1 }, { "?", 0 }, { "stib", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // c0
	{ "ldis", 1 }, { "?", 0 }, { "stis", 1 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // d0
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // e0
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 },

	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, // f0
	{ "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }, { "?", 0 }
};

static const mnemonic_t mnem_reg[100] =
{
	{ "notbit", 0x580 }, { "and", 0x581 }, { "andnot", 0x582 }, { "setbit", 0x583 }, { "notand",0x584 },
	{ "xor", 0x586 }, { "or", 0x587 }, { "nor", 0x588 }, { "xnor",0x589 }, { "not",0x58a },
	{ "clrbit", 0x58c },
	{ "addo", 0x590 }, { "addi",0x591 }, { "subo",0x592 }, { "subi",0x593 }, { "cmpob",0x594 }, { "shro",0x598 }, { "shrdi",0x59a }, { "shri",0x59b }, { "shlo",0x59c }, { "rotate",0x59d }, { "shli",0x59e },
	{ "cmpo",0x5a0 }, { "cmpi",0x5a1 }, { "concmpo",0x5a2 }, { "concmpi",0x5a3 }, { "cmpinco",0x5a4 }, { "cmpinci",0x5a5 }, { "cmpdeco",0x5a6 }, { "cmpdeci",0x5a7 }, { "scanbyte",0x5ac }, { "bswap",0x5ad }, { "chkbit",0x5ae },
	{ "addc",0x5b0 }, { "subc",0x5b2 },
	{ "mov", 0x5cc },
	{ "movl",0x5dc },
	{ "movq",0x5fc },
	{ "scanbit", 0x641 }, { "daddc", 0x642 }, { "dsubc", 0x643 }, { "dmovt", 0x644 }, { "modac",0x645 },
	{ "modify",0x650 }, { "extract",0x651 }, { "modtc",0x654 }, { "modpc",0x655 },
	{ "emul",0x670 }, { "ediv",0x671 }, { "cvtir", 0x674 }, { "cvtilr", 0x675 }, { "scalerl", 0x676 }, { "scaler", 0x677 },
	{ "atanr",0x680 }, { "logepr", 0x681 }, { "logr", 0x682 }, { "remr", 0x683 }, { "cmpor", 0x684 }, { "cmpr", 0x685 }, { "sqrtr", 0x688 },
	{ "expr", 0x689 }, { "logbnr", 0x68a }, { "roundr", 0x68b }, { "sinr", 0x68c }, { "cosr", 0x68d }, { "tanr", 0x68e }, { "classr", 0x68f },
	{ "atanrl",0x690 }, { "logeprl", 0x691 }, { "logrl", 0x692 }, { "remrl", 0x693 }, { "cmporl", 0x694 }, { "cmprl", 0x695 }, { "sqrtrl", 0x698 },
	{ "exprl", 0x699 }, { "logbnrl", 0x69a }, { "roundrl", 0x69b }, { "sinrl", 0x69c }, { "cosrl", 0x69d }, { "tanrl", 0x69e }, { "classrl", 0x69f },
	{ "cvtri", 0x6c0 }, { "cvtril", 0x6c1 }, { "cvtzri", 0x6c2 }, { "cvtzril", 0x6c3 }, { "movr", 0x6c9 },
	{ "movrl", 0x6d9 },
	{ "movre", 0x6e1 }, { "cpysre", 0x6e2 }, { "cpyrsre", 0x6e3 },
	{ "mulo", 0x701 }, { "remo",0x708 }, { "divo",0x70b },
	{ "muli",0x741 }, { "remi",0x748 }, { "modi",0x749 }, { "divi",0x74b },
	{ "ending_code",0 }
};

static const char *const constnames[32] =
{
	"0x0", "0x1", "0x2", "0x3", "0x4", "0x5", "0x6", "0x7", "0x8", "0x9", "0xa", "0xb", "0xc", "0xd", "0xe", "0xf",
	"0x10", "0x11", "0x12", "0x13", "0x14", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1a", "0x1b", "0x1c", "0x1d", "0x1e", "0x1f"
};

static const char *const regnames[32] =
{
	"pfp","sp","rip","r3", "r4","r5","r6","r7", "r8","r9","r10","r11", "r12","r13","r14","r15",
	"g0","g1","g2","g3", "g4","g5","g6","g7", "g8","g9","g10","g11", "g12","g13","g14","fp",
};

#define REG_DST     regnames[dst]
#define REG_ABASE   regnames[abase]
#define REG_REG2    regnames[reg2]
#define REG_COBR_SRC1   ((iCode & 0x2000) ? constnames[COBRSRC1] : regnames[COBRSRC1])
#define REG_COBR_SRC2   regnames[COBRSRC2]
#define NEM         mnemonic[op].mnem

// REG format
#define SRC1 (iCode & 0x1f)
#define S1   ((iCode >> 5) & 0x1)
#define S2   ((iCode >> 6) & 0x1)
#define OP2  ((iCode >> 7) & 0xf)
#define M1   ((iCode >> 11) & 0x1)
#define M2   ((iCode >> 12) & 0x1)
#define M3   ((iCode >> 13) & 0x1)
#define SRC2 ((iCode >> 14) & 0x1f)
#define DST  ((iCode >> 19) & 0x1f)
#define OP   ((iCode >> 24) & 0xff)

// COBR format
#define COBRSRC1 ((iCode >> 19) & 0x1f)
#define COBRSRC2 ((iCode >> 14) & 0x1f)

static char *dis_decode_reg(unsigned long iCode, char* tmpStr,unsigned char cnt)
{
	char src1[10];
	char src2[10];
	char dst[10];

	if (S1) src1[0] = 0;
	else
	{
		if(M1)  sprintf(src1,"0x%lx",SRC1);
		else        sprintf(src1,"%s",regnames[SRC1]);
	}
	if (S2) sprintf(src2,"reserved");
	else
	{
		if(M2)  sprintf(src2,"0x%lx,",SRC2);
		else        sprintf(src2,"%s,",regnames[SRC2]);
	}
	if(M3)      dst[0] = 0;
	else            sprintf(dst,"%s,",regnames[DST]);
	if (cnt == 1)
		sprintf(tmpStr,"%s%s",dst,src1);
	else
		sprintf(tmpStr,"%s%s%s",dst,src2,src1);
	return tmpStr;
}

#define READ32(dis,offs) ((dis)->oprom[(offs) + 0] | ((dis)->oprom[(offs) + 1] << 8) | ((dis)->oprom[(offs) + 2] << 16) | ((dis)->oprom[(offs) + 3] << 24))

static char *i960_disassemble(disassemble_t *diss)
{
	unsigned char op,op2;
	unsigned char /*mode,*/ modeh, model;
	unsigned char dst,abase,reg2;
	unsigned short opc;
	unsigned long iCode;
	char tmpStr[256];
	long i;

	iCode = READ32(diss,0);
	op = (unsigned char) (iCode >> 24);
	op2 = (unsigned char) (iCode >> 7)&0xf;

	model = (unsigned char) (iCode >> 10) &0x3;
	modeh = (unsigned char) (iCode >> 12) &0x3;
	//mode = (unsigned char) (iCode >> 10) &0x7;
	dst = (unsigned char) (iCode >> 19) &0x1f;
	abase = (unsigned char) (iCode>>14)&0x1f;
	reg2 = (unsigned char) (iCode)&0x1f;

	sprintf(diss->buffer,"???");
	diss->IPinc = 4;
	diss->disflags = 0;

	if (op == 0x09 || op == 0x0b || op == 0x66 || op == 0x85 || op == 0x86)
		diss->disflags = DASMFLAG_STEP_OVER;
	else if (op == 0x0a)
		diss->disflags = DASMFLAG_STEP_OUT;

	switch(mnemonic[op].type)
	{
	case 0: // not yet implemented
		sprintf(diss->buffer,"%s %02x:%01x %08lx %1x %1x",mnemonic[op].mnem,op,op2,iCode, modeh, model);
		break;
	case 1: // memory access
		switch(modeh)
		{
		case 0:
			sprintf(diss->buffer, "%-8s%s,0x%lx",NEM,REG_DST, iCode&0xfff);
			break;
		case 1:
			switch (model)
			{
			case 0:
				sprintf(diss->buffer, "%-8s%s,(%s)",NEM,REG_DST, REG_ABASE);
				break;
			case 3:
				sprintf(diss->buffer, "%-8s%s,(%s)[%s*%ld]",NEM,REG_DST, REG_ABASE,REG_REG2,(iCode>>7)&0x7);
				break;
			default:
				sprintf(diss->buffer,"%s %02x:%01x %08lx %1x %1x",mnemonic[op].mnem,op,op2,iCode, modeh, model);
				break;
			}
			break;
		case 2:
			sprintf(diss->buffer, "%-8s%s,0x%lx(%s)",NEM,REG_DST, iCode&0xfff,REG_ABASE);
			break;
		case 3:
			switch (model)
			{
			case 0:
				sprintf(diss->buffer, "%-8s%s,0x%x",NEM,REG_DST, READ32(diss,4));
				diss->IPinc = 8;
				break;
			case 1:
				sprintf(diss->buffer, "%-8s%s,0x%x(%s)",NEM,REG_DST, READ32(diss,4),REG_ABASE);
				diss->IPinc = 8;
				break;
			case 2:
				sprintf(diss->buffer, "%-8s%s,0x%x[%s*%ld]",NEM,REG_DST, READ32(diss,4),REG_REG2,(iCode>>7)&0x7);
				diss->IPinc = 8;
				break;
			case 3:
				sprintf(diss->buffer, "%-8s%s,0x%x(%s)[%s*%ld]",NEM,REG_DST, READ32(diss,4),REG_ABASE,REG_REG2,(iCode>>7)&0x7);
				diss->IPinc = 8;
				break;
			default:
				sprintf(diss->buffer,"%s %02x:%01x %08lx %1x %1x",mnemonic[op].mnem,op,op2,iCode, modeh, model);
				break;
			}
			break;
		default:
			sprintf(diss->buffer,"%s %02x:%01x %08lx %1x %1x",mnemonic[op].mnem,op,op2,iCode, modeh, model);
			break;
		}
		break;
	case 2:
		i = 0;
		opc = op<<4|op2;

		while(mnem_reg[i].type != 0)
		{
			if (mnem_reg[i].type == opc) break;
			i++;
		}

		if (mnem_reg[i].type == opc) sprintf(diss->buffer, "%-8s%s", mnem_reg[i].mnem,dis_decode_reg(iCode,tmpStr,1));
		else sprintf(diss->buffer,"%s %02x:%01x %08lx %1x %1x",mnemonic[op].mnem,op,op2,iCode, modeh, model);
		break;
	case 3:
		i = 0;
		opc = op<<4|op2;

		while(mnem_reg[i].type != 0)
		{
			if (mnem_reg[i].type == opc) break;
			i++;
		}

		if (mnem_reg[i].type == opc) sprintf(diss->buffer, "%-8s%s", mnem_reg[i].mnem,dis_decode_reg(iCode,tmpStr,0));
		else sprintf(diss->buffer,"%s %02x:%01x %08lx %1x %1x",mnemonic[op].mnem,op,op2,iCode, modeh, model);
		break;

	case 6: // bitpos and branch type
		sprintf(diss->buffer, "%-8s%ld,%s,0x%lx",NEM, COBRSRC1, REG_COBR_SRC2,((((long)iCode&0x00fffffc)<<19)>>19) + (diss->IP));
		break;
	case 7: // compare and branch type
		sprintf(diss->buffer, "%-8s%s,%s,0x%lx",NEM,REG_COBR_SRC1,REG_COBR_SRC2,((((long)iCode&0x00fffffc)<<19)>>19) + (diss->IP));
		break;
	case 8: // target type
		sprintf(diss->buffer, "%-8s%08lx",NEM,((((long)iCode&0x00fffffc)<<8)>>8) + (diss->IP));
		break;
	case 9: // no operands
		sprintf(diss->buffer, "%s",NEM);
		break;
	case 10: // TEST type: register only
		sprintf(diss->buffer, "%s %s", NEM, REG_DST);
		break;
	}
	return diss->buffer;
}



CPU_DISASSEMBLE( i960  )
{
	disassemble_t dis;

	dis.IP = pc;
	dis.buffer = buffer;
	dis.oprom = oprom;

	i960_disassemble(&dis);

	return dis.IPinc | dis.disflags | DASMFLAG_SUPPORTED;
}
