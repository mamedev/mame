// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "debugger.h"
#include "se3208.h"


static struct
{
	UINT32 PC;
	UINT32 SR;
	UINT32 ER;
} Context;

#define FLAG_E      0x0800

#define CLRFLAG(f)  Context.SR&=~(f);
#define SETFLAG(f)  Context.SR|=(f);
#define TESTFLAG(f) (Context.SR&(f))

#define EXTRACT(val,sbit,ebit)  (((val)>>sbit)&((1<<((ebit-sbit)+1))-1))
#define SEX8(val)   ((val&0x80)?(val|0xFFFFFF00):(val&0xFF))
#define SEX16(val)  ((val&0x8000)?(val|0xFFFF0000):(val&0xFFFF))
#define ZEX8(val)   ((val)&0xFF)
#define ZEX16(val)  ((val)&0xFFFF)
#define SEX(bits,val)   ((val)&(1<<(bits-1))?((val)|(~((1<<bits)-1))):(val&((1<<bits)-1)))

typedef UINT32 (*_OP)(UINT16 Opcode,char *dst);
#define INST(a) static UINT32 a(UINT16 Opcode,char *dst)


INST(INVALIDOP)
{
	sprintf(dst,"INVALID");
	return 0;
}

INST(LDB)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"LDB   (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		sprintf(dst,"LDB   (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STB)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"STB   %%R%d,(%%R%d,0x%x)",SrcDst,Index,Offset);
	else
		sprintf(dst,"STB   %%R%d,(0x%x)",SrcDst,Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDS)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"LDS   (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		sprintf(dst,"LDS   (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STS)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"STS   %%R%d,(%%R%d,0x%x)",SrcDst,Index,Offset);
	else
		sprintf(dst,"STS   %%R%d,(0x%x)",SrcDst,Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LD)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"LD    (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		sprintf(dst,"LD    (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ST)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"ST    %%R%d,(%%R%d,0x%x)",SrcDst,Index,Offset);
	else
		sprintf(dst,"ST    %%R%d,(0x%x)",SrcDst,Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBU)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"LDBU  (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		sprintf(dst,"LDBU  (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSU)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		sprintf(dst,"LDSU  (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		sprintf(dst,"LDSU  (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}


INST(LERI)
{
	UINT32 Imm=EXTRACT(Opcode,0,13);

	if(TESTFLAG(FLAG_E))
		Context.ER=(EXTRACT(Context.ER,0,17)<<14)|Imm;
	else
		Context.ER=SEX(14,Imm);

	//sprintf(dst,"LERI  0x%x\t\tER=%08X",Imm,Context.ER);
	sprintf(dst,"LERI  0x%x",Imm/*,Context.ER*/);

	SETFLAG(FLAG_E);
	return 0;
}

INST(LDSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"LD    (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"ST    %%R%d,(%%SP,0x%x)",SrcDst,Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(PUSH)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	char str[1024];
	strcpy(str,"PUSH  ");
	if(Set&(1<<10))
		strcat(str,"%PC-");
	if(Set&(1<<9))
		strcat(str,"%SR-");
	if(Set&(1<<8))
		strcat(str,"%ER-");
	if(Set&(1<<7))
		strcat(str,"%R7-");
	if(Set&(1<<6))
		strcat(str,"%R6-");
	if(Set&(1<<5))
		strcat(str,"%R5-");
	if(Set&(1<<4))
		strcat(str,"%R4-");
	if(Set&(1<<3))
		strcat(str,"%R3-");
	if(Set&(1<<2))
		strcat(str,"%R2-");
	if(Set&(1<<1))
		strcat(str,"%R1-");
	if(Set&(1<<0))
		strcat(str,"%R0-");
	str[strlen(str)-1]=0;
	strcpy(dst,str);
	return 0;
}

INST(POP)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	char str[1024];
	int Ret=0;
	strcpy(str,"POP   ");
	if(Set&(1<<0))
		strcat(str,"%R0-");
	if(Set&(1<<1))
		strcat(str,"%R1-");
	if(Set&(1<<2))
		strcat(str,"%R2-");
	if(Set&(1<<3))
		strcat(str,"%R3-");
	if(Set&(1<<4))
		strcat(str,"%R4-");
	if(Set&(1<<5))
		strcat(str,"%R5-");
	if(Set&(1<<6))
		strcat(str,"%R6-");
	if(Set&(1<<7))
		strcat(str,"%R7-");
	if(Set&(1<<8))
		strcat(str,"%ER-");
	if(Set&(1<<9))
		strcat(str,"%SR-");
	if(Set&(1<<10))
	{
		strcat(str,"%PC-");
		CLRFLAG(FLAG_E);    //Clear the flag, this is a ret so disassemble will start a new E block
		Ret=1;
	}
	str[strlen(str)-1]=0;
	if(Ret)
		strcat(str,"\n");
	strcpy(dst,str);
	return Ret ? DASMFLAG_STEP_OUT : 0;
}

INST(LEATOSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	if(Index)
		sprintf(dst,"LEA   (%%R%d,0x%x),%%SP",Index,Offset);
	else
		sprintf(dst,"LEA   (0x%x),%%SP",Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LEAFROMSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	sprintf(dst,"LEA   (%%SP,0x%x),%%R%d",Offset,Index);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LEASPTOSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,23)<<8)|(Offset&0xff);
	else
		Offset=SEX(10,Offset);


	sprintf(dst,"LEA   (%%SP,0x%x),%%SP",Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(MOV)
{
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,9,11);

	if(Src==0 && Dst==0)
		sprintf(dst,"NOP");
	else
		sprintf(dst,"MOV   %%SR%d,%%DR%d",Src,Dst);
	return 0;
}

INST(LDI)
{
	UINT32 Dst=EXTRACT(Opcode,8,10);
	UINT32 Imm=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX8(Imm);

	sprintf(dst,"LDI   0x%x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"LDB   (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"STB   %%R%d,(%%SP,0x%x)",SrcDst,Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"LDS   (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"STS   %%R%d,(%%SP,0x%x)",SrcDst,Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"LDBU  (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	sprintf(dst,"LDSU  (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ADDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"ADD   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SUBI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"SUB   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);

	return 0;
}

INST(ADCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"ADC   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SBCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"SBC   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ANDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"AND   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"OR    %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(XORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"XOR   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(CMPI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"CMP   %%SR%d,0x%x",Src,Imm2/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(TSTI)
{
		UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	sprintf(dst,"TST   %%SR%d,0x%x",Src,Imm2/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ADD)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"ADD   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(SUB)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"SUB   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(ADC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"ADC   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(SBC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"SBC   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(AND)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"AND   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(OR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"OR    %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(XOR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"XOR   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(CMP)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);

	sprintf(dst,"CMP   %%SR%d,%%SR%d",Src1,Src2);
	return 0;
}

INST(TST)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);

	sprintf(dst,"TST   %%SR%d,%%SR%d",Src1,Src2);
	return 0;
}

INST(MULS)
{
	UINT32 Src2=EXTRACT(Opcode,6,8);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	sprintf(dst,"MUL   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(NEG)
{
	UINT32 Dst=EXTRACT(Opcode,9,11);
	UINT32 Src=EXTRACT(Opcode,3,5);

	sprintf(dst,"NEG   %%SR%d,%%DR%d",Src,Dst);
	return 0;
}

INST(CALL)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"CALL  0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return DASMFLAG_STEP_OVER;
}

INST(JV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JV    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JNV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JNV   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JC    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JNC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JNC   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JP    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JM)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JM    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JNZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JNZ   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JZ    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JGE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JGE   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JLE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JLE   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JHI)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JHI   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JLS)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JLS   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JGT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JGT   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JLT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JLT   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}



INST(JMP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	sprintf(dst,"JMP   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);

	sprintf(dst,"JR    %%R%d",Src);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(CALLR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);

	sprintf(dst,"CALLR %%R%d",Src);

	CLRFLAG(FLAG_E);
	return DASMFLAG_STEP_OVER;
}

INST(ASR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		sprintf(dst,"ASR   %%R%d,%%R%d",Cnt,Dst);
	else
		sprintf(dst,"ASR   %x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LSR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		sprintf(dst,"LSR   %%R%d,%%R%d",Cnt,Dst);
	else
		sprintf(dst,"LSR   %x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ASL)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		sprintf(dst,"ASL   %%R%d,%%R%d",Cnt,Dst);
	else
		sprintf(dst,"ASL   %x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(EXTB)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);

	sprintf(dst,"EXTB  %%R%d",Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(EXTS)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);

	sprintf(dst,"EXTS  %%R%d",Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SET)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	sprintf(dst,"SET   0x%x",Imm);
	return 0;
}

INST(CLR)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	sprintf(dst,"CLR   0x%x",Imm);
	return 0;
}

INST(SWI)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	sprintf(dst,"SWI   0x%x",Imm);
	return 0;
}

INST(HALT)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	sprintf(dst,"HALT  0x%x",Imm);
	return 0;
}

INST(MVTC)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	sprintf(dst,"MVTC  %%R0,%%CR%d",Imm);
	return 0;
}

INST(MVFC)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	sprintf(dst,"MVFC  %%CR0%d,%%R0",Imm);
	return 0;
}

static _OP DecodeOp(UINT16 Opcode)
{
	switch(EXTRACT(Opcode,14,15))
	{
		case 0x0:
			{
				UINT8 Op=EXTRACT(Opcode,11,13);
				switch(Op)
				{
					case 0x0:
						return LDB;
					case 0x1:
						return LDS;
					case 0x2:
						return LD;
					case 0x3:
						return LDBU;
					case 0x4:
						return STB;
					case 0x5:
						return STS;
					case 0x6:
						return ST;
					case 0x7:
						return LDSU;
				}
			}
			break;
		case 0x1:
			return LERI;
		case 0x2:
			{
				switch(EXTRACT(Opcode,11,13))
				{
					case 0:
						return LDSP;
					case 1:
						return STSP;
					case 2:
						return PUSH;
					case 3:
						return POP;
					case 4:
					case 5:
					case 6:
					case 7:
					case 8: //arith
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
						switch(EXTRACT(Opcode,6,8))
						{
							case 0:
								return ADDI;
							case 1:
								return ADCI;
							case 2:
								return SUBI;
							case 3:
								return SBCI;
							case 4:
								return ANDI;
							case 5:
								return ORI;
							case 6:
								return XORI;
							case 7:
								switch(EXTRACT(Opcode,0,2))
								{
									case 0:
										return CMPI;
									case 1:
										return TSTI;
									case 2:
										return LEATOSP;
									case 3:
										return LEAFROMSP;
								}
								break;
						}
						break;
				}
			}
			break;
		case 3:
			switch(EXTRACT(Opcode,12,13))
			{
				case 0:
					switch(EXTRACT(Opcode,6,8))
					{
						case 0:
							return ADD;
						case 1:
							return ADC;
						case 2:
							return SUB;
						case 3:
							return SBC;
						case 4:
							return AND;
						case 5:
							return OR;
						case 6:
							return XOR;
						case 7:
							switch(EXTRACT(Opcode,0,2))
							{
								case 0:
									return CMP;
								case 1:
									return TST;
								case 2:
									return MOV;
								case 3:
									return NEG;
							}
							break;
					}
					break;
				case 1:     //Jumps
					switch(EXTRACT(Opcode,8,11))
					{
						case 0x0:
							return JNV;
						case 0x1:
							return JV;
						case 0x2:
							return JP;
						case 0x3:
							return JM;
						case 0x4:
							return JNZ;
						case 0x5:
							return JZ;
						case 0x6:
							return JNC;
						case 0x7:
							return JC;
						case 0x8:
							return JGT;
						case 0x9:
							return JLT;
						case 0xa:
							return JGE;
						case 0xb:
							return JLE;
						case 0xc:
							return JHI;
						case 0xd:
							return JLS;
						case 0xe:
							return JMP;
						case 0xf:
							return CALL;
					}
					break;
				case 2:
					if(Opcode&(1<<11))
						return LDI;
					else    //SP Ops
					{
						if(Opcode&(1<<10))
						{
							switch(EXTRACT(Opcode,7,9))
							{
								case 0:
									return LDBSP;
								case 1:
									return LDSSP;
								case 3:
									return LDBUSP;
								case 4:
									return STBSP;
								case 5:
									return STSSP;
								case 7:
									return LDSUSP;
							}
						}
						else
						{
							if(Opcode&(1<<9))
							{
								return LEASPTOSP;
							}
							else
							{
								if(Opcode&(1<<8))
								{
								}
								else
								{
									switch(EXTRACT(Opcode,4,7))
									{
										case 0:
											return EXTB;
										case 1:
											return EXTS;
										case 8:
											return JR;
										case 9:
											return CALLR;
										case 10:
											return SET;
										case 11:
											return CLR;
										case 12:
											return SWI;
										case 13:
											return HALT;
									}
								}
							}
						}
					}
					break;
				case 3:
					switch(EXTRACT(Opcode,9,11))
					{
						case 0:
						case 1:
						case 2:
						case 3:
							switch(EXTRACT(Opcode,3,4))
							{
								case 0:
									return ASR;
								case 1:
									return LSR;
								case 2:
									return ASL;
								//case 3:
								//  return LSL;
							}
							break;
						case 4:
							return MULS;
						case 6:
							if(Opcode&(1<<3))
								return MVFC;
							else
								return MVTC;
					}
					break;
			}
			break;

	}
	return INVALIDOP;
}


CPU_DISASSEMBLE( se3208 )
{
	UINT16 Opcode;

	CLRFLAG(FLAG_E);
	Context.ER=0;

	Context.PC=pc;
	Opcode=oprom[0] | (oprom[1] << 8);
	return 2 | ((*DecodeOp(Opcode))(Opcode,buffer)) | DASMFLAG_SUPPORTED;
}
