// license:BSD-3-Clause
// copyright-holders:Tony La Porta, hap
	/**************************************************************************\
	*                Texas Instruments TMS320x25 DSP Emulator                  *
	*                                                                          *
	*                 Copyright Tony La Porta                                  *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*                                                                          *
	*  Three versions of the chip are available, and they are:                 *
	*  TMS320C25   Internal ROM one time programmed at TI                      *
	*  TMS320E25   Internal ROM programmable as a normal EPROM                 *
	*  TMS320P25   Internal ROM programmable once as a normal EPROM only       *
	*  These devices can also be used as a MicroController with external ROM   *
	*                                                                          *
	*                                                                          *
	*      Notes : The term 'DMA' within this document, is in reference        *
	*                  to Direct Memory Addressing, and NOT the usual term     *
	*                  of Direct Memory Access.                                *
	*              This is a word based microcontroller, with addressing       *
	*                  architecture based on the Harvard addressing scheme.    *
	*                                                                          *
	*                                                                          *
	*                                                                          *
	*  **** Change Log ****                                                    *
	*                                                                          *
	*  TLP (2x-May-2001)                                                       *
	*   - Work began on this emulator                                          *
	*  TLP (12-Jul-2001)                                                       *
	*   - First private release                                                *
	*  TLP (xx-Dec-2001) Ver 0.11                                              *
	*   - Various undocumented fixes                                           *
	*  TLP (13-Jul-2002) Ver 0.12                                              *
	*   - Corrected IRQ2 vector pointer                                        *
	*   - Fixed the signedness in many equation based instructions             *
	*   - Adjusted the level sensing for the Signal inputs                     *
	*   - Added the ability to view the CPU in the debugger when it's halted   *
	*  TLP (16-Nov-2002)                                                       *
	*   - First public release after nearly 1.5 years!                         *
	*   - Adjusted more signedness instructions (ADDH, SUBC, SUBH, etc)        *
	*  TLP (21-Dec-2002)                                                       *
	*   - Added memory banking for the CNFD, CNFP and CONF instructions        *
	*   - Corrected IRQ masking checks                                         *
	*  TLP (25-Dec-2002) Ver 1.10                                              *
	*   - Added internal timer                                                 *
	*                                                                          *
	\**************************************************************************/

/*****************************************************************************
 To fix, or currently lacking from this emulator are:

 Fix the levels for S_IN and S_OUT - use assert/release line

 #  Support for the built in Timer/Counter Page 91
    When idling, Counter must still be activly counting down. When counter
    reaches 0 it should issue a TINT (if it's not masked), then come out of
    IDLE mode.
    If TINT is masked, the Timer still needs to count down.

 #  Support for the built in Serial Port
 #  Support for the Global memory register
 #  Support for the switch for RAM block 0 banking between RAM and ROM space
 #  Correct the multi-cycle instruction cycle counts
 #  Add support to set ROM & RAM as Internal/External in order to correctly
    compute cycle timings
 #  Check (read) Hold signal level during execution loop ?
 #  Fix bugs
 #  Fix more bugs :-)
 #  Add/fix other things I forgot
*****************************************************************************/

/*
     TMS32025 CONF Mode Decoding Table
|=======================================|
| Status bit |           Blocks         |
|     CNF    |   B0    |   B1    |  B2  |
|------------+---------+---------+------|
|     0  0   |  data   |  data   | data |
|     1  1   | program |  data   | data |
|=======================================|


     TMS32026 CONF Mode Decoding Table
|==================================================|
| Status bits |               Blocks               |
| CNF1 | CNF0 |   B0    |   B1    |  B2  |   B3    |
|------+------+---------+---------+------+---------|
|  0   |  0   |  data   |  data   | data |  data   |
|  0   |  1   | program |  data   | data |  data   |
|  1   |  0   | program | program | data |  data   |
|  1   |  1   | program | program | data | program |
|==================================================|



Table 3-2.  TMS32025/26 Memory Blocks
|=========================================================|
|             Configured As Data Memory                   |
|-------+-------TMS320C25--------+-------TMS320C26--------|
|       |         | Hexadecimal  |         | Hexadecimal  |
| Block |  Pages  |   Address    |  Pages  |   Address    |
|-------+---------+--------------+---------+--------------|
|   B2  |    0    | 0060h-007Fh  |    0    | 0060h-007Fh  |
|   B0  |   4-5   | 0200h-02FFh  |   4-7   | 0200h-03FFh  |
|   B1  |   6-7   | 0300h-03FFh  |   8-11  | 0400h-05FFh  |
|   B3  |   B3 does not exist    |  12-15  | 0600h-07FFh  |
|=========================================================|
|             Configured As Program Memory                |
|-------+-------TMS320C25--------+-------TMS320C26--------|
|       |         | Hexadecimal  |         | Hexadecimal  |
| Block |  Pages  |   Address    |  Pages  |   Address    |
|-------+---------+--------------+---------+--------------|
|   B2  | B2 is not configurable | B2 is not configurable |
|   B0  | 510-511 | FF00h-FFFFh  | 500-503 | FA00h-FBFFh  |
|   B1  | B1 is not configurable | 504-507 | FC00h-FDFFh  |
|   B3  | B3 does not exist      | 508-511 | FE00h-FFFFh  |
|=========================================================|
*/


#include "emu.h"
#include "debugger.h"
#include "tms32025.h"


#define CLK 4   /* 1 cycle equals 4 clock ticks */      /* PE/DI */



#define SET_PC(x)   do { m_PC = (x); } while (0)

#define P_IN(A)         (m_io->read_word((A)<<1))
#define P_OUT(A,V)      (m_io->write_word(((A)<<1),(V)))
#define S_IN(A)         (m_io->read_word((A)<<1))
#define S_OUT(A,V)      (m_io->write_word(((A)<<1),(V)))

#define M_RDOP(A)       ((m_pgmmap[(A) >> 7]) ? (m_pgmmap[(A) >> 7][(A) & 0x7f]) : m_direct->read_word((A)<<1))
#define M_RDOP_ARG(A)   ((m_pgmmap[(A) >> 7]) ? (m_pgmmap[(A) >> 7][(A) & 0x7f]) : m_direct->read_word((A)<<1))


/************************** Memory mapped registers ****************/
#define DRR     m_intRAM[0]
#define DXR     m_intRAM[1]
#define TIM     m_intRAM[2]
#define PRD     m_intRAM[3]
#define IMR     m_intRAM[4]
#define GREG    m_intRAM[5]


/****************************************************************************
 *******  The following is the Status (Flag) register 0 definition.  ********
| 15 | 14 | 13 | 12 |  11 | 10 |   9  | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| <----ARP---> | OV | OVM |  1 | INTM | <--------------DP---------------> | */

#define ARP_REG     0xe000  /* ARP  (Auxiliary Register Pointer) */
#define OV_FLAG     0x1000  /* OV   (Overflow flag) 1 indicates an overflow */
#define OVM_FLAG    0x0800  /* OVM  (Overflow Mode bit) 1 forces ACC overflow to greatest positive or negative saturation value */
#define INTM_FLAG   0x0200  /* INTM (Interrupt Mask flag) 0 enables maskable interrupts */
#define DP_REG      0x01ff  /* DP   (Data bank memory Pointer) */


/***********************************************************************************
 *** The following is the Status (Flag) register 1 definition for TMS32025. ********
| 15 | 14 | 13 |  12  | 11 |  10 | 9 | 8 | 7 |  6 |  5  |  4 |  3 |  2  | 1 | 0  |
| <----ARB---> | CNF0 | TC | SXM | C | 1 | 1 | HM | FSM | XF | FO | TXM | <-PM-> | */

/*** The following is the Status (Flag) register 1 definition for TMS32026. ***********
| 15 | 14 | 13 |  12  | 11 |  10 | 9 | 8 |   7  |  6 |  5  |  4 |  3 |  2  | 1 | 0  |
| <----ARB---> | CNF0 | TC | SXM | C | 1 | CNF1 | HM | FSM | XF | FO | TXM | <-PM-> | */

#define ARB_REG     0xe000  /* ARB  (Auxiliary Register pointer Backup) */
#define CNF0_REG    0x1000  /* CNF0 (Onchip RAM CoNFiguration) 0 means B0=data memory, 1means B0=program memory */
#define CNF1_REG    0x0080  /* CNF1 (Onchip RAM CoNFiguration) 0 means B0=data memory, 1means B0=program memory */
#define TC_FLAG     0x0800  /* TC   (Test Control flag) */
#define SXM_FLAG    0x0400  /* SXM  (Sign eXtension Mode) */
#define C_FLAG      0x0200  /* C    (Carry flag) */
#define HM_FLAG     0x0040  /* HM   (Processor Hold Mode) */
#define FSM_FLAG    0x0020  /* FSM  (Frame Synchronization Mode - for serial port) */
#define XF_FLAG     0x0010  /* XF   (XF output pin status) */
#define FO_FLAG     0x0008  /* FO   (Serial port Format In/Out mode) */
#define TXM_FLAG    0x0004  /* TXM  (Transmit Mode - for serial port) */
#define PM_REG      0x0003  /* PM   (Product shift Mode) */


#define OV      ( m_STR0 & OV_FLAG)         /* OV   (Overflow flag) */
#define OVM     ( m_STR0 & OVM_FLAG)        /* OVM  (Overflow Mode bit) 1 indicates an overflow */
#define INTM    ( m_STR0 & INTM_FLAG)       /* INTM (Interrupt enable flag) 0 enables maskable interrupts */
#define ARP     ((m_STR0 & ARP_REG) >> 13)  /* ARP  (Auxiliary Register Pointer) */
#define DP      ((m_STR0 & DP_REG) << 7)    /* DP   (Data memory Pointer bit) */
#define ARB     ( m_STR1 & ARB_REG)         /* ARB  (Backup Auxiliary Register pointer) */
#define CNF0    ( m_STR1 & CNF0_REG)        /* CNF0 (Onchip Ram Config register) */
#define TC      ( m_STR1 & TC_FLAG)         /* TC   (Test Control Flag) */
#define SXM     ( m_STR1 & SXM_FLAG)        /* SXM  (Sign Extension Mode) */
#define CARRY   ( m_STR1 & C_FLAG)          /* C    (Carry Flag for accumulator) */
#define HM      ( m_STR1 & HM_FLAG)         /* HM   (Processor Hold Mode) */
#define FSM     ( m_STR1 & FSM_FLAG)        /* FSM  (Frame Synchronization Mode - for serial port) */
#define XF      ( m_STR1 & FSM_FLAG)        /* XF   (XF output pin status) */
#define FO      ( m_STR1 & FO_FLAG)         /* FO   (Serial port Format In/Out mode) */
#define TXM     ( m_STR1 & TXM_FLAG)        /* TXM  (Transmit Mode - for serial port) */
#define PM      ( m_STR1 & PM_REG)          /* PM   (P register shift Mode. See SHIFT_Preg_TO_ALU below )*/

#define DMA     (DP | (m_opcode.b.l & 0x7f))    /* address used in direct memory access operations */
#define DMApg0  (m_opcode.b.l & 0x7f)           /* address used in direct memory access operations for sst instruction */
#define IND     m_AR[ARP]                       /* address used in indirect memory access operations */


const device_type TMS32025 = &device_creator<tms32025_device>;
const device_type TMS32026 = &device_creator<tms32026_device>;


tms32025_device::tms32025_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS32025, "TMS32025", tag, owner, clock, "tms32025", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 16, 16, -1)
	, m_data_config("data", ENDIANNESS_BIG, 16, 16, -1)
	, m_io_config("io", ENDIANNESS_BIG, 16, 17, -1)
{
}


tms32025_device::tms32025_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 16, 16, -1)
	, m_data_config("data", ENDIANNESS_BIG, 16, 16, -1)
	, m_io_config("io", ENDIANNESS_BIG, 16, 16, -1)
{
}


tms32026_device::tms32026_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms32025_device(mconfig, TMS32026, "TMS32026", tag, owner, clock, "tms32026", __FILE__)
{
}


offs_t tms32025_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms32025 );
	return CPU_DISASSEMBLE_NAME(tms32025)(this, buffer, pc, oprom, opram, options);
}


void tms32025_device::CLR0(UINT16 flag) { m_STR0 &= ~flag; m_STR0 |= 0x0400; }
void tms32025_device::SET0(UINT16 flag) { m_STR0 |=  flag; m_STR0 |= 0x0400; }
void tms32025_device::CLR1(UINT16 flag) { m_STR1 &= ~flag; m_STR1 |= 0x0180; }
void tms32025_device::SET1(UINT16 flag) { m_STR1 |=  flag; m_STR1 |= 0x0180; }

void tms32025_device::MODIFY_DP(int data)
{
	m_STR0 &= ~DP_REG;
	m_STR0 |= (data & DP_REG);
	m_STR0 |= 0x0400;
}
void tms32025_device::MODIFY_PM(int data)
{
	m_STR1 &= ~PM_REG;
	m_STR1 |= (data & PM_REG);
	m_STR1 |= 0x0180;
}
void tms32025_device::MODIFY_ARP(int data)
{
	m_STR1 &= ~ARB_REG;
	m_STR1 |= (m_STR0 & ARP_REG);
	m_STR1 |= 0x0180;
	m_STR0 &= ~ARP_REG;
	m_STR0 |= ((data << 13) & ARP_REG);
	m_STR0 |= 0x0400;
}

UINT16 tms32025_device::M_RDROM(offs_t addr)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = m_pgmmap[addr >> 7];
	if (ram) return ram[addr & 0x7f];
	return m_program->read_word(addr << 1);
}

void tms32025_device::M_WRTROM(offs_t addr, UINT16 data)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = m_pgmmap[addr >> 7];
	if (ram) { ram[addr & 0x7f] = data; }
	else m_program->write_word(addr << 1, data);
}

UINT16 tms32025_device::M_RDRAM(offs_t addr)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = m_datamap[addr >> 7];
	if (ram) return ram[addr & 0x7f];
	return m_data->read_word(addr << 1);
}

void tms32025_device::M_WRTRAM(offs_t addr, UINT16 data)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = m_datamap[addr >> 7];
	if (ram) {
		ram[addr & 0x7f] = data;
		if(addr == 1 && ram == m_intRAM && TXM) {
			if(FSM)
				m_waiting_for_serial_frame = 1;
			else
				m_IFR |= 0x20;
		}
	}
	else m_data->write_word(addr << 1, data);
}


UINT16 tms32025_device::reverse_carry_add(UINT16 arg0, UINT16 arg1 )
{
	UINT16 result = 0;
	int carry = 0;
	int count;
	for( count=0; count<16; count++ )
	{
		int sum = (arg0>>15)+(arg1>>15)+carry;
		result = (result<<1)|(sum&1);
		carry = sum>>1;
		arg0<<=1;
		arg1<<=1;
	}
	return result;
}

void tms32025_device::MODIFY_AR_ARP()
{ /* modify address register referenced by ARP */
	switch (m_opcode.b.l & 0x70)        /* Cases ordered by predicted useage */
	{
		case 0x00: /* 000   nop      */
			break;

		case 0x10: /* 001   *-       */
			m_AR[ARP] -- ;
			break;

		case 0x20: /* 010   *+       */
			m_AR[ARP] ++ ;
			break;

		case 0x30: /* 011   reserved */
			break;

		case 0x40: /* 100   *BR0-    */
			m_AR[ARP] = reverse_carry_add(m_AR[ARP],-m_AR[0]);
			break;

		case 0x50: /* 101   *0-      */
			m_AR[ARP] -= m_AR[0];
			break;

		case 0x60: /* 110   *0+      */
			m_AR[ARP] += m_AR[0];
			break;

		case 0x70: /* 111   *BR0+    */
			m_AR[ARP] += reverse_carry_add(m_AR[ARP],m_AR[0]);
			break;
	}

	if( !m_mHackIgnoreARP )
	{
		if (m_opcode.b.l & 8)
		{ /* bit 3 determines if new value is loaded into ARP */
			MODIFY_ARP((m_opcode.b.l & 7) );
		}
	}
}

void tms32025_device::CALCULATE_ADD_CARRY()
{
	if ( (UINT32)(m_oldacc.d) > (UINT32)(m_ACC.d) ) {
		SET1(C_FLAG);
	}
	else {
		CLR1(C_FLAG);
	}
}

void tms32025_device::CALCULATE_SUB_CARRY()
{
	if ( (UINT32)(m_oldacc.d) < (UINT32)(m_ACC.d) ) {
		CLR1(C_FLAG);
	}
	else {
		SET1(C_FLAG);
	}
}

void tms32025_device::CALCULATE_ADD_OVERFLOW(INT32 addval)
{
	if ((INT32)((m_ACC.d ^ addval) & (m_oldacc.d ^ m_ACC.d)) < 0)
	{
		SET0(OV_FLAG);
		if (OVM)
		{
			m_ACC.d = ((INT32)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
		}
	}
}
void tms32025_device::CALCULATE_SUB_OVERFLOW(INT32 subval)
{
	if ((INT32)((m_oldacc.d ^ subval) & (m_oldacc.d ^ m_ACC.d)) < 0)
	{
		SET0(OV_FLAG);
		if (OVM)
		{
			m_ACC.d = ((INT32)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
		}
	}
}

UINT16 tms32025_device::POP_STACK()
{
	UINT16 data = m_STACK[7];
	m_STACK[7] = m_STACK[6];
	m_STACK[6] = m_STACK[5];
	m_STACK[5] = m_STACK[4];
	m_STACK[4] = m_STACK[3];
	m_STACK[3] = m_STACK[2];
	m_STACK[2] = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return data;
}
void tms32025_device::PUSH_STACK(UINT16 data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = m_STACK[2];
	m_STACK[2] = m_STACK[3];
	m_STACK[3] = m_STACK[4];
	m_STACK[4] = m_STACK[5];
	m_STACK[5] = m_STACK[6];
	m_STACK[6] = m_STACK[7];
	m_STACK[7] = data;
}

void tms32025_device::SHIFT_Preg_TO_ALU()
{
	switch(PM)      /* PM (in STR1) is the shift mode for Preg */
	{
		case 0:     m_ALU.d = m_Preg.d; break;
		case 1:     m_ALU.d = (m_Preg.d << 1); break;
		case 2:     m_ALU.d = (m_Preg.d << 4); break;
		case 3:     m_ALU.d = (m_Preg.d >> 6); if (m_Preg.d & 0x80000000) m_ALU.d |= 0xfc000000; break;
		default:    break;
	}
}

void tms32025_device::GETDATA(int shift,int signext)
{
	if (m_opcode.b.l & 0x80)
	{ /* indirect memory access */
		m_memaccess = IND;
	}
	else
	{ /* direct memory address */
		m_memaccess = DMA;
	}

	if (m_memaccess >= 0x800)
	{
		m_external_mem_access = 1;  /* Pause if hold pin is active */
	}
	else
	{
		m_external_mem_access = 0;
	}

	m_ALU.d = (UINT16)M_RDRAM(m_memaccess);
	if (signext) m_ALU.d = (INT16)m_ALU.d;
	m_ALU.d <<= shift;

	/* next ARP */
	if (m_opcode.b.l & 0x80) MODIFY_AR_ARP();
}

void tms32025_device::PUTDATA(UINT16 data)
{
	if (m_opcode.b.l & 0x80) {
		if (m_memaccess >= 0x800) m_external_mem_access = 1;    /* Pause if hold pin is active */
		else m_external_mem_access = 0;

		M_WRTRAM(IND, data);
		MODIFY_AR_ARP();
	}
	else {
		if (m_memaccess >= 0x800) m_external_mem_access = 1;    /* Pause if hold pin is active */
		else m_external_mem_access = 0;

		M_WRTRAM(DMA, data);
	}
}
void tms32025_device::PUTDATA_SST(UINT16 data)
{
	if (m_opcode.b.l & 0x80) m_memaccess = IND;
	else m_memaccess = DMApg0;

	if (m_memaccess >= 0x800) m_external_mem_access = 1;        /* Pause if hold pin is active */
	else m_external_mem_access = 0;

	if (m_opcode.b.l & 0x80) {
		m_opcode.b.l &= 0xf7;                   /* Stop ARP changes */
		MODIFY_AR_ARP();
	}
	M_WRTRAM(m_memaccess, data);
}



/****************************************************************************
 *  Emulate the Instructions
 ****************************************************************************/

/* The following functions are here to fill the void for the */
/* opcode call functions. These functions are never actually called. */
void tms32025_device::opcodes_CE() { fatalerror("Should never get here!\n"); }
void tms32025_device::opcodes_Dx() { fatalerror("Should never get here!\n"); }

void tms32025_device::illegal()
{
	logerror("TMS32025:  PC = %04x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.w.l);
}

void tms32025_device::abst()
{
	if ( (INT32)(m_ACC.d) < 0 ) {
		m_ACC.d = -m_ACC.d;
		if (m_ACC.d == 0x80000000) {
			SET0(OV_FLAG);
			if (OVM) m_ACC.d-- ;
		}
	}
	CLR1(C_FLAG);
}
void tms32025_device::add()
{
	m_oldacc.d = m_ACC.d;
	GETDATA((m_opcode.b.h & 0xf), SXM);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::addc()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	if (CARRY) m_ACC.d++;
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	if (m_ACC.d == m_oldacc.d) {}   /* edge case, carry remains same */
	else CALCULATE_ADD_CARRY();
}
void tms32025_device::addh()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_ACC.w.h += m_ALU.w.l;
	if ( (UINT16)(m_oldacc.w.h) > (UINT16)(m_ACC.w.h) ) {
		SET1(C_FLAG); /* Carry flag is not cleared, if no carry occurred */
	}
	if ((INT16)((m_ACC.w.h ^ m_ALU.w.l) & (m_oldacc.w.h ^ m_ACC.w.h)) < 0) {
		SET0(OV_FLAG);
		if (OVM) m_ACC.w.h = ((INT16)m_oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
void tms32025_device::addk()
{
	m_oldacc.d = m_ACC.d;
	m_ALU.d = (UINT8)m_opcode.b.l;
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::adds()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::addt()
{
	m_oldacc.d = m_ACC.d;
	GETDATA((m_Treg & 0xf), SXM);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::adlk()
{
	m_oldacc.d = m_ACC.d;
	if (SXM) m_ALU.d =  (INT16)M_RDOP_ARG(m_PC);
	else     m_ALU.d = (UINT16)M_RDOP_ARG(m_PC);
	m_PC++;
	m_ALU.d <<= (m_opcode.b.h & 0xf);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::adrk()
{
	m_AR[ARP] += m_opcode.b.l;
}
void tms32025_device::and_()
{
	GETDATA(0, 0);
	m_ACC.d &= m_ALU.d;
}
void tms32025_device::andk()
{
	m_oldacc.d = m_ACC.d;
	m_ALU.d = (UINT16)M_RDOP_ARG(m_PC);
	m_PC++;
	m_ALU.d <<= (m_opcode.b.h & 0xf);
	m_ACC.d &= m_ALU.d;
}
void tms32025_device::apac()
{
	m_oldacc.d = m_ACC.d;
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::br()
{
	SET_PC(M_RDOP_ARG(m_PC));
	MODIFY_AR_ARP();
}
void tms32025_device::bacc()
{
	SET_PC(m_ACC.w.l);
}
void tms32025_device::banz()
{
	if (m_AR[ARP]) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bbnz()
{
	if (TC) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bbz()
{
	if (TC == 0) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bc()
{
	if (CARRY) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bgez()
{
	if ( (INT32)(m_ACC.d) >= 0 ) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bgz()
{
	if ( (INT32)(m_ACC.d) > 0 ) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bioz()
{
	if (S_IN(TMS32025_BIO) != CLEAR_LINE) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bit()
{
	GETDATA(0, 0);
	if (m_ALU.d & (0x8000 >> (m_opcode.b.h & 0xf))) SET1(TC_FLAG);
	else CLR1(TC_FLAG);
}
void tms32025_device::bitt()
{
	GETDATA(0, 0);
	if (m_ALU.d & (0x8000 >> (m_Treg & 0xf))) SET1(TC_FLAG);
	else CLR1(TC_FLAG);
}
void tms32025_device::blez()
{
	if ( (INT32)(m_ACC.d) <= 0 ) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::blkd()
{                                       /** Fix cycle timing **/
	if (m_init_load_addr) {
		m_PFC = M_RDOP_ARG(m_PC);
		m_PC++;
	}
	m_ALU.d = M_RDRAM(m_PFC);
	PUTDATA(m_ALU.d);
	m_PFC++;
	m_tms32025_dec_cycles += (1*CLK);
}
void tms32025_device::blkp()
{                                       /** Fix cycle timing **/
	if (m_init_load_addr) {
		m_PFC = M_RDOP_ARG(m_PC);
		m_PC++;
	}
	m_ALU.d = M_RDROM(m_PFC);
	PUTDATA(m_ALU.d);
	m_PFC++;
	m_tms32025_dec_cycles += (2*CLK);
}
void tms32025_device::blz()
{
	if ( (INT32)(m_ACC.d) <  0 ) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bnc()
{
	if (CARRY == 0) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bnv()
{
	if (OV == 0) SET_PC(M_RDOP_ARG(m_PC));
	else {
		m_PC++ ;
		CLR0(OV_FLAG);
	}
	MODIFY_AR_ARP();
}
void tms32025_device::bnz()
{
	if (m_ACC.d != 0) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bv()
{
	if (OV) {
		SET_PC(M_RDOP_ARG(m_PC));
		CLR0(OV_FLAG);
	}
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::bz()
{
	if (m_ACC.d == 0) SET_PC(M_RDOP_ARG(m_PC));
	else m_PC++ ;
	MODIFY_AR_ARP();
}
void tms32025_device::cala()
{
	PUSH_STACK(m_PC);
	SET_PC(m_ACC.w.l);
}
void tms32025_device::call()
{
	m_PC++ ;
	PUSH_STACK(m_PC);
	SET_PC(M_RDOP_ARG((m_PC - 1)));
	MODIFY_AR_ARP();
}
void tms32025_device::cmpl()
{
	m_ACC.d = (~m_ACC.d);
}
void tms32025_device::cmpr()
{
	switch (m_opcode.b.l & 3)
	{
		case 00:    if ( (UINT16)(m_AR[ARP]) == (UINT16)(m_AR[0]) ) SET1(TC_FLAG);
					else CLR1(TC_FLAG);
					break;
		case 01:    if ( (UINT16)(m_AR[ARP]) <  (UINT16)(m_AR[0]) ) SET1(TC_FLAG);
					else CLR1(TC_FLAG);
					break;
		case 02:    if ( (UINT16)(m_AR[ARP])  > (UINT16)(m_AR[0]) ) SET1(TC_FLAG);
					else CLR1(TC_FLAG);
					break;
		case 03:    if ( (UINT16)(m_AR[ARP]) != (UINT16)(m_AR[0]) ) SET1(TC_FLAG);
					else CLR1(TC_FLAG);
					break;
	}
}
void tms32025_device::cnfd()  /** next two fetches need to use previous CNF value ! **/
{
	CLR1(CNF0_REG);
	m_datamap[4] = &m_intRAM[0x200];            /* B0 */
	m_datamap[5] = &m_intRAM[0x280];            /* B0 */
	m_pgmmap[510] = nullptr;
	m_pgmmap[511] = nullptr;
}
void tms32025_device::cnfp()  /** next two fetches need to use previous CNF value ! **/
{
	SET1(CNF0_REG);
	m_datamap[4] = nullptr;                        /* B0 */
	m_datamap[5] = nullptr;                        /* B0 */
	m_pgmmap[510] = &m_intRAM[0x200];
	m_pgmmap[511] = &m_intRAM[0x280];
}
void tms32025_device::conf()  /** Need to reconfigure the memory blocks */
{
	switch (m_opcode.b.l & 3)
	{
		case 00:    CLR1(CNF1_REG); CLR1(CNF0_REG);
					m_datamap[4] = &m_intRAM[0x200];    /* B0 */
					m_datamap[5] = &m_intRAM[0x280];    /* B0 */
					m_datamap[6] = &m_intRAM[0x300];    /* B0 */
					m_datamap[7] = &m_intRAM[0x380];    /* B0 */
					m_datamap[8] = &m_intRAM[0x400];    /* B1 */
					m_datamap[9] = &m_intRAM[0x480];    /* B1 */
					m_datamap[10] = &m_intRAM[0x500];   /* B1 */
					m_datamap[11] = &m_intRAM[0x580];   /* B1 */
					m_datamap[12] = &m_intRAM[0x600];   /* B3 */
					m_datamap[13] = &m_intRAM[0x680];   /* B3 */
					m_datamap[14] = &m_intRAM[0x700];   /* B3 */
					m_datamap[15] = &m_intRAM[0x780];   /* B3 */
					m_pgmmap[500] = nullptr;
					m_pgmmap[501] = nullptr;
					m_pgmmap[502] = nullptr;
					m_pgmmap[503] = nullptr;
					m_pgmmap[504] = nullptr;
					m_pgmmap[505] = nullptr;
					m_pgmmap[506] = nullptr;
					m_pgmmap[507] = nullptr;
					m_pgmmap[508] = nullptr;
					m_pgmmap[509] = nullptr;
					m_pgmmap[510] = nullptr;
					m_pgmmap[511] = nullptr;
					break;

		case 01:    CLR1(CNF1_REG); SET1(CNF0_REG);
					m_datamap[4] = nullptr;
					m_datamap[5] = nullptr;
					m_datamap[6] = nullptr;
					m_datamap[7] = nullptr;
					m_datamap[8] = &m_intRAM[0x400];    /* B1 */
					m_datamap[9] = &m_intRAM[0x480];    /* B1 */
					m_datamap[10] = &m_intRAM[0x500];   /* B1 */
					m_datamap[11] = &m_intRAM[0x580];   /* B1 */
					m_datamap[12] = &m_intRAM[0x600];   /* B3 */
					m_datamap[13] = &m_intRAM[0x680];   /* B3 */
					m_datamap[14] = &m_intRAM[0x700];   /* B3 */
					m_datamap[15] = &m_intRAM[0x780];   /* B3 */
					m_pgmmap[500] = &m_intRAM[0x200];   /* B0 */
					m_pgmmap[501] = &m_intRAM[0x280];   /* B0 */
					m_pgmmap[502] = &m_intRAM[0x300];   /* B0 */
					m_pgmmap[503] = &m_intRAM[0x380];   /* B0 */
					m_pgmmap[504] = nullptr;
					m_pgmmap[505] = nullptr;
					m_pgmmap[506] = nullptr;
					m_pgmmap[507] = nullptr;
					m_pgmmap[508] = nullptr;
					m_pgmmap[509] = nullptr;
					m_pgmmap[510] = nullptr;
					m_pgmmap[511] = nullptr;
					break;

		case 02:    SET1(CNF1_REG); CLR1(CNF0_REG);
					m_datamap[4] = nullptr;
					m_datamap[5] = nullptr;
					m_datamap[6] = nullptr;
					m_datamap[7] = nullptr;
					m_datamap[8] = nullptr;
					m_datamap[9] = nullptr;
					m_datamap[10] = nullptr;
					m_datamap[11] = nullptr;
					m_datamap[12] = &m_intRAM[0x600];   /* B3 */
					m_datamap[13] = &m_intRAM[0x680];   /* B3 */
					m_datamap[14] = &m_intRAM[0x700];   /* B3 */
					m_datamap[15] = &m_intRAM[0x780];   /* B3 */
					m_pgmmap[500] = &m_intRAM[0x200];   /* B0 */
					m_pgmmap[501] = &m_intRAM[0x280];   /* B0 */
					m_pgmmap[502] = &m_intRAM[0x300];   /* B0 */
					m_pgmmap[503] = &m_intRAM[0x380];   /* B0 */
					m_pgmmap[504] = &m_intRAM[0x400];   /* B1 */
					m_pgmmap[505] = &m_intRAM[0x480];   /* B1 */
					m_pgmmap[506] = &m_intRAM[0x500];   /* B1 */
					m_pgmmap[507] = &m_intRAM[0x580];   /* B1 */
					m_pgmmap[508] = nullptr;
					m_pgmmap[509] = nullptr;
					m_pgmmap[510] = nullptr;
					m_pgmmap[511] = nullptr;
					break;

		case 03:    SET1(CNF1_REG); SET1(CNF0_REG);
					m_datamap[4] = nullptr;
					m_datamap[5] = nullptr;
					m_datamap[6] = nullptr;
					m_datamap[7] = nullptr;
					m_datamap[8] = nullptr;
					m_datamap[9] = nullptr;
					m_datamap[10] = nullptr;
					m_datamap[11] = nullptr;
					m_datamap[12] = nullptr;
					m_datamap[13] = nullptr;
					m_datamap[14] = nullptr;
					m_datamap[15] = nullptr;
					m_pgmmap[500] = &m_intRAM[0x200];   /* B0 */
					m_pgmmap[501] = &m_intRAM[0x280];   /* B0 */
					m_pgmmap[502] = &m_intRAM[0x300];   /* B0 */
					m_pgmmap[503] = &m_intRAM[0x380];   /* B0 */
					m_pgmmap[504] = &m_intRAM[0x400];   /* B1 */
					m_pgmmap[505] = &m_intRAM[0x480];   /* B1 */
					m_pgmmap[506] = &m_intRAM[0x500];   /* B1 */
					m_pgmmap[507] = &m_intRAM[0x580];   /* B1 */
					m_pgmmap[508] = &m_intRAM[0x600];   /* B3 */
					m_pgmmap[509] = &m_intRAM[0x680];   /* B3 */
					m_pgmmap[510] = &m_intRAM[0x700];   /* B3 */
					m_pgmmap[511] = &m_intRAM[0x780];   /* B3 */
					break;
	}
}
void tms32025_device::dint()
{
	SET0(INTM_FLAG);
}
void tms32025_device::dmov()  /** Careful with how memory is configured !! */
{
	GETDATA(0, 0);
	M_WRTRAM((m_memaccess + 1), m_ALU.w.l);
}
void tms32025_device::eint()
{
	CLR0(INTM_FLAG);
}
void tms32025_device::fort()
{
	if (m_opcode.b.l & 1) SET1(FO_FLAG);
	else CLR1(FO_FLAG);
}
void tms32025_device::idle()
{
	CLR0(INTM_FLAG);
	m_idle = 1;
}
void tms32025_device::in()
{
	m_ALU.w.l = P_IN( (m_opcode.b.h & 0xf) );
	PUTDATA(m_ALU.w.l);
}
void tms32025_device::lac()
{
	GETDATA((m_opcode.b.h & 0xf), SXM);
	m_ACC.d = m_ALU.d;
}
void tms32025_device::lack()      /* ZAC is a subset of this instruction */
{
	m_ACC.d = (UINT8)m_opcode.b.l;
}
void tms32025_device::lact()
{
	GETDATA((m_Treg & 0xf), SXM);
	m_ACC.d = m_ALU.d;
}
void tms32025_device::lalk()
{
	if (SXM) m_ALU.d =  (INT16)M_RDOP_ARG(m_PC);
	else     m_ALU.d = (UINT16)M_RDOP_ARG(m_PC);
	m_PC++;
	m_ALU.d <<= (m_opcode.b.h & 0xf);
	m_ACC.d = m_ALU.d;
}
void tms32025_device::lar_ar0()   { GETDATA(0, 0); m_AR[0] = m_ALU.w.l; }
void tms32025_device::lar_ar1()   { GETDATA(0, 0); m_AR[1] = m_ALU.w.l; }
void tms32025_device::lar_ar2()   { GETDATA(0, 0); m_AR[2] = m_ALU.w.l; }
void tms32025_device::lar_ar3()   { GETDATA(0, 0); m_AR[3] = m_ALU.w.l; }
void tms32025_device::lar_ar4()   { GETDATA(0, 0); m_AR[4] = m_ALU.w.l; }
void tms32025_device::lar_ar5()   { GETDATA(0, 0); m_AR[5] = m_ALU.w.l; }
void tms32025_device::lar_ar6()   { GETDATA(0, 0); m_AR[6] = m_ALU.w.l; }
void tms32025_device::lar_ar7()   { GETDATA(0, 0); m_AR[7] = m_ALU.w.l; }
void tms32025_device::lark_ar0()  { m_AR[0] = m_opcode.b.l; }
void tms32025_device::lark_ar1()  { m_AR[1] = m_opcode.b.l; }
void tms32025_device::lark_ar2()  { m_AR[2] = m_opcode.b.l; }
void tms32025_device::lark_ar3()  { m_AR[3] = m_opcode.b.l; }
void tms32025_device::lark_ar4()  { m_AR[4] = m_opcode.b.l; }
void tms32025_device::lark_ar5()  { m_AR[5] = m_opcode.b.l; }
void tms32025_device::lark_ar6()  { m_AR[6] = m_opcode.b.l; }
void tms32025_device::lark_ar7()  { m_AR[7] = m_opcode.b.l; }
void tms32025_device::ldp()
{
	GETDATA(0, 0);
	MODIFY_DP(m_ALU.d & 0x1ff);
}
void tms32025_device::ldpk()
{
	MODIFY_DP(m_opcode.w.l & 0x1ff);
}
void tms32025_device::lph()
{
	GETDATA(0, 0);
	m_Preg.w.h = m_ALU.w.l;
}
void tms32025_device::lrlk()
{
	m_ALU.d = (UINT16)M_RDOP_ARG(m_PC);
	m_PC++;
	m_AR[m_opcode.b.h & 7] = m_ALU.w.l;
}
void tms32025_device::lst()
{
	m_mHackIgnoreARP = 1;
	GETDATA(0, 0);
	m_mHackIgnoreARP = 0;

	m_ALU.w.l &= (~INTM_FLAG);
	m_STR0 &= INTM_FLAG;
	m_STR0 |= m_ALU.w.l;        /* Must not affect INTM */
	m_STR0 |= 0x0400;
}
void tms32025_device::lst1()
{
	m_mHackIgnoreARP = 1;
	GETDATA(0, 0);
	m_mHackIgnoreARP = 0;

	m_STR1 = m_ALU.w.l;
	m_STR1 |= 0x0180;
	m_STR0 &= (~ARP_REG);       /* ARB also gets copied to ARP */
	m_STR0 |= (m_STR1 & ARB_REG);
}
void tms32025_device::lt()
{
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
}
void tms32025_device::lta()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::ltd()   /** Careful with how memory is configured !! */
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	M_WRTRAM((m_memaccess+1), m_ALU.w.l);
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
}
void tms32025_device::ltp()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	SHIFT_Preg_TO_ALU();
	m_ACC.d = m_ALU.d;
}
void tms32025_device::lts()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	SHIFT_Preg_TO_ALU();
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::mac()           /** RAM blocks B0,B1,B2 may be important ! */
{                               /** Fix cycle timing **/
	m_oldacc.d = m_ACC.d;
	if (m_init_load_addr) {
		m_PFC = M_RDOP_ARG(m_PC);
		m_PC++;
	}
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	m_Preg.d = ( (INT16)m_ALU.w.l * (INT16)M_RDROM(m_PFC) );
	m_PFC++;
	m_tms32025_dec_cycles += (2*CLK);
}
void tms32025_device::macd()          /** RAM blocks B0,B1,B2 may be important ! */
{                                                   /** Fix cycle timing **/
	m_oldacc.d = m_ACC.d;
	if (m_init_load_addr) {
		m_PFC = M_RDOP_ARG(m_PC);
		m_PC++;
	}
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
	GETDATA(0, 0);
	if ( (m_opcode.b.l & 0x80) || m_init_load_addr ) {  /* No writing during repetition, or DMA mode */
		M_WRTRAM((m_memaccess+1), m_ALU.w.l);
	}
	m_Treg = m_ALU.w.l;
	m_Preg.d = ( (INT16)m_ALU.w.l * (INT16)M_RDROM(m_PFC) );
	m_PFC++;
	m_tms32025_dec_cycles += (2*CLK);
}
void tms32025_device::mar()       /* LARP and NOP are a subset of this instruction */
{
	if (m_opcode.b.l & 0x80) MODIFY_AR_ARP();
}
void tms32025_device::mpy()
{
	GETDATA(0, 0);
	m_Preg.d = (INT16)(m_ALU.w.l) * (INT16)(m_Treg);
}
void tms32025_device::mpya()
{
	m_oldacc.d = m_ACC.d;
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
	GETDATA(0, 0);
	m_Preg.d = (INT16)(m_ALU.w.l) * (INT16)(m_Treg);
}
void tms32025_device::mpyk()
{
	m_Preg.d = (INT16)m_Treg * ((INT16)(m_opcode.w.l << 3) >> 3);

}
void tms32025_device::mpys()
{
	m_oldacc.d = m_ACC.d;
	SHIFT_Preg_TO_ALU();
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
	GETDATA(0, 0);
	m_Preg.d = (INT16)(m_ALU.w.l) * (INT16)(m_Treg);
}
void tms32025_device::mpyu()
{
	GETDATA(0, 0);
	m_Preg.d = (UINT16)(m_ALU.w.l) * (UINT16)(m_Treg);
}
void tms32025_device::neg()
{
	if (m_ACC.d == 0x80000000) {
		SET0(OV_FLAG);
		if (OVM) m_ACC.d = 0x7fffffff;
	}
	else m_ACC.d = -m_ACC.d;
	if (m_ACC.d) CLR0(C_FLAG);
	else SET0(C_FLAG);
}
/*
void tms32025_device::nop() { }   // NOP is a subset of the MAR instruction
*/
void tms32025_device::norm()
{
	if (m_ACC.d !=0 && (INT32)(m_ACC.d ^ (m_ACC.d << 1)) >= 0)
	{
		CLR1(TC_FLAG);
		m_ACC.d <<= 1;
		MODIFY_AR_ARP();
	}
	else SET1(TC_FLAG);
}
void tms32025_device::or_()
{
	GETDATA(0, 0);
	m_ACC.w.l |= m_ALU.w.l;
}
void tms32025_device::ork()
{
	m_ALU.d = (UINT16)M_RDOP_ARG(m_PC);
	m_PC++;
	m_ALU.d <<= (m_opcode.b.h & 0xf);
	m_ACC.d |=  (m_ALU.d);
}
void tms32025_device::out()
{
	GETDATA(0, 0);
	P_OUT( (m_opcode.b.h & 0xf), m_ALU.w.l );
}
void tms32025_device::pac()
{
	SHIFT_Preg_TO_ALU();
	m_ACC.d = m_ALU.d;
}
void tms32025_device::pop()
{
	m_ACC.d = (UINT16)POP_STACK();
}
void tms32025_device::popd()
{
	m_ALU.d = (UINT16)POP_STACK();
	PUTDATA(m_ALU.w.l);
}
void tms32025_device::pshd()
{
	GETDATA(0, 0);
	PUSH_STACK(m_ALU.w.l);
}
void tms32025_device::push()
{
	PUSH_STACK(m_ACC.w.l);
}
void tms32025_device::rc()
{
	CLR1(C_FLAG);
}
void tms32025_device::ret()
{
	SET_PC(POP_STACK());
}
void tms32025_device::rfsm()              /** serial port mode */
{
	CLR1(FSM_FLAG);
}
void tms32025_device::rhm()
{
	CLR1(HM_FLAG);
}
void tms32025_device::rol()
{
	m_ALU.d = m_ACC.d;
	m_ACC.d <<= 1;
	if (CARRY) m_ACC.d |= 1;
	if (m_ALU.d & 0x80000000) SET1(C_FLAG);
	else CLR1(C_FLAG);
}
void tms32025_device::ror()
{
	m_ALU.d = m_ACC.d;
	m_ACC.d >>= 1;
	if (CARRY) m_ACC.d |= 0x80000000;
	if (m_ALU.d & 1) SET1(C_FLAG);
	else CLR1(C_FLAG);
}
void tms32025_device::rovm()
{
	CLR0(OVM_FLAG);
}
void tms32025_device::rpt()
{
	GETDATA(0, 0);
	m_RPTC = m_ALU.b.l;
	m_init_load_addr = 2;       /* Initiate repeat mode */
}
void tms32025_device::rptk()
{
	m_RPTC = m_opcode.b.l;
	m_init_load_addr = 2;       /* Initiate repeat mode */
}
void tms32025_device::rsxm()
{
	CLR1(SXM_FLAG);
}
void tms32025_device::rtc()
{
	CLR1(TC_FLAG);
}
void tms32025_device::rtxm()  /** Serial port stuff */
{
	CLR1(TXM_FLAG);
}
void tms32025_device::rxf()
{
	CLR1(XF_FLAG);
	S_OUT(TMS32025_XF,CLEAR_LINE);
}
void tms32025_device::sach()
{
	m_ALU.d = (m_ACC.d << (m_opcode.b.h & 7));
	PUTDATA(m_ALU.w.h);
}
void tms32025_device::sacl()
{
	m_ALU.d = (m_ACC.d << (m_opcode.b.h & 7));
	PUTDATA(m_ALU.w.l);
}
void tms32025_device::sar_ar0()   { PUTDATA(m_AR[0]); }
void tms32025_device::sar_ar1()   { PUTDATA(m_AR[1]); }
void tms32025_device::sar_ar2()   { PUTDATA(m_AR[2]); }
void tms32025_device::sar_ar3()   { PUTDATA(m_AR[3]); }
void tms32025_device::sar_ar4()   { PUTDATA(m_AR[4]); }
void tms32025_device::sar_ar5()   { PUTDATA(m_AR[5]); }
void tms32025_device::sar_ar6()   { PUTDATA(m_AR[6]); }
void tms32025_device::sar_ar7()   { PUTDATA(m_AR[7]); }

void tms32025_device::sblk()
{
	m_oldacc.d = m_ACC.d;
	if (SXM) m_ALU.d =  (INT16)M_RDOP_ARG(m_PC);
	else     m_ALU.d = (UINT16)M_RDOP_ARG(m_PC);
	m_PC++;
	m_ALU.d <<= (m_opcode.b.h & 0xf);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::sbrk_ar()
{
	m_AR[ARP] -= m_opcode.b.l;
}
void tms32025_device::sc()
{
	SET1(C_FLAG);
}
void tms32025_device::sfl()
{
	m_ALU.d = m_ACC.d;
	m_ACC.d <<= 1;
	if (m_ALU.d & 0x80000000) SET1(C_FLAG);
	else CLR1(C_FLAG);
}
void tms32025_device::sfr()
{
	m_ALU.d = m_ACC.d;
	m_ACC.d >>= 1;
	if (SXM) {
		if (m_ALU.d & 0x80000000) m_ACC.d |= 0x80000000;
	}
	if (m_ALU.d & 1) SET1(C_FLAG);
	else CLR1(C_FLAG);
}
void tms32025_device::sfsm()  /** Serial port mode */
{
	SET1(FSM_FLAG);
}
void tms32025_device::shm()
{
	SET1(HM_FLAG);
}
void tms32025_device::sovm()
{
	SET0(OVM_FLAG);
}
void tms32025_device::spac()
{
	m_oldacc.d = m_ACC.d;
	SHIFT_Preg_TO_ALU();
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::sph()
{
	SHIFT_Preg_TO_ALU();
	PUTDATA(m_ALU.w.h);
}
void tms32025_device::spl()
{
	SHIFT_Preg_TO_ALU();
	PUTDATA(m_ALU.w.l);
}
void tms32025_device::spm()
{
	MODIFY_PM((m_opcode.b.l & 3) );
}
void tms32025_device::sqra()
{
	m_oldacc.d = m_ACC.d;
	SHIFT_Preg_TO_ALU();
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
	CALCULATE_ADD_CARRY();
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	m_Preg.d = ((INT16)m_ALU.w.l * (INT16)m_ALU.w.l);
}
void tms32025_device::sqrs()
{
	m_oldacc.d = m_ACC.d;
	SHIFT_Preg_TO_ALU();
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
	GETDATA(0, 0);
	m_Treg = m_ALU.w.l;
	m_Preg.d = ((INT16)m_ALU.w.l * (INT16)m_ALU.w.l);
}
void tms32025_device::sst()
{
	PUTDATA_SST(m_STR0);
}
void tms32025_device::sst1()
{
	PUTDATA_SST(m_STR1);
}
void tms32025_device::ssxm()
{
	SET1(SXM_FLAG);
}
void tms32025_device::stc()
{
	SET1(TC_FLAG);
}
void tms32025_device::stxm()      /** Serial port stuff */
{
	SET1(TXM_FLAG);
}
void tms32025_device::sub()
{
	m_oldacc.d = m_ACC.d;
	GETDATA((m_opcode.b.h & 0xf), SXM);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::subb()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	if (CARRY == 0) m_ACC.d--;
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	if (m_ACC.d == m_oldacc.d) {}   /* edge case, carry remains same */
	else CALCULATE_SUB_CARRY();
}
void tms32025_device::subc()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(15, SXM);
	m_ACC.d -= m_ALU.d;     /* Temporary switch to ACC. Actual calculation is done as (ACC)-[mem] -> ALU, will be preserved later on. */
	if ((INT32)((m_oldacc.d ^ m_ALU.d) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET0(OV_FLAG);            /* Not affected by OVM */
	}
	CALCULATE_SUB_CARRY();
	if( m_oldacc.d >= m_ALU.d ) {
		m_ALU.d = m_ACC.d;
		m_ACC.d = m_ACC.d << 1 | 1;
	}
	else {
		m_ALU.d = m_ACC.d;
		m_ACC.d = m_oldacc.d << 1;
	}
}
void tms32025_device::subh()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_ACC.w.h -= m_ALU.w.l;
	if ( (UINT16)(m_oldacc.w.h) < (UINT16)(m_ACC.w.h) ) {
		CLR1(C_FLAG); /* Carry flag is not affected, if no borrow occurred */
	}
	if ((INT16)((m_oldacc.w.h ^ m_ALU.w.l) & (m_oldacc.w.h ^ m_ACC.w.h)) < 0) {
		SET0(OV_FLAG);
		if (OVM) m_ACC.w.h = ((INT16)m_oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
void tms32025_device::subk()
{
	m_oldacc.d = m_ACC.d;
	m_ALU.d = (UINT8)m_opcode.b.l;
	m_ACC.d -= m_ALU.b.l;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::subs()
{
	m_oldacc.d = m_ACC.d;
	GETDATA(0, 0);
	m_ACC.d -= m_ALU.w.l;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::subt()
{
	m_oldacc.d = m_ACC.d;
	GETDATA((m_Treg & 0xf), SXM);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
	CALCULATE_SUB_CARRY();
}
void tms32025_device::sxf()
{
	SET1(XF_FLAG);
	S_OUT(TMS32025_XF,ASSERT_LINE);
}
void tms32025_device::tblr()
{
	if (m_init_load_addr) {
		m_PFC = m_ACC.w.l;
	}
	m_ALU.w.l = M_RDROM(m_PFC);
	if ( (CNF0) && ( (UINT16)(m_PFC) >= 0xff00 ) ) {}   /** TMS32025 only */
	else m_tms32025_dec_cycles += (1*CLK);
	PUTDATA(m_ALU.w.l);
	m_PFC++;
}
void tms32025_device::tblw()
{
	if (m_init_load_addr) {
		m_PFC = m_ACC.w.l;
	}
	m_tms32025_dec_cycles += (1*CLK);
	GETDATA(0, 0);
	if (m_external_mem_access) m_tms32025_dec_cycles += (1*CLK);
	M_WRTROM(m_PFC, m_ALU.w.l);
	m_PFC++;
}
void tms32025_device::trap()
{
	PUSH_STACK(m_PC);
	SET_PC(0x001E);     /* Trap vector */
}
void tms32025_device::xor_()
{
	GETDATA(0, 0);
	m_ACC.w.l ^= m_ALU.w.l;
}
void tms32025_device::xork()
{
	m_ALU.d = M_RDOP_ARG(m_PC);
	m_PC++;
	m_ALU.d <<= (m_opcode.b.h & 0xf);
	m_ACC.d ^= m_ALU.d;
}
void tms32025_device::zalh()
{
	GETDATA(0, 0);
	m_ACC.w.h = m_ALU.w.l;
	m_ACC.w.l = 0x0000;
}
void tms32025_device::zalr()
{
	GETDATA(0, 0);
	m_ACC.w.h = m_ALU.w.l;
	m_ACC.w.l = 0x8000;
}
void tms32025_device::zals()
{
	GETDATA(0, 0);
	m_ACC.w.l = m_ALU.w.l;
	m_ACC.w.h = 0x0000;
}


/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

const tms32025_device::tms32025_opcode tms32025_device::s_opcode_main[256]=
{
/*00*/ {1*CLK, &tms32025_device::add      },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },
/*08*/ {1*CLK, &tms32025_device::add      },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },{1*CLK, &tms32025_device::add       },
/*10*/ {1*CLK, &tms32025_device::sub      },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },
/*18*/ {1*CLK, &tms32025_device::sub      },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },{1*CLK, &tms32025_device::sub       },
/*20*/ {1*CLK, &tms32025_device::lac      },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },
/*28*/ {1*CLK, &tms32025_device::lac      },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },{1*CLK, &tms32025_device::lac       },
/*30*/ {1*CLK, &tms32025_device::lar_ar0  },{1*CLK, &tms32025_device::lar_ar1   },{1*CLK, &tms32025_device::lar_ar2   },{1*CLK, &tms32025_device::lar_ar3   },{1*CLK, &tms32025_device::lar_ar4   },{1*CLK, &tms32025_device::lar_ar5   },{1*CLK, &tms32025_device::lar_ar6   },{1*CLK, &tms32025_device::lar_ar7   },
/*38*/ {1*CLK, &tms32025_device::mpy      },{1*CLK, &tms32025_device::sqra      },{1*CLK, &tms32025_device::mpya      },{1*CLK, &tms32025_device::mpys      },{1*CLK, &tms32025_device::lt        },{1*CLK, &tms32025_device::lta       },{1*CLK, &tms32025_device::ltp       },{1*CLK, &tms32025_device::ltd       },
/*40*/ {1*CLK, &tms32025_device::zalh     },{1*CLK, &tms32025_device::zals      },{1*CLK, &tms32025_device::lact      },{1*CLK, &tms32025_device::addc      },{1*CLK, &tms32025_device::subh      },{1*CLK, &tms32025_device::subs      },{1*CLK, &tms32025_device::subt      },{1*CLK, &tms32025_device::subc      },
/*48*/ {1*CLK, &tms32025_device::addh     },{1*CLK, &tms32025_device::adds      },{1*CLK, &tms32025_device::addt      },{1*CLK, &tms32025_device::rpt       },{1*CLK, &tms32025_device::xor_      },{1*CLK, &tms32025_device::or_       },{1*CLK, &tms32025_device::and_      },{1*CLK, &tms32025_device::subb      },
/*50*/ {1*CLK, &tms32025_device::lst      },{1*CLK, &tms32025_device::lst1      },{1*CLK, &tms32025_device::ldp       },{1*CLK, &tms32025_device::lph       },{1*CLK, &tms32025_device::pshd      },{1*CLK, &tms32025_device::mar       },{1*CLK, &tms32025_device::dmov      },{1*CLK, &tms32025_device::bitt      },
/*58*/ {3*CLK, &tms32025_device::tblr     },{2*CLK, &tms32025_device::tblw      },{1*CLK, &tms32025_device::sqrs      },{1*CLK, &tms32025_device::lts       },{2*CLK, &tms32025_device::macd      },{2*CLK, &tms32025_device::mac       },{2*CLK, &tms32025_device::bc        },{2*CLK, &tms32025_device::bnc       },
/*60*/ {1*CLK, &tms32025_device::sacl     },{1*CLK, &tms32025_device::sacl      },{1*CLK, &tms32025_device::sacl      },{1*CLK, &tms32025_device::sacl      },{1*CLK, &tms32025_device::sacl      },{1*CLK, &tms32025_device::sacl      },{1*CLK, &tms32025_device::sacl      },{1*CLK, &tms32025_device::sacl      },
/*68*/ {1*CLK, &tms32025_device::sach     },{1*CLK, &tms32025_device::sach      },{1*CLK, &tms32025_device::sach      },{1*CLK, &tms32025_device::sach      },{1*CLK, &tms32025_device::sach      },{1*CLK, &tms32025_device::sach      },{1*CLK, &tms32025_device::sach      },{1*CLK, &tms32025_device::sach      },
/*70*/ {1*CLK, &tms32025_device::sar_ar0  },{1*CLK, &tms32025_device::sar_ar1   },{1*CLK, &tms32025_device::sar_ar2   },{1*CLK, &tms32025_device::sar_ar3   },{1*CLK, &tms32025_device::sar_ar4   },{1*CLK, &tms32025_device::sar_ar5   },{1*CLK, &tms32025_device::sar_ar6   },{1*CLK, &tms32025_device::sar_ar7   },
/*78*/ {1*CLK, &tms32025_device::sst      },{1*CLK, &tms32025_device::sst1      },{1*CLK, &tms32025_device::popd      },{1*CLK, &tms32025_device::zalr      },{1*CLK, &tms32025_device::spl       },{1*CLK, &tms32025_device::sph       },{1*CLK, &tms32025_device::adrk      },{1*CLK, &tms32025_device::sbrk_ar   },
/*80*/ {2*CLK, &tms32025_device::in       },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },
/*88*/ {2*CLK, &tms32025_device::in       },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },{2*CLK, &tms32025_device::in        },
/*90*/ {1*CLK, &tms32025_device::bit      },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },
/*98*/ {1*CLK, &tms32025_device::bit      },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },{1*CLK, &tms32025_device::bit       },
/*A0*/ {1*CLK, &tms32025_device::mpyk     },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },
/*A8*/ {1*CLK, &tms32025_device::mpyk     },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },
/*B0*/ {1*CLK, &tms32025_device::mpyk     },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },
/*B8*/ {1*CLK, &tms32025_device::mpyk     },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },{1*CLK, &tms32025_device::mpyk      },
/*C0*/ {1*CLK, &tms32025_device::lark_ar0 },{1*CLK, &tms32025_device::lark_ar1  },{1*CLK, &tms32025_device::lark_ar2  },{1*CLK, &tms32025_device::lark_ar3  },{1*CLK, &tms32025_device::lark_ar4  },{1*CLK, &tms32025_device::lark_ar5  },{1*CLK, &tms32025_device::lark_ar6  },{1*CLK, &tms32025_device::lark_ar7  },
/*C8*/ {1*CLK, &tms32025_device::ldpk     },{1*CLK, &tms32025_device::ldpk      },{1*CLK, &tms32025_device::lack      },{1*CLK, &tms32025_device::rptk      },{1*CLK, &tms32025_device::addk      },{1*CLK, &tms32025_device::subk      },{1*CLK, &tms32025_device::opcodes_CE},{1*CLK, &tms32025_device::mpyu      },
/*D0*/ {1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{0*CLK, &tms32025_device::opcodes_Dx},
/*D8*/ {1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},{1*CLK, &tms32025_device::opcodes_Dx},
/*E0*/ {2*CLK, &tms32025_device::out      },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },
/*E8*/ {2*CLK, &tms32025_device::out      },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },{2*CLK, &tms32025_device::out       },
/*F0*/ {2*CLK, &tms32025_device::bv       },{2*CLK, &tms32025_device::bgz       },{2*CLK, &tms32025_device::blez      },{2*CLK, &tms32025_device::blz       },{2*CLK, &tms32025_device::bgez      },{2*CLK, &tms32025_device::bnz       },{2*CLK, &tms32025_device::bz        },{2*CLK, &tms32025_device::bnv       },
/*F8*/ {2*CLK, &tms32025_device::bbz      },{2*CLK, &tms32025_device::bbnz      },{2*CLK, &tms32025_device::bioz      },{2*CLK, &tms32025_device::banz      },{2*CLK, &tms32025_device::blkp      },{2*CLK, &tms32025_device::blkd      },{2*CLK, &tms32025_device::call      },{2*CLK, &tms32025_device::br        }
};

const tms32025_device::tms32025_opcode tms32025_device::s_opcode_CE_subset[256]=  /* Instructions living under the CExx opcode */
{
/*00*/ {1*CLK, &tms32025_device::eint     },{1*CLK, &tms32025_device::dint      },{1*CLK, &tms32025_device::rovm      },{1*CLK, &tms32025_device::sovm      },{1*CLK, &tms32025_device::cnfd      },{1*CLK, &tms32025_device::cnfp      },{1*CLK, &tms32025_device::rsxm      },{1*CLK, &tms32025_device::ssxm      },
/*08*/ {1*CLK, &tms32025_device::spm      },{1*CLK, &tms32025_device::spm       },{1*CLK, &tms32025_device::spm       },{1*CLK, &tms32025_device::spm       },{1*CLK, &tms32025_device::rxf       },{1*CLK, &tms32025_device::sxf       },{1*CLK, &tms32025_device::fort      },{1*CLK, &tms32025_device::fort      },
/*10*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::pac       },{1*CLK, &tms32025_device::apac      },{1*CLK, &tms32025_device::spac      },{0*CLK, &tms32025_device::illegal   },
/*18*/ {1*CLK, &tms32025_device::sfl      },{1*CLK, &tms32025_device::sfr       },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::abst      },{1*CLK, &tms32025_device::push      },{1*CLK, &tms32025_device::pop       },{2*CLK, &tms32025_device::trap      },{3*CLK, &tms32025_device::idle      },
/*20*/ {1*CLK, &tms32025_device::rtxm     },{1*CLK, &tms32025_device::stxm      },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::neg       },{2*CLK, &tms32025_device::cala      },{2*CLK, &tms32025_device::bacc      },{2*CLK, &tms32025_device::ret       },{1*CLK, &tms32025_device::cmpl      },
/*28*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*30*/ {1*CLK, &tms32025_device::rc       },{1*CLK, &tms32025_device::sc        },{1*CLK, &tms32025_device::rtc       },{1*CLK, &tms32025_device::stc       },{1*CLK, &tms32025_device::rol       },{1*CLK, &tms32025_device::ror       },{1*CLK, &tms32025_device::rfsm      },{1*CLK, &tms32025_device::sfsm      },
/*38*/ {1*CLK, &tms32025_device::rhm      },{1*CLK, &tms32025_device::shm       },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::conf      },{1*CLK, &tms32025_device::conf      },{1*CLK, &tms32025_device::conf      },{1*CLK, &tms32025_device::conf      },
/*40*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*48*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*50*/ {1*CLK, &tms32025_device::cmpr     },{1*CLK, &tms32025_device::cmpr      },{1*CLK, &tms32025_device::cmpr      },{1*CLK, &tms32025_device::cmpr      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*58*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*60*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*68*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*70*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*78*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*80*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*88*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*90*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*98*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*A0*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*A8*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*B0*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*B8*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*C0*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*C8*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*D0*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*D8*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*E0*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*E8*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*F0*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{1*CLK, &tms32025_device::norm      },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },
/*F8*/ {0*CLK, &tms32025_device::illegal  },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   },{0*CLK, &tms32025_device::illegal   }
};

const tms32025_device::tms32025_opcode tms32025_device::s_opcode_Dx_subset[8]=    /* Instructions living under the Dxxx opcode */
{
/*00*/ {2*CLK, &tms32025_device::lrlk     },{2*CLK, &tms32025_device::lalk      },{2*CLK, &tms32025_device::adlk      },{2*CLK, &tms32025_device::sblk      },{2*CLK, &tms32025_device::andk      },{2*CLK, &tms32025_device::ork       },{2*CLK, &tms32025_device::xork      },{0*CLK, &tms32025_device::illegal   }
};



/****************************************************************************
 *  Initialise the CPU emulation
 ****************************************************************************/
void tms32025_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	m_PREVPC = 0;
	m_PFC = 0;
	m_STR0 = 0;
	m_STR1 = 0;
	m_ACC.d = 0;
	m_Preg.d = 0;
	m_Treg = 0;
	m_AR[0] = m_AR[1] = m_AR[2] = m_AR[3] = m_AR[4] = m_AR[5] = m_AR[6] = m_AR[7] = 0;
	m_STACK[0] = m_STACK[1] = m_STACK[2] = m_STACK[3] = m_STACK[4] = m_STACK[5] = m_STACK[6] = m_STACK[7] = 0;
	m_ALU.d = 0;
	memset(m_intRAM, 0, sizeof(m_intRAM));
	m_timerover = 0;
	m_opcode.d = 0;
	m_external_mem_access = 0;
	m_tms32025_irq_cycles = 0;
	m_oldacc.d = 0;
	m_memaccess = 0;
	m_mHackIgnoreARP = 0;
	m_waiting_for_serial_frame = 0;

	save_item(NAME(m_PC));
	save_item(NAME(m_STR0));
	save_item(NAME(m_STR1));
	save_item(NAME(m_PFC));
	save_item(NAME(m_IFR));
	save_item(NAME(m_RPTC));
	save_item(NAME(m_ACC.d));
	save_item(NAME(m_ALU.d));
	save_item(NAME(m_Preg.d));
	save_item(NAME(m_Treg));
	save_item(NAME(m_AR[0]));
	save_item(NAME(m_AR[1]));
	save_item(NAME(m_AR[2]));
	save_item(NAME(m_AR[3]));
	save_item(NAME(m_AR[4]));
	save_item(NAME(m_AR[5]));
	save_item(NAME(m_AR[6]));
	save_item(NAME(m_AR[7]));
	save_item(NAME(m_STACK[0]));
	save_item(NAME(m_STACK[1]));
	save_item(NAME(m_STACK[2]));
	save_item(NAME(m_STACK[3]));
	save_item(NAME(m_STACK[4]));
	save_item(NAME(m_STACK[5]));
	save_item(NAME(m_STACK[6]));
	save_item(NAME(m_STACK[7]));

	save_item(NAME(m_oldacc));
	save_item(NAME(m_memaccess));
	save_item(NAME(m_mHackIgnoreARP));

	save_item(NAME(m_idle));
	save_item(NAME(m_hold));
	save_item(NAME(m_external_mem_access));
	save_item(NAME(m_init_load_addr));
	save_item(NAME(m_PREVPC));

	save_item(NAME(m_intRAM));

	state_add( TMS32025_PC,   "PC",   m_PC).formatstr("%04X");
	state_add( TMS32025_STR0, "STR0", m_STR0).formatstr("%04X");
	state_add( TMS32025_STR1, "STR1", m_STR1).formatstr("%04X");
	state_add( TMS32025_IFR,  "IFR",  m_IFR).formatstr("%04X");
	state_add( TMS32025_RPTC, "RPTC", m_RPTC).formatstr("%02X");
	state_add( TMS32025_STK7, "STK7", m_STACK[7]).formatstr("%04X");
	state_add( TMS32025_STK6, "STK6", m_STACK[6]).formatstr("%04X");
	state_add( TMS32025_STK5, "STK5", m_STACK[5]).formatstr("%04X");
	state_add( TMS32025_STK4, "STK4", m_STACK[4]).formatstr("%04X");
	state_add( TMS32025_STK3, "STK3", m_STACK[3]).formatstr("%04X");
	state_add( TMS32025_STK2, "STK2", m_STACK[2]).formatstr("%04X");
	state_add( TMS32025_STK1, "STK1", m_STACK[1]).formatstr("%04X");
	state_add( TMS32025_STK0, "STK0", m_STACK[0]).formatstr("%04X");
	state_add( TMS32025_ACC,  "ACC",  m_ACC.d).formatstr("%08X");
	state_add( TMS32025_PREG, "P",    m_Preg.d).formatstr("%08X");
	state_add( TMS32025_TREG, "T",    m_Treg).formatstr("%04X");
	state_add( TMS32025_AR0,  "AR0",  m_AR[0]).formatstr("%04X");
	state_add( TMS32025_AR1,  "AR1",  m_AR[1]).formatstr("%04X");
	state_add( TMS32025_AR2,  "AR2",  m_AR[2]).formatstr("%04X");
	state_add( TMS32025_AR3,  "AR3",  m_AR[3]).formatstr("%04X");
	state_add( TMS32025_AR4,  "AR4",  m_AR[4]).formatstr("%04X");
	state_add( TMS32025_AR5,  "AR5",  m_AR[5]).formatstr("%04X");
	state_add( TMS32025_AR6,  "AR6",  m_AR[6]).formatstr("%04X");
	state_add( TMS32025_AR7,  "AR7",  m_AR[7]).formatstr("%04X");
	state_add( TMS32025_DRR,  "DRR",  m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( TMS32025_DXR,  "DXR",  m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( TMS32025_TIM,  "TIM",  m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( TMS32025_PRD,  "PRD",  m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( TMS32025_IMR,  "IMR",  m_debugger_temp).callimport().callexport().formatstr("%04X");
	state_add( TMS32025_GREG, "GREG", m_debugger_temp).callimport().callexport().formatstr("%04X");

	state_add(STATE_GENPC, "GENPC", m_PC).formatstr("%04X").noshow();
	/* This is actually not a stack pointer, but the stack contents */
	state_add(STATE_GENSP, "GENSP", m_STACK[7]).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_STR0).formatstr("%33s").noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_PREVPC).formatstr("%04X").noshow();

	m_icountptr = &m_icount;
}


void tms32025_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS32025_DRR:
			M_WRTRAM(0, m_debugger_temp);
			break;

		case TMS32025_DXR:
			M_WRTRAM(1, m_debugger_temp);
			break;

		case TMS32025_TIM:
			M_WRTRAM(2, m_debugger_temp);
			break;

		case TMS32025_PRD:
			M_WRTRAM(3, m_debugger_temp);
			break;

		case TMS32025_IMR:
			M_WRTRAM(4,m_debugger_temp);
			break;

		case TMS32025_GREG:
			M_WRTRAM(5, m_debugger_temp);
			break;
	}
}


void tms32025_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case TMS32025_DRR:
			m_debugger_temp = M_RDRAM(0);
			break;

		case TMS32025_DXR:
			m_debugger_temp = M_RDRAM(1);
			break;

		case TMS32025_TIM:
			m_debugger_temp = M_RDRAM(2);
			break;

		case TMS32025_PRD:
			m_debugger_temp = M_RDRAM(3);
			break;

		case TMS32025_IMR:
			m_debugger_temp = M_RDRAM(4);
			break;

		case TMS32025_GREG:
			m_debugger_temp = M_RDRAM(5);
			break;
	}
}


void tms32025_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "arp%d%c%c%c%cdp%03x  arb%d%c%c%c%c%c%c%c%c%c%c%cpm%d",
				(m_STR0 & 0xe000) >> 13,
				m_STR0 & 0x1000 ? 'O':'.',
				m_STR0 & 0x0800 ? 'M':'.',
				m_STR0 & 0x0400 ? '.':'?',
				m_STR0 & 0x0200 ? 'I':'.',
				(m_STR0 & 0x01ff),

				(m_STR1 & 0xe000) >> 13,
				m_STR1 & 0x1000 ? 'P':'D',
				m_STR1 & 0x0800 ? 'T':'.',
				m_STR1 & 0x0400 ? 'S':'.',
				m_STR1 & 0x0200 ? 'C':'?',
				m_STR0 & 0x0100 ? '.':'?',
				m_STR1 & 0x0080 ? '.':'?',
				m_STR1 & 0x0040 ? 'H':'.',
				m_STR1 & 0x0020 ? 'F':'.',
				m_STR1 & 0x0010 ? 'X':'.',
				m_STR1 & 0x0008 ? 'f':'.',
				m_STR1 & 0x0004 ? 'o':'i',
				(m_STR1 & 0x0003)
			);
			break;
	}
}


/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/
void tms32025_device::device_reset()
{
	SET_PC(0);                  /* Starting address on a reset */
	m_STR0 |= 0x0600;   /* INTM and unused bit set to 1 */
	m_STR0 &= 0xefff;   /* OV cleared to 0. Remaining bits undefined */
	m_STR1 |= 0x07f0;   /* SXM, C, HM, FSM, XF and unused bits set to 1 */
	m_STR1 &= 0xeff0;   /* CNF, FO, TXM, PM bits cleared to 0. Remaining bits undefined */
	m_RPTC = 0;         /* Reset repeat counter to 0 */
	m_IFR = 0;          /* IRQ pending flags */

	S_OUT(TMS32025_XF,ASSERT_LINE); /* XF flag is high. Must set the pin */

	/* Set the internal memory mapped registers */
	GREG = 0;
	TIM  = 0xffff;
	PRD  = 0xffff;
	IMR  = 0xffc0;

	m_idle = 0;
	m_hold = 0;
	m_tms32025_dec_cycles = 0;
	m_init_load_addr = 1;

	/* Reset the Data/Program address banks */
	memset(m_pgmmap, 0, sizeof(m_pgmmap));
	memset(m_datamap, 0, sizeof(m_datamap));

	m_datamap[0] = &m_intRAM[0x000];            /* B2 */
	m_datamap[4] = &m_intRAM[0x200];            /* B0 */
	m_datamap[5] = &m_intRAM[0x280];            /* B0 */
	m_datamap[6] = &m_intRAM[0x300];            /* B1 */
	m_datamap[7] = &m_intRAM[0x380];            /* B1 */
}

void tms32026_device::device_reset()
{
	tms32025_device::device_reset();

	m_datamap[8] = &m_intRAM[0x400];            /* B1 */
	m_datamap[9] = &m_intRAM[0x480];            /* B1 */
	m_datamap[10] = &m_intRAM[0x500];           /* B1 */
	m_datamap[11] = &m_intRAM[0x580];           /* B1 */
	m_datamap[12] = &m_intRAM[0x600];           /* B3 */
	m_datamap[13] = &m_intRAM[0x680];           /* B3 */
	m_datamap[14] = &m_intRAM[0x700];           /* B3 */
	m_datamap[15] = &m_intRAM[0x780];           /* B3 */
}


/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/
int tms32025_device::process_IRQs()
{
	/********** Interrupt Flag Register (IFR) **********
	    |  5  |  4  |  3  |  2  |  1  |  0  |
	    | XINT| RINT| TINT| INT2| INT1| INT0|
	*/

	m_tms32025_irq_cycles = 0;

	/* Dont service Interrupts if masked, or prev instruction was EINT ! */

	if ( (INTM == 0) && (m_opcode.w.l != 0xce00) && (m_IFR & IMR) )
	{
		m_tms32025_irq_cycles = (3*CLK);    /* 3 clock cycles used due to PUSH and DINT operation ? */
		PUSH_STACK(m_PC);

		if ((m_IFR & 0x01) && (IMR & 0x01)) {       /* IRQ line 0 */
			//logerror("TMS32025:  Active INT0\n");
			SET_PC(0x0002);
			standard_irq_callback(0);
			m_idle = 0;
			m_IFR &= (~0x01);
			SET0(INTM_FLAG);
			return m_tms32025_irq_cycles;
		}
		if ((m_IFR & 0x02) && (IMR & 0x02)) {       /* IRQ line 1 */
			//logerror("TMS32025:  Active INT1\n");
			SET_PC(0x0004);
			standard_irq_callback(1);
			m_idle = 0;
			m_IFR &= (~0x02);
			SET0(INTM_FLAG);
			return m_tms32025_irq_cycles;
		}
		if ((m_IFR & 0x04) && (IMR & 0x04)) {       /* IRQ line 2 */
			//logerror("TMS32025:  Active INT2\n");
			SET_PC(0x0006);
			standard_irq_callback(2);
			m_idle = 0;
			m_IFR &= (~0x04);
			SET0(INTM_FLAG);
			return m_tms32025_irq_cycles;
		}
		if ((m_IFR & 0x08) && (IMR & 0x08)) {       /* Timer IRQ (internal) */
//          logerror("TMS32025:  Active TINT (Timer)\n");
			SET_PC(0x0018);
			m_idle = 0;
			m_IFR &= (~0x08);
			SET0(INTM_FLAG);
			return m_tms32025_irq_cycles;
		}
		if ((m_IFR & 0x10) && (IMR & 0x10)) {       /* Serial port receive IRQ (internal) */
//          logerror("TMS32025:  Active RINT (Serial receive)\n");
			DRR = S_IN(TMS32025_DR);
			SET_PC(0x001A);
			m_idle = 0;
			m_IFR &= (~0x10);
			SET0(INTM_FLAG);
			return m_tms32025_irq_cycles;
		}
		if ((m_IFR & 0x20) && (IMR & 0x20)) {       /* Serial port transmit IRQ (internal) */
//          logerror("TMS32025:  Active XINT (Serial transmit)\n");
			S_OUT(TMS32025_DX,DXR);
			SET_PC(0x001C);
			m_idle = 0;
			m_IFR &= (~0x20);
			SET0(INTM_FLAG);
			return m_tms32025_irq_cycles;
		}
	}
	return m_tms32025_irq_cycles;
}


void tms32025_device::process_timer(int clocks)
{
	int preclocks, ticks;

	/* easy case: no actual ticks */
again:
	preclocks = CLK - m_timerover;
	if (clocks < preclocks)
	{
		m_timerover += clocks;
		m_icount -= clocks;
		return;
	}

	/* if we're not going to overflow the timer, just count the clocks */
	ticks = 1 + (clocks - preclocks) / CLK;
	if (ticks <= TIM)
	{
		m_icount -= clocks;
		m_timerover = clocks - (ticks - 1) * CLK - preclocks;
		TIM -= ticks;
	}

	/* otherwise, overflow the timer and signal an interrupt */
	else
	{
		m_icount -= preclocks + CLK * TIM;
		m_timerover = 0;
		TIM = PRD;

		m_IFR |= 0x08;
		clocks = process_IRQs();        /* Handle Timer IRQ */
		goto again;
	}
}


/****************************************************************************
 *  Execute ICount cycles. Exit when 0 or less
 ****************************************************************************/
void tms32025_device::execute_run()
{
	/**** Respond to external hold signal */
	if (S_IN(TMS32025_HOLD) == ASSERT_LINE) {
		if (m_hold == 0) {
			S_OUT(TMS32025_HOLDA,ASSERT_LINE);  /* Hold-Ack (active low) */
		}
		m_hold = 1;
		if (HM) {
			m_icount = 0;       /* Exit */
		}
		else {
			if (m_external_mem_access) {
				m_icount = 0;   /* Exit */
			}
		}
	}
	else {
		if (m_hold == 1) {
			S_OUT(TMS32025_HOLDA,CLEAR_LINE);   /* Hold-Ack (active low) */
			process_timer(3);
		}
		m_hold = 0;
	}

	/**** If idling, update timer and/or exit execution, but test for irqs first */
	if (m_idle && m_IFR && m_icount > 0)
		m_icount -= process_IRQs();

	while (m_idle && m_icount > 0)
		process_timer(m_icount);

	if (m_icount <= 0) debugger_instruction_hook(this, m_PC);


	while (m_icount > 0)
	{
		m_tms32025_dec_cycles = 0;

		if (m_IFR) {    /* Check IRQ Flag Register for pending IRQs */
			m_tms32025_dec_cycles += process_IRQs();
		}

		m_PREVPC = m_PC;

		debugger_instruction_hook(this, m_PC);

		m_opcode.d = M_RDOP(m_PC);
		m_PC++;

		if (m_opcode.b.h == 0xCE)   /* Opcode 0xCExx has many sub-opcodes in its minor byte */
		{
			m_tms32025_dec_cycles += s_opcode_CE_subset[m_opcode.b.l].cycles;
			(this->*s_opcode_CE_subset[m_opcode.b.l].function)();
		}
		else if ((m_opcode.w.l & 0xf0f8) == 0xd000) /* Opcode 0xDxxx has many sub-opcodes in its minor byte */
		{
			m_tms32025_dec_cycles += s_opcode_Dx_subset[m_opcode.b.l].cycles;
			(this->*s_opcode_Dx_subset[m_opcode.b.l].function)();
		}
		else            /* Do all opcodes except the CExx and Dxxx ones */
		{
			m_tms32025_dec_cycles += s_opcode_main[m_opcode.b.h].cycles;
			(this->*s_opcode_main[m_opcode.b.h].function)();
		}


		if (m_init_load_addr == 2) {        /* Repeat next instruction */
			/****************************************************\
			******* These instructions are not repeatable ********
			** ADLK, ANDK, LALK, LRLK, ORK,  SBLK, XORK         **
			** ADDK, ADRK, LACK, LARK, LDPK, MPYK, RPTK         **
			** SBRK, SPM,  SUBK, ZAC,  IDLE, RPT,  TRAP         **
			** BACC, CALA, RET                                  **
			** B,    BANZ, BBNZ, BBZ,  BC,   BGEZ, BGZ,  BIOZ   **
			** BNC,  BNV,  BNZ,  BV,   BZ,   CALL, BLEZ, BLZ    **
			\****************************************************/
			m_PREVPC = m_PC;

			debugger_instruction_hook(this, m_PC);

			m_opcode.d = M_RDOP(m_PC);
			m_PC++;
			m_tms32025_dec_cycles += (1*CLK);

			do {
				if (m_opcode.b.h == 0xCE)
				{                           /* Do all 0xCExx Opcodes */
					if (m_init_load_addr) {
						m_tms32025_dec_cycles += (1*CLK);
					}
					else {
						m_tms32025_dec_cycles += (1*CLK);
					}
					(this->*s_opcode_CE_subset[m_opcode.b.l].function)();
				}
				else
				{                           /* Do all other opcodes */
					if (m_init_load_addr) {
						m_tms32025_dec_cycles += (1*CLK);
					}
					else {
						m_tms32025_dec_cycles += (1*CLK);
					}
					(this->*s_opcode_main[m_opcode.b.h].function)();
				}
				m_init_load_addr = 0;
				m_RPTC-- ;
			} while ((INT8)(m_RPTC) != -1);
			m_RPTC = 0;
			m_PFC = m_PC;
			m_init_load_addr = 1;
		}

		process_timer(m_tms32025_dec_cycles);

		/**** If device is put into idle mode, exit and wait for an interrupt */
		while (m_idle && m_icount > 0)
			process_timer(m_icount);


		/**** If hold pin is active, exit if accessing external memory or if HM is set */
		if (m_hold) {
			if (m_external_mem_access || (HM)) {
				if (m_icount > 0) {
					m_icount = 0;
				}
			}
		}
	}
}



/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/
void tms32025_device::execute_set_input(int irqline, int state)
{
	if ( irqline == TMS32025_FSX ) {
		if (state != CLEAR_LINE && m_waiting_for_serial_frame)
		{
			m_waiting_for_serial_frame = 0;
			m_IFR = 0x20;
		}
	}
	else
	{
		/* Pending IRQs cannot be cleared */
		if (state != CLEAR_LINE)
		{
			m_IFR |= (1 << irqline);
		}
	}
}


/****************************************************************************
 *  Opcode fetcher
 ****************************************************************************/
bool tms32025_device::memory_readop(offs_t offset, int size, UINT64 &value)
{
	void *ptr;

	/* skip if not custom */
	if (!m_pgmmap[offset >> 8])
		return 0;

	ptr = &((UINT8 *)&m_pgmmap[offset >> 8])[offset & 0xff];
	switch (size)
	{
		case 1: value = *((UINT8 *) ptr);
		case 2: value = *((UINT16 *) ptr);
		case 4: value = *((UINT32 *) ptr);
		case 8: value = *((UINT64 *) ptr);
	}
	return 1;
}


/****************************************************************************
 *  Memory reader
 ****************************************************************************/
bool tms32025_device::memory_read(address_spacenum spacenum, offs_t offset, int size, UINT64 &value)
{
	void *ptr = nullptr;
	UINT64 temp = 0;

	switch (spacenum)
	{
		case AS_PROGRAM:
			ptr = m_pgmmap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case AS_DATA:
			ptr = m_datamap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		default:
		case AS_IO:
			return 0;
	}

	switch (size)
	{
		case 1:
			value = ((UINT8 *)ptr)[BYTE_XOR_BE(offset & 0xff)];
			break;
		case 2:
			value = ((UINT16 *)ptr)[(offset & 0xff) / 2];
			break;
		case 4:
			memory_read(spacenum, offset + 0, 2, temp);
			value = temp << 16;
			memory_read(spacenum, offset + 2, 2, temp);
			value |= temp & 0xffff;
			break;
		case 8:
			memory_read(spacenum, offset + 0, 4, temp);
			value = temp << 32;
			memory_read(spacenum, offset + 4, 4, temp);
			value |= temp & 0xffffffff;
			break;
	}
	return 1;
}


/****************************************************************************
 *  Memory writer
 ****************************************************************************/
bool tms32025_device::memory_write(address_spacenum spacenum, offs_t offset, int size, UINT64 value)
{
	void *ptr = nullptr;

	switch (spacenum)
	{
		case AS_PROGRAM:
			ptr = m_pgmmap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case AS_DATA:
			ptr = m_datamap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		default:
		case AS_IO:
			return 0;
	}

	switch (size)
	{
		case 1:
			((UINT8 *)ptr)[BYTE_XOR_BE(offset & 0xff)] = value;
			break;
		case 2:
			((UINT16 *)ptr)[(offset & 0xff) / 2] = value;
			break;
		case 4:
			memory_write(spacenum, offset + 0, 2, value >> 16);
			memory_write(spacenum, offset + 2, 2, value);
			break;
		case 8:
			memory_write(spacenum, offset + 0, 4, value >> 32);
			memory_write(spacenum, offset + 4, 4, value);
			break;
	}

	return 1;
}
