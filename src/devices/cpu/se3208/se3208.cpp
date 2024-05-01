// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "se3208.h"
#include "se3208dis.h"


/*
    SE3208 CPU Emulator by ElSemi

    For information about this CPU:
    www.adc.co.kr

*/


enum : uint32_t
{
	FLAG_C      = 0x0080,
	FLAG_V      = 0x0010,
	FLAG_S      = 0x0020,
	FLAG_Z      = 0x0040,

	FLAG_M      = 0x0200,
	FLAG_E      = 0x0800,
	FLAG_AUT    = 0x1000,
	FLAG_ENI    = 0x2000,
	FLAG_NMI    = 0x4000
};

//Precompute the instruction decoding in a big table
#define INST(a) void se3208_device::a(uint16_t Opcode)

DEFINE_DEVICE_TYPE(SE3208, se3208_device, "se3208", "ADChips SE3208")


se3208_device::se3208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SE3208, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_machinex_cb(*this)
	, m_iackx_cb(*this, 0)
	, m_PC(0), m_SR(0), m_SP(0), m_ER(0), m_PPC(0), m_IRQ(0), m_NMI(0), m_icount(0)
{
}

device_memory_interface::space_config_vector se3208_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


uint8_t se3208_device::SE3208_Read8(uint32_t address)
{
	return m_program.read_byte(address);
}

uint16_t se3208_device::SE3208_Read16(uint32_t address)
{
	if (!WORD_ALIGNED(address))
		return m_program.read_byte(address) | m_program.read_byte(address+1)<<8;
	else
		return m_program.read_word(address);
}

uint32_t se3208_device::SE3208_Read32(uint32_t address)
{
	if (DWORD_ALIGNED(address))
		return m_program.read_dword(address);
	else
	{
		osd_printf_debug("%08x: dword READ unaligned %08x\n", m_PC, address);
		return m_program.read_byte(address) | m_program.read_byte(address + 1) << 8 | m_program.read_byte(address + 2) << 16 | m_program.read_byte(address + 3) << 24;
	}
}

void se3208_device::SE3208_Write8(uint32_t address,uint8_t data)
{
	m_program.write_byte(address,data);
}

void se3208_device::SE3208_Write16(uint32_t address,uint16_t data)
{
	if (!WORD_ALIGNED(address))
	{
		m_program.write_byte(address, data & 0xff);
		m_program.write_byte(address+1, (data>>8)&0xff);
	}
	else
	{
		m_program.write_word(address, data);
	}
}

void se3208_device::SE3208_Write32(uint32_t address, uint32_t data)
{
	if (DWORD_ALIGNED(address))
		m_program.write_dword(address, data);
	else
	{
		m_program.write_byte(address, data & 0xff);
		m_program.write_byte(address + 1, (data >> 8) & 0xff);
		m_program.write_byte(address + 2, (data >> 16) & 0xff);
		m_program.write_byte(address + 3, (data >> 24) & 0xff);
		osd_printf_debug("%08x: dword WRITE unaligned %08x\n", m_PC, address);
	}
}



uint32_t se3208_device::AddWithFlags(uint32_t a,uint32_t b)
{
	uint32_t r=a+b;
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

uint32_t se3208_device::SubWithFlags(uint32_t a,uint32_t b) //a-b
{
	uint32_t r=a-b;
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

uint32_t se3208_device::AdcWithFlags(uint32_t a,uint32_t b)
{
	uint32_t C=(m_SR&FLAG_C)?1:0;
	uint32_t r=a+b+C;
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

uint32_t se3208_device::SbcWithFlags(uint32_t a,uint32_t b)
{
	uint32_t C=(m_SR&FLAG_C)?1:0;
	uint32_t r=a-b-C;
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

uint32_t se3208_device::MulWithFlags(uint32_t a,uint32_t b)
{
	int64_t r=(int64_t) a*(int64_t) b;
	CLRFLAG(FLAG_V);
	if(r>>32)
		SETFLAG(FLAG_V);
	return (uint32_t) (r&0xffffffff);
}

uint32_t se3208_device::NegWithFlags(uint32_t a)
{
	return SubWithFlags(0,a);
}

uint32_t se3208_device::AsrWithFlags(uint32_t Val, uint8_t By)
{
	signed int v=(signed int) Val;
	v>>=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(BIT(Val,By-1))
		SETFLAG(FLAG_C);
	return (uint32_t) v;
}

uint32_t se3208_device::LsrWithFlags(uint32_t Val, uint8_t By)
{
	uint32_t v=Val;
	v>>=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(BIT(Val,By-1))
		SETFLAG(FLAG_C);
	return v;
}

uint32_t se3208_device::AslWithFlags(uint32_t Val, uint8_t By)
{
	uint32_t v=Val;
	v<<=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(BIT(Val,32-By))
		SETFLAG(FLAG_C);
	return v;
}


INST(INVALIDOP)
{
	//assert(false);
}

INST(LDB)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);
	uint32_t Val;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=int8_t(Val);

	CLRFLAG(FLAG_E);
}

INST(STB)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	SE3208_Write8(Index+Offset,m_R[SrcDst]&0xFF);

	CLRFLAG(FLAG_E);
}

INST(LDS)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);
	uint32_t Val;

	Offset<<=1;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=int16_t(Val);

	CLRFLAG(FLAG_E);
}

INST(STS)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);

	Offset<<=1;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	SE3208_Write16(Index+Offset,m_R[SrcDst]&0xFFFF);

	CLRFLAG(FLAG_E);
}

INST(LD)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);

	Offset<<=2;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	m_R[SrcDst]=SE3208_Read32(Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(ST)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);

	Offset<<=2;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	SE3208_Write32(Index+Offset,m_R[SrcDst]);

	CLRFLAG(FLAG_E);
}

INST(LDBU)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);
	uint32_t Val;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=Val;

	CLRFLAG(FLAG_E);
}

INST(LDSU)
{
	uint32_t Offset=BIT(Opcode,0,5);
	uint32_t Index=BIT(Opcode,5,3);
	uint32_t SrcDst=BIT(Opcode,8,3);
	uint32_t Val;

	Offset<<=1;

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=Val&0xFFFF;

	CLRFLAG(FLAG_E);
}


INST(LERI)
{
	uint32_t Imm=BIT(Opcode,0,14);
	if(TESTFLAG(FLAG_E))
		m_ER=(m_ER<<14)|Imm;
	else
		m_ER=util::sext(Imm,14);


	SETFLAG(FLAG_E);
}

INST(LDSP)
{
	uint32_t Offset=BIT(Opcode,0,8);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,8,3);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	m_R[SrcDst]=SE3208_Read32(Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(STSP)
{
	uint32_t Offset=BIT(Opcode,0,8);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,8,3);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	SE3208_Write32(Index+Offset,m_R[SrcDst]);

	CLRFLAG(FLAG_E);
}

void se3208_device::PushVal(uint32_t Val)
{
	m_SP-=4;
	SE3208_Write32(m_SP,Val);
}

uint32_t se3208_device::PopVal()
{
	uint32_t Val=SE3208_Read32(m_SP);
	m_SP+=4;
	return Val;
}

INST(PUSH)
{
	uint32_t Set=BIT(Opcode,0,11);
	if(BIT(Set,10))
		PushVal(m_PC);
	if(BIT(Set,9))
		PushVal(m_SR);
	if(BIT(Set,8))
		PushVal(m_ER);
	if(BIT(Set,7))
		PushVal(m_R[7]);
	if(BIT(Set,6))
		PushVal(m_R[6]);
	if(BIT(Set,5))
		PushVal(m_R[5]);
	if(BIT(Set,4))
		PushVal(m_R[4]);
	if(BIT(Set,3))
		PushVal(m_R[3]);
	if(BIT(Set,2))
		PushVal(m_R[2]);
	if(BIT(Set,1))
		PushVal(m_R[1]);
	if(BIT(Set,0))
		PushVal(m_R[0]);
}

INST(POP)
{
	uint32_t Set=BIT(Opcode,0,11);
	if(BIT(Set,0))
		m_R[0]=PopVal();
	if(BIT(Set,1))
		m_R[1]=PopVal();
	if(BIT(Set,2))
		m_R[2]=PopVal();
	if(BIT(Set,3))
		m_R[3]=PopVal();
	if(BIT(Set,4))
		m_R[4]=PopVal();
	if(BIT(Set,5))
		m_R[5]=PopVal();
	if(BIT(Set,6))
		m_R[6]=PopVal();
	if(BIT(Set,7))
		m_R[7]=PopVal();
	if(BIT(Set,8))
		m_ER=PopVal();
	if(BIT(Set,9))
		m_SR=PopVal();
	if(BIT(Set,10))
	{
		m_PC=PopVal()-2;        //PC automatically incresases by 2
	}
}

INST(LEATOSP)
{
	uint32_t Offset=BIT(Opcode,9,4);
	uint32_t Index=BIT(Opcode,3,3);

	if(Index)
		Index=m_R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|Offset;
	else
		Offset=util::sext(Offset,4);

	m_SP=(Index+Offset) & (~3);

	CLRFLAG(FLAG_E);
}

INST(LEAFROMSP)
{
	uint32_t Offset=BIT(Opcode,9,4);
	uint32_t Index=BIT(Opcode,3,3);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|Offset;
	else
		Offset=util::sext(Offset,4);

	m_R[Index]=m_SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEASPTOSP)
{
	uint32_t Offset=BIT(Opcode,0,8);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|(Offset&0xff);
	else
		Offset=util::sext(Offset,10);

	m_SP=(m_SP+Offset) & (~3);

	CLRFLAG(FLAG_E);
}

INST(MOV)
{
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,9,3);

	m_R[Dst]=m_R[Src];
}

INST(LDI)
{
	uint32_t Dst=BIT(Opcode,8,3);
	uint32_t Imm=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|(Imm&0xf);
	else
		Imm=int8_t(Imm);

	m_R[Dst]=Imm;

	CLRFLAG(FLAG_E);
}

INST(LDBSP)
{
	uint32_t Offset=BIT(Opcode,0,4);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,4,3);
	uint32_t Val;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|Offset;

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=int8_t(Val);

	CLRFLAG(FLAG_E);
}

INST(STBSP)
{
	uint32_t Offset=BIT(Opcode,0,4);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,4,3);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|Offset;

	SE3208_Write8(Index+Offset,m_R[SrcDst]&0xFF);

	CLRFLAG(FLAG_E);
}

INST(LDSSP)
{
	uint32_t Offset=BIT(Opcode,0,4);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,4,3);
	uint32_t Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=int16_t(Val);

	CLRFLAG(FLAG_E);
}

INST(STSSP)
{
	uint32_t Offset=BIT(Opcode,0,4);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,4,3);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	SE3208_Write16(Index+Offset,m_R[SrcDst]&0xFFFF);

	CLRFLAG(FLAG_E);
}

INST(LDBUSP)
{
	uint32_t Offset=BIT(Opcode,0,4);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,4,3);
	uint32_t Val;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|Offset;

	Val=SE3208_Read8(Index+Offset);
	m_R[SrcDst]=Val;

	CLRFLAG(FLAG_E);
}

INST(LDSUSP)
{
	uint32_t Offset=BIT(Opcode,0,4);
	uint32_t Index=m_SP;
	uint32_t SrcDst=BIT(Opcode,4,3);
	uint32_t Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<4)|(Offset&0xf);

	Val=SE3208_Read16(Index+Offset);
	m_R[SrcDst]=Val;

	CLRFLAG(FLAG_E);
}

INST(ADDI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=AddWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SUBI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=SubWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ADCI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=AdcWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SBCI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=SbcWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ANDI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=m_R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ORI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=m_R[Src]|Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(XORI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	m_R[Dst]=m_R[Src]^Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(CMPI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	SubWithFlags(m_R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(TSTI)
{
	uint32_t Imm=BIT(Opcode,9,4);
	uint32_t Src=BIT(Opcode,3,3);
	uint32_t Dst;

	if(TESTFLAG(FLAG_E))
		Imm=(m_ER<<4)|Imm;
	else
		Imm=util::sext(Imm,4);

	Dst=m_R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Dst)
		SETFLAG(FLAG_Z);
	if(Dst&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ADD)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=AddWithFlags(m_R[Src1],m_R[Src2]);
}

INST(SUB)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=SubWithFlags(m_R[Src1],m_R[Src2]);
}

INST(ADC)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=AdcWithFlags(m_R[Src1],m_R[Src2]);
}

INST(SBC)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=SbcWithFlags(m_R[Src1],m_R[Src2]);
}

INST(AND)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=m_R[Src1]&m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(OR)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=m_R[Src1]|m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(XOR)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=m_R[Src1]^m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(CMP)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);

	SubWithFlags(m_R[Src1],m_R[Src2]);
}

INST(TST)
{
	uint32_t Src2=BIT(Opcode,9,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst;

	Dst=m_R[Src1]&m_R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!Dst)
		SETFLAG(FLAG_Z);
	if(Dst&0x80000000)
		SETFLAG(FLAG_S);
}

INST(MULS)
{
	uint32_t Src2=BIT(Opcode,6,3);
	uint32_t Src1=BIT(Opcode,3,3);
	uint32_t Dst=BIT(Opcode,0,3);

	m_R[Dst]=MulWithFlags(m_R[Src1],m_R[Src2]);

	CLRFLAG(FLAG_E);
}

INST(NEG)
{
	uint32_t Dst=BIT(Opcode,9,3);
	uint32_t Src=BIT(Opcode,3,3);

	m_R[Dst]=NegWithFlags(m_R[Src]);
}

INST(CALL)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;
	PushVal(m_PC+2);
	m_PC=m_PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JV)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_V))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);

}

INST(JNV)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_V))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JC)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_C))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNC)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_C))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JP)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_S))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JM)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_S))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNZ)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_Z))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JZ)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGE)
{
	uint32_t Offset=BIT(Opcode,0,8);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!(S^V))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLE)
{
	uint32_t Offset=BIT(Opcode,0,8);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || (S^V))
	{
		m_PC=m_PC+Offset;
	}
	CLRFLAG(FLAG_E);
}

INST(JHI)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C)))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLS)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGT)
{
	uint32_t Offset=BIT(Opcode,0,8);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || (S^V)))
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLT)
{
	uint32_t Offset=BIT(Opcode,0,8);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);
	Offset<<=1;

	if(S^V)
	{
		m_PC=m_PC+Offset;
	}

	CLRFLAG(FLAG_E);
}



INST(JMP)
{
	uint32_t Offset=BIT(Opcode,0,8);

	if(TESTFLAG(FLAG_E))
		Offset=(m_ER<<8)|Offset;
	else
		Offset=int8_t(Offset);

	Offset<<=1;

	m_PC=m_PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JR)
{
	uint32_t Src=BIT(Opcode,0,4);

	m_PC=m_R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(CALLR)
{
	uint32_t Src=BIT(Opcode,0,4);
	PushVal(m_PC+2);
	m_PC=m_R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(ASR)
{
	uint32_t Dst=BIT(Opcode,0,3);

	if(BIT(Opcode,10))
	{
		uint32_t Cnt=BIT(Opcode,5,3);
		m_R[Dst]=AsrWithFlags(m_R[Dst],m_R[Cnt]&0x1f);
	}
	else
	{
		uint32_t Imm=BIT(Opcode,5,5);
		m_R[Dst]=AsrWithFlags(m_R[Dst],Imm);
	}

	CLRFLAG(FLAG_E);
}

INST(LSR)
{
	uint32_t Dst=BIT(Opcode,0,3);

	if(BIT(Opcode,10))
	{
		uint32_t Cnt=BIT(Opcode,5,3);
		m_R[Dst]=LsrWithFlags(m_R[Dst],m_R[Cnt]&0x1f);
	}
	else
	{
		uint32_t Imm=BIT(Opcode,5,5);
		m_R[Dst]=LsrWithFlags(m_R[Dst],Imm);
	}

	CLRFLAG(FLAG_E);
}

INST(ASL)
{
	uint32_t Dst=BIT(Opcode,0,3);

	if(BIT(Opcode,10))
	{
		uint32_t Cnt=BIT(Opcode,5,3);
		m_R[Dst]=AslWithFlags(m_R[Dst],m_R[Cnt]&0x1f);
	}
	else
	{
		uint32_t Imm=BIT(Opcode,5,5);
		m_R[Dst]=AslWithFlags(m_R[Dst],Imm);
	}

	CLRFLAG(FLAG_E);
}

INST(EXTB)
{
	uint32_t Dst=BIT(Opcode,0,4);
	uint32_t Val=m_R[Dst];

	m_R[Dst]=int8_t(Val&0xFF);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(EXTS)
{
	uint32_t Dst=BIT(Opcode,0,4);
	uint32_t Val=m_R[Dst];

	m_R[Dst]=int16_t(Val&0xFFFF);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!m_R[Dst])
		SETFLAG(FLAG_Z);
	if(m_R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(SET)
{
	uint32_t Imm=BIT(Opcode,0,4);

	m_SR|=(1<<Imm);
}

INST(CLR)
{
	uint32_t Imm=BIT(Opcode,0,4);

	m_SR&=~(1<<Imm);
}

void se3208_device::TakeExceptionVector(uint8_t vector)
{
	debugger_exception_hook(vector);
	m_PC=SE3208_Read32(4*vector);
}

INST(SWI)
{
	uint32_t Imm=BIT(Opcode,0,4);

	if(!TESTFLAG(FLAG_ENI))
		return;
	PushVal(m_PC);
	PushVal(m_SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	TakeExceptionVector(Imm+0x10);
	m_PC-=2;
}

INST(HALT)
{
	uint32_t Imm=BIT(Opcode,0,4);

	m_machinex_cb(0x10 | Imm);

//  DEBUGMESSAGE("HALT\t0x%x",Imm);
}

INST(MVTC)
{
//  uint32_t Imm=BIT(Opcode,0,4);

//  DEBUGMESSAGE("MVTC\t%%R0,%%CR%d",Imm);
}

INST(MVFC)
{
//  uint32_t Imm=BIT(Opcode,0,4);

//  DEBUGMESSAGE("MVFC\t%%CR0%d,%%R0",Imm);
}


se3208_device::OP se3208_device::DecodeOp(uint16_t Opcode)
{
	switch(BIT(Opcode,14,2))
	{
		case 0x0:
			{
				uint8_t Op=BIT(Opcode,11,3);
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
				switch(BIT(Opcode,11,3))
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
						switch(BIT(Opcode,6,3))
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
								switch(BIT(Opcode,0,3))
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
			switch(BIT(Opcode,12,2))
			{
				case 0:
					switch(BIT(Opcode,6,3))
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
							switch(BIT(Opcode,0,3))
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
					switch(BIT(Opcode,8,4))
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
					if(BIT(Opcode,11))
						return &se3208_device::LDI;
					else    //SP Ops
					{
						if(BIT(Opcode,10))
						{
							switch(BIT(Opcode,7,3))
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
							if(BIT(Opcode,9))
							{
								return &se3208_device::LEASPTOSP;
							}
							else
							{
								if(BIT(Opcode,8))
								{
								}
								else
								{
									switch(BIT(Opcode,4,4))
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
					switch(BIT(Opcode,9,3))
					{
						case 0:
						case 1:
						case 2:
						case 3:
							switch(BIT(Opcode,3,2))
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
							if(BIT(Opcode,3))
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
	for (auto & elem : m_R)
	{
		elem = 0;
	}
	m_SP = 0;
	m_ER = 0;
	m_PPC = 0;
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	m_PC=SE3208_Read32(0);
	m_SR=0;
	m_IRQ=CLEAR_LINE;
	m_NMI=CLEAR_LINE;
}

void se3208_device::SE3208_NMI()
{
	standard_irq_callback(INPUT_LINE_NMI, m_PC);
	m_machinex_cb(0x00);

	PushVal(m_PC);
	PushVal(m_SR);

	CLRFLAG(FLAG_NMI|FLAG_ENI|FLAG_E|FLAG_M);

	TakeExceptionVector(0x01);
}

void se3208_device::SE3208_Interrupt()
{
	if(!TESTFLAG(FLAG_ENI))
		return;

	standard_irq_callback(0, m_PC);
	m_machinex_cb(0x01);

	PushVal(m_PC);
	PushVal(m_SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	if(!(TESTFLAG(FLAG_AUT)))
		TakeExceptionVector(0x02);
	else
		TakeExceptionVector(m_iackx_cb());
}


void se3208_device::execute_run()
{
	do
	{
		uint16_t Opcode=m_cache.read_word(m_PC, WORD_XOR_LE(0));

		m_PPC = m_PC;
		debugger_instruction_hook(m_PC);

		(this->*OpTable[Opcode])(Opcode);
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

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

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
	state_add(STATE_GENPCBASE, "CURPC", m_PPC).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_SR).formatstr("%10s").noshow();

	set_icountptr(m_icount);
}


void se3208_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c %c%c%c%c%c",
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

std::unique_ptr<util::disasm_interface> se3208_device::create_disassembler()
{
	return std::make_unique<se3208_disassembler>();
}
