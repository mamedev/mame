// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller,Ernesto Corvi
/*****************************************************************************
 *
 *   z8000ops.inc
 *   Portable Z8000(2) emulator
 *   Opcode functions
 *
 *****************************************************************************/

/******************************************
 helper functions
 ******************************************/

/******************************************
 check new fcw for switch to system mode
 and swap stack pointer if needed
 ******************************************/
void z8002_device::CHANGE_FCW(uint16_t fcw)
{
	uint16_t tmp;
	if ((fcw ^ m_fcw) & F_S_N)            /* system/user mode change? */
	{
		tmp = RW(15);
		RW(15) = m_nspoff;
		m_nspoff = tmp;
	}

	fcw &= ~F_SEG;  /* never set segmented mode bit on Z8002 */

	if (!(m_fcw & F_NVIE) && (fcw & F_NVIE) && (m_irq_state[0] != CLEAR_LINE))
	{
		m_irq_req |= Z8000_NVI;
	}
	if (!(m_fcw & F_VIE) && (fcw & F_VIE) && (m_irq_state[1] != CLEAR_LINE))
	{
		m_irq_req |= Z8000_VI;
	}
	m_fcw = fcw;  /* set new m_fcw */
}

void z8001_device::CHANGE_FCW(uint16_t fcw)
{
	uint16_t tmp;
	if ((fcw ^ m_fcw) & F_S_N)            /* system/user mode change? */
	{
		tmp = RW(15);
		RW(15) = m_nspoff;
		m_nspoff = tmp;
	}
	/* User mode R14 is used in user mode and non-segmented system mode.
	   System mode R14 is only used in segmented system mode.
	   There is no transition from user mode to non-segmented system mode,
	   so this doesn't need to be handled here. */
	if (fcw & F_S_N)    /* new mode is system mode */
	{
		if (!(m_fcw & F_S_N)                /* old mode was user mode */
			|| ((fcw ^ m_fcw) & F_SEG))     /* or switch between segmented and non-segmented */
		{
			tmp = RW(14);
			RW(14) = m_nspseg;
			m_nspseg = tmp;
		}
	}
	else    /* new mode is user mode */
	{
		if (m_fcw & F_S_N          /* old mode was system mode */
			&& m_fcw & F_SEG)      /* and was segmented */
		{
			tmp = RW(14);
			RW(14) = m_nspseg;
			m_nspseg = tmp;
		}
	}

	if (!(m_fcw & F_NVIE) && (fcw & F_NVIE) && (m_irq_state[0] != CLEAR_LINE))
	{
		m_irq_req |= Z8000_NVI;
	}
	if (!(m_fcw & F_VIE) && (fcw & F_VIE) && (m_irq_state[1] != CLEAR_LINE))
	{
		m_irq_req |= Z8000_VI;
	}
	m_fcw = fcw;  /* set new m_fcw */
}

uint32_t z8002_device::make_segmented_addr(uint32_t addr)
{
	return ((addr & 0x007f0000) << 8) | 0x80000000 | (addr & 0xffff);
}

uint32_t z8002_device::segmented_addr(uint32_t addr)
{
	return ((addr & 0x7f000000) >> 8) | (addr & 0xffff);
}

uint32_t z8002_device::addr_from_reg(int regno)
{
	if (get_segmented_mode())
		return segmented_addr(RL(regno));
	else
		return RW(regno);
}

void z8002_device::addr_to_reg(int regno, uint32_t addr)
{
	if (get_segmented_mode()) {
		uint32_t segaddr = make_segmented_addr(addr);
		RW(regno) = (RW(regno) & 0x80ff) | ((segaddr >> 16) & 0x7f00);
		RW(regno | 1) = segaddr & 0xffff;
	}
	else
		RW(regno) = addr;
}

void z8002_device::add_to_addr_reg(int regno, uint16_t addend)
{
	if (get_segmented_mode())
		regno |= 1;
	RW(regno) += addend;
}

void z8002_device::sub_from_addr_reg(int regno, uint16_t subtrahend)
{
	if (get_segmented_mode())
		regno |= 1;
	RW(regno) -= subtrahend;
}

void z8002_device::set_pc(uint32_t addr)
{
	if (get_segmented_mode())
		m_pc = addr;
	else
		m_pc = (m_pc & 0xffff0000) | (addr & 0xffff);
}

uint8_t z8002_device::RDIR_B(uint8_t reg)
{
	return RDMEM_B(reg == SP ? m_stack : m_data, addr_from_reg(reg));
}

uint16_t z8002_device::RDIR_W(uint8_t reg)
{
	return RDMEM_W(reg == SP ? m_stack : m_data, addr_from_reg(reg));
}

uint32_t z8002_device::RDIR_L(uint8_t reg)
{
	return RDMEM_L(reg == SP ? m_stack : m_data, addr_from_reg(reg));
}

void z8002_device::WRIR_B(uint8_t reg, uint8_t value)
{
	WRMEM_B(reg == SP ? m_stack : m_data, addr_from_reg(reg), value);
}

void z8002_device::WRIR_W(uint8_t reg, uint16_t value)
{
	WRMEM_W(reg == SP ? m_stack : m_data, addr_from_reg(reg), value);
}

void z8002_device::WRIR_L(uint8_t reg, uint32_t value)
{
	WRMEM_L(reg == SP ? m_stack : m_data, addr_from_reg(reg), value);
}

uint8_t z8002_device::RDBX_B(uint8_t reg, uint16_t idx)
{
	return RDMEM_B(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx));
}

uint16_t z8002_device::RDBX_W(uint8_t reg, uint16_t idx)
{
	return RDMEM_W(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx));
}

uint32_t z8002_device::RDBX_L(uint8_t reg, uint16_t idx)
{
	return RDMEM_L(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx));
}

void z8002_device::WRBX_B(uint8_t reg, uint16_t idx, uint8_t value)
{
	WRMEM_B(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx), value);
}

void z8002_device::WRBX_W(uint8_t reg, uint16_t idx, uint16_t value)
{
	WRMEM_W(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx), value);
}

void z8002_device::WRBX_L(uint8_t reg, uint16_t idx, uint32_t value)
{
	WRMEM_L(reg == SP ? m_stack : m_data, addr_add(addr_from_reg(reg), idx), value);
}

void z8002_device::PUSHW(uint8_t dst, uint16_t value)
{
	if (get_segmented_mode())
		RW(dst | 1) -= 2;
	else
		RW(dst) -= 2;
	WRIR_W(dst, value);
}

uint16_t z8002_device::POPW(uint8_t src)
{
	uint16_t result = RDIR_W(src);
	if (get_segmented_mode())
		RW(src | 1) += 2;
	else
		RW(src) += 2;
	return result;
}

void z8002_device::PUSHL(uint8_t dst, uint32_t value)
{
	if (get_segmented_mode())
		RW(dst | 1) -= 4;
	else
		RW(dst) -= 4;
	WRIR_L(dst, value);
}

uint32_t z8002_device::POPL(uint8_t src)
{
	uint32_t result = RDIR_L(src);
	if (get_segmented_mode())
		RW(src | 1) += 4;
	else
		RW(src) += 4;
	return result;
}

/* check zero and sign flag for byte, word and long results */
#define CHK_XXXB_ZS if (!result) SET_Z; else if ((int8_t) result < 0) SET_S
#define CHK_XXXW_ZS if (!result) SET_Z; else if ((int16_t)result < 0) SET_S
#define CHK_XXXL_ZS if (!result) SET_Z; else if ((int32_t)result < 0) SET_S
#define CHK_XXXQ_ZS if (!result) SET_Z; else if ((int64_t)result < 0) SET_S

#define CHK_XXXB_ZSP m_fcw |= z8000_zsp[result]

/* check carry for addition and subtraction */
#define CHK_ADDX_C if (result < dest) SET_C
#define CHK_ADCX_C if (result < dest || (result == dest && value)) SET_C

#define CHK_SUBX_C if (result > dest) SET_C
#define CHK_SBCX_C if (result > dest || (result == dest && value)) SET_C

/* check half carry for A addition and S subtraction */
#define CHK_ADDB_H  if ((result & 15) < (dest & 15)) SET_H
#define CHK_ADCB_H  if ((result & 15) < (dest & 15) || ((result & 15) == (dest & 15) && (value & 15))) SET_H

#define CHK_SUBB_H  if ((result & 15) > (dest & 15)) SET_H
#define CHK_SBCB_H  if ((result & 15) > (dest & 15) || ((result & 15) == (dest & 15) && (value & 15))) SET_H

/* check overflow for addition for byte, word and long */
#define CHK_ADDB_V if (((value & dest & ~result) | (~value & ~dest & result)) & S08) SET_V
#define CHK_ADDW_V if (((value & dest & ~result) | (~value & ~dest & result)) & S16) SET_V
#define CHK_ADDL_V if (((value & dest & ~result) | (~value & ~dest & result)) & S32) SET_V

/* check overflow for subtraction for byte, word and long */
#define CHK_SUBB_V if (((~value & dest & ~result) | (value & ~dest & result)) & S08) SET_V
#define CHK_SUBW_V if (((~value & dest & ~result) | (value & ~dest & result)) & S16) SET_V
#define CHK_SUBL_V if (((~value & dest & ~result) | (value & ~dest & result)) & S32) SET_V

/* check for privileged instruction and trap if executed */
#define CHECK_PRIVILEGED_INSTR() if (!(m_fcw & F_S_N)) { m_irq_req |= Z8000_TRAP; return; }

/* if no EPU is present (it isn't), raise an extended intstuction trap */
#define CHECK_EXT_INSTR()  if (!(m_fcw & F_EPU)) { m_irq_req |= Z8000_EPU; return; }


/******************************************
 add byte
 flags:  CZSVDH
 ******************************************/
uint8_t z8002_device::ADDB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest + value;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	CLR_DA;         /* clear DA (decimal adjust) flag for addb */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDX_C;     /* set C if result overflowed              */
	CHK_ADDB_V;     /* set V if result has incorrect sign      */
	CHK_ADDB_H;     /* set H if lower nibble overflowed        */
	return result;
}

/******************************************
 add word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::ADDW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest + value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_ADDX_C;     /* set C if result overflowed              */
	CHK_ADDW_V;     /* set V if result has incorrect sign      */
	return result;
}

/******************************************
 add long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::ADDL(uint32_t dest, uint32_t value)
{
	uint32_t result = dest + value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_ADDX_C;     /* set C if result overflowed              */
	CHK_ADDL_V;     /* set V if result has incorrect sign      */
	return result;
}

/******************************************
 add with carry byte
 flags:  CZSVDH
 ******************************************/
uint8_t z8002_device::ADCB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest + value + GET_C;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	CLR_DA;         /* clear DA (decimal adjust) flag for adcb */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADCX_C;     /* set C if result overflowed              */
	CHK_ADDB_V;     /* set V if result has incorrect sign      */
	CHK_ADCB_H;     /* set H if lower nibble overflowed        */
	return result;
}

/******************************************
 add with carry word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::ADCW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest + value + GET_C;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_ADCX_C;     /* set C if result overflowed              */
	CHK_ADDW_V;     /* set V if result has incorrect sign      */
	return result;
}

/******************************************
 subtract byte
 flags:  CZSVDH
 ******************************************/
uint8_t z8002_device::SUBB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	SET_DA;         /* set DA (decimal adjust) flag for subb   */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;     /* set V if result has incorrect sign      */
	CHK_SUBB_H;     /* set H if lower nibble underflowed       */
	return result;
}

/******************************************
 subtract word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SUBW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;     /* set V if result has incorrect sign      */
	return result;
}

/******************************************
 subtract long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::SUBL(uint32_t dest, uint32_t value)
{
	uint32_t result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBL_V;     /* set V if result has incorrect sign      */
	return result;
}

/******************************************
 subtract with carry byte
 flags:  CZSVDH
 ******************************************/
uint8_t z8002_device::SBCB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value - GET_C;
	CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
	SET_DA;         /* set DA (decimal adjust) flag for sbcb   */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SBCX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;     /* set V if result has incorrect sign      */
	CHK_SBCB_H;     /* set H if lower nibble underflowed       */
	return result;
}

/******************************************
 subtract with carry word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SBCW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value - GET_C;
	CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SBCX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;     /* set V if result has incorrect sign      */
	return result;
}

/******************************************
 logical or byte
 flags:  -ZSP--
 ******************************************/
uint8_t z8002_device::ORB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest | value;
	CLR_ZSP;        /* first clear Z, S, P/V flags             */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical or word
 flags:  -ZS---
 ******************************************/
uint16_t z8002_device::ORW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest | value;
	CLR_ZS;         /* first clear Z, and S flags              */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 logical and byte
 flags:  -ZSP--
 ******************************************/
uint8_t z8002_device::ANDB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest & value;
	CLR_ZSP;        /* first clear Z,S and P/V flags           */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical and word
 flags:  -ZS---
 ******************************************/
uint16_t z8002_device::ANDW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest & value;
	CLR_ZS;         /* first clear Z and S flags               */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 logical exclusive or byte
 flags:  -ZSP--
 ******************************************/
uint8_t z8002_device::XORB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest ^ value;
	CLR_ZSP;        /* first clear Z, S and P/V flags          */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical exclusive or word
 flags:  -ZS---
 ******************************************/
uint16_t z8002_device::XORW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest ^ value;
	CLR_ZS;         /* first clear Z and S flags               */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}


/******************************************
 compare byte
 flags:  CZSV--
 ******************************************/
void z8002_device::CPB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S and P/V flags       */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;
}

/******************************************
 compare word
 flags:  CZSV--
 ******************************************/
void z8002_device::CPW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value;
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;
}

/******************************************
 compare long
 flags:  CZSV--
 ******************************************/
void z8002_device::CPL(uint32_t dest, uint32_t value)
{
	uint32_t result = dest - value;
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBL_V;
}

/******************************************
 complement byte
 flags: -ZSP--
 ******************************************/
uint8_t z8002_device::COMB(uint8_t dest)
{
	uint8_t result = ~dest;
	CLR_ZSP;
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 complement word
 flags: -ZS---
 ******************************************/
uint16_t z8002_device::COMW(uint16_t dest)
{
	uint16_t result = ~dest;
	CLR_ZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 negate byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::NEGB(uint8_t dest)
{
	uint8_t result = (uint8_t) -dest;
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (result > 0) SET_C;
	if (result == S08) SET_V;
	return result;
}

/******************************************
 negate word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::NEGW(uint16_t dest)
{
	uint16_t result = (uint16_t) -dest;
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (result > 0) SET_C;
	if (result == S16) SET_V;
	return result;
}

/******************************************
 test byte
 flags:  -ZSP--
 ******************************************/
void z8002_device::TESTB(uint8_t result)
{
	CLR_ZSP;
	CHK_XXXB_ZSP;   /* set Z and S flags for result byte       */
}

/******************************************
 test word
 flags:  -ZS---
 ******************************************/
void z8002_device::TESTW(uint16_t dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S16) SET_S;
}

/******************************************
 test long
 flags:  -ZS---
 ******************************************/
void z8002_device::TESTL(uint32_t dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S32) SET_S;
}

/******************************************
 increment byte
 flags: -ZSV--
 ******************************************/
uint8_t z8002_device::INCB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest + value;
	CLR_ZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDB_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 increment word
 flags: -ZSV--
 ******************************************/
uint16_t z8002_device::INCW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest + value;
	CLR_ZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDW_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 decrement byte
 flags: -ZSV--
 ******************************************/
uint8_t z8002_device::DECB(uint8_t dest, uint8_t value)
{
	uint8_t result = dest - value;
	CLR_ZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBB_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 decrement word
 flags: -ZSV--
 ******************************************/
uint16_t z8002_device::DECW(uint16_t dest, uint16_t value)
{
	uint16_t result = dest - value;
	CLR_ZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBW_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 multiply words
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::MULTW(uint16_t dest, uint16_t value)
{
	uint32_t result = (int32_t)(int16_t)dest * (int16_t)value;
	CLR_CZSV;
	CHK_XXXL_ZS;
	if(!value)
	{
		/* multiplication with zero is faster */
		m_icount += (70-18);
	}
	if((int32_t)result < -0x7fff || (int32_t)result >= 0x7fff) SET_C;
	return result;
}

/******************************************
 multiply longs
 flags:  CZSV--
 ******************************************/
uint64_t z8002_device::MULTL(uint32_t dest, uint32_t value)
{
	uint64_t result = (int64_t)(int32_t)dest * (int32_t)value;
	if(!value)
	{
		/* multiplication with zero is faster */
		m_icount += (282 - 30);
	}
	else
	{
		int n;
		for(n = 0; n < 32; n++)
			if(dest & (1L << n)) m_icount -= 7;
	}
	CLR_CZSV;
	CHK_XXXQ_ZS;
	if((int64_t)result < -0x7fffffffL || (int64_t)result >= 0x7fffffffL) SET_C;
	return result;
}

/******************************************
 divide long by word
 flags: CZSV--
 ******************************************/
uint32_t z8002_device::DIVW(uint32_t dest, uint16_t value)
{
	uint32_t result = dest;
	uint16_t remainder = 0;
	CLR_CZSV;
	if (value)
	{
		uint16_t qsign = ((dest >> 16) ^ value) & S16;
		uint16_t rsign = (dest >> 16) & S16;
		if ((int32_t)dest < 0) dest = -dest;
		if ((int16_t)value < 0) value = -value;
		result = dest / value;
		remainder = dest % value;
		if (qsign) result = -result;
		if (rsign) remainder = -remainder;
		if ((int32_t)result < -0x8000 || (int32_t)result > 0x7fff)
		{
			int32_t temp = (int32_t)result >> 1;
			SET_V;
			if (temp >= -0x8000 && temp <= 0x7fff)
			{
				result = (temp < 0) ? -1 : 0;
				CHK_XXXW_ZS;
				SET_C;
			}
		}
		else
		{
			CHK_XXXW_ZS;
		}
		result = ((uint32_t)remainder << 16) | (result & 0xffff);
	}
	else
	{
		SET_Z;
		SET_V;
	}
	return result;
}

/******************************************
 divide quad word by long
 flags: CZSV--
 ******************************************/
uint64_t z8002_device::DIVL(uint64_t dest, uint32_t value)
{
	uint64_t result = dest;
	uint32_t remainder = 0;
	CLR_CZSV;
	if (value)
	{
		uint32_t qsign = ((dest >> 32) ^ value) & S32;
		uint32_t rsign = (dest >> 32) & S32;
		if ((int64_t)dest < 0) dest = -dest;
		if ((int32_t)value < 0) value = -value;
		result = dest / value;
		remainder = dest % value;
		if (qsign) result = -result;
		if (rsign) remainder = -remainder;
		if ((int64_t)result < -0x80000000LL || (int64_t)result > 0x7fffffff)
		{
			int64_t temp = (int64_t)result >> 1;
			SET_V;
			if (temp >= -0x80000000LL && temp <= 0x7fffffff)
			{
				result = (temp < 0) ? -1 : 0;
				CHK_XXXL_ZS;
				SET_C;
			}
		}
		else
		{
			CHK_XXXL_ZS;
		}
		result = ((uint64_t)remainder << 32) | (result & 0xffffffff);
	}
	else
	{
		SET_Z;
		SET_V;
	}
	return result;
}

/******************************************
 rotate left byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::RLB(uint8_t dest, uint8_t twice)
{
	uint8_t result = (dest << 1) | (dest >> 7);
	CLR_CZSV;
	if (twice) result = (result << 1) | (result >> 7);
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (result & 0x01) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

/******************************************
 rotate left word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::RLW(uint16_t dest, uint8_t twice)
{
	uint16_t result = (dest << 1) | (dest >> 15);
	CLR_CZSV;
	if (twice) result = (result << 1) | (result >> 15);
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (result & 0x0001) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

/******************************************
 rotate left through carry byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::RLCB(uint8_t dest, uint8_t twice)
{
	uint8_t c = dest & S08;
	uint8_t result = (dest << 1) | GET_C;
	CLR_CZSV;
	if (twice) {
		uint8_t c1 = c >> 7;
		c = result & S08;
		result = (result << 1) | c1;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

/******************************************
 rotate left through carry word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::RLCW(uint16_t dest, uint8_t twice)
{
	uint16_t c = dest & S16;
	uint16_t result = (dest << 1) | GET_C;
	CLR_CZSV;
	if (twice) {
		uint16_t c1 = c >> 15;
		c = result & S16;
		result = (result << 1) | c1;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

/******************************************
 rotate right byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::RRB(uint8_t dest, uint8_t twice)
{
	uint8_t result = (dest >> 1) | (dest << 7);
	CLR_CZSV;
	if (twice) result = (result >> 1) | (result << 7);
	if (!result) SET_Z; else if (result & S08) SET_SC;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

/******************************************
 rotate right word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::RRW(uint16_t dest, uint8_t twice)
{
	uint16_t result = (dest >> 1) | (dest << 15);
	CLR_CZSV;
	if (twice) result = (result >> 1) | (result << 15);
	if (!result) SET_Z; else if (result & S16) SET_SC;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

/******************************************
 rotate right through carry byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::RRCB(uint8_t dest, uint8_t twice)
{
	uint8_t c = dest & 1;
	uint8_t result = (dest >> 1) | (GET_C << 7);
	CLR_CZSV;
	if (twice) {
		uint8_t c1 = c << 7;
		c = result & 1;
		result = (result >> 1) | c1;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

/******************************************
 rotate right through carry word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::RRCW(uint16_t dest, uint8_t twice)
{
	uint16_t c = dest & 1;
	uint16_t result = (dest >> 1) | (GET_C << 15);
	CLR_CZSV;
	if (twice) {
		uint16_t c1 = c << 15;
		c = result & 1;
		result = (result >> 1) | c1;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

/******************************************
 shift dynamic arithmetic byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::SDAB(uint8_t dest, int8_t count)
{
	int8_t result = (int8_t) dest;
	uint8_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S08;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x01;
		result >>= 1;
		count++;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return (uint8_t)result;
}

/******************************************
 shift dynamic arithmetic word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SDAW(uint16_t dest, int8_t count)
{
	int16_t result = (int16_t) dest;
	uint16_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S16;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x0001;
		result >>= 1;
		count++;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return (uint16_t)result;
}

/******************************************
 shift dynamic arithmetic long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::SDAL(uint32_t dest, int8_t count)
{
	int32_t result = (int32_t) dest;
	uint32_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S32;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x00000001;
		result >>= 1;
		count++;
	}
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	if ((result ^ dest) & S32) SET_V;
	return (uint32_t) result;
}

/******************************************
 shift dynamic logic byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::SDLB(uint8_t dest, int8_t count)
{
	uint8_t result = dest;
	uint8_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S08;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x01;
		result >>= 1;
		count++;
	}
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

/******************************************
 shift dynamic logic word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SDLW(uint16_t dest, int8_t count)
{
	uint16_t result = dest;
	uint16_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S16;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x0001;
		result >>= 1;
		count++;
	}
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

/******************************************
 shift dynamic logic long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::SDLL(uint32_t dest, int8_t count)
{
	uint32_t result = dest;
	uint32_t c = 0;
	CLR_CZSV;
	while (count > 0) {
		c = result & S32;
		result <<= 1;
		count--;
	}
	while (count < 0) {
		c = result & 0x00000001;
		result >>= 1;
		count++;
	}
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	if ((result ^ dest) & S32) SET_V;
	return result;
}

/******************************************
 shift left arithmetic byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::SLAB(uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest << (count - 1)) & S08 : 0;
	uint8_t result = (uint8_t)((int8_t)dest << count);
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	if ((result ^ dest) & S08) SET_V;
	return result;
}

/******************************************
 shift left arithmetic word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SLAW(uint16_t dest, uint8_t count)
{
	uint16_t c = (count) ? (dest << (count - 1)) & S16 : 0;
	uint16_t result = (uint16_t)((int16_t)dest << count);
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	if ((result ^ dest) & S16) SET_V;
	return result;
}

/******************************************
 shift left arithmetic long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::SLAL(uint32_t dest, uint8_t count)
{
	uint32_t c = (count) ? (dest << (count - 1)) & S32 : 0;
	uint32_t result = (uint32_t)((int32_t)dest << count);
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	if ((result ^ dest) & S32) SET_V;
	return result;
}

/******************************************
 shift left logic byte
 flags:  CZS---
 ******************************************/
uint8_t z8002_device::SLLB(uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest << (count - 1)) & S08 : 0;
	uint8_t result = dest << count;
	CLR_CZS;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift left logic word
 flags:  CZS---
 ******************************************/
uint16_t z8002_device::SLLW(uint16_t dest, uint8_t count)
{
	uint16_t c = (count) ? (dest << (count - 1)) & S16 : 0;
	uint16_t result = dest << count;
	CLR_CZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift left logic long
 flags:  CZS---
 ******************************************/
uint32_t z8002_device::SLLL(uint32_t dest, uint8_t count)
{
	uint32_t c = (count) ? (dest << (count - 1)) & S32 : 0;
	uint32_t result = dest << count;
	CLR_CZS;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right arithmetic byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::SRAB(uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? ((int8_t)dest >> (count - 1)) & 1 : 0;
	uint8_t result = (uint8_t)((int8_t)dest >> count);
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right arithmetic word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SRAW(uint16_t dest, uint8_t count)
{
	uint8_t c = (count) ? ((int16_t)dest >> (count - 1)) & 1 : 0;
	uint16_t result = (uint16_t)((int16_t)dest >> count);
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right arithmetic long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::SRAL(uint32_t dest, uint8_t count)
{
	uint8_t c = (count) ? ((int32_t)dest >> (count - 1)) & 1 : 0;
	uint32_t result = (uint32_t)((int32_t)dest >> count);
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right logic byte
 flags:  CZSV--
 ******************************************/
uint8_t z8002_device::SRLB(uint8_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest >> (count - 1)) & 1 : 0;
	uint8_t result = dest >> count;
	CLR_CZS;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right logic word
 flags:  CZSV--
 ******************************************/
uint16_t z8002_device::SRLW(uint16_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest >> (count - 1)) & 1 : 0;
	uint16_t result = dest >> count;
	CLR_CZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right logic long
 flags:  CZSV--
 ******************************************/
uint32_t z8002_device::SRLL(uint32_t dest, uint8_t count)
{
	uint8_t c = (count) ? (dest >> (count - 1)) & 1 : 0;
	uint32_t result = dest >> count;
	CLR_CZS;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

/******************************************
 invalid
 flags:  ------
 ******************************************/
void z8002_device::zinvalid()
{
	logerror("Z8000 invalid opcode %04x: %04x\n", m_pc, m_op[0]);
}

/******************************************
 addb    rbd,imm8
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z00_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ADDB(RB(dst), imm8);
}

/******************************************
 addb    rbd,@rs
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z00_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADDB(RB(dst), RDIR_B(src));
}

/******************************************
 add     rd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z01_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ADDW(RW(dst), imm16);
}

/******************************************
 add     rd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z01_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADDW(RW(dst), RDIR_W(src));
}

/******************************************
 subb    rbd,imm8
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z02_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = SUBB(RB(dst), imm8);
}

/******************************************
 subb    rbd,@rs
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z02_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SUBB(RB(dst), RDIR_B(src)); /* EHC */
}

/******************************************
 sub     rd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z03_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = SUBW(RW(dst), imm16);
}

/******************************************
 sub     rd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z03_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SUBW(RW(dst), RDIR_W(src));
}

/******************************************
 orb     rbd,imm8
 flags:  CZSP--
 ******************************************/
void z8002_device::Z04_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ORB(RB(dst), imm8);
}

/******************************************
 orb     rbd,@rs
 flags:  CZSP--
 ******************************************/
void z8002_device::Z04_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ORB(RB(dst), RDIR_B(src));
}

/******************************************
 or      rd,imm16
 flags:  CZS---
 ******************************************/
void z8002_device::Z05_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ORW(RW(dst), imm16);
}

/******************************************
 or      rd,@rs
 flags:  CZS---
 ******************************************/
void z8002_device::Z05_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ORW(RW(dst), RDIR_W(src));
}

/******************************************
 andb    rbd,imm8
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z06_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ANDB(RB(dst), imm8);
}

/******************************************
 andb    rbd,@rs
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z06_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ANDB(RB(dst), RDIR_B(src));
}

/******************************************
 and     rd,imm16
 flags:  -ZS---
 ******************************************/
void z8002_device::Z07_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ANDW(RW(dst), imm16);
}

/******************************************
 and     rd,@rs
 flags:  -ZS---
 ******************************************/
void z8002_device::Z07_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ANDW(RW(dst), RDIR_W(src));
}

/******************************************
 xorb    rbd,imm8
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z08_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = XORB(RB(dst), imm8);
}

/******************************************
 xorb    rbd,@rs
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z08_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = XORB(RB(dst), RDIR_B(src));
}

/******************************************
 xor     rd,imm16
 flags:  -ZS---
 ******************************************/
void z8002_device::Z09_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = XORW(RW(dst), imm16);
}

/******************************************
 xor     rd,@rs
 flags:  -ZS---
 ******************************************/
void z8002_device::Z09_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = XORW(RW(dst), RDIR_W(src));
}

/******************************************
 cpb     rbd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0A_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	CPB(RB(dst), imm8);
}

/******************************************
 cpb     rbd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0A_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB(RB(dst), RDIR_B(src));
}

/******************************************
 cp      rd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0B_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	CPW(RW(dst), imm16);
}

/******************************************
 cp      rd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0B_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW(RW(dst), RDIR_W(src));
}

/******************************************
 comb    @rd
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z0C_ddN0_0000()
{
	GET_DST(OP0,NIB3);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, COMB(RDMEM_B(space, addr)));
}

/******************************************
 cpb     @rd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0C_ddN0_0001_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	CPB(RDIR_B(dst), imm8); // @@@done
}

/******************************************
 negb    @rd
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0C_ddN0_0010()
{
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, NEGB(RDMEM_B(space, addr)));
}

/******************************************
 testb   @rd
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z0C_ddN0_0100()
{
	GET_DST(OP0,NIB2);
	TESTB(RDIR_B(dst));
}

/******************************************
 ldb     @rd,imm8
 flags:  ------
 ******************************************/
void z8002_device::Z0C_ddN0_0101_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	WRIR_B(dst, imm8);
}

/******************************************
 tsetb   @rd
 flags:  --S---
 ******************************************/
void z8002_device::Z0C_ddN0_0110()
{
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	if (RDMEM_B(space, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(space, addr, 0xff);
}

/******************************************
 clrb    @rd
 flags:  ------
 ******************************************/
void z8002_device::Z0C_ddN0_1000()
{
	GET_DST(OP0,NIB2);
	WRIR_B(dst, 0);
}

/******************************************
 com     @rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z0D_ddN0_0000()
{
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, COMW(RDMEM_W(space, addr)));
}

/******************************************
 cp      @rd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0D_ddN0_0001_imm16()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	CPW(RDIR_W(dst), imm16);
}

/******************************************
 neg     @rd
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0D_ddN0_0010()
{
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, NEGW(RDMEM_W(space, addr)));
}

/******************************************
 test    @rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z0D_ddN0_0100()
{
	GET_DST(OP0,NIB2);
	TESTW(RDIR_W(dst));
}

/******************************************
 ld      @rd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z0D_ddN0_0101_imm16()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	WRIR_W(dst, imm16);
}

/******************************************
 tset    @rd
 flags:  --S---
 ******************************************/
void z8002_device::Z0D_ddN0_0110()
{
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	if (RDMEM_W(space, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(space, addr, 0xffff);
}

/******************************************
 clr     @rd
 flags:  ------
 ******************************************/
void z8002_device::Z0D_ddN0_1000()
{
	GET_DST(OP0,NIB2);
	WRIR_W(dst, 0);
}

/******************************************
 push    @rd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z0D_ddN0_1001_imm16()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	PUSHW(dst, imm16);
}

/******************************************
 ext0e   imm8
 flags:  ------
 ******************************************/
void z8002_device::Z0E_imm8()
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG("Z8K %04x: ext0e  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ext0f   imm8
 flags:  ------
 ******************************************/
void z8002_device::Z0F_imm8()
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG("Z8K %04x: ext0f  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 cpl     rrd,imm32
 flags:  CZSV--
 ******************************************/
void z8002_device::Z10_0000_dddd_imm32()
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	CPL(RL(dst), imm32);
}

/******************************************
 cpl     rrd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z10_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPL(RL(dst), RDIR_L(src));
}

/******************************************
 pushl   @rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z11_ddN0_ssN0()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL(dst, RDIR_L(src));
}

/******************************************
 subl    rrd,imm32
 flags:  CZSV--
 ******************************************/
void z8002_device::Z12_0000_dddd_imm32()
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = SUBL(RL(dst), imm32);
}

/******************************************
 subl    rrd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z12_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = SUBL(RL(dst), RDIR_L(src));
}

/******************************************
 push    @rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z13_ddN0_ssN0()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(dst, RDIR_W(src));
}

/******************************************
 ldl     rrd,imm32
 flags:  ------
 ******************************************/
void z8002_device::Z14_0000_dddd_imm32()
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = imm32;
}

/******************************************
 ldl     rrd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z14_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = RDIR_L(src);
}

/******************************************
 popl    rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z15_ssN0_ddN0()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = POPL(src);
}

/******************************************
 addl    rrd,imm32
 flags:  CZSV--
 ******************************************/
void z8002_device::Z16_0000_dddd_imm32()
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = ADDL(RL(dst), imm32);
}

/******************************************
 addl    rrd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z16_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = ADDL(RL(dst), RDIR_L(src));
}

/******************************************
 pop     @rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z17_ssN0_ddN0()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	WRIR_W(dst, POPW(src));
}

/******************************************
 multl   rqd,imm32
 flags:  CZSV--
 ******************************************/
void z8002_device::Z18_00N0_dddd_imm32()
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RQ(dst) = MULTL(RQ(dst), imm32);
}

/******************************************
 multl   rqd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z18_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = MULTL(RQ(dst), RL(src)); //@@@
}

/******************************************
 mult    rrd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z19_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RL(dst) = MULTW(RL(dst), imm16);
}

/******************************************
 mult    rrd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z19_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = MULTW(RL(dst), RDIR_W(src));
}

/******************************************
 divl    rqd,imm32
 flags:  CZSV--
 ******************************************/
void z8002_device::Z1A_0000_dddd_imm32()
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RQ(dst) = DIVL(RQ(dst), imm32);
}

/******************************************
 divl    rqd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z1A_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = DIVL(RQ(dst), RDIR_L(src));
}

/******************************************
 div     rrd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z1B_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RL(dst) = DIVW(RL(dst), imm16);
}

/******************************************
 div     rrd,@rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z1B_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = DIVW(RL(dst), RDIR_W(src));
}

/******************************************
 testl   @rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z1C_ddN0_1000()
{
	GET_DST(OP0,NIB2);
	TESTL(RDIR_L(dst));
}

/******************************************
 ldm     @rd,rs,n
 flags:  ------
 ******************************************/
void z8002_device::Z1C_ddN0_1001_0000_ssss_0000_nmin1()
{
	GET_DST(OP0,NIB2);
	GET_CNT(OP1,NIB3);
	GET_SRC(OP1,NIB1);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	while (cnt-- >= 0) {
		WRMEM_W(space, addr, RW(src));
		addr = addr_add(addr, 2);
		src = (src+1) & 15;
	}
}

/******************************************
 ldm     rd,@rs,n
 flags:  ------
 ******************************************/
void z8002_device::Z1C_ssN0_0001_0000_dddd_0000_nmin1()
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB3);
	GET_DST(OP1,NIB1);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = src == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(src);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(space, addr);
		addr = addr_add(addr, 2);
		dst = (dst+1) & 15;
	}
}

/******************************************
 ldl     @rd,rrs
 flags:  ------
 ******************************************/
void z8002_device::Z1D_ddN0_ssss()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRIR_L(dst, RL(src));
}

/******************************************
 jp      cc,rd
 flags:  ------
 ******************************************/
void z8002_device::Z1E_ddN0_cccc()
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (cc) {
		case  0: if (CC0) set_pc(addr_from_reg(dst)); break;
		case  1: if (CC1) set_pc(addr_from_reg(dst)); break;
		case  2: if (CC2) set_pc(addr_from_reg(dst)); break;
		case  3: if (CC3) set_pc(addr_from_reg(dst)); break;
		case  4: if (CC4) set_pc(addr_from_reg(dst)); break;
		case  5: if (CC5) set_pc(addr_from_reg(dst)); break;
		case  6: if (CC6) set_pc(addr_from_reg(dst)); break;
		case  7: if (CC7) set_pc(addr_from_reg(dst)); break;
		case  8: if (CC8) set_pc(addr_from_reg(dst)); break;
		case  9: if (CC9) set_pc(addr_from_reg(dst)); break;
		case 10: if (CCA) set_pc(addr_from_reg(dst)); break;
		case 11: if (CCB) set_pc(addr_from_reg(dst)); break;
		case 12: if (CCC) set_pc(addr_from_reg(dst)); break;
		case 13: if (CCD) set_pc(addr_from_reg(dst)); break;
		case 14: if (CCE) set_pc(addr_from_reg(dst)); break;
		case 15: if (CCF) set_pc(addr_from_reg(dst)); break;
	}
}

/******************************************
 call    @rd
 flags:  ------
 ******************************************/
void z8002_device::Z1F_ddN0_0000()
{
	GET_DST(OP0,NIB2);
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	set_pc(addr_from_reg(dst));
}

/******************************************
 ldb     rbd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z20_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = RDIR_B(src);
}

/******************************************
 ld      rd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z21_0000_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = imm16;
}

/******************************************
 ld      rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z21_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = RDIR_W(src);
}

/******************************************
 resb    rbd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z22_0000_ssss_0000_dddd_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RB(dst) = RB(dst) & ~(1 << (RW(src) & 7));
}

/******************************************
 resb    @rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z22_ddN0_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, RDMEM_B(space, addr) & ~bit);
}

/******************************************
 res     rd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z23_0000_ssss_0000_dddd_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RW(dst) = RW(dst) & ~(1 << (RW(src) & 15));
}

/******************************************
 res     @rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z23_ddN0_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, RDMEM_W(space, addr) & ~bit);
}

/******************************************
 setb    rbd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z24_0000_ssss_0000_dddd_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RB(dst) = RB(dst) | (1 << (RW(src) & 7));
}

/******************************************
 setb    @rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z24_ddN0_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, RDMEM_B(space, addr) | bit);
}

/******************************************
 set     rd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z25_0000_ssss_0000_dddd_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RW(dst) = RW(dst) | (1 << (RW(src) & 15));
}

/******************************************
 set     @rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z25_ddN0_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, RDMEM_W(space, addr) | bit);
}

/******************************************
 bitb    rbd,rs
 flags:  -Z----
 ******************************************/
void z8002_device::Z26_0000_ssss_0000_dddd_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (RB(dst) & (1 << (RW(src) & 7))) CLR_Z; else SET_Z;
}

/******************************************
 bitb    @rd,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z26_ddN0_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDIR_B(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     rd,rs
 flags:  -Z----
 ******************************************/
void z8002_device::Z27_0000_ssss_0000_dddd_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (RW(dst) & (1 << (RW(src) & 15))) CLR_Z; else SET_Z;
}

/******************************************
 bit     @rd,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z27_ddN0_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDIR_W(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z28_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, INCB(RDMEM_B(space, addr), i4p1));
}

/******************************************
 inc     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z29_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, INCW(RDMEM_W(space, addr), i4p1));
}

/******************************************
 decb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z2A_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_B(space, addr, DECB(RDMEM_B(space, addr), i4p1));
}

/******************************************
 dec     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z2B_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = dst == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(dst);
	WRMEM_W(space, addr, DECW(RDMEM_W(space, addr), i4p1));
}

/******************************************
 exb     rbd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z2C_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = src == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(src);
	uint8_t tmp = RDMEM_B(space, addr);
	WRMEM_B(space, addr, RB(dst));
	RB(dst) = tmp;
}

/******************************************
 ex      rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z2D_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = src == SP ? m_stack : m_data;
	uint32_t addr = addr_from_reg(src);
	uint16_t tmp = RDMEM_W(space, addr);
	WRMEM_W(space, addr, RW(dst));
	RW(dst) = tmp;
}

/******************************************
 ldb     @rd,rbs
 flags:  ------
 ******************************************/
void z8002_device::Z2E_ddN0_ssss()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRIR_B(dst, RB(src));
}

/******************************************
 ld      @rd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z2F_ddN0_ssss()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRIR_W(dst, RW(src));
}

/******************************************
 ldrb    rbd,dsp16
 flags:  ------
 ******************************************/
void z8002_device::Z30_0000_dddd_dsp16()
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RB(dst) = RDMEM_B(m_program, dsp16);
}

/******************************************
 ldb     rbd,rs(idx16)
 flags:  ------
 ******************************************/
void z8002_device::Z30_ssN0_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	RB(dst) = RDBX_B(src, idx16);
}

/******************************************
 ldr     rd,dsp16
 flags:  ------
 ******************************************/
void z8002_device::Z31_0000_dddd_dsp16()
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RW(dst) = RDMEM_W(m_program, dsp16);
}

/******************************************
 ld      rd,rs(idx16)
 flags:  ------
 ******************************************/
void z8002_device::Z31_ssN0_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	RW(dst) = RDBX_W(src, idx16);
}

/******************************************
 ldrb    dsp16,rbs
 flags:  ------
 ******************************************/
void z8002_device::Z32_0000_ssss_dsp16()
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_B(m_program, dsp16, RB(src));
}

/******************************************
 ldb     rd(idx16),rbs
 flags:  ------
 ******************************************/
void z8002_device::Z32_ddN0_ssss_imm16()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	WRBX_B(dst, idx16, RB(src));
}

/******************************************
 ldr     dsp16,rs
 flags:  ------
 ******************************************/
void z8002_device::Z33_0000_ssss_dsp16()
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_W(m_program, dsp16, RW(src));
}

/******************************************
 ld      rd(idx16),rs
 flags:  ------
 ******************************************/
void z8002_device::Z33_ddN0_ssss_imm16()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	WRBX_W(dst, idx16, RW(src));
}

/******************************************
 ldar    prd,dsp16
 flags:  ------
 ******************************************/
void z8002_device::Z34_0000_dddd_dsp16()
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	addr_to_reg(dst, dsp16);
}

/******************************************
 lda     prd,rs(idx16)
 flags:  ------
 ******************************************/
void z8002_device::Z34_ssN0_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	if (get_segmented_mode()) {
		RL(dst) = RL(src);
	}
	else {
		RW(dst) = RW(src);
	}
	add_to_addr_reg(dst, idx16);
}

/******************************************
 ldrl    rrd,dsp16
 flags:  ------
 ******************************************/
void z8002_device::Z35_0000_dddd_dsp16()
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RL(dst) = RDMEM_L(m_program, dsp16);
}

/******************************************
 ldl     rrd,rs(idx16)
 flags:  ------
 ******************************************/
void z8002_device::Z35_ssN0_dddd_imm16()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	RL(dst) = RDBX_L(src, idx16);
}

/******************************************
 bpt
 flags:  ------
 ******************************************/
void z8002_device::Z36_0000_0000()
{
	/* execute break point trap m_irq_req */
	m_irq_req |= Z8000_TRAP;
}

/******************************************
 rsvd36
 flags:  ------
 ******************************************/
void z8002_device::Z36_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvd36 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ldrl    dsp16,rrs
 flags:  ------
 ******************************************/
void z8002_device::Z37_0000_ssss_dsp16()
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_L(m_program,  dsp16, RL(src));
}

/******************************************
 ldl     rd(idx16),rrs
 flags:  ------
 ******************************************/
void z8002_device::Z37_ddN0_ssss_imm16()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	WRBX_L(dst, idx16, RL(src));
}

/******************************************
 rsvd38
 flags:  ------
 ******************************************/
void z8002_device::Z38_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvd38 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ldps    @rs
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z39_ssN0_0000()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	uint16_t fcw;
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &space = src == SP ? m_stack : m_data;
	if (get_segmented_mode()) {
		uint32_t addr = addr_from_reg(src);
		fcw = RDMEM_W(space, addr + 2);
		set_pc(segmented_addr(RDMEM_L(space, addr + 4)));
	}
	else {
		fcw = RDMEM_W(space, RW(src));
		set_pc(RDMEM_W(space, (uint16_t)(RW(src) + 2)));
	}
	if ((fcw ^ m_fcw) & F_SEG) printf("ldps 1 (0x%05x): changing from %ssegmented mode to %ssegmented mode\n", m_pc, (m_fcw & F_SEG) ? "non-" : "", (fcw & F_SEG) ? "" : "non-");
	CHANGE_FCW(fcw); /* check for user/system mode change */
}

/******************************************
 inib(r) @rd,@rs,ra
 flags:  ---V--
 ******************************************/
void z8002_device::Z3A_ssss_0000_0000_aaaa_dddd_x000()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 0, RW(src)));
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 sinib   @rd,@rs,ra
 sinibr  @rd,@rs,ra
 flags:  ------
 ******************************************/
void z8002_device::Z3A_ssss_0001_0000_aaaa_dddd_x000()
{//@@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 1, RW(src)));
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 outib   @rd,@rs,ra
 outibr  @rd,@rs,ra
 flags:  ---V--
 ******************************************/
void z8002_device::Z3A_ssss_0010_0000_aaaa_dddd_x000()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 0, RW(dst), RDIR_B(src));
	add_to_addr_reg(src, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 soutib  @rd,@rs,ra
 soutibr @rd,@rs,ra
 flags:  ------
 ******************************************/
void z8002_device::Z3A_ssss_0011_0000_aaaa_dddd_x000()
{//@@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 1, RW(dst), RDIR_W(src));
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 inb     rbd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z3A_dddd_0100_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RB(dst) = RDPORT_B( 0, imm16);
}

/******************************************
 sinb    rbd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z3A_dddd_0101_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RB(dst) = RDPORT_B( 1, imm16);
}

/******************************************
 outb    imm16,rbs
 flags:  ---V--
 ******************************************/
void z8002_device::Z3A_ssss_0110_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_B( 0, imm16, RB(src));
}

/******************************************
 soutb   imm16,rbs
 flags:  ------
 ******************************************/
void z8002_device::Z3A_ssss_0111_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_B( 1, imm16, RB(src));
}

/******************************************
 indb    @rd,@rs,rba
 indbr   @rd,@rs,rba
 flags:  ---V--
 ******************************************/
void z8002_device::Z3A_ssss_1000_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 0, RW(src)));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 sindb   @rd,@rs,rba
 sindbr  @rd,@rs,rba
 flags:  ------
 ******************************************/
void z8002_device::Z3A_ssss_1001_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDPORT_B( 1, RW(src)));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 outdb   @rd,@rs,rba
 outdbr  @rd,@rs,rba
 flags:  ---V--
 ******************************************/
void z8002_device::Z3A_ssss_1010_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 0, RW(dst), RDIR_B(src));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 soutdb  @rd,@rs,rba
 soutdbr @rd,@rs,rba
 flags:  ------
 ******************************************/
void z8002_device::Z3A_ssss_1011_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 1, RW(dst), RDIR_B(src));
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 ini     @rd,@rs,ra
 inir    @rd,@rs,ra
 flags:  ---V--
 ******************************************/
void z8002_device::Z3B_ssss_0000_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 0, RW(src)));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 sini    @rd,@rs,ra
 sinir   @rd,@rs,ra
 flags:  ------
 ******************************************/
void z8002_device::Z3B_ssss_0001_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 1, RW(src)));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 outi    @rd,@rs,ra
 outir   @rd,@rs,ra
 flags:  ---V--
 ******************************************/
void z8002_device::Z3B_ssss_0010_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 0, RW(dst), RDIR_W(src));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 souti   @rd,@rs,ra
 soutir  @rd,@rs,ra
 flags:  ------
 ******************************************/
void z8002_device::Z3B_ssss_0011_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 1, RW(dst), RDIR_W(src));
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 in      rd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z3B_dddd_0100_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RW(dst) = RDPORT_W( 0, imm16);
}

/******************************************
 sin     rd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z3B_dddd_0101_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	RW(dst) = RDPORT_W( 1, imm16);
}

/******************************************
 out     imm16,rs
 flags:  ---V--
 ******************************************/
void z8002_device::Z3B_ssss_0110_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_W( 0, imm16, RW(src));
}

/******************************************
 sout    imm16,rbs
 flags:  ------
 ******************************************/
void z8002_device::Z3B_ssss_0111_imm16()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	WRPORT_W( 1, imm16, RW(src));
}

/******************************************
 ind     @rd,@rs,ra
 indr    @rd,@rs,ra
 flags:  ---V--
 ******************************************/
void z8002_device::Z3B_ssss_1000_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 0, RW(src)));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 sind    @rd,@rs,ra
 sindr   @rd,@rs,ra
 flags:  ------
 ******************************************/
void z8002_device::Z3B_ssss_1001_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDPORT_W( 1, RW(src)));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 outd    @rd,@rs,ra
 outdr   @rd,@rs,ra
 flags:  ---V--
 ******************************************/
void z8002_device::Z3B_ssss_1010_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 0, RW(dst), RDIR_W(src));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 soutd   @rd,@rs,ra
 soutdr  @rd,@rs,ra
 flags:  ------
 ******************************************/
void z8002_device::Z3B_ssss_1011_0000_aaaa_dddd_x000()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 1, RW(dst), RDIR_W(src));
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 inb     rbd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z3C_ssss_dddd()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	RB(dst) = RDPORT_B( 0, RW(src));
}

/******************************************
 in      rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z3D_ssss_dddd()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	RW(dst) = RDPORT_W( 0, RW(src));
}

/******************************************
 outb    @rd,rbs
 flags:  ---V--
 ******************************************/
void z8002_device::Z3E_dddd_ssss()
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_B( 0, RW(dst), RB(src));
}

/******************************************
 out     @rd,rs
 flags:  ---V--
 ******************************************/
void z8002_device::Z3F_dddd_ssss()
{
	CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_W( 0, RW(dst), RW(src));
}

/******************************************
 addb    rbd,addr
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z40_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ADDB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 addb    rbd,addr(rs)
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z40_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = ADDB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 add     rd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z41_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ADDW(RW(dst), RDMEM_W(m_data, addr)); /* EHC */
}

/******************************************
 add     rd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z41_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = ADDW(RW(dst), RDMEM_W(m_data, addr));    /* ASG */
}

/******************************************
 subb    rbd,addr
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z42_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = SUBB(RB(dst), RDMEM_B(m_data, addr)); /* EHC */
}

/******************************************
 subb    rbd,addr(rs)
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z42_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = SUBB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 sub     rd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z43_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = SUBW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 sub     rd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z43_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = SUBW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 orb     rbd,addr
 flags:  CZSP--
 ******************************************/
void z8002_device::Z44_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ORB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 orb     rbd,addr(rs)
 flags:  CZSP--
 ******************************************/
void z8002_device::Z44_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = ORB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 or      rd,addr
 flags:  CZS---
 ******************************************/
void z8002_device::Z45_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ORW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 or      rd,addr(rs)
 flags:  CZS---
 ******************************************/
void z8002_device::Z45_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = ORW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 andb    rbd,addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z46_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ANDB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 andb    rbd,addr(rs)
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z46_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = ANDB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 and     rd,addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z47_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ANDW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 and     rd,addr(rs)
 flags:  -ZS---
 ******************************************/
void z8002_device::Z47_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = ANDW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 xorb    rbd,addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z48_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = XORB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 xorb    rbd,addr(rs)
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z48_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = XORB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 xor     rd,addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z49_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = XORW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 xor     rd,addr(rs)
 flags:  -ZS---
 ******************************************/
void z8002_device::Z49_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = XORW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 cpb     rbd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4A_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 cpb     rbd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4A_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	CPB(RB(dst), RDMEM_B(m_data, addr));
}

/******************************************
 cp      rd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4B_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 cp      rd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4B_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	CPW(RW(dst), RDMEM_W(m_data, addr));
}

/******************************************
 comb    addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z4C_0000_0000_addr()
{
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, COMB(RDMEM_W(m_data, addr)));
}

/******************************************
 cpb     addr,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4C_0000_0001_addr_imm8()
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	CPB(RDMEM_B(m_data, addr), imm8);
}

/******************************************
 negb    addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4C_0000_0010_addr()
{
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, NEGB(RDMEM_B(m_data, addr)));
}

/******************************************
 testb   addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z4C_0000_0100_addr()
{
	GET_ADDR(OP1);
	TESTB(RDMEM_B(m_data, addr));
}

/******************************************
 ldb     addr,imm8
 flags:  ------
 ******************************************/
void z8002_device::Z4C_0000_0101_addr_imm8()
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	WRMEM_B(m_data,  addr, imm8);
}

/******************************************
 tsetb   addr
 flags:  --S---
 ******************************************/
void z8002_device::Z4C_0000_0110_addr()
{
	GET_ADDR(OP1);
	if (RDMEM_B(m_data, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(m_data, addr, 0xff);
}

/******************************************
 clrb    addr
 flags:  ------
 ******************************************/
void z8002_device::Z4C_0000_1000_addr()
{
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, 0);
}

/******************************************
 comb    addr(rd)
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z4C_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data,  addr, COMB(RDMEM_B(m_data, addr)));
}

/******************************************
 cpb     addr(rd),imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4C_ddN0_0001_addr_imm8()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr = addr_add(addr, RW(dst));
	CPB(RDMEM_B(m_data, addr), imm8);
}

/******************************************
 negb    addr(rd)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4C_ddN0_0010_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, NEGB(RDMEM_B(m_data, addr)));
}

/******************************************
 testb   addr(rd)
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z4C_ddN0_0100_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	TESTB(RDMEM_B(m_data, addr));
}

/******************************************
 ldb     addr(rd),imm8
 flags:  ------
 ******************************************/
void z8002_device::Z4C_ddN0_0101_addr_imm8()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, imm8);
}

/******************************************
 tsetb   addr(rd)
 flags:  --S---
 ******************************************/
void z8002_device::Z4C_ddN0_0110_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_B(m_data, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(m_data, addr, 0xff);
}

/******************************************
 clrb    addr(rd)
 flags:  ------
 ******************************************/
void z8002_device::Z4C_ddN0_1000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, 0);
}

/******************************************
 com     addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z4D_0000_0000_addr()
{
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, COMW(RDMEM_W(m_data, addr)));
}

/******************************************
 cp      addr,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4D_0000_0001_addr_imm16()
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	CPW(RDMEM_W(m_data, addr), imm16);
}

/******************************************
 neg     addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4D_0000_0010_addr()
{
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, NEGW(RDMEM_W(m_data, addr)));
}

/******************************************
 test    addr
 flags:  ------
 ******************************************/
void z8002_device::Z4D_0000_0100_addr()
{
	GET_ADDR(OP1);
	TESTW(RDMEM_W(m_data, addr));
}

/******************************************
 ld      addr,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z4D_0000_0101_addr_imm16()
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	WRMEM_W(m_data,  addr, imm16);
}

/******************************************
 tset    addr
 flags:  --S---
 ******************************************/
void z8002_device::Z4D_0000_0110_addr()
{
	GET_ADDR(OP1);
	if (RDMEM_W(m_data, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(m_data, addr, 0xffff);
}

/******************************************
 clr     addr
 flags:  ------
 ******************************************/
void z8002_device::Z4D_0000_1000_addr()
{
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, 0);
}

/******************************************
 com     addr(rd)
 flags:  -ZS---
 ******************************************/
void z8002_device::Z4D_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, COMW(RDMEM_W(m_data, addr)));
}

/******************************************
 cp      addr(rd),imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4D_ddN0_0001_addr_imm16()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr = addr_add(addr, RW(dst));
	CPW(RDMEM_W(m_data, addr), imm16);
}

/******************************************
 neg     addr(rd)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4D_ddN0_0010_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, NEGW(RDMEM_W(m_data, addr)));
}

/******************************************
 test    addr(rd)
 flags:  ------
 ******************************************/
void z8002_device::Z4D_ddN0_0100_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	TESTW(RDMEM_W(m_data, addr));
}

/******************************************
 ld      addr(rd),imm16
 flags:  ------
 ******************************************/
void z8002_device::Z4D_ddN0_0101_addr_imm16()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, imm16);
}

/******************************************
 tset    addr(rd)
 flags:  --S---
 ******************************************/
void z8002_device::Z4D_ddN0_0110_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_W(m_data, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(m_data, addr, 0xffff);
}

/******************************************
 clr     addr(rd)
 flags:  ------
 ******************************************/
void z8002_device::Z4D_ddN0_1000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, 0);
}

/******************************************
 ldb     addr(rd),rbs
 flags:  ------
 ******************************************/
void z8002_device::Z4E_ddN0_ssN0_addr()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RB(src));
}

/******************************************
 cpl     rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z50_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPL(RL(dst), RDMEM_L(m_data, addr));
}

/******************************************
 cpl     rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z50_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	CPL(RL(dst), RDMEM_L(m_data, addr));
}

/******************************************
 pushl   @rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z51_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHL(dst, RDMEM_L(m_data, addr));
}

/******************************************
 pushl   @rd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z51_ddN0_ssN0_addr()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	PUSHL(dst, RDMEM_L(m_data, addr));
}

/******************************************
 subl    rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z52_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = SUBL(RL(dst), RDMEM_L(m_data, addr));
}

/******************************************
 subl    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z52_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = SUBL(RL(dst), RDMEM_L(m_data, addr));
}

/******************************************
 push    @rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z53_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHW(dst, RDMEM_W(m_data, addr));
}

/******************************************
 push    @rd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z53_ddN0_ssN0_addr()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	PUSHW(dst, RDMEM_W(m_data, addr));
}

/******************************************
 ldl     rrd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z54_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = RDMEM_L(m_data, addr);
}

/******************************************
 ldl     rrd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z54_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = RDMEM_L(m_data, addr);
}

/******************************************
 popl    addr,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z55_ssN0_0000_addr()
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_L(m_data, addr, POPL(src));
}

/******************************************
 popl    addr(rd),@rs
 flags:  ------
 ******************************************/
void z8002_device::Z55_ssN0_ddN0_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_L(m_data, addr, POPL(src));
}

/******************************************
 addl    rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z56_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = ADDL(RL(dst), RDMEM_L(m_data, addr));
}

/******************************************
 addl    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z56_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = ADDL(RL(dst), RDMEM_L(m_data, addr));
}

/******************************************
 pop     addr,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z57_ssN0_0000_addr()
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, POPW(src));
}

/******************************************
 pop     addr(rd),@rs
 flags:  ------
 ******************************************/
void z8002_device::Z57_ssN0_ddN0_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, POPW(src));
}

/******************************************
 multl   rqd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z58_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = MULTL(RQ(dst), RDMEM_L(m_data, addr));
}

/******************************************
 multl   rqd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z58_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RQ(dst) = MULTL(RQ(dst), RDMEM_L(m_data, addr));
}

/******************************************
 mult    rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z59_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = MULTW(RL(dst), RDMEM_W(m_data, addr));
}

/******************************************
 mult    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z59_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = MULTW(RL(dst), RDMEM_W(m_data, addr));
}

/******************************************
 divl    rqd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z5A_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(m_data, addr));
}

/******************************************
 divl    rqd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z5A_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(m_data, addr));
}

/******************************************
 div     rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z5B_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = DIVW(RL(dst), RDMEM_W(m_data, addr));
}

/******************************************
 div     rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
void z8002_device::Z5B_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RL(dst) = DIVW(RL(dst), RDMEM_W(m_data, addr));
}

/******************************************
 ldm     rd,addr,n
 flags:  ------
 ******************************************/
void z8002_device::Z5C_0000_0001_0000_dddd_0000_nmin1_addr()
{
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(m_data, addr);
		dst = (dst+1) & 15;
		addr = addr_add (addr, 2);
	}
}

/******************************************
 testl   addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z5C_0000_1000_addr()
{
	GET_ADDR(OP1);
	TESTL(RDMEM_L(m_data, addr));
}

/******************************************
 ldm     addr,rs,n
 flags:  ------
 ******************************************/
void z8002_device::Z5C_0000_1001_0000_ssss_0000_nmin1_addr()
{
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		WRMEM_W(m_data, addr, RW(src));
		src = (src+1) & 15;
		addr = addr_add (addr, 2);
	}
}

/******************************************
 testl   addr(rd)
 flags:  -ZS---
 ******************************************/
void z8002_device::Z5C_ddN0_1000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	TESTL(RDMEM_L(m_data, addr));
}

/******************************************
 ldm     addr(rd),rs,n
 flags:  ------
 ******************************************/
void z8002_device::Z5C_ddN0_1001_0000_ssN0_0000_nmin1_addr()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr = addr_add(addr, RW(dst));
	while (cnt-- >= 0) {
		WRMEM_W(m_data, addr, RW(src));
		src = (src+1) & 15;
		addr = addr_add(addr, 2);
	}
}

/******************************************
 ldm     rd,addr(rs),n
 flags:  ------
 ******************************************/
void z8002_device::Z5C_ssN0_0001_0000_dddd_0000_nmin1_addr()
{
	GET_SRC(OP0,NIB2);
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr = addr_add(addr, RW(src));
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(m_data, addr);
		dst = (dst+1) & 15;
		addr = addr_add(addr, 2);
	}
}

/******************************************
 ldl     addr,rrs
 flags:  ------
 ******************************************/
void z8002_device::Z5D_0000_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_L(m_data, addr, RL(src));
}

/******************************************
 ldl     addr(rd),rrs
 flags:  ------
 ******************************************/
void z8002_device::Z5D_ddN0_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_L(m_data, addr, RL(src));
}

/******************************************
 jp      cc,addr
 flags:  ------
 ******************************************/
void z8002_device::Z5E_0000_cccc_addr()
{
	GET_CCC(OP0,NIB3);
	GET_ADDR(OP1);
	switch (cc) {
		case  0: if (CC0) set_pc(addr); break;
		case  1: if (CC1) set_pc(addr); break;
		case  2: if (CC2) set_pc(addr); break;
		case  3: if (CC3) set_pc(addr); break;
		case  4: if (CC4) set_pc(addr); break;
		case  5: if (CC5) set_pc(addr); break;
		case  6: if (CC6) set_pc(addr); break;
		case  7: if (CC7) set_pc(addr); break;
		case  8: if (CC8) set_pc(addr); break;
		case  9: if (CC9) set_pc(addr); break;
		case 10: if (CCA) set_pc(addr); break;
		case 11: if (CCB) set_pc(addr); break;
		case 12: if (CCC) set_pc(addr); break;
		case 13: if (CCD) set_pc(addr); break;
		case 14: if (CCE) set_pc(addr); break;
		case 15: if (CCF) set_pc(addr); break;
	}
}

/******************************************
 jp      cc,addr(rd)
 flags:  ------
 ******************************************/
void z8002_device::Z5E_ddN0_cccc_addr()
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	switch (cc) {
		case  0: if (CC0) set_pc(addr); break;
		case  1: if (CC1) set_pc(addr); break;
		case  2: if (CC2) set_pc(addr); break;
		case  3: if (CC3) set_pc(addr); break;
		case  4: if (CC4) set_pc(addr); break;
		case  5: if (CC5) set_pc(addr); break;
		case  6: if (CC6) set_pc(addr); break;
		case  7: if (CC7) set_pc(addr); break;
		case  8: if (CC8) set_pc(addr); break;
		case  9: if (CC9) set_pc(addr); break;
		case 10: if (CCA) set_pc(addr); break;
		case 11: if (CCB) set_pc(addr); break;
		case 12: if (CCC) set_pc(addr); break;
		case 13: if (CCD) set_pc(addr); break;
		case 14: if (CCE) set_pc(addr); break;
		case 15: if (CCF) set_pc(addr); break;
	}
}

/******************************************
 call    addr
 flags:  ------
 ******************************************/
void z8002_device::Z5F_0000_0000_addr()
{
	GET_ADDR(OP1);
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	set_pc(addr);
}

/******************************************
 call    addr(rd)
 flags:  ------
 ******************************************/
void z8002_device::Z5F_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	addr = addr_add(addr, RW(dst));
	set_pc(addr);
}

/******************************************
 ldb     rbd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z60_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = RDMEM_B(m_data, addr);
}

/******************************************
 ldb     rbd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z60_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RB(dst) = RDMEM_B(m_data, addr);
}

/******************************************
 ld      rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z61_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = RDMEM_W(m_data, addr);
}

/******************************************
 ld      rd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z61_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(src));
	RW(dst) = RDMEM_W(m_data, addr);
}

/******************************************
 resb    addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z62_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) & ~bit);
}

/******************************************
 resb    addr(rd),imm4
 flags:  ------
 ******************************************/
void z8002_device::Z62_ddN0_imm4_addr()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) & ~bit);
}

/******************************************
 res     addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z63_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) & ~bit);
}

/******************************************
 res     addr(rd),imm4
 flags:  ------
 ******************************************/
void z8002_device::Z63_ddN0_imm4_addr()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) & ~bit);
}

/******************************************
 setb    addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z64_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) | bit);
}

/******************************************
 setb    addr(rd),imm4
 flags:  ------
 ******************************************/
void z8002_device::Z64_ddN0_imm4_addr()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RDMEM_B(m_data, addr) | bit);
}

/******************************************
 set     addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z65_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) | bit);
}

/******************************************
 set     addr(rd),imm4
 flags:  ------
 ******************************************/
void z8002_device::Z65_ddN0_imm4_addr()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, RDMEM_W(m_data, addr) | bit);
}

/******************************************
 bitb    addr,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z66_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_B(m_data, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bitb    addr(rd),imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z66_ddN0_imm4_addr()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_B(m_data, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z67_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_W(m_data, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr(rd),imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z67_ddN0_imm4_addr()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	if (RDMEM_W(m_data, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z68_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, INCB(RDMEM_B(m_data, addr), i4p1));
}

/******************************************
 incb    addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z68_ddN0_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, INCB(RDMEM_B(m_data, addr), i4p1));
}

/******************************************
 inc     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z69_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, INCW(RDMEM_W(m_data, addr), i4p1));
}

/******************************************
 inc     addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z69_ddN0_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, INCW(RDMEM_W(m_data, addr), i4p1));
}

/******************************************
 decb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z6A_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(m_data, addr, DECB(RDMEM_B(m_data, addr), i4p1));
}

/******************************************
 decb    addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z6A_ddN0_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, DECB(RDMEM_B(m_data, addr), i4p1));
}

/******************************************
 dec     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z6B_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(m_data, addr, DECW(RDMEM_W(m_data, addr), i4p1));
}

/******************************************
 dec     addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z6B_ddN0_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, DECW(RDMEM_W(m_data, addr), i4p1));
}

/******************************************
 exb     rbd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z6C_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	uint8_t tmp = RDMEM_B(m_data, addr);
	WRMEM_B(m_data, addr, RB(dst));
	RB(dst) = tmp;
}

/******************************************
 exb     rbd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z6C_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	uint8_t tmp;
	addr = addr_add(addr, RW(src));
	tmp = RDMEM_B(m_data, addr);
	WRMEM_B(m_data, addr, RB(dst));
	RB(dst) = tmp;
}

/******************************************
 ex      rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z6D_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	uint16_t tmp = RDMEM_W(m_data, addr);
	WRMEM_W(m_data, addr, RW(dst));
	RW(dst) = tmp;
}

/******************************************
 ex      rd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z6D_ssN0_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	uint16_t tmp;
	addr = addr_add(addr, RW(src));
	tmp = RDMEM_W(m_data, addr);
	WRMEM_W(m_data, addr, RW(dst));
	RW(dst) = tmp;
}

/******************************************
 ldb     addr,rbs
 flags:  ------
 ******************************************/
void z8002_device::Z6E_0000_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(m_data,  addr, RB(src));
}

/******************************************
 ldb     addr(rd),rbs
 flags:  ------
 ******************************************/
void z8002_device::Z6E_ddN0_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_B(m_data, addr, RB(src));
}

/******************************************
 ld      addr,rs
 flags:  ------
 ******************************************/
void z8002_device::Z6F_0000_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(m_data,  addr, RW(src));
}

/******************************************
 ld      addr(rd),rs
 flags:  ------
 ******************************************/
void z8002_device::Z6F_ddN0_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(addr, RW(dst));
	WRMEM_W(m_data, addr, RW(src));
}

/******************************************
 ldb     rbd,rs(rx)
 flags:  ------
 ******************************************/
void z8002_device::Z70_ssN0_dddd_0000_xxxx_0000_0000()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RB(dst) = RDBX_B(src, RW(idx));
}

/******************************************
 ld      rd,rs(rx)
 flags:  ------
 ******************************************/
void z8002_device::Z71_ssN0_dddd_0000_xxxx_0000_0000()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RW(dst) = RDBX_W(src, RW(idx));
}

/******************************************
 ldb     rd(rx),rbs
 flags:  ------
 ******************************************/
void z8002_device::Z72_ddN0_ssss_0000_xxxx_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRBX_B(dst, RW(idx), RB(src));
}

/******************************************
 ld      rd(rx),rs
 flags:  ------
 ******************************************/
void z8002_device::Z73_ddN0_ssss_0000_xxxx_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRBX_W(dst, RW(idx), RW(src));
}

/******************************************
 lda     prd,rs(rx)
 flags:  ------
 ******************************************/
void z8002_device::Z74_ssN0_dddd_0000_xxxx_0000_0000()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	if (get_segmented_mode()) {
		RL(dst) = RL(src);
	}
	else {
		RW(dst) = RW(src);
	}
	add_to_addr_reg(dst, RW(idx));
}

/******************************************
 ldl     rrd,rs(rx)
 flags:  ------
 ******************************************/
void z8002_device::Z75_ssN0_dddd_0000_xxxx_0000_0000()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RL(dst) = RDBX_L(src, RW(idx));
}

/******************************************
 lda     prd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z76_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR_RAW(OP1);
	if (get_segmented_mode()) {
		RL(dst) = addr;
	}
	else {
		RW(dst) = addr;
	}
}

/******************************************
 lda     prd,addr(rs)
 flags:  ------
 ******************************************/
void z8002_device::Z76_ssN0_dddd_addr()
{//@@@
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR_RAW(OP1);
	uint16_t temp = RW(src);  // store src in case dst == src
	if (get_segmented_mode()) {
		RL(dst) = addr;
	}
	else {
		RW(dst) = addr;
	}
	add_to_addr_reg(dst, temp);
}

/******************************************
 ldl     rd(rx),rrs
 flags:  ------
 ******************************************/
void z8002_device::Z77_ddN0_ssss_0000_xxxx_0000_0000()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRBX_L(dst, RW(idx), RL(src));
}

/******************************************
 rsvd78
 flags:  ------
 ******************************************/
void z8002_device::Z78_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvd78 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ldps    addr
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z79_0000_0000_addr()
{
	CHECK_PRIVILEGED_INSTR();
	GET_ADDR(OP1);
	uint16_t fcw;
	if (get_segmented_mode()) {
		fcw = RDMEM_W(m_data, addr + 2);
		set_pc(segmented_addr(RDMEM_L(m_data, addr + 4)));
	}
	else {
		fcw = RDMEM_W(m_data, addr);
		set_pc(RDMEM_W(m_data, (uint16_t)(addr + 2)));
	}
	CHANGE_FCW(fcw); /* check for user/system mode change */
}

/******************************************
 ldps    addr(rs)
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z79_ssN0_0000_addr()
{
	CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	uint16_t fcw;
	addr = addr_add(addr, RW(src));
	if (get_segmented_mode()) {
		fcw = RDMEM_W(m_data, addr + 2);
		set_pc(segmented_addr(RDMEM_L(m_data, addr + 4)));
	}
	else {
		fcw = RDMEM_W(m_data, addr);
		m_pc = RDMEM_W(m_data, (uint16_t)(addr + 2));
	}
	if ((fcw ^ m_fcw) & F_SEG) printf("ldps 3 (0x%05x): changing from %ssegmented mode to %ssegmented mode\n", m_pc, (fcw & F_SEG) ? "non-" : "", (fcw & F_SEG) ? "" : "non-");
	CHANGE_FCW(fcw); /* check for user/system mode change */
}

/******************************************
 halt
 flags:  ------
 ******************************************/
void z8002_device::Z7A_0000_0000()
{
	CHECK_PRIVILEGED_INSTR();
	m_halt = true;
	if (m_icount > 0) m_icount = 0;
}

/******************************************
 iret
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z7B_0000_0000()
{
	uint16_t tag, fcw;
	CHECK_PRIVILEGED_INSTR();
	tag = POPW(SP);   /* get type tag */
	fcw = POPW(SP);   /* get m_fcw  */
	if (get_segmented_mode())
		set_pc(segmented_addr(POPL(SP)));
	else
		m_pc    = POPW(SP);   /* get m_pc   */
	CHANGE_FCW(fcw);       /* check for user/system mode change */
	LOG("Z8K IRET tag $%04x, fcw $%04x, pc $%04x\n", tag, fcw, m_pc);
}

/******************************************
 mset
 flags:  ------
 ******************************************/
void z8002_device::Z7B_0000_1000()
{
	CHECK_PRIVILEGED_INSTR();
	/* set mu-0 line */
}

/******************************************
 mres
 flags:  ------
 ******************************************/
void z8002_device::Z7B_0000_1001()
{
	CHECK_PRIVILEGED_INSTR();
	/* reset mu-0 line */
}

/******************************************
 mbit
 flags:  CZS---
 ******************************************/
void z8002_device::Z7B_0000_1010()
{
	CHECK_PRIVILEGED_INSTR();
	/* test mu-I line */
}

/******************************************
 mreq    rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z7B_dddd_1101()
{
	CHECK_PRIVILEGED_INSTR();
	/* test mu-I line, invert cascade to mu-0  */
	if (m_mi)
	{
		CLR_Z;
		CLR_S;
		m_mo_out(CLEAR_LINE);
		return;
	}
	SET_Z;
	m_mo_out(ASSERT_LINE);
	if (m_mi)
	{
		SET_S;
	}
	else
	{
		CLR_S;
		m_mo_out(CLEAR_LINE);
	}
}

/******************************************
 di      i2
 flags:  ------
 ******************************************/
void z8002_device::Z7C_0000_00ii()
{
	CHECK_PRIVILEGED_INSTR();
	GET_IMM2(OP0,NIB3);
	uint16_t fcw = m_fcw;
	fcw &= (imm2 << 11) | 0xe7ff;
	CHANGE_FCW(fcw);
}

/******************************************
 ei      i2
 flags:  ------
 ******************************************/
void z8002_device::Z7C_0000_01ii()
{
	CHECK_PRIVILEGED_INSTR();
	GET_IMM2(OP0,NIB3);
	uint16_t fcw = m_fcw;
	fcw |= ((~imm2) << 11) & 0x1800;
	CHANGE_FCW(fcw);
}

/******************************************
 ldctl   rd,ctrl
 flags:  ------
 ******************************************/
void z8002_device::Z7D_dddd_0ccc()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_IMM3(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (imm3) {
		case 2:
			RW(dst) = m_fcw;
			break;
		case 3:
			RW(dst) = m_refresh;
			break;
		case 4:
			RW(dst) = m_psapseg;
			break;
		case 5:
			RW(dst) = m_psapoff;
			break;
		case 6:
			RW(dst) = m_nspseg;
			break;
		case 7:
			RW(dst) = m_nspoff;
			break;
		default:
			LOG("Z8K LDCTL R%d,%d\n", dst, imm3);
	}
}

/******************************************
 ldctl   ctrl,rs
 flags:  ------
 ******************************************/
void z8002_device::Z7D_ssss_1ccc()
{//@@@
	CHECK_PRIVILEGED_INSTR();
	GET_IMM3(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	switch (imm3) {
		case 2:
			{
				uint16_t fcw;
				fcw = RW(src);
				CHANGE_FCW(fcw); /* check for user/system mode change */
			}
			break;
		case 3:
			m_refresh = RW(src);
			break;
		case 4:
			m_psapseg = RW(src);
			break;
		case 5:
			m_psapoff = RW(src);
			break;
		case 6:
			m_nspseg = RW(src);
			break;
		case 7:
			m_nspoff = RW(src);
			break;
		default:
			LOG("Z8K LDCTL %d,R%d\n", imm3, src);
	}
}

/******************************************
 rsvd7e
 flags:  ------
 ******************************************/
void z8002_device::Z7E_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvd7e $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 sc      imm8
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z7F_imm8()
{
	GET_IMM8(0);
	/* execute system call via IRQ */
	m_irq_req |= Z8000_SYSCALL;
	(void)imm8;
}

/******************************************
 addb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z80_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADDB(RB(dst), RB(src));
}

/******************************************
 add     rd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z81_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADDW(RW(dst), RW(src));
}

/******************************************
 subb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z82_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SUBB(RB(dst), RB(src));
}

/******************************************
 sub     rd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z83_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SUBW(RW(dst), RW(src));
}

/******************************************
 orb     rbd,rbs
 flags:  CZSP--
 ******************************************/
void z8002_device::Z84_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ORB(RB(dst), RB(src));
}

/******************************************
 or      rd,rs
 flags:  CZS---
 ******************************************/
void z8002_device::Z85_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ORW(RW(dst), RW(src));
}

/******************************************
 andb    rbd,rbs
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z86_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ANDB(RB(dst), RB(src));
}

/******************************************
 and     rd,rs
 flags:  -ZS---
 ******************************************/
void z8002_device::Z87_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ANDW(RW(dst), RW(src));
}

/******************************************
 xorb    rbd,rbs
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z88_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = XORB(RB(dst), RB(src));
}

/******************************************
 xor     rd,rs
 flags:  -ZS---
 ******************************************/
void z8002_device::Z89_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = XORW(RW(dst), RW(src));
}

/******************************************
 cpb     rbd,rbs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z8A_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB(RB(dst), RB(src));
}

/******************************************
 cp      rd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z8B_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW(RW(dst), RW(src));
}

/******************************************
 comb    rbd
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z8C_dddd_0000()
{
	GET_DST(OP0,NIB2);
	RB(dst) = COMB(RB(dst));
}

/******************************************
 negb    rbd
 flags:  CZSV--
 ******************************************/
void z8002_device::Z8C_dddd_0010()
{
	GET_DST(OP0,NIB2);
	RB(dst) = NEGB(RB(dst));
}

/******************************************
 testb   rbd
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z8C_dddd_0100()
{
	GET_DST(OP0,NIB2);
	TESTB(RB(dst));
}

/******************************************
 tsetb   rbd
 flags:  --S---
 ******************************************/
void z8002_device::Z8C_dddd_0110()
{
	GET_DST(OP0,NIB2);
	if (RB(dst) & S08) SET_S; else CLR_S;
	RB(dst) = 0xff;
}

/******************************************
 ldctlb rbd,flags
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z8C_dddd_0001()
{
	GET_DST(OP0,NIB2);
	RB(dst) = m_fcw & 0xfc;
}

/******************************************
 clrb    rbd
 flags:  ------
 ******************************************/
void z8002_device::Z8C_dddd_1000()
{
	GET_DST(OP0,NIB2);
	RB(dst) = 0;
}

/******************************************
 ldctlb flags,rbd
 flags:  ------
 ******************************************/
void z8002_device::Z8C_dddd_1001()
{
	GET_DST(OP0,NIB2);
	m_fcw &= ~0x00fc;
	m_fcw |= (RB(dst) & 0xfc);
}

/******************************************
 nop
 flags:  ------
 ******************************************/
void z8002_device::Z8D_0000_0111()
{
	/* nothing */
}

/******************************************
 com     rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z8D_dddd_0000()
{
	GET_DST(OP0,NIB2);
	RW(dst) = COMW(RW(dst));
}

/******************************************
 neg     rd
 flags:  CZSV--
 ******************************************/
void z8002_device::Z8D_dddd_0010()
{
	GET_DST(OP0,NIB2);
	RW(dst) = NEGW(RW(dst));
}

/******************************************
 test    rd
 flags:  ------
 ******************************************/
void z8002_device::Z8D_dddd_0100()
{
	GET_DST(OP0,NIB2);
	TESTW(RW(dst));
}

/******************************************
 tset    rd
 flags:  --S---
 ******************************************/
void z8002_device::Z8D_dddd_0110()
{
	GET_DST(OP0,NIB2);
	if (RW(dst) & S16) SET_S; else CLR_S;
	RW(dst) = 0xffff;
}

/******************************************
 clr     rd
 flags:  ------
 ******************************************/
void z8002_device::Z8D_dddd_1000()
{
	GET_DST(OP0,NIB2);
	RW(dst) = 0;
}

/******************************************
 setflg  imm4
 flags:  CZSV--
 ******************************************/
void z8002_device::Z8D_imm4_0001()
{
	m_fcw |= m_op[0] & 0x00f0;
}

/******************************************
 resflg  imm4
 flags:  CZSV--
 ******************************************/
void z8002_device::Z8D_imm4_0011()
{
	m_fcw &= ~(m_op[0] & 0x00f0);
}

/******************************************
 comflg  flags
 flags:  CZSP--
 ******************************************/
void z8002_device::Z8D_imm4_0101()
{
	m_fcw ^= (m_op[0] & 0x00f0);
}

/******************************************
 ext8e   imm8
 flags:  ------
 ******************************************/
void z8002_device::Z8E_imm8()
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG("Z8K %04x: ext8e  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ext8f   imm8
 flags:  ------
 ******************************************/
void z8002_device::Z8F_imm8()
{
	CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG("Z8K %04x: ext8f  $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 cpl     rrd,rrs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z90_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPL(RL(dst), RL(src));
}

/******************************************
 pushl   @rd,rrs
 flags:  ------
 ******************************************/
void z8002_device::Z91_ddN0_ssss()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL(dst, RL(src));
}

/******************************************
 subl    rrd,rrs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z92_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = SUBL(RL(dst), RL(src));
}

/******************************************
 push    @rd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z93_ddN0_ssss()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(dst, RW(src));
}

/******************************************
 ldl     rrd,rrs
 flags:  ------
 ******************************************/
void z8002_device::Z94_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = RL(src);
}

/******************************************
 popl    rrd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z95_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = POPL(src);
}

/******************************************
 addl    rrd,rrs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z96_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = ADDL(RL(dst), RL(src));
}

/******************************************
 pop     rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z97_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = POPW(src);
}

/******************************************
 multl   rqd,rrs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z98_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = MULTL(RQ(dst), RL(src));
}

/******************************************
 mult    rrd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z99_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = MULTW(RL(dst), RW(src));
}

/******************************************
 divl    rqd,rrs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z9A_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = DIVL(RQ(dst), RL(src));
}

/******************************************
 div     rrd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::Z9B_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = DIVW(RL(dst), RW(src));
}

/******************************************
 testl   rrd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z9C_dddd_1000()
{
	GET_DST(OP0,NIB2);
	CLR_ZS;
	if (!RL(dst)) SET_Z;
	else if (RL(dst) & S32) SET_S;
}

/******************************************
 rsvd9d
 flags:  ------
 ******************************************/
void z8002_device::Z9D_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvd9d $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ret     cc
 flags:  ------
 ******************************************/
void z8002_device::Z9E_0000_cccc()
{
	GET_CCC(OP0,NIB3);
	if (get_segmented_mode()) {
		switch (cc) {
			case  0: if (CC0) set_pc(segmented_addr(POPL(SP))); break;
			case  1: if (CC1) set_pc(segmented_addr(POPL(SP))); break;
			case  2: if (CC2) set_pc(segmented_addr(POPL(SP))); break;
			case  3: if (CC3) set_pc(segmented_addr(POPL(SP))); break;
			case  4: if (CC4) set_pc(segmented_addr(POPL(SP))); break;
			case  5: if (CC5) set_pc(segmented_addr(POPL(SP))); break;
			case  6: if (CC6) set_pc(segmented_addr(POPL(SP))); break;
			case  7: if (CC7) set_pc(segmented_addr(POPL(SP))); break;
			case  8: if (CC8) set_pc(segmented_addr(POPL(SP))); break;
			case  9: if (CC9) set_pc(segmented_addr(POPL(SP))); break;
			case 10: if (CCA) set_pc(segmented_addr(POPL(SP))); break;
			case 11: if (CCB) set_pc(segmented_addr(POPL(SP))); break;
			case 12: if (CCC) set_pc(segmented_addr(POPL(SP))); break;
			case 13: if (CCD) set_pc(segmented_addr(POPL(SP))); break;
			case 14: if (CCE) set_pc(segmented_addr(POPL(SP))); break;
			case 15: if (CCF) set_pc(segmented_addr(POPL(SP))); break;
		}
	}
	else {
		switch (cc) {
			case  0: if (CC0) set_pc(POPW(SP)); break;
			case  1: if (CC1) set_pc(POPW(SP)); break;
			case  2: if (CC2) set_pc(POPW(SP)); break;
			case  3: if (CC3) set_pc(POPW(SP)); break;
			case  4: if (CC4) set_pc(POPW(SP)); break;
			case  5: if (CC5) set_pc(POPW(SP)); break;
			case  6: if (CC6) set_pc(POPW(SP)); break;
			case  7: if (CC7) set_pc(POPW(SP)); break;
			case  8: if (CC8) set_pc(POPW(SP)); break;
			case  9: if (CC9) set_pc(POPW(SP)); break;
			case 10: if (CCA) set_pc(POPW(SP)); break;
			case 11: if (CCB) set_pc(POPW(SP)); break;
			case 12: if (CCC) set_pc(POPW(SP)); break;
			case 13: if (CCD) set_pc(POPW(SP)); break;
			case 14: if (CCE) set_pc(POPW(SP)); break;
			case 15: if (CCF) set_pc(POPW(SP)); break;
		}
	}
}

/******************************************
 rsvd9f
 flags:  ------
 ******************************************/
void z8002_device::Z9F_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvd9f $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ldb     rbd,rbs
 flags:  ------
 ******************************************/
void z8002_device::ZA0_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = RB(src);
}

/******************************************
 ld      rd,rs
 flags:  ------
 ******************************************/
void z8002_device::ZA1_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = RW(src);
}

/******************************************
 resb    rbd,imm4
 flags:  ------
 ******************************************/
void z8002_device::ZA2_dddd_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RB(dst) &= ~bit;
}

/******************************************
 res     rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::ZA3_dddd_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RW(dst) &= ~bit;
}

/******************************************
 setb    rbd,imm4
 flags:  ------
 ******************************************/
void z8002_device::ZA4_dddd_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RB(dst) |= bit;
}

/******************************************
 set     rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::ZA5_dddd_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RW(dst) |= bit;
}

/******************************************
 bitb    rbd,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::ZA6_dddd_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RB(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     rd,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::ZA7_dddd_imm4()
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RW(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    rbd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZA8_dddd_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RB(dst) = INCB(RB(dst), i4p1);
}

/******************************************
 inc     rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZA9_dddd_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RW(dst) = INCW(RW(dst), i4p1);
}

/******************************************
 decb    rbd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZAA_dddd_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RB(dst) = DECB(RB(dst), i4p1);
}

/******************************************
 dec     rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZAB_dddd_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RW(dst) = DECW(RW(dst), i4p1);
}

/******************************************
 exb     rbd,rbs
 flags:  ------
 ******************************************/
void z8002_device::ZAC_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	uint8_t tmp = RB(src);
	RB(src) = RB(dst);
	RB(dst) = tmp;
}

/******************************************
 ex      rd,rs
 flags:  ------
 ******************************************/
void z8002_device::ZAD_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	uint16_t tmp = RW(src);
	RW(src) = RW(dst);
	RW(dst) = tmp;
}

/******************************************
 tccb    cc,rbd
 flags:  ------
 ******************************************/
void z8002_device::ZAE_dddd_cccc()
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	uint8_t tmp = RB(dst) & ~1;
	switch (cc) {
		case  0: if (CC0) tmp |= 1; break;
		case  1: if (CC1) tmp |= 1; break;
		case  2: if (CC2) tmp |= 1; break;
		case  3: if (CC3) tmp |= 1; break;
		case  4: if (CC4) tmp |= 1; break;
		case  5: if (CC5) tmp |= 1; break;
		case  6: if (CC6) tmp |= 1; break;
		case  7: if (CC7) tmp |= 1; break;
		case  8: if (CC8) tmp |= 1; break;
		case  9: if (CC9) tmp |= 1; break;
		case 10: if (CCA) tmp |= 1; break;
		case 11: if (CCB) tmp |= 1; break;
		case 12: if (CCC) tmp |= 1; break;
		case 13: if (CCD) tmp |= 1; break;
		case 14: if (CCE) tmp |= 1; break;
		case 15: if (CCF) tmp |= 1; break;
	}
	RB(dst) = tmp;
}

/******************************************
 tcc     cc,rd
 flags:  ------
 ******************************************/
void z8002_device::ZAF_dddd_cccc()
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	uint16_t tmp = RW(dst) & ~1;
	switch (cc) {
		case  0: if (CC0) tmp |= 1; break;
		case  1: if (CC1) tmp |= 1; break;
		case  2: if (CC2) tmp |= 1; break;
		case  3: if (CC3) tmp |= 1; break;
		case  4: if (CC4) tmp |= 1; break;
		case  5: if (CC5) tmp |= 1; break;
		case  6: if (CC6) tmp |= 1; break;
		case  7: if (CC7) tmp |= 1; break;
		case  8: if (CC8) tmp |= 1; break;
		case  9: if (CC9) tmp |= 1; break;
		case 10: if (CCA) tmp |= 1; break;
		case 11: if (CCB) tmp |= 1; break;
		case 12: if (CCC) tmp |= 1; break;
		case 13: if (CCD) tmp |= 1; break;
		case 14: if (CCE) tmp |= 1; break;
		case 15: if (CCF) tmp |= 1; break;
	}
	RW(dst) = tmp;
}

/******************************************
 dab     rbd
 flags:  CZS---
 ******************************************/
void z8002_device::ZB0_dddd_0000()
{
	GET_DST(OP0,NIB2);
	uint8_t result;
	uint16_t idx = RB(dst);
	if (m_fcw & F_C)    idx |= 0x100;
	if (m_fcw & F_H)    idx |= 0x200;
	if (m_fcw & F_DA) idx |= 0x400;
	result = Z8000_dab[idx];
	CLR_CZS;
	CHK_XXXB_ZS;
	if (Z8000_dab[idx] & 0x100) SET_C;
	RB(dst) = result;
}

/******************************************
 extsb   rd
 flags:  ------
 ******************************************/
void z8002_device::ZB1_dddd_0000()
{
	GET_DST(OP0,NIB2);
	RW(dst) = (int16_t)(int8_t)RW(dst);
}

/******************************************
 extsl   rqd
 flags:  ------
 ******************************************/
void z8002_device::ZB1_dddd_0111()
{
	GET_DST(OP0,NIB2);
	RQ(dst) = (int64_t)(int32_t)RQ(dst);
}

/******************************************
 exts    rrd
 flags:  ------
 ******************************************/
void z8002_device::ZB1_dddd_1010()
{
	GET_DST(OP0,NIB2);
	RL(dst) = (int32_t)(int16_t)RL(dst);
}

/******************************************
 sllb    rbd,imm8
 flags:  CZS---
 srlb    rbd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB2_dddd_0001_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	if (imm8 & S08)
		RB(dst) = SRLB(RB(dst), -(int8_t)imm8);
	else
		RB(dst) = SLLB(RB(dst), imm8);
}

/******************************************
 sdlb    rbd,rs
 flags:  CZS---
 ******************************************/
void z8002_device::ZB2_dddd_0011_0000_ssss_0000_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RB(dst) = SRLB(RB(dst), (int8_t)RW(src));
}

/******************************************
 rlb     rbd,imm1or2
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB2_dddd_00I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RLB(RB(dst), imm1);
}

/******************************************
 rrb     rbd,imm1or2
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB2_dddd_01I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RRB(RB(dst), imm1);
}

/******************************************
 slab    rbd,imm8
 flags:  CZSV--
 srab    rbd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB2_dddd_1001_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	if (imm8 & S08)
		RB(dst) = SRAB(RB(dst), -(int8_t)imm8);
	else
		RB(dst) = SLAB(RB(dst), imm8);
}

/******************************************
 sdab    rbd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB2_dddd_1011_0000_ssss_0000_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RB(dst) = SDAB(RB(dst), (int8_t) RW(src));
}

/******************************************
 rlcb    rbd,imm1or2
 flags:  -Z----
 ******************************************/
void z8002_device::ZB2_dddd_10I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RLCB(RB(dst), imm1);
}

/******************************************
 rrcb    rbd,imm1or2
 flags:  -Z----
 ******************************************/
void z8002_device::ZB2_dddd_11I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RRCB(RB(dst), imm1);
}

/******************************************
 sll     rd,imm8
 flags:  CZS---
 srl     rd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_0001_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RW(dst) = SRLW(RW(dst), -(int16_t)imm16);
	else
		RW(dst) = SLLW(RW(dst), imm16);
}

/******************************************
 sdl     rd,rs
 flags:  CZS---
 ******************************************/
void z8002_device::ZB3_dddd_0011_0000_ssss_0000_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RW(dst) = SDLW(RW(dst), (int8_t)RW(src));
}

/******************************************
 rl      rd,imm1or2
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_00I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RLW(RW(dst), imm1);
}

/******************************************
 slll    rrd,imm8
 flags:  CZS---
 srll    rrd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_0101_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RL(dst) = SRLL(RL(dst), -(int16_t)imm16);
	else
		RL(dst) = SLLL(RL(dst), imm16);
}

/******************************************
 sdll    rrd,rs
 flags:  CZS---
 ******************************************/
void z8002_device::ZB3_dddd_0111_0000_ssss_0000_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RL(dst) = SDLL(RL(dst), RW(src) & 0xff);
}

/******************************************
 rr      rd,imm1or2
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_01I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RRW(RW(dst), imm1);
}

/******************************************
 sla     rd,imm8
 flags:  CZSV--
 sra     rd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_1001_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RW(dst) = SRAW(RW(dst), -(int16_t)imm16);
	else
		RW(dst) = SLAW(RW(dst), imm16);
}

/******************************************
 sda     rd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_1011_0000_ssss_0000_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RW(dst) = SDAW(RW(dst), (int8_t)RW(src));
}

/******************************************
 rlc     rd,imm1or2
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_10I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RLCW(RW(dst), imm1);
}

/******************************************
 slal    rrd,imm8
 flags:  CZSV--
 sral    rrd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_1101_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RL(dst) = SRAL(RL(dst), -(int16_t)imm16);
	else
		RL(dst) = SLAL(RL(dst), imm16);
}

/******************************************
 sdal    rrd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_1111_0000_ssss_0000_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RL(dst) = SDAL(RL(dst), RW(src) & 0xff);
}

/******************************************
 rrc     rd,imm1or2
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB3_dddd_11I0()
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RRCW(RW(dst), imm1);
}

/******************************************
 adcb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
void z8002_device::ZB4_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADCB(RB(dst), RB(src));
}

/******************************************
 adc     rd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB5_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADCW(RW(dst), RW(src));
}

/******************************************
 sbcb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
void z8002_device::ZB6_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SBCB(RB(dst), RB(src));
}

/******************************************
 sbc     rd,rs
 flags:  CZSV--
 ******************************************/
void z8002_device::ZB7_ssss_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SBCW(RW(dst), RW(src));
}

/******************************************
 trtib   @rd,@rs,rr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_0010_0000_rrrr_ssN0_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trtirb  @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_0110_0000_rrrr_ssN0_1110()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) {
		CLR_V;
		if (!xlt)
		m_pc -= 4;
	}
	else SET_V;
}

/******************************************
 trtdb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_1010_0000_rrrr_ssN0_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trtdrb  @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_1110_0000_rrrr_ssN0_1110()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	uint8_t xlt = RDBX_B(src, RDIR_B(dst));
	RB(1) = xlt;  /* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) {
		CLR_V;
		if (!xlt)
		m_pc -= 4;
	}
	else SET_V;
}

/******************************************
 trib    @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_0000_0000_rrrr_ssN0_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trirb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_0100_0000_rrrr_ssN0_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; m_pc -= 4; } else SET_V;
}

/******************************************
 trdb    @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_1000_0000_rrrr_ssN0_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trdrb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
void z8002_device::ZB8_ddN0_1100_0000_rrrr_ssN0_0000()
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	memory_access<23, 1, 0, ENDIANNESS_BIG>::specific &dstspace = dst == SP ? m_stack : m_data;
	uint32_t dstaddr = addr_from_reg(dst);
	uint8_t xlt = RDBX_B(src, RDMEM_B(dstspace, dstaddr));
	WRMEM_B(dstspace, dstaddr, xlt);
	RB(1) = xlt;  /* destroy RH1 */
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; m_pc -= 4; } else SET_V;
}

/******************************************
 rsvdb9
 flags:  ------
 ******************************************/
void z8002_device::ZB9_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvdb9 $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
	(void)imm8;
}

/******************************************
 cpib    rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_0000_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldib    @rd,@rs,rr
 ldibr   @rd,@rs,rr
 flags:  ---V--
 ******************************************/
void z8002_device::ZBA_ssN0_0001_0000_rrrr_ddN0_x000()
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);  /* repeat? */
	WRIR_B(dst, RDIR_B(src));
	add_to_addr_reg(src, 1);
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsib   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_0010_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpirb   rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_0100_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsirb  @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_0110_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 1);
	add_to_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpdb    rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_1000_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 lddb    @rs,@rd,rr
 lddbr   @rs,@rd,rr
 flags:  ---V--
 ******************************************/
void z8002_device::ZBA_ssN0_1001_0000_rrrr_ddN0_x000()
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_B(dst, RDIR_B(src));
	sub_from_addr_reg(src, 1);
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsdb   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_1010_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpdrb   rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_1100_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RB(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsdrb  @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBA_ssN0_1110_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(RDIR_B(dst), RDIR_B(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 1);
	sub_from_addr_reg(dst, 1);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpi     rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_0000_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldi     @rd,@rs,rr
 ldir    @rd,@rs,rr
 flags:  ---V--
 ******************************************/
void z8002_device::ZBB_ssN0_0001_0000_rrrr_ddN0_x000()
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDIR_W(src));
	add_to_addr_reg(src, 2);
	add_to_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsi    @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_0010_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	add_to_addr_reg(dst, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpir    rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_0100_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsir   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_0110_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	add_to_addr_reg(src, 2);
	add_to_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpd     rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_1000_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldd     @rs,@rd,rr
 lddr    @rs,@rd,rr
 flags:  ---V--
 ******************************************/
void z8002_device::ZBB_ssN0_1001_0000_rrrr_ddN0_x000()
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRIR_W(dst, RDIR_W(src));
	sub_from_addr_reg(src, 2);
	sub_from_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (cc == 0) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsd    @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_1010_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	sub_from_addr_reg(dst, 2);
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpdr    rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_1100_0000_rrrr_dddd_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RW(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 cpsdr   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
void z8002_device::ZBB_ssN0_1110_0000_rrrr_ddN0_cccc()
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(RDIR_W(dst), RDIR_W(src));
	switch (cc) {
		case  0: if (CC0) SET_Z; else CLR_Z; break;
		case  1: if (CC1) SET_Z; else CLR_Z; break;
		case  2: if (CC2) SET_Z; else CLR_Z; break;
		case  3: if (CC3) SET_Z; else CLR_Z; break;
		case  4: if (CC4) SET_Z; else CLR_Z; break;
		case  5: if (CC5) SET_Z; else CLR_Z; break;
		case  6: if (CC6) SET_Z; else CLR_Z; break;
		case  7: if (CC7) SET_Z; else CLR_Z; break;
		case  8: if (CC8) SET_Z; else CLR_Z; break;
		case  9: if (CC9) SET_Z; else CLR_Z; break;
		case 10: if (CCA) SET_Z; else CLR_Z; break;
		case 11: if (CCB) SET_Z; else CLR_Z; break;
		case 12: if (CCC) SET_Z; else CLR_Z; break;
		case 13: if (CCD) SET_Z; else CLR_Z; break;
		case 14: if (CCE) SET_Z; else CLR_Z; break;
		case 15: if (CCF) SET_Z; else CLR_Z; break;
	}
	sub_from_addr_reg(src, 2);
	sub_from_addr_reg(dst, 2);
	if (--RW(cnt)) { CLR_V; if (!(m_fcw & F_Z)) m_pc -= 4; } else SET_V;
}

/******************************************
 rrdb    rbb,rba
 flags:  -Z----
 ******************************************/
void z8002_device::ZBC_aaaa_bbbb()
{
	uint8_t b = m_op[0] & 15;
	uint8_t a = (m_op[0] >> 4) & 15;
	uint8_t tmp = RB(b);
	RB(a) = (RB(a) >> 4) | (RB(b) << 4);
	RB(b) = (RB(b) & 0xf0) | (tmp & 0x0f);
	if (RB(b)) CLR_Z; else SET_Z;
}

/******************************************
 ldk     rd,imm4
 flags:  ------
 ******************************************/
void z8002_device::ZBD_dddd_imm4()
{
	GET_DST(OP0,NIB2);
	GET_IMM4(OP0,NIB3);
	RW(dst) = imm4;
}

/******************************************
 rldb    rbb,rba
 flags:  -Z----
 ******************************************/
void z8002_device::ZBE_aaaa_bbbb()
{
	uint8_t b = m_op[0] & 15;
	uint8_t a = (m_op[0] >> 4) & 15;
	uint8_t tmp = RB(a);
	RB(a) = (RB(a) << 4) | (RB(b) & 0x0f);
	RB(b) = (RB(b) & 0xf0) | (tmp >> 4);
	if (RB(b)) CLR_Z; else SET_Z;
}

/******************************************
 rsvdbf
 flags:  ------
 ******************************************/
void z8002_device::ZBF_imm8()
{
	GET_IMM8(0);
	LOG("Z8K %04x: rsvdbf $%02x\n", m_pc, imm8);
	if (m_fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
	(void)imm8;
}

/******************************************
 ldb     rbd,imm8   (long version)
 flags:  ------
 ******************************************/
void z8002_device::Z20_0000_dddd_imm8()
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = imm8;
}

/******************************************
 ldb     rbd,imm8
 flags:  ------
 ******************************************/
void z8002_device::ZC_dddd_imm8()
{
	GET_DST(OP0,NIB1);
	GET_IMM8(0);
	RB(dst) = imm8;
}

/******************************************
 calr    dsp12
 flags:  ------
 ******************************************/
void z8002_device::ZD_dsp12()
{
	int16_t dsp12 = m_op[0] & 0xfff;
	if (get_segmented_mode())
		PUSHL(SP, make_segmented_addr(m_pc));
	else
		PUSHW(SP, m_pc);
	dsp12 = (dsp12 & 2048) ? 4096 - 2 * (dsp12 & 2047) : -2 * (dsp12 & 2047);
	set_pc(addr_add(m_pc, dsp12));
}

/******************************************
 jr      cc,dsp8
 flags:  ------
 ******************************************/
void z8002_device::ZE_cccc_dsp8()
{
	GET_DSP8;
	GET_CCC(OP0,NIB1);
	switch (cc) {
		case  0: if (CC0) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  1: if (CC1) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  2: if (CC2) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  3: if (CC3) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  4: if (CC4) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  5: if (CC5) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  6: if (CC6) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  7: if (CC7) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  8: if (CC8) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  9: if (CC9) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  10: if (CCA) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  11: if (CCB) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  12: if (CCC) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  13: if (CCD) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  14: if (CCE) set_pc(addr_add(m_pc, dsp8 * 2)); break;
		case  15: if (CCF) set_pc(addr_add(m_pc, dsp8 * 2)); break;
	}
}

/******************************************
 dbjnz   rbd,dsp7
 flags:  ------
 ******************************************/
void z8002_device::ZF_dddd_0dsp7()
{
	GET_DST(OP0,NIB1);
	GET_DSP7;
	RB(dst) -= 1;
	if (RB(dst)) {
		set_pc(addr_sub(m_pc, 2 * dsp7));
	}
}

/******************************************
 djnz    rd,dsp7
 flags:  ------
 ******************************************/
void z8002_device::ZF_dddd_1dsp7()
{
	GET_DST(OP0,NIB1);
	GET_DSP7;
	RW(dst) -= 1;
	if (RW(dst)) {
		set_pc(addr_sub(m_pc, 2 * dsp7));
	}
}
