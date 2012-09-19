/*****************************************************************************
 *
 *   f8.c
 *   Portable F8 emulator (Fairchild 3850)
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on Frank Palazzolo's F8 emulation in a standalone
 *  Fairchild Channel F emulator and the 'Fairchild F3850 CPU' data sheets.
 *
 *****************************************************************************/

/* PeT 25.June 2001
   added interrupt functionality
 */

#include "emu.h"
#include "debugger.h"
#include "f8.h"

#define S	0x01
#define C	0x02
#define Z	0x04
#define O	0x08
#define I	0x10

#define cS  4
#define cL	6

struct f8_Regs
{
	UINT16	pc0;	/* program counter 0 */
	UINT16	pc1;	/* program counter 1 */
	UINT16	dc0;	/* data counter 0 */
	UINT16	dc1;	/* data counter 1 */
	UINT8	a;		/* accumulator */
	UINT8	w;		/* processor status */
	UINT8	is; 	/* scratchpad pointer */
	UINT8	dbus;	/* data bus value */
	UINT16	io; 	/* last I/O address */
	UINT16  irq_vector;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *iospace;
	int icount;
	UINT8   r[64];  /* scratchpad RAM */
	int     irq_request;
};

INLINE f8_Regs *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == F8);
	return (f8_Regs *)downcast<legacy_cpu_device *>(device)->token();
}

/* timer shifter polynome values (will be used for timer interrupts) */
static UINT8 timer_shifter[256];

/* clear all flags */
#define CLR_OZCS                \
	cpustate->w &= ~(O|Z|C|S)

/* set sign and zero flags (note: the S flag is complementary) */
#define SET_SZ(n)               \
	if (n == 0) 				\
		cpustate->w |= Z | S;			\
	else						\
	if (n < 128)				\
		cpustate->w |= S

/* set overflow and carry flags */
#define SET_OC(n,m)             \
	if (n + m > 255)			\
		cpustate->w |= C;				\
	if ((n&127)+(m&127) > 127)	\
	{							\
		if (!(cpustate->w & C))		\
			cpustate->w |= O;			\
	}							\
	else						\
	{							\
		if (cpustate->w & C)			\
			cpustate->w |= O;			\
	}

/******************************************************************************
 * ROMC (ROM cycles)
 * This is what the Fairchild F8 CPUs use instead of an address bus
 * There are 5 control lines and each combination of those lines has
 * a special meaning. The devices attached to those control lines all
 * have their own program counters (PC0 and PC1) and at least one
 * data counter (DC0).
 * Currently the emulation does not handle distinct PCs and DCs, but
 * only one instance inside the CPU context.
 ******************************************************************************/
static void ROMC_00(f8_Regs *cpustate, int insttim)	/* SKR - added parameter to tell if  */
                                    /* it is long or short based on inst */
{
    /*
     * Instruction Fetch. The device whose address space includes the
     * contents of the PC0 register must place on the data bus the op
     * code addressed by PC0; then all devices increment the contents
     * of PC0.
     */

	cpustate->dbus = cpustate->direct->read_decrypted_byte(cpustate->pc0);
    cpustate->pc0 += 1;
    cpustate->icount -= insttim;	/* SKR - ROMC00 is usually short, not short+long, */
                            /* but DS is long */
}

static void ROMC_01(f8_Regs *cpustate)
{
    /*
     * The device whose address space includes the contents of the PC0
     * register must place on the data bus the contents of the memory
     * location addressed by PC0; then all devices add the 8-bit value
     * on the data bus as signed binary number to PC0.
     */
	cpustate->dbus = cpustate->direct->read_raw_byte(cpustate->pc0);
	cpustate->pc0 += (INT8)cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_02(f8_Regs *cpustate)
{
    /*
     * The device whose DC0 addresses a memory word within the address
     * space of that device must place on the data bus the contents of
     * the memory location addressed by DC0; then all devices increment
     * DC0.
     */
    cpustate->dbus = cpustate->program->read_byte(cpustate->dc0);
    cpustate->dc0 += 1;
    cpustate->icount -= cL;
}

static void ROMC_03(f8_Regs *cpustate, int insttim)	/* SKR - added parameter to tell if  */
{                                   /* it is long or short based on inst */
    /*
     * Similiar to 0x00, except that it is used for immediate operands
     * fetches (using PC0) instead of instruction fetches.
     */
    cpustate->dbus = cpustate->io = cpustate->direct->read_raw_byte(cpustate->pc0);
    cpustate->pc0 += 1;
    cpustate->icount -= insttim;
}

static void ROMC_04(f8_Regs *cpustate)
{
    /*
     * Copy the contents of PC1 into PC0
     */
    cpustate->pc0 = cpustate->pc1;
    cpustate->icount -= cS;
}

static void ROMC_05(f8_Regs *cpustate)
{
    /*
     * Store the data bus contents into the memory location pointed
     * to by DC0; increment DC0.
     */
    cpustate->program->write_byte(cpustate->dc0, cpustate->dbus);
    cpustate->dc0 += 1;
    cpustate->icount -= cL;
}

static void ROMC_06(f8_Regs *cpustate)
{
    /*
     * Place the high order byte of DC0 on the data bus.
     */
    cpustate->dbus = cpustate->dc0 >> 8;
    cpustate->icount -= cL;
}

static void ROMC_07(f8_Regs *cpustate)
{
    /*
     * Place the high order byte of PC1 on the data bus.
     */
    cpustate->dbus = cpustate->pc1 >> 8;
    cpustate->icount -= cL;
}

static void ROMC_08(f8_Regs *cpustate)
{
    /*
     * All devices copy the contents of PC0 into PC1. The CPU outputs
     * zero on the data bus in this ROMC state. Load the data bus into
     * both halves of PC0, thus clearing the register.
     */
    cpustate->pc1 = cpustate->pc0;
    cpustate->dbus = 0;
    cpustate->pc0 = 0;
    cpustate->icount -= cL;
}

static void ROMC_09(f8_Regs *cpustate)
{
    /*
     * The device whose address space includes the contents of the DC0
     * register must place the low order byte of DC0 onto the data bus.
     */
    cpustate->dbus = cpustate->dc0 & 0xff;
    cpustate->icount -= cL;
}

static void ROMC_0A(f8_Regs *cpustate)
{
    /*
     * All devices add the 8-bit value on the data bus, treated as
     * signed binary number, to the data counter.
     */
    cpustate->dc0 += (INT8)cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_0B(f8_Regs *cpustate)
{
    /*
     * The device whose address space includes the value in PC1
     * must place the low order byte of PC1 onto the data bus.
     */
    cpustate->dbus = cpustate->pc1 & 0xff;
    cpustate->icount -= cL;
}

static void ROMC_0C(f8_Regs *cpustate)
{
    /*
     * The device whose address space includes the contents of the PC0
     * register must place the contents of the memory word addressed
     * by PC0 into the data bus; then all devices move the value that
     * has just been placed on the data bus into the low order byte of PC0.
     */
    cpustate->dbus = cpustate->direct->read_raw_byte(cpustate->pc0);
    cpustate->pc0 = (cpustate->pc0 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_0D(f8_Regs *cpustate)
{
    /*
     * All devices store in PC1 the current contents of PC0, incremented
     * by 1; PC0 is unaltered.
     */
    cpustate->pc1 = cpustate->pc0 + 1;
    cpustate->icount -= cS;
}

static void ROMC_0E(f8_Regs *cpustate)
{
    /*
     * The device whose address space includes the contents of the PC0
     * register must place the word addressed by PC0 into the data bus.
     * The value on the data bus is then moved to the low order byte
     * of DC0 by all devices.
     */
    cpustate->dbus = cpustate->direct->read_raw_byte(cpustate->pc0);
    cpustate->dc0 = (cpustate->dc0 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_0F(f8_Regs *cpustate)
{
    /*
     * The interrupting device with highest priority must place the
     * low order byte of the interrupt vector on the data bus.
     * All devices must copy the contents of PC0 into PC1. All devices
     * must move the contents of the data bus into the low order
     * byte of PC0.
     */
    cpustate->irq_vector = (*cpustate->irq_callback)(cpustate->device, F8_INPUT_LINE_INT_REQ);
    cpustate->dbus = cpustate->irq_vector & 0x00ff;
    cpustate->pc1 = cpustate->pc0;
    cpustate->pc0 = (cpustate->pc0 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

#ifdef UNUSED_FUNCTION
static void ROMC_10(f8_Regs *cpustate)
{
    /*
     * Inhibit any modification to the interrupt priority logic.
     */
    cpustate->w |= 0x20;   /* ???? */
    cpustate->icount -= cL;
}
#endif

static void ROMC_11(f8_Regs *cpustate)
{
    /*
     * The device whose address space includes the contents of PC0
     * must place the contents of the addressed memory word on the
     * data bus. All devices must then move the contents of the
     * data bus to the upper byte of DC0.
     */
    cpustate->dbus = cpustate->direct->read_raw_byte(cpustate->pc0);
	cpustate->dc0 = (cpustate->dc0 & 0x00ff) | (cpustate->dbus << 8);
    cpustate->icount -= cL;
}

static void ROMC_12(f8_Regs *cpustate)
{
    /*
     * All devices copy the contents of PC0 into PC1. All devices then
     * move the contents of the data bus into the low order byte of PC0.
     */
    cpustate->pc1 = cpustate->pc0;
    cpustate->pc0 = (cpustate->pc0 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_13(f8_Regs *cpustate)
{
    /*
     * The interrupting device with highest priority must move the high
     * order half of the interrupt vector onto the data bus. All devices
     * must then move the contents of the data bus into the high order
     * byte of PC0. The interrupting device resets its interrupt circuitry
     * (so that it is no longer requesting CPU servicing and can respond
     * to another interrupt).
     */
    cpustate->dbus = cpustate->irq_vector >> 8;
    cpustate->pc0 = (cpustate->pc0 & 0x00ff) | (cpustate->dbus << 8);
    cpustate->w&=~I;
    cpustate->icount -= cL;
}

static void ROMC_14(f8_Regs *cpustate)
{
    /*
     * All devices move the contents of the data bus into the high
     * order byte of PC0.
     */
    cpustate->pc0 = (cpustate->pc0 & 0x00ff) | (cpustate->dbus << 8);
    cpustate->icount -= cL;
}

static void ROMC_15(f8_Regs *cpustate)
{
    /*
     * All devices move the contents of the data bus into the high
     * order byte of PC1.
     */
    cpustate->pc1 = (cpustate->pc1 & 0x00ff) | (cpustate->dbus << 8);
    cpustate->icount -= cL;
}

static void ROMC_16(f8_Regs *cpustate)
{
    /*
     * All devices move the contents of the data bus into the high
     * order byte of DC0.
     */
    cpustate->dc0 = (cpustate->dc0 & 0x00ff) | (cpustate->dbus << 8);
    cpustate->icount -= cL;
}

static void ROMC_17(f8_Regs *cpustate)
{
    /*
     * All devices move the contents of the data bus into the low
     * order byte of PC0.
     */
    cpustate->pc0 = (cpustate->pc0 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_18(f8_Regs *cpustate)
{
    /*
     * All devices move the contents of the data bus into the low
     * order byte of PC1.
     */
    cpustate->pc1 = (cpustate->pc1 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_19(f8_Regs *cpustate)
{
    /*
     * All devices move the contents of the data bus into the low
     * order byte of DC0.
     */
    cpustate->dc0 = (cpustate->dc0 & 0xff00) | cpustate->dbus;
    cpustate->icount -= cL;
}

static void ROMC_1A(f8_Regs *cpustate)
{
    /*
     * During the prior cycle, an I/O port timer or interrupt control
     * register was addressed; the device containing the addressed port
     * must place the contents of the data bus into the address port.
     */
    cpustate->iospace->write_byte(cpustate->io, cpustate->dbus);
    cpustate->icount -= cL;
}

static void ROMC_1B(f8_Regs *cpustate)
{
    /*
     * During the prior cycle, the data bus specified the address of an
     * I/O port. The device containing the addressed I/O port must place
     * the contents of the I/O port on the data bus. (Note that the
     * contents of timer and interrupt control registers cannot be read
     * back onto the data bus).
     */
	cpustate->dbus = cpustate->iospace->read_byte(cpustate->io);
    cpustate->icount -= cL;
}

static void ROMC_1C(f8_Regs *cpustate, int insttim)	/* SKR - added parameter to tell if  */
                                    /* it is long or short based on inst */
{
    /*
     * None.
     */
    cpustate->icount -= insttim;
}

static void ROMC_1D(f8_Regs *cpustate)
{
    /*
     * Devices with DC0 and DC1 registers must switch registers.
     * Devices without a DC1 register perform no operation.
     */
    UINT16 tmp = cpustate->dc0;
    cpustate->dc0 = cpustate->dc1;
    cpustate->dc1 = tmp;
    cpustate->icount -= cS;
}

#ifdef UNUSED_FUNCTION
static void ROMC_1E(f8_Regs *cpustate)
{
    /*
     * The devices whose address space includes the contents of PC0
     * must place the low order byte of PC0 onto the data bus.
     */
    cpustate->dbus = cpustate->pc0 & 0xff;
    cpustate->icount -= cL;
}

static void ROMC_1F(f8_Regs *cpustate)
{
    /*
     * The devices whose address space includes the contents of PC0
     * must place the high order byte of PC0 onto the data bus.
     */
    cpustate->dbus = (cpustate->pc0 >> 8) & 0xff;
    cpustate->icount -= cL;
}
#endif

/***********************************
 *  illegal opcodes
 ***********************************/
static void illegal(f8_Regs *cpustate)
{
    logerror("f8 illegal opcode at 0x%04x: %02x\n", cpustate->pc0, cpustate->dbus);
}

/***************************************************
 *  O Z C S 0000 0000
 *  - - - - LR  A,KU
 ***************************************************/
static void f8_lr_a_ku(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[12];
}

/***************************************************
 *  O Z C S 0000 0001
 *  - - - - LR  A,KL
 ***************************************************/
static void f8_lr_a_kl(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[13];
}

/***************************************************
 *  O Z C S 0000 0010
 *  - - - - LR  A,QU
 ***************************************************/
static void f8_lr_a_qu(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[14];
}

/***************************************************
 *  O Z C S 0000 0011
 *  - - - - LR  A,QL
 ***************************************************/
static void f8_lr_a_ql(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[15];
}

/***************************************************
 *  O Z C S 0000 0100
 *  - - - - LR  KU,A
 ***************************************************/
static void f8_lr_ku_a(f8_Regs *cpustate)
{
    cpustate->r[12] = cpustate->a;
}

/***************************************************
 *  O Z C S 0000 0101
 *  - - - - LR  KL,A
 ***************************************************/
static void f8_lr_kl_a(f8_Regs *cpustate)
{
    cpustate->r[13] = cpustate->a;
}

/***************************************************
 *  O Z C S 0000 0110
 *  - - - - LR  QU,A
 ***************************************************/
static void f8_lr_qu_a(f8_Regs *cpustate)
{
    cpustate->r[14] = cpustate->a;
}

/***************************************************
 *  O Z C S 0000 0111
 *  - - - - LR  QL,A
 ***************************************************/
static void f8_lr_ql_a(f8_Regs *cpustate)
{
    cpustate->r[15] = cpustate->a;
}

/***************************************************
 *  O Z C S 0000 1000
 *  - - - - LR  K,P
 ***************************************************/
static void f8_lr_k_p(f8_Regs *cpustate)
{
    ROMC_07(cpustate);
    cpustate->r[12] = cpustate->dbus;
    ROMC_0B(cpustate);
    cpustate->r[13] = cpustate->dbus;
}

/***************************************************
 *  O Z C S 0000 1001
 *  - - - - LR  P,K
 ***************************************************/
static void f8_lr_p_k(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->r[12];
    ROMC_15(cpustate);
    cpustate->dbus = cpustate->r[13];
    ROMC_18(cpustate);
}

/***************************************************
 *  O Z C S 0000 1010
 *  - - - - LR  A,IS
 ***************************************************/
static void f8_lr_a_is(f8_Regs *cpustate)
{
    cpustate->a = cpustate->is;
}

/***************************************************
 *  O Z C S 0000 1011
 *  - - - - LR  IS,A
 ***************************************************/
static void f8_lr_is_a(f8_Regs *cpustate)
{
    cpustate->is = cpustate->a & 0x3f;
}

/***************************************************
 *  O Z C S 0000 1100
 *  - - - - PK
 ***************************************************/
static void f8_pk(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->r[13];
    ROMC_12(cpustate);
    cpustate->dbus = cpustate->r[12];
    ROMC_14(cpustate);
}

/***************************************************
 *  O Z C S 0000 1101
 *  - - - - LR  P0,Q
 ***************************************************/
static void f8_lr_p0_q(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->r[15];
    ROMC_17(cpustate);
    cpustate->dbus = cpustate->r[14];
    ROMC_14(cpustate);
}

/***************************************************
 *  O Z C S 0000 1110
 *  - - - - LR   Q,DC
 ***************************************************/
static void f8_lr_q_dc(f8_Regs *cpustate)
{
    ROMC_06(cpustate);
    cpustate->r[14] = cpustate->dbus;
    ROMC_09(cpustate);
    cpustate->r[15] = cpustate->dbus;
}

/***************************************************
 *  O Z C S 0000 1111
 *  - - - - LR   DC,Q
 ***************************************************/
static void f8_lr_dc_q(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->r[14];
    ROMC_16(cpustate);
    cpustate->dbus = cpustate->r[15];
    ROMC_19(cpustate);
}

/***************************************************
 *  O Z C S 0001 0000
 *  - - - - LR   DC,H
 ***************************************************/
static void f8_lr_dc_h(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->r[10];
    ROMC_16(cpustate);
    cpustate->dbus = cpustate->r[11];
    ROMC_19(cpustate);
}

/***************************************************
 *  O Z C S 0001 0001
 *  - - - - LR   H,DC
 ***************************************************/
static void f8_lr_h_dc(f8_Regs *cpustate)
{
    ROMC_06(cpustate);
    cpustate->r[10] = cpustate->dbus;
    ROMC_09(cpustate);
    cpustate->r[11] = cpustate->dbus;
}

/***************************************************
 *  O Z C S 0001 0010
 *  0 x 0 1 SR   1
 ***************************************************/
static void f8_sr_1(f8_Regs *cpustate)
{
    cpustate->a >>= 1;
    CLR_OZCS;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0001 0011
 *  0 x 0 x SL   1
 ***************************************************/
static void f8_sl_1(f8_Regs *cpustate)
{
    cpustate->a <<= 1;
    CLR_OZCS;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0001 0100
 *  0 x 0 1 SR   4
 ***************************************************/
static void f8_sr_4(f8_Regs *cpustate)
{
    cpustate->a >>= 4;
    CLR_OZCS;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0001 0101
 *  0 x 0 x SL   4
 ***************************************************/
static void f8_sl_4(f8_Regs *cpustate)
{
    cpustate->a <<= 4;
    CLR_OZCS;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0001 0110
 *  - - - - LM
 ***************************************************/
static void f8_lm(f8_Regs *cpustate)
{
    ROMC_02(cpustate);
    cpustate->a = cpustate->dbus;
}

/***************************************************
 *  O Z C S 0001 0111
 *  - - - - ST
 ***************************************************/
static void f8_st(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->a;
    ROMC_05(cpustate);
}

/***************************************************
 *  O Z C S 0001 1000
 *  0 x 0 x COM
 ***************************************************/
static void f8_com(f8_Regs *cpustate)
{
    cpustate->a = ~cpustate->a;
    CLR_OZCS;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0001 1001
 *  x x x x LNK
 ***************************************************/
static void f8_lnk(f8_Regs *cpustate)
{
    if (cpustate->w & C)
    {
	CLR_OZCS;
	SET_OC(cpustate->a,1);
	cpustate->a += 1;
    }
    else
    {
	CLR_OZCS;
    }
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0001 1010
 *          DI
 ***************************************************/
static void f8_di(f8_Regs *cpustate)
{
    ROMC_1C(cpustate, cS);
    cpustate->w &= ~I;
}

/***************************************************
 *  O Z C S 0001 1011
 *          EI
 ***************************************************/
static void f8_ei(f8_Regs *cpustate)
{
    ROMC_1C(cpustate, cS);
    cpustate->w |= I;
}

/***************************************************
 *  O Z C S 0001 1100
 *          POP
 ***************************************************/
static void f8_pop(f8_Regs *cpustate)
{
    ROMC_04(cpustate);
}

/***************************************************
 *  O Z C S 0001 1101
 *  x x x x LR   W,J
 ***************************************************/
static void f8_lr_w_j(f8_Regs *cpustate)
{
    ROMC_1C(cpustate, cS);
    cpustate->w = cpustate->r[9];
}

/***************************************************
 *  O Z C S 0001 1110
 *  - - - - LR   J,W
 ***************************************************/
static void f8_lr_j_w(f8_Regs *cpustate)
{
    cpustate->r[9] = cpustate->w;
}

/***************************************************
 *  O Z C S 0001 1111
 *  x x x x INC
 ***************************************************/
static void f8_inc(f8_Regs *cpustate)
{
    CLR_OZCS;
    SET_OC(cpustate->a,1);
    cpustate->a += 1;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0010 0000   aaaa aaaa
 *  - - - - LI  aa
 ***************************************************/
static void f8_li(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    cpustate->a = cpustate->dbus;
}

/***************************************************
 *  O Z C S 0010 0001   aaaa aaaa
 *  0 x 0 x NI   aa
 ***************************************************/
static void f8_ni(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    CLR_OZCS;
    cpustate->a &= cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0010 0010   aaaa aaaa
 *  0 x 0 x OI   aa
 ***************************************************/
static void f8_oi(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    CLR_OZCS;
    cpustate->a |= cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0010 0011   aaaa aaaa
 *  0 x 0 x XI   aa
 ***************************************************/
static void f8_xi(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    CLR_OZCS;
    cpustate->a ^= cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0010 0100   aaaa aaaa
 *  x x x x AI   aa
 ***************************************************/
static void f8_ai(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    CLR_OZCS;
    SET_OC(cpustate->a,cpustate->dbus);
    cpustate->a += cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0010 0101   aaaa aaaa
 *  x x x x CI   aa
 ***************************************************/
static void f8_ci(f8_Regs *cpustate)
{
    UINT16 tmp = ((UINT8)~cpustate->a) + 1;
    ROMC_03(cpustate, cL);
    CLR_OZCS;
    SET_OC(tmp,cpustate->dbus);
    tmp += cpustate->dbus;
    SET_SZ((UINT8)tmp);
}

/***************************************************
 *  O Z C S 0010 0110   aaaa aaaa
 *  0 x 0 x IN   aa
 ***************************************************/
static void f8_in(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    CLR_OZCS;
    ROMC_1B(cpustate);
    cpustate->a = cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 0010 0111   aaaa aaaa
 *  - - - - OUT  aa
 ***************************************************/
static void f8_out(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    cpustate->dbus = cpustate->a;
    ROMC_1A(cpustate);
}

/***************************************************
 *  O Z C S 0010 1000   iiii iiii   jjjj jjjj
 *  - - - - PI   iijj
 ***************************************************/
static void f8_pi(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    cpustate->a = cpustate->dbus;
    ROMC_0D(cpustate);
    ROMC_0C(cpustate);
    cpustate->dbus = cpustate->a;
    ROMC_14(cpustate);
}

/***************************************************
 *  O Z C S 0010 1001   iiii iiii   jjjj jjjj
 *  - - - - JMP  iijj
 ***************************************************/
static void f8_jmp(f8_Regs *cpustate)
{
    ROMC_03(cpustate, cL);
    cpustate->a = cpustate->dbus;
    ROMC_0C(cpustate);
    cpustate->dbus = cpustate->a;
    ROMC_14(cpustate);
}

/***************************************************
 *  O Z C S 0010 1010   iiii iiii   jjjj jjjj
 *  - - - - DCI  iijj
 ***************************************************/
static void f8_dci(f8_Regs *cpustate)
{
    ROMC_11(cpustate);
    ROMC_03(cpustate, cS);
    ROMC_0E(cpustate);
    ROMC_03(cpustate, cS);
}

/***************************************************
 *  O Z C S 0010 1011
 *  - - - - NOP
 ***************************************************/
static void f8_nop(f8_Regs *cpustate)
{
}

/***************************************************
 *  O Z C S 0010 1100
 *  - - - - XDC
 ***************************************************/
static void f8_xdc(f8_Regs *cpustate)
{
    ROMC_1D(cpustate);
}

/***************************************************
 *  O Z C S 0011 rrrr
 *  x x x x DS   r
 ***************************************************/
static void f8_ds_r(f8_Regs *cpustate, int r)
{
    CLR_OZCS;
    SET_OC(cpustate->r[r], 0xff);
    cpustate->r[r] = cpustate->r[r] + 0xff;
    SET_SZ(cpustate->r[r]);
}

/***************************************************
 *  O Z C S 0011 1100
 *  x x x x DS   ISAR
 ***************************************************/
static void f8_ds_isar(f8_Regs *cpustate)
{
    CLR_OZCS;
    SET_OC(cpustate->r[cpustate->is], 0xff);
    cpustate->r[cpustate->is] = cpustate->r[cpustate->is] + 0xff;
    SET_SZ(cpustate->r[cpustate->is]);
}

/***************************************************
 *  O Z C S 0011 1101
 *  x x x x DS   ISAR++
 ***************************************************/
static void f8_ds_isar_i(f8_Regs *cpustate)
{
    CLR_OZCS;
    SET_OC(cpustate->r[cpustate->is], 0xff);
    cpustate->r[cpustate->is] = cpustate->r[cpustate->is] + 0xff;
    SET_SZ(cpustate->r[cpustate->is]);
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0011 1110
 *  x x x x DS  ISAR--
 ***************************************************/
static void f8_ds_isar_d(f8_Regs *cpustate)
{
    CLR_OZCS;
    SET_OC(cpustate->r[cpustate->is], 0xff);
    cpustate->r[cpustate->is] = cpustate->r[cpustate->is] + 0xff;
    SET_SZ(cpustate->r[cpustate->is]);
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0100 rrrr
 *  - - - - LR  A,r
 ***************************************************/
static void f8_lr_a_r(f8_Regs *cpustate, int r)
{
    cpustate->a = cpustate->r[r];
}

/***************************************************
 *  O Z C S 0100 1100
 *  - - - - LR  A,ISAR
 ***************************************************/
static void f8_lr_a_isar(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[cpustate->is];
}

/***************************************************
 *  O Z C S 0100 1101
 *  - - - - LR  A,ISAR++
 ***************************************************/
static void f8_lr_a_isar_i(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[cpustate->is];
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0100 1110
 *  - - - - LR  A,ISAR--
 ***************************************************/
static void f8_lr_a_isar_d(f8_Regs *cpustate)
{
    cpustate->a = cpustate->r[cpustate->is];
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0101 rrrr
 *  - - - - LR  r,A
 ***************************************************/
static void f8_lr_r_a(f8_Regs *cpustate, int r)
{
    cpustate->r[r] = cpustate->a;
}

/***************************************************
 *  O Z C S 0101 1100
 *  - - - - LR  ISAR,A
 ***************************************************/
static void f8_lr_isar_a(f8_Regs *cpustate)
{
    cpustate->r[cpustate->is] = cpustate->a;
}

/***************************************************
 *  O Z C S 0101 1101
 *  - - - - LR  ISAR++,A
 ***************************************************/
static void f8_lr_isar_i_a(f8_Regs *cpustate)
{
    cpustate->r[cpustate->is] = cpustate->a;
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0101 1110
 *  - - - - LR  ISAR--,A
 ***************************************************/
static void f8_lr_isar_d_a(f8_Regs *cpustate)
{
    cpustate->r[cpustate->is] = cpustate->a;
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0110 0eee
 *  - - - - LISU e
 ***************************************************/
static void f8_lisu(f8_Regs *cpustate, int e)
{
    cpustate->is = (cpustate->is & 0x07) | e;
}

/***************************************************
 *  O Z C S 0110 1eee
 *  - - - - LISL e
 ***************************************************/
static void f8_lisl(f8_Regs *cpustate, int e)
{
    cpustate->is = (cpustate->is & 0x38) | e;
}

/***************************************************
 *  O Z C S 0111 iiii
 *  - - - - LIS  i
 ***************************************************/
static void f8_lis(f8_Regs *cpustate, int i)
{
    cpustate->a = i;
}

/***************************************************
 *  O Z C S 1000 0eee   aaaa aaaa
 *          BT   e,aa
 ***************************************************/
static void f8_bt(f8_Regs *cpustate, int e)
{
    ROMC_1C(cpustate, cS);
    if (cpustate->w & e)
	ROMC_01(cpustate);	   /* take the relative branch */
    else
	ROMC_03(cpustate, cS);	   /* just read the argument on the data bus */
}

/***************************************************
 *  O Z C S 1000 1000
 *  x x x x AM
 ***************************************************/
static void f8_am(f8_Regs *cpustate)
{
    ROMC_02(cpustate);
    CLR_OZCS;
    SET_OC(cpustate->a, cpustate->dbus);
    cpustate->a += cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1000 1001
 *  x x x x AMD
 ***************************************************/
static void f8_amd(f8_Regs *cpustate)
{

/*SKR from F8 Guide To programming description of AMD

 binary add the addend to the binary sum of the augend and $66
   *NOTE* the binary addition of the augend to $66 is done before AMD is called
   record the status of the carry and intermediate carry
   add a factor to the sum based on the carry and intermediate carry:
     no carry, no intermediate carry, add $AA
     no carry, intermediate carry, add $A0
     carry, no intermediate carry, add $0A
     carry, intermediate carry, add $00
     any carry from the low-order digit is suppressed
   *NOTE* status flags are updated prior to the factor being added
*/

    UINT8 augend=cpustate->a;
    ROMC_02(cpustate);
    UINT8 addend=cpustate->dbus;
    UINT8 tmp=addend+augend;

    UINT8 c=0;              /* high order carry */
    UINT8 ic=0;             /* low order carry */
    if(((augend+addend)&0xff0)>0xf0)
    c=1;
    if((augend&0x0f)+(addend&0x0f)>0x0F)
    ic=1;

    CLR_OZCS;
    SET_OC(augend,addend);
    SET_SZ(tmp);

    if(c==0&&ic==0)
    tmp=((tmp+0xa0)&0xf0)+((tmp+0x0a)&0x0f);
    if(c==0&&ic==1)
    tmp=((tmp+0xa0)&0xf0)+(tmp&0x0f);
    if(c==1&&ic==0)
    tmp=(tmp&0xf0)+((tmp+0x0a)&0x0f);

    cpustate->a = tmp;
}

/***************************************************
 *  O Z C S 1000 1010
 *  0 x 0 x NM
 ***************************************************/
static void f8_nm(f8_Regs *cpustate)
{
    ROMC_02(cpustate);
    CLR_OZCS;
    cpustate->a &= cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1000 1011
 *  0 x 0 x OM
 ***************************************************/
static void f8_om(f8_Regs *cpustate)
{
    ROMC_02(cpustate);
    CLR_OZCS;
    cpustate->a |= cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1000 1100
 *  0 x 0 x XM
 ***************************************************/
static void f8_xm(f8_Regs *cpustate)
{
    ROMC_02(cpustate);
    CLR_OZCS;
    cpustate->a ^= cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1000 1101
 *  x x x x CM
 ***************************************************/
static void f8_cm(f8_Regs *cpustate)	/* SKR changed to match f8_ci(cpustate) */
{
    UINT16 tmp = ((UINT8)~cpustate->a) + 1;
	ROMC_02(cpustate);
	CLR_OZCS;
	SET_OC(tmp,cpustate->dbus);
	tmp += cpustate->dbus;
	SET_SZ((UINT8)tmp);
}

/***************************************************
 *  O Z C S 1000 1110
 *  - - - - ADC
 ***************************************************/
static void f8_adc(f8_Regs *cpustate)
{
    cpustate->dbus = cpustate->a;
    ROMC_0A(cpustate);			/* add data bus value to DC0 */
}

/***************************************************
 *  O Z C S 1000 1111
 *  - - - - BR7
 ***************************************************/
static void f8_br7(f8_Regs *cpustate)
{
    if ((cpustate->is & 7) == 7)
	ROMC_03(cpustate, cS);		/* just read the argument on the data bus */
    else
	ROMC_01(cpustate);		/* take the relative branch */
}

/***************************************************
 *  O Z C S 1001 tttt   aaaa aaaa
 *  - - - - BF   t,aa
 ***************************************************/
static void f8_bf(f8_Regs *cpustate, int t)
{
    ROMC_1C(cpustate, cS);
    if (cpustate->w & t)
        ROMC_03(cpustate, cS);      /* just read the argument on the data bus */
    else
        ROMC_01(cpustate);      /* take the relative branch */
}

/***************************************************
 *  O Z C S 1010 000n
 *  0 x 0 x INS  n              (n = 0-1)
 ***************************************************/
static void f8_ins_0(f8_Regs *cpustate, int n)
{
    ROMC_1C(cpustate, cS);
    CLR_OZCS;
    cpustate->a = cpustate->iospace->read_byte(n);
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1010 nnnn
 *  0 x 0 x INS  n              (n = 4-F)
 ***************************************************/
static void f8_ins_1(f8_Regs *cpustate, int n)
{
    ROMC_1C(cpustate, cL);
    cpustate->io = n;
    ROMC_1B(cpustate);
    CLR_OZCS;
    cpustate->a = cpustate->dbus;
    SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1011 000n
 *  - - - - OUTS n              (n = 0-1)
 ***************************************************/
static void f8_outs_0(f8_Regs *cpustate, int n)
{
    ROMC_1C(cpustate, cS);
    cpustate->iospace->write_byte(n, cpustate->a);
}

/***************************************************
 *  O Z C S 1011 nnnn
 *  - - - - OUTS n              (n = 4-F)
 ***************************************************/
static void f8_outs_1(f8_Regs *cpustate, int n)
{
    ROMC_1C(cpustate, cL);
    cpustate->io = n;
    cpustate->dbus = cpustate->a;
    ROMC_1A(cpustate);
}

/***************************************************
 *  O Z C S 1100 rrrr
 *  x x x x AS   r
 ***************************************************/
static void f8_as(f8_Regs *cpustate, int r)
{
	CLR_OZCS;
	SET_OC(cpustate->a, cpustate->r[r]);
	cpustate->a += cpustate->r[r];
	SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1100 1100
 *  x x x x AS   ISAR
 ***************************************************/
static void f8_as_isar(f8_Regs *cpustate)
{
	CLR_OZCS;
	SET_OC(cpustate->a, cpustate->r[cpustate->is]);
	cpustate->a += cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1100 1101
 *  x x x x AS   ISAR++
 ***************************************************/
static void f8_as_isar_i(f8_Regs *cpustate)
{
	CLR_OZCS;
	SET_OC(cpustate->a, cpustate->r[cpustate->is]);
	cpustate->a += cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
	cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1100 1110
 *  x x x x AS   ISAR--
 ***************************************************/
static void f8_as_isar_d(f8_Regs *cpustate)
{
	CLR_OZCS;
	SET_OC(cpustate->a, cpustate->r[cpustate->is]);
	cpustate->a += cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
	cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1101 rrrr
 *  x x x x ASD  r
 ***************************************************/
static void f8_asd(f8_Regs *cpustate, int r)
{
/*SKR from F8 Guide To programming description of AMD */
    UINT8 augend=cpustate->a;
    ROMC_1C(cpustate, cS);
    UINT8 addend=cpustate->r[r];
    UINT8 tmp=augend+addend;

    UINT8 c=0;
    UINT8 ic=0;
    if(((augend+addend)&0xff0)>0xf0)
    c=1;
    if((augend&0x0f)+(addend&0x0f)>0x0F)
    ic=1;

    CLR_OZCS;
    SET_OC(augend,addend);
    SET_SZ(tmp);

    if(c==0&&ic==0)
    tmp=((tmp+0xa0)&0xf0)+((tmp+0x0a)&0x0f);
    if(c==0&&ic==1)
    tmp=((tmp+0xa0)&0xf0)+(tmp&0x0f);
    if(c==1&&ic==0)
    tmp=(tmp&0xf0)+((tmp+0x0a)&0x0f);

    cpustate->a = tmp;
}

/***************************************************
 *  O Z C S 1101 1100
 *  x x x x ASD  ISAR
 ***************************************************/
static void f8_asd_isar(f8_Regs *cpustate)
{
/*SKR from F8 Guide To programming description of AMD */
    UINT8 augend=cpustate->a;
    ROMC_1C(cpustate, cS);
    UINT8 addend=cpustate->r[cpustate->is];
    UINT8 tmp=augend+addend;

    UINT8 c=0;
    UINT8 ic=0;
    if(((augend+addend)&0xff0)>0xf0)
    c=1;
    if((augend&0x0f)+(addend&0x0f)>0x0F)
    ic=1;

    CLR_OZCS;
    SET_OC(augend,addend);
    SET_SZ(tmp);

    if(c==0&&ic==0)
    tmp=((tmp+0xa0)&0xf0)+((tmp+0x0a)&0x0f);
    if(c==0&&ic==1)
    tmp=((tmp+0xa0)&0xf0)+(tmp&0x0f);
    if(c==1&&ic==0)
    tmp=(tmp&0xf0)+((tmp+0x0a)&0x0f);

    cpustate->a = tmp;
}

/***************************************************
 *  O Z C S 1101 1101
 *  x x x x ASD  ISAR++
 ***************************************************/
static void f8_asd_isar_i(f8_Regs *cpustate)
{
/*SKR from F8 Guide To programming description of AMD */
    UINT8 augend=cpustate->a;
    ROMC_1C(cpustate, cS);
    UINT8 addend=cpustate->r[cpustate->is];
    UINT8 tmp=augend+addend;

    UINT8 c=0;
    UINT8 ic=0;
    if(((augend+addend)&0xff0)>0xf0)
    c=1;
    if((augend&0x0f)+(addend&0x0f)>0x0F)
    ic=1;

    CLR_OZCS;
    SET_OC(augend,addend);
    SET_SZ(tmp);

    if(c==0&&ic==0)
    tmp=((tmp+0xa0)&0xf0)+((tmp+0x0a)&0x0f);
    if(c==0&&ic==1)
    tmp=((tmp+0xa0)&0xf0)+(tmp&0x0f);
    if(c==1&&ic==0)
    tmp=(tmp&0xf0)+((tmp+0x0a)&0x0f);

    cpustate->a = tmp;
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1101 1110
 *  x x x x ASD  ISAR--
 ***************************************************/
static void f8_asd_isar_d(f8_Regs *cpustate)
{
/*SKR from F8 Guide To programming description of AMD */
    UINT8 augend=cpustate->a;
    ROMC_1C(cpustate, cS);
    UINT8 addend=cpustate->r[cpustate->is];
    UINT8 tmp=augend+addend;

    UINT8 c=0;
    UINT8 ic=0;
    if(((augend+addend)&0xff0)>0xf0)
    c=1;
    if((augend&0x0f)+(addend&0x0f)>0x0F)
    ic=1;

    CLR_OZCS;
    SET_OC(augend,addend);
    SET_SZ(tmp);

    if(c==0&&ic==0)
    tmp=((tmp+0xa0)&0xf0)+((tmp+0x0a)&0x0f);
    if(c==0&&ic==1)
    tmp=((tmp+0xa0)&0xf0)+(tmp&0x0f);
    if(c==1&&ic==0)
    tmp=(tmp&0xf0)+((tmp+0x0a)&0x0f);

    cpustate->a = tmp;
    cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1110 rrrr
 *  0 x 0 x XS   r
 ***************************************************/
static void f8_xs(f8_Regs *cpustate, int r)
{
	CLR_OZCS;
	cpustate->a ^= cpustate->r[r];
	SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1110 1100
 *  0 x 0 x XS   ISAR
 ***************************************************/
static void f8_xs_isar(f8_Regs *cpustate)
{
	CLR_OZCS;
	cpustate->a ^= cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1110 1101
 *  0 x 0 x XS   ISAR++
 ***************************************************/
static void f8_xs_isar_i(f8_Regs *cpustate)
{
	CLR_OZCS;
	cpustate->a ^= cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
	cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1110 1110
 *  0 x 0 x XS   ISAR--
 ***************************************************/
static void f8_xs_isar_d(f8_Regs *cpustate)
{
	CLR_OZCS;
	cpustate->a ^= cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
	cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1111 rrrr
 *  0 x 0 x NS   r
 ***************************************************/
static void f8_ns(f8_Regs *cpustate, int r)
{
	CLR_OZCS;
	cpustate->a &= cpustate->r[r];
	SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1111 1100
 *  0 x 0 x NS   ISAR
 ***************************************************/
static void f8_ns_isar(f8_Regs *cpustate)
{
	CLR_OZCS;
	cpustate->a &= cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
}

/***************************************************
 *  O Z C S 1111 1101
 *  0 x 0 x NS   ISAR++
 ***************************************************/
static void f8_ns_isar_i(f8_Regs *cpustate)
{
	CLR_OZCS;
	cpustate->a &= cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
	cpustate->is = (cpustate->is & 0x38) | ((cpustate->is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1111 1110
 *  0 x 0 x NS   ISAR--
 ***************************************************/
static void f8_ns_isar_d(f8_Regs *cpustate)
{
	CLR_OZCS;
	cpustate->a &= cpustate->r[cpustate->is];
	SET_SZ(cpustate->a);
	cpustate->is = (cpustate->is & 0x38) | ((cpustate->is - 1) & 0x07);
}

static CPU_RESET( f8 )
{
	f8_Regs *cpustate = get_safe_token(device);
	UINT8 data;
	int i;
	device_irq_acknowledge_callback save_callback;

	save_callback = cpustate->irq_callback;
	memset(cpustate, 0, sizeof(f8_Regs));
	cpustate->irq_callback = save_callback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->iospace = &device->space(AS_IO);
	cpustate->w&=~I;

	/* save PC0 to PC1 and reset PC0 */
	ROMC_08(cpustate);
	/* fetch the first opcode */
	ROMC_00(cpustate, cS);

	/* initialize the timer shift register
     * this is an 8 bit polynome counter which can be loaded parallel
     * with 0xff the outputs never change and thus the timer is disabled.
     * with 0xfe the shifter starts cycling through 255 states until it
     * reaches 0xfe again (and then issues an interrupt).
     * the counter output values are not sequential, but go like this:
     * 0xfe, 0xfd, 0xfb, 0xf7, 0xee, 0xdc ... etc. :-)
     * We have to build a lookup table to tell how many cycles a write

     */
	data = 0xfe;	/* initial value */
	for (i = 0; i < 256; i++)
	{
		timer_shifter[i] = data;
		if ( (((data >> 3) ^ (data >> 4)) ^
			  ((data >> 5) ^ (data >> 7))) & 1 )
			data <<= 1;
		else
			data = (data << 1) | 1;
	}
}

/* Execute cycles - returns number of cycles actually run */
static CPU_EXECUTE( f8 )
{
    f8_Regs *cpustate = get_safe_token(device);

    do
    {
	UINT8 op=cpustate->dbus;
        debugger_instruction_hook(device, (cpustate->pc0 - 1) & 0xffff);

	switch( op )
        {
	/* opcode  bitmask */
	case 0x00: /* 0000 0000 */	f8_lr_a_ku(cpustate);		break;
	case 0x01: /* 0000 0001 */	f8_lr_a_kl(cpustate);		break;
	case 0x02: /* 0000 0010 */	f8_lr_a_qu(cpustate);		break;
	case 0x03: /* 0000 0011 */	f8_lr_a_ql(cpustate);		break;
	case 0x04: /* 0000 0100 */	f8_lr_ku_a(cpustate);		break;
	case 0x05: /* 0000 0101 */	f8_lr_kl_a(cpustate);		break;
	case 0x06: /* 0000 0110 */	f8_lr_qu_a(cpustate);		break;
	case 0x07: /* 0000 0111 */	f8_lr_ql_a(cpustate);		break;
	case 0x08: /* 0000 1000 */	f8_lr_k_p(cpustate);		break;
	case 0x09: /* 0000 1001 */	f8_lr_p_k(cpustate);		break;
	case 0x0a: /* 0000 1010 */	f8_lr_a_is(cpustate);		break;
	case 0x0b: /* 0000 1011 */	f8_lr_is_a(cpustate);		break;
	case 0x0c: /* 0000 1100 */	f8_pk(cpustate);			break;
	case 0x0d: /* 0000 1101 */	f8_lr_p0_q(cpustate);		break;
	case 0x0e: /* 0000 1110 */	f8_lr_q_dc(cpustate);		break;
	case 0x0f: /* 0000 1111 */	f8_lr_dc_q(cpustate);		break;

        case 0x10: /* 0001 0000 */	f8_lr_dc_h(cpustate);       break;
	case 0x11: /* 0001 0001 */	f8_lr_h_dc(cpustate);		break;
	case 0x12: /* 0001 0010 */	f8_sr_1(cpustate);			break;
	case 0x13: /* 0001 0011 */	f8_sl_1(cpustate);			break;
	case 0x14: /* 0001 0100 */	f8_sr_4(cpustate);			break;
	case 0x15: /* 0001 0101 */	f8_sl_4(cpustate);			break;
	case 0x16: /* 0001 0110 */	f8_lm(cpustate);			break;
	case 0x17: /* 0001 0111 */	f8_st(cpustate);			break;
	case 0x18: /* 0001 1000 */	f8_com(cpustate);			break;
	case 0x19: /* 0001 1001 */	f8_lnk(cpustate);			break;
	case 0x1a: /* 0001 1010 */	f8_di(cpustate);			break;
	case 0x1b: /* 0001 1011 */	f8_ei(cpustate);			break;
	case 0x1c: /* 0001 1100 */	f8_pop(cpustate);			break;
	case 0x1d: /* 0001 1101 */	f8_lr_w_j(cpustate);		break;
	case 0x1e: /* 0001 1110 */	f8_lr_j_w(cpustate);		break;
	case 0x1f: /* 0001 1111 */	f8_inc(cpustate);			break;

        case 0x20: /* 0010 0000 */	f8_li(cpustate);            break;
	case 0x21: /* 0010 0001 */	f8_ni(cpustate);			break;
	case 0x22: /* 0010 0010 */	f8_oi(cpustate);			break;
	case 0x23: /* 0010 0011 */	f8_xi(cpustate);			break;
	case 0x24: /* 0010 0100 */	f8_ai(cpustate);			break;
	case 0x25: /* 0010 0101 */	f8_ci(cpustate);			break;
	case 0x26: /* 0010 0110 */	f8_in(cpustate);			break;
	case 0x27: /* 0010 0111 */	f8_out(cpustate);			break;
	case 0x28: /* 0010 1000 */	f8_pi(cpustate);			break;
	case 0x29: /* 0010 1001 */	f8_jmp(cpustate);			break;
	case 0x2a: /* 0010 1010 */	f8_dci(cpustate);			break;
	case 0x2b: /* 0010 1011 */	f8_nop(cpustate);			break;
	case 0x2c: /* 0010 1100 */	f8_xdc(cpustate);			break;
	case 0x2d: /* 0010 1101 */	illegal(cpustate);			break;
	case 0x2e: /* 0010 1110 */	illegal(cpustate);			break;
	case 0x2f: /* 0010 1111 */	illegal(cpustate);			break;

        case 0x30: /* 0011 0000 */	f8_ds_r(cpustate,  0);        break;
	case 0x31: /* 0011 0001 */	f8_ds_r(cpustate,  1);		break;
	case 0x32: /* 0011 0010 */	f8_ds_r(cpustate,  2);		break;
	case 0x33: /* 0011 0011 */	f8_ds_r(cpustate,  3);		break;
	case 0x34: /* 0011 0100 */	f8_ds_r(cpustate,  4);		break;
	case 0x35: /* 0011 0101 */	f8_ds_r(cpustate,  5);		break;
	case 0x36: /* 0011 0110 */	f8_ds_r(cpustate,  6);		break;
	case 0x37: /* 0011 0111 */	f8_ds_r(cpustate,  7);		break;
	case 0x38: /* 0011 1000 */	f8_ds_r(cpustate,  8);		break;
	case 0x39: /* 0011 1001 */	f8_ds_r(cpustate,  9);		break;
	case 0x3a: /* 0011 1010 */	f8_ds_r(cpustate, 10);		break;
	case 0x3b: /* 0011 1011 */	f8_ds_r(cpustate, 11);		break;
	case 0x3c: /* 0011 1100 */	f8_ds_isar(cpustate);		break;
	case 0x3d: /* 0011 1101 */	f8_ds_isar_i(cpustate); 	break;
	case 0x3e: /* 0011 1110 */	f8_ds_isar_d(cpustate); 	break;
	case 0x3f: /* 0011 1111 */	illegal(cpustate);			break;

        case 0x40: /* 0100 0000 */	f8_lr_a_r(cpustate,  0);      break;
	case 0x41: /* 0100 0001 */	f8_lr_a_r(cpustate,  1);		break;
	case 0x42: /* 0100 0010 */	f8_lr_a_r(cpustate,  2);		break;
	case 0x43: /* 0100 0011 */	f8_lr_a_r(cpustate,  3);		break;
	case 0x44: /* 0100 0100 */	f8_lr_a_r(cpustate,  4);		break;
	case 0x45: /* 0100 0101 */	f8_lr_a_r(cpustate,  5);		break;
	case 0x46: /* 0100 0110 */	f8_lr_a_r(cpustate,  6);		break;
	case 0x47: /* 0100 0111 */	f8_lr_a_r(cpustate,  7);		break;
	case 0x48: /* 0100 1000 */	f8_lr_a_r(cpustate,  8);		break;
	case 0x49: /* 0100 1001 */	f8_lr_a_r(cpustate,  9);		break;
	case 0x4a: /* 0100 1010 */	f8_lr_a_r(cpustate, 10);		break;
	case 0x4b: /* 0100 1011 */	f8_lr_a_r(cpustate, 11);		break;
	case 0x4c: /* 0100 1100 */	f8_lr_a_isar(cpustate); 	break;
	case 0x4d: /* 0100 1101 */	f8_lr_a_isar_i(cpustate);	break;
	case 0x4e: /* 0100 1110 */	f8_lr_a_isar_d(cpustate);	break;
	case 0x4f: /* 0100 1111 */	illegal(cpustate);			break;

	case 0x50: /* 0101 0000 */	f8_lr_r_a(cpustate,  0);		break;
	case 0x51: /* 0101 0001 */	f8_lr_r_a(cpustate,  1);		break;
	case 0x52: /* 0101 0010 */	f8_lr_r_a(cpustate,  2);		break;
	case 0x53: /* 0101 0011 */	f8_lr_r_a(cpustate,  3);		break;
	case 0x54: /* 0101 0100 */	f8_lr_r_a(cpustate,  4);		break;
	case 0x55: /* 0101 0101 */	f8_lr_r_a(cpustate,  5);		break;
	case 0x56: /* 0101 0110 */	f8_lr_r_a(cpustate,  6);		break;
	case 0x57: /* 0101 0111 */	f8_lr_r_a(cpustate,  7);		break;
	case 0x58: /* 0101 1000 */	f8_lr_r_a(cpustate,  8);		break;
	case 0x59: /* 0101 1001 */	f8_lr_r_a(cpustate,  9);		break;
	case 0x5a: /* 0101 1010 */	f8_lr_r_a(cpustate, 10);		break;
	case 0x5b: /* 0101 1011 */	f8_lr_r_a(cpustate, 11);		break;
	case 0x5c: /* 0101 1100 */	f8_lr_isar_a(cpustate); 	break;
	case 0x5d: /* 0101 1101 */	f8_lr_isar_i_a(cpustate);	break;
	case 0x5e: /* 0101 1110 */	f8_lr_isar_d_a(cpustate);	break;
	case 0x5f: /* 0101 1111 */	illegal(cpustate);			break;

	case 0x60: /* 0110 0000 */	f8_lisu(cpustate, 0x00);		break;
	case 0x61: /* 0110 0001 */	f8_lisu(cpustate, 0x08);		break;
	case 0x62: /* 0110 0010 */	f8_lisu(cpustate, 0x10);		break;
	case 0x63: /* 0110 0011 */	f8_lisu(cpustate, 0x18);		break;
	case 0x64: /* 0110 0100 */	f8_lisu(cpustate, 0x20);		break;
	case 0x65: /* 0110 0101 */	f8_lisu(cpustate, 0x28);		break;
	case 0x66: /* 0110 0110 */	f8_lisu(cpustate, 0x30);		break;
	case 0x67: /* 0110 0111 */	f8_lisu(cpustate, 0x38);		break;
	case 0x68: /* 0110 1000 */	f8_lisl(cpustate, 0x00);		break;
	case 0x69: /* 0110 1001 */	f8_lisl(cpustate, 0x01);		break;
	case 0x6a: /* 0110 1010 */	f8_lisl(cpustate, 0x02);		break;
	case 0x6b: /* 0110 1011 */	f8_lisl(cpustate, 0x03);		break;
	case 0x6c: /* 0110 1100 */	f8_lisl(cpustate, 0x04);		break;
	case 0x6d: /* 0110 1101 */	f8_lisl(cpustate, 0x05);		break;
	case 0x6e: /* 0110 1110 */	f8_lisl(cpustate, 0x06);		break;
	case 0x6f: /* 0110 1111 */	f8_lisl(cpustate, 0x07);		break;

	case 0x70: /* 0111 0000 */	f8_lis(cpustate, 0x0);		break;
	case 0x71: /* 0111 0001 */	f8_lis(cpustate, 0x1);		break;
	case 0x72: /* 0111 0010 */	f8_lis(cpustate, 0x2);		break;
	case 0x73: /* 0111 0011 */	f8_lis(cpustate, 0x3);		break;
	case 0x74: /* 0111 0100 */	f8_lis(cpustate, 0x4);		break;
	case 0x75: /* 0111 0101 */	f8_lis(cpustate, 0x5);		break;
	case 0x76: /* 0111 0110 */	f8_lis(cpustate, 0x6);		break;
	case 0x77: /* 0111 0111 */	f8_lis(cpustate, 0x7);		break;
	case 0x78: /* 0111 1000 */	f8_lis(cpustate, 0x8);		break;
	case 0x79: /* 0111 1001 */	f8_lis(cpustate, 0x9);		break;
	case 0x7a: /* 0111 1010 */	f8_lis(cpustate, 0xa);		break;
	case 0x7b: /* 0111 1011 */	f8_lis(cpustate, 0xb);		break;
	case 0x7c: /* 0111 1100 */	f8_lis(cpustate, 0xc);		break;
	case 0x7d: /* 0111 1101 */	f8_lis(cpustate, 0xd);		break;
	case 0x7e: /* 0111 1110 */	f8_lis(cpustate, 0xe);		break;
	case 0x7f: /* 0111 1111 */	f8_lis(cpustate, 0xf);		break;

	case 0x80: /* 1000 0000 */	f8_bt(cpustate, 0);			break;
	case 0x81: /* 1000 0001 */	f8_bt(cpustate, 1);			break;
	case 0x82: /* 1000 0010 */	f8_bt(cpustate, 2);			break;
	case 0x83: /* 1000 0011 */	f8_bt(cpustate, 3);			break;
	case 0x84: /* 1000 0100 */	f8_bt(cpustate, 4);			break;
	case 0x85: /* 1000 0101 */	f8_bt(cpustate, 5);			break;
	case 0x86: /* 1000 0110 */	f8_bt(cpustate, 6);			break;
	case 0x87: /* 1000 0111 */	f8_bt(cpustate, 7);			break;
	case 0x88: /* 1000 1000 */	f8_am(cpustate);			break;
	case 0x89: /* 1000 1001 */	f8_amd(cpustate);			break;
	case 0x8a: /* 1000 1010 */	f8_nm(cpustate);			break;
	case 0x8b: /* 1000 1011 */	f8_om(cpustate);			break;
	case 0x8c: /* 1000 1100 */	f8_xm(cpustate);			break;
	case 0x8d: /* 1000 1101 */	f8_cm(cpustate);			break;
	case 0x8e: /* 1000 1110 */	f8_adc(cpustate);			break;
	case 0x8f: /* 1000 1111 */	f8_br7(cpustate);			break;

	case 0x90: /* 1001 0000 */	f8_bf(cpustate, 0x0);		break;
	case 0x91: /* 1001 0001 */	f8_bf(cpustate, 0x1);		break;
	case 0x92: /* 1001 0010 */	f8_bf(cpustate, 0x2);		break;
	case 0x93: /* 1001 0011 */	f8_bf(cpustate, 0x3);		break;
	case 0x94: /* 1001 0100 */	f8_bf(cpustate, 0x4);		break;
	case 0x95: /* 1001 0101 */	f8_bf(cpustate, 0x5);		break;
	case 0x96: /* 1001 0110 */	f8_bf(cpustate, 0x6);		break;
	case 0x97: /* 1001 0111 */	f8_bf(cpustate, 0x7);		break;
	case 0x98: /* 1001 1000 */	f8_bf(cpustate, 0x8);		break;
	case 0x99: /* 1001 1001 */	f8_bf(cpustate, 0x9);		break;
	case 0x9a: /* 1001 1010 */	f8_bf(cpustate, 0xa);		break;
	case 0x9b: /* 1001 1011 */	f8_bf(cpustate, 0xb);		break;
	case 0x9c: /* 1001 1100 */	f8_bf(cpustate, 0xc);		break;
	case 0x9d: /* 1001 1101 */	f8_bf(cpustate, 0xd);		break;
	case 0x9e: /* 1001 1110 */	f8_bf(cpustate, 0xe);		break;
	case 0x9f: /* 1001 1111 */	f8_bf(cpustate, 0xf);		break;

	case 0xa0: /* 1010 0000 */	f8_ins_0(cpustate, 0x0);		break;
	case 0xa1: /* 1010 0001 */	f8_ins_0(cpustate, 0x1);		break;
	case 0xa2: /* 1010 0010 */	illegal(cpustate);			break;
	case 0xa3: /* 1010 0011 */	illegal(cpustate);			break;
	case 0xa4: /* 1010 0100 */	f8_ins_1(cpustate, 0x4);		break;
	case 0xa5: /* 1010 0101 */	f8_ins_1(cpustate, 0x5);		break;
	case 0xa6: /* 1010 0110 */	f8_ins_1(cpustate, 0x6);		break;
	case 0xa7: /* 1010 0111 */	f8_ins_1(cpustate, 0x7);		break;
	case 0xa8: /* 1010 1000 */	f8_ins_1(cpustate, 0x8);		break;
	case 0xa9: /* 1010 1001 */	f8_ins_1(cpustate, 0x9);		break;
	case 0xaa: /* 1010 1010 */	f8_ins_1(cpustate, 0xa);		break;
	case 0xab: /* 1010 1011 */	f8_ins_1(cpustate, 0xb);		break;
	case 0xac: /* 1010 1100 */	f8_ins_1(cpustate, 0xc);		break;
	case 0xad: /* 1010 1101 */	f8_ins_1(cpustate, 0xd);		break;
	case 0xae: /* 1010 1110 */	f8_ins_1(cpustate, 0xe);		break;
	case 0xaf: /* 1010 1111 */	f8_ins_1(cpustate, 0xf);		break;

	case 0xb0: /* 1011 0000 */	f8_outs_0(cpustate, 0x0);	break;
	case 0xb1: /* 1011 0001 */	f8_outs_0(cpustate, 0x1);	break;
	case 0xb2: /* 1011 0010 */	illegal(cpustate);			break;
	case 0xb3: /* 1011 0011 */	illegal(cpustate);			break;
	case 0xb4: /* 1011 0100 */	f8_outs_1(cpustate, 0x4);	break;
	case 0xb5: /* 1011 0101 */	f8_outs_1(cpustate, 0x5);	break;
	case 0xb6: /* 1011 0110 */	f8_outs_1(cpustate, 0x6);	break;
	case 0xb7: /* 1011 0111 */	f8_outs_1(cpustate, 0x7);	break;
	case 0xb8: /* 1011 1000 */	f8_outs_1(cpustate, 0x8);	break;
	case 0xb9: /* 1011 1001 */	f8_outs_1(cpustate, 0x9);	break;
	case 0xba: /* 1011 1010 */	f8_outs_1(cpustate, 0xa);	break;
	case 0xbb: /* 1011 1011 */	f8_outs_1(cpustate, 0xb);	break;
	case 0xbc: /* 1011 1100 */	f8_outs_1(cpustate, 0xc);	break;
	case 0xbd: /* 1011 1101 */	f8_outs_1(cpustate, 0xd);	break;
	case 0xbe: /* 1011 1110 */	f8_outs_1(cpustate, 0xe);	break;
	case 0xbf: /* 1011 1111 */	f8_outs_1(cpustate, 0xf);	break;

	case 0xc0: /* 1100 0000 */	f8_as(cpustate, 0x0);		break;
	case 0xc1: /* 1100 0001 */	f8_as(cpustate, 0x1);		break;
	case 0xc2: /* 1100 0010 */	f8_as(cpustate, 0x2);		break;
	case 0xc3: /* 1100 0011 */	f8_as(cpustate, 0x3);		break;
	case 0xc4: /* 1100 0100 */	f8_as(cpustate, 0x4);		break;
	case 0xc5: /* 1100 0101 */	f8_as(cpustate, 0x5);		break;
	case 0xc6: /* 1100 0110 */	f8_as(cpustate, 0x6);		break;
	case 0xc7: /* 1100 0111 */	f8_as(cpustate, 0x7);		break;
	case 0xc8: /* 1100 1000 */	f8_as(cpustate, 0x8);		break;
	case 0xc9: /* 1100 1001 */	f8_as(cpustate, 0x9);		break;
	case 0xca: /* 1100 1010 */	f8_as(cpustate, 0xa);		break;
	case 0xcb: /* 1100 1011 */	f8_as(cpustate, 0xb);		break;
	case 0xcc: /* 1100 1100 */	f8_as_isar(cpustate);		break;
	case 0xcd: /* 1100 1101 */	f8_as_isar_i(cpustate); 	break;
	case 0xce: /* 1100 1110 */	f8_as_isar_d(cpustate); 	break;
	case 0xcf: /* 1100 1111 */	illegal(cpustate);			break;

	case 0xd0: /* 1101 0000 */	f8_asd(cpustate, 0x0);		break;
	case 0xd1: /* 1101 0001 */	f8_asd(cpustate, 0x1);		break;
	case 0xd2: /* 1101 0010 */	f8_asd(cpustate, 0x2);		break;
	case 0xd3: /* 1101 0011 */	f8_asd(cpustate, 0x3);		break;
	case 0xd4: /* 1101 0100 */	f8_asd(cpustate, 0x4);		break;
	case 0xd5: /* 1101 0101 */	f8_asd(cpustate, 0x5);		break;
	case 0xd6: /* 1101 0110 */	f8_asd(cpustate, 0x6);		break;
	case 0xd7: /* 1101 0111 */	f8_asd(cpustate, 0x7);		break;
	case 0xd8: /* 1101 1000 */	f8_asd(cpustate, 0x8);		break;
	case 0xd9: /* 1101 1001 */	f8_asd(cpustate, 0x9);		break;
	case 0xda: /* 1101 1010 */	f8_asd(cpustate, 0xa);		break;
	case 0xdb: /* 1101 1011 */	f8_asd(cpustate, 0xb);		break;
	case 0xdc: /* 1101 1100 */	f8_asd_isar(cpustate);		break;
	case 0xdd: /* 1101 1101 */	f8_asd_isar_i(cpustate);	break;
	case 0xde: /* 1101 1110 */	f8_asd_isar_d(cpustate);	break;
	case 0xdf: /* 1101 1111 */	illegal(cpustate);			break;

	case 0xe0: /* 1110 0000 */	f8_xs(cpustate, 0x0);		break;
	case 0xe1: /* 1110 0001 */	f8_xs(cpustate, 0x1);		break;
	case 0xe2: /* 1110 0010 */	f8_xs(cpustate, 0x2);		break;
	case 0xe3: /* 1110 0011 */	f8_xs(cpustate, 0x3);		break;
	case 0xe4: /* 1110 0100 */	f8_xs(cpustate, 0x4);		break;
	case 0xe5: /* 1110 0101 */	f8_xs(cpustate, 0x5);		break;
	case 0xe6: /* 1110 0110 */	f8_xs(cpustate, 0x6);		break;
	case 0xe7: /* 1110 0111 */	f8_xs(cpustate, 0x7);		break;
	case 0xe8: /* 1110 1000 */	f8_xs(cpustate, 0x8);		break;
	case 0xe9: /* 1110 1001 */	f8_xs(cpustate, 0x9);		break;
	case 0xea: /* 1110 1010 */	f8_xs(cpustate, 0xa);		break;
	case 0xeb: /* 1110 1011 */	f8_xs(cpustate, 0xb);		break;
	case 0xec: /* 1110 1100 */	f8_xs_isar(cpustate);		break;
	case 0xed: /* 1110 1101 */	f8_xs_isar_i(cpustate);		break;
	case 0xee: /* 1110 1110 */	f8_xs_isar_d(cpustate);		break;
	case 0xef: /* 1110 1111 */	illegal(cpustate);			break;

	case 0xf0: /* 1111 0000 */	f8_ns(cpustate, 0x0);		break;
	case 0xf1: /* 1111 0001 */	f8_ns(cpustate, 0x1);		break;
	case 0xf2: /* 1111 0010 */	f8_ns(cpustate, 0x2);		break;
	case 0xf3: /* 1111 0011 */	f8_ns(cpustate, 0x3);		break;
	case 0xf4: /* 1111 0100 */	f8_ns(cpustate, 0x4);		break;
	case 0xf5: /* 1111 0101 */	f8_ns(cpustate, 0x5);		break;
	case 0xf6: /* 1111 0110 */	f8_ns(cpustate, 0x6);		break;
	case 0xf7: /* 1111 0111 */	f8_ns(cpustate, 0x7);		break;
	case 0xf8: /* 1111 1000 */	f8_ns(cpustate, 0x8);		break;
	case 0xf9: /* 1111 1001 */	f8_ns(cpustate, 0x9);		break;
	case 0xfa: /* 1111 1010 */	f8_ns(cpustate, 0xa);		break;
	case 0xfb: /* 1111 1011 */	f8_ns(cpustate, 0xb);		break;
	case 0xfc: /* 1111 1100 */	f8_ns_isar(cpustate);		break;
	case 0xfd: /* 1111 1101 */	f8_ns_isar_i(cpustate);		break;
	case 0xfe: /* 1111 1110 */	f8_ns_isar_d(cpustate);		break;
	case 0xff: /* 1111 1111 */	illegal(cpustate);			break;
        }
	switch (op) {
	case 0x0d:case 0x1b:case 0x1c:case 0x1d:
	case 0x27:case 0x28:case 0x29:
	case 0xb4:case 0xb5:case 0xb6:case 0xb7:
	case 0xb8:case 0xb9:case 0xba:case 0xbb:
	case 0xbc:case 0xbd:case 0xbe:case 0xbf:
	    ROMC_00(cpustate, cS);
	    break;
	default:
	    if (cpustate->w&I && cpustate->irq_request) {
			ROMC_1C(cpustate, cL);
			ROMC_0F(cpustate);
			ROMC_13(cpustate);
	    }
	    if((op>=0x30)&&(op<=0x3f))	/* SKR - DS is a long cycle inst */
	    	ROMC_00(cpustate, cL);
	    else
		    ROMC_00(cpustate, cS);
	    break;
	}

    } while( cpustate->icount > 0 );
}

CPU_DISASSEMBLE( f8 );

static CPU_INIT( f8 )
{
	f8_Regs *cpustate = get_safe_token(device);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->iospace = &device->space(AS_IO);

	device->save_item(NAME(cpustate->pc0));
	device->save_item(NAME(cpustate->pc1));
	device->save_item(NAME(cpustate->dc0));
	device->save_item(NAME(cpustate->dc1));
	device->save_item(NAME(cpustate->a));
	device->save_item(NAME(cpustate->w));
	device->save_item(NAME(cpustate->is));
	device->save_item(NAME(cpustate->dbus));
	device->save_item(NAME(cpustate->io));
	device->save_item(NAME(cpustate->irq_vector));
	device->save_item(NAME(cpustate->irq_request));
	device->save_item(NAME(cpustate->r));
}

static CPU_SET_INFO( f8 )
{
	f8_Regs *cpustate = get_safe_token(device);
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_SP:			cpustate->pc1 = info->i;						break;
	case CPUINFO_INT_PC:
		cpustate->pc0 = info->i;
		cpustate->dbus = cpustate->direct->read_decrypted_byte(cpustate->pc0);
    	cpustate->pc0 += 1;
		break;
	case CPUINFO_INT_PREVIOUSPC:	break;	/* TODO? */
	case CPUINFO_INT_INPUT_STATE + F8_INPUT_LINE_INT_REQ:		cpustate->irq_request = info->i;				break;
	case CPUINFO_INT_REGISTER + F8_PC0:
		cpustate->pc0 = info->i;
		cpustate->dbus = cpustate->direct->read_decrypted_byte(cpustate->pc0);
    	cpustate->pc0 += 1;
		break;
	case CPUINFO_INT_REGISTER + F8_PC1: cpustate->pc1 = info->i; break;
	case CPUINFO_INT_REGISTER + F8_DC0: cpustate->dc0 = info->i; break;
	case CPUINFO_INT_REGISTER + F8_DC1: cpustate->dc1 = info->i; break;
	case CPUINFO_INT_REGISTER + F8_W:	cpustate->w = info->i; break;
	case CPUINFO_INT_REGISTER + F8_A:	cpustate->a = info->i; break;
	case CPUINFO_INT_REGISTER + F8_IS:  cpustate->is = info->i & 0x3f; break;
	case CPUINFO_INT_REGISTER + F8_J:	cpustate->r[ 9] = info->i; break;
	case CPUINFO_INT_REGISTER + F8_HU:  cpustate->r[10] = info->i; break;
	case CPUINFO_INT_REGISTER + F8_HL:  cpustate->r[11] = info->i; break;
	case CPUINFO_INT_REGISTER + F8_KU:  cpustate->r[12] = info->i; break;
	case CPUINFO_INT_REGISTER + F8_KL:  cpustate->r[13] = info->i; break;
	case CPUINFO_INT_REGISTER + F8_QU:  cpustate->r[14] = info->i; break;
	case CPUINFO_INT_REGISTER + F8_QL:  cpustate->r[15] = info->i; break;

	case CPUINFO_INT_REGISTER + F8_R0:
	case CPUINFO_INT_REGISTER + F8_R1:
	case CPUINFO_INT_REGISTER + F8_R2:
	case CPUINFO_INT_REGISTER + F8_R3:
	case CPUINFO_INT_REGISTER + F8_R4:
	case CPUINFO_INT_REGISTER + F8_R5:
	case CPUINFO_INT_REGISTER + F8_R6:
	case CPUINFO_INT_REGISTER + F8_R7:
	case CPUINFO_INT_REGISTER + F8_R8:
		cpustate->r[state - (CPUINFO_INT_REGISTER + F8_R0)]=info->i;
		break;

	case CPUINFO_INT_REGISTER + F8_R16:
	case CPUINFO_INT_REGISTER + F8_R17:
	case CPUINFO_INT_REGISTER + F8_R18:
	case CPUINFO_INT_REGISTER + F8_R19:
	case CPUINFO_INT_REGISTER + F8_R20:
	case CPUINFO_INT_REGISTER + F8_R21:
	case CPUINFO_INT_REGISTER + F8_R22:
	case CPUINFO_INT_REGISTER + F8_R23:
	case CPUINFO_INT_REGISTER + F8_R24:
	case CPUINFO_INT_REGISTER + F8_R25:
	case CPUINFO_INT_REGISTER + F8_R26:
	case CPUINFO_INT_REGISTER + F8_R27:
	case CPUINFO_INT_REGISTER + F8_R28:
	case CPUINFO_INT_REGISTER + F8_R29:
	case CPUINFO_INT_REGISTER + F8_R30:
	case CPUINFO_INT_REGISTER + F8_R31:
	case CPUINFO_INT_REGISTER + F8_R32:
	case CPUINFO_INT_REGISTER + F8_R33:
	case CPUINFO_INT_REGISTER + F8_R34:
	case CPUINFO_INT_REGISTER + F8_R35:
	case CPUINFO_INT_REGISTER + F8_R36:
	case CPUINFO_INT_REGISTER + F8_R37:
	case CPUINFO_INT_REGISTER + F8_R38:
	case CPUINFO_INT_REGISTER + F8_R39:
	case CPUINFO_INT_REGISTER + F8_R40:
	case CPUINFO_INT_REGISTER + F8_R41:
	case CPUINFO_INT_REGISTER + F8_R42:
	case CPUINFO_INT_REGISTER + F8_R43:
	case CPUINFO_INT_REGISTER + F8_R44:
	case CPUINFO_INT_REGISTER + F8_R45:
	case CPUINFO_INT_REGISTER + F8_R46:
	case CPUINFO_INT_REGISTER + F8_R47:
	case CPUINFO_INT_REGISTER + F8_R48:
	case CPUINFO_INT_REGISTER + F8_R49:
	case CPUINFO_INT_REGISTER + F8_R50:
	case CPUINFO_INT_REGISTER + F8_R51:
	case CPUINFO_INT_REGISTER + F8_R52:
	case CPUINFO_INT_REGISTER + F8_R53:
	case CPUINFO_INT_REGISTER + F8_R54:
	case CPUINFO_INT_REGISTER + F8_R55:
	case CPUINFO_INT_REGISTER + F8_R56:
	case CPUINFO_INT_REGISTER + F8_R57:
	case CPUINFO_INT_REGISTER + F8_R58:
	case CPUINFO_INT_REGISTER + F8_R59:
	case CPUINFO_INT_REGISTER + F8_R60:
	case CPUINFO_INT_REGISTER + F8_R61:
	case CPUINFO_INT_REGISTER + F8_R62:
	case CPUINFO_INT_REGISTER + F8_R63:
		cpustate->r[state - (CPUINFO_INT_REGISTER + F8_R16) + 16]=info->i;
		break;
	}
	return;
}

CPU_GET_INFO( f8 )
{
	f8_Regs *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(f8_Regs);	break;
	case CPUINFO_INT_INPUT_LINES:						info->i = 1;			break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;			break;
	case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;	break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;			break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;			break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;			break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;			break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1;			break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 7;			break;

	case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;	break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;	break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;	break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;	break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;	break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 8;	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;	break;

	case CPUINFO_INT_SP:				info->i = cpustate->pc1;					break;
	case CPUINFO_INT_PC:				info->i = (cpustate->pc0 - 1) & 0xffff;	break;
	case CPUINFO_INT_PREVIOUSPC:		info->i = 0;	/* TODO??? */		break;

	case CPUINFO_INT_INPUT_STATE + F8_INPUT_LINE_INT_REQ:			info->i = cpustate->irq_request;			break;

	case CPUINFO_INT_REGISTER + F8_PC0:	info->i = (cpustate->pc0 - 1) & 0xffff;	break;
	case CPUINFO_INT_REGISTER + F8_PC1: info->i = cpustate->pc1;					break;
	case CPUINFO_INT_REGISTER + F8_DC0: info->i = cpustate->dc0;					break;
	case CPUINFO_INT_REGISTER + F8_DC1: info->i = cpustate->dc1;					break;
    case CPUINFO_INT_REGISTER + F8_W:	info->i = cpustate->w;						break;
	case CPUINFO_INT_REGISTER + F8_A:	info->i = cpustate->a;						break;
	case CPUINFO_INT_REGISTER + F8_IS:	info->i = cpustate->is;					break;
	case CPUINFO_INT_REGISTER + F8_J:	info->i = cpustate->r[ 9];					break;
	case CPUINFO_INT_REGISTER + F8_HU:  info->i = cpustate->r[10];					break;
	case CPUINFO_INT_REGISTER + F8_HL:  info->i = cpustate->r[11];					break;
	case CPUINFO_INT_REGISTER + F8_KU:  info->i = cpustate->r[12];					break;
	case CPUINFO_INT_REGISTER + F8_KL:  info->i = cpustate->r[13];					break;
	case CPUINFO_INT_REGISTER + F8_QU:  info->i = cpustate->r[14];					break;
	case CPUINFO_INT_REGISTER + F8_QL:  info->i = cpustate->r[15];					break;

    case CPUINFO_INT_REGISTER + F8_R0:
    case CPUINFO_INT_REGISTER + F8_R1:
    case CPUINFO_INT_REGISTER + F8_R2:
    case CPUINFO_INT_REGISTER + F8_R3:
    case CPUINFO_INT_REGISTER + F8_R4:
    case CPUINFO_INT_REGISTER + F8_R5:
    case CPUINFO_INT_REGISTER + F8_R6:
    case CPUINFO_INT_REGISTER + F8_R7:
    case CPUINFO_INT_REGISTER + F8_R8:
    	info->i = cpustate->r[state - (CPUINFO_INT_REGISTER + F8_R0)];	break;


    case CPUINFO_INT_REGISTER + F8_R16:
    case CPUINFO_INT_REGISTER + F8_R17:
    case CPUINFO_INT_REGISTER + F8_R18:
    case CPUINFO_INT_REGISTER + F8_R19:
    case CPUINFO_INT_REGISTER + F8_R20:
    case CPUINFO_INT_REGISTER + F8_R21:
    case CPUINFO_INT_REGISTER + F8_R22:
    case CPUINFO_INT_REGISTER + F8_R23:
    case CPUINFO_INT_REGISTER + F8_R24:
    case CPUINFO_INT_REGISTER + F8_R25:
    case CPUINFO_INT_REGISTER + F8_R26:
    case CPUINFO_INT_REGISTER + F8_R27:
    case CPUINFO_INT_REGISTER + F8_R28:
    case CPUINFO_INT_REGISTER + F8_R29:
    case CPUINFO_INT_REGISTER + F8_R30:
    case CPUINFO_INT_REGISTER + F8_R31:
    case CPUINFO_INT_REGISTER + F8_R32:
    case CPUINFO_INT_REGISTER + F8_R33:
    case CPUINFO_INT_REGISTER + F8_R34:
    case CPUINFO_INT_REGISTER + F8_R35:
    case CPUINFO_INT_REGISTER + F8_R36:
    case CPUINFO_INT_REGISTER + F8_R37:
    case CPUINFO_INT_REGISTER + F8_R38:
    case CPUINFO_INT_REGISTER + F8_R39:
    case CPUINFO_INT_REGISTER + F8_R40:
    case CPUINFO_INT_REGISTER + F8_R41:
    case CPUINFO_INT_REGISTER + F8_R42:
    case CPUINFO_INT_REGISTER + F8_R43:
    case CPUINFO_INT_REGISTER + F8_R44:
    case CPUINFO_INT_REGISTER + F8_R45:
    case CPUINFO_INT_REGISTER + F8_R46:
    case CPUINFO_INT_REGISTER + F8_R47:
    case CPUINFO_INT_REGISTER + F8_R48:
    case CPUINFO_INT_REGISTER + F8_R49:
    case CPUINFO_INT_REGISTER + F8_R50:
    case CPUINFO_INT_REGISTER + F8_R51:
    case CPUINFO_INT_REGISTER + F8_R52:
    case CPUINFO_INT_REGISTER + F8_R53:
    case CPUINFO_INT_REGISTER + F8_R54:
    case CPUINFO_INT_REGISTER + F8_R55:
    case CPUINFO_INT_REGISTER + F8_R56:
    case CPUINFO_INT_REGISTER + F8_R57:
    case CPUINFO_INT_REGISTER + F8_R58:
    case CPUINFO_INT_REGISTER + F8_R59:
    case CPUINFO_INT_REGISTER + F8_R60:
    case CPUINFO_INT_REGISTER + F8_R61:
    case CPUINFO_INT_REGISTER + F8_R62:
    case CPUINFO_INT_REGISTER + F8_R63:
    	info->i = cpustate->r[state - (CPUINFO_INT_REGISTER + F8_R16) + 16];	break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(f8);			break;
	case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(f8);					break;
	case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(f8);					break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(f8);				break;
	case CPUINFO_FCT_BURN:							info->burn = NULL;						break;

	case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(f8);			break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:			strcpy(info->s, "F8");			break;
	case CPUINFO_STR_FAMILY:	strcpy(info->s, "Fairchild F8");	break;
	case CPUINFO_STR_VERSION:	strcpy(info->s, "1.0");			break;
	case CPUINFO_STR_SOURCE_FILE:		strcpy(info->s, __FILE__);		break;
	case CPUINFO_STR_CREDITS:	strcpy(info->s,
									"Copyright Juergen Buchmueller, all rights reserved.");
									break;

    case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c",
				cpustate->w & 0x10 ? 'I':'.',
				cpustate->w & 0x08 ? 'O':'.',
				cpustate->w & 0x04 ? 'Z':'.',
				cpustate->w & 0x02 ? 'C':'.',
				cpustate->w & 0x01 ? 'S':'.');
			break;

	case CPUINFO_STR_REGISTER+F8_PC0:sprintf(info->s, "PC0:%04X", ((cpustate->pc0) - 1) & 0xffff); break;
	case CPUINFO_STR_REGISTER+F8_PC1:sprintf(info->s, "PC1:%04X", cpustate->pc1); break;
	case CPUINFO_STR_REGISTER+F8_DC0:sprintf(info->s, "DC0:%04X", cpustate->dc0); break;
	case CPUINFO_STR_REGISTER+F8_DC1:sprintf(info->s, "DC1:%04X", cpustate->dc1); break;
	case CPUINFO_STR_REGISTER+F8_W:  sprintf(info->s, "W  :%02X", cpustate->w); break;
	case CPUINFO_STR_REGISTER+F8_A:  sprintf(info->s, "A  :%02X", cpustate->a); break;
	case CPUINFO_STR_REGISTER+F8_IS: sprintf(info->s, "IS :%02X", cpustate->is); break;
	case CPUINFO_STR_REGISTER+F8_J:  sprintf(info->s, "J  :%02X", cpustate->r[9]); break;
	case CPUINFO_STR_REGISTER+F8_HU: sprintf(info->s, "HU :%02X", cpustate->r[10]); break;
	case CPUINFO_STR_REGISTER+F8_HL: sprintf(info->s, "HL :%02X", cpustate->r[11]); break;
	case CPUINFO_STR_REGISTER+F8_KU: sprintf(info->s, "KU :%02X", cpustate->r[12]); break;
	case CPUINFO_STR_REGISTER+F8_KL: sprintf(info->s, "KL :%02X", cpustate->r[13]); break;
	case CPUINFO_STR_REGISTER+F8_QU: sprintf(info->s, "QU :%02X", cpustate->r[14]); break;
	case CPUINFO_STR_REGISTER+F8_QL: sprintf(info->s, "QL :%02X", cpustate->r[15]); break;
	case CPUINFO_STR_REGISTER+F8_R0: sprintf(info->s, "R0 :%02X", cpustate->r[0]); break;
	case CPUINFO_STR_REGISTER+F8_R1: sprintf(info->s, "R1 :%02X", cpustate->r[1]); break;
	case CPUINFO_STR_REGISTER+F8_R2: sprintf(info->s, "R2 :%02X", cpustate->r[2]); break;
	case CPUINFO_STR_REGISTER+F8_R3: sprintf(info->s, "R3 :%02X", cpustate->r[3]); break;
	case CPUINFO_STR_REGISTER+F8_R4: sprintf(info->s, "R4 :%02X", cpustate->r[4]); break;
	case CPUINFO_STR_REGISTER+F8_R5: sprintf(info->s, "R5 :%02X", cpustate->r[5]); break;
	case CPUINFO_STR_REGISTER+F8_R6: sprintf(info->s, "R6 :%02X", cpustate->r[6]); break;
	case CPUINFO_STR_REGISTER+F8_R7: sprintf(info->s, "R7 :%02X", cpustate->r[7]); break;
	case CPUINFO_STR_REGISTER+F8_R8: sprintf(info->s, "R8 :%02X", cpustate->r[8]); break;
	case CPUINFO_STR_REGISTER+F8_R16: sprintf(info->s, "R16 :%02X", cpustate->r[16]); break;
	case CPUINFO_STR_REGISTER+F8_R17: sprintf(info->s, "R17 :%02X", cpustate->r[17]); break;
	case CPUINFO_STR_REGISTER+F8_R18: sprintf(info->s, "R18 :%02X", cpustate->r[18]); break;
	case CPUINFO_STR_REGISTER+F8_R19: sprintf(info->s, "R19 :%02X", cpustate->r[19]); break;
	case CPUINFO_STR_REGISTER+F8_R20: sprintf(info->s, "R20 :%02X", cpustate->r[20]); break;
	case CPUINFO_STR_REGISTER+F8_R21: sprintf(info->s, "R21 :%02X", cpustate->r[21]); break;
	case CPUINFO_STR_REGISTER+F8_R22: sprintf(info->s, "R22 :%02X", cpustate->r[22]); break;
	case CPUINFO_STR_REGISTER+F8_R23: sprintf(info->s, "R23 :%02X", cpustate->r[23]); break;
	case CPUINFO_STR_REGISTER+F8_R24: sprintf(info->s, "R24 :%02X", cpustate->r[24]); break;
	case CPUINFO_STR_REGISTER+F8_R25: sprintf(info->s, "R25 :%02X", cpustate->r[25]); break;
	case CPUINFO_STR_REGISTER+F8_R26: sprintf(info->s, "R26 :%02X", cpustate->r[26]); break;
	case CPUINFO_STR_REGISTER+F8_R27: sprintf(info->s, "R27 :%02X", cpustate->r[27]); break;
	case CPUINFO_STR_REGISTER+F8_R28: sprintf(info->s, "R28 :%02X", cpustate->r[28]); break;
	case CPUINFO_STR_REGISTER+F8_R29: sprintf(info->s, "R29 :%02X", cpustate->r[29]); break;
	case CPUINFO_STR_REGISTER+F8_R30: sprintf(info->s, "R30 :%02X", cpustate->r[30]); break;
	case CPUINFO_STR_REGISTER+F8_R31: sprintf(info->s, "R31 :%02X", cpustate->r[31]); break;
	case CPUINFO_STR_REGISTER+F8_R32: sprintf(info->s, "R32 :%02X", cpustate->r[32]); break;
	case CPUINFO_STR_REGISTER+F8_R33: sprintf(info->s, "R33 :%02X", cpustate->r[33]); break;
	case CPUINFO_STR_REGISTER+F8_R34: sprintf(info->s, "R34 :%02X", cpustate->r[34]); break;
	case CPUINFO_STR_REGISTER+F8_R35: sprintf(info->s, "R35 :%02X", cpustate->r[35]); break;
	case CPUINFO_STR_REGISTER+F8_R36: sprintf(info->s, "R36 :%02X", cpustate->r[36]); break;
	case CPUINFO_STR_REGISTER+F8_R37: sprintf(info->s, "R37 :%02X", cpustate->r[37]); break;
	case CPUINFO_STR_REGISTER+F8_R38: sprintf(info->s, "R38 :%02X", cpustate->r[38]); break;
	case CPUINFO_STR_REGISTER+F8_R39: sprintf(info->s, "R39 :%02X", cpustate->r[39]); break;
	case CPUINFO_STR_REGISTER+F8_R40: sprintf(info->s, "R40 :%02X", cpustate->r[40]); break;
	case CPUINFO_STR_REGISTER+F8_R41: sprintf(info->s, "R41 :%02X", cpustate->r[41]); break;
	case CPUINFO_STR_REGISTER+F8_R42: sprintf(info->s, "R42 :%02X", cpustate->r[42]); break;
	case CPUINFO_STR_REGISTER+F8_R43: sprintf(info->s, "R43 :%02X", cpustate->r[43]); break;
	case CPUINFO_STR_REGISTER+F8_R44: sprintf(info->s, "R44 :%02X", cpustate->r[44]); break;
	case CPUINFO_STR_REGISTER+F8_R45: sprintf(info->s, "R45 :%02X", cpustate->r[45]); break;
	case CPUINFO_STR_REGISTER+F8_R46: sprintf(info->s, "R46 :%02X", cpustate->r[46]); break;
	case CPUINFO_STR_REGISTER+F8_R47: sprintf(info->s, "R47 :%02X", cpustate->r[47]); break;
	case CPUINFO_STR_REGISTER+F8_R48: sprintf(info->s, "R48 :%02X", cpustate->r[48]); break;
	case CPUINFO_STR_REGISTER+F8_R49: sprintf(info->s, "R49 :%02X", cpustate->r[49]); break;
	case CPUINFO_STR_REGISTER+F8_R50: sprintf(info->s, "R50 :%02X", cpustate->r[50]); break;
	case CPUINFO_STR_REGISTER+F8_R51: sprintf(info->s, "R51 :%02X", cpustate->r[51]); break;
	case CPUINFO_STR_REGISTER+F8_R52: sprintf(info->s, "R52 :%02X", cpustate->r[52]); break;
	case CPUINFO_STR_REGISTER+F8_R53: sprintf(info->s, "R53 :%02X", cpustate->r[53]); break;
	case CPUINFO_STR_REGISTER+F8_R54: sprintf(info->s, "R54 :%02X", cpustate->r[54]); break;
	case CPUINFO_STR_REGISTER+F8_R55: sprintf(info->s, "R55 :%02X", cpustate->r[55]); break;
	case CPUINFO_STR_REGISTER+F8_R56: sprintf(info->s, "R56 :%02X", cpustate->r[56]); break;
	case CPUINFO_STR_REGISTER+F8_R57: sprintf(info->s, "R57 :%02X", cpustate->r[57]); break;
	case CPUINFO_STR_REGISTER+F8_R58: sprintf(info->s, "R58 :%02X", cpustate->r[58]); break;
	case CPUINFO_STR_REGISTER+F8_R59: sprintf(info->s, "R59 :%02X", cpustate->r[59]); break;
	case CPUINFO_STR_REGISTER+F8_R60: sprintf(info->s, "R60 :%02X", cpustate->r[60]); break;
	case CPUINFO_STR_REGISTER+F8_R61: sprintf(info->s, "R61 :%02X", cpustate->r[61]); break;
	case CPUINFO_STR_REGISTER+F8_R62: sprintf(info->s, "R62 :%02X", cpustate->r[62]); break;
	case CPUINFO_STR_REGISTER+F8_R63: sprintf(info->s, "R63 :%02X", cpustate->r[63]); break;

	}

	return;
}

DEFINE_LEGACY_CPU_DEVICE(F8, f8);
