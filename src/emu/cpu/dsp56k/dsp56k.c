/***************************************************************************

    dsp56k.c
    Core implementation for the portable DSP56k emulator.
    Written by Andrew Gardner

****************************************************************************

    Note:
    This CPU emulator is very much a work-in-progress.

    DONE:
    1:  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
       11,  ,  ,  ,  ,  ,  ,18,  ,   ,

    TODO:
    X 1-6 Explore CORE naming scheme.
    - 1-9 paragraph 1 : memory access timings
    - 1-9 Data ALU arithmetic operations generally use fractional two's complement arithmetic
          (Unsigned numbers are only supported by the multiply and multiply-accumulate instruction)
    - 1-9 For fractional arithmetic, the 31-bit product is added to the 40-bit contents of A or B.  No pipeline!
    - 1-10 Two types of rounding: convergent rounding and two's complement rounding.  See status register bit R.
    - 1-10 Logic unit is 16-bits wide and works on MSP portion of accum register
    - 1-10 The AGU can implement three types of arithmetic: linear, modulo, and reverse carry.
    - 1-12 "Two external interrupt pins!!!"
    - 1-12 Take care of all interrupt priority (IPR) stuff!
    - 1-19 Memory WAIT states
    - 1-20 The timer's interesting!
    - 1-21 Vectored exception requests on the Host Interface!
***************************************************************************/

#include "opcode.h"

#include "emu.h"
#include "debugger.h"
#include "dsp56k.h"

#include "dsp56def.h"

using namespace DSP56K;

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
static CPU_RESET( dsp56k );


/***************************************************************************
    COMPONENT FUNCTIONALITY
***************************************************************************/
/* 1-9 ALU */
// #include "dsp56alu.h"

/* 1-10 Address Generation Unit (AGU) */
// #include "dsp56agu.h"

/* 1-11 Program Control Unit (PCU) */
#include "dsp56pcu.h"

/* 5-1 Host Interface (HI) */
//#include "dsp56hi.h"

/* 4-8 Memory handlers for on-chip peripheral memory. */
#include "dsp56mem.h"


/***************************************************************************
    Direct Update Handler
***************************************************************************/
DIRECT_UPDATE_HANDLER( dsp56k_direct_handler )
{
	if (address <= (0x07ff<<1))
	{
		dsp56k_core* cpustate = get_safe_token(&direct.space().device());
		direct.explicit_configure(0x0000<<1, 0x07ff<<1, (0x07ff<<1) | 1, cpustate->program_ram);
		return ~0;
	}

	return address;
}


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/
#define ROPCODE(pc)   cpustate->direct->read_decrypted_word(pc)


/***************************************************************************
    IRQ HANDLING
***************************************************************************/
static void set_irq_line(dsp56k_core* cpustate, int irqline, int state)
{
	//logerror("DSP56k set irq line %d %d\n", irqline, state);

	switch(irqline)
	{
		case DSP56K_IRQ_MODA:
			// TODO: 1-12 Get this triggering right
			if (irqa_trigger(cpustate))
				logerror("DSP56k IRQA is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				cpustate->modA_state = TRUE;
			else
				cpustate->modA_state = FALSE;

			if (cpustate->reset_state != TRUE)
				dsp56k_add_pending_interrupt(cpustate, "IRQA");
			break;

		case DSP56K_IRQ_MODB:
			// TODO: 1-12 Get this triggering right
			if (irqb_trigger(cpustate))
				logerror("DSP56k IRQB is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				cpustate->modB_state = TRUE;
			else
				cpustate->modB_state = FALSE;

			if (cpustate->reset_state != TRUE)
				dsp56k_add_pending_interrupt(cpustate, "IRQB");
			break;

		case DSP56K_IRQ_MODC:
			if (state != CLEAR_LINE)
				cpustate->modC_state = TRUE;
			else
				cpustate->modC_state = FALSE;

			// TODO : Set bus mode or whatever
			break;

		case DSP56K_IRQ_RESET:
			if (state != CLEAR_LINE)
				cpustate->reset_state = TRUE;
			else
			{
				/* If it changes state from asserted to cleared.  Call the reset function. */
				if (cpustate->reset_state == TRUE)
					CPU_RESET_NAME(dsp56k)(cpustate->device);

				cpustate->reset_state = FALSE;
			}

			// dsp56k_add_pending_interrupt("Hardware RESET");
			break;

		default:
			logerror("DSP56k setting some weird irq line : %d", irqline);
			break;
	}

	/* If the reset line isn't asserted, service interrupts */
	// TODO: Is it right to immediately service interrupts?
	//if (cpustate->reset_state != TRUE)
	//  pcu_service_interrupts();
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/
static void agu_init(dsp56k_core* cpustate, device_t *device)
{
	/* save states - dsp56k_agu members */
	device->save_item(NAME(cpustate->AGU.r0));
	device->save_item(NAME(cpustate->AGU.r1));
	device->save_item(NAME(cpustate->AGU.r2));
	device->save_item(NAME(cpustate->AGU.r3));
	device->save_item(NAME(cpustate->AGU.n0));
	device->save_item(NAME(cpustate->AGU.n1));
	device->save_item(NAME(cpustate->AGU.n2));
	device->save_item(NAME(cpustate->AGU.n3));
	device->save_item(NAME(cpustate->AGU.m0));
	device->save_item(NAME(cpustate->AGU.m1));
	device->save_item(NAME(cpustate->AGU.m2));
	device->save_item(NAME(cpustate->AGU.m3));
	device->save_item(NAME(cpustate->AGU.temp));
}

static void alu_init(dsp56k_core* cpustate, device_t *device)
{
	/* save states - dsp56k_alu members */
	device->save_item(NAME(cpustate->ALU.x));
	device->save_item(NAME(cpustate->ALU.y));
	device->save_item(NAME(cpustate->ALU.a));
	device->save_item(NAME(cpustate->ALU.b));
}

static CPU_INIT( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);

	/* Call specific module inits */
	pcu_init(cpustate, device);
	agu_init(cpustate, device);
	alu_init(cpustate, device);

	/* HACK - You're not in bootstrap mode upon bootup */
	cpustate->bootstrap_mode = BOOTSTRAP_OFF;

	/* Clear the irq states */
	cpustate->modA_state = FALSE;
	cpustate->modB_state = FALSE;
	cpustate->modC_state = FALSE;
	cpustate->reset_state = FALSE;

	/* save states - dsp56k_core members */
	device->save_item(NAME(cpustate->modA_state));
	device->save_item(NAME(cpustate->modB_state));
	device->save_item(NAME(cpustate->modC_state));
	device->save_item(NAME(cpustate->reset_state));
	device->save_item(NAME(cpustate->bootstrap_mode));
	device->save_item(NAME(cpustate->repFlag));
	device->save_item(NAME(cpustate->repAddr));
	device->save_item(NAME(cpustate->icount));
	device->save_item(NAME(cpustate->ppc));
	device->save_item(NAME(cpustate->op));
	device->save_item(NAME(cpustate->interrupt_cycles));

	/* save states - dsp56k_host_interface members */
	device->save_item(NAME(cpustate->HI.icr));
	device->save_item(NAME(cpustate->HI.cvr));
	device->save_item(NAME(cpustate->HI.isr));
	device->save_item(NAME(cpustate->HI.ivr));
	device->save_item(NAME(cpustate->HI.trxh));
	device->save_item(NAME(cpustate->HI.trxl));
	device->save_item(NAME(cpustate->HI.bootstrap_offset));

	//cpustate->config = device->static_config();
	//cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = &device->space(AS_DATA);

	/* Setup the direct memory handler for this CPU */
	/* NOTE: Be sure to grab this guy and call him if you ever install another direct_update_hander in a driver! */
	cpustate->program->set_direct_update_handler(direct_update_delegate(FUNC(dsp56k_direct_handler), &device->machine()));
}


/***************************************************************************
    RESET BEHAVIOR
***************************************************************************/
static void agu_reset(dsp56k_core* cpustate)
{
	/* FM.4-3 */
	R0 = 0x0000;
	R1 = 0x0000;
	R2 = 0x0000;
	R3 = 0x0000;

	N0 = 0x0000;
	N1 = 0x0000;
	N2 = 0x0000;
	N3 = 0x0000;

	M0 = 0xffff;
	M1 = 0xffff;
	M2 = 0xffff;
	M3 = 0xffff;

	TEMP = 0x0000;
}

static void alu_reset(dsp56k_core* cpustate)
{
	X = 0x00000000;
	Y = 0x00000000;
	A = 0x0000000000;
	B = 0x0000000000;
}

static CPU_RESET( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);
	logerror("Dsp56k reset\n");

	cpustate->interrupt_cycles = 0;
	cpustate->ppc = 0x0000;

	cpustate->repFlag = 0;
	cpustate->repAddr = 0x0000;

	pcu_reset(cpustate);
	mem_reset(cpustate);
	agu_reset(cpustate);
	alu_reset(cpustate);

	/* HACK - Put a jump to 0x0000 at 0x0000 - this keeps the CPU locked to the instruction at address 0x0000 */
	cpustate->program->write_word(0x0000, 0x0124);
}


static CPU_EXIT( dsp56k )
{
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/
#include "dsp56ops.c"


/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/
// Execute a single opcode and return how many cycles it took.
static size_t execute_one_new(dsp56k_core* cpustate)
{
	// For MAME
	cpustate->op = ROPCODE(ADDRESS(PC));
	debugger_instruction_hook(cpustate->device, PC);

	UINT16 w0 = ROPCODE(ADDRESS(PC));
	UINT16 w1 = ROPCODE(ADDRESS(PC) + ADDRESS(1));

	Opcode op(w0, w1);
	op.evaluate(cpustate);
	PC += op.evalSize();	// Special size function needed to handle jmps, etc.

	// TODO: Currently all operations take up 4 cycles (inst->cycles()).
	return 4;
}

static CPU_EXECUTE( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);

	/* If reset line is asserted, do nothing */
	if (cpustate->reset_state)
	{
		cpustate->icount = 0;
		return;
	}

	/* HACK - if you're in bootstrap mode, simply pretend you ate up all your cycles waiting for data. */
	if (cpustate->bootstrap_mode != BOOTSTRAP_OFF)
	{
		cpustate->icount = 0;
		return;
	}

	//cpustate->icount -= cpustate->interrupt_cycles;
	//cpustate->interrupt_cycles = 0;

	while(cpustate->icount > 0)
	{
		execute_one(cpustate);
		if (0) cpustate->icount -= execute_one_new(cpustate);
		pcu_service_interrupts(cpustate);	// TODO: Is it incorrect to service after each instruction?
	}
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/
extern CPU_DISASSEMBLE( dsp56k );


/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/
static ADDRESS_MAP_START( dsp56156_program_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x0000,0x07ff) AM_READWRITE_LEGACY(DSP56K::program_r, DSP56K::program_w)	/* 1-5 */
//  AM_RANGE(0x2f00,0x2fff) AM_ROM                              /* 1-5 PROM reserved memory.  Is this the right spot for it? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp56156_x_data_map, AS_DATA, 16, legacy_cpu_device )
	AM_RANGE(0x0000,0x07ff) AM_RAM								/* 1-5 */
	AM_RANGE(0xffc0,0xffff) AM_READWRITE_LEGACY(DSP56K::peripheral_register_r, DSP56K::peripheral_register_w)	/* 1-5 On-chip peripheral registers memory mapped in data space */
ADDRESS_MAP_END


/**************************************************************************
 * Generic set_info/get_info
 **************************************************************************/
enum
{
	// PCU
	DSP56K_PC=1,
	DSP56K_SR,
	DSP56K_LC,
	DSP56K_LA,
	DSP56K_SP,
	DSP56K_OMR,

	// ALU
	DSP56K_X, DSP56K_Y,
	DSP56K_A, DSP56K_B,

	// AGU
	DSP56K_R0,DSP56K_R1,DSP56K_R2,DSP56K_R3,
	DSP56K_N0,DSP56K_N1,DSP56K_N2,DSP56K_N3,
	DSP56K_M0,DSP56K_M1,DSP56K_M2,DSP56K_M3,
	DSP56K_TEMP,
	DSP56K_STATUS,

	// CPU STACK
	DSP56K_ST0,
	DSP56K_ST1,
	DSP56K_ST2,
	DSP56K_ST3,
	DSP56K_ST4,
	DSP56K_ST5,
	DSP56K_ST6,
	DSP56K_ST7,
	DSP56K_ST8,
	DSP56K_ST9,
	DSP56K_ST10,
	DSP56K_ST11,
	DSP56K_ST12,
	DSP56K_ST13,
	DSP56K_ST14,
	DSP56K_ST15
};

static CPU_SET_INFO( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODA:   set_irq_line(cpustate, DSP56K_IRQ_MODA, info->i);  break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODB:   set_irq_line(cpustate, DSP56K_IRQ_MODB, info->i);  break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODC:   set_irq_line(cpustate, DSP56K_IRQ_MODC, info->i);  break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_RESET:  set_irq_line(cpustate, DSP56K_IRQ_RESET, info->i); break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP56K_PC:			PC  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_SR:			SR  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_LC:			LC  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_LA:			LA  = info->i & 0xffff;					break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP56K_SP:			SP  = info->i & 0xff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_OMR:			OMR = info->i & 0xff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_X:			X   = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_Y:			Y   = info->i & 0xffffffff;				break;

		case CPUINFO_INT_REGISTER + DSP56K_A:			A   = info->i & (UINT64)U64(0xffffffffffffffff); break;	/* could benefit from a better mask? */
		case CPUINFO_INT_REGISTER + DSP56K_B:			B   = info->i & (UINT64)U64(0xffffffffffffffff); break;	/* could benefit from a better mask? */

		case CPUINFO_INT_REGISTER + DSP56K_R0:			R0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R1:			R1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R2:			R2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R3:			R3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_N0:			N0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N1:			N1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N2:			N2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N3:			N3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_M0:			M0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M1:			M1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M2:			M2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M3:			M3  = info->i & 0xffff;					break;

		/*  case CPUINFO_INT_REGISTER + DSP56K_TEMP:        TEMP   = info->i & 0xffff;          break;  */
		/*  case CPUINFO_INT_REGISTER + DSP56K_STATUS:      STATUS = info->i & 0xff;            break;  */

		/* The CPU stack */
		case CPUINFO_INT_REGISTER + DSP56K_ST0:			ST0 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST1:			ST1 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST2:			ST2 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST3:			ST3 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST4:			ST4 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST5:			ST5 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST6:			ST6 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST7:			ST7 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST8:			ST8 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST9:			ST9 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST10:		ST10 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST11:		ST11 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST12:		ST12 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST13:		ST13 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST14:		ST14 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST15:		ST15 = info->i & 0xffffffff;			break;
	}
}


CPU_GET_INFO( dsp56k )
{
	dsp56k_core* cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dsp56k_core);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0x0000;						break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;	// ?                    break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 8;	// ?                    break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODA:	info->i = DSP56K_IRQ_MODA;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODB:	info->i = DSP56K_IRQ_MODB;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODC:	info->i = DSP56K_IRQ_MODC;				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP56K_PC:			info->i = PC;							break;
		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc;				break;

		case CPUINFO_INT_REGISTER + DSP56K_SR:			info->i = SR;							break;
		case CPUINFO_INT_REGISTER + DSP56K_LC:			info->i = LC;							break;
		case CPUINFO_INT_REGISTER + DSP56K_LA:			info->i = LA;							break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP56K_SP:			info->i = SP;							break;

		case CPUINFO_INT_REGISTER + DSP56K_OMR:			info->i = OMR;							break;

		case CPUINFO_INT_REGISTER + DSP56K_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + DSP56K_Y:			info->i = Y;							break;

		case CPUINFO_INT_REGISTER + DSP56K_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + DSP56K_B:			info->i = B;							break;

		case CPUINFO_INT_REGISTER + DSP56K_R0:			info->i = R0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R1:			info->i = R1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R2:			info->i = R2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R3:			info->i = R3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_N0:			info->i = N0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N1:			info->i = N1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N2:			info->i = N2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N3:			info->i = N3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_M0:			info->i = M0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M1:			info->i = M1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M2:			info->i = M2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M3:			info->i = M3;							break;

		/* case CPUINFO_INT_REGISTER + DSP56K_TEMP:    info->i = TEMP;                         break;  */
		/* case CPUINFO_INT_REGISTER + DSP56K_STATUS:  info->i = STATUS;                       break;  */

		/* The CPU stack */
		case CPUINFO_INT_REGISTER + DSP56K_ST0:			info->i = ST0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST1:			info->i = ST1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST2:			info->i = ST2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST3:			info->i = ST3;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST4:			info->i = ST4;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST5:			info->i = ST5;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST6:			info->i = ST6;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST7:			info->i = ST7;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST8:			info->i = ST8;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST9:			info->i = ST9;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST10:		info->i = ST10;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST11:		info->i = ST11;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST12:		info->i = ST12;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST13:		info->i = ST13;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST14:		info->i = ST14;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST15:		info->i = ST15;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(dsp56k); break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(dsp56k);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(dsp56k);	break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(dsp56k);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(dsp56k); break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(dsp56k); break;
		case CPUINFO_FCT_DEBUG_INIT:					info->debug_init = NULL;				break;
		case CPUINFO_FCT_TRANSLATE:						info->translate = NULL;					break;
		case CPUINFO_FCT_READ:							info->read = NULL;						break;
		case CPUINFO_FCT_WRITE:							info->write = NULL;						break;
		case CPUINFO_FCT_READOP:						info->readop = NULL;					break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:
			info->internal_map16 = ADDRESS_MAP_NAME(dsp56156_x_data_map);						break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:
			info->internal_map16 = ADDRESS_MAP_NAME(dsp56156_program_map);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "DSP56156");			break;
		case CPUINFO_STR_FAMILY:						strcpy(info->s, "Motorola DSP56156");	break;
		case CPUINFO_STR_VERSION:						strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:						strcpy(info->s, "Andrew Gardner");		break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s%s %s%s%s%s%s%s%s%s %s%s",
				/* Status Register */
				LF_bit(cpustate) ? "L" : ".",
				FV_bit(cpustate) ? "F" : ".",

				S_bit(cpustate) ? "S" : ".",
				L_bit(cpustate) ? "L" : ".",
				E_bit(cpustate) ? "E" : ".",
				U_bit(cpustate) ? "U" : ".",
				N_bit(cpustate) ? "N" : ".",
				Z_bit(cpustate) ? "Z" : ".",
				V_bit(cpustate) ? "V" : ".",
				C_bit(cpustate) ? "C" : ".",

				/* Stack Pointer */
				UF_bit(cpustate) ? "U" : ".",
				SE_bit(cpustate) ? "S" : ".");
			break;

		case CPUINFO_STR_REGISTER + DSP56K_PC:			sprintf(info->s, "PC : %04x", PC);		break;
		case CPUINFO_STR_REGISTER + DSP56K_SR:			sprintf(info->s, "SR : %04x", SR);		break;
		case CPUINFO_STR_REGISTER + DSP56K_LC:			sprintf(info->s, "LC : %04x", LC);		break;
		case CPUINFO_STR_REGISTER + DSP56K_LA:			sprintf(info->s, "LA : %04x", LA);		break;
		case CPUINFO_STR_REGISTER + DSP56K_SP:			sprintf(info->s, "SP : %02x", SP);		break;
		case CPUINFO_STR_REGISTER + DSP56K_OMR:			sprintf(info->s, "OMR: %02x", OMR);		break;

		case CPUINFO_STR_REGISTER + DSP56K_X:			sprintf(info->s, "X  : %04x %04x", X1, X0); break;
		case CPUINFO_STR_REGISTER + DSP56K_Y:			sprintf(info->s, "Y  : %04x %04x", Y1, Y0); break;

		case CPUINFO_STR_REGISTER + DSP56K_A:			sprintf(info->s, "A  : %02x %04x %04x", A2,A1,A0); break;
		case CPUINFO_STR_REGISTER + DSP56K_B:			sprintf(info->s, "B  : %02x %04x %04x", B2,B1,B0); break;

		case CPUINFO_STR_REGISTER + DSP56K_R0:			sprintf(info->s, "R0 : %04x", R0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R1:			sprintf(info->s, "R1 : %04x", R1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R2:			sprintf(info->s, "R2 : %04x", R2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R3:			sprintf(info->s, "R3 : %04x", R3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_N0:			sprintf(info->s, "N0 : %04x", N0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N1:			sprintf(info->s, "N1 : %04x", N1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N2:			sprintf(info->s, "N2 : %04x", N2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N3:			sprintf(info->s, "N3 : %04x", N3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_M0:			sprintf(info->s, "M0 : %04x", M0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M1:			sprintf(info->s, "M1 : %04x", M1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M2:			sprintf(info->s, "M2 : %04x", M2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M3:			sprintf(info->s, "M3 : %04x", M3);		break;

		/*  case CPUINFO_STR_REGISTER + DSP56K_TEMP:    sprintf(info->s, "TMP: %04x", TEMP);    break;  */
		/*  case CPUINFO_STR_REGISTER + DSP56K_STATUS:  sprintf(info->s, "STS: %02x", STATUS);  break;  */

		/* The CPU stack */
		case CPUINFO_STR_REGISTER + DSP56K_ST0:			sprintf(info->s, "ST0 : %08x", ST0);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST1:			sprintf(info->s, "ST1 : %08x", ST1);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST2:			sprintf(info->s, "ST2 : %08x", ST2);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST3:			sprintf(info->s, "ST3 : %08x", ST3);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST4:			sprintf(info->s, "ST4 : %08x", ST4);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST5:			sprintf(info->s, "ST5 : %08x", ST5);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST6:			sprintf(info->s, "ST6 : %08x", ST6);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST7:			sprintf(info->s, "ST7 : %08x", ST7);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST8:			sprintf(info->s, "ST8 : %08x", ST8);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST9:			sprintf(info->s, "ST9 : %08x", ST9);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST10:		sprintf(info->s, "ST10: %08x", ST10);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST11:		sprintf(info->s, "ST11: %08x", ST11);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST12:		sprintf(info->s, "ST12: %08x", ST12);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST13:		sprintf(info->s, "ST13: %08x", ST13);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST14:		sprintf(info->s, "ST14: %08x", ST14);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST15:		sprintf(info->s, "ST15: %08x", ST15);	break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(DSP56156, dsp56k);
