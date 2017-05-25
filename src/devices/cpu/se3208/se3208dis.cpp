// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "debugger.h"
#include "se3208.h"


static struct
{
	uint32_t PC;
	uint32_t SR;
	uint32_t ER;
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

typedef uint32_t (*_OP)(uint16_t Opcode, std::ostream &stream);
#define INST(a) static uint32_t a(uint16_t Opcode, std::ostream &stream)


INST(INVALIDOP)
{
	util::stream_format(stream, "INVALID");
	return 0;
}

INST(LDB)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "LDB   (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		util::stream_format(stream, "LDB   (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STB)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "STB   %%R%d,(%%R%d,0x%x)",SrcDst,Index,Offset);
	else
		util::stream_format(stream, "STB   %%R%d,(0x%x)",SrcDst,Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDS)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "LDS   (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		util::stream_format(stream, "LDS   (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STS)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "STS   %%R%d,(%%R%d,0x%x)",SrcDst,Index,Offset);
	else
		util::stream_format(stream, "STS   %%R%d,(0x%x)",SrcDst,Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LD)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "LD    (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		util::stream_format(stream, "LD    (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ST)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "ST    %%R%d,(%%R%d,0x%x)",SrcDst,Index,Offset);
	else
		util::stream_format(stream, "ST    %%R%d,(0x%x)",SrcDst,Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBU)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "LDBU  (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		util::stream_format(stream, "LDBU  (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSU)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	if(Index)
		util::stream_format(stream, "LDSU  (%%R%d,0x%x),%%R%d",Index,Offset,SrcDst);
	else
		util::stream_format(stream, "LDSU  (0x%x),%%R%d",Index+Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}


INST(LERI)
{
	uint32_t Imm=EXTRACT(Opcode,0,13);

	if(TESTFLAG(FLAG_E))
		Context.ER=(EXTRACT(Context.ER,0,17)<<14)|Imm;
	else
		Context.ER=SEX(14,Imm);

	//util::stream_format(stream, "LERI  0x%x\t\tER=%08X",Imm,Context.ER);
	util::stream_format(stream, "LERI  0x%x",Imm/*,Context.ER*/);

	SETFLAG(FLAG_E);
	return 0;
}

INST(LDSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "LD    (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "ST    %%R%d,(%%SP,0x%x)",SrcDst,Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(PUSH)
{
	uint32_t Set=EXTRACT(Opcode,0,10);
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
	stream << str;
	return 0;
}

INST(POP)
{
	uint32_t Set=EXTRACT(Opcode,0,10);
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
	stream << str;
	return Ret ? DASMFLAG_STEP_OUT : 0;
}

INST(LEATOSP)
{
	uint32_t Offset=EXTRACT(Opcode,9,12);
	uint32_t Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	if(Index)
		util::stream_format(stream, "LEA   (%%R%d,0x%x),%%SP",Index,Offset);
	else
		util::stream_format(stream, "LEA   (0x%x),%%SP",Index+Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LEAFROMSP)
{
	uint32_t Offset=EXTRACT(Opcode,9,12);
	uint32_t Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	util::stream_format(stream, "LEA   (%%SP,0x%x),%%R%d",Offset,Index);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LEASPTOSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,23)<<8)|(Offset&0xff);
	else
		Offset=SEX(10,Offset);


	util::stream_format(stream, "LEA   (%%SP,0x%x),%%SP",Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(MOV)
{
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,9,11);

	if(Src==0 && Dst==0)
		util::stream_format(stream, "NOP");
	else
		util::stream_format(stream, "MOV   %%SR%d,%%DR%d",Src,Dst);
	return 0;
}

INST(LDI)
{
	uint32_t Dst=EXTRACT(Opcode,8,10);
	uint32_t Imm=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX8(Imm);

	util::stream_format(stream, "LDI   0x%x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "LDB   (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STBSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "STB   %%R%d,(%%SP,0x%x)",SrcDst,Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "LDS   (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(STSSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "STS   %%R%d,(%%SP,0x%x)",SrcDst,Offset);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDBUSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "LDBU  (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LDSUSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	util::stream_format(stream, "LDSU  (%%SP,0x%x),%%R%d",Offset,SrcDst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ADDI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "ADD   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SUBI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "SUB   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);

	return 0;
}

INST(ADCI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "ADC   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SBCI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "SBC   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ANDI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "AND   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ORI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "OR    %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(XORI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "XOR   %%SR%d,0x%x,%%DR%d",Src,Imm2,Dst/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(CMPI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "CMP   %%SR%d,0x%x",Src,Imm2/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(TSTI)
{
		uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Imm2=Imm;

	if(TESTFLAG(FLAG_E))
		Imm2=(EXTRACT(Context.ER,0,27)<<4)|(Imm2&0xf);
	else
		Imm2=SEX(4,Imm2);

	util::stream_format(stream, "TST   %%SR%d,0x%x",Src,Imm2/*,Imm2*/);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ADD)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "ADD   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(SUB)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "SUB   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(ADC)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "ADC   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(SBC)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "SBC   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(AND)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "AND   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(OR)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "OR    %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(XOR)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "XOR   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);
	return 0;
}

INST(CMP)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);

	util::stream_format(stream, "CMP   %%SR%d,%%SR%d",Src1,Src2);
	return 0;
}

INST(TST)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);

	util::stream_format(stream, "TST   %%SR%d,%%SR%d",Src1,Src2);
	return 0;
}

INST(MULS)
{
	uint32_t Src2=EXTRACT(Opcode,6,8);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	util::stream_format(stream, "MUL   %%SR%d,%%SR%d,%%DR%d",Src1,Src2,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(NEG)
{
	uint32_t Dst=EXTRACT(Opcode,9,11);
	uint32_t Src=EXTRACT(Opcode,3,5);

	util::stream_format(stream, "NEG   %%SR%d,%%DR%d",Src,Dst);
	return 0;
}

INST(CALL)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "CALL  0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return DASMFLAG_STEP_OVER;
}

INST(JV)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JV    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JNV)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JNV   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JC)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JC    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JNC)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JNC   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JP    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JM)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JM    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JNZ)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JNZ   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JZ)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JZ    0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JGE)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JGE   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JLE)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JLE   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JHI)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JHI   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JLS)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JLS   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JGT)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JGT   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JLT)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JLT   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}



INST(JMP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Offset2;

	if(TESTFLAG(FLAG_E))
		Offset2=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset2=SEX(8,Offset);
	Offset2<<=1;
	util::stream_format(stream, "JMP   0x%x",Context.PC+2+Offset2);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(JR)
{
	uint32_t Src=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "JR    %%R%d",Src);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(CALLR)
{
	uint32_t Src=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "CALLR %%R%d",Src);

	CLRFLAG(FLAG_E);
	return DASMFLAG_STEP_OVER;
}

INST(ASR)
{
	uint32_t CS=Opcode&(1<<10);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm=EXTRACT(Opcode,5,9);
	uint32_t Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		util::stream_format(stream, "ASR   %%R%d,%%R%d",Cnt,Dst);
	else
		util::stream_format(stream, "ASR   %x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(LSR)
{
	uint32_t CS=Opcode&(1<<10);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm=EXTRACT(Opcode,5,9);
	uint32_t Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		util::stream_format(stream, "LSR   %%R%d,%%R%d",Cnt,Dst);
	else
		util::stream_format(stream, "LSR   %x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(ASL)
{
	uint32_t CS=Opcode&(1<<10);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm=EXTRACT(Opcode,5,9);
	uint32_t Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		util::stream_format(stream, "ASL   %%R%d,%%R%d",Cnt,Dst);
	else
		util::stream_format(stream, "ASL   %x,%%R%d",Imm,Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(EXTB)
{
	uint32_t Dst=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "EXTB  %%R%d",Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(EXTS)
{
	uint32_t Dst=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "EXTS  %%R%d",Dst);

	CLRFLAG(FLAG_E);
	return 0;
}

INST(SET)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "SET   0x%x",Imm);
	return 0;
}

INST(CLR)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "CLR   0x%x",Imm);
	return 0;
}

INST(SWI)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "SWI   0x%x",Imm);
	return 0;
}

INST(HALT)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "HALT  0x%x",Imm);
	return 0;
}

INST(MVTC)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "MVTC  %%R0,%%CR%d",Imm);
	return 0;
}

INST(MVFC)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	util::stream_format(stream, "MVFC  %%CR0%d,%%R0",Imm);
	return 0;
}

static _OP DecodeOp(uint16_t Opcode)
{
	switch(EXTRACT(Opcode,14,15))
	{
		case 0x0:
			{
				uint8_t Op=EXTRACT(Opcode,11,13);
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


CPU_DISASSEMBLE(se3208)
{
	uint16_t Opcode;

	CLRFLAG(FLAG_E);
	Context.ER=0;

	Context.PC=pc;
	Opcode=oprom[0] | (oprom[1] << 8);
	return 2 | ((*DecodeOp(Opcode))(Opcode, stream)) | DASMFLAG_SUPPORTED;
}
