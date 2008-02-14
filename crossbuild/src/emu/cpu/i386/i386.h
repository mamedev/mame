#ifndef __I386_H_
#define __I386_H_

#include "cpuintrf.h"

#define I386OP(XX)		i386_##XX
#define I486OP(XX)		i486_##XX
#define PENTIUMOP(XX)	pentium_##XX
#define MMXOP(XX)		mmx_##XX

#define INPUT_LINE_A20		1

#ifdef ENABLE_DEBUGGER
extern int i386_dasm_one(char *buffer, UINT32 pc, const UINT8 *oprom, int mode);
#endif

typedef enum { ES, CS, SS, DS, FS, GS } SREGS;

#ifdef LSB_FIRST
typedef enum {
	AL = 0,
	AH = 1,
	CL = 4,
	CH = 5,
	DL = 8,
	DH = 9,
	BL = 12,
	BH = 13
} BREGS;
#else
typedef enum {
	AL = 3,
	AH = 2,
	CL = 7,
	CH = 6,
	DL = 11,
	DH = 10,
	BL = 15,
	BH = 14
} BREGS;
#endif

#ifdef LSB_FIRST
typedef enum {
	AX = 0,
	CX = 2,
	DX = 4,
	BX = 6,
	SP = 8,
	BP = 10,
	SI = 12,
	DI = 14
} WREGS;
#else
typedef enum {
	AX = 1,
	CX = 3,
	DX = 5,
	BX = 7,
	SP = 9,
	BP = 11,
	SI = 13,
	DI = 15
} WREGS;
#endif

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

typedef struct {
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
	int (*irq_callback)(int);
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

	void (*opcode_table1_16[256])(void);
	void (*opcode_table1_32[256])(void);
	void (*opcode_table2_16[256])(void);
	void (*opcode_table2_32[256])(void);

	UINT8 *cycle_table_pm;
	UINT8 *cycle_table_rm;
} I386_REGS;


static I386_REGS I;


extern int i386_parity_table[256];

#define PROTECTED_MODE		(I.cr[0] & 0x1)
#define STACK_32BIT			(I.sreg[SS].d)
#define V8086_MODE			(I.eflags & 0x00020000)

#define SetOF_Add32(r,s,d)	(I.OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80000000) ? 1: 0)
#define SetOF_Add16(r,s,d)	(I.OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? 1 : 0)
#define SetOF_Add8(r,s,d)	(I.OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? 1 : 0)

#define SetOF_Sub32(r,s,d)	(I.OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80000000) ? 1 : 0)
#define SetOF_Sub16(r,s,d)	(I.OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? 1 : 0)
#define SetOF_Sub8(r,s,d)	(I.OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? 1 : 0)

#define SetCF8(x)			{I.CF = ((x) & 0x100) ? 1 : 0; }
#define SetCF16(x)			{I.CF = ((x) & 0x10000) ? 1 : 0; }
#define SetCF32(x)			{I.CF = ((x) & (((UINT64)1) << 32)) ? 1 : 0; }

#define SetSF(x)			(I.SF = (x))
#define SetZF(x)			(I.ZF = (x))
#define SetAF(x,y,z)		(I.AF = (((x) ^ ((y) ^ (z))) & 0x10) ? 1 : 0)
#define SetPF(x)			(I.PF = i386_parity_table[(x) & 0xFF])

#define SetSZPF8(x)			{I.ZF = ((UINT8)(x)==0);  I.SF = ((x)&0x80) ? 1 : 0; I.PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF16(x)		{I.ZF = ((UINT16)(x)==0);  I.SF = ((x)&0x8000) ? 1 : 0; I.PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF32(x)		{I.ZF = ((UINT32)(x)==0);  I.SF = ((x)&0x80000000) ? 1 : 0; I.PF = i386_parity_table[x & 0xFF]; }

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

#define REG8(x)			(I.reg.b[x])
#define REG16(x)		(I.reg.w[x])
#define REG32(x)		(I.reg.d[x])

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

INLINE UINT32 i386_translate(int segment, UINT32 ip)
{
	// TODO: segment limit
	return I.sreg[segment].base + ip;
}

INLINE int translate_address(UINT32 *address)
{
	UINT32 a = *address;
	UINT32 pdbr = I.cr[3] & 0xfffff000;
	UINT32 directory = (a >> 22) & 0x3ff;
	UINT32 table = (a >> 12) & 0x3ff;
	UINT32 offset = a & 0xfff;

	// TODO: 4MB pages
	UINT32 page_dir = program_read_dword_32le(pdbr + directory * 4);
	UINT32 page_entry = program_read_dword_32le((page_dir & 0xfffff000) + (table * 4));

	*address = (page_entry & 0xfffff000) | offset;
	return 1;
}

INLINE void CHANGE_PC(UINT32 pc)
{
	UINT32 address;
	I.pc = i386_translate( CS, pc );

	address = I.pc;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	change_pc(address & I.a20_mask);
}

INLINE void NEAR_BRANCH(INT32 offs)
{
	UINT32 address;
	/* TODO: limit */
	I.eip += offs;
	I.pc += offs;

	address = I.pc;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	change_pc(address & I.a20_mask);
}

INLINE UINT8 FETCH(void)
{
	UINT8 value;
	UINT32 address = I.pc;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	value = cpu_readop(address & I.a20_mask);
	I.eip++;
	I.pc++;
	return value;
}
INLINE UINT16 FETCH16(void)
{
	UINT16 value;
	UINT32 address = I.pc;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	if( address & 0x1 ) {		/* Unaligned read */
		address &= I.a20_mask;
		value = (cpu_readop(address+0) << 0) |
				(cpu_readop(address+1) << 8);
	} else {
		address &= I.a20_mask;
		value = cpu_readop16(address);
	}
	I.eip += 2;
	I.pc += 2;
	return value;
}
INLINE UINT32 FETCH32(void)
{
	UINT32 value;
	UINT32 address = I.pc;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	if( I.pc & 0x3 ) {		/* Unaligned read */
		address &= I.a20_mask;
		value = (cpu_readop(address+0) << 0) |
				(cpu_readop(address+1) << 8) |
				(cpu_readop(address+2) << 16) |
				(cpu_readop(address+3) << 24);
	} else {
		address &= I.a20_mask;
		value = cpu_readop32(address);
	}
	I.eip += 4;
	I.pc += 4;
	return value;
}

INLINE UINT8 READ8(UINT32 ea)
{
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	address &= I.a20_mask;
	return program_read_byte_32le(address);
}
INLINE UINT16 READ16(UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	address &= I.a20_mask;
	if( ea & 0x1 ) {		/* Unaligned read */
		value = (program_read_byte_32le( address+0 ) << 0) |
				(program_read_byte_32le( address+1 ) << 8);
	} else {
		value = program_read_word_32le( address );
	}
	return value;
}
INLINE UINT32 READ32(UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	address &= I.a20_mask;
	if( ea & 0x3 ) {		/* Unaligned read */
		value = (program_read_byte_32le( address+0 ) << 0) |
				(program_read_byte_32le( address+1 ) << 8) |
				(program_read_byte_32le( address+2 ) << 16) |
				(program_read_byte_32le( address+3 ) << 24);
	} else {
		value = program_read_dword_32le( address );
	}
	return value;
}

INLINE UINT64 READ64(UINT32 ea)
{
	UINT64 value;
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	address &= I.a20_mask;
	if( ea & 0x7 ) {		/* Unaligned read */
		value = (((UINT64) program_read_byte_32le( address+0 )) << 0) |
				(((UINT64) program_read_byte_32le( address+1 )) << 8) |
				(((UINT64) program_read_byte_32le( address+2 )) << 16) |
				(((UINT64) program_read_byte_32le( address+3 )) << 24) |
				(((UINT64) program_read_byte_32le( address+4 )) << 32) |
				(((UINT64) program_read_byte_32le( address+5 )) << 40) |
				(((UINT64) program_read_byte_32le( address+6 )) << 48) |
				(((UINT64) program_read_byte_32le( address+7 )) << 56);
	} else {
		value = (((UINT64) program_read_dword_32le( address+0 )) << 0) |
				(((UINT64) program_read_dword_32le( address+4 )) << 32);
	}
	return value;
}

INLINE void WRITE8(UINT32 ea, UINT8 value)
{
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	address &= I.a20_mask;
	program_write_byte_32le(address, value);
}
INLINE void WRITE16(UINT32 ea, UINT16 value)
{
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	address &= I.a20_mask;
	if( ea & 0x1 ) {		/* Unaligned write */
		program_write_byte_32le( address+0, value & 0xff );
		program_write_byte_32le( address+1, (value >> 8) & 0xff );
	} else {
		program_write_word_32le(address, value);
	}
}
INLINE void WRITE32(UINT32 ea, UINT32 value)
{
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	ea &= I.a20_mask;
	if( ea & 0x3 ) {		/* Unaligned write */
		program_write_byte_32le( address+0, value & 0xff );
		program_write_byte_32le( address+1, (value >> 8) & 0xff );
		program_write_byte_32le( address+2, (value >> 16) & 0xff );
		program_write_byte_32le( address+3, (value >> 24) & 0xff );
	} else {
		program_write_dword_32le(address, value);
	}
}

INLINE void WRITE64(UINT32 ea, UINT64 value)
{
	UINT32 address = ea;

	if (I.cr[0] & 0x80000000)		// page translation enabled
	{
		translate_address(&address);
	}

	ea &= I.a20_mask;
	if( ea & 0x7 ) {		/* Unaligned write */
		program_write_byte_32le( address+0, value & 0xff );
		program_write_byte_32le( address+1, (value >> 8) & 0xff );
		program_write_byte_32le( address+2, (value >> 16) & 0xff );
		program_write_byte_32le( address+3, (value >> 24) & 0xff );
		program_write_byte_32le( address+4, (value >> 32) & 0xff );
		program_write_byte_32le( address+5, (value >> 40) & 0xff );
		program_write_byte_32le( address+6, (value >> 48) & 0xff );
		program_write_byte_32le( address+7, (value >> 56) & 0xff );
	} else {
		program_write_dword_32le(address+0, value & 0xffffffff);
		program_write_dword_32le(address+4, (value >> 32) & 0xffffffff);
	}
}

/***********************************************************************************/

INLINE UINT8 OR8(UINT8 dst, UINT8 src)
{
	UINT8 res = dst | src;
	I.CF = I.OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 OR16(UINT16 dst, UINT16 src)
{
	UINT16 res = dst | src;
	I.CF = I.OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 OR32(UINT32 dst, UINT32 src)
{
	UINT32 res = dst | src;
	I.CF = I.OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 AND8(UINT8 dst, UINT8 src)
{
	UINT8 res = dst & src;
	I.CF = I.OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 AND16(UINT16 dst, UINT16 src)
{
	UINT16 res = dst & src;
	I.CF = I.OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 AND32(UINT32 dst, UINT32 src)
{
	UINT32 res = dst & src;
	I.CF = I.OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 XOR8(UINT8 dst, UINT8 src)
{
	UINT8 res = dst ^ src;
	I.CF = I.OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 XOR16(UINT16 dst, UINT16 src)
{
	UINT16 res = dst ^ src;
	I.CF = I.OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 XOR32(UINT32 dst, UINT32 src)
{
	UINT32 res = dst ^ src;
	I.CF = I.OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 SUB8(UINT8 dst, UINT8 src)
{
	UINT16 res = (UINT16)dst - (UINT16)src;
	SetCF8(res);
	SetOF_Sub8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 SUB16(UINT16 dst, UINT16 src)
{
	UINT32 res = (UINT32)dst - (UINT32)src;
	SetCF16(res);
	SetOF_Sub16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 SUB32(UINT32 dst, UINT32 src)
{
	UINT64 res = (UINT64)dst - (UINT64)src;
	SetCF32(res);
	SetOF_Sub32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 ADD8(UINT8 dst, UINT8 src)
{
	UINT16 res = (UINT16)dst + (UINT16)src;
	SetCF8(res);
	SetOF_Add8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 ADD16(UINT16 dst, UINT16 src)
{
	UINT32 res = (UINT32)dst + (UINT32)src;
	SetCF16(res);
	SetOF_Add16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 ADD32(UINT32 dst, UINT32 src)
{
	UINT64 res = (UINT64)dst + (UINT64)src;
	SetCF32(res);
	SetOF_Add32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 INC8(UINT8 dst)
{
	UINT16 res = (UINT16)dst + 1;
	SetOF_Add8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 INC16(UINT16 dst)
{
	UINT32 res = (UINT32)dst + 1;
	SetOF_Add16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 INC32(UINT32 dst)
{
	UINT64 res = (UINT64)dst + 1;
	SetOF_Add32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 DEC8(UINT8 dst)
{
	UINT16 res = (UINT16)dst - 1;
	SetOF_Sub8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 DEC16(UINT16 dst)
{
	UINT32 res = (UINT32)dst - 1;
	SetOF_Sub16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 DEC32(UINT32 dst)
{
	UINT64 res = (UINT64)dst - 1;
	SetOF_Sub32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}



INLINE void PUSH16(UINT16 value)
{
	UINT32 ea;
	if( STACK_32BIT ) {
		REG32(ESP) -= 2;
		ea = i386_translate( SS, REG32(ESP) );
		WRITE16( ea, value );
	} else {
		REG16(SP) -= 2;
		ea = i386_translate( SS, REG16(SP) );
		WRITE16( ea, value );
	}
}
INLINE void PUSH32(UINT32 value)
{
	UINT32 ea;
	if( STACK_32BIT ) {
		REG32(ESP) -= 4;
		ea = i386_translate( SS, REG32(ESP) );
		WRITE32( ea, value );
	} else {
		REG16(SP) -= 4;
		ea = i386_translate( SS, REG16(SP) );
		WRITE32( ea, value );
	}
}
INLINE void PUSH8(UINT8 value)
{
	if( I.operand_size ) {
		PUSH32((INT32)(INT8)value);
	} else {
		PUSH16((INT16)(INT8)value);
	}
}

INLINE UINT8 POP8(void)
{
	UINT8 value;
	UINT32 ea;
	if( STACK_32BIT ) {
		ea = i386_translate( SS, REG32(ESP) );
		value = READ8( ea );
		REG32(ESP) += 1;
	} else {
		ea = i386_translate( SS, REG16(SP) );
		value = READ8( ea );
		REG16(SP) += 1;
	}
	return value;
}
INLINE UINT16 POP16(void)
{
	UINT16 value;
	UINT32 ea;
	if( STACK_32BIT ) {
		ea = i386_translate( SS, REG32(ESP) );
		value = READ16( ea );
		REG32(ESP) += 2;
	} else {
		ea = i386_translate( SS, REG16(SP) );
		value = READ16( ea );
		REG16(SP) += 2;
	}
	return value;
}
INLINE UINT32 POP32(void)
{
	UINT32 value;
	UINT32 ea;
	if( STACK_32BIT ) {
		ea = i386_translate( SS, REG32(ESP) );
		value = READ32( ea );
		REG32(ESP) += 4;
	} else {
		ea = i386_translate( SS, REG16(SP) );
		value = READ32( ea );
		REG16(SP) += 4;
	}
	return value;
}

INLINE void BUMP_SI(int adjustment)
{
	if ( I.address_size )
		REG32(ESI) += ((I.DF) ? -adjustment : +adjustment);
	else
		REG16(SI) += ((I.DF) ? -adjustment : +adjustment);
}

INLINE void BUMP_DI(int adjustment)
{
	if ( I.address_size )
		REG32(EDI) += ((I.DF) ? -adjustment : +adjustment);
	else
		REG16(DI) += ((I.DF) ? -adjustment : +adjustment);
}



/***********************************************************************************/

#define READPORT8(port)		       	(io_read_byte_32le(port))
#define READPORT16(port)	       	(io_read_word_32le(port))
#define READPORT32(port)	       	(io_read_dword_32le(port))
#define WRITEPORT8(port, value)		(io_write_byte_32le(port, value))
#define WRITEPORT16(port, value)	(io_write_word_32le(port, value))
#define WRITEPORT32(port, value)	(io_write_dword_32le(port, value))

#endif
