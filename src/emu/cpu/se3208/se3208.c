#include "emu.h"
#include "debugger.h"
#include "se3208.h"

/*
    SE3208 CPU Emulator by ElSemi

    For information about this CPU:
    www.adc.co.kr

*/

struct se3208_state_t
{
	//GPR
	UINT32 R[8];
	//SPR
	UINT32 PC;
	UINT32 SR;
	UINT32 SP;
	UINT32 ER;
	UINT32 PPC;

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	UINT8 IRQ;
	UINT8 NMI;

	int icount;
};

#define FLAG_C		0x0080
#define FLAG_V		0x0010
#define FLAG_S		0x0020
#define FLAG_Z		0x0040

#define FLAG_M		0x0200
#define FLAG_E		0x0800
#define FLAG_AUT	0x1000
#define FLAG_ENI	0x2000
#define FLAG_NMI	0x4000

#define CLRFLAG(f)	se3208_state->SR&=~(f);
#define SETFLAG(f)	se3208_state->SR|=(f);
#define TESTFLAG(f)	(se3208_state->SR&(f))

#define EXTRACT(val,sbit,ebit)	(((val)>>sbit)&((1<<((ebit-sbit)+1))-1))
#define SEX8(val)	((val&0x80)?(val|0xFFFFFF00):(val&0xFF))
#define SEX16(val)	((val&0x8000)?(val|0xFFFF0000):(val&0xFFFF))
#define ZEX8(val)	((val)&0xFF)
#define ZEX16(val)	((val)&0xFFFF)
#define SEX(bits,val)	((val)&(1<<(bits-1))?((val)|(~((1<<bits)-1))):(val&((1<<bits)-1)))

//Precompute the instruction decoding in a big table
typedef void (*_OP)(se3208_state_t *se3208_state, UINT16 Opcode);
#define INST(a) static void a(se3208_state_t *se3208_state, UINT16 Opcode)
static _OP *OpTable=NULL;

INLINE se3208_state_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SE3208);
	return (se3208_state_t *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE UINT32 read_dword_unaligned(address_space &space, UINT32 address)
{
	if (address & 3)
		return space.read_byte(address) | space.read_byte(address+1)<<8 | space.read_byte(address+2)<<16 | space.read_byte(address+3)<<24;
	else
		return space.read_dword(address);
}

INLINE UINT16 read_word_unaligned(address_space &space, UINT32 address)
{
	if (address & 1)
		return space.read_byte(address) | space.read_byte(address+1)<<8;
	else
		return space.read_word(address);
}

INLINE void write_dword_unaligned(address_space &space, UINT32 address, UINT32 data)
{
	if (address & 3)
	{
		space.write_byte(address, data & 0xff);
		space.write_byte(address+1, (data>>8)&0xff);
		space.write_byte(address+2, (data>>16)&0xff);
		space.write_byte(address+3, (data>>24)&0xff);
	}
	else
	{
		space.write_dword(address, data);
	}
}

INLINE void write_word_unaligned(address_space &space, UINT32 address, UINT16 data)
{
	if (address & 1)
	{
		space.write_byte(address, data & 0xff);
		space.write_byte(address+1, (data>>8)&0xff);
	}
	else
	{
		space.write_word(address, data);
	}
}


INLINE UINT8 SE3208_Read8(se3208_state_t *se3208_state, UINT32 addr)
{
	return se3208_state->program->read_byte(addr);
}

INLINE UINT16 SE3208_Read16(se3208_state_t *se3208_state, UINT32 addr)
{
	return read_word_unaligned(*se3208_state->program,addr);
}

INLINE UINT32 SE3208_Read32(se3208_state_t *se3208_state, UINT32 addr)
{
	return read_dword_unaligned(*se3208_state->program,addr);
}

INLINE void SE3208_Write8(se3208_state_t *se3208_state, UINT32 addr,UINT8 val)
{
	se3208_state->program->write_byte(addr,val);
}

INLINE void SE3208_Write16(se3208_state_t *se3208_state, UINT32 addr,UINT16 val)
{
	write_word_unaligned(*se3208_state->program,addr,val);
}

INLINE void SE3208_Write32(se3208_state_t *se3208_state, UINT32 addr,UINT32 val)
{
	write_dword_unaligned(*se3208_state->program,addr,val);
}



INLINE UINT32 AddWithFlags(se3208_state_t *se3208_state, UINT32 a,UINT32 b)
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

INLINE UINT32 SubWithFlags(se3208_state_t *se3208_state, UINT32 a,UINT32 b)	//a-b
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

INLINE UINT32 AdcWithFlags(se3208_state_t *se3208_state,UINT32 a,UINT32 b)
{
	UINT32 C=(se3208_state->SR&FLAG_C)?1:0;
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

INLINE UINT32 SbcWithFlags(se3208_state_t *se3208_state,UINT32 a,UINT32 b)
{
	UINT32 C=(se3208_state->SR&FLAG_C)?1:0;
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

INLINE UINT32 MulWithFlags(se3208_state_t *se3208_state,UINT32 a,UINT32 b)
{
	INT64 r=(INT64) a*(INT64) b;
	CLRFLAG(FLAG_V);
	if(r>>32)
		SETFLAG(FLAG_V);
	return (UINT32) (r&0xffffffff);
}

INLINE UINT32 NegWithFlags(se3208_state_t *se3208_state,UINT32 a)
{
	return SubWithFlags(se3208_state,0,a);
}

INLINE UINT32 AsrWithFlags(se3208_state_t *se3208_state,UINT32 Val, UINT8 By)
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

INLINE UINT32 LsrWithFlags(se3208_state_t *se3208_state,UINT32 Val, UINT8 By)
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

INLINE UINT32 AslWithFlags(se3208_state_t *se3208_state,UINT32 Val, UINT8 By)
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
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STB)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(se3208_state, Index+Offset,ZEX8(se3208_state->R[SrcDst]));

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
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STS)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(se3208_state, Index+Offset,ZEX16(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LD)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	se3208_state->R[SrcDst]=SE3208_Read32(se3208_state, Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(ST)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(se3208_state, Index+Offset,se3208_state->R[SrcDst]);

	CLRFLAG(FLAG_E);
}

INST(LDBU)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);
	UINT32 Val;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX8(Val);

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
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}


INST(LERI)
{
	UINT32 Imm=EXTRACT(Opcode,0,13);
	if(TESTFLAG(FLAG_E))
		se3208_state->ER=(EXTRACT(se3208_state->ER,0,17)<<14)|Imm;
	else
		se3208_state->ER=SEX(14,Imm);


	SETFLAG(FLAG_E);
}

INST(LDSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	se3208_state->R[SrcDst]=SE3208_Read32(se3208_state, Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(STSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(se3208_state, Index+Offset,se3208_state->R[SrcDst]);

	CLRFLAG(FLAG_E);
}

static void PushVal(se3208_state_t *se3208_state, UINT32 Val)
{
	se3208_state->SP-=4;
	SE3208_Write32(se3208_state, se3208_state->SP,Val);
}

static UINT32 PopVal(se3208_state_t *se3208_state)
{
	UINT32 Val=SE3208_Read32(se3208_state, se3208_state->SP);
	se3208_state->SP+=4;
	return Val;
}

INST(PUSH)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<10))
		PushVal(se3208_state,se3208_state->PC);
	if(Set&(1<<9))
		PushVal(se3208_state,se3208_state->SR);
	if(Set&(1<<8))
		PushVal(se3208_state,se3208_state->ER);
	if(Set&(1<<7))
		PushVal(se3208_state,se3208_state->R[7]);
	if(Set&(1<<6))
		PushVal(se3208_state,se3208_state->R[6]);
	if(Set&(1<<5))
		PushVal(se3208_state,se3208_state->R[5]);
	if(Set&(1<<4))
		PushVal(se3208_state,se3208_state->R[4]);
	if(Set&(1<<3))
		PushVal(se3208_state,se3208_state->R[3]);
	if(Set&(1<<2))
		PushVal(se3208_state,se3208_state->R[2]);
	if(Set&(1<<1))
		PushVal(se3208_state,se3208_state->R[1]);
	if(Set&(1<<0))
		PushVal(se3208_state,se3208_state->R[0]);
}

INST(POP)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<0))
		se3208_state->R[0]=PopVal(se3208_state);
	if(Set&(1<<1))
		se3208_state->R[1]=PopVal(se3208_state);
	if(Set&(1<<2))
		se3208_state->R[2]=PopVal(se3208_state);
	if(Set&(1<<3))
		se3208_state->R[3]=PopVal(se3208_state);
	if(Set&(1<<4))
		se3208_state->R[4]=PopVal(se3208_state);
	if(Set&(1<<5))
		se3208_state->R[5]=PopVal(se3208_state);
	if(Set&(1<<6))
		se3208_state->R[6]=PopVal(se3208_state);
	if(Set&(1<<7))
		se3208_state->R[7]=PopVal(se3208_state);
	if(Set&(1<<8))
		se3208_state->ER=PopVal(se3208_state);
	if(Set&(1<<9))
		se3208_state->SR=PopVal(se3208_state);
	if(Set&(1<<10))
	{
		se3208_state->PC=PopVal(se3208_state)-2;		//PC automatically incresases by 2
	}
}

INST(LEATOSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	se3208_state->SP=Index+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEAFROMSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	se3208_state->R[Index]=se3208_state->SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEASPTOSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,23)<<8)|(Offset&0xff);
	else
		Offset=SEX(10,Offset);

	se3208_state->SP=se3208_state->SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(MOV)
{
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,9,11);

	se3208_state->R[Dst]=se3208_state->R[Src];
}

INST(LDI)
{
	UINT32 Dst=EXTRACT(Opcode,8,10);
	UINT32 Imm=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX8(Imm);

	se3208_state->R[Dst]=Imm;

	CLRFLAG(FLAG_E);
}

INST(LDBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(se3208_state, Index+Offset,ZEX8(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(se3208_state, Index+Offset,ZEX16(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDBUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(LDSUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=se3208_state->SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(ADDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=AddWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SUBI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=SubWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ADCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=AdcWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SBCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=SbcWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ANDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=se3208_state->R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=se3208_state->R[Src]|Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(XORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=se3208_state->R[Src]^Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(CMPI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	SubWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(TSTI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst;

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Dst=se3208_state->R[Src]&Imm;

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

	se3208_state->R[Dst]=AddWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(SUB)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=SubWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(ADC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=AdcWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(SBC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=SbcWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(AND)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=se3208_state->R[Src1]&se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(OR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=se3208_state->R[Src1]|se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(XOR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=se3208_state->R[Src1]^se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(CMP)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);

	SubWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(TST)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst;

	Dst=se3208_state->R[Src1]&se3208_state->R[Src2];

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

	se3208_state->R[Dst]=MulWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);

	CLRFLAG(FLAG_E);
}

INST(NEG)
{
	UINT32 Dst=EXTRACT(Opcode,9,11);
	UINT32 Src=EXTRACT(Opcode,3,5);

	se3208_state->R[Dst]=NegWithFlags(se3208_state,se3208_state->R[Src]);
}

INST(CALL)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;
	PushVal(se3208_state,se3208_state->PC+2);
	se3208_state->PC=se3208_state->PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);

}

INST(JNV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_C))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_C))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_S))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JM)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_S))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_Z))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(S^V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || (S^V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}
	CLRFLAG(FLAG_E);
}

INST(JHI)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C)))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLS)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || (S^V)))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(S^V)
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}



INST(JMP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);

	Offset<<=1;

	se3208_state->PC=se3208_state->PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);

	se3208_state->PC=se3208_state->R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(CALLR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);
	PushVal(se3208_state,se3208_state->PC+2);
	se3208_state->PC=se3208_state->R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(ASR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		se3208_state->R[Dst]=AsrWithFlags(se3208_state,se3208_state->R[Dst],se3208_state->R[Cnt]&0x1f);
	else
		se3208_state->R[Dst]=AsrWithFlags(se3208_state,se3208_state->R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(LSR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		se3208_state->R[Dst]=LsrWithFlags(se3208_state,se3208_state->R[Dst],se3208_state->R[Cnt]&0x1f);
	else
		se3208_state->R[Dst]=LsrWithFlags(se3208_state,se3208_state->R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(ASL)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		se3208_state->R[Dst]=AslWithFlags(se3208_state,se3208_state->R[Dst],se3208_state->R[Cnt]&0x1f);
	else
		se3208_state->R[Dst]=AslWithFlags(se3208_state,se3208_state->R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(EXTB)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);
	UINT32 Val=se3208_state->R[Dst];

	se3208_state->R[Dst]=SEX8(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(EXTS)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);
	UINT32 Val=se3208_state->R[Dst];

	se3208_state->R[Dst]=SEX16(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(SET)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	se3208_state->SR|=(1<<Imm);
}

INST(CLR)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	se3208_state->SR&=~(1<<Imm);
}

INST(SWI)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	if(!TESTFLAG(FLAG_ENI))
		return;
	PushVal(se3208_state,se3208_state->PC);
	PushVal(se3208_state,se3208_state->SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	se3208_state->PC=SE3208_Read32(se3208_state, 4*Imm+0x40)-2;
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
		OpTable=global_alloc_array(_OP, 0x10000);
	for(i=0;i<0x10000;++i)
		OpTable[i]=DecodeOp(i);
}

static CPU_RESET( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	device_irq_acknowledge_callback save_irqcallback = se3208_state->irq_callback;
	memset(se3208_state,0,sizeof(se3208_state_t));
	se3208_state->irq_callback = save_irqcallback;
	se3208_state->device = device;
	se3208_state->program = device->space(AS_PROGRAM);
	se3208_state->direct = &se3208_state->program->direct();
	se3208_state->PC=SE3208_Read32(se3208_state, 0);
	se3208_state->SR=0;
	se3208_state->IRQ=CLEAR_LINE;
	se3208_state->NMI=CLEAR_LINE;
}

static void SE3208_NMI(se3208_state_t *se3208_state)
{
	PushVal(se3208_state,se3208_state->PC);
	PushVal(se3208_state,se3208_state->SR);

	CLRFLAG(FLAG_NMI|FLAG_ENI|FLAG_E|FLAG_M);

	se3208_state->PC=SE3208_Read32(se3208_state, 4);
}

static void SE3208_Interrupt(se3208_state_t *se3208_state)
{
	if(!TESTFLAG(FLAG_ENI))
		return;

	PushVal(se3208_state,se3208_state->PC);
	PushVal(se3208_state,se3208_state->SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);


	if(!(TESTFLAG(FLAG_AUT)))
		se3208_state->PC=SE3208_Read32(se3208_state, 8);
	else
		se3208_state->PC=SE3208_Read32(se3208_state, 4*se3208_state->irq_callback(se3208_state->device, 0));
}


static CPU_EXECUTE( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	do
	{
		UINT16 Opcode=se3208_state->direct->read_decrypted_word(se3208_state->PC, WORD_XOR_LE(0));

		debugger_instruction_hook(device, se3208_state->PC);

		OpTable[Opcode](se3208_state, Opcode);
		se3208_state->PPC=se3208_state->PC;
		se3208_state->PC+=2;
		//Check interrupts
		if(se3208_state->NMI==ASSERT_LINE)
		{
			SE3208_NMI(se3208_state);
			se3208_state->NMI=CLEAR_LINE;
		}
		else if(se3208_state->IRQ==ASSERT_LINE && TESTFLAG(FLAG_ENI))
		{
			SE3208_Interrupt(se3208_state);
		}
		--(se3208_state->icount);
	} while(se3208_state->icount>0);
}

static CPU_INIT( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	BuildTable();

	se3208_state->irq_callback = irqcallback;
	se3208_state->device = device;
	se3208_state->program = device->space(AS_PROGRAM);
	se3208_state->direct = &se3208_state->program->direct();
}

static CPU_EXIT( se3208 )
{
	if(OpTable) {
		global_free(OpTable);
		OpTable = NULL;
	}
}


static void set_irq_line(se3208_state_t *se3208_state, int line,int state)
{
	if(line==INPUT_LINE_NMI)	//NMI
		se3208_state->NMI=state;
	else
		se3208_state->IRQ=state;
}


static CPU_SET_INFO( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SE3208_INT:		set_irq_line(se3208_state, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(se3208_state, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_REGISTER + SE3208_PC:
		case CPUINFO_INT_PC:							se3208_state->PC = info->i;					break;
		case CPUINFO_INT_REGISTER + SE3208_SP:
		case CPUINFO_INT_SP:							se3208_state->SP = info->i; 				break;
		case CPUINFO_INT_REGISTER + SE3208_ER:  		se3208_state->ER = info->i;					break;
		case CPUINFO_INT_REGISTER + SE3208_SR:			se3208_state->SR = info->i;				    break;
		case CPUINFO_INT_REGISTER + SE3208_R0:			se3208_state->R[ 0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R1:			se3208_state->R[ 1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R2:			se3208_state->R[ 2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R3:			se3208_state->R[ 3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R4:			se3208_state->R[ 4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R5:			se3208_state->R[ 5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R6:			se3208_state->R[ 6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R7:			se3208_state->R[ 7] = info->i;				break;
	}
}


CPU_GET_INFO( se3208 )
{
	se3208_state_t *se3208_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(se3208_state_t);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SE3208_INT:		info->i = se3208_state->IRQ;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = se3208_state->NMI;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = se3208_state->PPC;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SE3208_PC:			info->i = se3208_state->PC;					break;
		case CPUINFO_INT_REGISTER + SE3208_SP:
		case CPUINFO_INT_SP:							info->i = se3208_state->SP;					break;
		case CPUINFO_INT_REGISTER + SE3208_SR:			info->i = se3208_state->SR;					break;
		case CPUINFO_INT_REGISTER + SE3208_ER:			info->i = se3208_state->ER;					break;
		case CPUINFO_INT_REGISTER + SE3208_R0:			info->i = se3208_state->R[ 0];				break;
		case CPUINFO_INT_REGISTER + SE3208_R1:			info->i = se3208_state->R[ 1];				break;
		case CPUINFO_INT_REGISTER + SE3208_R2:			info->i = se3208_state->R[ 2];				break;
		case CPUINFO_INT_REGISTER + SE3208_R3:			info->i = se3208_state->R[ 3];				break;
		case CPUINFO_INT_REGISTER + SE3208_R4:			info->i = se3208_state->R[ 4];				break;
		case CPUINFO_INT_REGISTER + SE3208_R5:			info->i = se3208_state->R[ 5];				break;
		case CPUINFO_INT_REGISTER + SE3208_R6:			info->i = se3208_state->R[ 6];				break;
		case CPUINFO_INT_REGISTER + SE3208_R7:			info->i = se3208_state->R[ 7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(se3208);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(se3208);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(se3208);	break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(se3208);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(se3208);break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(se3208);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &se3208_state->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "SE3208");				break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Advanced Digital Chips Inc."); break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.00");				break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Miguel Angel Horna, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c %c%c%c%c%c",
					se3208_state->SR&FLAG_C?'C':'.',
					se3208_state->SR&FLAG_V?'V':'.',
					se3208_state->SR&FLAG_S?'S':'.',
					se3208_state->SR&FLAG_Z?'Z':'.',

					se3208_state->SR&FLAG_M?'M':'.',
					se3208_state->SR&FLAG_E?'E':'.',
					se3208_state->SR&FLAG_AUT?'A':'.',
					se3208_state->SR&FLAG_ENI?'I':'.',
					se3208_state->SR&FLAG_NMI?'N':'.'

					);

			break;

		case CPUINFO_STR_REGISTER + SE3208_PC:				sprintf(info->s, "PC  :%08X", se3208_state->PC); break;
		case CPUINFO_STR_REGISTER + SE3208_SR:				sprintf(info->s, "SR  :%08X", se3208_state->SR); break;
		case CPUINFO_STR_REGISTER + SE3208_ER:				sprintf(info->s, "ER  :%08X", se3208_state->ER); break;
		case CPUINFO_STR_REGISTER + SE3208_SP:				sprintf(info->s, "SP  :%08X", se3208_state->SP); break;
		case CPUINFO_STR_REGISTER + SE3208_R0:				sprintf(info->s, "R0  :%08X", se3208_state->R[ 0]); break;
		case CPUINFO_STR_REGISTER + SE3208_R1:				sprintf(info->s, "R1  :%08X", se3208_state->R[ 1]); break;
		case CPUINFO_STR_REGISTER + SE3208_R2:				sprintf(info->s, "R2  :%08X", se3208_state->R[ 2]); break;
		case CPUINFO_STR_REGISTER + SE3208_R3:				sprintf(info->s, "R3  :%08X", se3208_state->R[ 3]); break;
		case CPUINFO_STR_REGISTER + SE3208_R4:				sprintf(info->s, "R4  :%08X", se3208_state->R[ 4]); break;
		case CPUINFO_STR_REGISTER + SE3208_R5:				sprintf(info->s, "R5  :%08X", se3208_state->R[ 5]); break;
		case CPUINFO_STR_REGISTER + SE3208_R6:				sprintf(info->s, "R6  :%08X", se3208_state->R[ 6]); break;
		case CPUINFO_STR_REGISTER + SE3208_R7:				sprintf(info->s, "R7  :%08X", se3208_state->R[ 7]); break;
		case CPUINFO_STR_REGISTER + SE3208_PPC:				sprintf(info->s, "PPC  :%08X", se3208_state->PPC); break;
	}
}


DEFINE_LEGACY_CPU_DEVICE(SE3208, se3208);
