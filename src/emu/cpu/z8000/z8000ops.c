/*****************************************************************************
 *
 *   z8000ops.c
 *   Portable Z8000(2) emulator
 *   Opcode functions
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   Bug fixes and MSB_FIRST compliance Ernesto Corvi.
 *   Bug fixes and segmented mode support Christian Groessler.
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
INLINE void CHANGE_FCW(z8000_state *cpustate, UINT16 fcw)
{
	if (fcw & F_S_N) {			/* system mode now? */
		if (!(cpustate->fcw & F_S_N)) {	/* and not before? */
            if (cpustate->device->type() == Z8001) {
                UINT16 tmp = cpustate->RW(15);
                cpustate->RW(15) = cpustate->nspoff;
                cpustate->nspoff = tmp;
                tmp = cpustate->RW(14);
                cpustate->RW(14) = cpustate->nspseg;
                cpustate->nspseg = tmp;
            }
            else {
                UINT16 tmp = cpustate->RW(SP);
                cpustate->RW(SP) = cpustate->nspoff;
                cpustate->nspoff = tmp;
            }
		}
	} else {					/* user mode now */
		if (cpustate->fcw & F_S_N) {		/* and not before? */
            if (cpustate->device->type() == Z8001) {
                UINT16 tmp = cpustate->RW(15);
                cpustate->RW(15) = cpustate->nspoff;
                cpustate->nspoff = tmp;
                tmp = cpustate->RW(14);
                cpustate->RW(14) = cpustate->nspseg;
                cpustate->nspseg = tmp;
            }
            else {
                UINT16 tmp = cpustate->RW(SP);
                cpustate->RW(SP) = cpustate->nspoff;
                cpustate->nspoff = tmp;
            }
        }
    }
    if (!(cpustate->fcw & F_NVIE) && (fcw & F_NVIE) && (cpustate->irq_state[0] != CLEAR_LINE))
		cpustate->irq_req |= Z8000_NVI;
	if (!(cpustate->fcw & F_VIE) && (fcw & F_VIE) && (cpustate->irq_state[1] != CLEAR_LINE))
		cpustate->irq_req |= Z8000_VI;
    cpustate->fcw = fcw;  /* set new cpustate->fcw */
}

INLINE UINT32 make_segmented_addr(UINT32 addr)
{
    return ((addr & 0x007f0000) << 8) | 0x80000000 | (addr & 0xffff);
}

INLINE UINT32 segmented_addr(UINT32 addr)
{
    return ((addr & 0x7f000000) >> 8) | (addr & 0xffff);
}

INLINE UINT32 addr_from_reg(z8000_state *cpustate, int regno)
{
    if (segmented_mode(cpustate))
        return segmented_addr(cpustate->RL(regno));
    else
        return cpustate->RW(regno);
}

INLINE void addr_to_reg(z8000_state *cpustate, int regno, UINT32 addr)
{
	if (segmented_mode(cpustate)) {
		UINT32 segaddr = make_segmented_addr(addr);
		cpustate->RW(regno) = (cpustate->RW(regno) & 0x80ff) | ((segaddr >> 16) & 0x7f00);
		cpustate->RW(regno | 1) = segaddr & 0xffff;
	}
    else
        cpustate->RW(regno) = addr;
}

INLINE void add_to_addr_reg(z8000_state *cpustate, int regno, UINT16 addend)
{
	if (segmented_mode(cpustate))
		regno |= 1;
	cpustate->RW(regno) += addend;
}

INLINE void sub_from_addr_reg(z8000_state *cpustate, int regno, UINT16 subtrahend)
{
	if (segmented_mode(cpustate))
		regno |= 1;
	cpustate->RW(regno) -= subtrahend;
}

INLINE void PUSHW(z8000_state *cpustate, UINT8 dst, UINT16 value)
{
    if (segmented_mode(cpustate))
        cpustate->RW(dst | 1) -= 2;
    else
        cpustate->RW(dst) -= 2;
	WRMEM_W(cpustate, addr_from_reg(cpustate, dst), value);
}

INLINE UINT16 POPW(z8000_state *cpustate, UINT8 src)
{
	UINT16 result = RDMEM_W(cpustate, addr_from_reg(cpustate, src));
    if (segmented_mode(cpustate))
        cpustate->RW(src | 1) += 2;
    else
        cpustate->RW(src) += 2;
	return result;
}

INLINE void PUSHL(z8000_state *cpustate, UINT8 dst, UINT32 value)
{
    if (segmented_mode(cpustate))
        cpustate->RW(dst | 1) -= 4;
    else
        cpustate->RW(dst) -= 4;
	WRMEM_L(cpustate,  addr_from_reg(cpustate, dst), value);
}

INLINE UINT32 POPL(z8000_state *cpustate, UINT8 src)
{
	UINT32 result = RDMEM_L(cpustate, addr_from_reg(cpustate, src));
    if (segmented_mode(cpustate))
        cpustate->RW(src | 1) += 4;
    else
        cpustate->RW(src) += 4;
	return result;
}

/* check zero and sign flag for byte, word and long results */
#define CHK_XXXB_ZS if (!result) SET_Z; else if ((INT8) result < 0) SET_S
#define CHK_XXXW_ZS if (!result) SET_Z; else if ((INT16)result < 0) SET_S
#define CHK_XXXL_ZS if (!result) SET_Z; else if ((INT32)result < 0) SET_S
#define CHK_XXXQ_ZS if (!result) SET_Z; else if ((INT64)result < 0) SET_S

#define CHK_XXXB_ZSP cpustate->fcw |= z8000_zsp[result]

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

/* check for privileged instruction and trap if executed */
#define CHECK_PRIVILEGED_INSTR() if (!(cpustate->fcw & F_S_N)) { cpustate->irq_req = Z8000_TRAP; return; }

/* if no EPU is present (it isn't), raise an extended intstuction trap */
#define CHECK_EXT_INSTR()  if (!(cpustate->fcw & F_EPU)) { cpustate->irq_req = Z8000_EPU; return; }


/******************************************
 add byte
 flags:  CZSVDH
 ******************************************/
INLINE UINT8 ADDB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 ADDW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT32 ADDL(z8000_state *cpustate, UINT32 dest, UINT32 value)
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
INLINE UINT8 ADCB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 ADCW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT8 SUBB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 SUBW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT32 SUBL(z8000_state *cpustate, UINT32 dest, UINT32 value)
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
INLINE UINT8 SBCB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 SBCW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT8 ORB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 ORW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT8 ANDB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 ANDW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT8 XORB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 XORW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE void CPB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE void CPW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE void CPL(z8000_state *cpustate, UINT32 dest, UINT32 value)
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
INLINE UINT8 COMB(z8000_state *cpustate, UINT8 dest)
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
INLINE UINT16 COMW(z8000_state *cpustate, UINT16 dest)
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
INLINE UINT8 NEGB(z8000_state *cpustate, UINT8 dest)
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
INLINE UINT16 NEGW(z8000_state *cpustate, UINT16 dest)
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
INLINE void TESTB(z8000_state *cpustate, UINT8 result)
{
	CLR_ZSP;
	CHK_XXXB_ZSP;	/* set Z and S flags for result byte       */
}

/******************************************
 test word
 flags:  -ZS---
 ******************************************/
INLINE void TESTW(z8000_state *cpustate, UINT16 dest)
{
	CLR_ZS;
    if (!dest) SET_Z; else if (dest & S16) SET_S;
}

/******************************************
 test long
 flags:  -ZS---
 ******************************************/
INLINE void TESTL(z8000_state *cpustate, UINT32 dest)
{
	CLR_ZS;
	if (!dest) SET_Z; else if (dest & S32) SET_S;
}

/******************************************
 increment byte
 flags: -ZSV--
 ******************************************/
INLINE UINT8 INCB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 INCW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT8 DECB(z8000_state *cpustate, UINT8 dest, UINT8 value)
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
INLINE UINT16 DECW(z8000_state *cpustate, UINT16 dest, UINT16 value)
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
INLINE UINT32 MULTW(z8000_state *cpustate, UINT16 dest, UINT16 value)
{
	UINT32 result = (INT32)(INT16)dest * (INT16)value;
	CLR_CZSV;
    CHK_XXXL_ZS;
	if(!value)
	{
		/* multiplication with zero is faster */
        cpustate->icount += (70-18);
	}
	if((INT32)result < -0x7fff || (INT32)result >= 0x7fff) SET_C;
	return result;
}

/******************************************
 multiply longs
 flags:  CZSV--
 ******************************************/
INLINE UINT64 MULTL(z8000_state *cpustate, UINT32 dest, UINT32 value)
{
	UINT64 result = (INT64)(INT32)dest * (INT32)value;
    if(!value)
	{
		/* multiplication with zero is faster */
		cpustate->icount += (282 - 30);
	}
	else
	{
		int n;
		for(n = 0; n < 32; n++)
			if(dest & (1L << n)) cpustate->icount -= 7;
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
INLINE UINT32 DIVW(z8000_state *cpustate, UINT32 dest, UINT16 value)
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
INLINE UINT64 DIVL(z8000_state *cpustate, UINT64 dest, UINT32 value)
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
INLINE UINT8 RLB(z8000_state *cpustate, UINT8 dest, UINT8 twice)
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
INLINE UINT16 RLW(z8000_state *cpustate, UINT16 dest, UINT8 twice)
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
INLINE UINT8 RLCB(z8000_state *cpustate, UINT8 dest, UINT8 twice)
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
INLINE UINT16 RLCW(z8000_state *cpustate, UINT16 dest, UINT8 twice)
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
INLINE UINT8 RRB(z8000_state *cpustate, UINT8 dest, UINT8 twice)
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
INLINE UINT16 RRW(z8000_state *cpustate, UINT16 dest, UINT8 twice)
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
INLINE UINT8 RRCB(z8000_state *cpustate, UINT8 dest, UINT8 twice)
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
INLINE UINT16 RRCW(z8000_state *cpustate, UINT16 dest, UINT8 twice)
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
INLINE UINT8 SDAB(z8000_state *cpustate, UINT8 dest, INT8 count)
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
INLINE UINT16 SDAW(z8000_state *cpustate, UINT16 dest, INT8 count)
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
INLINE UINT32 SDAL(z8000_state *cpustate, UINT32 dest, INT8 count)
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
INLINE UINT8 SDLB(z8000_state *cpustate, UINT8 dest, INT8 count)
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
INLINE UINT16 SDLW(z8000_state *cpustate, UINT16 dest, INT8 count)
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
INLINE UINT32 SDLL(z8000_state *cpustate, UINT32 dest, INT8 count)
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
INLINE UINT8 SLAB(z8000_state *cpustate, UINT8 dest, UINT8 count)
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
INLINE UINT16 SLAW(z8000_state *cpustate, UINT16 dest, UINT8 count)
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
INLINE UINT32 SLAL(z8000_state *cpustate, UINT32 dest, UINT8 count)
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
INLINE UINT8 SLLB(z8000_state *cpustate, UINT8 dest, UINT8 count)
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
INLINE UINT16 SLLW(z8000_state *cpustate, UINT16 dest, UINT8 count)
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
INLINE UINT32 SLLL(z8000_state *cpustate, UINT32 dest, UINT8 count)
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
INLINE UINT8 SRAB(z8000_state *cpustate, UINT8 dest, UINT8 count)
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
INLINE UINT16 SRAW(z8000_state *cpustate, UINT16 dest, UINT8 count)
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
INLINE UINT32 SRAL(z8000_state *cpustate, UINT32 dest, UINT8 count)
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
INLINE UINT8 SRLB(z8000_state *cpustate, UINT8 dest, UINT8 count)
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
INLINE UINT16 SRLW(z8000_state *cpustate, UINT16 dest, UINT8 count)
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
INLINE UINT32 SRLL(z8000_state *cpustate, UINT32 dest, UINT8 count)
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
static void  zinvalid(z8000_state *cpustate)
{
	logerror("Z8000 invalid opcode %04x: %04x\n", cpustate->pc, cpustate->op[0]);
}

/******************************************
 addb    rbd,imm8
 flags:  CZSVDH
 ******************************************/
static void  Z00_0000_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	cpustate->RB(dst) = ADDB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 addb    rbd,@rs
 flags:  CZSVDH
 ******************************************/
static void Z00_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ADDB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 add     rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z01_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RW(dst) = ADDW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 add     rd,@rs
 flags:  CZSV--
 ******************************************/
static void Z01_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ADDW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 subb    rbd,imm8
 flags:  CZSVDH
 ******************************************/
static void Z02_0000_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	cpustate->RB(dst) = SUBB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 subb    rbd,@rs
 flags:  CZSVDH
 ******************************************/
static void Z02_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = SUBB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src))); /* EHC */
}

/******************************************
 sub     rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z03_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RW(dst) = SUBW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 sub     rd,@rs
 flags:  CZSV--
 ******************************************/
static void Z03_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = SUBW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 orb     rbd,imm8
 flags:  CZSP--
 ******************************************/
static void Z04_0000_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	cpustate->RB(dst) = ORB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 orb     rbd,@rs
 flags:  CZSP--
 ******************************************/
static void Z04_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ORB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 or      rd,imm16
 flags:  CZS---
 ******************************************/
static void Z05_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RW(dst) = ORW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 or      rd,@rs
 flags:  CZS---
 ******************************************/
static void Z05_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ORW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 andb    rbd,imm8
 flags:  -ZSP--
 ******************************************/
static void Z06_0000_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	cpustate->RB(dst) = ANDB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 andb    rbd,@rs
 flags:  -ZSP--
 ******************************************/
static void Z06_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ANDB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 and     rd,imm16
 flags:  -ZS---
 ******************************************/
static void Z07_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RW(dst) = ANDW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 and     rd,@rs
 flags:  -ZS---
 ******************************************/
static void Z07_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ANDW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 xorb    rbd,imm8
 flags:  -ZSP--
 ******************************************/
static void Z08_0000_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	cpustate->RB(dst) = XORB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 xorb    rbd,@rs
 flags:  -ZSP--
 ******************************************/
static void Z08_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = XORB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 xor     rd,imm16
 flags:  -ZS---
 ******************************************/
static void Z09_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RW(dst) = XORW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 xor     rd,@rs
 flags:  -ZS---
 ******************************************/
static void Z09_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = XORW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 cpb     rbd,imm8
 flags:  CZSV--
 ******************************************/
static void Z0A_0000_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM8(OP1);
	CPB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 cpb     rbd,@rs
 flags:  CZSV--
 ******************************************/
static void Z0A_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 cp      rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z0B_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	CPW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 cp      rd,@rs
 flags:  CZSV--
 ******************************************/
static void Z0B_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate,src)));
}

/******************************************
 comb    @rd
 flags:  -ZSP--
 ******************************************/
static void Z0C_ddN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_B(cpustate, addr, COMB(cpustate, RDMEM_B(cpustate, addr)));
}

/******************************************
 cpb     @rd,imm8
 flags:  CZSV--
 ******************************************/
static void Z0C_ddN0_0001_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	CPB(cpustate, RDMEM_B(cpustate, addr_from_reg(cpustate, dst)), imm8); // @@@done
}

/******************************************
 negb    @rd
 flags:  CZSV--
 ******************************************/
static void Z0C_ddN0_0010(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_B(cpustate,  addr, NEGB(cpustate, RDMEM_B(cpustate, addr)));
}

/******************************************
 testb   @rd
 flags:  -ZSP--
 ******************************************/
static void Z0C_ddN0_0100(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	TESTB(cpustate, RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
}

/******************************************
 ldb     @rd,imm8
 flags:  ------
 ******************************************/
static void Z0C_ddN0_0101_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	WRMEM_B(cpustate, addr_from_reg(cpustate, dst), imm8);
}

/******************************************
 tsetb   @rd
 flags:  --S---
 ******************************************/
static void Z0C_ddN0_0110(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
    if (RDMEM_B(cpustate, addr) & S08) SET_S; else CLR_S;
    WRMEM_B(cpustate, addr, 0xff);
}

/******************************************
 clrb    @rd
 flags:  ------
 ******************************************/
static void Z0C_ddN0_1000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	WRMEM_B(cpustate,  addr_from_reg(cpustate, dst), 0);
}

/******************************************
 com     @rd
 flags:  -ZS---
 ******************************************/
static void Z0D_ddN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_W(cpustate, addr, COMW(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 cp      @rd,imm16
 flags:  CZSV--
 ******************************************/
static void Z0D_ddN0_0001_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	CPW(cpustate, RDMEM_W(cpustate, addr_from_reg(cpustate, dst)), imm16);
}

/******************************************
 neg     @rd
 flags:  CZSV--
 ******************************************/
static void Z0D_ddN0_0010(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_W(cpustate, addr, NEGW(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 test    @rd
 flags:  -ZS---
 ******************************************/
static void Z0D_ddN0_0100(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	TESTW(cpustate, RDMEM_W(cpustate, addr_from_reg(cpustate, dst)));
}

/******************************************
 ld      @rd,imm16
 flags:  ------
 ******************************************/
static void Z0D_ddN0_0101_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	WRMEM_W(cpustate, addr_from_reg(cpustate, dst), imm16);
}

/******************************************
 tset    @rd
 flags:  --S---
 ******************************************/
static void Z0D_ddN0_0110(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
    if (RDMEM_W(cpustate, addr) & S16) SET_S; else CLR_S;
    WRMEM_W(cpustate, addr, 0xffff);
}

/******************************************
 clr     @rd
 flags:  ------
 ******************************************/
static void Z0D_ddN0_1000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	WRMEM_W(cpustate, addr_from_reg(cpustate, dst), 0);
}

/******************************************
 push    @rd,imm16
 flags:  ------
 ******************************************/
static void Z0D_ddN0_1001_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	PUSHW(cpustate, dst, imm16);
}

/******************************************
 ext0e   imm8
 flags:  ------
 ******************************************/
static void Z0E_imm8(z8000_state *cpustate)
{
    CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: ext0e  $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
	}
}

/******************************************
 ext0f   imm8
 flags:  ------
 ******************************************/
static void Z0F_imm8(z8000_state *cpustate)
{
    CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: ext0f  $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 cpl     rrd,imm32
 flags:  CZSV--
 ******************************************/
static void Z10_0000_dddd_imm32(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	CPL(cpustate, cpustate->RL(dst), imm32);
}

/******************************************
 cpl     rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z10_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 pushl   @rd,@rs
 flags:  ------
 ******************************************/
static void Z11_ddN0_ssN0(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHL(cpustate, dst, RDMEM_L(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 subl    rrd,imm32
 flags:  CZSV--
 ******************************************/
static void Z12_0000_dddd_imm32(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	cpustate->RL(dst) = SUBL(cpustate, cpustate->RL(dst), imm32);
}

/******************************************
 subl    rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z12_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = SUBL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 push    @rd,@rs
 flags:  ------
 ******************************************/
static void Z13_ddN0_ssN0(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(cpustate, dst, RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 ldl     rrd,imm32
 flags:  ------
 ******************************************/
static void Z14_0000_dddd_imm32(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	cpustate->RL(dst) = imm32;
}

/******************************************
 ldl     rrd,@rs
 flags:  ------
 ******************************************/
static void Z14_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = RDMEM_L(cpustate,  addr_from_reg(cpustate, src));
}

/******************************************
 popl    rd,@rs
 flags:  ------
 ******************************************/
static void Z15_ssN0_ddN0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = POPL(cpustate, src);
}

/******************************************
 addl    rrd,imm32
 flags:  CZSV--
 ******************************************/
static void Z16_0000_dddd_imm32(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	cpustate->RL(dst) = ADDL(cpustate, cpustate->RL(dst), imm32);
}

/******************************************
 addl    rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z16_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = ADDL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 pop     @rd,@rs
 flags:  ------
 ******************************************/
static void Z17_ssN0_ddN0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	WRMEM_W(cpustate, addr_from_reg(cpustate, dst), POPW(cpustate, src));
}

/******************************************
 multl   rqd,imm32
 flags:  CZSV--
 ******************************************/
static void Z18_00N0_dddd_imm32(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	cpustate->RQ(dst) = MULTL(cpustate, cpustate->RQ(dst), imm32);
}

/******************************************
 multl   rqd,@rs
 flags:  CZSV--
 ******************************************/
static void Z18_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RQ(dst) = MULTL(cpustate, cpustate->RQ(dst), cpustate->RL(src)); //@@@
}

/******************************************
 mult    rrd,imm16
 flags:  CZSV--
 ******************************************/
static void Z19_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RL(dst) = MULTW(cpustate, cpustate->RL(dst), imm16);
}

/******************************************
 mult    rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z19_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = MULTW(cpustate, cpustate->RL(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 divl    rqd,imm32
 flags:  CZSV--
 ******************************************/
static void Z1A_0000_dddd_imm32(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM32;
	cpustate->RQ(dst) = DIVL(cpustate, cpustate->RQ(dst), imm32);
}

/******************************************
 divl    rqd,@rs
 flags:  CZSV--
 ******************************************/
static void Z1A_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RQ(dst) = DIVL(cpustate, cpustate->RQ(dst), RDMEM_L(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 div     rrd,imm16
 flags:  CZSV--
 ******************************************/
static void Z1B_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RL(dst) = DIVW(cpustate, cpustate->RL(dst), imm16);
}

/******************************************
 div     rrd,@rs
 flags:  CZSV--
 ******************************************/
static void Z1B_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = DIVW(cpustate, cpustate->RL(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
}

/******************************************
 testl   @rd
 flags:  -ZS---
 ******************************************/
static void Z1C_ddN0_1000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	TESTL(cpustate, RDMEM_L(cpustate, addr_from_reg(cpustate, dst)));
}

/******************************************
 ldm     @rd,rs,n
 flags:  ------
 ******************************************/
static void Z1C_ddN0_1001_0000_ssss_0000_nmin1(z8000_state *cpustate)
{
    GET_DST(OP0,NIB2);
    GET_CNT(OP1,NIB3);
    GET_SRC(OP1,NIB1);
	UINT32 addr = addr_from_reg(cpustate, dst);
    while (cnt-- >= 0) {
        WRMEM_W(cpustate, addr, cpustate->RW(src));
		addr = addr_add(cpustate, addr, 2);
		src = (src+1) & 15;
    }
}

/******************************************
 ldm     rd,@rs,n
 flags:  ------
 ******************************************/
static void Z1C_ssN0_0001_0000_dddd_0000_nmin1(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB3);
	GET_DST(OP1,NIB1);
	UINT32 addr = addr_from_reg(cpustate, src);
	while (cnt-- >= 0) {
		cpustate->RW(dst) = RDMEM_W(cpustate, addr);
		addr = addr_add(cpustate, addr, 2);
		dst = (dst+1) & 15;
    }
}

/******************************************
 ldl     @rd,rrs
 flags:  ------
 ******************************************/
static void Z1D_ddN0_ssss(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_L(cpustate,  addr_from_reg(cpustate, dst), cpustate->RL(src));
}

/******************************************
 jp      cc,rd
 flags:  ------
 ******************************************/
static void Z1E_ddN0_cccc(z8000_state *cpustate)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (cc) {
		case  0: if (CC0) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  1: if (CC1) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  2: if (CC2) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  3: if (CC3) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  4: if (CC4) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  5: if (CC5) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  6: if (CC6) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  7: if (CC7) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  8: if (CC8) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case  9: if (CC9) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case 10: if (CCA) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case 11: if (CCB) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case 12: if (CCC) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case 13: if (CCD) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case 14: if (CCE) cpustate->pc = addr_from_reg(cpustate, dst); break;
		case 15: if (CCF) cpustate->pc = addr_from_reg(cpustate, dst); break;
	}
}

/******************************************
 call    @rd
 flags:  ------
 ******************************************/
static void Z1F_ddN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    if (segmented_mode(cpustate))
        PUSHL(cpustate, SP, make_segmented_addr(cpustate->pc));
    else
        PUSHW(cpustate, SP, cpustate->pc);
    cpustate->pc = addr_from_reg(cpustate, dst);
}

/******************************************
 ldb     rbd,@rs
 flags:  ------
 ******************************************/
static void Z20_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
    cpustate->RB(dst) = RDMEM_B(cpustate,  addr_from_reg(cpustate, src));
}

/******************************************
 ld      rd,imm16
 flags:  ------
 ******************************************/
static void Z21_0000_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_IMM16(OP1);
	cpustate->RW(dst) = imm16;
}

/******************************************
 ld      rd,@rs
 flags:  ------
 ******************************************/
static void Z21_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = RDMEM_W(cpustate,  addr_from_reg(cpustate, src));
}

/******************************************
 resb    rbd,rs
 flags:  ------
 ******************************************/
static void Z22_0000_ssss_0000_dddd_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	cpustate->RB(dst) = cpustate->RB(dst) & ~(1 << (cpustate->RW(src) & 7));
}

/******************************************
 resb    @rd,imm4
 flags:  ------
 ******************************************/
static void Z22_ddN0_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_B(cpustate, addr, RDMEM_B(cpustate, addr) & ~bit);
}

/******************************************
 res     rd,rs
 flags:  ------
 ******************************************/
static void Z23_0000_ssss_0000_dddd_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	cpustate->RW(dst) = cpustate->RW(dst) & ~(1 << (cpustate->RW(src) & 15));
}

/******************************************
 res     @rd,imm4
 flags:  ------
 ******************************************/
static void Z23_ddN0_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_W(cpustate, addr, RDMEM_W(cpustate, addr) & ~bit);
}

/******************************************
 setb    rbd,rs
 flags:  ------
 ******************************************/
static void Z24_0000_ssss_0000_dddd_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	cpustate->RB(dst) = cpustate->RB(dst) | (1 << (cpustate->RW(src) & 7));
}

/******************************************
 setb    @rd,imm4
 flags:  ------
 ******************************************/
static void Z24_ddN0_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_B(cpustate, addr, RDMEM_B(cpustate, addr) | bit);
}

/******************************************
 set     rd,rs
 flags:  ------
 ******************************************/
static void Z25_0000_ssss_0000_dddd_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	cpustate->RW(dst) = cpustate->RW(dst) | (1 << (cpustate->RW(src) & 15));
}

/******************************************
 set     @rd,imm4
 flags:  ------
 ******************************************/
static void Z25_ddN0_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_W(cpustate, addr, RDMEM_W(cpustate, addr) | bit);
}

/******************************************
 bitb    rbd,rs
 flags:  -Z----
 ******************************************/
static void Z26_0000_ssss_0000_dddd_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (cpustate->RB(dst) & (1 << (cpustate->RW(src) & 7))) CLR_Z; else SET_Z;
}

/******************************************
 bitb    @rd,imm4
 flags:  -Z----
 ******************************************/
static void Z26_ddN0_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDMEM_B(cpustate, addr_from_reg(cpustate, dst)) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     rd,rs
 flags:  -Z----
 ******************************************/
static void Z27_0000_ssss_0000_dddd_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP1,NIB1);
	if (cpustate->RW(dst) & (1 << (cpustate->RW(src) & 15))) CLR_Z; else SET_Z;
}

/******************************************
 bit     @rd,imm4
 flags:  -Z----
 ******************************************/
static void Z27_ddN0_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (RDMEM_W(cpustate, addr_from_reg(cpustate, dst)) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z28_ddN0_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_B(cpustate,  addr, INCB(cpustate, RDMEM_B(cpustate, addr), i4p1));
}

/******************************************
 inc     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z29_ddN0_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_W(cpustate, addr, INCW(cpustate, RDMEM_W(cpustate, addr), i4p1));
}

/******************************************
 decb    @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z2A_ddN0_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_B(cpustate, addr, DECB(cpustate, RDMEM_B(cpustate, addr), i4p1));
}

/******************************************
 dec     @rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z2B_ddN0_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, dst);
	WRMEM_W(cpustate, addr, DECW(cpustate, RDMEM_W(cpustate, addr), i4p1));
}

/******************************************
 exb     rbd,@rs
 flags:  ------
 ******************************************/
static void Z2C_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, src);
	UINT8 tmp = RDMEM_B(cpustate,  addr);
	WRMEM_B(cpustate, addr, cpustate->RB(dst));
	cpustate->RB(dst) = tmp;
}

/******************************************
 ex      rd,@rs
 flags:  ------
 ******************************************/
static void Z2D_ssN0_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
    UINT32 addr = addr_from_reg(cpustate, src);
	UINT16 tmp = RDMEM_W(cpustate, addr);
	WRMEM_W(cpustate, addr, cpustate->RW(dst));
	cpustate->RW(dst) = tmp;
}

/******************************************
 ldb     @rd,rbs
 flags:  ------
 ******************************************/
static void Z2E_ddN0_ssss(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_B(cpustate,  addr_from_reg(cpustate, dst), cpustate->RB(src));
}

/******************************************
 ld      @rd,rs
 flags:  ------
 ******************************************/
static void Z2F_ddN0_ssss(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	WRMEM_W(cpustate,  addr_from_reg(cpustate, dst), cpustate->RW(src));
}

/******************************************
 ldrb    rbd,dsp16
 flags:  ------
 ******************************************/
static void Z30_0000_dddd_dsp16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	cpustate->RB(dst) = RDMEM_B(cpustate, dsp16);
}

/******************************************
 ldb     rbd,rs(idx16)
 flags:  ------
 ******************************************/
static void Z30_ssN0_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	idx16 = addr_add(cpustate, addr_from_reg(cpustate, src), idx16);
	cpustate->RB(dst) = RDMEM_B(cpustate,  idx16);
}

/******************************************
 ldr     rd,dsp16
 flags:  ------
 ******************************************/
static void Z31_0000_dddd_dsp16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	cpustate->RW(dst) = RDMEM_W(cpustate, dsp16);
}

/******************************************
 ld      rd,rs(idx16)
 flags:  ------
 ******************************************/
static void Z31_ssN0_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	idx16 = addr_add(cpustate, addr_from_reg(cpustate, src), idx16);
	cpustate->RW(dst) = RDMEM_W(cpustate,  idx16);
}

/******************************************
 ldrb    dsp16,rbs
 flags:  ------
 ******************************************/
static void Z32_0000_ssss_dsp16(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_B(cpustate,  dsp16, cpustate->RB(src));
}

/******************************************
 ldb     rd(idx16),rbs
 flags:  ------
 ******************************************/
static void Z32_ddN0_ssss_imm16(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	idx16 = addr_add(cpustate, addr_from_reg(cpustate, dst), idx16);
	WRMEM_B(cpustate,  idx16, cpustate->RB(src));
}

/******************************************
 ldr     dsp16,rs
 flags:  ------
 ******************************************/
static void Z33_0000_ssss_dsp16(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_W(cpustate,  dsp16, cpustate->RW(src));
}

/******************************************
 ld      rd(idx16),rs
 flags:  ------
 ******************************************/
static void Z33_ddN0_ssss_imm16(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	idx16 = addr_add(cpustate, addr_from_reg(cpustate,dst), idx16);
	WRMEM_W(cpustate,  idx16, cpustate->RW(src));
}

/******************************************
 ldar    prd,dsp16
 flags:  ------
 ******************************************/
static void Z34_0000_dddd_dsp16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	addr_to_reg(cpustate, dst, dsp16);
}

/******************************************
 lda     prd,rs(idx16)
 flags:  ------
 ******************************************/
static void Z34_ssN0_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	if (segmented_mode(cpustate)) {
		cpustate->RL(dst) = cpustate->RL(src);
	}
	else {
		cpustate->RW(dst) = cpustate->RW(src);
	}
	add_to_addr_reg(cpustate, dst, idx16);
}

/******************************************
 ldrl    rrd,dsp16
 flags:  ------
 ******************************************/
static void Z35_0000_dddd_dsp16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_DSP16;
	cpustate->RL(dst) = RDMEM_L(cpustate,  dsp16);
}

/******************************************
 ldl     rrd,rs(idx16)
 flags:  ------
 ******************************************/
static void Z35_ssN0_dddd_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX16(OP1);
	idx16 = addr_add(cpustate, addr_from_reg(cpustate, src), idx16);
	cpustate->RL(dst) = RDMEM_L(cpustate,  idx16);
}

/******************************************
 bpt
 flags:  ------
 ******************************************/
static void Z36_0000_0000(z8000_state *cpustate)
{
	/* execute break point trap cpustate->irq_req */
	cpustate->irq_req = Z8000_TRAP;
}

/******************************************
 rsvd36
 flags:  ------
 ******************************************/
static void Z36_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd36 $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldrl    dsp16,rrs
 flags:  ------
 ******************************************/
static void Z37_0000_ssss_dsp16(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DSP16;
	WRMEM_L(cpustate,  dsp16, cpustate->RL(src));
}

/******************************************
 ldl     rd(idx16),rrs
 flags:  ------
 ******************************************/
static void Z37_ddN0_ssss_imm16(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX16(OP1);
	idx16 = addr_add(cpustate, addr_from_reg(cpustate, dst), idx16);
	WRMEM_L(cpustate,  idx16, cpustate->RL(src));
}

/******************************************
 rsvd38
 flags:  ------
 ******************************************/
static void Z38_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd38 $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldps    @rs
 flags:  CZSVDH
 ******************************************/
static void Z39_ssN0_0000(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	UINT16 fcw;
    if (segmented_mode(cpustate)) {
        UINT32 addr = addr_from_reg(cpustate, src);
        fcw = RDMEM_W(cpustate,  addr + 2);
        cpustate->pc = segmented_addr(RDMEM_L(cpustate, addr + 4));
    }
    else {
        fcw = RDMEM_W(cpustate,  cpustate->RW(src));
        cpustate->pc = RDMEM_W(cpustate,  (UINT16)(cpustate->RW(src) + 2));
    }
	CHANGE_FCW(cpustate, fcw); /* check for user/system mode change */
}

/******************************************
 inib(r) @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_0000_0000_aaaa_dddd_x000(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRMEM_B(cpustate, addr_from_reg(cpustate, dst), RDPORT_B(cpustate,  0, cpustate->RW(src)));
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 sinib   @rd,@rs,ra
 sinibr  @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3A_ssss_0001_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@@
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRMEM_B(cpustate,  cpustate->RW(dst), RDPORT_B(cpustate,  1, cpustate->RW(src)));
    cpustate->RW(dst)++;
    cpustate->RW(src)++;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 outib   @rd,@rs,ra
 outibr  @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_0010_0000_aaaa_dddd_x000(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRPORT_B(cpustate,  0, cpustate->RW(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
	add_to_addr_reg(cpustate, src, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 soutib  @rd,@rs,ra
 soutibr @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3A_ssss_0011_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@@
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
    GET_CCC(OP1,NIB3);
    WRPORT_B(cpustate,  1, cpustate->RW(dst), RDMEM_B(cpustate,  cpustate->RW(src)));
    cpustate->RW(dst)++;
    cpustate->RW(src)++;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 inb     rbd,imm16
 flags:  ------
 ******************************************/
static void Z3A_dddd_0100_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
    cpustate->RB(dst) = RDPORT_B(cpustate,  0, imm16);
}

/******************************************
 sinb    rbd,imm16
 flags:  ------
 ******************************************/
static void Z3A_dddd_0101_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
    cpustate->RB(dst) = RDPORT_B(cpustate,  1, imm16);
}

/******************************************
 outb    imm16,rbs
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_0110_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
    WRPORT_B(cpustate,  0, imm16, cpustate->RB(src));
}

/******************************************
 soutb   imm16,rbs
 flags:  ------
 ******************************************/
static void Z3A_ssss_0111_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
    WRPORT_B(cpustate,  1, imm16, cpustate->RB(src));
}

/******************************************
 indb    @rd,@rs,rba
 indbr   @rd,@rs,rba
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_1000_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_B(cpustate,  cpustate->RW(dst), RDPORT_B(cpustate,  0, cpustate->RW(src)));
	cpustate->RW(dst)--;
	cpustate->RW(src)--;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 sindb   @rd,@rs,rba
 sindbr  @rd,@rs,rba
 flags:  ------
 ******************************************/
static void Z3A_ssss_1001_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_B(cpustate,  cpustate->RW(dst), RDPORT_B(cpustate,  1, cpustate->RW(src)));
	cpustate->RW(dst)--;
	cpustate->RW(src)--;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 outdb   @rd,@rs,rba
 outdbr  @rd,@rs,rba
 flags:  ---V--
 ******************************************/
static void Z3A_ssss_1010_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B(cpustate,  0, cpustate->RW(dst), RDMEM_B(cpustate,  cpustate->RW(src)));
	cpustate->RW(dst)--;
	cpustate->RW(src)--;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 soutdb  @rd,@rs,rba
 soutdbr @rd,@rs,rba
 flags:  ------
 ******************************************/
static void Z3A_ssss_1011_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_B(cpustate,  1, cpustate->RW(dst), RDMEM_B(cpustate,  cpustate->RW(src)));
	cpustate->RW(dst)--;
	cpustate->RW(src)--;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 ini     @rd,@rs,ra
 inir    @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_0000_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W(cpustate,  cpustate->RW(dst), RDPORT_W(cpustate,  0, cpustate->RW(src)));
	cpustate->RW(dst) += 2;
	cpustate->RW(src) += 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 sini    @rd,@rs,ra
 sinir   @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_0001_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W(cpustate,  cpustate->RW(dst), RDPORT_W(cpustate,  1, cpustate->RW(src)));
	cpustate->RW(dst) += 2;
	cpustate->RW(src) += 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 outi    @rd,@rs,ra
 outir   @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_0010_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W(cpustate,  0, cpustate->RW(dst), RDMEM_W(cpustate,  cpustate->RW(src)));
	cpustate->RW(dst) += 2;
	cpustate->RW(src) += 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 souti   @rd,@rs,ra
 soutir  @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_0011_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W(cpustate,  1, cpustate->RW(dst), RDMEM_W(cpustate,  cpustate->RW(src)));
	cpustate->RW(dst) += 2;
	cpustate->RW(src) += 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 in      rd,imm16
 flags:  ------
 ******************************************/
static void Z3B_dddd_0100_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
	cpustate->RW(dst) = RDPORT_W(cpustate,  0, imm16);
}

/******************************************
 sin     rd,imm16
 flags:  ------
 ******************************************/
static void Z3B_dddd_0101_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_DST(OP0,NIB2);
    GET_IMM16(OP1);
	cpustate->RW(dst) = RDPORT_W(cpustate,  1, imm16);
}

/******************************************
 out     imm16,rs
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_0110_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
	WRPORT_W(cpustate,  0, imm16, cpustate->RW(src));
}

/******************************************
 sout    imm16,rbs
 flags:  ------
 ******************************************/
static void Z3B_ssss_0111_imm16(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
    GET_SRC(OP0,NIB2);
    GET_IMM16(OP1);
	WRPORT_W(cpustate,  1, imm16, cpustate->RW(src));
}

/******************************************
 ind     @rd,@rs,ra
 indr    @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_1000_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W(cpustate,  cpustate->RW(dst), RDPORT_W(cpustate,  0, cpustate->RW(src)));
	cpustate->RW(dst) -= 2;
	cpustate->RW(src) -= 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 sind    @rd,@rs,ra
 sindr   @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_1001_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W(cpustate,  cpustate->RW(dst), RDPORT_W(cpustate,  1, cpustate->RW(src)));
	cpustate->RW(dst) -= 2;
	cpustate->RW(src) -= 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 outd    @rd,@rs,ra
 outdr   @rd,@rs,ra
 flags:  ---V--
 ******************************************/
static void Z3B_ssss_1010_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W(cpustate,  0, cpustate->RW(dst), RDMEM_W(cpustate,  cpustate->RW(src)));
	cpustate->RW(dst) -= 2;
	cpustate->RW(src) -= 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 soutd   @rd,@rs,ra
 soutdr  @rd,@rs,ra
 flags:  ------
 ******************************************/
static void Z3B_ssss_1011_0000_aaaa_dddd_x000(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRPORT_W(cpustate,  1, cpustate->RW(dst), RDMEM_W(cpustate,  cpustate->RW(src)));
	cpustate->RW(dst) -= 2;
	cpustate->RW(src) -= 2;
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 inb     rbd,@rs
 flags:  ------
 ******************************************/
static void Z3C_ssss_dddd(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	cpustate->RB(dst) = RDPORT_B(cpustate,  0, cpustate->RW(src));
}

/******************************************
 in      rd,@rs
 flags:  ------
 ******************************************/
static void Z3D_ssss_dddd(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_DST(OP0,NIB3);
	cpustate->RW(dst) = RDPORT_W(cpustate,  0, cpustate->RW(src));
}

/******************************************
 outb    @rd,rbs
 flags:  ---V--
 ******************************************/
static void Z3E_dddd_ssss(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_B(cpustate,  0, cpustate->RW(dst), cpustate->RB(src));
}

/******************************************
 out     @rd,rs
 flags:  ---V--
 ******************************************/
static void Z3F_dddd_ssss(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	WRPORT_W(cpustate,  0, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 addb    rbd,addr
 flags:  CZSVDH
 ******************************************/
static void Z40_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RB(dst) = ADDB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 addb    rbd,addr(rs)
 flags:  CZSVDH
 ******************************************/
static void Z40_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RB(dst) = ADDB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 add     rd,addr
 flags:  CZSV--
 ******************************************/
static void Z41_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RW(dst) = ADDW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr)); /* EHC */
}

/******************************************
 add     rd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z41_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RW(dst) = ADDW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));	/* ASG */
}

/******************************************
 subb    rbd,addr
 flags:  CZSVDH
 ******************************************/
static void Z42_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RB(dst) = SUBB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr)); /* EHC */
}

/******************************************
 subb    rbd,addr(rs)
 flags:  CZSVDH
 ******************************************/
static void Z42_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RB(dst) = SUBB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 sub     rd,addr
 flags:  CZSV--
 ******************************************/
static void Z43_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RW(dst) = SUBW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 sub     rd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z43_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RW(dst) = SUBW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 orb     rbd,addr
 flags:  CZSP--
 ******************************************/
static void Z44_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RB(dst) = ORB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 orb     rbd,addr(rs)
 flags:  CZSP--
 ******************************************/
static void Z44_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RB(dst) = ORB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 or      rd,addr
 flags:  CZS---
 ******************************************/
static void Z45_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RW(dst) = ORW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 or      rd,addr(rs)
 flags:  CZS---
 ******************************************/
static void Z45_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RW(dst) = ORW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 andb    rbd,addr
 flags:  -ZSP--
 ******************************************/
static void Z46_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RB(dst) = ANDB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 andb    rbd,addr(rs)
 flags:  -ZSP--
 ******************************************/
static void Z46_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RB(dst) = ANDB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 and     rd,addr
 flags:  -ZS---
 ******************************************/
static void Z47_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RW(dst) = ANDW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 and     rd,addr(rs)
 flags:  -ZS---
 ******************************************/
static void Z47_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RW(dst) = ANDW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 xorb    rbd,addr
 flags:  -ZSP--
 ******************************************/
static void Z48_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RB(dst) = XORB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 xorb    rbd,addr(rs)
 flags:  -ZSP--
 ******************************************/
static void Z48_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RB(dst) = XORB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 xor     rd,addr
 flags:  -ZS---
 ******************************************/
static void Z49_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RW(dst) = XORW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 xor     rd,addr(rs)
 flags:  -ZS---
 ******************************************/
static void Z49_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RW(dst) = XORW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 cpb     rbd,addr
 flags:  CZSV--
 ******************************************/
static void Z4A_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 cpb     rbd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z4A_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr));
}

/******************************************
 cp      rd,addr
 flags:  CZSV--
 ******************************************/
static void Z4B_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 cp      rd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z4B_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 comb    addr
 flags:  -ZSP--
 ******************************************/
static void Z4C_0000_0000_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, COMB(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 cpb     addr,imm8
 flags:  CZSV--
 ******************************************/
static void Z4C_0000_0001_addr_imm8(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	CPB(cpustate, RDMEM_B(cpustate, addr), imm8);
}

/******************************************
 negb    addr
 flags:  CZSV--
 ******************************************/
static void Z4C_0000_0010_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, NEGB(cpustate, RDMEM_B(cpustate, addr)));
}

/******************************************
 testb   addr
 flags:  -ZSP--
 ******************************************/
static void Z4C_0000_0100_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	TESTB(cpustate, RDMEM_B(cpustate, addr));
}

/******************************************
 ldb     addr,imm8
 flags:  ------
 ******************************************/
static void Z4C_0000_0101_addr_imm8(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	WRMEM_B(cpustate,  addr, imm8);
}

/******************************************
 tsetb   addr
 flags:  --S---
 ******************************************/
static void Z4C_0000_0110_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
    if (RDMEM_B(cpustate, addr) & S08) SET_S; else CLR_S;
    WRMEM_B(cpustate, addr, 0xff);
}

/******************************************
 clrb    addr
 flags:  ------
 ******************************************/
static void Z4C_0000_1000_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, 0);
}

/******************************************
 comb    addr(rd)
 flags:  -ZSP--
 ******************************************/
static void Z4C_ddN0_0000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, COMB(cpustate, RDMEM_B(cpustate, addr)));
}

/******************************************
 cpb     addr(rd),imm8
 flags:  CZSV--
 ******************************************/
static void Z4C_ddN0_0001_addr_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	CPB(cpustate, RDMEM_B(cpustate, addr), imm8);
}

/******************************************
 negb    addr(rd)
 flags:  CZSV--
 ******************************************/
static void Z4C_ddN0_0010_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, NEGB(cpustate, RDMEM_B(cpustate, addr)));
}

/******************************************
 testb   addr(rd)
 flags:  -ZSP--
 ******************************************/
static void Z4C_ddN0_0100_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	TESTB(cpustate, RDMEM_B(cpustate, addr));
}

/******************************************
 ldb     addr(rd),imm8
 flags:  ------
 ******************************************/
static void Z4C_ddN0_0101_addr_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM8(OP2);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, imm8);
}

/******************************************
 tsetb   addr(rd)
 flags:  --S---
 ******************************************/
static void Z4C_ddN0_0110_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
    if (RDMEM_B(cpustate, addr) & S08) SET_S; else CLR_S;
    WRMEM_B(cpustate, addr, 0xff);
}

/******************************************
 clrb    addr(rd)
 flags:  ------
 ******************************************/
static void Z4C_ddN0_1000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, 0);
}

/******************************************
 com     addr
 flags:  -ZS---
 ******************************************/
static void Z4D_0000_0000_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, COMW(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 cp      addr,imm16
 flags:  CZSV--
 ******************************************/
static void Z4D_0000_0001_addr_imm16(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	CPW(cpustate, RDMEM_W(cpustate, addr), imm16);
}

/******************************************
 neg     addr
 flags:  CZSV--
 ******************************************/
static void Z4D_0000_0010_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, NEGW(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 test    addr
 flags:  ------
 ******************************************/
static void Z4D_0000_0100_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	TESTW(cpustate, RDMEM_W(cpustate, addr));
}

/******************************************
 ld      addr,imm16
 flags:  ------
 ******************************************/
static void Z4D_0000_0101_addr_imm16(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	WRMEM_W(cpustate,  addr, imm16);
}

/******************************************
 tset    addr
 flags:  --S---
 ******************************************/
static void Z4D_0000_0110_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
    if (RDMEM_W(cpustate, addr) & S16) SET_S; else CLR_S;
    WRMEM_W(cpustate, addr, 0xffff);
}

/******************************************
 clr     addr
 flags:  ------
 ******************************************/
static void Z4D_0000_1000_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, 0);
}

/******************************************
 com     addr(rd)
 flags:  -ZS---
 ******************************************/
static void Z4D_ddN0_0000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, COMW(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 cp      addr(rd),imm16
 flags:  CZSV--
 ******************************************/
static void Z4D_ddN0_0001_addr_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	CPW(cpustate, RDMEM_W(cpustate, addr), imm16);
}

/******************************************
 neg     addr(rd)
 flags:  CZSV--
 ******************************************/
static void Z4D_ddN0_0010_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, NEGW(cpustate, RDMEM_W(cpustate, addr)));
}

/******************************************
 test    addr(rd)
 flags:  ------
 ******************************************/
static void Z4D_ddN0_0100_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	TESTW(cpustate, RDMEM_W(cpustate, addr));
}

/******************************************
 ld      addr(rd),imm16
 flags:  ------
 ******************************************/
static void Z4D_ddN0_0101_addr_imm16(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	GET_IMM16(OP2);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, imm16);
}

/******************************************
 tset    addr(rd)
 flags:  --S---
 ******************************************/
static void Z4D_ddN0_0110_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
    if (RDMEM_W(cpustate, addr) & S16) SET_S; else CLR_S;
    WRMEM_W(cpustate, addr, 0xffff);
}

/******************************************
 clr     addr(rd)
 flags:  ------
 ******************************************/
static void Z4D_ddN0_1000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, 0);
}

/******************************************
 ldb     addr(rd),rbs
 flags:  ------
 ******************************************/
static void Z4E_ddN0_ssN0_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, cpustate->RB(src));
}

/******************************************
 cpl     rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z50_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	CPL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 cpl     rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z50_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	CPL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 pushl   @rd,addr
 flags:  ------
 ******************************************/
static void Z51_ddN0_0000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHL(cpustate, dst, RDMEM_L(cpustate, addr));
}

/******************************************
 pushl   @rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z51_ddN0_ssN0_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	PUSHL(cpustate, dst, RDMEM_L(cpustate, addr));
}

/******************************************
 subl    rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z52_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RL(dst) = SUBL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 subl    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z52_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RL(dst) = SUBL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 push    @rd,addr
 flags:  ------
 ******************************************/
static void Z53_ddN0_0000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	PUSHW(cpustate, dst, RDMEM_W(cpustate, addr));
}

/******************************************
 push    @rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z53_ddN0_ssN0_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	PUSHW(cpustate, dst, RDMEM_W(cpustate, addr));
}

/******************************************
 ldl     rrd,addr
 flags:  ------
 ******************************************/
static void Z54_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RL(dst) = RDMEM_L(cpustate,  addr);
}

/******************************************
 ldl     rrd,addr(rs)
 flags:  ------
 ******************************************/
static void Z54_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RL(dst) = RDMEM_L(cpustate,  addr);
}

/******************************************
 popl    addr,@rs
 flags:  ------
 ******************************************/
static void Z55_ssN0_0000_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_L(cpustate,  addr, POPL(cpustate, src));
}

/******************************************
 popl    addr(rd),@rs
 flags:  ------
 ******************************************/
static void Z55_ssN0_ddN0_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_L(cpustate,  addr, POPL(cpustate, src));
}

/******************************************
 addl    rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z56_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RL(dst) = ADDL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 addl    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z56_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RL(dst) = ADDL(cpustate, cpustate->RL(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 pop     addr,@rs
 flags:  ------
 ******************************************/
static void Z57_ssN0_0000_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, POPW(cpustate, src));
}

/******************************************
 pop     addr(rd),@rs
 flags:  ------
 ******************************************/
static void Z57_ssN0_ddN0_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, POPW(cpustate, src));
}

/******************************************
 multl   rqd,addr
 flags:  CZSV--
 ******************************************/
static void Z58_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RQ(dst) = MULTL(cpustate, cpustate->RQ(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 multl   rqd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z58_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RQ(dst) = MULTL(cpustate, cpustate->RQ(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 mult    rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z59_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RL(dst) = MULTW(cpustate, cpustate->RL(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 mult    rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z59_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RL(dst) = MULTW(cpustate, cpustate->RL(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 divl    rqd,addr
 flags:  CZSV--
 ******************************************/
static void Z5A_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RQ(dst) = DIVL(cpustate, cpustate->RQ(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 divl    rqd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z5A_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RQ(dst) = DIVL(cpustate, cpustate->RQ(dst), RDMEM_L(cpustate, addr));
}

/******************************************
 div     rrd,addr
 flags:  CZSV--
 ******************************************/
static void Z5B_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RL(dst) = DIVW(cpustate, cpustate->RL(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 div     rrd,addr(rs)
 flags:  CZSV--
 ******************************************/
static void Z5B_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RL(dst) = DIVW(cpustate, cpustate->RL(dst), RDMEM_W(cpustate, addr));
}

/******************************************
 ldm     rd,addr,n
 flags:  ------
 ******************************************/
static void Z5C_0000_0001_0000_dddd_0000_nmin1_addr(z8000_state *cpustate)
{
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		cpustate->RW(dst) = RDMEM_W(cpustate, addr);
		dst = (dst+1) & 15;
        addr = addr_add (cpustate, addr, 2);
	}
}

/******************************************
 testl   addr
 flags:  -ZS---
 ******************************************/
static void Z5C_0000_1000_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
	TESTL(cpustate, RDMEM_L(cpustate, addr));
}

/******************************************
 ldm     addr,rs,n
 flags:  ------
 ******************************************/
static void Z5C_0000_1001_0000_ssss_0000_nmin1_addr(z8000_state *cpustate)
{
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	while (cnt-- >= 0) {
		WRMEM_W(cpustate,  addr, cpustate->RW(src));
		src = (src+1) & 15;
        addr = addr_add (cpustate, addr, 2);
	}
}

/******************************************
 testl   addr(rd)
 flags:  -ZS---
 ******************************************/
static void Z5C_ddN0_1000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	TESTL(cpustate, RDMEM_L(cpustate, addr));
}

/******************************************
 ldm     addr(rd),rs,n
 flags:  ------
 ******************************************/
static void Z5C_ddN0_1001_0000_ssN0_0000_nmin1_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	while (cnt-- >= 0) {
		WRMEM_W(cpustate,  addr, cpustate->RW(src));
		src = (src+1) & 15;
		addr = addr_add(cpustate, addr, 2);
	}
}

/******************************************
 ldm     rd,addr(rs),n
 flags:  ------
 ******************************************/
static void Z5C_ssN0_0001_0000_dddd_0000_nmin1_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_DST(OP1,NIB1);
	GET_CNT(OP1,NIB3);
	GET_ADDR(OP2);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	while (cnt-- >= 0) {
		cpustate->RW(dst) = RDMEM_W(cpustate, addr);
		dst = (dst+1) & 15;
		addr = addr_add(cpustate, addr, 2);
	}
}

/******************************************
 ldl     addr,rrs
 flags:  ------
 ******************************************/
static void Z5D_0000_ssss_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_L(cpustate,  addr, cpustate->RL(src));
}

/******************************************
 ldl     addr(rd),rrs
 flags:  ------
 ******************************************/
static void Z5D_ddN0_ssss_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_L(cpustate,  addr, cpustate->RL(src));
}

/******************************************
 jp      cc,addr
 flags:  ------
 ******************************************/
static void Z5E_0000_cccc_addr(z8000_state *cpustate)
{
	GET_CCC(OP0,NIB3);
	GET_ADDR(OP1);
	switch (cc) {
		case  0: if (CC0) cpustate->pc = addr; break;
		case  1: if (CC1) cpustate->pc = addr; break;
		case  2: if (CC2) cpustate->pc = addr; break;
		case  3: if (CC3) cpustate->pc = addr; break;
		case  4: if (CC4) cpustate->pc = addr; break;
		case  5: if (CC5) cpustate->pc = addr; break;
		case  6: if (CC6) cpustate->pc = addr; break;
		case  7: if (CC7) cpustate->pc = addr; break;
		case  8: if (CC8) cpustate->pc = addr; break;
		case  9: if (CC9) cpustate->pc = addr; break;
		case 10: if (CCA) cpustate->pc = addr; break;
		case 11: if (CCB) cpustate->pc = addr; break;
		case 12: if (CCC) cpustate->pc = addr; break;
		case 13: if (CCD) cpustate->pc = addr; break;
		case 14: if (CCE) cpustate->pc = addr; break;
		case 15: if (CCF) cpustate->pc = addr; break;
	}
}

/******************************************
 jp      cc,addr(rd)
 flags:  ------
 ******************************************/
static void Z5E_ddN0_cccc_addr(z8000_state *cpustate)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	switch (cc) {
		case  0: if (CC0) cpustate->pc = addr; break;
		case  1: if (CC1) cpustate->pc = addr; break;
		case  2: if (CC2) cpustate->pc = addr; break;
		case  3: if (CC3) cpustate->pc = addr; break;
		case  4: if (CC4) cpustate->pc = addr; break;
		case  5: if (CC5) cpustate->pc = addr; break;
		case  6: if (CC6) cpustate->pc = addr; break;
		case  7: if (CC7) cpustate->pc = addr; break;
		case  8: if (CC8) cpustate->pc = addr; break;
		case  9: if (CC9) cpustate->pc = addr; break;
		case 10: if (CCA) cpustate->pc = addr; break;
		case 11: if (CCB) cpustate->pc = addr; break;
		case 12: if (CCC) cpustate->pc = addr; break;
		case 13: if (CCD) cpustate->pc = addr; break;
		case 14: if (CCE) cpustate->pc = addr; break;
		case 15: if (CCF) cpustate->pc = addr; break;
	}
}

/******************************************
 call    addr
 flags:  ------
 ******************************************/
static void Z5F_0000_0000_addr(z8000_state *cpustate)
{
	GET_ADDR(OP1);
    if (segmented_mode(cpustate))
        PUSHL(cpustate, SP, make_segmented_addr(cpustate->pc));
    else
        PUSHW(cpustate, SP, cpustate->pc);
	cpustate->pc = addr;
}

/******************************************
 call    addr(rd)
 flags:  ------
 ******************************************/
static void Z5F_ddN0_0000_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
    if (segmented_mode(cpustate))
        PUSHL(cpustate, SP, make_segmented_addr(cpustate->pc));
    else
        PUSHW(cpustate, SP, cpustate->pc);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	cpustate->pc = addr;
}

/******************************************
 ldb     rbd,addr
 flags:  ------
 ******************************************/
static void Z60_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RB(dst) = RDMEM_B(cpustate, addr);
}

/******************************************
 ldb     rbd,addr(rs)
 flags:  ------
 ******************************************/
static void Z60_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RB(dst) = RDMEM_B(cpustate, addr);
}

/******************************************
 ld      rd,addr
 flags:  ------
 ******************************************/
static void Z61_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	cpustate->RW(dst) = RDMEM_W(cpustate, addr);
}

/******************************************
 ld      rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z61_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	cpustate->RW(dst) = RDMEM_W(cpustate, addr);
}

/******************************************
 resb    addr,imm4
 flags:  ------
 ******************************************/
static void Z62_0000_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, RDMEM_B(cpustate, addr) & ~bit);
}

/******************************************
 resb    addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z62_ddN0_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, RDMEM_B(cpustate, addr) & ~bit);
}

/******************************************
 res     addr,imm4
 flags:  ------
 ******************************************/
static void Z63_0000_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, RDMEM_W(cpustate, addr) & ~bit);
}

/******************************************
 res     addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z63_ddN0_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, RDMEM_W(cpustate, addr) & ~bit);
}

/******************************************
 setb    addr,imm4
 flags:  ------
 ******************************************/
static void Z64_0000_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, RDMEM_B(cpustate, addr) | bit);
}

/******************************************
 setb    addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z64_ddN0_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, RDMEM_B(cpustate, addr) | bit);
}

/******************************************
 set     addr,imm4
 flags:  ------
 ******************************************/
static void Z65_0000_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, RDMEM_W(cpustate, addr) | bit);
}

/******************************************
 set     addr(rd),imm4
 flags:  ------
 ******************************************/
static void Z65_ddN0_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, RDMEM_W(cpustate, addr) | bit);
}

/******************************************
 bitb    addr,imm4
 flags:  -Z----
 ******************************************/
static void Z66_0000_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_B(cpustate, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bitb    addr(rd),imm4
 flags:  -Z----
 ******************************************/
static void Z66_ddN0_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
    if (RDMEM_B(cpustate, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr,imm4
 flags:  -Z----
 ******************************************/
static void Z67_0000_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_ADDR(OP1);
	if (RDMEM_W(cpustate, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     addr(rd),imm4
 flags:  -Z----
 ******************************************/
static void Z67_ddN0_imm4_addr(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	if (RDMEM_W(cpustate, addr) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z68_0000_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, INCB(cpustate, RDMEM_B(cpustate, addr), i4p1));
}

/******************************************
 incb    addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z68_ddN0_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, INCB(cpustate, RDMEM_B(cpustate, addr), i4p1));
}

/******************************************
 inc     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z69_0000_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, INCW(cpustate, RDMEM_W(cpustate, addr), i4p1));
}

/******************************************
 inc     addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z69_ddN0_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, INCW(cpustate, RDMEM_W(cpustate, addr), i4p1));
}

/******************************************
 decb    addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6A_0000_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, DECB(cpustate, RDMEM_B(cpustate, addr), i4p1));
}

/******************************************
 decb    addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6A_ddN0_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, DECB(cpustate, RDMEM_B(cpustate, addr), i4p1));
}

/******************************************
 dec     addr,imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6B_0000_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, DECW(cpustate, RDMEM_W(cpustate, addr), i4p1));
}

/******************************************
 dec     addr(rd),imm4m1
 flags:  -ZSV--
 ******************************************/
static void Z6B_ddN0_imm4m1_addr(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, DECW(cpustate, RDMEM_W(cpustate, addr), i4p1));
}

/******************************************
 exb     rbd,addr
 flags:  ------
 ******************************************/
static void Z6C_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	UINT8 tmp = RDMEM_B(cpustate, addr);
	WRMEM_B(cpustate, addr, cpustate->RB(dst));
	cpustate->RB(dst) = tmp;
}

/******************************************
 exb     rbd,addr(rs)
 flags:  ------
 ******************************************/
static void Z6C_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	UINT8 tmp;
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	tmp = RDMEM_B(cpustate, addr);
	WRMEM_B(cpustate, addr, cpustate->RB(dst));
    cpustate->RB(dst) = tmp;
}

/******************************************
 ex      rd,addr
 flags:  ------
 ******************************************/
static void Z6D_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR(OP1);
	UINT16 tmp = RDMEM_W(cpustate, addr);
	WRMEM_W(cpustate,  addr, cpustate->RW(dst));
	cpustate->RW(dst) = tmp;
}

/******************************************
 ex      rd,addr(rs)
 flags:  ------
 ******************************************/
static void Z6D_ssN0_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	UINT16 tmp;
	addr = addr_add(cpustate, addr, cpustate->RW(src));
	tmp = RDMEM_W(cpustate, addr);
	WRMEM_W(cpustate,  addr, cpustate->RW(dst));
    cpustate->RW(dst) = tmp;
}

/******************************************
 ldb     addr,rbs
 flags:  ------
 ******************************************/
static void Z6E_0000_ssss_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_B(cpustate,  addr, cpustate->RB(src));
}

/******************************************
 ldb     addr(rd),rbs
 flags:  ------
 ******************************************/
static void Z6E_ddN0_ssss_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_B(cpustate,  addr, cpustate->RB(src));
}

/******************************************
 ld      addr,rs
 flags:  ------
 ******************************************/
static void Z6F_0000_ssss_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_ADDR(OP1);
	WRMEM_W(cpustate,  addr, cpustate->RW(src));
}

/******************************************
 ld      addr(rd),rs
 flags:  ------
 ******************************************/
static void Z6F_ddN0_ssss_addr(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_ADDR(OP1);
	addr = addr_add(cpustate, addr, cpustate->RW(dst));
	WRMEM_W(cpustate,  addr, cpustate->RW(src));
}

/******************************************
 ldb     rbd,rs(rx)
 flags:  ------
 ******************************************/
static void Z70_ssN0_dddd_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	cpustate->RB(dst) = RDMEM_B(cpustate, addr_add(cpustate, addr_from_reg(cpustate, src), cpustate->RW(idx)));
}

/******************************************
 ld      rd,rs(rx)
 flags:  ------
 ******************************************/
static void Z71_ssN0_dddd_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	cpustate->RW(dst) = RDMEM_W(cpustate, addr_add(cpustate, addr_from_reg(cpustate, src), cpustate->RW(idx)));
}

/******************************************
 ldb     rd(rx),rbs
 flags:  ------
 ******************************************/
static void Z72_ddN0_ssss_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRMEM_B(cpustate,  addr_add(cpustate, addr_from_reg(cpustate, dst), cpustate->RW(idx)), cpustate->RB(src));
}

/******************************************
 ld      rd(rx),rs
 flags:  ------
 ******************************************/
static void Z73_ddN0_ssss_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRMEM_W(cpustate,  addr_add(cpustate, addr_from_reg(cpustate, dst), cpustate->RW(idx)), cpustate->RW(src));
}

/******************************************
 lda     prd,rs(rx)
 flags:  ------
 ******************************************/
static void Z74_ssN0_dddd_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	if (segmented_mode(cpustate)) {
		cpustate->RL(dst) = cpustate->RL(src);
	}
	else {
		cpustate->RW(dst) = cpustate->RW(src);
	}
	add_to_addr_reg(cpustate, dst, cpustate->RW(idx));
}

/******************************************
 ldl     rrd,rs(rx)
 flags:  ------
 ******************************************/
static void Z75_ssN0_dddd_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	cpustate->RL(dst) = RDMEM_L(cpustate,  addr_add(cpustate, addr_from_reg(cpustate, src), cpustate->RW(idx)));
}

/******************************************
 lda     prd,addr
 flags:  ------
 ******************************************/
static void Z76_0000_dddd_addr(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_ADDR_RAW(OP1);
	if (segmented_mode(cpustate)) {
		cpustate->RL(dst) = addr;
	}
	else {
		cpustate->RW(dst) = addr;
	}
}

/******************************************
 lda     prd,addr(rs)
 flags:  ------
 ******************************************/
static void Z76_ssN0_dddd_addr(z8000_state *cpustate)
{//@@@
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	GET_ADDR_RAW(OP1);
	if (segmented_mode(cpustate)) {
		cpustate->RL(dst) = addr;
	}
	else {
		cpustate->RW(dst) = addr;
	}
	add_to_addr_reg(cpustate, dst, cpustate->RW(src));
}

/******************************************
 ldl     rd(rx),rrs
 flags:  ------
 ******************************************/
static void Z77_ddN0_ssss_0000_xxxx_0000_0000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	GET_IDX(OP1,NIB1);
	WRMEM_L(cpustate,  addr_add(cpustate, addr_from_reg(cpustate, dst), cpustate->RW(idx)), cpustate->RL(src));
}

/******************************************
 rsvd78
 flags:  ------
 ******************************************/
static void Z78_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd78 $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldps    addr
 flags:  CZSVDH
 ******************************************/
static void Z79_0000_0000_addr(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_ADDR(OP1);
	UINT16 fcw;
    //printf("LDPS from 0x%x: old pc: 0x%x\n", addr, cpustate->pc);
    if (segmented_mode(cpustate)) {
        fcw = RDMEM_W(cpustate,  addr + 2);
        cpustate->pc = segmented_addr(RDMEM_L(cpustate, addr + 4));
    }
    else {
        fcw = RDMEM_W(cpustate, addr);
        cpustate->pc = RDMEM_W(cpustate, (UINT16)(addr + 2));
    }
	CHANGE_FCW(cpustate, fcw); /* check for user/system mode change */
    //printf("LDPS: new pc: 0x%x\n", cpustate->pc);
}

/******************************************
 ldps    addr(rs)
 flags:  CZSVDH
 ******************************************/
static void Z79_ssN0_0000_addr(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_SRC(OP0,NIB2);
	GET_ADDR(OP1);
	UINT16 fcw;
	addr = addr_add(cpustate, addr, cpustate->RW(src));
    if (segmented_mode(cpustate)) {
        fcw = RDMEM_W(cpustate,  addr + 2);
        cpustate->pc = segmented_addr(RDMEM_L(cpustate, addr + 4));
    }
    else {
        fcw = RDMEM_W(cpustate, addr);
        cpustate->pc	= RDMEM_W(cpustate, (UINT16)(addr + 2));
    }
	CHANGE_FCW(cpustate, fcw); /* check for user/system mode change */
}

/******************************************
 halt
 flags:  ------
 ******************************************/
static void Z7A_0000_0000(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	cpustate->irq_req |= Z8000_HALT;
	if (cpustate->icount > 0) cpustate->icount = 0;
}

/******************************************
 iret
 flags:  CZSVDH
 ******************************************/
static void Z7B_0000_0000(z8000_state *cpustate)
{
	UINT16 tag, fcw;
    CHECK_PRIVILEGED_INSTR();
	tag = POPW(cpustate, SP);	/* get type tag */
	fcw = POPW(cpustate, SP);	/* get cpustate->fcw  */
    if (segmented_mode(cpustate))
        cpustate->pc = segmented_addr(POPL(cpustate, SP));
    else
        cpustate->pc	= POPW(cpustate, SP);	/* get cpustate->pc   */
    cpustate->irq_srv &= ~tag;    /* remove IRQ serviced flag */
	CHANGE_FCW(cpustate, fcw);		 /* check for user/system mode change */
	LOG(("Z8K '%s' IRET tag $%04x, fcw $%04x, pc $%04x\n", cpustate->device->tag(), tag, fcw, cpustate->pc));
}

/******************************************
 mset
 flags:  ------
 ******************************************/
static void Z7B_0000_1000(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	/* set mu-0 line */
}

/******************************************
 mres
 flags:  ------
 ******************************************/
static void Z7B_0000_1001(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	/* reset mu-0 line */
}

/******************************************
 mbit
 flags:  CZS---
 ******************************************/
static void Z7B_0000_1010(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	/* test mu-I line */
}

/******************************************
 mreq    rd
 flags:  -ZS---
 ******************************************/
static void Z7B_dddd_1101(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	/* test mu-I line, invert cascade to mu-0  */
}

/******************************************
 di      i2
 flags:  ------
 ******************************************/
static void Z7C_0000_00ii(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_IMM2(OP0,NIB3);
	UINT16 fcw = cpustate->fcw;
	fcw &= (imm2 << 11) | 0xe7ff;
	CHANGE_FCW(cpustate, fcw);
}

/******************************************
 ei      i2
 flags:  ------
 ******************************************/
static void Z7C_0000_01ii(z8000_state *cpustate)
{
    CHECK_PRIVILEGED_INSTR();
	GET_IMM2(OP0,NIB3);
	UINT16 fcw = cpustate->fcw;
	fcw |= ((~imm2) << 11) & 0x1800;
	CHANGE_FCW(cpustate, fcw);
}

/******************************************
 ldctl   rd,ctrl
 flags:  ------
 ******************************************/
static void Z7D_dddd_0ccc(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_IMM3(OP0,NIB3);
	GET_DST(OP0,NIB2);
	switch (imm3) {
		case 2:
			cpustate->RW(dst) = cpustate->fcw;
			break;
		case 3:
			cpustate->RW(dst) = cpustate->refresh;
			break;
		case 4:
			cpustate->RW(dst) = cpustate->psapseg;
			break;
		case 5:
			cpustate->RW(dst) = cpustate->psapoff;
			break;
		case 6:
			cpustate->RW(dst) = cpustate->nspseg;
			break;
		case 7:
			cpustate->RW(dst) = cpustate->nspoff;
			break;
		default:
			LOG(("Z8K '%s' LDCTL R%d,%d\n", cpustate->device->tag(), dst, imm3));
    }
}

/******************************************
 ldctl   ctrl,rs
 flags:  ------
 ******************************************/
static void Z7D_ssss_1ccc(z8000_state *cpustate)
{//@@@
    CHECK_PRIVILEGED_INSTR();
	GET_IMM3(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	switch (imm3) {
		case 2:
			{
				UINT16 fcw;
				fcw = cpustate->RW(src);
				CHANGE_FCW(cpustate, fcw); /* check for user/system mode change */
			}
            break;
		case 3:
			cpustate->refresh = cpustate->RW(src);
			break;
		case 4:
			cpustate->psapseg = cpustate->RW(src);
			break;
		case 5:
			cpustate->psapoff = cpustate->RW(src);
			break;
		case 6:
			cpustate->nspseg = cpustate->RW(src);
			break;
		case 7:
			cpustate->nspoff = cpustate->RW(src);
			break;
		default:
			LOG(("Z8K '%s' LDCTL %d,R%d\n", cpustate->device->tag(), imm3, src));
    }
}

/******************************************
 rsvd7e
 flags:  ------
 ******************************************/
static void Z7E_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd7e $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 sc      imm8
 flags:  CZSVDH
 ******************************************/
static void Z7F_imm8(z8000_state *cpustate)
{
    GET_IMM8(0);
	/* execute system call via IRQ */
	cpustate->irq_req = Z8000_SYSCALL | imm8;

}

/******************************************
 addb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void Z80_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ADDB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 add     rd,rs
 flags:  CZSV--
 ******************************************/
static void Z81_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ADDW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 subb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void Z82_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = SUBB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 sub     rd,rs
 flags:  CZSV--
 ******************************************/
static void Z83_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = SUBW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 orb     rbd,rbs
 flags:  CZSP--
 ******************************************/
static void Z84_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ORB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 or      rd,rs
 flags:  CZS---
 ******************************************/
static void Z85_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ORW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 andb    rbd,rbs
 flags:  -ZSP--
 ******************************************/
static void Z86_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ANDB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 and     rd,rs
 flags:  -ZS---
 ******************************************/
static void Z87_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ANDW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 xorb    rbd,rbs
 flags:  -ZSP--
 ******************************************/
static void Z88_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = XORB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 xor     rd,rs
 flags:  -ZS---
 ******************************************/
static void Z89_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = XORW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 cpb     rbd,rbs
 flags:  CZSV--
 ******************************************/
static void Z8A_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 cp      rd,rs
 flags:  CZSV--
 ******************************************/
static void Z8B_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	CPW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 comb    rbd
 flags:  -ZSP--
 ******************************************/
static void Z8C_dddd_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) = COMB(cpustate, cpustate->RB(dst));
}

/******************************************
 negb    rbd
 flags:  CZSV--
 ******************************************/
static void Z8C_dddd_0010(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) = NEGB(cpustate, cpustate->RB(dst));
}

/******************************************
 testb   rbd
 flags:  -ZSP--
 ******************************************/
static void Z8C_dddd_0100(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	TESTB(cpustate, cpustate->RB(dst));
}

/******************************************
 tsetb   rbd
 flags:  --S---
 ******************************************/
static void Z8C_dddd_0110(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    if (cpustate->RB(dst) & S08) SET_S; else CLR_S;
    cpustate->RB(dst) = 0xff;
}

/******************************************
 ldctlb rbd,flags
 flags:  CZSVDH
 ******************************************/
static void Z8C_dddd_0001(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) = cpustate->fcw & 0xfc;
}

/******************************************
 clrb    rbd
 flags:  ------
 ******************************************/
static void Z8C_dddd_1000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) = 0;
}

/******************************************
 ldctlb flags,rbd
 flags:  ------
 ******************************************/
static void Z8C_dddd_1001(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    cpustate->fcw &= ~0x00fc;
    cpustate->fcw |= (cpustate->RB(dst) & 0xfc);
}

/******************************************
 nop
 flags:  ------
 ******************************************/
static void Z8D_0000_0111(z8000_state *cpustate)
{
	/* nothing */
}

/******************************************
 com     rd
 flags:  -ZS---
 ******************************************/
static void Z8D_dddd_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) = COMW(cpustate, cpustate->RW(dst));
}

/******************************************
 neg     rd
 flags:  CZSV--
 ******************************************/
static void Z8D_dddd_0010(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) = NEGW(cpustate, cpustate->RW(dst));
}

/******************************************
 test    rd
 flags:  ------
 ******************************************/
static void Z8D_dddd_0100(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	TESTW(cpustate, cpustate->RW(dst));
}

/******************************************
 tset    rd
 flags:  --S---
 ******************************************/
static void Z8D_dddd_0110(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    if (cpustate->RW(dst) & S16) SET_S; else CLR_S;
    cpustate->RW(dst) = 0xffff;
}

/******************************************
 clr     rd
 flags:  ------
 ******************************************/
static void Z8D_dddd_1000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) = 0;
}

/******************************************
 setflg  imm4
 flags:  CZSV--
 ******************************************/
static void Z8D_imm4_0001(z8000_state *cpustate)
{
	cpustate->fcw |= cpustate->op[0] & 0x00f0;
}

/******************************************
 resflg  imm4
 flags:  CZSV--
 ******************************************/
static void Z8D_imm4_0011(z8000_state *cpustate)
{
	cpustate->fcw &= ~(cpustate->op[0] & 0x00f0);
}

/******************************************
 comflg  flags
 flags:  CZSP--
 ******************************************/
static void Z8D_imm4_0101(z8000_state *cpustate)
{
	cpustate->fcw ^= (cpustate->op[0] & 0x00f0);
}

/******************************************
 ext8e   imm8
 flags:  ------
 ******************************************/
static void Z8E_imm8(z8000_state *cpustate)
{
    CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: ext8e  $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ext8f   imm8
 flags:  ------
 ******************************************/
static void Z8F_imm8(z8000_state *cpustate)
{
    CHECK_EXT_INSTR();
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: ext8f  $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 cpl     rrd,rrs
 flags:  CZSV--
 ******************************************/
static void Z90_ssss_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    CPL(cpustate, cpustate->RL(dst), cpustate->RL(src));
}

/******************************************
 pushl   @rd,rrs
 flags:  ------
 ******************************************/
static void Z91_ddN0_ssss(z8000_state *cpustate)
{
    GET_SRC(OP0,NIB3);
    GET_DST(OP0,NIB2);
    PUSHL(cpustate, dst, cpustate->RL(src));
}

/******************************************
 subl    rrd,rrs
 flags:  CZSV--
 ******************************************/
static void Z92_ssss_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    cpustate->RL(dst) = SUBL(cpustate, cpustate->RL(dst), cpustate->RL(src));
}

/******************************************
 push    @rd,rs
 flags:  ------
 ******************************************/
static void Z93_ddN0_ssss(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	PUSHW(cpustate, dst, cpustate->RW(src));
}

/******************************************
 ldl     rrd,rrs
 flags:  ------
 ******************************************/
static void Z94_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = cpustate->RL(src);
}

/******************************************
 popl    rrd,@rs
 flags:  ------
 ******************************************/
static void Z95_ssN0_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    cpustate->RL(dst) = POPL(cpustate, src);
}

/******************************************
 addl    rrd,rrs
 flags:  CZSV--
 ******************************************/
static void Z96_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = ADDL(cpustate, cpustate->RL(dst), cpustate->RL(src));
}

/******************************************
 pop     rd,@rs
 flags:  ------
 ******************************************/
static void Z97_ssN0_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    cpustate->RW(dst) = POPW(cpustate, src);
}

/******************************************
 multl   rqd,rrs
 flags:  CZSV--
 ******************************************/
static void Z98_ssss_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
	cpustate->RQ(dst) = MULTL(cpustate, cpustate->RQ(dst), cpustate->RL(src));
}

/******************************************
 mult    rrd,rs
 flags:  CZSV--
 ******************************************/
static void Z99_ssss_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
	cpustate->RL(dst) = MULTW(cpustate, cpustate->RL(dst), cpustate->RW(src));
}

/******************************************
 divl    rqd,rrs
 flags:  CZSV--
 ******************************************/
static void Z9A_ssss_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    cpustate->RQ(dst) = DIVL(cpustate, cpustate->RQ(dst), cpustate->RL(src));
}

/******************************************
 div     rrd,rs
 flags:  CZSV--
 ******************************************/
static void Z9B_ssss_dddd(z8000_state *cpustate)
{
    GET_DST(OP0,NIB3);
    GET_SRC(OP0,NIB2);
    cpustate->RL(dst) = DIVW(cpustate, cpustate->RL(dst), cpustate->RW(src));
}

/******************************************
 testl   rrd
 flags:  -ZS---
 ******************************************/
static void Z9C_dddd_1000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	CLR_ZS;
	if (!cpustate->RL(dst)) SET_Z;
    else if (cpustate->RL(dst) & S32) SET_S;
}

/******************************************
 rsvd9d
 flags:  ------
 ******************************************/
static void Z9D_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd9d $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ret     cc
 flags:  ------
 ******************************************/
static void Z9E_0000_cccc(z8000_state *cpustate)
{
	GET_CCC(OP0,NIB3);
    if (segmented_mode(cpustate))
        switch (cc) {
            case  0: if (CC0) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  1: if (CC1) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  2: if (CC2) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  3: if (CC3) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  4: if (CC4) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  5: if (CC5) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  6: if (CC6) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  7: if (CC7) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  8: if (CC8) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case  9: if (CC9) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case 10: if (CCA) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case 11: if (CCB) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case 12: if (CCC) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case 13: if (CCD) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case 14: if (CCE) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
            case 15: if (CCF) cpustate->pc = segmented_addr(POPL(cpustate, SP)); break;
        }
    else
        switch (cc) {
		    case  0: if (CC0) cpustate->pc = POPW(cpustate, SP); break;
		    case  1: if (CC1) cpustate->pc = POPW(cpustate, SP); break;
		    case  2: if (CC2) cpustate->pc = POPW(cpustate, SP); break;
		    case  3: if (CC3) cpustate->pc = POPW(cpustate, SP); break;
		    case  4: if (CC4) cpustate->pc = POPW(cpustate, SP); break;
		    case  5: if (CC5) cpustate->pc = POPW(cpustate, SP); break;
		    case  6: if (CC6) cpustate->pc = POPW(cpustate, SP); break;
		    case  7: if (CC7) cpustate->pc = POPW(cpustate, SP); break;
		    case  8: if (CC8) cpustate->pc = POPW(cpustate, SP); break;
		    case  9: if (CC9) cpustate->pc = POPW(cpustate, SP); break;
		    case 10: if (CCA) cpustate->pc = POPW(cpustate, SP); break;
		    case 11: if (CCB) cpustate->pc = POPW(cpustate, SP); break;
		    case 12: if (CCC) cpustate->pc = POPW(cpustate, SP); break;
		    case 13: if (CCD) cpustate->pc = POPW(cpustate, SP); break;
		    case 14: if (CCE) cpustate->pc = POPW(cpustate, SP); break;
		    case 15: if (CCF) cpustate->pc = POPW(cpustate, SP); break;
        }
}

/******************************************
 rsvd9f
 flags:  ------
 ******************************************/
static void Z9F_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvd9f $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
}

/******************************************
 ldb     rbd,rbs
 flags:  ------
 ******************************************/
static void ZA0_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = cpustate->RB(src);
}

/******************************************
 ld      rd,rs
 flags:  ------
 ******************************************/
static void ZA1_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = cpustate->RW(src);
}

/******************************************
 resb    rbd,imm4
 flags:  ------
 ******************************************/
static void ZA2_dddd_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) &= ~bit;
}

/******************************************
 res     rd,imm4
 flags:  ------
 ******************************************/
static void ZA3_dddd_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) &= ~bit;
}

/******************************************
 setb    rbd,imm4
 flags:  ------
 ******************************************/
static void ZA4_dddd_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) |= bit;
}

/******************************************
 set     rd,imm4
 flags:  ------
 ******************************************/
static void ZA5_dddd_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) |= bit;
}

/******************************************
 bitb    rbd,imm4
 flags:  -Z----
 ******************************************/
static void ZA6_dddd_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (cpustate->RB(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 bit     rd,imm4
 flags:  -Z----
 ******************************************/
static void ZA7_dddd_imm4(z8000_state *cpustate)
{
	GET_BIT(OP0);
	GET_DST(OP0,NIB2);
	if (cpustate->RW(dst) & bit) CLR_Z; else SET_Z;
}

/******************************************
 incb    rbd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZA8_dddd_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) = INCB(cpustate, cpustate->RB(dst), i4p1);
}

/******************************************
 inc     rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZA9_dddd_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) = INCW(cpustate, cpustate->RW(dst), i4p1);
}

/******************************************
 decb    rbd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZAA_dddd_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	cpustate->RB(dst) = DECB(cpustate, cpustate->RB(dst), i4p1);
}

/******************************************
 dec     rd,imm4m1
 flags:  -ZSV--
 ******************************************/
static void ZAB_dddd_imm4m1(z8000_state *cpustate)
{
	GET_I4M1(OP0,NIB3);
	GET_DST(OP0,NIB2);
	cpustate->RW(dst) = DECW(cpustate, cpustate->RW(dst), i4p1);
}

/******************************************
 exb     rbd,rbs
 flags:  ------
 ******************************************/
static void ZAC_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	UINT8 tmp = cpustate->RB(src);
	cpustate->RB(src) = cpustate->RB(dst);
	cpustate->RB(dst) = tmp;
}

/******************************************
 ex      rd,rs
 flags:  ------
 ******************************************/
static void ZAD_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	UINT16 tmp = cpustate->RW(src);
	cpustate->RW(src) = cpustate->RW(dst);
	cpustate->RW(dst) = tmp;
}

/******************************************
 tccb    cc,rbd
 flags:  ------
 ******************************************/
static void ZAE_dddd_cccc(z8000_state *cpustate)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	UINT8 tmp = cpustate->RB(dst) & ~1;
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
	cpustate->RB(dst) = tmp;
}

/******************************************
 tcc     cc,rd
 flags:  ------
 ******************************************/
static void ZAF_dddd_cccc(z8000_state *cpustate)
{
	GET_CCC(OP0,NIB3);
	GET_DST(OP0,NIB2);
	UINT16 tmp = cpustate->RW(dst) & ~1;
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
	cpustate->RW(dst) = tmp;
}

/******************************************
 dab     rbd
 flags:  CZS---
 ******************************************/
static void ZB0_dddd_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	UINT8 result;
	UINT16 idx = cpustate->RB(dst);
	if (cpustate->fcw & F_C)	idx |= 0x100;
	if (cpustate->fcw & F_H)	idx |= 0x200;
	if (cpustate->fcw & F_DA) idx |= 0x400;
	result = Z8000_dab[idx];
	CLR_CZS;
	CHK_XXXB_ZS;
	if (Z8000_dab[idx] & 0x100) SET_C;
	cpustate->RB(dst) = result;
}

/******************************************
 extsb   rd
 flags:  ------
 ******************************************/
static void ZB1_dddd_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    cpustate->RW(dst) = (cpustate->RW(dst) & 0xff) | ((cpustate->RW(dst) & S08) ? 0xff00 : 0x0000);
}

/******************************************
 extsl   rqd
 flags:  ------
 ******************************************/
static void ZB1_dddd_0111(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	cpustate->RQ(dst) = CONCAT_64((cpustate->RQ(dst) & S32) ?
		0xfffffffful : 0, EXTRACT_64LO(cpustate->RQ(dst)));
}

/******************************************
 exts    rrd
 flags:  ------
 ******************************************/
static void ZB1_dddd_1010(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
    cpustate->RL(dst) = (cpustate->RL(dst) & 0xffff) | ((cpustate->RL(dst) & S16) ?
		0xffff0000ul : 0x00000000ul);
}

/******************************************
 sllb    rbd,imm8
 flags:  CZS---
 srlb    rbd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_0001_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	if (imm8 & S08)
		cpustate->RB(dst) = SRLB(cpustate, cpustate->RB(dst), -(INT8)imm8);
	else
		cpustate->RB(dst) = SLLB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 sdlb    rbd,rs
 flags:  CZS---
 ******************************************/
static void ZB2_dddd_0011_0000_ssss_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	cpustate->RB(dst) = SRLB(cpustate, cpustate->RB(dst), (INT8)cpustate->RW(src));
}

/******************************************
 rlb     rbd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_00I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RB(dst) = RLB(cpustate, cpustate->RB(dst), imm1);
}

/******************************************
 rrb     rbd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_01I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RB(dst) = RRB(cpustate, cpustate->RB(dst), imm1);
}

/******************************************
 slab    rbd,imm8
 flags:  CZSV--
 srab    rbd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_1001_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM8(OP1);
	if (imm8 & S08)
		cpustate->RB(dst) = SRAB(cpustate, cpustate->RB(dst), -(INT8)imm8);
	else
		cpustate->RB(dst) = SLAB(cpustate, cpustate->RB(dst), imm8);
}

/******************************************
 sdab    rbd,rs
 flags:  CZSV--
 ******************************************/
static void ZB2_dddd_1011_0000_ssss_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	cpustate->RB(dst) = SDAB(cpustate, cpustate->RB(dst), (INT8) cpustate->RW(src));
}

/******************************************
 rlcb    rbd,imm1or2
 flags:  -Z----
 ******************************************/
static void ZB2_dddd_10I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RB(dst) = RLCB(cpustate, cpustate->RB(dst), imm1);
}

/******************************************
 rrcb    rbd,imm1or2
 flags:  -Z----
 ******************************************/
static void ZB2_dddd_11I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RB(dst) = RRCB(cpustate, cpustate->RB(dst), imm1);
}

/******************************************
 sll     rd,imm8
 flags:  CZS---
 srl     rd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_0001_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		cpustate->RW(dst) = SRLW(cpustate, cpustate->RW(dst), -(INT16)imm16);
	else
        cpustate->RW(dst) = SLLW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 sdl     rd,rs
 flags:  CZS---
 ******************************************/
static void ZB3_dddd_0011_0000_ssss_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	cpustate->RW(dst) = SDLW(cpustate, cpustate->RW(dst), (INT8)cpustate->RW(src));
}

/******************************************
 rl      rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_00I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RW(dst) = RLW(cpustate, cpustate->RW(dst), imm1);
}

/******************************************
 slll    rrd,imm8
 flags:  CZS---
 srll    rrd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_0101_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		cpustate->RL(dst) = SRLL(cpustate, cpustate->RL(dst), -(INT16)imm16);
	else
		cpustate->RL(dst) = SLLL(cpustate, cpustate->RL(dst), imm16);
}

/******************************************
 sdll    rrd,rs
 flags:  CZS---
 ******************************************/
static void ZB3_dddd_0111_0000_ssss_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	cpustate->RL(dst) = SDLL(cpustate, cpustate->RL(dst), cpustate->RW(src) & 0xff);
}

/******************************************
 rr      rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_01I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RW(dst) = RRW(cpustate, cpustate->RW(dst), imm1);
}

/******************************************
 sla     rd,imm8
 flags:  CZSV--
 sra     rd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1001_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		cpustate->RW(dst) = SRAW(cpustate, cpustate->RW(dst), -(INT16)imm16);
	else
        cpustate->RW(dst) = SLAW(cpustate, cpustate->RW(dst), imm16);
}

/******************************************
 sda     rd,rs
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1011_0000_ssss_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	cpustate->RW(dst) = SDAW(cpustate, cpustate->RW(dst), (INT8)cpustate->RW(src));
}

/******************************************
 rlc     rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_10I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RW(dst) = RLCW(cpustate, cpustate->RW(dst), imm1);
}

/******************************************
 slal    rrd,imm8
 flags:  CZSV--
 sral    rrd,imm8
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1101_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM16(OP1);
	if (imm16 & S16)
		cpustate->RL(dst) = SRAL(cpustate, cpustate->RL(dst), -(INT16)imm16);
	else
		cpustate->RL(dst) = SLAL(cpustate, cpustate->RL(dst), imm16);
}

/******************************************
 sdal    rrd,rs
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_1111_0000_ssss_0000_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB1);
	cpustate->RL(dst) = SDAL(cpustate, cpustate->RL(dst), cpustate->RW(src) & 0xff);
}

/******************************************
 rrc     rd,imm1or2
 flags:  CZSV--
 ******************************************/
static void ZB3_dddd_11I0(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM1(OP0,NIB3);
	cpustate->RW(dst) = RRCW(cpustate, cpustate->RW(dst), imm1);
}

/******************************************
 adcb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void ZB4_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = ADCB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 adc     rd,rs
 flags:  CZSV--
 ******************************************/
static void ZB5_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = ADCW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 sbcb    rbd,rbs
 flags:  CZSVDH
 ******************************************/
static void ZB6_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RB(dst) = SBCB(cpustate, cpustate->RB(dst), cpustate->RB(src));
}

/******************************************
 sbc     rd,rs
 flags:  CZSV--
 ******************************************/
static void ZB7_ssss_dddd(z8000_state *cpustate)
{
	GET_DST(OP0,NIB3);
	GET_SRC(OP0,NIB2);
	cpustate->RW(dst) = SBCW(cpustate, cpustate->RW(dst), cpustate->RW(src));
}

/******************************************
 trtib   @rd,@rs,rr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0010_0000_rrrr_ssN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	cpustate->RB(1) = xlt;	/* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trtirb  @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0110_0000_rrrr_ssN0_1110(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	cpustate->RB(1) = xlt;	/* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) {
	  CLR_V;
	  if (!xlt)
	    cpustate->pc -= 4;
	}
	else SET_V;
}

/******************************************
 trtdb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1010_0000_rrrr_ssN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	cpustate->RB(1) = xlt;	/* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trtdrb  @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1110_0000_rrrr_ssN0_1110(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	cpustate->RB(1) = xlt;	/* load RH1 */
	if (xlt) CLR_Z; else SET_Z;
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) {
	  CLR_V;
	  if (!xlt)
	    cpustate->pc -= 4;
	}
	else SET_V;
}

/******************************************
 trib    @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0000_0000_rrrr_ssN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	WRMEM_B(cpustate, addr_from_reg(cpustate, dst), xlt);
	cpustate->RB(1) = xlt;	/* destroy RH1 */
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trirb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_0100_0000_rrrr_ssN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	WRMEM_B(cpustate, addr_from_reg(cpustate, dst), xlt);
	cpustate->RB(1) = xlt;	/* destroy RH1 */
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; cpustate->pc -= 4; } else SET_V;
}

/******************************************
 trdb    @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1000_0000_rrrr_ssN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	WRMEM_B(cpustate, addr_from_reg(cpustate, dst), xlt);
	cpustate->RB(1) = xlt;	/* destroy RH1 */
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 trdrb   @rd,@rs,rbr
 flags:  -ZSV--
 ******************************************/
static void ZB8_ddN0_1100_0000_rrrr_ssN0_0000(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_SRC(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	UINT8 xlt = RDMEM_B(cpustate, addr_from_reg(cpustate, src) + RDMEM_B(cpustate, addr_from_reg(cpustate, dst)));
	WRMEM_B(cpustate, addr_from_reg(cpustate, dst), xlt);
	cpustate->RB(1) = xlt;	/* destroy RH1 */
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; cpustate->pc -= 4; } else SET_V;
}

/******************************************
 rsvdb9
 flags:  ------
 ******************************************/
static void ZB9_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvdb9 $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
	(void)imm8;
}

/******************************************
 cpib    rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0000_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldib    @rd,@rs,rr
 ldibr   @rd,@rs,rr
 flags:  ---V--
 ******************************************/
static void ZBA_ssN0_0001_0000_rrrr_ddN0_x000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);	/* repeat? */
    WRMEM_B(cpustate,  addr_from_reg(cpustate, dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
	add_to_addr_reg(cpustate, src, 1);
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsib   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0010_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(cpustate, RDMEM_B(cpustate, addr_from_reg(cpustate, dst)), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 1);
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpirb   rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0100_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsirb  @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_0110_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(cpustate, RDMEM_B(cpustate, addr_from_reg(cpustate, dst)), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 1);
	add_to_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpdb    rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1000_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
    CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 lddb    @rs,@rd,rr
 lddbr   @rs,@rd,rr
 flags:  ---V--
 ******************************************/
static void ZBA_ssN0_1001_0000_rrrr_ddN0_x000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_B(cpustate,  addr_from_reg(cpustate, dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
	sub_from_addr_reg(cpustate, src, 1);
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsdb   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1010_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
    CPB(cpustate, RDMEM_B(cpustate, addr_from_reg(cpustate, dst)), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 1);
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpdrb   rbd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1100_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPB(cpustate, cpustate->RB(dst), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsdrb  @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBA_ssN0_1110_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
    CPB(cpustate, RDMEM_B(cpustate, addr_from_reg(cpustate, dst)), RDMEM_B(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 1);
	sub_from_addr_reg(cpustate, dst, 1);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpi     rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0000_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 2);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldi     @rd,@rs,rr
 ldir    @rd,@rs,rr
 flags:  ---V--
 ******************************************/
static void ZBB_ssN0_0001_0000_rrrr_ddN0_x000(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
	GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
	WRMEM_W(cpustate,  addr_from_reg(cpustate, dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
	add_to_addr_reg(cpustate, src, 2);
	add_to_addr_reg(cpustate, dst, 2);
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsi    @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0010_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, RDMEM_W(cpustate, addr_from_reg(cpustate, dst)), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 2);
	add_to_addr_reg(cpustate, dst, 2);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpir    rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0100_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 2);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsir   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_0110_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, RDMEM_W(cpustate, addr_from_reg(cpustate, dst)), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	add_to_addr_reg(cpustate, src, 2);
	add_to_addr_reg(cpustate, dst, 2);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpd     rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1000_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 2);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 ldd     @rs,@rd,rr
 lddr    @rs,@rd,rr
 flags:  ---V--
 ******************************************/
static void ZBB_ssN0_1001_0000_rrrr_ddN0_x000(z8000_state *cpustate)
{
    GET_SRC(OP0,NIB2);
    GET_CNT(OP1,NIB1);
    GET_DST(OP1,NIB2);
	GET_CCC(OP1,NIB3);
    WRMEM_W(cpustate,  addr_from_reg(cpustate, dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
	sub_from_addr_reg(cpustate, src, 2);
	sub_from_addr_reg(cpustate, dst, 2);
	if (--cpustate->RW(cnt)) { CLR_V; if (cc == 0) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsd    @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1010_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, RDMEM_W(cpustate, addr_from_reg(cpustate, dst)), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 2);
	sub_from_addr_reg(cpustate, dst, 2);
	if (--cpustate->RW(cnt)) CLR_V; else SET_V;
}

/******************************************
 cpdr    rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1100_0000_rrrr_dddd_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, cpustate->RW(dst), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 2);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 cpsdr   @rd,@rs,rr,cc
 flags:  CZSV--
 ******************************************/
static void ZBB_ssN0_1110_0000_rrrr_ddN0_cccc(z8000_state *cpustate)
{
	GET_SRC(OP0,NIB2);
	GET_CCC(OP1,NIB3);
	GET_DST(OP1,NIB2);
	GET_CNT(OP1,NIB1);
	CPW(cpustate, RDMEM_W(cpustate, addr_from_reg(cpustate, dst)), RDMEM_W(cpustate, addr_from_reg(cpustate, src)));
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
	sub_from_addr_reg(cpustate, src, 2);
	sub_from_addr_reg(cpustate, dst, 2);
	if (--cpustate->RW(cnt)) { CLR_V; if (!(cpustate->fcw & F_Z)) cpustate->pc -= 4; } else SET_V;
}

/******************************************
 rrdb    rbb,rba
 flags:  -Z----
 ******************************************/
static void ZBC_aaaa_bbbb(z8000_state *cpustate)
{
	UINT8 b = cpustate->op[0] & 15;
	UINT8 a = (cpustate->op[0] >> 4) & 15;
	UINT8 tmp = cpustate->RB(b);
	cpustate->RB(a) = (cpustate->RB(a) >> 4) | (cpustate->RB(b) << 4);
	cpustate->RB(b) = (cpustate->RB(b) & 0xf0) | (tmp & 0x0f);
    if (cpustate->RB(b)) CLR_Z; else SET_Z;
}

/******************************************
 ldk     rd,imm4
 flags:  ------
 ******************************************/
static void ZBD_dddd_imm4(z8000_state *cpustate)
{
	GET_DST(OP0,NIB2);
	GET_IMM4(OP0,NIB3);
	cpustate->RW(dst) = imm4;
}

/******************************************
 rldb    rbb,rba
 flags:  -Z----
 ******************************************/
static void ZBE_aaaa_bbbb(z8000_state *cpustate)
{
	UINT8 b = cpustate->op[0] & 15;
	UINT8 a = (cpustate->op[0] >> 4) & 15;
	UINT8 tmp = cpustate->RB(a);
	cpustate->RB(a) = (cpustate->RB(a) << 4) | (cpustate->RB(b) & 0x0f);
	cpustate->RB(b) = (cpustate->RB(b) & 0xf0) | (tmp >> 4);
	if (cpustate->RB(b)) CLR_Z; else SET_Z;
}

/******************************************
 rsvdbf
 flags:  ------
 ******************************************/
static void ZBF_imm8(z8000_state *cpustate)
{
	GET_IMM8(0);
	LOG(("Z8K '%s' %04x: rsvdbf $%02x\n", cpustate->device->tag(), cpustate->pc, imm8));
    if (cpustate->fcw & F_EPU) {
		/* Z8001 EPU code goes here */
		(void)imm8;
    }
	(void)imm8;
}

/******************************************
 ldb     rbd,imm8
 flags:  ------
 ******************************************/
static void ZC_dddd_imm8(z8000_state *cpustate)
{
	GET_DST(OP0,NIB1);
	GET_IMM8(0);
	cpustate->RB(dst) = imm8;
}

/******************************************
 calr    dsp12
 flags:  ------
 ******************************************/
static void ZD_dsp12(z8000_state *cpustate)
{
	INT16 dsp12 = cpustate->op[0] & 0xfff;
    if (segmented_mode(cpustate))
        PUSHL(cpustate, SP, make_segmented_addr(cpustate->pc));
    else
        PUSHW(cpustate, SP, cpustate->pc);
	dsp12 = (dsp12 & 2048) ? 4096 - 2 * (dsp12 & 2047) : -2 * (dsp12 & 2047);
	cpustate->pc = addr_add(cpustate, cpustate->pc, dsp12);
}

/******************************************
 jr      cc,dsp8
 flags:  ------
 ******************************************/
static void ZE_cccc_dsp8(z8000_state *cpustate)
{
	GET_DSP8;
	GET_CCC(OP0,NIB1);
	switch (cc) {
		case  0: if (CC0) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  1: if (CC1) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  2: if (CC2) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  3: if (CC3) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  4: if (CC4) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  5: if (CC5) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  6: if (CC6) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  7: if (CC7) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  8: if (CC8) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  9: if (CC9) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  10: if (CCA) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  11: if (CCB) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  12: if (CCC) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  13: if (CCD) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  14: if (CCE) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
		case  15: if (CCF) cpustate->pc = addr_add(cpustate, cpustate->pc, dsp8 * 2); break;
	}
}

/******************************************
 dbjnz   rbd,dsp7
 flags:  ------
 ******************************************/
static void ZF_dddd_0dsp7(z8000_state *cpustate)
{
	GET_DST(OP0,NIB1);
    GET_DSP7;
    cpustate->RB(dst) -= 1;
    if (cpustate->RB(dst)) {
        cpustate->pc = addr_sub(cpustate, cpustate->pc, 2 * dsp7);
    }
}

/******************************************
 djnz    rd,dsp7
 flags:  ------
 ******************************************/
static void ZF_dddd_1dsp7(z8000_state *cpustate)
{
	GET_DST(OP0,NIB1);
	GET_DSP7;
	cpustate->RW(dst) -= 1;
	if (cpustate->RW(dst)) {
        cpustate->pc = addr_sub(cpustate, cpustate->pc, 2 * dsp7);
	}
}
