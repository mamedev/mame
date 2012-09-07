// V60.C
// Undiscover the beast!
// Main hacking and coding by Farfetch'd
// Portability fixes by Richter Belmont

#include "emu.h"
#include "debugger.h"
#include "v60.h"

// memory accessors
#if defined(LSB_FIRST) && !defined(ALIGN_INTS)
#define OpRead8(s, a)	((s)->direct->read_decrypted_byte(a))
#define OpRead16(s, a)	((s)->direct->read_decrypted_word(a))
#define OpRead32(s, a)	((s)->direct->read_decrypted_dword(a))
#else
#define OpRead8(s, a)   ((s)->direct->read_decrypted_byte((a), (s)->fetch_xor))
#define OpRead16(s, a)	(((s)->direct->read_decrypted_byte(((a)+0), (s)->fetch_xor) << 0) | \
						 ((s)->direct->read_decrypted_byte(((a)+1), (s)->fetch_xor) << 8))
#define OpRead32(s, a)	(((s)->direct->read_decrypted_byte(((a)+0), (s)->fetch_xor) << 0) | \
						 ((s)->direct->read_decrypted_byte(((a)+1), (s)->fetch_xor) << 8) | \
						 ((s)->direct->read_decrypted_byte(((a)+2), (s)->fetch_xor) << 16) | \
						 ((s)->direct->read_decrypted_byte(((a)+3), (s)->fetch_xor) << 24))
#endif


// macros stolen from MAME for flags calc
// note that these types are in x86 naming:
// byte = 8 bit, word = 16 bit, long = 32 bit

// parameter x = result, y = source 1, z = source 2

#define SetOFL_Add(x, y,z)	(cpustate->_OV = (((x) ^ (y)) & ((x) ^ (z)) & 0x80000000) ? 1: 0)
#define SetOFW_Add(x, y,z)	(cpustate->_OV = (((x) ^ (y)) & ((x) ^ (z)) & 0x8000) ? 1 : 0)
#define SetOFB_Add(x, y,z)	(cpustate->_OV = (((x) ^ (y)) & ((x) ^ (z)) & 0x80) ? 1 : 0)

#define SetOFL_Sub(x, y,z)	(cpustate->_OV = (((z) ^ (y)) & ((z) ^ (x)) & 0x80000000) ? 1 : 0)
#define SetOFW_Sub(x, y,z)	(cpustate->_OV = (((z) ^ (y)) & ((z) ^ (x)) & 0x8000) ? 1 : 0)
#define SetOFB_Sub(x, y,z)	(cpustate->_OV = (((z) ^ (y)) & ((z) ^ (x)) & 0x80) ? 1 : 0)

#define SetCFB(x)			{cpustate->_CY = ((x) & 0x100) ? 1 : 0; }
#define SetCFW(x)			{cpustate->_CY = ((x) & 0x10000) ? 1 : 0; }
#define SetCFL(x)			{cpustate->_CY = ((x) & (((UINT64)1) << 32)) ? 1 : 0; }

#define SetSF(x)			(cpustate->_S = (x))
#define SetZF(x)			(cpustate->_Z = (x))

#define SetSZPF_Byte(x) 	{cpustate->_Z = ((UINT8)(x) == 0);  cpustate->_S = ((x)&0x80) ? 1 : 0; }
#define SetSZPF_Word(x) 	{cpustate->_Z = ((UINT16)(x) == 0);  cpustate->_S = ((x)&0x8000) ? 1 : 0; }
#define SetSZPF_Long(x) 	{cpustate->_Z = ((UINT32)(x) == 0);  cpustate->_S = ((x)&0x80000000) ? 1 : 0; }

#define ORB(dst, src)		{ (dst) |= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Byte(dst); }
#define ORW(dst, src)		{ (dst) |= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Word(dst); }
#define ORL(dst, src)		{ (dst) |= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Long(dst); }

#define ANDB(dst, src)		{ (dst) &= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Byte(dst); }
#define ANDW(dst, src)		{ (dst) &= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Word(dst); }
#define ANDL(dst, src)		{ (dst) &= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Long(dst); }

#define XORB(dst, src)		{ (dst) ^= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Byte(dst); }
#define XORW(dst, src)		{ (dst) ^= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Word(dst); }
#define XORL(dst, src)		{ (dst) ^= (src); cpustate->_CY = cpustate->_OV = 0; SetSZPF_Long(dst); }

#define SUBB(dst, src)		{ unsigned res = (dst) - (src); SetCFB(res); SetOFB_Sub(res, src, dst); SetSZPF_Byte(res); dst = (UINT8)res; }
#define SUBW(dst, src)		{ unsigned res = (dst) - (src); SetCFW(res); SetOFW_Sub(res, src, dst); SetSZPF_Word(res); dst = (UINT16)res; }
#define SUBL(dst, src)		{ UINT64 res = (UINT64)(dst) - (INT64)(src); SetCFL(res); SetOFL_Sub(res, src, dst); SetSZPF_Long(res); dst = (UINT32)res; }

#define ADDB(dst, src)		{ unsigned res = (dst) + (src); SetCFB(res); SetOFB_Add(res, src, dst); SetSZPF_Byte(res); dst = (UINT8)res; }
#define ADDW(dst, src)		{ unsigned res = (dst) + (src); SetCFW(res); SetOFW_Add(res, src, dst); SetSZPF_Word(res); dst = (UINT16)res; }
#define ADDL(dst, src)		{ UINT64 res = (UINT64)(dst) + (UINT64)(src); SetCFL(res); SetOFL_Add(res, src, dst); SetSZPF_Long(res); dst = (UINT32)res; }

#define SETREG8(a, b)		(a) = ((a) & ~0xff) | ((b) & 0xff)
#define SETREG16(a, b)		(a) = ((a) & ~0xffff) | ((b) & 0xffff)

typedef struct _v60_flags v60_flags;
struct _v60_flags
{
	UINT8 CY;
	UINT8 OV;
	UINT8 S;
	UINT8 Z;
};

// v60 Register Inside (Hm... It's not a pentium inside :-))) )
typedef struct _v60_state v60_state;
struct _v60_state
{
	offs_t				fetch_xor;
	offs_t				start_pc;
	UINT32				reg[68];
	v60_flags			flags;
	UINT8				irq_line;
	UINT8				nmi_line;
	device_irq_acknowledge_callback	irq_cb;
	legacy_cpu_device *		device;
	address_space *program;
	direct_read_data *	direct;
	address_space *io;
	UINT32				PPC;
	int					icount;
	int					stall_io;

	UINT32				op1, op2;
	UINT8				flag1, flag2;
	UINT8				instflags;
	UINT32				lenop1, lenop2;
	UINT8				subop;
	UINT32				bamoffset1, bamoffset2;

	// Output variables for ReadAMAddress(cpustate)
	UINT8				amflag;
	UINT32				amout;
	UINT32				bamoffset;

	// Appo temp var
	UINT32				amlength1, amlength2;

	// Global vars used by AM functions
	UINT32				modadd;
	UINT8				modm;
	UINT8				modval;
	UINT8				modval2;
	UINT8				modwritevalb;
	UINT16				modwritevalh;
	UINT32				modwritevalw;
	UINT8				moddim;
};

INLINE v60_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == V60 ||
		   device->type() == V70);
	return (v60_state *)downcast<legacy_cpu_device *>(device)->token();
}

/*
 * Prevent warnings on NetBSD.  All identifiers beginning with an underscore
 * followed by an uppercase letter are reserved by the C standard (ISO / IEC
 * 9899:1999, 7.1.3) to be used by the implementation.  It'd be best to rename
 * all such instances, but this is less intrusive and error-prone.
 */
#undef _S

#define _CY 	flags.CY
#define _OV 	flags.OV
#define _S		flags.S
#define _Z		flags.Z


// Defines of all v60 register...
#define R0		reg[0]
#define R1		reg[1]
#define R2		reg[2]
#define R3		reg[3]
#define R4		reg[4]
#define R5		reg[5]
#define R6		reg[6]
#define R7		reg[7]
#define R8		reg[8]
#define R9		reg[9]
#define R10		reg[10]
#define R11		reg[11]
#define R12		reg[12]
#define R13		reg[13]
#define R14		reg[14]
#define R15		reg[15]
#define R16		reg[16]
#define R17		reg[17]
#define R18		reg[18]
#define R19		reg[19]
#define R20		reg[20]
#define R21		reg[21]
#define R22		reg[22]
#define R23		reg[23]
#define R24		reg[24]
#define R25		reg[25]
#define R26		reg[26]
#define R27		reg[27]
#define R28		reg[28]
#define AP		reg[29]
#define FP		reg[30]
#define SP		reg[31]

#define PC		reg[32]
#define PSW		reg[33]

// Privileged registers
#define ISP		reg[36]
#define L0SP	reg[37]
#define L1SP	reg[38]
#define L2SP	reg[39]
#define L3SP	reg[40]
#define SBR		reg[41]
#define TR		reg[42]
#define SYCW	reg[43]
#define TKCW	reg[44]
#define PIR		reg[45]
//10-14 reserved
#define PSW2	reg[51]
#define ATBR0	reg[52]
#define ATLR0	reg[53]
#define ATBR1	reg[54]
#define ATLR1	reg[55]
#define ATBR2	reg[56]
#define ATLR2	reg[57]
#define ATBR3	reg[58]
#define ATLR3	reg[59]
#define TRMODE	reg[60]
#define ADTR0	reg[61]
#define ADTR1	reg[62]
#define ADTMR0	reg[63]
#define ADTMR1	reg[64]
//29-31 reserved

// Defines...
#define NORMALIZEFLAGS(cs) \
{ \
	(cs)->_S	= (cs)->_S  ? 1 : 0; \
	(cs)->_OV	= (cs)->_OV ? 1 : 0; \
	(cs)->_Z	= (cs)->_Z  ? 1 : 0; \
	(cs)->_CY	= (cs)->_CY ? 1 : 0; \
}

static void v60_try_irq(v60_state *cpustate);


INLINE void v60SaveStack(v60_state *cpustate)
{
	if (cpustate->PSW & 0x10000000)
		cpustate->ISP = cpustate->SP;
	else
		cpustate->reg[37 + ((cpustate->PSW >> 24) & 3)] = cpustate->SP;
}

INLINE void v60ReloadStack(v60_state *cpustate)
{
	if (cpustate->PSW & 0x10000000)
		cpustate->SP = cpustate->ISP;
	else
		cpustate->SP = cpustate->reg[37 + ((cpustate->PSW >> 24) & 3)];
}

INLINE UINT32 v60ReadPSW(v60_state *cpustate)
{
	cpustate->PSW &= 0xfffffff0;
	cpustate->PSW |= (cpustate->_Z?1:0) | (cpustate->_S?2:0) | (cpustate->_OV?4:0) | (cpustate->_CY?8:0);
	return cpustate->PSW;
}

INLINE void v60WritePSW(v60_state *cpustate, UINT32 newval)
{
	/* determine if we need to save / restore the stacks */
	int updateStack = 0;

	/* if the interrupt state is changing, we definitely need to update */
	if ((newval ^ cpustate->PSW) & 0x10000000)
		updateStack = 1;

	/* if we are not in interrupt mode and the level is changing, we also must update */
	else if (!(cpustate->PSW & 0x10000000) && ((newval ^ cpustate->PSW) & 0x03000000))
		updateStack = 1;

	/* save the previous stack value */
	if (updateStack)
		v60SaveStack(cpustate);

	/* set the new value and update the flags */
	cpustate->PSW = newval;
	cpustate->_Z =  (UINT8)(cpustate->PSW & 1);
	cpustate->_S =  (UINT8)(cpustate->PSW & 2);
	cpustate->_OV = (UINT8)(cpustate->PSW & 4);
	cpustate->_CY = (UINT8)(cpustate->PSW & 8);

	/* fetch the new stack value */
	if (updateStack)
		v60ReloadStack(cpustate);
}


INLINE UINT32 v60_update_psw_for_exception(v60_state *cpustate, int is_interrupt, int target_level)
{
	UINT32 oldPSW = v60ReadPSW(cpustate);
	UINT32 newPSW = oldPSW;

	// Change to interrupt context
	newPSW &= ~(3 << 24);  // cpustate->PSW.EL = 0
	newPSW |= target_level << 24; // set target level
	newPSW &= ~(1 << 18);  // cpustate->PSW.IE = 0
	newPSW &= ~(1 << 16);  // cpustate->PSW.TE = 0
	newPSW &= ~(1 << 27);  // cpustate->PSW.TP = 0
	newPSW &= ~(1 << 17);  // cpustate->PSW.AE = 0
	newPSW &= ~(1 << 29);  // cpustate->PSW.EM = 0
	if (is_interrupt)
		newPSW |=  (1 << 28);// cpustate->PSW.IS = 1
	newPSW |=  (1 << 31);  // cpustate->PSW.ASA = 1
	v60WritePSW(cpustate, newPSW);

	return oldPSW;
}


#define GETINTVECT(cs, nint)					(cs)->program->read_dword(((cs)->SBR & ~0xfff) + (nint) * 4)
#define EXCEPTION_CODE_AND_SIZE(code, size)	(((code) << 16) | (size))


// Addressing mode decoding functions
#include "am.c"

// Opcode functions
#include "op12.c"
#include "op2.c"
#include "op3.c"
#include "op4.c"
#include "op5.c"
#include "op6.c"
#include "op7a.c"

static UINT32 opUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled OpCode found : %02x at %08x\n", OpRead16(cpustate, cpustate->PC), cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

// Opcode jump table
#include "optable.c"

static void base_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback)
{
	v60_state *cpustate = get_safe_token(device);

	cpustate->stall_io = 0;
	cpustate->irq_cb = irqcallback;
	cpustate->device = device;
	cpustate->irq_line = CLEAR_LINE;
	cpustate->nmi_line = CLEAR_LINE;

	device->save_item(NAME(cpustate->reg));
	device->save_item(NAME(cpustate->irq_line));
	device->save_item(NAME(cpustate->nmi_line));
	device->save_item(NAME(cpustate->PPC));
	device->save_item(NAME(cpustate->_CY));
	device->save_item(NAME(cpustate->_OV));
	device->save_item(NAME(cpustate->_S));
	device->save_item(NAME(cpustate->_Z));
}

static CPU_INIT( v60 )
{
	v60_state *cpustate = get_safe_token(device);

	base_init(device, irqcallback);
	// Set cpustate->PIR (Processor ID) for NEC cpustate-> LSB is reserved to NEC,
	// so I don't know what it contains.
	cpustate->PIR = 0x00006000;
	cpustate->fetch_xor = BYTE_XOR_LE(0);
	cpustate->start_pc = 0xfffff0;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);
}

static CPU_INIT( v70 )
{
	v60_state *cpustate = get_safe_token(device);

	base_init(device, irqcallback);
	// Set cpustate->PIR (Processor ID) for NEC v70. LSB is reserved to NEC,
	// so I don't know what it contains.
	cpustate->PIR = 0x00007000;
	cpustate->fetch_xor = BYTE4_XOR_LE(0);
	cpustate->start_pc = 0xfffffff0;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);
}

static CPU_RESET( v60 )
{
	v60_state *cpustate = get_safe_token(device);

	cpustate->PSW	= 0x10000000;
	cpustate->PC	= cpustate->start_pc;
	cpustate->SBR	= 0x00000000;
	cpustate->SYCW	= 0x00000070;
	cpustate->TKCW	= 0x0000e000;
	cpustate->PSW2	= 0x0000f002;

	cpustate->_CY	= 0;
	cpustate->_OV	= 0;
	cpustate->_S	= 0;
	cpustate->_Z	= 0;
}

static CPU_EXIT( v60 )
{
}

void v60_stall(device_t *device)
{
	v60_state *cpustate = get_safe_token(device);
	cpustate->stall_io = 1;
}

static void v60_do_irq(v60_state *cpustate, int vector)
{
	UINT32 oldPSW = v60_update_psw_for_exception(cpustate, 1, 0);

	// Push cpustate->PC and cpustate->PSW onto the stack
	cpustate->SP-=4;
	cpustate->program->write_dword_unaligned(cpustate->SP, oldPSW);
	cpustate->SP-=4;
	cpustate->program->write_dword_unaligned(cpustate->SP, cpustate->PC);

	// Jump to vector for user interrupt
	cpustate->PC = GETINTVECT(cpustate, vector);
}

static void v60_try_irq(v60_state *cpustate)
{
	if(cpustate->irq_line == CLEAR_LINE)
		return;
	if((cpustate->PSW & (1 << 18)) != 0) {
		int vector;
		if(cpustate->irq_line != ASSERT_LINE)
			cpustate->irq_line = CLEAR_LINE;

		vector = cpustate->irq_cb(cpustate->device, 0);

		v60_do_irq(cpustate, vector + 0x40);
	}
}

static void set_irq_line(v60_state *cpustate, int irqline, int state)
{
	if(irqline == INPUT_LINE_NMI) {
		switch(state) {
		case ASSERT_LINE:
			if(cpustate->nmi_line == CLEAR_LINE) {
				cpustate->nmi_line = ASSERT_LINE;
				v60_do_irq(cpustate, 2);
			}
			break;
		case CLEAR_LINE:
			cpustate->nmi_line = CLEAR_LINE;
			break;
		}
	} else {
		cpustate->irq_line = state;
		v60_try_irq(cpustate);
	}
}

// Actual cycles / instruction is unknown

static CPU_EXECUTE( v60 )
{
	v60_state *cpustate = get_safe_token(device);

	if (cpustate->irq_line != CLEAR_LINE)
		v60_try_irq(cpustate);

	while (cpustate->icount > 0)
	{
		UINT32 inc;
		cpustate->PPC = cpustate->PC;
		debugger_instruction_hook(device, cpustate->PC);
		cpustate->icount -= 8;	/* fix me -- this is just an average */
		inc = OpCodeTable[OpRead8(cpustate, cpustate->PC)](cpustate);
		cpustate->PC += inc;
		if (cpustate->irq_line != CLEAR_LINE)
			v60_try_irq(cpustate);
	}
}


CPU_DISASSEMBLE( v60 );
CPU_DISASSEMBLE( v70 );


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( v60 )
{
	v60_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);break;

		case CPUINFO_INT_PC:							cpustate->PC = info->i; 						break;
		case CPUINFO_INT_SP:							cpustate->SP = info->i;							break;

		case CPUINFO_INT_REGISTER + V60_R0:				cpustate->R0 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R1:				cpustate->R1 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R2:				cpustate->R2 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R3:				cpustate->R3 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R4:				cpustate->R4 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R5:				cpustate->R5 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R6:				cpustate->R6 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R7:				cpustate->R7 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R8:				cpustate->R8 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R9:				cpustate->R9 = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_R10:			cpustate->R10 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R11:			cpustate->R11 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R12:			cpustate->R12 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R13:			cpustate->R13 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R14:			cpustate->R14 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R15:			cpustate->R15 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R16:			cpustate->R16 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R17:			cpustate->R17 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R18:			cpustate->R18 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R19:			cpustate->R19 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R20:			cpustate->R20 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R21:			cpustate->R21 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R22:			cpustate->R22 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R23:			cpustate->R23 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R24:			cpustate->R24 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R25:			cpustate->R25 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R26:			cpustate->R26 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R27:			cpustate->R27 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_R28:			cpustate->R28 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_AP:				cpustate->AP = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_FP:				cpustate->FP = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_SP:				cpustate->SP = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_PC:				cpustate->PC = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_PSW:			v60WritePSW(cpustate, info->i);					break;
		case CPUINFO_INT_REGISTER + V60_ISP:			cpustate->ISP = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_L0SP:			cpustate->L0SP = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_L1SP:			cpustate->L1SP = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_L2SP:			cpustate->L2SP = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_L3SP:			cpustate->L3SP = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_SBR:			cpustate->SBR = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_TR:				cpustate->TR = info->i;							break;
		case CPUINFO_INT_REGISTER + V60_SYCW:			cpustate->SYCW = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_TKCW:			cpustate->TKCW = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_PIR:			cpustate->PIR = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_PSW2:			cpustate->PSW2 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATBR0:			cpustate->ATBR0 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATLR0:			cpustate->ATLR0 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATBR1:			cpustate->ATBR1 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATLR1:			cpustate->ATLR1 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATBR2:			cpustate->ATBR2 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATLR2:			cpustate->ATLR2 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATBR3:			cpustate->ATBR3 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ATLR3:			cpustate->ATLR3 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_TRMODE:			cpustate->TRMODE = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ADTR0:			cpustate->ADTR0 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ADTR1:			cpustate->ADTR1 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ADTMR0:			cpustate->ADTMR0 = info->i;						break;
		case CPUINFO_INT_REGISTER + V60_ADTMR1:			cpustate->ADTMR1 = info->i;						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( v60 )
{
	v60_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(v60_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 22;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = cpustate->irq_line;			break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_line;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->PPC;				break;

		case CPUINFO_INT_REGISTER + V60_R0:				info->i = cpustate->R0;					break;
		case CPUINFO_INT_REGISTER + V60_R1:				info->i = cpustate->R1;					break;
		case CPUINFO_INT_REGISTER + V60_R2:				info->i = cpustate->R2;					break;
		case CPUINFO_INT_REGISTER + V60_R3:				info->i = cpustate->R3;					break;
		case CPUINFO_INT_REGISTER + V60_R4:				info->i = cpustate->R4;					break;
		case CPUINFO_INT_REGISTER + V60_R5:				info->i = cpustate->R5;					break;
		case CPUINFO_INT_REGISTER + V60_R6:				info->i = cpustate->R6;					break;
		case CPUINFO_INT_REGISTER + V60_R7:				info->i = cpustate->R7;					break;
		case CPUINFO_INT_REGISTER + V60_R8:				info->i = cpustate->R8;					break;
		case CPUINFO_INT_REGISTER + V60_R9:				info->i = cpustate->R9;					break;
		case CPUINFO_INT_REGISTER + V60_R10:			info->i = cpustate->R10;				break;
		case CPUINFO_INT_REGISTER + V60_R11:			info->i = cpustate->R11;				break;
		case CPUINFO_INT_REGISTER + V60_R12:			info->i = cpustate->R12;				break;
		case CPUINFO_INT_REGISTER + V60_R13:			info->i = cpustate->R13;				break;
		case CPUINFO_INT_REGISTER + V60_R14:			info->i = cpustate->R14;				break;
		case CPUINFO_INT_REGISTER + V60_R15:			info->i = cpustate->R15;				break;
		case CPUINFO_INT_REGISTER + V60_R16:			info->i = cpustate->R16;				break;
		case CPUINFO_INT_REGISTER + V60_R17:			info->i = cpustate->R17;				break;
		case CPUINFO_INT_REGISTER + V60_R18:			info->i = cpustate->R18;				break;
		case CPUINFO_INT_REGISTER + V60_R19:			info->i = cpustate->R19;				break;
		case CPUINFO_INT_REGISTER + V60_R20:			info->i = cpustate->R20;				break;
		case CPUINFO_INT_REGISTER + V60_R21:			info->i = cpustate->R21;				break;
		case CPUINFO_INT_REGISTER + V60_R22:			info->i = cpustate->R22;				break;
		case CPUINFO_INT_REGISTER + V60_R23:			info->i = cpustate->R23;				break;
		case CPUINFO_INT_REGISTER + V60_R24:			info->i = cpustate->R24;				break;
		case CPUINFO_INT_REGISTER + V60_R25:			info->i = cpustate->R25;				break;
		case CPUINFO_INT_REGISTER + V60_R26:			info->i = cpustate->R26;				break;
		case CPUINFO_INT_REGISTER + V60_R27:			info->i = cpustate->R27;				break;
		case CPUINFO_INT_REGISTER + V60_R28:			info->i = cpustate->R28;				break;
		case CPUINFO_INT_REGISTER + V60_AP:				info->i = cpustate->AP;					break;
		case CPUINFO_INT_REGISTER + V60_FP:				info->i = cpustate->FP;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + V60_SP:				info->i = cpustate->SP;					break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + V60_PC:				info->i = cpustate->PC;					break;
		case CPUINFO_INT_REGISTER + V60_PSW:			info->i = v60ReadPSW(cpustate);			break;
		case CPUINFO_INT_REGISTER + V60_ISP:			info->i = cpustate->ISP;				break;
		case CPUINFO_INT_REGISTER + V60_L0SP:			info->i = cpustate->L0SP;				break;
		case CPUINFO_INT_REGISTER + V60_L1SP:			info->i = cpustate->L1SP;				break;
		case CPUINFO_INT_REGISTER + V60_L2SP:			info->i = cpustate->L2SP;				break;
		case CPUINFO_INT_REGISTER + V60_L3SP:			info->i = cpustate->L3SP;				break;
		case CPUINFO_INT_REGISTER + V60_SBR:			info->i = cpustate->SBR;				break;
		case CPUINFO_INT_REGISTER + V60_TR:				info->i = cpustate->TR;					break;
		case CPUINFO_INT_REGISTER + V60_SYCW:			info->i = cpustate->SYCW;				break;
		case CPUINFO_INT_REGISTER + V60_TKCW:			info->i = cpustate->TKCW;				break;
		case CPUINFO_INT_REGISTER + V60_PIR:			info->i = cpustate->PIR;				break;
		case CPUINFO_INT_REGISTER + V60_PSW2:			info->i = cpustate->PSW2;				break;
		case CPUINFO_INT_REGISTER + V60_ATBR0:			info->i = cpustate->ATBR0;				break;
		case CPUINFO_INT_REGISTER + V60_ATLR0:			info->i = cpustate->ATLR0;				break;
		case CPUINFO_INT_REGISTER + V60_ATBR1:			info->i = cpustate->ATBR1;				break;
		case CPUINFO_INT_REGISTER + V60_ATLR1:			info->i = cpustate->ATLR1;				break;
		case CPUINFO_INT_REGISTER + V60_ATBR2:			info->i = cpustate->ATBR2;				break;
		case CPUINFO_INT_REGISTER + V60_ATLR2:			info->i = cpustate->ATLR2;				break;
		case CPUINFO_INT_REGISTER + V60_ATBR3:			info->i = cpustate->ATBR3;				break;
		case CPUINFO_INT_REGISTER + V60_ATLR3:			info->i = cpustate->ATLR3;				break;
		case CPUINFO_INT_REGISTER + V60_TRMODE:			info->i = cpustate->TRMODE;				break;
		case CPUINFO_INT_REGISTER + V60_ADTR0:			info->i = cpustate->ADTR0;				break;
		case CPUINFO_INT_REGISTER + V60_ADTR1:			info->i = cpustate->ADTR1;				break;
		case CPUINFO_INT_REGISTER + V60_ADTMR0:			info->i = cpustate->ADTMR0;				break;
		case CPUINFO_INT_REGISTER + V60_ADTMR1:			info->i = cpustate->ADTMR1;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(v60);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v60);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(v60);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(v60);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(v60);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(v60);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "V60");					break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "NEC V60");				break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Farfetch'd and R.Belmont"); break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + V60_R0:				sprintf(info->s, "R0:%08X", cpustate->R0);			break;
		case CPUINFO_STR_REGISTER + V60_R1:				sprintf(info->s, "R1:%08X", cpustate->R1);			break;
		case CPUINFO_STR_REGISTER + V60_R2:				sprintf(info->s, "R2:%08X", cpustate->R2);			break;
		case CPUINFO_STR_REGISTER + V60_R3:				sprintf(info->s, "R3:%08X", cpustate->R3);			break;
		case CPUINFO_STR_REGISTER + V60_R4:				sprintf(info->s, "R4:%08X", cpustate->R4);			break;
		case CPUINFO_STR_REGISTER + V60_R5:				sprintf(info->s, "R5:%08X", cpustate->R5);			break;
		case CPUINFO_STR_REGISTER + V60_R6:				sprintf(info->s, "R6:%08X", cpustate->R6);			break;
		case CPUINFO_STR_REGISTER + V60_R7:				sprintf(info->s, "R7:%08X", cpustate->R7);			break;
		case CPUINFO_STR_REGISTER + V60_R8:				sprintf(info->s, "R8:%08X", cpustate->R8);			break;
		case CPUINFO_STR_REGISTER + V60_R9:				sprintf(info->s, "R9:%08X", cpustate->R9);			break;
		case CPUINFO_STR_REGISTER + V60_R10:			sprintf(info->s, "R10:%08X", cpustate->R10);		break;
		case CPUINFO_STR_REGISTER + V60_R11:			sprintf(info->s, "R11:%08X", cpustate->R11);		break;
		case CPUINFO_STR_REGISTER + V60_R12:			sprintf(info->s, "R12:%08X", cpustate->R12);		break;
		case CPUINFO_STR_REGISTER + V60_R13:			sprintf(info->s, "R13:%08X", cpustate->R13);		break;
		case CPUINFO_STR_REGISTER + V60_R14:			sprintf(info->s, "R14:%08X", cpustate->R14);		break;
		case CPUINFO_STR_REGISTER + V60_R15:			sprintf(info->s, "R15:%08X", cpustate->R15);		break;
		case CPUINFO_STR_REGISTER + V60_R16:			sprintf(info->s, "R16:%08X", cpustate->R16);		break;
		case CPUINFO_STR_REGISTER + V60_R17:			sprintf(info->s, "R17:%08X", cpustate->R17);		break;
		case CPUINFO_STR_REGISTER + V60_R18:			sprintf(info->s, "R18:%08X", cpustate->R18);		break;
		case CPUINFO_STR_REGISTER + V60_R19:			sprintf(info->s, "R19:%08X", cpustate->R19);		break;
		case CPUINFO_STR_REGISTER + V60_R20:			sprintf(info->s, "R20:%08X", cpustate->R20);		break;
		case CPUINFO_STR_REGISTER + V60_R21:			sprintf(info->s, "R21:%08X", cpustate->R21);		break;
		case CPUINFO_STR_REGISTER + V60_R22:			sprintf(info->s, "R22:%08X", cpustate->R22);		break;
		case CPUINFO_STR_REGISTER + V60_R23:			sprintf(info->s, "R23:%08X", cpustate->R23);		break;
		case CPUINFO_STR_REGISTER + V60_R24:			sprintf(info->s, "R24:%08X", cpustate->R24);		break;
		case CPUINFO_STR_REGISTER + V60_R25:			sprintf(info->s, "R25:%08X", cpustate->R25);		break;
		case CPUINFO_STR_REGISTER + V60_R26:			sprintf(info->s, "R26:%08X", cpustate->R26);		break;
		case CPUINFO_STR_REGISTER + V60_R27:			sprintf(info->s, "R27:%08X", cpustate->R27);		break;
		case CPUINFO_STR_REGISTER + V60_R28:			sprintf(info->s, "R28:%08X", cpustate->R28);		break;
		case CPUINFO_STR_REGISTER + V60_AP:				sprintf(info->s, "AP:%08X", cpustate->AP);			break;
		case CPUINFO_STR_REGISTER + V60_FP:				sprintf(info->s, "FP:%08X", cpustate->FP);			break;
		case CPUINFO_STR_REGISTER + V60_SP:				sprintf(info->s, "SP:%08X", cpustate->SP);			break;
		case CPUINFO_STR_REGISTER + V60_PC:				sprintf(info->s, "PC:%08X", cpustate->PC);			break;
		case CPUINFO_STR_REGISTER + V60_PSW:			sprintf(info->s, "PSW:%08X", v60ReadPSW(cpustate)); break;
		case CPUINFO_STR_REGISTER + V60_ISP:			sprintf(info->s, "ISP:%08X", cpustate->ISP);		break;
		case CPUINFO_STR_REGISTER + V60_L0SP:			sprintf(info->s, "L0SP:%08X", cpustate->L0SP);		break;
		case CPUINFO_STR_REGISTER + V60_L1SP:			sprintf(info->s, "L1SP:%08X", cpustate->L1SP);		break;
		case CPUINFO_STR_REGISTER + V60_L2SP:			sprintf(info->s, "L2SP:%08X", cpustate->L2SP);		break;
		case CPUINFO_STR_REGISTER + V60_L3SP:			sprintf(info->s, "L3SP:%08X", cpustate->L3SP);		break;
		case CPUINFO_STR_REGISTER + V60_SBR:			sprintf(info->s, "SBR:%08X", cpustate->SBR);		break;
		case CPUINFO_STR_REGISTER + V60_TR:				sprintf(info->s, "TR:%08X", cpustate->TR);			break;
		case CPUINFO_STR_REGISTER + V60_SYCW:			sprintf(info->s, "SYCW:%08X", cpustate->SYCW);		break;
		case CPUINFO_STR_REGISTER + V60_TKCW:			sprintf(info->s, "TKCW:%08X", cpustate->TKCW);		break;
		case CPUINFO_STR_REGISTER + V60_PIR:			sprintf(info->s, "PIR:%08X", cpustate->PIR);		break;
		case CPUINFO_STR_REGISTER + V60_PSW2:			sprintf(info->s, "PSW2:%08X", cpustate->PSW2);		break;
		case CPUINFO_STR_REGISTER + V60_ATBR0:			sprintf(info->s, "ATBR0:%08X", cpustate->ATBR0);	break;
		case CPUINFO_STR_REGISTER + V60_ATLR0:			sprintf(info->s, "ATLR0:%08X", cpustate->ATLR0);	break;
		case CPUINFO_STR_REGISTER + V60_ATBR1:			sprintf(info->s, "ATBR1:%08X", cpustate->ATBR1);	break;
		case CPUINFO_STR_REGISTER + V60_ATLR1:			sprintf(info->s, "ATLR1:%08X", cpustate->ATLR1);	break;
		case CPUINFO_STR_REGISTER + V60_ATBR2:			sprintf(info->s, "ATBR2:%08X", cpustate->ATBR2);	break;
		case CPUINFO_STR_REGISTER + V60_ATLR2:			sprintf(info->s, "ATLR2:%08X", cpustate->ATLR2);	break;
		case CPUINFO_STR_REGISTER + V60_ATBR3:			sprintf(info->s, "ATBR3:%08X", cpustate->ATBR3);	break;
		case CPUINFO_STR_REGISTER + V60_ATLR3:			sprintf(info->s, "ATLR3:%08X", cpustate->ATLR3);	break;
		case CPUINFO_STR_REGISTER + V60_TRMODE:			sprintf(info->s, "TRMODE:%08X", cpustate->TRMODE);	break;
		case CPUINFO_STR_REGISTER + V60_ADTR0:			sprintf(info->s, "ADTR0:%08X", cpustate->ADTR0);	break;
		case CPUINFO_STR_REGISTER + V60_ADTR1:			sprintf(info->s, "ADTR1:%08X", cpustate->ADTR1);	break;
		case CPUINFO_STR_REGISTER + V60_ADTMR0:			sprintf(info->s, "ADTMR0:%08X", cpustate->ADTMR0);	break;
		case CPUINFO_STR_REGISTER + V60_ADTMR1:			sprintf(info->s, "ADTMR1:%08X", cpustate->ADTMR1);	break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( v70 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(v70);					break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(v70);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "V70");					break;

		default:										CPU_GET_INFO_CALL(v60);					break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(V60, v60);
DEFINE_LEGACY_CPU_DEVICE(V70, v70);
