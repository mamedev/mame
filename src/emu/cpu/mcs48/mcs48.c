/*
EA pin - defined by architecture, must implement:
   1 means external access, bypassing internal ROM
   reimplement as a push, not a pull
T0 output clock
*/

/***************************************************************************

    mcs48.c

    Intel MCS-48/UPI-41 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator
    You are not allowed to distribute this software commercially

****************************************************************************

    Note that the default internal divisor for this chip is by 3 and
    then again by 5, or by 15 total.

****************************************************************************

    Chip   RAM  ROM  I/O
    ----   ---  ---  ---
    8021    64   1k   21  (ROM, reduced instruction set)

    8035    64    0   27  (external ROM)
    8048    64   1k   27  (ROM)
    8648    64   1k   27  (OTPROM)
    8748    64   1k   27  (EPROM)
    8884    64   1k
    N7751  128   2k

    8039   128    0   27  (external ROM)
    8049   128   2k   27  (ROM)
    8749   128   2k   27  (EPROM)
    M58715 128    0       (external ROM)

****************************************************************************

    UPI-41/42 chips are MCS-48 derived, with some opcode changes:

            MCS-48 opcode       UPI-41/42 opcode
            -------------       ----------------
        02: OUTL BUS,A          OUT  DBB,A
        08: INS  BUS,A          <illegal>
        22: <illegal>           IN   DBB,A
        75: ENT0 CLK            <illegal>
        80: MOVX A,@R0          <illegal>
        81: MOVX A,@R1          <illegal>
        86: JNI  <dest>         JOBF <dest>
        88: ORL  BUS,#n         <illegal>
        90: MOVX @R0,A          MOV  STS,A
        91: MOVX @R1,A          <illegal>
        98: ANL  BUS,#n         <illegal>
        D6: <illegal>           JNIBF <dest>
        E5: SEL  MB0            EN   DMA
        F5: SEL  MB1            EN   FLAGS

    Chip numbers are similar to the MCS-48 series:

    Chip   RAM  ROM  I/O
    ----   ---  ---  ---
    8041   128   1k
    8741   128   1k       (EPROM)

    8042   256   2k
    8242   256   2k
    8242   256   2k

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mcs48.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* timer/counter enable bits */
#define TIMER_ENABLED	0x01
#define COUNTER_ENABLED	0x02

/* flag bits */
#define C_FLAG			0x80
#define A_FLAG			0x40
#define F_FLAG			0x20
#define B_FLAG			0x10

/* status bits (UPI-41) */
#define STS_F1			0x08
#define STS_F0			0x04
#define STS_IBF			0x02
#define STS_OBF			0x01

/* port 2 bits (UPI-41) */
#define P2_OBF			0x10
#define P2_NIBF			0x20
#define P2_DRQ			0x40
#define P2_NDACK		0x80

/* enable bits (UPI-41) */
#define ENABLE_FLAGS	0x01
#define ENABLE_DMA		0x02

/* feature masks */
#define MCS48_FEATURE	0x01
#define UPI41_FEATURE	0x02



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* live processor state */
struct mcs48_state
{
	UINT16		prevpc;				/* 16-bit previous program counter */
	UINT16		pc;					/* 16-bit program counter */

	UINT8		a;					/* 8-bit accumulator */
	UINT8 *		regptr;				/* pointer to r0-r7 */
	UINT8		psw;				/* 8-bit cpustate->psw */
	UINT8		p1;					/* 8-bit latched port 1 */
	UINT8		p2;					/* 8-bit latched port 2 */
	UINT8		ea;					/* 1-bit latched ea input */
	UINT8		timer;				/* 8-bit timer */
	UINT8		prescaler;			/* 5-bit timer prescaler */
	UINT8		t1_history;			/* 8-bit history of the T1 input */
	UINT8		sts;				/* 8-bit status register (UPI-41 only, except for F1) */
	UINT8		dbbi;				/* 8-bit input data buffer (UPI-41 only) */
	UINT8		dbbo;				/* 8-bit output data buffer (UPI-41 only) */

	UINT8		irq_state;			/* TRUE if an IRQ is pending */
	UINT8		irq_in_progress;	/* TRUE if an IRQ is in progress */
	UINT8		timer_overflow;		/* TRUE on a timer overflow; cleared by taking interrupt */
	UINT8		timer_flag;			/* TRUE on a timer overflow; cleared on JTF */
	UINT8		tirq_enabled;		/* TRUE if the timer IRQ is enabled */
	UINT8		xirq_enabled;		/* TRUE if the external IRQ is enabled */
	UINT8		timecount_enabled;	/* bitmask of timer/counter enabled */
	UINT8		flags_enabled;		/* TRUE if I/O flags have been enabled (UPI-41 only) */
	UINT8		dma_enabled;		/* TRUE if DMA has been enabled (UPI-41 only) */

	UINT16		a11;				/* A11 value, either 0x000 or 0x800 */

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	int			icount;

	/* Memory spaces */
    address_space *program;
    direct_read_data *direct;
    address_space *data;
    address_space *io;

	UINT8		feature_mask;		/* processor feature flags */
	UINT16		int_rom_size;		/* internal rom size */

	UINT8		rtemp;				/* temporary for import/export */
};


/* opcode table entry */
typedef int (*mcs48_ophandler)(mcs48_state *state);



/***************************************************************************
    MACROS
***************************************************************************/

/* ROM is mapped to AS_PROGRAM */
#define program_r(a)	cpustate->program->read_byte(a)

/* RAM is mapped to AS_DATA */
#define ram_r(a)		cpustate->data->read_byte(a)
#define ram_w(a,V)		cpustate->data->write_byte(a, V)

/* ports are mapped to AS_IO */
#define ext_r(a)		cpustate->io->read_byte(a)
#define ext_w(a,V)		cpustate->io->write_byte(a, V)
#define port_r(a)		cpustate->io->read_byte(MCS48_PORT_P0 + a)
#define port_w(a,V)		cpustate->io->write_byte(MCS48_PORT_P0 + a, V)
#define test_r(a)		cpustate->io->read_byte(MCS48_PORT_T0 + a)
#define test_w(a,V)		cpustate->io->write_byte(MCS48_PORT_T0 + a, V)
#define bus_r()			cpustate->io->read_byte(MCS48_PORT_BUS)
#define bus_w(V)		cpustate->io->write_byte(MCS48_PORT_BUS, V)
#define ea_r()			cpustate->io->read_byte(MCS48_PORT_EA)
#define prog_w(V)		cpustate->io->write_byte(MCS48_PORT_PROG, V)

/* r0-r7 map to memory via the regptr */
#define R0				regptr[0]
#define R1				regptr[1]
#define R2				regptr[2]
#define R3				regptr[3]
#define R4				regptr[4]
#define R5				regptr[5]
#define R6				regptr[6]
#define R7				regptr[7]



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int check_irqs(mcs48_state *cpustate);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE mcs48_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8021 ||
		   device->type() == I8022 ||
		   device->type() == I8035 ||
		   device->type() == I8048 ||
		   device->type() == I8648 ||
		   device->type() == I8748 ||
		   device->type() == I8039 ||
		   device->type() == I8049 ||
		   device->type() == I8749 ||
		   device->type() == I8040 ||
		   device->type() == I8050 ||
		   device->type() == I8041 ||
		   device->type() == I8741 ||
		   device->type() == I8042 ||
		   device->type() == I8242 ||
		   device->type() == I8742 ||
		   device->type() == MB8884 ||
		   device->type() == N7751 ||
		   device->type() == M58715);
	return (mcs48_state *)downcast<legacy_cpu_device *>(device)->token();
}


/*-------------------------------------------------
    opcode_fetch - fetch an opcode byte
-------------------------------------------------*/

INLINE UINT8 opcode_fetch(mcs48_state *cpustate)
{
	return cpustate->direct->read_decrypted_byte(cpustate->pc++);
}


/*-------------------------------------------------
    argument_fetch - fetch an opcode argument
    byte
-------------------------------------------------*/

INLINE UINT8 argument_fetch(mcs48_state *cpustate)
{
	return cpustate->direct->read_raw_byte(cpustate->pc++);
}


/*-------------------------------------------------
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

INLINE void update_regptr(mcs48_state *cpustate)
{
	cpustate->regptr = (UINT8 *)cpustate->data->get_write_ptr((cpustate->psw & B_FLAG) ? 24 : 0);
}


/*-------------------------------------------------
    push_pc_psw - push the cpustate->pc and cpustate->psw values onto
    the stack
-------------------------------------------------*/

INLINE void push_pc_psw(mcs48_state *cpustate)
{
	UINT8 sp = cpustate->psw & 0x07;
	ram_w(8 + 2*sp, cpustate->pc);
	ram_w(9 + 2*sp, ((cpustate->pc >> 8) & 0x0f) | (cpustate->psw & 0xf0));
	cpustate->psw = (cpustate->psw & 0xf8) | ((sp + 1) & 0x07);
}


/*-------------------------------------------------
    pull_pc_psw - pull the PC and PSW values from
    the stack
-------------------------------------------------*/

INLINE void pull_pc_psw(mcs48_state *cpustate)
{
	UINT8 sp = (cpustate->psw - 1) & 0x07;
	cpustate->pc = ram_r(8 + 2*sp);
	cpustate->pc |= ram_r(9 + 2*sp) << 8;
	cpustate->psw = ((cpustate->pc >> 8) & 0xf0) | 0x08 | sp;
	cpustate->pc &= 0xfff;
	update_regptr(cpustate);
}


/*-------------------------------------------------
    pull_pc - pull the PC value from the stack,
    leaving the upper part of PSW intact
-------------------------------------------------*/

INLINE void pull_pc(mcs48_state *cpustate)
{
	UINT8 sp = (cpustate->psw - 1) & 0x07;
	cpustate->pc = ram_r(8 + 2*sp);
	cpustate->pc |= ram_r(9 + 2*sp) << 8;
	cpustate->pc &= 0xfff;
	cpustate->psw = (cpustate->psw & 0xf0) | 0x08 | sp;
}


/*-------------------------------------------------
    execute_add - perform the logic of an ADD
    instruction
-------------------------------------------------*/

INLINE void execute_add(mcs48_state *cpustate, UINT8 dat)
{
	UINT16 temp = cpustate->a + dat;
	UINT16 temp4 = (cpustate->a & 0x0f) + (dat & 0x0f);

	cpustate->psw &= ~(C_FLAG | A_FLAG);
	cpustate->psw |= (temp4 << 2) & A_FLAG;
	cpustate->psw |= (temp >> 1) & C_FLAG;
	cpustate->a = temp;
}


/*-------------------------------------------------
    execute_addc - perform the logic of an ADDC
    instruction
-------------------------------------------------*/

INLINE void execute_addc(mcs48_state *cpustate, UINT8 dat)
{
	UINT8 carryin = (cpustate->psw & C_FLAG) >> 7;
	UINT16 temp = cpustate->a + dat + carryin;
	UINT16 temp4 = (cpustate->a & 0x0f) + (dat & 0x0f) + carryin;

	cpustate->psw &= ~(C_FLAG | A_FLAG);
	cpustate->psw |= (temp4 << 2) & A_FLAG;
	cpustate->psw |= (temp >> 1) & C_FLAG;
	cpustate->a = temp;
}


/*-------------------------------------------------
    execute_jmp - perform the logic of a JMP
    instruction
-------------------------------------------------*/

INLINE void execute_jmp(mcs48_state *cpustate, UINT16 address)
{
	UINT16 a11 = (cpustate->irq_in_progress) ? 0 : cpustate->a11;
	cpustate->pc = address | a11;
}


/*-------------------------------------------------
    execute_call - perform the logic of a CALL
    instruction
-------------------------------------------------*/

INLINE void execute_call(mcs48_state *cpustate, UINT16 address)
{
	push_pc_psw(cpustate);
	execute_jmp(cpustate, address);
}


/*-------------------------------------------------
    execute_jcc - perform the logic of a
    conditional jump instruction
-------------------------------------------------*/

INLINE void execute_jcc(mcs48_state *cpustate, UINT8 result)
{
	UINT8 offset = argument_fetch(cpustate);
	if (result != 0)
		cpustate->pc = ((cpustate->pc - 1) & 0xf00) | offset;
}


/*-------------------------------------------------
    p2_mask - return the mask of bits that the
    code can directly affect
-------------------------------------------------*/

INLINE UINT8 p2_mask(mcs48_state *cpustate)
{
	UINT8 result = 0xff;
	if ((cpustate->feature_mask & UPI41_FEATURE) == 0)
		return result;
	if (cpustate->flags_enabled)
		result &= ~(P2_OBF | P2_NIBF);
	if (cpustate->dma_enabled)
		result &= ~(P2_DRQ | P2_NDACK);
	return result;
}


/*-------------------------------------------------
    expander_operation - perform an operation via
    the 8243 expander chip
-------------------------------------------------*/

INLINE void expander_operation(mcs48_state *cpustate, UINT8 operation, UINT8 port)
{
	/* put opcode/data on low 4 bits of P2 */
	port_w(2, cpustate->p2 = (cpustate->p2 & 0xf0) | (operation << 2) | (port & 3));

	/* generate high-to-low transition on PROG line */
	prog_w(0);

	/* put data on low 4 bits of P2 */
	if (operation != 0)
		port_w(2, cpustate->p2 = (cpustate->p2 & 0xf0) | (cpustate->a & 0x0f));
	else
		cpustate->a = port_r(2) | 0x0f;

	/* generate low-to-high transition on PROG line */
	prog_w(1);
}



/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define OPHANDLER(_name) static int _name(mcs48_state *cpustate)

#define SPLIT_OPHANDLER(_name, _mcs48name, _upi41name) \
OPHANDLER(_name) { return (!(cpustate->feature_mask & UPI41_FEATURE)) ? _mcs48name(cpustate) : _upi41name(cpustate); }


OPHANDLER( illegal )
{
	logerror("MCS-48 PC:%04X - Illegal opcode = %02x\n", cpustate->pc - 1, program_r(cpustate->pc - 1));
	return 1;
}

OPHANDLER( add_a_r0 )		{ execute_add(cpustate, cpustate->R0); return 1; }
OPHANDLER( add_a_r1 )		{ execute_add(cpustate, cpustate->R1); return 1; }
OPHANDLER( add_a_r2 )		{ execute_add(cpustate, cpustate->R2); return 1; }
OPHANDLER( add_a_r3 )		{ execute_add(cpustate, cpustate->R3); return 1; }
OPHANDLER( add_a_r4 )		{ execute_add(cpustate, cpustate->R4); return 1; }
OPHANDLER( add_a_r5 )		{ execute_add(cpustate, cpustate->R5); return 1; }
OPHANDLER( add_a_r6 )		{ execute_add(cpustate, cpustate->R6); return 1; }
OPHANDLER( add_a_r7 )		{ execute_add(cpustate, cpustate->R7); return 1; }
OPHANDLER( add_a_xr0 )		{ execute_add(cpustate, ram_r(cpustate->R0)); return 1; }
OPHANDLER( add_a_xr1 )		{ execute_add(cpustate, ram_r(cpustate->R1)); return 1; }
OPHANDLER( add_a_n )		{ execute_add(cpustate, argument_fetch(cpustate)); return 2; }

OPHANDLER( adc_a_r0 )		{ execute_addc(cpustate, cpustate->R0); return 1; }
OPHANDLER( adc_a_r1 )		{ execute_addc(cpustate, cpustate->R1); return 1; }
OPHANDLER( adc_a_r2 )		{ execute_addc(cpustate, cpustate->R2); return 1; }
OPHANDLER( adc_a_r3 )		{ execute_addc(cpustate, cpustate->R3); return 1; }
OPHANDLER( adc_a_r4 )		{ execute_addc(cpustate, cpustate->R4); return 1; }
OPHANDLER( adc_a_r5 )		{ execute_addc(cpustate, cpustate->R5); return 1; }
OPHANDLER( adc_a_r6 )		{ execute_addc(cpustate, cpustate->R6); return 1; }
OPHANDLER( adc_a_r7 )		{ execute_addc(cpustate, cpustate->R7); return 1; }
OPHANDLER( adc_a_xr0 )		{ execute_addc(cpustate, ram_r(cpustate->R0)); return 1; }
OPHANDLER( adc_a_xr1 )		{ execute_addc(cpustate, ram_r(cpustate->R1)); return 1; }
OPHANDLER( adc_a_n )		{ execute_addc(cpustate, argument_fetch(cpustate)); return 2; }

OPHANDLER( anl_a_r0 )		{ cpustate->a &= cpustate->R0; return 1; }
OPHANDLER( anl_a_r1 )		{ cpustate->a &= cpustate->R1; return 1; }
OPHANDLER( anl_a_r2 )		{ cpustate->a &= cpustate->R2; return 1; }
OPHANDLER( anl_a_r3 )		{ cpustate->a &= cpustate->R3; return 1; }
OPHANDLER( anl_a_r4 )		{ cpustate->a &= cpustate->R4; return 1; }
OPHANDLER( anl_a_r5 )		{ cpustate->a &= cpustate->R5; return 1; }
OPHANDLER( anl_a_r6 )		{ cpustate->a &= cpustate->R6; return 1; }
OPHANDLER( anl_a_r7 )		{ cpustate->a &= cpustate->R7; return 1; }
OPHANDLER( anl_a_xr0 )		{ cpustate->a &= ram_r(cpustate->R0); return 1; }
OPHANDLER( anl_a_xr1 )		{ cpustate->a &= ram_r(cpustate->R1); return 1; }
OPHANDLER( anl_a_n )		{ cpustate->a &= argument_fetch(cpustate); return 2; }

OPHANDLER( anl_bus_n )		{ bus_w(bus_r() & argument_fetch(cpustate)); return 2; }
OPHANDLER( anl_p1_n )		{ port_w(1, cpustate->p1 &= argument_fetch(cpustate)); return 2; }
OPHANDLER( anl_p2_n )		{ port_w(2, cpustate->p2 &= argument_fetch(cpustate) | ~p2_mask(cpustate)); return 2; }
OPHANDLER( anld_p4_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 4); return 2; }
OPHANDLER( anld_p5_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 5); return 2; }
OPHANDLER( anld_p6_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 6); return 2; }
OPHANDLER( anld_p7_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 7); return 2; }

OPHANDLER( call_0 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x000); return 2; }
OPHANDLER( call_1 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x100); return 2; }
OPHANDLER( call_2 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x200); return 2; }
OPHANDLER( call_3 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x300); return 2; }
OPHANDLER( call_4 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x400); return 2; }
OPHANDLER( call_5 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x500); return 2; }
OPHANDLER( call_6 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x600); return 2; }
OPHANDLER( call_7 )			{ execute_call(cpustate, argument_fetch(cpustate) | 0x700); return 2; }

OPHANDLER( clr_a )			{ cpustate->a = 0; return 1; }
OPHANDLER( clr_c )			{ cpustate->psw &= ~C_FLAG; return 1; }
OPHANDLER( clr_f0 )			{ cpustate->psw &= ~F_FLAG; cpustate->sts &= ~STS_F0; return 1; }
OPHANDLER( clr_f1 )			{ cpustate->sts &= ~STS_F1; return 1; }

OPHANDLER( cpl_a )			{ cpustate->a ^= 0xff; return 1; }
OPHANDLER( cpl_c )			{ cpustate->psw ^= C_FLAG; return 1; }
OPHANDLER( cpl_f0 )			{ cpustate->psw ^= F_FLAG; cpustate->sts ^= STS_F0; return 1; }
OPHANDLER( cpl_f1 )			{ cpustate->sts ^= STS_F1; return 1; }

OPHANDLER( da_a )
{
	if ((cpustate->a & 0x0f) > 0x09 || (cpustate->psw & A_FLAG))
	{
		cpustate->a += 0x06;
		if ((cpustate->a & 0xf0) == 0x00)
			cpustate->psw |= C_FLAG;
	}
	if ((cpustate->a & 0xf0) > 0x90 || (cpustate->psw & C_FLAG))
	{
		cpustate->a += 0x60;
		cpustate->psw |= C_FLAG;
	}
	else
		cpustate->psw &= ~C_FLAG;
	return 1;
}

OPHANDLER( dec_a )			{ cpustate->a--; return 1; }
OPHANDLER( dec_r0 )			{ cpustate->R0--; return 1; }
OPHANDLER( dec_r1 )			{ cpustate->R1--; return 1; }
OPHANDLER( dec_r2 )			{ cpustate->R2--; return 1; }
OPHANDLER( dec_r3 )			{ cpustate->R3--; return 1; }
OPHANDLER( dec_r4 )			{ cpustate->R4--; return 1; }
OPHANDLER( dec_r5 )			{ cpustate->R5--; return 1; }
OPHANDLER( dec_r6 )			{ cpustate->R6--; return 1; }
OPHANDLER( dec_r7 )			{ cpustate->R7--; return 1; }

OPHANDLER( dis_i )			{ cpustate->xirq_enabled = FALSE; return 1; }
OPHANDLER( dis_tcnti )		{ cpustate->tirq_enabled = FALSE; cpustate->timer_overflow = FALSE; return 1; }

OPHANDLER( djnz_r0 )		{ execute_jcc(cpustate, --cpustate->R0 != 0); return 2; }
OPHANDLER( djnz_r1 )		{ execute_jcc(cpustate, --cpustate->R1 != 0); return 2; }
OPHANDLER( djnz_r2 )		{ execute_jcc(cpustate, --cpustate->R2 != 0); return 2; }
OPHANDLER( djnz_r3 )		{ execute_jcc(cpustate, --cpustate->R3 != 0); return 2; }
OPHANDLER( djnz_r4 )		{ execute_jcc(cpustate, --cpustate->R4 != 0); return 2; }
OPHANDLER( djnz_r5 )		{ execute_jcc(cpustate, --cpustate->R5 != 0); return 2; }
OPHANDLER( djnz_r6 )		{ execute_jcc(cpustate, --cpustate->R6 != 0); return 2; }
OPHANDLER( djnz_r7 )		{ execute_jcc(cpustate, --cpustate->R7 != 0); return 2; }

OPHANDLER( en_i )			{ cpustate->xirq_enabled = TRUE; return 1 + check_irqs(cpustate); }
OPHANDLER( en_tcnti )		{ cpustate->tirq_enabled = TRUE; return 1 + check_irqs(cpustate); }
OPHANDLER( en_dma )			{ cpustate->dma_enabled = TRUE; port_w(2, cpustate->p2); return 1; }
OPHANDLER( en_flags )		{ cpustate->flags_enabled = TRUE; port_w(2, cpustate->p2); return 1; }
OPHANDLER( ent0_clk )
{
	logerror("MCS-48 PC:%04X - Unimplemented opcode = %02x\n", cpustate->pc - 1, program_r(cpustate->pc - 1));
	return 1;
}

OPHANDLER( in_a_p1 )		{ cpustate->a = port_r(1) & cpustate->p1; return 2; }
OPHANDLER( in_a_p2 )		{ cpustate->a = port_r(2) & cpustate->p2; return 2; }
OPHANDLER( ins_a_bus )		{ cpustate->a = bus_r(); return 2; }
OPHANDLER( in_a_dbb )
{
	/* acknowledge the IBF IRQ and clear the bit in STS */
	if ((cpustate->sts & STS_IBF) != 0 && cpustate->irq_callback != NULL)
		(*cpustate->irq_callback)(cpustate->device, UPI41_INPUT_IBF);
	cpustate->sts &= ~STS_IBF;

	/* if P2 flags are enabled, update the state of P2 */
	if (cpustate->flags_enabled && (cpustate->p2 & P2_NIBF) == 0)
		port_w(2, cpustate->p2 |= P2_NIBF);
	cpustate->a = cpustate->dbbi;
	return 2;
}

OPHANDLER( inc_a )			{ cpustate->a++; return 1; }
OPHANDLER( inc_r0 )			{ cpustate->R0++; return 1; }
OPHANDLER( inc_r1 )			{ cpustate->R1++; return 1; }
OPHANDLER( inc_r2 )			{ cpustate->R2++; return 1; }
OPHANDLER( inc_r3 )			{ cpustate->R3++; return 1; }
OPHANDLER( inc_r4 )			{ cpustate->R4++; return 1; }
OPHANDLER( inc_r5 )			{ cpustate->R5++; return 1; }
OPHANDLER( inc_r6 )			{ cpustate->R6++; return 1; }
OPHANDLER( inc_r7 )			{ cpustate->R7++; return 1; }
OPHANDLER( inc_xr0 )		{ ram_w(cpustate->R0, ram_r(cpustate->R0) + 1); return 1; }
OPHANDLER( inc_xr1 )		{ ram_w(cpustate->R1, ram_r(cpustate->R1) + 1); return 1; }

OPHANDLER( jb_0 )			{ execute_jcc(cpustate, (cpustate->a & 0x01) != 0); return 2; }
OPHANDLER( jb_1 )			{ execute_jcc(cpustate, (cpustate->a & 0x02) != 0); return 2; }
OPHANDLER( jb_2 )			{ execute_jcc(cpustate, (cpustate->a & 0x04) != 0); return 2; }
OPHANDLER( jb_3 )			{ execute_jcc(cpustate, (cpustate->a & 0x08) != 0); return 2; }
OPHANDLER( jb_4 )			{ execute_jcc(cpustate, (cpustate->a & 0x10) != 0); return 2; }
OPHANDLER( jb_5 )			{ execute_jcc(cpustate, (cpustate->a & 0x20) != 0); return 2; }
OPHANDLER( jb_6 )			{ execute_jcc(cpustate, (cpustate->a & 0x40) != 0); return 2; }
OPHANDLER( jb_7 )			{ execute_jcc(cpustate, (cpustate->a & 0x80) != 0); return 2; }
OPHANDLER( jc )				{ execute_jcc(cpustate, (cpustate->psw & C_FLAG) != 0); return 2; }
OPHANDLER( jf0 )			{ execute_jcc(cpustate, (cpustate->psw & F_FLAG) != 0); return 2; }
OPHANDLER( jf1 )			{ execute_jcc(cpustate, (cpustate->sts & STS_F1) != 0); return 2; }
OPHANDLER( jnc )			{ execute_jcc(cpustate, (cpustate->psw & C_FLAG) == 0); return 2; }
OPHANDLER( jni )			{ execute_jcc(cpustate, cpustate->irq_state != 0); return 2; }
OPHANDLER( jnibf )			{ execute_jcc(cpustate, (cpustate->sts & STS_IBF) == 0); return 2; }
OPHANDLER( jnt_0 )  		{ execute_jcc(cpustate, test_r(0) == 0); return 2; }
OPHANDLER( jnt_1 )  		{ execute_jcc(cpustate, test_r(1) == 0); return 2; }
OPHANDLER( jnz )			{ execute_jcc(cpustate, cpustate->a != 0); return 2; }
OPHANDLER( jobf )			{ execute_jcc(cpustate, (cpustate->sts & STS_OBF) != 0); return 2; }
OPHANDLER( jtf )			{ execute_jcc(cpustate, cpustate->timer_flag); cpustate->timer_flag = FALSE; return 2; }
OPHANDLER( jt_0 )			{ execute_jcc(cpustate, test_r(0) != 0); return 2; }
OPHANDLER( jt_1 )			{ execute_jcc(cpustate, test_r(1) != 0); return 2; }
OPHANDLER( jz )				{ execute_jcc(cpustate, cpustate->a == 0); return 2; }

OPHANDLER( jmp_0 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x000); return 2; }
OPHANDLER( jmp_1 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x100); return 2; }
OPHANDLER( jmp_2 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x200); return 2; }
OPHANDLER( jmp_3 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x300); return 2; }
OPHANDLER( jmp_4 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x400); return 2; }
OPHANDLER( jmp_5 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x500); return 2; }
OPHANDLER( jmp_6 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x600); return 2; }
OPHANDLER( jmp_7 )			{ execute_jmp(cpustate, argument_fetch(cpustate) | 0x700); return 2; }
OPHANDLER( jmpp_xa )		{ cpustate->pc &= 0xf00; cpustate->pc |= program_r(cpustate->pc | cpustate->a); return 2; }

OPHANDLER( mov_a_n )		{ cpustate->a = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_a_psw )		{ cpustate->a = cpustate->psw; return 1; }
OPHANDLER( mov_a_r0 )		{ cpustate->a = cpustate->R0; return 1; }
OPHANDLER( mov_a_r1 )		{ cpustate->a = cpustate->R1; return 1; }
OPHANDLER( mov_a_r2 )		{ cpustate->a = cpustate->R2; return 1; }
OPHANDLER( mov_a_r3 )		{ cpustate->a = cpustate->R3; return 1; }
OPHANDLER( mov_a_r4 )		{ cpustate->a = cpustate->R4; return 1; }
OPHANDLER( mov_a_r5 )		{ cpustate->a = cpustate->R5; return 1; }
OPHANDLER( mov_a_r6 )		{ cpustate->a = cpustate->R6; return 1; }
OPHANDLER( mov_a_r7 )		{ cpustate->a = cpustate->R7; return 1; }
OPHANDLER( mov_a_xr0 )		{ cpustate->a = ram_r(cpustate->R0); return 1; }
OPHANDLER( mov_a_xr1 )		{ cpustate->a = ram_r(cpustate->R1); return 1; }
OPHANDLER( mov_a_t )		{ cpustate->a = cpustate->timer; return 1; }

OPHANDLER( mov_psw_a )		{ cpustate->psw = cpustate->a; update_regptr(cpustate); return 1; }
OPHANDLER( mov_sts_a )		{ cpustate->sts = (cpustate->sts & 0x0f) | (cpustate->a & 0xf0); return 1; }
OPHANDLER( mov_r0_a )		{ cpustate->R0 = cpustate->a; return 1; }
OPHANDLER( mov_r1_a )		{ cpustate->R1 = cpustate->a; return 1; }
OPHANDLER( mov_r2_a )		{ cpustate->R2 = cpustate->a; return 1; }
OPHANDLER( mov_r3_a )		{ cpustate->R3 = cpustate->a; return 1; }
OPHANDLER( mov_r4_a )		{ cpustate->R4 = cpustate->a; return 1; }
OPHANDLER( mov_r5_a )		{ cpustate->R5 = cpustate->a; return 1; }
OPHANDLER( mov_r6_a )		{ cpustate->R6 = cpustate->a; return 1; }
OPHANDLER( mov_r7_a )		{ cpustate->R7 = cpustate->a; return 1; }
OPHANDLER( mov_r0_n )		{ cpustate->R0 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r1_n )		{ cpustate->R1 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r2_n )		{ cpustate->R2 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r3_n )		{ cpustate->R3 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r4_n )		{ cpustate->R4 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r5_n )		{ cpustate->R5 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r6_n )		{ cpustate->R6 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_r7_n )		{ cpustate->R7 = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_t_a )		{ cpustate->timer = cpustate->a; return 1; }
OPHANDLER( mov_xr0_a )		{ ram_w(cpustate->R0, cpustate->a); return 1; }
OPHANDLER( mov_xr1_a )		{ ram_w(cpustate->R1, cpustate->a); return 1; }
OPHANDLER( mov_xr0_n )		{ ram_w(cpustate->R0, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_xr1_n )		{ ram_w(cpustate->R1, argument_fetch(cpustate)); return 2; }

OPHANDLER( movd_a_p4 )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 4); return 2; }
OPHANDLER( movd_a_p5 )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 5); return 2; }
OPHANDLER( movd_a_p6 )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 6); return 2; }
OPHANDLER( movd_a_p7 )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 7); return 2; }
OPHANDLER( movd_p4_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 4); return 2; }
OPHANDLER( movd_p5_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 5); return 2; }
OPHANDLER( movd_p6_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 6); return 2; }
OPHANDLER( movd_p7_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 7); return 2; }

OPHANDLER( movp_a_xa )		{ cpustate->a = program_r((cpustate->pc & 0xf00) | cpustate->a); return 2; }
OPHANDLER( movp3_a_xa )		{ cpustate->a = program_r(0x300 | cpustate->a); return 2; }

OPHANDLER( movx_a_xr0 )		{ cpustate->a = ext_r(cpustate->R0); return 2; }
OPHANDLER( movx_a_xr1 )		{ cpustate->a = ext_r(cpustate->R1); return 2; }
OPHANDLER( movx_xr0_a )		{ ext_w(cpustate->R0, cpustate->a); return 2; }
OPHANDLER( movx_xr1_a )		{ ext_w(cpustate->R1, cpustate->a); return 2; }

OPHANDLER( nop )			{ return 1; }

OPHANDLER( orl_a_r0 )		{ cpustate->a |= cpustate->R0; return 1; }
OPHANDLER( orl_a_r1 )		{ cpustate->a |= cpustate->R1; return 1; }
OPHANDLER( orl_a_r2 )		{ cpustate->a |= cpustate->R2; return 1; }
OPHANDLER( orl_a_r3 )		{ cpustate->a |= cpustate->R3; return 1; }
OPHANDLER( orl_a_r4 )		{ cpustate->a |= cpustate->R4; return 1; }
OPHANDLER( orl_a_r5 )		{ cpustate->a |= cpustate->R5; return 1; }
OPHANDLER( orl_a_r6 )		{ cpustate->a |= cpustate->R6; return 1; }
OPHANDLER( orl_a_r7 )		{ cpustate->a |= cpustate->R7; return 1; }
OPHANDLER( orl_a_xr0 )		{ cpustate->a |= ram_r(cpustate->R0); return 1; }
OPHANDLER( orl_a_xr1 )		{ cpustate->a |= ram_r(cpustate->R1); return 1; }
OPHANDLER( orl_a_n )		{ cpustate->a |= argument_fetch(cpustate); return 2; }

OPHANDLER( orl_bus_n )		{ bus_w(bus_r() | argument_fetch(cpustate)); return 2; }
OPHANDLER( orl_p1_n )		{ port_w(1, cpustate->p1 |= argument_fetch(cpustate)); return 2; }
OPHANDLER( orl_p2_n )		{ port_w(2, cpustate->p2 |= argument_fetch(cpustate) & p2_mask(cpustate)); return 2; }
OPHANDLER( orld_p4_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 4); return 2; }
OPHANDLER( orld_p5_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 5); return 2; }
OPHANDLER( orld_p6_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 6); return 2; }
OPHANDLER( orld_p7_a )		{ expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 7); return 2; }

OPHANDLER( outl_bus_a )		{ bus_w(cpustate->a); return 2; }
OPHANDLER( outl_p1_a )		{ port_w(1, cpustate->p1 = cpustate->a); return 2; }
OPHANDLER( outl_p2_a )		{ UINT8 mask = p2_mask(cpustate); port_w(2, cpustate->p2 = (cpustate->p2 & ~mask) | (cpustate->a & mask)); return 2; }
OPHANDLER( out_dbb_a )
{
	/* copy to the DBBO and update the bit in STS */
	cpustate->dbbo = cpustate->a;
	cpustate->sts |= STS_OBF;

	/* if P2 flags are enabled, update the state of P2 */
	if (cpustate->flags_enabled && (cpustate->p2 & P2_OBF) == 0)
		port_w(2, cpustate->p2 |= P2_OBF);
	return 2;
}


OPHANDLER( ret )			{ pull_pc(cpustate); return 2; }
OPHANDLER( retr )
{
	pull_pc_psw(cpustate);

	/* implicitly clear the IRQ in progress flip flop and re-check interrupts */
	cpustate->irq_in_progress = FALSE;
	return 2 + check_irqs(cpustate);
}

OPHANDLER( rl_a )			{ cpustate->a = (cpustate->a << 1) | (cpustate->a >> 7); return 1; }
OPHANDLER( rlc_a )			{ UINT8 newc = cpustate->a & C_FLAG; cpustate->a = (cpustate->a << 1) | (cpustate->psw >> 7); cpustate->psw = (cpustate->psw & ~C_FLAG) | newc; return 1; }

OPHANDLER( rr_a )			{ cpustate->a = (cpustate->a >> 1) | (cpustate->a << 7); return 1; }
OPHANDLER( rrc_a )			{ UINT8 newc = (cpustate->a << 7) & C_FLAG; cpustate->a = (cpustate->a >> 1) | (cpustate->psw & C_FLAG); cpustate->psw = (cpustate->psw & ~C_FLAG) | newc; return 1; }

OPHANDLER( sel_mb0 )		{ cpustate->a11 = 0x000; return 1; }
OPHANDLER( sel_mb1 )		{ cpustate->a11 = 0x800; return 1; }

OPHANDLER( sel_rb0 )		{ cpustate->psw &= ~B_FLAG; update_regptr(cpustate); return 1; }
OPHANDLER( sel_rb1 )		{ cpustate->psw |=  B_FLAG; update_regptr(cpustate); return 1; }

OPHANDLER( stop_tcnt )		{ cpustate->timecount_enabled = 0; return 1; }

OPHANDLER( strt_cnt )		{ cpustate->timecount_enabled = COUNTER_ENABLED; cpustate->t1_history = test_r(1); return 1; }
OPHANDLER( strt_t )			{ cpustate->timecount_enabled = TIMER_ENABLED; cpustate->prescaler = 0; return 1; }

OPHANDLER( swap_a )			{ cpustate->a = (cpustate->a << 4) | (cpustate->a >> 4); return 1; }

OPHANDLER( xch_a_r0 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R0; cpustate->R0 = tmp; return 1; }
OPHANDLER( xch_a_r1 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R1; cpustate->R1 = tmp; return 1; }
OPHANDLER( xch_a_r2 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R2; cpustate->R2 = tmp; return 1; }
OPHANDLER( xch_a_r3 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R3; cpustate->R3 = tmp; return 1; }
OPHANDLER( xch_a_r4 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R4; cpustate->R4 = tmp; return 1; }
OPHANDLER( xch_a_r5 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R5; cpustate->R5 = tmp; return 1; }
OPHANDLER( xch_a_r6 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R6; cpustate->R6 = tmp; return 1; }
OPHANDLER( xch_a_r7 )		{ UINT8 tmp = cpustate->a; cpustate->a = cpustate->R7; cpustate->R7 = tmp; return 1; }
OPHANDLER( xch_a_xr0 )		{ UINT8 tmp = cpustate->a; cpustate->a = ram_r(cpustate->R0); ram_w(cpustate->R0, tmp); return 1; }
OPHANDLER( xch_a_xr1 )		{ UINT8 tmp = cpustate->a; cpustate->a = ram_r(cpustate->R1); ram_w(cpustate->R1, tmp); return 1; }

OPHANDLER( xchd_a_xr0 )		{ UINT8 oldram = ram_r(cpustate->R0); ram_w(cpustate->R0, (oldram & 0xf0) | (cpustate->a & 0x0f)); cpustate->a = (cpustate->a & 0xf0) | (oldram & 0x0f); return 1; }
OPHANDLER( xchd_a_xr1 )		{ UINT8 oldram = ram_r(cpustate->R1); ram_w(cpustate->R1, (oldram & 0xf0) | (cpustate->a & 0x0f)); cpustate->a = (cpustate->a & 0xf0) | (oldram & 0x0f); return 1; }

OPHANDLER( xrl_a_r0 )		{ cpustate->a ^= cpustate->R0; return 1; }
OPHANDLER( xrl_a_r1 )		{ cpustate->a ^= cpustate->R1; return 1; }
OPHANDLER( xrl_a_r2 )		{ cpustate->a ^= cpustate->R2; return 1; }
OPHANDLER( xrl_a_r3 )		{ cpustate->a ^= cpustate->R3; return 1; }
OPHANDLER( xrl_a_r4 )		{ cpustate->a ^= cpustate->R4; return 1; }
OPHANDLER( xrl_a_r5 )		{ cpustate->a ^= cpustate->R5; return 1; }
OPHANDLER( xrl_a_r6 )		{ cpustate->a ^= cpustate->R6; return 1; }
OPHANDLER( xrl_a_r7 )		{ cpustate->a ^= cpustate->R7; return 1; }
OPHANDLER( xrl_a_xr0 )		{ cpustate->a ^= ram_r(cpustate->R0); return 1; }
OPHANDLER( xrl_a_xr1 )		{ cpustate->a ^= ram_r(cpustate->R1); return 1; }
OPHANDLER( xrl_a_n )		{ cpustate->a ^= argument_fetch(cpustate); return 2; }

SPLIT_OPHANDLER( split_02, outl_bus_a, out_dbb_a )
SPLIT_OPHANDLER( split_08, ins_a_bus,  illegal )
SPLIT_OPHANDLER( split_22, illegal,    in_a_dbb )
SPLIT_OPHANDLER( split_75, ent0_clk,   illegal )
SPLIT_OPHANDLER( split_80, movx_a_xr0, illegal )
SPLIT_OPHANDLER( split_81, movx_a_xr1, illegal )
SPLIT_OPHANDLER( split_86, jni,        jobf )
SPLIT_OPHANDLER( split_88, orl_bus_n,  illegal )
SPLIT_OPHANDLER( split_90, movx_xr0_a, mov_sts_a )
SPLIT_OPHANDLER( split_91, movx_xr1_a, illegal )
SPLIT_OPHANDLER( split_98, anl_bus_n,  illegal )
SPLIT_OPHANDLER( split_d6, illegal,    jnibf )
SPLIT_OPHANDLER( split_e5, sel_mb0,    en_dma )
SPLIT_OPHANDLER( split_f5, sel_mb1,    en_flags )



/***************************************************************************
    OPCODE TABLES
***************************************************************************/

static const mcs48_ophandler opcode_table[256]=
{
	nop,        illegal,    split_02,  add_a_n,   jmp_0,     en_i,       illegal,   dec_a,         /* 00 */
	split_08,   in_a_p1,    in_a_p2,   illegal,   movd_a_p4, movd_a_p5,  movd_a_p6, movd_a_p7,
	inc_xr0,    inc_xr1,    jb_0,      adc_a_n,   call_0,    dis_i,      jtf,       inc_a,         /* 10 */
	inc_r0,     inc_r1,     inc_r2,    inc_r3,    inc_r4,    inc_r5,     inc_r6,    inc_r7,
	xch_a_xr0,  xch_a_xr1,  split_22,  mov_a_n,   jmp_1,     en_tcnti,   jnt_0,     clr_a,         /* 20 */
	xch_a_r0,   xch_a_r1,   xch_a_r2,  xch_a_r3,  xch_a_r4,  xch_a_r5,   xch_a_r6,  xch_a_r7,
	xchd_a_xr0, xchd_a_xr1, jb_1,      illegal,   call_1,    dis_tcnti,  jt_0,      cpl_a,         /* 30 */
	illegal,    outl_p1_a,  outl_p2_a, illegal,   movd_p4_a, movd_p5_a,  movd_p6_a, movd_p7_a,
	orl_a_xr0,  orl_a_xr1,  mov_a_t,   orl_a_n,   jmp_2,     strt_cnt,   jnt_1,     swap_a,        /* 40 */
	orl_a_r0,   orl_a_r1,   orl_a_r2,  orl_a_r3,  orl_a_r4,  orl_a_r5,   orl_a_r6,  orl_a_r7,
	anl_a_xr0,  anl_a_xr1,  jb_2,      anl_a_n,   call_2,    strt_t,     jt_1,      da_a,          /* 50 */
	anl_a_r0,   anl_a_r1,   anl_a_r2,  anl_a_r3,  anl_a_r4,  anl_a_r5,   anl_a_r6,  anl_a_r7,
	add_a_xr0,  add_a_xr1,  mov_t_a,   illegal,   jmp_3,     stop_tcnt,  illegal,   rrc_a,         /* 60 */
	add_a_r0,   add_a_r1,   add_a_r2,  add_a_r3,  add_a_r4,  add_a_r5,   add_a_r6,  add_a_r7,
	adc_a_xr0,  adc_a_xr1,  jb_3,      illegal,   call_3,    split_75,   jf1,       rr_a,          /* 70 */
	adc_a_r0,   adc_a_r1,   adc_a_r2,  adc_a_r3,  adc_a_r4,  adc_a_r5,   adc_a_r6,  adc_a_r7,
	split_80,   split_81,   illegal,   ret,       jmp_4,     clr_f0,     split_86,  illegal,       /* 80 */
	split_88,   orl_p1_n,   orl_p2_n,  illegal,   orld_p4_a, orld_p5_a,  orld_p6_a, orld_p7_a,
	split_90,   split_91,   jb_4,      retr,      call_4,    cpl_f0,     jnz,       clr_c,         /* 90 */
	split_98,   anl_p1_n,   anl_p2_n,  illegal,   anld_p4_a, anld_p5_a,  anld_p6_a, anld_p7_a,
	mov_xr0_a,  mov_xr1_a,  illegal,   movp_a_xa, jmp_5,     clr_f1,     illegal,   cpl_c,         /* A0 */
	mov_r0_a,   mov_r1_a,   mov_r2_a,  mov_r3_a,  mov_r4_a,  mov_r5_a,   mov_r6_a,  mov_r7_a,
	mov_xr0_n,  mov_xr1_n,  jb_5,      jmpp_xa,   call_5,    cpl_f1,     jf0,       illegal,       /* B0 */
	mov_r0_n,   mov_r1_n,   mov_r2_n,  mov_r3_n,  mov_r4_n,  mov_r5_n,   mov_r6_n,  mov_r7_n,
	illegal,    illegal,    illegal,   illegal,   jmp_6,     sel_rb0,    jz,        mov_a_psw,     /* C0 */
	dec_r0,     dec_r1,     dec_r2,    dec_r3,    dec_r4,    dec_r5,     dec_r6,    dec_r7,
	xrl_a_xr0,  xrl_a_xr1,  jb_6,      xrl_a_n,   call_6,    sel_rb1,    split_d6,  mov_psw_a,     /* D0 */
	xrl_a_r0,   xrl_a_r1,   xrl_a_r2,  xrl_a_r3,  xrl_a_r4,  xrl_a_r5,   xrl_a_r6,  xrl_a_r7,
	illegal,    illegal,    illegal,   movp3_a_xa,jmp_7,     split_e5,   jnc,       rl_a,          /* E0 */
	djnz_r0,    djnz_r1,    djnz_r2,   djnz_r3,   djnz_r4,   djnz_r5,    djnz_r6,   djnz_r7,
	mov_a_xr0,  mov_a_xr1,  jb_7,      illegal,   call_7,    split_f5,   jc,        rlc_a,         /* F0 */
	mov_a_r0,   mov_a_r1,   mov_a_r2,  mov_a_r3,  mov_a_r4,  mov_a_r5,   mov_a_r6,  mov_a_r7
};



/***************************************************************************
    INITIALIZATION/RESET
***************************************************************************/

/*-------------------------------------------------
    mcs48_init - generic MCS-48 initialization
-------------------------------------------------*/

static void mcs48_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback, UINT8 feature_mask, UINT16 romsize)
{
	mcs48_state *cpustate = get_safe_token(device);

	/* External access line
     * EA=1 : read from external rom
     * EA=0 : read from internal rom
     */

	/* FIXME: Current implementation suboptimal */
	cpustate->ea = (romsize ? 0 : 1);

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->int_rom_size = romsize;
	cpustate->feature_mask = feature_mask;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	/* set up the state table */
	{
		device_state_interface *state;
		device->interface(state);
		state->state_add(MCS48_PC,        "PC",        cpustate->pc).mask(0xfff);
		state->state_add(STATE_GENPC,     "GENPC",     cpustate->pc).mask(0xfff).noshow();
		state->state_add(STATE_GENPCBASE, "GENPCBASE", cpustate->prevpc).mask(0xfff).noshow();
		state->state_add(STATE_GENSP,     "GENSP",     cpustate->psw).mask(0x7).noshow();
		state->state_add(STATE_GENFLAGS,  "GENFLAGS",  cpustate->psw).noshow().formatstr("%10s");
		state->state_add(MCS48_A,         "A",         cpustate->a);
		state->state_add(MCS48_TC,        "TC",        cpustate->timer);
		state->state_add(MCS48_TPRE,      "TPRE",      cpustate->prescaler).mask(0x1f);
		state->state_add(MCS48_P1,        "P1",        cpustate->p1);
		state->state_add(MCS48_P2,        "P2",        cpustate->p2);

		astring tempstr;
		for (int regnum = 0; regnum < 8; regnum++)
			state->state_add(MCS48_R0 + regnum, tempstr.format("R%d", regnum), cpustate->rtemp).callimport().callexport();

		state->state_add(MCS48_EA,        "EA",        cpustate->ea).mask(0x1);

		if (feature_mask & UPI41_FEATURE)
		{
			state->state_add(MCS48_STS,   "STS",       cpustate->sts);
			state->state_add(MCS48_DBBI,  "DBBI",      cpustate->dbbi);
			state->state_add(MCS48_DBBO,  "DBBO",      cpustate->dbbo);
		}

	}

	/* ensure that regptr is valid before get_info gets called */
	update_regptr(cpustate);

	device->save_item(NAME(cpustate->prevpc));
	device->save_item(NAME(cpustate->pc));

	device->save_item(NAME(cpustate->a));
	device->save_item(NAME(cpustate->psw));
	device->save_item(NAME(cpustate->p1));
	device->save_item(NAME(cpustate->p2));
	device->save_item(NAME(cpustate->ea));
	device->save_item(NAME(cpustate->timer));
	device->save_item(NAME(cpustate->prescaler));
	device->save_item(NAME(cpustate->t1_history));
	device->save_item(NAME(cpustate->sts));
	device->save_item(NAME(cpustate->dbbi));
	device->save_item(NAME(cpustate->dbbo));

	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->irq_in_progress));
	device->save_item(NAME(cpustate->timer_overflow));
	device->save_item(NAME(cpustate->timer_flag));
	device->save_item(NAME(cpustate->tirq_enabled));
	device->save_item(NAME(cpustate->xirq_enabled));
	device->save_item(NAME(cpustate->timecount_enabled));
	device->save_item(NAME(cpustate->flags_enabled));
	device->save_item(NAME(cpustate->dma_enabled));

	device->save_item(NAME(cpustate->a11));
}


/*-------------------------------------------------
    mcs48_norom_init - initialization for systems
    with no internal ROM
-------------------------------------------------*/

static CPU_INIT( mcs48_norom )
{
	mcs48_init(device, irqcallback, MCS48_FEATURE, 0x0);
}


/*-------------------------------------------------
    mcs48_1k_rom_init - initialization for systems
    with 1k of internal ROM
-------------------------------------------------*/

static CPU_INIT( mcs48_1k_rom )
{
	mcs48_init(device, irqcallback, MCS48_FEATURE, 0x400);
}


/*-------------------------------------------------
    mcs48_2k_rom - initialization for systems
    with 2k of internal ROM
-------------------------------------------------*/

static CPU_INIT( mcs48_2k_rom )
{
	mcs48_init(device, irqcallback, MCS48_FEATURE, 0x800);
}


/*-------------------------------------------------
    mcs48_4k_rom - initialization for systems
    with 2k of internal ROM
-------------------------------------------------*/

static CPU_INIT( mcs48_4k_rom )
{
	mcs48_init(device, irqcallback, MCS48_FEATURE, 0x1000);
}


/*-------------------------------------------------
    upi41_1k_rom_init - initialization for systems
    with 1k of internal ROM
-------------------------------------------------*/

static CPU_INIT( upi41_1k_rom )
{
	mcs48_init(device, irqcallback, UPI41_FEATURE, 0x400);
}


/*-------------------------------------------------
    upi41_2k_rom_init - initialization for systems
    with 2k of internal ROM
-------------------------------------------------*/

static CPU_INIT( upi41_2k_rom )
{
	mcs48_init(device, irqcallback, UPI41_FEATURE, 0x800);
}


/*-------------------------------------------------
    mcs48_reset - general reset routine
-------------------------------------------------*/

static CPU_RESET( mcs48 )
{
	mcs48_state *cpustate = get_safe_token(device);

	/* confirmed from reset description */
	cpustate->pc = 0;
	cpustate->psw = (cpustate->psw & (C_FLAG | A_FLAG)) | 0x08;
	cpustate->a11 = 0x000;
	bus_w(0xff);
	cpustate->p1 = 0xff;
	cpustate->p2 = 0xff;
	port_w(1, cpustate->p1);
	port_w(2, cpustate->p2);
	cpustate->tirq_enabled = FALSE;
	cpustate->xirq_enabled = FALSE;
	cpustate->timecount_enabled = 0;
	cpustate->timer_flag = FALSE;
	cpustate->sts = 0;
	cpustate->flags_enabled = FALSE;
	cpustate->dma_enabled = FALSE;

	/* confirmed from interrupt logic description */
	cpustate->irq_in_progress = FALSE;
	cpustate->timer_overflow = FALSE;
}



/***************************************************************************
    EXECUTION
***************************************************************************/

/*-------------------------------------------------
    check_irqs - check for and process IRQs
-------------------------------------------------*/

static int check_irqs(mcs48_state *cpustate)
{
	/* if something is in progress, we do nothing */
	if (cpustate->irq_in_progress)
		return 0;

	/* external interrupts take priority */
	if ((cpustate->irq_state || (cpustate->sts & STS_IBF) != 0) && cpustate->xirq_enabled)
	{
		cpustate->irq_in_progress = TRUE;

		/* transfer to location 0x03 */
		push_pc_psw(cpustate);
		cpustate->pc = 0x03;

		/* indicate we took the external IRQ */
		if (cpustate->irq_callback != NULL)
			(*cpustate->irq_callback)(cpustate->device, 0);
		return 2;
	}

	/* timer overflow interrupts follow */
	if (cpustate->timer_overflow && cpustate->tirq_enabled)
	{
		cpustate->irq_in_progress = TRUE;

		/* transfer to location 0x07 */
		push_pc_psw(cpustate);
		cpustate->pc = 0x07;

		/* timer overflow flip-flop is reset once taken */
		cpustate->timer_overflow = FALSE;
		return 2;
	}
	return 0;
}


/*-------------------------------------------------
    burn_cycles - burn cycles, processing timers
    and counters
-------------------------------------------------*/

static void burn_cycles(mcs48_state *cpustate, int count)
{
	int timerover = FALSE;

	/* if the timer is enabled, accumulate prescaler cycles */
	if (cpustate->timecount_enabled & TIMER_ENABLED)
	{
		UINT8 oldtimer = cpustate->timer;
		cpustate->prescaler += count;
		cpustate->timer += cpustate->prescaler >> 5;
		cpustate->prescaler &= 0x1f;
		timerover = (oldtimer != 0 && cpustate->timer == 0);
	}

	/* if the counter is enabled, poll the T1 test input once for each cycle */
	else if (cpustate->timecount_enabled & COUNTER_ENABLED)
		for ( ; count > 0; count--)
		{
			cpustate->t1_history = (cpustate->t1_history << 1) | (test_r(1) & 1);
			if ((cpustate->t1_history & 3) == 2)
				timerover = (++cpustate->timer == 0);
		}

	/* if either source caused a timer overflow, set the flags and check IRQs */
	if (timerover)
	{
		cpustate->timer_flag = TRUE;

		/* according to the docs, if an overflow occurs with interrupts disabled, the overflow is not stored */
		if (cpustate->tirq_enabled)
		{
			cpustate->timer_overflow = TRUE;
			check_irqs(cpustate);
		}
	}
}


/*-------------------------------------------------
    mcs48_execute - execute until we run out
    of cycles
-------------------------------------------------*/

static CPU_EXECUTE( mcs48 )
{
	mcs48_state *cpustate = get_safe_token(device);
	int curcycles;

	update_regptr(cpustate);

	/* external interrupts may have been set since we last checked */
	curcycles = check_irqs(cpustate);
	cpustate->icount -= curcycles;
	if (cpustate->timecount_enabled != 0)
		burn_cycles(cpustate, curcycles);

	/* iterate over remaining cycles, guaranteeing at least one instruction */
	do
	{
		unsigned opcode;

		/* fetch next opcode */
		cpustate->prevpc = cpustate->pc;
		debugger_instruction_hook(device, cpustate->pc);
		opcode = opcode_fetch(cpustate);

		/* process opcode and count cycles */
		curcycles = (*opcode_table[opcode])(cpustate);

		/* burn the cycles */
		cpustate->icount -= curcycles;
		if (cpustate->timecount_enabled != 0)
			burn_cycles(cpustate, curcycles);

	} while (cpustate->icount > 0);
}



/***************************************************************************
    DATA ACCESS HELPERS
***************************************************************************/

/*-------------------------------------------------
    upi41_master_r - master CPU data/status
    read
-------------------------------------------------*/

UINT8 upi41_master_r(device_t *device, UINT8 a0)
{
	mcs48_state *cpustate = get_safe_token(device);

	/* if just reading the status, return it */
	if ((a0 & 1) != 0)
		return cpustate->sts;

	/* if the output buffer was full, it gets cleared now */
	if (cpustate->sts & STS_OBF)
	{
		cpustate->sts &= ~STS_OBF;
		if (cpustate->flags_enabled)
			port_w(2, cpustate->p2 &= ~P2_OBF);
	}
	return cpustate->dbbo;
}


/*-------------------------------------------------
    upi41_master_w - master CPU command/data
    write
-------------------------------------------------*/

static TIMER_CALLBACK( master_callback )
{
	legacy_cpu_device *device = (legacy_cpu_device *)ptr;
	mcs48_state *cpustate = get_safe_token(device);
	UINT8 a0 = (param >> 8) & 1;
	UINT8 data = param;

	/* data always goes to the input buffer */
	cpustate->dbbi = data;

	/* set the appropriate flags */
	if ((cpustate->sts & STS_IBF) == 0)
	{
		cpustate->sts |= STS_IBF;
		if (cpustate->flags_enabled)
			port_w(2, cpustate->p2 &= ~P2_NIBF);
	}

	/* set F1 accordingly */
	if (a0 == 0)
		cpustate->sts &= ~STS_F1;
	else
		cpustate->sts |= STS_F1;
}

void upi41_master_w(device_t *_device, UINT8 a0, UINT8 data)
{
	legacy_cpu_device *device = downcast<legacy_cpu_device *>(_device);
	device->machine().scheduler().synchronize(FUNC(master_callback), (a0 << 8) | data, (void *)device);
}



/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

/* FIXME: the memory maps should probably support rom banking for EA */
static ADDRESS_MAP_START(program_10bit, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_11bit, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_12bit, AS_PROGRAM, 8, legacy_cpu_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_6bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_7bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_8bit, AS_DATA, 8, legacy_cpu_device)
	AM_RANGE(0x00, 0xff) AM_RAM
ADDRESS_MAP_END



/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

/*-------------------------------------------------
    mcs48_import_state - import state from the
    debugger into our internal format
-------------------------------------------------*/

static CPU_IMPORT_STATE( mcs48 )
{
	mcs48_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case MCS48_R0:
		case MCS48_R1:
		case MCS48_R2:
		case MCS48_R3:
		case MCS48_R4:
		case MCS48_R5:
		case MCS48_R6:
		case MCS48_R7:
			cpustate->regptr[entry.index() - MCS48_R0] = cpustate->rtemp;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(mcs48) called for unexpected value\n");
			break;
	}
}


/*-------------------------------------------------
    mcs48_export_state - prepare state for
    exporting to the debugger
-------------------------------------------------*/

static CPU_EXPORT_STATE( mcs48 )
{
	mcs48_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case MCS48_R0:
		case MCS48_R1:
		case MCS48_R2:
		case MCS48_R3:
		case MCS48_R4:
		case MCS48_R5:
		case MCS48_R6:
		case MCS48_R7:
			cpustate->rtemp = cpustate->regptr[entry.index() - MCS48_R0];
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(mcs48) called for unexpected value\n");
			break;
	}
}

static CPU_EXPORT_STRING( mcs48 )
{
	mcs48_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case CPUINFO_STR_FLAGS:
			string.printf("%c%c %c%c%c%c%c%c%c%c",
				cpustate->irq_state ? 'I':'.',
				cpustate->a11       ? 'M':'.',
				cpustate->psw & 0x80 ? 'C':'.',
				cpustate->psw & 0x40 ? 'A':'.',
				cpustate->psw & 0x20 ? 'F':'.',
				cpustate->psw & 0x10 ? 'B':'.',
				cpustate->psw & 0x08 ? '?':'.',
				cpustate->psw & 0x04 ? '4':'.',
				cpustate->psw & 0x02 ? '2':'.',
				cpustate->psw & 0x01 ? '1':'.');
			break;
	}
}

/*-------------------------------------------------
    mcs48_set_info - set a piece of information
    on the CPU core
-------------------------------------------------*/

static CPU_SET_INFO( mcs48 )
{
	mcs48_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_IRQ:	cpustate->irq_state = (info->i != CLEAR_LINE);	break;
		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_EA:	cpustate->ea = (info->i != CLEAR_LINE);	break;
	}
}


/*-------------------------------------------------
    mcs48_get_info - retrieve a piece of
    information from the CPU core
-------------------------------------------------*/

static CPU_GET_INFO( mcs48 )
{
	mcs48_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mcs48_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = MCS48_INPUT_IRQ;				break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 12;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			/*info->i = 6 or 7 or 8;*/				break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 9;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;							break;

		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_IRQ:	info->i = cpustate->irq_state ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_EA:	info->i = cpustate->ea;					break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(mcs48);				break;
		case CPUINFO_FCT_INIT:			/* set per-core */										break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(mcs48);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(mcs48);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(mcs48);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(mcs48);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(mcs48);		break;
		case CPUINFO_FCT_EXPORT_STRING:	info->export_string = CPU_EXPORT_STRING_NAME(mcs48);	break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	/* set per-core */						break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		/* set per-core */						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							/* set per-core */						break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Intel 8039");			break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Mirko Buffoni\nBased on the original work Copyright Dan Boris"); break;
	}
}



/***************************************************************************
    CPU-SPECIFIC CONTEXT ACCESS
***************************************************************************/

static void mcs48_generic_get_info(legacy_cpu_device *device, UINT32 state, cpuinfo *info, UINT8 features, int romsize, int ramsize, int cycle_states, const char *name)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:
			info->i = 3 * cycle_states;
			break;

		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:
			if (ramsize == 64)
				info->i = 6;
			else if (ramsize == 128)
				info->i = 7;
			else if (ramsize == 256)
				info->i = 8;
			else
				fatalerror("mcs48_generic_get_info: Invalid RAM size\n");
			break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:
			if (romsize == 0)
				info->init = CPU_INIT_NAME(mcs48_norom);
			else if (romsize == 1024)
				info->init = (features == UPI41_FEATURE) ? CPU_INIT_NAME(upi41_1k_rom) : CPU_INIT_NAME(mcs48_1k_rom);
			else if (romsize == 2048)
				info->init = (features == UPI41_FEATURE) ? CPU_INIT_NAME(upi41_2k_rom) : CPU_INIT_NAME(mcs48_2k_rom);
			else if (romsize == 4096)
				info->init = CPU_INIT_NAME(mcs48_4k_rom);
			else
				fatalerror("mcs48_generic_get_info: Invalid ROM size\n");
			break;

		case CPUINFO_FCT_DISASSEMBLE:
			if (features == UPI41_FEATURE)
				info->disassemble = CPU_DISASSEMBLE_NAME(upi41);
			else
				info->disassemble = CPU_DISASSEMBLE_NAME(mcs48);
			break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:
			if (romsize == 0)
				info->internal_map8 = NULL;
			else if (romsize == 1024)
				info->internal_map8 = ADDRESS_MAP_NAME(program_10bit);
			else if (romsize == 2048)
				info->internal_map8 = ADDRESS_MAP_NAME(program_11bit);
			else if (romsize == 4096)
				info->internal_map8 = ADDRESS_MAP_NAME(program_12bit);
			else
				fatalerror("mcs48_generic_get_info: Invalid RAM size\n");
			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:
			if (ramsize == 64)
				info->internal_map8 = ADDRESS_MAP_NAME(data_6bit);
			else if (ramsize == 128)
				info->internal_map8 = ADDRESS_MAP_NAME(data_7bit);
			else if (ramsize == 256)
				info->internal_map8 = ADDRESS_MAP_NAME(data_8bit);
			else
				fatalerror("mcs48_generic_get_info: Invalid RAM size\n");
			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:
			strcpy(info->s, name);
			break;

		/* default case */
		default:
			CPU_GET_INFO_CALL(mcs48);
			break;
	}
}


/* Official Intel MCS-48 parts */
CPU_GET_INFO( i8021 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 1024,  64, 10, "I8021"); }
CPU_GET_INFO( i8022 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 2048, 128, 10, "I8022"); }
CPU_GET_INFO( i8035 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE,    0,  64,  5, "I8035"); }
CPU_GET_INFO( i8048 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 1024,  64,  5, "I8048"); }
CPU_GET_INFO( i8648 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 1024,  64,  5, "I8648"); }
CPU_GET_INFO( i8748 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 1024,  64,  5, "I8748"); }
CPU_GET_INFO( i8039 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE,    0, 128,  5, "I8039"); }
CPU_GET_INFO( i8049 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 2048, 128,  5, "I8049"); }
CPU_GET_INFO( i8749 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 2048, 128,  5, "I8749"); }
CPU_GET_INFO( i8040 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE,    0, 256,  5, "I8040"); }
CPU_GET_INFO( i8050 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 4096, 256,  5, "I8050"); }


/* Official Intel UPI-41 parts */
CPU_GET_INFO( i8041 )  { mcs48_generic_get_info(device, state, info, UPI41_FEATURE, 1024, 128,  5, "I8041"); }
CPU_GET_INFO( i8741 )  { mcs48_generic_get_info(device, state, info, UPI41_FEATURE, 1024, 128,  5, "I8741"); }
CPU_GET_INFO( i8042 )  { mcs48_generic_get_info(device, state, info, UPI41_FEATURE, 2048, 256,  5, "I8042"); }
CPU_GET_INFO( i8242 )  { mcs48_generic_get_info(device, state, info, UPI41_FEATURE, 2048, 256,  5, "I8242"); }
CPU_GET_INFO( i8742 )  { mcs48_generic_get_info(device, state, info, UPI41_FEATURE, 2048, 256,  5, "I8742"); }


/* Clones */
CPU_GET_INFO( mb8884 ) { mcs48_generic_get_info(device, state, info, MCS48_FEATURE,    0,  64,  5, "MB8884"); }
CPU_GET_INFO( n7751 )  { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 1024,  64,  5, "N7751"); }
CPU_GET_INFO( m58715 ) { mcs48_generic_get_info(device, state, info, MCS48_FEATURE, 2048, 128,  5, "M58715"); }

/* Official Intel MCS-48 parts */
DEFINE_LEGACY_CPU_DEVICE(I8021, i8021);			/* 1k internal ROM,      64 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8022, i8022);			/* 2k internal ROM,     128 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8035, i8035);			/* external ROM,         64 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8048, i8048);			/* 1k internal ROM,      64 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8648, i8648);			/* 1k internal OTP ROM,  64 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8748, i8748);			/* 1k internal EEPROM,   64 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8039, i8039);			/* external ROM,        128 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8049, i8049);			/* 2k internal ROM,     128 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8749, i8749);			/* 2k internal EEPROM,  128 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8040, i8040);			/* external ROM,        256 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8050, i8050);			/* 4k internal ROM,     256 bytes internal RAM */

/* Official Intel UPI-41 parts */
DEFINE_LEGACY_CPU_DEVICE(I8041, i8041);			/* 1k internal ROM,     128 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8741, i8741);			/* 1k internal EEPROM,  128 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8042, i8042);			/* 2k internal ROM,     256 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8242, i8242);			/* 2k internal ROM,     256 bytes internal RAM */
DEFINE_LEGACY_CPU_DEVICE(I8742, i8742);			/* 2k internal EEPROM,  256 bytes internal RAM */

/* Clones */
DEFINE_LEGACY_CPU_DEVICE(MB8884, mb8884);		/* 8035 clone */
DEFINE_LEGACY_CPU_DEVICE(N7751, n7751);			/* 8048 clone */
DEFINE_LEGACY_CPU_DEVICE(M58715, m58715);		/* 8049 clone */
