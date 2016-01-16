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
void z8002_device::CHANGE_FCW(UINT16 fcw)
{
	UINT16 tmp;
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

void z8001_device::CHANGE_FCW(UINT16 fcw)
{
	UINT16 tmp;
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

UINT32 z8002_device::make_segmented_addr(UINT32 addr)
{
	return ((addr & 0x007f0000) << 8) | 0x80000000 | (addr & 0xffff);
}

UINT32 z8002_device::segmented_addr(UINT32 addr)
{
	return ((addr & 0x7f000000) >> 8) | (addr & 0xffff);
}

UINT32 z8002_device::addr_from_reg(int regno)
{
	if (segmented_mode())
		return segmented_addr(RL(regno));
	else
		return RW(regno);
}

void z8002_device::addr_to_reg(int regno, UINT32 addr)
{
	if (segmented_mode()) {
		UINT32 segaddr = make_segmented_addr(addr);
		RW(regno) = (RW(regno) & 0x80ff) | ((segaddr >> 16) & 0x7f00);
		RW(regno | 1) = segaddr & 0xffff;
	}
	else
		RW(regno) = addr;
}

void z8002_device::add_to_addr_reg(int regno, UINT16 addend)
{
	if (segmented_mode())
		regno |= 1;
	RW(regno) += addend;
}

void z8002_device::sub_from_addr_reg(int regno, UINT16 subtrahend)
{
	if (segmented_mode())
		regno |= 1;
	RW(regno) -= subtrahend;
}

void z8002_device::set_pc(UINT32 addr)
{
	if (segmented_mode())
		m_pc = addr;
	else
		m_pc = (m_pc & 0xffff0000) | (addr & 0xffff);
}

void z8002_device::PUSHW(UINT8 dst, UINT16 value)
{
	if (segmented_mode())
		RW(dst | 1) -= 2;
	else
		RW(dst) -= 2;
	WRMEM_W(AS_DATA, addr_from_reg(dst), value);
}

UINT16 z8002_device::POPW(UINT8 src)
{
	UINT16 result = RDMEM_W(AS_DATA, addr_from_reg(src));
	if (segmented_mode())
		RW(src | 1) += 2;
	else
		RW(src) += 2;
	return result;
}

void z8002_device::PUSHL(UINT8 dst, UINT32 value)
{
	if (segmented_mode())
		RW(dst | 1) -= 4;
	else
		RW(dst) -= 4;
	WRMEM_L(AS_DATA,  addr_from_reg(dst), value);
}

UINT32 z8002_device::POPL(UINT8 src)
{
	UINT32 result = RDMEM_L(AS_DATA, addr_from_reg(src));
	if (segmented_mode())
		RW(src | 1) += 4;
	else
		RW(src) += 4;
	return result;
}

/* check zero and sign flag for byte, word and long results */
#define CHK_XXXB_ZS if (!result) SET_Z; else if ((INT8) result < 0) SET_S
#define CHK_XXXW_ZS if (!result) SET_Z; else if ((INT16)result < 0) SET_S
#define CHK_XXXL_ZS if (!result) SET_Z; else if ((INT32)result < 0) SET_S
#define CHK_XXXQ_ZS if (!result) SET_Z; else if ((INT64)result < 0) SET_S

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
#define CHECK_PRIVILEGED_INSTR() if (!(m_fcw & F_S_N)) { m_irq_req = Z8000_TRAP; return; }

/* if no EPU is present (it isn't), raise an extended intstuction trap */
#define CHECK_EXT_INSTR()  if (!(m_fcw & F_EPU)) { m_irq_req = Z8000_EPU; return; }


/******************************************
 add byte
 flags:  CZSVDH
 ******************************************/
UINT8 z8002_device::ADDB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest + value;
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
UINT16 z8002_device::ADDW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest + value;
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
UINT32 z8002_device::ADDL(UINT32 dest, UINT32 value)
{
	UINT32 result = dest + value;
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
UINT8 z8002_device::ADCB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest + value + GET_C;
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
UINT16 z8002_device::ADCW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest + value + GET_C;
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
UINT8 z8002_device::SUBB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value;
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
UINT16 z8002_device::SUBW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value;
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
UINT32 z8002_device::SUBL(UINT32 dest, UINT32 value)
{
	UINT32 result = dest - value;
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
UINT8 z8002_device::SBCB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value - GET_C;
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
UINT16 z8002_device::SBCW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value - GET_C;
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
UINT8 z8002_device::ORB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest | value;
	CLR_ZSP;        /* first clear Z, S, P/V flags             */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical or word
 flags:  -ZS---
 ******************************************/
UINT16 z8002_device::ORW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest | value;
	CLR_ZS;         /* first clear Z, and S flags              */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 logical and byte
 flags:  -ZSP--
 ******************************************/
UINT8 z8002_device::ANDB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest & value;
	CLR_ZSP;        /* first clear Z,S and P/V flags           */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical and word
 flags:  -ZS---
 ******************************************/
UINT16 z8002_device::ANDW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest & value;
	CLR_ZS;         /* first clear Z and S flags               */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 logical exclusive or byte
 flags:  -ZSP--
 ******************************************/
UINT8 z8002_device::XORB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest ^ value;
	CLR_ZSP;        /* first clear Z, S and P/V flags          */
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical exclusive or word
 flags:  -ZS---
 ******************************************/
UINT16 z8002_device::XORW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest ^ value;
	CLR_ZS;         /* first clear Z and S flags               */
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}


/******************************************
 compare byte
 flags:  CZSV--
 ******************************************/
void z8002_device::CPB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value;
	CLR_CZSV;       /* first clear C, Z, S and P/V flags       */
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBB_V;
}

/******************************************
 compare word
 flags:  CZSV--
 ******************************************/
void z8002_device::CPW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value;
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBW_V;
}

/******************************************
 compare long
 flags:  CZSV--
 ******************************************/
void z8002_device::CPL(UINT32 dest, UINT32 value)
{
	UINT32 result = dest - value;
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C;     /* set C if result underflowed             */
	CHK_SUBL_V;
}

/******************************************
 complement byte
 flags: -ZSP--
 ******************************************/
UINT8 z8002_device::COMB(UINT8 dest)
{
	UINT8 result = ~dest;
	CLR_ZSP;
	CHK_XXXB_ZSP;   /* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 complement word
 flags: -ZS---
 ******************************************/
UINT16 z8002_device::COMW(UINT16 dest)
{
	UINT16 result = ~dest;
	CLR_ZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 negate byte
 flags:  CZSV--
 ******************************************/
UINT8 z8002_device::NEGB(UINT8 dest)
{
	UINT8 result = (UINT8) -dest;
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
UINT16 z8002_device::NEGW(UINT16 dest)
{
	UINT16 result = (UINT16) -dest;
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
void z8002_device::TESTB(UINT8 result)
{
	CLR_ZSP;
	CHK_XXXB_ZSP;   /* set Z and S flags for result byte       */
}

/******************************************
 test word
 flags:  -ZS---
 ******************************************/
void z8002_device::TESTW(UINT16 dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S16) SET_S;
}

/******************************************
 test long
 flags:  -ZS---
 ******************************************/
void z8002_device::TESTL(UINT32 dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S32) SET_S;
}

/******************************************
 increment byte
 flags: -ZSV--
 ******************************************/
UINT8 z8002_device::INCB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest + value;
	CLR_ZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDB_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 increment word
 flags: -ZSV--
 ******************************************/
UINT16 z8002_device::INCW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest + value;
	CLR_ZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDW_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 decrement byte
 flags: -ZSV--
 ******************************************/
UINT8 z8002_device::DECB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value;
	CLR_ZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBB_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 decrement word
 flags: -ZSV--
 ******************************************/
UINT16 z8002_device::DECW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value;
	CLR_ZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBW_V;     /* set V if result overflowed              */
	return result;
}

/******************************************
 multiply words
 flags:  CZSV--
 ******************************************/
UINT32 z8002_device::MULTW(UINT16 dest, UINT16 value)
{
	UINT32 result = (INT32)(INT16)dest * (INT16)value;
	CLR_CZSV;
	CHK_XXXL_ZS;
	if(!value)
	{
		/* multiplication with zero is faster */
		m_icount += (70-18);
	}
	if((INT32)result < -0x7fff || (INT32)result >= 0x7fff) SET_C;
	return result;
}

/******************************************
 multiply longs
 flags:  CZSV--
 ******************************************/
UINT64 z8002_device::MULTL(UINT32 dest, UINT32 value)
{
	UINT64 result = (INT64)(INT32)dest * (INT32)value;
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
	if((INT64)result < -0x7fffffffL || (INT64)result >= 0x7fffffffL) SET_C;
	return result;
}

/******************************************
 divide long by word
 flags: CZSV--
 ******************************************/
UINT32 z8002_device::DIVW(UINT32 dest, UINT16 value)
{
	UINT32 result = dest;
	UINT16 remainder = 0;
	CLR_CZSV;
	if (value)
	{
		UINT16 qsign = ((dest >> 16) ^ value) & S16;
		UINT16 rsign = (dest >> 16) & S16;
		if ((INT32)dest < 0) dest = -dest;
		if ((INT16)value < 0) value = -value;
		result = dest / value;
		remainder = dest % value;
		if (qsign) result = -result;
		if (rsign) remainder = -remainder;
		if ((INT32)result < -0x8000 || (INT32)result > 0x7fff)
		{
			INT32 temp = (INT32)result >> 1;
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
		result = ((UINT32)remainder << 16) | (result & 0xffff);
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
UINT64 z8002_device::DIVL(UINT64 dest, UINT32 value)
{
	UINT64 result = dest;
	UINT32 remainder = 0;
	CLR_CZSV;
	if (value)
	{
		UINT32 qsign = ((dest >> 32) ^ value) & S32;
		UINT32 rsign = (dest >> 32) & S32;
		if ((INT64)dest < 0) dest = -dest;
		if ((INT32)value < 0) value = -value;
		result = dest / value;
		remainder = dest % value;
		if (qsign) result = -result;
		if (rsign) remainder = -remainder;
		if ((INT64)result < -0x80000000 || (INT64)result > 0x7fffffff)
		{
			INT64 temp = (INT64)result >> 1;
			SET_V;
			if (temp >= -0x80000000 && temp <= 0x7fffffff)
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
		result = ((UINT64)remainder << 32) | (result & 0xffffffff);
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
UINT8 z8002_device::RLB(UINT8 dest, UINT8 twice)
{
	UINT8 result = (dest << 1) | (dest >> 7);
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
UINT16 z8002_device::RLW(UINT16 dest, UINT8 twice)
{
	UINT16 result = (dest << 1) | (dest >> 15);
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
UINT8 z8002_device::RLCB(UINT8 dest, UINT8 twice)
{
	UINT8 c = dest & S08;
	UINT8 result = (dest << 1) | GET_C;
	CLR_CZSV;
	if (twice) {
		UINT8 c1 = c >> 7;
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
UINT16 z8002_device::RLCW(UINT16 dest, UINT8 twice)
{
	UINT16 c = dest & S16;
	UINT16 result = (dest << 1) | GET_C;
	CLR_CZSV;
	if (twice) {
		UINT16 c1 = c >> 15;
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
UINT8 z8002_device::RRB(UINT8 dest, UINT8 twice)
{
	UINT8 result = (dest >> 1) | (dest << 7);
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
UINT16 z8002_device::RRW(UINT16 dest, UINT8 twice)
{
	UINT16 result = (dest >> 1) | (dest << 15);
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
UINT8 z8002_device::RRCB(UINT8 dest, UINT8 twice)
{
	UINT8 c = dest & 1;
	UINT8 result = (dest >> 1) | (GET_C << 7);
	CLR_CZSV;
	if (twice) {
		UINT8 c1 = c << 7;
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
UINT16 z8002_device::RRCW(UINT16 dest, UINT8 twice)
{
	UINT16 c = dest & 1;
	UINT16 result = (dest >> 1) | (GET_C << 15);
	CLR_CZSV;
	if (twice) {
		UINT16 c1 = c << 15;
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
UINT8 z8002_device::SDAB(UINT8 dest, INT8 count)
{
	INT8 result = (INT8) dest;
	UINT8 c = 0;
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
	return (UINT8)result;
}

/******************************************
 shift dynamic arithmetic word
 flags:  CZSV--
 ******************************************/
UINT16 z8002_device::SDAW(UINT16 dest, INT8 count)
{
	INT16 result = (INT16) dest;
	UINT16 c = 0;
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
	return (UINT16)result;
}

/******************************************
 shift dynamic arithmetic long
 flags:  CZSV--
 ******************************************/
UINT32 z8002_device::SDAL(UINT32 dest, INT8 count)
{
	INT32 result = (INT32) dest;
	UINT32 c = 0;
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
	return (UINT32) result;
}

/******************************************
 shift dynamic logic byte
 flags:  CZSV--
 ******************************************/
UINT8 z8002_device::SDLB(UINT8 dest, INT8 count)
{
	UINT8 result = dest;
	UINT8 c = 0;
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
UINT16 z8002_device::SDLW(UINT16 dest, INT8 count)
{
	UINT16 result = dest;
	UINT16 c = 0;
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
UINT32 z8002_device::SDLL(UINT32 dest, INT8 count)
{
	UINT32 result = dest;
	UINT32 c = 0;
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
UINT8 z8002_device::SLAB(UINT8 dest, UINT8 count)
{
	UINT8 c = (count) ? (dest << (count - 1)) & S08 : 0;
	UINT8 result = (UINT8)((INT8)dest << count);
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
UINT16 z8002_device::SLAW(UINT16 dest, UINT8 count)
{
	UINT16 c = (count) ? (dest << (count - 1)) & S16 : 0;
	UINT16 result = (UINT16)((INT16)dest << count);
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
UINT32 z8002_device::SLAL(UINT32 dest, UINT8 count)
{
	UINT32 c = (count) ? (dest << (count - 1)) & S32 : 0;
	UINT32 result = (UINT32)((INT32)dest << count);
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
UINT8 z8002_device::SLLB(UINT8 dest, UINT8 count)
{
	UINT8 c = (count) ? (dest << (count - 1)) & S08 : 0;
	UINT8 result = dest << count;
	CLR_CZS;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift left logic word
 flags:  CZS---
 ******************************************/
UINT16 z8002_device::SLLW(UINT16 dest, UINT8 count)
{
	UINT16 c = (count) ? (dest << (count - 1)) & S16 : 0;
	UINT16 result = dest << count;
	CLR_CZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift left logic long
 flags:  CZS---
 ******************************************/
UINT32 z8002_device::SLLL(UINT32 dest, UINT8 count)
{
	UINT32 c = (count) ? (dest << (count - 1)) & S32 : 0;
	UINT32 result = dest << count;
	CLR_CZS;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right arithmetic byte
 flags:  CZSV--
 ******************************************/
UINT8 z8002_device::SRAB(UINT8 dest, UINT8 count)
{
	UINT8 c = (count) ? ((INT8)dest >> (count - 1)) & 1 : 0;
	UINT8 result = (UINT8)((INT8)dest >> count);
	CLR_CZSV;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right arithmetic word
 flags:  CZSV--
 ******************************************/
UINT16 z8002_device::SRAW(UINT16 dest, UINT8 count)
{
	UINT8 c = (count) ? ((INT16)dest >> (count - 1)) & 1 : 0;
	UINT16 result = (UINT16)((INT16)dest >> count);
	CLR_CZSV;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right arithmetic long
 flags:  CZSV--
 ******************************************/
UINT32 z8002_device::SRAL(UINT32 dest, UINT8 count)
{
	UINT8 c = (count) ? ((INT32)dest >> (count - 1)) & 1 : 0;
	UINT32 result = (UINT32)((INT32)dest >> count);
	CLR_CZSV;
	CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right logic byte
 flags:  CZSV--
 ******************************************/
UINT8 z8002_device::SRLB(UINT8 dest, UINT8 count)
{
	UINT8 c = (count) ? (dest >> (count - 1)) & 1 : 0;
	UINT8 result = dest >> count;
	CLR_CZS;
	CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right logic word
 flags:  CZSV--
 ******************************************/
UINT16 z8002_device::SRLW(UINT16 dest, UINT8 count)
{
	UINT8 c = (count) ? (dest >> (count - 1)) & 1 : 0;
	UINT16 result = dest >> count;
	CLR_CZS;
	CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	if (c) SET_C;
	return result;
}

/******************************************
 shift right logic long
 flags:  CZSV--
 ******************************************/
UINT32 z8002_device::SRLL(UINT32 dest, UINT8 count)
{
	UINT8 c = (count) ? (dest >> (count - 1)) & 1 : 0;
	UINT32 result = dest >> count;
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
	RB(dst) = ADDB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	RW(dst) = ADDW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	RB(dst) = SUBB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src))); /* EHC */
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
	RW(dst) = SUBW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	RB(dst) = ORB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	RW(dst) = ORW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	RB(dst) = ANDB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	RW(dst) = ANDW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	RB(dst) = XORB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	RW(dst) = XORW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
}

/******************************************
 comb    @rd
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z0C_ddN0_0000()
{
	GET_DST(OP0,NIB3);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_B(AS_DATA, addr, COMB(RDMEM_B(AS_DATA, addr)));
}

/******************************************
 cpb     @rd,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0C_ddN0_0001_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	CPB(RDMEM_B(AS_DATA, addr_from_reg(dst)), imm8); // @@@done
}

/******************************************
 negb    @rd
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0C_ddN0_0010()
{
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_B(AS_DATA,  addr, NEGB(RDMEM_B(AS_DATA, addr)));
}

/******************************************
 testb   @rd
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z0C_ddN0_0100()
{
	GET_DST(OP0,NIB2);
	TESTB(RDMEM_B(AS_DATA, addr_from_reg(dst)));
}

/******************************************
 ldb     @rd,imm8
 flags:  ------
 ******************************************/
void z8002_device::Z0C_ddN0_0101_imm8()
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	WRMEM_B(AS_DATA, addr_from_reg(dst), imm8);
}

/******************************************
 tsetb   @rd
 flags:  --S---
 ******************************************/
void z8002_device::Z0C_ddN0_0110()
{
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	if (RDMEM_B(AS_DATA, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(AS_DATA, addr, 0xff);
}

/******************************************
 clrb    @rd
 flags:  ------
 ******************************************/
void z8002_device::Z0C_ddN0_1000()
{
	GET_DST(OP0,NIB2);
	WRMEM_B(AS_DATA,  addr_from_reg(dst), 0);
}

/******************************************
 com     @rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z0D_ddN0_0000()
{
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_W(AS_DATA, addr, COMW(RDMEM_W(AS_DATA, addr)));
}

/******************************************
 cp      @rd,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0D_ddN0_0001_imm16()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	CPW(RDMEM_W(AS_DATA, addr_from_reg(dst)), imm16);
}

/******************************************
 neg     @rd
 flags:  CZSV--
 ******************************************/
void z8002_device::Z0D_ddN0_0010()
{
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_W(AS_DATA, addr, NEGW(RDMEM_W(AS_DATA, addr)));
}

/******************************************
 test    @rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z0D_ddN0_0100()
{
	GET_DST(OP0,NIB2);
	TESTW(RDMEM_W(AS_DATA, addr_from_reg(dst)));
}

/******************************************
 ld      @rd,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z0D_ddN0_0101_imm16()
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	WRMEM_W(AS_DATA, addr_from_reg(dst), imm16);
}

/******************************************
 tset    @rd
 flags:  --S---
 ******************************************/
void z8002_device::Z0D_ddN0_0110()
{
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	if (RDMEM_W(AS_DATA, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(AS_DATA, addr, 0xffff);
}

/******************************************
 clr     @rd
 flags:  ------
 ******************************************/
void z8002_device::Z0D_ddN0_1000()
{
	GET_DST(OP0,NIB2);
	WRMEM_W(AS_DATA, addr_from_reg(dst), 0);
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
	LOG(("Z8K '%s' %04x: ext0e  $%02x\n", tag().c_str(), m_pc, imm8));
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
	LOG(("Z8K '%s' %04x: ext0f  $%02x\n", tag().c_str(), m_pc, imm8));
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
	CPL(RL(dst), RDMEM_L(AS_DATA, addr_from_reg(src)));
}

/******************************************
 pushl   @rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z11_ddN0_ssN0()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL(dst, RDMEM_L(AS_DATA, addr_from_reg(src)));
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
	RL(dst) = SUBL(RL(dst), RDMEM_L(AS_DATA, addr_from_reg(src)));
}

/******************************************
 push    @rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z13_ddN0_ssN0()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(dst, RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	RL(dst) = RDMEM_L( AS_DATA, addr_from_reg(src));
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
	RL(dst) = ADDL(RL(dst), RDMEM_L(AS_DATA, addr_from_reg(src)));
}

/******************************************
 pop     @rd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z17_ssN0_ddN0()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	WRMEM_W(AS_DATA, addr_from_reg(dst), POPW(src));
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
	RL(dst) = MULTW(RL(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(AS_DATA, addr_from_reg(src)));
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
	RL(dst) = DIVW(RL(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
}

/******************************************
 testl   @rd
 flags:  -ZS---
 ******************************************/
void z8002_device::Z1C_ddN0_1000()
{
	GET_DST(OP0,NIB2);
	TESTL(RDMEM_L(AS_DATA, addr_from_reg(dst)));
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
	UINT32 addr = addr_from_reg(dst);
	while (cnt-- >= 0) {
		WRMEM_W(AS_DATA, addr, RW(src));
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
	UINT32 addr = addr_from_reg(src);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(AS_DATA, addr);
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
	WRMEM_L(AS_DATA,  addr_from_reg(dst), RL(src));
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
	if (segmented_mode())
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
	RB(dst) = RDMEM_B(AS_DATA, addr_from_reg(src));
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
	RW(dst) = RDMEM_W(AS_DATA, addr_from_reg(src));
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
	UINT32 addr = addr_from_reg(dst);
	WRMEM_B(AS_DATA, addr, RDMEM_B(AS_DATA, addr) & ~bit);
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
	UINT32 addr = addr_from_reg(dst);
	WRMEM_W(AS_DATA, addr, RDMEM_W(AS_DATA, addr) & ~bit);
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
	UINT32 addr = addr_from_reg(dst);
	WRMEM_B(AS_DATA, addr, RDMEM_B(AS_DATA, addr) | bit);
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
	UINT32 addr = addr_from_reg(dst);
	WRMEM_W(AS_DATA, addr, RDMEM_W(AS_DATA, addr) | bit);
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
	if (RDMEM_B(AS_DATA, addr_from_reg(dst)) & bit) CLR_Z; else SET_Z;
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
	if (RDMEM_W(AS_DATA, addr_from_reg(dst)) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z28_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_B(AS_DATA,  addr, INCB(RDMEM_B(AS_DATA, addr), i4p1));
}

/******************************************
 inc     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z29_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_W(AS_DATA, addr, INCW(RDMEM_W(AS_DATA, addr), i4p1));
}

/******************************************
 decb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z2A_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_B(AS_DATA, addr, DECB(RDMEM_B(AS_DATA, addr), i4p1));
}

/******************************************
 dec     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z2B_ddN0_imm4m1()
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	UINT32 addr = addr_from_reg(dst);
	WRMEM_W(AS_DATA, addr, DECW(RDMEM_W(AS_DATA, addr), i4p1));
}

/******************************************
 exb     rbd,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z2C_ssN0_dddd()
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	UINT32 addr = addr_from_reg(src);
	UINT8 tmp = RDMEM_B( AS_DATA, addr);
	WRMEM_B(AS_DATA, addr, RB(dst));
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
	UINT32 addr = addr_from_reg(src);
	UINT16 tmp = RDMEM_W(AS_DATA, addr);
	WRMEM_W(AS_DATA, addr, RW(dst));
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
	WRMEM_B(AS_DATA,  addr_from_reg(dst), RB(src));
}

/******************************************
 ld      @rd,rs
 flags:  ------
 ******************************************/
void z8002_device::Z2F_ddN0_ssss()
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_W(AS_DATA,  addr_from_reg(dst), RW(src));
}

/******************************************
 ldrb    rbd,dsp16
 flags:  ------
 ******************************************/
void z8002_device::Z30_0000_dddd_dsp16()
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RB(dst) = RDMEM_B(AS_PROGRAM, dsp16);
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
	idx16 = addr_add(addr_from_reg(src), idx16);
	RB(dst) = RDMEM_B(AS_DATA, idx16);
}

/******************************************
 ldr     rd,dsp16
 flags:  ------
 ******************************************/
void z8002_device::Z31_0000_dddd_dsp16()
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RW(dst) = RDMEM_W(AS_PROGRAM, dsp16);
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
	idx16 = addr_add(addr_from_reg(src), idx16);
	RW(dst) = RDMEM_W(AS_DATA, idx16);
}

/******************************************
 ldrb    dsp16,rbs
 flags:  ------
 ******************************************/
void z8002_device::Z32_0000_ssss_dsp16()
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_B(AS_PROGRAM,  dsp16, RB(src));
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
	idx16 = addr_add(addr_from_reg(dst), idx16);
	WRMEM_B(AS_DATA,  idx16, RB(src));
}

/******************************************
 ldr     dsp16,rs
 flags:  ------
 ******************************************/
void z8002_device::Z33_0000_ssss_dsp16()
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_W(AS_PROGRAM,  dsp16, RW(src));
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
	idx16 = addr_add(addr_from_reg(dst), idx16);
	WRMEM_W(AS_DATA,  idx16, RW(src));
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
	if (segmented_mode()) {
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
	RL(dst) = RDMEM_L(AS_PROGRAM, dsp16);
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
	idx16 = addr_add(addr_from_reg(src), idx16);
	RL(dst) = RDMEM_L(AS_DATA, idx16);
}

/******************************************
 bpt
 flags:  ------
 ******************************************/
void z8002_device::Z36_0000_0000()
{
	/* execute break point trap m_irq_req */
	m_irq_req = Z8000_TRAP;
}

/******************************************
 rsvd36
 flags:  ------
 ******************************************/
void z8002_device::Z36_imm8()
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd36 $%02x\n", tag().c_str(), m_pc, imm8));
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
	WRMEM_L(AS_PROGRAM,  dsp16, RL(src));
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
	idx16 = addr_add(addr_from_reg(dst), idx16);
	WRMEM_L(AS_DATA,  idx16, RL(src));
}

/******************************************
 rsvd38
 flags:  ------
 ******************************************/
void z8002_device::Z38_imm8()
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd38 $%02x\n", tag().c_str(), m_pc, imm8));
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
	UINT16 fcw;
	if (segmented_mode()) {
		UINT32 addr = addr_from_reg(src);
		fcw = RDMEM_W(AS_DATA, addr + 2);
		set_pc(segmented_addr(RDMEM_L(AS_DATA, addr + 4)));
	}
	else {
		fcw = RDMEM_W(AS_DATA,  RW(src));
		set_pc(RDMEM_W(AS_DATA, (UINT16)(RW(src) + 2)));
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
	WRMEM_B(AS_DATA, addr_from_reg(dst), RDPORT_B( 0, RW(src)));
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
	WRMEM_B(AS_DATA,  RW(dst), RDPORT_B( 1, RW(src)));
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
	WRPORT_B( 0, RW(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	WRPORT_B( 1, RW(dst), RDMEM_B(AS_DATA, RW(src)));
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
	WRMEM_B(AS_DATA,  RW(dst), RDPORT_B( 0, RW(src)));
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
	WRMEM_B(AS_DATA,  RW(dst), RDPORT_B( 1, RW(src)));
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
	WRPORT_B( 0, RW(dst), RDMEM_B(AS_DATA, RW(src)));
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
	WRPORT_B( 1, RW(dst), RDMEM_B(AS_DATA, RW(src)));
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
	WRMEM_W(AS_DATA,  RW(dst), RDPORT_W( 0, RW(src)));
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
	WRMEM_W(AS_DATA,  RW(dst), RDPORT_W( 1, RW(src)));
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
	WRPORT_W( 0, RW(dst), RDMEM_W(AS_DATA, RW(src)));
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
	WRPORT_W( 1, RW(dst), RDMEM_W(AS_DATA, RW(src)));
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
	WRMEM_W(AS_DATA,  RW(dst), RDPORT_W( 0, RW(src)));
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
	WRMEM_W(AS_DATA,  RW(dst), RDPORT_W( 1, RW(src)));
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
	WRPORT_W( 0, RW(dst), RDMEM_W(AS_DATA, RW(src)));
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
	WRPORT_W( 1, RW(dst), RDMEM_W(AS_DATA, RW(src)));
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
	RB(dst) = ADDB(RB(dst), RDMEM_B(AS_DATA, addr));
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
	RB(dst) = ADDB(RB(dst), RDMEM_B(AS_DATA, addr));
}

/******************************************
 add     rd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z41_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ADDW(RW(dst), RDMEM_W(AS_DATA, addr)); /* EHC */
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
	RW(dst) = ADDW(RW(dst), RDMEM_W(AS_DATA, addr));    /* ASG */
}

/******************************************
 subb    rbd,addr
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z42_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = SUBB(RB(dst), RDMEM_B(AS_DATA, addr)); /* EHC */
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
	RB(dst) = SUBB(RB(dst), RDMEM_B(AS_DATA, addr));
}

/******************************************
 sub     rd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z43_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = SUBW(RW(dst), RDMEM_W(AS_DATA, addr));
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
	RW(dst) = SUBW(RW(dst), RDMEM_W(AS_DATA, addr));
}

/******************************************
 orb     rbd,addr
 flags:  CZSP--
 ******************************************/
void z8002_device::Z44_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ORB(RB(dst), RDMEM_B(AS_DATA, addr));
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
	RB(dst) = ORB(RB(dst), RDMEM_B(AS_DATA, addr));
}

/******************************************
 or      rd,addr
 flags:  CZS---
 ******************************************/
void z8002_device::Z45_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ORW(RW(dst), RDMEM_W(AS_DATA, addr));
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
	RW(dst) = ORW(RW(dst), RDMEM_W(AS_DATA, addr));
}

/******************************************
 andb    rbd,addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z46_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ANDB(RB(dst), RDMEM_B(AS_DATA, addr));
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
	RB(dst) = ANDB(RB(dst), RDMEM_B(AS_DATA, addr));
}

/******************************************
 and     rd,addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z47_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ANDW(RW(dst), RDMEM_W(AS_DATA, addr));
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
	RW(dst) = ANDW(RW(dst), RDMEM_W(AS_DATA, addr));
}

/******************************************
 xorb    rbd,addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z48_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = XORB(RB(dst), RDMEM_B(AS_DATA, addr));
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
	RB(dst) = XORB(RB(dst), RDMEM_B(AS_DATA, addr));
}

/******************************************
 xor     rd,addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z49_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = XORW(RW(dst), RDMEM_W(AS_DATA, addr));
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
	RW(dst) = XORW(RW(dst), RDMEM_W(AS_DATA, addr));
}

/******************************************
 cpb     rbd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4A_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPB(RB(dst), RDMEM_B(AS_DATA, addr));
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
	CPB(RB(dst), RDMEM_B(AS_DATA, addr));
}

/******************************************
 cp      rd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4B_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPW(RW(dst), RDMEM_W(AS_DATA, addr));
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
	CPW(RW(dst), RDMEM_W(AS_DATA, addr));
}

/******************************************
 comb    addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z4C_0000_0000_addr()
{
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, COMB(RDMEM_W(AS_DATA, addr)));
}

/******************************************
 cpb     addr,imm8
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4C_0000_0001_addr_imm8()
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	CPB(RDMEM_B(AS_DATA, addr), imm8);
}

/******************************************
 negb    addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4C_0000_0010_addr()
{
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, NEGB(RDMEM_B(AS_DATA, addr)));
}

/******************************************
 testb   addr
 flags:  -ZSP--
 ******************************************/
void z8002_device::Z4C_0000_0100_addr()
{
	GET_ADDR(OP1);
	TESTB(RDMEM_B(AS_DATA, addr));
}

/******************************************
 ldb     addr,imm8
 flags:  ------
 ******************************************/
void z8002_device::Z4C_0000_0101_addr_imm8()
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	WRMEM_B(AS_DATA,  addr, imm8);
}

/******************************************
 tsetb   addr
 flags:  --S---
 ******************************************/
void z8002_device::Z4C_0000_0110_addr()
{
	GET_ADDR(OP1);
	if (RDMEM_B(AS_DATA, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(AS_DATA, addr, 0xff);
}

/******************************************
 clrb    addr
 flags:  ------
 ******************************************/
void z8002_device::Z4C_0000_1000_addr()
{
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, 0);
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
	WRMEM_B(AS_DATA,  addr, COMB(RDMEM_B(AS_DATA, addr)));
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
	CPB(RDMEM_B(AS_DATA, addr), imm8);
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
	WRMEM_B(AS_DATA,  addr, NEGB(RDMEM_B(AS_DATA, addr)));
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
	TESTB(RDMEM_B(AS_DATA, addr));
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
	WRMEM_B(AS_DATA,  addr, imm8);
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
	if (RDMEM_B(AS_DATA, addr) & S08) SET_S; else CLR_S;
	WRMEM_B(AS_DATA, addr, 0xff);
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
	WRMEM_B(AS_DATA,  addr, 0);
}

/******************************************
 com     addr
 flags:  -ZS---
 ******************************************/
void z8002_device::Z4D_0000_0000_addr()
{
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, COMW(RDMEM_W(AS_DATA, addr)));
}

/******************************************
 cp      addr,imm16
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4D_0000_0001_addr_imm16()
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	CPW(RDMEM_W(AS_DATA, addr), imm16);
}

/******************************************
 neg     addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z4D_0000_0010_addr()
{
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, NEGW(RDMEM_W(AS_DATA, addr)));
}

/******************************************
 test    addr
 flags:  ------
 ******************************************/
void z8002_device::Z4D_0000_0100_addr()
{
	GET_ADDR(OP1);
	TESTW(RDMEM_W(AS_DATA, addr));
}

/******************************************
 ld      addr,imm16
 flags:  ------
 ******************************************/
void z8002_device::Z4D_0000_0101_addr_imm16()
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	WRMEM_W(AS_DATA,  addr, imm16);
}

/******************************************
 tset    addr
 flags:  --S---
 ******************************************/
void z8002_device::Z4D_0000_0110_addr()
{
	GET_ADDR(OP1);
	if (RDMEM_W(AS_DATA, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(AS_DATA, addr, 0xffff);
}

/******************************************
 clr     addr
 flags:  ------
 ******************************************/
void z8002_device::Z4D_0000_1000_addr()
{
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, 0);
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
	WRMEM_W(AS_DATA,  addr, COMW(RDMEM_W(AS_DATA, addr)));
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
	CPW(RDMEM_W(AS_DATA, addr), imm16);
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
	WRMEM_W(AS_DATA,  addr, NEGW(RDMEM_W(AS_DATA, addr)));
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
	TESTW(RDMEM_W(AS_DATA, addr));
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
	WRMEM_W(AS_DATA,  addr, imm16);
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
	if (RDMEM_W(AS_DATA, addr) & S16) SET_S; else CLR_S;
	WRMEM_W(AS_DATA, addr, 0xffff);
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
	WRMEM_W(AS_DATA,  addr, 0);
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
	WRMEM_B(AS_DATA,  addr, RB(src));
}

/******************************************
 cpl     rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z50_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPL(RL(dst), RDMEM_L(AS_DATA, addr));
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
	CPL(RL(dst), RDMEM_L(AS_DATA, addr));
}

/******************************************
 pushl   @rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z51_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHL(dst, RDMEM_L(AS_DATA, addr));
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
	PUSHL(dst, RDMEM_L(AS_DATA, addr));
}

/******************************************
 subl    rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z52_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = SUBL(RL(dst), RDMEM_L(AS_DATA, addr));
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
	RL(dst) = SUBL(RL(dst), RDMEM_L(AS_DATA, addr));
}

/******************************************
 push    @rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z53_ddN0_0000_addr()
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHW(dst, RDMEM_W(AS_DATA, addr));
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
	PUSHW(dst, RDMEM_W(AS_DATA, addr));
}

/******************************************
 ldl     rrd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z54_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = RDMEM_L(AS_DATA, addr);
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
	RL(dst) = RDMEM_L(AS_DATA, addr);
}

/******************************************
 popl    addr,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z55_ssN0_0000_addr()
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_L(AS_DATA,  addr, POPL(src));
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
	WRMEM_L(AS_DATA,  addr, POPL(src));
}

/******************************************
 addl    rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z56_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = ADDL(RL(dst), RDMEM_L(AS_DATA, addr));
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
	RL(dst) = ADDL(RL(dst), RDMEM_L(AS_DATA, addr));
}

/******************************************
 pop     addr,@rs
 flags:  ------
 ******************************************/
void z8002_device::Z57_ssN0_0000_addr()
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, POPW(src));
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
	WRMEM_W(AS_DATA,  addr, POPW(src));
}

/******************************************
 multl   rqd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z58_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = MULTL(RQ(dst), RDMEM_L(AS_DATA, addr));
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
	RQ(dst) = MULTL(RQ(dst), RDMEM_L(AS_DATA, addr));
}

/******************************************
 mult    rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z59_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = MULTW(RL(dst), RDMEM_W(AS_DATA, addr));
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
	RL(dst) = MULTW(RL(dst), RDMEM_W(AS_DATA, addr));
}

/******************************************
 divl    rqd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z5A_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(AS_DATA, addr));
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
	RQ(dst) = DIVL(RQ(dst), RDMEM_L(AS_DATA, addr));
}

/******************************************
 div     rrd,addr
 flags:  CZSV--
 ******************************************/
void z8002_device::Z5B_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = DIVW(RL(dst), RDMEM_W(AS_DATA, addr));
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
	RL(dst) = DIVW(RL(dst), RDMEM_W(AS_DATA, addr));
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
		RW(dst) = RDMEM_W(AS_DATA, addr);
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
	TESTL(RDMEM_L(AS_DATA, addr));
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
		WRMEM_W(AS_DATA,  addr, RW(src));
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
	TESTL(RDMEM_L(AS_DATA, addr));
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
		WRMEM_W(AS_DATA,  addr, RW(src));
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
		RW(dst) = RDMEM_W(AS_DATA, addr);
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
	WRMEM_L(AS_DATA,  addr, RL(src));
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
	WRMEM_L(AS_DATA,  addr, RL(src));
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
	if (segmented_mode())
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
	if (segmented_mode())
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
	RB(dst) = RDMEM_B(AS_DATA, addr);
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
	RB(dst) = RDMEM_B(AS_DATA, addr);
}

/******************************************
 ld      rd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z61_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = RDMEM_W(AS_DATA, addr);
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
	RW(dst) = RDMEM_W(AS_DATA, addr);
}

/******************************************
 resb    addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z62_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, RDMEM_B(AS_DATA, addr) & ~bit);
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
	WRMEM_B(AS_DATA,  addr, RDMEM_B(AS_DATA, addr) & ~bit);
}

/******************************************
 res     addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z63_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, RDMEM_W(AS_DATA, addr) & ~bit);
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
	WRMEM_W(AS_DATA,  addr, RDMEM_W(AS_DATA, addr) & ~bit);
}

/******************************************
 setb    addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z64_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, RDMEM_B(AS_DATA, addr) | bit);
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
	WRMEM_B(AS_DATA,  addr, RDMEM_B(AS_DATA, addr) | bit);
}

/******************************************
 set     addr,imm4
 flags:  ------
 ******************************************/
void z8002_device::Z65_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, RDMEM_W(AS_DATA, addr) | bit);
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
	WRMEM_W(AS_DATA,  addr, RDMEM_W(AS_DATA, addr) | bit);
}

/******************************************
 bitb    addr,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z66_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_B(AS_DATA, addr) & bit) CLR_Z; else SET_Z;
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
	if (RDMEM_B(AS_DATA, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr,imm4
 flags:  -Z----
 ******************************************/
void z8002_device::Z67_0000_imm4_addr()
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_W(AS_DATA, addr) & bit) CLR_Z; else SET_Z;
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
	if (RDMEM_W(AS_DATA, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z68_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, INCB(RDMEM_B(AS_DATA, addr), i4p1));
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
	WRMEM_B(AS_DATA,  addr, INCB(RDMEM_B(AS_DATA, addr), i4p1));
}

/******************************************
 inc     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z69_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, INCW(RDMEM_W(AS_DATA, addr), i4p1));
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
	WRMEM_W(AS_DATA,  addr, INCW(RDMEM_W(AS_DATA, addr), i4p1));
}

/******************************************
 decb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z6A_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(AS_DATA,  addr, DECB(RDMEM_B(AS_DATA, addr), i4p1));
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
	WRMEM_B(AS_DATA,  addr, DECB(RDMEM_B(AS_DATA, addr), i4p1));
}

/******************************************
 dec     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
void z8002_device::Z6B_0000_imm4m1_addr()
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, DECW(RDMEM_W(AS_DATA, addr), i4p1));
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
	WRMEM_W(AS_DATA,  addr, DECW(RDMEM_W(AS_DATA, addr), i4p1));
}

/******************************************
 exb     rbd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z6C_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	UINT8 tmp = RDMEM_B(AS_DATA, addr);
	WRMEM_B(AS_DATA, addr, RB(dst));
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
	UINT8 tmp;
	addr = addr_add(addr, RW(src));
	tmp = RDMEM_B(AS_DATA, addr);
	WRMEM_B(AS_DATA, addr, RB(dst));
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
	UINT16 tmp = RDMEM_W(AS_DATA, addr);
	WRMEM_W(AS_DATA,  addr, RW(dst));
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
	UINT16 tmp;
	addr = addr_add(addr, RW(src));
	tmp = RDMEM_W(AS_DATA, addr);
	WRMEM_W(AS_DATA,  addr, RW(dst));
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
	WRMEM_B(AS_DATA,  addr, RB(src));
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
	WRMEM_B(AS_DATA,  addr, RB(src));
}

/******************************************
 ld      addr,rs
 flags:  ------
 ******************************************/
void z8002_device::Z6F_0000_ssss_addr()
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(AS_DATA,  addr, RW(src));
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
	WRMEM_W(AS_DATA,  addr, RW(src));
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
	RB(dst) = RDMEM_B(AS_DATA, addr_add(addr_from_reg(src), RW(idx)));
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
	RW(dst) = RDMEM_W(AS_DATA, addr_add(addr_from_reg(src), RW(idx)));
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
	WRMEM_B(AS_DATA,  addr_add(addr_from_reg(dst), RW(idx)), RB(src));
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
	WRMEM_W(AS_DATA,  addr_add(addr_from_reg(dst), RW(idx)), RW(src));
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
	if (segmented_mode()) {
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
	RL(dst) = RDMEM_L(AS_DATA, addr_add(addr_from_reg(src), RW(idx)));
}

/******************************************
 lda     prd,addr
 flags:  ------
 ******************************************/
void z8002_device::Z76_0000_dddd_addr()
{
	GET_DST(OP0,NIB3);
	GET_ADDR_RAW(OP1);
	if (segmented_mode()) {
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
	UINT16 temp = RW(src);  // store src in case dst == src
	if (segmented_mode()) {
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
	WRMEM_L(AS_DATA,  addr_add(addr_from_reg(dst), RW(idx)), RL(src));
}

/******************************************
 rsvd78
 flags:  ------
 ******************************************/
void z8002_device::Z78_imm8()
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd78 $%02x\n", tag().c_str(), m_pc, imm8));
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
	UINT16 fcw;
	if (segmented_mode()) {
		fcw = RDMEM_W(AS_DATA,  addr + 2);
		set_pc(segmented_addr(RDMEM_L(AS_DATA, addr + 4)));
	}
	else {
		fcw = RDMEM_W(AS_DATA, addr);
		set_pc(RDMEM_W(AS_DATA, (UINT16)(addr + 2)));
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
	UINT16 fcw;
	addr = addr_add(addr, RW(src));
	if (segmented_mode()) {
		fcw = RDMEM_W(AS_DATA,  addr + 2);
		set_pc(segmented_addr(RDMEM_L(AS_DATA, addr + 4)));
	}
	else {
		fcw = RDMEM_W(AS_DATA, addr);
		m_pc    = RDMEM_W(AS_DATA, (UINT16)(addr + 2));
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
	m_irq_req |= Z8000_HALT;
	if (m_icount > 0) m_icount = 0;
}

/******************************************
 iret
 flags:  CZSVDH
 ******************************************/
void z8002_device::Z7B_0000_0000()
{
	UINT16 tag, fcw;
	CHECK_PRIVILEGED_INSTR();
	tag = POPW(SP);   /* get type tag */
	fcw = POPW(SP);   /* get m_fcw  */
	if (segmented_mode())
		set_pc(segmented_addr(POPL(SP)));
	else
		m_pc    = POPW(SP);   /* get m_pc   */
	CHANGE_FCW(fcw);       /* check for user/system mode change */
	LOG(("Z8K '%s' IRET tag $%04x, fcw $%04x, pc $%04x\n", this->tag().c_str(), tag, fcw, m_pc));
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
	UINT16 fcw = m_fcw;
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
	UINT16 fcw = m_fcw;
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
			LOG(("Z8K '%s' LDCTL R%d,%d\n", tag().c_str(), dst, imm3));
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
				UINT16 fcw;
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
			LOG(("Z8K '%s' LDCTL %d,R%d\n", tag().c_str(), imm3, src));
	}
}

/******************************************
 rsvd7e
 flags:  ------
 ******************************************/
void z8002_device::Z7E_imm8()
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd7e $%02x\n", tag().c_str(), m_pc, imm8));
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
	m_irq_req = Z8000_SYSCALL | imm8;

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
	LOG(("Z8K '%s' %04x: ext8e  $%02x\n", tag().c_str(), m_pc, imm8));
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
	LOG(("Z8K '%s' %04x: ext8f  $%02x\n", tag().c_str(), m_pc, imm8));
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
	LOG(("Z8K '%s' %04x: rsvd9d $%02x\n", tag().c_str(), m_pc, imm8));
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
	if (segmented_mode())
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
	else
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

/******************************************
 rsvd9f
 flags:  ------
 ******************************************/
void z8002_device::Z9F_imm8()
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd9f $%02x\n", tag().c_str(), m_pc, imm8));
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
	UINT8 tmp = RB(src);
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
	UINT16 tmp = RW(src);
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
	UINT8 tmp = RB(dst) & ~1;
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
	UINT16 tmp = RW(dst) & ~1;
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
	UINT8 result;
	UINT16 idx = RB(dst);
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
	RW(dst) = (RW(dst) & 0xff) | ((RW(dst) & S08) ? 0xff00 : 0x0000);
}

/******************************************
 extsl   rqd
 flags:  ------
 ******************************************/
void z8002_device::ZB1_dddd_0111()
{
	GET_DST(OP0,NIB2);
	RQ(dst) = CONCAT_64((RQ(dst) & S32) ?
		0xfffffffful : 0, EXTRACT_64LO(RQ(dst)));
}

/******************************************
 exts    rrd
 flags:  ------
 ******************************************/
void z8002_device::ZB1_dddd_1010()
{
	GET_DST(OP0,NIB2);
	RL(dst) = (RL(dst) & 0xffff) | ((RL(dst) & S16) ?
		0xffff0000ul : 0x00000000ul);
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
		RB(dst) = SRLB(RB(dst), -(INT8)imm8);
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
	RB(dst) = SRLB(RB(dst), (INT8)RW(src));
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
		RB(dst) = SRAB(RB(dst), -(INT8)imm8);
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
	RB(dst) = SDAB(RB(dst), (INT8) RW(src));
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
		RW(dst) = SRLW(RW(dst), -(INT16)imm16);
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
	RW(dst) = SDLW(RW(dst), (INT8)RW(src));
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
		RL(dst) = SRLL(RL(dst), -(INT16)imm16);
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
		RW(dst) = SRAW(RW(dst), -(INT16)imm16);
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
	RW(dst) = SDAW(RW(dst), (INT8)RW(src));
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
		RL(dst) = SRAL(RL(dst), -(INT16)imm16);
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
	WRMEM_B(AS_DATA, addr_from_reg(dst), xlt);
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
	WRMEM_B(AS_DATA, addr_from_reg(dst), xlt);
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
	WRMEM_B(AS_DATA, addr_from_reg(dst), xlt);
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
	UINT8 xlt = RDMEM_B(AS_DATA, addr_from_reg(src) + RDMEM_B(AS_DATA, addr_from_reg(dst)));
	WRMEM_B(AS_DATA, addr_from_reg(dst), xlt);
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
	LOG(("Z8K '%s' %04x: rsvdb9 $%02x\n", tag().c_str(), m_pc, imm8));
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
	CPB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	WRMEM_B(AS_DATA,  addr_from_reg(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RDMEM_B(AS_DATA, addr_from_reg(dst)), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RDMEM_B(AS_DATA, addr_from_reg(dst)), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	WRMEM_B(AS_DATA,  addr_from_reg(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RDMEM_B(AS_DATA, addr_from_reg(dst)), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RB(dst), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPB(RDMEM_B(AS_DATA, addr_from_reg(dst)), RDMEM_B(AS_DATA, addr_from_reg(src)));
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
	CPW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	WRMEM_W(AS_DATA,  addr_from_reg(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RDMEM_W(AS_DATA, addr_from_reg(dst)), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RDMEM_W(AS_DATA, addr_from_reg(dst)), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	WRMEM_W(AS_DATA,  addr_from_reg(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RDMEM_W(AS_DATA, addr_from_reg(dst)), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RW(dst), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	CPW(RDMEM_W(AS_DATA, addr_from_reg(dst)), RDMEM_W(AS_DATA, addr_from_reg(src)));
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
	UINT8 b = m_op[0] & 15;
	UINT8 a = (m_op[0] >> 4) & 15;
	UINT8 tmp = RB(b);
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
	UINT8 b = m_op[0] & 15;
	UINT8 a = (m_op[0] >> 4) & 15;
	UINT8 tmp = RB(a);
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
	LOG(("Z8K '%s' %04x: rsvdbf $%02x\n", tag().c_str(), m_pc, imm8));
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
	INT16 dsp12 = m_op[0] & 0xfff;
	if (segmented_mode())
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
