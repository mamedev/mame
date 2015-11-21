// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "debugger.h"
#include "se3208.h"

/*
    SE3208 CPU Emulator by ElSemi

    For information about this CPU:
    www.adc.co.kr

*/


#define FLAG_C      0x0080
#define FLAG_V      0x0010
#define FLAG_S      0x0020
#define FLAG_Z      0x0040

#define FLAG_M      0x0200
#define FLAG_E      0x0800
#define FLAG_AUT    0x1000
#define FLAG_ENI    0x2000
#define FLAG_NMI    0x4000

#define CLRFLAG(f)  m_SR&=~(f);
#define SETFLAG(f)  m_SR|=(f);
#define TESTFLAG(f) (m_SR&(f))

#define EXTRACT(val,sbit,ebit)  (((val)>>sbit)&((1<<((ebit-sbit)+1))-1))
#define SEX8(val)   ((val&0x80)?(val|0xFFFFFF00):(val&0xFF))
#define SEX16(val)  ((val&0x8000)?(val|0xFFFF0000):(val&0xFFFF))
#define ZEX8(val)   ((val)&0xFF)
#define ZEX16(val)  ((val)&0xFFFF)
#define SEX(bits,val)   ((val)&(1<<(bits-1))?((val)|(~((1<<bits)-1))):(val&((1<<bits)-1)))

//Precompute the instruction decoding in a big table
#define INST(a) void se3208_device::a(UINT16 Opcode)


const device_type SE3208 = &device_creator<se3208_device>;


se3208_device::se3208_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SE3208, "SE3208", tag, owner, clock, "se3208", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0), m_PC(0), m_SR(0), m_SP(0), m_ER(0), m_PPC(0), m_program(nullptr), m_direct(nullptr), m_IRQ(0), m_NMI(0), m_icount(0)
{
}


UINT32 se3208_device::read_dword_unaligned(address_space &space, UINT32 address)
{
	if (address & 3)
		return space.read_byte(address) | space.read_byte(address+1)<<8 | space.read_byte(address+2)<<16 | space.read_byte(address+3)<<24;
	else
		return space.read_dword(address);
}

UINT16 se3208_device::read_word_unaligned(address_space &space, UINT32 address)
{
	if (address & 1)
		return space.read_byte(address) | space.read_byte(address+1)<<8;
	else
		return space.read_word(address);
}

void se3208_device::write_dword_unaligned(address_space &space, UINT32 address, UINT32 data)
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

void se3208_device::write_word_unaligned(address_space &space, UINT32 address, UINT16 data)
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


UINT8 se3208_device::SE3208_Read8(UINT32 addr)
{
	return m_program->read_byte(addr);
}

UINT16 se3208_device::SE3208_Read16(UINT32 addr)
{
	return read_word_unaligned(*m_program,addr);
}

UINT32 se3208_device::SE3208_Read32(UINT32 addr)
{
	return read_dword_unaligned(*m_program,addr);
}

void se3208_device::SE3208_Write8(UINT32 addr,UINT8 val)
{
	m_program->write_byte(addr,val);
}

void se3208_device::SE3208_Write16(UINT32 addr,UINT16 val)
{
	write_word_unaligned(*m_program,addr,val);
}

void se3208_device::SE3208_Write32(UINT32 addr,UINT32 val)
{
	write_dword_unaligned(*m_program,addr,val);
}



UINT32 se3208_device::AddWithFlags(UINT32 a,UINT32 b)
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

UINT32 se3208_device::SubWithFlags(UINT32 a,UINT32 b) //a-b
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

UINT32 se3208_device::AdcWithFlags(UINT32 a,UINT32 b)
{
	UINT32 C=(m_SR&FLAG_C)?1:0;
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

UINT32 se3208_device::SbcWithFlags(UINT32 a,UINT32 b)
{
	UINT32 C=(m_SR&FLAG_C)?1:0;
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

UINT32 se3208_device::MulWithFlags(UINT32 a,UINT32 b)
{
	INT64 r=(INT64) a*(INT64) b;
	CLRFLAG(FLAG_V);
	if(r>>32)
		SETFLAG(FLAG_V);
	return (UINT32) (r&0xffffffff);
}

UINT32 se3208_device::NegWithFlags(UINT32 a)
{
	return SubWithFlags(0,a);
}

UINT32 se3208_device::AsrWithFlags(UINT32 Val, UINT8 By)
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

UINT32 se3208_device::LsrWithFlags(UINT32 Val, UINT8 By)
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

UINT32 se3208_device::AslWithFlags(UINT32 Val, UINT8 By)
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
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STB)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(Index+Offset,ZEX8(m_R[SrcDst]));

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
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STS)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(Index+Offset,ZEX16(m_R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LD)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	m_R[SrcDst]=SE3208_Read32(Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(ST)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(Index+Offset,m_R[SrcDst]);

	CLRFLAG(FLAG_E);
}

INST(LDBU)
{
	UINT32 Offset=EXTRACT(Opcode,0,4);
	UINT32 Index=EXTRACT(Opcode,5,7);
	UINT32 SrcDst=EXTRACT(Opcode,8,10);
	UINT32 Val;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=ZEX8(Val);

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
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}


INST(LERI)
{
	UINT32 Imm=EXTRACT(Opcode,0,13);
	if(TESTFLAG(FLAG_E))
		m_ER=(EXTRACT(m_ER,0,17)<<14)|Imm;
	else
		m_ER=SEX(14,Imm);


	SETFLAG(FLAG_E);
}

INST(LDSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	m_R[SrcDst]=SE3208_Read32(Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(STSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(Index+Offset,m_R[SrcDst]);

	CLRFLAG(FLAG_E);
}

void se3208_device::PushVal(UINT32 Val)
{
	m_SP-=4;
	SE3208_Write32(m_SP,Val);
}

UINT32 se3208_device::PopVal()
{
	UINT32 Val=SE3208_Read32(m_SP);
	m_SP+=4;
	return Val;
}

INST(PUSH)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<10))
		PushVal(m_PC);
	if(Set&(1<<9))
		PushVal(m_SR);
	if(Set&(1<<8))
		PushVal(m_ER);
	if(Set&(1<<7))
		PushVal(m_R[7]);
	if(Set&(1<<6))
		PushVal(m_R[6]);
	if(Set&(1<<5))
		PushVal(m_R[5]);
	if(Set&(1<<4))
		PushVal(m_R[4]);
	if(Set&(1<<3))
		PushVal(m_R[3]);
	if(Set&(1<<2))
		PushVal(m_R[2]);
	if(Set&(1<<1))
		PushVal(m_R[1]);
	if(Set&(1<<0))
		PushVal(m_R[0]);
}

INST(POP)
{
	UINT32 Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<0))
		m_R[0]=PopVal();
	if(Set&(1<<1))
		m_R[1]=PopVal();
	if(Set&(1<<2))
		m_R[2]=PopVal();
	if(Set&(1<<3))
		m_R[3]=PopVal();
	if(Set&(1<<4))
		m_R[4]=PopVal();
	if(Set&(1<<5))
		m_R[5]=PopVal();
	if(Set&(1<<6))
		m_R[6]=PopVal();
	if(Set&(1<<7))
		m_R[7]=PopVal();
	if(Set&(1<<8))
		m_ER=PopVal();
	if(Set&(1<<9))
		m_SR=PopVal();
	if(Set&(1<<10))
	{
		m_PC=PopVal()-2;        //PC automatically incresases by 2
	}
}

INST(LEATOSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	m_SP=(Index+Offset) & (~3);

	CLRFLAG(FLAG_E);
}

INST(LEAFROMSP)
{
	UINT32 Offset=EXTRACT(Opcode,9,12);
	UINT32 Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	m_R[Index]=m_SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEASPTOSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,23)<<8)|(Offset&0xff);
	else
		Offset=SEX(10,Offset);

	m_SP=(m_SP+Offset) & (~3);

	CLRFLAG(FLAG_E);
}

INST(MOV)
{
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,9,11);

	m_R[Dst]=m_R[Src];
}

INST(LDI)
{
	UINT32 Dst=EXTRACT(Opcode,8,10);
	UINT32 Imm=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX8(Imm);

	m_R[Dst]=Imm;

	CLRFLAG(FLAG_E);
}

INST(LDBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STBSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(Index+Offset,ZEX8(m_R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STSSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(Index+Offset,ZEX16(m_R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDBUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=ZEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(LDSUSP)
{
	UINT32 Offset=EXTRACT(Opcode,0,3);
	UINT32 Index=m_SP;
	UINT32 SrcDst=EXTRACT(Opcode,4,6);
	UINT32 Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(ADDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=AddWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SUBI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=SubWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ADCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=AdcWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SBCI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=SbcWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ANDI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=m_R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=m_R[Src]|Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(XORI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	m_R[Dst]=m_R[Src]^Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(CMPI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	SubWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(TSTI)
{
	UINT32 Imm=EXTRACT(Opcode,9,12);
	UINT32 Src=EXTRACT(Opcode,3,5);
	UINT32 Dst;

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(m_ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Dst=m_R[Src]&Imm;

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

	m_R[Dst]=AddWithFlags(m_R[Src1],m_R[Src2]);
}

INST(SUB)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	m_R[Dst]=SubWithFlags(m_R[Src1],m_R[Src2]);
}

INST(ADC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	m_R[Dst]=AdcWithFlags(m_R[Src1],m_R[Src2]);
}

INST(SBC)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	m_R[Dst]=SbcWithFlags(m_R[Src1],m_R[Src2]);
}

INST(AND)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	m_R[Dst]=m_R[Src1]&m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(OR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	m_R[Dst]=m_R[Src1]|m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(XOR)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst=EXTRACT(Opcode,0,2);

	m_R[Dst]=m_R[Src1]^m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(CMP)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);

	SubWithFlags(m_R[Src1],m_R[Src2]);
}

INST(TST)
{
	UINT32 Src2=EXTRACT(Opcode,9,11);
	UINT32 Src1=EXTRACT(Opcode,3,5);
	UINT32 Dst;

	Dst=m_R[Src1]&m_R[Src2];

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

	m_R[Dst]=MulWithFlags(m_R[Src1],m_R[Src2]);

	CLRFLAG(FLAG_E);
}

INST(NEG)
{
	UINT32 Dst=EXTRACT(Opcode,9,11);
	UINT32 Src=EXTRACT(Opcode,3,5);

	m_R[Dst]=NegWithFlags(m_R[Src]);
}

INST(CALL)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;
	PushVal(m_PC+2);
	m_PC=m_PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_V))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);

}

INST(JNV)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_V))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_C))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNC)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_C))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_S))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JM)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_S))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_Z))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JZ)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(S^V))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLE)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || (S^V))
	{
		m_PC=m_PC+Offset;
	}
	CLRFLAG(FLAG_E);
}

INST(JHI)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C)))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLS)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || (S^V)))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLT)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);
	UINT32 S=TESTFLAG(FLAG_S)?1:0;
	UINT32 V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(S^V)
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}



INST(JMP)
{
	UINT32 Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(m_ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);

	Offset<<=1;

	m_PC=m_PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);

	m_PC=m_R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(CALLR)
{
	UINT32 Src=EXTRACT(Opcode,0,3);
	PushVal(m_PC+2);
	m_PC=m_R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(ASR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		m_R[Dst]=AsrWithFlags(m_R[Dst],m_R[Cnt]&0x1f);
	else
		m_R[Dst]=AsrWithFlags(m_R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(LSR)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		m_R[Dst]=LsrWithFlags(m_R[Dst],m_R[Cnt]&0x1f);
	else
		m_R[Dst]=LsrWithFlags(m_R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(ASL)
{
	UINT32 CS=Opcode&(1<<10);
	UINT32 Dst=EXTRACT(Opcode,0,2);
	UINT32 Imm=EXTRACT(Opcode,5,9);
	UINT32 Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		m_R[Dst]=AslWithFlags(m_R[Dst],m_R[Cnt]&0x1f);
	else
		m_R[Dst]=AslWithFlags(m_R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(EXTB)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);
	UINT32 Val=m_R[Dst];

	m_R[Dst]=SEX8(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(EXTS)
{
	UINT32 Dst=EXTRACT(Opcode,0,3);
	UINT32 Val=m_R[Dst];

	m_R[Dst]=SEX16(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(SET)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	m_SR|=(1<<Imm);
}

INST(CLR)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	m_SR&=~(1<<Imm);
}

INST(SWI)
{
	UINT32 Imm=EXTRACT(Opcode,0,3);

	if(!TESTFLAG(FLAG_ENI))
		return;
	PushVal(m_PC);
	PushVal(m_SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	m_PC=SE3208_Read32(4*Imm+0x40)-2;
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


se3208_device::_OP se3208_device::DecodeOp(UINT16 Opcode)
{
	switch(EXTRACT(Opcode,14,15))
	{
		case 0x0:
			{
				UINT8 Op=EXTRACT(Opcode,11,13);
				switch(Op)
				{
					case 0x0:
						return &se3208_device::LDB;
					case 0x1:
						return &se3208_device::LDS;
					case 0x2:
						return &se3208_device::LD;
					case 0x3:
						return &se3208_device::LDBU;
					case 0x4:
						return &se3208_device::STB;
					case 0x5:
						return &se3208_device::STS;
					case 0x6:
						return &se3208_device::ST;
					case 0x7:
						return &se3208_device::LDSU;
				}
			}
			break;
		case 0x1:
			return &se3208_device::LERI;
		case 0x2:
			{
				switch(EXTRACT(Opcode,11,13))
				{
					case 0:
						return &se3208_device::LDSP;
					case 1:
						return &se3208_device::STSP;
					case 2:
						return &se3208_device::PUSH;
					case 3:
						return &se3208_device::POP;
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
								return &se3208_device::ADDI;
							case 1:
								return &se3208_device::ADCI;
							case 2:
								return &se3208_device::SUBI;
							case 3:
								return &se3208_device::SBCI;
							case 4:
								return &se3208_device::ANDI;
							case 5:
								return &se3208_device::ORI;
							case 6:
								return &se3208_device::XORI;
							case 7:
								switch(EXTRACT(Opcode,0,2))
								{
									case 0:
										return &se3208_device::CMPI;
									case 1:
										return &se3208_device::TSTI;
									case 2:
										return &se3208_device::LEATOSP;
									case 3:
										return &se3208_device::LEAFROMSP;
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
							return &se3208_device::ADD;
						case 1:
							return &se3208_device::ADC;
						case 2:
							return &se3208_device::SUB;
						case 3:
							return &se3208_device::SBC;
						case 4:
							return &se3208_device::AND;
						case 5:
							return &se3208_device::OR;
						case 6:
							return &se3208_device::XOR;
						case 7:
							switch(EXTRACT(Opcode,0,2))
							{
								case 0:
									return &se3208_device::CMP;
								case 1:
									return &se3208_device::TST;
								case 2:
									return &se3208_device::MOV;
								case 3:
									return &se3208_device::NEG;
							}
							break;
					}
					break;
				case 1:     //Jumps
					switch(EXTRACT(Opcode,8,11))
					{
						case 0x0:
							return &se3208_device::JNV;
						case 0x1:
							return &se3208_device::JV;
						case 0x2:
							return &se3208_device::JP;
						case 0x3:
							return &se3208_device::JM;
						case 0x4:
							return &se3208_device::JNZ;
						case 0x5:
							return &se3208_device::JZ;
						case 0x6:
							return &se3208_device::JNC;
						case 0x7:
							return &se3208_device::JC;
						case 0x8:
							return &se3208_device::JGT;
						case 0x9:
							return &se3208_device::JLT;
						case 0xa:
							return &se3208_device::JGE;
						case 0xb:
							return &se3208_device::JLE;
						case 0xc:
							return &se3208_device::JHI;
						case 0xd:
							return &se3208_device::JLS;
						case 0xe:
							return &se3208_device::JMP;
						case 0xf:
							return &se3208_device::CALL;
					}
					break;
				case 2:
					if(Opcode&(1<<11))
						return &se3208_device::LDI;
					else    //SP Ops
					{
						if(Opcode&(1<<10))
						{
							switch(EXTRACT(Opcode,7,9))
							{
								case 0:
									return &se3208_device::LDBSP;
								case 1:
									return &se3208_device::LDSSP;
								case 3:
									return &se3208_device::LDBUSP;
								case 4:
									return &se3208_device::STBSP;
								case 5:
									return &se3208_device::STSSP;
								case 7:
									return &se3208_device::LDSUSP;
							}
						}
						else
						{
							if(Opcode&(1<<9))
							{
								return &se3208_device::LEASPTOSP;
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
											return &se3208_device::EXTB;
										case 1:
											return &se3208_device::EXTS;
										case 8:
											return &se3208_device::JR;
										case 9:
											return &se3208_device::CALLR;
										case 10:
											return &se3208_device::SET;
										case 11:
											return &se3208_device::CLR;
										case 12:
											return &se3208_device::SWI;
										case 13:
											return &se3208_device::HALT;
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
									return &se3208_device::ASR;
								case 1:
									return &se3208_device::LSR;
								case 2:
									return &se3208_device::ASL;
								//case 3:
								//  return &se3208_device::LSL;
							}
							break;
						case 4:
							return &se3208_device::MULS;
						case 6:
							if(Opcode&(1<<3))
								return &se3208_device::MVFC;
							else
								return &se3208_device::MVTC;
					}
					break;
			}
			break;

	}
	return &se3208_device::INVALIDOP;
}


void se3208_device::BuildTable(void)
{
	int i;
	for(i=0;i<0x10000;++i)
		OpTable[i]=DecodeOp(i);
}

void se3208_device::device_reset()
{
	for ( int i = 0; i < 8; i++ )
	{
		m_R[i] = 0;
	}
	m_SP = 0;
	m_ER = 0;
	m_PPC = 0;
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_PC=SE3208_Read32(0);
	m_SR=0;
	m_IRQ=CLEAR_LINE;
	m_NMI=CLEAR_LINE;
}

void se3208_device::SE3208_NMI()
{
	PushVal(m_PC);
	PushVal(m_SR);

	CLRFLAG(FLAG_NMI|FLAG_ENI|FLAG_E|FLAG_M);

	m_PC=SE3208_Read32(4);
}

void se3208_device::SE3208_Interrupt()
{
	if(!TESTFLAG(FLAG_ENI))
		return;

	PushVal(m_PC);
	PushVal(m_SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);


	if(!(TESTFLAG(FLAG_AUT)))
		m_PC=SE3208_Read32(8);
	else
		m_PC=SE3208_Read32(4*standard_irq_callback(0));
}


void se3208_device::execute_run()
{
	do
	{
		UINT16 Opcode=m_direct->read_word(m_PC, WORD_XOR_LE(0));

		debugger_instruction_hook(this, m_PC);

		(this->*OpTable[Opcode])(Opcode);
		m_PPC=m_PC;
		m_PC+=2;
		//Check interrupts
		if(m_NMI==ASSERT_LINE)
		{
			SE3208_NMI();
			m_NMI=CLEAR_LINE;
		}
		else if(m_IRQ==ASSERT_LINE && TESTFLAG(FLAG_ENI))
		{
			SE3208_Interrupt();
		}
		--(m_icount);
	} while(m_icount>0);
}

void se3208_device::device_start()
{
	BuildTable();

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	save_item(NAME(m_R));
	save_item(NAME(m_PC));
	save_item(NAME(m_SR));
	save_item(NAME(m_SP));
	save_item(NAME(m_ER));
	save_item(NAME(m_IRQ));
	save_item(NAME(m_NMI));

	state_add( SE3208_PC,  "PC", m_PC).formatstr("%08X");
	state_add( SE3208_SR,  "SR", m_SR).formatstr("%08X");
	state_add( SE3208_ER,  "ER", m_ER).formatstr("%08X");
	state_add( SE3208_SP,  "SP", m_SP).formatstr("%08X");
	state_add( SE3208_R0,  "R0", m_R[ 0]).formatstr("%08X");
	state_add( SE3208_R1,  "R1", m_R[ 1]).formatstr("%08X");
	state_add( SE3208_R2,  "R2", m_R[ 2]).formatstr("%08X");
	state_add( SE3208_R3,  "R3", m_R[ 3]).formatstr("%08X");
	state_add( SE3208_R4,  "R4", m_R[ 4]).formatstr("%08X");
	state_add( SE3208_R5,  "R5", m_R[ 5]).formatstr("%08X");
	state_add( SE3208_R6,  "R6", m_R[ 6]).formatstr("%08X");
	state_add( SE3208_R7,  "R7", m_R[ 7]).formatstr("%08X");
	state_add( SE3208_PPC, "PPC", m_PPC).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_PC).noshow();
	state_add(STATE_GENSP, "GENSP", m_SP).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_SR).formatstr("%10s").noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_PPC).noshow();

	m_icountptr = &m_icount;
}


void se3208_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c %c%c%c%c%c",
					m_SR&FLAG_C?'C':'.',
					m_SR&FLAG_V?'V':'.',
					m_SR&FLAG_S?'S':'.',
					m_SR&FLAG_Z?'Z':'.',

					m_SR&FLAG_M?'M':'.',
					m_SR&FLAG_E?'E':'.',
					m_SR&FLAG_AUT?'A':'.',
					m_SR&FLAG_ENI?'I':'.',
					m_SR&FLAG_NMI?'N':'.'
			);
			break;
	}
}

void se3208_device::execute_set_input( int line, int state )
{
	if(line==INPUT_LINE_NMI)    //NMI
		m_NMI=state;
	else
		m_IRQ=state;
}

offs_t se3208_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( se3208 );
	return CPU_DISASSEMBLE_NAME(se3208)(this, buffer, pc, oprom, opram, options);
}
