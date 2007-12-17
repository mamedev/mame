/*****************************************************************************
 *
 *   z8000ops.c
 *   Portable Z8000(2) emulator
 *   Opcode functions
 *
 *   Copyright (c) 1998 Juergen Buchmueller, all rights reserved.
 *   Bug fixes and MSB_FIRST compliance Ernesto Corvi.
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
 *****************************************************************************/

/******************************************
 helper functions
 ******************************************/

/******************************************
 check new fcw for switch to system mode
 and swap stack pointer if needed
 ******************************************/
INLINE void CHANGE_FCW(UINT16 fcw)
{
	if (fcw & F_S_N) {			/* system mode now? */
		if (!(FCW & F_S_N)) {	/* and not before? */
			UINT16 tmp = RW(SP);
			RW(SP) = NSP;
			NSP = tmp;
		}
	} else {					/* user mode now */
		if (FCW & F_S_N) {		/* and not before? */
			UINT16 tmp = RW(SP);
			RW(SP) = NSP;
			NSP = tmp;
        }
    }
    if (!(FCW & F_NVIE) && (fcw & F_NVIE) && (Z.irq_state[0] != CLEAR_LINE))
		IRQ_REQ |= Z8000_NVI;
	if (!(FCW & F_VIE) && (fcw & F_VIE) && (Z.irq_state[1] != CLEAR_LINE))
		IRQ_REQ |= Z8000_VI;
    FCW = fcw;  /* set new FCW */
}

INLINE void PUSHW(UINT8 dst, UINT16 value)
{
    RW(dst) -= 2;
	WRMEM_W( RW(dst), value );
}

INLINE UINT16 POPW(UINT8 src)
{
	UINT16 result = RDMEM_W( RW(src) );
    RW(src) += 2;
	return result;
}

INLINE void PUSHL(UINT8 dst, UINT32 value)
{
	RW(dst) -= 4;
	WRMEM_L( RW(dst), value );
}

INLINE UINT32 POPL(UINT8 src)
{
	UINT32 result = RDMEM_L( RW(src) );
    RW(src) += 4;
	return result;
}

/* check zero and sign flag for byte, word and long results */
#define CHK_XXXB_ZS if (!result) SET_Z; else if ((INT8) result < 0) SET_S
#define CHK_XXXW_ZS if (!result) SET_Z; else if ((INT16)result < 0) SET_S
#define CHK_XXXL_ZS if (!result) SET_Z; else if ((INT32)result < 0) SET_S
#define CHK_XXXQ_ZS if (!result) SET_Z; else if ((INT64)result < 0) SET_S

#define CHK_XXXB_ZSP FCW |= z8000_zsp[result]

/* check carry for addition and subtraction */
#define CHK_ADDX_C if (result < dest) SET_C
#define CHK_ADCX_C if (result < dest || (result == dest && value)) SET_C

#define CHK_SUBX_C if (result > dest) SET_C
#define CHK_SBCX_C if (result > dest || (result == dest && value)) SET_C

/* check half carry for A addition and S subtraction */
#define CHK_ADDB_H  if ((result & 15) < (dest & 15)) SET_H
#define CHK_ADCB_H	if ((result & 15) < (dest & 15) || ((result & 15) == (dest & 15) && (value & 15))) SET_H

#define CHK_SUBB_H  if ((result & 15) > (dest & 15)) SET_H
#define CHK_SBCB_H	if ((result & 15) > (dest & 15) || ((result & 15) == (dest & 15) && (value & 15))) SET_H

/* check overflow for addition for byte, word and long */
#define CHK_ADDB_V if (((value & dest & ~result) | (~value & ~dest & result)) & S08) SET_V
#define CHK_ADDW_V if (((value & dest & ~result) | (~value & ~dest & result)) & S16) SET_V
#define CHK_ADDL_V if (((value & dest & ~result) | (~value & ~dest & result)) & S32) SET_V

/* check overflow for subtraction for byte, word and long */
#define CHK_SUBB_V if (((~value & dest & ~result) | (value & ~dest & result)) & S08) SET_V
#define CHK_SUBW_V if (((~value & dest & ~result) | (value & ~dest & result)) & S16) SET_V
#define CHK_SUBL_V if (((~value & dest & ~result) | (value & ~dest & result)) & S32) SET_V


/******************************************
 add byte
 flags:  CZSVDH
 ******************************************/
INLINE UINT8 ADDB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest + value;
    CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
    CLR_DA;         /* clear DA (decimal adjust) flag for addb */
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDX_C; 	/* set C if result overflowed              */
	CHK_ADDB_V; 	/* set V if result has incorrect sign      */
    CHK_ADDB_H;     /* set H if lower nibble overflowed        */
	return result;
}

/******************************************
 add word
 flags:  CZSV--
 ******************************************/
INLINE UINT16 ADDW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest + value;
    CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_ADDX_C; 	/* set C if result overflowed              */
	CHK_ADDW_V; 	/* set V if result has incorrect sign      */
	return result;
}

/******************************************
 add long
 flags:  CZSV--
 ******************************************/
INLINE UINT32 ADDL(UINT32 dest, UINT32 value)
{
	UINT32 result = dest + value;
    CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
    CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_ADDX_C; 	/* set C if result overflowed              */
	CHK_ADDL_V; 	/* set V if result has incorrect sign      */
	return result;
}

/******************************************
 add with carry byte
 flags:  CZSVDH
 ******************************************/
INLINE UINT8 ADCB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest + value + GET_C;
    CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
    CLR_DA;         /* clear DA (decimal adjust) flag for adcb */
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADCX_C; 	/* set C if result overflowed              */
	CHK_ADDB_V; 	/* set V if result has incorrect sign      */
	CHK_ADCB_H; 	/* set H if lower nibble overflowed        */
	return result;
}

/******************************************
 add with carry word
 flags:  CZSV--
 ******************************************/
INLINE UINT16 ADCW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest + value + GET_C;
    CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_ADCX_C; 	/* set C if result overflowed              */
	CHK_ADDW_V; 	/* set V if result has incorrect sign      */
	return result;
}

/******************************************
 subtract byte
 flags:  CZSVDH
 ******************************************/
INLINE UINT8 SUBB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value;
    CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
    SET_DA;         /* set DA (decimal adjust) flag for subb   */
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C; 	/* set C if result underflowed             */
	CHK_SUBB_V; 	/* set V if result has incorrect sign      */
    CHK_SUBB_H;     /* set H if lower nibble underflowed       */
	return result;
}

/******************************************
 subtract word
 flags:  CZSV--
 ******************************************/
INLINE UINT16 SUBW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value;
    CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C; 	/* set C if result underflowed             */
	CHK_SUBW_V; 	/* set V if result has incorrect sign      */
	return result;
}

/******************************************
 subtract long
 flags:  CZSV--
 ******************************************/
INLINE UINT32 SUBL(UINT32 dest, UINT32 value)
{
	UINT32 result = dest - value;
    CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
    CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C; 	/* set C if result underflowed             */
	CHK_SUBL_V; 	/* set V if result has incorrect sign      */
	return result;
}

/******************************************
 subtract with carry byte
 flags:  CZSVDH
 ******************************************/
INLINE UINT8 SBCB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value - GET_C;
    CLR_CZSVH;      /* first clear C, Z, S, P/V and H flags    */
    SET_DA;         /* set DA (decimal adjust) flag for sbcb   */
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SBCX_C; 	/* set C if result underflowed             */
	CHK_SUBB_V; 	/* set V if result has incorrect sign      */
	CHK_SBCB_H; 	/* set H if lower nibble underflowed       */
	return result;
}

/******************************************
 subtract with carry word
 flags:  CZSV--
 ******************************************/
INLINE UINT16 SBCW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value - GET_C;
    CLR_CZSV;       /* first clear C, Z, S, P/V flags          */
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SBCX_C; 	/* set C if result underflowed             */
	CHK_SUBW_V; 	/* set V if result has incorrect sign      */
	return result;
}

/******************************************
 logical or byte
 flags:  -ZSP--
 ******************************************/
INLINE UINT8 ORB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest | value;
	CLR_ZSP;		/* first clear Z, S, P/V flags             */
	CHK_XXXB_ZSP;	/* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical or word
 flags:  -ZS---
 ******************************************/
INLINE UINT16 ORW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest | value;
	CLR_ZS; 		/* first clear Z, and S flags              */
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	return result;
}

/******************************************
 logical and byte
 flags:  -ZSP--
 ******************************************/
INLINE UINT8 ANDB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest & value;
    CLR_ZSP;        /* first clear Z,S and P/V flags           */
	CHK_XXXB_ZSP;	/* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical and word
 flags:  -ZS---
 ******************************************/
INLINE UINT16 ANDW(UINT16 dest, UINT16 value)
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
INLINE UINT8 XORB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest ^ value;
    CLR_ZSP;        /* first clear Z, S and P/V flags          */
	CHK_XXXB_ZSP;	/* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 logical exclusive or word
 flags:  -ZS---
 ******************************************/
INLINE UINT16 XORW(UINT16 dest, UINT16 value)
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
INLINE void CPB(UINT8 dest, UINT8 value)
{
	UINT8 result = dest - value;
    CLR_CZSV;       /* first clear C, Z, S and P/V flags       */
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBX_C; 	/* set C if result underflowed             */
	CHK_SUBB_V;
}

/******************************************
 compare word
 flags:  CZSV--
 ******************************************/
INLINE void CPW(UINT16 dest, UINT16 value)
{
	UINT16 result = dest - value;
	CLR_CZSV;
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBX_C; 	/* set C if result underflowed             */
	CHK_SUBW_V;
}

/******************************************
 compare long
 flags:  CZSV--
 ******************************************/
INLINE void CPL(UINT32 dest, UINT32 value)
{
	UINT32 result = dest - value;
	CLR_CZSV;
    CHK_XXXL_ZS;    /* set Z and S flags for result long       */
	CHK_SUBX_C; 	/* set C if result underflowed             */
	CHK_SUBL_V;
}

/******************************************
 complement byte
 flags: -ZSP--
 ******************************************/
INLINE UINT8 COMB(UINT8 dest)
{
	UINT8 result = ~dest;
	CLR_ZSP;
	CHK_XXXB_ZSP;	/* set Z, S and P flags for result byte    */
	return result;
}

/******************************************
 complement word
 flags: -ZS---
 ******************************************/
INLINE UINT16 COMW(UINT16 dest)
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
INLINE UINT8 NEGB(UINT8 dest)
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
INLINE UINT16 NEGW(UINT16 dest)
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
INLINE void TESTB(UINT8 result)
{
	CLR_ZSP;
	CHK_XXXB_ZSP;	/* set Z and S flags for result byte       */
}

/******************************************
 test word
 flags:  -ZS---
 ******************************************/
INLINE void TESTW(UINT16 dest)
{
	CLR_ZS;
    if (!dest) SET_Z; else if (dest & S16) SET_S;
}

/******************************************
 test long
 flags:  -ZS---
 ******************************************/
INLINE void TESTL(UINT32 dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S32) SET_S;
}

/******************************************
 increment byte
 flags: -ZSV--
 ******************************************/
INLINE UINT8 INCB(UINT8 dest, UINT8 value)
{
    UINT8 result = dest + value;
	CLR_ZSV;
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDB_V; 	/* set V if result overflowed              */
	return result;
}

/******************************************
 increment word
 flags: -ZSV--
 ******************************************/
INLINE UINT16 INCW(UINT16 dest, UINT16 value)
{
    UINT16 result = dest + value;
	CLR_ZSV;
    CHK_XXXW_ZS;    /* set Z and S flags for result byte       */
	CHK_ADDW_V; 	/* set V if result overflowed              */
	return result;
}

/******************************************
 decrement byte
 flags: -ZSV--
 ******************************************/
INLINE UINT8 DECB(UINT8 dest, UINT8 value)
{
    UINT8 result = dest - value;
	CLR_ZSV;
    CHK_XXXB_ZS;    /* set Z and S flags for result byte       */
	CHK_SUBB_V; 	/* set V if result overflowed              */
	return result;
}

/******************************************
 decrement word
 flags: -ZSV--
 ******************************************/
INLINE UINT16 DECW(UINT16 dest, UINT16 value)
{
    UINT16 result = dest - value;
	CLR_ZSV;
    CHK_XXXW_ZS;    /* set Z and S flags for result word       */
	CHK_SUBW_V; 	/* set V if result overflowed              */
	return result;
}

/******************************************
 multiply words
 flags:  CZSV--
 ******************************************/
INLINE UINT32 MULTW(UINT16 dest, UINT16 value)
{
	UINT32 result = (INT32)(INT16)dest * (INT16)value;
	CLR_CZSV;
    CHK_XXXL_ZS;
	if( !value )
	{
		/* multiplication with zero is faster */
        z8000_ICount += (70-18);
	}
	if( (INT32)result < -0x7fff || (INT32)result >= 0x7fff ) SET_C;
	return result;
}

/******************************************
 multiply longs
 flags:  CZSV--
 ******************************************/
INLINE UINT64 MULTL(UINT32 dest, UINT32 value)
{
	UINT64 result = (INT64)(INT32)dest * (INT32)value;
    if( !value )
	{
		/* multiplication with zero is faster */
		z8000_ICount += (282 - 30);
	}
	else
	{
		int n;
		for( n = 0; n < 32; n++ )
			if( dest & (1L << n) ) z8000_ICount -= 7;
    }
    CLR_CZSV;
	CHK_XXXQ_ZS;
	if( (INT64)result < -0x7fffffffL || (INT64)result >= 0x7fffffffL ) SET_C;
	return result;
}

/******************************************
 divide long by word
 flags: CZSV--
 ******************************************/
INLINE UINT32 DIVW(UINT32 dest, UINT16 value)
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
INLINE UINT64 DIVL(UINT64 dest, UINT32 value)
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
INLINE UINT8 RLB(UINT8 dest, UINT8 twice)
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
INLINE UINT16 RLW(UINT16 dest, UINT8 twice)
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
INLINE UINT8 RLCB(UINT8 dest, UINT8 twice)
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
INLINE UINT16 RLCW(UINT16 dest, UINT8 twice)
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
INLINE UINT8 RRB(UINT8 dest, UINT8 twice)
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
INLINE UINT16 RRW(UINT16 dest, UINT8 twice)
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
INLINE UINT8 RRCB(UINT8 dest, UINT8 twice)
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
INLINE UINT16 RRCW(UINT16 dest, UINT8 twice)
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
INLINE UINT8 SDAB(UINT8 dest, INT8 count)
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
INLINE UINT16 SDAW(UINT16 dest, INT8 count)
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
INLINE UINT32 SDAL(UINT32 dest, INT8 count)
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
INLINE UINT8 SDLB(UINT8 dest, INT8 count)
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
INLINE UINT16 SDLW(UINT16 dest, INT8 count)
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
INLINE UINT32 SDLL(UINT32 dest, INT8 count)
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
INLINE UINT8 SLAB(UINT8 dest, UINT8 count)
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
INLINE UINT16 SLAW(UINT16 dest, UINT8 count)
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
INLINE UINT32 SLAL(UINT32 dest, UINT8 count)
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
INLINE UINT8 SLLB(UINT8 dest, UINT8 count)
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
INLINE UINT16 SLLW(UINT16 dest, UINT8 count)
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
INLINE UINT32 SLLL(UINT32 dest, UINT8 count)
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
INLINE UINT8 SRAB(UINT8 dest, UINT8 count)
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
INLINE UINT16 SRAW(UINT16 dest, UINT8 count)
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
INLINE UINT32 SRAL(UINT32 dest, UINT8 count)
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
INLINE UINT8 SRLB(UINT8 dest, UINT8 count)
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
INLINE UINT16 SRLW(UINT16 dest, UINT8 count)
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
INLINE UINT32 SRLL(UINT32 dest, UINT8 count)
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
static void  zinvalid(void)
{
	logerror("Z8000 invalid opcode %04x: %04x\n", PC, Z.op[0]);
}

/******************************************
 addb    rbd,imm8
 flags:  CZSVDH
 ******************************************/
static void  Z00_0000_dddd_imm8(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ADDB( RB(dst), imm8);
}

/******************************************
 addb    rbd,@rs
 flags:  CZSVDH
 ******************************************/
static void Z00_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADDB( RB(dst), RDMEM_B(RW(src)) );
}

/******************************************
 add     rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z01_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ADDW( RW(dst), imm16 );
}

/******************************************
 add     rd,@rs
 flags:  CZSV--
 ******************************************/
static void Z01_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADDW( RW(dst), RDMEM_W(RW(src)) );
}

/******************************************
 subb    rbd,imm8
 flags:  CZSVDH
 ******************************************/
static void Z02_0000_dddd_imm8(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = SUBB( RB(dst), imm8 );
}

/******************************************
 subb    rbd,@rs
 flags:  CZSVDH
 ******************************************/
static void Z02_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SUBB( RB(dst), RDMEM_B(RW(src)) ); /* EHC */
}

/******************************************
 sub     rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z03_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = SUBW( RW(dst), imm16 );
}

/******************************************
 sub     rd,@rs
 flags:  CZSV--
 ******************************************/
static void Z03_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SUBW( RW(dst), RDMEM_W(RW(src)) );
}

/******************************************
 orb     rbd,imm8
 flags:  CZSP--
 ******************************************/
static void Z04_0000_dddd_imm8(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ORB( RB(dst), imm8 );
}

/******************************************
 orb     rbd,@rs
 flags:  CZSP--
 ******************************************/
static void Z04_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ORB( RB(dst), RDMEM_B(RW(src)) );
}

/******************************************
 or      rd,imm16
 flags:  CZS---
 ******************************************/
static void Z05_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ORW( RW(dst), imm16 );
}

/******************************************
 or      rd,@rs
 flags:  CZS---
 ******************************************/
static void Z05_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ORW( RW(dst), RDMEM_W(RW(src)) );
}

/******************************************
 andb    rbd,imm8
 flags:  -ZSP--
 ******************************************/
static void Z06_0000_dddd_imm8(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = ANDB( RB(dst), imm8 );
}

/******************************************
 andb    rbd,@rs
 flags:  -ZSP--
 ******************************************/
static void Z06_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ANDB( RB(dst), RDMEM_B(RW(src)) );
}

/******************************************
 and     rd,imm16
 flags:  -ZS---
 ******************************************/
static void Z07_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = ANDW( RW(dst), imm16 );
}

/******************************************
 and     rd,@rs
 flags:  -ZS---
 ******************************************/
static void Z07_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ANDW( RW(dst), RDMEM_W(RW(src)) );
}

/******************************************
 xorb    rbd,imm8
 flags:  -ZSP--
 ******************************************/
static void Z08_0000_dddd_imm8(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	RB(dst) = XORB(RB(dst), imm8);
}

/******************************************
 xorb    rbd,@rs
 flags:  -ZSP--
 ******************************************/
static void Z08_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = XORB( RB(dst), RDMEM_B(RW(src)) );
}

/******************************************
 xor     rd,imm16
 flags:  -ZS---
 ******************************************/
static void Z09_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = XORW( RW(dst), imm16 );
}

/******************************************
 xor     rd,@rs
 flags:  -ZS---
 ******************************************/
static void Z09_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = XORW( RW(dst), RDMEM_W(RW(src)) );
}

/******************************************
 cpb     rbd,imm8
 flags:  CZSV--
 ******************************************/
static void Z0A_0000_dddd_imm8(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	CPB(RB(dst), imm8);
}

/******************************************
 cpb     rbd,@rs
 flags:  CZSV--
 ******************************************/
static void Z0A_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB( RB(dst), RDMEM_B(RW(src)) );
}

/******************************************
 cp      rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z0B_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	CPW( RW(dst), imm16 );
}

/******************************************
 cp      rd,@rs
 flags:  CZSV--
 ******************************************/
static void Z0B_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW( RW(dst), RDMEM_W(RW(src)) );
}

/******************************************
 comb    @rd
 flags:  -ZSP--
 ******************************************/
static void Z0C_ddN0_0000(void)
{
	GET_DST(OP0,NIB3);
	WRMEM_B( RW(dst), COMB(RDMEM_B(RW(dst))) );
}

/******************************************
 cpb     @rd,imm8
 flags:  CZSV--
 ******************************************/
static void Z0C_ddN0_0001_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	CPB( RB(dst), imm8 );
}

/******************************************
 negb    @rd
 flags:  CZSV--
 ******************************************/
static void Z0C_ddN0_0010(void)
{
	GET_DST(OP0,NIB2);
	WRMEM_B( RW(dst), NEGB(RDMEM_B(RW(dst))) );
}

/******************************************
 testb   @rd
 flags:  -ZSP--
 ******************************************/
static void Z0C_ddN0_0100(void)
{
	GET_DST(OP0,NIB2);
	TESTB(RDMEM_B(RW(dst)));
}

/******************************************
 ldb     @rd,imm8
 flags:  ------
 ******************************************/
static void Z0C_ddN0_0101_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	WRMEM_B( RW(dst), imm8 );
}

/******************************************
 tsetb   @rd
 flags:  --S---
 ******************************************/
static void Z0C_ddN0_0110(void)
{
	GET_DST(OP0,NIB2);
    if (RDMEM_B(RW(dst)) & S08) SET_S; else CLR_S;
    WRMEM_B(RW(dst), 0xff);
}

/******************************************
 clrb    @rd
 flags:  ------
 ******************************************/
static void Z0C_ddN0_1000(void)
{
	GET_DST(OP0,NIB2);
	WRMEM_B( RW(dst), 0 );
}

/******************************************
 com     @rd
 flags:  -ZS---
 ******************************************/
static void Z0D_ddN0_0000(void)
{
	GET_DST(OP0,NIB2);
	WRMEM_W( RW(dst), COMW(RDMEM_W(RW(dst))) );
}

/******************************************
 cp      @rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z0D_ddN0_0001_imm16(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	CPW( RDMEM_W(RW(dst)), imm16 );
}

/******************************************
 neg     @rd
 flags:  CZSV--
 ******************************************/
static void Z0D_ddN0_0010(void)
{
	GET_DST(OP0,NIB2);
	WRMEM_W( RW(dst), NEGW(RDMEM_W(RW(dst))) );
}

/******************************************
 test    @rd
 flags:  -ZS---
 ******************************************/
static void Z0D_ddN0_0100(void)
{
	GET_DST(OP0,NIB2);
	TESTW( RDMEM_W(RW(dst)) );
}

/******************************************
 ld      @rd,imm16
 flags:  ------
 ******************************************/
static void Z0D_ddN0_0101_imm16(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	WRMEM_W( RW(dst), imm16);
}

/******************************************
 tset    @rd
 flags:  --S---
 ******************************************/
static void Z0D_ddN0_0110(void)
{
	GET_DST(OP0,NIB2);
    if (RDMEM_W(RW(dst)) & S16) SET_S; else CLR_S;
    WRMEM_W(RW(dst), 0xffff);
}

/******************************************
 clr     @rd
 flags:  ------
 ******************************************/
static void Z0D_ddN0_1000(void)
{
	GET_DST(OP0,NIB2);
	WRMEM_W( RDMEM_W(RW(dst)), 0 );
}

/******************************************
 push    @rd,imm16
 flags:  ------
 ******************************************/
static void Z0D_ddN0_1001_imm16(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	PUSHW( dst, imm16 );
}

/******************************************
 ext0e   imm8
 flags:  ------
 ******************************************/
static void Z0E_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: ext0e  $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ext0f   imm8
 flags:  ------
 ******************************************/
static void Z0F_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: ext0f  $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 cpl     rrd,imm32
 flags:  CZSV--
 ******************************************/
static void Z10_0000_dddd_imm32(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	CPL( RL(dst), imm32 );
}

/******************************************
 cpl     rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z10_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPL( RL(dst), RDMEM_L(RW(src)) );
}

/******************************************
 pushl   @rd,@rs
 flags:  ------
 ******************************************/
static void Z11_ddN0_ssN0(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL( dst, RDMEM_L(RW(src)) );
}

/******************************************
 subl    rrd,imm32
 flags:  CZSV--
 ******************************************/
static void Z12_0000_dddd_imm32(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = SUBL( RL(dst), imm32 );
}

/******************************************
 subl    rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z12_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = SUBL( RL(dst), RDMEM_L(RW(src)) );
}

/******************************************
 push    @rd,@rs
 flags:  ------
 ******************************************/
static void Z13_ddN0_ssN0(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW( dst, RDMEM_W(RW(src)) );
}

/******************************************
 ldl     rrd,imm32
 flags:  ------
 ******************************************/
static void Z14_0000_dddd_imm32(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = imm32;
}

/******************************************
 ldl     rrd,@rs
 flags:  ------
 ******************************************/
static void Z14_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = RDMEM_L( RW(src) );
}

/******************************************
 popl    @rd,@rs
 flags:  ------
 ******************************************/
static void Z15_ssN0_ddN0(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = POPL( src );
}

/******************************************
 addl    rrd,imm32
 flags:  CZSV--
 ******************************************/
static void Z16_0000_dddd_imm32(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RL(dst) = ADDL( RL(dst), imm32 );
}

/******************************************
 addl    rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z16_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = ADDL( RL(dst), RDMEM_L(RW(src)) );
}

/******************************************
 pop     @rd,@rs
 flags:  ------
 ******************************************/
static void Z17_ssN0_ddN0(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = POPW( src );
}

/******************************************
 multl   rqd,@rs
 flags:  CZSV--
 ******************************************/
static void Z18_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = MULTL( RQ(dst), RL(src) );
}

/******************************************
 mult    rrd,imm16
 flags:  CZSV--
 ******************************************/
static void Z19_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RL(dst) = MULTW( RL(dst), imm16 );
}

/******************************************
 mult    rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z19_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = MULTW( RL(dst), RDMEM_W(RW(src)) );
}

/******************************************
 divl    rqd,imm32
 flags:  CZSV--
 ******************************************/
static void Z1A_0000_dddd_imm32(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	RQ(dst) = DIVL( RQ(dst), imm32 );
}

/******************************************
 divl    rqd,@rs
 flags:  CZSV--
 ******************************************/
static void Z1A_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RQ(dst) = DIVL( RQ(dst), RDMEM_L(RW(src)) );
}

/******************************************
 div     rrd,imm16
 flags:  CZSV--
 ******************************************/
static void Z1B_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RL(dst) = DIVW( RL(dst), imm16 );
}

/******************************************
 div     rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z1B_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = DIVW( RL(dst), RDMEM_W(RW(src)) );
}

/******************************************
 testl   @rd
 flags:  -ZS---
 ******************************************/
static void Z1C_ddN0_1000(void)
{
	GET_DST(OP0,NIB2);
	TESTL( RDMEM_L(RW(dst)) );
}

/******************************************
 ldm     @rd,rs,n
 flags:  ------
 ******************************************/
static void Z1C_ddN0_1001_0000_ssss_0000_nmin1(void)
{
    GET_DST(OP0,NIB2);
    GET_CNT(OP1,NIB3);
    GET_SRC(OP1,NIB1);
	UINT16 idx = RW(dst);
    while (cnt-- >= 0) {
        WRMEM_W( idx, RW(src) );
		idx = (idx + 2) & 0xffff;
		src = (src+1) & 15;
    }
}

/******************************************
 ldm     rd,@rs,n
 flags:  ------
 ******************************************/
static void Z1C_ssN0_0001_0000_dddd_0000_nmin1(void)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB3);
	GET_DST(OP1,NIB1);
	UINT16 idx = RW(src);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W( idx );
		idx = (idx + 2) & 0xffff;
		dst = (dst+1) & 15;
    }
}

/******************************************
 ldl     @rd,rrs
 flags:  ------
 ******************************************/
static void Z1D_ddN0_ssss(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_L( RW(dst), RL(src) );
}

/******************************************
 jp      cc,rd
 flags:  ------
 ******************************************/
static void Z1E_ddN0_cccc(void)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (cc) {
		case  0: if (CC0) PC = RW(dst); break;
		case  1: if (CC1) PC = RW(dst); break;
		case  2: if (CC2) PC = RW(dst); break;
		case  3: if (CC3) PC = RW(dst); break;
		case  4: if (CC4) PC = RW(dst); break;
		case  5: if (CC5) PC = RW(dst); break;
		case  6: if (CC6) PC = RW(dst); break;
		case  7: if (CC7) PC = RW(dst); break;
		case  8: if (CC8) PC = RW(dst); break;
		case  9: if (CC9) PC = RW(dst); break;
		case 10: if (CCA) PC = RW(dst); break;
		case 11: if (CCB) PC = RW(dst); break;
		case 12: if (CCC) PC = RW(dst); break;
		case 13: if (CCD) PC = RW(dst); break;
		case 14: if (CCE) PC = RW(dst); break;
		case 15: if (CCF) PC = RW(dst); break;
	}
	change_pc(PC);
}

/******************************************
 call    @rd
 flags:  ------
 ******************************************/
static void Z1F_ddN0_0000(void)
{
	GET_DST(OP0,NIB2);
	PUSHW( SP, PC );
    PC = RW(dst);
	change_pc(PC);
}

/******************************************
 ldb     rbd,@rs
 flags:  ------
 ******************************************/
static void Z20_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = RDMEM_B( RW(src) );
}

/******************************************
 ld      rd,imm16
 flags:  ------
 ******************************************/
static void Z21_0000_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	RW(dst) = imm16;
}

/******************************************
 ld      rd,@rs
 flags:  ------
 ******************************************/
static void Z21_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = RDMEM_W( RW(src) );
}

/******************************************
 resb    rbd,rs
 flags:  ------
 ******************************************/
static void Z22_0000_ssss_0000_dddd_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RB(dst) = RB(dst) & ~(1 << (RW(src) & 7));
}

/******************************************
 resb    @rd,imm4
 flags:  ------
 ******************************************/
static void Z22_ddN0_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	WRMEM_B(RW(dst), RDMEM_B(RW(dst)) & ~bit);
}

/******************************************
 result     rd,rs
 flags:  ------
 ******************************************/
static void Z23_0000_ssss_0000_dddd_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RW(dst) = RW(dst) & ~(1 << (RW(src) & 15));
}

/******************************************
 res     @rd,imm4
 flags:  ------
 ******************************************/
static void Z23_ddN0_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	WRMEM_W(RW(dst), RDMEM_W(RW(dst)) & ~bit);
}

/******************************************
 setb    rbd,rs
 flags:  ------
 ******************************************/
static void Z24_0000_ssss_0000_dddd_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RB(dst) = RB(dst) | (1 << (RW(src) & 7));
}

/******************************************
 setb    @rd,imm4
 flags:  ------
 ******************************************/
static void Z24_ddN0_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	WRMEM_B(RW(dst), RDMEM_B(RW(dst)) | bit);
}

/******************************************
 set     rd,rs
 flags:  ------
 ******************************************/
static void Z25_0000_ssss_0000_dddd_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	RW(dst) = RW(dst) | (1 << (RW(src) & 15));
}

/******************************************
 set     @rd,imm4
 flags:  ------
 ******************************************/
static void Z25_ddN0_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	WRMEM_W(RW(dst), RDMEM_W(RW(dst)) | bit);
}

/******************************************
 bitb    rbd,rs
 flags:  -Z----
 ******************************************/
static void Z26_0000_ssss_0000_dddd_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (RB(dst) & (1 << (RW(src) & 7))) CLR_Z; else SET_Z;
}

/******************************************
 bitb    @rd,imm4
 flags:  -Z----
 ******************************************/
static void Z26_ddN0_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDMEM_B(RW(dst)) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     rd,rs
 flags:  -Z----
 ******************************************/
static void Z27_0000_ssss_0000_dddd_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (RW(dst) & (1 << (RW(src) & 15))) CLR_Z; else SET_Z;
}

/******************************************
 bit     @rd,imm4
 flags:  -Z----
 ******************************************/
static void Z27_ddN0_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDMEM_W(RW(dst)) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z28_ddN0_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_B( RW(dst), INCB( RDMEM_B(RW(dst)), i4p1) );
}

/******************************************
 inc     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z29_ddN0_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_W( RW(dst), INCW( RDMEM_W(RW(dst)), i4p1 ) );
}

/******************************************
 decb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z2A_ddN0_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_B( RW(dst), DECB( RDMEM_B(RW(dst)), i4p1 ) );
}

/******************************************
 dec     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z2B_ddN0_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_W( RW(dst), DECW( RDMEM_W(RW(dst)), i4p1 ) );
}

/******************************************
 exb     rbd,@rs
 flags:  ------
 ******************************************/
static void Z2C_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	UINT8 tmp = RDMEM_B( RW(src) );
	WRMEM_B( RW(src), RB(dst) );
	RB(dst) = tmp;
}

/******************************************
 ex      rd,@rs
 flags:  ------
 ******************************************/
static void Z2D_ssN0_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	UINT16 tmp = RDMEM_W( RW(src) );
	WRMEM_W( RW(src), RW(dst) );
	RW(dst) = tmp;
}

/******************************************
 ldb     @rd,rbs
 flags:  ------
 ******************************************/
static void Z2E_ddN0_ssss(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_B( RW(dst), RB(src) );
}

/******************************************
 ld      @rd,rs
 flags:  ------
 ******************************************/
static void Z2F_ddN0_ssss(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_W( RW(dst), RW(src) );
}

/******************************************
 ldrb    rbd,dsp16
 flags:  ------
 ******************************************/
static void Z30_0000_dddd_dsp16(void)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RB(dst) = RDMEM_B(dsp16);
}

/******************************************
 ldb     rbd,rs(imm16)
 flags:  ------
 ******************************************/
static void Z30_ssN0_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(src);
	RB(dst) = RDMEM_B( imm16 );
}

/******************************************
 ldr     rd,dsp16
 flags:  ------
 ******************************************/
static void Z31_0000_dddd_dsp16(void)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RW(dst) = RDMEM_W(dsp16);
}

/******************************************
 ld      rd,rs(imm16)
 flags:  ------
 ******************************************/
static void Z31_ssN0_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(src);
	RW(dst) = RDMEM_W( imm16 );
}

/******************************************
 ldrb    dsp16,rbs
 flags:  ------
 ******************************************/
static void Z32_0000_ssss_dsp16(void)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_B( dsp16, RB(src) );
}

/******************************************
 ldb     rd(imm16),rbs
 flags:  ------
 ******************************************/
static void Z32_ddN0_ssss_imm16(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(dst);
	WRMEM_B( imm16, RB(src) );
}

/******************************************
 ldr     dsp16,rs
 flags:  ------
 ******************************************/
static void Z33_0000_ssss_dsp16(void)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_W( dsp16, RW(src) );
}

/******************************************
 ld      rd(imm16),rs
 flags:  ------
 ******************************************/
static void Z33_ddN0_ssss_imm16(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(dst);
	WRMEM_W( imm16, RW(src) );
}

/******************************************
 ldar    prd,dsp16
 flags:  ------
 ******************************************/
static void Z34_0000_dddd_dsp16(void)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RW(dst) = dsp16;
}

/******************************************
 lda     prd,rs(imm16)
 flags:  ------
 ******************************************/
static void Z34_ssN0_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(src);
	RW(dst) = imm16;
}

/******************************************
 ldrl    rrd,dsp16
 flags:  ------
 ******************************************/
static void Z35_0000_dddd_dsp16(void)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	RL(dst) = RDMEM_L( dsp16 );
}

/******************************************
 ldl     rrd,rs(imm16)
 flags:  ------
 ******************************************/
static void Z35_ssN0_dddd_imm16(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(src);
	RL(dst) = RDMEM_L( imm16 );
}

/******************************************
 bpt
 flags:  ------
 ******************************************/
static void Z36_0000_0000(void)
{
	/* execute break point trap IRQ_REQ */
	IRQ_REQ = Z8000_TRAP;
}

/******************************************
 rsvd36
 flags:  ------
 ******************************************/
static void Z36_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvd36 $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldrl    dsp16,rrs
 flags:  ------
 ******************************************/
static void Z37_0000_ssss_dsp16(void)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_L( dsp16, RL(src) );
}

/******************************************
 ldl     rd(imm16),rrs
 flags:  ------
 ******************************************/
static void Z37_ddN0_ssss_imm16(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	imm16 += RW(dst);
	WRMEM_L( imm16, RL(src) );
}

/******************************************
 rsvd38
 flags:  ------
 ******************************************/
static void Z38_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvd38 $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldps    @rs
 flags:  CZSVDH
 ******************************************/
static void Z39_ssN0_0000(void)
{
	GET_SRC(OP0,NIB2);
	UINT16 fcw;
	fcw = RDMEM_W( RW(src) );
	PC	= RDMEM_W( (UINT16)(RW(src) + 2) );
	CHANGE_FCW(fcw); /* check for user/system mode change */
    change_pc(PC);
}

/******************************************
 inib(r) @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_0000_0000_aaaa_dddd_x000(void)
{
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRMEM_B( RW(dst), RDPORT_B( 0, RW(src) ) );
    RW(dst)++;
    RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 sinib   @rd,@rs,ra
 sinibr  @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3A_ssss_0001_0000_aaaa_dddd_x000(void)
{
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRMEM_B( RW(dst), RDPORT_B( 1, RW(src) ) );
    RW(dst)++;
    RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 outib   @rd,@rs,ra
 outibr  @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_0010_0000_aaaa_dddd_x000(void)
{
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRPORT_B( 0, RW(dst), RDMEM_B( RW(src) ) );
    RW(dst)++;
    RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 soutib  @rd,@rs,ra
 soutibr @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3A_ssss_0011_0000_aaaa_dddd_x000(void)
{
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRPORT_B( 1, RW(dst), RDMEM_B( RW(src) ) );
    RW(dst)++;
    RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 inb     rbd,imm16
 flags:  ------
 ******************************************/
static void Z3A_dddd_0100_imm16(void)
{
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
    RB(dst) = RDPORT_B( 0, imm16 );
}

/******************************************
 sinb    rbd,imm16
 flags:  ------
 ******************************************/
static void Z3A_dddd_0101_imm16(void)
{
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
    RB(dst) = RDPORT_B( 1, imm16 );
}

/******************************************
 outb    imm16,rbs
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_0110_imm16(void)
{
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
    WRPORT_B( 0, imm16, RB(src) );
}

/******************************************
 soutb   imm16,rbs
 flags:  ------
 ******************************************/
static void Z3A_ssss_0111_imm16(void)
{
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
    WRPORT_B( 1, imm16, RB(src) );
}

/******************************************
 indb    @rd,@rs,rba
 indbr   @rd,@rs,rba
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_1000_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_B( RW(dst), RDPORT_B( 0, RW(src) ) );
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 sindb   @rd,@rs,rba
 sindbr  @rd,@rs,rba
 flags:  ------
 ******************************************/
static void Z3A_ssss_1001_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_B( RW(dst), RDPORT_B( 1, RW(src) ) );
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 outdb   @rd,@rs,rba
 outdbr  @rd,@rs,rba
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_1010_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 0, RW(dst), RDMEM_B( RW(src) ) );
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 soutdb  @rd,@rs,rba
 soutdbr @rd,@rs,rba
 flags:  ------
 ******************************************/
static void Z3A_ssss_1011_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B( 1, RW(dst), RDMEM_B( RW(src) ) );
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 ini     @rd,@rs,ra
 inir    @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_0000_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W( RW(dst), RDPORT_W( 0, RW(src) ) );
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 sini    @rd,@rs,ra
 sinir   @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_0001_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W( RW(dst), RDPORT_W( 1, RW(src) ) );
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 outi    @rd,@rs,ra
 outir   @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_0010_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 0, RW(dst), RDMEM_W( RW(src) ) );
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 souti   @rd,@rs,ra
 soutir  @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_0011_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 1, RW(dst), RDMEM_W( RW(src) ) );
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 in      rd,imm16
 flags:  ------
 ******************************************/
static void Z3B_dddd_0100_imm16(void)
{
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
	RW(dst) = RDPORT_W( 0, imm16 );
}

/******************************************
 sin     rd,imm16
 flags:  ------
 ******************************************/
static void Z3B_dddd_0101_imm16(void)
{
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
	RW(dst) = RDPORT_W( 1, imm16 );
}

/******************************************
 out     imm16,rs
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_0110_imm16(void)
{
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
	WRPORT_W( 0, imm16, RW(src) );
}

/******************************************
 sout    imm16,rbs
 flags:  ------
 ******************************************/
static void Z3B_ssss_0111_imm16(void)
{
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
	WRPORT_W( 1, imm16, RW(src) );
}

/******************************************
 ind     @rd,@rs,ra
 indr    @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_1000_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W( RW(dst), RDPORT_W( 0, RW(src) ) );
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 sind    @rd,@rs,ra
 sindr   @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_1001_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W( RW(dst), RDPORT_W( 1, RW(src) ) );
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 outd    @rd,@rs,ra
 outdr   @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_1010_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 0, RW(dst), RDMEM_W( RW(src) ) );
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 soutd   @rd,@rs,ra
 soutdr  @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_1011_0000_aaaa_dddd_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W( 1, RW(dst), RDMEM_W( RW(src) ) );
	RW(dst) -= 2;
	RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 inb     rbd,@rs
 flags:  ------
 ******************************************/
static void Z3C_ssss_dddd(void)
{
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	RB(dst) = RDPORT_B( 0, RDMEM_W( RW(src) ) );
}

/******************************************
 in      rd,@rs
 flags:  ------
 ******************************************/
static void Z3D_ssss_dddd(void)
{
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	RW(dst) = RDPORT_W( 0, RDMEM_W( RW(src) ) );
}

/******************************************
 outb    @rd,rbs
 flags:  ---V--
 ******************************************/
static void Z3E_dddd_ssss(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_B( 0, RDMEM_W( RW(dst) ), RB(src) );
}

/******************************************
 out     @rd,rs
 flags:  ---V--
 ******************************************/
static void Z3F_dddd_ssss(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_W( 0, RDMEM_W( RW(dst) ), RW(src) );
}

/******************************************
 addb    rbd,addr
 flags:  CZSVDH
 ******************************************/
static void Z40_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ADDB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 addb    rbd,addr(rs)
 flags:  CZSVDH
 ******************************************/
static void Z40_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RB(dst) = ADDB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 add     rd,addr
 flags:  CZSV--
 ******************************************/
static void Z41_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ADDW( RW(dst), RDMEM_W(addr)); /* EHC */
}

/******************************************
 add     rd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z41_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RW(dst) = ADDW( RW(dst), RDMEM_W(addr) );	/* ASG */
}

/******************************************
 subb    rbd,addr
 flags:  CZSVDH
 ******************************************/
static void Z42_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = SUBB( RB(dst), RDMEM_B(addr)); /* EHC */
}

/******************************************
 subb    rbd,addr(rs)
 flags:  CZSVDH
 ******************************************/
static void Z42_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RB(dst) = SUBB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 sub     rd,addr
 flags:  CZSV--
 ******************************************/
static void Z43_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = SUBW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 sub     rd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z43_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RW(dst) = SUBW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 orb     rbd,addr
 flags:  CZSP--
 ******************************************/
static void Z44_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ORB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 orb     rbd,addr(rs)
 flags:  CZSP--
 ******************************************/
static void Z44_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RB(dst) = ORB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 or      rd,addr
 flags:  CZS---
 ******************************************/
static void Z45_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ORW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 or      rd,addr(rs)
 flags:  CZS---
 ******************************************/
static void Z45_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RW(dst) = ORW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 andb    rbd,addr
 flags:  -ZSP--
 ******************************************/
static void Z46_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = ANDB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 andb    rbd,addr(rs)
 flags:  -ZSP--
 ******************************************/
static void Z46_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RB(dst) = ANDB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 and     rd,addr
 flags:  -ZS---
 ******************************************/
static void Z47_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = ANDW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 and     rd,addr(rs)
 flags:  -ZS---
 ******************************************/
static void Z47_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RW(dst) = ANDW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 xorb    rbd,addr
 flags:  -ZSP--
 ******************************************/
static void Z48_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = XORB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 xorb    rbd,addr(rs)
 flags:  -ZSP--
 ******************************************/
static void Z48_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RB(dst) = XORB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 xor     rd,addr
 flags:  -ZS---
 ******************************************/
static void Z49_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = XORW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 xor     rd,addr(rs)
 flags:  -ZS---
 ******************************************/
static void Z49_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RW(dst) = XORW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 cpb     rbd,addr
 flags:  CZSV--
 ******************************************/
static void Z4A_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 cpb     rbd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z4A_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	CPB( RB(dst), RDMEM_B(addr) );
}

/******************************************
 cp      rd,addr
 flags:  CZSV--
 ******************************************/
static void Z4B_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 cp      rd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z4B_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	CPW( RW(dst), RDMEM_W(addr) );
}

/******************************************
 comb    addr
 flags:  -ZSP--
 ******************************************/
static void Z4C_0000_0000_addr(void)
{
	GET_ADDR(OP1);
	WRMEM_B( addr, COMB(RDMEM_W(addr)) );
}

/******************************************
 cpb     addr,imm8
 flags:  CZSV--
 ******************************************/
static void Z4C_0000_0001_addr_imm8(void)
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	CPB( RDMEM_B(addr), imm8 );
}

/******************************************
 negb    addr
 flags:  CZSV--
 ******************************************/
static void Z4C_0000_0010_addr(void)
{
	GET_ADDR(OP1);
	WRMEM_B( addr, NEGB(RDMEM_B(addr)) );
}

/******************************************
 testb   addr
 flags:  -ZSP--
 ******************************************/
static void Z4C_0000_0100_addr(void)
{
	GET_ADDR(OP1);
	TESTB(RDMEM_B(addr));
}

/******************************************
 ldb     addr,imm8
 flags:  ------
 ******************************************/
static void Z4C_0000_0101_addr_imm8(void)
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	WRMEM_B( addr, imm8 );
}

/******************************************
 tsetb   addr
 flags:  --S---
 ******************************************/
static void Z4C_0000_0110_addr(void)
{
	GET_ADDR(OP1);
    if (RDMEM_B(addr) & S08) SET_S; else CLR_S;
    WRMEM_B(addr, 0xff);
}

/******************************************
 clrb    addr
 flags:  ------
 ******************************************/
static void Z4C_0000_1000_addr(void)
{
	GET_ADDR(OP1);
	WRMEM_B( addr, 0 );
}

/******************************************
 comb    addr(rd)
 flags:  -ZSP--
 ******************************************/
static void Z4C_ddN0_0000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, COMB(RDMEM_B(addr)) );
}

/******************************************
 cpb     addr(rd),imm8
 flags:  CZSV--
 ******************************************/
static void Z4C_ddN0_0001_addr_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr += RW(dst);
	CPB( RDMEM_B(addr), imm8 );
}

/******************************************
 negb    addr(rd)
 flags:  CZSV--
 ******************************************/
static void Z4C_ddN0_0010_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, NEGB(RDMEM_B(addr)) );
}

/******************************************
 testb   addr(rd)
 flags:  -ZSP--
 ******************************************/
static void Z4C_ddN0_0100_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	TESTB( RDMEM_B(addr) );
}

/******************************************
 ldb     addr(rd),imm8
 flags:  ------
 ******************************************/
static void Z4C_ddN0_0101_addr_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr += RW(dst);
	WRMEM_B( addr, imm8 );
}

/******************************************
 tsetb   addr(rd)
 flags:  --S---
 ******************************************/
static void Z4C_ddN0_0110_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
    if (RDMEM_B(addr) & S08) SET_S; else CLR_S;
    WRMEM_B(addr, 0xff);
}

/******************************************
 clrb    addr(rd)
 flags:  ------
 ******************************************/
static void Z4C_ddN0_1000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, 0 );
}

/******************************************
 com     addr
 flags:  -ZS---
 ******************************************/
static void Z4D_0000_0000_addr(void)
{
	GET_ADDR(OP1);
	WRMEM_W( addr, COMW(RDMEM_W(addr)) );
}

/******************************************
 cp      addr,imm16
 flags:  CZSV--
 ******************************************/
static void Z4D_0000_0001_addr_imm16(void)
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	CPW( RDMEM_W(addr), imm16 );
}

/******************************************
 neg     addr
 flags:  CZSV--
 ******************************************/
static void Z4D_0000_0010_addr(void)
{
	GET_ADDR(OP1);
	WRMEM_W( addr, NEGW(RDMEM_W(addr)) );
}

/******************************************
 test    addr
 flags:  ------
 ******************************************/
static void Z4D_0000_0100_addr(void)
{
	GET_ADDR(OP1);
	TESTW( RDMEM_W(addr) );
}

/******************************************
 ld      addr,imm16
 flags:  ------
 ******************************************/
static void Z4D_0000_0101_addr_imm16(void)
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	WRMEM_W( addr, imm16 );
}

/******************************************
 tset    addr
 flags:  --S---
 ******************************************/
static void Z4D_0000_0110_addr(void)
{
	GET_ADDR(OP1);
    if (RDMEM_W(addr) & S16) SET_S; else CLR_S;
    WRMEM_W(addr, 0xffff);
}

/******************************************
 clr     addr
 flags:  ------
 ******************************************/
static void Z4D_0000_1000_addr(void)
{
	GET_ADDR(OP1);
	WRMEM_W( addr, 0 );
}

/******************************************
 com     addr(rd)
 flags:  -ZS---
 ******************************************/
static void Z4D_ddN0_0000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, COMW(RDMEM_W(addr)) );
}

/******************************************
 cp      addr(rd),imm16
 flags:  CZSV--
 ******************************************/
static void Z4D_ddN0_0001_addr_imm16(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr += RW(dst);
	CPW( RDMEM_W(addr), imm16 );
}

/******************************************
 neg     addr(rd)
 flags:  CZSV--
 ******************************************/
static void Z4D_ddN0_0010_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, NEGW(RDMEM_W(addr)) );
}

/******************************************
 test    addr(rd)
 flags:  ------
 ******************************************/
static void Z4D_ddN0_0100_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	TESTW( RDMEM_W(addr) );
}

/******************************************
 ld      addr(rd),imm16
 flags:  ------
 ******************************************/
static void Z4D_ddN0_0101_addr_imm16(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr += RW(dst);
	WRMEM_W( addr, imm16 );
}

/******************************************
 tset    addr(rd)
 flags:  --S---
 ******************************************/
static void Z4D_ddN0_0110_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
    if (RDMEM_W(addr) & S16) SET_S; else CLR_S;
    WRMEM_W(addr, 0xffff);
}

/******************************************
 clr     addr(rd)
 flags:  ------
 ******************************************/
static void Z4D_ddN0_1000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, 0 );
}

/******************************************
 ldb     addr(rd),rbs
 flags:  ------
 ******************************************/
static void Z4E_ddN0_ssN0_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, RB(src) );
}

/******************************************
 cpl     rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z50_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPL( RL(dst), RDMEM_L(addr) );
}

/******************************************
 cpl     rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z50_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	CPL( RL(dst), RDMEM_L(addr) );
}

/******************************************
 pushl   @rd,addr
 flags:  ------
 ******************************************/
static void Z51_ddN0_0000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHL( dst, RDMEM_L(addr) );
}

/******************************************
 pushl   @rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z51_ddN0_ssN0_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	PUSHL( dst, RDMEM_L(addr) );
}

/******************************************
 subl    rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z52_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = SUBL( RL(dst), RDMEM_L(addr) );
}

/******************************************
 subl    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z52_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RL(dst) = SUBL( RL(dst), RDMEM_L(addr) );
}

/******************************************
 push    @rd,addr
 flags:  ------
 ******************************************/
static void Z53_ddN0_0000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHW( dst, RDMEM_W(addr) );
}

/******************************************
 push    @rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z53_ddN0_ssN0_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr += RW(src);
	PUSHW( dst, RDMEM_W(addr) );
}

/******************************************
 ldl     rrd,addr
 flags:  ------
 ******************************************/
static void Z54_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = RDMEM_L( addr );
}

/******************************************
 ldl     rrd,addr(rs)
 flags:  ------
 ******************************************/
static void Z54_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RL(dst) = RDMEM_L( addr );
}

/******************************************
 popl    addr,@rs
 flags:  ------
 ******************************************/
static void Z55_ssN0_0000_addr(void)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_L( addr, POPL(src) );
}

/******************************************
 popl    addr(rd),@rs
 flags:  ------
 ******************************************/
static void Z55_ssN0_ddN0_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_L( addr, POPL(src) );
}

/******************************************
 addl    rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z56_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = ADDL( RL(dst), RDMEM_L(addr) );
}

/******************************************
 addl    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z56_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RL(dst) = ADDL( RL(dst), RDMEM_L(addr) );
}

/******************************************
 pop     addr,@rs
 flags:  ------
 ******************************************/
static void Z57_ssN0_0000_addr(void)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_W( addr, POPW(src) );
}

/******************************************
 pop     addr(rd),@rs
 flags:  ------
 ******************************************/
static void Z57_ssN0_ddN0_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, POPW(src) );
}

/******************************************
 multl   rqd,addr
 flags:  CZSV--
 ******************************************/
static void Z58_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = MULTL( RQ(dst), RDMEM_L(addr) );
}

/******************************************
 multl   rqd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z58_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RQ(dst) = MULTL( RQ(dst), RDMEM_L(addr) );
}

/******************************************
 mult    rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z59_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = MULTW( RL(dst), RDMEM_W(addr) );
}

/******************************************
 mult    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z59_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RL(dst) = MULTW( RL(dst), RDMEM_W(addr) );
}

/******************************************
 divl    rqd,addr
 flags:  CZSV--
 ******************************************/
static void Z5A_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RQ(dst) = DIVL( RQ(dst), RDMEM_L(addr) );
}

/******************************************
 divl    rqd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z5A_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RQ(dst) = DIVL( RQ(dst), RDMEM_L(addr) );
}

/******************************************
 div     rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z5B_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RL(dst) = DIVW( RL(dst), RDMEM_W(addr) );
}

/******************************************
 div     rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z5B_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RL(dst) = DIVW( RL(dst), RDMEM_W(addr) );
}

/******************************************
 ldm     rd,addr,n
 flags:  ------
 ******************************************/
static void Z5C_0000_0001_0000_dddd_0000_nmin1_addr(void)
{
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(addr);
		dst = (dst+1) & 15;
		addr = (addr + 2) & 0xffff;
	}
}

/******************************************
 testl   addr
 flags:  -ZS---
 ******************************************/
static void Z5C_0000_1000_addr(void)
{
	GET_ADDR(OP1);
	TESTL( RDMEM_L(addr) );
}

/******************************************
 ldm     addr,rs,n
 flags:  ------
 ******************************************/
static void Z5C_0000_1001_0000_ssss_0000_nmin1_addr(void)
{
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		WRMEM_W( addr, RW(src) );
		src = (src+1) & 15;
		addr = (addr + 2) & 0xffff;
	}
}

/******************************************
 testl   addr(rd)
 flags:  -ZS---
 ******************************************/
static void Z5C_ddN0_1000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	TESTL( RDMEM_L(addr) );
}

/******************************************
 ldm     addr(rd),rs,n
 flags:  ------
 ******************************************/
static void Z5C_ddN0_1001_0000_ssN0_0000_nmin1_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr += RW(dst);
	while (cnt-- >= 0) {
		WRMEM_W( addr, RW(src) );
		src = (src+1) & 15;
		addr = (addr + 2) & 0xffff;
	}
}

/******************************************
 ldm     rd,addr(rs),n
 flags:  ------
 ******************************************/
static void Z5C_ssN0_0001_0000_dddd_0000_nmin1_addr(void)
{
	GET_SRC(OP0,NIB2);
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr += RW(src);
	while (cnt-- >= 0) {
		RW(dst) = RDMEM_W(addr);
		dst = (dst+1) & 15;
		addr = (addr + 2) & 0xffff;
	}
}

/******************************************
 ldl     addr,rrs
 flags:  ------
 ******************************************/
static void Z5D_0000_ssss_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_L( addr, RL(src) );
}

/******************************************
 ldl     addr(rd),rrs
 flags:  ------
 ******************************************/
static void Z5D_ddN0_ssss_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_L( addr, RL(src) );
}

/******************************************
 jp      cc,addr
 flags:  ------
 ******************************************/
static void Z5E_0000_cccc_addr(void)
{
	GET_CCC(OP0,NIB3);
	GET_ADDR(OP1);
	switch (cc) {
		case  0: if (CC0) PC = addr; break;
		case  1: if (CC1) PC = addr; break;
		case  2: if (CC2) PC = addr; break;
		case  3: if (CC3) PC = addr; break;
		case  4: if (CC4) PC = addr; break;
		case  5: if (CC5) PC = addr; break;
		case  6: if (CC6) PC = addr; break;
		case  7: if (CC7) PC = addr; break;
		case  8: if (CC8) PC = addr; break;
		case  9: if (CC9) PC = addr; break;
		case 10: if (CCA) PC = addr; break;
		case 11: if (CCB) PC = addr; break;
		case 12: if (CCC) PC = addr; break;
		case 13: if (CCD) PC = addr; break;
		case 14: if (CCE) PC = addr; break;
		case 15: if (CCF) PC = addr; break;
	}
	change_pc(PC);
}

/******************************************
 jp      cc,addr(rd)
 flags:  ------
 ******************************************/
static void Z5E_ddN0_cccc_addr(void)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	switch (cc) {
		case  0: if (CC0) PC = addr; break;
		case  1: if (CC1) PC = addr; break;
		case  2: if (CC2) PC = addr; break;
		case  3: if (CC3) PC = addr; break;
		case  4: if (CC4) PC = addr; break;
		case  5: if (CC5) PC = addr; break;
		case  6: if (CC6) PC = addr; break;
		case  7: if (CC7) PC = addr; break;
		case  8: if (CC8) PC = addr; break;
		case  9: if (CC9) PC = addr; break;
		case 10: if (CCA) PC = addr; break;
		case 11: if (CCB) PC = addr; break;
		case 12: if (CCC) PC = addr; break;
		case 13: if (CCD) PC = addr; break;
		case 14: if (CCE) PC = addr; break;
		case 15: if (CCF) PC = addr; break;
	}
	change_pc(PC);
}

/******************************************
 call    addr
 flags:  ------
 ******************************************/
static void Z5F_0000_0000_addr(void)
{
	GET_ADDR(OP1);
	PUSHW( SP, PC );
	PC = addr;
	change_pc(PC);
}

/******************************************
 call    addr(rd)
 flags:  ------
 ******************************************/
static void Z5F_ddN0_0000_addr(void)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHW( SP, PC );
	addr += RW(dst);
	PC = addr;
	change_pc(PC);
}

/******************************************
 ldb     rbd,addr
 flags:  ------
 ******************************************/
static void Z60_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RB(dst) = RDMEM_B(addr);
}

/******************************************
 ldb     rbd,addr(rs)
 flags:  ------
 ******************************************/
static void Z60_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RB(dst) = RDMEM_B(addr);
}

/******************************************
 ld      rd,addr
 flags:  ------
 ******************************************/
static void Z61_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = RDMEM_W(addr);
}

/******************************************
 ld      rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z61_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
	RW(dst) = RDMEM_W(addr);
}

/******************************************
 resb    addr,imm4
 flags:  ------
 ******************************************/
static void Z62_0000_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B( addr, RDMEM_B(addr) & ~bit );
}

/******************************************
 resb    addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z62_ddN0_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, RDMEM_B(addr) & ~bit );
}

/******************************************
 res     addr,imm4
 flags:  ------
 ******************************************/
static void Z63_0000_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W( addr, RDMEM_W(addr) & ~bit );
}

/******************************************
 res     addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z63_ddN0_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, RDMEM_W(addr) & ~bit );
}

/******************************************
 setb    addr,imm4
 flags:  ------
 ******************************************/
static void Z64_0000_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B( addr, RDMEM_B(addr) | bit );
}

/******************************************
 setb    addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z64_ddN0_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, RDMEM_B(addr) | bit );
}

/******************************************
 set     addr,imm4
 flags:  ------
 ******************************************/
static void Z65_0000_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W( addr, RDMEM_W(addr) | bit );
}

/******************************************
 set     addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z65_ddN0_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, RDMEM_W(addr) | bit );
}

/******************************************
 bitb    addr,imm4
 flags:  -Z----
 ******************************************/
static void Z66_0000_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if ( RDMEM_B(addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bitb    addr(rd),imm4
 flags:  -Z----
 ******************************************/
static void Z66_ddN0_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
    addr += RW(dst);
    if ( RDMEM_B(addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr,imm4
 flags:  -Z----
 ******************************************/
static void Z67_0000_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if ( RDMEM_W(addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr(rd),imm4
 flags:  -Z----
 ******************************************/
static void Z67_ddN0_imm4_addr(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
    addr += RW(dst);
	if ( RDMEM_W(addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z68_0000_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B( addr, INCB(RDMEM_B(addr), i4p1) );
}

/******************************************
 incb    addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z68_ddN0_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, INCB(RDMEM_B(addr), i4p1) );
}

/******************************************
 inc     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z69_0000_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W( addr, INCW(RDMEM_W(addr), i4p1) );
}

/******************************************
 inc     addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z69_ddN0_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, INCW(RDMEM_W(addr), i4p1) );
}

/******************************************
 decb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6A_0000_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B( addr, DECB(RDMEM_B(addr), i4p1) );
}

/******************************************
 decb    addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6A_ddN0_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, DECB(RDMEM_B(addr), i4p1) );
}

/******************************************
 dec     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6B_0000_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W( addr, DECW(RDMEM_W(addr), i4p1) );
}

/******************************************
 dec     addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6B_ddN0_imm4m1_addr(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, DECW(RDMEM_W(addr), i4p1) );
}

/******************************************
 exb     rbd,addr
 flags:  ------
 ******************************************/
static void Z6C_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	UINT8 tmp = RDMEM_B(addr);
	WRMEM_B(addr, RB(dst));
	RB(dst) = tmp;
}

/******************************************
 exb     rbd,addr(rs)
 flags:  ------
 ******************************************/
static void Z6C_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	UINT8 tmp;
	addr += RW(src);
	tmp = RDMEM_B(addr);
	WRMEM_B(addr, RB(dst));
    RB(dst) = tmp;
}

/******************************************
 ex      rd,addr
 flags:  ------
 ******************************************/
static void Z6D_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	UINT16 tmp = RDMEM_W(addr);
	WRMEM_W( addr, RW(dst) );
	RW(dst) = tmp;
}

/******************************************
 ex      rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z6D_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	UINT16 tmp;
	addr += RW(src);
	tmp = RDMEM_W(addr);
	WRMEM_W( addr, RW(dst) );
    RW(dst) = tmp;
}

/******************************************
 ldb     addr,rbs
 flags:  ------
 ******************************************/
static void Z6E_0000_ssss_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B( addr, RB(src) );
}

/******************************************
 ldb     addr(rd),rbs
 flags:  ------
 ******************************************/
static void Z6E_ddN0_ssss_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_B( addr, RB(src) );
}

/******************************************
 ld      addr,rs
 flags:  ------
 ******************************************/
static void Z6F_0000_ssss_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W( addr, RW(src) );
}

/******************************************
 ld      addr(rd),rs
 flags:  ------
 ******************************************/
static void Z6F_ddN0_ssss_addr(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(dst);
	WRMEM_W( addr, RW(src) );
}

/******************************************
 ldb     rbd,rs(rx)
 flags:  ------
 ******************************************/
static void Z70_ssN0_dddd_0000_xxxx_0000_0000(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RB(dst) = RDMEM_B( (UINT16)(RW(src) + RW(idx)) );
}

/******************************************
 ld      rd,rs(rx)
 flags:  ------
 ******************************************/
static void Z71_ssN0_dddd_0000_xxxx_0000_0000(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RW(dst) = RDMEM_W( (UINT16)(RW(src) + RW(idx)) );
}

/******************************************
 ldb     rd(rx),rbs
 flags:  ------
 ******************************************/
static void Z72_ddN0_ssss_0000_xxxx_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRMEM_B( (UINT16)(RW(dst) + RW(idx)), RB(src) );
}

/******************************************
 ld      rd(rx),rs
 flags:  ------
 ******************************************/
static void Z73_ddN0_ssss_0000_xxxx_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRMEM_W( (UINT16)(RW(dst) + RW(idx)), RW(src) );
}

/******************************************
 lda     prd,rs(rx)
 flags:  ------
 ******************************************/
static void Z74_ssN0_dddd_0000_xxxx_0000_0000(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RW(dst) = (UINT16)(RW(src) + RW(idx));
}

/******************************************
 ldl     rrd,rs(rx)
 flags:  ------
 ******************************************/
static void Z75_ssN0_dddd_0000_xxxx_0000_0000(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	RL(dst) = RDMEM_L( (UINT16)(RW(src) + RW(idx)) );
}

/******************************************
 lda     prd,addr
 flags:  ------
 ******************************************/
static void Z76_0000_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	RW(dst) = addr;
}

/******************************************
 lda     prd,addr(rs)
 flags:  ------
 ******************************************/
static void Z76_ssN0_dddd_addr(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr += RW(src);
    RW(dst) = addr;
}

/******************************************
 ldl     rd(rx),rrs
 flags:  ------
 ******************************************/
static void Z77_ddN0_ssss_0000_xxxx_0000_0000(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRMEM_L( (UINT16)(RW(dst) + RW(idx)), RL(src) );
}

/******************************************
 rsvd78
 flags:  ------
 ******************************************/
static void Z78_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvd78 $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldps    addr
 flags:  CZSVDH
 ******************************************/
static void Z79_0000_0000_addr(void)
{
	GET_ADDR(OP1);
	UINT16 fcw;
	fcw = RDMEM_W(addr);
	PC	= RDMEM_W((UINT16)(addr + 2));
	CHANGE_FCW(fcw); /* check for user/system mode change */
    change_pc(PC);
}

/******************************************
 ldps    addr(rs)
 flags:  CZSVDH
 ******************************************/
static void Z79_ssN0_0000_addr(void)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	UINT16 fcw;
	addr += RW(src);
	fcw = RDMEM_W(addr);
	PC	= RDMEM_W((UINT16)(addr + 2));
	CHANGE_FCW(fcw); /* check for user/system mode change */
    change_pc(PC);
}

/******************************************
 halt
 flags:  ------
 ******************************************/
static void Z7A_0000_0000(void)
{
	IRQ_REQ |= Z8000_HALT;
	if (z8000_ICount > 0) z8000_ICount = 0;
}

/******************************************
 iret
 flags:  CZSVDH
 ******************************************/
static void Z7B_0000_0000(void)
{
	UINT16 tag, fcw;
	tag = POPW( SP );	/* get type tag */
	fcw = POPW( SP );	/* get FCW  */
	PC	= POPW( SP );	/* get PC   */
    IRQ_SRV &= ~tag;    /* remove IRQ serviced flag */
	CHANGE_FCW(fcw);		 /* check for user/system mode change */
    change_pc(PC);
	LOG(("Z8K#%d IRET tag $%04x, fcw $%04x, pc $%04x\n", cpu_getactivecpu(), tag, fcw, PC));
}

/******************************************
 mset
 flags:  ------
 ******************************************/
static void Z7B_0000_1000(void)
{
	/* set mu-0 line */
}

/******************************************
 mres
 flags:  ------
 ******************************************/
static void Z7B_0000_1001(void)
{
	/* reset mu-0 line */
}

/******************************************
 mbit
 flags:  CZS---
 ******************************************/
static void Z7B_0000_1010(void)
{
	/* test mu-I line */
}

/******************************************
 mreq    rd
 flags:  -ZS---
 ******************************************/
static void Z7B_dddd_1101(void)
{
	/* test mu-I line, invert cascade to mu-0  */
}

/******************************************
 di      i2
 flags:  ------
 ******************************************/
static void Z7C_0000_00ii(void)
{
	GET_IMM2(OP0,NIB3);
	UINT16 fcw = FCW;
	fcw &= ~(imm2 << 11);
	CHANGE_FCW(fcw);
}

/******************************************
 ei      i2
 flags:  ------
 ******************************************/
static void Z7C_0000_01ii(void)
{
	GET_IMM2(OP0,NIB3);
	UINT16 fcw = FCW;
	fcw |= imm2 << 11;
	CHANGE_FCW(fcw);
}

/******************************************
 ldctl   rd,ctrl
 flags:  ------
 ******************************************/
static void Z7D_dddd_0ccc(void)
{
	GET_IMM3(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (imm3) {
		case 0:
			RW(dst) = FCW;
			break;
		case 3:
			RW(dst) = REFRESH;
			break;
		case 5:
			RW(dst) = PSAP;
			break;
		case 7:
			RW(dst) = NSP;
			break;
		default:
			LOG(("Z8K#%d LDCTL R%d,%d\n", cpu_getactivecpu(), dst, imm3));
    }
}

/******************************************
 ldctl   ctrl,rs
 flags:  ------
 ******************************************/
static void Z7D_ssss_1ccc(void)
{
	GET_IMM3(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	switch (imm3) {
		case 0:
			{
				UINT16 fcw;
				fcw = RW(src);
				CHANGE_FCW(fcw); /* check for user/system mode change */
			}
            break;
		case 3:
			REFRESH = RW(src);
			break;
		case 5:
			PSAP = RW(src);
			break;
		case 7:
			NSP = RW(src);
			break;
		default:
			LOG(("Z8K#%d LDCTL %d,R%d\n", cpu_getactivecpu(), imm3, src));
    }
}

/******************************************
 rsvd7e
 flags:  ------
 ******************************************/
static void Z7E_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvd7e $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 sc      imm8
 flags:  CZSVDH
 ******************************************/
static void Z7F_imm8(void)
{
    GET_IMM8(0);
	/* execute system call via IRQ */
	IRQ_REQ = Z8000_SYSCALL | imm8;

}

/******************************************
 addb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void Z80_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADDB( RB(dst), RB(src) );
}

/******************************************
 add     rd,rs
 flags:  CZSV--
 ******************************************/
static void Z81_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADDW( RW(dst), RW(src) );
}

/******************************************
 subb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void Z82_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SUBB( RB(dst), RB(src) );
}

/******************************************
 sub     rd,rs
 flags:  CZSV--
 ******************************************/
static void Z83_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SUBW( RW(dst), RW(src) );
}

/******************************************
 orb     rbd,rbs
 flags:  CZSP--
 ******************************************/
static void Z84_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ORB( RB(dst), RB(src) );
}

/******************************************
 or      rd,rs
 flags:  CZS---
 ******************************************/
static void Z85_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ORW( RW(dst), RW(src) );
}

/******************************************
 andb    rbd,rbs
 flags:  -ZSP--
 ******************************************/
static void Z86_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ANDB( RB(dst), RB(src) );
}

/******************************************
 and     rd,rs
 flags:  -ZS---
 ******************************************/
static void Z87_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ANDW( RW(dst), RW(src) );
}

/******************************************
 xorb    rbd,rbs
 flags:  -ZSP--
 ******************************************/
static void Z88_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = XORB( RB(dst), RB(src) );
}

/******************************************
 xor     rd,rs
 flags:  -ZS---
 ******************************************/
static void Z89_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = XORW( RW(dst), RW(src) );
}

/******************************************
 cpb     rbd,rbs
 flags:  CZSV--
 ******************************************/
static void Z8A_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB( RB(dst), RB(src) );
}

/******************************************
 cp      rd,rs
 flags:  CZSV--
 ******************************************/
static void Z8B_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW( RW(dst), RW(src) );
}

/******************************************
 comb    rbd
 flags:  -ZSP--
 ******************************************/
static void Z8C_dddd_0000(void)
{
	GET_DST(OP0,NIB2);
	RB(dst) = COMB( RB(dst) );
}

/******************************************
 negb    rbd
 flags:  CZSV--
 ******************************************/
static void Z8C_dddd_0010(void)
{
	GET_DST(OP0,NIB2);
	RB(dst) = NEGB( RB(dst) );
}

/******************************************
 testb   rbd
 flags:  -ZSP--
 ******************************************/
static void Z8C_dddd_0100(void)
{
	GET_DST(OP0,NIB2);
	TESTB( RB(dst) );
}

/******************************************
 tsetb   rbd
 flags:  --S---
 ******************************************/
static void Z8C_dddd_0110(void)
{
	GET_DST(OP0,NIB2);
    if (RB(dst) & S08) SET_S; else CLR_S;
    RB(dst) = 0xff;
}

/******************************************
 clrb    rbd
 flags:  ------
 ******************************************/
static void Z8C_dddd_1000(void)
{
	GET_DST(OP0,NIB2);
	RB(dst) = 0;
}

/******************************************
 nop
 flags:  ------
 ******************************************/
static void Z8D_0000_0111(void)
{
	/* nothing */
}

/******************************************
 com     rd
 flags:  -ZS---
 ******************************************/
static void Z8D_dddd_0000(void)
{
	GET_DST(OP0,NIB2);
	RW(dst) = COMW( RW(dst) );
}

/******************************************
 neg     rd
 flags:  CZSV--
 ******************************************/
static void Z8D_dddd_0010(void)
{
	GET_DST(OP0,NIB2);
	RW(dst) = NEGW( RW(dst) );
}

/******************************************
 test    rd
 flags:  ------
 ******************************************/
static void Z8D_dddd_0100(void)
{
	GET_DST(OP0,NIB2);
	TESTW( RW(dst) );
}

/******************************************
 tset    rd
 flags:  --S---
 ******************************************/
static void Z8D_dddd_0110(void)
{
	GET_DST(OP0,NIB2);
    if (RW(dst) & S16) SET_S; else CLR_S;
    RW(dst) = 0xffff;
}

/******************************************
 clr     rd
 flags:  ------
 ******************************************/
static void Z8D_dddd_1000(void)
{
	GET_DST(OP0,NIB2);
	RW(dst) = 0;
}

/******************************************
 setflg  imm4
 flags:  CZSV--
 ******************************************/
static void Z8D_imm4_0001(void)
{
	FCW |= Z.op[0] & 0x00f0;
}

/******************************************
 resflg  imm4
 flags:  CZSV--
 ******************************************/
static void Z8D_imm4_0011(void)
{
	FCW &= ~(Z.op[0] & 0x00f0);
}

/******************************************
 comflg  flags
 flags:  CZSP--
 ******************************************/
static void Z8D_imm4_0101(void)
{
	FCW ^= (Z.op[0] & 0x00f0);
}

/******************************************
 ext8e   imm8
 flags:  ------
 ******************************************/
static void Z8E_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: ext8e  $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ext8f   imm8
 flags:  ------
 ******************************************/
static void Z8F_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: ext8f  $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 cpl     rrd,rrs
 flags:  CZSV--
 ******************************************/
static void Z90_ssss_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    CPL( RL(dst), RL(src) );
}

/******************************************
 pushl   @rd,rrs
 flags:  ------
 ******************************************/
static void Z91_ddN0_ssss(void)
{
    GET_SRC(OP0,NIB3);
    GET_DST(OP0,NIB2);
    PUSHL( dst, RL(src) );
}

/******************************************
 subl    rrd,rrs
 flags:  CZSV--
 ******************************************/
static void Z92_ssss_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    RL(dst) = SUBL( RL(dst), RL(src) );
}

/******************************************
 push    @rd,rs
 flags:  ------
 ******************************************/
static void Z93_ddN0_ssss(void)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(dst, RW(src));
}

/******************************************
 ldl     rrd,rrs
 flags:  ------
 ******************************************/
static void Z94_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = RL(src);
}

/******************************************
 popl    rrd,@rs
 flags:  ------
 ******************************************/
static void Z95_ssN0_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    RL(dst) = POPL( src );
}

/******************************************
 addl    rrd,rrs
 flags:  CZSV--
 ******************************************/
static void Z96_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RL(dst) = ADDL( RL(dst), RL(src) );
}

/******************************************
 pop     rd,@rs
 flags:  ------
 ******************************************/
static void Z97_ssN0_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    RW(dst) = POPW( src );
}

/******************************************
 multl   rqd,rrs
 flags:  CZSV--
 ******************************************/
static void Z98_ssss_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
	RQ(dst) = MULTL( RQ(dst), RL(src) );
}

/******************************************
 mult    rrd,rs
 flags:  CZSV--
 ******************************************/
static void Z99_ssss_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
	RL(dst) = MULTW( RL(dst), RW(src) );
}

/******************************************
 divl    rqd,rrs
 flags:  CZSV--
 ******************************************/
static void Z9A_ssss_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    RQ(dst) = DIVL( RQ(dst), RL(src) );
}

/******************************************
 div     rrd,rs
 flags:  CZSV--
 ******************************************/
static void Z9B_ssss_dddd(void)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    RL(dst) = DIVW( RL(dst), RW(src) );
}

/******************************************
 testl   rrd
 flags:  -ZS---
 ******************************************/
static void Z9C_dddd_1000(void)
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
static void Z9D_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvd9d $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ret     cc
 flags:  ------
 ******************************************/
static void Z9E_0000_cccc(void)
{
	GET_CCC(OP0,NIB3);
	switch (cc) {
		case  0: if (CC0) PC = POPW( SP ); break;
		case  1: if (CC1) PC = POPW( SP ); break;
		case  2: if (CC2) PC = POPW( SP ); break;
		case  3: if (CC3) PC = POPW( SP ); break;
		case  4: if (CC4) PC = POPW( SP ); break;
		case  5: if (CC5) PC = POPW( SP ); break;
		case  6: if (CC6) PC = POPW( SP ); break;
		case  7: if (CC7) PC = POPW( SP ); break;
		case  8: if (CC8) PC = POPW( SP ); break;
		case  9: if (CC9) PC = POPW( SP ); break;
		case 10: if (CCA) PC = POPW( SP ); break;
		case 11: if (CCB) PC = POPW( SP ); break;
		case 12: if (CCC) PC = POPW( SP ); break;
		case 13: if (CCD) PC = POPW( SP ); break;
		case 14: if (CCE) PC = POPW( SP ); break;
		case 15: if (CCF) PC = POPW( SP ); break;
	}
	change_pc(PC);
}

/******************************************
 rsvd9f
 flags:  ------
 ******************************************/
static void Z9F_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvd9f $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldb     rbd,rbs
 flags:  ------
 ******************************************/
static void ZA0_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = RB(src);
}

/******************************************
 ld      rd,rs
 flags:  ------
 ******************************************/
static void ZA1_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = RW(src);
}

/******************************************
 resb    rbd,imm4
 flags:  ------
 ******************************************/
static void ZA2_dddd_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RB(dst) &= ~bit;
}

/******************************************
 res     rd,imm4
 flags:  ------
 ******************************************/
static void ZA3_dddd_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RW(dst) &= ~bit;
}

/******************************************
 setb    rbd,imm4
 flags:  ------
 ******************************************/
static void ZA4_dddd_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RB(dst) |= bit;
}

/******************************************
 set     rd,imm4
 flags:  ------
 ******************************************/
static void ZA5_dddd_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	RW(dst) |= bit;
}

/******************************************
 bitb    rbd,imm4
 flags:  -Z----
 ******************************************/
static void ZA6_dddd_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RB(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     rd,imm4
 flags:  -Z----
 ******************************************/
static void ZA7_dddd_imm4(void)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RW(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    rbd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZA8_dddd_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RB(dst) = INCB( RB(dst), i4p1);
}

/******************************************
 inc     rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZA9_dddd_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RW(dst) = INCW( RW(dst), i4p1 );
}

/******************************************
 decb    rbd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZAA_dddd_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RB(dst) = DECB( RB(dst), i4p1 );
}

/******************************************
 dec     rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZAB_dddd_imm4m1(void)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	RW(dst) = DECW( RW(dst), i4p1 );
}

/******************************************
 exb     rbd,rbs
 flags:  ------
 ******************************************/
static void ZAC_ssss_dddd(void)
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
static void ZAD_ssss_dddd(void)
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
static void ZAE_dddd_cccc(void)
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
static void ZAF_dddd_cccc(void)
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
static void ZB0_dddd_0000(void)
{
	GET_DST(OP0,NIB2);
	UINT8 result;
	UINT16 idx = RB(dst);
	if (FCW & F_C)	idx |= 0x100;
	if (FCW & F_H)	idx |= 0x200;
	if (FCW & F_DA) idx |= 0x400;
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
static void ZB1_dddd_0000(void)
{
	GET_DST(OP0,NIB2);
    RW(dst) = (RW(dst) & 0xff) | ((RW(dst) & S08) ? 0xff00 : 0x0000);
}

/******************************************
 extsl   rqd
 flags:  ------
 ******************************************/
static void ZB1_dddd_0111(void)
{
	GET_DST(OP0,NIB2);
	RQ(dst) = COMBINE_U64_U32_U32( (RQ(dst) & S32) ?
		0xfffffffful : 0, LO32_U32_U64(RQ(dst)));
}

/******************************************
 exts    rrd
 flags:  ------
 ******************************************/
static void ZB1_dddd_1010(void)
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
static void ZB2_dddd_0001_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RB(dst) = SRLB( RB(dst), -(INT16)imm16 );
	else
		RB(dst) = SLLB( RB(dst), imm16 );
}

/******************************************
 sdlb    rbd,rs
 flags:  CZS---
 ******************************************/
static void ZB2_dddd_0011_0000_ssss_0000_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RB(dst) = SRLB( RB(dst), (INT8)RW(src) );
}

/******************************************
 rlb     rbd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_00I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RLB( RB(dst), imm1 );
}

/******************************************
 rrb     rbd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_01I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RRB( RB(dst), imm1 );
}

/******************************************
 slab    rbd,imm8
 flags:  CZSV--
 srab    rbd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_1001_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RB(dst) = SRAB( RB(dst), -(INT16)imm16 );
	else
		RB(dst) = SLAB( RB(dst), imm16 );
}

/******************************************
 sdab    rbd,rs
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_1011_0000_ssss_0000_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RB(dst) = SDAB( RB(dst), (INT8) RW(src) );
}

/******************************************
 rlcb    rbd,imm1or2
 flags:  -Z----
 ******************************************/
static void ZB2_dddd_10I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RLCB( RB(dst), imm1 );
}

/******************************************
 rrcb    rbd,imm1or2
 flags:  -Z----
 ******************************************/
static void ZB2_dddd_11I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RB(dst) = RRCB( RB(dst), imm1 );
}

/******************************************
 sll     rd,imm8
 flags:  CZS---
 srl     rd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_0001_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RW(dst) = SRLW( RW(dst), -(INT16)imm16 );
	else
        RW(dst) = SLLW( RW(dst), imm16 );
}

/******************************************
 sdl     rd,rs
 flags:  CZS---
 ******************************************/
static void ZB3_dddd_0011_0000_ssss_0000_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RW(dst) = SDLW( RW(dst), (INT8)RW(src) );
}

/******************************************
 rl      rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_00I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RLW( RW(dst), imm1 );
}

/******************************************
 slll    rrd,imm8
 flags:  CZS---
 srll    rrd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_0101_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RL(dst) = SRLL( RL(dst), -(INT16)imm16 );
	else
		RL(dst) = SLLL( RL(dst), imm16 );
}

/******************************************
 sdll    rrd,rs
 flags:  CZS---
 ******************************************/
static void ZB3_dddd_0111_0000_ssss_0000_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RL(dst) = SDLL( RL(dst), RW(src) & 0xff );
}

/******************************************
 rr      rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_01I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RRW( RW(dst), imm1 );
}

/******************************************
 sla     rd,imm8
 flags:  CZSV--
 sra     rd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1001_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RW(dst) = SRAW( RW(dst), -(INT16)imm16 );
	else
        RW(dst) = SLAW( RW(dst), imm16 );
}

/******************************************
 sda     rd,rs
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1011_0000_ssss_0000_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RW(dst) = SDAW( RW(dst), (INT8)RW(src) );
}

/******************************************
 rlc     rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_10I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RLCW( RW(dst), imm1 );
}

/******************************************
 slal    rrd,imm8
 flags:  CZSV--
 sral    rrd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1101_imm8(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		RL(dst) = SRAL( RL(dst), -(INT16)imm16 );
	else
		RL(dst) = SLAL( RL(dst), imm16 );
}

/******************************************
 sdal    rrd,rs
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1111_0000_ssss_0000_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	RL(dst) = SDAL( RL(dst), RW(src) & 0xff );
}

/******************************************
 rrc     rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_11I0(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	RW(dst) = RRCW( RW(dst), imm1 );
}

/******************************************
 adcb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void ZB4_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = ADCB( RB(dst), RB(src) );
}

/******************************************
 adc     rd,rs
 flags:  CZSV--
 ******************************************/
static void ZB5_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = ADCW( RW(dst), RW(src) );
}

/******************************************
 sbcb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void ZB6_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RB(dst) = SBCB( RB(dst), RB(src) );
}

/******************************************
 sbc     rd,rs
 flags:  CZSV--
 ******************************************/
static void ZB7_ssss_dddd(void)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	RW(dst) = SBCW( RW(dst), RW(src) );
}

/******************************************
 trtib   @rd,@rs,rr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0010_0000_rrrr_ssN0_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	RB(2) = xlt;
	if (xlt) CLR_Z; else SET_Z;
	RW(dst)++;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trtirb  @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0110_0000_rrrr_ssN0_1110(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	RB(2) = xlt;
	if (xlt) CLR_Z; else SET_Z;
	RW(dst)++;
	if (--RW(cnt)) { CLR_V; PC -= 4; } else SET_V;
}

/******************************************
 trtdb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1010_0000_rrrr_ssN0_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	RB(2) = xlt;
	if (xlt) CLR_Z; else SET_Z;
    RW(dst)--;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trtdrb  @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1110_0000_rrrr_ssN0_1110(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	RB(2) = xlt;
	if (xlt) CLR_Z; else SET_Z;
    RW(dst)--;
	if (--RW(cnt)) { CLR_V; PC -= 4; } else SET_V;
}

/******************************************
 trib    @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0000_0000_rrrr_ssN0_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	WRMEM_B( RW(dst), xlt );
	RW(dst)++;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trirb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0100_0000_rrrr_ssN0_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	WRMEM_B( RW(dst), xlt );
	RW(dst)++;
	if (--RW(cnt)) { CLR_V; PC -= 4; } else SET_V;
}

/******************************************
 trdb    @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1000_0000_rrrr_ssN0_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	WRMEM_B( RW(dst), xlt );
    RW(dst)--;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trdrb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1100_0000_rrrr_ssN0_0000(void)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B( (UINT16)(RW(src) + RDMEM_B(RW(dst))) );
	WRMEM_B( RW(dst), xlt );
    RW(dst)--;
	if (--RW(cnt)) { CLR_V; PC -= 4; } else SET_V;
}

/******************************************
 rsvdb9
 flags:  ------
 ******************************************/
static void ZB9_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvdb9 $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
	(void)imm8;
}

/******************************************
 cpib    rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0000_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB( RB(dst), RDMEM_B(RW(src)) );
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
	RW(src)++;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldib    @rd,@rs,rr
 ldibr   @rd,@rs,rr
 flags:  ---V--
 ******************************************/
static void ZBA_ssN0_0001_0000_rrrr_ddN0_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);	/* repeat? */
    WRMEM_B( RW(dst), RDMEM_B(RW(src)) );
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 cpsib   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0010_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB( RDMEM_B(RW(dst)), RDMEM_B(RW(src)) );
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
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpirb   rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0100_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB( RB(dst), RDMEM_B(RW(src)) );
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
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpsirb  @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0110_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB( RDMEM_B(RW(dst)), RDMEM_B(RW(src)) );
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
	RW(dst)++;
	RW(src)++;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpdb    rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1000_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
    CPB( RB(dst), RDMEM_B(RW(src)) );
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
    RW(src)--;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 lddb    @rs,@rd,rr
 lddbr   @rs,@rd,rr
 flags:  ---V--
 ******************************************/
static void ZBA_ssN0_1001_0000_rrrr_ddN0_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_B( RW(dst), RDMEM_B(RW(src)) );
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 cpsdb   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1010_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
    CPB( RDMEM_B(RW(dst)), RDMEM_B(RW(src)) );
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
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpdrb   rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1100_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB( RB(dst), RDMEM_B(RW(src)) );
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
    RW(src)--;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpsdrb  @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1110_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
    CPB( RDMEM_B(RW(dst)), RDMEM_B(RW(src)) );
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
	RW(dst)--;
	RW(src)--;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpi     rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0000_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RW(dst), RDMEM_W(RW(src)) );
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
    RW(src) += 2;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldi     @rd,@rs,rr
 ldir    @rd,@rs,rr
 flags:  ---V--
 ******************************************/
static void ZBB_ssN0_0001_0000_rrrr_ddN0_x000(void)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W( RW(dst), RDMEM_W(RW(src)) );
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 cpsi    @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0010_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RDMEM_W(RW(dst)), RDMEM_W(RW(src)) );
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
	RW(dst) += 2;
	RW(src) += 2;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpir    rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0100_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RW(dst), RDMEM_W(RW(src)) );
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
	RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpsir   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0110_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RDMEM_W(RW(dst)), RDMEM_W(RW(src)) );
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
	RW(dst) += 2;
    RW(src) += 2;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpd     rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1000_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RW(dst), RDMEM_W(RW(src)) );
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
    RW(src) -= 2;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldd     @rs,@rd,rr
 lddr    @rs,@rd,rr
 flags:  ---V--
 ******************************************/
static void ZBB_ssN0_1001_0000_rrrr_ddN0_x000(void)
{
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
    WRMEM_W( RW(dst), RDMEM_W(RW(src)) );
    RW(dst) -= 2;
    RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (cc == 0) PC -= 4; } else SET_V;
}

/******************************************
 cpsd    @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1010_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RDMEM_W(RW(dst)), RDMEM_W(RW(src)) );
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
    RW(dst) -= 2;
    RW(src) -= 2;
	if (--RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpdr    rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1100_0000_rrrr_dddd_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RW(dst), RDMEM_W(RW(src)) );
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
    RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 cpsdr   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1110_0000_rrrr_ddN0_cccc(void)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW( RDMEM_W(RW(dst)), RDMEM_W(RW(src)) );
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
    RW(dst) -= 2;
    RW(src) -= 2;
	if (--RW(cnt)) { CLR_V; if (!(FCW & F_Z)) PC -= 4; } else SET_V;
}

/******************************************
 rrdb    rbb,rba
 flags:  -Z----
 ******************************************/
static void ZBC_aaaa_bbbb(void)
{
	UINT8 b = Z.op[0] & 15;
	UINT8 a = (Z.op[0] >> 4) & 15;
	UINT8 tmp = RB(b);
	RB(a) = (RB(a) >> 4) | (RB(b) << 4);
	RB(b) = (RB(b) & 0xf0) | (tmp & 0x0f);
    if (RB(b)) CLR_Z; else SET_Z;
}

/******************************************
 ldk     rd,imm4
 flags:  ------
 ******************************************/
static void ZBD_dddd_imm4(void)
{
	GET_DST(OP0,NIB2);
	GET_IMM4(OP0,NIB3);
	RW(dst) = imm4;
}

/******************************************
 rldb    rbb,rba
 flags:  -Z----
 ******************************************/
static void ZBE_aaaa_bbbb(void)
{
	UINT8 b = Z.op[0] & 15;
	UINT8 a = (Z.op[0] >> 4) & 15;
	UINT8 tmp = RB(a);
	RB(a) = (RB(a) << 4) | (RB(b) & 0x0f);
	RB(b) = (RB(b) & 0xf0) | (tmp >> 4);
	if (RB(b)) CLR_Z; else SET_Z;
}

/******************************************
 rsvdbf
 flags:  ------
 ******************************************/
static void ZBF_imm8(void)
{
	GET_IMM8(0);
	LOG(("Z8K#%d %04x: rsvdbf $%02x\n", cpu_getactivecpu(), PC, imm8));
    if (FCW & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
	(void)imm8;
}

/******************************************
 ldb     rbd,imm8
 flags:  ------
 ******************************************/
static void ZC_dddd_imm8(void)
{
	GET_DST(OP0,NIB1);
	GET_IMM8(0);
	RB(dst) = imm8;
}

/******************************************
 calr    dsp12
 flags:  ------
 ******************************************/
static void ZD_dsp12(void)
{
	INT16 dsp12 = Z.op[0] & 0xfff;
	PUSHW( SP, PC );
	dsp12 = (dsp12 & 2048) ? 4096 -2 * (dsp12 & 2047) : -2 * (dsp12 & 2047);
	PC += dsp12;
	change_pc(PC);
}

/******************************************
 jr      cc,dsp8
 flags:  ------
 ******************************************/
static void ZE_cccc_dsp8(void)
{
	GET_DSP8;
	GET_CCC(OP0,NIB1);
	switch (cc) {
		case  0: if (CC0) PC += dsp8 * 2; break;
		case  1: if (CC1) PC += dsp8 * 2; break;
		case  2: if (CC2) PC += dsp8 * 2; break;
		case  3: if (CC3) PC += dsp8 * 2; break;
		case  4: if (CC4) PC += dsp8 * 2; break;
		case  5: if (CC5) PC += dsp8 * 2; break;
		case  6: if (CC6) PC += dsp8 * 2; break;
		case  7: if (CC7) PC += dsp8 * 2; break;
		case  8: if (CC8) PC += dsp8 * 2; break;
		case  9: if (CC9) PC += dsp8 * 2; break;
		case 10: if (CCA) PC += dsp8 * 2; break;
		case 11: if (CCB) PC += dsp8 * 2; break;
		case 12: if (CCC) PC += dsp8 * 2; break;
		case 13: if (CCD) PC += dsp8 * 2; break;
		case 14: if (CCE) PC += dsp8 * 2; break;
		case 15: if (CCF) PC += dsp8 * 2; break;
    }
	change_pc(PC);
}

/******************************************
 dbjnz   rbd,dsp7
 flags:  ------
 ******************************************/
static void ZF_dddd_0dsp7(void)
{
	GET_DST(OP0,NIB1);
    GET_DSP7;
    RB(dst) -= 1;
    if (RB(dst)) {
        PC = PC - 2 * dsp7;
        change_pc(PC);
    }
}

/******************************************
 djnz    rd,dsp7
 flags:  ------
 ******************************************/
static void ZF_dddd_1dsp7(void)
{
	GET_DST(OP0,NIB1);
	GET_DSP7;
	RW(dst) -= 1;
	if (RW(dst)) {
		PC = PC - 2 * dsp7;
		change_pc(PC);
	}
}


