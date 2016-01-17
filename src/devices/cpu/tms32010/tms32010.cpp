// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                 Texas Instruments TMS32010 DSP Emulator                  *
	*                                                                          *
	*                  Copyright Tony La Porta                                 *
	*      You are not allowed to distribute this software commercially.       *
	*                      Written for the MAME project.                       *
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
	*  TLP (13-Jul-2002)                                                       *
	*   - Added Save-State support                                             *
	*   - Converted the pending_irq flag to INTF (a real flag in this device)  *
	*   - Fixed the ignore Interrupt Request for previous critical             *
	*     instructions requiring an extra instruction to be processed. For     *
	*     this reason, instant IRQ servicing cannot be supported here, so      *
	*     INTF needs to be polled within the instruction execution loop        *
	*   - Removed IRQ callback (IRQ ACK not supported on this device)          *
	*   - A pending IRQ will remain pending until it's serviced. De-asserting  *
	*     the IRQ Pin does not remove a pending IRQ state                      *
	*   - BIO is no longer treated as an IRQ line. It's polled when required.  *
	*     This is the true behaviour of the device                             *
	*   - Removed the Clear OV flag from overflow instructions. Overflow       *
	*     instructions can only set the flag. Flag test instructions clear it  *
	*   - Fixed the ABST, SUBC and SUBH instructions                           *
	*   - Fixed the signedness in many equation based instructions             *
	*   - Added the missing Previous PC to the get_register function           *
	*   - Changed Cycle timings to include clock ticks                         *
	*   - Converted some registers from ints to pairs for much cleaner code    *
	*  TLP (20-Jul-2002) Ver 1.10                                              *
	*   - Fixed the dissasembly from the debugger                              *
	*   - Changed all references from TMS320C10 to TMS32010                    *
	*  ASG (24-Sep-2002) Ver 1.20                                              *
	*   - Fixed overflow handling                                              *
	*   - Simplified logic in a few locations                                  *
	*  TLP (22-Feb-2004) Ver 1.21                                              *
	*   - Overflow for ADDH only affects upper 16bits (was modifying 32 bits)  *
	*   - Internal Data Memory map is assigned here now                        *
	*   - Cycle counts for invalid opcodes 7F1E and 7F1F are now 0             *
	*  RK  (23-Nov-2006) Ver 1.22                                              *
	*   - Fixed state of the Overflow Flag on reset                            *
	*   - Fixed the SUBC instruction which was incorrectly zeroing the divisor *
	*  TLP (13-Jul-2010) Ver 1.30                                              *
	*   - LST instruction was incorrectly setting an Indirect Addressing       *
	*     feature when Direct Addressing mode was selected                     *
	*   - Added TMS32015 and TMS32016 variants                                 *
	*  TLP (27-Jul-2010) Ver 1.31                                              *
	*   - Corrected cycle timing for conditional branch instructions           *
	*                                                                          *
	\**************************************************************************/


#include "emu.h"
#include "debugger.h"
#include "tms32010.h"



#define M_RDROM(A)      TMS32010_ROM_RDMEM(A)
#define M_WRTROM(A,V)   TMS32010_ROM_WRMEM(A,V)
#define M_RDRAM(A)      TMS32010_RAM_RDMEM(A)
#define M_WRTRAM(A,V)   TMS32010_RAM_WRMEM(A,V)
#define M_RDOP(A)       TMS32010_RDOP(A)
#define M_RDOP_ARG(A)   TMS32010_RDOP_ARG(A)
#define P_IN(A)         TMS32010_In(A)
#define P_OUT(A,V)      TMS32010_Out(A,V)
#define BIO_IN          TMS32010_BIO_In


const device_type TMS32010 = &device_creator<tms32010_device>;
const device_type TMS32015 = &device_creator<tms32015_device>;
const device_type TMS32016 = &device_creator<tms32016_device>;


/****************************************************************************
 *  TMS32010 Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( tms32010_ram, AS_DATA, 16, tms32010_device )
	AM_RANGE(0x00, 0x7f) AM_RAM     /* Page 0 */
	AM_RANGE(0x80, 0x8f) AM_RAM     /* Page 1 */
ADDRESS_MAP_END

/****************************************************************************
 *  TMS32015/6 Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( tms32015_ram, AS_DATA, 16, tms32010_device )
	AM_RANGE(0x00, 0x7f) AM_RAM     /* Page 0 */
	AM_RANGE(0x80, 0xff) AM_RAM     /* Page 1 */
ADDRESS_MAP_END


tms32010_device::tms32010_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS32010, "TMS32010", tag, owner, clock, "tms32010", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 16, 12, -1)
	, m_data_config("data", ENDIANNESS_BIG, 16, 8, -1, ADDRESS_MAP_NAME(tms32010_ram))
	, m_io_config("io", ENDIANNESS_BIG, 16, 5, -1)
	, m_addr_mask(0x0fff)
{
}


tms32010_device::tms32010_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source, int addr_mask)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_BIG, 16, 12, -1)
	, m_data_config("data", ENDIANNESS_BIG, 16, 8, -1, ADDRESS_MAP_NAME(tms32015_ram))
	, m_io_config("io", ENDIANNESS_BIG, 16, 5, -1)
	, m_addr_mask(addr_mask)
{
}


tms32015_device::tms32015_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: tms32010_device(mconfig, TMS32015, "TMS32015", tag, owner, clock, "tms32015", __FILE__, 0x0fff)
{
}


tms32016_device::tms32016_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: tms32010_device(mconfig, TMS32016, "TMS32016", tag, owner, clock, "tms32016", __FILE__, 0xffff)
{
}


offs_t tms32010_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms32010 );
	return CPU_DISASSEMBLE_NAME(tms32010)(this, buffer, pc, oprom, opram, options);
}


/*********  The following is the Status (Flag) register definition.  *********/
/* 15 | 14  |  13  | 12 | 11 | 10 | 9 |  8  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0  */
/* OV | OVM | INTM |  1 |  1 |  1 | 1 | ARP | 1 | 1 | 1 | 1 | 1 | 1 | 1 | DP */
#define OV_FLAG     0x8000  /* OV   (Overflow flag) 1 indicates an overflow */
#define OVM_FLAG    0x4000  /* OVM  (Overflow Mode bit) 1 forces ACC overflow to greatest positive or negative saturation value */
#define INTM_FLAG   0x2000  /* INTM (Interrupt Mask flag) 0 enables maskable interrupts */
#define ARP_REG     0x0100  /* ARP  (Auxiliary Register Pointer) */
#define DP_REG      0x0001  /* DP   (Data memory Pointer (bank) bit) */

#define OV      ( m_STR & OV_FLAG)          /* OV   (Overflow flag) */
#define OVM     ( m_STR & OVM_FLAG)         /* OVM  (Overflow Mode bit) 1 indicates an overflow */
#define INTM    ( m_STR & INTM_FLAG)        /* INTM (Interrupt enable flag) 0 enables maskable interrupts */
#define ARP     ((m_STR & ARP_REG) >> 8)    /* ARP  (Auxiliary Register Pointer) */
#define DP      ((m_STR & DP_REG) << 7)     /* DP   (Data memory Pointer bit) */

#define DMA_DP  (DP | (m_opcode.b.l & 0x7f))    /* address used in direct memory access operations */
#define DMA_DP1 (0x80 | m_opcode.b.l)           /* address used in direct memory access operations for sst instruction */
#define IND     (m_AR[ARP] & 0xff)              /* address used in indirect memory access operations */



/****************************************************************************
 *  Read the state of the BIO pin
 */

#define TMS32010_BIO_In (m_io->read_word(TMS32010_BIO<<1))


/****************************************************************************
 *  Input a word from given I/O port
 */

#define TMS32010_In(Port) (m_io->read_word((Port)<<1))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define TMS32010_Out(Port,Value) (m_io->write_word((Port)<<1,Value))



/****************************************************************************
 *  Read a word from given ROM memory location
 */

#define TMS32010_ROM_RDMEM(A) (m_program->read_word((A)<<1))


/****************************************************************************
 *  Write a word to given ROM memory location
 */

#define TMS32010_ROM_WRMEM(A,V) (m_program->write_word((A)<<1,V))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define TMS32010_RAM_RDMEM(A) (m_data->read_word((A)<<1))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define TMS32010_RAM_WRMEM(A,V) (m_data->write_word((A)<<1,V))



/****************************************************************************
 *  TMS32010_RDOP() is identical to TMS32010_RDMEM() except it is used for reading
 *  opcodes. In case of system with memory mapped I/O, this function can be
 *  used to greatly speed up emulation
 */

#define TMS32010_RDOP(A) (m_direct->read_word((A)<<1))


/****************************************************************************
 *  TMS32010_RDOP_ARG() is identical to TMS32010_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define TMS32010_RDOP_ARG(A) (m_direct->read_word((A)<<1))


/************************************************************************
 *  Shortcuts
 ************************************************************************/

void tms32010_device::CLR(UINT16 flag) { m_STR &= ~flag; m_STR |= 0x1efe; }
void tms32010_device::SET_FLAG(UINT16 flag) { m_STR |=  flag; m_STR |= 0x1efe; }


void tms32010_device::CALCULATE_ADD_OVERFLOW(INT32 addval)
{
	if ((INT32)(~(m_oldacc.d ^ addval) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.d = ((INT32)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}
void tms32010_device::CALCULATE_SUB_OVERFLOW(INT32 subval)
{
	if ((INT32)((m_oldacc.d ^ subval) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.d = ((INT32)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}

UINT16 tms32010_device::POP_STACK()
{
	UINT16 data = m_STACK[3];
	m_STACK[3] = m_STACK[2];
	m_STACK[2] = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return (data & m_addr_mask);
}
void tms32010_device::PUSH_STACK(UINT16 data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = m_STACK[2];
	m_STACK[2] = m_STACK[3];
	m_STACK[3] = (data & m_addr_mask);
}

void tms32010_device::UPDATE_AR()
{
	if (m_opcode.b.l & 0x30) {
		UINT16 tmpAR = m_AR[ARP];
		if (m_opcode.b.l & 0x20) tmpAR++ ;
		if (m_opcode.b.l & 0x10) tmpAR-- ;
		m_AR[ARP] = (m_AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
	}
}
void tms32010_device::UPDATE_ARP()
{
	if (~m_opcode.b.l & 0x08) {
		if (m_opcode.b.l & 0x01) SET_FLAG(ARP_REG);
		else CLR(ARP_REG);
	}
}


void tms32010_device::getdata(UINT8 shift,UINT8 signext)
{
	if (m_opcode.b.l & 0x80)
		m_memaccess = IND;
	else
		m_memaccess = DMA_DP;

	m_ALU.d = (UINT16)M_RDRAM(m_memaccess);
	if (signext) m_ALU.d = (INT16)m_ALU.d;
	m_ALU.d <<= shift;
	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}

void tms32010_device::putdata(UINT16 data)
{
	if (m_opcode.b.l & 0x80)
		m_memaccess = IND;
	else
		m_memaccess = DMA_DP;

	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
	M_WRTRAM(m_memaccess,data);
}
void tms32010_device::putdata_sar(UINT8 data)
{
	if (m_opcode.b.l & 0x80)
		m_memaccess = IND;
	else
		m_memaccess = DMA_DP;

	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
	M_WRTRAM(m_memaccess,m_AR[data]);
}
void tms32010_device::putdata_sst(UINT16 data)
{
	if (m_opcode.b.l & 0x80)
		m_memaccess = IND;
	else
		m_memaccess = DMA_DP1;  /* Page 1 only */

	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
	}
	M_WRTRAM(m_memaccess,data);
}



/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */

void tms32010_device::opcodes_7F()  { fatalerror("Should never get here!\n"); }


void tms32010_device::illegal()
{
	logerror("TMS32010:  PC=%04x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.w.l);
}

void tms32010_device::abst()
{
	if ( (INT32)(m_ACC.d) < 0 ) {
		m_ACC.d = -m_ACC.d;
		if (OVM && (m_ACC.d == 0x80000000)) m_ACC.d-- ;
	}
}

/*** The manual doesn't mention overflow with the ADD? instructions however ***
 *** overflow is implemented here, because it makes little sense otherwise ****
 *** while newer generations of this type of chip supported it. The ***********
 *** manual may be wrong wrong (apart from other errors the manual has). ******

void tms32010_device::add_sh()    { getdata(m_opcode.b.h,1); m_ACC.d += m_ALU.d; }
void tms32010_device::addh()      { getdata(0,0); m_ACC.d += (m_ALU.d << 16); }
 ***/

void tms32010_device::add_sh()
{
	m_oldacc.d = m_ACC.d;
	getdata((m_opcode.b.h & 0xf),1);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
}
void tms32010_device::addh()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.w.h += m_ALU.w.l;
	if ((INT16)(~(m_oldacc.w.h ^ m_ALU.w.h) & (m_oldacc.w.h ^ m_ACC.w.h)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.w.h = ((INT16)m_oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
void tms32010_device::adds()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
}
void tms32010_device::and_()
{
	getdata(0,0);
	m_ACC.d &= m_ALU.d;
}
void tms32010_device::apac()
{
	m_oldacc.d = m_ACC.d;
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
void tms32010_device::br()
{
	m_PC = M_RDOP_ARG(m_PC);
}
void tms32010_device::banz()
{
	if (m_AR[ARP] & 0x01ff) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
	m_ALU.w.l = m_AR[ARP];
	m_ALU.w.l-- ;
	m_AR[ARP] = (m_AR[ARP] & 0xfe00) | (m_ALU.w.l & 0x01ff);
}
void tms32010_device::bgez()
{
	if ( (INT32)(m_ACC.d) >= 0 ) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::bgz()
{
	if ( (INT32)(m_ACC.d) > 0 ) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::bioz()
{
	if (BIO_IN != CLEAR_LINE) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::blez()
{
	if ( (INT32)(m_ACC.d) <= 0 ) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::blz()
{
	if ( (INT32)(m_ACC.d) <  0 ) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::bnz()
{
	if (m_ACC.d != 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::bv()
{
	if (OV) {
		CLR(OV_FLAG);
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::bz()
{
	if (m_ACC.d == 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
void tms32010_device::cala()
{
	PUSH_STACK(m_PC);
	m_PC = m_ACC.w.l & m_addr_mask;
}
void tms32010_device::call()
{
	m_PC++ ;
	PUSH_STACK(m_PC);
	m_PC = M_RDOP_ARG((m_PC - 1));
}
void tms32010_device::dint()
{
	SET_FLAG(INTM_FLAG);
}
void tms32010_device::dmov()
{
	getdata(0,0);
	M_WRTRAM((m_memaccess + 1),m_ALU.w.l);
}
void tms32010_device::eint()
{
	CLR(INTM_FLAG);
}
void tms32010_device::in_p()
{
	m_ALU.w.l = P_IN( (m_opcode.b.h & 7) );
	putdata(m_ALU.w.l);
}
void tms32010_device::lac_sh()
{
	getdata((m_opcode.b.h & 0x0f),1);
	m_ACC.d = m_ALU.d;
}
void tms32010_device::lack()
{
	m_ACC.d = m_opcode.b.l;
}
void tms32010_device::lar_ar0()
{
	getdata(0,0);
	m_AR[0] = m_ALU.w.l;
}
void tms32010_device::lar_ar1()
{
	getdata(0,0);
	m_AR[1] = m_ALU.w.l;
}
void tms32010_device::lark_ar0()
{
	m_AR[0] = m_opcode.b.l;
}
void tms32010_device::lark_ar1()
{
	m_AR[1] = m_opcode.b.l;
}
void tms32010_device::larp_mar()
{
	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}
void tms32010_device::ldp()
{
	getdata(0,0);
	if (m_ALU.d & 1)
		SET_FLAG(DP_REG);
	else
		CLR(DP_REG);
}
void tms32010_device::ldpk()
{
	if (m_opcode.b.l & 1)
		SET_FLAG(DP_REG);
	else
		CLR(DP_REG);
}
void tms32010_device::lst()
{
	if (m_opcode.b.l & 0x80) {
		m_opcode.b.l |= 0x08; /* In Indirect Addressing mode, next ARP is not supported here so mask it */
	}
	getdata(0,0);
	m_ALU.w.l &= (~INTM_FLAG);  /* Must not affect INTM */
	m_STR &= INTM_FLAG;
	m_STR |= m_ALU.w.l;
	m_STR |= 0x1efe;
}
void tms32010_device::lt()
{
	getdata(0,0);
	m_Treg = m_ALU.w.l;
}
void tms32010_device::lta()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_Treg = m_ALU.w.l;
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
void tms32010_device::ltd()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_Treg = m_ALU.w.l;
	M_WRTRAM((m_memaccess + 1),m_ALU.w.l);
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
void tms32010_device::mpy()
{
	getdata(0,0);
	m_Preg.d = (INT16)m_ALU.w.l * (INT16)m_Treg;
	if (m_Preg.d == 0x40000000) m_Preg.d = 0xc0000000;
}
void tms32010_device::mpyk()
{
	m_Preg.d = (INT16)m_Treg * ((INT16)(m_opcode.w.l << 3) >> 3);
}
void tms32010_device::nop()
{
	/* Nothing to do */
}
void tms32010_device::or_()
{
	getdata(0,0);
	m_ACC.w.l |= m_ALU.w.l;
}
void tms32010_device::out_p()
{
	getdata(0,0);
	P_OUT( (m_opcode.b.h & 7), m_ALU.w.l );
}
void tms32010_device::pac()
{
	m_ACC.d = m_Preg.d;
}
void tms32010_device::pop()
{
	m_ACC.w.l = POP_STACK();
	m_ACC.w.h = 0x0000;
}
void tms32010_device::push()
{
	PUSH_STACK(m_ACC.w.l);
}
void tms32010_device::ret()
{
	m_PC = POP_STACK();
}
void tms32010_device::rovm()
{
	CLR(OVM_FLAG);
}
void tms32010_device::sach_sh()
{
	m_ALU.d = (m_ACC.d << (m_opcode.b.h & 7));
	putdata(m_ALU.w.h);
}
void tms32010_device::sacl()
{
	putdata(m_ACC.w.l);
}
void tms32010_device::sar_ar0()
{
	putdata_sar(0);
}
void tms32010_device::sar_ar1()
{
	putdata_sar(1);
}
void tms32010_device::sovm()
{
	SET_FLAG(OVM_FLAG);
}
void tms32010_device::spac()
{
	m_oldacc.d = m_ACC.d;
	m_ACC.d -= m_Preg.d;
	CALCULATE_SUB_OVERFLOW(m_Preg.d);
}
void tms32010_device::sst()
{
	putdata_sst(m_STR);
}
void tms32010_device::sub_sh()
{
	m_oldacc.d = m_ACC.d;
	getdata((m_opcode.b.h & 0x0f),1);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
void tms32010_device::subc()
{
	m_oldacc.d = m_ACC.d;
	getdata(15,0);
	m_ALU.d = (INT32) m_ACC.d - m_ALU.d;
	if ((INT32)((m_oldacc.d ^ m_ALU.d) & (m_oldacc.d ^ m_ACC.d)) < 0)
		SET_FLAG(OV_FLAG);
	if ( (INT32)(m_ALU.d) >= 0 )
		m_ACC.d = ((m_ALU.d << 1) + 1);
	else
		m_ACC.d = (m_ACC.d << 1);
}
void tms32010_device::subh()
{
	m_oldacc.d = m_ACC.d;
	getdata(16,0);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
void tms32010_device::subs()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
void tms32010_device::tblr()
{
	m_ALU.d = M_RDROM((m_ACC.w.l & m_addr_mask));
	putdata(m_ALU.w.l);
	m_STACK[0] = m_STACK[1];
}
void tms32010_device::tblw()
{
	getdata(0,0);
	M_WRTROM(((m_ACC.w.l & m_addr_mask)),m_ALU.w.l);
	m_STACK[0] = m_STACK[1];
}
void tms32010_device::xor_()
{
	getdata(0,0);
	m_ACC.w.l ^= m_ALU.w.l;
}
void tms32010_device::zac()
{
	m_ACC.d = 0;
}
void tms32010_device::zalh()
{
	getdata(0,0);
	m_ACC.w.h = m_ALU.w.l;
	m_ACC.w.l = 0x0000;
}
void tms32010_device::zals()
{
	getdata(0,0);
	m_ACC.w.l = m_ALU.w.l;
	m_ACC.w.h = 0x0000;
}



/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

/* Conditional Branch instructions take two cycles when the test condition is met and the branch performed */

const tms32010_device::tms32010_opcode tms32010_device::s_opcode_main[256]=
{
/*00*/  {1, &tms32010_device::add_sh  },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },
/*08*/  {1, &tms32010_device::add_sh  },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },{1, &tms32010_device::add_sh    },
/*10*/  {1, &tms32010_device::sub_sh  },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },
/*18*/  {1, &tms32010_device::sub_sh  },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },{1, &tms32010_device::sub_sh    },
/*20*/  {1, &tms32010_device::lac_sh  },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },
/*28*/  {1, &tms32010_device::lac_sh  },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },{1, &tms32010_device::lac_sh    },
/*30*/  {1, &tms32010_device::sar_ar0 },{1, &tms32010_device::sar_ar1   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*38*/  {1, &tms32010_device::lar_ar0 },{1, &tms32010_device::lar_ar1   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*40*/  {2, &tms32010_device::in_p    },{2, &tms32010_device::in_p      },{2, &tms32010_device::in_p      },{2, &tms32010_device::in_p      },{2, &tms32010_device::in_p      },{2, &tms32010_device::in_p      },{2, &tms32010_device::in_p      },{2, &tms32010_device::in_p      },
/*48*/  {2, &tms32010_device::out_p   },{2, &tms32010_device::out_p     },{2, &tms32010_device::out_p     },{2, &tms32010_device::out_p     },{2, &tms32010_device::out_p     },{2, &tms32010_device::out_p     },{2, &tms32010_device::out_p     },{2, &tms32010_device::out_p     },
/*50*/  {1, &tms32010_device::sacl    },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*58*/  {1, &tms32010_device::sach_sh },{1, &tms32010_device::sach_sh   },{1, &tms32010_device::sach_sh   },{1, &tms32010_device::sach_sh   },{1, &tms32010_device::sach_sh   },{1, &tms32010_device::sach_sh   },{1, &tms32010_device::sach_sh   },{1, &tms32010_device::sach_sh   },
/*60*/  {1, &tms32010_device::addh    },{1, &tms32010_device::adds      },{1, &tms32010_device::subh      },{1, &tms32010_device::subs      },{1, &tms32010_device::subc      },{1, &tms32010_device::zalh      },{1, &tms32010_device::zals      },{3, &tms32010_device::tblr      },
/*68*/  {1, &tms32010_device::larp_mar},{1, &tms32010_device::dmov      },{1, &tms32010_device::lt        },{1, &tms32010_device::ltd       },{1, &tms32010_device::lta       },{1, &tms32010_device::mpy       },{1, &tms32010_device::ldpk      },{1, &tms32010_device::ldp       },
/*70*/  {1, &tms32010_device::lark_ar0},{1, &tms32010_device::lark_ar1  },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*78*/  {1, &tms32010_device::xor_    },{1, &tms32010_device::and_      },{1, &tms32010_device::or_       },{1, &tms32010_device::lst       },{1, &tms32010_device::sst       },{3, &tms32010_device::tblw      },{1, &tms32010_device::lack      },{0, &tms32010_device::opcodes_7F    },
/*80*/  {1, &tms32010_device::mpyk    },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },
/*88*/  {1, &tms32010_device::mpyk    },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },
/*90*/  {1, &tms32010_device::mpyk    },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },
/*98*/  {1, &tms32010_device::mpyk    },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },{1, &tms32010_device::mpyk      },
/*A0*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*A8*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*B0*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*B8*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*C0*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*C8*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*D0*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*D8*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*E0*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*E8*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*F0*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{1, &tms32010_device::banz      },{1, &tms32010_device::bv        },{1, &tms32010_device::bioz      },{0, &tms32010_device::illegal   },
/*F8*/  {2, &tms32010_device::call    },{2, &tms32010_device::br        },{1, &tms32010_device::blz       },{1, &tms32010_device::blez      },{1, &tms32010_device::bgz       },{1, &tms32010_device::bgez      },{1, &tms32010_device::bnz       },{1, &tms32010_device::bz        }
};

const tms32010_device::tms32010_opcode tms32010_device::s_opcode_7F[32]=
{
/*80*/  {1, &tms32010_device::nop     },{1, &tms32010_device::dint      },{1, &tms32010_device::eint      },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*88*/  {1, &tms32010_device::abst    },{1, &tms32010_device::zac       },{1, &tms32010_device::rovm      },{1, &tms32010_device::sovm      },{2, &tms32010_device::cala      },{2, &tms32010_device::ret       },{1, &tms32010_device::pac       },{1, &tms32010_device::apac      },
/*90*/  {1, &tms32010_device::spac    },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },
/*98*/  {0, &tms32010_device::illegal },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   },{2, &tms32010_device::push      },{2, &tms32010_device::pop       },{0, &tms32010_device::illegal   },{0, &tms32010_device::illegal   }
};

int tms32010_device::add_branch_cycle()
{
	return s_opcode_main[m_opcode.b.h].cycles;
}

/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

void tms32010_device::device_start()
{
	save_item(NAME(m_PC));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_STR));
	save_item(NAME(m_ACC.d));
	save_item(NAME(m_ALU.d));
	save_item(NAME(m_Preg.d));
	save_item(NAME(m_Treg));
	save_item(NAME(m_AR[0]));
	save_item(NAME(m_AR[1]));
	save_item(NAME(m_STACK[0]));
	save_item(NAME(m_STACK[1]));
	save_item(NAME(m_STACK[2]));
	save_item(NAME(m_STACK[3]));
	save_item(NAME(m_INTF));
	save_item(NAME(m_opcode.d));
	save_item(NAME(m_oldacc.d));
	save_item(NAME(m_memaccess));
	save_item(NAME(m_addr_mask));

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);
	m_io = &space(AS_IO);

	m_PREVPC = 0;
	m_ALU.d = 0;
	m_Preg.d = 0;
	m_Treg = 0;
	m_AR[0] = m_AR[1] = 0;
	m_STACK[0] = m_STACK[1] = m_STACK[2] = m_STACK[3] = 0;
	m_opcode.d = 0;
	m_oldacc.d = 0;
	m_memaccess = 0;
	m_PC = 0;
	m_STR = 0;
	m_ACC.d = 0;

	state_add( TMS32010_PC,   "PC",   m_PC).formatstr("%04X");
	state_add( TMS32010_STR,  "STR",  m_STR).formatstr("%04X");
	state_add( TMS32010_ACC,  "ACC",  m_ACC.d).formatstr("%08X");
	state_add( TMS32010_PREG, "P",    m_Preg.d).formatstr("%08X");
	state_add( TMS32010_TREG, "T",    m_Treg).formatstr("%04X");
	state_add( TMS32010_AR0,  "AR0",  m_AR[0]).formatstr("%04X");
	state_add( TMS32010_AR1,  "AR1",  m_AR[1]).formatstr("%04X");
	state_add( TMS32010_STK0, "STK0", m_STACK[0]).formatstr("%04X");
	state_add( TMS32010_STK1, "STK1", m_STACK[1]).formatstr("%04X");
	state_add( TMS32010_STK2, "STK2", m_STACK[2]).formatstr("%04X");
	state_add( TMS32010_STK3, "STK3", m_STACK[3]).formatstr("%04X");

	state_add(STATE_GENPC, "GENPC", m_PC).formatstr("%04X").noshow();
	/* This is actually not a stack pointer, but the stack contents */
	state_add(STATE_GENSP, "GENSP", m_STACK[3]).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_STR).formatstr("%16s").noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_PREVPC).formatstr("%04X").noshow();

	m_icountptr = &m_icount;
}


/****************************************************************************
 *  TMS32010 Reset registers to their initial values
 ****************************************************************************/

void tms32010_device::device_reset()
{
	m_PC    = 0;
	m_ACC.d = 0;
	m_INTF  = TMS32010_INT_NONE;
	/* Setup Status Register : 7efe */
	CLR((OV_FLAG | ARP_REG | DP_REG));
	SET_FLAG((OVM_FLAG | INTM_FLAG));
}


void tms32010_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				m_STR & 0x8000 ? 'O':'.',
				m_STR & 0x4000 ? 'M':'.',
				m_STR & 0x2000 ? 'I':'.',
				m_STR & 0x1000 ? '.':'?',
				m_STR & 0x0800 ? 'a':'?',
				m_STR & 0x0400 ? 'r':'?',
				m_STR & 0x0200 ? 'p':'?',
				m_STR & 0x0100 ? '1':'0',
				m_STR & 0x0080 ? '.':'?',
				m_STR & 0x0040 ? '.':'?',
				m_STR & 0x0020 ? '.':'?',
				m_STR & 0x0010 ? '.':'?',
				m_STR & 0x0008 ? '.':'?',
				m_STR & 0x0004 ? 'd':'?',
				m_STR & 0x0002 ? 'p':'?',
				m_STR & 0x0001 ? '1':'0'
			);
			break;
	}
}


/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/

void tms32010_device::execute_set_input(int irqline, int state)
{
	/* Pending Interrupts cannot be cleared! */
	if (state == ASSERT_LINE) m_INTF |= TMS32010_INT_PENDING;
}



/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/

int tms32010_device::Ext_IRQ()
{
	if (INTM == 0)
	{
		logerror("TMS32010:  EXT INTERRUPT\n");
		m_INTF = TMS32010_INT_NONE;
		SET_FLAG(INTM_FLAG);
		PUSH_STACK(m_PC);
		m_PC = 0x0002;
		return (s_opcode_7F[0x1c].cycles + s_opcode_7F[0x01].cycles);   /* 3 cycles used due to PUSH and DINT operation ? */
	}
	return (0);
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

void tms32010_device::execute_run()
{
	do
	{
		if (m_INTF) {
			/* Dont service INT if previous instruction was MPY, MPYK or EINT */
			if ((m_opcode.b.h != 0x6d) && ((m_opcode.b.h & 0xe0) != 0x80) && (m_opcode.w.l != 0x7f82))
				m_icount -= Ext_IRQ();
		}

		m_PREVPC = m_PC;

		debugger_instruction_hook(this, m_PC);

		m_opcode.d = M_RDOP(m_PC);
		m_PC++;

		if (m_opcode.b.h != 0x7f)   { /* Do all opcodes except the 7Fxx ones */
			m_icount -= s_opcode_main[m_opcode.b.h].cycles;
			(this->*s_opcode_main[m_opcode.b.h].function)();
		}
		else { /* Opcode major byte 7Fxx has many opcodes in its minor byte */
			m_icount -= s_opcode_7F[(m_opcode.b.l & 0x1f)].cycles;
			(this->*s_opcode_7F[(m_opcode.b.l & 0x1f)].function)();
		}
	} while (m_icount > 0);
}
