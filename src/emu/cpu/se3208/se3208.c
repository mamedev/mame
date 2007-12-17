#include "debugger.h"
#include "se3208.h"

/*
    SE3208 CPU Emulator by ElSemi

    For information about this CPU:
    www.adc.co.kr

*/

typedef struct
{
	//GPR
	UINT32 R[8];
	//SPR
	UINT32 PC;
	UINT32 SR;
	UINT32 SP;
	UINT32 ER;
	UINT32 PPC;

	int (*irq_callback)(int irqline);
	UINT8 IRQ;
	UINT8 NMI;
} _SE3208Context;

#define FLAG_C		0x0080
#define FLAG_V		0x0010
#define FLAG_S		0x0020
#define FLAG_Z		0x0040

#define FLAG_M		0x0200
#define FLAG_E		0x0800
#define FLAG_AUT	0x1000
#define FLAG_ENI	0x2000
#define FLAG_NMI	0x4000

static _SE3208Context Context;
static int SE3208_ICount;

#define CLRFLAG(f)	Context.SR&=~(f);
#define SETFLAG(f)	Context.SR|=(f);
#define TESTFLAG(f)	(Context.SR&(f))

#define EXTRACT(val,sbit,ebit)	(((val)>>sbit)&((1<<((ebit-sbit)+1))-1))
#define SEX8(val)	((val&0x80)?(val|0xFFFFFF00):(val&0xFF))
#define SEX16(val)	((val&0x8000)?(val|0xFFFF0000):(val&0xFFFF))
#define ZEX8(val)	((val)&0xFF)
#define ZEX16(val)	((val)&0xFFFF)
#define SEX(bits,val)	((val)&(1<<(bits-1))?((val)|(~((1<<bits)-1))):(val&((1<<bits)-1)))

//Precompute the instruction decoding in a big table
typedef void (*_OP)(UINT16 Opcode);
#define INST(a) static void a(UINT16 Opcode)
static _OP *OpTable=NULL;

INLINE UINT32 read_dword_unaligned(UINT32 address)
{
	if (address & 3)
		return program_read_byte_32le(address) | program_read_byte_32le(address+1)<<8 | program_read_byte_32le(address+2)<<16 | program_read_byte_32le(address+3)<<24;
	else
		return program_read_dword_32le(address);
}

INLINE UINT16 read_word_unaligned(UINT32 address)
{
	if (address & 1)
		return program_read_byte_32le(address) | program_read_byte_32le(address+1)<<8;
	else
		return program_read_word_32le(address);
}

INLINE void write_dword_unaligned(UINT32 address, UINT32 data)
{
	if (address & 3)
	{
		program_write_byte_32le(address, data & 0xff);
		program_write_byte_32le(address+1, (data>>8)&0xff);
		program_write_byte_32le(address+2, (data>>16)&0xff);
		program_write_byte_32le(address+3, (data>>24)&0xff);
	}
	else
	{
		program_write_dword_32le(address, data);
	}
}

INLINE void write_word_unaligned(UINT32 address, UINT16 data)
{
	if (address & 1)
	{
		program_write_byte_32le(address, data & 0xff);
		program_write_byte_32le(address+1, (data>>8)&0xff);
	}
	else
	{
		program_write_word_32le(address, data);
	}
}


INLINE UINT8 SE3208_Read8(UINT32 addr)
{
	return program_read_byte_32le(addr);
}

INLINE UINT16 SE3208_Read16(UINT32 addr)
{
	return read_word_unaligned(addr);
}

INLINE UINT32 SE3208_Read32(UINT32 addr)
{
	return read_dword_unaligned(addr);
}

INLINE void SE3208_Write8(UINT32 addr,UINT8 val)
{
	program_write_byte_32le(addr,val);
}

INLINE void SE3208_Write16(UINT32 addr,UINT16 val)
{
	write_word_unaligned(addr,val);
}

INLINE void SE3208_Write32(UINT32 addr,UINT32 val)
{
	write_dword_unaligned(addr,val);
}



INLINE UINT32 AddWithFlags(UINT32 a,UINT32 b)
{
	UINT32 r=a+b;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if(((((a&b)|(~r&(a|b)))>>31))&1)
		SETFLAG(FLAG_C);
	if(((((a^r)&(b^r))>>31))&1)
		SETFLAG(FLAG_V);
	return r;
}

INLINE UINT32 SubWithFlags(UINT32 a,UINT32 b)	//a-b
{
	UINT32 r=a-b;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if((((b&r)|(~a&(b|r)))>>31)&1)
		SETFLAG(FLAG_C);
	if((((b^a)&(r^a))>>31)&1)
		SETFLAG(FLAG_V);
	return r;
}

INLINE UINT32 AdcWithFlags(UINT32 a,UINT32 b)
{
	UINT32 C=(Context.SR&FLAG_C)?1:0;
	UINT32 r=a+b+C;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if(((((a&b)|(~r&(a|b)))>>31))&1)
		SETFLAG(FLAG_C);
	if(((((a^r)&(b^r))>>31))&1)
		SETFLAG(FLAG_V);
	return r;

}

INLINE UINT32 SbcWithFlags(UINT32 a,UINT32 b)
{
	UINT32 C=(Context.SR&FLAG_C)?1:0;
	UINT32 r=a-b-C;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if((((b&r)|(~a&(b|r)))>>31)&1)
		SETFLAG(FLAG_C);
	if((((b^a)&(r^a))>>31)&1)
		SETFLAG(FLAG_V);
	return r;
}

INLINE UINT32 MulWithFlags(UINT32 a,UINT32 b)
{
	INT64 r=(INT64) a*(INT64) b;
	CLRFLAG(FLAG_V);
	if(r>>32)
		SETFLAG(FLAG_V);
	return (UINT32) (r&0xffffffff);
}

INLINE UINT32 NegWithFlags(UINT32 a)
{
	return SubWithFlags(0,a);
}

INLINE UINT32 AsrWithFlags(UINT32 Val, UINT8 By)
{
	signed int v=(signed int) Val;
	v>>=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(Val&(1<<(By-1)))
		SETFLAG(FLAG_C);
	return (UINT32) v;
}

INLINE UINT32 LsrWithFlags(UINT32 Val, UINT8 By)
{
	UINT32 v=Val;
	v>>=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(Val&(1<<(By-1)))
		SETFLAG(FLAG_C);
	return v;
}

INLINE UINT32 AslWithFlags(UINT32 Val, UINT8 By)
{
	UINT32 v=Val;
	v<<=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(Val&(1<<(32-By)))
		SETFLAG(FLAG_C);
	return v;
}


INST(INVALIDOP)
{
	//assert(false);
}

INST(LDB)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);
	UINT32 Val;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	Context.R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STB)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(Index+Offset,ZEX8(Context.R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDS)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);
	UINT32 Val;

	Offset<<=1;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	Context.R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STS)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(Index+Offset,ZEX16(Context.R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LD)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Context.R[SrcDst]=SE3208_Read32(Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(ST)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(Index+Offset,Context.R[SrcDst]);

	CLRFLAG(FLAG_E);
}

INST(LDBU)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);
	UINT32 Val;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	Context.R[SrcDst]=ZEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(LDSU)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);
	UINT32 Val;

	Offset<<=1;

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	Context.R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}


INST(LERI)
{
	UINT32 Imm=EXTRACT(Opcode,0,13);
	if(TESTFLAG(FLAG_E))
		Context.ER=(EXTRACT(Context.ER,0,17)<<14)|Imm;
	else
		Context.ER=SEX(14,Imm);


	SETFLAG(FLAG_E);
}

INST(LDSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Context.R[SrcDst]=SE3208_Read32(Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(STSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(Index+Offset,Context.R[SrcDst]);

	CLRFLAG(FLAG_E);
}

static void PushVal(UINT32 Val)
{
	Context.SP-=4;
	SE3208_Write32(Context.SP,Val);
}

static UINT32 PopVal(void)
{
	UINT32 Val=SE3208_Read32(Context.SP);
	Context.SP+=4;
	return Val;
}

INST(PUSH)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<10))
		PushVal(Context.PC);
	if(Set&(1<<9))
		PushVal(Context.SR);
	if(Set&(1<<8))
		PushVal(Context.ER);
	if(Set&(1<<7))
		PushVal(Context.R[7]);
	if(Set&(1<<6))
		PushVal(Context.R[6]);
	if(Set&(1<<5))
		PushVal(Context.R[5]);
	if(Set&(1<<4))
		PushVal(Context.R[4]);
	if(Set&(1<<3))
		PushVal(Context.R[3]);
	if(Set&(1<<2))
		PushVal(Context.R[2]);
	if(Set&(1<<1))
		PushVal(Context.R[1]);
	if(Set&(1<<0))
		PushVal(Context.R[0]);
}

INST(POP)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<0))
		Context.R[0]=PopVal();
	if(Set&(1<<1))
		Context.R[1]=PopVal();
	if(Set&(1<<2))
		Context.R[2]=PopVal();
	if(Set&(1<<3))
		Context.R[3]=PopVal();
	if(Set&(1<<4))
		Context.R[4]=PopVal();
	if(Set&(1<<5))
		Context.R[5]=PopVal();
	if(Set&(1<<6))
		Context.R[6]=PopVal();
	if(Set&(1<<7))
		Context.R[7]=PopVal();
	if(Set&(1<<8))
		Context.ER=PopVal();
	if(Set&(1<<9))
		Context.SR=PopVal();
	if(Set&(1<<10))
	{
		Context.PC=PopVal()-2;		//PC automatically incresases by 2
		change_pc(Context.PC+2);
	}
}

INST(LEATOSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(Index)
		Index=Context.R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	Context.SP=Index+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEAFROMSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	Context.R[Index]=Context.SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEASPTOSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,23)<<8)|(Offset&0xff);
	else
		Offset=SEX(10,Offset);

	Context.SP=Context.SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(MOV)
{
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,9,11);

	Context.R[Dst]=Context.R[Src];
}

INST(LDI)
{
	UINT32 Dst=EXTRACT(Opcode,8,10);
	UINT32 Imm=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX8(Imm);

	Context.R[Dst]=Imm;

	CLRFLAG(FLAG_E);
}

INST(LDBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	Context.R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(Index+Offset,ZEX8(Context.R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	Context.R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(Index+Offset,ZEX16(Context.R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDBUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	Context.R[SrcDst]=ZEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(LDSUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=Context.SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	Context.R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(ADDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=AddWithFlags(Context.R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SUBI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=SubWithFlags(Context.R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ADCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=AdcWithFlags(Context.R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SBCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=SbcWithFlags(Context.R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ANDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=Context.R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=Context.R[Src]|Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(XORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Context.R[Dst]=Context.R[Src]^Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(CMPI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	SubWithFlags(Context.R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(TSTI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst;

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(Context.ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Dst=Context.R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Dst)
		SETFLAG(FLAG_Z);
	if(Dst&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ADD)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=AddWithFlags(Context.R[Src1],Context.R[Src2]);
}

INST(SUB)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=SubWithFlags(Context.R[Src1],Context.R[Src2]);
}

INST(ADC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=AdcWithFlags(Context.R[Src1],Context.R[Src2]);
}

INST(SBC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=SbcWithFlags(Context.R[Src1],Context.R[Src2]);
}

INST(AND)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=Context.R[Src1]&Context.R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(OR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=Context.R[Src1]|Context.R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(XOR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=Context.R[Src1]^Context.R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(CMP)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);

	SubWithFlags(Context.R[Src1],Context.R[Src2]);
}

INST(TST)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst;

	Dst=Context.R[Src1]&Context.R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!Dst)
		SETFLAG(FLAG_Z);
	if(Dst&0x80000000)
		SETFLAG(FLAG_S);
}

INST(MULS)
{
	UINT32 Src2=EXTRACT(Opcode,6,8);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	Context.R[Dst]=MulWithFlags(Context.R[Src1],Context.R[Src2]);

	CLRFLAG(FLAG_E);
}

INST(NEG)
{
	UINT32 Dst=EXTRACT(Opcode,9,11);
	UINT32 Src=EXTRACT(Opcode,3,5);

	Context.R[Dst]=NegWithFlags(Context.R[Src]);
}

INST(CALL)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;
	PushVal(Context.PC+2);
	Context.PC=Context.PC+Offset;
	change_pc(Context.PC+2);

	CLRFLAG(FLAG_E);
}

INST(JV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_V))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);

}

INST(JNV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_V))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_C))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JNC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_C))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_S))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JM)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_S))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JNZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_Z))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JGE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(S^V))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JLE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || (S^V))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}
	CLRFLAG(FLAG_E);
}

INST(JHI)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C)))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JLS)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JGT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || (S^V)))
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}

INST(JLT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(S^V)
	{
		Context.PC=Context.PC+Offset;
		change_pc(Context.PC+2);
	}

	CLRFLAG(FLAG_E);
}



INST(JMP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(Context.ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);

	Offset<<=1;

	Context.PC=Context.PC+Offset;
	change_pc(Context.PC+2);

	CLRFLAG(FLAG_E);
}

INST(JR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);

	Context.PC=Context.R[Src]-2;
	change_pc(Context.PC+2);

	CLRFLAG(FLAG_E);
}

INST(CALLR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);
	PushVal(Context.PC+2);
	Context.PC=Context.R[Src]-2;
	change_pc(Context.PC+2);

	CLRFLAG(FLAG_E);
}

INST(ASR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		Context.R[Dst]=AsrWithFlags(Context.R[Dst],Context.R[Cnt]&0x1f);
	else
		Context.R[Dst]=AsrWithFlags(Context.R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(LSR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		Context.R[Dst]=LsrWithFlags(Context.R[Dst],Context.R[Cnt]&0x1f);
	else
		Context.R[Dst]=LsrWithFlags(Context.R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(ASL)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		Context.R[Dst]=AslWithFlags(Context.R[Dst],Context.R[Cnt]&0x1f);
	else
		Context.R[Dst]=AslWithFlags(Context.R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(EXTB)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);
	UINT32 Val=Context.R[Dst];

	Context.R[Dst]=SEX8(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(EXTS)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);
	UINT32 Val=Context.R[Dst];

	Context.R[Dst]=SEX16(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Context.R[Dst])
		SETFLAG(FLAG_Z);
	if(Context.R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(SET)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	Context.SR|=(1<<Imm);
}

INST(CLR)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	Context.SR&=~(1<<Imm);
}

INST(SWI)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	if(!TESTFLAG(FLAG_ENI))
		return;
	PushVal(Context.PC);
	PushVal(Context.SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	Context.PC=SE3208_Read32(4*Imm+0x40)-2;
	change_pc(Context.PC+2);
}

INST(HALT)
{
//  UINT32 Imm=EXTRACT(Opcode,0,3);

//  DEBUGMESSAGE("HALT\t0x%x",Imm);
}

INST(MVTC)
{
//  UINT32 Imm=EXTRACT(Opcode,0,3);

//  DEBUGMESSAGE("MVTC\t%%R0,%%CR%d",Imm);
}

INST(MVFC)
{
//  UINT32 Imm=EXTRACT(Opcode,0,3);

//  DEBUGMESSAGE("MVFC\t%%CR0%d,%%R0",Imm);
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
			break;
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
					case 8:	//arith
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
				case 1:		//Jumps
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
					else	//SP Ops
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
							break;
					}
					break;
			}
			break;

	}
	return INVALIDOP;
}


static void BuildTable(void)
{
	int i;
	if(!OpTable)
		OpTable=malloc_or_die(sizeof(_OP)*0x10000);
	for(i=0;i<0x10000;++i)
		OpTable[i]=DecodeOp(i);
}

static void SE3208_Reset(void)
{
	int (*save_irqcallback)(int) = Context.irq_callback;
	memset(&Context,0,sizeof(_SE3208Context));
	Context.irq_callback = save_irqcallback;
	Context.PC=SE3208_Read32(0);
	Context.SR=0;
	Context.IRQ=CLEAR_LINE;
	Context.NMI=CLEAR_LINE;
	change_pc(Context.PC+2);
}

static void SE3208_NMI(void)
{
	PushVal(Context.PC);
	PushVal(Context.SR);

	CLRFLAG(FLAG_NMI|FLAG_ENI|FLAG_E|FLAG_M);

	Context.PC=SE3208_Read32(4);
	change_pc(Context.PC+2);
}

static void SE3208_Interrupt(void)
{
	if(!TESTFLAG(FLAG_ENI))
		return;

	PushVal(Context.PC);
	PushVal(Context.SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);


	if(!(TESTFLAG(FLAG_AUT)))
		Context.PC=SE3208_Read32(8);
	else
		Context.PC=SE3208_Read32(4*Context.irq_callback(0));
	change_pc(Context.PC+2);
}


static int SE3208_Run(int cycles)
{
	SE3208_ICount=cycles;
	do
	{
		UINT16 Opcode=cpu_readop16(WORD_XOR_LE(Context.PC));

		CALL_MAME_DEBUG;

		OpTable[Opcode](Opcode);
		Context.PPC=Context.PC;
		Context.PC+=2;
		//Check interrupts
		if(Context.NMI==ASSERT_LINE)
		{
			SE3208_NMI();
			Context.NMI=CLEAR_LINE;
		}
		else if(Context.IRQ==ASSERT_LINE && TESTFLAG(FLAG_ENI))
		{
			SE3208_Interrupt();
		}
		--SE3208_ICount;
	} while(SE3208_ICount>0);

	return cycles-SE3208_ICount;
}

static void SE3208_Init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	BuildTable();

	Context.irq_callback = irqcallback;
}

static void SE3208_Exit(void)
{
	if(OpTable)
		free(OpTable);
}


static void SE3208_get_context(void *dst)
{
	if(dst)
		memcpy(dst,&Context,sizeof(Context));
}

static void SE3208_set_context(void *src)
{
	if(src)
		memcpy(&Context,src,sizeof(Context));
}

static void set_irq_line(int line,int state)
{
	if(line==INPUT_LINE_NMI)	//NMI
		Context.NMI=state;
	else
		Context.IRQ=state;
}


static void SE3208_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SE3208_INT:		set_irq_line(0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_REGISTER + SE3208_PC:
		case CPUINFO_INT_PC:							Context.PC = info->i;					break;
		case CPUINFO_INT_REGISTER + SE3208_SP:
		case CPUINFO_INT_SP:							Context.SP = info->i;    				break;
		case CPUINFO_INT_REGISTER + SE3208_ER:   		Context.ER = info->i;	   				break;
		case CPUINFO_INT_REGISTER + SE3208_SR:			Context.SR = info->i;				    break;
		case CPUINFO_INT_REGISTER + SE3208_R0:			Context.R[ 0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R1:			Context.R[ 1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R2:			Context.R[ 2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R3:			Context.R[ 3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R4:			Context.R[ 4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R5:			Context.R[ 5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R6:			Context.R[ 6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R7:			Context.R[ 7] = info->i;				break;
	}
}


void SE3208_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(Context);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SE3208_INT:		info->i = Context.IRQ;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = Context.NMI;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = Context.PPC;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SE3208_PC:			info->i = Context.PC;					break;
		case CPUINFO_INT_REGISTER + SE3208_SP:
		case CPUINFO_INT_SP:   							info->i = Context.SP;					break;
		case CPUINFO_INT_REGISTER + SE3208_SR:			info->i = Context.SR;					break;
		case CPUINFO_INT_REGISTER + SE3208_ER:			info->i = Context.ER;					break;
		case CPUINFO_INT_REGISTER + SE3208_R0:			info->i = Context.R[ 0];				break;
		case CPUINFO_INT_REGISTER + SE3208_R1:			info->i = Context.R[ 1];				break;
		case CPUINFO_INT_REGISTER + SE3208_R2:			info->i = Context.R[ 2];				break;
		case CPUINFO_INT_REGISTER + SE3208_R3:			info->i = Context.R[ 3];				break;
		case CPUINFO_INT_REGISTER + SE3208_R4:			info->i = Context.R[ 4];				break;
		case CPUINFO_INT_REGISTER + SE3208_R5:			info->i = Context.R[ 5];				break;
		case CPUINFO_INT_REGISTER + SE3208_R6:			info->i = Context.R[ 6];				break;
		case CPUINFO_INT_REGISTER + SE3208_R7:			info->i = Context.R[ 7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = SE3208_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = SE3208_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = SE3208_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = SE3208_Init;				break;
		case CPUINFO_PTR_RESET:							info->reset = SE3208_Reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = SE3208_Exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = SE3208_Run;				break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = SE3208_Dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &SE3208_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "SE3208");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Advanced Digital Chips Inc."); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.00");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 2005 ElSemi, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c %c%c%c%c%c",
					Context.SR&FLAG_C?'C':'.',
					Context.SR&FLAG_V?'V':'.',
					Context.SR&FLAG_S?'S':'.',
					Context.SR&FLAG_Z?'Z':'.',

					Context.SR&FLAG_M?'M':'.',
					Context.SR&FLAG_E?'E':'.',
					Context.SR&FLAG_AUT?'A':'.',
					Context.SR&FLAG_ENI?'I':'.',
					Context.SR&FLAG_NMI?'N':'.'

					);

			break;

		case CPUINFO_STR_REGISTER + SE3208_PC:				sprintf(info->s, "PC  :%08X", Context.PC); break;
		case CPUINFO_STR_REGISTER + SE3208_SR:				sprintf(info->s, "SR  :%08X", Context.SR); break;
		case CPUINFO_STR_REGISTER + SE3208_ER:				sprintf(info->s, "ER  :%08X", Context.ER); break;
		case CPUINFO_STR_REGISTER + SE3208_SP: 				sprintf(info->s, "SP  :%08X", Context.SP); break;
		case CPUINFO_STR_REGISTER + SE3208_R0:				sprintf(info->s, "R0  :%08X", Context.R[ 0]); break;
		case CPUINFO_STR_REGISTER + SE3208_R1:				sprintf(info->s, "R1  :%08X", Context.R[ 1]); break;
		case CPUINFO_STR_REGISTER + SE3208_R2:				sprintf(info->s, "R2  :%08X", Context.R[ 2]); break;
		case CPUINFO_STR_REGISTER + SE3208_R3:				sprintf(info->s, "R3  :%08X", Context.R[ 3]); break;
		case CPUINFO_STR_REGISTER + SE3208_R4:				sprintf(info->s, "R4  :%08X", Context.R[ 4]); break;
		case CPUINFO_STR_REGISTER + SE3208_R5:				sprintf(info->s, "R5  :%08X", Context.R[ 5]); break;
		case CPUINFO_STR_REGISTER + SE3208_R6:				sprintf(info->s, "R6  :%08X", Context.R[ 6]); break;
		case CPUINFO_STR_REGISTER + SE3208_R7:				sprintf(info->s, "R7  :%08X", Context.R[ 7]); break;
		case CPUINFO_STR_REGISTER + SE3208_PPC:				sprintf(info->s, "PPC  :%08X", Context.PPC); break;
	}
}

