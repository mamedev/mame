// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina, Angelo Salese
/******************************************************************
 NEC V810 (μpd70732) core
  Tomasz Slanina
  Angelo Salese

 Change Log
 - 07/04/2023 - Update opcode timings (Angelo Salese)
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
  - NVC (vboy CPU) onchip peripherals ($0200'0000 area);
  - bit string cycle counts are approximate: the fixed part does not depend on the alignment case;
  - cache handling;
  - external bus timing (on load/store opcodes):
  \- 3 cycles for ROM
  \- 2 cycles for RAM
  \- 1 cycle for cache
  - pipeline;

******************************************************************/

#include "emu.h"
#include "v810.h"
#include "v810dasm.h"

#include "corefloat.h"

#include "softfloat3/source/include/softfloat.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>

#define clkIF 3
#define clkMEM 3
#define clkIRQ 14

namespace {

// the SoftFloat state is shared with other cores: set what the V810 needs, and restore it afterwards
class softfloat_scope
{
public:
	softfloat_scope()
		: m_rounding(softfloat_roundingMode)
		, m_tininess(softfloat_detectTininess)
		, m_flags(softfloat_exceptionFlags)
	{
		softfloat_roundingMode = softfloat_round_near_even;
		softfloat_detectTininess = softfloat_tininess_afterRounding;
		softfloat_exceptionFlags = 0;
	}

	~softfloat_scope()
	{
		softfloat_roundingMode = m_rounding;
		softfloat_detectTininess = m_tininess;
		softfloat_exceptionFlags = m_flags;
	}

private:
	const uint_fast8_t m_rounding;
	const uint_fast8_t m_tininess;
	const uint_fast8_t m_flags;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(V810, v810_device, "v810", "NEC V810")
DEFINE_DEVICE_TYPE(NINTENDO_NVC, nintendo_nvc_device, "nintendo_nvc", "Nintendo NVC")


v810_device::v810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: v810_device(mconfig, V810, tag, owner, clock, s_opcodes, 0x00008100) // V810 family: 0000810xh
{
}

v810_device::v810_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const opcode_table &opcodes, uint32_t pir)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_opcodes(opcodes)
	, m_pir(pir)
	, m_halted(false)
	, m_bus_wait(*this)
{
}

nintendo_nvc_device::nintendo_nvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: v810_device(mconfig, NINTENDO_NVC, tag, owner, clock, s_nvc_opcodes, 0x00005346) // Virtual Boy Development Manual 4-7-1
{
}

device_memory_interface::space_config_vector v810_device::memory_space_config() const
{
	if(has_configured_map(AS_IO))
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	}
	else
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
	}
}


std::unique_ptr<util::disasm_interface> v810_device::create_disassembler()
{
	return std::make_unique<v810_disassembler>();
}

std::unique_ptr<util::disasm_interface> nintendo_nvc_device::create_disassembler()
{
	return std::make_unique<nintendo_nvc_disassembler>();
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

#define R_B(addr) (m_program.read_byte(addr))
#define R_H(addr) (m_program.read_word(addr))
#define R_W(addr) (m_program.read_dword(addr))


#define W_B(addr, val) (m_program.write_byte(addr,val))
#define W_H(addr, val) (m_program.write_word(addr,val))
#define W_W(addr, val) (m_program.write_dword(addr,val))


#define RIO_B(addr) (m_io.read_byte(addr))
#define RIO_H(addr) (m_io.read_word(addr))
#define RIO_W(addr) (m_io.read_dword(addr))


#define WIO_B(addr, val) (m_io.write_byte(addr,val))
#define WIO_H(addr, val) (m_io.write_word(addr,val))
#define WIO_W(addr, val) (m_io.write_dword(addr,val))

#define R_OP(addr)  (fetch_op(addr, 2))

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

#define CHECK_CY(x) PSW=(PSW & ~8)|(((x) & (((uint64_t)1) << 32)) ? 8 : 0)
#define CHECK_OVADD(x,y,z) PSW=(PSW & ~0x00000004) |(( ((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) ? 4 : 0)
#define CHECK_OVSUB(x,y,z) PSW=(PSW & ~0x00000004) |(( ((y) ^ (z)) & ((x) ^ (y)) & 0x80000000) ? 4 : 0)
#define CHECK_ZS(x) PSW=(PSW & ~3)|((uint32_t)(x)==0)|(((x)&0x80000000) ? 2 : 0)


// r0 is literally a "register zero", reading returns 0, writing is ignored.
void v810_device::SETREG(uint32_t reg,uint32_t val)
{
	if(reg)
		m_reg[reg]=val;
}

uint32_t v810_device::GETREG(uint32_t reg)
{
	if(reg)
		return m_reg[reg];
	else
		return 0;
}

// opcode fetch: with the cache off the second halfword of a 32-bit instruction costs 2 cycles more
inline uint16_t v810_device::fetch_op(offs_t addr, int cycles)
{
	if (m_bus_timing)
	{
		if (BIT(CHCW, 1))
		{
			const u32 index = (addr >> 3) & 0x7f;
			const u32 sub = BIT(addr, 2);
			if (m_icache_tag[index] != (addr >> 10))
			{
				m_icache_tag[index] = addr >> 10;
				m_icache_valid[index][0] = m_icache_valid[index][1] = false;
			}
			if (!m_icache_valid[index][sub])
			{
				m_icache_valid[index][sub] = true;
				m_icount -= 2 + m_bus_wait(addr & ~3, BUS_FETCH, 4);
			}
		}
		else
		{
			m_icount -= cycles + m_bus_wait(addr, BUS_FETCH, 2);
		}
	}
	return m_cache.read_word(addr);
}

// cycles for a load/store/in/out with bus timing: base cost, adjusted when it follows the same kind of access
inline uint32_t v810_device::mem_cycles(offs_t addr, int access, int size, int base, int pair, u8 kind)
{
	const int cycles = base + ((m_lastop == kind) ? pair : 0) + m_bus_wait(addr, access, size);
	m_curop = kind;
	return cycles;
}

void v810_device::icache_clear(u32 start, u32 count)
{
	for (u32 i = start; i < start + count && i < 128; i++)
	{
		m_icache_tag[i] = ~0U;
		m_icache_valid[i][0] = m_icache_valid[i][1] = false;
	}
}

// invalid instruction exception, restarts the instruction
uint32_t v810_device::opUNDEF(uint32_t op)
{
	logerror("V810: invalid opcode %04x @ %08x\n", op, m_PPC);
	PC = m_PPC;
	take_exception(0xffffff90, 0xff90);
	return 1;
}

uint32_t v810_device::opMOVr(uint32_t op) // mov reg1, reg2
{
	SETREG(GET2,GETREG(GET1));
	return 1;
}

uint32_t v810_device::opMOVEA(uint32_t op)   // movea imm16, reg1, reg2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=R_OP(PC);
	PC+=2;
	op2=I16(op2);
	SETREG(GET2,op1+op2);
	return 1;
}

uint32_t v810_device::opMOVHI(uint32_t op)   // movhi imm16, reg1 ,reg2
{
	uint32_t op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2<<=16;
	SETREG(GET2,GETREG(GET1)+op2);
	return 1;
}

uint32_t v810_device::opMOVi(uint32_t op)    // mov imm5,r2
{
	SETREG(GET2,I5(op));
	return 1;
}

uint32_t v810_device::opADDr(uint32_t op)    // add r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=GETREG(GET2);
	uint64_t res=(uint64_t)op2+(uint64_t)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return 1;
}

uint32_t v810_device::opADDi(uint32_t op)    // add imm5,r2
{
	uint32_t op1=I5(op);
	uint32_t op2=GETREG(GET2);
	uint64_t res=(uint64_t)op2+(uint64_t)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return 1;
}


uint32_t v810_device::opADDI(uint32_t op)    // addi imm16, reg1, reg2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=R_OP(PC);
	uint64_t res;
	PC+=2;
	op2=I16(op2);
	res=(uint64_t)op2+(uint64_t)op1;
	CHECK_CY(res);
	CHECK_OVADD(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return 1;
}

uint32_t v810_device::opSUBr(uint32_t op)    // sub r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=GETREG(GET2);
	uint64_t res=(uint64_t)op2-(uint64_t)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	SETREG(GET2,res);
	return 1;
}


uint32_t v810_device::opCMPr(uint32_t op)    // cmp r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=GETREG(GET2);
	uint64_t res=(uint64_t)op2-(uint64_t)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return 1;
}

uint32_t v810_device::opCMPi(uint32_t op)    // cmpi imm5,r2
{
	uint32_t op1=I5(op);
	uint32_t op2=GETREG(GET2);
	uint64_t res=(uint64_t)op2-(uint64_t)op1;
	CHECK_CY(res);
	CHECK_OVSUB(op1,op2,res);
	CHECK_ZS(res);
	return 1;
}

uint32_t v810_device::opSETFi(uint32_t op)   // setf imm5,r2
{
	uint32_t op1=I5(op);
	uint8_t res=0;
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
	return 1;
}


uint32_t v810_device::opANDr(uint32_t op)    // and r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=GETREG(GET2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return 1;
}

uint32_t v810_device::opANDI(uint32_t op)    // andi imm16,r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2&=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SET_S(0);
	SETREG(GET2,op2);
	return 1;
}

uint32_t v810_device::opORr(uint32_t op) // or r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=GETREG(GET2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return 1;
}

uint32_t v810_device::opORI(uint32_t op) // ori imm16,r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2|=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return 1;
}

uint32_t v810_device::opXORr(uint32_t op)    // xor r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=GETREG(GET2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return 1;
}


uint32_t v810_device::opLDSR(uint32_t op) // ldsr reg2,regID
{
	const uint32_t val = GETREG(GET2);
	switch (UI5(op))
	{
		case 4: // ECR
		case 6: // PIR
		case 7: // TKCW
			// read only
			break;

		case 1: // EIPSW
		case 3: // FEPSW
		case 5: // PSW
			// bits 31-20, 11 and 10 are fixed to 0
			m_reg[32 + UI5(op)] = val & 0x000ff3ff;
			break;

		case 0:  // EIPC
		case 2:  // FEPC
		case 25: // ADTRE
			// bit 0 is fixed to 0
			m_reg[32 + UI5(op)] = val & ~1;
			break;

		case 24: // CHCW
			// only ICE reads back, the other fields are commands
			CHCW = val & 0x00000002;
			// ICC: clear CEC entries starting at CEN
			if (BIT(val, 0))
				icache_clear(val >> 20, (val >> 8) & 0xfff);
			if (val & 0x00000030)
				logerror("V810: unimplemented instruction cache %s, CHCW %08x\n", BIT(val, 4) ? "dump" : "restore", val);
			break;

		default:
			m_reg[32 + UI5(op)] = val;
			break;
	}
	return 2;
}

uint32_t v810_device::opSTSR(uint32_t op) // ldsr regID,reg2
{
	uint32_t op1=UI5(op);
	SETREG(GET2,GETREG(32+op1));
	return 2;
}


uint32_t v810_device::opXORI(uint32_t op)    // xori imm16,r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=R_OP(PC);
	PC+=2;
	op2=UI16(op2);
	op2^=op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return 1;
}

uint32_t v810_device::opNOTr(uint32_t op)    // not r1,r2
{
	uint32_t op1=GETREG(GET1);
	uint32_t op2=~op1;
	CHECK_ZS(op2);
	SET_OV(0);
	SETREG(GET2,op2);
	return 1;
}

uint32_t v810_device::opSHLr(uint32_t op)    // shl r1,r2
{
	uint64_t tmp;
	uint32_t count=GETREG(GET1);
	count&=0x1f;

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
	return 1;
}

uint32_t v810_device::opSHLi(uint32_t op)    // shl imm5,r2
{
	uint64_t tmp;
	uint32_t count=UI5(op);

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
	return 1;
}

uint32_t v810_device::opSHRr(uint32_t op)    // shr r1,r2
{
	uint64_t tmp;
	uint32_t count=GETREG(GET1);
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
	return 1;
}

uint32_t v810_device::opSHRi(uint32_t op)    // shr imm5,r2
{
	uint64_t tmp;
	uint32_t count=UI5(op);
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
	return 1;
}

uint32_t v810_device::opSARr(uint32_t op)    // sar r1,r2
{
	int32_t tmp;
	uint32_t count=GETREG(GET1);
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
	return 1;
}

uint32_t v810_device::opSARi(uint32_t op)    // sar imm5,r2
{
	int32_t tmp;
	uint32_t count=UI5(op);
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
	return 1;
}

uint32_t v810_device::opJMPr(uint32_t op)
{
	PC=GETREG(GET1)&~1;
	return 3;
}


uint32_t v810_device::opJR(uint32_t op)
{
	uint32_t tmp=R_OP(PC);
	PC=PC-2+(D26(op,tmp)&~1);
	return 3;
}

uint32_t v810_device::opJAL(uint32_t op)
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	R31=PC;
	PC+=D26(op,tmp);
	PC-=4;
	PC&=~1;
	return 3;
}

// CLI: clear PSW.ID (Virtual Boy Development Manual 4-6-3)
uint32_t nintendo_nvc_device::opCLI(uint32_t op)
{
	SET_ID(0);
	return 1;
}

// SEI: set PSW.ID (Virtual Boy Development Manual 4-6-4)
uint32_t nintendo_nvc_device::opSEI(uint32_t op)
{
	SET_ID(1);
	return 1;
}

// software exception, vectors 0-15 at 0xffffffa0, 16-31 at 0xffffffb0; returns to the next instruction
uint32_t v810_device::opTRAP(uint32_t op)
{
	const u8 vector = op & 0x1f;
	take_exception(0xffffffa0 | (vector & 0x10), 0xffa0 | vector);
	return 15;
}

void v810_device::take_exception(u32 handler, u16 code)
{
	if (GET_NP)
	{
		// fatal exception: the exception code, PSW and PC are written to 00000000h,
		// 00000004h and 00000008h, and the CPU halts until reset
		logerror("V810: fatal exception %04X @ %X, halted\n", code, m_PPC);
		W_W(0x00000000, 0xffff0000 | code);
		W_W(0x00000004, PSW);
		W_W(0x00000008, PC);
		m_halted = true;
		m_icount = 0;
		return;
	}
	else if (GET_EP)
	{
		// duplexed exception
		FEPC = PC;
		FEPSW = PSW;
		ECR = (ECR & 0xffff) | (code << 16);
		SET_NP(1);
		SET_ID(1);
		SET_AE(0);
		PC = 0xffffffd0;
	}
	else
	{
		EIPC = PC;
		EIPSW = PSW;
		ECR = (ECR & 0xffff0000) | code;
		SET_EP(1);
		SET_ID(1);
		SET_AE(0);
		PC = handler;
	}
}

uint32_t v810_device::opRETI(uint32_t op)
{
	if(GET_NP) {
		PC = FEPC;
		PSW = FEPSW;
	} else {
		PC = EIPC;
		PSW = EIPSW;
	}
	return 10;
}

// stops until an interrupt is accepted (or reset); the interrupt saves the address of the next instruction
uint32_t v810_device::opHALT(uint32_t op)
{
	m_halt = true;
	return 1;
}

uint32_t v810_device::opB(uint32_t op)
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
		PC = PC - 2 + (D9(op) &~1);
		return 3;
	}
	return 1;
}

uint32_t v810_device::opLDB(uint32_t op) // ld.b disp16[reg1],reg2
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	const uint32_t addr=tmp;
	tmp=R_B(tmp);
	tmp|=(tmp&0x80)?0xffffff00:0;
	SETREG(GET2,tmp);
	return m_bus_timing ? mem_cycles(addr, BUS_READ, 1, 3, -1, LAST_LD) : 2+clkMEM;
}

uint32_t v810_device::opLDH(uint32_t op) // ld.h disp16[reg1],reg2
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	const uint32_t addr=tmp&~1;
	tmp=R_H(tmp&~1);
	tmp|=(tmp&0x8000)?0xffff0000:0;
	SETREG(GET2,tmp);
	return m_bus_timing ? mem_cycles(addr, BUS_READ, 2, 3, -1, LAST_LD) : 2+clkMEM;
}

uint32_t v810_device::opLDW(uint32_t op) // ld.w disp16[reg1],reg2
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	const uint32_t addr=tmp&~3;
	tmp=R_W(tmp&~3);
	SETREG(GET2,tmp);
	return m_bus_timing ? mem_cycles(addr, BUS_READ, 4, 3, -1, LAST_LD) : 2+clkMEM;
}

uint32_t v810_device::opINB(uint32_t op) // in.b disp16[reg1],reg2
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	const uint32_t addr=tmp;
	tmp=RIO_B(tmp);
	SETREG(GET2,tmp);
	return m_bus_timing ? mem_cycles(addr, BUS_IO_READ, 1, 3, 0, LAST_OTHER) : 2+clkMEM;
}

// compare and exchange interlocked: the flags are those of reg2 - lock word; the lock word is
// replaced by r30 if they are equal (otherwise written back unchanged), and reg2 gets the lock word
uint32_t v810_device::opCAXI(uint32_t op)    // caxi disp16[reg1],reg2
{
	const uint32_t disp = R_OP(PC);
	const uint32_t addr = (GETREG(GET1) + D16(disp)) & ~3;
	PC += 2;
	const uint32_t lock = R_W(addr);
	const uint32_t op2 = GETREG(GET2);
	const uint64_t res = uint64_t(op2) - uint64_t(lock);
	CHECK_CY(res);
	CHECK_OVSUB(lock, op2, res);
	CHECK_ZS(res);
	W_W(addr, uint32_t(res) ? lock : GETREG(30));
	SETREG(GET2, lock);
	return m_bus_timing ? (22 + m_bus_wait(addr, BUS_READ, 4) + m_bus_wait(addr, BUS_WRITE, 4)) : 22;
}

uint32_t v810_device::opINH(uint32_t op) // in.h disp16[reg1],reg2
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	const uint32_t addr=tmp&~1;
	tmp=RIO_H(tmp&~1);
	SETREG(GET2,tmp);
	return m_bus_timing ? mem_cycles(addr, BUS_IO_READ, 2, 3, 0, LAST_OTHER) : 2+clkMEM;
}

uint32_t v810_device::opINW(uint32_t op) // in.w disp16[reg1],reg2
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	const uint32_t addr=tmp&~3;
	tmp=RIO_W(tmp&~3);
	SETREG(GET2,tmp);
	return m_bus_timing ? mem_cycles(addr, BUS_IO_READ, 4, 3, 0, LAST_OTHER) : 2+clkMEM;
}

uint32_t v810_device::opSTB(uint32_t op) // st.b reg2, disp16[reg1]
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_B(tmp,GETREG(GET2)&0xff);
	return m_bus_timing ? mem_cycles(tmp, BUS_WRITE, 1, 1, 1, LAST_ST) : 1+clkMEM;
}

uint32_t v810_device::opSTH(uint32_t op) // st.h reg2, disp16[reg1]
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_H(tmp&~1,GETREG(GET2)&0xffff);
	return m_bus_timing ? mem_cycles(tmp&~1, BUS_WRITE, 2, 1, 1, LAST_ST) : 1+clkMEM;
}

uint32_t v810_device::opSTW(uint32_t op) // st.w reg2, disp16[reg1]
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	W_W(tmp&~3,GETREG(GET2));
	return m_bus_timing ? mem_cycles(tmp&~3, BUS_WRITE, 4, 1, 1, LAST_ST) : 1+clkMEM;
}

uint32_t v810_device::opOUTB(uint32_t op)    // out.b reg2, disp16[reg1]
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_B(tmp,GETREG(GET2)&0xff);
	return m_bus_timing ? mem_cycles(tmp, BUS_IO_WRITE, 1, 1, 1, LAST_OUT) : 1+clkMEM;
}

uint32_t v810_device::opOUTH(uint32_t op)    // out.h reg2, disp16[reg1]
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_H(tmp&~1,GETREG(GET2)&0xffff);
	return m_bus_timing ? mem_cycles(tmp&~1, BUS_IO_WRITE, 2, 1, 1, LAST_OUT) : 1+clkMEM;
}

uint32_t v810_device::opOUTW(uint32_t op)    // out.w reg2, disp16[reg1]
{
	uint32_t tmp=R_OP(PC);
	PC+=2;
	tmp=D16(tmp);
	tmp+=GETREG(GET1);
	WIO_W(tmp&~3,GETREG(GET2));
	return m_bus_timing ? mem_cycles(tmp&~3, BUS_IO_WRITE, 4, 1, 1, LAST_OUT) : 1+clkMEM;
}

uint32_t v810_device::opMULr(uint32_t op)    // mul r1,r2
{
	const int64_t result = int64_t(int32_t(GETREG(GET1))) * int64_t(int32_t(GETREG(GET2)));
	const uint32_t low = uint32_t(result);
	// high word to r30 first: when reg2 is r30 it gets the low word; CY is not affected
	SETREG(30, uint32_t(uint64_t(result) >> 32));
	SETREG(GET2, low);
	SET_OV(result != int64_t(int32_t(low)));
	CHECK_ZS(low);
	return 13;
}

uint32_t v810_device::opMULUr(uint32_t op)   // mulu r1,r2
{
	const uint64_t result = uint64_t(GETREG(GET1)) * uint64_t(GETREG(GET2));
	const uint32_t low = uint32_t(result);
	// high word to r30 first: when reg2 is r30 it gets the low word; CY is not affected
	SETREG(30, uint32_t(result >> 32));
	SETREG(GET2, low);
	SET_OV((result >> 32) != 0);
	CHECK_ZS(low);
	return 13;
}

uint32_t v810_device::opDIVr(uint32_t op)    // div r1,r2
{
	const int32_t op1 = GETREG(GET1);
	const int32_t op2 = GETREG(GET2);
	if (!op1)
	{
		// zero division exception, restarts the instruction; registers and flags are unchanged
		PC = m_PPC;
		take_exception(0xffffff80, 0xff80);
		return 38;
	}

	// the only overflow is 80000000h / -1: quotient 80000000h, remainder 0
	const bool overflow = (op2 == std::numeric_limits<int32_t>::min()) && (op1 == -1);
	const uint32_t quotient = overflow ? op2 : (op2 / op1);
	const uint32_t remainder = overflow ? 0 : (op2 % op1);
	// remainder to r30 first: when reg2 is r30 it gets the quotient
	SETREG(30, remainder);
	SETREG(GET2, quotient);
	SET_OV(overflow);
	CHECK_ZS(quotient);
	return 38;
}

uint32_t v810_device::opDIVUr(uint32_t op)   // divu r1,r2
{
	const uint32_t op1 = GETREG(GET1);
	const uint32_t op2 = GETREG(GET2);
	if (!op1)
	{
		// zero division exception, restarts the instruction; registers and flags are unchanged
		PC = m_PPC;
		take_exception(0xffffff80, 0xff80);
		return 36;
	}

	const uint32_t quotient = op2 / op1;
	// remainder to r30 first: when reg2 is r30 it gets the quotient
	SETREG(30, op2 % op1);
	SETREG(GET2, quotient);
	SET_OV(0);
	CHECK_ZS(quotient);
	return 36;
}

// floating point operand that raises the reserved operand exception: denormal, NaN or infinity
static inline bool fp_reserved(uint32_t v)
{
	const uint32_t exp = BIT(v, 23, 8);
	return (v & 0x7fffffff) && (exp == 0 || exp == 0xff);
}

// floating point exception: the PSW flag is set before PSW is saved; the instruction is restarted
void v810_device::fp_exception(uint32_t psw_bit, u16 code)
{
	PSW |= psw_bit;
	PC = m_PPC;
	take_exception(0xffffff60, code);
}

// CY, OV, S and Z of a floating point result: CY and S are the sign, both clear for zero
void v810_device::fp_set_flags(uint32_t result)
{
	const bool zero = !(result & 0x7fffffff);
	const bool negative = !zero && BIT(result, 31);
	SET_OV(0);
	SET_Z(zero ? 1 : 0);
	SET_S(negative ? 1 : 0);
	SET_CY(negative ? 1 : 0);
}

// ADDF.S, SUBF.S, MULF.S, DIVF.S: reg2 = reg2 op reg1, rounded to nearest
void v810_device::fp_arith(uint32_t op, fp_op kind)
{
	const uint32_t op1 = GETREG(GET1);
	const uint32_t op2 = GETREG(GET2);
	if (fp_reserved(op1) || fp_reserved(op2))
	{
		fp_exception(PSW_FRO, 0xff60);
		return;
	}
	if (kind == FP_DIV && !(op1 & 0x7fffffff))
	{
		if (!(op2 & 0x7fffffff))
			fp_exception(PSW_FIV, 0xff70); // 0 / 0
		else
			fp_exception(PSW_FZD, 0xff68);
		return;
	}

	const softfloat_scope fpstate;
	float32_t result;
	switch (kind)
	{
		case FP_ADD:
			result = f32_add(float32_t{ op2 }, float32_t{ op1 });
			break;
		case FP_SUB:
			result = f32_sub(float32_t{ op2 }, float32_t{ op1 });
			break;
		case FP_MUL:
			result = f32_mul(float32_t{ op2 }, float32_t{ op1 });
			break;
		default:
			result = f32_div(float32_t{ op2 }, float32_t{ op1 });
			break;
	}
	uint32_t res = result.v;
	const uint8_t flags = softfloat_exceptionFlags;
	if (flags & softfloat_flag_inexact)
		PSW |= PSW_FPR;

	if (flags & softfloat_flag_overflow)
	{
		// the result is stored with its exponent corrected (divided by 2^192), then the exception is raised
		const double x = u2f(op2), y = u2f(op1);
		const double exact = (kind == FP_ADD) ? (x + y) : (kind == FP_SUB) ? (x - y) : (kind == FP_MUL) ? (x * y) : (x / y);
		res = f2u(float(std::ldexp(exact, -192)));
		SETREG(GET2, res);
		fp_set_flags(res);
		fp_exception(PSW_FOV, 0xff64);
		return;
	}
	if ((flags & softfloat_flag_underflow) || (!(res & 0x7f800000) && (res & 0x7fffffff)))
	{
		// underflow: no exception, zero is stored
		res &= 0x80000000;
		PSW |= PSW_FUD | PSW_FPR;
	}
	SETREG(GET2, res);
	fp_set_flags(res);
}

// CVT.SW (rounded to nearest) and TRNC.SW (truncated): CY is not affected, S and Z come from the integer
void v810_device::fp_to_int(uint32_t op, bool truncate)
{
	const uint32_t op1 = GETREG(GET1);
	if (fp_reserved(op1))
	{
		fp_exception(PSW_FRO, 0xff60);
		return;
	}

	const softfloat_scope fpstate;
	const int32_t res = f32_to_i32(float32_t{ op1 }, truncate ? softfloat_round_minMag : softfloat_round_near_even, true);
	if (softfloat_exceptionFlags & softfloat_flag_invalid)
	{
		// out of range: reg2 is unchanged
		fp_exception(PSW_FIV, 0xff70);
		return;
	}
	if (softfloat_exceptionFlags & softfloat_flag_inexact)
		PSW |= PSW_FPR;
	SETREG(GET2, res);
	SET_OV(0);
	CHECK_ZS(res);
}

uint32_t v810_device::opADDF(uint32_t op)
{
	fp_arith(op, FP_ADD);
	return 24;
}

uint32_t v810_device::opSUBF(uint32_t op)
{
	fp_arith(op, FP_SUB);
	return 26;
}

uint32_t v810_device::opMULF(uint32_t op)
{
	fp_arith(op, FP_MUL);
	return 27;
}

uint32_t v810_device::opDIVF(uint32_t op)
{
	fp_arith(op, FP_DIV);
	return 44;
}

uint32_t v810_device::opTRNC(uint32_t op)
{
	fp_to_int(op, true);
	return 14;
}

uint32_t v810_device::opCVTS(uint32_t op) // CVT.SW
{
	fp_to_int(op, false);
	return 14;
}

uint32_t v810_device::opCMPF(uint32_t op)
{
	const uint32_t op1 = GETREG(GET1);
	const uint32_t op2 = GETREG(GET2);
	if (fp_reserved(op1) || fp_reserved(op2))
	{
		fp_exception(PSW_FRO, 0xff60);
		return 10;
	}
	const float val1 = u2f(op1);
	const float val2 = u2f(op2);
	SET_OV(0);
	SET_Z((val2 == val1) ? 1 : 0);
	SET_S((val2 < val1) ? 1 : 0);
	SET_CY((val2 < val1) ? 1 : 0);
	return 10;
}

uint32_t v810_device::opCVTW(uint32_t op) // CVT.WS
{
	const softfloat_scope fpstate;
	const uint32_t res = i32_to_f32(int32_t(GETREG(GET1))).v;
	if (softfloat_exceptionFlags & softfloat_flag_inexact)
		PSW |= PSW_FPR;
	SETREG(GET2, res);
	fp_set_flags(res);
	return 16;
}

// format VII with the NVC's sub-opcodes XB, XH, REV and MPYHW (Virtual Boy Development Manual 4-6-1)
uint32_t nintendo_nvc_device::opFpoint(uint32_t op)
{
	const u8 sub = BIT(R_OP(PC), 10, 6);
	PC += 2;
	switch (sub)
	{
		case 0x8:
			return opXB(op);
		case 0x9:
			return opXH(op);
		case 0xa:
			return opREV(op);
		case 0xc:
			return opMPYHW(op);
		default:
			return fp_execute(op, sub);
	}
}

uint32_t nintendo_nvc_device::opXB(uint32_t op)
{
	int val=GETREG(GET2);
	val = (val & 0xffff0000) | swapendian_int16(val & 0xffff);
	// TODO: verify flags really being unaffected
	//SET_OV(0);
	//SET_Z((val==0.0f)?1:0);
	//SET_S((val<0.0f)?1:0);
	SETREG(GET2,val);
	// TODO: unknown
	return 1;
}


uint32_t nintendo_nvc_device::opXH(uint32_t op)
{
	int val=GETREG(GET2);
	val = ((val & 0xffff0000)>>16) | ((val & 0xffff)<<16);
	// TODO: verify flags really being unaffected
	//SET_OV(0);
	//SET_Z((val==0.0f)?1:0);
	//SET_S((val<0.0f)?1:0);
	SETREG(GET2,val);
	// TODO: unknown
	return 1;
}

uint32_t nintendo_nvc_device::opMPYHW(uint32_t op)
{
	s16 val1 = (s16)GETREG(GET1);
	s16 val2 = (s16)GETREG(GET2);
	s32 result = (s32)(val1 * val2);
	// TODO: verify flags really being unaffected
	//SET_OV(0);
	//SET_Z((result == 0) ? 1 : 0);
	//SET_S((result < 0) ? 1 : 0);
	SETREG(GET2,result);
	return 9;
}

uint32_t nintendo_nvc_device::opREV(uint32_t op)
{
	uint32_t val = GETREG(GET1);
	val = ((val >> 1) & 0x55555555) | ((val & 0x55555555) << 1);
	val = ((val >> 2) & 0x33333333) | ((val & 0x33333333) << 2);
	val = ((val >> 4) & 0x0f0f0f0f) | ((val & 0x0f0f0f0f) << 4);
	val = swapendian_int32(val);
	SETREG(GET2, val);
	return 1;
}

// format VII: the sub-opcode is in bits 15-10 of the second halfword
uint32_t v810_device::opFpoint(uint32_t op)
{
	const u8 sub = BIT(R_OP(PC), 10, 6);
	PC += 2;
	return fp_execute(op, sub);
}

uint32_t v810_device::fp_execute(uint32_t op, u8 sub)
{
	switch (sub)
	{
		case 0x0:
			return opCMPF(op);
		case 0x2:
			return opCVTW(op);
		case 0x3:
			return opCVTS(op);
		case 0x4:
			return opADDF(op);
		case 0x5:
			return opSUBF(op);
		case 0x6:
			return opMULF(op);
		case 0x7:
			return opDIVF(op);
		case 0xb:
			return opTRNC(op);
		default:
			return opUNDEF(op);
	}
}

// cycles of one memory access by a bit string instruction: a 2 cycle bus cycle, plus the wait states with bus timing
inline uint32_t v810_device::bs_access(offs_t addr, int access)
{
	return m_bus_timing ? (2 + m_bus_wait(addr, access, 4)) : 2;
}

// the bit string instructions stop between words when an interrupt can be accepted or the timeslice
// is used up: the registers then hold the resume state and the instruction is executed again
inline bool v810_device::bs_suspend(uint32_t cycles) const
{
	return (m_nmi_pending && !GET_NP) || irq_acceptable() || (m_icount <= int(cycles));
}

// SCH0BSU/SCH0BSD/SCH1BSU/SCH1BSD (sub-opcodes 0-3): search the string at r30/r27, length r28,
// upward or downward (bit 0) for a 0 or a 1 (bit 1). Each word is read once. r29 counts the bits
// skipped. When the bit is found, r30/r27 point to the bit before it (in the search direction) and
// Z is cleared; when it is not found, r28 is 0 and Z is set.
// Cycles follow Table 5-12 of the architecture manual (32-bit bus): 3 per word, plus a fixed part
// that matches the one word cases and lies within the range given for longer strings.
uint32_t v810_device::opBSU_search(uint32_t op)
{
	const bool down = BIT(op, 0);
	const uint32_t match = BIT(op, 1);
	uint32_t bit = R27 & 0x1f;
	uint32_t src = R30 & ~3;

	// a zero length sets Z and leaves r27-r30 unchanged
	if (!R28)
	{
		SET_Z(1);
		return down ? 15 : 13;
	}

	uint32_t cycles = 0;
	uint32_t words = 0;
	while (true)
	{
		const uint32_t data = R_W(src);
		cycles += bs_access(src, BUS_READ) + 1;
		words++;

		bool next_word = false;
		while (R28 && !next_word)
		{
			if (BIT(data, bit) == match)
			{
				// found: step back one bit
				if (down)
				{
					if (++bit == 32)
					{
						bit = 0;
						src += 4;
					}
				}
				else
				{
					if (bit-- == 0)
					{
						bit = 31;
						src -= 4;
					}
				}
				R27 = bit;
				R30 = src;
				SET_Z(0);
				return cycles + (down ? 23 : 26) + ((words > 1) ? (down ? 17 : 9) : 0);
			}

			// skipped
			if (down)
			{
				if (bit-- == 0)
				{
					bit = 31;
					src -= 4;
					next_word = true;
				}
			}
			else
			{
				if (++bit == 32)
				{
					bit = 0;
					src += 4;
					next_word = true;
				}
			}
			R28--;
			R29++;
		}

		R27 = bit;
		R30 = src;
		if (!R28)
		{
			// not found
			SET_Z(1);
			return cycles + (down ? 25 : 26) + ((words > 1) ? (down ? 17 : 9) : 0);
		}
		if (bs_suspend(cycles))
		{
			PC -= 2;
			return cycles;
		}
	}
}

// arithmetic bit string instructions (sub-opcodes 8-Fh): ORBSU, ANDBSU, XORBSU, MOVBSU and their
// NOT source forms. The source (r30/r27) is combined into the destination (r29/r26) upward, r28
// bits; each destination word is read and written once, and each source word read once.
// Cycles follow Table 5-13 of the architecture manual (32-bit bus): 6 per word (three accesses),
// plus a fixed part that matches the one word cases and lies within the range given for longer strings.
uint32_t v810_device::opBSU(uint32_t op)
{
	// sub-opcodes 04h-07h and 10h-1Fh are invalid
	if ((op & 0x10) || (op & 0x0c) == 0x04)
		return opUNDEF(op);
	if (!(op & 8))
		return opBSU_search(op);

	if (!R28)
	{
		// zero length: nothing is transferred, the offsets and addresses are only masked
		R26 &= 0x1f;
		R27 &= 0x1f;
		R29 &= ~3;
		R30 &= ~3;
		return 14;
	}

	const uint32_t invert = BIT(op, 2);
	uint32_t dstbit = R26 & 0x1f;
	uint32_t srcbit = R27 & 0x1f;
	uint32_t dst = R29 & ~3;
	uint32_t src = R30 & ~3;
	uint32_t srcword = 0;
	bool src_loaded = false;
	uint32_t cycles = 0;
	uint32_t words = 0;

	while (true)
	{
		if (!src_loaded)
		{
			srcword = R_W(src);
			cycles += bs_access(src, BUS_READ);
			src_loaded = true;
		}
		uint32_t dstword = R_W(dst);
		cycles += bs_access(dst, BUS_READ);

		do
		{
			if (!src_loaded)
			{
				// the source moves into the next word part way through the destination word
				srcword = (src == dst) ? dstword : R_W(src);
				cycles += bs_access(src, BUS_READ);
				src_loaded = true;
			}

			const uint32_t s = BIT(srcword, srcbit) ^ invert;
			switch (op & 3)
			{
				case 0: // OR
					dstword |= s << dstbit;
					break;
				case 1: // AND
					dstword &= ~((s ^ 1) << dstbit);
					break;
				case 2: // XOR
					dstword ^= s << dstbit;
					break;
				case 3: // MOV
					dstword = (dstword & ~(1U << dstbit)) | (s << dstbit);
					break;
			}
			// source and destination in the same word: the source sees the bits already written
			if (src == dst)
				srcword = dstword;

			R28--;
			dstbit = (dstbit + 1) & 0x1f;
			srcbit = (srcbit + 1) & 0x1f;
			if (!srcbit)
			{
				src += 4;
				src_loaded = false;
			}
		} while (R28 && dstbit);

		W_W(dst, dstword);
		cycles += bs_access(dst, BUS_WRITE);
		words++;
		if (!dstbit)
			dst += 4;

		R26 = dstbit;
		R27 = srcbit;
		R29 = dst;
		R30 = src;
		if (!R28)
			return cycles + 26 + ((words > 1) ? 3 : 0);
		if (bs_suspend(cycles))
		{
			PC -= 2;
			return cycles;
		}
	}
}

constexpr v810_device::opcode_table v810_device::s_opcodes =
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
	/* 0x16 */ &v810_device::opUNDEF,
	/* 0x17 */ &v810_device::opSARi,      // sar imm5,r2          2
	/* 0x18 */ &v810_device::opTRAP,
	/* 0x19 */ &v810_device::opRETI,
	/* 0x1a */ &v810_device::opHALT,      // halt             2
	/* 0x1b */ &v810_device::opUNDEF,
	/* 0x1c */ &v810_device::opLDSR,      // ldsr reg2,regID          2
	/* 0x1d */ &v810_device::opSTSR,      // stsr regID,reg2          2
	/* 0x1e */ &v810_device::opUNDEF,
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

// the V810 table with the NVC's CLI and SEI, and its format VII sub-opcodes
constexpr nintendo_nvc_device::opcode_table nintendo_nvc_device::make_opcodes()
{
	opcode_table opcodes = s_opcodes;
	opcodes[0x16] = static_cast<opcode_func>(&nintendo_nvc_device::opCLI);
	opcodes[0x1e] = static_cast<opcode_func>(&nintendo_nvc_device::opSEI);
	opcodes[0x3e] = static_cast<opcode_func>(&nintendo_nvc_device::opFpoint);
	return opcodes;
}

constexpr nintendo_nvc_device::opcode_table nintendo_nvc_device::s_nvc_opcodes = make_opcodes();

void v810_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(has_space(AS_IO) ? AS_IO : AS_PROGRAM).specific(m_io);

	m_irq_state = 0;
	m_nmi_line = CLEAR_LINE;
	m_nmi_pending = false;
	memset(m_reg, 0x00, sizeof(m_reg));

	save_item(NAME(m_reg));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_nmi_line));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_PPC));
	save_item(NAME(m_halted));
	save_item(NAME(m_halt));

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
	state_add(STATE_GENPCBASE, "CURPC", m_PPC).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", PSW).formatstr("%8s").noshow();

	m_bus_wait.resolve();
	m_bus_timing = !m_bus_wait.isnull();
	m_lastop = m_curop = LAST_OTHER;
	icache_clear(0, 128);
	save_item(NAME(m_lastop));
	save_item(NAME(m_icache_tag));
	save_item(NAME(m_icache_valid));

	set_icountptr(m_icount);
}

void v810_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
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
	// everything else is "Undefind" (sic)
	m_reg[0] = 0;
	PC = 0xfffffff0;
	PSW = 0x00008000;
	ECR = 0x0000fff0;
	PIR = m_pir;
	TKCW = 0x000000e0;                     // fixed: traps on invalid operation, zero division and overflow
	CHCW = 0;
	m_halted = false;
	m_halt = false;
	m_nmi_pending = false;
	m_lastop = LAST_OTHER;
	icache_clear(0, 128);
}

// an interrupt that would be accepted now: requested, not masked by NP, EP or ID, and at or above the PSW level
inline bool v810_device::irq_acceptable() const
{
	if (!m_irq_state || GET_NP || GET_EP || GET_ID)
		return false;
	return (31 - std::countl_zero(uint32_t(m_irq_state))) >= int(BIT(PSW, 16, 4));
}

// NMI: masked only by NP, and saved in FEPC/FEPSW like a duplexed exception
void v810_device::take_nmi()
{
	m_nmi_pending = false;
	m_halt = false;

	standard_irq_callback(INPUT_LINE_NMI, PC);

	FEPC = PC;
	FEPSW = PSW;
	ECR = (ECR & 0x0000ffff) | 0xffd00000;
	SET_NP(1);
	SET_ID(1);
	SET_AE(0);
	PC = 0xffffffd0;

	m_icount -= clkIRQ;
}

// called with an interrupt requested and accepted by NP, EP and ID: takes the highest one above the PSW level
void v810_device::check_interrupts()
{
	for (int irq_line = 15; irq_line >= int(BIT(PSW, 16, 4)); irq_line--)
	{
		if (!((1 << irq_line) & m_irq_state))
			continue;

		m_halt = false;

		standard_irq_callback(irq_line, PC);

		EIPC = PC;
		EIPSW = PSW;

		PC = 0xfffffe00 | (irq_line << 4);
		ECR = (ECR & 0xffff0000) | 0xfe00 | (irq_line << 4); // EICC only

		const uint8_t num = std::min(irq_line + 1, 15);

		// clear interrupt level
		PSW &= 0xfff0ffff;
		SET_EP(1);
		SET_ID(1);
		SET_AE(0);
		PSW |= num << 16;

		m_icount -= clkIRQ;
		return;
	}
}

void v810_device::execute_run()
{
	if (m_halted)
	{
		// fatal exception: nothing runs until reset
		m_icount = 0;
		return;
	}

	do
	{
		// interrupts are sampled at the end of every instruction, and while halted
		if (m_nmi_pending && !GET_NP)
			take_nmi();
		else if (irq_acceptable())
			check_interrupts();
		if (m_halt)
		{
			debugger_wait_hook();
			m_icount = 0;
			break;
		}

		m_PPC = PC;
		if (GET_AE && (PC == ADTRE))
		{
			// address trap: taken before the instruction executes, which it restores to
			take_exception(0xffffffc0, 0xffc0);
			m_icount -= clkIRQ;
			continue;
		}
		debugger_instruction_hook(PC);
		m_curop = LAST_OTHER;
		uint32_t op = fetch_op(PC, 0);
		PC += 2;

		int cnt = (this->*m_opcodes[op >> 10])(op);
		m_icount -= cnt;
		m_lastop = m_curop;

	} while(m_icount > 0);
}


// TODO: logically connects to /INTV0-/INTV3 pins
// v810 just exposes an INT and 4 V(ector?) lines.
// - on pcfx there's an unknown chip irq priority/mask dispatcher;
// - on vboy it's implied in the NVC specs with a laconic "Interrupt Encoder".
//   It's also 5 possible lines, which wouldn't work bitwise;
void v810_device::execute_set_input(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		// edge triggered: a request is held until it is accepted
		if (state == ASSERT_LINE && m_nmi_line == CLEAR_LINE)
			m_nmi_pending = true;
		m_nmi_line = state;
		return;
	}

	const u16 mask = 1 << irqline;
	if (state == ASSERT_LINE)
		m_irq_state |= mask;
	else
		m_irq_state &= ~mask;
}
