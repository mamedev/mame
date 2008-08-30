/*
EA pin - defined by architecture, must implement:
   1 means external access, bypassing internal ROM
   reimplement as a push, not a pull
T0 output clock
*/

/***************************************************************************

    mcs48.c

    Intel MCS-48 Portable Emulator

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
    8041    64   1k   18  (ROM)
    8048    64   1k   27  (ROM)
    8648    64   1k   27  (OTPROM)
    8741    64   1k   18  (EPROM)
    8748    64   1k   27  (EPROM)
    8884    64   1k
    N7751  128   2k

    8039   128    0   27  (external ROM)
    8049   128   2k   27  (ROM)
    8749   128   2k   27  (EPROM)
    M58715 128    0       (external ROM)

***************************************************************************/

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



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* live processor state */
typedef struct _mcs48_regs mcs48_regs;
struct _mcs48_regs
{
	PAIR		prevpc;				/* 16-bit previous program counter */
	PAIR		pc;					/* 16-bit program counter */
	UINT8		a;					/* 8-bit accumulator */
	UINT8 *		regptr;				/* pointer to r0-r7 */
	UINT8		psw;				/* 8-bit PSW */
	UINT8		p1;					/* 8-bit latched port 1 */
	UINT8		p2;					/* 8-bit latched port 2 */
	UINT8		f1;					/* 1-bit flag 1 */
	UINT8		ea;					/* 1-bit latched ea input */
	UINT8		timer;				/* 8-bit timer */
	UINT8		prescaler;			/* 5-bit timer prescaler */
	UINT8		t1_history;			/* 8-bit history of the T1 input */

	UINT8		irq_state;			/* TRUE if an IRQ is pending */
	UINT8		irq_in_progress;	/* TRUE if an IRQ is in progress */
	UINT8		timer_overflow;		/* TRUE on a timer overflow; cleared by taking interrupt */
	UINT8		timer_flag;			/* TRUE on a timer overflow; cleared on JTF */
	UINT8		tirq_enabled;		/* TRUE if the timer IRQ is enabled */
	UINT8		xirq_enabled;		/* TRUE if the external IRQ is enabled */
	UINT8		timecount_enabled;	/* bitmask of timer/counter enabled */

	UINT16		a11;				/* A11 value, either 0x000 or 0x800 */

	int			(*irq_callback)(int irqline);
	int			inst_cycles;		/* cycles for the current instruction */
	UINT8		cpu_feature;		/* processor feature flags */
	UINT16		int_rom_size;		/* internal rom size */
};


/* opcode table entry */
typedef struct _mcs48_opcode mcs48_opcode;
struct _mcs48_opcode
{
	UINT8		cycles;
	void 		(*function)(void);
};



/***************************************************************************
    MACROS
***************************************************************************/

/*** Cycle times for the jump on condition instructions, are unusual.
     Condition is tested during the first cycle, so if condition is not
     met, second address fetch cycle may not really be taken. For now we
     just use the cycle counts as listed in the i8048 user manual.
***/

#if 0
#define ADJUST_CYCLES { inst_cycles -= 1; }	/* Possible real cycles setting */
#else
#define ADJUST_CYCLES { }					/* User Manual cycles setting */
#endif


/* ROM is mapped to ADDRESS_SPACE_PROGRAM */
#define program_r(a)	program_read_byte_8le(a)

/* RAM is mapped to ADDRESS_SPACE_DATA */
#define ram_r(a)		data_read_byte_8le(a)
#define ram_w(a,V)		data_write_byte_8le(a, V)

/* ports are mapped to ADDRESS_SPACE_IO */
#define ext_r(a)		io_read_byte_8le(a)
#define ext_w(a,V)		io_write_byte_8le(a, V)
#define port_r(a)		io_read_byte_8le(MCS48_PORT_P0 + a)
#define port_w(a,V)		io_write_byte_8le(MCS48_PORT_P0 + a, V)
#define test_r(a)		io_read_byte_8le(MCS48_PORT_T0 + a)
#define test_w(a,V)		io_write_byte_8le(MCS48_PORT_T0 + a, V)
#define bus_r()			io_read_byte_8le(MCS48_PORT_BUS)
#define bus_w(V)		io_write_byte_8le(MCS48_PORT_BUS, V)
#define ea_r()			io_read_byte_8le(MCS48_PORT_EA)

/* simplfied access to common bits */
#undef A
#define A				mcs48.a
#undef PC
#define PC				mcs48.pc.w.l
#undef PSW
#define PSW				mcs48.psw

/* r0-r7 map to memory via the regptr */
#define R0				mcs48.regptr[0]
#define R1				mcs48.regptr[1]
#define R2				mcs48.regptr[2]
#define R3				mcs48.regptr[3]
#define R4				mcs48.regptr[4]
#define R5				mcs48.regptr[5]
#define R6				mcs48.regptr[6]
#define R7				mcs48.regptr[7]

/* the carry flag as 0 or 1, used for carry-in */
#define CARRYIN			((PSW & C_FLAG) >> 7)



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static mcs48_regs	mcs48;
static int			mcs48_icount;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void check_irqs(void);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    opcode_fetch - fetch an opcode byte
-------------------------------------------------*/

INLINE UINT8 opcode_fetch(offs_t address)
{
	return cpu_readop(address);
}


/*-------------------------------------------------
    argument_fetch - fetch an opcode argument
    byte
-------------------------------------------------*/

INLINE UINT8 argument_fetch(offs_t address)
{
	return cpu_readop_arg(address);
}


/*-------------------------------------------------
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

INLINE void update_regptr(void)
{
	mcs48.regptr = memory_get_write_ptr(cpu_getactivecpu(), ADDRESS_SPACE_DATA, (PSW & B_FLAG) ? 24 : 0);
}


/*-------------------------------------------------
    push_pc_psw - push the PC and PSW values onto
    the stack
-------------------------------------------------*/

INLINE void push_pc_psw(void)
{
	UINT8 sp = PSW & 0x07;
	ram_w(8 + 2*sp, mcs48.pc.b.l);
	ram_w(9 + 2*sp, (mcs48.pc.b.h & 0x0f) | (PSW & 0xf0));
	PSW = (PSW & 0xf8) | ((sp + 1) & 0x07);
}


/*-------------------------------------------------
    pull_pc_psw - pull the PC and PSW values from
    the stack
-------------------------------------------------*/

INLINE void pull_pc_psw(void)
{
	UINT8 sp = (PSW - 1) & 0x07;
	mcs48.pc.b.l = ram_r(8 + 2*sp);
	mcs48.pc.b.h = ram_r(9 + 2*sp);
	PSW = (mcs48.pc.b.h & 0xf0) | 0x08 | sp;
	mcs48.pc.b.h &= 0x0f;
	update_regptr();
	change_pc(PC);
}


/*-------------------------------------------------
    pull_pc - pull the PC value from the stack,
    leaving the upper part of PSW intact
-------------------------------------------------*/

INLINE void pull_pc(void)
{
	UINT8 sp = (PSW - 1) & 0x07;
	mcs48.pc.b.l = ram_r(8 + 2*sp);
	mcs48.pc.b.h = ram_r(9 + 2*sp) & 0x0f;
	PSW = (PSW & 0xf0) | 0x08 | sp;
	change_pc(PC);
}


/*-------------------------------------------------
    execute_add - perform the logic of an ADD
    instruction
-------------------------------------------------*/

INLINE void execute_add(UINT8 dat)
{
	UINT16 temp = A + dat;

	PSW &= ~(C_FLAG | A_FLAG);
	PSW |= ((A ^ temp) << 2) & A_FLAG;
	PSW |= (temp >> 1) & C_FLAG;
	A = temp;
}


/*-------------------------------------------------
    execute_addc - perform the logic of an ADDC
    instruction
-------------------------------------------------*/

INLINE void execute_addc(UINT8 dat)
{
	UINT16 temp = A + dat + CARRYIN;

	PSW &= ~(C_FLAG | A_FLAG);
	PSW |= ((A ^ temp) << 2) & A_FLAG;
	PSW |= (temp >> 1) & C_FLAG;
	A = temp;
}


/*-------------------------------------------------
    execute_jmp - perform the logic of a JMP
    instruction
-------------------------------------------------*/

INLINE void execute_jmp(UINT16 address)
{
	UINT16 a11 = (mcs48.irq_in_progress) ? 0 : mcs48.a11;
	PC = address | a11;
	change_pc(PC);
}


/*-------------------------------------------------
    execute_call - perform the logic of a CALL
    instruction
-------------------------------------------------*/

INLINE void execute_call(UINT16 address)
{
	push_pc_psw();
	execute_jmp(address);
}


/*-------------------------------------------------
    execute_jcc - perform the logic of a
    conditional jump instruction
-------------------------------------------------*/

INLINE void execute_jcc(UINT8 result)
{
	UINT8 offset = argument_fetch(PC++);
	if (result != 0)
	{
		PC = ((PC - 1) & 0xf00) | offset;
		change_pc(PC);
	}
	else
		ADJUST_CYCLES;
}



/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

static void illegal(void)
{
	logerror("I8039:  pc = %04x,  Illegal opcode = %02x\n", PC-1, program_r(PC-1));
}

static void add_a_r0(void)		{ execute_add(R0); }
static void add_a_r1(void)		{ execute_add(R1); }
static void add_a_r2(void)		{ execute_add(R2); }
static void add_a_r3(void)		{ execute_add(R3); }
static void add_a_r4(void)		{ execute_add(R4); }
static void add_a_r5(void)		{ execute_add(R5); }
static void add_a_r6(void)		{ execute_add(R6); }
static void add_a_r7(void)		{ execute_add(R7); }
static void add_a_xr0(void)		{ execute_add(ram_r(R0)); }
static void add_a_xr1(void)		{ execute_add(ram_r(R1)); }
static void add_a_n(void)		{ execute_add(argument_fetch(PC++)); }

static void adc_a_r0(void)		{ execute_addc(R0); }
static void adc_a_r1(void)		{ execute_addc(R1); }
static void adc_a_r2(void)		{ execute_addc(R2); }
static void adc_a_r3(void)		{ execute_addc(R3); }
static void adc_a_r4(void)		{ execute_addc(R4); }
static void adc_a_r5(void)		{ execute_addc(R5); }
static void adc_a_r6(void)		{ execute_addc(R6); }
static void adc_a_r7(void)		{ execute_addc(R7); }
static void adc_a_xr0(void)		{ execute_addc(ram_r(R0)); }
static void adc_a_xr1(void)		{ execute_addc(ram_r(R1)); }
static void adc_a_n(void)		{ execute_addc(argument_fetch(PC++)); }

static void anl_a_r0(void)		{ A &= R0; }
static void anl_a_r1(void)		{ A &= R1; }
static void anl_a_r2(void)		{ A &= R2; }
static void anl_a_r3(void)		{ A &= R3; }
static void anl_a_r4(void)		{ A &= R4; }
static void anl_a_r5(void)		{ A &= R5; }
static void anl_a_r6(void)		{ A &= R6; }
static void anl_a_r7(void)		{ A &= R7; }
static void anl_a_xr0(void)		{ A &= ram_r(R0); }
static void anl_a_xr1(void)		{ A &= ram_r(R1); }
static void anl_a_n(void)		{ A &= argument_fetch(PC++); }
static void anl_bus_n(void)		{ bus_w(bus_r() & argument_fetch(PC++)); }
static void anl_p1_n(void)		{ port_w(1, mcs48.p1 &= argument_fetch(PC++)); }
static void anl_p2_n(void)		{ port_w(2, mcs48.p2 &= argument_fetch(PC++)); }

static void anld_p4_a(void)		{ port_w(4, port_r(4) & A & 0x0f); }
static void anld_p5_a(void)		{ port_w(5, port_r(5) & A & 0x0f); }
static void anld_p6_a(void)		{ port_w(6, port_r(6) & A & 0x0f); }
static void anld_p7_a(void)		{ port_w(7, port_r(7) & A & 0x0f); }

static void call_0(void)		{ execute_call(argument_fetch(PC++) | 0x000); }
static void call_1(void)		{ execute_call(argument_fetch(PC++) | 0x100); }
static void call_2(void)		{ execute_call(argument_fetch(PC++) | 0x200); }
static void call_3(void)		{ execute_call(argument_fetch(PC++) | 0x300); }
static void call_4(void)		{ execute_call(argument_fetch(PC++) | 0x400); }
static void call_5(void)		{ execute_call(argument_fetch(PC++) | 0x500); }
static void call_6(void)		{ execute_call(argument_fetch(PC++) | 0x600); }
static void call_7(void)		{ execute_call(argument_fetch(PC++) | 0x700); }

static void clr_a(void)			{ A = 0; }
static void clr_c(void)			{ PSW &= ~C_FLAG; }
static void clr_f0(void)		{ PSW &= ~F_FLAG; }
static void clr_f1(void)		{ mcs48.f1 = 0; }

static void cpl_a(void)			{ A ^= 0xff; }
static void cpl_c(void)			{ PSW ^= C_FLAG; }
static void cpl_f0(void)		{ PSW ^= F_FLAG; }
static void cpl_f1(void)		{ mcs48.f1 ^= 1; }

static void da_a(void)
{
	if ((A & 0x0f) > 0x09 || (PSW & A_FLAG))
	{
		A += 0x06;
		if ((A & 0xf0) == 0x00)
			PSW |= C_FLAG;
	}
	if ((A & 0xf0) > 0x90 || (PSW & C_FLAG))
	{
		A += 0x60;
		PSW |= C_FLAG;
	}
	else
		PSW &= ~C_FLAG;
}

static void dec_a(void)			{ A--; }
static void dec_r0(void)		{ R0--; }
static void dec_r1(void)		{ R1--; }
static void dec_r2(void)		{ R2--; }
static void dec_r3(void)		{ R3--; }
static void dec_r4(void)		{ R4--; }
static void dec_r5(void)		{ R5--; }
static void dec_r6(void)		{ R6--; }
static void dec_r7(void)		{ R7--; }

static void dis_i(void)			{ mcs48.xirq_enabled = FALSE; }
static void dis_tcnti(void)		{ mcs48.tirq_enabled = FALSE; mcs48.timer_overflow = FALSE; }

static void djnz_r0(void)		{ execute_jcc(--R0 != 0); }
static void djnz_r1(void)		{ execute_jcc(--R1 != 0); }
static void djnz_r2(void)		{ execute_jcc(--R2 != 0); }
static void djnz_r3(void)		{ execute_jcc(--R3 != 0); }
static void djnz_r4(void)		{ execute_jcc(--R4 != 0); }
static void djnz_r5(void)		{ execute_jcc(--R5 != 0); }
static void djnz_r6(void)		{ execute_jcc(--R6 != 0); }
static void djnz_r7(void)		{ execute_jcc(--R7 != 0); }

static void en_i(void)			{ mcs48.xirq_enabled = TRUE; check_irqs(); }
static void en_tcnti(void)		{ mcs48.tirq_enabled = TRUE; check_irqs(); }

static void ento_clk(void)
{
	logerror("I8039:  pc = %04x,  Unimplemented opcode = %02x\n", PC-1, program_r(PC-1));
}

static void in_a_p1(void)		{ A = port_r(1) & mcs48.p1; }
static void in_a_p2(void)		{ A = port_r(2) & mcs48.p2; }
static void ins_a_bus(void)		{ A = bus_r(); }

static void inc_a(void)			{ A++; }
static void inc_r0(void)		{ R0++; }
static void inc_r1(void)		{ R1++; }
static void inc_r2(void)		{ R2++; }
static void inc_r3(void)		{ R3++; }
static void inc_r4(void)		{ R4++; }
static void inc_r5(void)		{ R5++; }
static void inc_r6(void)		{ R6++; }
static void inc_r7(void)		{ R7++; }
static void inc_xr0(void)		{ ram_w(R0, ram_r(R0) + 1); }
static void inc_xr1(void)		{ ram_w(R1, ram_r(R1) + 1); }

static void jb_0(void)			{ execute_jcc((A & 0x01) != 0); }
static void jb_1(void)			{ execute_jcc((A & 0x02) != 0); }
static void jb_2(void)			{ execute_jcc((A & 0x04) != 0); }
static void jb_3(void)			{ execute_jcc((A & 0x08) != 0); }
static void jb_4(void)			{ execute_jcc((A & 0x10) != 0); }
static void jb_5(void)			{ execute_jcc((A & 0x20) != 0); }
static void jb_6(void)			{ execute_jcc((A & 0x40) != 0); }
static void jb_7(void)			{ execute_jcc((A & 0x80) != 0); }
static void jc(void)			{ execute_jcc((PSW & C_FLAG) != 0); }
static void jf0(void)			{ execute_jcc((PSW & F_FLAG) != 0); }
static void jf1(void)			{ execute_jcc(mcs48.f1 != 0); }

static void jmp_0(void)			{ execute_jmp(argument_fetch(PC) | 0x000); }
static void jmp_1(void)			{ execute_jmp(argument_fetch(PC) | 0x100); }
static void jmp_2(void)			{ execute_jmp(argument_fetch(PC) | 0x200); }
static void jmp_3(void)			{ execute_jmp(argument_fetch(PC) | 0x300); }
static void jmp_4(void)			{ execute_jmp(argument_fetch(PC) | 0x400); }
static void jmp_5(void)			{ execute_jmp(argument_fetch(PC) | 0x500); }
static void jmp_6(void)			{ execute_jmp(argument_fetch(PC) | 0x600); }
static void jmp_7(void)			{ execute_jmp(argument_fetch(PC) | 0x700); }
static void jmpp_xa(void)		{ PC &= 0xf00; PC |= program_r(PC | A); change_pc(PC); }

static void jnc(void)			{ execute_jcc((PSW & C_FLAG) == 0); }
static void jni(void)			{ execute_jcc(mcs48.irq_state != 0); }
static void jnt_0(void)  		{ execute_jcc(test_r(0) == 0); }
static void jnt_1(void)  		{ execute_jcc(test_r(1) == 0); }
static void jnz(void)			{ execute_jcc(A != 0); }
static void jtf(void)			{ execute_jcc(mcs48.timer_flag); mcs48.timer_flag = FALSE; }
static void jt_0(void)  		{ execute_jcc(test_r(0) != 0); }
static void jt_1(void)  		{ execute_jcc(test_r(1) != 0); }
static void jz(void)			{ execute_jcc(A == 0); }

static void mov_a_n(void)		{ A = argument_fetch(PC++); }
static void mov_a_psw(void)		{ A = PSW; }
static void mov_a_r0(void)		{ A = R0; }
static void mov_a_r1(void)		{ A = R1; }
static void mov_a_r2(void)		{ A = R2; }
static void mov_a_r3(void)		{ A = R3; }
static void mov_a_r4(void)		{ A = R4; }
static void mov_a_r5(void)		{ A = R5; }
static void mov_a_r6(void)		{ A = R6; }
static void mov_a_r7(void)		{ A = R7; }
static void mov_a_xr0(void)		{ A = ram_r(R0); }
static void mov_a_xr1(void)		{ A = ram_r(R1); }
static void mov_a_t(void)		{ A = mcs48.timer; }

static void mov_psw_a(void)		{ PSW = A; update_regptr(); }
static void mov_r0_a(void)		{ R0 = A; }
static void mov_r1_a(void)		{ R1 = A; }
static void mov_r2_a(void)		{ R2 = A; }
static void mov_r3_a(void)		{ R3 = A; }
static void mov_r4_a(void)		{ R4 = A; }
static void mov_r5_a(void)		{ R5 = A; }
static void mov_r6_a(void)		{ R6 = A; }
static void mov_r7_a(void)		{ R7 = A; }
static void mov_r0_n(void)		{ R0 = argument_fetch(PC++); }
static void mov_r1_n(void)		{ R1 = argument_fetch(PC++); }
static void mov_r2_n(void)		{ R2 = argument_fetch(PC++); }
static void mov_r3_n(void)		{ R3 = argument_fetch(PC++); }
static void mov_r4_n(void)		{ R4 = argument_fetch(PC++); }
static void mov_r5_n(void)		{ R5 = argument_fetch(PC++); }
static void mov_r6_n(void)		{ R6 = argument_fetch(PC++); }
static void mov_r7_n(void)		{ R7 = argument_fetch(PC++); }
static void mov_t_a(void)		{ mcs48.timer = A; }
static void mov_xr0_a(void)		{ ram_w(R0, A); }
static void mov_xr1_a(void)		{ ram_w(R1, A); }
static void mov_xr0_n(void)		{ ram_w(R0, argument_fetch(PC++)); }
static void mov_xr1_n(void)		{ ram_w(R1, argument_fetch(PC++)); }

static void movd_a_p4(void)		{ A = port_r(4) & 0x0f; }
static void movd_a_p5(void)		{ A = port_r(5) & 0x0f; }
static void movd_a_p6(void)		{ A = port_r(6) & 0x0f; }
static void movd_a_p7(void)		{ A = port_r(7) & 0x0f; }
static void movd_p4_a(void)		{ port_w(4, A & 0x0f); }
static void movd_p5_a(void)		{ port_w(5, A & 0x0f); }
static void movd_p6_a(void)		{ port_w(6, A & 0x0f); }
static void movd_p7_a(void)		{ port_w(7, A & 0x0f); }

static void movp_a_xa(void)		{ A = program_r((PC & 0xf00) | A); }
static void movp3_a_xa(void)	{ A = program_r(0x300 | A); }

static void movx_a_xr0(void)	{ A = ext_r(R0); }
static void movx_a_xr1(void)	{ A = ext_r(R1); }
static void movx_xr0_a(void)	{ ext_w(R0, A); }
static void movx_xr1_a(void)	{ ext_w(R1, A); }

static void nop(void)			{ }

static void orl_a_r0(void)		{ A |= R0; }
static void orl_a_r1(void)		{ A |= R1; }
static void orl_a_r2(void)		{ A |= R2; }
static void orl_a_r3(void)		{ A |= R3; }
static void orl_a_r4(void)		{ A |= R4; }
static void orl_a_r5(void)		{ A |= R5; }
static void orl_a_r6(void)		{ A |= R6; }
static void orl_a_r7(void)		{ A |= R7; }
static void orl_a_xr0(void)		{ A |= ram_r(R0); }
static void orl_a_xr1(void)		{ A |= ram_r(R1); }
static void orl_a_n(void)		{ A |= argument_fetch(PC++); }
static void orl_bus_n(void)		{ bus_w(bus_r() | argument_fetch(PC++)); }
static void orl_p1_n(void)		{ port_w(1, mcs48.p1 |= argument_fetch(PC++)); }
static void orl_p2_n(void)		{ port_w(2, mcs48.p2 |= argument_fetch(PC++)); }
static void orld_p4_a(void)		{ port_w(4, port_r(4) | A); }
static void orld_p5_a(void)		{ port_w(5, port_r(5) | A); }
static void orld_p6_a(void)		{ port_w(6, port_r(6) | A); }
static void orld_p7_a(void)		{ port_w(7, port_r(7) | A); }

static void outl_bus_a(void)	{ bus_w(A); }
static void outl_p1_a(void)		{ port_w(1, mcs48.p1 = A); }
static void outl_p2_a(void)		{ port_w(2, mcs48.p2 = A); }
static void ret(void)			{ pull_pc(); }

static void retr(void)
{
	pull_pc_psw();

	/* implicitly clear the IRQ in progress flip flop and re-check interrupts */
	mcs48.irq_in_progress = FALSE;
	check_irqs();
}

static void rl_a(void)			{ A = (A << 1) | (A >> 7); }
static void rlc_a(void) 		{ UINT8 newc = A & C_FLAG; A = (A << 1) | (PSW >> 7); PSW = (PSW & ~C_FLAG) | newc; }

static void rr_a(void)			{ A = (A >> 1) | (A << 7); }
static void rrc_a(void)			{ UINT8 newc = (A << 7) & C_FLAG; A = (A >> 1) | (PSW & C_FLAG); PSW = (PSW & ~C_FLAG) | newc; }

static void sel_mb0(void)		{ mcs48.a11 = 0x000; }
static void sel_mb1(void)		{ mcs48.a11 = 0x800; }

static void sel_rb0(void)		{ PSW &= ~B_FLAG; update_regptr();  }
static void sel_rb1(void)		{ PSW |=  B_FLAG; update_regptr(); }

static void stop_tcnt(void)		{ mcs48.timecount_enabled = 0; }

static void strt_cnt(void)		{ mcs48.timecount_enabled = COUNTER_ENABLED; mcs48.t1_history = test_r(1); }
static void strt_t(void)		{ mcs48.timecount_enabled = TIMER_ENABLED; mcs48.prescaler = 0; }

static void swap_a(void)		{ A = (A << 4) | (A >> 4); }

static void xch_a_r0(void)		{ UINT8 tmp = A; A = R0; R0 = tmp; }
static void xch_a_r1(void)		{ UINT8 tmp = A; A = R1; R1 = tmp; }
static void xch_a_r2(void)		{ UINT8 tmp = A; A = R2; R2 = tmp; }
static void xch_a_r3(void)		{ UINT8 tmp = A; A = R3; R3 = tmp; }
static void xch_a_r4(void)		{ UINT8 tmp = A; A = R4; R4 = tmp; }
static void xch_a_r5(void)		{ UINT8 tmp = A; A = R5; R5 = tmp; }
static void xch_a_r6(void)		{ UINT8 tmp = A; A = R6; R6 = tmp; }
static void xch_a_r7(void)		{ UINT8 tmp = A; A = R7; R7 = tmp; }
static void xch_a_xr0(void)		{ UINT8 tmp = A; A = ram_r(R0); ram_w(R0, tmp); }
static void xch_a_xr1(void)		{ UINT8 tmp = A; A = ram_r(R1); ram_w(R1, tmp); }

static void xchd_a_xr0(void)	{ UINT8 oldram = ram_r(R0); ram_w(R0, (oldram & 0xf0) | (A & 0x0f)); A = (A & 0xf0) | (oldram & 0x0f); }
static void xchd_a_xr1(void)	{ UINT8 oldram = ram_r(R1); ram_w(R1, (oldram & 0xf0) | (A & 0x0f)); A = (A & 0xf0) | (oldram & 0x0f); }

static void xrl_a_r0(void)		{ A ^= R0; }
static void xrl_a_r1(void)		{ A ^= R1; }
static void xrl_a_r2(void)		{ A ^= R2; }
static void xrl_a_r3(void)		{ A ^= R3; }
static void xrl_a_r4(void)		{ A ^= R4; }
static void xrl_a_r5(void)		{ A ^= R5; }
static void xrl_a_r6(void)		{ A ^= R6; }
static void xrl_a_r7(void)		{ A ^= R7; }
static void xrl_a_xr0(void)		{ A ^= ram_r(R0); }
static void xrl_a_xr1(void)		{ A ^= ram_r(R1); }
static void xrl_a_n(void)		{ A ^= argument_fetch(PC++); }



/***************************************************************************
    OPCODE TABLES
***************************************************************************/

static const mcs48_opcode opcode_table[256]=
{
	{1, nop	 	   },{1, illegal	},{2, outl_bus_a},{2, add_a_n	},{2, jmp_0	   	},{1, en_i		},{1, illegal	},{1, dec_a		},
	{2, ins_a_bus  },{2, in_a_p1	},{2, in_a_p2	},{1, illegal	},{2, movd_a_p4 },{2, movd_a_p5	},{2, movd_a_p6	},{2, movd_a_p7 },
	{1, inc_xr0    },{1, inc_xr1	},{2, jb_0		},{2, adc_a_n	},{2, call_0   	},{1, dis_i		},{2, jtf		},{1, inc_a		},
	{1, inc_r0	   },{1, inc_r1 	},{1, inc_r2	},{1, inc_r3	},{1, inc_r4	},{1, inc_r5 	},{1, inc_r6	},{1, inc_r7	},
	{1, xch_a_xr0  },{1, xch_a_xr1	},{1, illegal	},{2, mov_a_n	},{2, jmp_1	   	},{1, en_tcnti	},{2, jnt_0 	},{1, clr_a	  	},
	{1, xch_a_r0   },{1, xch_a_r1	},{1, xch_a_r2	},{1, xch_a_r3  },{1, xch_a_r4  },{1, xch_a_r5	},{1, xch_a_r6	},{1, xch_a_r7  },
	{1, xchd_a_xr0 },{1, xchd_a_xr1	},{2, jb_1		},{1, illegal	},{2, call_1	},{1, dis_tcnti	},{2, jt_0		},{1, cpl_a	  	},
	{0, illegal    },{2, outl_p1_a	},{2, outl_p2_a	},{1, illegal	},{2, movd_p4_a },{2, movd_p5_a	},{2, movd_p6_a },{2, movd_p7_a },
	{1, orl_a_xr0  },{1, orl_a_xr1	},{1, mov_a_t	},{2, orl_a_n	},{2, jmp_2	   	},{1, strt_cnt	},{2, jnt_1 	},{1, swap_a	},
	{1, orl_a_r0   },{1, orl_a_r1	},{1, orl_a_r2	},{1, orl_a_r3  },{1, orl_a_r4  },{1, orl_a_r5	},{1, orl_a_r6	},{1, orl_a_r7  },
	{1, anl_a_xr0  },{1, anl_a_xr1	},{2, jb_2		},{2, anl_a_n	},{2, call_2	},{1, strt_t 	},{2, jt_1		},{1, da_a	  	},
	{1, anl_a_r0   },{1, anl_a_r1	},{1, anl_a_r2	},{1, anl_a_r3  },{1, anl_a_r4  },{1, anl_a_r5	},{1, anl_a_r6	},{1, anl_a_r7  },
	{1, add_a_xr0  },{1, add_a_xr1	},{1, mov_t_a	},{1, illegal	},{2, jmp_3	   	},{1, stop_tcnt	},{1, illegal	},{1, rrc_a	  	},
	{1, add_a_r0   },{1, add_a_r1	},{1, add_a_r2	},{1, add_a_r3  },{1, add_a_r4  },{1, add_a_r5	},{1, add_a_r6	},{1, add_a_r7  },
	{1, adc_a_xr0  },{1, adc_a_xr1	},{2, jb_3		},{1, illegal	},{2, call_3	},{1, ento_clk	},{2, jf1		},{1, rr_a 	  	},
	{1, adc_a_r0   },{1, adc_a_r1	},{1, adc_a_r2	},{1, adc_a_r3  },{1, adc_a_r4  },{1, adc_a_r5	},{1, adc_a_r6	},{1, adc_a_r7  },
	{2, movx_a_xr0 },{2, movx_a_xr1 },{1, illegal	},{2, ret		},{2, jmp_4	   	},{1, clr_f0 	},{2, jni		},{1, illegal	},
	{2, orl_bus_n  },{2, orl_p1_n	},{2, orl_p2_n	},{1, illegal	},{2, orld_p4_a },{2, orld_p5_a	},{2, orld_p6_a },{2, orld_p7_a },
	{2, movx_xr0_a },{2, movx_xr1_a },{2, jb_4		},{2, retr		},{2, call_4	},{1, cpl_f0 	},{2, jnz		},{1, clr_c	  	},
	{2, anl_bus_n  },{2, anl_p1_n	},{2, anl_p2_n	},{1, illegal	},{2, anld_p4_a },{2, anld_p5_a	},{2, anld_p6_a },{2, anld_p7_a },
	{1, mov_xr0_a  },{1, mov_xr1_a	},{1, illegal	},{2, movp_a_xa },{2, jmp_5	   	},{1, clr_f1 	},{1, illegal	},{1, cpl_c	  	},
	{1, mov_r0_a   },{1, mov_r1_a	},{1, mov_r2_a	},{1, mov_r3_a  },{1, mov_r4_a  },{1, mov_r5_a	},{1, mov_r6_a	},{1, mov_r7_a  },
	{2, mov_xr0_n  },{2, mov_xr1_n	},{2, jb_5		},{2, jmpp_xa	},{2, call_5	},{1, cpl_f1 	},{2, jf0		},{1, illegal	},
	{2, mov_r0_n   },{2, mov_r1_n	},{2, mov_r2_n	},{2, mov_r3_n  },{2, mov_r4_n  },{2, mov_r5_n	},{2, mov_r6_n	},{2, mov_r7_n  },
	{0, illegal    },{1, illegal	},{1, illegal	},{1, illegal	},{2, jmp_6	   	},{1, sel_rb0	},{2, jz		},{1, mov_a_psw },
	{1, dec_r0	   },{1, dec_r1 	},{1, dec_r2	},{1, dec_r3	},{1, dec_r4	},{1, dec_r5 	},{1, dec_r6	},{1, dec_r7	},
	{1, xrl_a_xr0  },{1, xrl_a_xr1	},{2, jb_6		},{2, xrl_a_n	},{2, call_6	},{1, sel_rb1	},{1, illegal	},{1, mov_psw_a },
	{1, xrl_a_r0   },{1, xrl_a_r1	},{1, xrl_a_r2	},{1, xrl_a_r3  },{1, xrl_a_r4  },{1, xrl_a_r5	},{1, xrl_a_r6	},{1, xrl_a_r7  },
	{0, illegal    },{1, illegal	},{1, illegal	},{2, movp3_a_xa},{2, jmp_7		},{1, sel_mb0	},{2, jnc		},{1, rl_a 	  	},
	{2, djnz_r0    },{2, djnz_r1	},{2, djnz_r2	},{2, djnz_r3	},{2, djnz_r4   },{2, djnz_r5	},{2, djnz_r6	},{2, djnz_r7	},
	{1, mov_a_xr0  },{1, mov_a_xr1	},{2, jb_7		},{1, illegal	},{2, call_7	},{1, sel_mb1	},{2, jc		},{1, rlc_a	  	},
	{1, mov_a_r0   },{1, mov_a_r1	},{1, mov_a_r2	},{1, mov_a_r3  },{1, mov_a_r4  },{1, mov_a_r5	},{1, mov_a_r6	},{1, mov_a_r7  }
};



/***************************************************************************
    INITIALIZATION/RESET
***************************************************************************/

/*-------------------------------------------------
    mcs48_init - generic MCS-48 initialization
-------------------------------------------------*/

static void mcs48_init(int index, int clock, const void *config, int (*irqcallback)(int), UINT16 romsize)
{
	memset(&mcs48, 0, sizeof(mcs48));
	
	/* External access line
	 * EA=1 : read from external rom
	 * EA=0 : read from internal rom
	 */
	
	/* FIXME: Current implementation suboptimal */
	mcs48.ea = (romsize ? 0 : 1);

	mcs48.irq_callback = irqcallback;
	mcs48.int_rom_size = romsize;

	state_save_register_item("mcs48", index, mcs48.prevpc.w.l);
	state_save_register_item("mcs48", index, PC);
	state_save_register_item("mcs48", index, A);
	state_save_register_item("mcs48", index, PSW);
	state_save_register_item("mcs48", index, mcs48.p1);
	state_save_register_item("mcs48", index, mcs48.p2);
	state_save_register_item("mcs48", index, mcs48.f1);
	state_save_register_item("mcs48", index, mcs48.ea);
	state_save_register_item("mcs48", index, mcs48.timer);
	state_save_register_item("mcs48", index, mcs48.prescaler);
	state_save_register_item("mcs48", index, mcs48.t1_history);
	state_save_register_item("mcs48", index, mcs48.irq_state);
	state_save_register_item("mcs48", index, mcs48.irq_in_progress);
	state_save_register_item("mcs48", index, mcs48.timer_overflow);
	state_save_register_item("mcs48", index, mcs48.timer_flag);
	state_save_register_item("mcs48", index, mcs48.tirq_enabled);
	state_save_register_item("mcs48", index, mcs48.xirq_enabled);
	state_save_register_item("mcs48", index, mcs48.timecount_enabled);
	state_save_register_item("mcs48", index, mcs48.a11);
}


/*-------------------------------------------------
    i8035_init - initialization for systems with
    0k of internal ROM and 64 bytes of internal
    RAM
-------------------------------------------------*/

#if (HAS_I8035 || HAS_MB8884)
static void i8035_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs48_init(index, clock, config, irqcallback, 0x0);
}
#endif


/*-------------------------------------------------
    i8041_init - initialization for systems with
    1k of internal ROM and 64 bytes of internal
    RAM
-------------------------------------------------*/

#if (HAS_I8041 || HAS_I8048 || HAS_I8648 || HAS_I8748 || HAS_N7751)
static void i8041_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs48_init(index, clock, config, irqcallback, 0x400);
}
#endif


/*-------------------------------------------------
    i8039_init - initialization for systems with
    0k of internal ROM and 128 bytes of internal
    RAM
-------------------------------------------------*/

#if (HAS_I8039)
static void i8039_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs48_init(index, clock, config, irqcallback, 0x0);
}
#endif


/*-------------------------------------------------
    i8049_init - initialization for systems with
    2k of internal ROM and 128 bytes of internal
    RAM
-------------------------------------------------*/

#if (HAS_I8049 || HAS_I8749 || HAS_M58715)
static void i8049_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mcs48_init(index, clock, config, irqcallback, 0x800);
}
#endif


/*-------------------------------------------------
    mcs48_reset - general reset routine
-------------------------------------------------*/

static void mcs48_reset(void)
{
	/* confirmed from reset description */
	PC = 0;
	PSW = (PSW & (C_FLAG | A_FLAG)) | 0x08;
	mcs48.a11 = 0x000;
	bus_w(0xff);
	mcs48.p1 = 0xff;
	mcs48.p2 = 0xff;
	port_w(1, mcs48.p1);
	port_w(2, mcs48.p2);
	mcs48.tirq_enabled = FALSE;
	mcs48.xirq_enabled = FALSE;
	mcs48.timecount_enabled = 0;
	mcs48.timer_flag = FALSE;
	mcs48.f1 = 0;

	/* confirmed from interrupt logic description */
	mcs48.irq_in_progress = FALSE;
	mcs48.timer_overflow = FALSE;
}



/***************************************************************************
    EXECUTION
***************************************************************************/

/*-------------------------------------------------
    check_irqs - check for and process IRQs
-------------------------------------------------*/

static void check_irqs(void)
{
	/* if something is in progress, we do nothing */
	if (mcs48.irq_in_progress)
		return;

	/* external interrupts take priority */
	if (mcs48.irq_state && mcs48.xirq_enabled)
	{
		mcs48.irq_in_progress = TRUE;

		/* transfer to location 0x03 */
		push_pc_psw();
		PC = 0x03;
		change_pc(0x03);
		mcs48.inst_cycles += 2;

		/* indicate we took the external IRQ */
		if (mcs48.irq_callback != NULL)
			(*mcs48.irq_callback)(0);
	}

	/* timer overflow interrupts follow */
	if (mcs48.timer_overflow && mcs48.tirq_enabled)
	{
		mcs48.irq_in_progress = TRUE;

		/* transfer to location 0x07 */
		push_pc_psw();
		PC = 0x07;
		change_pc(0x07);
		mcs48.inst_cycles += 2;

		/* timer overflow flip-flop is reset once taken */
		mcs48.timer_overflow = FALSE;
	}
}


/*-------------------------------------------------
    burn_cycles - burn cycles, processing timers
    and counters
-------------------------------------------------*/

static void burn_cycles(int count)
{
	int timerover = FALSE;

	/* if the timer is enabled, accumulate prescaler cycles */
	if (mcs48.timecount_enabled & TIMER_ENABLED)
	{
		UINT8 oldtimer = mcs48.timer;
		mcs48.prescaler += count;
		mcs48.timer += mcs48.prescaler >> 5;
		mcs48.prescaler &= 0x1f;
		timerover = (oldtimer != 0 && mcs48.timer == 0);
	}

	/* if the counter is enabled, poll the T1 test input once for each cycle */
	else if (mcs48.timecount_enabled & COUNTER_ENABLED)
		for ( ; count > 0; count--)
		{
			mcs48.t1_history = (mcs48.t1_history << 1) | (test_r(1) & 1);
			if ((mcs48.t1_history & 3) == 2)
				timerover = (++mcs48.timer == 0);
		}

	/* if either source caused a timer overflow, set the flags and check IRQs */
	if (timerover)
	{
		mcs48.timer_flag = TRUE;

		/* according to the docs, if an overflow occurs with interrupts disabled, the overflow is not stored */
		if (mcs48.tirq_enabled)
		{
			mcs48.timer_overflow = TRUE;
			check_irqs();
		}
	}
}


/*-------------------------------------------------
    mcs48_execute - execute until we run out
    of cycles
-------------------------------------------------*/

static int mcs48_execute(int cycles)
{
	unsigned opcode;

	mcs48_icount = cycles;

	/* external interrupts may have been set since we last checked */
	mcs48.inst_cycles = 0;
	check_irqs();
	mcs48_icount -= mcs48.inst_cycles;
	if (mcs48.timecount_enabled != 0)
		burn_cycles(mcs48.inst_cycles);

	/* iterate over remaining cycles, guaranteeing at least one instruction */
	do
	{
		/* fetch next opcode */
		mcs48.prevpc = mcs48.pc;
		debugger_instruction_hook(Machine, PC);
		opcode = opcode_fetch(PC++);

		/* process opcode and count cycles */
		mcs48.inst_cycles = opcode_table[opcode].cycles;
		(*opcode_table[opcode].function)();

		/* burn the cycles */
		mcs48_icount -= mcs48.inst_cycles;
		if (mcs48.timecount_enabled != 0)
			burn_cycles(mcs48.inst_cycles);

	} while (mcs48_icount > 0);

	return cycles - mcs48_icount;
}



/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

/* FIXME: the memory maps should probably support rom banking for EA */
static ADDRESS_MAP_START(program_10bit, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00, 0x3ff) AM_ROMBANK(MCS48_INTERNAL_ROMBANK)
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_11bit, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00, 0x7ff) AM_ROMBANK(MCS48_INTERNAL_ROMBANK)
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_6bit, ADDRESS_SPACE_DATA, 8)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_7bit, ADDRESS_SPACE_DATA, 8)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END



/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

/*-------------------------------------------------
    mcs48_get_context - copy the context to the
    destination
-------------------------------------------------*/

static void mcs48_get_context(void *dst)
{
	if (dst != NULL)
		*(mcs48_regs *)dst = mcs48;
}


/*-------------------------------------------------
    mcs48_set_context - set the current context
    from the source
-------------------------------------------------*/

static void mcs48_set_context(void *src)
{
	if (src != NULL)
	{
		mcs48 = *(mcs48_regs *)src;
		update_regptr();
		change_pc(PC);
	}
}


/*-------------------------------------------------
    mcs48_set_info - set a piece of information
    on the CPU core
-------------------------------------------------*/

static void mcs48_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_IRQ:	mcs48.irq_state = (info->i != CLEAR_LINE);	break;
		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_EA:	mcs48.ea = (info->i != CLEAR_LINE);
														memory_set_bank(MCS48_INTERNAL_ROMBANK, mcs48.ea);
														break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + MCS48_PC:			PC = info->i;								break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + MCS48_PSW:			PSW = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_A:			A = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_TC:			mcs48.timer = info->i;						break;
		case CPUINFO_INT_REGISTER + MCS48_P1:			mcs48.p1 = info->i;							break;
		case CPUINFO_INT_REGISTER + MCS48_P2:			mcs48.p2 = info->i;							break;
		case CPUINFO_INT_REGISTER + MCS48_R0:			R0 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R1:			R1 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R2:			R2 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R3:			R3 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R4:			R4 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R5:			R5 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R6:			R6 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_R7:			R7 = info->i;								break;
		case CPUINFO_INT_REGISTER + MCS48_EA:			mcs48.ea = info->i;							break;
	}
}


/*-------------------------------------------------
    mcs48_get_info - retrieve a piece of
    information from the CPU core
-------------------------------------------------*/

static void mcs48_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:							info->i = sizeof(mcs48);				break;
		case CPUINFO_INT_INPUT_LINES:							info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:					info->i = MCS48_INPUT_IRQ;				break;
		case CPUINFO_INT_ENDIANNESS:							info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:						info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:							info->i = 3*5;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:					info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:							info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:							info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: /*info->i = 10 or 11 or 12;*/			break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	/*info->i = 6 or 7;*/					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 9;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;							break;

		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_IRQ:			info->i = mcs48.irq_state ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + MCS48_INPUT_EA:			info->i = mcs48.ea;						break;

		case CPUINFO_INT_PREVIOUSPC:							info->i = mcs48.prevpc.w.l;				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + MCS48_PC:					info->i = PC;							break;
		case CPUINFO_INT_REGISTER + MCS48_PSW:					info->i = PSW;							break;
		case CPUINFO_INT_REGISTER + MCS48_A:					info->i = A;							break;
		case CPUINFO_INT_REGISTER + MCS48_TC:					info->i = mcs48.timer;					break;
		case CPUINFO_INT_REGISTER + MCS48_P1:					info->i = mcs48.p1;						break;
		case CPUINFO_INT_REGISTER + MCS48_P2:					info->i = mcs48.p2;						break;
		case CPUINFO_INT_REGISTER + MCS48_R0:					info->i = R0;							break;
		case CPUINFO_INT_REGISTER + MCS48_R1:					info->i = R1;							break;
		case CPUINFO_INT_REGISTER + MCS48_R2:					info->i = R2;							break;
		case CPUINFO_INT_REGISTER + MCS48_R3:					info->i = R3;							break;
		case CPUINFO_INT_REGISTER + MCS48_R4:					info->i = R4;							break;
		case CPUINFO_INT_REGISTER + MCS48_R5:					info->i = R5;							break;
		case CPUINFO_INT_REGISTER + MCS48_R6:					info->i = R6;							break;
		case CPUINFO_INT_REGISTER + MCS48_R7:					info->i = R7;							break;
		case CPUINFO_INT_REGISTER + MCS48_EA:					info->i = mcs48.ea;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:								info->setinfo = mcs48_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:							info->getcontext = mcs48_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:							info->setcontext = mcs48_set_context;	break;
		case CPUINFO_PTR_INIT:									/*info->init = i8039_init;*/			break;
		case CPUINFO_PTR_RESET:									info->reset = mcs48_reset;				break;
		case CPUINFO_PTR_EXECUTE:								info->execute = mcs48_execute;			break;
		case CPUINFO_PTR_BURN:									info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:							info->disassemble = mcs48_dasm;			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:					info->icount = &mcs48_icount;			break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: /*info->internal_map8 = address_map_program_10bit;*/ break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 	/*info->internal_map8 = address_map_data_7bit;*/ break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:									/*strcpy(info->s, "I8039");*/			break;
		case CPUINFO_STR_CORE_FAMILY:							strcpy(info->s, "Intel 8039");			break;
		case CPUINFO_STR_CORE_VERSION:							strcpy(info->s, "1.2");					break;
		case CPUINFO_STR_CORE_FILE:								strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:							strcpy(info->s, "Copyright Mirko Buffoni\nBased on the original work Copyright Dan Boris"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c %c%c%c%c%c%c%c%c",
				mcs48.a11        ? 'M':'.',
				PSW & 0x80 ? 'C':'.',
				PSW & 0x40 ? 'a':'.',
				PSW & 0x20 ? 'F':'.',
				PSW & 0x10 ? 'B':'.',
				PSW & 0x08 ? '?':'.',
				PSW & 0x04 ? '4':'.',
				PSW & 0x02 ? '2':'.',
				PSW & 0x01 ? '1':'.');
			break;

		case CPUINFO_STR_REGISTER + MCS48_PC:					sprintf(info->s, "PC:%04X", PC); 		break;
		case CPUINFO_STR_REGISTER + MCS48_PSW:					sprintf(info->s, "PSW:%02X", PSW); 		break;
		case CPUINFO_STR_REGISTER + MCS48_A:					sprintf(info->s, "A:%02X", A); 			break;
		case CPUINFO_STR_REGISTER + MCS48_TC:					sprintf(info->s, "TC:%02X", mcs48.timer); break;
		case CPUINFO_STR_REGISTER + MCS48_P1:					sprintf(info->s, "P1:%02X", mcs48.p1); 	break;
		case CPUINFO_STR_REGISTER + MCS48_P2:					sprintf(info->s, "P2:%02X", mcs48.p2); 	break;
		case CPUINFO_STR_REGISTER + MCS48_R0:					sprintf(info->s, "R0:%02X", R0);		break;
		case CPUINFO_STR_REGISTER + MCS48_R1:					sprintf(info->s, "R1:%02X", R1); 		break;
		case CPUINFO_STR_REGISTER + MCS48_R2:					sprintf(info->s, "R2:%02X", R2); 		break;
		case CPUINFO_STR_REGISTER + MCS48_R3:					sprintf(info->s, "R3:%02X", R3); 		break;
		case CPUINFO_STR_REGISTER + MCS48_R4:					sprintf(info->s, "R4:%02X", R4); 		break;
		case CPUINFO_STR_REGISTER + MCS48_R5:					sprintf(info->s, "R5:%02X", R5); 		break;
		case CPUINFO_STR_REGISTER + MCS48_R6:					sprintf(info->s, "R6:%02X", R6); 		break;
		case CPUINFO_STR_REGISTER + MCS48_R7:					sprintf(info->s, "R7:%02X", R7); 		break;
		case CPUINFO_STR_REGISTER + MCS48_EA:					sprintf(info->s, "EA:%02X", mcs48.ea); 	break;
	}
}



/***************************************************************************
    CPU-SPECIFIC CONTEXT ACCESS
***************************************************************************/

#if (HAS_I8035)
void i8035_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 12;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8035_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8035");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_I8041)
void i8041_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 10;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_10bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8041_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8041");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_I8048)
void i8048_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 10;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_10bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8041_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8048");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_I8648)
void i8648_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 10;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_10bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8041_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8648");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_I8748)
void i8748_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 10;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_10bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8041_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8748");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_MB8884)
void mb8884_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 12;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8035_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "MB8884");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_N7751)
void n7751_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 10;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 6;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_10bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_6bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8041_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "N7751");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif



#if (HAS_I8039)
void i8039_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 12;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 7;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_7bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8039_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8039");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_I8049)
void i8049_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 11;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 7;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_11bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_7bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8049_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8049");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_I8749)
void i8749_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 11;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 7;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_11bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_7bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8049_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8749");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif

#if (HAS_M58715)
void m58715_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 12;										break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 			info->i = 7;										break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = address_map_program_11bit;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = address_map_data_7bit;		break;
		case CPUINFO_PTR_INIT:											info->init = i8049_init;							break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "M58715");							break;
		default:														mcs48_get_info(state, info);						break;
	}
}
#endif
