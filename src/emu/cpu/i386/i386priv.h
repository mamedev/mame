#pragma once

#ifndef __I386_H__
#define __I386_H__

#include "i386.h"
#include "../../../lib/softfloat/milieu.h"
#include "../../../lib/softfloat/softfloat.h"

//#define DEBUG_MISSING_OPCODE

#define I386OP(XX)		i386_##XX
#define I486OP(XX)		i486_##XX
#define PENTIUMOP(XX)	pentium_##XX
#define MMXOP(XX)		mmx_##XX
#define SSEOP(XX)		sse_##XX

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
	X87_ST7
};

enum
{
	/* mmx registers aliased to x87 ones */
	MMX_MM0=X87_ST0,
	MMX_MM1=X87_ST1,
	MMX_MM2=X87_ST2,
	MMX_MM3=X87_ST3,
	MMX_MM4=X87_ST4,
	MMX_MM5=X87_ST5,
	MMX_MM6=X87_ST6,
	MMX_MM7=X87_ST7
};

/* Protected mode exceptions */
#define FAULT_UD 6   // Invalid Opcode
#define FAULT_NM 7   // Coprocessor not available
#define FAULT_DF 8   // Double Fault
#define FAULT_TS 10  // Invalid TSS
#define FAULT_NP 11  // Segment or Gate not present
#define FAULT_SS 12  // Stack fault
#define FAULT_GP 13  // General Protection Fault
#define FAULT_PF 14  // Page Fault
#define FAULT_MF 16  // Match (Coprocessor) Fault

/* MXCSR Control and Status Register */
#define MXCSR_IE  (1<<0)  // Invalid Operation Flag
#define MXCSR_DE  (1<<1)  // Denormal Flag
#define MXCSR_ZE  (1<<2)  // Divide-by-Zero Flag
#define MXCSR_OE  (1<<3)  // Overflow Flag
#define MXCSR_UE  (1<<4)  // Underflow Flag
#define MXCSR_PE  (1<<5)  // Precision Flag
#define MXCSR_DAZ (1<<6)  // Denormals Are Zeros
#define MXCSR_IM  (1<<7)  // Invalid Operation Mask
#define MXCSR_DM  (1<<8)  // Denormal Operation Mask
#define MXCSR_ZM  (1<<9)  // Divide-by-Zero Mask
#define MXCSR_OM  (1<<10) // Overflow Mask
#define MXCSR_UM  (1<<11) // Underflow Mask
#define MXCSR_PM  (1<<12) // Precision Mask
#define MXCSR_RC  (3<<13) // Rounding Control
#define MXCSR_FZ  (1<<15) // Flush to Zero

typedef struct {
	UINT16 selector;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
	int d;		// Operand size
	bool valid;
} I386_SREG;

typedef struct
{
	UINT16 segment;
	UINT16 selector;
	UINT32 offset;
	UINT8 ar;  // access rights
	UINT8 dpl;
	UINT8 dword_count;
	UINT8 present;
} I386_CALL_GATE;

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

typedef UINT64 MMX_REG;

typedef union {
	UINT32 d[4];
	UINT16 w[8];
	UINT8 b[16];
	UINT64 q[2];
	float f[4];
} XMM_REG;

typedef struct _i386_state i386_state;
struct _i386_state
{
	I386_GPR reg;
	I386_SREG sreg[6];
	UINT32 eip;
	UINT32 pc;
	UINT32 prev_eip;
	UINT32 eflags;
	UINT32 eflags_mask;
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
	UINT8 RF;
	UINT8 VM;
	UINT8 AC;
	UINT8 VIF;
	UINT8 VIP;
	UINT8 ID;

	UINT8 CPL;  // current privilege level

	UINT8 performed_intersegment_jump;
	UINT8 delayed_interrupt_enable;

	UINT32 cr[5];		// Control registers
	UINT32 dr[8];		// Debug registers
	UINT32 tr[8];		// Test registers

	I386_SYS_TABLE gdtr;	// Global Descriptor Table Register
	I386_SYS_TABLE idtr;	// Interrupt Descriptor Table Register
	I386_SEG_DESC task;		// Task register
	I386_SEG_DESC ldtr;		// Local Descriptor Table Register

	UINT8 ext;  // external interrupt

	int halted;

	int operand_size;
	int address_size;
	int operand_prefix;
	int address_prefix;

	int segment_prefix;
	int segment_override;

	int cycles;
	int base_cycles;
	UINT8 opcode;

	UINT8 irq_state;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	UINT32 a20_mask;

	int cpuid_max_input_value_eax;
	UINT32 cpuid_id0, cpuid_id1, cpuid_id2;
	UINT32 cpu_version;
	UINT32 feature_flags;
	UINT64 tsc;


	// FPU
	floatx80 x87_reg[8];

	UINT16 x87_cw;
	UINT16 x87_sw;
	UINT16 x87_tw;
	UINT64 x87_data_ptr;
	UINT64 x87_inst_ptr;
	UINT16 x87_opcode;

	void (*opcode_table_x87_d8[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_d9[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_da[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_db[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_dc[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_dd[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_de[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_df[256])(i386_state *cpustate, UINT8 modrm);

	// SSE
	XMM_REG sse_reg[8];
	UINT32 mxcsr;

	void (*opcode_table1_16[256])(i386_state *cpustate);
	void (*opcode_table1_32[256])(i386_state *cpustate);
	void (*opcode_table2_16[256])(i386_state *cpustate);
	void (*opcode_table2_32[256])(i386_state *cpustate);
	void (*opcode_table366_16[256])(i386_state *cpustate);
	void (*opcode_table366_32[256])(i386_state *cpustate);
	void (*opcode_table3f2_16[256])(i386_state *cpustate);
	void (*opcode_table3f2_32[256])(i386_state *cpustate);
	void (*opcode_table3f3_16[256])(i386_state *cpustate);
	void (*opcode_table3f3_32[256])(i386_state *cpustate);

	UINT8 *cycle_table_pm;
	UINT8 *cycle_table_rm;

	// bytes in current opcode, debug only
#ifdef DEBUG_MISSING_OPCODE
	UINT8 opcode_bytes[16];
	UINT32 opcode_pc;
	int opcode_bytes_length;
#endif
};

INLINE i386_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I386 ||
		   device->type() == I486 ||
		   device->type() == PENTIUM ||
		   device->type() == MEDIAGX);
	return (i386_state *)downcast<legacy_cpu_device *>(device)->token();
}

extern int i386_parity_table[256];
static int i386_limit_check(i386_state *cpustate, int seg, UINT32 offset);

#define FAULT_THROW(fault,error) { throw (UINT64)(fault | (UINT64)error << 32); }
#define PF_THROW(error) { cpustate->cr[2] = address; FAULT_THROW(FAULT_PF,error); }

#define PROTECTED_MODE		(cpustate->cr[0] & 0x1)
#define STACK_32BIT			(cpustate->sreg[SS].d)
#define V8086_MODE			(cpustate->VM)
#define NESTED_TASK			(cpustate->NT)

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

#define MMX(n)				cpustate->fpu_reg[(n)].i
#define XMM(n)				cpustate->sse_reg[(n)]

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

#define SWITCH_ENDIAN_32(x) (((((x) << 24) & (0xff << 24)) | (((x) << 8) & (0xff << 16)) | (((x) >> 8) & (0xff << 8)) | (((x) >> 24) & (0xff << 0))))

/***********************************************************************************/

INLINE UINT32 i386_translate(i386_state *cpustate, int segment, UINT32 ip, int rwn)
{
	// TODO: segment limit access size, execution permission, handle exception thrown from exception handler
	if(PROTECTED_MODE && !V8086_MODE && (rwn != -1))
	{
		if(!(cpustate->sreg[segment].valid))
			FAULT_THROW((segment==SS)?FAULT_SS:FAULT_GP, 0);
		if(i386_limit_check(cpustate, segment, ip))
			FAULT_THROW((segment==SS)?FAULT_SS:FAULT_GP, 0);
		if((rwn == 0) && ((cpustate->sreg[segment].flags & 8) && !(cpustate->sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
		if((rwn == 1) && ((cpustate->sreg[segment].flags & 8) || !(cpustate->sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
	}
	return cpustate->sreg[segment].base + ip;
}

// rwn; read = 0, write = 1, none = -1, read at PL 0 = -2
INLINE int translate_address(i386_state *cpustate, int rwn, UINT32 *address, UINT32 *error)
{
	UINT32 a = *address;
	UINT32 pdbr = cpustate->cr[3] & 0xfffff000;
	UINT32 directory = (a >> 22) & 0x3ff;
	UINT32 table = (a >> 12) & 0x3ff;
	UINT32 offset = a & 0xfff;
	UINT32 page_entry;
	UINT32 ret = 1;
	bool user = (cpustate->CPL == 3) && (rwn >= 0);
	*error = 0;

	// TODO: cr0 wp bit, 486 and higher
	UINT32 page_dir = cpustate->program->read_dword(pdbr + directory * 4);
	if((page_dir & 1) && ((page_dir & 4) || !user))
	{
		if (!(cpustate->cr[4] & 0x10))
		{
			page_entry = cpustate->program->read_dword((page_dir & 0xfffff000) + (table * 4));
			if(!(page_entry & 1))
				ret = 0;
			else if((!(page_entry & 2) && user && (rwn == 1)) || (!(page_entry & 4) && user))
			{
				*error = 1;
				ret = 0;
			}
			else
			{
				if(!(page_dir & 0x20) && (rwn != -1))
					cpustate->program->write_dword(pdbr + directory * 4, page_dir | 0x20);
				if(!(page_entry & 0x40) && (rwn == 1))
					cpustate->program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x60);
				else if(!(page_entry & 0x20) && (rwn != -1))
					cpustate->program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x20);
				*address = (page_entry & 0xfffff000) | offset;
			}
		}
		else
		{
			if (page_dir & 0x80)
			{
				if(!(page_dir & 2) && user && (rwn == 1))
				{
					*error = 1;
					ret = 0;
				}
				else
				{
					if(!(page_dir & 0x40) && (rwn == 1))
						cpustate->program->write_dword(pdbr + directory * 4, page_dir | 0x60);
					else if(!(page_dir & 0x20) && (rwn != -1))
						cpustate->program->write_dword(pdbr + directory * 4, page_dir | 0x20);
					*address = (page_dir & 0xffc00000) | (a & 0x003fffff);
				}
			}
			else
			{
				page_entry = cpustate->program->read_dword((page_dir & 0xfffff000) + (table * 4));
				if(!(page_entry & 1))
					ret = 0;
				else if((!(page_entry & 2) && user && (rwn == 1)) || (!(page_entry & 4) && user))
				{
					*error = 1;
					ret = 0;
				}
				else
				{
					if(!(page_dir & 0x20) && (rwn != -1))
						cpustate->program->write_dword(pdbr + directory * 4, page_dir | 0x20);
					if(!(page_entry & 0x40) && (rwn == 1))
						cpustate->program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x60);
					else if(!(page_entry & 0x20) && (rwn != -1))
						cpustate->program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x20);
					*address = (page_entry & 0xfffff000) | offset;
				}
			}
		}
	}
	else
	{
		if(page_dir & 1)
			*error = 1;
		ret = 0;
	}
	if(!ret)
	{
		if(rwn != -1)
			*error |= ((rwn & 1)<<1) | ((cpustate->CPL == 3)<<2);
		return 0;
	}
	return 1;
}

INLINE void CHANGE_PC(i386_state *cpustate, UINT32 pc)
{
	UINT32 address, error;
	cpustate->pc = i386_translate(cpustate, CS, pc, -1 );

	address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,-1,&address,&error);
	}
}

INLINE void NEAR_BRANCH(i386_state *cpustate, INT32 offs)
{
	UINT32 address, error;
	/* TODO: limit */
	cpustate->eip += offs;
	cpustate->pc += offs;

	address = cpustate->pc;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(cpustate,-1,&address,&error);
	}
}

INLINE UINT8 FETCH(i386_state *cpustate)
{
	UINT8 value;
	UINT32 address = cpustate->pc, error;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		if(!translate_address(cpustate,0,&address,&error))
			PF_THROW(error);
	}

	value = cpustate->direct->read_decrypted_byte(address & cpustate->a20_mask);
#ifdef DEBUG_MISSING_OPCODE
	cpustate->opcode_bytes[cpustate->opcode_bytes_length] = value;
	cpustate->opcode_bytes_length = (cpustate->opcode_bytes_length + 1) & 15;
#endif	
	cpustate->eip++;
	cpustate->pc++;
	return value;
}
INLINE UINT16 FETCH16(i386_state *cpustate)
{
	UINT16 value;
	UINT32 address = cpustate->pc, error;

	if( address & 0x1 ) {		/* Unaligned read */
		value = (FETCH(cpustate) << 0) |
				(FETCH(cpustate) << 8);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,0,&address,&error))
				PF_THROW(error);
		}
		address &= cpustate->a20_mask;
		value = cpustate->direct->read_decrypted_word(address);
		cpustate->eip += 2;
		cpustate->pc += 2;
	}
	return value;
}
INLINE UINT32 FETCH32(i386_state *cpustate)
{
	UINT32 value;
	UINT32 address = cpustate->pc, error;

	if( cpustate->pc & 0x3 ) {		/* Unaligned read */
		value = (FETCH(cpustate) << 0) |
				(FETCH(cpustate) << 8) |
				(FETCH(cpustate) << 16) |
				(FETCH(cpustate) << 24);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,0,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		value = cpustate->direct->read_decrypted_dword(address);
		cpustate->eip += 4;
		cpustate->pc += 4;
	}
	return value;
}

INLINE UINT8 READ8(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea, error;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		if(!translate_address(cpustate,0,&address,&error))
			PF_THROW(error);
	}

	address &= cpustate->a20_mask;
	return cpustate->program->read_byte(address);
}
INLINE UINT16 READ16(i386_state *cpustate,UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea, error;

	if( ea & 0x1 ) {		/* Unaligned read */
		value = (READ8( cpustate, address+0 ) << 0) |
				(READ8( cpustate, address+1 ) << 8);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,0,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		value = cpustate->program->read_word( address );
	}
	return value;
}
INLINE UINT32 READ32(i386_state *cpustate,UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea, error;

	if( ea & 0x3 ) {		/* Unaligned read */
		value = (READ8( cpustate, address+0 ) << 0) |
				(READ8( cpustate, address+1 ) << 8) |
				(READ8( cpustate, address+2 ) << 16) |
				(READ8( cpustate, address+3 ) << 24);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,0,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		value = cpustate->program->read_dword( address );
	}
	return value;
}

INLINE UINT64 READ64(i386_state *cpustate,UINT32 ea)
{
	UINT64 value;
	UINT32 address = ea, error;

	if( ea & 0x7 ) {		/* Unaligned read */
		value = (((UINT64) READ8( cpustate, address+0 )) << 0) |
				(((UINT64) READ8( cpustate, address+1 )) << 8) |
				(((UINT64) READ8( cpustate, address+2 )) << 16) |
				(((UINT64) READ8( cpustate, address+3 )) << 24) |
				(((UINT64) READ8( cpustate, address+4 )) << 32) |
				(((UINT64) READ8( cpustate, address+5 )) << 40) |
				(((UINT64) READ8( cpustate, address+6 )) << 48) |
				(((UINT64) READ8( cpustate, address+7 )) << 56);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,0,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		value = (((UINT64) cpustate->program->read_dword( address+0 )) << 0) |
				(((UINT64) cpustate->program->read_dword( address+4 )) << 32);
	}
	return value;
}
INLINE UINT8 READ8PL0(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea, error;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		if(!translate_address(cpustate,-2,&address,&error))
			PF_THROW(error);
	}

	address &= cpustate->a20_mask;
	return cpustate->program->read_byte(address);
}
INLINE UINT16 READ16PL0(i386_state *cpustate,UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea, error;

	if( ea & 0x1 ) {		/* Unaligned read */
		value = (READ8PL0( cpustate, address+0 ) << 0) |
				(READ8PL0( cpustate, address+1 ) << 8);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,-2,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		value = cpustate->program->read_word( address );
	}
	return value;
}
INLINE UINT32 READ32PL0(i386_state *cpustate,UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea, error;

	if( ea & 0x3 ) {		/* Unaligned read */
		value = (READ8PL0( cpustate, address+0 ) << 0) |
				(READ8PL0( cpustate, address+1 ) << 8) |
				(READ8PL0( cpustate, address+2 ) << 16) |
				(READ8PL0( cpustate, address+3 ) << 24);
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,-2,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		value = cpustate->program->read_dword( address );
	}
	return value;
}

INLINE void WRITE_TEST(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea, error;
	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		if(!translate_address(cpustate,1,&address,&error))
			PF_THROW(error);
	}
}

INLINE void WRITE8(i386_state *cpustate,UINT32 ea, UINT8 value)
{
	UINT32 address = ea, error;

	if (cpustate->cr[0] & 0x80000000)		// page translation enabled
	{
		if(!translate_address(cpustate,1,&address,&error))
			PF_THROW(error);
	}

	address &= cpustate->a20_mask;
	cpustate->program->write_byte(address, value);
}
INLINE void WRITE16(i386_state *cpustate,UINT32 ea, UINT16 value)
{
	UINT32 address = ea, error;

	if( ea & 0x1 ) {		/* Unaligned write */
		WRITE8( cpustate, address+0, value & 0xff );
		WRITE8( cpustate, address+1, (value >> 8) & 0xff );
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,1,&address,&error))
				PF_THROW(error);
		}

		address &= cpustate->a20_mask;
		cpustate->program->write_word(address, value);
	}
}
INLINE void WRITE32(i386_state *cpustate,UINT32 ea, UINT32 value)
{
	UINT32 address = ea, error;

	if( ea & 0x3 ) {		/* Unaligned write */
		WRITE8( cpustate, address+0, value & 0xff );
		WRITE8( cpustate, address+1, (value >> 8) & 0xff );
		WRITE8( cpustate, address+2, (value >> 16) & 0xff );
		WRITE8( cpustate, address+3, (value >> 24) & 0xff );
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,1,&address,&error))
				PF_THROW(error);
		}

		ea &= cpustate->a20_mask;
		cpustate->program->write_dword(address, value);
	}
}

INLINE void WRITE64(i386_state *cpustate,UINT32 ea, UINT64 value)
{
	UINT32 address = ea, error;

	if( ea & 0x7 ) {		/* Unaligned write */
		WRITE8( cpustate, address+0, value & 0xff );
		WRITE8( cpustate, address+1, (value >> 8) & 0xff );
		WRITE8( cpustate, address+2, (value >> 16) & 0xff );
		WRITE8( cpustate, address+3, (value >> 24) & 0xff );
		WRITE8( cpustate, address+4, (value >> 32) & 0xff );
		WRITE8( cpustate, address+5, (value >> 40) & 0xff );
		WRITE8( cpustate, address+6, (value >> 48) & 0xff );
		WRITE8( cpustate, address+7, (value >> 56) & 0xff );
	} else {
		if (cpustate->cr[0] & 0x80000000)		// page translation enabled
		{
			if(!translate_address(cpustate,1,&address,&error))
				PF_THROW(error);
		}

		ea &= cpustate->a20_mask;
		cpustate->program->write_dword(address+0, value & 0xffffffff);
		cpustate->program->write_dword(address+4, (value >> 32) & 0xffffffff);
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
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 2;
		ea = i386_translate(cpustate, SS, new_esp, 1);
		WRITE16(cpustate, ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 2) & 0xffff;
		ea = i386_translate(cpustate, SS, new_esp, 1);
		WRITE16(cpustate, ea, value );
		REG16(SP) = new_esp;
	}
}
INLINE void PUSH32(i386_state *cpustate,UINT32 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate(cpustate, SS, new_esp, 1);
		WRITE32(cpustate, ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate(cpustate, SS, new_esp, 1);
		WRITE32(cpustate, ea, value );
		REG16(SP) = new_esp;
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
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 1;
		ea = i386_translate(cpustate, SS, new_esp - 1, 0);
		value = READ8(cpustate, ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 1;
		ea = i386_translate(cpustate, SS, (new_esp - 1) & 0xffff, 0);
		value = READ8(cpustate, ea );
		REG16(SP) = new_esp;
	}
	return value;
}
INLINE UINT16 POP16(i386_state *cpustate)
{
	UINT16 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 2;
		ea = i386_translate(cpustate, SS, new_esp - 2, 0);
		value = READ16(cpustate, ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 2;
		ea = i386_translate(cpustate, SS, (new_esp - 2) & 0xffff, 0);
		value = READ16(cpustate, ea );
		REG16(SP) = new_esp;
	}
	return value;
}
INLINE UINT32 POP32(i386_state *cpustate)
{
	UINT32 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 4;
		ea = i386_translate(cpustate, SS, new_esp - 4, 0);
		value = READ32(cpustate, ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 4;
		ea = i386_translate(cpustate, SS, (new_esp - 4) & 0xffff, 0);
		value = READ32(cpustate, ea );
		REG16(SP) = new_esp;
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



/***********************************************************************************
    I/O ACCESS
***********************************************************************************/

INLINE void check_ioperm(i386_state *cpustate, offs_t port, UINT8 mask)
{
	UINT8 IOPL, map;
	UINT16 IOPB;
	UINT32 address;

	if(!PROTECTED_MODE)
		return;

	IOPL = cpustate->IOP1 | (cpustate->IOP2 << 1);
	if(!V8086_MODE && (cpustate->CPL <= IOPL))
		return;

	if((cpustate->task.limit < 0x67) || ((cpustate->task.flags & 0xd) != 9))
		FAULT_THROW(FAULT_GP,0);

	address = cpustate->task.base;
	IOPB = READ16PL0(cpustate, address+0x66);
	if((IOPB+(port/8)) > cpustate->task.limit)
		FAULT_THROW(FAULT_GP,0);

	map = READ8PL0(cpustate, address+IOPB+(port/8));
	map >>= (port%8);
	if(map & mask)
		FAULT_THROW(FAULT_GP,0);
}

INLINE UINT8 READPORT8(i386_state *cpustate, offs_t port)
{
	check_ioperm(cpustate, port, 1);
	return cpustate->io->read_byte(port);
}

INLINE void WRITEPORT8(i386_state *cpustate, offs_t port, UINT8 value)
{
	check_ioperm(cpustate, port, 1);
	cpustate->io->write_byte(port, value);
}

INLINE UINT16 READPORT16(i386_state *cpustate, offs_t port)
{
	if (port & 1)
	{
		return  READPORT8(cpustate, port) |
		       (READPORT8(cpustate, port + 1) << 8);
	}
	else
	{
		check_ioperm(cpustate, port, 3);
		return cpustate->io->read_word(port);
	}
}

INLINE void WRITEPORT16(i386_state *cpustate, offs_t port, UINT16 value)
{
	if (port & 1)
	{
		WRITEPORT8(cpustate, port, value & 0xff);
		WRITEPORT8(cpustate, port + 1, (value >> 8) & 0xff);
	}
	else
	{
		check_ioperm(cpustate, port, 3);
		cpustate->io->write_word(port, value);
	}
}

INLINE UINT32 READPORT32(i386_state *cpustate, offs_t port)
{
	if (port & 3)
	{
		return  READPORT8(cpustate, port) |
		       (READPORT8(cpustate, port + 1) << 8) |
		       (READPORT8(cpustate, port + 2) << 16) |
		       (READPORT8(cpustate, port + 3) << 24);
	}
	else
	{
		check_ioperm(cpustate, port, 0xf);
		return cpustate->io->read_dword(port);
	}
}

INLINE void WRITEPORT32(i386_state *cpustate, offs_t port, UINT32 value)
{
	if (port & 3)
	{
		WRITEPORT8(cpustate, port, value & 0xff);
		WRITEPORT8(cpustate, port + 1, (value >> 8) & 0xff);
		WRITEPORT8(cpustate, port + 2, (value >> 16) & 0xff);
		WRITEPORT8(cpustate, port + 3, (value >> 24) & 0xff);
	}
	else
	{
		check_ioperm(cpustate, port, 0xf);
		cpustate->io->write_dword(port, value);
	}
}

/***********************************************************************************
    MSR ACCESS
***********************************************************************************/

INLINE UINT64 MSR_READ(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	UINT64 res;

	*valid_msr = 0;

	switch(offset)
	{
		default:
			logerror("RDMSR: unimplemented register called %08x at %08x\n",offset,cpustate->pc-2);
			res = -1;
			*valid_msr = 1;
			break;
	}

	return res;
}

INLINE void MSR_WRITE(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	*valid_msr = 0;

	switch(offset)
	{
		default:
			logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
			*valid_msr = 1;
			break;
	}
}

#endif /* __I386_H__ */
