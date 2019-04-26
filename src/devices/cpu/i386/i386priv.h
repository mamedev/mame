// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
#pragma once

#ifndef __I386_H__
#define __I386_H__

#include "i386dasm.h"

//#define DEBUG_MISSING_OPCODE

#define I386OP(XX)      i386_##XX
#define I486OP(XX)      i486_##XX
#define PENTIUMOP(XX)   pentium_##XX
#define MMXOP(XX)       mmx_##XX
#define SSEOP(XX)       sse_##XX

enum SREGS { ES, CS, SS, DS, FS, GS };

enum BREGS
{
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0,3),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(1,2),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(4,7),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(5,6),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(8,11),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(9,10),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(12,15),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(13,14)
};

enum WREGS
{
	AX = NATIVE_ENDIAN_VALUE_LE_BE(0,1),
	CX = NATIVE_ENDIAN_VALUE_LE_BE(2,3),
	DX = NATIVE_ENDIAN_VALUE_LE_BE(4,5),
	BX = NATIVE_ENDIAN_VALUE_LE_BE(6,7),
	SP = NATIVE_ENDIAN_VALUE_LE_BE(8,9),
	BP = NATIVE_ENDIAN_VALUE_LE_BE(10,11),
	SI = NATIVE_ENDIAN_VALUE_LE_BE(12,13),
	DI = NATIVE_ENDIAN_VALUE_LE_BE(14,15)
};

enum DREGS { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI };

enum
{
	I386_PC = 0,

	/* 8-bit registers */
	I386_AL,
	I386_AH,
	I386_BL,
	I386_BH,
	I386_CL,
	I386_CH,
	I386_DL,
	I386_DH,

	/* 16-bit registers */
	I386_AX,
	I386_BX,
	I386_CX,
	I386_DX,
	I386_BP,
	I386_SP,
	I386_SI,
	I386_DI,
	I386_IP,

	/* 32-bit registers */
	I386_EAX,
	I386_ECX,
	I386_EDX,
	I386_EBX,
	I386_EBP,
	I386_ESP,
	I386_ESI,
	I386_EDI,
	I386_EIP,

	/* segment registers */
	I386_CS,
	I386_CS_BASE,
	I386_CS_LIMIT,
	I386_CS_FLAGS,
	I386_SS,
	I386_SS_BASE,
	I386_SS_LIMIT,
	I386_SS_FLAGS,
	I386_DS,
	I386_DS_BASE,
	I386_DS_LIMIT,
	I386_DS_FLAGS,
	I386_ES,
	I386_ES_BASE,
	I386_ES_LIMIT,
	I386_ES_FLAGS,
	I386_FS,
	I386_FS_BASE,
	I386_FS_LIMIT,
	I386_FS_FLAGS,
	I386_GS,
	I386_GS_BASE,
	I386_GS_LIMIT,
	I386_GS_FLAGS,

	/* other */
	I386_EFLAGS,

	I386_CR0,
	I386_CR1,
	I386_CR2,
	I386_CR3,
	I386_CR4,

	I386_DR0,
	I386_DR1,
	I386_DR2,
	I386_DR3,
	I386_DR4,
	I386_DR5,
	I386_DR6,
	I386_DR7,

	I386_TR6,
	I386_TR7,

	I386_GDTR_BASE,
	I386_GDTR_LIMIT,
	I386_IDTR_BASE,
	I386_IDTR_LIMIT,
	I386_TR,
	I386_TR_BASE,
	I386_TR_LIMIT,
	I386_TR_FLAGS,
	I386_LDTR,
	I386_LDTR_BASE,
	I386_LDTR_LIMIT,
	I386_LDTR_FLAGS,

	I386_CPL,

	X87_CTRL,
	X87_STATUS,
	X87_TAG,
	X87_ST0,
	X87_ST1,
	X87_ST2,
	X87_ST3,
	X87_ST4,
	X87_ST5,
	X87_ST6,
	X87_ST7,

	SSE_XMM0,
	SSE_XMM1,
	SSE_XMM2,
	SSE_XMM3,
	SSE_XMM4,
	SSE_XMM5,
	SSE_XMM6,
	SSE_XMM7
};

enum
{
	/* mmx registers aliased to x87 ones */
	MMX_MM0 = X87_ST0,
	MMX_MM1 = X87_ST1,
	MMX_MM2 = X87_ST2,
	MMX_MM3 = X87_ST3,
	MMX_MM4 = X87_ST4,
	MMX_MM5 = X87_ST5,
	MMX_MM6 = X87_ST6,
	MMX_MM7 = X87_ST7
};

enum smram
{
	SMRAM_SMBASE = 0xF8,
	SMRAM_SMREV  = 0xFC,
	SMRAM_IORSRT = 0x100,
	SMRAM_AHALT  = 0x102,
	SMRAM_IOEDI  = 0x104,
	SMRAM_IOECX  = 0x108,
	SMRAM_IOESI  = 0x10C,

	SMRAM_ES     = 0x1A8,
	SMRAM_CS     = 0x1AC,
	SMRAM_SS     = 0x1B0,
	SMRAM_DS     = 0x1B4,
	SMRAM_FS     = 0x1B8,
	SMRAM_GS     = 0x1BC,
	SMRAM_LDTR   = 0x1C0,
	SMRAM_TR     = 0x1C4,
	SMRAM_DR7    = 0x1C8,
	SMRAM_DR6    = 0x1CC,
	SMRAM_EAX    = 0x1D0,
	SMRAM_ECX    = 0x1D4,
	SMRAM_EDX    = 0x1D8,
	SMRAM_EBX    = 0x1DC,
	SMRAM_ESP    = 0x1E0,
	SMRAM_EBP    = 0x1E4,
	SMRAM_ESI    = 0x1E8,
	SMRAM_EDI    = 0x1EC,
	SMRAM_EIP    = 0x1F0,
	SMRAM_EFLAGS = 0x1F4,
	SMRAM_CR3    = 0x1F8,
	SMRAM_CR0    = 0x1FC
};

enum smram_intel_p5
{
	SMRAM_IP5_IOEIP   = 0x110,
	SMRAM_IP5_CR4     = 0x128,
	SMRAM_IP5_ESLIM   = 0x130,
	SMRAM_IP5_ESBASE  = 0x134,
	SMRAM_IP5_ESACC   = 0x138,
	SMRAM_IP5_CSLIM   = 0x13C,
	SMRAM_IP5_CSBASE  = 0x140,
	SMRAM_IP5_CSACC   = 0x144,
	SMRAM_IP5_SSLIM   = 0x148,
	SMRAM_IP5_SSBASE  = 0x14C,
	SMRAM_IP5_SSACC   = 0x150,
	SMRAM_IP5_DSLIM   = 0x154,
	SMRAM_IP5_DSBASE  = 0x158,
	SMRAM_IP5_DSACC   = 0x15C,
	SMRAM_IP5_FSLIM   = 0x160,
	SMRAM_IP5_FSBASE  = 0x164,
	SMRAM_IP5_FSACC   = 0x168,
	SMRAM_IP5_GSLIM   = 0x16C,
	SMRAM_IP5_GSBASE  = 0x170,
	SMRAM_IP5_GSACC   = 0x174,
	SMRAM_IP5_LDTLIM  = 0x178,
	SMRAM_IP5_LDTBASE = 0x17C,
	SMRAM_IP5_LDTACC  = 0x180,
	SMRAM_IP5_GDTLIM  = 0x184,
	SMRAM_IP5_GDTBASE = 0x188,
	SMRAM_IP5_GDTACC  = 0x18C,
	SMRAM_IP5_IDTLIM  = 0x190,
	SMRAM_IP5_IDTBASE = 0x194,
	SMRAM_IP5_IDTACC  = 0x198,
	SMRAM_IP5_TRLIM   = 0x19C,
	SMRAM_IP5_TRBASE  = 0x1A0,
	SMRAM_IP5_TRACC   = 0x1A4
};

/* Protected mode exceptions */
enum pm_faults
{
	FAULT_UD = 6,  // Invalid Opcode
	FAULT_NM = 7,  // Coprocessor not available
	FAULT_DF = 8,  // Double Fault
	FAULT_TS = 10, // Invalid TSS
	FAULT_NP = 11, // Segment or Gate not present
	FAULT_SS = 12, // Stack fault
	FAULT_GP = 13, // General Protection Fault
	FAULT_PF = 14, // Page Fault
	FAULT_MF = 16  // Match (Coprocessor) Fault
};

/* MXCSR Control and Status Register */
enum mxcsr_bits
{
	MXCSR_IE  = 1 << 0,  // Invalid Operation Flag
	MXCSR_DE  = 1 << 1,  // Denormal Flag
	MXCSR_ZE  = 1 << 2,  // Divide-by-Zero Flag
	MXCSR_OE  = 1 << 3,  // Overflow Flag
	MXCSR_UE  = 1 << 4,  // Underflow Flag
	MXCSR_PE  = 1 << 5,  // Precision Flag
	MXCSR_DAZ = 1 << 6,  // Denormals Are Zeros
	MXCSR_IM  = 1 << 7,  // Invalid Operation Mask
	MXCSR_DM  = 1 << 8,  // Denormal Operation Mask
	MXCSR_ZM  = 1 << 9,  // Divide-by-Zero Mask
	MXCSR_OM  = 1 << 10, // Overflow Mask
	MXCSR_UM  = 1 << 11, // Underflow Mask
	MXCSR_PM  = 1 << 12, // Precision Mask
	MXCSR_RC  = 3 << 13, // Rounding Control
	MXCSR_FZ  = 1 << 15  // Flush to Zero
};

union MMX_REG {
	uint32_t d[2];
	int32_t  i[2];
	uint16_t w[4];
	int16_t  s[4];
	uint8_t  b[8];
	int8_t   c[8];
	float    f[2];
	uint64_t q;
	int64_t  l;
};

extern int i386_parity_table[256];

#define FAULT_THROW(fault,error) { throw (uint64_t)(fault | (uint64_t)error << 32); }
#define PF_THROW(error) { m_cr[2] = address; FAULT_THROW(FAULT_PF,error); }

#define PROTECTED_MODE      (m_cr[0] & 0x1)
#define STACK_32BIT         (m_sreg[SS].d)
#define V8086_MODE          (m_VM)
#define NESTED_TASK         (m_NT)
#define WP                  (m_cr[0] & 0x10000)

#define SetOF_Add32(r,s,d)  (m_OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80000000) ? 1: 0)
#define SetOF_Add16(r,s,d)  (m_OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? 1 : 0)
#define SetOF_Add8(r,s,d)   (m_OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? 1 : 0)

#define SetOF_Sub32(r,s,d)  (m_OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80000000) ? 1 : 0)
#define SetOF_Sub16(r,s,d)  (m_OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? 1 : 0)
#define SetOF_Sub8(r,s,d)   (m_OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? 1 : 0)

#define SetCF8(x)           {m_CF = ((x) & 0x100) ? 1 : 0; }
#define SetCF16(x)          {m_CF = ((x) & 0x10000) ? 1 : 0; }
#define SetCF32(x)          {m_CF = ((x) & (((uint64_t)1) << 32)) ? 1 : 0; }

#define SetSF(x)            (m_SF = (x))
#define SetZF(x)            (m_ZF = (x))
#define SetAF(x,y,z)        (m_AF = (((x) ^ ((y) ^ (z))) & 0x10) ? 1 : 0)
#define SetPF(x)            (m_PF = i386_parity_table[(x) & 0xFF])

#define SetSZPF8(x)         {m_ZF = ((uint8_t)(x)==0);  m_SF = ((x)&0x80) ? 1 : 0; m_PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF16(x)        {m_ZF = ((uint16_t)(x)==0);  m_SF = ((x)&0x8000) ? 1 : 0; m_PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF32(x)        {m_ZF = ((uint32_t)(x)==0);  m_SF = ((x)&0x80000000) ? 1 : 0; m_PF = i386_parity_table[x & 0xFF]; }

#define MMX(n)              (*((MMX_REG *)(&m_x87_reg[(n)].low)))
#define XMM(n)              m_sse_reg[(n)]

#define VTLB_FLAG_DIRTY     0x100
#define CYCLES_NUM(x)       (m_cycles -= (x))

#define FAULT(fault,error)  {m_ext = 1; i386_trap_with_error(fault,0,0,error); return;}
#define FAULT_EXP(fault,error) {m_ext = 1; i386_trap_with_error(fault,0,trap_level+1,error); return;}

/***********************************************************************************/

struct MODRM_TABLE {
	struct {
		int b;
		int w;
		int d;
	} reg;
	struct {
		int b;
		int w;
		int d;
	} rm;
};

extern MODRM_TABLE i386_MODRM_table[256];

#define REG8(x)         (m_reg.b[x])
#define REG16(x)        (m_reg.w[x])
#define REG32(x)        (m_reg.d[x])

#define LOAD_REG8(x)    (REG8(i386_MODRM_table[x].reg.b))
#define LOAD_REG16(x)   (REG16(i386_MODRM_table[x].reg.w))
#define LOAD_REG32(x)   (REG32(i386_MODRM_table[x].reg.d))
#define LOAD_RM8(x)     (REG8(i386_MODRM_table[x].rm.b))
#define LOAD_RM16(x)    (REG16(i386_MODRM_table[x].rm.w))
#define LOAD_RM32(x)    (REG32(i386_MODRM_table[x].rm.d))

#define STORE_REG8(x, value)    (REG8(i386_MODRM_table[x].reg.b) = value)
#define STORE_REG16(x, value)   (REG16(i386_MODRM_table[x].reg.w) = value)
#define STORE_REG32(x, value)   (REG32(i386_MODRM_table[x].reg.d) = value)
#define STORE_RM8(x, value)     (REG8(i386_MODRM_table[x].rm.b) = value)
#define STORE_RM16(x, value)    (REG16(i386_MODRM_table[x].rm.w) = value)
#define STORE_RM32(x, value)    (REG32(i386_MODRM_table[x].rm.d) = value)

#define SWITCH_ENDIAN_32(x) (((((x) << 24) & (0xff << 24)) | (((x) << 8) & (0xff << 16)) | (((x) >> 8) & (0xff << 8)) | (((x) >> 24) & (0xff << 0))))

/***********************************************************************************/

void i386_device::CHANGE_PC(uint32_t pc)
{
	m_pc = i386_translate(CS, pc, -1 );
}

void i386_device::NEAR_BRANCH(int32_t offs)
{
	/* TODO: limit */
	m_eip += offs;
	m_pc += offs;
}

uint8_t i386_device::FETCH()
{
	uint8_t value;
	uint32_t address = m_pc, error;

	if(!translate_address(m_CPL,TRANSLATE_FETCH,&address,&error))
		PF_THROW(error);

	value = mem_pr8(address & m_a20_mask);
#ifdef DEBUG_MISSING_OPCODE
	m_opcode_bytes[m_opcode_bytes_length] = value;
	m_opcode_bytes_length = (m_opcode_bytes_length + 1) & 15;
#endif
	m_eip++;
	m_pc++;
	return value;
}
uint16_t i386_device::FETCH16()
{
	uint16_t value;
	uint32_t address = m_pc, error;

	if( !WORD_ALIGNED(address) ) {       /* Unaligned read */
		value = (FETCH() << 0);
		value |= (FETCH() << 8);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);
		address &= m_a20_mask;
		value = mem_pr16(address);
		m_eip += 2;
		m_pc += 2;
	}
	return value;
}
uint32_t i386_device::FETCH32()
{
	uint32_t value;
	uint32_t address = m_pc, error;

	if( !DWORD_ALIGNED(m_pc) ) {      /* Unaligned read */
		value = (FETCH() << 0);
		value |= (FETCH() << 8);
		value |= (FETCH() << 16);
		value |= (FETCH() << 24);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = mem_pr32(address);
		m_eip += 4;
		m_pc += 4;
	}
	return value;
}

uint8_t i386_device::READ8(uint32_t ea)
{
	uint32_t address = ea, error;

	if(!translate_address(m_CPL,TRANSLATE_READ,&address, &error))
		PF_THROW(error);

	address &= m_a20_mask;
	return mem_prd8(address);
}
uint16_t i386_device::READ16(uint32_t ea)
{
	uint16_t value;
	uint32_t address = ea, error;

	if( !WORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8( address+0 ) << 0);
		value |= (READ8( address+1 ) << 8);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = mem_prd16( address );
	}
	return value;
}
uint32_t i386_device::READ32(uint32_t ea)
{
	uint32_t value;
	uint32_t address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8( address+0 ) << 0);
		value |= (READ8( address+1 ) << 8);
		value |= (READ8( address+2 ) << 16),
		value |= (READ8( address+3 ) << 24);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = mem_prd32( address );
	}
	return value;
}

uint64_t i386_device::READ64(uint32_t ea)
{
	uint64_t value;
	uint32_t address = ea, error;

	if( !QWORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (((uint64_t) READ8( address+0 )) << 0);
		value |= (((uint64_t) READ8( address+1 )) << 8);
		value |= (((uint64_t) READ8( address+2 )) << 16);
		value |= (((uint64_t) READ8( address+3 )) << 24);
		value |= (((uint64_t) READ8( address+4 )) << 32);
		value |= (((uint64_t) READ8( address+5 )) << 40);
		value |= (((uint64_t) READ8( address+6 )) << 48);
		value |= (((uint64_t) READ8( address+7 )) << 56);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = (((uint64_t) mem_prd32( address+0 )) << 0);
		value |= (((uint64_t) mem_prd32( address+4 )) << 32);
	}
	return value;
}
uint8_t i386_device::READ8PL0(uint32_t ea)
{
	uint32_t address = ea, error;

	if(!translate_address(0,TRANSLATE_READ,&address,&error))
		PF_THROW(error);

	address &= m_a20_mask;
	return mem_prd8(address);
}
uint16_t i386_device::READ16PL0(uint32_t ea)
{
	uint16_t value;
	uint32_t address = ea, error;

	if( !WORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8PL0( address+0 ) << 0);
		value |= (READ8PL0( address+1 ) << 8);
	} else {
		if(!translate_address(0,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = mem_prd16( address );
	}
	return value;
}

uint32_t i386_device::READ32PL0(uint32_t ea)
{
	uint32_t value;
	uint32_t address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8PL0( address+0 ) << 0);
		value |= (READ8PL0( address+1 ) << 8);
		value |= (READ8PL0( address+2 ) << 16);
		value |= (READ8PL0( address+3 ) << 24);
	} else {
		if(!translate_address(0,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = mem_prd32( address );
	}
	return value;
}

void i386_device::WRITE_TEST(uint32_t ea)
{
	uint32_t address = ea, error;
	if(!translate_address(m_CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);
}

void i386_device::WRITE8(uint32_t ea, uint8_t value)
{
	uint32_t address = ea, error;

	if(!translate_address(m_CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);

	address &= m_a20_mask;
	mem_pwd8(address, value);
}
void i386_device::WRITE16(uint32_t ea, uint16_t value)
{
	uint32_t address = ea, error;

	if( !WORD_ALIGNED(ea) ) {        /* Unaligned write */
		WRITE8( address+0, value & 0xff );
		WRITE8( address+1, (value >> 8) & 0xff );
	} else {
		if(!translate_address(m_CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		mem_pwd16(address, value);
	}
}
void i386_device::WRITE32(uint32_t ea, uint32_t value)
{
	uint32_t address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {        /* Unaligned write */
		WRITE8( address+0, value & 0xff );
		WRITE8( address+1, (value >> 8) & 0xff );
		WRITE8( address+2, (value >> 16) & 0xff );
		WRITE8( address+3, (value >> 24) & 0xff );
	} else {
		if(!translate_address(m_CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		ea &= m_a20_mask;
		mem_pwd32(address, value);
	}
}

void i386_device::WRITE64(uint32_t ea, uint64_t value)
{
	uint32_t address = ea, error;

	if( !QWORD_ALIGNED(ea) ) {        /* Unaligned write */
		WRITE8( address+0, value & 0xff );
		WRITE8( address+1, (value >> 8) & 0xff );
		WRITE8( address+2, (value >> 16) & 0xff );
		WRITE8( address+3, (value >> 24) & 0xff );
		WRITE8( address+4, (value >> 32) & 0xff );
		WRITE8( address+5, (value >> 40) & 0xff );
		WRITE8( address+6, (value >> 48) & 0xff );
		WRITE8( address+7, (value >> 56) & 0xff );
	} else {
		if(!translate_address(m_CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		ea &= m_a20_mask;
		mem_pwd32(address+0, value & 0xffffffff);
		mem_pwd32(address+4, (value >> 32) & 0xffffffff);
	}
}

/***********************************************************************************/

uint8_t i386_device::OR8(uint8_t dst, uint8_t src)
{
	uint8_t res = dst | src;
	m_CF = m_OF = 0;
	SetSZPF8(res);
	return res;
}
uint16_t i386_device::OR16(uint16_t dst, uint16_t src)
{
	uint16_t res = dst | src;
	m_CF = m_OF = 0;
	SetSZPF16(res);
	return res;
}
uint32_t i386_device::OR32(uint32_t dst, uint32_t src)
{
	uint32_t res = dst | src;
	m_CF = m_OF = 0;
	SetSZPF32(res);
	return res;
}

uint8_t i386_device::AND8(uint8_t dst, uint8_t src)
{
	uint8_t res = dst & src;
	m_CF = m_OF = 0;
	SetSZPF8(res);
	return res;
}
uint16_t i386_device::AND16(uint16_t dst, uint16_t src)
{
	uint16_t res = dst & src;
	m_CF = m_OF = 0;
	SetSZPF16(res);
	return res;
}
uint32_t i386_device::AND32(uint32_t dst, uint32_t src)
{
	uint32_t res = dst & src;
	m_CF = m_OF = 0;
	SetSZPF32(res);
	return res;
}

uint8_t i386_device::XOR8(uint8_t dst, uint8_t src)
{
	uint8_t res = dst ^ src;
	m_CF = m_OF = 0;
	SetSZPF8(res);
	return res;
}
uint16_t i386_device::XOR16(uint16_t dst, uint16_t src)
{
	uint16_t res = dst ^ src;
	m_CF = m_OF = 0;
	SetSZPF16(res);
	return res;
}
uint32_t i386_device::XOR32(uint32_t dst, uint32_t src)
{
	uint32_t res = dst ^ src;
	m_CF = m_OF = 0;
	SetSZPF32(res);
	return res;
}

#define SUB8(dst, src) SBB8(dst, src, 0)
uint8_t i386_device::SBB8(uint8_t dst, uint8_t src, uint8_t b)
{
	uint16_t res = (uint16_t)dst - (uint16_t)src - (uint8_t)b;
	SetCF8(res);
	SetOF_Sub8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}

#define SUB16(dst, src) SBB16(dst, src, 0)
uint16_t i386_device::SBB16(uint16_t dst, uint16_t src, uint16_t b)
{
	uint32_t res = (uint32_t)dst - (uint32_t)src - (uint32_t)b;
	SetCF16(res);
	SetOF_Sub16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}

#define SUB32(dst, src) SBB32(dst, src, 0)
uint32_t i386_device::SBB32(uint32_t dst, uint32_t src, uint32_t b)
{
	uint64_t res = (uint64_t)dst - (uint64_t)src - (uint64_t) b;
	SetCF32(res);
	SetOF_Sub32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}

#define ADD8(dst, src) ADC8(dst, src, 0)
uint8_t i386_device::ADC8(uint8_t dst, uint8_t src, uint8_t c)
{
	uint16_t res = (uint16_t)dst + (uint16_t)src + (uint16_t)c;
	SetCF8(res);
	SetOF_Add8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}

#define ADD16(dst, src) ADC16(dst, src, 0)
uint16_t i386_device::ADC16(uint16_t dst, uint16_t src, uint8_t c)
{
	uint32_t res = (uint32_t)dst + (uint32_t)src + (uint32_t)c;
	SetCF16(res);
	SetOF_Add16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}

#define ADD32(dst, src) ADC32(dst, src, 0)
uint32_t i386_device::ADC32(uint32_t dst, uint32_t src, uint32_t c)
{
	uint64_t res = (uint64_t)dst + (uint64_t)src + (uint64_t) c;
	SetCF32(res);
	SetOF_Add32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}

uint8_t i386_device::INC8(uint8_t dst)
{
	uint16_t res = (uint16_t)dst + 1;
	SetOF_Add8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}
uint16_t i386_device::INC16(uint16_t dst)
{
	uint32_t res = (uint32_t)dst + 1;
	SetOF_Add16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}
uint32_t i386_device::INC32(uint32_t dst)
{
	uint64_t res = (uint64_t)dst + 1;
	SetOF_Add32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}

uint8_t i386_device::DEC8(uint8_t dst)
{
	uint16_t res = (uint16_t)dst - 1;
	SetOF_Sub8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}
uint16_t i386_device::DEC16(uint16_t dst)
{
	uint32_t res = (uint32_t)dst - 1;
	SetOF_Sub16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}
uint32_t i386_device::DEC32(uint32_t dst)
{
	uint64_t res = (uint64_t)dst - 1;
	SetOF_Sub32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}



void i386_device::PUSH16(uint16_t value)
{
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 2;
		ea = i386_translate(SS, new_esp, 1);
		WRITE16(ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 2) & 0xffff;
		ea = i386_translate(SS, new_esp, 1);
		WRITE16(ea, value );
		REG16(SP) = new_esp;
	}
}
void i386_device::PUSH32(uint32_t value)
{
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate(SS, new_esp, 1);
		WRITE32(ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate(SS, new_esp, 1);
		WRITE32(ea, value );
		REG16(SP) = new_esp;
	}
}

void i386_device::PUSH32SEG(uint32_t value)
{
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate(SS, new_esp, 1);
		((m_cpu_version & 0xf00) == 0x300) ? WRITE16(ea, value) : WRITE32(ea, value ); // 486 also?
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate(SS, new_esp, 1);
		((m_cpu_version & 0xf00) == 0x300) ? WRITE16(ea, value) : WRITE32(ea, value );
		REG16(SP) = new_esp;
	}
}

void i386_device::PUSH8(uint8_t value)
{
	if( m_operand_size ) {
		PUSH32((int32_t)(int8_t)value);
	} else {
		PUSH16((int16_t)(int8_t)value);
	}
}

uint8_t i386_device::POP8()
{
	uint8_t value;
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 1;
		ea = i386_translate(SS, new_esp - 1, 0);
		value = READ8(ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 1;
		ea = i386_translate(SS, (new_esp - 1) & 0xffff, 0);
		value = READ8(ea );
		REG16(SP) = new_esp;
	}
	return value;
}
uint16_t i386_device::POP16()
{
	uint16_t value;
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 2;
		ea = i386_translate(SS, new_esp - 2, 0);
		value = READ16(ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 2;
		ea = i386_translate(SS, (new_esp - 2) & 0xffff, 0);
		value = READ16(ea );
		REG16(SP) = new_esp;
	}
	return value;
}
uint32_t i386_device::POP32()
{
	uint32_t value;
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 4;
		ea = i386_translate(SS, new_esp - 4, 0);
		value = READ32(ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 4;
		ea = i386_translate(SS, (new_esp - 4) & 0xffff, 0);
		value = READ32(ea );
		REG16(SP) = new_esp;
	}
	return value;
}

void i386_device::BUMP_SI(int adjustment)
{
	if ( m_address_size )
		REG32(ESI) += ((m_DF) ? -adjustment : +adjustment);
	else
		REG16(SI) += ((m_DF) ? -adjustment : +adjustment);
}

void i386_device::BUMP_DI(int adjustment)
{
	if ( m_address_size )
		REG32(EDI) += ((m_DF) ? -adjustment : +adjustment);
	else
		REG16(DI) += ((m_DF) ? -adjustment : +adjustment);
}

void i386_device::CYCLES(int x)
{
	if (PROTECTED_MODE)
	{
		m_cycles -= m_cycle_table_pm[x];
	}
	else
	{
		m_cycles -= m_cycle_table_rm[x];
	}
}

void i386_device::CYCLES_RM(int modrm, int r, int m)
{
	if (modrm >= 0xc0)
	{
		if (PROTECTED_MODE)
		{
			m_cycles -= m_cycle_table_pm[r];
		}
		else
		{
			m_cycles -= m_cycle_table_rm[r];
		}
	}
	else
	{
		if (PROTECTED_MODE)
		{
			m_cycles -= m_cycle_table_pm[m];
		}
		else
		{
			m_cycles -= m_cycle_table_rm[m];
		}
	}
}



/***********************************************************************************
    I/O ACCESS
***********************************************************************************/

void i386_device::check_ioperm(offs_t port, uint8_t mask)
{
	uint8_t IOPL, map;
	uint16_t IOPB;
	uint32_t address;

	if(!PROTECTED_MODE)
		return;

	IOPL = m_IOP1 | (m_IOP2 << 1);
	if(!V8086_MODE && (m_CPL <= IOPL))
		return;

	if((m_task.limit < 0x67) || ((m_task.flags & 0xd) != 9))
		FAULT_THROW(FAULT_GP,0);

	address = m_task.base;
	IOPB = READ16PL0(address+0x66);
	if((IOPB+(port/8)) > m_task.limit)
		FAULT_THROW(FAULT_GP,0);

	map = READ8PL0(address+IOPB+(port/8));
	map >>= (port%8);
	if(map & mask)
		FAULT_THROW(FAULT_GP,0);
}

uint8_t i386_device::READPORT8(offs_t port)
{
	check_ioperm(port, 1);
	return m_io->read_byte(port);
}

void i386_device::WRITEPORT8(offs_t port, uint8_t value)
{
	check_ioperm(port, 1);
	m_io->write_byte(port, value);
}

uint16_t i386_device::READPORT16(offs_t port)
{
	if (port & 1)
	{
		uint16_t value = READPORT8(port);
		value |= (READPORT8(port + 1) << 8);
		return value;
	}
	else
	{
		check_ioperm(port, 3);
		return m_io->read_word(port);
	}
}

void i386_device::WRITEPORT16(offs_t port, uint16_t value)
{
	if (port & 1)
	{
		WRITEPORT8(port, value & 0xff);
		WRITEPORT8(port + 1, (value >> 8) & 0xff);
	}
	else
	{
		check_ioperm(port, 3);
		m_io->write_word(port, value);
	}
}

uint32_t i386_device::READPORT32(offs_t port)
{
	if (port & 3)
	{
		uint32_t value = READPORT8(port);
		value |= (READPORT8(port + 1) << 8);
		value |= (READPORT8(port + 2) << 16);
		value |= (READPORT8(port + 3) << 24);
		return value;
	}
	else
	{
		check_ioperm(port, 0xf);
		return m_io->read_dword(port);
	}
}

void i386_device::WRITEPORT32(offs_t port, uint32_t value)
{
	if (port & 3)
	{
		WRITEPORT8(port, value & 0xff);
		WRITEPORT8(port + 1, (value >> 8) & 0xff);
		WRITEPORT8(port + 2, (value >> 16) & 0xff);
		WRITEPORT8(port + 3, (value >> 24) & 0xff);
	}
	else
	{
		check_ioperm(port, 0xf);
		m_io->write_dword(port, value);
	}
}

#endif /* __I386_H__ */
