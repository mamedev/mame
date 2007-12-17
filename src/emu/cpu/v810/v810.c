/********************************************
 NEC V810 (upd70732) core
  Tomasz Slanina - analog[at]op.pl

 Change Log
 - 20/07/2004 - first public release


 TODO:
    - CY flag in few floating point opcodes
        (all floating point opcodes are NOT tested!)
  - traps/interrupts/exceptions
  - bitstring opcodes
  - timing
  - missing opcodes : reti , trap

********************************************/

#include "debugger.h"
#include "v810.h"

#define clkIF 3
#define clkMEM 3

typedef struct
{
	UINT32 reg[65];
	UINT8 irq_line;
	UINT8 nmi_line;
	int (*irq_cb)(int irqline);
	UINT32 PPC;
	UINT32 op;
} v810info;

static v810info v810;

int v810_ICount;

#define R0 v810.reg[0]
#define R1 v810.reg[1]
#define R2 v810.reg[2]
#define SP v810.reg[3]
#define R4 v810.reg[4]
#define R5 v810.reg[5]
#define R6 v810.reg[6]
#define R7 v810.reg[7]
#define R8 v810.reg[8]
#define R9 v810.reg[9]
#define R10 v810.reg[10]
#define R11 v810.reg[11]
#define R12 v810.reg[12]
#define R13 v810.reg[13]
#define R14 v810.reg[14]
#define R15 v810.reg[15]
#define R16 v810.reg[16]
#define R17 v810.reg[17]
#define R18 v810.reg[18]
#define R19 v810.reg[19]
#define R20 v810.reg[20]
#define R21 v810.reg[21]
#define R22 v810.reg[22]
#define R23 v810.reg[23]
#define R24 v810.reg[24]
#define R25 v810.reg[25]
#define R26 v810.reg[26]
#define R27 v810.reg[27]
#define R28 v810.reg[28]
#define R29 v810.reg[29]
#define R30 v810.reg[30]
#define R31 v810.reg[31]

#define EIPC	v810.reg[32]
#define EIPSW	v810.reg[33]
#define FEPC	v810.reg[34]
#define FEPSW	v810.reg[35]
#define ECR	v810.reg[36]
#define PSW	v810.reg[37]
#define PIR	v810.reg[38]
#define TKCW	v810.reg[39]
#define CHCW	v810.reg[56]
#define ADTRE	v810.reg[57]

#define OP	v810.op
#define PC	v810.reg[64]

/* Flags */
#define GET_Z					( PSW & 0x00000001)
#define GET_S					((PSW & 0x00000002)>>1)
#define GET_OV				((PSW & 0x00000004)>>2)
#define GET_CY				((PSW & 0x00000008)>>3)
#define GET_ID				((PSW & 0x00008000)>>15)
#define GET_EP				((PSW & 0x00010000)>>16)
#define GET_NP				((PSW & 0x00020000)>>17)
#define GET_AE				((PSW & 0x00040000)>>18)

#define SET_Z(val)				(PSW = (PSW & ~0x00000001) | (val))
#define SET_S(val)				(PSW = (PSW & ~0x00000002) | ((val) << 1))
#define SET_OV(val)				(PSW = (PSW & ~0x00000004) | ((val) << 2))
#define SET_CY(val)				(PSW = (PSW & ~0x00000008) | ((val) << 3))
#define SET_ID(val)				(PSW = (PSW & ~0x00008000) | ((val) << 15))
#define SET_EP(val)				(PSW = (PSW & ~0x00010000) | ((val) << 16))
#define SET_NP(val)				(PSW = (PSW & ~0x00020000) | ((val) << 17))
#define SET_AE(val)				(PSW = (PSW & ~0x00040000) | ((val) << 18))




static void SETREG(UINT32 reg,UINT32 val)
{
	if(reg)
		v810.reg[reg]=val;
}

static UINT32 GETREG(UINT32 reg)
{
	if(reg)
		return v810.reg[reg];
	else
		return 0;
}

static UINT32 opUNDEF(void)
{
	logerror("V810: Unknown opcode %x @ %x",OP,PC-2);
	return clkIF;
}

static UINT32 opMOVr(void) // mov reg1, reg2
{
	SETREG(GET2,GETREG(GET1));
	return clkIF;
}

static UINT32 opMOVEA(void)	// movea imm16, reg1, reg2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=I16(op2);
	SETREG(GET2,op1+op2);
	return clkIF;
}

static UINT32 opMOVHI(void) 	// movhi imm16, reg1 ,reg2
{
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2<<=16;
	SETREG(GET2,GETREG(GET1)+op2);
	return clkIF;
}

static UINT32 opMOVi(void) 	// mov imm5,r2
{
	SETREG(GET2,I5(OP));
	return clkIF;
}

static UINT32 opADDr(void)	// add r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return clkIF;
}

static UINT32 opADDi(void)	// add imm5,r2
{
	UINT32 op1=I5(OP);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return clkIF;
}


static UINT32 opADDI(void)	// addi imm16, reg1, reg2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=R_OP(PC);
	UINT64 res;
	PC+=2;
	op2=I16(op2);
	res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return clkIF;
}

static UINT32 opSUBr(void)	// sub r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return clkIF;
}


static UINT32 opCMPr(void)	// cmp r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return clkIF;
}

static UINT32 opCMPi(void)	// cmpi imm5,r2
{
	UINT32 op1=I5(OP);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return clkIF;
}

static UINT32 opSETFi(void)	// setf imm5,r2
{
	UINT32 op1=I5(OP);
	UINT32 op2=PSW&0xf;
	op1&=0xf;
	SETREG(GET2,(op1==op2)?1:0);
	return clkIF;
}


static UINT32 opANDr(void)	// and r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}

static UINT32 opANDI(void)	// andi imm16,r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(GET2,op2);
	return clkIF;
}

static UINT32 opORr(void)	// or r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}

static UINT32 opORI(void)	// ori imm16,r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(GET2,op2);
	return clkIF;
}

static UINT32 opXORr(void)	// xor r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}


static UINT32 opLDSR(void) // ldsr reg2,regID
{
	UINT32 op1=UI5(OP);
	SETREG(32+op1,GETREG(GET2));
	return clkIF;
}

static UINT32 opSTSR(void) // ldsr regID,reg2
{
	UINT32 op1=UI5(OP);
	SETREG(GET2,GETREG(32+op1));
	return clkIF;
}


static UINT32 opXORI(void)	// xori imm16,r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(GET2,op2);
	return clkIF;
}

static UINT32 opNOTr(void)	// not r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=~op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}

static UINT32 opSHLr(void)	// shl r1,r2
{
	UINT64 tmp;
	UINT32 count=GETREG(GET1);
	count&=0x1f;

	SET_OV(0);
	SET_CY(0);

	if(count)
	{
		tmp=GETREG(GET2);
		tmp<<=count;
		CHECK_CY(tmp);
		SETREG(GET2,tmp&0xffffffff);
		CHECK_ZS(GETREG(GET2));
	}
	return clkIF;
}

static UINT32 opSHLi(void)	// shl imm5,r2
{
	UINT64 tmp;
	UINT32 count=UI5(OP);

	SET_OV(0);
	SET_CY(0);

	if(count)
	{
		tmp=GETREG(GET2);
		tmp<<=count;
		CHECK_CY(tmp);
		SETREG(GET2,tmp&0xffffffff);
	}
	CHECK_ZS(GETREG(GET2));
	return clkIF;
}

static UINT32 opSHRr(void)	// shr r1,r2
{
	UINT64 tmp;
	UINT32 count=GETREG(GET1);
	count&=0x1f;
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		SETREG(GET2,(tmp>>1)&0xffffffff);
	}
	CHECK_ZS(GETREG(GET2));
	return clkIF;
}

static UINT32 opSHRi(void)	// shr imm5,r2
{
	UINT64 tmp;
	UINT32 count=UI5(OP);
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		tmp>>=1;
		SETREG(GET2,tmp&0xffffffff);
	}
	CHECK_ZS(GETREG(GET2));
	return clkIF;
}

static UINT32 opSARr(void)	// sar r1,r2
{
	INT32 tmp;
	UINT32 count=GETREG(GET1);
	count&=0x1f;
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		tmp>>=1;
		SETREG(GET2,tmp);
	}
	CHECK_ZS(GETREG(GET2));
	return clkIF;
}

static UINT32 opSARi(void)	// sar imm5,r2
{
	INT32 tmp;
	UINT32 count=UI5(OP);
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		tmp>>=1;
		SETREG(GET2,tmp);
	}
	CHECK_ZS(GETREG(GET2));
	return clkIF;
}

static UINT32 opJMPr(void)
{
	PC=GETREG(GET1)&~1;
	return clkIF+2;
}


static UINT32 opJR(void)
{
	UINT32 tmp=R_OP(PC);
	PC=PC-2+(D26(OP,tmp)&~1);
	return clkIF+2;
}

static UINT32 opJAL(void)
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	R31=PC;
	PC+=D26(OP,tmp);
	PC-=4;
	PC&=~1;
	return clkIF+2;
}


static UINT32 opEI(void)
{
	SET_ID(0);
	return clkIF;
}

static UINT32 opDI(void)
{
	SET_ID(1);
	return clkIF;
}

static UINT32 opHALT(void)
{
	logerror("V810: HALT @ %X",PC-2);
	return clkIF;
}

static UINT32 opB(void)
{
	int doBranch=0;
	switch((OP>>9)&0xf)
	{
		case 0: //bv
			doBranch=GET_OV;
		break;

		case 1: //bl
			doBranch=GET_CY;
		break;

		case 2: //be
			doBranch=GET_Z;
		break;

		case 3: //bnh
			doBranch=GET_Z||GET_CY;
		break;

		case 4: //bn
			doBranch=GET_S;
		break;

		case 5: //br
			doBranch=1;
		break;

		case 6: //blt
			doBranch=GET_S^GET_OV;
		break;

		case 7: //ble
			doBranch=GET_Z||(GET_S^GET_OV);
		break;

		case 8: //bnv
			doBranch=!GET_OV;
		break;

		case 9: //bnl
			doBranch=!GET_CY;
		break;

		case 10: //bne
			doBranch=!GET_Z;
		break;

		case 11: //bh
			doBranch=!(GET_Z||GET_CY);
		break;

		case 12: //bp
			doBranch=!GET_S;
		break;

		case 13: //nop

		break;

		case 14: //bge
			doBranch=!(GET_OV^GET_S);
		break;

		case 15: //bgt
			doBranch=!(GET_Z||(GET_OV^GET_S));
		break;
	}
	if(doBranch)
	{
			PC=PC-2+(D9(OP)&~1);
	}
	return clkIF;
}

static UINT32 opLDB(void)	// ld.b disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=R_B(tmp);
	tmp|=(tmp&0x80)?0xffffff00:0;
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opLDH(void)	// ld.h disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=R_H(tmp&~1);
	tmp|=(tmp&0x8000)?0xffff0000:0;
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opLDW(void)	// ld.w disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=R_W(tmp&~3);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opINB(void)	// in.b disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=RIO_B(tmp);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opCAXI(void)	// caxi disp16[reg1],reg2
{
	PC+=2;
	return clkIF;
}

static UINT32 opINH(void)	// in.h disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=RIO_H(tmp&~1);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opINW(void)	// in.w disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=RIO_W(tmp&~3);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opSTB(void)	// st.b reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_B(tmp,GETREG(GET2)&0xff);
	return clkIF+clkMEM;
}

static UINT32 opSTH(void)	// st.h reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_H(tmp&~1,GETREG(GET2)&0xffff);
	return clkIF+clkMEM;
}

static UINT32 opSTW(void)	// st.w reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_W(tmp&~3,GETREG(GET2));
	return clkIF+clkMEM;
}

static UINT32 opOUTB(void)	// out.b reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_B(tmp,GETREG(GET2)&0xff);
	return clkIF+clkMEM;
}

static UINT32 opOUTH(void)	// out.h reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_H(tmp&~1,GETREG(GET2)&0xffff);
	return clkIF+clkMEM;
}

static UINT32 opOUTW(void)	// out.w reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_W(tmp&~3,GETREG(GET2));
	return clkIF+clkMEM;
}

static UINT32 opMULr(void)	// mul r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	UINT64 tmp;
	tmp=(INT64)(INT32)op1*(INT64)(INT32)op2;
	op2=tmp&0xffffffff;
	tmp>>=32;
	CHECK_ZS(tmp);//z = bad!
	SET_Z( (tmp|op2)==0 );
	SET_OV((tmp!=0));
	SET_CY((tmp!=0));
	SETREG(GET2,op2);
	SETREG(30,tmp);
	return clkIF;
}

static UINT32 opMULUr(void)	// mulu r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	UINT64 tmp;
	tmp=(UINT64)op1*(UINT64)op2;
	op2=tmp&0xffffffff;
	tmp>>=32;
	CHECK_ZS(tmp);//z = bad!
	SET_Z( (tmp|op2)==0 );
	SET_OV((tmp!=0));
	SET_CY((tmp!=0));
	SETREG(GET2,op2);
	SETREG(30,tmp);
	return clkIF;
}

static UINT32 opDIVr(void)	// div r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	if(op1)
	{
		SETREG(30,(INT32)((INT32)op2%(INT32)op1));
		SETREG(GET2,(INT32)((INT32)op2/(INT32)op1));
		SET_OV((op1^op2^GETREG(GET2)) == 0x80000000);
		CHECK_ZS(GETREG(GET2));
	}
	return clkIF;
}

static UINT32 opDIVUr(void)	// divu r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	if(op1)
	{
		SETREG(30,(INT32)(op2%op1));
		SETREG(GET2,(INT32)(op2/op1));
		SET_OV((op1^op2^GETREG(GET2)) == 0x80000000);
		CHECK_ZS(GETREG(GET2));
	}
	return clkIF;
}

static void opADDF(void)
{
	//TODO: CY
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	val2+=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(GET2,f2u(val2));
}

static void opSUBF(void)
{
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	SET_CY((val2<val1)?1:0);
	val2-=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(GET2,f2u(val2));
}

static void opMULF(void)
{
	//TODO: CY
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	val2*=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(GET2,f2u(val2));
}

static void opDIVF(void)
{
	//TODO: CY
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	if(val1!=0)
		val2/=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(GET2,f2u(val2));
}

static void opTRNC(void)
{
	float val1=u2f(GETREG(GET1));
	SET_OV(0);
	SET_Z((val1==0.0)?1:0);
	SET_S((val1<0.0)?1:0);
	SETREG(GET2,(INT32)val1);
}

static void opCMPF(void)
{
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	SET_CY((val2<val1)?1:0);
	val2-=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
}

static void opCVTS(void)
{
	float val1=u2f(GETREG(GET1));
	SET_OV(0);
	SET_Z((val1==0.0)?1:0);
	SET_S((val1<0.0)?1:0);
	SETREG(GET2,(INT32)val1);
}

static void opCVTW(void)
{
	//TODO: CY
	float val1=GETREG(GET1);
	SET_OV(0);
	SET_Z((val1==0.0)?1:0);
	SET_S((val1<0.0)?1:0);
	SETREG(GET2,f2u(val1));
}

static UINT32 opFpoint(void)
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	switch((tmp&0xfc00)>>10)
	{
			case 0x0: opCMPF();break;
			case 0x2: opCVTW();break;
			case 0x3: opCVTS();break;
			case 0x4: opADDF();break;
			case 0x5: opSUBF();break;
			case 0x6: opMULF();break;
			case 0x7: opDIVF();break;
			case 0xb: opTRNC();break;
	}
	return clkIF+1;
}

static UINT32 (*OpCodeTable[64])(void) =
{
	/* 0x00 */ opMOVr,  	// mov r1,r2            1
	/* 0x01 */ opADDr,  	// add r1,r2            1
	/* 0x02 */ opSUBr,  	// sub r1,r2            1
	/* 0x03 */ opCMPr,  	// cmp2 r1,r2           1
	/* 0x04 */ opSHLr,  	// shl r1,r2            1
	/* 0x05 */ opSHRr,  	// shr r1,r2            1
	/* 0x06 */ opJMPr,  	// jmp [r1]             1
	/* 0x07 */ opSARr,  	// sar r1,r2            1
	/* 0x08 */ opMULr,  	// mul r1,r2            1
	/* 0x09 */ opDIVr,  	// div r1,r2            1
	/* 0x0a */ opMULUr, 	// mulu r1,r2           1
	/* 0x0b */ opDIVUr, 	// divu r1,r2           1
	/* 0x0c */ opORr,   	// or r1,r2             1
	/* 0x0d */ opANDr,  	// and r1,r2            1
	/* 0x0e */ opXORr,  	// xor r1,r2            1
	/* 0x0f */ opNOTr,  	// not r1,r2            1
	/* 0x10 */ opMOVi,  	// mov imm5,r2          2
	/* 0x11 */ opADDi,  	// add imm5,r2          2
	/* 0x12 */ opSETFi, 	// setf imm5,r2         2
	/* 0x13 */ opCMPi,  	// cmp imm5,r2          2
	/* 0x14 */ opSHLi,  	// shl imm5,r2          2
	/* 0x15 */ opSHRi,  	// shr imm5,r2          2
	/* 0x16 */ opEI,    	// ei               2
	/* 0x17 */ opSARi,  	// sar imm5,r2          2
	/* 0x18 */ opUNDEF,
	/* 0x19 */ opUNDEF,
	/* 0x1a */ opHALT,  	// halt             2
	/* 0x1b */ opUNDEF,
	/* 0x1c */ opLDSR,  	// ldsr reg2,regID          2
	/* 0x1d */ opSTSR,  	// stsr regID,reg2          2
	/* 0x1e */ opDI,  	// DI               2
	/* 0x1f */ opUNDEF,
	/* 0x20 */ opB,  	// Branch (7 bit opcode)
	/* 0x21 */ opB,  	// Branch (7 bit opcode)
	/* 0x22 */ opB,  	// Branch (7 bit opcode)
	/* 0x23 */ opB,  	// Branch (7 bit opcode)
	/* 0x24 */ opB,  	// Branch (7 bit opcode)
	/* 0x25 */ opB,  	// Branch (7 bit opcode)
	/* 0x26 */ opB,  	// Branch (7 bit opcode)
	/* 0x27 */ opB,  	// Branch (7 bit opcode)
	/* 0x28 */ opMOVEA,  	// movea imm16, reg1, reg2  5
	/* 0x29 */ opADDI,  	// addi imm16, reg1, reg2   5
	/* 0x2a */ opJR,  	// jr disp26            4
	/* 0x2b */ opJAL,  	// jal disp26           4
	/* 0x2c */ opORI,  	// ori imm16, reg1, reg2    5
	/* 0x2d */ opANDI,  	// andi imm16, reg1, reg2   5
	/* 0x2e */ opXORI,  	// xori imm16, reg1, reg2   5
	/* 0x2f */ opMOVHI,  	// movhi imm16, reg1 ,reg2  5
	/* 0x30 */ opLDB,  	// ld.b disp16[reg1],reg2   6a
	/* 0x31 */ opLDH,  	// ld.h disp16[reg1],reg2   6a
	/* 0x32 */ opUNDEF,
	/* 0x33 */ opLDW,  	// ld.w disp16[reg1],reg2   6a
	/* 0x34 */ opSTB,  	// st.b reg2, disp16[reg1]  6b
	/* 0x35 */ opSTH,  	// st.h reg2, disp16[reg1]  6b
	/* 0x36 */ opUNDEF,
	/* 0x37 */ opSTW,  	// st.w reg2, disp16[reg1]  6b
	/* 0x38 */ opINB,  	// in.b disp16[reg1], reg2  6a
	/* 0x39 */ opINH,  	// in.h disp16[reg1], reg2  6a
	/* 0x3a */ opCAXI,  	// caxi disp16[reg1],reg2   6a
	/* 0x3b */ opINW,  	// in.w disp16[reg1], reg2  6a
	/* 0x3c */ opOUTB,  	// out.b reg2, disp16[reg1]     6b
	/* 0x3d */ opOUTH,  	// out.h reg2, disp16[reg1]     6b
	/* 0x3e */ opFpoint, //floating point opcodes
	/* 0x3f */ opOUTW  	// out.w reg2, disp16[reg1]     6b
};

void v810_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	v810.irq_line = CLEAR_LINE;
	v810.nmi_line = CLEAR_LINE;
	v810.irq_cb = irqcallback;

	state_save_register_item_array("v810", index, v810.reg);
	state_save_register_item("v810", index, v810.irq_line);
	state_save_register_item("v810", index, v810.nmi_line);
	state_save_register_item("v810", index, v810.PPC);

}

void v810_reset(void)
{
	int i;
	for(i=0;i<64;i++)	v810.reg[i]=0;
	PC = 0xfffffff0;
	PSW = 0x8000;
	ECR	= 0x0000fff0;
}

int v810_execute(int cycles)
{
	v810_ICount = cycles;
	while(v810_ICount>=0)
	{
		v810.PPC=PC;
		CALL_MAME_DEBUG;
		OP=R_OP(PC);
		PC+=2;
		v810_ICount-= OpCodeTable[OP>>10]();
	}
	return cycles-v810_ICount;
}

void v810_get_context(void *dst)
{
	if(dst)
		*(v810info *)dst = v810;
}

void v810_set_context(void *src)
{
	if(src)
		v810 = *(v810info *)src;
}

static void set_irq_line(int irqline, int state)
{
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void v810_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(1, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				set_irq_line(2, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				set_irq_line(3, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				set_irq_line(4, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				set_irq_line(5, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				set_irq_line(6, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				set_irq_line(7, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 8:				set_irq_line(8, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 9:				set_irq_line(9, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 10:				set_irq_line(10, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 11:				set_irq_line(11, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 12:				set_irq_line(12, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 13:				set_irq_line(13, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 14:				set_irq_line(14, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 15:				set_irq_line(15, info->i);				break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PREVIOUSPC:					v810.PPC = info->i;						break;

		case CPUINFO_INT_REGISTER + V810_PC:
		case CPUINFO_INT_PC:							PC = info->i; 							break;

		case CPUINFO_INT_REGISTER + V810_R0:			R0 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R1:			R1 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R2:			R2 = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + V810_SP:			SP = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R4:			R4 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R5:			R5 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R6:			R6 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R7:			R7 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R8:			R8 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R9:			R9 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R10:			R10 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R11:			R11 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R12:			R12 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R13:			R13 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R14:			R14 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R15:			R15 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R16:			R16 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R17:			R17 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R18:			R18 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R19:			R19 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R20:			R20 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R21:			R21 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R22:			R22 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R23:			R23 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R24:			R24 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R25:			R25 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R26:			R26 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R27:			R27 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R28:			R28 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R29:			R29 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R30:			R30 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R31:			R31 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_PSW:			PSW = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_EIPC:			EIPC = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_EIPSW:			EIPSW = info->i;						break;
		case CPUINFO_INT_REGISTER + V810_FEPC:			FEPC = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_FEPSW:			FEPSW = info->i;						break;
		case CPUINFO_INT_REGISTER + V810_ECR:			ECR = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_PIR:			PIR = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_TKCW:			TKCW = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_CHCW:			CHCW = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_ADTRE:			ADTRE = info->i;						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void v810_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				GET_AE ? 'A':'.',
				GET_NP ? 'N':'.',
				GET_EP ? 'E':'.',
				GET_ID ? 'I':'.',
				GET_CY ? 'C':'.',
				GET_OV ? 'V':'.',
				GET_S ?  'S':'.',
				GET_Z ?  'Z':'.');
			break;

		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(v810);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 9;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = v810.irq_line;				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = v810.nmi_line;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = v810.PPC;						break;

		case CPUINFO_INT_REGISTER + V810_PC:
		case CPUINFO_INT_PC:							info->i = PC;  							break;

		case CPUINFO_INT_REGISTER + V810_R0:			info->i = R0;							break;
		case CPUINFO_INT_REGISTER + V810_R1:			info->i = R1;							break;
		case CPUINFO_INT_REGISTER + V810_R2:			info->i = R2;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + V810_SP:			info->i = SP;							break;
		case CPUINFO_INT_REGISTER + V810_R4:			info->i = R4;							break;
		case CPUINFO_INT_REGISTER + V810_R5:			info->i = R5;							break;
		case CPUINFO_INT_REGISTER + V810_R6:			info->i = R6;							break;
		case CPUINFO_INT_REGISTER + V810_R7:			info->i = R7;							break;
		case CPUINFO_INT_REGISTER + V810_R8:			info->i = R8;							break;
		case CPUINFO_INT_REGISTER + V810_R9:			info->i = R9;							break;
		case CPUINFO_INT_REGISTER + V810_R10:			info->i = R10;							break;
		case CPUINFO_INT_REGISTER + V810_R11:			info->i = R11;							break;
		case CPUINFO_INT_REGISTER + V810_R12:			info->i = R12;							break;
		case CPUINFO_INT_REGISTER + V810_R13:			info->i = R13;							break;
		case CPUINFO_INT_REGISTER + V810_R14:			info->i = R14;							break;
		case CPUINFO_INT_REGISTER + V810_R15:			info->i = R15;							break;
		case CPUINFO_INT_REGISTER + V810_R16:			info->i = R16;							break;
		case CPUINFO_INT_REGISTER + V810_R17:			info->i = R17;							break;
		case CPUINFO_INT_REGISTER + V810_R18:			info->i = R18;							break;
		case CPUINFO_INT_REGISTER + V810_R19:			info->i = R19;							break;
		case CPUINFO_INT_REGISTER + V810_R20:			info->i = R20;							break;
		case CPUINFO_INT_REGISTER + V810_R21:			info->i = R21;							break;
		case CPUINFO_INT_REGISTER + V810_R22:			info->i = R22;							break;
		case CPUINFO_INT_REGISTER + V810_R23:			info->i = R23;							break;
		case CPUINFO_INT_REGISTER + V810_R24:			info->i = R24;							break;
		case CPUINFO_INT_REGISTER + V810_R25:			info->i = R25;							break;
		case CPUINFO_INT_REGISTER + V810_R26:			info->i = R26;							break;
		case CPUINFO_INT_REGISTER + V810_R27:			info->i = R27;							break;
		case CPUINFO_INT_REGISTER + V810_R28:			info->i = R28;							break;
		case CPUINFO_INT_REGISTER + V810_R29:			info->i = R29;							break;
		case CPUINFO_INT_REGISTER + V810_R30:			info->i = R30;							break;
		case CPUINFO_INT_REGISTER + V810_R31:			info->i = R31;							break;
		case CPUINFO_INT_REGISTER + V810_PSW:			info->i = PSW;							break;
		case CPUINFO_INT_REGISTER + V810_EIPC:			info->i = EIPC;							break;
		case CPUINFO_INT_REGISTER + V810_EIPSW:			info->i = EIPSW;						break;
		case CPUINFO_INT_REGISTER + V810_FEPC:			info->i = FEPC;							break;
		case CPUINFO_INT_REGISTER + V810_FEPSW:			info->i = FEPSW;						break;
		case CPUINFO_INT_REGISTER + V810_ECR:			info->i = ECR;							break;
		case CPUINFO_INT_REGISTER + V810_PIR:			info->i = PIR;							break;
		case CPUINFO_INT_REGISTER + V810_TKCW:			info->i = TKCW;							break;
		case CPUINFO_INT_REGISTER + V810_CHCW:			info->i = CHCW;							break;
		case CPUINFO_INT_REGISTER + V810_ADTRE:			info->i = ADTRE;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = v810_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = v810_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = v810_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = v810_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = v810_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = NULL;						break;
		case CPUINFO_PTR_EXECUTE:						info->execute = v810_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = v810_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &v810_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "V810");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "NEC V810");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Tomasz Slanina");		break;

		case CPUINFO_STR_REGISTER + V810_PC:			sprintf(info->s, "PC:%08X", PC);		break;
		case CPUINFO_STR_REGISTER + V810_R0:			sprintf(info->s, "R0 :%08X", R0);		break;
		case CPUINFO_STR_REGISTER + V810_R1:			sprintf(info->s, "R1 :%08X", R1);		break;
		case CPUINFO_STR_REGISTER + V810_R2:			sprintf(info->s, "R2 :%08X", R2);		break;
		case CPUINFO_STR_REGISTER + V810_SP:			sprintf(info->s, "SP :%08X", SP);		break;
		case CPUINFO_STR_REGISTER + V810_R4:			sprintf(info->s, "R4 :%08X", R4);		break;
		case CPUINFO_STR_REGISTER + V810_R5:			sprintf(info->s, "R5 :%08X", R5);		break;
		case CPUINFO_STR_REGISTER + V810_R6:			sprintf(info->s, "R6 :%08X", R6);		break;
		case CPUINFO_STR_REGISTER + V810_R7:			sprintf(info->s, "R7 :%08X", R7);		break;
		case CPUINFO_STR_REGISTER + V810_R8:			sprintf(info->s, "R8 :%08X", R8);		break;
		case CPUINFO_STR_REGISTER + V810_R9:			sprintf(info->s, "R9 :%08X", R9);		break;
		case CPUINFO_STR_REGISTER + V810_R10:			sprintf(info->s, "R10:%08X", R10);		break;
		case CPUINFO_STR_REGISTER + V810_R11:			sprintf(info->s, "R11:%08X", R11);		break;
		case CPUINFO_STR_REGISTER + V810_R12:			sprintf(info->s, "R12:%08X", R12);		break;
		case CPUINFO_STR_REGISTER + V810_R13:			sprintf(info->s, "R13:%08X", R13);		break;
		case CPUINFO_STR_REGISTER + V810_R14:			sprintf(info->s, "R14:%08X", R14);		break;
		case CPUINFO_STR_REGISTER + V810_R15:			sprintf(info->s, "R15:%08X", R15);		break;
		case CPUINFO_STR_REGISTER + V810_R16:			sprintf(info->s, "R16:%08X", R16);		break;
		case CPUINFO_STR_REGISTER + V810_R17:			sprintf(info->s, "R17:%08X", R17);		break;
		case CPUINFO_STR_REGISTER + V810_R18:			sprintf(info->s, "R18:%08X", R18);		break;
		case CPUINFO_STR_REGISTER + V810_R19:			sprintf(info->s, "R19:%08X", R19);		break;
		case CPUINFO_STR_REGISTER + V810_R20:			sprintf(info->s, "R20:%08X", R20);		break;
		case CPUINFO_STR_REGISTER + V810_R21:			sprintf(info->s, "R21:%08X", R21);		break;
		case CPUINFO_STR_REGISTER + V810_R22:			sprintf(info->s, "R22:%08X", R22);		break;
		case CPUINFO_STR_REGISTER + V810_R23:			sprintf(info->s, "R23:%08X", R23);		break;
		case CPUINFO_STR_REGISTER + V810_R24:			sprintf(info->s, "R24:%08X", R24);		break;
		case CPUINFO_STR_REGISTER + V810_R25:			sprintf(info->s, "R25:%08X", R25);		break;
		case CPUINFO_STR_REGISTER + V810_R26:			sprintf(info->s, "R26:%08X", R26);		break;
		case CPUINFO_STR_REGISTER + V810_R27:			sprintf(info->s, "R27:%08X", R27);		break;
		case CPUINFO_STR_REGISTER + V810_R28:			sprintf(info->s, "R28:%08X", R28);		break;
		case CPUINFO_STR_REGISTER + V810_R29:			sprintf(info->s, "R29:%08X", R29);		break;
		case CPUINFO_STR_REGISTER + V810_R30:			sprintf(info->s, "R30:%08X", R30);		break;
		case CPUINFO_STR_REGISTER + V810_R31:			sprintf(info->s, "R31:%08X", R31);		break;
		case CPUINFO_STR_REGISTER + V810_EIPC:			sprintf(info->s, "EIPC :%08X", EIPC);	break;
		case CPUINFO_STR_REGISTER + V810_PSW:			sprintf(info->s, "PSW  :%08X", PSW);	break;
		case CPUINFO_STR_REGISTER + V810_EIPSW:			sprintf(info->s, "EIPSW:%08X", EIPSW);	break;
		case CPUINFO_STR_REGISTER + V810_FEPC:			sprintf(info->s, "FEPC :%08X", FEPC);	break;
		case CPUINFO_STR_REGISTER + V810_FEPSW:			sprintf(info->s, "FEPSW:%08X", FEPSW);	break;
		case CPUINFO_STR_REGISTER + V810_ECR:			sprintf(info->s, "ECR  :%08X", ECR);	break;
		case CPUINFO_STR_REGISTER + V810_PIR:			sprintf(info->s, "PIR  :%08X", PIR);	break;
		case CPUINFO_STR_REGISTER + V810_TKCW:			sprintf(info->s, "TKCW :%08X", TKCW);	break;
		case CPUINFO_STR_REGISTER + V810_CHCW:			sprintf(info->s, "CHCW :%08X", CHCW);	break;
		case CPUINFO_STR_REGISTER + V810_ADTRE:			sprintf(info->s, "ADTRE:%08X", ADTRE);	break;
	}
}
