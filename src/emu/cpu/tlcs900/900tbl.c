/*******************************************************************

TLCS-900/H instruction set

*******************************************************************/


enum e_operand
{
	_A=1,       /* currect register set register A */
	_C8,        /* current register set byte */
	_C16,       /* current register set word */
	_C32,       /* current register set long word */
	_MC16,      /* current register set mul/div register word */
	_CC,        /* condition */
	_CR8,
	_CR16,
	_CR32,
	_D8,        /* byte displacement */
	_D16,       /* word displacement */
	_F,         /* F register */
	_I3,        /* immediate 3 bit (part of last byte) */
	_I8,        /* immediate byte */
	_I16,       /* immediate word */
	_I24,       /* immediate 3 byte address */
	_I32,       /* immediate long word */
	_M,         /* memory location (defined by extension) */
	_M8,        /* (8) */
	_M16,       /* (i16) */
	_R,     /* register (defined by extension) */
	_SR,        /* status register */
};


INLINE int condition_true( tlcs900_state *cpustate, UINT8 cond )
{
	switch ( cond & 0x0f )
	{
	/* F */
	case 0x00:
		return 0;

	/* LT */
	case 0x01:
		return ( ( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) );

	/* LE */
	case 0x02:
		return ( ( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) ||
			( cpustate->sr.b.l & FLAG_ZF ) );

	/* ULE */
	case 0x03:
		return ( cpustate->sr.b.l & ( FLAG_ZF | FLAG_CF ) );

	/* OV */
	case 0x04:
		return ( cpustate->sr.b.l & FLAG_VF );

	/* MI */
	case 0x05:
		return ( cpustate->sr.b.l & FLAG_SF );

	/* Z */
	case 0x06:
		return ( cpustate->sr.b.l & FLAG_ZF );

	/* C */
	case 0x07:
		return ( cpustate->sr.b.l & FLAG_CF );

	/* T */
	case 0x08:
		return 1;

	/* GE */
	case 0x09:
		return ! ( ( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) );

	/* GT */
	case 0x0A:
		return ! ( ( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( cpustate->sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) ||
			( cpustate->sr.b.l & FLAG_ZF ) );

	/* UGT */
	case 0x0B:
		return ! ( cpustate->sr.b.l & ( FLAG_ZF | FLAG_CF ) );

	/* NOV */
	case 0x0C:
		return ! ( cpustate->sr.b.l & FLAG_VF );

	/* PL */
	case 0x0D:
		return ! ( cpustate->sr.b.l & FLAG_SF );

	/* NZ */
	case 0x0E:
		return ! ( cpustate->sr.b.l & FLAG_ZF );

	/* NC */
	case 0x0F:
		return ! ( cpustate->sr.b.l & FLAG_CF );
	}
	return 0;
}


INLINE UINT8 *get_reg8_current( tlcs900_state *cpustate, UINT8 reg )
{
	switch( reg & 7 )
	{
	/* W */
	case 0:
		return &cpustate->xwa[cpustate->regbank].b.h;

	/* A */
	case 1:
		return &cpustate->xwa[cpustate->regbank].b.l;

	/* B */
	case 2:
		return &cpustate->xbc[cpustate->regbank].b.h;

	/* C */
	case 3:
		return &cpustate->xbc[cpustate->regbank].b.l;

	/* D */
	case 4:
		return &cpustate->xde[cpustate->regbank].b.h;

	/* E */
	case 5:
		return &cpustate->xde[cpustate->regbank].b.l;

	/* H */
	case 6:
		return &cpustate->xhl[cpustate->regbank].b.h;

	/* L */
	case 7:
		return &cpustate->xhl[cpustate->regbank].b.l;
	}
	/* keep compiler happy */
	return &cpustate->dummy.b.l;
}


INLINE UINT16 *get_reg16_current( tlcs900_state *cpustate, UINT8 reg )
{
	switch( reg & 7 )
	{
	/* WA */
	case 0:
		return &cpustate->xwa[cpustate->regbank].w.l;

	/* BC */
	case 1:
		return &cpustate->xbc[cpustate->regbank].w.l;

	/* DE */
	case 2:
		return &cpustate->xde[cpustate->regbank].w.l;

	/* HL */
	case 3:
		return &cpustate->xhl[cpustate->regbank].w.l;

	/* IX */
	case 4:
		return &cpustate->xix.w.l;

	/* IY */
	case 5:
		return &cpustate->xiy.w.l;

	/* IZ */
	case 6:
		return &cpustate->xiz.w.l;

	/* SP */
	/* TODO: Use correct user/system SP */
	case 7:
		return &cpustate->xssp.w.l;
	}
	/* keep compiler happy */
	return &cpustate->dummy.w.l;
}


INLINE UINT32 *get_reg32_current( tlcs900_state *cpustate, UINT8 reg )
{
	switch( reg & 7 )
	{
	/* XWA */
	case 0:
		return &cpustate->xwa[cpustate->regbank].d;

	/* XBC */
	case 1:
		return &cpustate->xbc[cpustate->regbank].d;

	/* XDE */
	case 2:
		return &cpustate->xde[cpustate->regbank].d;

	/* XHL */
	case 3:
		return &cpustate->xhl[cpustate->regbank].d;

	/* XIX */
	case 4:
		return &cpustate->xix.d;

	/* XIY */
	case 5:
		return &cpustate->xiy.d;

	/* XIZ */
	case 6:
		return &cpustate->xiz.d;

	/* XSP */
	case 7:
		/* TODO: Add selector for user/system stack pointer */
		return &cpustate->xssp.d;
	}
	/* keep compiler happy */
	return &cpustate->dummy.d;
}


INLINE PAIR *get_reg( tlcs900_state *cpustate, UINT8 reg )
{
	UINT8   regbank;

	switch( reg & 0xf0 )
	{
	case 0x00: case 0x10: case 0x20: case 0x30: /* explicit register bank */
	case 0xd0:                                  /* "previous" register bank */
	case 0xe0:                                  /* current register bank */
		regbank = ( reg & 0xf0 ) >> 4;
		if ( regbank == 0x0d )
			regbank = ( cpustate->regbank - 1 ) & 0x03;

		if ( regbank == 0x0e )
			regbank = cpustate->regbank;

		switch ( reg & 0x0c )
		{
		case 0x00:  return &cpustate->xwa[regbank];
		case 0x04:  return &cpustate->xbc[regbank];
		case 0x08:  return &cpustate->xde[regbank];
		case 0x0c:  return &cpustate->xhl[regbank];
		}
		break;
	case 0xf0:  /* index registers and sp */
		switch ( reg & 0x0c )
		{
		case 0x00:  return &cpustate->xix;
		case 0x04:  return &cpustate->xiy;
		case 0x08:  return &cpustate->xiz;
		/* TODO: Use correct SP */
		case 0x0c:  return &cpustate->xssp;
		}
		break;
	}

	/* illegal/unknown register reference */
	logerror( "Access to unknown tlcs-900 cpu register %02x\n", reg );
	return &cpustate->dummy;
}


INLINE UINT8 *get_reg8( tlcs900_state *cpustate, UINT8 reg )
{
	PAIR    *r = get_reg( cpustate, reg );

	switch ( reg & 0x03 )
	{
	case 0x00:      return &r->b.l;
	case 0x01:      return &r->b.h;
	case 0x02:      return &r->b.h2;
	case 0x03:      return &r->b.h3;
	}

	return &r->b.l;
}


INLINE UINT16 *get_reg16( tlcs900_state *cpustate, UINT8 reg )
{
	PAIR    *r = get_reg( cpustate, reg );

	return ( reg & 0x02 ) ? &r->w.h : &r->w.l;
}


INLINE UINT32 *get_reg32( tlcs900_state *cpustate, UINT8 reg )
{
	PAIR    *r = get_reg( cpustate, reg );

	return &r->d;
}



INLINE void parity8( tlcs900_state *cpustate, UINT8 a )
{
	int i, j;

	j = 0;
	for ( i = 0; i < 8; i++ )
	{
		if ( a & 1 ) j++;
		a >>= 1;
	}
	cpustate->sr.b.l |= ( ( j & 1 ) ? 0 : FLAG_VF );
}


INLINE void parity16( tlcs900_state *cpustate, UINT16 a )
{
	int i, j;

	j = 0;
	for ( i = 0; i < 16; i++ )
	{
		if ( a & 1 ) j++;
		a >>= 1;
	}
	cpustate->sr.b.l |= ( ( j & 1 ) ? 0 : FLAG_VF );
}


INLINE void parity32( tlcs900_state *cpustate, UINT32 a )
{
	int i, j;

	j = 0;
	for ( i = 0; i < 32; i++ )
	{
		if ( a & 1 ) j++;
		a >>= 1;
	}
	cpustate->sr.b.l |= ( ( j & 1 ) ? 0 : FLAG_VF );
}


INLINE UINT8 adc8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 cy = cpustate->sr.b.l & FLAG_CF;
	UINT8 result = a + b + cy;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


INLINE UINT16 adc16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT8 cy = cpustate->sr.b.l & FLAG_CF;
	UINT16 result = a + b + cy;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


INLINE UINT32 adc32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT8 cy = cpustate->sr.b.l & FLAG_CF;
	UINT32 result = a + b + cy;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


INLINE UINT8 add8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 result = a + b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


INLINE UINT16 add16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT16 result = a + b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


INLINE UINT32 add32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT32 result = a + b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


INLINE UINT8 sbc8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 cy = cpustate->sr.b.l & FLAG_CF;
	UINT8 result = a - b - cy;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


INLINE UINT16 sbc16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT8 cy = cpustate->sr.b.l & FLAG_CF;
	UINT16 result = a - b - cy;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFFFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


INLINE UINT32 sbc32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT8 cy = cpustate->sr.b.l & FLAG_CF;
	UINT32 result = a - b - cy;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFFFFFFFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


INLINE UINT8 sub8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 result = a - b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


INLINE UINT16 sub16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT16 result = a - b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


INLINE UINT32 sub32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT32 result = a - b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


INLINE UINT8 and8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 result = a & b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	parity8( cpustate, result );

	return result;
}


INLINE UINT16 and16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT16 result = a & b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	parity16( cpustate, result );

	return result;
}


INLINE UINT32 and32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT32 result = a & b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	return result;
}


INLINE UINT8 or8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 result = a | b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity8( cpustate, result );

	return result;
}


INLINE UINT16 or16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT16 result = a | b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity16( cpustate, result );

	return result;
}


INLINE UINT32 or32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT32 result = a | b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	return result;
}


INLINE UINT8 xor8( tlcs900_state *cpustate, UINT8 a, UINT8 b)
{
	UINT8 result = a ^ b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity8( cpustate, result );

	return result;
}


INLINE UINT16 xor16( tlcs900_state *cpustate, UINT16 a, UINT16 b)
{
	UINT16 result = a ^ b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity16( cpustate, result );

	return result;
}


INLINE UINT32 xor32( tlcs900_state *cpustate, UINT32 a, UINT32 b)
{
	UINT32 result = a ^ b;

	cpustate->sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	cpustate->sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	return result;
}


INLINE void ldcf8( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		cpustate->sr.b.l |= FLAG_CF;
	else
		cpustate->sr.b.l &= ~ FLAG_CF;
}


INLINE void ldcf16( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		cpustate->sr.b.l |= FLAG_CF;
	else
		cpustate->sr.b.l &= ~ FLAG_CF;
}


INLINE void andcf8( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( ( b & ( 1 << ( a & 0x07 ) ) ) && ( cpustate->sr.b.l & FLAG_CF ) )
		cpustate->sr.b.l |= FLAG_CF;
	else
		cpustate->sr.b.l &= ~ FLAG_CF;
}


INLINE void andcf16( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( ( b & ( 1 << ( a & 0x0f ) ) ) && ( cpustate->sr.b.l & FLAG_CF ) )
		cpustate->sr.b.l |= FLAG_CF;
	else
		cpustate->sr.b.l &= ~ FLAG_CF;
}


INLINE void orcf8( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		cpustate->sr.b.l |= FLAG_CF;
}


INLINE void orcf16( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		cpustate->sr.b.l |= FLAG_CF;
}


INLINE void xorcf8( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		cpustate->sr.b.l ^= FLAG_CF;
}


INLINE void xorcf16( tlcs900_state *cpustate, UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		cpustate->sr.b.l ^= FLAG_CF;
}


INLINE UINT8 rl8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( a & 0x80 )
		{
			a = ( a << 1 ) | ( cpustate->sr.b.l & FLAG_CF );
			cpustate->sr.b.l |= FLAG_CF;
		}
		else
		{
			a = ( a << 1 ) | ( cpustate->sr.b.l & FLAG_CF );
			cpustate->sr.b.l &= ~ FLAG_CF;
		}
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 rl16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( a & 0x8000 )
		{
			a = ( a << 1 ) | ( cpustate->sr.b.l & FLAG_CF );
			cpustate->sr.b.l |= FLAG_CF;
		}
		else
		{
			a = ( a << 1 ) | ( cpustate->sr.b.l & FLAG_CF );
			cpustate->sr.b.l &= ~ FLAG_CF;
		}
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 rl32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( a & 0x80000000 )
		{
			a = ( a << 1 ) | ( cpustate->sr.b.l & FLAG_CF );
			cpustate->sr.b.l |= FLAG_CF;
		}
		else
		{
			a = ( a << 1 ) | ( cpustate->sr.b.l & FLAG_CF );
			cpustate->sr.b.l &= ~ FLAG_CF;
		}
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( cpustate, a );

	return a;
}

INLINE UINT8 rlc8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x80 ) ? 1 : 0 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 rlc16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x8000 ) ? 1 : 0 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 rlc32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x80000000 ) ? 1 : 0 );
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity32( cpustate, a );

	return a;
}


INLINE UINT8 rr8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( cpustate->sr.b.l & FLAG_CF )
		{
			cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 ) | 0x80;
		}
		else
		{
			cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 );
		}
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 rr16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( cpustate->sr.b.l & FLAG_CF )
		{
			cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 ) | 0x8000;
		}
		else
		{
			cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 );
		}
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 rr32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( cpustate->sr.b.l & FLAG_CF )
		{
			cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 ) | 0x80000000;
		}
		else
		{
			cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 );
		}
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( cpustate, a );

	return a;
}


INLINE UINT8 rrc8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x01 ) ? 0x80 : 0 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( ( a & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 rrc16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x0001 ) ? 0x8000 : 0 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( ( ( a >> 8 ) & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 rrc32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x00000001 ) ? 0x80000000 : 0 );
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( ( ( a >> 24 ) & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity32( cpustate, a );

	return a;
}


INLINE UINT8 sla8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( ( a & 0x80 ) ? FLAG_CF : 0 );
		a = ( a << 1 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 sla16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( ( a & 0x8000 ) ? FLAG_CF : 0 );
		a = ( a << 1 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 sla32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( ( a & 0x80000000 ) ? FLAG_CF : 0 );
		a = ( a << 1 );
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( cpustate, a );

	return a;
}


INLINE UINT8 sra8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a & 0x80 ) | ( a >> 1 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 sra16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a & 0x8000 ) | ( a >> 1 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 sra32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a & 0x80000000 ) | ( a >> 1 );
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( cpustate, a );

	return a;
}


INLINE UINT8 srl8( tlcs900_state *cpustate, UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a >> 1 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( cpustate, a );

	return a;
}


INLINE UINT16 srl16( tlcs900_state *cpustate, UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a >> 1 );
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( cpustate, a );

	return a;
}


INLINE UINT32 srl32( tlcs900_state *cpustate, UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a >> 1 );
		cpustate->cycles += 2;
	}

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( cpustate, a );

	return a;
}


INLINE UINT16 div8( tlcs900_state *cpustate, UINT16 a, UINT8 b )
{
	ldiv_t  result;

	if ( !b )
	{
		cpustate->sr.b.l |= FLAG_VF;
		return ( a << 8 ) | ( ( a >> 8 ) ^ 0xff );
	}

	if ( a >= ( 0x0200 * b ) ) {
		UINT16 diff = a - ( 0x0200 * b );
		UINT16 range = 0x100 - b;

		result = ldiv( diff, range );
		result.quot = 0x1ff - result.quot;
		result.rem = result.rem + b;
	}
	else
	{
		result = ldiv( a, b );
	}

	if ( result.quot > 0xff )
		cpustate->sr.b.l |= FLAG_VF;
	else
		cpustate->sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xff ) | ( ( result.rem & 0xff ) << 8 );
}


INLINE UINT32 div16( tlcs900_state *cpustate, UINT32 a, UINT16 b )
{
	ldiv_t  result;

	if ( !b )
	{
		cpustate->sr.b.l |= FLAG_VF;
		return ( a << 16 ) | ( ( a >> 16 ) ^ 0xffff );
	}

//  if ( a >= ( 0x02000000 * b ) ) {
//      UINT32 diff = a - ( 0x02000000 * b );
//      UINT32 range = 0x1000000 - b;
//
//      result = ldiv( diff, range );
//      result.quot = 0x1ffffff - result.quot;
//      result.rem = result.rem + b;
//  }
//  else
//  {
		result = ldiv( a, b );
//  }

	if ( result.quot > 0xffff )
		cpustate->sr.b.l |= FLAG_VF;
	else
		cpustate->sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xffff ) | ( ( result.rem & 0xffff ) << 16 );
}


INLINE UINT16 divs8( tlcs900_state *cpustate, INT16 a, INT8 b )
{
	ldiv_t  result;

	if ( !b )
	{
		cpustate->sr.b.l |= FLAG_VF;
		return ( a << 8 ) | ( ( a >> 8 ) ^ 0xff );
	}

	result = ldiv( a, b );

	if ( result.quot > 0xff )
		cpustate->sr.b.l |= FLAG_VF;
	else
		cpustate->sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xff ) | ( ( result.rem & 0xff ) << 8 );
}


INLINE UINT32 divs16( tlcs900_state *cpustate, INT32 a, INT16 b )
{
	ldiv_t  result;

	if ( !b )
	{
		cpustate->sr.b.l |= FLAG_VF;
		return ( a << 16 ) | ( ( a >> 16 ) ^ 0xffff );
	}

	result = ldiv( a, b );

	if ( result.quot > 0xffff )
		cpustate->sr.b.l |= FLAG_VF;
	else
		cpustate->sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xffff ) | ( ( result.rem & 0xffff ) << 16 );
}


static void _ADCBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, adc8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _ADCBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, adc8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _ADCBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = adc8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _ADCBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = adc8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _ADCBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = adc8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _ADCWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, adc16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _ADCWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, adc16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _ADCWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = adc16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _ADCWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = adc16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _ADCWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = adc16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _ADCLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, adc32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _ADCLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = adc32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _ADCLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = adc32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _ADCLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = adc32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _ADDBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, add8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _ADDBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, add8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _ADDBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = add8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _ADDBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = add8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _ADDBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = add8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _ADDWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, add16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _ADDWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, add16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _ADDWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = add16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _ADDWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = add16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _ADDWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = add16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _ADDLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, add32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _ADDLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = add32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _ADDLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = add32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _ADDLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = add32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _ANDBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, and8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _ANDBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, and8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _ANDBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = and8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _ANDBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = and8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _ANDBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = and8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _ANDWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, and16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _ANDWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, and16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _ANDWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = and16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _ANDWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = and16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _ANDWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = and16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _ANDLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, and32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _ANDLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = and32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _ANDLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = and32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _ANDLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = and32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _ANDCFBIM(tlcs900_state *cpustate)
{
	andcf8( cpustate, cpustate->imm1.b.l, RDMEM( cpustate->ea2.d ) );
}


static void _ANDCFBIR(tlcs900_state *cpustate)
{
	andcf8( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg8 );
}


static void _ANDCFBRM(tlcs900_state *cpustate)
{
	andcf8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _ANDCFBRR(tlcs900_state *cpustate)
{
	andcf8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _ANDCFWIR(tlcs900_state *cpustate)
{
	andcf16( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg16 );
}


static void _ANDCFWRR(tlcs900_state *cpustate)
{
	andcf16( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg16 );
}


static void _BITBIM(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( RDMEM( cpustate->ea2.d ) & ( 1 << ( cpustate->imm1.b.l & 0x07 ) ) )
		cpustate->sr.b.l |= FLAG_HF;
	else
		cpustate->sr.b.l |= FLAG_HF | FLAG_ZF;
}


static void _BITBIR(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( *cpustate->p2_reg8 & ( 1 << ( cpustate->imm1.b.l & 0x07 ) ) )
		cpustate->sr.b.l |= FLAG_HF;
	else
		cpustate->sr.b.l |= FLAG_HF | FLAG_ZF;
}


static void _BITWIR(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( *cpustate->p2_reg16 & ( 1 << ( cpustate->imm1.b.l & 0x0f ) ) )
		cpustate->sr.b.l |= FLAG_HF;
	else
		cpustate->sr.b.l |= FLAG_HF | FLAG_ZF;
}


static void _BS1BRR(tlcs900_state *cpustate)
{
	UINT16  r = *cpustate->p2_reg16;

	if ( r )
	{
		cpustate->sr.b.l &= ~ FLAG_VF;
		*cpustate->p1_reg8 = 15;
		while( r < 0x8000 )
		{
			r <<= 1;
			*cpustate->p1_reg8 -= 1;
		}
	}
	else
		cpustate->sr.b.l |= FLAG_VF;
}


static void _BS1FRR(tlcs900_state *cpustate)
{
	UINT16  r = *cpustate->p2_reg16;

	if ( r )
	{
		cpustate->sr.b.l &= ~ FLAG_VF;
		*cpustate->p1_reg8 = 0;
		while( ! ( r & 0x0001 ) )
		{
			r >>= 1;
			*cpustate->p1_reg8 += 1;
		}
	}
	else
		cpustate->sr.b.l |= FLAG_VF;
}


static void _CALLI(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 4;
	WRMEML( cpustate->xssp.d, cpustate->pc.d );
	cpustate->pc.d = cpustate->imm1.d;
	cpustate->prefetch_clear = true;
}


static void _CALLM(tlcs900_state *cpustate)
{
	if ( condition_true( cpustate, cpustate->op ) )
	{
		cpustate->xssp.d -= 4;
		WRMEML( cpustate->xssp.d, cpustate->pc.d );
		cpustate->pc.d = cpustate->ea2.d;
		cpustate->cycles += 6;
		cpustate->prefetch_clear = true;
	}
}


static void _CALR(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 4;
	WRMEML( cpustate->xssp.d, cpustate->pc.d );
	cpustate->pc.d = cpustate->ea1.d;
	cpustate->prefetch_clear = true;
}


static void _CCF(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ FLAG_NF;
	cpustate->sr.b.l ^= FLAG_CF;
}


static void _CHGBIM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) ^ ( 1 << ( cpustate->imm1.b.l & 0x07 ) ) );
}


static void _CHGBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 ^= ( 1 << ( cpustate->imm1.b.l & 0x07 ) );
}


static void _CHGWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 ^= ( 1 << ( cpustate->imm1.b.l & 0x0f ) );
}


static void _CPBMI(tlcs900_state *cpustate)
{
	sub8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l );
}


static void _CPBMR(tlcs900_state *cpustate)
{
	sub8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 );
}


static void _CPBRI(tlcs900_state *cpustate)
{
	sub8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _CPBRM(tlcs900_state *cpustate)
{
	sub8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _CPBRR(tlcs900_state *cpustate)
{
	sub8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _CPWMI(tlcs900_state *cpustate)
{
	sub16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l );
}


static void _CPWMR(tlcs900_state *cpustate)
{
	sub16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 );
}


static void _CPWRI(tlcs900_state *cpustate)
{
	sub16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _CPWRM(tlcs900_state *cpustate)
{
	sub16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _CPWRR(tlcs900_state *cpustate)
{
	sub16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _CPLMR(tlcs900_state *cpustate)
{
	sub32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 );
}


static void _CPLRI(tlcs900_state *cpustate)
{
	sub32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _CPLRM(tlcs900_state *cpustate)
{
	sub32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _CPLRR(tlcs900_state *cpustate)
{
	sub32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _CPD(tlcs900_state *cpustate)
{
	UINT8   result = *get_reg8_current( cpustate, 1 ) - RDMEM( *cpustate->p2_reg32 );
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	*cpustate->p2_reg32 -= 1;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


static void _CPDR(tlcs900_state *cpustate)
{
	_CPD( cpustate );

	if ( ( cpustate->sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _CPDW(tlcs900_state *cpustate)
{
	UINT16  result = *get_reg16_current( cpustate, 0 ) - RDMEMW( *cpustate->p2_reg32 );
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	*cpustate->p2_reg32 -= 2;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


static void _CPDRW(tlcs900_state *cpustate)
{
	_CPDW( cpustate );

	if ( ( cpustate->sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _CPI(tlcs900_state *cpustate)
{
	UINT8   result = *get_reg8_current( cpustate, 1 ) - RDMEM( *cpustate->p2_reg32 );
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	*cpustate->p2_reg32 += 1;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	cpustate->sr.b.l |= ( result & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


static void _CPIR(tlcs900_state *cpustate)
{
	_CPI( cpustate );

	if ( ( cpustate->sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _CPIW(tlcs900_state *cpustate)
{
	UINT16  result = *get_reg16_current( cpustate, 0 ) - RDMEMW( *cpustate->p2_reg32 );
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	*cpustate->p2_reg32 += 2;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	cpustate->sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


static void _CPIRW(tlcs900_state *cpustate)
{
	_CPIW( cpustate );

	if ( ( cpustate->sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _CPLBR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = ~ *cpustate->p1_reg8;
	cpustate->sr.b.l |= FLAG_HF | FLAG_NF;
}


static void _CPLWR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = ~ *cpustate->p1_reg16;
	cpustate->sr.b.l |= FLAG_HF | FLAG_NF;
}


static void _DAABR(tlcs900_state *cpustate)
{
	UINT8   oldval = *cpustate->p1_reg8;
	UINT8   fixval = 0;
	UINT8   carry = 0;
	UINT8   high = *cpustate->p1_reg8 & 0xf0;
	UINT8   low = *cpustate->p1_reg8 & 0x0f;

	if ( cpustate->sr.b.l & FLAG_CF )
	{
		if ( cpustate->sr.b.l & FLAG_HF )
		{
			fixval = 0x66;
		}
		else
		{
			if ( low < 0x0a )
				fixval = 0x60;
			else
				fixval = 0x66;
		}
		carry = 1;
	}
	else
	{
		if ( cpustate->sr.b.l & FLAG_HF )
		{
			if ( *cpustate->p1_reg8 < 0x9a )
				fixval = 0x06;
			else
				fixval = 0x66;
		}
		else
		{
			if ( high < 0x90 && low > 0x09 )
				fixval = 0x06;
			else if ( high > 0x80 && low > 0x09 )
				fixval = 0x66;
			else if ( high > 0x90 && low < 0x0a )
				fixval = 0x60;
		}
	}
	cpustate->sr.b.l &= ~ ( FLAG_VF | FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_CF );
	if ( cpustate->sr.b.l & FLAG_NF )
	{
		/* after SUB, SBC, or NEG operation */
		*cpustate->p1_reg8 -= fixval;
		cpustate->sr.b.l |= ( ( *cpustate->p1_reg8 > oldval || carry ) ? FLAG_CF : 0 );
	}
	else
	{
		/* after ADD or ADC operation */
		*cpustate->p1_reg8 += fixval;
		cpustate->sr.b.l |= ( ( *cpustate->p1_reg8 < oldval || carry ) ? FLAG_CF : 0 );
	}
	cpustate->sr.b.l |= ( *cpustate->p1_reg8 & FLAG_SF ) | ( *cpustate->p1_reg8 ? 0 : FLAG_ZF ) |
		( ( ( oldval ^ fixval ) ^ *cpustate->p1_reg8 ) & FLAG_HF );

	parity8( cpustate, *cpustate->p1_reg8 );
}


static void _DB(tlcs900_state *cpustate)
{
	logerror("%08x: invalid or illegal instruction\n", cpustate->pc.d );
}


static void _DECBIM(tlcs900_state *cpustate)
{
	UINT8   cy = cpustate->sr.b.l & FLAG_CF;

	WRMEM( cpustate->ea2.d, sub8( cpustate, RDMEM( cpustate->ea2.d ), cpustate->imm1.b.l ? cpustate->imm1.b.l : 8 ) );
	cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | cy;
}


static void _DECBIR(tlcs900_state *cpustate)
{
	UINT8   cy = cpustate->sr.b.l & FLAG_CF;

	*cpustate->p2_reg8 = sub8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l ? cpustate->imm1.b.l : 8 );
	cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | cy;
}


static void _DECWIM(tlcs900_state *cpustate)
{
	UINT8   cy = cpustate->sr.b.l & FLAG_CF;

	WRMEMW( cpustate->ea2.d, sub16( cpustate, RDMEMW( cpustate->ea2.d ), cpustate->imm1.b.l ? cpustate->imm1.b.l : 8 ) );
	cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | cy;
}


static void _DECWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 -= cpustate->imm1.b.l ? cpustate->imm1.b.l : 8;
}


static void _DECLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 -= cpustate->imm1.b.l ? cpustate->imm1.b.l : 8;
}


static void _DECF(tlcs900_state *cpustate)
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	cpustate->sr.b.h = ( cpustate->sr.b.h & 0xf8 ) | ( ( cpustate->sr.b.h - 1 ) & 0x07 );
	cpustate->regbank = cpustate->sr.b.h & 0x03;
}


static void _DIVBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = div8( cpustate, *cpustate->p1_reg16, cpustate->imm2.b.l );
}


static void _DIVBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = div8( cpustate, *cpustate->p1_reg16, RDMEM( cpustate->ea2.d ) );
}


static void _DIVBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = div8( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg8 );
}


static void _DIVWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = div16( cpustate, *cpustate->p1_reg32, cpustate->imm2.w.l );
}


static void _DIVWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = div16( cpustate, *cpustate->p1_reg32, RDMEMW( cpustate->ea2.d ) );
}


static void _DIVWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = div16( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg16 );
}


static void _DIVSBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = divs8( cpustate, *cpustate->p1_reg16, cpustate->imm2.b.l );
}


static void _DIVSBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = divs8( cpustate, *cpustate->p1_reg16, RDMEM( cpustate->ea2.d ) );
}


static void _DIVSBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = divs8( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg8 );
}


static void _DIVSWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = divs16( cpustate, *cpustate->p1_reg32, cpustate->imm2.w.l );
}


static void _DIVSWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = divs16( cpustate, *cpustate->p1_reg32, RDMEMW( cpustate->ea2.d ) );
}


static void _DIVSWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = divs16( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg16 );
}


static void _DJNZB(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 -= 1;
	if ( *cpustate->p1_reg8 )
	{
		cpustate->pc.d = cpustate->ea2.d;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _DJNZW(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 -= 1;
	if ( *cpustate->p1_reg16 )
	{
		cpustate->pc.d = cpustate->ea2.d;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _EI(tlcs900_state *cpustate)
{
	cpustate->sr.b.h = ( cpustate->sr.b.h & 0x8f ) | ( ( cpustate->imm1.b.l & 0x07 ) << 4 );
	cpustate->check_irqs = 1;
}


static void _EXBMR(tlcs900_state *cpustate)
{
	UINT8   i = RDMEM( cpustate->ea1.d );

	WRMEM( cpustate->ea1.d, *cpustate->p2_reg8 );
	*cpustate->p2_reg8 = i;
}


static void _EXBRR(tlcs900_state *cpustate)
{
	UINT8   i = *cpustate->p2_reg8;

	*cpustate->p2_reg8 = *cpustate->p1_reg8;
	*cpustate->p1_reg8 = i;
}


static void _EXWMR(tlcs900_state *cpustate)
{
	UINT16  i = RDMEMW( cpustate->ea1.d );

	WRMEMW( cpustate->ea1.d, *cpustate->p2_reg16 );
	*cpustate->p2_reg16 = i;
}


static void _EXWRR(tlcs900_state *cpustate)
{
	UINT16  i = *cpustate->p2_reg16;

	*cpustate->p2_reg16 = *cpustate->p1_reg16;
	*cpustate->p1_reg16 = i;
}


static void _EXTSWR(tlcs900_state *cpustate)
{
	if ( *cpustate->p1_reg16 & 0x0080 )
		*cpustate->p1_reg16 |= 0xff00;
	else
		*cpustate->p1_reg16 &= 0x00ff;
}


static void _EXTSLR(tlcs900_state *cpustate)
{
	if ( *cpustate->p1_reg32 & 0x00008000 )
		*cpustate->p1_reg32 |= 0xffff0000;
	else
		*cpustate->p1_reg32 &= 0x0000ffff;
}


static void _EXTZWR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 &= 0x00ff;
}


static void _EXTZLR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 &= 0x0000ffff;
}


static void _HALT(tlcs900_state *cpustate)
{
	cpustate->halted = 1;
}


static void _INCBIM(tlcs900_state *cpustate)
{
	UINT8   cy = cpustate->sr.b.l & FLAG_CF;

	WRMEM( cpustate->ea2.d, add8( cpustate, RDMEM( cpustate->ea2.d ), cpustate->imm1.b.l ? cpustate->imm1.b.l : 8 ) );
	cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | cy;
}


static void _INCBIR(tlcs900_state *cpustate)
{
	UINT8   cy = cpustate->sr.b.l & FLAG_CF;

	*cpustate->p2_reg8 = add8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l ? cpustate->imm1.b.l : 8 );
	cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | cy;
}


static void _INCWIM(tlcs900_state *cpustate)
{
	UINT8   cy = cpustate->sr.b.l & FLAG_CF;

	WRMEMW( cpustate->ea2.d, add16( cpustate, RDMEMW( cpustate->ea2.d ), cpustate->imm1.b.l ? cpustate->imm1.b.l : 8 ) );
	cpustate->sr.b.l = ( cpustate->sr.b.l & ~ FLAG_CF ) | cy;
}


static void _INCWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 += cpustate->imm1.b.l ? cpustate->imm1.b.l : 8;
}


static void _INCLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 += cpustate->imm1.b.l ? cpustate->imm1.b.l : 8;
}


static void _INCF(tlcs900_state *cpustate)
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	cpustate->sr.b.h = ( cpustate->sr.b.h & 0xf8 ) | ( ( cpustate->sr.b.h + 1 ) & 0x07 );
	cpustate->regbank = cpustate->sr.b.h & 0x03;
}


static void _JPI(tlcs900_state *cpustate)
{
	cpustate->pc.d = cpustate->imm1.d;
	cpustate->prefetch_clear = true;
}


static void _JPM(tlcs900_state *cpustate)
{
	if ( condition_true( cpustate, cpustate->op ) )
	{
		cpustate->pc.d = cpustate->ea2.d;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _JR(tlcs900_state *cpustate)
{
	if ( condition_true( cpustate, cpustate->op ) )
	{
		cpustate->pc.d = cpustate->ea2.d;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _JRL(tlcs900_state *cpustate)
{
	if ( condition_true( cpustate, cpustate->op ) )
	{
		cpustate->pc.d = cpustate->ea2.d;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _LDBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, cpustate->imm2.b.l );
}


static void _LDBMM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, RDMEM( cpustate->ea2.d ) );
}


static void _LDBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, *cpustate->p2_reg8 );
}


static void _LDBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = cpustate->imm2.b.l;
}


static void _LDBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = RDMEM( cpustate->ea2.d );
}


static void _LDBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = *cpustate->p2_reg8;
}


static void _LDWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, cpustate->imm2.w.l );
}


static void _LDWMM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, RDMEMW( cpustate->ea2.d ) );
}


static void _LDWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, *cpustate->p2_reg16 );
}


static void _LDWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = cpustate->imm2.w.l;
}


static void _LDWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = RDMEMW( cpustate->ea2.d );
}


static void _LDWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = *cpustate->p2_reg16;
}


static void _LDLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = cpustate->imm2.d;
}


static void _LDLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = RDMEML( cpustate->ea2.d );
}


static void _LDLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = *cpustate->p2_reg32;
}


static void _LDLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, *cpustate->p2_reg32 );
}


static void _LDAW(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = cpustate->ea2.w.l;
}


static void _LDAL(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = cpustate->ea2.d;
}


static void _LDCBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = *cpustate->p2_reg8;
}


static void _LDCWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = *cpustate->p2_reg16;
}


static void _LDCLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = *cpustate->p2_reg32;
}


static void _LDCFBIM(tlcs900_state *cpustate)
{
	ldcf8( cpustate, cpustate->imm1.b.l, RDMEM( cpustate->ea2.d ) );
}


static void _LDCFBIR(tlcs900_state *cpustate)
{
	ldcf8( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg8 );
}


static void _LDCFBRM(tlcs900_state *cpustate)
{
	ldcf8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _LDCFBRR(tlcs900_state *cpustate)
{
	ldcf8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _LDCFWIR(tlcs900_state *cpustate)
{
	ldcf16( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg16 );
}


static void _LDCFWRR(tlcs900_state *cpustate)
{
	ldcf16( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg16 );
}


static void _LDD(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEM( *cpustate->p1_reg32, RDMEM( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 -= 1;
	*cpustate->p2_reg32 -= 1;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
	}
}


static void _LDDR(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEM( *cpustate->p1_reg32, RDMEM( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 -= 1;
	*cpustate->p2_reg32 -= 1;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _LDDRW(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEMW( *cpustate->p1_reg32, RDMEMW( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 -= 2;
	*cpustate->p2_reg32 -= 2;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _LDDW(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEMW( *cpustate->p1_reg32, RDMEMW( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 -= 2;
	*cpustate->p2_reg32 -= 2;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
	}
}


static void _LDF(tlcs900_state *cpustate)
{
	cpustate->sr.b.h = ( cpustate->sr.b.h & 0xf8 ) | ( cpustate->imm1.b.l & 0x07 );
	cpustate->regbank = cpustate->imm1.b.l & 0x03;
}


static void _LDI(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEM( *cpustate->p1_reg32, RDMEM( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 += 1;
	*cpustate->p2_reg32 += 1;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
	}
}


static void _LDIR(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEM( *cpustate->p1_reg32, RDMEM( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 += 1;
	*cpustate->p2_reg32 += 1;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _LDIRW(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEMW( *cpustate->p1_reg32, RDMEMW( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 += 2;
	*cpustate->p2_reg32 += 2;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
		cpustate->pc.d -= 2;
		cpustate->cycles += 4;
		cpustate->prefetch_clear = true;
	}
}


static void _LDIW(tlcs900_state *cpustate)
{
	UINT16  *bc = get_reg16_current( cpustate, 1 );

	WRMEMW( *cpustate->p1_reg32, RDMEMW( *cpustate->p2_reg32 ) );
	*cpustate->p1_reg32 += 2;
	*cpustate->p2_reg32 += 2;
	*bc -= 1;
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		cpustate->sr.b.l |= FLAG_VF;
	}
}


static void _LDX(tlcs900_state *cpustate)
{
	UINT8   a, b;

	RDOP( cpustate );
	a = RDOP( cpustate );
	RDOP( cpustate );
	b = RDOP( cpustate );
	RDOP( cpustate );
	WRMEM( a, b );
}


static void _LINK(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 4;
	WRMEML( cpustate->xssp.d, *cpustate->p1_reg32 );
	*cpustate->p1_reg32 = cpustate->xssp.d;
	cpustate->xssp.d += cpustate->imm2.sw.l;
}


static void _MAX(tlcs900_state *cpustate)
{
	cpustate->sr.b.h |= 0x04;
}


static void _MDEC1(tlcs900_state *cpustate)
{
	if ( ( *cpustate->p2_reg16 & cpustate->imm1.w.l ) == cpustate->imm1.w.l )
		*cpustate->p2_reg16 += cpustate->imm1.w.l;
	else
		*cpustate->p2_reg16 -= 1;
}


static void _MDEC2(tlcs900_state *cpustate)
{
	if ( ( *cpustate->p2_reg16 & cpustate->imm1.w.l ) == cpustate->imm1.w.l )
		*cpustate->p2_reg16 += cpustate->imm1.w.l;
	else
		*cpustate->p2_reg16 -= 2;
}


static void _MDEC4(tlcs900_state *cpustate)
{
	if ( ( *cpustate->p2_reg16 & cpustate->imm1.w.l ) == cpustate->imm1.w.l )
		*cpustate->p2_reg16 += cpustate->imm1.w.l;
	else
		*cpustate->p2_reg16 -= 4;
}


static void _MINC1(tlcs900_state *cpustate)
{
	if ( ( *cpustate->p2_reg16 & cpustate->imm1.w.l ) == cpustate->imm1.w.l )
		*cpustate->p2_reg16 -= cpustate->imm1.w.l;
	else
		*cpustate->p2_reg16 += 1;
}


static void _MINC2(tlcs900_state *cpustate)
{
	if ( ( *cpustate->p2_reg16 & cpustate->imm1.w.l ) == cpustate->imm1.w.l )
		*cpustate->p2_reg16 -= cpustate->imm1.w.l;
	else
		*cpustate->p2_reg16 += 2;
}


static void _MINC4(tlcs900_state *cpustate)
{
	if ( ( *cpustate->p2_reg16 & cpustate->imm1.w.l ) == cpustate->imm1.w.l )
		*cpustate->p2_reg16 -= cpustate->imm1.w.l;
	else
		*cpustate->p2_reg16 += 4;
}


static void _MIRRW(tlcs900_state *cpustate)
{
	UINT16  r = *cpustate->p1_reg16;
	UINT16  s = ( r & 0x01 );
	int i;


	for ( i = 0; i < 15; i++ )
	{
		r >>= 1;
		s <<= 1;
		s |= ( r & 0x01 );
	}

	*cpustate->p1_reg16 = s;
}


static void _MULBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = ( *cpustate->p1_reg16 & 0xff ) * cpustate->imm2.b.l;
}


static void _MULBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = ( *cpustate->p1_reg16 & 0xff ) * RDMEM( cpustate->ea2.d );
}


static void _MULBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = ( *cpustate->p1_reg16 & 0xff ) * *cpustate->p2_reg8;
}


static void _MULWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = ( *cpustate->p1_reg32 & 0xffff ) * cpustate->imm2.w.l;
}


static void _MULWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = ( *cpustate->p1_reg32 & 0xffff ) * RDMEMW( cpustate->ea2.d );
}


static void _MULWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = ( *cpustate->p1_reg32 & 0xffff ) * *cpustate->p2_reg16;
}


static void _MULAR(tlcs900_state *cpustate)
{
	UINT32  *xde = get_reg32_current( cpustate, 2 );
	UINT32  *xhl = get_reg32_current( cpustate, 3 );

	*cpustate->p1_reg32 = *cpustate->p1_reg32 + ( ((INT16)RDMEMW( *xde )) * ((INT16)RDMEMW( *xhl )) );
	*xhl -= 2;

	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_VF );
	cpustate->sr.b.l |= ( ( *cpustate->p1_reg32 >> 24 ) & FLAG_SF ) | ( *cpustate->p1_reg32 ? 0 : FLAG_ZF );
}


static void _MULSBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = (INT8)( *cpustate->p1_reg16 & 0xff ) * cpustate->imm2.sb.l;
}


static void _MULSBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = (INT8)( *cpustate->p1_reg16 & 0xff ) * (INT8)RDMEM( cpustate->ea2.d );
}


static void _MULSBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = (INT8)( *cpustate->p1_reg16 & 0xff ) * (INT8)*cpustate->p2_reg8;
}


static void _MULSWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = (INT16)( *cpustate->p1_reg32 & 0xffff ) * cpustate->imm2.sw.l;
}


static void _MULSWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = (INT16)( *cpustate->p1_reg32 & 0xffff ) * (INT16)RDMEMW( cpustate->ea2.d );
}


static void _MULSWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = (INT16)( *cpustate->p1_reg32 & 0xffff ) * (INT16)*cpustate->p2_reg16;
}


static void _NEGBR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sub8( cpustate, 0, *cpustate->p1_reg8 );
}


static void _NEGWR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sub16( cpustate, 0, *cpustate->p1_reg16 );
}


static void _NOP(tlcs900_state *cpustate)
{
	/* Do nothing */
}


static void _NORMAL(tlcs900_state *cpustate)
{
	cpustate->sr.b.h &= 0x7F;
}


static void _ORBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, or8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _ORBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, or8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _ORBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = or8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _ORBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = or8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _ORBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = or8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _ORWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, or16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _ORWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, or16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _ORWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = or16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _ORWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = or16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _ORWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = or16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _ORLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, or32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _ORLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = or32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _ORLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = or32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _ORLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = or32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _ORCFBIM(tlcs900_state *cpustate)
{
	orcf8( cpustate, cpustate->imm1.b.l, RDMEM( cpustate->ea2.d ) );
}


static void _ORCFBIR(tlcs900_state *cpustate)
{
	orcf8( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg8 );
}


static void _ORCFBRM(tlcs900_state *cpustate)
{
	orcf8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _ORCFBRR(tlcs900_state *cpustate)
{
	orcf8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _ORCFWIR(tlcs900_state *cpustate)
{
	orcf16( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg16 );
}


static void _ORCFWRR(tlcs900_state *cpustate)
{
	orcf16( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg16 );
}


static void _PAAWR(tlcs900_state *cpustate)
{
	if ( *cpustate->p1_reg16 & 1 )
		*cpustate->p1_reg16 += 1;
}


static void _PAALR(tlcs900_state *cpustate)
{
	if ( *cpustate->p1_reg32 & 1 )
		*cpustate->p1_reg32 += 1;
}


static void _POPBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, RDMEM( cpustate->xssp.d ) );
	cpustate->xssp.d += 1;
}


static void _POPBR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = RDMEM( cpustate->xssp.d );
	cpustate->xssp.d += 1;
}


static void _POPWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, RDMEMW( cpustate->xssp.d ) );
	cpustate->xssp.d += 2;
}


static void _POPWR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = RDMEMW( cpustate->xssp.d );
	cpustate->xssp.d += 2;
}


static void _POPWSR(tlcs900_state *cpustate)
{
	_POPWR( cpustate );
	cpustate->regbank = cpustate->sr.b.h & 0x03;
	cpustate->check_irqs = 1;
}


static void _POPLR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = RDMEML( cpustate->xssp.d );
	cpustate->xssp.d += 4;
}


static void _PUSHBI(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 1;
	WRMEM( cpustate->xssp.d, cpustate->imm1.b.l );
}


static void _PUSHBM(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 1;
	WRMEM( cpustate->xssp.d, RDMEM( cpustate->ea1.d ) );
}


static void _PUSHBR(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 1;
	WRMEM( cpustate->xssp.d, *cpustate->p1_reg8 );
}


static void _PUSHWI(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 2;
	WRMEMW( cpustate->xssp.d, cpustate->imm1.w.l );
}


static void _PUSHWM(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 2;
	WRMEMW( cpustate->xssp.d, RDMEMW( cpustate->ea1.d ) );
}


static void _PUSHWR(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 2;
	WRMEMW( cpustate->xssp.d, *cpustate->p1_reg16 );
}


static void _PUSHLR(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 4;
	WRMEML( cpustate->xssp.d, *cpustate->p1_reg32 );
}


static void _RCF(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_NF | FLAG_CF );
}


static void _RESBIM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) & ~( 1 << ( cpustate->imm1.d & 0x07 ) ) );
}


static void _RESBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = *cpustate->p2_reg8 & ~( 1 << ( cpustate->imm1.d & 0x07 ) );
}


static void _RESWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = *cpustate->p2_reg16 & ~( 1 << ( cpustate->imm1.d & 0x0f ) );
}


static void _RET(tlcs900_state *cpustate)
{
	cpustate->pc.d = RDMEML( cpustate->xssp.d );
	cpustate->xssp.d += 4;
	cpustate->prefetch_clear = true;
}


static void _RETCC(tlcs900_state *cpustate)
{
	if ( condition_true( cpustate, cpustate->op ) )
	{
		cpustate->pc.d = RDMEML( cpustate->xssp.d );
		cpustate->xssp.d += 4;
		cpustate->cycles += 6;
		cpustate->prefetch_clear = true;
	}
}


static void _RETD(tlcs900_state *cpustate)
{
	cpustate->pc.d = RDMEML( cpustate->xssp.d );
	cpustate->xssp.d += 4 + cpustate->imm1.sw.l;
	cpustate->prefetch_clear = true;
}


static void _RETI(tlcs900_state *cpustate)
{
	cpustate->sr.w.l = RDMEMW( cpustate->xssp.d );
	cpustate->xssp.d += 2;
	cpustate->pc.d = RDMEML( cpustate->xssp.d );
	cpustate->xssp.d += 4;
	cpustate->regbank = cpustate->sr.b.h & 0x03;
	cpustate->check_irqs = 1;
	cpustate->prefetch_clear = true;
}


static void _RLBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, rl8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _RLWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, rl16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _RLBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rl8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _RLBRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rl8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _RLWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rl16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _RLWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rl16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _RLLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rl32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _RLLRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rl32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _RLCBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, rlc8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _RLCWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, rlc16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _RLCBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rlc8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _RLCBRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rlc8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _RLCWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rlc16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _RLCWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rlc16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _RLCLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rlc32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _RLCLRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rlc32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _RLDRM(tlcs900_state *cpustate)
{
	UINT8   a = *cpustate->p1_reg8 & 0x0f;
	UINT8   b = RDMEM( cpustate->ea2.d );

	*cpustate->p1_reg8 = ( *cpustate->p1_reg8 & 0xf0 ) | ( ( b & 0xf0 ) >> 4 );
	WRMEM( cpustate->ea2.d, ( ( b & 0x0f ) << 4 ) | a );
	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( *cpustate->p1_reg8 & FLAG_SF ) | ( *cpustate->p1_reg8 ? 0 : FLAG_ZF );
	parity8( cpustate, *cpustate->p1_reg8 );
}


static void _RRBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, rr8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _RRWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, rr16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _RRBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rr8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _RRBRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rr8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _RRWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rr16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _RRWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rr16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _RRLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rr32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _RRLRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rr32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _RRCBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, rrc8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _RRCWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, rrc16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _RRCBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rrc8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _RRCBRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = rrc8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _RRCWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rrc16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _RRCWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = rrc16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _RRCLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rrc32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _RRCLRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = rrc32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _RRDRM(tlcs900_state *cpustate)
{
	UINT8   a = *cpustate->p1_reg8 & 0x0f;
	UINT8   b = RDMEM( cpustate->ea2.d );

	*cpustate->p1_reg8 = ( *cpustate->p1_reg8 & 0xf0 ) | ( b & 0x0f );
	WRMEM( cpustate->ea2.d, ( ( b & 0xf0 ) >> 4 ) | ( a << 4 ) );
	cpustate->sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( *cpustate->p1_reg8 & FLAG_SF ) | ( *cpustate->p1_reg8 ? 0 : FLAG_ZF );
	parity8( cpustate, *cpustate->p1_reg8 );
}


static void _SBCBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, sbc8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _SBCBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, sbc8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _SBCBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sbc8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _SBCBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sbc8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _SBCBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sbc8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _SBCWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, sbc16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _SBCWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, sbc16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _SBCWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sbc16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _SBCWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sbc16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _SBCWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sbc16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _SBCLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, sbc32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _SBCLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = sbc32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _SBCLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = sbc32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _SBCLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = sbc32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _SCCBR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = condition_true( cpustate, cpustate->op ) ? 1 : 0;
}


static void _SCCWR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = condition_true( cpustate, cpustate->op ) ? 1 : 0;
}


static void _SCF(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ ( FLAG_HF | FLAG_NF );
	cpustate->sr.b.l |= FLAG_CF;
}


static void _SETBIM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) | ( 1 << ( cpustate->imm1.d & 0x07 ) ) );
}


static void _SETBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = *cpustate->p2_reg8 | ( 1 << ( cpustate->imm1.d & 0x07 ) );
}


static void _SETWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = *cpustate->p2_reg16 | ( 1 << ( cpustate->imm1.d & 0x0f ) );
}


static void _SLABM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, sla8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _SLAWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, sla16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _SLABIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = sla8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _SLABRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = sla8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _SLAWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = sla16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _SLAWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = sla16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _SLALIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = sla32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _SLALRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = sla32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _SLLBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, sla8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _SLLWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, sla16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _SLLBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = sla8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _SLLBRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = sla8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _SLLWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = sla16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _SLLWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = sla16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _SLLLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = sla32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _SLLLRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = sla32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _SRABM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, sra8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _SRAWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, sra16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _SRABIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = sra8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _SRABRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = sra8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _SRAWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = sra16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _SRAWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = sra16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _SRALIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = sra32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _SRALRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = sra32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _SRLBM(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea2.d, srl8( cpustate, RDMEM( cpustate->ea2.d ), 1 ) );
}


static void _SRLWM(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea2.d, srl16( cpustate, RDMEMW( cpustate->ea2.d ), 1 ) );
}


static void _SRLBIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = srl8( cpustate, *cpustate->p2_reg8, cpustate->imm1.b.l );
}


static void _SRLBRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg8 = srl8( cpustate, *cpustate->p2_reg8, *cpustate->p1_reg8 );
}


static void _SRLWIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = srl16( cpustate, *cpustate->p2_reg16, cpustate->imm1.b.l );
}


static void _SRLWRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg16 = srl16( cpustate, *cpustate->p2_reg16, *cpustate->p1_reg8 );
}


static void _SRLLIR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = srl32( cpustate, *cpustate->p2_reg32, cpustate->imm1.b.l );
}


static void _SRLLRR(tlcs900_state *cpustate)
{
	*cpustate->p2_reg32 = srl32( cpustate, *cpustate->p2_reg32, *cpustate->p1_reg8 );
}


static void _STCFBIM(tlcs900_state *cpustate)
{
	if ( cpustate->sr.b.l & FLAG_CF )
		WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) | ( 1 << ( cpustate->imm1.b.l & 0x07 ) ) );
	else
		WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) & ~ ( 1 << ( cpustate->imm1.b.l & 0x07 ) ) );
}


static void _STCFBIR(tlcs900_state *cpustate)
{
	if ( cpustate->sr.b.l & FLAG_CF )
		*cpustate->p2_reg8 |= ( 1 << ( cpustate->imm1.b.l & 0x07 ) );
	else
		*cpustate->p2_reg8 &= ~ ( 1 << ( cpustate->imm1.b.l & 0x07 ) );
}


static void _STCFBRM(tlcs900_state *cpustate)
{
	if ( cpustate->sr.b.l & FLAG_CF )
		WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) | ( 1 << ( *cpustate->p1_reg8 & 0x07 ) ) );
	else
		WRMEM( cpustate->ea2.d, RDMEM( cpustate->ea2.d ) & ~ ( 1 << ( *cpustate->p1_reg8 & 0x07 ) ) );
}


static void _STCFBRR(tlcs900_state *cpustate)
{
	if ( cpustate->sr.b.l & FLAG_CF )
		*cpustate->p2_reg8 |= ( 1 << ( *cpustate->p1_reg8 & 0x07 ) );
	else
		*cpustate->p2_reg8 &= ~ ( 1 << ( *cpustate->p1_reg8 & 0x07 ) );
}


static void _STCFWIR(tlcs900_state *cpustate)
{
	if ( cpustate->sr.b.l & FLAG_CF )
		*cpustate->p2_reg16 |= ( 1 << ( cpustate->imm1.b.l & 0x0f ) );
	else
		*cpustate->p2_reg16 &= ~ ( 1 << ( cpustate->imm1.b.l & 0x0f ) );
}


static void _STCFWRR(tlcs900_state *cpustate)
{
	if ( cpustate->sr.b.l & FLAG_CF )
		*cpustate->p2_reg16 |= ( 1 << ( *cpustate->p1_reg8 & 0x0f ) );
	else
		*cpustate->p2_reg16 &= ~ ( 1 << ( *cpustate->p1_reg8 & 0x0f ) );
}


static void _SUBBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, sub8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _SUBBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, sub8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _SUBBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sub8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _SUBBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sub8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _SUBBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = sub8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _SUBWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, sub16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _SUBWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, sub16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _SUBWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sub16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _SUBWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sub16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _SUBWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = sub16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _SUBLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, sub32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _SUBLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = sub32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _SUBLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = sub32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _SUBLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = sub32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _SWI(tlcs900_state *cpustate)
{
	cpustate->xssp.d -= 4;
	WRMEML( cpustate->xssp.d, cpustate->pc.d );
	cpustate->xssp.d -= 2;
	WRMEMW( cpustate->xssp.d, cpustate->sr.w.l );
	cpustate->pc.d = RDMEML( 0x00ffff00 + 4 * cpustate->imm1.b.l );
	cpustate->prefetch_clear = true;
}


static void _TSETBIM(tlcs900_state *cpustate)
{
	UINT8   b = 1 << ( cpustate->imm1.b.l & 0x07 );
	UINT8   a = RDMEM( cpustate->ea2.d );

	cpustate->sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	cpustate->sr.b.l |= ( ( a & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	WRMEM( cpustate->ea2.d, a | b );
}


static void _TSETBIR(tlcs900_state *cpustate)
{
	UINT8   b = 1 << ( cpustate->imm1.b.l & 0x07 );

	cpustate->sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	cpustate->sr.b.l |= ( ( *cpustate->p2_reg8 & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	*cpustate->p2_reg8 |= b;
}


static void _TSETWIR(tlcs900_state *cpustate)
{
	UINT16  b = 1 << ( cpustate->imm1.b.l & 0x0f );

	cpustate->sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	cpustate->sr.b.l |= ( ( *cpustate->p2_reg16 & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	*cpustate->p2_reg16 |= b;
}


static void _UNLK(tlcs900_state *cpustate)
{
	cpustate->xssp.d = *cpustate->p1_reg32;
	*cpustate->p1_reg32 = RDMEML( cpustate->xssp.d );
	cpustate->xssp.d += 4;
}


static void _XORBMI(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, xor8( cpustate, RDMEM( cpustate->ea1.d ), cpustate->imm2.b.l ) );
}


static void _XORBMR(tlcs900_state *cpustate)
{
	WRMEM( cpustate->ea1.d, xor8( cpustate, RDMEM( cpustate->ea1.d ), *cpustate->p2_reg8 ) );
}


static void _XORBRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = xor8( cpustate, *cpustate->p1_reg8, cpustate->imm2.b.l );
}


static void _XORBRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = xor8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _XORBRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg8 = xor8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _XORWMI(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, xor16( cpustate, RDMEMW( cpustate->ea1.d ), cpustate->imm2.w.l ) );
}


static void _XORWMR(tlcs900_state *cpustate)
{
	WRMEMW( cpustate->ea1.d, xor16( cpustate, RDMEMW( cpustate->ea1.d ), *cpustate->p2_reg16 ) );
}


static void _XORWRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = xor16( cpustate, *cpustate->p1_reg16, cpustate->imm2.w.l );
}


static void _XORWRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = xor16( cpustate, *cpustate->p1_reg16, RDMEMW( cpustate->ea2.d ) );
}


static void _XORWRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg16 = xor16( cpustate, *cpustate->p1_reg16, *cpustate->p2_reg16 );
}


static void _XORLMR(tlcs900_state *cpustate)
{
	WRMEML( cpustate->ea1.d, xor32( cpustate, RDMEML( cpustate->ea1.d ), *cpustate->p2_reg32 ) );
}


static void _XORLRI(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = xor32( cpustate, *cpustate->p1_reg32, cpustate->imm2.d );
}


static void _XORLRM(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = xor32( cpustate, *cpustate->p1_reg32, RDMEML( cpustate->ea2.d ) );
}


static void _XORLRR(tlcs900_state *cpustate)
{
	*cpustate->p1_reg32 = xor32( cpustate, *cpustate->p1_reg32, *cpustate->p2_reg32 );
}


static void _XORCFBIM(tlcs900_state *cpustate)
{
	xorcf8( cpustate, cpustate->imm1.b.l, RDMEM( cpustate->ea2.d ) );
}


static void _XORCFBIR(tlcs900_state *cpustate)
{
	xorcf8( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg8 );
}


static void _XORCFBRM(tlcs900_state *cpustate)
{
	xorcf8( cpustate, *cpustate->p1_reg8, RDMEM( cpustate->ea2.d ) );
}


static void _XORCFBRR(tlcs900_state *cpustate)
{
	xorcf8( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg8 );
}


static void _XORCFWIR(tlcs900_state *cpustate)
{
	xorcf16( cpustate, cpustate->imm1.b.l, *cpustate->p2_reg16 );
}


static void _XORCFWRR(tlcs900_state *cpustate)
{
	xorcf16( cpustate, *cpustate->p1_reg8, *cpustate->p2_reg16 );
}


static void _ZCF(tlcs900_state *cpustate)
{
	cpustate->sr.b.l &= ~ ( FLAG_NF | FLAG_CF );
	cpustate->sr.b.l |= ( ( cpustate->sr.b.l & FLAG_ZF ) ? 0 : FLAG_CF );
}


struct tlcs900inst
{
	void (*opfunc)(tlcs900_state *cpustate);
	int     operand1;
	int     operand2;
	int     cycles;
};


static void prepare_operands(tlcs900_state *cpustate, const tlcs900inst *inst)
{
	switch ( inst->operand1 )
	{
	case _A:
		cpustate->p1_reg8 = &cpustate->xwa[cpustate->regbank].b.l;
		break;
	case _F:
		cpustate->p1_reg8 = &cpustate->sr.b.l;
		break;
	case _SR:
		cpustate->p1_reg16 = &cpustate->sr.w.l;
		break;
	case _C8:
		cpustate->p1_reg8 = get_reg8_current( cpustate, cpustate->op );
		break;
	case _C16:
		cpustate->p1_reg16 = get_reg16_current( cpustate, cpustate->op );
		break;
	case _MC16: /* For MUL and DIV operations */
		cpustate->p1_reg16 = get_reg16_current( cpustate, ( cpustate->op >> 1 ) & 0x03 );
		break;
	case _C32:
		cpustate->p1_reg32 = get_reg32_current( cpustate, cpustate->op );
		break;
	case _CR8:
		cpustate->imm1.d = RDOP( cpustate );
		switch( cpustate->imm1.d )
		{
		case 0x22:
			cpustate->p1_reg8 = &cpustate->dmam[0].b.l;
			break;
		case 0x26:
			cpustate->p1_reg8 = &cpustate->dmam[1].b.l;
			break;
		case 0x2a:
			cpustate->p1_reg8 = &cpustate->dmam[2].b.l;
			break;
		case 0x2e:
			cpustate->p1_reg8 = &cpustate->dmam[3].b.l;
			break;
		default:
			cpustate->p1_reg8 = &cpustate->dummy.b.l;
			break;
		}
		break;
	case _CR16:
		cpustate->imm1.d = RDOP( cpustate );
		switch( cpustate->imm1.d )
		{
		case 0x20:
			cpustate->p1_reg16 = &cpustate->dmac[0].w.l;
			break;
		case 0x24:
			cpustate->p1_reg16 = &cpustate->dmac[1].w.l;
			break;
		case 0x28:
			cpustate->p1_reg16 = &cpustate->dmac[2].w.l;
			break;
		case 0x2c:
			cpustate->p1_reg16 = &cpustate->dmac[3].w.l;
			break;
		default:
			cpustate->p1_reg16 = &cpustate->dummy.w.l;
			break;
		}
		break;
	case _CR32:
		cpustate->imm1.d = RDOP( cpustate );
		switch( cpustate->imm1.d )
		{
		case 0x00:
			cpustate->p1_reg32 = &cpustate->dmas[0].d;
			break;
		case 0x04:
			cpustate->p1_reg32 = &cpustate->dmas[1].d;
			break;
		case 0x08:
			cpustate->p1_reg32 = &cpustate->dmas[2].d;
			break;
		case 0x0c:
			cpustate->p1_reg32 = &cpustate->dmas[3].d;
			break;
		case 0x10:
			cpustate->p1_reg32 = &cpustate->dmad[0].d;
			break;
		case 0x14:
			cpustate->p1_reg32 = &cpustate->dmad[1].d;
			break;
		case 0x18:
			cpustate->p1_reg32 = &cpustate->dmad[2].d;
			break;
		case 0x1c:
			cpustate->p1_reg32 = &cpustate->dmad[3].d;
			break;
		default:
			cpustate->p1_reg32 = &cpustate->dummy.d;
			break;
		}
		break;
	case _D8:
		cpustate->ea1.d = RDOP( cpustate );
		cpustate->ea1.d = cpustate->pc.d + cpustate->ea1.sb.l;
		break;
	case _D16:
		cpustate->ea1.d = RDOP( cpustate );
		cpustate->ea1.b.h = RDOP( cpustate );
		cpustate->ea1.d = cpustate->pc.d + cpustate->ea1.sw.l;
		break;
	case _I3:
		cpustate->imm1.d = cpustate->op & 0x07;
		break;
	case _I8:
		cpustate->imm1.d = RDOP( cpustate );
		break;
	case _I16:
		cpustate->imm1.d = RDOP( cpustate );
		cpustate->imm1.b.h = RDOP( cpustate );
		break;
	case _I24:
		cpustate->imm1.d = RDOP( cpustate );
		cpustate->imm1.b.h = RDOP( cpustate );
		cpustate->imm1.b.h2 = RDOP( cpustate );
		break;
	case _I32:
		cpustate->imm1.d = RDOP( cpustate );
		cpustate->imm1.b.h = RDOP( cpustate );
		cpustate->imm1.b.h2 = RDOP( cpustate );
		cpustate->imm1.b.h3 = RDOP( cpustate );
		break;
	case _M:
		cpustate->ea1.d = cpustate->ea2.d;
		break;
	case _M8:
		cpustate->ea1.d = RDOP( cpustate );
		break;
	case _M16:
		cpustate->ea1.d = RDOP( cpustate );
		cpustate->ea1.b.h = RDOP( cpustate );
		break;
	case _R:
		cpustate->p1_reg8 = cpustate->p2_reg8;
		cpustate->p1_reg16 = cpustate->p2_reg16;
		cpustate->p1_reg32 = cpustate->p2_reg32;
		break;
	}

	switch ( inst->operand2 )
	{
	case _A:
		cpustate->p2_reg8 = &cpustate->xwa[cpustate->regbank].b.l;
		break;
	case _F:        /* F' */
		cpustate->p2_reg8 = &cpustate->f2.b.l;
		break;
	case _SR:
		cpustate->p2_reg16 = &cpustate->sr.w.l;
		break;
	case _C8:
		cpustate->p2_reg8 = get_reg8_current( cpustate, cpustate->op );
		break;
	case _C16:
		cpustate->p2_reg16 = get_reg16_current( cpustate, cpustate->op );
		break;
	case _C32:
		cpustate->p2_reg32 = get_reg32_current( cpustate, cpustate->op );
		break;
	case _CR8:
		cpustate->imm1.d = RDOP( cpustate );
		switch( cpustate->imm1.d )
		{
		case 0x22:
			cpustate->p2_reg8 = &cpustate->dmam[0].b.l;
			break;
		case 0x26:
			cpustate->p2_reg8 = &cpustate->dmam[1].b.l;
			break;
		case 0x2a:
			cpustate->p2_reg8 = &cpustate->dmam[2].b.l;
			break;
		case 0x2e:
			cpustate->p2_reg8 = &cpustate->dmam[3].b.l;
			break;
		default:
			cpustate->p2_reg8 = &cpustate->dummy.b.l;
			break;
		}
		break;
	case _CR16:
		cpustate->imm1.d = RDOP( cpustate );
		switch( cpustate->imm1.d )
		{
		case 0x20:
			cpustate->p2_reg16 = &cpustate->dmac[0].w.l;
			break;
		case 0x24:
			cpustate->p2_reg16 = &cpustate->dmac[1].w.l;
			break;
		case 0x28:
			cpustate->p2_reg16 = &cpustate->dmac[2].w.l;
			break;
		case 0x2c:
			cpustate->p2_reg16 = &cpustate->dmac[3].w.l;
			break;
		default:
			cpustate->p2_reg16 = &cpustate->dummy.w.l;
			break;
		}
		break;
	case _CR32:
		cpustate->imm1.d = RDOP( cpustate );
		switch( cpustate->imm1.d )
		{
		case 0x00:
			cpustate->p2_reg32 = &cpustate->dmas[0].d;
			break;
		case 0x04:
			cpustate->p2_reg32 = &cpustate->dmas[1].d;
			break;
		case 0x08:
			cpustate->p2_reg32 = &cpustate->dmas[2].d;
			break;
		case 0x0c:
			cpustate->p2_reg32 = &cpustate->dmas[3].d;
			break;
		case 0x10:
			cpustate->p2_reg32 = &cpustate->dmad[0].d;
			break;
		case 0x14:
			cpustate->p2_reg32 = &cpustate->dmad[1].d;
			break;
		case 0x18:
			cpustate->p2_reg32 = &cpustate->dmad[2].d;
			break;
		case 0x1c:
			cpustate->p2_reg32 = &cpustate->dmad[3].d;
			break;
		default:
			cpustate->p2_reg32 = &cpustate->dummy.d;
			break;
		}
		break;
	case _D8:
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.d = cpustate->pc.d + cpustate->ea2.sb.l;
		break;
	case _D16:
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->ea2.d = cpustate->pc.d + cpustate->ea2.sw.l;
		break;
	case _I3:
		cpustate->imm2.d = cpustate->op & 0x07;
		break;
	case _I8:
		cpustate->imm2.d = RDOP( cpustate );
		break;
	case _I16:
		cpustate->imm2.d = RDOP( cpustate );
		cpustate->imm2.b.h = RDOP( cpustate );
		break;
	case _I32:
		cpustate->imm2.d = RDOP( cpustate );
		cpustate->imm2.b.h = RDOP( cpustate );
		cpustate->imm2.b.h2 = RDOP( cpustate );
		cpustate->imm2.b.h3 = RDOP( cpustate );
		break;
	case _M8:
		cpustate->ea2.d = RDOP( cpustate );
		break;
	case _M16:
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		break;
	}
}


static const tlcs900inst mnemonic_80[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _PUSHBM, _M, 0, 7 }, { _DB, 0, 0, 1 }, { _RLDRM, _A, _M, 12 }, { _RRDRM, _A, _M, 12 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDI, 0, 0, 10 }, { _LDIR, 0, 0, 10 }, { _LDD, 0, 0, 10 }, { _LDDR, 0, 0, 10 },
	{ _CPI, 0, 0, 8 }, { _CPIR, 0, 0, 10 }, { _CPD, 0, 0, 8 }, { _CPDR, 0, 0, 10 },
	{ _DB, 0, 0, 1 }, { _LDBMM, _M16, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 },
	{ _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 },
	{ _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 },
	{ _ADDBMI, _M, _I8, 7 }, { _ADCBMI, _M, _I8, 7 }, { _SUBBMI, _M, _I8, 7 }, { _SBCBMI, _M, _I8, 7 },
	{ _ANDBMI, _M, _I8, 7 }, { _XORBMI, _M, _I8, 7 }, { _ORBMI, _M, _I8, 7 }, { _CPBMI, _M, _I8, 6 },

	/* 40 - 5F */
	{ _MULBRM, _MC16, _M, 18}, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 },
	{ _MULBRM, _MC16, _M, 18}, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 },
	{ _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 },
	{ _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 },
	{ _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 },
	{ _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 },
	{ _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 },
	{ _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 },

	/* 60 - 7F */
	{ _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 },
	{ _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 },
	{ _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 },
	{ _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _RLCBM, _M, 0, 8 }, { _RRCBM, _M, 0, 8 }, { _RLBM, _M, 0, 8 }, { _RRBM, _M, 0, 8 },
	{ _SLABM, _M, 0, 8 }, { _SRABM, _M, 0, 8 }, { _SLLBM, _M, 0, 8 }, { _SRLBM, _M, 0, 8 },

	/* 80 - 9F */
	{ _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 },
	{ _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 },
	{ _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 },
	{ _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 },
	{ _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 },
	{ _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 },
	{ _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 },
	{ _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 },

	/* A0 - BF */
	{ _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 },
	{ _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 },
	{ _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 },
	{ _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 },
	{ _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 },
	{ _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 },
	{ _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 },
	{ _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 },

	/* C0 - DF */
	{ _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 },
	{ _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 },
	{ _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 },
	{ _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 },
	{ _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 },
	{ _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 },
	{ _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 },
	{ _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 },

	/* E0 - FF */
	{ _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 },
	{ _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 },
	{ _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 },
	{ _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 },
	{ _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 },
	{ _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 },
	{ _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 },
	{ _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 },
};


static const tlcs900inst mnemonic_88[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _PUSHBM, _M, 0, 7 }, { _DB, 0, 0, 1 }, { _RLDRM, _A, _M, 12 }, { _RRDRM, _A, _M, 12 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _LDBMM, _M16, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 },
	{ _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 },
	{ _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 },
	{ _ADDBMI, _M, _I8, 7 }, { _ADCBMI, _M, _I8, 7 }, { _SUBBMI, _M, _I8, 7 }, { _SBCBMI, _M, _I8, 7 },
	{ _ANDBMI, _M, _I8, 7 }, { _XORBMI, _M, _I8, 7 }, { _ORBMI, _M, _I8, 7 }, { _CPBMI, _M, _I8, 6 },

	/* 40 - 5F */
	{ _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 },
	{ _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 },
	{ _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 },
	{ _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 },
	{ _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 },
	{ _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 },
	{ _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 },
	{ _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 },

	/* 60 - 7F */
	{ _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 },
	{ _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 },
	{ _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 },
	{ _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _RLCBM, _M, 0, 8 }, { _RRCBM, _M, 0, 8 }, { _RLBM, _M, 0, 8 }, { _RRBM, _M, 0, 8 },
	{ _SLABM, _M, 0, 8 }, { _SRABM, _M, 0, 8 }, { _SLLBM, _M, 0, 8 }, { _SRLBM, _M, 0, 8 },

	/* 80 - 9F */
	{ _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 },
	{ _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 },
	{ _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 },
	{ _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 },
	{ _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 },
	{ _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 },
	{ _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 },
	{ _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 },

	/* A0 - BF */
	{ _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 },
	{ _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 },
	{ _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 },
	{ _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 },
	{ _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 },
	{ _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 },
	{ _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 },
	{ _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 },

	/* C0 - DF */
	{ _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 },
	{ _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 },
	{ _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 },
	{ _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 },
	{ _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 },
	{ _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 },
	{ _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 },
	{ _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 },

	/* E0 - FF */
	{ _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 },
	{ _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 },
	{ _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 },
	{ _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 },
	{ _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 },
	{ _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 },
	{ _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 },
	{ _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 },
};


static const tlcs900inst mnemonic_90[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _PUSHWM, _M, 0, 7 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDIW, 0, 0, 10 }, { _LDIRW, 0, 0, 10 }, { _LDDW, 0, 0, 10 }, { _LDDRW, 0, 0, 10 },
	{ _CPIW, 0, 0, 8 }, { _CPIRW, 0, 0, 10 }, { _CPDW, 0, 0, 8 }, { _CPDRW, 0, 0, 10 },
	{ _DB, 0, 0, 1 }, { _LDWMM, _M16, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 },
	{ _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 },
	{ _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 },
	{ _ADDWMI, _M, _I16, 8 }, { _ADCWMI, _M, _I16, 8 }, { _SUBWMI, _M, _I16, 8 }, { _SBCWMI, _M, _I16, 8 },
	{ _ANDWMI, _M, _I16, 8 }, { _XORWMI, _M, _I16, 8 }, { _ORWMI, _M, _I16, 8 }, { _CPWMI, _M, _I16, 6 },

	/* 40 - 5F */
	{ _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 },
	{ _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 },
	{ _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 },
	{ _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 },
	{ _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 },
	{ _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 },
	{ _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 },
	{ _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 },

	/* 60 - 7F */
	{ _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 },
	{ _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 },
	{ _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 },
	{ _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _RLCWM, _M, 0, 8 }, { _RRCWM, _M, 0, 8 }, { _RLWM, _M, 0, 8 }, { _RRWM, _M, 0, 8 },
	{ _SLAWM, _M, 0, 8 }, { _SRAWM, _M, 0, 8 }, { _SLLWM, _M, 0, 8 }, { _SRLWM, _M, 0, 8 },

	/* 80 - 9F */
	{ _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 },
	{ _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 },
	{ _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 },
	{ _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 },
	{ _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 },
	{ _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 },
	{ _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 },
	{ _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 },

	/* A0 - BF */
	{ _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 },
	{ _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 },
	{ _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 },
	{ _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 },
	{ _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 },
	{ _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 },
	{ _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 },
	{ _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 },

	/* C0 - DF */
	{ _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 },
	{ _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 },
	{ _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 },
	{ _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 },
	{ _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 },
	{ _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 },
	{ _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 },
	{ _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 },

	/* E0 - FF */
	{ _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 },
	{ _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 },
	{ _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 },
	{ _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 },
	{ _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 },
	{ _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 },
	{ _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 },
	{ _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 },
};


static const tlcs900inst mnemonic_98[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _PUSHWM, _M, 0, 7 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _LDWMM, _M16, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 },
	{ _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 },
	{ _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 },
	{ _ADDWMI, _M, _I16, 8 }, { _ADCWMI, _M, _I16, 8 }, { _SUBWMI, _M, _I16, 8 }, { _SBCWMI, _M, _I16, 8 },
	{ _ANDWMI, _M, _I16, 8 }, { _XORWMI, _M, _I16, 8 }, { _ORWMI, _M, _I16, 8 }, { _CPWMI, _M, _I16, 6 },

	/* 40 - 5F */
	{ _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 },
	{ _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 },
	{ _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 },
	{ _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 },
	{ _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 },
	{ _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 },
	{ _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 },
	{ _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 },

	/* 60 - 7F */
	{ _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 },
	{ _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 },
	{ _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 },
	{ _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _RLCWM, _M, 0, 8 }, { _RRCWM, _M, 0, 8 }, { _RLWM, _M, 0, 8 }, { _RRWM, _M, 0, 8 },
	{ _SLAWM, _M, 0, 8 }, { _SRAWM, _M, 0, 8 }, { _SLLWM, _M, 0, 8 }, { _SRLWM, _M, 0, 8 },

	/* 80 - 9F */
	{ _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 },
	{ _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 },
	{ _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 },
	{ _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 },
	{ _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 },
	{ _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 },
	{ _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 },
	{ _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 },

	/* A0 - BF */
	{ _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 },
	{ _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 },
	{ _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 },
	{ _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 },
	{ _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 },
	{ _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 },
	{ _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 },
	{ _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 },

	/* C0 - DF */
	{ _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 },
	{ _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 },
	{ _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 },
	{ _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 },
	{ _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 },
	{ _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 },
	{ _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 },
	{ _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 },

	/* E0 - FF */
	{ _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 },
	{ _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 },
	{ _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 },
	{ _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 },
	{ _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 },
	{ _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 },
	{ _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 },
	{ _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 },
};


static const tlcs900inst mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 },
	{ _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 60 - 7F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 80 - 9F */
	{ _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 },
	{ _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 },
	{ _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 },
	{ _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 },
	{ _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 },
	{ _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 },
	{ _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 },
	{ _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 },

	/* A0 - BF */
	{ _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 },
	{ _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 },
	{ _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 },
	{ _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 },
	{ _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 },
	{ _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 },
	{ _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 },
	{ _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 },

	/* C0 - DF */
	{ _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 },
	{ _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 },
	{ _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 },
	{ _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 },
	{ _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 },
	{ _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 },
	{ _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 },
	{ _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 },

	/* E0 - FF */
	{ _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 },
	{ _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 },
	{ _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 },
	{ _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 },
	{ _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 },
	{ _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 },
	{ _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 },
	{ _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 },
};


static const tlcs900inst mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ _LDBMI, _M, _I8, 5 }, { _DB, 0, 0, 1 }, { _LDWMI, _M, _I16, 6 }, { _DB, 0, 0, 1 },
	{ _POPBM, _M, 0, 6 }, { _DB, 0, 0, 1 }, { _POPWM, _M, 0, 6 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDBMM, _M, _M16, 8 }, { _DB, 0, 0, 1 }, { _LDWMM, _M, _M16, 8 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 },
	{ _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 },
	{ _ANDCFBRM, _A, _M, 8 }, { _ORCFBRM, _A, _M, 8 }, { _XORCFBRM, _A, _M, 8 }, { _LDCFBRM, _A, _M, 8 },
	{ _STCFBRM, _A, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 },
	{ _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 },
	{ _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 },
	{ _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 60 - 7F */
	{ _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 },
	{ _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 80 - 9F */
	{ _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 },
	{ _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 },
	{ _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 },
	{ _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 },
	{ _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 },
	{ _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 },
	{ _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 },
	{ _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 },

	/* A0 - BF */
	{ _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 },
	{ _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 },
	{ _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 },
	{ _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 },
	{ _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 },
	{ _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 },
	{ _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 },
	{ _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 },

	/* C0 - DF */
	{ _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 },
	{ _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 },
	{ _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 },
	{ _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },

	/* E0 - FF */
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 },
	{ _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 },
	{ _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 },
	{ _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }, { _RETCC, _CC, 0, 6 }
};


static const tlcs900inst mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ _LDBMI, _M, _I8, 5 }, { _DB, 0, 0, 1 }, { _LDWMI, _M, _I16, 6 }, { _DB, 0, 0, 1 },
	{ _POPBM, _M, 0, 6 }, { _DB, 0, 0, 1 }, { _POPWM, _M, 0, 6 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDBMM, _M, _M16, 8 }, { _DB, 0, 0, 1 }, { _LDWMM, _M, _M16, 8 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 },
	{ _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 },
	{ _ANDCFBRM, _A, _M, 8 }, { _ORCFBRM, _A, _M, 8 }, { _XORCFBRM, _A, _M, 8 }, { _LDCFBRM, _A, _M, 8 },
	{ _STCFBRM, _A, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 },
	{ _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 },
	{ _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 },
	{ _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 60 - 7F */
	{ _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 },
	{ _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 80 - 9F */
	{ _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 },
	{ _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 }, { _ANDCFBIM, _I3, _M, 8 },
	{ _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 },
	{ _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 }, { _ORCFBIM, _I3, _M, 8 },
	{ _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 },
	{ _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 }, { _XORCFBIM, _I3, _M, 8 },
	{ _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 },
	{ _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 }, { _LDCFBIM, _I3, _M, 8 },

	/* A0 - BF */
	{ _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 },
	{ _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 }, { _STCFBIM, _I3, _M, 8 },
	{ _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 },
	{ _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 },
	{ _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 },
	{ _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 },
	{ _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 },
	{ _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 },

	/* C0 - DF */
	{ _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 },
	{ _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 },
	{ _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 },
	{ _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 }, { _BITBIM, _I3, _M, 8 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },

	/* E0 - FF */
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }
};


static const tlcs900inst mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _PUSHBM, _M, 0, 7 }, { _DB, 0, 0, 1 }, { _RLDRM, _A, _M, 12 }, { _RRDRM, _A, _M, 12 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _LDBMM, _M16, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 },
	{ _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 }, { _LDBRM, _C8, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 },
	{ _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 }, { _EXBMR, _M, _C8, 6 },
	{ _ADDBMI, _M, _I8, 7 }, { _ADCBMI, _M, _I8, 7 }, { _SUBBMI, _M, _I8, 7 }, { _SBCBMI, _M, _I8, 7 },
	{ _ANDBMI, _M, _I8, 7 }, { _XORBMI, _M, _I8, 7 }, { _ORBMI, _M, _I8, 7 }, { _CPBMI, _M, _I8, 6 },

	/* 40 - 5F */
	{ _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 },
	{ _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 }, { _MULBRM, _MC16, _M, 18 },
	{ _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 },
	{ _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 }, { _MULSBRM, _MC16, _M, 18 },
	{ _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 },
	{ _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 }, { _DIVBRM, _MC16, _M, 22 },
	{ _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 },
	{ _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 }, { _DIVSBRM, _MC16, _M, 24 },

	/* 60 - 7F */
	{ _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 },
	{ _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 }, { _INCBIM, _I3, _M, 6 },
	{ _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 },
	{ _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 }, { _DECBIM, _I3, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _RLCBM, _M, 0, 8 }, { _RRCBM, _M, 0, 8 }, { _RLBM, _M, 0, 8 }, { _RRBM, _M, 0, 8 },
	{ _SLABM, _M, 0, 8 }, { _SRABM, _M, 0, 8 }, { _SLLBM, _M, 0, 8 }, { _SRLBM, _M, 0, 8 },

	/* 80 - 9F */
	{ _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 },
	{ _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 }, { _ADDBRM, _C8, _M, 4 },
	{ _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 },
	{ _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 }, { _ADDBMR, _M, _C8, 6 },
	{ _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 },
	{ _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 }, { _ADCBRM, _C8, _M, 4 },
	{ _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 },
	{ _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 }, { _ADCBMR, _M, _C8, 6 },

	/* A0 - BF */
	{ _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 },
	{ _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 }, { _SUBBRM, _C8, _M, 4 },
	{ _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 },
	{ _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 }, { _SUBBMR, _M, _C8, 6 },
	{ _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 },
	{ _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 }, { _SBCBRM, _C8, _M, 4 },
	{ _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 },
	{ _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 }, { _SBCBMR, _M, _C8, 6 },

	/* C0 - DF */
	{ _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 },
	{ _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 }, { _ANDBRM, _C8, _M, 4 },
	{ _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 },
	{ _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 }, { _ANDBMR, _M, _C8, 6 },
	{ _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 },
	{ _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 }, { _XORBRM, _C8, _M, 4 },
	{ _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 },
	{ _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 }, { _XORBMR, _M, _C8, 6 },

	/* E0 - FF */
	{ _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 },
	{ _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 }, { _ORBRM, _C8, _M, 4 },
	{ _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 },
	{ _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 }, { _ORBMR, _M, _C8, 6 },
	{ _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 },
	{ _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 }, { _CPBRM, _C8, _M, 4 },
	{ _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 },
	{ _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 }, { _CPBMR, _M, _C8, 6 },
};


static const tlcs900inst mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _LDBRI, _R, _I8, 4 },
	{ _PUSHBR, _R, 0, 6 }, { _POPBR, _R, 0, 6 }, { _CPLBR, _R, 0, 4 }, { _NEGBR, _R, 0, 5 },
	{ _MULBRI, _R, _I8, 18}, { _MULSBRI, _R, _I8, 18 }, { _DIVBRI, _R, _I8, 22 }, { _DIVSBRI, _R, _I8, 24 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DAABR, _R, 0, 6 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DJNZB, _R, _D8, 7 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _ANDCFBIR, _I8, _R, 4 }, { _ORCFBIR, _I8, _R, 4 }, { _XORCFBIR, _I8, _R, 4 }, { _LDCFBIR, _I8, _R, 4 },
	{ _STCFBIR, _I8, _R, 4 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _ANDCFBRR, _A, _R, 4 }, { _ORCFBRR, _A, _R, 4 }, { _XORCFBRR, _A, _R, 4 }, { _LDCFBRR, _A, _R, 4 },
	{ _STCFBRR, _A, _R, 4 }, { _DB, 0, 0, 1 }, { _LDCBRR, _CR8, _R, 1 }, { _LDCBRR, _R, _CR8, 1 },
	{ _RESBIR, _I8, _R, 4 }, { _SETBIR, _I8, _R, 4 }, { _CHGBIR, _I8, _R, 4 }, { _BITBIR, _I8, _R, 4 },
	{ _TSETBIR, _I8, _R, 6 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _MULBRR, _MC16, _R, 18 }, { _MULBRR, _MC16, _R, 18 }, { _MULBRR, _MC16, _R, 18 }, { _MULBRR, _MC16, _R, 18 },
	{ _MULBRR, _MC16, _R, 18 }, { _MULBRR, _MC16, _R, 18 }, { _MULBRR, _MC16, _R, 18 }, { _MULBRR, _MC16, _R, 18 },
	{ _MULSBRR, _MC16, _R, 18 }, { _MULSBRR, _MC16, _R, 18 }, { _MULSBRR, _MC16, _R, 18 }, { _MULSBRR, _MC16, _R, 18 },
	{ _MULSBRR, _MC16, _R, 18 }, { _MULSBRR, _MC16, _R, 18 }, { _MULSBRR, _MC16, _R, 18 }, { _MULSBRR, _MC16, _R, 18 },
	{ _DIVBRR, _MC16, _R, 22 }, { _DIVBRR, _MC16, _R, 22 }, { _DIVBRR, _MC16, _R, 22 }, { _DIVBRR, _MC16, _R, 22 },
	{ _DIVBRR, _MC16, _R, 22 }, { _DIVBRR, _MC16, _R, 22 }, { _DIVBRR, _MC16, _R, 22 }, { _DIVBRR, _MC16, _R, 22 },
	{ _DIVSBRR, _MC16, _R, 24 }, { _DIVSBRR, _MC16, _R, 24 }, { _DIVSBRR, _MC16, _R, 24 }, { _DIVSBRR, _MC16, _R, 24 },
	{ _DIVSBRR, _MC16, _R, 24 }, { _DIVSBRR, _MC16, _R, 24 }, { _DIVSBRR, _MC16, _R, 24 }, { _DIVSBRR, _MC16, _R, 24 },

	/* 60 - 7F */
	{ _INCBIR, _I3, _R, 4 }, { _INCBIR, _I3, _R, 4 }, { _INCBIR, _I3, _R, 4 }, { _INCBIR, _I3, _R, 4 },
	{ _INCBIR, _I3, _R, 4 }, { _INCBIR, _I3, _R, 4 }, { _INCBIR, _I3, _R, 4 }, { _INCBIR, _I3, _R, 4 },
	{ _DECBIR, _I3, _R, 4 }, { _DECBIR, _I3, _R, 4 }, { _DECBIR, _I3, _R, 4 }, { _DECBIR, _I3, _R, 4 },
	{ _DECBIR, _I3, _R, 4 }, { _DECBIR, _I3, _R, 4 }, { _DECBIR, _I3, _R, 4 }, { _DECBIR, _I3, _R, 4 },
	{ _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 },
	{ _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 },
	{ _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 },
	{ _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 }, { _SCCBR, _CC, _R, 6 },

	/* 80 - 9F */
	{ _ADDBRR, _C8, _R, 4 }, { _ADDBRR, _C8, _R, 4 }, { _ADDBRR, _C8, _R, 4 }, { _ADDBRR, _C8, _R, 4 },
	{ _ADDBRR, _C8, _R, 4 }, { _ADDBRR, _C8, _R, 4 }, { _ADDBRR, _C8, _R, 4 }, { _ADDBRR, _C8, _R, 4 },
	{ _LDBRR, _C8, _R, 4 }, { _LDBRR, _C8, _R, 4 }, { _LDBRR, _C8, _R, 4 }, { _LDBRR, _C8, _R, 4 },
	{ _LDBRR, _C8, _R, 4 }, { _LDBRR, _C8, _R, 4 }, { _LDBRR, _C8, _R, 4 }, { _LDBRR, _C8, _R, 4 },
	{ _ADCBRR, _C8, _R, 4 }, { _ADCBRR, _C8, _R, 4 }, { _ADCBRR, _C8, _R, 4 }, { _ADCBRR, _C8, _R, 4 },
	{ _ADCBRR, _C8, _R, 4 }, { _ADCBRR, _C8, _R, 4 }, { _ADCBRR, _C8, _R, 4 }, { _ADCBRR, _C8, _R, 4 },
	{ _LDBRR, _R, _C8, 4 }, { _LDBRR, _R, _C8, 4 }, { _LDBRR, _R, _C8, 4 }, { _LDBRR, _R, _C8, 4 },
	{ _LDBRR, _R, _C8, 4 }, { _LDBRR, _R, _C8, 4 }, { _LDBRR, _R, _C8, 4 }, { _LDBRR, _R, _C8, 4 },

	/* A0 - BF */
	{ _SUBBRR, _C8, _R, 4 }, { _SUBBRR, _C8, _R, 4 }, { _SUBBRR, _C8, _R, 4 }, { _SUBBRR, _C8, _R, 4 },
	{ _SUBBRR, _C8, _R, 4 }, { _SUBBRR, _C8, _R, 4 }, { _SUBBRR, _C8, _R, 4 }, { _SUBBRR, _C8, _R, 4 },
	{ _LDBRI, _R, _I3, 4 }, { _LDBRI, _R, _I3, 4 }, { _LDBRI, _R, _I3, 4 }, { _LDBRI, _R, _I3, 4 },
	{ _LDBRI, _R, _I3, 4 }, { _LDBRI, _R, _I3, 4 }, { _LDBRI, _R, _I3, 4 }, { _LDBRI, _R, _I3, 4 },
	{ _SBCBRR, _C8, _R, 4 }, { _SBCBRR, _C8, _R, 4 }, { _SBCBRR, _C8, _R, 4 }, { _SBCBRR, _C8, _R, 4 },
	{ _SBCBRR, _C8, _R, 4 }, { _SBCBRR, _C8, _R, 4 }, { _SBCBRR, _C8, _R, 4 }, { _SBCBRR, _C8, _R, 4 },
	{ _EXBRR, _C8, _R, 5 }, { _EXBRR, _C8, _R, 5 }, { _EXBRR, _C8, _R, 5 }, { _EXBRR, _C8, _R, 5 },
	{ _EXBRR, _C8, _R, 5 }, { _EXBRR, _C8, _R, 5 }, { _EXBRR, _C8, _R, 5 }, { _EXBRR, _C8, _R, 5 },

	/* C0 - DF */
	{ _ANDBRR, _C8, _R, 4 }, { _ANDBRR, _C8, _R, 4 }, { _ANDBRR, _C8, _R, 4 }, { _ANDBRR, _C8, _R, 4 },
	{ _ANDBRR, _C8, _R, 4 }, { _ANDBRR, _C8, _R, 4 }, { _ANDBRR, _C8, _R, 4 }, { _ANDBRR, _C8, _R, 4 },
	{ _ADDBRI, _R, _I8, 4 }, { _ADCBRI, _R, _I8, 4 }, { _SUBBRI, _R, _I8, 4 }, { _SBCBRI, _R, _I8, 4 },
	{ _ANDBRI, _R, _I8, 4 }, { _XORBRI, _R, _I8, 4 }, { _ORBRI, _R, _I8, 4 }, { _CPBRI, _R, _I8, 4 },
	{ _XORBRR, _C8, _R, 4 }, { _XORBRR, _C8, _R, 4 }, { _XORBRR, _C8, _R, 4 }, { _XORBRR, _C8, _R, 4 },
	{ _XORBRR, _C8, _R, 4 }, { _XORBRR, _C8, _R, 4 }, { _XORBRR, _C8, _R, 4 }, { _XORBRR, _C8, _R, 4 },
	{ _CPBRI, _R, _I3, 4 }, { _CPBRI, _R, _I3, 4 }, { _CPBRI, _R, _I3, 4 }, { _CPBRI, _R, _I3, 4 },
	{ _CPBRI, _R, _I3, 4 }, { _CPBRI, _R, _I3, 4 }, { _CPBRI, _R, _I3, 4 }, { _CPBRI, _R, _I3, 4 },

	/* E0 - FF */
	{ _ORBRR, _C8, _R, 4 }, { _ORBRR, _C8, _R, 4 }, { _ORBRR, _C8, _R, 4 }, { _ORBRR, _C8, _R, 4 },
	{ _ORBRR, _C8, _R, 4 }, { _ORBRR, _C8, _R, 4 }, { _ORBRR, _C8, _R, 4 }, { _ORBRR, _C8, _R, 4 },
	{ _RLCBIR, _I8, _R, 6 }, { _RRCBIR, _I8, _R, 6 }, { _RLBIR, _I8, _R, 6 }, { _RRBIR, _I8, _R, 6 },
	{ _SLABIR, _I8, _R, 6 }, { _SRABIR, _I8, _R, 6 }, { _SLLBIR, _I8, _R, 6 }, { _SRLBIR, _I8, _R, 6 },
	{ _CPBRR, _C8, _R, 4 }, { _CPBRR, _C8, _R, 4 }, { _CPBRR, _C8, _R, 4 }, { _CPBRR, _C8, _R, 4 },
	{ _CPBRR, _C8, _R, 4 }, { _CPBRR, _C8, _R, 4 }, { _CPBRR, _C8, _R, 4 }, { _CPBRR, _C8, _R, 4 },
	{ _RLCBRR, _A, _R, 6 }, { _RRCBRR, _A, _R, 6 }, { _RLBRR, _A, _R, 6 }, { _RRBRR, _A, _R, 6 },
	{ _SLABRR, _A, _R, 6 }, { _SRABRR, _A, _R, 6 }, { _SLLBRR, _A, _R, 6 }, { _SRLBRR, _A, _R, 6 }
};


static const tlcs900inst mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _PUSHWM, _M, 0, 7 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _LDWMM, _M16, _M, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 },
	{ _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 }, { _LDWRM, _C16, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 },
	{ _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 }, { _EXWMR, _M, _C16, 6 },
	{ _ADDWMI, _M, _I16, 8 }, { _ADCWMI, _M, _I16, 8 }, { _SUBWMI, _M, _I16, 8 }, { _SBCWMI, _M, _I16, 8 },
	{ _ANDWMI, _M, _I16, 8 }, { _XORWMI, _M, _I16, 8 }, { _ORWMI, _M, _I16, 8 }, { _CPWMI, _M, _I16, 6 },

	/* 40 - 5F */
	{ _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 },
	{ _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 }, { _MULWRM, _C32, _M, 26 },
	{ _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 },
	{ _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 }, { _MULSWRM, _C32, _M, 26 },
	{ _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 },
	{ _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 }, { _DIVWRM, _C32, _M, 30 },
	{ _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 },
	{ _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 }, { _DIVSWRM, _C32, _M, 32 },

	/* 60 - 7F */
	{ _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 },
	{ _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 }, { _INCWIM, _I3, _M, 6 },
	{ _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 },
	{ _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 }, { _DECWIM, _I3, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _RLCWM, _M, 0, 8 }, { _RRCWM, _M, 0, 8 }, { _RLWM, _M, 0, 8 }, { _RRWM, _M, 0, 8 },
	{ _SLAWM, _M, 0, 8 }, { _SRAWM, _M, 0, 8 }, { _SLLWM, _M, 0, 8 }, { _SRLWM, _M, 0, 8 },

	/* 80 - 9F */
	{ _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 },
	{ _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 }, { _ADDWRM, _C16, _M, 4 },
	{ _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 },
	{ _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 }, { _ADDWMR, _M, _C16, 6 },
	{ _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 },
	{ _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 }, { _ADCWRM, _C16, _M, 4 },
	{ _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 },
	{ _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 }, { _ADCWMR, _M, _C16, 6 },

	/* A0 - BF */
	{ _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 },
	{ _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 }, { _SUBWRM, _C16, _M, 4 },
	{ _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 },
	{ _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 }, { _SUBWMR, _M, _C16, 6 },
	{ _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 },
	{ _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 }, { _SBCWRM, _C16, _M, 4 },
	{ _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 },
	{ _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 }, { _SBCWMR, _M, _C16, 6 },

	/* C0 - DF */
	{ _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 },
	{ _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 }, { _ANDWRM, _C16, _M, 4 },
	{ _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 },
	{ _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 }, { _ANDWMR, _M, _C16, 6 },
	{ _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 },
	{ _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 }, { _XORWRM, _C16, _M, 4 },
	{ _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 },
	{ _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 }, { _XORWMR, _M, _C16, 6 },

	/* E0 - FF */
	{ _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 },
	{ _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 }, { _ORWRM, _C16, _M, 4 },
	{ _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 },
	{ _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 }, { _ORWMR, _M, _C16, 6 },
	{ _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 },
	{ _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 }, { _CPWRM, _C16, _M, 4 },
	{ _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 },
	{ _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 }, { _CPWMR, _M, _C16, 6 },
};


static const tlcs900inst mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _LDWRI, _R, _I16, 4 },
	{ _PUSHWR, _R, 0, 5 }, { _POPWR, _R, 0, 6 }, { _CPLWR, _R, 0, 4 }, { _NEGWR, _R, 0, 5 },
	{ _MULWRI, _R, _I16, 26 }, { _MULSWRI, _R, _I16, 26 }, { _DIVWRI, _R, _I16, 30 }, { _DIVSWRI, _R, _I16, 32 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _BS1FRR, _A, _R, 4 }, { _BS1BRR, _A, _R, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _EXTZWR, _R, 0, 4 }, { _EXTSWR, _R, 0, 5 },
	{ _PAAWR, _R, 0, 4 }, { _DB, 0, 0, 1 }, { _MIRRW, _R, 0, 4 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _MULAR, _R, 0, 31 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DJNZW, _R, _D8, 7 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _ANDCFWIR, _I8, _R, 4 }, { _ORCFWIR, _I8, _R, 4 }, { _XORCFWIR, _I8, _R, 4 }, { _LDCFWIR, _I8, _R, 4 },
	{ _STCFWIR, _I8, _R, 4 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _ANDCFWRR, _A, _R, 4 }, { _ORCFWRR, _A, _R, 4 }, { _XORCFWRR, _A, _R, 4 }, { _LDCFWRR, _A, _R, 4 },
	{ _STCFWRR, _A, _R, 4 }, { _DB, 0, 0, 1 }, { _LDCWRR, _CR16, _R, 1 }, { _LDCWRR, _R, _CR16, 1 },
	{ _RESWIR, _I8, _R, 4 }, { _SETWIR, _I8, _R, 4 }, { _CHGWIR, _I8, _R, 4 }, { _BITWIR, _I8, _R, 4 },
	{ _TSETWIR, _I8, _R, 6 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _MINC1, _I16, _R, 8 }, { _MINC2, _I16, _R, 8 }, { _MINC4, _I16, _R, 8 }, { _DB, 0, 0, 1 },
	{ _MDEC1, _I16, _R, 7 }, { _MDEC2, _I16, _R, 7 }, { _MDEC4, _I16, _R, 7 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _MULWRR, _C32, _R, 26 }, { _MULWRR, _C32, _R, 26 }, { _MULWRR, _C32, _R, 26 }, { _MULWRR, _C32, _R, 26 },
	{ _MULWRR, _C32, _R, 26 }, { _MULWRR, _C32, _R, 26 }, { _MULWRR, _C32, _R, 26 }, { _MULWRR, _C32, _R, 26 },
	{ _MULSWRR, _C32, _R, 26 }, { _MULSWRR, _C32, _R, 26 }, { _MULSWRR, _C32, _R, 26 }, { _MULSWRR, _C32, _R, 26 },
	{ _MULSWRR, _C32, _R, 26 }, { _MULSWRR, _C32, _R, 26 }, { _MULSWRR, _C32, _R, 26 }, { _MULSWRR, _C32, _R, 26 },
	{ _DIVWRR, _C32, _R, 30 }, { _DIVWRR, _C32, _R, 30 }, { _DIVWRR, _C32, _R, 30 }, { _DIVWRR, _C32, _R, 30 },
	{ _DIVWRR, _C32, _R, 30 }, { _DIVWRR, _C32, _R, 30 }, { _DIVWRR, _C32, _R, 30 }, { _DIVWRR, _C32, _R, 30 },
	{ _DIVSWRR, _C32, _R, 32 }, { _DIVSWRR, _C32, _R, 32 }, { _DIVSWRR, _C32, _R, 32 }, { _DIVSWRR, _C32, _R, 32 },
	{ _DIVSWRR, _C32, _R, 32 }, { _DIVSWRR, _C32, _R, 32 }, { _DIVSWRR, _C32, _R, 32 }, { _DIVSWRR, _C32, _R, 32 },

	/* 60 - 7F */
	{ _INCWIR, _I3, _R, 4 }, { _INCWIR, _I3, _R, 4 }, { _INCWIR, _I3, _R, 4 }, { _INCWIR, _I3, _R, 4 },
	{ _INCWIR, _I3, _R, 4 }, { _INCWIR, _I3, _R, 4 }, { _INCWIR, _I3, _R, 4 }, { _INCWIR, _I3, _R, 4 },
	{ _DECWIR, _I3, _R, 4 }, { _DECWIR, _I3, _R, 4 }, { _DECWIR, _I3, _R, 4 }, { _DECWIR, _I3, _R, 4 },
	{ _DECWIR, _I3, _R, 4 }, { _DECWIR, _I3, _R, 4 }, { _DECWIR, _I3, _R, 4 }, { _DECWIR, _I3, _R, 4 },
	{ _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 },
	{ _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 },
	{ _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 },
	{ _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 }, { _SCCWR, _CC, _R, 6 },

	/* 80 - 9F */
	{ _ADDWRR, _C16, _R, 4 }, { _ADDWRR, _C16, _R, 4 }, { _ADDWRR, _C16, _R, 4 }, { _ADDWRR, _C16, _R, 4 },
	{ _ADDWRR, _C16, _R, 4 }, { _ADDWRR, _C16, _R, 4 }, { _ADDWRR, _C16, _R, 4 }, { _ADDWRR, _C16, _R, 4 },
	{ _LDWRR, _C16, _R, 4 }, { _LDWRR, _C16, _R, 4 }, { _LDWRR, _C16, _R, 4 }, { _LDWRR, _C16, _R, 4 },
	{ _LDWRR, _C16, _R, 4 }, { _LDWRR, _C16, _R, 4 }, { _LDWRR, _C16, _R, 4 }, { _LDWRR, _C16, _R, 4 },
	{ _ADCWRR, _C16, _R, 4 }, { _ADCWRR, _C16, _R, 4 }, { _ADCWRR, _C16, _R, 4 }, { _ADCWRR, _C16, _R, 4 },
	{ _ADCWRR, _C16, _R, 4 }, { _ADCWRR, _C16, _R, 4 }, { _ADCWRR, _C16, _R, 4 }, { _ADCWRR, _C16, _R, 4 },
	{ _LDWRR, _R, _C16, 4 }, { _LDWRR, _R, _C16, 4 }, { _LDWRR, _R, _C16, 4 }, { _LDWRR, _R, _C16, 4 },
	{ _LDWRR, _R, _C16, 4 }, { _LDWRR, _R, _C16, 4 }, { _LDWRR, _R, _C16, 4 }, { _LDWRR, _R, _C16, 4 },

	/* A0 - BF */
	{ _SUBWRR, _C16, _R, 4 }, { _SUBWRR, _C16, _R, 4 }, { _SUBWRR, _C16, _R, 4 }, { _SUBWRR, _C16, _R, 4 },
	{ _SUBWRR, _C16, _R, 4 }, { _SUBWRR, _C16, _R, 4 }, { _SUBWRR, _C16, _R, 4 }, { _SUBWRR, _C16, _R, 4 },
	{ _LDWRI, _R, _I3, 4 }, { _LDWRI, _R, _I3, 4 }, { _LDWRI, _R, _I3, 4 }, { _LDWRI, _R, _I3, 4 },
	{ _LDWRI, _R, _I3, 4 }, { _LDWRI, _R, _I3, 4 }, { _LDWRI, _R, _I3, 4 }, { _LDWRI, _R, _I3, 4 },
	{ _SBCWRR, _C16, _R, 4 }, { _SBCWRR, _C16, _R, 4 }, { _SBCWRR, _C16, _R, 4 }, { _SBCWRR, _C16, _R, 4 },
	{ _SBCWRR, _C16, _R, 4 }, { _SBCWRR, _C16, _R, 4 }, { _SBCWRR, _C16, _R, 4 }, { _SBCWRR, _C16, _R, 4 },
	{ _EXWRR, _C16, _R, 5 }, { _EXWRR, _C16, _R, 5 }, { _EXWRR, _C16, _R, 5 }, { _EXWRR, _C16, _R, 5 },
	{ _EXWRR, _C16, _R, 5 }, { _EXWRR, _C16, _R, 5 }, { _EXWRR, _C16, _R, 5 }, { _EXWRR, _C16, _R, 5 },

	/* C0 - DF */
	{ _ANDWRR, _C16, _R, 4 }, { _ANDWRR, _C16, _R, 4 }, { _ANDWRR, _C16, _R, 4 }, { _ANDWRR, _C16, _R, 4 },
	{ _ANDWRR, _C16, _R, 4 }, { _ANDWRR, _C16, _R, 4 }, { _ANDWRR, _C16, _R, 4 }, { _ANDWRR, _C16, _R, 4 },
	{ _ADDWRI, _R, _I16, 4 }, { _ADCWRI, _R, _I16, 4 }, { _SUBWRI, _R, _I16, 4 }, { _SBCWRI, _R, _I16, 4 },
	{ _ANDWRI, _R, _I16, 4 }, { _XORWRI, _R, _I16, 4 }, { _ORWRI, _R, _I16, 4 }, { _CPWRI, _R, _I16, 4 },
	{ _XORWRR, _C16, _R, 4 }, { _XORWRR, _C16, _R, 4 }, { _XORWRR, _C16, _R, 4 }, { _XORWRR, _C16, _R, 4 },
	{ _XORWRR, _C16, _R, 4 }, { _XORWRR, _C16, _R, 4 }, { _XORWRR, _C16, _R, 4 }, { _XORWRR, _C16, _R, 4 },
	{ _CPWRI, _R, _I3, 4 }, { _CPWRI, _R, _I3, 4 }, { _CPWRI, _R, _I3, 4 }, { _CPWRI, _R, _I3, 4 },
	{ _CPWRI, _R, _I3, 4 }, { _CPWRI, _R, _I3, 4 }, { _CPWRI, _R, _I3, 4 }, { _CPWRI, _R, _I3, 4 },

	/* E0 - FF */
	{ _ORWRR, _C16, _R, 4 }, { _ORWRR, _C16, _R, 4 }, { _ORWRR, _C16, _R, 4 }, { _ORWRR, _C16, _R, 4 },
	{ _ORWRR, _C16, _R, 4 }, { _ORWRR, _C16, _R, 4 }, { _ORWRR, _C16, _R, 4 }, { _ORWRR, _C16, _R, 4 },
	{ _RLCWIR, _I8, _R, 6 }, { _RRCWIR, _I8, _R, 6 }, { _RLWIR, _I8, _R, 6 }, { _RRWIR, _I8, _R, 6 },
	{ _SLAWIR, _I8, _R, 6 }, { _SRAWIR, _I8, _R, 6 }, { _SLLWIR, _I8, _R, 6 }, { _SRLWIR, _I8, _R, 6 },
	{ _CPWRR, _C16, _R, 4 }, { _CPWRR, _C16, _R, 4 }, { _CPWRR, _C16, _R, 4 }, { _CPWRR, _C16, _R, 4 },
	{ _CPWRR, _C16, _R, 4 }, { _CPWRR, _C16, _R, 4 }, { _CPWRR, _C16, _R, 4 }, { _CPWRR, _C16, _R, 4 },
	{ _RLCWRR, _A, _R, 6 }, { _RRCWRR, _A, _R, 6 }, { _RLWRR, _A, _R, 6 }, { _RRWRR, _A, _R, 6 },
	{ _SLAWRR, _A, _R, 6 }, { _SRAWRR, _A, _R, 6 }, { _SLLWRR, _A, _R, 6 }, { _SRLWRR, _A, _R, 6 }
};


static const tlcs900inst mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 },
	{ _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 }, { _LDLRM, _C32, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 60 - 7F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 80 - 9F */
	{ _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 },
	{ _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 }, { _ADDLRM, _C32, _M, 6 },
	{ _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 },
	{ _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 }, { _ADDLMR, _M, _C32, 10 },
	{ _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 },
	{ _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 }, { _ADCLRM, _C32, _M, 6 },
	{ _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 },
	{ _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 }, { _ADCLMR, _M, _C32, 10 },

	/* A0 - BF */
	{ _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 },
	{ _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 }, { _SUBLRM, _C32, _M, 6 },
	{ _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 },
	{ _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 }, { _SUBLMR, _M, _C32, 10 },
	{ _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 },
	{ _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 }, { _SBCLRM, _C32, _M, 6 },
	{ _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 },
	{ _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 }, { _SBCLMR, _M, _C32, 10 },

	/* C0 - DF */
	{ _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 },
	{ _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 }, { _ANDLRM, _C32, _M, 6 },
	{ _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 },
	{ _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 }, { _ANDLMR, _M, _C32, 10 },
	{ _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 },
	{ _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 }, { _XORLRM, _C32, _M, 6 },
	{ _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 },
	{ _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 }, { _XORLMR, _M, _C32, 10 },

	/* E0 - FF */
	{ _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 },
	{ _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 }, { _ORLRM, _C32, _M, 6 },
	{ _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 },
	{ _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 }, { _ORLMR, _M, _C32, 10 },
	{ _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 },
	{ _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 }, { _CPLRM, _C32, _M, 6 },
	{ _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 },
	{ _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 }, { _CPLMR, _M, _C32, 6 },
};


static const tlcs900inst mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _LDLRI, _R, _I32, 6 },
	{ _PUSHLR, _R, 0, 7 }, { _POPLR, _R, 0, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LINK, _R, _I16, 10 }, { _UNLK, _R, 0, 8 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _EXTZLR, _R, 0, 4 }, { _EXTSLR, _R, 0, 5 },
	{ _PAALR, _R, 0, 4 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _LDCLRR, _CR32, _R, 1 }, { _LDCLRR, _R, _CR32, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 60 - 7F */
	{ _INCLIR, _I3, _R, 4 }, { _INCLIR, _I3, _R, 4 }, { _INCLIR, _I3, _R, 4 }, { _INCLIR, _I3, _R, 4 },
	{ _INCLIR, _I3, _R, 4 }, { _INCLIR, _I3, _R, 4 }, { _INCLIR, _I3, _R, 4 }, { _INCLIR, _I3, _R, 4 },
	{ _DECLIR, _I3, _R, 4 }, { _DECLIR, _I3, _R, 4 }, { _DECLIR, _I3, _R, 4 }, { _DECLIR, _I3, _R, 4 },
	{ _DECLIR, _I3, _R, 4 }, { _DECLIR, _I3, _R, 4 }, { _DECLIR, _I3, _R, 4 }, { _DECLIR, _I3, _R, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 80 - 9F */
	{ _ADDLRR, _C32, _R, 7 }, { _ADDLRR, _C32, _R, 7 }, { _ADDLRR, _C32, _R, 7 }, { _ADDLRR, _C32, _R, 7 },
	{ _ADDLRR, _C32, _R, 7 }, { _ADDLRR, _C32, _R, 7 }, { _ADDLRR, _C32, _R, 7 }, { _ADDLRR, _C32, _R, 7 },
	{ _LDLRR, _C32, _R, 4 }, { _LDLRR, _C32, _R, 4 }, { _LDLRR, _C32, _R, 4 }, { _LDLRR, _C32, _R, 4 },
	{ _LDLRR, _C32, _R, 4 }, { _LDLRR, _C32, _R, 4 }, { _LDLRR, _C32, _R, 4 }, { _LDLRR, _C32, _R, 4 },
	{ _ADCLRR, _C32, _R, 7 }, { _ADCLRR, _C32, _R, 7 }, { _ADCLRR, _C32, _R, 7 }, { _ADCLRR, _C32, _R, 7 },
	{ _ADCLRR, _C32, _R, 7 }, { _ADCLRR, _C32, _R, 7 }, { _ADCLRR, _C32, _R, 7 }, { _ADCLRR, _C32, _R, 7 },
	{ _LDLRR, _R, _C32, 4 }, { _LDLRR, _R, _C32, 4 }, { _LDLRR, _R, _C32, 4 }, { _LDLRR, _R, _C32, 4 },
	{ _LDLRR, _R, _C32, 4 }, { _LDLRR, _R, _C32, 4 }, { _LDLRR, _R, _C32, 4 }, { _LDLRR, _R, _C32, 4 },

	/* A0 - BF */
	{ _SUBLRR, _C32, _R, 7 }, { _SUBLRR, _C32, _R, 7 }, { _SUBLRR, _C32, _R, 7 }, { _SUBLRR, _C32, _R, 7 },
	{ _SUBLRR, _C32, _R, 7 }, { _SUBLRR, _C32, _R, 7 }, { _SUBLRR, _C32, _R, 7 }, { _SUBLRR, _C32, _R, 7 },
	{ _LDLRI, _R, _I3, 4 }, { _LDLRI, _R, _I3, 4 }, { _LDLRI, _R, _I3, 4 }, { _LDLRI, _R, _I3, 4 },
	{ _LDLRI, _R, _I3, 4 }, { _LDLRI, _R, _I3, 4 }, { _LDLRI, _R, _I3, 4 }, { _LDLRI, _R, _I3, 4 },
	{ _SBCLRR, _C32, _R, 7 }, { _SBCLRR, _C32, _R, 7 }, { _SBCLRR, _C32, _R, 7 }, { _SBCLRR, _C32, _R, 7 },
	{ _SBCLRR, _C32, _R, 7 }, { _SBCLRR, _C32, _R, 7 }, { _SBCLRR, _C32, _R, 7 }, { _SBCLRR, _C32, _R, 7 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* C0 - DF */
	{ _ANDLRR, _C32, _R, 7 }, { _ANDLRR, _C32, _R, 7 }, { _ANDLRR, _C32, _R, 7 }, { _ANDLRR, _C32, _R, 7 },
	{ _ANDLRR, _C32, _R, 7 }, { _ANDLRR, _C32, _R, 7 }, { _ANDLRR, _C32, _R, 7 }, { _ANDLRR, _C32, _R, 7 },
	{ _ADDLRI, _R, _I32, 7 }, { _ADCLRI, _R, _I32, 7 }, { _SUBLRI, _R, _I32, 7 }, { _SBCLRI, _R, _I32, 7 },
	{ _ANDLRI, _R, _I32, 7 }, { _XORLRI, _R, _I32, 7 }, { _ORLRI, _R, _I32, 7 }, { _CPLRI, _R, _I32, 7 },
	{ _XORLRR, _C32, _R, 7 }, { _XORLRR, _C32, _R, 7 }, { _XORLRR, _C32, _R, 7 }, { _XORLRR, _C32, _R, 7 },
	{ _XORLRR, _C32, _R, 7 }, { _XORLRR, _C32, _R, 7 }, { _XORLRR, _C32, _R, 7 }, { _XORLRR, _C32, _R, 7 },
	{ _CPLRI, _R, _I3, 6 }, { _CPLRI, _R, _I3, 6 }, { _CPLRI, _R, _I3, 6 }, { _CPLRI, _R, _I3, 6 },
	{ _CPLRI, _R, _I3, 6 }, { _CPLRI, _R, _I3, 6 }, { _CPLRI, _R, _I3, 6 }, { _CPLRI, _R, _I3, 6 },

	/* E0 - FF */
	{ _ORLRR, _C32, _R, 7 }, { _ORLRR, _C32, _R, 7 }, { _ORLRR, _C32, _R, 7 }, { _ORLRR, _C32, _R, 7 },
	{ _ORLRR, _C32, _R, 7 }, { _ORLRR, _C32, _R, 7 }, { _ORLRR, _C32, _R, 7 }, { _ORLRR, _C32, _R, 7 },
	{ _RLCLIR, _I8, _R, 8 }, { _RRCLIR, _I8, _R, 8 }, { _RLLIR, _I8, _R, 8 }, { _RRLIR, _I8, _R, 8 },
	{ _SLALIR, _I8, _R, 8 }, { _SRALIR, _I8, _R, 8 }, { _SLLLIR, _I8, _R, 8 }, { _SRLLIR, _I8, _R, 8 },
	{ _CPLRR, _C32, _R, 7 }, { _CPLRR, _C32, _R, 7 }, { _CPLRR, _C32, _R, 7 }, { _CPLRR, _C32, _R, 7 },
	{ _CPLRR, _C32, _R, 7 }, { _CPLRR, _C32, _R, 7 }, { _CPLRR, _C32, _R, 7 }, { _CPLRR, _C32, _R, 7 },
	{ _RLCLRR, _A, _R, 8 }, { _RRCLRR, _A, _R, 8 }, { _RLLRR, _A, _R, 8 }, { _RRLRR, _A, _R, 8 },
	{ _SLALRR, _A, _R, 8 }, { _SRALRR, _A, _R, 8 }, { _SLLLRR, _A, _R, 8 }, { _SRLLRR, _A, _R, 8 }
};


static const tlcs900inst mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ _LDBMI, _M, _I8, 5 }, { _DB, 0, 0, 1 }, { _LDWMI, _M, _I16, 6 }, { _DB, 0, 0, 1 },
	{ _POPBM, _M, 0, 6 }, { _DB, 0, 0, 1 }, { _POPWM, _M, 0, 6 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDBMM, _M, _M16, 8 }, { _DB, 0, 0, 1 }, { _LDWMM, _M, _M16, 8 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 },
	{ _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 }, { _LDAW, _C16, _M, 4 },
	{ _ANDCFBRM, _A, _M, 4 }, { _ORCFBRM, _A, _M, 4 }, { _XORCFBRM, _A, _M, 4 }, { _LDCFBRM, _A, _M, 4 },
	{ _STCFBRM, _A, _M, 4 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 },
	{ _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 }, { _LDAL, _C32, _M, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 40 - 5F */
	{ _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 },
	{ _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 }, { _LDBMR, _M, _C8, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 },
	{ _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 }, { _LDWMR, _M, _C16, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 60 - 7F */
	{ _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 },
	{ _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 }, { _LDLMR, _M, _C32, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },

	/* 80 - 9F */
	{ _ANDCFBIM, _I3, _M, 4 }, { _ANDCFBIM, _I3, _M, 4 }, { _ANDCFBIM, _I3, _M, 4 }, { _ANDCFBIM, _I3, _M, 4 },
	{ _ANDCFBIM, _I3, _M, 4 }, { _ANDCFBIM, _I3, _M, 4 }, { _ANDCFBIM, _I3, _M, 4 }, { _ANDCFBIM, _I3, _M, 4 },
	{ _ORCFBIM, _I3, _M, 4 }, { _ORCFBIM, _I3, _M, 4 }, { _ORCFBIM, _I3, _M, 4 }, { _ORCFBIM, _I3, _M, 4 },
	{ _ORCFBIM, _I3, _M, 4 }, { _ORCFBIM, _I3, _M, 4 }, { _ORCFBIM, _I3, _M, 4 }, { _ORCFBIM, _I3, _M, 4 },
	{ _XORCFBIM, _I3, _M, 4 }, { _XORCFBIM, _I3, _M, 4 }, { _XORCFBIM, _I3, _M, 4 }, { _XORCFBIM, _I3, _M, 4 },
	{ _XORCFBIM, _I3, _M, 4 }, { _XORCFBIM, _I3, _M, 4 }, { _XORCFBIM, _I3, _M, 4 }, { _XORCFBIM, _I3, _M, 4 },
	{ _LDCFBIM, _I3, _M, 4 }, { _LDCFBIM, _I3, _M, 4 }, { _LDCFBIM, _I3, _M, 4 }, { _LDCFBIM, _I3, _M, 4 },
	{ _LDCFBIM, _I3, _M, 4 }, { _LDCFBIM, _I3, _M, 4 }, { _LDCFBIM, _I3, _M, 4 }, { _LDCFBIM, _I3, _M, 4 },

	/* A0 - BF */
	{ _STCFBIM, _I3, _M, 4 }, { _STCFBIM, _I3, _M, 4 }, { _STCFBIM, _I3, _M, 4 }, { _STCFBIM, _I3, _M, 4 },
	{ _STCFBIM, _I3, _M, 4 }, { _STCFBIM, _I3, _M, 4 }, { _STCFBIM, _I3, _M, 4 }, { _STCFBIM, _I3, _M, 4 },
	{ _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 },
	{ _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 }, { _TSETBIM, _I3, _M, 10 },
	{ _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 },
	{ _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 }, { _RESBIM, _I3, _M, 8 },
	{ _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 },
	{ _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 }, { _SETBIM, _I3, _M, 8 },

	/* C0 - DF */
	{ _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 },
	{ _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 }, { _CHGBIM, _I3, _M, 8 },
	{ _BITBIM, _I3, _M, 4 }, { _BITBIM, _I3, _M, 4 }, { _BITBIM, _I3, _M, 4 }, { _BITBIM, _I3, _M, 4 },
	{ _BITBIM, _I3, _M, 4 }, { _BITBIM, _I3, _M, 4 }, { _BITBIM, _I3, _M, 4 }, { _BITBIM, _I3, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },
	{ _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 }, { _JPM, _CC, _M, 4 },

	/* E0 - FF */
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 }, { _CALLM, _CC, _M, 6 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }
};


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in byte operations */
static void _80(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	cpustate->p1_reg32 = get_reg32_current( cpustate, cpustate->op - 1 );
	cpustate->p2_reg32 = get_reg32_current( cpustate, cpustate->op );

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_80[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in byte operations */
static void _88(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	cpustate->p1_reg32 = get_reg32_current( cpustate, cpustate->op - 1 );
	cpustate->p2_reg32 = get_reg32_current( cpustate, cpustate->op );

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	cpustate->ea2.d += (INT8)cpustate->op;
	cpustate->cycles += 2;
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_80[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIXI/XIY/XIZ/XSP) used as source in word operations */
static void _90(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	cpustate->p1_reg32 = get_reg32_current( cpustate, cpustate->op - 1 );
	cpustate->p2_reg32 = get_reg32_current( cpustate, cpustate->op );

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_90[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in word operations */
static void _98(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	cpustate->ea2.d += (INT8)cpustate->op;
	cpustate->cycles += 2;
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_98[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in long word operations */
static void _A0(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_a0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in long word operations */
static void _A8(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	cpustate->ea2.d += (INT8)cpustate->op;
	cpustate->cycles += 2;
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_a0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as destination in operations */
static void _B0(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_b0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as destination in operations */
static void _B8(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	cpustate->ea2.d = *get_reg32_current( cpustate, cpustate->op );
	cpustate->op = RDOP( cpustate );
	cpustate->ea2.d += (INT8)cpustate->op;
	cpustate->cycles += 2;
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_b8[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* memory used as source in byte operations */
static void _C0(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( cpustate->op & 0x07 )
	{
	case 0x00:  /* (n) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x01:  /* (nn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->ea2.b.h2 = RDOP( cpustate );
		cpustate->cycles += 3;
		break;

	case 0x03:
		cpustate->op = RDOP( cpustate );
		switch ( cpustate->op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
			cpustate->cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			cpustate->ea2.b.l = RDOP( cpustate );
			cpustate->ea2.b.h = RDOP( cpustate );
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op ) + cpustate->ea2.sw.l;
			cpustate->cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( cpustate->op )
			{
			/* (xrr+r8) */
			case 0x03:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT8) *get_reg8( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT16) *get_reg16( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				cpustate->ea2.b.l = RDOP( cpustate );
				cpustate->ea2.b.h = RDOP( cpustate );
				cpustate->ea2.d = cpustate->pc.d + cpustate->ea2.sw.l;
				cpustate->cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		*reg -= ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->ea2.d = *reg;
		cpustate->cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		cpustate->ea2.d = *reg;
		*reg += ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->cycles += 3;
		break;
	}
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_c0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


static void oC8(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	if ( cpustate->op & 0x08 )
	{
		cpustate->p2_reg8 = get_reg8_current( cpustate, cpustate->op );
		/* For MUL and DIV operations */
		cpustate->p2_reg16 = get_reg16_current( cpustate, ( cpustate->op >> 1 ) & 0x03 );
	}
	else
	{
		cpustate->op = RDOP( cpustate );
		cpustate->p2_reg8 = get_reg8( cpustate, cpustate->op );
		/* For MUL and DIV operations */
		cpustate->p2_reg16 = get_reg16( cpustate, cpustate->op );
	}
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_c8[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* memory used as source in word operations */
static void _D0(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( cpustate->op & 0x07 )
	{
	case 0x00:  /* (n) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x01:  /* (nn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->ea2.b.h2 = RDOP( cpustate );
		cpustate->cycles += 3;
		break;

	case 0x03:
		cpustate->op = RDOP( cpustate );
		switch ( cpustate->op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
			cpustate->cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			cpustate->ea2.b.l = RDOP( cpustate );
			cpustate->ea2.b.h = RDOP( cpustate );
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op ) + cpustate->ea2.sw.l;
			cpustate->cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( cpustate->op )
			{
			/* (xrr+r8) */
			case 0x03:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT8) *get_reg8( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT16) *get_reg16( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				cpustate->ea2.b.l = RDOP( cpustate );
				cpustate->ea2.b.h = RDOP( cpustate );
				cpustate->ea2.d = cpustate->pc.d + cpustate->ea2.sw.l;
				cpustate->cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		*reg -= ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->ea2.d = *reg;
		cpustate->cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		cpustate->ea2.d = *reg;
		*reg += ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->cycles += 3;
		break;
	}
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_d0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


static void oD8(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	if ( cpustate->op & 0x08 )
	{
		cpustate->p2_reg16 = get_reg16_current( cpustate, cpustate->op );
		cpustate->p2_reg32 = get_reg32_current( cpustate, cpustate->op );
	}
	else
	{
		cpustate->op = RDOP( cpustate );
		cpustate->p2_reg16 = get_reg16( cpustate, cpustate->op );
		cpustate->p2_reg32 = get_reg32( cpustate, cpustate->op );
	}
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_d8[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* memory used as source in long word operations */
static void _E0(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( cpustate->op & 0x07 )
	{
	case 0x00:  /* (n) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x01:  /* (nn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->ea2.b.h2 = RDOP( cpustate );
		cpustate->cycles += 3;
		break;

	case 0x03:
		cpustate->op = RDOP( cpustate );
		switch ( cpustate->op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
			cpustate->cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			cpustate->ea2.b.l = RDOP( cpustate );
			cpustate->ea2.b.h = RDOP( cpustate );
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op ) + cpustate->ea2.sw.l;
			cpustate->cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( cpustate->op )
			{
			/* (xrr+r8) */
			case 0x03:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT8) *get_reg8( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT16) *get_reg16( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				cpustate->ea2.b.l = RDOP( cpustate );
				cpustate->ea2.b.h = RDOP( cpustate );
				cpustate->ea2.d = cpustate->pc.d + cpustate->ea2.sw.l;
				cpustate->cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		*reg -= ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->ea2.d = *reg;
		cpustate->cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		cpustate->ea2.d = *reg;
		*reg += ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->cycles += 3;
		break;
	}
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_e0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


static void _E8(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;

	if ( cpustate->op & 0x08 )
	{
		cpustate->p2_reg32 = get_reg32_current( cpustate, cpustate->op );
	}
	else
	{
		cpustate->op = RDOP( cpustate );
		cpustate->p2_reg32 = get_reg32( cpustate, cpustate->op );
	}
	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_e8[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


/* memory used as destination operations */
static void _F0(tlcs900_state *cpustate)
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( cpustate->op & 0x07 )
	{
	case 0x00:  /* (n) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x01:  /* (nn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		cpustate->ea2.d = RDOP( cpustate );
		cpustate->ea2.b.h = RDOP( cpustate );
		cpustate->ea2.b.h2 = RDOP( cpustate );
		cpustate->cycles += 3;
		break;

	case 0x03:
		cpustate->op = RDOP( cpustate );
		switch ( cpustate->op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
			cpustate->cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			cpustate->ea2.b.l = RDOP( cpustate );
			cpustate->ea2.b.h = RDOP( cpustate );
			cpustate->ea2.d = *get_reg32( cpustate, cpustate->op ) + cpustate->ea2.sw.l;
			cpustate->cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( cpustate->op )
			{
			/* (xrr+r8) */
			case 0x03:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT8) *get_reg8( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d = *get_reg32( cpustate, cpustate->op );
				cpustate->op = RDOP( cpustate );
				cpustate->ea2.d += (INT16) *get_reg16( cpustate, cpustate->op );
				cpustate->cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				cpustate->ea2.b.l = RDOP( cpustate );
				cpustate->ea2.b.h = RDOP( cpustate );
				cpustate->ea2.d = cpustate->pc.d + cpustate->ea2.sw.l;
				cpustate->cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		*reg -= ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->ea2.d = *reg;
		cpustate->cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		cpustate->op = RDOP( cpustate );
		reg = get_reg32( cpustate, cpustate->op );
		cpustate->ea2.d = *reg;
		*reg += ( 1 << ( cpustate->op & 0x03 ) );
		cpustate->cycles += 3;
		break;
	}

	cpustate->op = RDOP( cpustate );
	inst = &mnemonic_f0[cpustate->op];
	prepare_operands( cpustate, inst );
	inst->opfunc( cpustate );
	cpustate->cycles += inst->cycles;
}


static const tlcs900inst mnemonic[256] =
{
	/* 00 - 1F */
	{ _NOP, 0, 0, 1 }, { _NORMAL, 0, 0, 4 }, { _PUSHWR, _SR, 0, 4 }, { _POPWSR, _SR, 0, 6 },
	{ _MAX, 0, 0, 4 }, { _HALT, 0, 0, 8 }, { _EI, _I8, 0, 5 }, { _RETI, 0, 0, 12 },
	{ _LDBMI, _M8, _I8, 5 }, { _PUSHBI, _I8, 0, 4 }, { _LDWMI, _M8, _I16, 6 }, { _PUSHWI, _I16, 0, 5 },
	{ _INCF, 0, 0, 2 }, { _DECF, 0, 0, 2 }, { _RET, 0, 0, 9 }, { _RETD, _I16, 0, 9 },
	{ _RCF, 0, 0, 2 }, { _SCF, 0, 0, 2 }, { _CCF, 0, 0, 2 }, { _ZCF, 0, 0, 2 },
	{ _PUSHBR, _A, 0, 3 }, { _POPBR, _A, 0, 4 }, { _EXBRR, _F, _F, 2 }, { _LDF, _I8, 0, 2 },
	{ _PUSHBR, _F, 0, 3 }, { _POPBR, _F, 0, 4 }, { _JPI, _I16, 0, 7 }, { _JPI, _I24, 0, 7 },
	{ _CALLI, _I16, 0, 12 }, { _CALLI, _I24, 0, 12 }, { _CALR, _D16, 0, 12 }, { _DB, 0, 0, 1 },

	/* 20 - 3F */
	{ _LDBRI, _C8, _I8, 2 }, { _LDBRI, _C8, _I8, 2 }, { _LDBRI, _C8, _I8, 2 }, { _LDBRI, _C8, _I8, 2 },
	{ _LDBRI, _C8, _I8, 2 }, { _LDBRI, _C8, _I8, 2 }, { _LDBRI, _C8, _I8, 2 }, { _LDBRI, _C8, _I8, 2 },
	{ _PUSHWR, _C16, 0, 3 }, { _PUSHWR, _C16, 0, 3 }, { _PUSHWR, _C16, 0, 3 }, { _PUSHWR, _C16, 0, 3 },
	{ _PUSHWR, _C16, 0, 3 }, { _PUSHWR, _C16, 0, 3 }, { _PUSHWR, _C16, 0, 3 }, { _PUSHWR, _C16, 0, 3 },
	{ _LDWRI, _C16, _I16, 3 }, { _LDWRI, _C16, _I16, 3 }, { _LDWRI, _C16, _I16, 3 }, { _LDWRI, _C16, _I16, 3 },
	{ _LDWRI, _C16, _I16, 3 }, { _LDWRI, _C16, _I16, 3 }, { _LDWRI, _C16, _I16, 3 }, { _LDWRI, _C16, _I16, 3 },
	{ _PUSHLR, _C32, 0, 5 }, { _PUSHLR, _C32, 0, 5 }, { _PUSHLR, _C32, 0, 5 }, { _PUSHLR, _C32, 0, 5 },
	{ _PUSHLR, _C32, 0, 5 }, { _PUSHLR, _C32, 0, 5 }, { _PUSHLR, _C32, 0, 5 }, { _PUSHLR, _C32, 0, 5 },

	/* 40 - 5F */
	{ _LDLRI, _C32, _I32, 5 }, { _LDLRI, _C32, _I32, 5 }, { _LDLRI, _C32, _I32, 5 }, { _LDLRI, _C32, _I32, 5 },
	{ _LDLRI, _C32, _I32, 5 }, { _LDLRI, _C32, _I32, 5 }, { _LDLRI, _C32, _I32, 5 }, { _LDLRI, _C32, _I32, 5 },
	{ _POPWR, _C16, 0, 4 }, { _POPWR, _C16, 0, 4 }, { _POPWR, _C16, 0, 4 }, { _POPWR, _C16, 0, 4 },
	{ _POPWR, _C16, 0, 4 }, { _POPWR, _C16, 0, 4 }, { _POPWR, _C16, 0, 4 }, { _POPWR, _C16, 0, 4 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 }, { _DB, 0, 0, 1 },
	{ _POPLR, _C32, 0, 6 }, { _POPLR, _C32, 0, 6 }, { _POPLR, _C32, 0, 6 }, { _POPLR, _C32, 0, 6 },
	{ _POPLR, _C32, 0, 6 }, { _POPLR, _C32, 0, 6 }, { _POPLR, _C32, 0, 6 }, { _POPLR, _C32, 0, 6 },

	/* 60 - 7F */
	{ _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 },
	{ _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 },
	{ _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 },
	{ _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 }, { _JR, _CC, _D8, 4 },
	{ _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 },
	{ _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 },
	{ _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 },
	{ _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 }, { _JRL, _CC, _D16, 4 },

	/* 80 - 9F */
	{ _80, 0, 0, 0 }, { _80, 0, 0, 0 }, { _80, 0, 0, 0 }, { _80, 0, 0, 0 },
	{ _80, 0, 0, 0 }, { _80, 0, 0, 0 }, { _80, 0, 0, 0 }, { _80, 0, 0, 0 },
	{ _88, 0, 0, 0 }, { _88, 0, 0, 0 }, { _88, 0, 0, 0 }, { _88, 0, 0, 0 },
	{ _88, 0, 0, 0 }, { _88, 0, 0, 0 }, { _88, 0, 0, 0 }, { _88, 0, 0, 0 },
	{ _90, 0, 0, 0 }, { _90, 0, 0, 0 }, { _90, 0, 0, 0 }, { _90, 0, 0, 0 },
	{ _90, 0, 0, 0 }, { _90, 0, 0, 0 }, { _90, 0, 0, 0 }, { _90, 0, 0, 0 },
	{ _98, 0, 0, 0 }, { _98, 0, 0, 0 }, { _98, 0, 0, 0 }, { _98, 0, 0, 0 },
	{ _98, 0, 0, 0 }, { _98, 0, 0, 0 }, { _98, 0, 0, 0 }, { _98, 0, 0, 0 },

	/* A0 - BF */
	{ _A0, 0, 0, 0 }, { _A0, 0, 0, 0 }, { _A0, 0, 0, 0 }, { _A0, 0, 0, 0 },
	{ _A0, 0, 0, 0 }, { _A0, 0, 0, 0 }, { _A0, 0, 0, 0 }, { _A0, 0, 0, 0 },
	{ _A8, 0, 0, 0 }, { _A8, 0, 0, 0 }, { _A8, 0, 0, 0 }, { _A8, 0, 0, 0 },
	{ _A8, 0, 0, 0 }, { _A8, 0, 0, 0 }, { _A8, 0, 0, 0 }, { _A8, 0, 0, 0 },
	{ _B0, 0, 0, 0 }, { _B0, 0, 0, 0 }, { _B0, 0, 0, 0 }, { _B0, 0, 0, 0 },
	{ _B0, 0, 0, 0 }, { _B0, 0, 0, 0 }, { _B0, 0, 0, 0 }, { _B0, 0, 0, 0 },
	{ _B8, 0, 0, 0 }, { _B8, 0, 0, 0 }, { _B8, 0, 0, 0 }, { _B8, 0, 0, 0 },
	{ _B8, 0, 0, 0 }, { _B8, 0, 0, 0 }, { _B8, 0, 0, 0 }, { _B8, 0, 0, 0 },

	/* C0 - DF */
	{ _C0, 0, 0, 0 }, { _C0, 0, 0, 0 }, { _C0, 0, 0, 0 }, { _C0, 0, 0, 0 },
	{ _C0, 0, 0, 0 }, { _C0, 0, 0, 0 }, { _DB, 0, 0, 0 }, { oC8, 0, 0, 0 },
	{ oC8, 0, 0, 0 }, { oC8, 0, 0, 0 }, { oC8, 0, 0, 0 }, { oC8, 0, 0, 0 },
	{ oC8, 0, 0, 0 }, { oC8, 0, 0, 0 }, { oC8, 0, 0, 0 }, { oC8, 0, 0, 0 },
	{ _D0, 0, 0, 0 }, { _D0, 0, 0, 0 }, { _D0, 0, 0, 0 }, { _D0, 0, 0, 0 },
	{ _D0, 0, 0, 0 }, { _D0, 0, 0, 0 }, { _DB, 0, 0, 0 }, { oD8, 0, 0, 0 },
	{ oD8, 0, 0, 0 }, { oD8, 0, 0, 0 }, { oD8, 0, 0, 0 }, { oD8, 0, 0, 0 },
	{ oD8, 0, 0, 0 }, { oD8, 0, 0, 0 }, { oD8, 0, 0, 0 }, { oD8, 0, 0, 0 },

	/* E0 - FF */
	{ _E0, 0, 0, 0 }, { _E0, 0, 0, 0 }, { _E0, 0, 0, 0 }, { _E0, 0, 0, 0 },
	{ _E0, 0, 0, 0 }, { _E0, 0, 0, 0 }, { _DB, 0, 0, 0 }, { _E8, 0, 0, 0 },
	{ _E8, 0, 0, 0 }, { _E8, 0, 0, 0 }, { _E8, 0, 0, 0 }, { _E8, 0, 0, 0 },
	{ _E8, 0, 0, 0 }, { _E8, 0, 0, 0 }, { _E8, 0, 0, 0 }, { _E8, 0, 0, 0 },
	{ _F0, 0, 0, 0 }, { _F0, 0, 0, 0 }, { _F0, 0, 0, 0 }, { _F0, 0, 0, 0 },
	{ _F0, 0, 0, 0 }, { _F0, 0, 0, 0 }, { _DB, 0, 0, 0 }, { _LDX, 0, 0, 9 },
	{ _SWI, _I3, 0, 16 }, { _SWI, _I3, 0, 16 }, { _SWI, _I3, 0, 16 }, { _SWI, _I3, 0, 16 },
	{ _SWI, _I3, 0, 16 }, { _SWI, _I3, 0, 16 }, { _SWI, _I3, 0, 16 }, { _SWI, _I3, 0, 16 }
};
