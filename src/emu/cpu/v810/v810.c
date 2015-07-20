// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Tomasz Slanina
/******************************************************************
 NEC V810 (upd70732) core
  Tomasz Slanina - analog[at]op.pl
  Angelo Salese - lordkale[at]libero.it

 Change Log
 - 23/08/2012 - Implemented remaining BSU opcodes (Angelo Salese)
 - 21/08/2012 - Fixed SET.F behaviour (Angelo Salese)
 - 20/08/2012 - Fixed a sign bug with CVT.WS opcode (Angelo Salese)
 - 16/08/2012 - Added XB, XH, MPYHW, MOVBSU, ORBSU and ANDNBSU opcodes
                (Angelo Salese)
 - 19/11/2010 - Fixed interrupt handing and flag position in PSW register
                (Miodrag Milanovic)
 - 18/11/2010 - Added bare bones irq support (Miodrag Milanovic)
 - 20/07/2004 - first public release


 TODO:
    - CY flag in few floating point opcodes
        (all floating point opcodes are NOT tested!)
  - traps/interrupts/exceptions
  - bitstring opcodes currently makes the emulation to drop to 0%
  - timing
  - missing opcodes : trap, caxi

******************************************************************/

#include "emu.h"
#include "debugger.h"
#include "v810.h"

#define clkIF 3
#define clkMEM 3


const device_type V810 = &device_creator<v810_device>;


v810_device::v810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, V810, "V810", tag, owner, clock, "v810", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 32, 32, 0)
{
}


offs_t v810_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( v810 );
	return CPU_DISASSEMBLE_NAME(v810)(this, buffer, pc, oprom, opram, options);
}


#define R0 m_reg[0]
#define R1 m_reg[1]
#define R2 m_reg[2]
#define SP m_reg[3]
#define R4 m_reg[4]
#define R5 m_reg[5]
#define R6 m_reg[6]
#define R7 m_reg[7]
#define R8 m_reg[8]
#define R9 m_reg[9]
#define R10 m_reg[10]
#define R11 m_reg[11]
#define R12 m_reg[12]
#define R13 m_reg[13]
#define R14 m_reg[14]
#define R15 m_reg[15]
#define R16 m_reg[16]
#define R17 m_reg[17]
#define R18 m_reg[18]
#define R19 m_reg[19]
#define R20 m_reg[20]
#define R21 m_reg[21]
#define R22 m_reg[22]
#define R23 m_reg[23]
#define R24 m_reg[24]
#define R25 m_reg[25]
#define R26 m_reg[26]
#define R27 m_reg[27]
#define R28 m_reg[28]
#define R29 m_reg[29]
#define R30 m_reg[30]
#define R31 m_reg[31]

#define EIPC    m_reg[32]
#define EIPSW   m_reg[33]
#define FEPC    m_reg[34]
#define FEPSW   m_reg[35]
#define ECR     m_reg[36]
#define PSW     m_reg[37]
#define PIR     m_reg[38]
#define TKCW    m_reg[39]
#define CHCW    m_reg[56]
#define ADTRE   m_reg[57]

#define PC      m_reg[64]

/* Flags */
#define GET_Z                   ( PSW & 0x00000001)
#define GET_S                   ((PSW & 0x00000002)>>1)
#define GET_OV                  ((PSW & 0x00000004)>>2)
#define GET_CY                  ((PSW & 0x00000008)>>3)
#define GET_ID                  ((PSW & 0x00001000)>>12)
#define GET_AE                  ((PSW & 0x00002000)>>13)
#define GET_EP                  ((PSW & 0x00004000)>>14)
#define GET_NP                  ((PSW & 0x00008000)>>15)

#define SET_Z(val)              (PSW = (PSW & ~0x00000001) | (val))
#define SET_S(val)              (PSW = (PSW & ~0x00000002) | ((val) << 1))
#define SET_OV(val)             (PSW = (PSW & ~0x00000004) | ((val) << 2))
#define SET_CY(val)             (PSW = (PSW & ~0x00000008) | ((val) << 3))
#define SET_ID(val)             (PSW = (PSW & ~0x00001000) | ((val) << 12))
#define SET_AE(val)             (PSW = (PSW & ~0x00002000) | ((val) << 13))
#define SET_EP(val)             (PSW = (PSW & ~0x00004000) | ((val) << 14))
#define SET_NP(val)             (PSW = (PSW & ~0x00008000) | ((val) << 15))

#define R_B(addr) (m_program->read_byte(addr))
#define R_H(addr) (m_program->read_word(addr))
#define R_W(addr) (m_program->read_dword(addr))


#define W_B(addr, val) (m_program->write_byte(addr,val))
#define W_H(addr, val) (m_program->write_word(addr,val))
#define W_W(addr, val) (m_program->write_dword(addr,val))


#define RIO_B(addr) (m_io->read_byte(addr))
#define RIO_H(addr) (m_io->read_word(addr))
#define RIO_W(addr) (m_io->read_dword(addr))


#define WIO_B(addr, val) (m_io->write_byte(addr,val))
#define WIO_H(addr, val) (m_io->write_word(addr,val))
#define WIO_W(addr, val) (m_io->write_dword(addr,val))

#define R_OP(addr)  (m_direct->read_word(addr))

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

#define CHECK_CY(x) PSW=(PSW & ~8)|(((x) & (((UINT64)1) << 32)) ? 8 : 0)
#define CHECK_OVADD(x,y,z)  PSW=(PSW & ~0x00000004) |(( ((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) ? 4: 0)
#define CHECK_OVSUB(x,y,z)  PSW=(PSW & ~0x00000004) |(( ((y) ^ (z)) & ((x) ^ (y)) & 0x80000000) ? 4: 0)
#define CHECK_ZS(x) PSW=(PSW & ~3)|((UINT32)(x)==0)|(((x)&0x80000000) ? 2: 0)


#define ADD(dst, src)       { UINT64 res=(UINT64)(dst)+(UINT64)(src); SetCF(res); SetOF_Add(res,src,dst); SetSZPF(res); dst=(UINT32)res; }
#define SUB(dst, src)       { UINT64 res=(UINT64)(dst)-(INT64)(src); SetCF(res); SetOF_Sub(res,src,dst); SetSZPF(res); dst=(UINT32)res; }




void v810_device::SETREG(UINT32 reg,UINT32 val)
{
	if(reg)
		m_reg[reg]=val;
}

UINT32 v810_device::GETREG(UINT32 reg)
{
	if(reg)
		return m_reg[reg];
	else
		return 0;
}

UINT32 v810_device::opUNDEF(UINT32 op)
{
	logerror("V810: Unknown opcode %x @ %x",op,PC-2);
	return clkIF;
}

UINT32 v810_device::opMOVr(UINT32 op) // mov reg1, reg2
{
	SETREG(GET2,GETREG(GET1));
	return clkIF;
}

UINT32 v810_device::opMOVEA(UINT32 op)   // movea imm16, reg1, reg2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=I16(op2);
	SETREG(GET2,op1+op2);
	return clkIF;
}

UINT32 v810_device::opMOVHI(UINT32 op)   // movhi imm16, reg1 ,reg2
{
	UINT32 op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2<<=16;
	SETREG(GET2,GETREG(GET1)+op2);
	return clkIF;
}

UINT32 v810_device::opMOVi(UINT32 op)    // mov imm5,r2
{
	SETREG(GET2,I5(op));
	return clkIF;
}

UINT32 v810_device::opADDr(UINT32 op)    // add r1,r2
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

UINT32 v810_device::opADDi(UINT32 op)    // add imm5,r2
{
	UINT32 op1=I5(op);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2+(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return clkIF;
}


UINT32 v810_device::opADDI(UINT32 op)    // addi imm16, reg1, reg2
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

UINT32 v810_device::opSUBr(UINT32 op)    // sub r1,r2
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


UINT32 v810_device::opCMPr(UINT32 op)    // cmp r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return clkIF;
}

UINT32 v810_device::opCMPi(UINT32 op)    // cmpi imm5,r2
{
	UINT32 op1=I5(op);
	UINT32 op2=GETREG(GET2);
	UINT64 res=(UINT64)op2-(UINT64)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return clkIF;
}

UINT32 v810_device::opSETFi(UINT32 op)   // setf imm5,r2
{
	UINT32 op1=I5(op);
	UINT8 res=0;
	op1&=0xf;
	switch(op1)
	{
		case 0: //bv
			res=GET_OV;
			break;

		case 1: //bl
			res=GET_CY;
			break;

		case 2: //be
			res=GET_Z;
			break;

		case 3: //bnh
			res=GET_Z||GET_CY;
			break;

		case 4: //bn
			res=GET_S;
			break;

		case 5: //br
			res=1;
			break;

		case 6: //blt
			res=GET_S^GET_OV;
			break;

		case 7: //ble
			res=GET_Z||(GET_S^GET_OV);
		break;

		case 8: //bnv
			res=!GET_OV;
		break;

		case 9: //bnl
			res=!GET_CY;
		break;

		case 10: //bne
			res=!GET_Z;
		break;

		case 11: //bh
			res=!(GET_Z||GET_CY);
		break;

		case 12: //bp
			res=!GET_S;
		break;

		case 13: //nop

			break;

		case 14: //bge
			res=!(GET_OV^GET_S);
		break;

		case 15: //bgt
			res=!(GET_Z||(GET_OV^GET_S));
			break;
	}
	SETREG(GET2,res);
	return clkIF;
}


UINT32 v810_device::opANDr(UINT32 op)    // and r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}

UINT32 v810_device::opANDI(UINT32 op)    // andi imm16,r1,r2
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

UINT32 v810_device::opORr(UINT32 op) // or r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}

UINT32 v810_device::opORI(UINT32 op) // ori imm16,r1,r2
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

UINT32 v810_device::opXORr(UINT32 op)    // xor r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=GETREG(GET2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}


UINT32 v810_device::opLDSR(UINT32 op) // ldsr reg2,regID
{
	UINT32 op1=UI5(op);
	SETREG(32+op1,GETREG(GET2));
	return clkIF;
}

UINT32 v810_device::opSTSR(UINT32 op) // ldsr regID,reg2
{
	UINT32 op1=UI5(op);
	SETREG(GET2,GETREG(32+op1));
	return clkIF;
}


UINT32 v810_device::opXORI(UINT32 op)    // xori imm16,r1,r2
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

UINT32 v810_device::opNOTr(UINT32 op)    // not r1,r2
{
	UINT32 op1=GETREG(GET1);
	UINT32 op2=~op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return clkIF;
}

UINT32 v810_device::opSHLr(UINT32 op)    // shl r1,r2
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

UINT32 v810_device::opSHLi(UINT32 op)    // shl imm5,r2
{
	UINT64 tmp;
	UINT32 count=UI5(op);

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

UINT32 v810_device::opSHRr(UINT32 op)    // shr r1,r2
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

UINT32 v810_device::opSHRi(UINT32 op)    // shr imm5,r2
{
	UINT64 tmp;
	UINT32 count=UI5(op);
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

UINT32 v810_device::opSARr(UINT32 op)    // sar r1,r2
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

UINT32 v810_device::opSARi(UINT32 op)    // sar imm5,r2
{
	INT32 tmp;
	UINT32 count=UI5(op);
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

UINT32 v810_device::opJMPr(UINT32 op)
{
	PC=GETREG(GET1)&~1;
	return clkIF+2;
}


UINT32 v810_device::opJR(UINT32 op)
{
	UINT32 tmp=R_OP(PC);
	PC=PC-2+(D26(op,tmp)&~1);
	return clkIF+2;
}

UINT32 v810_device::opJAL(UINT32 op)
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	R31=PC;
	PC+=D26(op,tmp);
	PC-=4;
	PC&=~1;
	return clkIF+2;
}


UINT32 v810_device::opEI(UINT32 op)
{
	SET_ID(0);
	return clkIF;
}

UINT32 v810_device::opDI(UINT32 op)
{
	SET_ID(1);
	return clkIF;
}

UINT32 v810_device::opTRAP(UINT32 op)
{
	printf("V810: TRAP @ %X\n",PC-2);
	return clkIF;
}

UINT32 v810_device::opRETI(UINT32 op)
{
	if(GET_NP) {
		PC = FEPC;
		PSW = FEPSW;
	} else {
		PC = EIPC;
		PSW = EIPSW;
	}
	return clkIF;
}

UINT32 v810_device::opHALT(UINT32 op)
{
	printf("V810: HALT @ %X",PC-2);
	return clkIF;
}

UINT32 v810_device::opB(UINT32 op)
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
			PC=PC-2+(D9(op)&~1);
	}
	return clkIF;
}

UINT32 v810_device::opLDB(UINT32 op) // ld.b disp16[reg1],reg2
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

UINT32 v810_device::opLDH(UINT32 op) // ld.h disp16[reg1],reg2
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

UINT32 v810_device::opLDW(UINT32 op) // ld.w disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=R_W(tmp&~3);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

UINT32 v810_device::opINB(UINT32 op) // in.b disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=RIO_B(tmp);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

UINT32 v810_device::opCAXI(UINT32 op)    // caxi disp16[reg1],reg2
{
	printf("V810 CAXI execute\n");
	PC+=2;
	return clkIF;
}

UINT32 v810_device::opINH(UINT32 op) // in.h disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=RIO_H(tmp&~1);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

UINT32 v810_device::opINW(UINT32 op) // in.w disp16[reg1],reg2
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	tmp=RIO_W(tmp&~3);
	SETREG(GET2,tmp);
	return clkIF+clkMEM;
}

UINT32 v810_device::opSTB(UINT32 op) // st.b reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_B(tmp,GETREG(GET2)&0xff);
	return clkIF+clkMEM;
}

UINT32 v810_device::opSTH(UINT32 op) // st.h reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_H(tmp&~1,GETREG(GET2)&0xffff);
	return clkIF+clkMEM;
}

UINT32 v810_device::opSTW(UINT32 op) // st.w reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_W(tmp&~3,GETREG(GET2));
	return clkIF+clkMEM;
}

UINT32 v810_device::opOUTB(UINT32 op)    // out.b reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_B(tmp,GETREG(GET2)&0xff);
	return clkIF+clkMEM;
}

UINT32 v810_device::opOUTH(UINT32 op)    // out.h reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_H(tmp&~1,GETREG(GET2)&0xffff);
	return clkIF+clkMEM;
}

UINT32 v810_device::opOUTW(UINT32 op)    // out.w reg2, disp16[reg1]
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_W(tmp&~3,GETREG(GET2));
	return clkIF+clkMEM;
}

UINT32 v810_device::opMULr(UINT32 op)    // mul r1,r2
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

UINT32 v810_device::opMULUr(UINT32 op)   // mulu r1,r2
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

UINT32 v810_device::opDIVr(UINT32 op)    // div r1,r2
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
	else
		printf("DIVr divide by zero?\n");
	return clkIF;
}

UINT32 v810_device::opDIVUr(UINT32 op)   // divu r1,r2
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
	else
		printf("DIVUr divide by zero?\n");
	return clkIF;
}

void v810_device::opADDF(UINT32 op)
{
	//TODO: CY
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	val2+=val1;
	SET_Z((val2==0.0f)?1:0);
	SET_S((val2<0.0f)?1:0);
	SETREG(GET2,f2u(val2));
}

void v810_device::opSUBF(UINT32 op)
{
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	SET_CY((val2<val1)?1:0);
	val2-=val1;
	SET_Z((val2==0.0f)?1:0);
	SET_S((val2<0.0f)?1:0);
	SETREG(GET2,f2u(val2));
}

void v810_device::opMULF(UINT32 op)
{
	//TODO: CY
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	val2*=val1;
	SET_Z((val2==0.0f)?1:0);
	SET_S((val2<0.0f)?1:0);
	SETREG(GET2,f2u(val2));
}

void v810_device::opDIVF(UINT32 op)
{
	//TODO: CY
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	if(val1!=0)
		val2/=val1;
	else
		printf("DIVF divide by zero?\n");
	SET_Z((val2==0.0f)?1:0);
	SET_S((val2<0.0f)?1:0);
	SETREG(GET2,f2u(val2));
}

void v810_device::opTRNC(UINT32 op)
{
	float val1=u2f(GETREG(GET1));
	SET_OV(0);
	SET_Z((val1==0.0f)?1:0);
	SET_S((val1<0.0f)?1:0);
	SETREG(GET2,(INT32)val1);
}

void v810_device::opCMPF(UINT32 op)
{
	float val1=u2f(GETREG(GET1));
	float val2=u2f(GETREG(GET2));
	SET_OV(0);
	SET_CY((val2<val1)?1:0);
	val2-=val1;
	SET_Z((val2==0.0f)?1:0);
	SET_S((val2<0.0f)?1:0);
}

void v810_device::opCVTS(UINT32 op)
{
	float val1=u2f(GETREG(GET1));
	SET_OV(0);
	SET_Z((val1==0.0f)?1:0);
	SET_S((val1<0.0f)?1:0);
	SETREG(GET2,(INT32)val1);
}

void v810_device::opCVTW(UINT32 op)
{
	//TODO: CY
	float val1=(INT32)GETREG(GET1);
	SET_OV(0);
	SET_Z((val1==0.0f)?1:0);
	SET_S((val1<0.0f)?1:0);
	SETREG(GET2,f2u(val1));
}

void v810_device::opMPYHW(UINT32 op)
{
	int val1=(GETREG(GET1) & 0xffff);
	int val2=(GETREG(GET2) & 0xffff);
	SET_OV(0);
	val2*=val1;
	SET_Z((val2==0.0f)?1:0);
	SET_S((val2<0.0f)?1:0);
	SETREG(GET2,val2);
}

void v810_device::opXB(UINT32 op)
{
	int val=GETREG(GET2);
	SET_OV(0);
	val = (val & 0xffff0000) | ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
	SET_Z((val==0.0f)?1:0);
	SET_S((val<0.0f)?1:0);
	SETREG(GET2,val);
}


void v810_device::opXH(UINT32 op)
{
	int val=GETREG(GET2);
	SET_OV(0);
	val = ((val & 0xffff0000)>>16) | ((val & 0xffff)<<16);
	SET_Z((val==0.0f)?1:0);
	SET_S((val<0.0f)?1:0);
	SETREG(GET2,val);
}

UINT32 v810_device::opFpoint(UINT32 op)
{
	UINT32 tmp=R_OP(PC);
	PC+=2;
	switch((tmp&0xfc00)>>10)
	{
			case 0x0: opCMPF(op);break;
			case 0x2: opCVTW(op);break;
			case 0x3: opCVTS(op);break;
			case 0x4: opADDF(op);break;
			case 0x5: opSUBF(op);break;
			case 0x6: opMULF(op);break;
			case 0x7: opDIVF(op);break;
			case 0x8: opXB(op);  break; // *
			case 0x9: opXH(op);  break; // *
			case 0xb: opTRNC(op);break;
			case 0xc: opMPYHW(op); break; // *
			// * <- Virtual Boy specific?
			default: printf("Floating point %02x\n",(tmp&0xfc00) >> 10);break;
	}
	return clkIF+1;
}

/* TODO: clocks */
UINT32 v810_device::opBSU(UINT32 op)
{
	if(!(op & 8))
		fatalerror("V810: unknown BSU opcode %04x\n",op);

	{
		UINT32 srcbit,dstbit,src,dst,size;
		UINT32 dsttmp,tmp;
		UINT8 srctmp;

//      printf("BDST %08x BSRC %08x SIZE %08x DST %08x SRC %08x\n",R26,R27,R28,R29,R30);

		dstbit = R26 & 0x1f;
		srcbit = R27 & 0x1f;
		size =  R28;
		dst = R29 & ~3;
		src = R30 & ~3;

		switch(op & 0xf)
		{
			case 0x8: // ORBSU
				srctmp = (R_W(src) >> srcbit) & 1;
				dsttmp = R_W(dst);

				tmp = dsttmp | (srctmp << dstbit);

				W_W(dst,tmp);
				break;
			case 0x9: // ANDBSU
				srctmp = ((R_W(src) >> srcbit) & 1) ^ 1;
				dsttmp = R_W(dst);

				tmp = dsttmp & (~(srctmp << dstbit));

				W_W(dst,tmp);
				break;
			case 0xa: // XORBSU
				srctmp = (R_W(src) >> srcbit) & 1;
				dsttmp = R_W(dst);

				tmp = dsttmp ^ (srctmp << dstbit);

				W_W(dst,tmp);
				break;
			case 0xb: // MOVBSU
				srctmp = (R_W(src) >> srcbit) & 1;
				dsttmp = (R_W(dst) & ~(1 << dstbit));

				tmp = (srctmp << dstbit) | dsttmp;

				W_W(dst,tmp);
				break;
			case 0xc: // ORNBSU
				srctmp = ((R_W(src) >> srcbit) & 1) ^ 1;
				dsttmp = R_W(dst);

				tmp = dsttmp | (srctmp << dstbit);

				W_W(dst,tmp);
				break;
			case 0xd: // ANDNBSU
				srctmp = (R_W(src) >> srcbit) & 1;
				dsttmp = R_W(dst);

				tmp = dsttmp & (~(srctmp << dstbit));

				W_W(dst,tmp);
				break;
			case 0xe: // XORNBSU
				srctmp = ((R_W(src) >> srcbit) & 1) ^ 1;
				dsttmp = R_W(dst);

				tmp = dsttmp ^ (srctmp << dstbit);

				W_W(dst,tmp);
				break;
			case 0xf: // NOTBSU
				srctmp = ((R_W(src) >> srcbit) & 1) ^ 1;
				dsttmp = (R_W(dst) & ~(1 << dstbit));

				tmp = (srctmp << dstbit) | dsttmp;

				W_W(dst,tmp);
				break;
			default: fatalerror("V810: unemulated BSU opcode %04x\n",op);
		}

		srcbit++;
		dstbit++;

		srcbit&=0x1f;
		dstbit&=0x1f;

		if(srcbit == 0)
			src+=4;

		if(dstbit == 0)
			dst+=4;

		size --;

		R26 = dstbit;
		R27 = srcbit;
		R28 = size;
		R29 = dst;
		R30 = src;

		if(size != 0)
			PC-=2;
	}

	return clkIF+1; //TODO: correct?
}

const v810_device::opcode_func v810_device::s_OpCodeTable[64] =
{
	/* 0x00 */ &v810_device::opMOVr,      // mov r1,r2            1
	/* 0x01 */ &v810_device::opADDr,      // add r1,r2            1
	/* 0x02 */ &v810_device::opSUBr,      // sub r1,r2            1
	/* 0x03 */ &v810_device::opCMPr,      // cmp2 r1,r2           1
	/* 0x04 */ &v810_device::opSHLr,      // shl r1,r2            1
	/* 0x05 */ &v810_device::opSHRr,      // shr r1,r2            1
	/* 0x06 */ &v810_device::opJMPr,      // jmp [r1]             1
	/* 0x07 */ &v810_device::opSARr,      // sar r1,r2            1
	/* 0x08 */ &v810_device::opMULr,      // mul r1,r2            1
	/* 0x09 */ &v810_device::opDIVr,      // div r1,r2            1
	/* 0x0a */ &v810_device::opMULUr,     // mulu r1,r2           1
	/* 0x0b */ &v810_device::opDIVUr,     // divu r1,r2           1
	/* 0x0c */ &v810_device::opORr,       // or r1,r2             1
	/* 0x0d */ &v810_device::opANDr,      // and r1,r2            1
	/* 0x0e */ &v810_device::opXORr,      // xor r1,r2            1
	/* 0x0f */ &v810_device::opNOTr,      // not r1,r2            1
	/* 0x10 */ &v810_device::opMOVi,      // mov imm5,r2          2
	/* 0x11 */ &v810_device::opADDi,      // add imm5,r2          2
	/* 0x12 */ &v810_device::opSETFi,     // setf imm5,r2         2
	/* 0x13 */ &v810_device::opCMPi,      // cmp imm5,r2          2
	/* 0x14 */ &v810_device::opSHLi,      // shl imm5,r2          2
	/* 0x15 */ &v810_device::opSHRi,      // shr imm5,r2          2
	/* 0x16 */ &v810_device::opEI,        // ei               2
	/* 0x17 */ &v810_device::opSARi,      // sar imm5,r2          2
	/* 0x18 */ &v810_device::opTRAP,
	/* 0x19 */ &v810_device::opRETI,
	/* 0x1a */ &v810_device::opHALT,      // halt             2
	/* 0x1b */ &v810_device::opUNDEF,
	/* 0x1c */ &v810_device::opLDSR,      // ldsr reg2,regID          2
	/* 0x1d */ &v810_device::opSTSR,      // stsr regID,reg2          2
	/* 0x1e */ &v810_device::opDI,    // DI               2
	/* 0x1f */ &v810_device::opBSU,
	/* 0x20 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x21 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x22 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x23 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x24 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x25 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x26 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x27 */ &v810_device::opB,     // Branch (7 bit opcode)
	/* 0x28 */ &v810_device::opMOVEA,     // movea imm16, reg1, reg2  5
	/* 0x29 */ &v810_device::opADDI,      // addi imm16, reg1, reg2   5
	/* 0x2a */ &v810_device::opJR,    // jr disp26            4
	/* 0x2b */ &v810_device::opJAL,   // jal disp26           4
	/* 0x2c */ &v810_device::opORI,   // ori imm16, reg1, reg2    5
	/* 0x2d */ &v810_device::opANDI,      // andi imm16, reg1, reg2   5
	/* 0x2e */ &v810_device::opXORI,      // xori imm16, reg1, reg2   5
	/* 0x2f */ &v810_device::opMOVHI,     // movhi imm16, reg1 ,reg2  5
	/* 0x30 */ &v810_device::opLDB,   // ld.b disp16[reg1],reg2   6a
	/* 0x31 */ &v810_device::opLDH,   // ld.h disp16[reg1],reg2   6a
	/* 0x32 */ &v810_device::opUNDEF,
	/* 0x33 */ &v810_device::opLDW,   // ld.w disp16[reg1],reg2   6a
	/* 0x34 */ &v810_device::opSTB,   // st.b reg2, disp16[reg1]  6b
	/* 0x35 */ &v810_device::opSTH,   // st.h reg2, disp16[reg1]  6b
	/* 0x36 */ &v810_device::opUNDEF,
	/* 0x37 */ &v810_device::opSTW,   // st.w reg2, disp16[reg1]  6b
	/* 0x38 */ &v810_device::opINB,   // in.b disp16[reg1], reg2  6a
	/* 0x39 */ &v810_device::opINH,   // in.h disp16[reg1], reg2  6a
	/* 0x3a */ &v810_device::opCAXI,      // caxi disp16[reg1],reg2   6a
	/* 0x3b */ &v810_device::opINW,   // in.w disp16[reg1], reg2  6a
	/* 0x3c */ &v810_device::opOUTB,      // out.b reg2, disp16[reg1]     6b
	/* 0x3d */ &v810_device::opOUTH,      // out.h reg2, disp16[reg1]     6b
	/* 0x3e */ &v810_device::opFpoint, //floating point opcodes
	/* 0x3f */ &v810_device::opOUTW   // out.w reg2, disp16[reg1]     6b
};

void v810_device::device_start()
{
	m_irq_state = CLEAR_LINE;
	m_irq_line = 0;
	m_nmi_line = CLEAR_LINE;
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	m_irq_line = 0;
	m_irq_state = 0;
	m_nmi_line = 0;
	memset(m_reg, 0x00, sizeof(m_reg));

	save_item(NAME(m_reg));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_nmi_line));
	save_item(NAME(m_PPC));

	state_add( V810_PC,    "PC",    PC).formatstr("%08X");
	state_add( V810_R0,    "R0",    R0).formatstr("%08X");
	state_add( V810_R1,    "R1",    R1).formatstr("%08X");
	state_add( V810_R2,    "R2",    R2).formatstr("%08X");
	state_add( V810_SP,    "SP",    SP).formatstr("%08X");
	state_add( V810_R4,    "R4",    R4).formatstr("%08X");
	state_add( V810_R5,    "R5",    R5).formatstr("%08X");
	state_add( V810_R6,    "R6",    R6).formatstr("%08X");
	state_add( V810_R7,    "R7",    R7).formatstr("%08X");
	state_add( V810_R8,    "R8",    R8).formatstr("%08X");
	state_add( V810_R9,    "R9",    R9).formatstr("%08X");
	state_add( V810_R10,   "R10",   R10).formatstr("%08X");
	state_add( V810_R11,   "R11",   R11).formatstr("%08X");
	state_add( V810_R12,   "R12",   R12).formatstr("%08X");
	state_add( V810_R13,   "R13",   R13).formatstr("%08X");
	state_add( V810_R14,   "R14",   R14).formatstr("%08X");
	state_add( V810_R15,   "R15",   R15).formatstr("%08X");
	state_add( V810_R16,   "R16",   R16).formatstr("%08X");
	state_add( V810_R17,   "R17",   R17).formatstr("%08X");
	state_add( V810_R18,   "R18",   R18).formatstr("%08X");
	state_add( V810_R19,   "R19",   R19).formatstr("%08X");
	state_add( V810_R20,   "R20",   R20).formatstr("%08X");
	state_add( V810_R21,   "R21",   R21).formatstr("%08X");
	state_add( V810_R22,   "R22",   R22).formatstr("%08X");
	state_add( V810_R23,   "R23",   R23).formatstr("%08X");
	state_add( V810_R24,   "R24",   R24).formatstr("%08X");
	state_add( V810_R25,   "R25",   R25).formatstr("%08X");
	state_add( V810_R26,   "R26",   R26).formatstr("%08X");
	state_add( V810_R27,   "R27",   R27).formatstr("%08X");
	state_add( V810_R28,   "R28",   R28).formatstr("%08X");
	state_add( V810_R29,   "R29",   R29).formatstr("%08X");
	state_add( V810_R30,   "R30",   R30).formatstr("%08X");
	state_add( V810_R31,   "R31",   R31).formatstr("%08X");
	state_add( V810_EIPC,  "EIPC",  EIPC).formatstr("%08X");
	state_add( V810_PSW,   "PSW",   PSW).formatstr("%08X");
	state_add( V810_EIPSW, "EIPSW", EIPSW).formatstr("%08X");
	state_add( V810_FEPC,  "FEPC",  FEPC).formatstr("%08X");
	state_add( V810_FEPSW, "FEPSW", FEPSW).formatstr("%08X");
	state_add( V810_ECR,   "ECR",   ECR).formatstr("%08X");
	state_add( V810_PIR,   "PIR",   PIR).formatstr("%08X");
	state_add( V810_TKCW,  "TKCW",  TKCW).formatstr("%08X");
	state_add( V810_CHCW,  "CHCW",  CHCW).formatstr("%08X");
	state_add( V810_ADTRE, "ADTRE", ADTRE).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", PC).noshow();
	state_add(STATE_GENSP, "GENSP", SP).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", PSW).formatstr("%8s").noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_PPC).noshow();

	m_icountptr = &m_icount;
}

void v810_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
				GET_AE ? 'A':'.',
				GET_NP ? 'N':'.',
				GET_EP ? 'E':'.',
				GET_ID ? 'I':'.',
				GET_CY ? 'C':'.',
				GET_OV ? 'V':'.',
				GET_S ?  'S':'.',
				GET_Z ?  'Z':'.'
			);
			break;
	}
}

void v810_device::device_reset()
{
	int i;
	for(i=0;i<64;i++)   m_reg[i]=0;
	PC = 0xfffffff0;
	PSW = 0x1000;
	ECR   = 0x0000fff0;
}

void v810_device::take_interrupt()
{
	EIPC = PC;
	EIPSW = PSW;

	PC = 0xfffffe00 | (m_irq_line << 4);
	ECR = 0xfe00 | (m_irq_line << 4);

	UINT8 num = m_irq_line + 1;
	if (num==0x10) num=0x0f;

	PSW &= 0xfff0ffff; // clear interrupt level
	SET_EP(1);
	SET_ID(1);
	PSW |= num << 16;

	m_icount-= clkIF;
}

void v810_device::execute_run()
{
	if (m_irq_state != CLEAR_LINE) {
		if (!(GET_NP | GET_EP | GET_ID)) {
			if (m_irq_line >=((PSW & 0xF0000) >> 16)) {
				take_interrupt();
			}
		}
	}
	while(m_icount>0)
	{
		UINT32 op;

		m_PPC=PC;
		debugger_instruction_hook(this, PC);
		op=R_OP(PC);
		PC+=2;
		int cnt;
		cnt = (this->*s_OpCodeTable[op>>10])(op);
		m_icount-= cnt;
	}
}


void v810_device::execute_set_input( int irqline, int state)
{
	m_irq_state = state;
	m_irq_line = irqline;
}
