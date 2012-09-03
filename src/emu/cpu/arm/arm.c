/* arm.c

    ARM 2/3/6 Emulation (26 bit address bus)

    Todo:
      Timing - Currently very approximated, nothing relies on proper timing so far.
      IRQ timing not yet correct (again, nothing is affected by this so far).

    Recent changes (2005):
      Fixed software interrupts
      Fixed various mode change bugs
      Added preliminary co-processor support.

    By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino

*/

#include "emu.h"
#include "debugger.h"
#include "arm.h"

CPU_DISASSEMBLE( arm );
CPU_DISASSEMBLE( arm_be );

#define READ8(addr)			cpu_read8(cpustate,addr)
#define WRITE8(addr,data)	cpu_write8(cpustate,addr,data)
#define READ32(addr)		cpu_read32(cpustate,addr)
#define WRITE32(addr,data)	cpu_write32(cpustate,addr,data)

#define ARM_DEBUG_CORE 0
#define ARM_DEBUG_COPRO 0

enum
{
	eARM_MODE_USER	= 0x0,
	eARM_MODE_FIQ	= 0x1,
	eARM_MODE_IRQ	= 0x2,
	eARM_MODE_SVC	= 0x3,

	kNumModes
};

/* There are 27 32 bit processor registers */
enum
{
	eR0=0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
	eR8,eR9,eR10,eR11,eR12,
	eR13, /* Stack Pointer */
	eR14, /* Link Register (holds return address) */
	eR15, /* Program Counter */

	/* Fast Interrupt */
	eR8_FIQ,eR9_FIQ,eR10_FIQ,eR11_FIQ,eR12_FIQ,eR13_FIQ,eR14_FIQ,

	/* IRQ */
	eR13_IRQ,eR14_IRQ,

	/* Software Interrupt */
	eR13_SVC,eR14_SVC,

	kNumRegisters
};

/* 16 processor registers are visible at any given time,
 * banked depending on processor mode.
 */
static const int sRegisterTable[kNumModes][16] =
{
	{ /* USR */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8,eR9,eR10,eR11,eR12,
		eR13,eR14,
		eR15
	},
	{ /* FIQ */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8_FIQ,eR9_FIQ,eR10_FIQ,eR11_FIQ,eR12_FIQ,
		eR13_FIQ,eR14_FIQ,
		eR15
	},
	{ /* IRQ */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8,eR9,eR10,eR11,eR12,
		eR13_IRQ,eR14_IRQ,
		eR15
	},
	{ /* SVC */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8,eR9,eR10,eR11,eR12,
		eR13_SVC,eR14_SVC,
		eR15
	}
};

#define N_BIT	31
#define Z_BIT	30
#define C_BIT	29
#define V_BIT	28
#define I_BIT	27
#define F_BIT	26

#define N_MASK	((UINT32)(1<<N_BIT)) /* Negative flag */
#define Z_MASK	((UINT32)(1<<Z_BIT)) /* Zero flag */
#define C_MASK	((UINT32)(1<<C_BIT)) /* Carry flag */
#define V_MASK	((UINT32)(1<<V_BIT)) /* oVerflow flag */
#define I_MASK	((UINT32)(1<<I_BIT)) /* Interrupt request disable */
#define F_MASK	((UINT32)(1<<F_BIT)) /* Fast interrupt request disable */

#define N_IS_SET(pc)	((pc) & N_MASK)
#define Z_IS_SET(pc)	((pc) & Z_MASK)
#define C_IS_SET(pc)	((pc) & C_MASK)
#define V_IS_SET(pc)	((pc) & V_MASK)
#define I_IS_SET(pc)	((pc) & I_MASK)
#define F_IS_SET(pc)	((pc) & F_MASK)

#define N_IS_CLEAR(pc)	(!N_IS_SET(pc))
#define Z_IS_CLEAR(pc)	(!Z_IS_SET(pc))
#define C_IS_CLEAR(pc)	(!C_IS_SET(pc))
#define V_IS_CLEAR(pc)	(!V_IS_SET(pc))
#define I_IS_CLEAR(pc)	(!I_IS_SET(pc))
#define F_IS_CLEAR(pc)	(!F_IS_SET(pc))

#define PSR_MASK		((UINT32) 0xf0000000u)
#define IRQ_MASK		((UINT32) 0x0c000000u)
#define ADDRESS_MASK	((UINT32) 0x03fffffcu)
#define MODE_MASK		((UINT32) 0x00000003u)

#define R15						cpustate->sArmRegister[eR15]
#define MODE					(R15&0x03)
#define SIGN_BIT				((UINT32)(1<<31))
#define SIGN_BITS_DIFFER(a,b)	(((a)^(b)) >> 31)

/* Deconstructing an instruction */

#define INSN_COND			((UINT32) 0xf0000000u)
#define INSN_SDT_L			((UINT32) 0x00100000u)
#define INSN_SDT_W			((UINT32) 0x00200000u)
#define INSN_SDT_B			((UINT32) 0x00400000u)
#define INSN_SDT_U			((UINT32) 0x00800000u)
#define INSN_SDT_P			((UINT32) 0x01000000u)
#define INSN_BDT_L			((UINT32) 0x00100000u)
#define INSN_BDT_W			((UINT32) 0x00200000u)
#define INSN_BDT_S			((UINT32) 0x00400000u)
#define INSN_BDT_U			((UINT32) 0x00800000u)
#define INSN_BDT_P			((UINT32) 0x01000000u)
#define INSN_BDT_REGS		((UINT32) 0x0000ffffu)
#define INSN_SDT_IMM		((UINT32) 0x00000fffu)
#define INSN_MUL_A			((UINT32) 0x00200000u)
#define INSN_MUL_RM			((UINT32) 0x0000000fu)
#define INSN_MUL_RS			((UINT32) 0x00000f00u)
#define INSN_MUL_RN			((UINT32) 0x0000f000u)
#define INSN_MUL_RD			((UINT32) 0x000f0000u)
#define INSN_I				((UINT32) 0x02000000u)
#define INSN_OPCODE			((UINT32) 0x01e00000u)
#define INSN_S				((UINT32) 0x00100000u)
#define INSN_BL				((UINT32) 0x01000000u)
#define INSN_BRANCH			((UINT32) 0x00ffffffu)
#define INSN_SWI			((UINT32) 0x00ffffffu)
#define INSN_RN				((UINT32) 0x000f0000u)
#define INSN_RD				((UINT32) 0x0000f000u)
#define INSN_OP2			((UINT32) 0x00000fffu)
#define INSN_OP2_SHIFT		((UINT32) 0x00000f80u)
#define INSN_OP2_SHIFT_TYPE	((UINT32) 0x00000070u)
#define INSN_OP2_RM			((UINT32) 0x0000000fu)
#define INSN_OP2_ROTATE		((UINT32) 0x00000f00u)
#define INSN_OP2_IMM		((UINT32) 0x000000ffu)
#define INSN_OP2_SHIFT_TYPE_SHIFT	4
#define INSN_OP2_SHIFT_SHIFT		7
#define INSN_OP2_ROTATE_SHIFT		8
#define INSN_MUL_RS_SHIFT			8
#define INSN_MUL_RN_SHIFT			12
#define INSN_MUL_RD_SHIFT			16
#define INSN_OPCODE_SHIFT			21
#define INSN_RN_SHIFT				16
#define INSN_RD_SHIFT				12
#define INSN_COND_SHIFT				28

#define S_CYCLE 1
#define N_CYCLE 1
#define I_CYCLE 1

enum
{
	OPCODE_AND,	/* 0000 */
	OPCODE_EOR,	/* 0001 */
	OPCODE_SUB,	/* 0010 */
	OPCODE_RSB,	/* 0011 */
	OPCODE_ADD,	/* 0100 */
	OPCODE_ADC,	/* 0101 */
	OPCODE_SBC,	/* 0110 */
	OPCODE_RSC,	/* 0111 */
	OPCODE_TST,	/* 1000 */
	OPCODE_TEQ,	/* 1001 */
	OPCODE_CMP,	/* 1010 */
	OPCODE_CMN,	/* 1011 */
	OPCODE_ORR,	/* 1100 */
	OPCODE_MOV,	/* 1101 */
	OPCODE_BIC,	/* 1110 */
	OPCODE_MVN	/* 1111 */
};

enum
{
	COND_EQ = 0,	/* Z: equal */
	COND_NE,		/* ~Z: not equal */
	COND_CS, COND_HS = 2,	/* C: unsigned higher or same */
	COND_CC, COND_LO = 3,	/* ~C: unsigned lower */
	COND_MI,		/* N: negative */
	COND_PL,		/* ~N: positive or zero */
	COND_VS,		/* V: overflow */
	COND_VC,		/* ~V: no overflow */
	COND_HI,		/* C && ~Z: unsigned higher */
	COND_LS,		/* ~C || Z: unsigned lower or same */
	COND_GE,		/* N == V: greater or equal */
	COND_LT,		/* N != V: less than */
	COND_GT,		/* ~Z && (N == V): greater than */
	COND_LE,		/* Z || (N != V): less than or equal */
	COND_AL,		/* always */
	COND_NV			/* never */
};

#define LSL(v,s) ((v) << (s))
#define LSR(v,s) ((v) >> (s))
#define ROL(v,s) (LSL((v),(s)) | (LSR((v),32u - (s))))
#define ROR(v,s) (LSR((v),(s)) | (LSL((v),32u - (s))))

/* Private Data */

/* sArmRegister defines the CPU state */
typedef struct
{
	int icount;
	UINT32 sArmRegister[kNumRegisters];
	UINT32 coproRegister[16];
	UINT8 pendingIrq;
	UINT8 pendingFiq;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	endianness_t endian;
} ARM_REGS;

/* Prototypes */
static void HandleALU( ARM_REGS* cpustate, UINT32 insn);
static void HandleMul( ARM_REGS* cpustate, UINT32 insn);
static void HandleBranch( ARM_REGS* cpustate, UINT32 insn);
static void HandleMemSingle( ARM_REGS* cpustate, UINT32 insn);
static void HandleMemBlock( ARM_REGS* cpustate, UINT32 insn);
static void HandleCoPro( ARM_REGS* cpustate, UINT32 insn);
static UINT32 decodeShift( ARM_REGS* cpustate, UINT32 insn, UINT32 *pCarry);
static void arm_check_irq_state(ARM_REGS* cpustate);

/***************************************************************************/

INLINE ARM_REGS *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ARM || device->type() == ARM_BE);
	return (ARM_REGS *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void cpu_write32( ARM_REGS* cpustate, int addr, UINT32 data )
{
	/* Unaligned writes are treated as normal writes */
	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_dword(addr&ADDRESS_MASK,data);
	else
		cpustate->program->write_dword(addr&ADDRESS_MASK,data);
	if (ARM_DEBUG_CORE && addr&3) logerror("%08x: Unaligned write %08x\n",R15,addr);
}

INLINE void cpu_write8( ARM_REGS* cpustate, int addr, UINT8 data )
{
	if ( cpustate->endian == ENDIANNESS_BIG )
		cpustate->program->write_byte(addr,data);
	else
		cpustate->program->write_byte(addr,data);
}

INLINE UINT32 cpu_read32( ARM_REGS* cpustate, int addr )
{
	UINT32 result;

	if ( cpustate->endian == ENDIANNESS_BIG )
		result = cpustate->program->read_dword(addr&ADDRESS_MASK);
	else
		result = cpustate->program->read_dword(addr&ADDRESS_MASK);

	/* Unaligned reads rotate the word, they never combine words */
	if (addr&3) {
		if (ARM_DEBUG_CORE && addr&1)
			logerror("%08x: Unaligned byte read %08x\n",R15,addr);

		if ((addr&3)==1)
			return ((result&0x000000ff)<<24)|((result&0xffffff00)>> 8);
		if ((addr&3)==2)
			return ((result&0x0000ffff)<<16)|((result&0xffff0000)>>16);
		if ((addr&3)==3)
			return ((result&0x00ffffff)<< 8)|((result&0xff000000)>>24);
	}

	return result;
}

INLINE UINT8 cpu_read8( ARM_REGS* cpustate, int addr )
{
	if ( cpustate->endian == ENDIANNESS_BIG )
		return cpustate->program->read_byte(addr);
	else
		return cpustate->program->read_byte(addr);
}

INLINE UINT32 GetRegister( ARM_REGS* cpustate, int rIndex )
{
	return cpustate->sArmRegister[sRegisterTable[MODE][rIndex]];
}

INLINE void SetRegister( ARM_REGS* cpustate, int rIndex, UINT32 value )
{
	cpustate->sArmRegister[sRegisterTable[MODE][rIndex]] = value;
}

INLINE UINT32 GetModeRegister( ARM_REGS* cpustate, int mode, int rIndex )
{
	return cpustate->sArmRegister[sRegisterTable[mode][rIndex]];
}

INLINE void SetModeRegister( ARM_REGS* cpustate, int mode, int rIndex, UINT32 value )
{
	cpustate->sArmRegister[sRegisterTable[mode][rIndex]] = value;
}


/***************************************************************************/

static CPU_RESET( arm )
{
	ARM_REGS *cpustate = get_safe_token(device);

	device_irq_acknowledge_callback save_irqcallback = cpustate->irq_callback;
	endianness_t save_endian = cpustate->endian;

	memset(cpustate, 0, sizeof(ARM_REGS));
	cpustate->irq_callback = save_irqcallback;
	cpustate->endian = save_endian;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	/* start up in SVC mode with interrupts disabled. */
	R15 = eARM_MODE_SVC|I_MASK|F_MASK;
}

static CPU_EXIT( arm )
{
	/* nothing to do here */
}

static CPU_EXECUTE( arm )
{
	UINT32 pc;
	UINT32 insn;
	ARM_REGS *cpustate = get_safe_token(device);

	do
	{
		debugger_instruction_hook(device, R15 & ADDRESS_MASK);

		/* load instruction */
		pc = R15;
		insn = cpustate->direct->read_decrypted_dword( pc & ADDRESS_MASK );

		switch (insn >> INSN_COND_SHIFT)
		{
		case COND_EQ:
			if (Z_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_NE:
			if (Z_IS_SET(pc)) goto L_Next;
			break;
		case COND_CS:
			if (C_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_CC:
			if (C_IS_SET(pc)) goto L_Next;
			break;
		case COND_MI:
			if (N_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_PL:
			if (N_IS_SET(pc)) goto L_Next;
			break;
		case COND_VS:
			if (V_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_VC:
			if (V_IS_SET(pc)) goto L_Next;
			break;
		case COND_HI:
			if (C_IS_CLEAR(pc) || Z_IS_SET(pc)) goto L_Next;
			break;
		case COND_LS:
			if (C_IS_SET(pc) && Z_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_GE:
			if (!(pc & N_MASK) != !(pc & V_MASK)) goto L_Next; /* Use x ^ (x >> ...) method */
			break;
		case COND_LT:
			if (!(pc & N_MASK) == !(pc & V_MASK)) goto L_Next;
			break;
		case COND_GT:
			if (Z_IS_SET(pc) || (!(pc & N_MASK) != !(pc & V_MASK))) goto L_Next;
			break;
		case COND_LE:
			if (Z_IS_CLEAR(pc) && (!(pc & N_MASK) == !(pc & V_MASK))) goto L_Next;
			break;
		case COND_NV:
			goto L_Next;
		}
		/* Condition satisfied, so decode the instruction */
		if ((insn & 0x0fc000f0u) == 0x00000090u)	/* Multiplication */
		{
			HandleMul(cpustate, insn);
			R15 += 4;
		}
		else if (!(insn & 0x0c000000u)) /* Data processing */
		{
			HandleALU(cpustate, insn);
		}
		else if ((insn & 0x0c000000u) == 0x04000000u) /* Single data access */
		{
			HandleMemSingle(cpustate, insn);
			R15 += 4;
		}
		else if ((insn & 0x0e000000u) == 0x08000000u ) /* Block data access */
		{
			HandleMemBlock(cpustate, insn);
			R15 += 4;
		}
		else if ((insn & 0x0e000000u) == 0x0a000000u)	/* Branch */
		{
			HandleBranch(cpustate, insn);
		}
		else if ((insn & 0x0f000000u) == 0x0e000000u)	/* Coprocessor */
		{
			HandleCoPro(cpustate, insn);
			R15 += 4;
		}
		else if ((insn & 0x0f000000u) == 0x0f000000u)	/* Software interrupt */
		{
			pc=R15+4;
			R15 = eARM_MODE_SVC;	/* Set SVC mode so PC is saved to correct R14 bank */
			SetRegister( cpustate, 14, pc );	/* save PC */
			R15 = (pc&PSR_MASK)|(pc&IRQ_MASK)|0x8|eARM_MODE_SVC|I_MASK|(pc&MODE_MASK);
			cpustate->icount -= 2 * S_CYCLE + N_CYCLE;
		}
		else /* Undefined */
		{
			logerror("%08x:  Undefined instruction\n",R15);
		L_Next:
			cpustate->icount -= S_CYCLE;
			R15 += 4;
		}

		arm_check_irq_state(cpustate);

	} while( cpustate->icount > 0 );
} /* arm_execute */


static void arm_check_irq_state(ARM_REGS* cpustate)
{
	UINT32 pc = R15+4; /* save old pc (already incremented in pipeline) */;

	/* Exception priorities (from ARM6, not specifically ARM2/3):

        Reset
        Data abort
        FIRQ
        IRQ
        Prefetch abort
        Undefined instruction
    */

	if (cpustate->pendingFiq && (pc&F_MASK)==0) {
		R15 = eARM_MODE_FIQ;	/* Set FIQ mode so PC is saved to correct R14 bank */
		SetRegister( cpustate, 14, pc );	/* save PC */
		R15 = (pc&PSR_MASK)|(pc&IRQ_MASK)|0x1c|eARM_MODE_FIQ|I_MASK|F_MASK; /* Mask both IRQ & FIRQ, set PC=0x1c */
		cpustate->pendingFiq=0;
		return;
	}

	if (cpustate->pendingIrq && (pc&I_MASK)==0) {
		R15 = eARM_MODE_IRQ;	/* Set IRQ mode so PC is saved to correct R14 bank */
		SetRegister( cpustate, 14, pc );	/* save PC */
		R15 = (pc&PSR_MASK)|(pc&IRQ_MASK)|0x18|eARM_MODE_IRQ|I_MASK|(pc&F_MASK); /* Mask only IRQ, set PC=0x18 */
		cpustate->pendingIrq=0;
		return;
	}
}

static void set_irq_line(ARM_REGS* cpustate, int irqline, int state)
{
	switch (irqline) {

	case ARM_IRQ_LINE: /* IRQ */
		if (state && (R15&0x3)!=eARM_MODE_IRQ) /* Don't allow nested IRQs */
			cpustate->pendingIrq=1;
		else
			cpustate->pendingIrq=0;
		break;

	case ARM_FIRQ_LINE: /* FIRQ */
		if (state && (R15&0x3)!=eARM_MODE_FIQ) /* Don't allow nested FIRQs */
			cpustate->pendingFiq=1;
		else
			cpustate->pendingFiq=0;
		break;
	}

	arm_check_irq_state(cpustate);
}

static CPU_INIT( arm )
{
	ARM_REGS *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->endian = ENDIANNESS_LITTLE;

	device->save_item(NAME(cpustate->sArmRegister));
	device->save_item(NAME(cpustate->coproRegister));
	device->save_item(NAME(cpustate->pendingIrq));
	device->save_item(NAME(cpustate->pendingFiq));
}


static CPU_INIT( arm_be )
{
	ARM_REGS *cpustate = get_safe_token(device);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->endian = ENDIANNESS_BIG;

	device->save_item(NAME(cpustate->sArmRegister));
	device->save_item(NAME(cpustate->coproRegister));
	device->save_item(NAME(cpustate->pendingIrq));
	device->save_item(NAME(cpustate->pendingFiq));
}


/***************************************************************************/

static void HandleBranch( ARM_REGS* cpustate, UINT32 insn )
{
	UINT32 off = (insn & INSN_BRANCH) << 2;

	/* Save PC into LR if this is a branch with link */
	if (insn & INSN_BL)
	{
		SetRegister(cpustate, 14,R15 + 4);
	}

	/* Sign-extend the 24-bit offset in our calculations */
	if (off & 0x2000000u)
	{
		R15 -= ((~(off | 0xfc000000u)) + 1) - 8;
	}
	else
	{
		R15 += off + 8;
	}
	cpustate->icount -= 2 * S_CYCLE + N_CYCLE;
}

static void HandleMemSingle( ARM_REGS* cpustate, UINT32 insn )
{
	UINT32 rn, rnv, off, rd;

	/* Fetch the offset */
	if (insn & INSN_I)
	{
		off = decodeShift(cpustate, insn, NULL);
	}
	else
	{
		off = insn & INSN_SDT_IMM;
	}

	/* Calculate Rn, accounting for PC */
	rn = (insn & INSN_RN) >> INSN_RN_SHIFT;

//  if (rn==0xf) logerror("%08x:  Source R15\n",R15);

	if (insn & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if (insn & INSN_SDT_U)
		{
			if (rn != eR15)
				rnv = (GetRegister(cpustate, rn) + off);
			else
				rnv = (R15 & ADDRESS_MASK) + off;
		}
		else
		{
			if (rn != eR15)
				rnv = (GetRegister(cpustate, rn) - off);
			else
				rnv = (R15 & ADDRESS_MASK) - off;
		}

		if (insn & INSN_SDT_W)
		{
			SetRegister(cpustate, rn,rnv);
			if (ARM_DEBUG_CORE && rn == eR15)
				logerror("writeback R15 %08x\n", R15);
		}
		else if (rn == eR15)
		{
			rnv = rnv + 8;
		}
	}
	else
	{
		/* Post-indexed addressing */
		if (rn == eR15)
		{
			rnv = (R15 & ADDRESS_MASK) + 8;
		}
		else
		{
			rnv = GetRegister(cpustate, rn);
		}
	}

	/* Do the transfer */
	rd = (insn & INSN_RD) >> INSN_RD_SHIFT;
	if (insn & INSN_SDT_L)
	{
		/* Load */
		cpustate->icount -= S_CYCLE + I_CYCLE + N_CYCLE;
		if (insn & INSN_SDT_B)
		{
			if (ARM_DEBUG_CORE && rd == eR15)
				logerror("read byte R15 %08x\n", R15);
			SetRegister(cpustate, rd,(UINT32) READ8(rnv) );
		}
		else
		{
			if (rd == eR15)
			{
				R15 = (READ32(rnv) & ADDRESS_MASK) | (R15 & PSR_MASK) | (R15 & MODE_MASK);

				/*
                The docs are explicit in that the bottom bits should be masked off
                when writing to R15 in this way, however World Cup Volleyball 95 has
                an example of an unaligned jump (bottom bits = 2) where execution
                should definitely continue from the rounded up address.

                In other cases, 4 is subracted from R15 here to account for pipelining.
                */
				if ((READ32(rnv)&3)==0)
					R15 -= 4;

				cpustate->icount -= S_CYCLE + N_CYCLE;
			}
			else
			{
				SetRegister(cpustate, rd, READ32(rnv));
			}
		}
	}
	else
	{
		/* Store */
		cpustate->icount -= 2 * N_CYCLE;
		if (insn & INSN_SDT_B)
		{
			if (ARM_DEBUG_CORE && rd==eR15)
				logerror("Wrote R15 in byte mode\n");

			WRITE8(rnv, (UINT8) GetRegister(cpustate, rd) & 0xffu);
		}
		else
		{
			if (ARM_DEBUG_CORE && rd==eR15)
				logerror("Wrote R15 in 32bit mode\n");

			WRITE32(rnv, rd == eR15 ? R15 + 8 : GetRegister(cpustate, rd));
		}
	}

	/* Do post-indexing writeback */
	if (!(insn & INSN_SDT_P)/* && (insn&INSN_SDT_W)*/)
	{
		if (insn & INSN_SDT_U)
		{
			/* Writeback is applied in pipeline, before value is read from mem,
                so writeback is effectively ignored */
			if (rd==rn) {
				SetRegister(cpustate, rn,GetRegister(cpustate, rd));
			}
			else {

				if ((insn&INSN_SDT_W)!=0)
				logerror("%08x:  RegisterWritebackIncrement %d %d %d\n",R15,(insn & INSN_SDT_P)!=0,(insn&INSN_SDT_W)!=0,(insn & INSN_SDT_U)!=0);

				SetRegister(cpustate, rn,(rnv + off));
			}
		}
		else
		{
			/* Writeback is applied in pipeline, before value is read from mem,
                so writeback is effectively ignored */
			if (rd==rn) {
				SetRegister(cpustate, rn,GetRegister(cpustate, rd));
			}
			else {
				SetRegister(cpustate, rn,(rnv - off));

				if ((insn&INSN_SDT_W)!=0)
				logerror("%08x:  RegisterWritebackDecrement %d %d %d\n",R15,(insn & INSN_SDT_P)!=0,(insn&INSN_SDT_W)!=0,(insn & INSN_SDT_U)!=0);
			}
		}
	}
} /* HandleMemSingle */

#define IsNeg(i) ((i) >> 31)
#define IsPos(i) ((~(i)) >> 31)

/* Set NZCV flags for ADDS / SUBS */

#define HandleALUAddFlags(rd, rn, op2) \
  if (insn & INSN_S) \
    R15 = \
      ((R15 &~ (N_MASK | Z_MASK | V_MASK | C_MASK)) \
      | (((!SIGN_BITS_DIFFER(rn, op2)) && SIGN_BITS_DIFFER(rn, rd)) \
          << V_BIT) \
      | (((~(rn)) < (op2)) << C_BIT) \
      | HandleALUNZFlags(rd)) \
      + 4; \
  else R15 += 4;

#define HandleALUSubFlags(rd, rn, op2) \
  if (insn & INSN_S) \
    R15 = \
      ((R15 &~ (N_MASK | Z_MASK | V_MASK | C_MASK)) \
      | ((SIGN_BITS_DIFFER(rn, op2) && SIGN_BITS_DIFFER(rn, rd)) \
          << V_BIT) \
      | (((IsNeg(rn) & IsPos(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsPos(op2) & IsPos(rd))) ? C_MASK : 0) \
      | HandleALUNZFlags(rd)) \
      + 4; \
  else R15 += 4;

/* Set NZC flags for logical operations. */

#define HandleALUNZFlags(rd) \
  (((rd) & SIGN_BIT) | ((!(rd)) << Z_BIT))

#define HandleALULogicalFlags(rd, sc) \
  if (insn & INSN_S) \
    R15 = ((R15 &~ (N_MASK | Z_MASK | C_MASK)) \
                     | HandleALUNZFlags(rd) \
                     | (((sc) != 0) << C_BIT)) + 4; \
  else R15 += 4;

static void HandleALU( ARM_REGS* cpustate, UINT32 insn )
{
	UINT32 op2, sc=0, rd, rn, opcode;
	UINT32 by, rdn;

	opcode = (insn & INSN_OPCODE) >> INSN_OPCODE_SHIFT;
	cpustate->icount -= S_CYCLE;

	rd = 0;
	rn = 0;

	/* Construct Op2 */
	if (insn & INSN_I)
	{
		/* Immediate constant */
		by = (insn & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT;
		if (by)
		{
			op2 = ROR(insn & INSN_OP2_IMM, by << 1);
			sc = op2 & SIGN_BIT;
		}
		else
		{
			op2 = insn & INSN_OP2;
			sc = R15 & C_MASK;
		}
	}
	else
	{
		op2 = decodeShift(cpustate, insn, (insn & INSN_S) ? &sc : NULL);

        	if (!(insn & INSN_S))
			sc=0;
	}

	/* Calculate Rn to account for pipelining */
	if ((opcode & 0xd) != 0xd) /* No Rn in MOV */
	{
		if ((rn = (insn & INSN_RN) >> INSN_RN_SHIFT) == eR15)
		{
			if (ARM_DEBUG_CORE)
				logerror("%08x:  Pipelined R15 (Shift %d)\n",R15,(insn&INSN_I?8:insn&0x10u?12:12));

			/* Docs strongly suggest the mode bits should be included here, but it breaks Captain
            America, as it starts doing unaligned reads */
			rn=(R15+8)&ADDRESS_MASK;
		}
		else
		{
			rn = GetRegister(cpustate, rn);
		}
	}

	/* Perform the operation */
	switch ((insn & INSN_OPCODE) >> INSN_OPCODE_SHIFT)
	{
	/* Arithmetic operations */
	case OPCODE_SBC:
		rd = (rn - op2 - (R15 & C_MASK ? 0 : 1));
		HandleALUSubFlags(rd, rn, op2);
		break;
	case OPCODE_CMP:
	case OPCODE_SUB:
		rd = (rn - op2);
		HandleALUSubFlags(rd, rn, op2);
		break;
	case OPCODE_RSC:
		rd = (op2 - rn - (R15 & C_MASK ? 0 : 1));
		HandleALUSubFlags(rd, op2, rn);
		break;
	case OPCODE_RSB:
		rd = (op2 - rn);
		HandleALUSubFlags(rd, op2, rn);
		break;
	case OPCODE_ADC:
		rd = (rn + op2 + ((R15 & C_MASK) >> C_BIT));
		HandleALUAddFlags(rd, rn, op2);
		break;
	case OPCODE_CMN:
	case OPCODE_ADD:
		rd = (rn + op2);
		HandleALUAddFlags(rd, rn, op2);
		break;
	/* Logical operations */
	case OPCODE_AND:
	case OPCODE_TST:
		rd = rn & op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_BIC:
		rd = rn &~ op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_TEQ:
	case OPCODE_EOR:
		rd = rn ^ op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_ORR:
		rd = rn | op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_MOV:
		rd = op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_MVN:
		rd = (~op2);
		HandleALULogicalFlags(rd, sc);
		break;
	}

	/* Put the result in its register if not a test */
	rdn = (insn & INSN_RD) >> INSN_RD_SHIFT;
	if ((opcode & 0xc) != 0x8)
	{
		if (rdn == eR15 && !(insn & INSN_S))
		{
			/* Merge the old NZCV flags into the new PC value */
			R15 = (rd & ADDRESS_MASK) | (R15 & PSR_MASK) | (R15 & IRQ_MASK) | (R15&MODE_MASK);
			cpustate->icount -= S_CYCLE + N_CYCLE;
		}
		else
		{
			if (rdn==eR15)
			{
				/* S Flag is set - update PSR & mode if in non-user mode only */
				if ((R15&MODE_MASK)!=0)
				{
					SetRegister(cpustate,rdn,rd);
				}
				else
				{
					SetRegister(cpustate, rdn,(rd&ADDRESS_MASK) | (rd&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK));
				}
				cpustate->icount -= S_CYCLE + N_CYCLE;
			}
			else
			{
				SetRegister(cpustate,rdn,rd);
			}
		}
	/* TST & TEQ can affect R15 (the condition code register) with the S bit set */
	}
	else if ((rdn==eR15) && (insn & INSN_S))
	{
		// update only the flags
		if ((R15&MODE_MASK)!=0)
		{
			// combine the flags from rd with the address from R15
			rd &= ~ADDRESS_MASK;
			rd |= (R15 & ADDRESS_MASK);
			SetRegister(cpustate,rdn,rd);
		}
		else
		{
			// combine the flags from rd with the address from R15
			rd &= ~ADDRESS_MASK;	// clear address part of RD
			rd |= (R15 & ADDRESS_MASK);	// RD = address part of R15
			SetRegister(cpustate, rdn,(rd&ADDRESS_MASK) | (rd&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK));
		}
		cpustate->icount -= S_CYCLE + N_CYCLE;
	}
}

static void HandleMul( ARM_REGS* cpustate, UINT32 insn)
{
	UINT32 r;

	cpustate->icount -= S_CYCLE + I_CYCLE;
	/* should be:
            Range of Rs            Number of cycles

               &0 -- &1            1S + 1I
               &2 -- &7            1S + 2I
               &8 -- &1F           1S + 3I
              &20 -- &7F           1S + 4I
              &80 -- &1FF          1S + 5I
             &200 -- &7FF          1S + 6I
             &800 -- &1FFF         1S + 7I
            &2000 -- &7FFF         1S + 8I
            &8000 -- &1FFFF        1S + 9I
           &20000 -- &7FFFF        1S + 10I
           &80000 -- &1FFFFF       1S + 11I
          &200000 -- &7FFFFF       1S + 12I
          &800000 -- &1FFFFFF      1S + 13I
         &2000000 -- &7FFFFFF      1S + 14I
         &8000000 -- &1FFFFFFF     1S + 15I
        &20000000 -- &FFFFFFFF     1S + 16I
  */

	/* Do the basic multiply of Rm and Rs */
	r =	GetRegister( cpustate, insn&INSN_MUL_RM ) *
		GetRegister( cpustate, (insn&INSN_MUL_RS)>>INSN_MUL_RS_SHIFT );

	if (ARM_DEBUG_CORE && ((insn&INSN_MUL_RM)==0xf
		|| ((insn&INSN_MUL_RS)>>INSN_MUL_RS_SHIFT )==0xf
		|| ((insn&INSN_MUL_RN)>>INSN_MUL_RN_SHIFT)==0xf)
		)
		logerror("%08x:  R15 used in mult\n",R15);

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		r += GetRegister(cpustate, (insn&INSN_MUL_RN)>>INSN_MUL_RN_SHIFT);
	}

	/* Write the result */
	SetRegister(cpustate,(insn&INSN_MUL_RD)>>INSN_MUL_RD_SHIFT,r);

	/* Set N and Z if asked */
	if( insn & INSN_S )
	{
		R15 = (R15 &~ (N_MASK | Z_MASK)) | HandleALUNZFlags(r);
	}
}

static int loadInc ( ARM_REGS* cpustate, UINT32 pat, UINT32 rbv, UINT32 s)
{
	int i,result;

	result = 0;
	for( i=0; i<16; i++ )
	{
		if( (pat>>i)&1 )
		{
			if (i==15) {
				if (s) /* Pull full contents from stack */
					SetRegister( cpustate, 15, READ32(rbv+=4) );
				else /* Pull only address, preserve mode & status flags */
					SetRegister( cpustate, 15, (R15&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK) | ((READ32(rbv+=4))&ADDRESS_MASK) );
			} else
				SetRegister( cpustate, i, READ32(rbv+=4) );

			result++;
		}
	}
	return result;
}

static int loadDec( ARM_REGS* cpustate, UINT32 pat, UINT32 rbv, UINT32 s, UINT32* deferredR15, int* defer)
{
	int i,result;

	result = 0;
	for( i=15; i>=0; i-- )
	{
		if( (pat>>i)&1 )
		{
			if (i==15) {
				*defer=1;
				if (s) /* Pull full contents from stack */
					*deferredR15=READ32(rbv-=4);
				else /* Pull only address, preserve mode & status flags */
					*deferredR15=(R15&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK) | ((READ32(rbv-=4))&ADDRESS_MASK);
			}
			else
				SetRegister( cpustate, i, READ32(rbv -=4) );
			result++;
		}
	}
	return result;
}

static int storeInc( ARM_REGS* cpustate, UINT32 pat, UINT32 rbv)
{
	int i,result;

	result = 0;
	for( i=0; i<16; i++ )
	{
		if( (pat>>i)&1 )
		{
			if (ARM_DEBUG_CORE && i==15) /* R15 is plus 12 from address of STM */
				logerror("%08x: StoreInc on R15\n",R15);

			WRITE32( rbv += 4, GetRegister(cpustate, i) );
			result++;
		}
	}
	return result;
} /* storeInc */

static int storeDec( ARM_REGS* cpustate, UINT32 pat, UINT32 rbv)
{
	int i,result;

	result = 0;
	for( i=15; i>=0; i-- )
	{
		if( (pat>>i)&1 )
		{
			if (ARM_DEBUG_CORE && i==15) /* R15 is plus 12 from address of STM */
				logerror("%08x: StoreDec on R15\n",R15);

			WRITE32( rbv -= 4, GetRegister(cpustate, i) );
			result++;
		}
	}
	return result;
} /* storeDec */

static void HandleMemBlock( ARM_REGS* cpustate, UINT32 insn )
{
	UINT32 rb = (insn & INSN_RN) >> INSN_RN_SHIFT;
	UINT32 rbp = GetRegister(cpustate, rb);
	int result;

	if (ARM_DEBUG_CORE && insn & INSN_BDT_S)
		logerror("%08x:  S Bit set in MEMBLOCK\n",R15);

	if (insn & INSN_BDT_L)
	{
		/* Loading */
		if (insn & INSN_BDT_U)
		{
			int mode = MODE;

			/* Incrementing */
			if (!(insn & INSN_BDT_P)) rbp = rbp + (- 4);

			result = loadInc( cpustate, insn & 0xffff, rbp, insn&INSN_BDT_S );

			if (insn & 0x8000) {
				R15-=4;
				cpustate->icount -= S_CYCLE + N_CYCLE;
			}

			if (insn & INSN_BDT_W)
			{
				/* Arm docs notes: The base register can always be loaded without any problems.
                However, don't specify writeback if the base register is being loaded -
                you can't end up with both a written-back value and a loaded value in the base register!

                However - Fighter's History does exactly that at 0x121e4 (LDMUW [R13], { R13-R15 })!

                This emulator implementation skips applying writeback in this case, which is confirmed
                correct for this situation, but that is not necessarily true for all ARM hardware
                implementations (the results are officially undefined).
                */

				if (ARM_DEBUG_CORE && rb==15)
					logerror("%08x:  Illegal LDRM writeback to r15\n",R15);

				if ((insn&(1<<rb))==0)
					SetModeRegister(cpustate, mode, rb, GetModeRegister(cpustate, mode, rb) + result * 4);
				else if (ARM_DEBUG_CORE)
					logerror("%08x:  Illegal LDRM writeback to base register (%d)\n",R15, rb);
			}
		}
		else
		{
			UINT32 deferredR15=0;
			int defer=0;

			/* Decrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp - (- 4);
			}

			result = loadDec( cpustate, insn&0xffff, rbp, insn&INSN_BDT_S, &deferredR15, &defer );

			if (insn & INSN_BDT_W)
			{
				if (rb==0xf)
					logerror("%08x:  Illegal LDRM writeback to r15\n",R15);
				SetRegister(cpustate,rb,GetRegister(cpustate, rb)-result*4);
			}

			// If R15 is pulled from memory we defer setting it until after writeback
			// is performed, else we may writeback to the wrong context (ie, the new
			// context if the mode has changed as a result of the R15 read)
			if (defer)
				SetRegister(cpustate, 15, deferredR15);

			if (insn & 0x8000) {
				cpustate->icount -= S_CYCLE + N_CYCLE;
				R15-=4;
			}
		}
		cpustate->icount -= result * S_CYCLE + N_CYCLE + I_CYCLE;
	} /* Loading */
	else
	{
		/* Storing

            ARM docs notes: Storing a list of registers including the base register using writeback
            will write the value of the base register before writeback to memory only if the base
            register is the first in the list. Otherwise, the value which is used is not defined.

        */
		if (insn & (1<<eR15))
		{
			if (ARM_DEBUG_CORE)
				logerror("%08x: Writing R15 in strm\n",R15);

			/* special case handling if writing to PC */
			R15 += 12;
		}
		if (insn & INSN_BDT_U)
		{
			/* Incrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp + (- 4);
			}
			result = storeInc( cpustate, insn&0xffff, rbp );
			if( insn & INSN_BDT_W )
			{
				SetRegister(cpustate,rb,GetRegister(cpustate, rb)+result*4);
			}
		}
		else
		{
			/* Decrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp - (- 4);
			}
			result = storeDec( cpustate, insn&0xffff, rbp );
			if( insn & INSN_BDT_W )
			{
				SetRegister(cpustate,rb,GetRegister(cpustate, rb)-result*4);
			}
		}
		if( insn & (1<<eR15) )
			R15 -= 12;

		cpustate->icount -= (result - 1) * S_CYCLE + 2 * N_CYCLE;
	}
} /* HandleMemBlock */



/* Decodes an Op2-style shifted-register form.  If @carry@ is non-zero the
 * shifter carry output will manifest itself as @*carry == 0@ for carry clear
 * and @*carry != 0@ for carry set.
 */
static UINT32 decodeShift( ARM_REGS* cpustate, UINT32 insn, UINT32 *pCarry)
{
	UINT32 k	= (insn & INSN_OP2_SHIFT) >> INSN_OP2_SHIFT_SHIFT;
	UINT32 rm	= GetRegister( cpustate, insn & INSN_OP2_RM );
	UINT32 t	= (insn & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;

	if ((insn & INSN_OP2_RM)==0xf) {
		/* If hardwired shift, then PC is 8 bytes ahead, else if register shift
        is used, then 12 bytes - TODO?? */
		rm+=8;
	}

	/* All shift types ending in 1 are Rk, not #k */
	if( t & 1 )
	{
//      logerror("%08x:  RegShift %02x %02x\n",R15, k>>1,GetRegister(cpustate, k >> 1));
		if (ARM_DEBUG_CORE && (insn&0x80)==0x80)
			logerror("%08x:  RegShift ERROR (p36)\n",R15);

		//see p35 for check on this
		k = GetRegister(cpustate, k >> 1)&0x1f;
		cpustate->icount -= S_CYCLE;
		if( k == 0 ) /* Register shift by 0 is a no-op */
		{
//          logerror("%08x:  NO-OP Regshift\n",R15);
			if (pCarry) *pCarry = R15 & C_MASK;
			return rm;
		}
	}
	/* Decode the shift type and perform the shift */
	switch (t >> 1)
	{
	case 0:						/* LSL */
		if (pCarry)
		{
			*pCarry = k ? (rm & (1 << (32 - k))) : (R15 & C_MASK);
		}
		return k ? LSL(rm, k) : rm;

	case 1:			    			/* LSR */
		if (k == 0 || k == 32)
		{
			if (pCarry) *pCarry = rm & SIGN_BIT;
			return 0;
		}
		else if (k > 32)
		{
			if (pCarry) *pCarry = 0;
			return 0;
		}
		else
		{
			if (pCarry) *pCarry = (rm & (1 << (k - 1)));
			return LSR(rm, k);
		}
		break;

	case 2:						/* ASR */
		if (k == 0 || k > 32)
			k = 32;
		if (pCarry) *pCarry = (rm & (1 << (k - 1)));
		if (k >= 32)
			return rm & SIGN_BIT ? 0xffffffffu : 0;
		else
		{
			if (rm & SIGN_BIT)
				return LSR(rm, k) | (0xffffffffu << (32 - k));
			else
				return LSR(rm, k);
		}
		break;

	case 3:						/* ROR and RRX */
		if (k)
		{
			while (k > 32) k -= 32;
			if (pCarry) *pCarry = rm & SIGN_BIT;
			return ROR(rm, k);
		}
		else
		{
			if (pCarry) *pCarry = (rm & 1);
			return LSR(rm, 1) | ((R15 & C_MASK) << 2);
		}
		break;
	}

	logerror("%08x: Decodeshift error\n",R15);
	return 0;
} /* decodeShift */


static UINT32 BCDToDecimal(UINT32 value)
{
	UINT32	accumulator = 0;
	UINT32	multiplier = 1;
	int		i;

	for(i = 0; i < 8; i++)
	{
		accumulator += (value & 0xF) * multiplier;

		multiplier *= 10;
		value >>= 4;
	}

	return accumulator;
}

static UINT32 DecimalToBCD(UINT32 value)
{
	UINT32	accumulator = 0;
	UINT32	divisor = 10;
	int		i;

	for(i = 0; i < 8; i++)
	{
		UINT32	temp;

		temp = value % divisor;
		value -= temp;
		temp /= divisor / 10;

		accumulator += temp << (i * 4);

		divisor *= 10;
	}

	return accumulator;
}

static void HandleCoPro( ARM_REGS* cpustate, UINT32 insn )
{
	UINT32 rn=(insn>>12)&0xf;
	UINT32 crn=(insn>>16)&0xf;

	cpustate->icount -= S_CYCLE;

	/* MRC - transfer copro register to main register */
	if( (insn&0x0f100010)==0x0e100010 )
	{
		SetRegister(cpustate, rn, cpustate->coproRegister[crn]);

		if (ARM_DEBUG_COPRO)
			logerror("%08x:  Copro read CR%d (%08x) to R%d\n", R15, crn, cpustate->coproRegister[crn], rn);
	}
	/* MCR - transfer main register to copro register */
	else if( (insn&0x0f100010)==0x0e000010 )
	{
		cpustate->coproRegister[crn]=GetRegister(cpustate, rn);

		/* Data East 156 copro specific - trigger BCD operation */
		if (crn==2)
		{
			if (cpustate->coproRegister[crn]==0)
			{
				/* Unpack BCD */
				int v0=BCDToDecimal(cpustate->coproRegister[0]);
				int v1=BCDToDecimal(cpustate->coproRegister[1]);

				/* Repack vcd */
				cpustate->coproRegister[5]=DecimalToBCD(v0+v1);

				if (ARM_DEBUG_COPRO)
					logerror("Cmd:  Add 0 + 1, result in 5 (%08x + %08x == %08x)\n", v0, v1, cpustate->coproRegister[5]);
			}
			else if (cpustate->coproRegister[crn]==1)
			{
				/* Unpack BCD */
				int v0=BCDToDecimal(cpustate->coproRegister[0]);
				int v1=BCDToDecimal(cpustate->coproRegister[1]);

				/* Repack vcd */
				cpustate->coproRegister[5]=DecimalToBCD(v0*v1);

				if (ARM_DEBUG_COPRO)
					logerror("Cmd:  Multiply 0 * 1, result in 5 (%08x * %08x == %08x)\n", v0, v1, cpustate->coproRegister[5]);
			}
			else if (cpustate->coproRegister[crn]==3)
			{
				/* Unpack BCD */
				int v0=BCDToDecimal(cpustate->coproRegister[0]);
				int v1=BCDToDecimal(cpustate->coproRegister[1]);

				/* Repack vcd */
				cpustate->coproRegister[5]=DecimalToBCD(v0-v1);

				if (ARM_DEBUG_COPRO)
					logerror("Cmd:  Sub 0 - 1, result in 5 (%08x - %08x == %08x)\n", v0, v1, cpustate->coproRegister[5]);
			}
			else
			{
				logerror("Unknown bcd copro command %08x\n", cpustate->coproRegister[crn]);
			}
		}

		if (ARM_DEBUG_COPRO)
			logerror("%08x:  Copro write R%d (%08x) to CR%d\n", R15, rn, GetRegister(cpustate, rn), crn);
	}
	/* CDP - perform copro operation */
	else if( (insn&0x0f000010)==0x0e000000 )
	{
		/* Data East 156 copro specific divider - result in reg 3/4 */
		if (cpustate->coproRegister[1])
		{
			cpustate->coproRegister[3]=cpustate->coproRegister[0] / cpustate->coproRegister[1];
			cpustate->coproRegister[4]=cpustate->coproRegister[0] % cpustate->coproRegister[1];
		}
		else
		{
			/* Unverified */
			cpustate->coproRegister[3]=0xffffffff;
			cpustate->coproRegister[4]=0xffffffff;
		}

		if (ARM_DEBUG_COPRO)
			logerror("%08x:  Copro cdp (%08x) (3==> %08x, 4==> %08x)\n", R15, insn, cpustate->coproRegister[3], cpustate->coproRegister[4]);
	}
	else
	{
		logerror("%08x:  Unimplemented copro instruction %08x\n", R15, insn);
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( arm )
{
	ARM_REGS *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + ARM_IRQ_LINE:	set_irq_line(cpustate, ARM_IRQ_LINE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + ARM_FIRQ_LINE:	set_irq_line(cpustate, ARM_FIRQ_LINE, info->i);	break;

		case CPUINFO_INT_REGISTER + ARM32_R0:	cpustate->sArmRegister[ 0]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R1:	cpustate->sArmRegister[ 1]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R2:	cpustate->sArmRegister[ 2]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R3:	cpustate->sArmRegister[ 3]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R4:	cpustate->sArmRegister[ 4]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R5:	cpustate->sArmRegister[ 5]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R6:	cpustate->sArmRegister[ 6]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R7:	cpustate->sArmRegister[ 7]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R8:	cpustate->sArmRegister[ 8]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R9:	cpustate->sArmRegister[ 9]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R10:	cpustate->sArmRegister[10]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R11:	cpustate->sArmRegister[11]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R12:	cpustate->sArmRegister[12]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R13:	cpustate->sArmRegister[13]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R14:	cpustate->sArmRegister[14]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_R15:	cpustate->sArmRegister[15]= info->i;					break;
		case CPUINFO_INT_REGISTER + ARM32_FR8:	cpustate->sArmRegister[eR8_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_FR9:	cpustate->sArmRegister[eR9_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_FR10:	cpustate->sArmRegister[eR10_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_FR11:	cpustate->sArmRegister[eR11_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_FR12:	cpustate->sArmRegister[eR12_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_FR13:	cpustate->sArmRegister[eR13_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_FR14:	cpustate->sArmRegister[eR14_FIQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_IR13:	cpustate->sArmRegister[eR13_IRQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_IR14:	cpustate->sArmRegister[eR14_IRQ] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_SR13:	cpustate->sArmRegister[eR13_SVC] = info->i;			break;
		case CPUINFO_INT_REGISTER + ARM32_SR14:	cpustate->sArmRegister[eR14_SVC] = info->i;			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ARM32_PC:	R15 = (R15&~ADDRESS_MASK)|info->i;				break;
		case CPUINFO_INT_SP:					SetRegister(cpustate,13,info->i);						break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( arm )
{
	ARM_REGS *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(ARM_REGS);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 3;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 26;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + ARM_IRQ_LINE:	info->i = cpustate->pendingIrq;			break;
		case CPUINFO_INT_INPUT_STATE + ARM_FIRQ_LINE:	info->i = cpustate->pendingFiq;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = 0;	/* not implemented */	break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ARM32_PC:			info->i = cpustate->sArmRegister[15]&ADDRESS_MASK; break;
		case CPUINFO_INT_SP:							info->i = GetRegister(cpustate, 13);	break;

		case CPUINFO_INT_REGISTER + ARM32_R0:			info->i = cpustate->sArmRegister[ 0];	break;
		case CPUINFO_INT_REGISTER + ARM32_R1:			info->i = cpustate->sArmRegister[ 1];	break;
		case CPUINFO_INT_REGISTER + ARM32_R2:			info->i = cpustate->sArmRegister[ 2];	break;
		case CPUINFO_INT_REGISTER + ARM32_R3:			info->i = cpustate->sArmRegister[ 3];	break;
		case CPUINFO_INT_REGISTER + ARM32_R4:			info->i = cpustate->sArmRegister[ 4];	break;
		case CPUINFO_INT_REGISTER + ARM32_R5:			info->i = cpustate->sArmRegister[ 5];	break;
		case CPUINFO_INT_REGISTER + ARM32_R6:			info->i = cpustate->sArmRegister[ 6];	break;
		case CPUINFO_INT_REGISTER + ARM32_R7:			info->i = cpustate->sArmRegister[ 7];	break;
		case CPUINFO_INT_REGISTER + ARM32_R8:			info->i = cpustate->sArmRegister[ 8];	break;
		case CPUINFO_INT_REGISTER + ARM32_R9:			info->i = cpustate->sArmRegister[ 9];	break;
		case CPUINFO_INT_REGISTER + ARM32_R10:			info->i = cpustate->sArmRegister[10];	break;
		case CPUINFO_INT_REGISTER + ARM32_R11:			info->i = cpustate->sArmRegister[11];	break;
		case CPUINFO_INT_REGISTER + ARM32_R12:			info->i = cpustate->sArmRegister[12];	break;
		case CPUINFO_INT_REGISTER + ARM32_R13:			info->i = cpustate->sArmRegister[13];	break;
		case CPUINFO_INT_REGISTER + ARM32_R14:			info->i = cpustate->sArmRegister[14];	break;
		case CPUINFO_INT_REGISTER + ARM32_R15:			info->i = cpustate->sArmRegister[15];	break;

		case CPUINFO_INT_REGISTER + ARM32_FR8:			info->i = cpustate->sArmRegister[eR8_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_FR9:			info->i = cpustate->sArmRegister[eR9_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_FR10:			info->i = cpustate->sArmRegister[eR10_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_FR11:			info->i = cpustate->sArmRegister[eR11_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_FR12:			info->i = cpustate->sArmRegister[eR12_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_FR13:			info->i = cpustate->sArmRegister[eR13_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_FR14:			info->i = cpustate->sArmRegister[eR14_FIQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_IR13:			info->i = cpustate->sArmRegister[eR13_IRQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_IR14:			info->i = cpustate->sArmRegister[eR14_IRQ];	break;
		case CPUINFO_INT_REGISTER + ARM32_SR13:			info->i = cpustate->sArmRegister[eR13_SVC];	break;
		case CPUINFO_INT_REGISTER + ARM32_SR14:			info->i = cpustate->sArmRegister[eR14_SVC];	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(arm);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(arm);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(arm);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(arm);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(arm);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(arm);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "ARM");					break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Acorn Risc Machine");	break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.3");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Bryan McPhail, bmcphail@tendril.co.uk"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c",
				(cpustate->sArmRegister[15] & N_MASK) ? 'N' : '-',
				(cpustate->sArmRegister[15] & Z_MASK) ? 'Z' : '-',
				(cpustate->sArmRegister[15] & C_MASK) ? 'C' : '-',
				(cpustate->sArmRegister[15] & V_MASK) ? 'V' : '-',
				(cpustate->sArmRegister[15] & I_MASK) ? 'I' : '-',
				(cpustate->sArmRegister[15] & F_MASK) ? 'F' : '-');
			switch (cpustate->sArmRegister[15] & 3)
			{
			case 0:
				strcat(info->s, " USER");
				break;
			case 1:
				strcat(info->s, " FIRQ");
				break;
			case 2:
				strcat(info->s, " IRQ ");
				break;
			default:
				strcat(info->s, " SVC ");
				break;
			}
			break;

		case CPUINFO_STR_REGISTER + ARM32_PC:	sprintf( info->s, "PC  :%08x", cpustate->sArmRegister[15]&ADDRESS_MASK ); break;
		case CPUINFO_STR_REGISTER + ARM32_R0:	sprintf( info->s, "R0  :%08x", cpustate->sArmRegister[ 0] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R1:	sprintf( info->s, "R1  :%08x", cpustate->sArmRegister[ 1] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R2:	sprintf( info->s, "R2  :%08x", cpustate->sArmRegister[ 2] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R3:	sprintf( info->s, "R3  :%08x", cpustate->sArmRegister[ 3] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R4:	sprintf( info->s, "R4  :%08x", cpustate->sArmRegister[ 4] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R5:	sprintf( info->s, "R5  :%08x", cpustate->sArmRegister[ 5] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R6:	sprintf( info->s, "R6  :%08x", cpustate->sArmRegister[ 6] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R7:	sprintf( info->s, "R7  :%08x", cpustate->sArmRegister[ 7] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R8:	sprintf( info->s, "R8  :%08x", cpustate->sArmRegister[ 8] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R9:	sprintf( info->s, "R9  :%08x", cpustate->sArmRegister[ 9] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R10:	sprintf( info->s, "R10 :%08x", cpustate->sArmRegister[10] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R11:	sprintf( info->s, "R11 :%08x", cpustate->sArmRegister[11] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R12:	sprintf( info->s, "R12 :%08x", cpustate->sArmRegister[12] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R13:	sprintf( info->s, "R13 :%08x", cpustate->sArmRegister[13] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R14:	sprintf( info->s, "R14 :%08x", cpustate->sArmRegister[14] ); break;
		case CPUINFO_STR_REGISTER + ARM32_R15:	sprintf( info->s, "R15 :%08x", cpustate->sArmRegister[15] ); break;
		case CPUINFO_STR_REGISTER + ARM32_FR8:	sprintf( info->s, "FR8 :%08x", cpustate->sArmRegister[eR8_FIQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_FR9:	sprintf( info->s, "FR9 :%08x", cpustate->sArmRegister[eR9_FIQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_FR10:	sprintf( info->s, "FR10:%08x", cpustate->sArmRegister[eR10_FIQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_FR11:	sprintf( info->s, "FR11:%08x", cpustate->sArmRegister[eR11_FIQ]); break;
		case CPUINFO_STR_REGISTER + ARM32_FR12:	sprintf( info->s, "FR12:%08x", cpustate->sArmRegister[eR12_FIQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_FR13:	sprintf( info->s, "FR13:%08x", cpustate->sArmRegister[eR13_FIQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_FR14:	sprintf( info->s, "FR14:%08x", cpustate->sArmRegister[eR14_FIQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_IR13:	sprintf( info->s, "IR13:%08x", cpustate->sArmRegister[eR13_IRQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_IR14:	sprintf( info->s, "IR14:%08x", cpustate->sArmRegister[eR14_IRQ] ); break;
		case CPUINFO_STR_REGISTER + ARM32_SR13:	sprintf( info->s, "SR13:%08x", cpustate->sArmRegister[eR13_SVC] ); break;
		case CPUINFO_STR_REGISTER + ARM32_SR14:	sprintf( info->s, "SR14:%08x", cpustate->sArmRegister[eR14_SVC] ); break;
	}
}


CPU_GET_INFO( arm_be )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(arm_be);					break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(arm_be);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
        case CPUINFO_STR_NAME:							strcpy(info->s, "ARM (big endian)");				break;

		default:										CPU_GET_INFO_CALL(arm);								break;
	}
}


DEFINE_LEGACY_CPU_DEVICE(ARM, arm);
DEFINE_LEGACY_CPU_DEVICE(ARM_BE, arm_be);

