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

#include "emu.h"
#include "debugger.h"
#include "v810.h"

#define clkIF 3
#define clkMEM 3

typedef struct _v810_state v810_state;
struct _v810_state
{
	UINT32 reg[65];
	UINT8 irq_line;
	UINT8 irq_state;
	UINT8 nmi_line;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	UINT32 PPC;
	int icount;
};

INLINE v810_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == V810);
	return (v810_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define R0 reg[0]
#define R1 reg[1]
#define R2 reg[2]
#define SP reg[3]
#define R4 reg[4]
#define R5 reg[5]
#define R6 reg[6]
#define R7 reg[7]
#define R8 reg[8]
#define R9 reg[9]
#define R10 reg[10]
#define R11 reg[11]
#define R12 reg[12]
#define R13 reg[13]
#define R14 reg[14]
#define R15 reg[15]
#define R16 reg[16]
#define R17 reg[17]
#define R18 reg[18]
#define R19 reg[19]
#define R20 reg[20]
#define R21 reg[21]
#define R22 reg[22]
#define R23 reg[23]
#define R24 reg[24]
#define R25 reg[25]
#define R26 reg[26]
#define R27 reg[27]
#define R28 reg[28]
#define R29 reg[29]
#define R30 reg[30]
#define R31 reg[31]

#define EIPC	reg[32]
#define EIPSW	reg[33]
#define FEPC	reg[34]
#define FEPSW	reg[35]
#define ECR		reg[36]
#define PSW		reg[37]
#define PIR		reg[38]
#define TKCW	reg[39]
#define CHCW	reg[56]
#define ADTRE	reg[57]

#define PC		reg[64]

/* Flags */
#define GET_Z					( cpustate->PSW & 0x00000001)
#define GET_S					((cpustate->PSW & 0x00000002)>>1)
#define GET_OV					((cpustate->PSW & 0x00000004)>>2)
#define GET_CY					((cpustate->PSW & 0x00000008)>>3)
#define GET_ID					((cpustate->PSW & 0x00001000)>>12)
#define GET_AE					((cpustate->PSW & 0x00002000)>>13)
#define GET_EP					((cpustate->PSW & 0x00004000)>>14)
#define GET_NP					((cpustate->PSW & 0x00008000)>>15)

#define SET_Z(val)				(cpustate->PSW = (cpustate->PSW & ~0x00000001) | (val))
#define SET_S(val)				(cpustate->PSW = (cpustate->PSW & ~0x00000002) | ((val) << 1))
#define SET_OV(val)				(cpustate->PSW = (cpustate->PSW & ~0x00000004) | ((val) << 2))
#define SET_CY(val)				(cpustate->PSW = (cpustate->PSW & ~0x00000008) | ((val) << 3))
#define SET_ID(val)				(cpustate->PSW = (cpustate->PSW & ~0x00001000) | ((val) << 12))
#define SET_AE(val)				(cpustate->PSW = (cpustate->PSW & ~0x00002000) | ((val) << 13))
#define SET_EP(val)				(cpustate->PSW = (cpustate->PSW & ~0x00004000) | ((val) << 14))
#define SET_NP(val)				(cpustate->PSW = (cpustate->PSW & ~0x00008000) | ((val) << 15))

#define R_B(cs, addr) ((cs)->program->read_byte(addr))
#define R_H(cs, addr) ((cs)->program->read_word(addr))
#define R_W(cs, addr) ((cs)->program->read_dword(addr))


#define W_B(cs, addr, val) ((cs)->program->write_byte(addr,val))
#define W_H(cs, addr, val) ((cs)->program->write_word(addr,val))
#define W_W(cs, addr, val) ((cs)->program->write_dword(addr,val))


#define RIO_B(cs, addr) ((cs)->io->read_byte(addr))
#define RIO_H(cs, addr) ((cs)->io->read_word(addr))
#define RIO_W(cs, addr) ((cs)->io->read_dword(addr))


#define WIO_B(cs, addr, val) ((cs)->io->write_byte(addr,val))
#define WIO_H(cs, addr, val) ((cs)->io->write_word(addr,val))
#define WIO_W(cs, addr, val) ((cs)->io->write_dword(addr,val))

#define R_OP(cs, addr)	((cs)->direct->read_decrypted_word(addr))

#define GET1 (op&0x1f)
#define GET2 ((op>>5)&0x1f)
#define I5(x) (((x)&0x1f)|(((x)&0x10)?0xffffffe0:0))
#define UI5(x) ((x)&0x1f)
#define I16(x) (((x)&0xffff)|(((x)&0x8000)?0xffff0000:0))
#define UI16(x) ((x)&0xffff)
#define D16(x) (((x)&0xffff)|(((x)&0x8000)?0xffff0000:0))
#define D26(x,y) ((y)|((x&0x3ff)<<16 )|((x&0x200)?0xfc000000:0))
#define D9(x) ((x&0x1ff)|((x&0x100)?0xfffffe00:0))
#define SO(opcode) (((opcode)&0xfc00)>>10)

#define CHECK_CY(x)	cpustate->PSW=(cpustate->PSW & ~8)|(((x) & (((UINT64)1) << 32)) ? 8 : 0)
#define CHECK_OVADD(x,y,z)	cpustate->PSW=(cpustate->PSW & ~0x00000004) |(( ((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) ? 4: 0)
#define CHECK_OVSUB(x,y,z)	cpustate->PSW=(cpustate->PSW & ~0x00000004) |(( ((y) ^ (z)) & ((x) ^ (y)) & 0x80000000) ? 4: 0)
#define CHECK_ZS(x)	cpustate->PSW=(cpustate->PSW & ~3)|((UINT32)(x)==0)|(((x)&0x80000000) ? 2: 0)


#define ADD(dst, src)		{ UINT64 res=(UINT64)(dst)+(UINT64)(src); SetCF(res); SetOF_Add(res,src,dst); SetSZPF(res); dst=(UINT32)res; }
#define SUB(dst, src)		{ UINT64 res=(UINT64)(dst)-(INT64)(src); SetCF(res); SetOF_Sub(res,src,dst); SetSZPF(res); dst=(UINT32)res; }




static void SETREG(v810_state *cpustate,UINT32 reg,UINT32 val)
{
	if(reg)
		cpustate->reg[reg]=val;
}

static UINT32 GETREG(v810_state *cpustate,UINT32 reg)
{
	if(reg)
		return cpustate->reg[reg];
	else
		return 0;
}

static UINT32 opUNDEF(v810_state *cpustate,UINT32 op)
{
	logerror("V810: Unknown opcode %x @ %x",op,cpustate->PC-2);
	return clkIF;
}

static UINT32 opMOVr(v810_state *cpustate,UINT32 op) // mov reg1, reg2
{
	SETREG(cpustate,GET2,GETREG(cpustate,GET1));
	return clkIF;
}

static UINT32 opMOVEA(v810_state *cpustate,UINT32 op)	// movea imm16, reg1, reg2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	op2=I16(op2);
	SETREG(cpustate,GET2,op1+op2);
	return clkIF;
}

static UINT32 opMOVHI(v810_state *cpustate,UINT32 op)	// movhi imm16, reg1 ,reg2
{
	UINT32 op2=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	op2=UI16(op2);
	op2<<=16;
	SETREG(cpustate,GET2,GETREG(cpustate,GET1)+op2);
	return clkIF;
}

static UINT32 opMOVi(v810_state *cpustate,UINT32 op)	// mov imm5,r2
{
	SETREG(cpustate,GET2,I5(op));
	return clkIF;
}

static UINT32 opADDr(v810_state *cpustate,UINT32 op)	// add r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(cpustate,GET2,res);
	return clkIF;
}

static UINT32 opADDi(v810_state *cpustate,UINT32 op)	// add imm5,r2
{
	UINT32 op1=I5(op);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(cpustate,GET2,res);
	return clkIF;
}


static UINT32 opADDI(v810_state *cpustate,UINT32 op)	// addi imm16, reg1, reg2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=R_OP(cpustate,cpustate->PC);
	UINT64 res;
	cpustate->PC+=2;
	op2=I16(op2);
	res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(cpustate,GET2,res);
	return clkIF;
}

static UINT32 opSUBr(v810_state *cpustate,UINT32 op)	// sub r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	SETREG(cpustate,GET2,res);
	return clkIF;
}


static UINT32 opCMPr(v810_state *cpustate,UINT32 op)	// cmp r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return clkIF;
}

static UINT32 opCMPi(v810_state *cpustate,UINT32 op)	// cmpi imm5,r2
{
	UINT32 op1=I5(op);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return clkIF;
}

static UINT32 opSETFi(v810_state *cpustate,UINT32 op)	// setf imm5,r2
{
	UINT32 op1=I5(op);
	UINT32 op2=cpustate->PSW&0xf;
	op1&=0xf;
	SETREG(cpustate,GET2,(op1==op2)?1:0);
	return clkIF;
}


static UINT32 opANDr(v810_state *cpustate,UINT32 op)	// and r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}

static UINT32 opANDI(v810_state *cpustate,UINT32 op)	// andi imm16,r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	op2=UI16(op2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}

static UINT32 opORr(v810_state *cpustate,UINT32 op)	// or r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}

static UINT32 opORI(v810_state *cpustate,UINT32 op)	// ori imm16,r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	op2=UI16(op2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}

static UINT32 opXORr(v810_state *cpustate,UINT32 op)	// xor r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}


static UINT32 opLDSR(v810_state *cpustate,UINT32 op) // ldsr reg2,regID
{
	UINT32 op1=UI5(op);
	SETREG(cpustate,32+op1,GETREG(cpustate,GET2));
	return clkIF;
}

static UINT32 opSTSR(v810_state *cpustate,UINT32 op) // ldsr regID,reg2
{
	UINT32 op1=UI5(op);
	SETREG(cpustate,GET2,GETREG(cpustate,32+op1));
	return clkIF;
}


static UINT32 opXORI(v810_state *cpustate,UINT32 op)	// xori imm16,r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	op2=UI16(op2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}

static UINT32 opNOTr(v810_state *cpustate,UINT32 op)	// not r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=~op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(cpustate,GET2,op2);
	return clkIF;
}

static UINT32 opSHLr(v810_state *cpustate,UINT32 op)	// shl r1,r2
{
	UINT64 tmp;
	UINT32 count=GETREG(cpustate,GET1);
	count&=0x1f;

	SET_OV(0);
	SET_CY(0);

	if(count)
	{
		tmp=GETREG(cpustate,GET2);
		tmp<<=count;
		CHECK_CY(tmp);
		SETREG(cpustate,GET2,tmp&0xffffffff);
		CHECK_ZS(GETREG(cpustate,GET2));
	}
	return clkIF;
}

static UINT32 opSHLi(v810_state *cpustate,UINT32 op)	// shl imm5,r2
{
	UINT64 tmp;
	UINT32 count=UI5(op);

	SET_OV(0);
	SET_CY(0);

	if(count)
	{
		tmp=GETREG(cpustate,GET2);
		tmp<<=count;
		CHECK_CY(tmp);
		SETREG(cpustate,GET2,tmp&0xffffffff);
	}
	CHECK_ZS(GETREG(cpustate,GET2));
	return clkIF;
}

static UINT32 opSHRr(v810_state *cpustate,UINT32 op)	// shr r1,r2
{
	UINT64 tmp;
	UINT32 count=GETREG(cpustate,GET1);
	count&=0x1f;
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(cpustate,GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		SETREG(cpustate,GET2,(tmp>>1)&0xffffffff);
	}
	CHECK_ZS(GETREG(cpustate,GET2));
	return clkIF;
}

static UINT32 opSHRi(v810_state *cpustate,UINT32 op)	// shr imm5,r2
{
	UINT64 tmp;
	UINT32 count=UI5(op);
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(cpustate,GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		tmp>>=1;
		SETREG(cpustate,GET2,tmp&0xffffffff);
	}
	CHECK_ZS(GETREG(cpustate,GET2));
	return clkIF;
}

static UINT32 opSARr(v810_state *cpustate,UINT32 op)	// sar r1,r2
{
	INT32 tmp;
	UINT32 count=GETREG(cpustate,GET1);
	count&=0x1f;
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(cpustate,GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		tmp>>=1;
		SETREG(cpustate,GET2,tmp);
	}
	CHECK_ZS(GETREG(cpustate,GET2));
	return clkIF;
}

static UINT32 opSARi(v810_state *cpustate,UINT32 op)	// sar imm5,r2
{
	INT32 tmp;
	UINT32 count=UI5(op);
	SET_OV(0);
	SET_CY(0);
	if(count)
	{
		tmp=GETREG(cpustate,GET2);
		tmp>>=count-1;
		SET_CY(tmp&1);
		tmp>>=1;
		SETREG(cpustate,GET2,tmp);
	}
	CHECK_ZS(GETREG(cpustate,GET2));
	return clkIF;
}

static UINT32 opJMPr(v810_state *cpustate,UINT32 op)
{
	cpustate->PC=GETREG(cpustate,GET1)&~1;
	return clkIF+2;
}


static UINT32 opJR(v810_state *cpustate,UINT32 op)
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC=cpustate->PC-2+(D26(op,tmp)&~1);
	return clkIF+2;
}

static UINT32 opJAL(v810_state *cpustate,UINT32 op)
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	cpustate->R31=cpustate->PC;
	cpustate->PC+=D26(op,tmp);
	cpustate->PC-=4;
	cpustate->PC&=~1;
	return clkIF+2;
}


static UINT32 opEI(v810_state *cpustate,UINT32 op)
{
	SET_ID(0);
	return clkIF;
}

static UINT32 opDI(v810_state *cpustate,UINT32 op)
{
	SET_ID(1);
	return clkIF;
}

static UINT32 opTRAP(v810_state *cpustate,UINT32 op)
{
	logerror("V810: TRAP @ %X\n",cpustate->PC-2);
	return clkIF;
}

static UINT32 opRETI(v810_state *cpustate,UINT32 op)
{
	if(GET_NP) {
		cpustate->PC = cpustate->FEPC;
		cpustate->PSW = cpustate->FEPSW;
	} else {
		cpustate->PC = cpustate->EIPC;
		cpustate->PSW = cpustate->EIPSW;
	}
	return clkIF;
}

static UINT32 opHALT(v810_state *cpustate,UINT32 op)
{
	logerror("V810: HALT @ %X",cpustate->PC-2);
	return clkIF;
}

static UINT32 opB(v810_state *cpustate,UINT32 op)
{
	int doBranch=0;
	switch((op>>9)&0xf)
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
			cpustate->PC=cpustate->PC-2+(D9(op)&~1);
	}
	return clkIF;
}

static UINT32 opLDB(v810_state *cpustate,UINT32 op)	// ld.b disp16[reg1],reg2
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	tmp=R_B(cpustate,tmp);
	tmp|=(tmp&0x80)?0xffffff00:0;
	SETREG(cpustate,GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opLDH(v810_state *cpustate,UINT32 op)	// ld.h disp16[reg1],reg2
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	tmp=R_H(cpustate,tmp&~1);
	tmp|=(tmp&0x8000)?0xffff0000:0;
	SETREG(cpustate,GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opLDW(v810_state *cpustate,UINT32 op)	// ld.w disp16[reg1],reg2
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	tmp=R_W(cpustate,tmp&~3);
	SETREG(cpustate,GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opINB(v810_state *cpustate,UINT32 op)	// in.b disp16[reg1],reg2
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	tmp=RIO_B(cpustate,tmp);
	SETREG(cpustate,GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opCAXI(v810_state *cpustate,UINT32 op)	// caxi disp16[reg1],reg2
{
	cpustate->PC+=2;
	return clkIF;
}

static UINT32 opINH(v810_state *cpustate,UINT32 op)	// in.h disp16[reg1],reg2
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	tmp=RIO_H(cpustate,tmp&~1);
	SETREG(cpustate,GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opINW(v810_state *cpustate,UINT32 op)	// in.w disp16[reg1],reg2
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	tmp=RIO_W(cpustate,tmp&~3);
	SETREG(cpustate,GET2,tmp);
	return clkIF+clkMEM;
}

static UINT32 opSTB(v810_state *cpustate,UINT32 op)	// st.b reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	W_B(cpustate,tmp,GETREG(cpustate,GET2)&0xff);
	return clkIF+clkMEM;
}

static UINT32 opSTH(v810_state *cpustate,UINT32 op)	// st.h reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	W_H(cpustate,tmp&~1,GETREG(cpustate,GET2)&0xffff);
	return clkIF+clkMEM;
}

static UINT32 opSTW(v810_state *cpustate,UINT32 op)	// st.w reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	W_W(cpustate,tmp&~3,GETREG(cpustate,GET2));
	return clkIF+clkMEM;
}

static UINT32 opOUTB(v810_state *cpustate,UINT32 op)	// out.b reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	WIO_B(cpustate,tmp,GETREG(cpustate,GET2)&0xff);
	return clkIF+clkMEM;
}

static UINT32 opOUTH(v810_state *cpustate,UINT32 op)	// out.h reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	WIO_H(cpustate,tmp&~1,GETREG(cpustate,GET2)&0xffff);
	return clkIF+clkMEM;
}

static UINT32 opOUTW(v810_state *cpustate,UINT32 op)	// out.w reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(cpustate,GET1);
	WIO_W(cpustate,tmp&~3,GETREG(cpustate,GET2));
	return clkIF+clkMEM;
}

static UINT32 opMULr(v810_state *cpustate,UINT32 op)	// mul r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 tmp;
	tmp=(INT64)(INT32)op1*(INT64)(INT32)op2;
	op2=tmp&0xffffffff;
	tmp>>=32;
	CHECK_ZS(tmp);//z = bad!
	SET_Z( (tmp|op2)==0 );
	SET_OV((tmp!=0));
	SET_CY((tmp!=0));
	SETREG(cpustate,GET2,op2);
	SETREG(cpustate,30,tmp);
	return clkIF;
}

static UINT32 opMULUr(v810_state *cpustate,UINT32 op)	// mulu r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	UINT64 tmp;
	tmp=(UINT64)op1*(UINT64)op2;
	op2=tmp&0xffffffff;
	tmp>>=32;
	CHECK_ZS(tmp);//z = bad!
	SET_Z( (tmp|op2)==0 );
	SET_OV((tmp!=0));
	SET_CY((tmp!=0));
	SETREG(cpustate,GET2,op2);
	SETREG(cpustate,30,tmp);
	return clkIF;
}

static UINT32 opDIVr(v810_state *cpustate,UINT32 op)	// div r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	if(op1)
	{
		SETREG(cpustate,30,(INT32)((INT32)op2%(INT32)op1));
		SETREG(cpustate,GET2,(INT32)((INT32)op2/(INT32)op1));
		SET_OV((op1^op2^GETREG(cpustate,GET2)) == 0x80000000);
		CHECK_ZS(GETREG(cpustate,GET2));
	}
	return clkIF;
}

static UINT32 opDIVUr(v810_state *cpustate,UINT32 op)	// divu r1,r2
{
	UINT32 op1=GETREG(cpustate,GET1);
	UINT32 op2=GETREG(cpustate,GET2);
	if(op1)
	{
		SETREG(cpustate,30,(INT32)(op2%op1));
		SETREG(cpustate,GET2,(INT32)(op2/op1));
		SET_OV((op1^op2^GETREG(cpustate,GET2)) == 0x80000000);
		CHECK_ZS(GETREG(cpustate,GET2));
	}
	return clkIF;
}

static void opADDF(v810_state *cpustate,UINT32 op)
{
	//TODO: CY
	float val1=u2f(GETREG(cpustate,GET1));
	float val2=u2f(GETREG(cpustate,GET2));
	SET_OV(0);
	val2+=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(cpustate,GET2,f2u(val2));
}

static void opSUBF(v810_state *cpustate,UINT32 op)
{
	float val1=u2f(GETREG(cpustate,GET1));
	float val2=u2f(GETREG(cpustate,GET2));
	SET_OV(0);
	SET_CY((val2<val1)?1:0);
	val2-=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(cpustate,GET2,f2u(val2));
}

static void opMULF(v810_state *cpustate,UINT32 op)
{
	//TODO: CY
	float val1=u2f(GETREG(cpustate,GET1));
	float val2=u2f(GETREG(cpustate,GET2));
	SET_OV(0);
	val2*=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(cpustate,GET2,f2u(val2));
}

static void opDIVF(v810_state *cpustate,UINT32 op)
{
	//TODO: CY
	float val1=u2f(GETREG(cpustate,GET1));
	float val2=u2f(GETREG(cpustate,GET2));
	SET_OV(0);
	if(val1!=0)
		val2/=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
	SETREG(cpustate,GET2,f2u(val2));
}

static void opTRNC(v810_state *cpustate,UINT32 op)
{
	float val1=u2f(GETREG(cpustate,GET1));
	SET_OV(0);
	SET_Z((val1==0.0)?1:0);
	SET_S((val1<0.0)?1:0);
	SETREG(cpustate,GET2,(INT32)val1);
}

static void opCMPF(v810_state *cpustate,UINT32 op)
{
	float val1=u2f(GETREG(cpustate,GET1));
	float val2=u2f(GETREG(cpustate,GET2));
	SET_OV(0);
	SET_CY((val2<val1)?1:0);
	val2-=val1;
	SET_Z((val2==0.0)?1:0);
	SET_S((val2<0.0)?1:0);
}

static void opCVTS(v810_state *cpustate,UINT32 op)
{
	float val1=u2f(GETREG(cpustate,GET1));
	SET_OV(0);
	SET_Z((val1==0.0)?1:0);
	SET_S((val1<0.0)?1:0);
	SETREG(cpustate,GET2,(INT32)val1);
}

static void opCVTW(v810_state *cpustate,UINT32 op)
{
	//TODO: CY
	float val1=GETREG(cpustate,GET1);
	SET_OV(0);
	SET_Z((val1==0.0)?1:0);
	SET_S((val1<0.0)?1:0);
	SETREG(cpustate,GET2,f2u(val1));
}

static UINT32 opFpoint(v810_state *cpustate,UINT32 op)
{
	UINT32 tmp=R_OP(cpustate,cpustate->PC);
	cpustate->PC+=2;
	switch((tmp&0xfc00)>>10)
	{
			case 0x0: opCMPF(cpustate,op);break;
			case 0x2: opCVTW(cpustate,op);break;
			case 0x3: opCVTS(cpustate,op);break;
			case 0x4: opADDF(cpustate,op);break;
			case 0x5: opSUBF(cpustate,op);break;
			case 0x6: opMULF(cpustate,op);break;
			case 0x7: opDIVF(cpustate,op);break;
			case 0xb: opTRNC(cpustate,op);break;
	}
	return clkIF+1;
}

/* TODO: clocks */
static void opMOVBSU(v810_state *cpustate,UINT32 op)
{
	UINT32 srcoff,dstoff,src,dst,size;
	UINT32 tmp;

//	printf("BDST %08x BSRC %08x SIZE %08x DST %08x SRC %08x\n",cpustate->R26,cpustate->R27,cpustate->R28,cpustate->R29,cpustate->R30);

	dstoff = cpustate->R26;
	srcoff = cpustate->R27;
	size =  cpustate->R28;
	dst = cpustate->R29 & ~3;
	src = cpustate->R30 & ~3;

	tmp = R_W(cpustate,src);
	W_W(cpustate,dst,tmp);

	srcoff++;
	dstoff++;

	srcoff&=0x1f;
	dstoff&=0x1f;

	if(srcoff == 0)
		src+=4;

	if(dstoff == 0)
		dst+=4;

	size --;

	cpustate->R26 = dstoff;
	cpustate->R27 = srcoff;
	cpustate->R28 = size;
	cpustate->R29 = dst;
	cpustate->R30 = src;

	if(size != 0)
		cpustate->PC-=2;
}

static UINT32 opBSU(v810_state *cpustate,UINT32 op)
{
	if(!(op & 8))
		fatalerror("V810: unknown BSU opcode %04x",op);

	switch(op & 0xf)
	{
		case 0xb: opMOVBSU(cpustate,op); break;
		default: fatalerror("V810: unemulated BSU opcode %04x\n",op);
	}

	return clkIF+1; //TODO: correct?
}

static UINT32 (*const OpCodeTable[64])(v810_state *cpustate,UINT32 op) =
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
	/* 0x18 */ opTRAP,
	/* 0x19 */ opRETI,
	/* 0x1a */ opHALT,  	// halt             2
	/* 0x1b */ opUNDEF,
	/* 0x1c */ opLDSR,  	// ldsr reg2,regID          2
	/* 0x1d */ opSTSR,  	// stsr regID,reg2          2
	/* 0x1e */ opDI,	// DI               2
	/* 0x1f */ opBSU,
	/* 0x20 */ opB, 	// Branch (7 bit opcode)
	/* 0x21 */ opB, 	// Branch (7 bit opcode)
	/* 0x22 */ opB, 	// Branch (7 bit opcode)
	/* 0x23 */ opB, 	// Branch (7 bit opcode)
	/* 0x24 */ opB, 	// Branch (7 bit opcode)
	/* 0x25 */ opB, 	// Branch (7 bit opcode)
	/* 0x26 */ opB, 	// Branch (7 bit opcode)
	/* 0x27 */ opB, 	// Branch (7 bit opcode)
	/* 0x28 */ opMOVEA, 	// movea imm16, reg1, reg2  5
	/* 0x29 */ opADDI,  	// addi imm16, reg1, reg2   5
	/* 0x2a */ opJR,	// jr disp26            4
	/* 0x2b */ opJAL,	// jal disp26           4
	/* 0x2c */ opORI,	// ori imm16, reg1, reg2    5
	/* 0x2d */ opANDI,  	// andi imm16, reg1, reg2   5
	/* 0x2e */ opXORI,  	// xori imm16, reg1, reg2   5
	/* 0x2f */ opMOVHI, 	// movhi imm16, reg1 ,reg2  5
	/* 0x30 */ opLDB,	// ld.b disp16[reg1],reg2   6a
	/* 0x31 */ opLDH,	// ld.h disp16[reg1],reg2   6a
	/* 0x32 */ opUNDEF,
	/* 0x33 */ opLDW,	// ld.w disp16[reg1],reg2   6a
	/* 0x34 */ opSTB,	// st.b reg2, disp16[reg1]  6b
	/* 0x35 */ opSTH,	// st.h reg2, disp16[reg1]  6b
	/* 0x36 */ opUNDEF,
	/* 0x37 */ opSTW,	// st.w reg2, disp16[reg1]  6b
	/* 0x38 */ opINB,	// in.b disp16[reg1], reg2  6a
	/* 0x39 */ opINH,	// in.h disp16[reg1], reg2  6a
	/* 0x3a */ opCAXI,  	// caxi disp16[reg1],reg2   6a
	/* 0x3b */ opINW,	// in.w disp16[reg1], reg2  6a
	/* 0x3c */ opOUTB,  	// out.b reg2, disp16[reg1]     6b
	/* 0x3d */ opOUTH,  	// out.h reg2, disp16[reg1]     6b
	/* 0x3e */ opFpoint, //floating point opcodes
	/* 0x3f */ opOUTW	// out.w reg2, disp16[reg1]     6b
};

static CPU_INIT( v810 )
{
	v810_state *cpustate = get_safe_token(device);

	cpustate->irq_state = CLEAR_LINE;
	cpustate->irq_line = 0;
	cpustate->nmi_line = CLEAR_LINE;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	device->save_item(NAME(cpustate->reg));
	device->save_item(NAME(cpustate->irq_line));
	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->nmi_line));
	device->save_item(NAME(cpustate->PPC));

}

static CPU_RESET( v810 )
{
	v810_state *cpustate = get_safe_token(device);
	int i;
	for(i=0;i<64;i++)	cpustate->reg[i]=0;
	cpustate->PC = 0xfffffff0;
	cpustate->PSW = 0x1000;
	cpustate->ECR	= 0x0000fff0;
}

static void take_interrupt(v810_state *cpustate)
{
	cpustate->EIPC = cpustate->PC;
	cpustate->EIPSW = cpustate->PSW;

	cpustate->PC = 0xfffffe00 | (cpustate->irq_line << 4);
	cpustate->ECR = 0xfe00 | (cpustate->irq_line << 4);

	UINT8 num = cpustate->irq_line + 1;
	if (num==0x10) num=0x0f;

	cpustate->PSW &= 0xfff0ffff; // clear interrupt level
	SET_EP(1);
    SET_ID(1);
	cpustate->PSW |= num << 16;

	cpustate->icount-= clkIF;
}

static CPU_EXECUTE( v810 )
{
	v810_state *cpustate = get_safe_token(device);

	if (cpustate->irq_state != CLEAR_LINE) {
		if (!(GET_NP | GET_EP | GET_ID)) {
			if (cpustate->irq_line >=((cpustate->PSW & 0xF0000) >> 16)) {
				take_interrupt(cpustate);
			}
		}
	}
	while(cpustate->icount>0)
	{
		UINT32 op;

		cpustate->PPC=cpustate->PC;
		debugger_instruction_hook(device, cpustate->PC);
		op=R_OP(cpustate,cpustate->PC);
		cpustate->PC+=2;
		cpustate->icount-= OpCodeTable[op>>10](cpustate,op);
	}
}


static void set_irq_line(v810_state *cpustate, int irqline, int state)
{
	cpustate->irq_state = state;
	cpustate->irq_line = irqline;
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( v810 )
{
	v810_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(cpustate, 1, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				set_irq_line(cpustate, 2, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				set_irq_line(cpustate, 3, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				set_irq_line(cpustate, 4, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				set_irq_line(cpustate, 5, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				set_irq_line(cpustate, 6, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				set_irq_line(cpustate, 7, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 8:				set_irq_line(cpustate, 8, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 9:				set_irq_line(cpustate, 9, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 10:				set_irq_line(cpustate, 10, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 11:				set_irq_line(cpustate, 11, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 12:				set_irq_line(cpustate, 12, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 13:				set_irq_line(cpustate, 13, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 14:				set_irq_line(cpustate, 14, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 15:				set_irq_line(cpustate, 15, info->i);				break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_PREVIOUSPC:					cpustate->PPC = info->i;						break;

		case CPUINFO_INT_REGISTER + V810_PC:
		case CPUINFO_INT_PC:							cpustate->PC = info->i; 							break;

		case CPUINFO_INT_REGISTER + V810_R0:			cpustate->R0 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R1:			cpustate->R1 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R2:			cpustate->R2 = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + V810_SP:			cpustate->SP = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R4:			cpustate->R4 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R5:			cpustate->R5 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R6:			cpustate->R6 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R7:			cpustate->R7 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R8:			cpustate->R8 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R9:			cpustate->R9 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R10:			cpustate->R10 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R11:			cpustate->R11 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R12:			cpustate->R12 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R13:			cpustate->R13 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R14:			cpustate->R14 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R15:			cpustate->R15 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R16:			cpustate->R16 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R17:			cpustate->R17 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R18:			cpustate->R18 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R19:			cpustate->R19 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R20:			cpustate->R20 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R21:			cpustate->R21 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R22:			cpustate->R22 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R23:			cpustate->R23 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R24:			cpustate->R24 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R25:			cpustate->R25 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R26:			cpustate->R26 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R27:			cpustate->R27 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R28:			cpustate->R28 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R29:			cpustate->R29 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R30:			cpustate->R30 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_R31:			cpustate->R31 = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_PSW:			cpustate->PSW = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_EIPC:			cpustate->EIPC = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_EIPSW:			cpustate->EIPSW = info->i;						break;
		case CPUINFO_INT_REGISTER + V810_FEPC:			cpustate->FEPC = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_FEPSW:			cpustate->FEPSW = info->i;						break;
		case CPUINFO_INT_REGISTER + V810_ECR:			cpustate->ECR = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_PIR:			cpustate->PIR = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_TKCW:			cpustate->TKCW = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_CHCW:			cpustate->CHCW = info->i;							break;
		case CPUINFO_INT_REGISTER + V810_ADTRE:			cpustate->ADTRE = info->i;						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( v810 )
{
	v810_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

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

		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(v810_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 16;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 3;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 6;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = (cpustate->irq_line == 0) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = (cpustate->irq_line == 1) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = (cpustate->irq_line == 2) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = (cpustate->irq_line == 3) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = (cpustate->irq_line == 4) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = (cpustate->irq_line == 5) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = (cpustate->irq_line == 6) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = (cpustate->irq_line == 7) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 8:				info->i = (cpustate->irq_line == 8) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 9:				info->i = (cpustate->irq_line == 9) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 10:				info->i = (cpustate->irq_line == 10) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 11:				info->i = (cpustate->irq_line == 11) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 12:				info->i = (cpustate->irq_line == 12) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 13:				info->i = (cpustate->irq_line == 13) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 14:				info->i = (cpustate->irq_line == 14) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + 15:				info->i = (cpustate->irq_line == 15) ? cpustate->irq_state : CLEAR_LINE;			break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_line;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->PPC;				break;

		case CPUINFO_INT_REGISTER + V810_PC:
		case CPUINFO_INT_PC:							info->i = cpustate->PC; 				break;

		case CPUINFO_INT_REGISTER + V810_R0:			info->i = cpustate->R0;					break;
		case CPUINFO_INT_REGISTER + V810_R1:			info->i = cpustate->R1;					break;
		case CPUINFO_INT_REGISTER + V810_R2:			info->i = cpustate->R2;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + V810_SP:			info->i = cpustate->SP;					break;
		case CPUINFO_INT_REGISTER + V810_R4:			info->i = cpustate->R4;					break;
		case CPUINFO_INT_REGISTER + V810_R5:			info->i = cpustate->R5;					break;
		case CPUINFO_INT_REGISTER + V810_R6:			info->i = cpustate->R6;					break;
		case CPUINFO_INT_REGISTER + V810_R7:			info->i = cpustate->R7;					break;
		case CPUINFO_INT_REGISTER + V810_R8:			info->i = cpustate->R8;					break;
		case CPUINFO_INT_REGISTER + V810_R9:			info->i = cpustate->R9;					break;
		case CPUINFO_INT_REGISTER + V810_R10:			info->i = cpustate->R10;				break;
		case CPUINFO_INT_REGISTER + V810_R11:			info->i = cpustate->R11;				break;
		case CPUINFO_INT_REGISTER + V810_R12:			info->i = cpustate->R12;				break;
		case CPUINFO_INT_REGISTER + V810_R13:			info->i = cpustate->R13;				break;
		case CPUINFO_INT_REGISTER + V810_R14:			info->i = cpustate->R14;				break;
		case CPUINFO_INT_REGISTER + V810_R15:			info->i = cpustate->R15;				break;
		case CPUINFO_INT_REGISTER + V810_R16:			info->i = cpustate->R16;				break;
		case CPUINFO_INT_REGISTER + V810_R17:			info->i = cpustate->R17;				break;
		case CPUINFO_INT_REGISTER + V810_R18:			info->i = cpustate->R18;				break;
		case CPUINFO_INT_REGISTER + V810_R19:			info->i = cpustate->R19;				break;
		case CPUINFO_INT_REGISTER + V810_R20:			info->i = cpustate->R20;				break;
		case CPUINFO_INT_REGISTER + V810_R21:			info->i = cpustate->R21;				break;
		case CPUINFO_INT_REGISTER + V810_R22:			info->i = cpustate->R22;				break;
		case CPUINFO_INT_REGISTER + V810_R23:			info->i = cpustate->R23;				break;
		case CPUINFO_INT_REGISTER + V810_R24:			info->i = cpustate->R24;				break;
		case CPUINFO_INT_REGISTER + V810_R25:			info->i = cpustate->R25;				break;
		case CPUINFO_INT_REGISTER + V810_R26:			info->i = cpustate->R26;				break;
		case CPUINFO_INT_REGISTER + V810_R27:			info->i = cpustate->R27;				break;
		case CPUINFO_INT_REGISTER + V810_R28:			info->i = cpustate->R28;				break;
		case CPUINFO_INT_REGISTER + V810_R29:			info->i = cpustate->R29;				break;
		case CPUINFO_INT_REGISTER + V810_R30:			info->i = cpustate->R30;				break;
		case CPUINFO_INT_REGISTER + V810_R31:			info->i = cpustate->R31;				break;
		case CPUINFO_INT_REGISTER + V810_PSW:			info->i = cpustate->PSW;				break;
		case CPUINFO_INT_REGISTER + V810_EIPC:			info->i = cpustate->EIPC;				break;
		case CPUINFO_INT_REGISTER + V810_EIPSW:			info->i = cpustate->EIPSW;				break;
		case CPUINFO_INT_REGISTER + V810_FEPC:			info->i = cpustate->FEPC;				break;
		case CPUINFO_INT_REGISTER + V810_FEPSW:			info->i = cpustate->FEPSW;				break;
		case CPUINFO_INT_REGISTER + V810_ECR:			info->i = cpustate->ECR;				break;
		case CPUINFO_INT_REGISTER + V810_PIR:			info->i = cpustate->PIR;				break;
		case CPUINFO_INT_REGISTER + V810_TKCW:			info->i = cpustate->TKCW;				break;
		case CPUINFO_INT_REGISTER + V810_CHCW:			info->i = cpustate->CHCW;				break;
		case CPUINFO_INT_REGISTER + V810_ADTRE:			info->i = cpustate->ADTRE;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(v810);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v810);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(v810);				break;
		case CPUINFO_FCT_EXIT:							info->exit = NULL;								break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(v810);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(v810);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "V810");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "NEC V810");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Tomasz Slanina");		break;

		case CPUINFO_STR_REGISTER + V810_PC:			sprintf(info->s, "PC:%08X", cpustate->PC);		break;
		case CPUINFO_STR_REGISTER + V810_R0:			sprintf(info->s, "R0 :%08X", cpustate->R0);		break;
		case CPUINFO_STR_REGISTER + V810_R1:			sprintf(info->s, "R1 :%08X", cpustate->R1);		break;
		case CPUINFO_STR_REGISTER + V810_R2:			sprintf(info->s, "R2 :%08X", cpustate->R2);		break;
		case CPUINFO_STR_REGISTER + V810_SP:			sprintf(info->s, "SP :%08X", cpustate->SP);		break;
		case CPUINFO_STR_REGISTER + V810_R4:			sprintf(info->s, "R4 :%08X", cpustate->R4);		break;
		case CPUINFO_STR_REGISTER + V810_R5:			sprintf(info->s, "R5 :%08X", cpustate->R5);		break;
		case CPUINFO_STR_REGISTER + V810_R6:			sprintf(info->s, "R6 :%08X", cpustate->R6);		break;
		case CPUINFO_STR_REGISTER + V810_R7:			sprintf(info->s, "R7 :%08X", cpustate->R7);		break;
		case CPUINFO_STR_REGISTER + V810_R8:			sprintf(info->s, "R8 :%08X", cpustate->R8);		break;
		case CPUINFO_STR_REGISTER + V810_R9:			sprintf(info->s, "R9 :%08X", cpustate->R9);		break;
		case CPUINFO_STR_REGISTER + V810_R10:			sprintf(info->s, "R10:%08X", cpustate->R10);		break;
		case CPUINFO_STR_REGISTER + V810_R11:			sprintf(info->s, "R11:%08X", cpustate->R11);		break;
		case CPUINFO_STR_REGISTER + V810_R12:			sprintf(info->s, "R12:%08X", cpustate->R12);		break;
		case CPUINFO_STR_REGISTER + V810_R13:			sprintf(info->s, "R13:%08X", cpustate->R13);		break;
		case CPUINFO_STR_REGISTER + V810_R14:			sprintf(info->s, "R14:%08X", cpustate->R14);		break;
		case CPUINFO_STR_REGISTER + V810_R15:			sprintf(info->s, "R15:%08X", cpustate->R15);		break;
		case CPUINFO_STR_REGISTER + V810_R16:			sprintf(info->s, "R16:%08X", cpustate->R16);		break;
		case CPUINFO_STR_REGISTER + V810_R17:			sprintf(info->s, "R17:%08X", cpustate->R17);		break;
		case CPUINFO_STR_REGISTER + V810_R18:			sprintf(info->s, "R18:%08X", cpustate->R18);		break;
		case CPUINFO_STR_REGISTER + V810_R19:			sprintf(info->s, "R19:%08X", cpustate->R19);		break;
		case CPUINFO_STR_REGISTER + V810_R20:			sprintf(info->s, "R20:%08X", cpustate->R20);		break;
		case CPUINFO_STR_REGISTER + V810_R21:			sprintf(info->s, "R21:%08X", cpustate->R21);		break;
		case CPUINFO_STR_REGISTER + V810_R22:			sprintf(info->s, "R22:%08X", cpustate->R22);		break;
		case CPUINFO_STR_REGISTER + V810_R23:			sprintf(info->s, "R23:%08X", cpustate->R23);		break;
		case CPUINFO_STR_REGISTER + V810_R24:			sprintf(info->s, "R24:%08X", cpustate->R24);		break;
		case CPUINFO_STR_REGISTER + V810_R25:			sprintf(info->s, "R25:%08X", cpustate->R25);		break;
		case CPUINFO_STR_REGISTER + V810_R26:			sprintf(info->s, "R26:%08X", cpustate->R26);		break;
		case CPUINFO_STR_REGISTER + V810_R27:			sprintf(info->s, "R27:%08X", cpustate->R27);		break;
		case CPUINFO_STR_REGISTER + V810_R28:			sprintf(info->s, "R28:%08X", cpustate->R28);		break;
		case CPUINFO_STR_REGISTER + V810_R29:			sprintf(info->s, "R29:%08X", cpustate->R29);		break;
		case CPUINFO_STR_REGISTER + V810_R30:			sprintf(info->s, "R30:%08X", cpustate->R30);		break;
		case CPUINFO_STR_REGISTER + V810_R31:			sprintf(info->s, "R31:%08X", cpustate->R31);		break;
		case CPUINFO_STR_REGISTER + V810_EIPC:			sprintf(info->s, "EIPC :%08X", cpustate->EIPC);	break;
		case CPUINFO_STR_REGISTER + V810_PSW:			sprintf(info->s, "PSW  :%08X", cpustate->PSW);	break;
		case CPUINFO_STR_REGISTER + V810_EIPSW:			sprintf(info->s, "EIPSW:%08X", cpustate->EIPSW);	break;
		case CPUINFO_STR_REGISTER + V810_FEPC:			sprintf(info->s, "FEPC :%08X", cpustate->FEPC);	break;
		case CPUINFO_STR_REGISTER + V810_FEPSW:			sprintf(info->s, "FEPSW:%08X", cpustate->FEPSW);	break;
		case CPUINFO_STR_REGISTER + V810_ECR:			sprintf(info->s, "ECR  :%08X", cpustate->ECR);	break;
		case CPUINFO_STR_REGISTER + V810_PIR:			sprintf(info->s, "PIR  :%08X", cpustate->PIR);	break;
		case CPUINFO_STR_REGISTER + V810_TKCW:			sprintf(info->s, "TKCW :%08X", cpustate->TKCW);	break;
		case CPUINFO_STR_REGISTER + V810_CHCW:			sprintf(info->s, "CHCW :%08X", cpustate->CHCW);	break;
		case CPUINFO_STR_REGISTER + V810_ADTRE:			sprintf(info->s, "ADTRE:%08X", cpustate->ADTRE);	break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V810, v810);
