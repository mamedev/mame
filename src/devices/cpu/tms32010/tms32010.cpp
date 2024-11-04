// license:BSD-3-Clause
// copyright-holders:Tony La Porta
/**************************************************************************

    Texas Instruments TMS32010 DSP Emulator

    Copyright Tony La Porta

    Notes:
    * The term 'DMA' within this document, is in reference to Direct
      Memory Addressing, and NOT the usual term of Direct Memory Access.
    * This is a word based microcontroller, with addressing architecture
      based on the Harvard addressing scheme.

**************************************************************************/


#include "emu.h"
#include "tms32010.h"
#include "32010dsm.h"



#define M_RDROM(A)      TMS32010_ROM_RDMEM(A)
#define M_WRTROM(A,V)   TMS32010_ROM_WRMEM(A,V)
#define M_RDRAM(A)      TMS32010_RAM_RDMEM(A)
#define M_WRTRAM(A,V)   TMS32010_RAM_WRMEM(A,V)
#define M_RDOP(A)       TMS32010_RDOP(A)
#define M_RDOP_ARG(A)   TMS32010_RDOP_ARG(A)
#define P_IN(A)         TMS32010_In(A)
#define P_OUT(A,V)      TMS32010_Out(A,V)


DEFINE_DEVICE_TYPE(TMS32010, tms32010_device, "tms32010", "Texas Instruments TMS32010")
DEFINE_DEVICE_TYPE(TMS32015, tms32015_device, "tms32015", "Texas Instruments TMS32015")
DEFINE_DEVICE_TYPE(TMS32016, tms32016_device, "tms32016", "Texas Instruments TMS32016")


/****************************************************************************
 *  TMS32010 Internal Memory Map
 ****************************************************************************/

template <int HighBits>
void tms3201x_base_device<HighBits>::tms32010_ram(address_map &map)
{
	map(0x00, 0x7f).ram();     /* Page 0 */
	map(0x80, 0x8f).ram();     /* Page 1 */
}

/****************************************************************************
 *  TMS32015/6 Internal Memory Map
 ****************************************************************************/

template <int HighBits>
void tms3201x_base_device<HighBits>::tms32015_ram(address_map &map)
{
	map(0x00, 0x7f).ram();     /* Page 0 */
	map(0x80, 0xff).ram();     /* Page 1 */
}


template <int HighBits>
tms3201x_base_device<HighBits>::tms3201x_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, HighBits, -1)
	, m_data_config("data", ENDIANNESS_BIG, 16, 8, -1, data_map)
	, m_io_config("io", ENDIANNESS_BIG, 16, 4, -1)
	, m_bio_in(*this, 0)
	, m_addr_mask(util::make_bitmask<uint32_t>(HighBits))
{
}


tms32010_device::tms32010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3201x_base_device(mconfig, TMS32010, tag, owner, clock, address_map_constructor(FUNC(tms32010_device::tms32010_ram), this))
{
}


tms32015_device::tms32015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3201x_base_device(mconfig, TMS32015, tag, owner, clock, address_map_constructor(FUNC(tms32015_device::tms32015_ram), this))
{
}


tms32016_device::tms32016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms3201x_base_device(mconfig, TMS32016, tag, owner, clock, address_map_constructor(FUNC(tms32016_device::tms32015_ram), this))
{
}

template <int HighBits>
device_memory_interface::space_config_vector tms3201x_base_device<HighBits>::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

template <int HighBits>
std::unique_ptr<util::disasm_interface> tms3201x_base_device<HighBits>::create_disassembler()
{
	return std::make_unique<tms32010_disassembler>();
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

#define TMS32010_In(Port) (m_io.read_word(Port))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define TMS32010_Out(Port,Value) (m_io.write_word(Port,Value))



/****************************************************************************
 *  Read a word from given ROM memory location
 */

#define TMS32010_ROM_RDMEM(A) (m_program.read_word(A))


/****************************************************************************
 *  Write a word to given ROM memory location
 */

#define TMS32010_ROM_WRMEM(A,V) (m_program.write_word(A,V))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define TMS32010_RAM_RDMEM(A) (m_data.read_word(A))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define TMS32010_RAM_WRMEM(A,V) (m_data.write_word(A,V))



/****************************************************************************
 *  TMS32010_RDOP() is identical to TMS32010_RDMEM() except it is used for reading
 *  opcodes. In case of system with memory mapped I/O, this function can be
 *  used to greatly speed up emulation
 */

#define TMS32010_RDOP(A) (m_cache.read_word(A))


/****************************************************************************
 *  TMS32010_RDOP_ARG() is identical to TMS32010_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define TMS32010_RDOP_ARG(A) (m_cache.read_word(A))


/************************************************************************
 *  Shortcuts
 ************************************************************************/

template <int HighBits>
void tms3201x_base_device<HighBits>::CLR(uint16_t flag) { m_STR &= ~flag; m_STR |= 0x1efe; }
template <int HighBits>
void tms3201x_base_device<HighBits>::SET_FLAG(uint16_t flag) { m_STR |=  flag; m_STR |= 0x1efe; }


template <int HighBits>
void tms3201x_base_device<HighBits>::CALCULATE_ADD_OVERFLOW(int32_t addval)
{
	if (int32_t(~(m_oldacc.d ^ addval) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.d = ((int32_t)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}
template <int HighBits>
void tms3201x_base_device<HighBits>::CALCULATE_SUB_OVERFLOW(int32_t subval)
{
	if (int32_t((m_oldacc.d ^ subval) & (m_oldacc.d ^ m_ACC.d)) < 0) {
		SET_FLAG(OV_FLAG);
		if (OVM)
			m_ACC.d = ((int32_t)m_oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}

template <int HighBits>
uint16_t tms3201x_base_device<HighBits>::POP_STACK()
{
	uint16_t data = m_STACK[3];
	m_STACK[3] = m_STACK[2];
	m_STACK[2] = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return (data & m_addr_mask);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::PUSH_STACK(uint16_t data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = m_STACK[2];
	m_STACK[2] = m_STACK[3];
	m_STACK[3] = (data & m_addr_mask);
}

template <int HighBits>
void tms3201x_base_device<HighBits>::UPDATE_AR()
{
	if (m_opcode.b.l & 0x30) {
		uint16_t tmpAR = m_AR[ARP];
		if (m_opcode.b.l & 0x20) tmpAR++ ;
		if (m_opcode.b.l & 0x10) tmpAR-- ;
		m_AR[ARP] = (m_AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
	}
}
template <int HighBits>
void tms3201x_base_device<HighBits>::UPDATE_ARP()
{
	if (~m_opcode.b.l & 0x08) {
		if (m_opcode.b.l & 0x01) SET_FLAG(ARP_REG);
		else CLR(ARP_REG);
	}
}


template <int HighBits>
void tms3201x_base_device<HighBits>::getdata(uint8_t shift,uint8_t signext)
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
void tms3201x_base_device<HighBits>::putdata(uint16_t data)
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
void tms3201x_base_device<HighBits>::putdata_sar(uint8_t data)
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
void tms3201x_base_device<HighBits>::putdata_sst(uint16_t data)
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
void tms3201x_base_device<HighBits>::opcodes_7F()
{
	fatalerror("Should never get here!\n");
}


template <int HighBits>
void tms3201x_base_device<HighBits>::illegal()
{
	logerror("TMS32010:  PC=%04x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.w.l);
}

template <int HighBits>
void tms3201x_base_device<HighBits>::abst()
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
void tms3201x_base_device<HighBits>::add_sh()    { getdata(m_opcode.b.h,1); m_ACC.d += m_ALU.d; }
template <int HighBits>
void tms3201x_base_device<HighBits>::addh()      { getdata(0,0); m_ACC.d += (m_ALU.d << 16); }
 ***/

template <int HighBits>
void tms3201x_base_device<HighBits>::add_sh()
{
	m_oldacc.d = m_ACC.d;
	getdata((m_opcode.b.h & 0xf),1);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::addh()
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
void tms3201x_base_device<HighBits>::adds()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.d += m_ALU.d;
	CALCULATE_ADD_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::and_()
{
	getdata(0,0);
	m_ACC.d &= m_ALU.d;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::apac()
{
	m_oldacc.d = m_ACC.d;
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::br()
{
	m_PC = M_RDOP_ARG(m_PC);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::banz()
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
void tms3201x_base_device<HighBits>::bgez()
{
	if (int32_t(m_ACC.d) >= 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::bgz()
{
	if (int32_t(m_ACC.d) > 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::bioz()
{
	if (m_bio_in() != CLEAR_LINE) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::blez()
{
	if (int32_t(m_ACC.d) <= 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::blz()
{
	if (int32_t(m_ACC.d) <  0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::bnz()
{
	if (m_ACC.d != 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::bv()
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
void tms3201x_base_device<HighBits>::bz()
{
	if (m_ACC.d == 0) {
		m_PC = M_RDOP_ARG(m_PC);
		m_icount -= add_branch_cycle();
	}
	else
		m_PC++ ;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::cala()
{
	PUSH_STACK(m_PC);
	m_PC = m_ACC.w.l & m_addr_mask;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::call()
{
	m_PC++ ;
	PUSH_STACK(m_PC);
	m_PC = M_RDOP_ARG((m_PC - 1));
}
template <int HighBits>
void tms3201x_base_device<HighBits>::dint()
{
	SET_FLAG(INTM_FLAG);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::dmov()
{
	getdata(0,0);
	M_WRTRAM((m_memaccess + 1),m_ALU.w.l);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::eint()
{
	CLR(INTM_FLAG);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::in_p()
{
	m_ALU.w.l = P_IN(m_opcode.b.h & 7);
	putdata(m_ALU.w.l);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lac_sh()
{
	getdata((m_opcode.b.h & 0x0f),1);
	m_ACC.d = m_ALU.d;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lack()
{
	m_ACC.d = m_opcode.b.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lar_ar0()
{
	getdata(0,0);
	m_AR[0] = m_ALU.w.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lar_ar1()
{
	getdata(0,0);
	m_AR[1] = m_ALU.w.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lark_ar0()
{
	m_AR[0] = m_opcode.b.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lark_ar1()
{
	m_AR[1] = m_opcode.b.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::larp_mar()
{
	if (m_opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}
template <int HighBits>
void tms3201x_base_device<HighBits>::ldp()
{
	getdata(0,0);
	if (m_ALU.d & 1)
		SET_FLAG(DP_REG);
	else
		CLR(DP_REG);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::ldpk()
{
	if (m_opcode.b.l & 1)
		SET_FLAG(DP_REG);
	else
		CLR(DP_REG);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lst()
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
void tms3201x_base_device<HighBits>::lt()
{
	getdata(0,0);
	m_Treg = m_ALU.w.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::lta()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_Treg = m_ALU.w.l;
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::ltd()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_Treg = m_ALU.w.l;
	M_WRTRAM((m_memaccess + 1),m_ALU.w.l);
	m_ACC.d += m_Preg.d;
	CALCULATE_ADD_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::mpy()
{
	getdata(0,0);
	m_Preg.d = (int16_t)m_ALU.w.l * (int16_t)m_Treg;
	if (m_Preg.d == 0x40000000) m_Preg.d = 0xc0000000;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::mpyk()
{
	m_Preg.d = (int16_t)m_Treg * (int16_t(m_opcode.w.l << 3) >> 3);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::nop()
{
	/* Nothing to do */
}
template <int HighBits>
void tms3201x_base_device<HighBits>::or_()
{
	getdata(0,0);
	m_ACC.w.l |= m_ALU.w.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::out_p()
{
	getdata(0,0);
	P_OUT( (m_opcode.b.h & 7), m_ALU.w.l );
}
template <int HighBits>
void tms3201x_base_device<HighBits>::pac()
{
	m_ACC.d = m_Preg.d;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::pop()
{
	m_ACC.w.l = POP_STACK();
	m_ACC.w.h = 0x0000;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::push()
{
	PUSH_STACK(m_ACC.w.l);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::ret()
{
	m_PC = POP_STACK();
}
template <int HighBits>
void tms3201x_base_device<HighBits>::rovm()
{
	CLR(OVM_FLAG);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sach_sh()
{
	m_ALU.d = (m_ACC.d << (m_opcode.b.h & 7));
	putdata(m_ALU.w.h);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sacl()
{
	putdata(m_ACC.w.l);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sar_ar0()
{
	putdata_sar(0);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sar_ar1()
{
	putdata_sar(1);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sovm()
{
	SET_FLAG(OVM_FLAG);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::spac()
{
	m_oldacc.d = m_ACC.d;
	m_ACC.d -= m_Preg.d;
	CALCULATE_SUB_OVERFLOW(m_Preg.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sst()
{
	putdata_sst(m_STR);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::sub_sh()
{
	m_oldacc.d = m_ACC.d;
	getdata((m_opcode.b.h & 0x0f),1);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::subc()
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
void tms3201x_base_device<HighBits>::subh()
{
	m_oldacc.d = m_ACC.d;
	getdata(16,0);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::subs()
{
	m_oldacc.d = m_ACC.d;
	getdata(0,0);
	m_ACC.d -= m_ALU.d;
	CALCULATE_SUB_OVERFLOW(m_ALU.d);
}
template <int HighBits>
void tms3201x_base_device<HighBits>::tblr()
{
	m_ALU.d = M_RDROM((m_ACC.w.l & m_addr_mask));
	putdata(m_ALU.w.l);
	m_STACK[0] = m_STACK[1];
}
template <int HighBits>
void tms3201x_base_device<HighBits>::tblw()
{
	getdata(0,0);
	M_WRTROM(((m_ACC.w.l & m_addr_mask)),m_ALU.w.l);
	m_STACK[0] = m_STACK[1];
}
template <int HighBits>
void tms3201x_base_device<HighBits>::xor_()
{
	getdata(0,0);
	m_ACC.w.l ^= m_ALU.w.l;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::zac()
{
	m_ACC.d = 0;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::zalh()
{
	getdata(0,0);
	m_ACC.w.h = m_ALU.w.l;
	m_ACC.w.l = 0x0000;
}
template <int HighBits>
void tms3201x_base_device<HighBits>::zals()
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
const typename tms3201x_base_device<HighBits>::tms32010_opcode tms3201x_base_device<HighBits>::s_opcode_main[256]=
{
/*00*/  {1, &tms3201x_base_device::add_sh  },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },
/*08*/  {1, &tms3201x_base_device::add_sh  },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },{1, &tms3201x_base_device::add_sh    },
/*10*/  {1, &tms3201x_base_device::sub_sh  },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },
/*18*/  {1, &tms3201x_base_device::sub_sh  },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },{1, &tms3201x_base_device::sub_sh    },
/*20*/  {1, &tms3201x_base_device::lac_sh  },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },
/*28*/  {1, &tms3201x_base_device::lac_sh  },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },{1, &tms3201x_base_device::lac_sh    },
/*30*/  {1, &tms3201x_base_device::sar_ar0 },{1, &tms3201x_base_device::sar_ar1   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*38*/  {1, &tms3201x_base_device::lar_ar0 },{1, &tms3201x_base_device::lar_ar1   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*40*/  {2, &tms3201x_base_device::in_p    },{2, &tms3201x_base_device::in_p      },{2, &tms3201x_base_device::in_p      },{2, &tms3201x_base_device::in_p      },{2, &tms3201x_base_device::in_p      },{2, &tms3201x_base_device::in_p      },{2, &tms3201x_base_device::in_p      },{2, &tms3201x_base_device::in_p      },
/*48*/  {2, &tms3201x_base_device::out_p   },{2, &tms3201x_base_device::out_p     },{2, &tms3201x_base_device::out_p     },{2, &tms3201x_base_device::out_p     },{2, &tms3201x_base_device::out_p     },{2, &tms3201x_base_device::out_p     },{2, &tms3201x_base_device::out_p     },{2, &tms3201x_base_device::out_p     },
/*50*/  {1, &tms3201x_base_device::sacl    },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*58*/  {1, &tms3201x_base_device::sach_sh },{1, &tms3201x_base_device::sach_sh   },{1, &tms3201x_base_device::sach_sh   },{1, &tms3201x_base_device::sach_sh   },{1, &tms3201x_base_device::sach_sh   },{1, &tms3201x_base_device::sach_sh   },{1, &tms3201x_base_device::sach_sh   },{1, &tms3201x_base_device::sach_sh   },
/*60*/  {1, &tms3201x_base_device::addh    },{1, &tms3201x_base_device::adds      },{1, &tms3201x_base_device::subh      },{1, &tms3201x_base_device::subs      },{1, &tms3201x_base_device::subc      },{1, &tms3201x_base_device::zalh      },{1, &tms3201x_base_device::zals      },{3, &tms3201x_base_device::tblr      },
/*68*/  {1, &tms3201x_base_device::larp_mar},{1, &tms3201x_base_device::dmov      },{1, &tms3201x_base_device::lt        },{1, &tms3201x_base_device::ltd       },{1, &tms3201x_base_device::lta       },{1, &tms3201x_base_device::mpy       },{1, &tms3201x_base_device::ldpk      },{1, &tms3201x_base_device::ldp       },
/*70*/  {1, &tms3201x_base_device::lark_ar0},{1, &tms3201x_base_device::lark_ar1  },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*78*/  {1, &tms3201x_base_device::xor_    },{1, &tms3201x_base_device::and_      },{1, &tms3201x_base_device::or_       },{1, &tms3201x_base_device::lst       },{1, &tms3201x_base_device::sst       },{3, &tms3201x_base_device::tblw      },{1, &tms3201x_base_device::lack      },{0, &tms3201x_base_device::opcodes_7F    },
/*80*/  {1, &tms3201x_base_device::mpyk    },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },
/*88*/  {1, &tms3201x_base_device::mpyk    },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },
/*90*/  {1, &tms3201x_base_device::mpyk    },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },
/*98*/  {1, &tms3201x_base_device::mpyk    },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },{1, &tms3201x_base_device::mpyk      },
/*A0*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*A8*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*B0*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*B8*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*C0*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*C8*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*D0*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*D8*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*E0*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*E8*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*F0*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{1, &tms3201x_base_device::banz      },{1, &tms3201x_base_device::bv        },{1, &tms3201x_base_device::bioz      },{0, &tms3201x_base_device::illegal   },
/*F8*/  {2, &tms3201x_base_device::call    },{2, &tms3201x_base_device::br        },{1, &tms3201x_base_device::blz       },{1, &tms3201x_base_device::blez      },{1, &tms3201x_base_device::bgz       },{1, &tms3201x_base_device::bgez      },{1, &tms3201x_base_device::bnz       },{1, &tms3201x_base_device::bz        }
};

template <int HighBits>
const typename tms3201x_base_device<HighBits>::tms32010_opcode tms3201x_base_device<HighBits>::s_opcode_7F[32]=
{
/*80*/  {1, &tms3201x_base_device::nop     },{1, &tms3201x_base_device::dint      },{1, &tms3201x_base_device::eint      },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*88*/  {1, &tms3201x_base_device::abst    },{1, &tms3201x_base_device::zac       },{1, &tms3201x_base_device::rovm      },{1, &tms3201x_base_device::sovm      },{2, &tms3201x_base_device::cala      },{2, &tms3201x_base_device::ret       },{1, &tms3201x_base_device::pac       },{1, &tms3201x_base_device::apac      },
/*90*/  {1, &tms3201x_base_device::spac    },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },
/*98*/  {0, &tms3201x_base_device::illegal },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   },{2, &tms3201x_base_device::push      },{2, &tms3201x_base_device::pop       },{0, &tms3201x_base_device::illegal   },{0, &tms3201x_base_device::illegal   }
};

template <int HighBits>
int tms3201x_base_device<HighBits>::add_branch_cycle()
{
	return s_opcode_main[m_opcode.b.h].cycles;
}

/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

template <int HighBits>
void tms3201x_base_device<HighBits>::device_start()
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
	state_add(STATE_GENPCBASE, "CURPC", m_PREVPC).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_STR).formatstr("%16s").noshow();

	set_icountptr(m_icount);
}


/****************************************************************************
 *  TMS32010 Reset registers to their initial values
 ****************************************************************************/

template <int HighBits>
void tms3201x_base_device<HighBits>::device_reset()
{
	m_PC    = 0;
	m_ACC.d = 0;
	m_INTF  = TMS32010_INT_NONE;
	/* Setup Status Register : 7efe */
	CLR((OV_FLAG | ARP_REG | DP_REG));
	SET_FLAG((OVM_FLAG | INTM_FLAG));
}


template <int HighBits>
void tms3201x_base_device<HighBits>::state_string_export(const device_state_entry &entry, std::string &str) const
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
void tms3201x_base_device<HighBits>::execute_set_input(int irqline, int state)
{
	/* Pending Interrupts cannot be cleared! */
	if (state == ASSERT_LINE) m_INTF |= TMS32010_INT_PENDING;
}



/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/

template <int HighBits>
int tms3201x_base_device<HighBits>::Ext_IRQ()
{
	if (INTM == 0)
	{
		standard_irq_callback(0, m_PC);
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

template <int HighBits>
void tms3201x_base_device<HighBits>::execute_run()
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
