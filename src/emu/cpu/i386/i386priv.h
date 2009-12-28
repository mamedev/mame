#pragma once

#ifndef __I386_H__
#define __I386_H__

#include "i386.h"
#include "cpuintrf.h"

#define I386OP(XX)		i386_##XX
#define I486OP(XX)		i486_##XX
#define PENTIUMOP(XX)	pentium_##XX
#define MMXOP(XX)		mmx_##XX

extern int i386_dasm_one(char *buffer, UINT32 pc, const UINT8 *oprom, int mode);

typedef enum { ES, CS, SS, DS, FS, GS } SREGS;

typedef enum
{
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0,3),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(1,2),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(4,7),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(5,6),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(8,11),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(9,10),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(12,15),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(13,14)
} BREGS;

typedef enum
{
	AX = NATIVE_ENDIAN_VALUE_LE_BE(0,1),
	CX = NATIVE_ENDIAN_VALUE_LE_BE(2,3),
	DX = NATIVE_ENDIAN_VALUE_LE_BE(4,5),
	BX = NATIVE_ENDIAN_VALUE_LE_BE(6,7),
	SP = NATIVE_ENDIAN_VALUE_LE_BE(8,9),
	BP = NATIVE_ENDIAN_VALUE_LE_BE(10,11),
	SI = NATIVE_ENDIAN_VALUE_LE_BE(12,13),
	DI = NATIVE_ENDIAN_VALUE_LE_BE(14,15)
} WREGS;

typedef enum { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI } DREGS;

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

	X87_CTRL,
	X87_STATUS,
	X87_ST0,
	X87_ST1,
	X87_ST2,
	X87_ST3,
	X87_ST4,
	X87_ST5,
	X87_ST6,
	X87_ST7,
};

typedef struct {
	UINT16 selector;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
	int d;		// Operand size
} I386_SREG;

typedef struct {
	UINT32 base;
	UINT16 limit;
} I386_SYS_TABLE;

typedef struct {
	UINT16 segment;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
} I386_SEG_DESC;

typedef union {
	UINT32 d[8];
	UINT16 w[16];
	UINT8 b[32];
} I386_GPR;

typedef union {
	UINT64 i;
	double f;
} X87_REG;

typedef struct _i386_state i386_state;
struct _i386_state
{
	I386_GPR reg;
	I386_SREG sreg[6];
	UINT32 eip;
	UINT32 pc;
	UINT32 prev_eip;
	UINT32 eflags;
	UINT8 CF;
	UINT8 DF;
	UINT8 SF;
	UINT8 OF;
	UINT8 ZF;
	UINT8 PF;
	UINT8 AF;
	UINT8 IF;
	UINT8 TF;
	UINT8 IOP1;
	UINT8 IOP2;
	UINT8 NT;

	UINT8 performed_intersegment_jump;

	UINT32 cr[4];		// Control registers
	UINT32 dr[8];		// Debug registers
	UINT32 tr[8];		// Test registers

	I386_SYS_TABLE gdtr;	// Global Descriptor Table Register
	I386_SYS_TABLE idtr;	// Interrupt Descriptor Table Register
	I386_SEG_DESC task;		// Task register
	I386_SEG_DESC ldtr;		// Local Descriptor Table Register

	int halted;

	int operand_size;
	int address_size;

	int segment_prefix;
	int segment_override;

	int cycles;
	int base_cycles;
	UINT8 opcode;

	UINT8 irq_state;
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *io;
	UINT32 a20_mask;

	int cpuid_max_input_value_eax;
	UINT32 cpuid_id0, cpuid_id1, cpuid_id2;
	UINT32 cpu_version;
	UINT32 feature_flags;
	UINT64 tsc;

	// FPU
	X87_REG fpu_reg[8];
	UINT16 fpu_control_word;
	UINT16 fpu_status_word;
	UINT16 fpu_tag_word;
	UINT64 fpu_data_ptr;
	UINT64 fpu_inst_ptr;
	UINT16 fpu_opcode;
	int fpu_top;

	void (*opcode_table1_16[256])(i386_state *cpustate);
	void (*opcode_table1_32[256])(i386_state *cpustate);
	void (*opcode_table2_16[256])(i386_state *cpustate);
	void (*opcode_table2_32[256])(i386_state *cpustate);

	UINT8 *cycle_table_pm;
	UINT8 *cycle_table_rm;
};

INLINE i386_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_I386 ||
		   cpu_get_type(device) == CPU_I486 ||
		   cpu_get_type(device) == CPU_PENTIUM ||
		   cpu_get_type(device) == CPU_MEDIAGX);
	return (i386_state *)device->token;
}

extern int i386_parity_table[256];

#define PROTECTED_MODE		(cpustate->cr[0] & 0x1)
#define STACK_32BIT			(cpustate->sreg[SS].d)
#define V8086_MODE			(cpustate->eflags & 0x00020000)

#define SetOF_Add32(r,s,d)	(cpustate->OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80000000) ? 1: 0)
#define SetOF_Add16(r,s,d)	(cpustate->OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? 1 : 0)
#define SetOF_Add8(r,s,d)	(cpustate->OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? 1 : 0)

#define SetOF_Sub32(r,s,d)	(cpustate->OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80000000) ? 1 : 0)
#define SetOF_Sub16(r,s,d)	(cpustate->OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? 1 : 0)
#define SetOF_Sub8(r,s,d)	(cpustate->OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? 1 : 0)

#define SetCF8(x)			{cpustate->CF = ((x) & 0x100) ? 1 : 0; }
#define SetCF16(x)			{cpustate->CF = ((x) & 0x10000) ? 1 : 0; }
#define SetCF32(x)			{cpustate->CF = ((x) & (((UINT64)1) << 32)) ? 1 : 0; }

#define SetSF(x)			(cpustate->SF = (x))
#define SetZF(x)			(cpustate->ZF = (x))
#define SetAF(x,y,z)		(cpustate->AF = (((x) ^ ((y) ^ (z))) & 0x10) ? 1 : 0)
#define SetPF(x)			(cpustate->PF = i386_parity_table[(x) & 0xFF])

#define SetSZPF8(x)			{cpustate->ZF = ((UINT8)(x)==0);  cpustate->SF = ((x)&0x80) ? 1 : 0; cpustate->PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF16(x)		{cpustate->ZF = ((UINT16)(x)==0);  cpustate->SF = ((x)&0x8000) ? 1 : 0; cpustate->PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF32(x)		{cpustate->ZF = ((UINT32)(x)==0);  cpustate->SF = ((x)&0x80000000) ? 1 : 0; cpustate->PF = i386_parity_table[x & 0xFF]; }

/***********************************************************************************/

typedef struct {
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
} MODRM_TABLE;

extern MODRM_TABLE i386_MODRM_table[256];

#define REG8(x)			(cpustate->reg.b[x])
#define REG16(x)		(cpustate->reg.w[x])
#define REG32(x)		(cpustate->reg.d[x])

#define LOAD_REG8(x)	(REG8(i386_MODRM_table[x].reg.b))
#define LOAD_REG16(x)	(REG16(i386_MODRM_table[x].reg.w))
#define LOAD_REG32(x)	(REG32(i386_MODRM_table[x].reg.d))
#define LOAD_RM8(x)		(REG8(i386_MODRM_table[x].rm.b))
#define LOAD_RM16(x)	(REG16(i386_MODRM_table[x].rm.w))
#define LOAD_RM32(x)	(REG32(i386_MODRM_table[x].rm.d))

#define STORE_REG8(x, value)	(REG8(i386_MODRM_table[x].reg.b) = value)
#define STORE_REG16(x, value)	(REG16(i386_MODRM_table[x].reg.w) = value)
#define STORE_REG32(x, value)	(REG32(i386_MODRM_table[x].reg.d) = value)
#define STORE_RM8(x, value)		(REG8(i386_MODRM_table[x].rm.b) = value)
#define STORE_RM16(x, value)	(REG16(i386_MODRM_table[x].rm.w) = value)
#define STORE_RM32(x, value)	(REG32(i386_MODRM_table[x].rm.d) = value)

/***********************************************************************************/

INLINE UINT32 i386_translate(i386_state *cpustate, int segment, UINT32 ip)
{
	// TODO: segment limit
	return cpustate->sreg[segment].base + ip;
}

INLINE int translate_address(i386_state *cpustate, UINT32 *address)
{
	UINT32 a = *address;
	UINT32 pdbr = cpustate->cr[3] & 0xfffff000;
	UINT32 directory = (a >> 22) & 0x3ff;
	UINT32 table = (a >> 12) & 0x3ff;
	UINT32 offset = a & 0xfff;

	// TODO: 4MB pages
	UINT32 page_dir = memory_read_dword_32le(cpustate->program, pdbr + directory * 4);
	UINT32 page_entry = memory_read_dword_32le(cpustate->program, (page_dir & 0xfffff000) + (table * 4));

	*address = (page_entry & 0xfffff000) | offset;
	return 1;
}

INLINE void CHANGE_PC(i386_state *cpustate, UINT32 pc)
{
	UINT32 address;
	cpustate->pc = i386_translate(cpustate, CS, pc );

	address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}
}

INLINE void NEAR_BRANCH(i386_state *cpustate, INT32 offs)
{
	UINT32 address;
	/* TODO: limit */
	cpustate->eip += offs;
	cpustate->pc += offs;

	address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}
}

INLINE UINT8 FETCH(i386_state *cpustate)
{
	UINT8 value;
	UINT32 address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	value = memory_decrypted_read_byte(cpustate->program, address & cpustate->a20_mask);
	cpustate->eip++;
	cpustate->pc++;
	return value;
}
INLINE UINT16 FETCH16(i386_state *cpustate)
{
	UINT16 value;
	UINT32 address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	if( address & 0x1 ) {		/* Unaligned read */
		address &= cpustate->a20_mask;
		value = (memory_decrypted_read_byte(cpustate->program, address+0) << 0) |
				(memory_decrypted_read_byte(cpustate->program, address+1) << 8);
	} else {
		address &= cpustate->a20_mask;
		value = memory_decrypted_read_word(cpustate->program, address);
	}
	cpustate->eip += 2;
	cpustate->pc += 2;
	return value;
}
INLINE UINT32 FETCH32(i386_state *cpustate)
{
	UINT32 value;
	UINT32 address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	if( cpustate->pc & 0x3 ) {		/* Unaligned read */
		address &= cpustate->a20_mask;
		value = (memory_decrypted_read_byte(cpustate->program, address+0) << 0) |
				(memory_decrypted_read_byte(cpustate->program, address+1) << 8) |
				(memory_decrypted_read_byte(cpustate->program, address+2) << 16) |
				(memory_decrypted_read_byte(cpustate->program, address+3) << 24);
	} else {
		address &= cpustate->a20_mask;
		value = memory_decrypted_read_dword(cpustate->program, address);
	}
	cpustate->eip += 4;
	cpustate->pc += 4;
	return value;
}

INLINE UINT8 READ8(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	address &= cpustate->a20_mask;
	return memory_read_byte_32le(cpustate->program, address);
}
INLINE UINT16 READ16(i386_state *cpustate,UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	address &= cpustate->a20_mask;
	if( ea & 0x1 ) {		/* Unaligned read */
		value = (memory_read_byte_32le( cpustate->program, address+0 ) << 0) |
				(memory_read_byte_32le( cpustate->program, address+1 ) << 8);
	} else {
		value = memory_read_word_32le( cpustate->program, address );
	}
	return value;
}
INLINE UINT32 READ32(i386_state *cpustate,UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	address &= cpustate->a20_mask;
	if( ea & 0x3 ) {		/* Unaligned read */
		value = (memory_read_byte_32le( cpustate->program, address+0 ) << 0) |
				(memory_read_byte_32le( cpustate->program, address+1 ) << 8) |
				(memory_read_byte_32le( cpustate->program, address+2 ) << 16) |
				(memory_read_byte_32le( cpustate->program, address+3 ) << 24);
	} else {
		value = memory_read_dword_32le( cpustate->program, address );
	}
	return value;
}

INLINE UINT64 READ64(i386_state *cpustate,UINT32 ea)
{
	UINT64 value;
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	address &= cpustate->a20_mask;
	if( ea & 0x7 ) {		/* Unaligned read */
		value = (((UINT64) memory_read_byte_32le( cpustate->program, address+0 )) << 0) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+1 )) << 8) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+2 )) << 16) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+3 )) << 24) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+4 )) << 32) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+5 )) << 40) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+6 )) << 48) |
				(((UINT64) memory_read_byte_32le( cpustate->program, address+7 )) << 56);
	} else {
		value = (((UINT64) memory_read_dword_32le( cpustate->program, address+0 )) << 0) |
				(((UINT64) memory_read_dword_32le( cpustate->program, address+4 )) << 32);
	}
	return value;
}

INLINE void WRITE8(i386_state *cpustate,UINT32 ea, UINT8 value)
{
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	address &= cpustate->a20_mask;
	memory_write_byte_32le(cpustate->program, address, value);
}
INLINE void WRITE16(i386_state *cpustate,UINT32 ea, UINT16 value)
{
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	address &= cpustate->a20_mask;
	if( ea & 0x1 ) {		/* Unaligned write */
		memory_write_byte_32le( cpustate->program, address+0, value & 0xff );
		memory_write_byte_32le( cpustate->program, address+1, (value >> 8) & 0xff );
	} else {
		memory_write_word_32le(cpustate->program, address, value);
	}
}
INLINE void WRITE32(i386_state *cpustate,UINT32 ea, UINT32 value)
{
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	ea &= cpustate->a20_mask;
	if( ea & 0x3 ) {		/* Unaligned write */
		memory_write_byte_32le( cpustate->program, address+0, value & 0xff );
		memory_write_byte_32le( cpustate->program, address+1, (value >> 8) & 0xff );
		memory_write_byte_32le( cpustate->program, address+2, (value >> 16) & 0xff );
		memory_write_byte_32le( cpustate->program, address+3, (value >> 24) & 0xff );
	} else {
		memory_write_dword_32le(cpustate->program, address, value);
	}
}

INLINE void WRITE64(i386_state *cpustate,UINT32 ea, UINT64 value)
{
	UINT32 address = ea;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,&address);
	}

	ea &= cpustate->a20_mask;
	if( ea & 0x7 ) {		/* Unaligned write */
		memory_write_byte_32le( cpustate->program, address+0, value & 0xff );
		memory_write_byte_32le( cpustate->program, address+1, (value >> 8) & 0xff );
		memory_write_byte_32le( cpustate->program, address+2, (value >> 16) & 0xff );
		memory_write_byte_32le( cpustate->program, address+3, (value >> 24) & 0xff );
		memory_write_byte_32le( cpustate->program, address+4, (value >> 32) & 0xff );
		memory_write_byte_32le( cpustate->program, address+5, (value >> 40) & 0xff );
		memory_write_byte_32le( cpustate->program, address+6, (value >> 48) & 0xff );
		memory_write_byte_32le( cpustate->program, address+7, (value >> 56) & 0xff );
	} else {
		memory_write_dword_32le(cpustate->program, address+0, value & 0xffffffff);
		memory_write_dword_32le(cpustate->program, address+4, (value >> 32) & 0xffffffff);
	}
}

/***********************************************************************************/

INLINE UINT8 OR8(i386_state *cpustate,UINT8 dst, UINT8 src)
{
	UINT8 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 OR16(i386_state *cpustate,UINT16 dst, UINT16 src)
{
	UINT16 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 OR32(i386_state *cpustate,UINT32 dst, UINT32 src)
{
	UINT32 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 AND8(i386_state *cpustate,UINT8 dst, UINT8 src)
{
	UINT8 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 AND16(i386_state *cpustate,UINT16 dst, UINT16 src)
{
	UINT16 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 AND32(i386_state *cpustate,UINT32 dst, UINT32 src)
{
	UINT32 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 XOR8(i386_state *cpustate,UINT8 dst, UINT8 src)
{
	UINT8 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 XOR16(i386_state *cpustate,UINT16 dst, UINT16 src)
{
	UINT16 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 XOR32(i386_state *cpustate,UINT32 dst, UINT32 src)
{
	UINT32 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

#define SUB8(cpu, dst, src) SBB8(cpu, dst, src, 0)
INLINE UINT8 SBB8(i386_state *cpustate,UINT8 dst, UINT8 src, UINT8 b)
{
	UINT16 res = (UINT16)dst - (UINT16)src - (UINT8)b;
	SetCF8(res);
	SetOF_Sub8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}

#define SUB16(cpu, dst, src) SBB16(cpu, dst, src, 0)
INLINE UINT16 SBB16(i386_state *cpustate,UINT16 dst, UINT16 src, UINT16 b)
{
	UINT32 res = (UINT32)dst - (UINT32)src - (UINT32)b;
	SetCF16(res);
	SetOF_Sub16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}

#define SUB32(cpu, dst, src) SBB32(cpu, dst, src, 0)
INLINE UINT32 SBB32(i386_state *cpustate,UINT32 dst, UINT32 src, UINT32 b)
{
	UINT64 res = (UINT64)dst - (UINT64)src - (UINT64) b;
	SetCF32(res);
	SetOF_Sub32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

#define ADD8(cpu, dst, src) ADC8(cpu, dst, src, 0)
INLINE UINT8 ADC8(i386_state *cpustate,UINT8 dst, UINT8 src, UINT8 c)
{
	UINT16 res = (UINT16)dst + (UINT16)src + (UINT16)c;
	SetCF8(res);
	SetOF_Add8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}

#define ADD16(cpu, dst, src) ADC16(cpu, dst, src, 0)
INLINE UINT16 ADC16(i386_state *cpustate,UINT16 dst, UINT16 src, UINT8 c)
{
	UINT32 res = (UINT32)dst + (UINT32)src + (UINT32)c;
	SetCF16(res);
	SetOF_Add16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}

#define ADD32(cpu, dst, src) ADC32(cpu, dst, src, 0)
INLINE UINT32 ADC32(i386_state *cpustate,UINT32 dst, UINT32 src, UINT32 c)
{
	UINT64 res = (UINT64)dst + (UINT64)src + (UINT64) c;
	SetCF32(res);
	SetOF_Add32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 INC8(i386_state *cpustate,UINT8 dst)
{
	UINT16 res = (UINT16)dst + 1;
	SetOF_Add8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 INC16(i386_state *cpustate,UINT16 dst)
{
	UINT32 res = (UINT32)dst + 1;
	SetOF_Add16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 INC32(i386_state *cpustate,UINT32 dst)
{
	UINT64 res = (UINT64)dst + 1;
	SetOF_Add32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 DEC8(i386_state *cpustate,UINT8 dst)
{
	UINT16 res = (UINT16)dst - 1;
	SetOF_Sub8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 DEC16(i386_state *cpustate,UINT16 dst)
{
	UINT32 res = (UINT32)dst - 1;
	SetOF_Sub16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 DEC32(i386_state *cpustate,UINT32 dst)
{
	UINT64 res = (UINT64)dst - 1;
	SetOF_Sub32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}



INLINE void PUSH16(i386_state *cpustate,UINT16 value)
{
	UINT32 ea;
	if( STACK_32BIT ) {
		REG32(ESP) -= 2;
		ea = i386_translate(cpustate, SS, REG32(ESP) );
		WRITE16(cpustate, ea, value );
	} else {
		REG16(SP) -= 2;
		ea = i386_translate(cpustate, SS, REG16(SP) );
		WRITE16(cpustate, ea, value );
	}
}
INLINE void PUSH32(i386_state *cpustate,UINT32 value)
{
	UINT32 ea;
	if( STACK_32BIT ) {
		REG32(ESP) -= 4;
		ea = i386_translate(cpustate, SS, REG32(ESP) );
		WRITE32(cpustate, ea, value );
	} else {
		REG16(SP) -= 4;
		ea = i386_translate(cpustate, SS, REG16(SP) );
		WRITE32(cpustate, ea, value );
	}
}
INLINE void PUSH8(i386_state *cpustate,UINT8 value)
{
	if( cpustate->operand_size ) {
		PUSH32(cpustate,(INT32)(INT8)value);
	} else {
		PUSH16(cpustate,(INT16)(INT8)value);
	}
}

INLINE UINT8 POP8(i386_state *cpustate)
{
	UINT8 value;
	UINT32 ea;
	if( STACK_32BIT ) {
		ea = i386_translate(cpustate, SS, REG32(ESP) );
		value = READ8(cpustate, ea );
		REG32(ESP) += 1;
	} else {
		ea = i386_translate(cpustate, SS, REG16(SP) );
		value = READ8(cpustate, ea );
		REG16(SP) += 1;
	}
	return value;
}
INLINE UINT16 POP16(i386_state *cpustate)
{
	UINT16 value;
	UINT32 ea;
	if( STACK_32BIT ) {
		ea = i386_translate(cpustate, SS, REG32(ESP) );
		value = READ16(cpustate, ea );
		REG32(ESP) += 2;
	} else {
		ea = i386_translate(cpustate, SS, REG16(SP) );
		value = READ16(cpustate, ea );
		REG16(SP) += 2;
	}
	return value;
}
INLINE UINT32 POP32(i386_state *cpustate)
{
	UINT32 value;
	UINT32 ea;
	if( STACK_32BIT ) {
		ea = i386_translate(cpustate, SS, REG32(ESP) );
		value = READ32(cpustate, ea );
		REG32(ESP) += 4;
	} else {
		ea = i386_translate(cpustate, SS, REG16(SP) );
		value = READ32(cpustate, ea );
		REG16(SP) += 4;
	}
	return value;
}

INLINE void BUMP_SI(i386_state *cpustate,int adjustment)
{
	if ( cpustate->address_size )
		REG32(ESI) += ((cpustate->DF) ? -adjustment : +adjustment);
	else
		REG16(SI) += ((cpustate->DF) ? -adjustment : +adjustment);
}

INLINE void BUMP_DI(i386_state *cpustate,int adjustment)
{
	if ( cpustate->address_size )
		REG32(EDI) += ((cpustate->DF) ? -adjustment : +adjustment);
	else
		REG16(DI) += ((cpustate->DF) ? -adjustment : +adjustment);
}



/***********************************************************************************/

#define READPORT8(port)		    	(memory_read_byte_32le(cpustate->io, port))
#define READPORT16(port)	    	(memory_read_word_32le(cpustate->io, port))
#define READPORT32(port)	    	(memory_read_dword_32le(cpustate->io, port))
#define WRITEPORT8(port, value)		(memory_write_byte_32le(cpustate->io, port, value))
#define WRITEPORT16(port, value)	(memory_write_word_32le(cpustate->io, port, value))
#define WRITEPORT32(port, value)	(memory_write_dword_32le(cpustate->io, port, value))

#endif /* __I386_H__ */
