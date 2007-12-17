/************************************************************************

    ST register functions

************************************************************************/

/*
    remember that the OP ST bit is maintained in lastparity
*/

/*
    setstat sets the ST_OP bit according to lastparity

    It must be called before reading the ST register.
*/

static void setstat(void)
{
	int i;
	UINT8 a;

	I.STATUS &= ~ ST_OP;

	/* We set the parity bit. */
	a = lastparity;

	for (i=0; i<8; i++)     /* 8 bits to test */
	{
		if (a & 1)  /* If current bit is set */
			I.STATUS ^= ST_OP;  /* we toggle the ST_OP bit */

		a >>= 1;    /* Next bit. */
	}
}

/*
    getstat sets emulator's lastparity variable according to 9900's STATUS bits.
    It must be called on interrupt return, or when, for some reason,
    the emulated program sets the STATUS register directly.
*/
static void getstat(void)
{
#if (USE_ST_MASK)
	I.STATUS &= ST_MASK;  /* unused bits are forced to 0 */
#endif

	if (I.STATUS & ST_OP)
		lastparity = 1;
	else
		lastparity = 0;

#if HAS_MAPPING
	I.cur_map = (I.STATUS & ST_MF) ? 1 : 0;
#endif
}

/*
    A few words about the following functions.

    A big portability issue is the behavior of the ">>" instruction with the sign bit, which has
    not been normalised.  Every compiler does whatever it thinks smartest.
    My code assumed that when shifting right signed numbers, the operand is left-filled with a
    copy of sign bit, and that when shifting unsigned variables, it is left-filled with 0s.
    This is probably the most logical behaviour, and it is the behavior of CW PRO3 - most time
    (the exception is that ">>=" instructions always copy the sign bit (!)).  But some compilers
    are bound to disagree.

    So, I had to create special functions with predefined tables included, so that this code work
    on every compiler.  BUT this is a real slow-down.
    So, you might have to include a few lines in assembly to make this work better.
    Sorry about this, this problem is really unpleasant and absurd, but it is not my fault.
*/


static const UINT16 right_shift_mask_table[17] =
{
	0xFFFF,
	0x7FFF,
	0x3FFF,
	0x1FFF,
	0x0FFF,
	0x07FF,
	0x03FF,
	0x01FF,
	0x00FF,
	0x007F,
	0x003F,
	0x001F,
	0x000F,
	0x0007,
	0x0003,
	0x0001,
	0x0000
};

static const UINT16 inverted_right_shift_mask_table[17] =
{
	0x0000,
	0x8000,
	0xC000,
	0xE000,
	0xF000,
	0xF800,
	0xFC00,
	0xFE00,
	0xFF00,
	0xFF80,
	0xFFC0,
	0xFFE0,
	0xFFF0,
	0xFFF8,
	0xFFFC,
	0xFFFE,
	0xFFFF
};

INLINE UINT16 logical_right_shift(UINT16 val, int c)
{
	return((val>>c) & right_shift_mask_table[c]);
}

INLINE INT16 arithmetic_right_shift(INT16 val, int c)
{
	if (val < 0)
		return((val>>c) | inverted_right_shift_mask_table[c]);
	else
		return((val>>c) & right_shift_mask_table[c]);
}





/*
    Set lae
*/
INLINE void setst_lae(INT16 val)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ);

	if (val > 0)
		I.STATUS |= (ST_LGT | ST_AGT);
	else if (val < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;
}


/*
    Set laep (BYTE)
*/
INLINE void setst_byte_laep(INT8 val)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ);

	if (val > 0)
		I.STATUS |= (ST_LGT | ST_AGT);
	else if (val < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	lastparity = val;
}

/*
    For COC, CZC, and TB
*/
INLINE void setst_e(UINT16 val, UINT16 to)
{
	if (val == to)
		I.STATUS |= ST_EQ;
	else
		I.STATUS &= ~ ST_EQ;
}

/*
    For CI, C, CB
*/
INLINE void setst_c_lae(UINT16 to, UINT16 val)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ);

	if (val == to)
		I.STATUS |= ST_EQ;
	else
	{
		if ( ((INT16) val) > ((INT16) to) )
			I.STATUS |= ST_AGT;
		if ( ((UINT16) val) > ((UINT16) to) )
		I.STATUS |= ST_LGT;
	}
}

/*
    Set laeco for add
*/
INLINE INT16 setst_add_laeco(int a, int b)
{
	UINT32 res;
	INT16 res2;

	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);

	res = (a & 0xffff) + (b & 0xffff);

	if (res & 0x10000)
		I.STATUS |= ST_C;

	if ((res ^ b) & (res ^ a) & 0x8000)
		I.STATUS |= ST_OV;

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
	if (((a & b) | ((a | b) & ~ res)) & 0x0800)
		I.STATUS |= ST_DC;
#endif

	res2 = (INT16) res;

	if (res2 > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (res2 < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	return res2;
}


/*
    Set laeco for subtract
*/
INLINE INT16 setst_sub_laeco(int a, int b)
{
	UINT32 res;
	INT16 res2;

	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);

	res = (a & 0xffff) - (b & 0xffff);

	if (! (res & 0x10000))
		I.STATUS |= ST_C;

	if ((a ^ b) & (a ^ res) & 0x8000)
		I.STATUS |= ST_OV;

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
	if (((a & ~ b) | ((a | ~ b) & ~ res)) & 0x0800)
		I.STATUS |= ST_DC;
#endif

	res2 = (INT16) res;

	if (res2 > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (res2 < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	return res2;
}


/*
    Set laecop for add (BYTE)
*/
INLINE INT8 setst_addbyte_laecop(int a, int b)
{
	unsigned int res;
	INT8 res2;

	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV | ST_OP);

	res = (a & 0xff) + (b & 0xff);

	if (res & 0x100)
		I.STATUS |= ST_C;

	if ((res ^ b) & (res ^ a) & 0x80)
		I.STATUS |= ST_OV;

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
	if (((a & b) | ((a | b) & ~ res)) & 0x08)
		I.STATUS |= ST_DC;
#endif

	res2 = (INT8) res;

	if (res2 > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (res2 < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	lastparity = res2;

	return res2;
}


/*
    Set laecop for subtract (BYTE)
*/
INLINE INT8 setst_subbyte_laecop(int a, int b)
{
	unsigned int res;
	INT8 res2;

	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV | ST_OP);

	res = (a & 0xff) - (b & 0xff);

	if (! (res & 0x100))
		I.STATUS |= ST_C;

	if ((a ^ b) & (a ^ res) & 0x80)
		I.STATUS |= ST_OV;

#if (TMS99XX_MODEL == TMS9940_ID) || (TMS99XX_MODEL == TMS9985_ID)
	if (((a & ~ b) | ((a | ~ b) & ~ res)) & 0x08)
		I.STATUS |= ST_DC;
#endif

	res2 = (INT8) res;

	if (res2 > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (res2 < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	lastparity = res2;

	return res2;
}



/*
    For NEG
*/
INLINE void setst_laeo(INT16 val)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_OV);

	if (val > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (val < 0)
	{
	I.STATUS |= ST_LGT;
	if (((UINT16) val) == 0x8000)
		I.STATUS |= ST_OV;
	}
	else
		I.STATUS |= ST_EQ;
}



/*
    Meat of SRA
*/
INLINE UINT16 setst_sra_laec(INT16 a, UINT16 c)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C);

	if (c != 0)
	{
		a = arithmetic_right_shift(a, c-1);
		if (a & 1)  // The carry bit equals the last bit that is shifted out
			I.STATUS |= ST_C;
		a = arithmetic_right_shift(a, 1);
	}

	if (a > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (a < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	return a;
}


/*
    Meat of SRL.  Same algorithm as SRA, except that we fills in with 0s.
*/
INLINE UINT16 setst_srl_laec(UINT16 a,UINT16 c)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C);

	if (c != 0)
	{
		a = logical_right_shift(a, c-1);
		if (a & 1)
			I.STATUS |= ST_C;
		a = logical_right_shift(a, 1);
	}

	if (((INT16) a) > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (((INT16) a) < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	return a;
}


//
// Meat of SRC
//
INLINE UINT16 setst_src_laec(UINT16 a,UINT16 c)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C);

	if (c != 0)
	{
		a = logical_right_shift(a, c) | (a << (16-c));
		if (a & 0x8000) // The carry bit equals the last bit that is shifted out
			I.STATUS |= ST_C;
	}

	if (((INT16) a) > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (((INT16) a) < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	return a;
}


//
// Meat of SLA
//
INLINE UINT16 setst_sla_laeco(UINT16 a, UINT16 c)
{
	I.STATUS &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);

	if (c != 0)
	{
		{
			register UINT16 mask;
			register UINT16 ousted_bits;

			mask = 0xFFFF << (16-c-1);
			ousted_bits = a & mask;

			if (ousted_bits)        // If ousted_bits is neither all 0s
				if (ousted_bits ^ mask)   // nor all 1s,
					I.STATUS |= ST_OV;  // we set overflow
		}

		a <<= c-1;
		if (a & 0x8000) // The carry bit equals the last bit that is shifted out
			I.STATUS |= ST_C;

		a <<= 1;
	}

	if (((INT16) a) > 0)
		I.STATUS |= ST_LGT | ST_AGT;
	else if (((INT16) a) < 0)
		I.STATUS |= ST_LGT;
	else
		I.STATUS |= ST_EQ;

	return a;
}


/***********************************************************************/

