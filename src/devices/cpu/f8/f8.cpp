// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   f8.c
 *   Portable F8 emulator (Fairchild 3850)
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

#define S   0x01
#define C   0x02
#define Z   0x04
#define O   0x08
#define I   0x10

#define cS  4
#define cL  6


/* clear all flags */
#define CLR_OZCS                \
	m_w &= ~(O|Z|C|S)

/* set sign and zero flags (note: the S flag is complementary) */
#define SET_SZ(n)               \
	if (n == 0)                 \
		m_w |= Z | S;           \
	else                        \
	if (n < 128)                \
		m_w |= S

/* set overflow and carry flags */
#define SET_OC(n,m)             \
	if (n + m > 255)            \
		m_w |= C;               \
	if ((n&127)+(m&127) > 127)  \
	{                           \
		if (!(m_w & C))     \
			m_w |= O;           \
	}                           \
	else                        \
	{                           \
		if (m_w & C)            \
			m_w |= O;           \
	}


const device_type F8 = &device_creator<f8_cpu_device>;


f8_cpu_device::f8_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, F8, "Fairchild F8", tag, owner, clock, "f8", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_BIG, 8, 8, 0)
	, m_pc0(0)
	, m_pc1(0)
	, m_dc0(0)
	, m_dc1(0)
	, m_a(0)
	, m_w(0)
	, m_is(0)
	, m_pc(0)
{
	memset(m_r, 0x00, sizeof(m_r));
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
void f8_cpu_device::ROMC_00(int insttim) /* SKR - added parameter to tell if  */
									/* it is long or short based on inst */
{
	/*
	 * Instruction Fetch. The device whose address space includes the
	 * contents of the PC0 register must place on the data bus the op
	 * code addressed by PC0; then all devices increment the contents
	 * of PC0.
	 */

	m_dbus = m_direct->read_byte(m_pc0);
	m_pc0 += 1;
	m_icount -= insttim;    /* SKR - ROMC00 is usually short, not short+long, */
							/* but DS is long */
}

void f8_cpu_device::ROMC_01()
{
	/*
	 * The device whose address space includes the contents of the PC0
	 * register must place on the data bus the contents of the memory
	 * location addressed by PC0; then all devices add the 8-bit value
	 * on the data bus as signed binary number to PC0.
	 */
	m_dbus = m_direct->read_byte(m_pc0);
	m_pc0 += (INT8)m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_02()
{
	/*
	 * The device whose DC0 addresses a memory word within the address
	 * space of that device must place on the data bus the contents of
	 * the memory location addressed by DC0; then all devices increment
	 * DC0.
	 */
	m_dbus = m_program->read_byte(m_dc0);
	m_dc0 += 1;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_03(int insttim) /* SKR - added parameter to tell if  */
{                                   /* it is long or short based on inst */
	/*
	 * Similiar to 0x00, except that it is used for immediate operands
	 * fetches (using PC0) instead of instruction fetches.
	 */
	m_dbus = m_io = m_direct->read_byte(m_pc0);
	m_pc0 += 1;
	m_icount -= insttim;
}

void f8_cpu_device::ROMC_04()
{
	/*
	 * Copy the contents of PC1 into PC0
	 */
	m_pc0 = m_pc1;
	m_icount -= cS;
}

void f8_cpu_device::ROMC_05()
{
	/*
	 * Store the data bus contents into the memory location pointed
	 * to by DC0; increment DC0.
	 */
	m_program->write_byte(m_dc0, m_dbus);
	m_dc0 += 1;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_06()
{
	/*
	 * Place the high order byte of DC0 on the data bus.
	 */
	m_dbus = m_dc0 >> 8;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_07()
{
	/*
	 * Place the high order byte of PC1 on the data bus.
	 */
	m_dbus = m_pc1 >> 8;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_08()
{
	/*
	 * All devices copy the contents of PC0 into PC1. The CPU outputs
	 * zero on the data bus in this ROMC state. Load the data bus into
	 * both halves of PC0, thus clearing the register.
	 */
	m_pc1 = m_pc0;
	m_dbus = 0;
	m_pc0 = 0;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_09()
{
	/*
	 * The device whose address space includes the contents of the DC0
	 * register must place the low order byte of DC0 onto the data bus.
	 */
	m_dbus = m_dc0 & 0xff;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0A()
{
	/*
	 * All devices add the 8-bit value on the data bus, treated as
	 * signed binary number, to the data counter.
	 */
	m_dc0 += (INT8)m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0B()
{
	/*
	 * The device whose address space includes the value in PC1
	 * must place the low order byte of PC1 onto the data bus.
	 */
	m_dbus = m_pc1 & 0xff;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0C()
{
	/*
	 * The device whose address space includes the contents of the PC0
	 * register must place the contents of the memory word addressed
	 * by PC0 into the data bus; then all devices move the value that
	 * has just been placed on the data bus into the low order byte of PC0.
	 */
	m_dbus = m_direct->read_byte(m_pc0);
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0D()
{
	/*
	 * All devices store in PC1 the current contents of PC0, incremented
	 * by 1; PC0 is unaltered.
	 */
	m_pc1 = m_pc0 + 1;
	m_icount -= cS;
}

void f8_cpu_device::ROMC_0E()
{
	/*
	 * The device whose address space includes the contents of the PC0
	 * register must place the word addressed by PC0 into the data bus.
	 * The value on the data bus is then moved to the low order byte
	 * of DC0 by all devices.
	 */
	m_dbus = m_direct->read_byte(m_pc0);
	m_dc0 = (m_dc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_0F()
{
	/*
	 * The interrupting device with highest priority must place the
	 * low order byte of the interrupt vector on the data bus.
	 * All devices must copy the contents of PC0 into PC1. All devices
	 * must move the contents of the data bus into the low order
	 * byte of PC0.
	 */
	m_irq_vector = standard_irq_callback(F8_INPUT_LINE_INT_REQ);
	m_dbus = m_irq_vector & 0x00ff;
	m_pc1 = m_pc0;
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_10()
{
	/*
	 * Inhibit any modification to the interrupt priority logic.
	 */
	m_w |= 0x20;   /* ???? */
	m_icount -= cL;
}

void f8_cpu_device::ROMC_11()
{
	/*
	 * The device whose address space includes the contents of PC0
	 * must place the contents of the addressed memory word on the
	 * data bus. All devices must then move the contents of the
	 * data bus to the upper byte of DC0.
	 */
	m_dbus = m_direct->read_byte(m_pc0);
	m_dc0 = (m_dc0 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_12()
{
	/*
	 * All devices copy the contents of PC0 into PC1. All devices then
	 * move the contents of the data bus into the low order byte of PC0.
	 */
	m_pc1 = m_pc0;
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_13()
{
	/*
	 * The interrupting device with highest priority must move the high
	 * order half of the interrupt vector onto the data bus. All devices
	 * must then move the contents of the data bus into the high order
	 * byte of PC0. The interrupting device resets its interrupt circuitry
	 * (so that it is no longer requesting CPU servicing and can respond
	 * to another interrupt).
	 */
	m_dbus = m_irq_vector >> 8;
	m_pc0 = (m_pc0 & 0x00ff) | (m_dbus << 8);
	m_w&=~I;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_14()
{
	/*
	 * All devices move the contents of the data bus into the high
	 * order byte of PC0.
	 */
	m_pc0 = (m_pc0 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_15()
{
	/*
	 * All devices move the contents of the data bus into the high
	 * order byte of PC1.
	 */
	m_pc1 = (m_pc1 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_16()
{
	/*
	 * All devices move the contents of the data bus into the high
	 * order byte of DC0.
	 */
	m_dc0 = (m_dc0 & 0x00ff) | (m_dbus << 8);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_17()
{
	/*
	 * All devices move the contents of the data bus into the low
	 * order byte of PC0.
	 */
	m_pc0 = (m_pc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_18()
{
	/*
	 * All devices move the contents of the data bus into the low
	 * order byte of PC1.
	 */
	m_pc1 = (m_pc1 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_19()
{
	/*
	 * All devices move the contents of the data bus into the low
	 * order byte of DC0.
	 */
	m_dc0 = (m_dc0 & 0xff00) | m_dbus;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1A()
{
	/*
	 * During the prior cycle, an I/O port timer or interrupt control
	 * register was addressed; the device containing the addressed port
	 * must place the contents of the data bus into the address port.
	 */
	m_iospace->write_byte(m_io, m_dbus);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1B()
{
	/*
	 * During the prior cycle, the data bus specified the address of an
	 * I/O port. The device containing the addressed I/O port must place
	 * the contents of the I/O port on the data bus. (Note that the
	 * contents of timer and interrupt control registers cannot be read
	 * back onto the data bus).
	 */
	m_dbus = m_iospace->read_byte(m_io);
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1C(int insttim) /* SKR - added parameter to tell if  */
									/* it is long or short based on inst */
{
	/*
	 * None.
	 */
	m_icount -= insttim;
}

void f8_cpu_device::ROMC_1D()
{
	/*
	 * Devices with DC0 and DC1 registers must switch registers.
	 * Devices without a DC1 register perform no operation.
	 */
	UINT16 tmp = m_dc0;
	m_dc0 = m_dc1;
	m_dc1 = tmp;
	m_icount -= cS;
}

void f8_cpu_device::ROMC_1E()
{
	/*
	 * The devices whose address space includes the contents of PC0
	 * must place the low order byte of PC0 onto the data bus.
	 */
	m_dbus = m_pc0 & 0xff;
	m_icount -= cL;
}

void f8_cpu_device::ROMC_1F()
{
	/*
	 * The devices whose address space includes the contents of PC0
	 * must place the high order byte of PC0 onto the data bus.
	 */
	m_dbus = (m_pc0 >> 8) & 0xff;
	m_icount -= cL;
}

/***********************************
 *  illegal opcodes
 ***********************************/
void f8_cpu_device::illegal()
{
	logerror("f8 illegal opcode at 0x%04x: %02x\n", m_pc0, m_dbus);
}

/***************************************************
 *  O Z C S 0000 0000
 *  - - - - LR  A,KU
 ***************************************************/
void f8_cpu_device::f8_lr_a_ku()
{
	m_a = m_r[12];
}

/***************************************************
 *  O Z C S 0000 0001
 *  - - - - LR  A,KL
 ***************************************************/
void f8_cpu_device::f8_lr_a_kl()
{
	m_a = m_r[13];
}

/***************************************************
 *  O Z C S 0000 0010
 *  - - - - LR  A,QU
 ***************************************************/
void f8_cpu_device::f8_lr_a_qu()
{
	m_a = m_r[14];
}

/***************************************************
 *  O Z C S 0000 0011
 *  - - - - LR  A,QL
 ***************************************************/
void f8_cpu_device::f8_lr_a_ql()
{
	m_a = m_r[15];
}

/***************************************************
 *  O Z C S 0000 0100
 *  - - - - LR  KU,A
 ***************************************************/
void f8_cpu_device::f8_lr_ku_a()
{
	m_r[12] = m_a;
}

/***************************************************
 *  O Z C S 0000 0101
 *  - - - - LR  KL,A
 ***************************************************/
void f8_cpu_device::f8_lr_kl_a()
{
	m_r[13] = m_a;
}

/***************************************************
 *  O Z C S 0000 0110
 *  - - - - LR  QU,A
 ***************************************************/
void f8_cpu_device::f8_lr_qu_a()
{
	m_r[14] = m_a;
}

/***************************************************
 *  O Z C S 0000 0111
 *  - - - - LR  QL,A
 ***************************************************/
void f8_cpu_device::f8_lr_ql_a()
{
	m_r[15] = m_a;
}

/***************************************************
 *  O Z C S 0000 1000
 *  - - - - LR  K,P
 ***************************************************/
void f8_cpu_device::f8_lr_k_p()
{
	ROMC_07();
	m_r[12] = m_dbus;
	ROMC_0B();
	m_r[13] = m_dbus;
}

/***************************************************
 *  O Z C S 0000 1001
 *  - - - - LR  P,K
 ***************************************************/
void f8_cpu_device::f8_lr_p_k()
{
	m_dbus = m_r[12];
	ROMC_15();
	m_dbus = m_r[13];
	ROMC_18();
}

/***************************************************
 *  O Z C S 0000 1010
 *  - - - - LR  A,IS
 ***************************************************/
void f8_cpu_device::f8_lr_a_is()
{
	m_a = m_is;
}

/***************************************************
 *  O Z C S 0000 1011
 *  - - - - LR  IS,A
 ***************************************************/
void f8_cpu_device::f8_lr_is_a()
{
	m_is = m_a & 0x3f;
}

/***************************************************
 *  O Z C S 0000 1100
 *  - - - - PK
 ***************************************************/
void f8_cpu_device::f8_pk()
{
	m_dbus = m_r[13];
	ROMC_12();
	m_dbus = m_r[12];
	ROMC_14();
}

/***************************************************
 *  O Z C S 0000 1101
 *  - - - - LR  P0,Q
 ***************************************************/
void f8_cpu_device::f8_lr_p0_q()
{
	m_dbus = m_r[15];
	ROMC_17();
	m_dbus = m_r[14];
	ROMC_14();
}

/***************************************************
 *  O Z C S 0000 1110
 *  - - - - LR   Q,DC
 ***************************************************/
void f8_cpu_device::f8_lr_q_dc()
{
	ROMC_06();
	m_r[14] = m_dbus;
	ROMC_09();
	m_r[15] = m_dbus;
}

/***************************************************
 *  O Z C S 0000 1111
 *  - - - - LR   DC,Q
 ***************************************************/
void f8_cpu_device::f8_lr_dc_q()
{
	m_dbus = m_r[14];
	ROMC_16();
	m_dbus = m_r[15];
	ROMC_19();
}

/***************************************************
 *  O Z C S 0001 0000
 *  - - - - LR   DC,H
 ***************************************************/
void f8_cpu_device::f8_lr_dc_h()
{
	m_dbus = m_r[10];
	ROMC_16();
	m_dbus = m_r[11];
	ROMC_19();
}

/***************************************************
 *  O Z C S 0001 0001
 *  - - - - LR   H,DC
 ***************************************************/
void f8_cpu_device::f8_lr_h_dc()
{
	ROMC_06();
	m_r[10] = m_dbus;
	ROMC_09();
	m_r[11] = m_dbus;
}

/***************************************************
 *  O Z C S 0001 0010
 *  0 x 0 1 SR   1
 ***************************************************/
void f8_cpu_device::f8_sr_1()
{
	m_a >>= 1;
	CLR_OZCS;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0011
 *  0 x 0 x SL   1
 ***************************************************/
void f8_cpu_device::f8_sl_1()
{
	m_a <<= 1;
	CLR_OZCS;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0100
 *  0 x 0 1 SR   4
 ***************************************************/
void f8_cpu_device::f8_sr_4()
{
	m_a >>= 4;
	CLR_OZCS;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0101
 *  0 x 0 x SL   4
 ***************************************************/
void f8_cpu_device::f8_sl_4()
{
	m_a <<= 4;
	CLR_OZCS;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 0110
 *  - - - - LM
 ***************************************************/
void f8_cpu_device::f8_lm()
{
	ROMC_02();
	m_a = m_dbus;
}

/***************************************************
 *  O Z C S 0001 0111
 *  - - - - ST
 ***************************************************/
void f8_cpu_device::f8_st()
{
	m_dbus = m_a;
	ROMC_05();
}

/***************************************************
 *  O Z C S 0001 1000
 *  0 x 0 x COM
 ***************************************************/
void f8_cpu_device::f8_com()
{
	m_a = ~m_a;
	CLR_OZCS;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 1001
 *  x x x x LNK
 ***************************************************/
void f8_cpu_device::f8_lnk()
{
	if (m_w & C)
	{
	CLR_OZCS;
	SET_OC(m_a,1);
	m_a += 1;
	}
	else
	{
	CLR_OZCS;
	}
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0001 1010
 *          DI
 ***************************************************/
void f8_cpu_device::f8_di()
{
	ROMC_1C(cS);
	m_w &= ~I;
}

/***************************************************
 *  O Z C S 0001 1011
 *          EI
 ***************************************************/
void f8_cpu_device::f8_ei()
{
	ROMC_1C(cS);
	m_w |= I;
}

/***************************************************
 *  O Z C S 0001 1100
 *          POP
 ***************************************************/
void f8_cpu_device::f8_pop()
{
	ROMC_04();
}

/***************************************************
 *  O Z C S 0001 1101
 *  x x x x LR   W,J
 ***************************************************/
void f8_cpu_device::f8_lr_w_j()
{
	ROMC_1C(cS);
	m_w = m_r[9];
}

/***************************************************
 *  O Z C S 0001 1110
 *  - - - - LR   J,W
 ***************************************************/
void f8_cpu_device::f8_lr_j_w()
{
	m_r[9] = m_w;
}

/***************************************************
 *  O Z C S 0001 1111
 *  x x x x INC
 ***************************************************/
void f8_cpu_device::f8_inc()
{
	CLR_OZCS;
	SET_OC(m_a,1);
	m_a += 1;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0000   aaaa aaaa
 *  - - - - LI  aa
 ***************************************************/
void f8_cpu_device::f8_li()
{
	ROMC_03(cL);
	m_a = m_dbus;
}

/***************************************************
 *  O Z C S 0010 0001   aaaa aaaa
 *  0 x 0 x NI   aa
 ***************************************************/
void f8_cpu_device::f8_ni()
{
	ROMC_03(cL);
	CLR_OZCS;
	m_a &= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0010   aaaa aaaa
 *  0 x 0 x OI   aa
 ***************************************************/
void f8_cpu_device::f8_oi()
{
	ROMC_03(cL);
	CLR_OZCS;
	m_a |= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0011   aaaa aaaa
 *  0 x 0 x XI   aa
 ***************************************************/
void f8_cpu_device::f8_xi()
{
	ROMC_03(cL);
	CLR_OZCS;
	m_a ^= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0100   aaaa aaaa
 *  x x x x AI   aa
 ***************************************************/
void f8_cpu_device::f8_ai()
{
	ROMC_03(cL);
	CLR_OZCS;
	SET_OC(m_a,m_dbus);
	m_a += m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0101   aaaa aaaa
 *  x x x x CI   aa
 ***************************************************/
void f8_cpu_device::f8_ci()
{
	UINT16 tmp = ((UINT8)~m_a) + 1;
	ROMC_03(cL);
	CLR_OZCS;
	SET_OC(tmp,m_dbus);
	tmp += m_dbus;
	SET_SZ((UINT8)tmp);
}

/***************************************************
 *  O Z C S 0010 0110   aaaa aaaa
 *  0 x 0 x IN   aa
 ***************************************************/
void f8_cpu_device::f8_in()
{
	ROMC_03(cL);
	CLR_OZCS;
	ROMC_1B();
	m_a = m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 0010 0111   aaaa aaaa
 *  - - - - OUT  aa
 ***************************************************/
void f8_cpu_device::f8_out()
{
	ROMC_03(cL);
	m_dbus = m_a;
	ROMC_1A();
}

/***************************************************
 *  O Z C S 0010 1000   iiii iiii   jjjj jjjj
 *  - - - - PI   iijj
 ***************************************************/
void f8_cpu_device::f8_pi()
{
	ROMC_03(cL);
	m_a = m_dbus;
	ROMC_0D();
	ROMC_0C();
	m_dbus = m_a;
	ROMC_14();
}

/***************************************************
 *  O Z C S 0010 1001   iiii iiii   jjjj jjjj
 *  - - - - JMP  iijj
 ***************************************************/
void f8_cpu_device::f8_jmp()
{
	ROMC_03(cL);
	m_a = m_dbus;
	ROMC_0C();
	m_dbus = m_a;
	ROMC_14();
}

/***************************************************
 *  O Z C S 0010 1010   iiii iiii   jjjj jjjj
 *  - - - - DCI  iijj
 ***************************************************/
void f8_cpu_device::f8_dci()
{
	ROMC_11();
	ROMC_03(cS);
	ROMC_0E();
	ROMC_03(cS);
}

/***************************************************
 *  O Z C S 0010 1011
 *  - - - - NOP
 ***************************************************/
void f8_cpu_device::f8_nop()
{
}

/***************************************************
 *  O Z C S 0010 1100
 *  - - - - XDC
 ***************************************************/
void f8_cpu_device::f8_xdc()
{
	ROMC_1D();
}

/***************************************************
 *  O Z C S 0011 rrrr
 *  x x x x DS   r
 ***************************************************/
void f8_cpu_device::f8_ds_r(int r)
{
	CLR_OZCS;
	SET_OC(m_r[r], 0xff);
	m_r[r] = m_r[r] + 0xff;
	SET_SZ(m_r[r]);
}

/***************************************************
 *  O Z C S 0011 1100
 *  x x x x DS   ISAR
 ***************************************************/
void f8_cpu_device::f8_ds_isar()
{
	CLR_OZCS;
	SET_OC(m_r[m_is], 0xff);
	m_r[m_is] = m_r[m_is] + 0xff;
	SET_SZ(m_r[m_is]);
}

/***************************************************
 *  O Z C S 0011 1101
 *  x x x x DS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_ds_isar_i()
{
	CLR_OZCS;
	SET_OC(m_r[m_is], 0xff);
	m_r[m_is] = m_r[m_is] + 0xff;
	SET_SZ(m_r[m_is]);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0011 1110
 *  x x x x DS  ISAR--
 ***************************************************/
void f8_cpu_device::f8_ds_isar_d()
{
	CLR_OZCS;
	SET_OC(m_r[m_is], 0xff);
	m_r[m_is] = m_r[m_is] + 0xff;
	SET_SZ(m_r[m_is]);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0100 rrrr
 *  - - - - LR  A,r
 ***************************************************/
void f8_cpu_device::f8_lr_a_r(int r)
{
	m_a = m_r[r];
}

/***************************************************
 *  O Z C S 0100 1100
 *  - - - - LR  A,ISAR
 ***************************************************/
void f8_cpu_device::f8_lr_a_isar()
{
	m_a = m_r[m_is];
}

/***************************************************
 *  O Z C S 0100 1101
 *  - - - - LR  A,ISAR++
 ***************************************************/
void f8_cpu_device::f8_lr_a_isar_i()
{
	m_a = m_r[m_is];
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0100 1110
 *  - - - - LR  A,ISAR--
 ***************************************************/
void f8_cpu_device::f8_lr_a_isar_d()
{
	m_a = m_r[m_is];
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0101 rrrr
 *  - - - - LR  r,A
 ***************************************************/
void f8_cpu_device::f8_lr_r_a(int r)
{
	m_r[r] = m_a;
}

/***************************************************
 *  O Z C S 0101 1100
 *  - - - - LR  ISAR,A
 ***************************************************/
void f8_cpu_device::f8_lr_isar_a()
{
	m_r[m_is] = m_a;
}

/***************************************************
 *  O Z C S 0101 1101
 *  - - - - LR  ISAR++,A
 ***************************************************/
void f8_cpu_device::f8_lr_isar_i_a()
{
	m_r[m_is] = m_a;
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 0101 1110
 *  - - - - LR  ISAR--,A
 ***************************************************/
void f8_cpu_device::f8_lr_isar_d_a()
{
	m_r[m_is] = m_a;
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 0110 0eee
 *  - - - - LISU e
 ***************************************************/
void f8_cpu_device::f8_lisu(int e)
{
	m_is = (m_is & 0x07) | e;
}

/***************************************************
 *  O Z C S 0110 1eee
 *  - - - - LISL e
 ***************************************************/
void f8_cpu_device::f8_lisl(int e)
{
	m_is = (m_is & 0x38) | e;
}

/***************************************************
 *  O Z C S 0111 iiii
 *  - - - - LIS  i
 ***************************************************/
void f8_cpu_device::f8_lis(int i)
{
	m_a = i;
}

/***************************************************
 *  O Z C S 1000 0eee   aaaa aaaa
 *          BT   e,aa
 ***************************************************/
void f8_cpu_device::f8_bt(int e)
{
	ROMC_1C(cS);
	if (m_w & e)
	ROMC_01();     /* take the relative branch */
	else
	ROMC_03(cS);     /* just read the argument on the data bus */
}

/***************************************************
 *  O Z C S 1000 1000
 *  x x x x AM
 ***************************************************/
void f8_cpu_device::f8_am()
{
	ROMC_02();
	CLR_OZCS;
	SET_OC(m_a, m_dbus);
	m_a += m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1001
 *  x x x x AMD
 ***************************************************/
void f8_cpu_device::f8_amd()
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

	UINT8 augend=m_a;
	ROMC_02();
	UINT8 addend=m_dbus;
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

	m_a = tmp;
}

/***************************************************
 *  O Z C S 1000 1010
 *  0 x 0 x NM
 ***************************************************/
void f8_cpu_device::f8_nm()
{
	ROMC_02();
	CLR_OZCS;
	m_a &= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1011
 *  0 x 0 x OM
 ***************************************************/
void f8_cpu_device::f8_om()
{
	ROMC_02();
	CLR_OZCS;
	m_a |= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1100
 *  0 x 0 x XM
 ***************************************************/
void f8_cpu_device::f8_xm()
{
	ROMC_02();
	CLR_OZCS;
	m_a ^= m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1000 1101
 *  x x x x CM
 ***************************************************/
void f8_cpu_device::f8_cm()    /* SKR changed to match f8_ci() */
{
	UINT16 tmp = ((UINT8)~m_a) + 1;
	ROMC_02();
	CLR_OZCS;
	SET_OC(tmp,m_dbus);
	tmp += m_dbus;
	SET_SZ((UINT8)tmp);
}

/***************************************************
 *  O Z C S 1000 1110
 *  - - - - ADC
 ***************************************************/
void f8_cpu_device::f8_adc()
{
	m_dbus = m_a;
	ROMC_0A();          /* add data bus value to DC0 */
}

/***************************************************
 *  O Z C S 1000 1111
 *  - - - - BR7
 ***************************************************/
void f8_cpu_device::f8_br7()
{
	if ((m_is & 7) == 7)
	ROMC_03(cS);      /* just read the argument on the data bus */
	else
	ROMC_01();      /* take the relative branch */
}

/***************************************************
 *  O Z C S 1001 tttt   aaaa aaaa
 *  - - - - BF   t,aa
 ***************************************************/
void f8_cpu_device::f8_bf(int t)
{
	ROMC_1C(cS);
	if (m_w & t)
		ROMC_03(cS);      /* just read the argument on the data bus */
	else
		ROMC_01();      /* take the relative branch */
}

/***************************************************
 *  O Z C S 1010 000n
 *  0 x 0 x INS  n              (n = 0-1)
 ***************************************************/
void f8_cpu_device::f8_ins_0(int n)
{
	ROMC_1C(cS);
	CLR_OZCS;
	m_a = m_iospace->read_byte(n);
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1010 nnnn
 *  0 x 0 x INS  n              (n = 4-F)
 ***************************************************/
void f8_cpu_device::f8_ins_1(int n)
{
	ROMC_1C(cL);
	m_io = n;
	ROMC_1B();
	CLR_OZCS;
	m_a = m_dbus;
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1011 000n
 *  - - - - OUTS n              (n = 0-1)
 ***************************************************/
void f8_cpu_device::f8_outs_0(int n)
{
	ROMC_1C(cS);
	m_iospace->write_byte(n, m_a);
}

/***************************************************
 *  O Z C S 1011 nnnn
 *  - - - - OUTS n              (n = 4-F)
 ***************************************************/
void f8_cpu_device::f8_outs_1(int n)
{
	ROMC_1C(cL);
	m_io = n;
	m_dbus = m_a;
	ROMC_1A();
}

/***************************************************
 *  O Z C S 1100 rrrr
 *  x x x x AS   r
 ***************************************************/
void f8_cpu_device::f8_as(int r)
{
	CLR_OZCS;
	SET_OC(m_a, m_r[r]);
	m_a += m_r[r];
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1100 1100
 *  x x x x AS   ISAR
 ***************************************************/
void f8_cpu_device::f8_as_isar()
{
	CLR_OZCS;
	SET_OC(m_a, m_r[m_is]);
	m_a += m_r[m_is];
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1100 1101
 *  x x x x AS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_as_isar_i()
{
	CLR_OZCS;
	SET_OC(m_a, m_r[m_is]);
	m_a += m_r[m_is];
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1100 1110
 *  x x x x AS   ISAR--
 ***************************************************/
void f8_cpu_device::f8_as_isar_d()
{
	CLR_OZCS;
	SET_OC(m_a, m_r[m_is]);
	m_a += m_r[m_is];
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1101 rrrr
 *  x x x x ASD  r
 ***************************************************/
void f8_cpu_device::f8_asd(int r)
{
/*SKR from F8 Guide To programming description of AMD */
	UINT8 augend=m_a;
	ROMC_1C(cS);
	UINT8 addend=m_r[r];
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

	m_a = tmp;
}

/***************************************************
 *  O Z C S 1101 1100
 *  x x x x ASD  ISAR
 ***************************************************/
void f8_cpu_device::f8_asd_isar()
{
/*SKR from F8 Guide To programming description of AMD */
	UINT8 augend=m_a;
	ROMC_1C(cS);
	UINT8 addend=m_r[m_is];
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

	m_a = tmp;
}

/***************************************************
 *  O Z C S 1101 1101
 *  x x x x ASD  ISAR++
 ***************************************************/
void f8_cpu_device::f8_asd_isar_i()
{
/*SKR from F8 Guide To programming description of AMD */
	UINT8 augend=m_a;
	ROMC_1C(cS);
	UINT8 addend=m_r[m_is];
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

	m_a = tmp;
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1101 1110
 *  x x x x ASD  ISAR--
 ***************************************************/
void f8_cpu_device::f8_asd_isar_d()
{
/*SKR from F8 Guide To programming description of AMD */
	UINT8 augend=m_a;
	ROMC_1C(cS);
	UINT8 addend=m_r[m_is];
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

	m_a = tmp;
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1110 rrrr
 *  0 x 0 x XS   r
 ***************************************************/
void f8_cpu_device::f8_xs(int r)
{
	CLR_OZCS;
	m_a ^= m_r[r];
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1110 1100
 *  0 x 0 x XS   ISAR
 ***************************************************/
void f8_cpu_device::f8_xs_isar()
{
	CLR_OZCS;
	m_a ^= m_r[m_is];
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1110 1101
 *  0 x 0 x XS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_xs_isar_i()
{
	CLR_OZCS;
	m_a ^= m_r[m_is];
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1110 1110
 *  0 x 0 x XS   ISAR--
 ***************************************************/
void f8_cpu_device::f8_xs_isar_d()
{
	CLR_OZCS;
	m_a ^= m_r[m_is];
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

/***************************************************
 *  O Z C S 1111 rrrr
 *  0 x 0 x NS   r
 ***************************************************/
void f8_cpu_device::f8_ns(int r)
{
	CLR_OZCS;
	m_a &= m_r[r];
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1111 1100
 *  0 x 0 x NS   ISAR
 ***************************************************/
void f8_cpu_device::f8_ns_isar()
{
	CLR_OZCS;
	m_a &= m_r[m_is];
	SET_SZ(m_a);
}

/***************************************************
 *  O Z C S 1111 1101
 *  0 x 0 x NS   ISAR++
 ***************************************************/
void f8_cpu_device::f8_ns_isar_i()
{
	CLR_OZCS;
	m_a &= m_r[m_is];
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is + 1) & 0x07);
}

/***************************************************
 *  O Z C S 1111 1110
 *  0 x 0 x NS   ISAR--
 ***************************************************/
void f8_cpu_device::f8_ns_isar_d()
{
	CLR_OZCS;
	m_a &= m_r[m_is];
	SET_SZ(m_a);
	m_is = (m_is & 0x38) | ((m_is - 1) & 0x07);
}

void f8_cpu_device::device_reset()
{
	UINT8 data;
	int i;

	m_pc0 = 0;
	m_pc1 = 0;
	m_dc0 = 0;
	m_dc1 = 0;
	m_a = 0;
	m_w = 0;
	m_is = 0;
	m_dbus = 0;
	m_io = 0;
	m_irq_vector = 0;
	memset(m_r, 0, sizeof(m_r));
	m_irq_request = 0;

	m_w&=~I;

	/* save PC0 to PC1 and reset PC0 */
	ROMC_08();
	/* fetch the first opcode */
	ROMC_00(cS);

	/* initialize the timer shift register
	 * this is an 8 bit polynome counter which can be loaded parallel
	 * with 0xff the outputs never change and thus the timer is disabled.
	 * with 0xfe the shifter starts cycling through 255 states until it
	 * reaches 0xfe again (and then issues an interrupt).
	 * the counter output values are not sequential, but go like this:
	 * 0xfe, 0xfd, 0xfb, 0xf7, 0xee, 0xdc ... etc. :-)
	 * We have to build a lookup table to tell how many cycles a write

	 */
	data = 0xfe;    /* initial value */
	for (i = 0; i < 256; i++)
	{
		timer_shifter[i] = data;
		if ( (((data >> 3) ^ (data >> 4)) ^ ((data >> 5) ^ (data >> 7))) & 1 )
		{
			data <<= 1;
		}
		else
		{
			data = (data << 1) | 1;
		}
	}
}

/* Execute cycles - returns number of cycles actually run */
void f8_cpu_device::execute_run()
{
	do
	{
		UINT8 op=m_dbus;

		m_pc = (m_pc0 - 1) & 0xffff;
		debugger_instruction_hook(this, (m_pc0 - 1) & 0xffff);

		switch( op )
		{
			/* opcode  bitmask */
			case 0x00: /* 0000 0000 */  f8_lr_a_ku();       break;
			case 0x01: /* 0000 0001 */  f8_lr_a_kl();       break;
			case 0x02: /* 0000 0010 */  f8_lr_a_qu();       break;
			case 0x03: /* 0000 0011 */  f8_lr_a_ql();       break;
			case 0x04: /* 0000 0100 */  f8_lr_ku_a();       break;
			case 0x05: /* 0000 0101 */  f8_lr_kl_a();       break;
			case 0x06: /* 0000 0110 */  f8_lr_qu_a();       break;
			case 0x07: /* 0000 0111 */  f8_lr_ql_a();       break;
			case 0x08: /* 0000 1000 */  f8_lr_k_p();        break;
			case 0x09: /* 0000 1001 */  f8_lr_p_k();        break;
			case 0x0a: /* 0000 1010 */  f8_lr_a_is();       break;
			case 0x0b: /* 0000 1011 */  f8_lr_is_a();       break;
			case 0x0c: /* 0000 1100 */  f8_pk();            break;
			case 0x0d: /* 0000 1101 */  f8_lr_p0_q();       break;
			case 0x0e: /* 0000 1110 */  f8_lr_q_dc();       break;
			case 0x0f: /* 0000 1111 */  f8_lr_dc_q();       break;

			case 0x10: /* 0001 0000 */  f8_lr_dc_h();       break;
			case 0x11: /* 0001 0001 */  f8_lr_h_dc();       break;
			case 0x12: /* 0001 0010 */  f8_sr_1();          break;
			case 0x13: /* 0001 0011 */  f8_sl_1();          break;
			case 0x14: /* 0001 0100 */  f8_sr_4();          break;
			case 0x15: /* 0001 0101 */  f8_sl_4();          break;
			case 0x16: /* 0001 0110 */  f8_lm();            break;
			case 0x17: /* 0001 0111 */  f8_st();            break;
			case 0x18: /* 0001 1000 */  f8_com();           break;
			case 0x19: /* 0001 1001 */  f8_lnk();           break;
			case 0x1a: /* 0001 1010 */  f8_di();            break;
			case 0x1b: /* 0001 1011 */  f8_ei();            break;
			case 0x1c: /* 0001 1100 */  f8_pop();           break;
			case 0x1d: /* 0001 1101 */  f8_lr_w_j();        break;
			case 0x1e: /* 0001 1110 */  f8_lr_j_w();        break;
			case 0x1f: /* 0001 1111 */  f8_inc();           break;

			case 0x20: /* 0010 0000 */  f8_li();            break;
			case 0x21: /* 0010 0001 */  f8_ni();            break;
			case 0x22: /* 0010 0010 */  f8_oi();            break;
			case 0x23: /* 0010 0011 */  f8_xi();            break;
			case 0x24: /* 0010 0100 */  f8_ai();            break;
			case 0x25: /* 0010 0101 */  f8_ci();            break;
			case 0x26: /* 0010 0110 */  f8_in();            break;
			case 0x27: /* 0010 0111 */  f8_out();           break;
			case 0x28: /* 0010 1000 */  f8_pi();            break;
			case 0x29: /* 0010 1001 */  f8_jmp();           break;
			case 0x2a: /* 0010 1010 */  f8_dci();           break;
			case 0x2b: /* 0010 1011 */  f8_nop();           break;
			case 0x2c: /* 0010 1100 */  f8_xdc();           break;
			case 0x2d: /* 0010 1101 */  illegal();          break;
			case 0x2e: /* 0010 1110 */  illegal();          break;
			case 0x2f: /* 0010 1111 */  illegal();          break;

			case 0x30: /* 0011 0000 */  f8_ds_r( 0);        break;
			case 0x31: /* 0011 0001 */  f8_ds_r( 1);      break;
			case 0x32: /* 0011 0010 */  f8_ds_r( 2);      break;
			case 0x33: /* 0011 0011 */  f8_ds_r( 3);      break;
			case 0x34: /* 0011 0100 */  f8_ds_r( 4);      break;
			case 0x35: /* 0011 0101 */  f8_ds_r( 5);      break;
			case 0x36: /* 0011 0110 */  f8_ds_r( 6);      break;
			case 0x37: /* 0011 0111 */  f8_ds_r( 7);      break;
			case 0x38: /* 0011 1000 */  f8_ds_r( 8);      break;
			case 0x39: /* 0011 1001 */  f8_ds_r( 9);      break;
			case 0x3a: /* 0011 1010 */  f8_ds_r(10);      break;
			case 0x3b: /* 0011 1011 */  f8_ds_r(11);      break;
			case 0x3c: /* 0011 1100 */  f8_ds_isar();       break;
			case 0x3d: /* 0011 1101 */  f8_ds_isar_i();     break;
			case 0x3e: /* 0011 1110 */  f8_ds_isar_d();     break;
			case 0x3f: /* 0011 1111 */  illegal();          break;

			case 0x40: /* 0100 0000 */  f8_lr_a_r( 0);      break;
			case 0x41: /* 0100 0001 */  f8_lr_a_r( 1);        break;
			case 0x42: /* 0100 0010 */  f8_lr_a_r( 2);        break;
			case 0x43: /* 0100 0011 */  f8_lr_a_r( 3);        break;
			case 0x44: /* 0100 0100 */  f8_lr_a_r( 4);        break;
			case 0x45: /* 0100 0101 */  f8_lr_a_r( 5);        break;
			case 0x46: /* 0100 0110 */  f8_lr_a_r( 6);        break;
			case 0x47: /* 0100 0111 */  f8_lr_a_r( 7);        break;
			case 0x48: /* 0100 1000 */  f8_lr_a_r( 8);        break;
			case 0x49: /* 0100 1001 */  f8_lr_a_r( 9);        break;
			case 0x4a: /* 0100 1010 */  f8_lr_a_r(10);        break;
			case 0x4b: /* 0100 1011 */  f8_lr_a_r(11);        break;
			case 0x4c: /* 0100 1100 */  f8_lr_a_isar();     break;
			case 0x4d: /* 0100 1101 */  f8_lr_a_isar_i();   break;
			case 0x4e: /* 0100 1110 */  f8_lr_a_isar_d();   break;
			case 0x4f: /* 0100 1111 */  illegal();          break;

			case 0x50: /* 0101 0000 */  f8_lr_r_a( 0);        break;
			case 0x51: /* 0101 0001 */  f8_lr_r_a( 1);        break;
			case 0x52: /* 0101 0010 */  f8_lr_r_a( 2);        break;
			case 0x53: /* 0101 0011 */  f8_lr_r_a( 3);        break;
			case 0x54: /* 0101 0100 */  f8_lr_r_a( 4);        break;
			case 0x55: /* 0101 0101 */  f8_lr_r_a( 5);        break;
			case 0x56: /* 0101 0110 */  f8_lr_r_a( 6);        break;
			case 0x57: /* 0101 0111 */  f8_lr_r_a( 7);        break;
			case 0x58: /* 0101 1000 */  f8_lr_r_a( 8);        break;
			case 0x59: /* 0101 1001 */  f8_lr_r_a( 9);        break;
			case 0x5a: /* 0101 1010 */  f8_lr_r_a(10);        break;
			case 0x5b: /* 0101 1011 */  f8_lr_r_a(11);        break;
			case 0x5c: /* 0101 1100 */  f8_lr_isar_a();     break;
			case 0x5d: /* 0101 1101 */  f8_lr_isar_i_a();   break;
			case 0x5e: /* 0101 1110 */  f8_lr_isar_d_a();   break;
			case 0x5f: /* 0101 1111 */  illegal();          break;

			case 0x60: /* 0110 0000 */  f8_lisu(0x00);        break;
			case 0x61: /* 0110 0001 */  f8_lisu(0x08);        break;
			case 0x62: /* 0110 0010 */  f8_lisu(0x10);        break;
			case 0x63: /* 0110 0011 */  f8_lisu(0x18);        break;
			case 0x64: /* 0110 0100 */  f8_lisu(0x20);        break;
			case 0x65: /* 0110 0101 */  f8_lisu(0x28);        break;
			case 0x66: /* 0110 0110 */  f8_lisu(0x30);        break;
			case 0x67: /* 0110 0111 */  f8_lisu(0x38);        break;
			case 0x68: /* 0110 1000 */  f8_lisl(0x00);        break;
			case 0x69: /* 0110 1001 */  f8_lisl(0x01);        break;
			case 0x6a: /* 0110 1010 */  f8_lisl(0x02);        break;
			case 0x6b: /* 0110 1011 */  f8_lisl(0x03);        break;
			case 0x6c: /* 0110 1100 */  f8_lisl(0x04);        break;
			case 0x6d: /* 0110 1101 */  f8_lisl(0x05);        break;
			case 0x6e: /* 0110 1110 */  f8_lisl(0x06);        break;
			case 0x6f: /* 0110 1111 */  f8_lisl(0x07);        break;

			case 0x70: /* 0111 0000 */  f8_lis(0x0);      break;
			case 0x71: /* 0111 0001 */  f8_lis(0x1);      break;
			case 0x72: /* 0111 0010 */  f8_lis(0x2);      break;
			case 0x73: /* 0111 0011 */  f8_lis(0x3);      break;
			case 0x74: /* 0111 0100 */  f8_lis(0x4);      break;
			case 0x75: /* 0111 0101 */  f8_lis(0x5);      break;
			case 0x76: /* 0111 0110 */  f8_lis(0x6);      break;
			case 0x77: /* 0111 0111 */  f8_lis(0x7);      break;
			case 0x78: /* 0111 1000 */  f8_lis(0x8);      break;
			case 0x79: /* 0111 1001 */  f8_lis(0x9);      break;
			case 0x7a: /* 0111 1010 */  f8_lis(0xa);      break;
			case 0x7b: /* 0111 1011 */  f8_lis(0xb);      break;
			case 0x7c: /* 0111 1100 */  f8_lis(0xc);      break;
			case 0x7d: /* 0111 1101 */  f8_lis(0xd);      break;
			case 0x7e: /* 0111 1110 */  f8_lis(0xe);      break;
			case 0x7f: /* 0111 1111 */  f8_lis(0xf);      break;

			case 0x80: /* 1000 0000 */  f8_bt(0);         break;
			case 0x81: /* 1000 0001 */  f8_bt(1);         break;
			case 0x82: /* 1000 0010 */  f8_bt(2);         break;
			case 0x83: /* 1000 0011 */  f8_bt(3);         break;
			case 0x84: /* 1000 0100 */  f8_bt(4);         break;
			case 0x85: /* 1000 0101 */  f8_bt(5);         break;
			case 0x86: /* 1000 0110 */  f8_bt(6);         break;
			case 0x87: /* 1000 0111 */  f8_bt(7);         break;
			case 0x88: /* 1000 1000 */  f8_am();            break;
			case 0x89: /* 1000 1001 */  f8_amd();           break;
			case 0x8a: /* 1000 1010 */  f8_nm();            break;
			case 0x8b: /* 1000 1011 */  f8_om();            break;
			case 0x8c: /* 1000 1100 */  f8_xm();            break;
			case 0x8d: /* 1000 1101 */  f8_cm();            break;
			case 0x8e: /* 1000 1110 */  f8_adc();           break;
			case 0x8f: /* 1000 1111 */  f8_br7();           break;

			case 0x90: /* 1001 0000 */  f8_bf(0x0);       break;
			case 0x91: /* 1001 0001 */  f8_bf(0x1);       break;
			case 0x92: /* 1001 0010 */  f8_bf(0x2);       break;
			case 0x93: /* 1001 0011 */  f8_bf(0x3);       break;
			case 0x94: /* 1001 0100 */  f8_bf(0x4);       break;
			case 0x95: /* 1001 0101 */  f8_bf(0x5);       break;
			case 0x96: /* 1001 0110 */  f8_bf(0x6);       break;
			case 0x97: /* 1001 0111 */  f8_bf(0x7);       break;
			case 0x98: /* 1001 1000 */  f8_bf(0x8);       break;
			case 0x99: /* 1001 1001 */  f8_bf(0x9);       break;
			case 0x9a: /* 1001 1010 */  f8_bf(0xa);       break;
			case 0x9b: /* 1001 1011 */  f8_bf(0xb);       break;
			case 0x9c: /* 1001 1100 */  f8_bf(0xc);       break;
			case 0x9d: /* 1001 1101 */  f8_bf(0xd);       break;
			case 0x9e: /* 1001 1110 */  f8_bf(0xe);       break;
			case 0x9f: /* 1001 1111 */  f8_bf(0xf);       break;

			case 0xa0: /* 1010 0000 */  f8_ins_0(0x0);        break;
			case 0xa1: /* 1010 0001 */  f8_ins_0(0x1);        break;
			case 0xa2: /* 1010 0010 */  illegal();          break;
			case 0xa3: /* 1010 0011 */  illegal();          break;
			case 0xa4: /* 1010 0100 */  f8_ins_1(0x4);        break;
			case 0xa5: /* 1010 0101 */  f8_ins_1(0x5);        break;
			case 0xa6: /* 1010 0110 */  f8_ins_1(0x6);        break;
			case 0xa7: /* 1010 0111 */  f8_ins_1(0x7);        break;
			case 0xa8: /* 1010 1000 */  f8_ins_1(0x8);        break;
			case 0xa9: /* 1010 1001 */  f8_ins_1(0x9);        break;
			case 0xaa: /* 1010 1010 */  f8_ins_1(0xa);        break;
			case 0xab: /* 1010 1011 */  f8_ins_1(0xb);        break;
			case 0xac: /* 1010 1100 */  f8_ins_1(0xc);        break;
			case 0xad: /* 1010 1101 */  f8_ins_1(0xd);        break;
			case 0xae: /* 1010 1110 */  f8_ins_1(0xe);        break;
			case 0xaf: /* 1010 1111 */  f8_ins_1(0xf);        break;

			case 0xb0: /* 1011 0000 */  f8_outs_0(0x0);   break;
			case 0xb1: /* 1011 0001 */  f8_outs_0(0x1);   break;
			case 0xb2: /* 1011 0010 */  illegal();          break;
			case 0xb3: /* 1011 0011 */  illegal();          break;
			case 0xb4: /* 1011 0100 */  f8_outs_1(0x4);   break;
			case 0xb5: /* 1011 0101 */  f8_outs_1(0x5);   break;
			case 0xb6: /* 1011 0110 */  f8_outs_1(0x6);   break;
			case 0xb7: /* 1011 0111 */  f8_outs_1(0x7);   break;
			case 0xb8: /* 1011 1000 */  f8_outs_1(0x8);   break;
			case 0xb9: /* 1011 1001 */  f8_outs_1(0x9);   break;
			case 0xba: /* 1011 1010 */  f8_outs_1(0xa);   break;
			case 0xbb: /* 1011 1011 */  f8_outs_1(0xb);   break;
			case 0xbc: /* 1011 1100 */  f8_outs_1(0xc);   break;
			case 0xbd: /* 1011 1101 */  f8_outs_1(0xd);   break;
			case 0xbe: /* 1011 1110 */  f8_outs_1(0xe);   break;
			case 0xbf: /* 1011 1111 */  f8_outs_1(0xf);   break;

			case 0xc0: /* 1100 0000 */  f8_as(0x0);       break;
			case 0xc1: /* 1100 0001 */  f8_as(0x1);       break;
			case 0xc2: /* 1100 0010 */  f8_as(0x2);       break;
			case 0xc3: /* 1100 0011 */  f8_as(0x3);       break;
			case 0xc4: /* 1100 0100 */  f8_as(0x4);       break;
			case 0xc5: /* 1100 0101 */  f8_as(0x5);       break;
			case 0xc6: /* 1100 0110 */  f8_as(0x6);       break;
			case 0xc7: /* 1100 0111 */  f8_as(0x7);       break;
			case 0xc8: /* 1100 1000 */  f8_as(0x8);       break;
			case 0xc9: /* 1100 1001 */  f8_as(0x9);       break;
			case 0xca: /* 1100 1010 */  f8_as(0xa);       break;
			case 0xcb: /* 1100 1011 */  f8_as(0xb);       break;
			case 0xcc: /* 1100 1100 */  f8_as_isar();       break;
			case 0xcd: /* 1100 1101 */  f8_as_isar_i();     break;
			case 0xce: /* 1100 1110 */  f8_as_isar_d();     break;
			case 0xcf: /* 1100 1111 */  illegal();          break;

			case 0xd0: /* 1101 0000 */  f8_asd(0x0);      break;
			case 0xd1: /* 1101 0001 */  f8_asd(0x1);      break;
			case 0xd2: /* 1101 0010 */  f8_asd(0x2);      break;
			case 0xd3: /* 1101 0011 */  f8_asd(0x3);      break;
			case 0xd4: /* 1101 0100 */  f8_asd(0x4);      break;
			case 0xd5: /* 1101 0101 */  f8_asd(0x5);      break;
			case 0xd6: /* 1101 0110 */  f8_asd(0x6);      break;
			case 0xd7: /* 1101 0111 */  f8_asd(0x7);      break;
			case 0xd8: /* 1101 1000 */  f8_asd(0x8);      break;
			case 0xd9: /* 1101 1001 */  f8_asd(0x9);      break;
			case 0xda: /* 1101 1010 */  f8_asd(0xa);      break;
			case 0xdb: /* 1101 1011 */  f8_asd(0xb);      break;
			case 0xdc: /* 1101 1100 */  f8_asd_isar();      break;
			case 0xdd: /* 1101 1101 */  f8_asd_isar_i();    break;
			case 0xde: /* 1101 1110 */  f8_asd_isar_d();    break;
			case 0xdf: /* 1101 1111 */  illegal();          break;

			case 0xe0: /* 1110 0000 */  f8_xs(0x0);       break;
			case 0xe1: /* 1110 0001 */  f8_xs(0x1);       break;
			case 0xe2: /* 1110 0010 */  f8_xs(0x2);       break;
			case 0xe3: /* 1110 0011 */  f8_xs(0x3);       break;
			case 0xe4: /* 1110 0100 */  f8_xs(0x4);       break;
			case 0xe5: /* 1110 0101 */  f8_xs(0x5);       break;
			case 0xe6: /* 1110 0110 */  f8_xs(0x6);       break;
			case 0xe7: /* 1110 0111 */  f8_xs(0x7);       break;
			case 0xe8: /* 1110 1000 */  f8_xs(0x8);       break;
			case 0xe9: /* 1110 1001 */  f8_xs(0x9);       break;
			case 0xea: /* 1110 1010 */  f8_xs(0xa);       break;
			case 0xeb: /* 1110 1011 */  f8_xs(0xb);       break;
			case 0xec: /* 1110 1100 */  f8_xs_isar();       break;
			case 0xed: /* 1110 1101 */  f8_xs_isar_i();     break;
			case 0xee: /* 1110 1110 */  f8_xs_isar_d();     break;
			case 0xef: /* 1110 1111 */  illegal();          break;

			case 0xf0: /* 1111 0000 */  f8_ns(0x0);       break;
			case 0xf1: /* 1111 0001 */  f8_ns(0x1);       break;
			case 0xf2: /* 1111 0010 */  f8_ns(0x2);       break;
			case 0xf3: /* 1111 0011 */  f8_ns(0x3);       break;
			case 0xf4: /* 1111 0100 */  f8_ns(0x4);       break;
			case 0xf5: /* 1111 0101 */  f8_ns(0x5);       break;
			case 0xf6: /* 1111 0110 */  f8_ns(0x6);       break;
			case 0xf7: /* 1111 0111 */  f8_ns(0x7);       break;
			case 0xf8: /* 1111 1000 */  f8_ns(0x8);       break;
			case 0xf9: /* 1111 1001 */  f8_ns(0x9);       break;
			case 0xfa: /* 1111 1010 */  f8_ns(0xa);       break;
			case 0xfb: /* 1111 1011 */  f8_ns(0xb);       break;
			case 0xfc: /* 1111 1100 */  f8_ns_isar();       break;
			case 0xfd: /* 1111 1101 */  f8_ns_isar_i();     break;
			case 0xfe: /* 1111 1110 */  f8_ns_isar_d();     break;
			case 0xff: /* 1111 1111 */  illegal();          break;
		}
		switch (op) {
			case 0x0d: case 0x1b: case 0x1c: case 0x1d:
			case 0x27: case 0x28: case 0x29:
			case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb:
			case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				ROMC_00(cS);
				break;

			default:
				if (m_w&I && m_irq_request)
				{
					ROMC_1C(cL);
					ROMC_0F();
					ROMC_13();
				}
				if( ( op >= 0x30 ) && ( op <= 0x3f) )  /* SKR - DS is a long cycle inst */
				{
					ROMC_00(cL);
				}
				else
				{
					ROMC_00(cS);
				}
				break;
		}
	} while( m_icount > 0 );
}


void f8_cpu_device::device_start()
{
	// TODO register debug state
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_iospace = &space(AS_IO);

	save_item(NAME(m_pc0));
	save_item(NAME(m_pc1));
	save_item(NAME(m_dc0));
	save_item(NAME(m_dc1));
	save_item(NAME(m_a));
	save_item(NAME(m_w));
	save_item(NAME(m_is));
	save_item(NAME(m_dbus));
	save_item(NAME(m_io));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_request));
	save_item(NAME(m_r));

	state_add( F8_PC0, "PC0", m_pc0).formatstr("%04X");
	state_add( F8_PC1, "PC1", m_pc1).formatstr("%04X");
	state_add( F8_DC0, "DC0", m_dc0).formatstr("%04X");
	state_add( F8_DC1, "DC1", m_dc1).formatstr("%04X");
	state_add( F8_W,   "W",   m_w).formatstr("%02X");
	state_add( F8_A,   "A",   m_a).formatstr("%02X");
	state_add( F8_IS,  "IS",  m_is).mask(0x3f).formatstr("%02X");
	state_add( F8_J,   "J",   m_r[9]).formatstr("%02X");
	state_add( F8_HU,  "HU",  m_r[10]).formatstr("%02X");
	state_add( F8_HL,  "HL",  m_r[11]).formatstr("%02X");
	state_add( F8_KU,  "KU",  m_r[12]).formatstr("%02X");
	state_add( F8_KL,  "KL",  m_r[13]).formatstr("%02X");
	state_add( F8_QU,  "QU",  m_r[14]).formatstr("%02X");
	state_add( F8_QL,  "QL",  m_r[15]).formatstr("%02X");
	state_add( F8_R0,  "R0",  m_r[0]).formatstr("%02X");
	state_add( F8_R1,  "R1",  m_r[1]).formatstr("%02X");
	state_add( F8_R2,  "R2",  m_r[2]).formatstr("%02X");
	state_add( F8_R3,  "R3",  m_r[3]).formatstr("%02X");
	state_add( F8_R4,  "R4",  m_r[4]).formatstr("%02X");
	state_add( F8_R5,  "R5",  m_r[5]).formatstr("%02X");
	state_add( F8_R6,  "R6",  m_r[6]).formatstr("%02X");
	state_add( F8_R7,  "R7",  m_r[7]).formatstr("%02X");
	state_add( F8_R8,  "R8",  m_r[8]).formatstr("%02X");
	state_add( F8_R16, "R16", m_r[16]).formatstr("%02X");
	state_add( F8_R17, "R17", m_r[17]).formatstr("%02X");
	state_add( F8_R18, "R18", m_r[18]).formatstr("%02X");
	state_add( F8_R19, "R19", m_r[19]).formatstr("%02X");
	state_add( F8_R20, "R20", m_r[20]).formatstr("%02X");
	state_add( F8_R21, "R21", m_r[21]).formatstr("%02X");
	state_add( F8_R22, "R22", m_r[22]).formatstr("%02X");
	state_add( F8_R23, "R23", m_r[23]).formatstr("%02X");
	state_add( F8_R24, "R24", m_r[24]).formatstr("%02X");
	state_add( F8_R25, "R25", m_r[25]).formatstr("%02X");
	state_add( F8_R26, "R26", m_r[26]).formatstr("%02X");
	state_add( F8_R27, "R27", m_r[27]).formatstr("%02X");
	state_add( F8_R28, "R28", m_r[28]).formatstr("%02X");
	state_add( F8_R29, "R29", m_r[29]).formatstr("%02X");
	state_add( F8_R30, "R30", m_r[30]).formatstr("%02X");
	state_add( F8_R31, "R31", m_r[31]).formatstr("%02X");
	state_add( F8_R32, "R32", m_r[32]).formatstr("%02X");
	state_add( F8_R33, "R33", m_r[33]).formatstr("%02X");
	state_add( F8_R34, "R34", m_r[34]).formatstr("%02X");
	state_add( F8_R35, "R35", m_r[35]).formatstr("%02X");
	state_add( F8_R36, "R36", m_r[36]).formatstr("%02X");
	state_add( F8_R37, "R37", m_r[37]).formatstr("%02X");
	state_add( F8_R38, "R38", m_r[38]).formatstr("%02X");
	state_add( F8_R39, "R39", m_r[39]).formatstr("%02X");
	state_add( F8_R40, "R40", m_r[40]).formatstr("%02X");
	state_add( F8_R41, "R41", m_r[41]).formatstr("%02X");
	state_add( F8_R42, "R42", m_r[42]).formatstr("%02X");
	state_add( F8_R43, "R43", m_r[43]).formatstr("%02X");
	state_add( F8_R44, "R44", m_r[44]).formatstr("%02X");
	state_add( F8_R45, "R45", m_r[45]).formatstr("%02X");
	state_add( F8_R46, "R46", m_r[46]).formatstr("%02X");
	state_add( F8_R47, "R47", m_r[47]).formatstr("%02X");
	state_add( F8_R48, "R48", m_r[48]).formatstr("%02X");
	state_add( F8_R49, "R49", m_r[49]).formatstr("%02X");
	state_add( F8_R50, "R50", m_r[50]).formatstr("%02X");
	state_add( F8_R51, "R51", m_r[51]).formatstr("%02X");
	state_add( F8_R52, "R52", m_r[52]).formatstr("%02X");
	state_add( F8_R53, "R53", m_r[53]).formatstr("%02X");
	state_add( F8_R54, "R54", m_r[54]).formatstr("%02X");
	state_add( F8_R55, "R55", m_r[55]).formatstr("%02X");
	state_add( F8_R56, "R56", m_r[56]).formatstr("%02X");
	state_add( F8_R57, "R57", m_r[57]).formatstr("%02X");
	state_add( F8_R58, "R58", m_r[58]).formatstr("%02X");
	state_add( F8_R59, "R59", m_r[59]).formatstr("%02X");
	state_add( F8_R60, "R60", m_r[60]).formatstr("%02X");
	state_add( F8_R61, "R61", m_r[61]).formatstr("%02X");
	state_add( F8_R62, "R62", m_r[62]).formatstr("%02X");
	state_add( F8_R63, "R63", m_r[63]).formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_w).formatstr("%5s").noshow();

	m_icountptr = &m_icount;
}


void f8_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c",
					m_w & 0x10 ? 'I':'.',
					m_w & 0x08 ? 'O':'.',
					m_w & 0x04 ? 'Z':'.',
					m_w & 0x02 ? 'C':'.',
					m_w & 0x01 ? 'S':'.');
			break;
	}
}


offs_t f8_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( f8 );
	return CPU_DISASSEMBLE_NAME(f8)(this, buffer, pc, oprom, opram, options);
}


void f8_cpu_device::execute_set_input( int inptnum, int state )
{
	m_irq_request = state;
}
