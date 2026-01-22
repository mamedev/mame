// license:BSD-3-Clause
// copyright-holders:Tony La Porta
/**************************************************************************

    Texas Instruments TMS320C1x DSP Emulator

    Copyright Tony La Porta

    Notes:
    * The term 'DMA' within this document, is in reference to Direct
      Memory Addressing, and NOT the usual term of Direct Memory Access.
    * This is a word based microcontroller, with addressing architecture
      based on the Harvard addressing scheme.

**************************************************************************/


#include "emu.h"
#include "tms320c1x.h"

#include "tms320c1x_dasm.h"



#define M_RDROM(A)      TMS320C1X_ROM_RDMEM(A)
#define M_WRTROM(A,V)   TMS320C1X_ROM_WRMEM(A,V)
#define M_RDRAM(A)      TMS320C1X_RAM_RDMEM(A)
#define M_WRTRAM(A,V)   TMS320C1X_RAM_WRMEM(A,V)
#define M_RDOP(A)       TMS320C1X_RDOP(A)
#define M_RDOP_ARG(A)   TMS320C1X_RDOP_ARG(A)
#define P_IN(A)         TMS320C1X_In(A)
#define P_OUT(A,V)      TMS320C1X_Out(A,V)


DEFINE_DEVICE_TYPE(TMS320C10, tms320c10_device, "tms320c10", "Texas Instruments TMS320C10")
DEFINE_DEVICE_TYPE(TMS320C15, tms320c15_device, "tms320c15", "Texas Instruments TMS320C15")
DEFINE_DEVICE_TYPE(TMS320C16, tms320c16_device, "tms320c16", "Texas Instruments TMS320C16")


/****************************************************************************
 *  TMS320C1x Internal Memory Map
 ****************************************************************************/

template <int HighBits>
void tms320c1x_device_base<HighBits>::tms320c10_ram(address_map &map)
{
	map(0x00, 0x7f).ram();     /* Page 0 */
	map(0x80, 0x8f).ram();     /* Page 1 */
}

/****************************************************************************
 *  TMS320C15/6 Internal Memory Map
 ****************************************************************************/

template <int HighBits>
void tms320c1x_device_base<HighBits>::tms320c15_ram(address_map &map)
{
	map(0x00, 0x7f).ram();     /* Page 0 */
	map(0x80, 0xff).ram();     /* Page 1 */
}


template <int HighBits>
tms320c1x_device_base<HighBits>::tms320c1x_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, HighBits, -1)
	, m_data_config("data", ENDIANNESS_BIG, 16, 8, -1, data_map)
	, m_io_config("io", ENDIANNESS_BIG, 16, 4, -1)
	, m_bio_in(*this, 0)
	, m_addr_mask(util::make_bitmask<uint32_t>(HighBits))
{
}


tms320c10_device::tms320c10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms320c1x_device_base(mconfig, TMS320C10, tag, owner, clock, address_map_constructor(FUNC(tms320c10_device::tms320c10_ram), this))
{
}


tms320c15_device::tms320c15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms320c1x_device_base(mconfig, TMS320C15, tag, owner, clock, address_map_constructor(FUNC(tms320c15_device::tms320c15_ram), this))
{
}


tms320c16_device::tms320c16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms320c1x_device_base(mconfig, TMS320C16, tag, owner, clock, address_map_constructor(FUNC(tms320c16_device::tms320c15_ram), this))
{
}

template <int HighBits>
device_memory_interface::space_config_vector tms320c1x_device_base<HighBits>::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

template <int HighBits>
std::unique_ptr<util::disasm_interface> tms320c1x_device_base<HighBits>::create_disassembler()
{
	return std::make_unique<tms320c1x_disassembler>();
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
 *  Input a word from given I/O port
 */

#define TMS320C1X_In(Port) (m_io.read_word(Port))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define TMS320C1X_Out(Port,Value) (m_io.write_word(Port,Value))



/****************************************************************************
 *  Read a word from given ROM memory location
 */

#define TMS320C1X_ROM_RDMEM(A) (m_program.read_word(A))


/****************************************************************************
 *  Write a word to given ROM memory location
 */

#define TMS320C1X_ROM_WRMEM(A,V) (m_program.write_word(A,V))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define TMS320C1X_RAM_RDMEM(A) (m_data.read_word(A))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define TMS320C1X_RAM_WRMEM(A,V) (m_data.write_word(A,V))



/****************************************************************************
 *  TMS320C1X_RDOP() is identical to TMS320C1X_RDMEM() except it is used for reading
 *  opcodes. In case of system with memory mapped I/O, this function can be
 *  used to greatly speed up emulation
 */

#define TMS320C1X_RDOP(A) (m_cache.read_word(A))


/****************************************************************************
 *  TMS320C1X_RDOP_ARG() is identical to TMS320C1X_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define TMS320C1X_RDOP_ARG(A) (m_cache.read_word(A))


/************************************************************************
 *  Shortcuts
 ************************************************************************/

template <int HighBits>
void tms320c1x_device_base<HighBits>::CLR(uint16_t flag) { m_STR &= ~flag; m_STR |= 0x1efe; }
template <int HighBits>
void tms320c1x_device_base<HighBits>::SET_FLAG(uint16_t flag) { m_STR |=  flag; m_STR |= 0x1efe; }


template <int HighBits>
void tms320c1x_device_base<HighBits>::CALCULATE_ADD_OVERFLOW(int32_t addval)
{
	if (int32_t(~(m_oldacc.d ^ addval) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.d = ((int32_t)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::CALCULATE_SUB_OVERFLOW(int32_t subval)
{
	if (int32_t((m_oldacc.d ^ subval) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.d = ((int32_t)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}

template <int HighBits>
uint16_t tms320c1x_device_base<HighBits>::POP_STACK()
{
	uint16_t data = m_STACK[3];
	m_STACK[3] = m_STACK[2];
	m_STACK[2] = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return (data & m_addr_mask);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::PUSH_STACK(uint16_t data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = m_STACK[2];
	m_STACK[2] = m_STACK[3];
	m_STACK[3] = (data & m_addr_mask);
}

template <int HighBits>
void tms320c1x_device_base<HighBits>::UPDATE_AR()
{
	if (m_opcode.b.l & 0x30) {
		uint16_t tmpAR = m_AR[ARP];
		if (m_opcode.b.l & 0x20) tmpAR++ ;
		if (m_opcode.b.l & 0x10) tmpAR-- ;
		m_AR[ARP] = (m_AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
	}
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::UPDATE_ARP()
{
	if (~m_opcode.b.l & 0x08) {
		if (m_opcode.b.l & 0x01) SET_FLAG(ARP_REG);
		else CLR(ARP_REG);
	}
}


template <int HighBits>
void tms320c1x_device_base<HighBits>::getdata(uint8_t shift,uint8_t signext)
{
	if (m_opcode.b.l & 0x80)
		m_memaccess = IND;
	else
		m_memaccess = DMA_DP;

	m_ALU.d = (uint16_t)M_RDRAM(m_memaccess);
	if (signext) m_ALU.d = (int16_t)m_ALU.d;
	m_ALU.d <<= shift;
	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}

template <int HighBits>
void tms320c1x_device_base<HighBits>::putdata(uint16_t data)
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
template <int HighBits>
void tms320c1x_device_base<HighBits>::putdata_sar(uint8_t data)
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
template <int HighBits>
void tms320c1x_device_base<HighBits>::putdata_sst(uint16_t data)
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

template <int HighBits>
void tms320c1x_device_base<HighBits>::opcodes_7F()
{
	fatalerror("Should never get here!\n");
}


template <int HighBits>
void tms320c1x_device_base<HighBits>::illegal()
{
	logerror("TMS320C1x:  PC=%04x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.w.l);
}

template <int HighBits>
void tms320c1x_device_base<HighBits>::abst()
{
	if (int32_t(m_ACC.d) < 0) {
		m_ACC.d = -m_ACC.d;
		if (OVM && (m_ACC.d == 0x80000000)) m_ACC.d-- ;
	}
}

/*** The manual doesn't mention overflow with the ADD? instructions however ***
 *** overflow is implemented here, because it makes little sense otherwise ****
 *** while newer generations of this type of chip supported it. The ***********
 *** manual may be wrong wrong (apart from other errors the manual has). ******

template <int HighBits>
void tms320c1x_device_base<HighBits>::add_sh()    { getdata(m_opcode.b.h,1); m_ACC.d += m_ALU.d; }
template <int HighBits>
void tms320c1x_device_base<HighBits>::addh()      { getdata(0,0); m_ACC.d += (m_ALU.d << 16); }
 ***/

template <int HighBits>
void tms320c1x_device_base<HighBits>::add_sh()
{
	m_oldacc.d = m_ACC.d;
	getdata((m_opcode.b.h & 0xf),1);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::addh()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.w.h += m_ALU.w.l;
	if (int16_t(~(m_oldacc.w.h ^ m_ALU.w.h) & (m_oldacc.w.h ^ m_ACC.w.h)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.w.h = ((int16_t)m_oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::adds()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::and_()
{
	getdata(0,0);
	m_ACC.d &= m_ALU.d;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::apac()
{
	m_oldacc.d = m_ACC.d;
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::br()
{
	m_PC = M_RDOP_ARG(m_PC);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::banz()
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
template <int HighBits>
void tms320c1x_device_base<HighBits>::bgez()
{
	if (int32_t(m_ACC.d) >= 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::bgz()
{
	if (int32_t(m_ACC.d) > 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::bioz()
{
	if (m_bio_in() != CLEAR_LINE) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::blez()
{
	if (int32_t(m_ACC.d) <= 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::blz()
{
	if (int32_t(m_ACC.d) <  0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::bnz()
{
	if (m_ACC.d != 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::bv()
{
	if (OV) {
		CLR(OV_FLAG);
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::bz()
{
	if (m_ACC.d == 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::cala()
{
	PUSH_STACK(m_PC);
	m_PC = m_ACC.w.l & m_addr_mask;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::call()
{
	m_PC++ ;
	PUSH_STACK(m_PC);
	m_PC = M_RDOP_ARG((m_PC - 1));
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::dint()
{
	SET_FLAG(INTM_FLAG);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::dmov()
{
	getdata(0,0);
	M_WRTRAM((m_memaccess + 1),m_ALU.w.l);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::eint()
{
	CLR(INTM_FLAG);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::in_p()
{
	m_ALU.w.l = P_IN(m_opcode.b.h & 7);
	putdata(m_ALU.w.l);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lac_sh()
{
	getdata((m_opcode.b.h & 0x0f),1);
	m_ACC.d = m_ALU.d;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lack()
{
	m_ACC.d = m_opcode.b.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lar_ar0()
{
	getdata(0,0);
	m_AR[0] = m_ALU.w.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lar_ar1()
{
	getdata(0,0);
	m_AR[1] = m_ALU.w.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lark_ar0()
{
	m_AR[0] = m_opcode.b.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lark_ar1()
{
	m_AR[1] = m_opcode.b.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::larp_mar()
{
	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::ldp()
{
	getdata(0,0);
	if (m_ALU.d & 1)
		SET_FLAG(DP_REG);
	else
		CLR(DP_REG);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::ldpk()
{
	if (m_opcode.b.l & 1)
		SET_FLAG(DP_REG);
	else
		CLR(DP_REG);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lst()
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
template <int HighBits>
void tms320c1x_device_base<HighBits>::lt()
{
	getdata(0,0);
	m_Treg = m_ALU.w.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::lta()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_Treg = m_ALU.w.l;
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::ltd()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_Treg = m_ALU.w.l;
	M_WRTRAM((m_memaccess + 1),m_ALU.w.l);
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::mpy()
{
	getdata(0,0);
	m_Preg.d = (int16_t)m_ALU.w.l * (int16_t)m_Treg;
	if (m_Preg.d == 0x40000000) m_Preg.d = 0xc0000000;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::mpyk()
{
	m_Preg.d = (int16_t)m_Treg * (int16_t(m_opcode.w.l << 3) >> 3);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::nop()
{
	/* Nothing to do */
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::or_()
{
	getdata(0,0);
	m_ACC.w.l |= m_ALU.w.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::out_p()
{
	getdata(0,0);
	P_OUT( (m_opcode.b.h & 7), m_ALU.w.l );
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::pac()
{
	m_ACC.d = m_Preg.d;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::pop()
{
	m_ACC.w.l = POP_STACK();
	m_ACC.w.h = 0x0000;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::push()
{
	PUSH_STACK(m_ACC.w.l);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::ret()
{
	m_PC = POP_STACK();
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::rovm()
{
	CLR(OVM_FLAG);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sach_sh()
{
	m_ALU.d = (m_ACC.d << (m_opcode.b.h & 7));
	putdata(m_ALU.w.h);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sacl()
{
	putdata(m_ACC.w.l);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sar_ar0()
{
	putdata_sar(0);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sar_ar1()
{
	putdata_sar(1);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sovm()
{
	SET_FLAG(OVM_FLAG);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::spac()
{
	m_oldacc.d = m_ACC.d;
	m_ACC.d -= m_Preg.d;
	CALCULATE_SUB_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sst()
{
	putdata_sst(m_STR);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::sub_sh()
{
	m_oldacc.d = m_ACC.d;
	getdata((m_opcode.b.h & 0x0f),1);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::subc()
{
	m_oldacc.d = m_ACC.d;
	getdata(15,0);
	m_ALU.d = (int32_t) m_ACC.d - m_ALU.d;
	if (int32_t((m_oldacc.d ^ m_ALU.d) & (m_oldacc.d ^ m_ACC.d)) < 0)
		SET_FLAG(OV_FLAG);
	if (int32_t(m_ALU.d) >= 0)
		m_ACC.d = (m_ALU.d << 1) + 1;
	else
		m_ACC.d = m_ACC.d << 1;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::subh()
{
	m_oldacc.d = m_ACC.d;
	getdata(16,0);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::subs()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::tblr()
{
	m_ALU.d = M_RDROM((m_ACC.w.l & m_addr_mask));
	putdata(m_ALU.w.l);
	m_STACK[0] = m_STACK[1];
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::tblw()
{
	getdata(0,0);
	M_WRTROM(((m_ACC.w.l & m_addr_mask)),m_ALU.w.l);
	m_STACK[0] = m_STACK[1];
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::xor_()
{
	getdata(0,0);
	m_ACC.w.l ^= m_ALU.w.l;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::zac()
{
	m_ACC.d = 0;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::zalh()
{
	getdata(0,0);
	m_ACC.w.h = m_ALU.w.l;
	m_ACC.w.l = 0x0000;
}
template <int HighBits>
void tms320c1x_device_base<HighBits>::zals()
{
	getdata(0,0);
	m_ACC.w.l = m_ALU.w.l;
	m_ACC.w.h = 0x0000;
}



/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

/* Conditional Branch instructions take two cycles when the test condition is met and the branch performed */

template <int HighBits>
const typename tms320c1x_device_base<HighBits>::tms320c1x_opcode tms320c1x_device_base<HighBits>::s_opcode_main[256]=
{
/*00*/  {1, &tms320c1x_device_base::add_sh  },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },
/*08*/  {1, &tms320c1x_device_base::add_sh  },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },{1, &tms320c1x_device_base::add_sh    },
/*10*/  {1, &tms320c1x_device_base::sub_sh  },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },
/*18*/  {1, &tms320c1x_device_base::sub_sh  },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },{1, &tms320c1x_device_base::sub_sh    },
/*20*/  {1, &tms320c1x_device_base::lac_sh  },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },
/*28*/  {1, &tms320c1x_device_base::lac_sh  },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },{1, &tms320c1x_device_base::lac_sh    },
/*30*/  {1, &tms320c1x_device_base::sar_ar0 },{1, &tms320c1x_device_base::sar_ar1   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*38*/  {1, &tms320c1x_device_base::lar_ar0 },{1, &tms320c1x_device_base::lar_ar1   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*40*/  {2, &tms320c1x_device_base::in_p    },{2, &tms320c1x_device_base::in_p      },{2, &tms320c1x_device_base::in_p      },{2, &tms320c1x_device_base::in_p      },{2, &tms320c1x_device_base::in_p      },{2, &tms320c1x_device_base::in_p      },{2, &tms320c1x_device_base::in_p      },{2, &tms320c1x_device_base::in_p      },
/*48*/  {2, &tms320c1x_device_base::out_p   },{2, &tms320c1x_device_base::out_p     },{2, &tms320c1x_device_base::out_p     },{2, &tms320c1x_device_base::out_p     },{2, &tms320c1x_device_base::out_p     },{2, &tms320c1x_device_base::out_p     },{2, &tms320c1x_device_base::out_p     },{2, &tms320c1x_device_base::out_p     },
/*50*/  {1, &tms320c1x_device_base::sacl    },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*58*/  {1, &tms320c1x_device_base::sach_sh },{1, &tms320c1x_device_base::sach_sh   },{1, &tms320c1x_device_base::sach_sh   },{1, &tms320c1x_device_base::sach_sh   },{1, &tms320c1x_device_base::sach_sh   },{1, &tms320c1x_device_base::sach_sh   },{1, &tms320c1x_device_base::sach_sh   },{1, &tms320c1x_device_base::sach_sh   },
/*60*/  {1, &tms320c1x_device_base::addh    },{1, &tms320c1x_device_base::adds      },{1, &tms320c1x_device_base::subh      },{1, &tms320c1x_device_base::subs      },{1, &tms320c1x_device_base::subc      },{1, &tms320c1x_device_base::zalh      },{1, &tms320c1x_device_base::zals      },{3, &tms320c1x_device_base::tblr      },
/*68*/  {1, &tms320c1x_device_base::larp_mar},{1, &tms320c1x_device_base::dmov      },{1, &tms320c1x_device_base::lt        },{1, &tms320c1x_device_base::ltd       },{1, &tms320c1x_device_base::lta       },{1, &tms320c1x_device_base::mpy       },{1, &tms320c1x_device_base::ldpk      },{1, &tms320c1x_device_base::ldp       },
/*70*/  {1, &tms320c1x_device_base::lark_ar0},{1, &tms320c1x_device_base::lark_ar1  },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*78*/  {1, &tms320c1x_device_base::xor_    },{1, &tms320c1x_device_base::and_      },{1, &tms320c1x_device_base::or_       },{1, &tms320c1x_device_base::lst       },{1, &tms320c1x_device_base::sst       },{3, &tms320c1x_device_base::tblw      },{1, &tms320c1x_device_base::lack      },{0, &tms320c1x_device_base::opcodes_7F    },
/*80*/  {1, &tms320c1x_device_base::mpyk    },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },
/*88*/  {1, &tms320c1x_device_base::mpyk    },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },
/*90*/  {1, &tms320c1x_device_base::mpyk    },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },
/*98*/  {1, &tms320c1x_device_base::mpyk    },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },{1, &tms320c1x_device_base::mpyk      },
/*A0*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*A8*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*B0*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*B8*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*C0*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*C8*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*D0*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*D8*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*E0*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*E8*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*F0*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{1, &tms320c1x_device_base::banz      },{1, &tms320c1x_device_base::bv        },{1, &tms320c1x_device_base::bioz      },{0, &tms320c1x_device_base::illegal   },
/*F8*/  {2, &tms320c1x_device_base::call    },{2, &tms320c1x_device_base::br        },{1, &tms320c1x_device_base::blz       },{1, &tms320c1x_device_base::blez      },{1, &tms320c1x_device_base::bgz       },{1, &tms320c1x_device_base::bgez      },{1, &tms320c1x_device_base::bnz       },{1, &tms320c1x_device_base::bz        }
};

template <int HighBits>
const typename tms320c1x_device_base<HighBits>::tms320c1x_opcode tms320c1x_device_base<HighBits>::s_opcode_7F[32]=
{
/*80*/  {1, &tms320c1x_device_base::nop     },{1, &tms320c1x_device_base::dint      },{1, &tms320c1x_device_base::eint      },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*88*/  {1, &tms320c1x_device_base::abst    },{1, &tms320c1x_device_base::zac       },{1, &tms320c1x_device_base::rovm      },{1, &tms320c1x_device_base::sovm      },{2, &tms320c1x_device_base::cala      },{2, &tms320c1x_device_base::ret       },{1, &tms320c1x_device_base::pac       },{1, &tms320c1x_device_base::apac      },
/*90*/  {1, &tms320c1x_device_base::spac    },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },
/*98*/  {0, &tms320c1x_device_base::illegal },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   },{2, &tms320c1x_device_base::push      },{2, &tms320c1x_device_base::pop       },{0, &tms320c1x_device_base::illegal   },{0, &tms320c1x_device_base::illegal   }
};

template <int HighBits>
int tms320c1x_device_base<HighBits>::add_branch_cycle()
{
	return s_opcode_main[m_opcode.b.h].cycles;
}

/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

template <int HighBits>
void tms320c1x_device_base<HighBits>::device_start()
{
	save_item(NAME(m_PC));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_STR));
	save_item(NAME(m_ACC.d));
	save_item(NAME(m_ALU.d));
	save_item(NAME(m_Preg.d));
	save_item(NAME(m_Treg));
	save_item(NAME(m_AR));
	save_item(NAME(m_STACK));
	save_item(NAME(m_INTF));
	save_item(NAME(m_opcode.d));
	save_item(NAME(m_oldacc.d));
	save_item(NAME(m_memaccess));
	save_item(NAME(m_addr_mask));

	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);
	space(AS_IO).specific(m_io);

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

	state_add( TMS320C1X_PC,   "PC",   m_PC).formatstr("%04X");
	state_add( TMS320C1X_STR,  "STR",  m_STR).formatstr("%04X");
	state_add( TMS320C1X_ACC,  "ACC",  m_ACC.d).formatstr("%08X");
	state_add( TMS320C1X_PREG, "P",    m_Preg.d).formatstr("%08X");
	state_add( TMS320C1X_TREG, "T",    m_Treg).formatstr("%04X");
	state_add( TMS320C1X_AR0,  "AR0",  m_AR[0]).formatstr("%04X");
	state_add( TMS320C1X_AR1,  "AR1",  m_AR[1]).formatstr("%04X");
	state_add( TMS320C1X_STK0, "STK0", m_STACK[0]).formatstr("%04X");
	state_add( TMS320C1X_STK1, "STK1", m_STACK[1]).formatstr("%04X");
	state_add( TMS320C1X_STK2, "STK2", m_STACK[2]).formatstr("%04X");
	state_add( TMS320C1X_STK3, "STK3", m_STACK[3]).formatstr("%04X");

	state_add(STATE_GENPC, "GENPC", m_PC).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_PREVPC).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_STR).formatstr("%16s").noshow();

	set_icountptr(m_icount);
}


/****************************************************************************
 *  TMS320C1x Reset registers to their initial values
 ****************************************************************************/

template <int HighBits>
void tms320c1x_device_base<HighBits>::device_reset()
{
	m_PC    = 0;
	m_ACC.d = 0;
	m_INTF  = TMS320C1X_INT_NONE;
	/* Setup Status Register : 7efe */
	CLR((OV_FLAG | ARP_REG | DP_REG));
	SET_FLAG((OVM_FLAG | INTM_FLAG));
}


template <int HighBits>
void tms320c1x_device_base<HighBits>::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
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

template <int HighBits>
void tms320c1x_device_base<HighBits>::execute_set_input(int irqline, int state)
{
	/* Pending Interrupts cannot be cleared! */
	if (state == ASSERT_LINE) m_INTF |= TMS320C1X_INT_PENDING;
}



/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/

template <int HighBits>
int tms320c1x_device_base<HighBits>::Ext_IRQ()
{
	if (INTM == 0)
	{
		standard_irq_callback(0, m_PC);
		m_INTF = TMS320C1X_INT_NONE;
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

template <int HighBits>
void tms320c1x_device_base<HighBits>::execute_run()
{
	do
	{
		if (m_INTF) {
			/* Don't service INT if previous instruction was MPY, MPYK or EINT */
			if ((m_opcode.b.h != 0x6d) && ((m_opcode.b.h & 0xe0) != 0x80) && (m_opcode.w.l != 0x7f82))
				m_icount -= Ext_IRQ();
		}

		m_PREVPC = m_PC;

		debugger_instruction_hook(m_PC);

		m_opcode.d = M_RDOP(m_PC);
		m_PC++;

		if (m_opcode.b.h != 0x7f) { /* Do all opcodes except the 7Fxx ones */
			m_icount -= s_opcode_main[m_opcode.b.h].cycles;
			(this->*s_opcode_main[m_opcode.b.h].function)();
		}
		else { /* Opcode major byte 7Fxx has many opcodes in its minor byte */
			m_icount -= s_opcode_7F[(m_opcode.b.l & 0x1f)].cycles;
			(this->*s_opcode_7F[(m_opcode.b.l & 0x1f)].function)();
		}
	} while (m_icount > 0);
}
