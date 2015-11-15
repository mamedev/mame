// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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
	_SR         /* status register */
};


int tlcs900h_device::condition_true( UINT8 cond )
{
	switch ( cond & 0x0f )
	{
	/* F */
	case 0x00:
		return 0;

	/* LT */
	case 0x01:
		return ( ( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) );

	/* LE */
	case 0x02:
		return ( ( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) ||
			( m_sr.b.l & FLAG_ZF ) );

	/* ULE */
	case 0x03:
		return ( m_sr.b.l & ( FLAG_ZF | FLAG_CF ) );

	/* OV */
	case 0x04:
		return ( m_sr.b.l & FLAG_VF );

	/* MI */
	case 0x05:
		return ( m_sr.b.l & FLAG_SF );

	/* Z */
	case 0x06:
		return ( m_sr.b.l & FLAG_ZF );

	/* C */
	case 0x07:
		return ( m_sr.b.l & FLAG_CF );

	/* T */
	case 0x08:
		return 1;

	/* GE */
	case 0x09:
		return ! ( ( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) );

	/* GT */
	case 0x0A:
		return ! ( ( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_SF ) ||
			( ( m_sr.b.l & ( FLAG_SF | FLAG_VF ) ) == FLAG_VF ) ||
			( m_sr.b.l & FLAG_ZF ) );

	/* UGT */
	case 0x0B:
		return ! ( m_sr.b.l & ( FLAG_ZF | FLAG_CF ) );

	/* NOV */
	case 0x0C:
		return ! ( m_sr.b.l & FLAG_VF );

	/* PL */
	case 0x0D:
		return ! ( m_sr.b.l & FLAG_SF );

	/* NZ */
	case 0x0E:
		return ! ( m_sr.b.l & FLAG_ZF );

	/* NC */
	case 0x0F:
		return ! ( m_sr.b.l & FLAG_CF );
	}
	return 0;
}


UINT8* tlcs900h_device::get_reg8_current( UINT8 reg )
{
	switch( reg & 7 )
	{
	/* W */
	case 0:
		return &m_xwa[m_regbank].b.h;

	/* A */
	case 1:
		return &m_xwa[m_regbank].b.l;

	/* B */
	case 2:
		return &m_xbc[m_regbank].b.h;

	/* C */
	case 3:
		return &m_xbc[m_regbank].b.l;

	/* D */
	case 4:
		return &m_xde[m_regbank].b.h;

	/* E */
	case 5:
		return &m_xde[m_regbank].b.l;

	/* H */
	case 6:
		return &m_xhl[m_regbank].b.h;

	/* L */
	case 7:
		return &m_xhl[m_regbank].b.l;
	}
	/* keep compiler happy */
	return &m_dummy.b.l;
}


UINT16* tlcs900h_device::get_reg16_current( UINT8 reg )
{
	switch( reg & 7 )
	{
	/* WA */
	case 0:
		return &m_xwa[m_regbank].w.l;

	/* BC */
	case 1:
		return &m_xbc[m_regbank].w.l;

	/* DE */
	case 2:
		return &m_xde[m_regbank].w.l;

	/* HL */
	case 3:
		return &m_xhl[m_regbank].w.l;

	/* IX */
	case 4:
		return &m_xix.w.l;

	/* IY */
	case 5:
		return &m_xiy.w.l;

	/* IZ */
	case 6:
		return &m_xiz.w.l;

	/* SP */
	/* TODO: Use correct user/system SP */
	case 7:
		return &m_xssp.w.l;
	}
	/* keep compiler happy */
	return &m_dummy.w.l;
}


UINT32* tlcs900h_device::get_reg32_current( UINT8 reg )
{
	switch( reg & 7 )
	{
	/* XWA */
	case 0:
		return &m_xwa[m_regbank].d;

	/* XBC */
	case 1:
		return &m_xbc[m_regbank].d;

	/* XDE */
	case 2:
		return &m_xde[m_regbank].d;

	/* XHL */
	case 3:
		return &m_xhl[m_regbank].d;

	/* XIX */
	case 4:
		return &m_xix.d;

	/* XIY */
	case 5:
		return &m_xiy.d;

	/* XIZ */
	case 6:
		return &m_xiz.d;

	/* XSP */
	case 7:
		/* TODO: Add selector for user/system stack pointer */
		return &m_xssp.d;
	}
	/* keep compiler happy */
	return &m_dummy.d;
}


PAIR* tlcs900h_device::get_reg( UINT8 reg )
{
	UINT8   regbank;

	switch( reg & 0xf0 )
	{
	case 0x00: case 0x10: case 0x20: case 0x30: /* explicit register bank */
	case 0xd0:                                  /* "previous" register bank */
	case 0xe0:                                  /* current register bank */
		regbank = ( reg & 0xf0 ) >> 4;
		if ( regbank == 0x0d )
			regbank = ( m_regbank - 1 ) & 0x03;

		if ( regbank == 0x0e )
			regbank = m_regbank;

		switch ( reg & 0x0c )
		{
		case 0x00:  return &m_xwa[regbank];
		case 0x04:  return &m_xbc[regbank];
		case 0x08:  return &m_xde[regbank];
		case 0x0c:  return &m_xhl[regbank];
		}
		break;
	case 0xf0:  /* index registers and sp */
		switch ( reg & 0x0c )
		{
		case 0x00:  return &m_xix;
		case 0x04:  return &m_xiy;
		case 0x08:  return &m_xiz;
		/* TODO: Use correct SP */
		case 0x0c:  return &m_xssp;
		}
		break;
	}

	/* illegal/unknown register reference */
	logerror( "Access to unknown tlcs-900 cpu register %02x\n", reg );
	return &m_dummy;
}


UINT8* tlcs900h_device::get_reg8( UINT8 reg )
{
	PAIR    *r = get_reg( reg );

	switch ( reg & 0x03 )
	{
	case 0x00:      return &r->b.l;
	case 0x01:      return &r->b.h;
	case 0x02:      return &r->b.h2;
	case 0x03:      return &r->b.h3;
	}

	return &r->b.l;
}


UINT16* tlcs900h_device::get_reg16( UINT8 reg )
{
	PAIR    *r = get_reg( reg );

	return ( reg & 0x02 ) ? &r->w.h : &r->w.l;
}


UINT32* tlcs900h_device::get_reg32( UINT8 reg )
{
	PAIR    *r = get_reg( reg );

	return &r->d;
}



void tlcs900h_device::parity8( UINT8 a )
{
	int i, j;

	j = 0;
	for ( i = 0; i < 8; i++ )
	{
		if ( a & 1 ) j++;
		a >>= 1;
	}
	m_sr.b.l |= ( ( j & 1 ) ? 0 : FLAG_VF );
}


void tlcs900h_device::parity16( UINT16 a )
{
	int i, j;

	j = 0;
	for ( i = 0; i < 16; i++ )
	{
		if ( a & 1 ) j++;
		a >>= 1;
	}
	m_sr.b.l |= ( ( j & 1 ) ? 0 : FLAG_VF );
}


void tlcs900h_device::parity32( UINT32 a )
{
	int i, j;

	j = 0;
	for ( i = 0; i < 32; i++ )
	{
		if ( a & 1 ) j++;
		a >>= 1;
	}
	m_sr.b.l |= ( ( j & 1 ) ? 0 : FLAG_VF );
}


UINT8 tlcs900h_device::adc8( UINT8 a, UINT8 b)
{
	UINT8 cy = m_sr.b.l & FLAG_CF;
	UINT8 result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


UINT16 tlcs900h_device::adc16( UINT16 a, UINT16 b)
{
	UINT8 cy = m_sr.b.l & FLAG_CF;
	UINT16 result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


UINT32 tlcs900h_device::adc32( UINT32 a, UINT32 b)
{
	UINT8 cy = m_sr.b.l & FLAG_CF;
	UINT32 result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


UINT8 tlcs900h_device::add8( UINT8 a, UINT8 b)
{
	UINT8 result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


UINT16 tlcs900h_device::add16( UINT16 a, UINT16 b)
{
	UINT16 result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


UINT32 tlcs900h_device::add32( UINT32 a, UINT32 b)
{
	UINT32 result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


UINT8 tlcs900h_device::sbc8( UINT8 a, UINT8 b)
{
	UINT8 cy = m_sr.b.l & FLAG_CF;
	UINT8 result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


UINT16 tlcs900h_device::sbc16( UINT16 a, UINT16 b)
{
	UINT8 cy = m_sr.b.l & FLAG_CF;
	UINT16 result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFFFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


UINT32 tlcs900h_device::sbc32( UINT32 a, UINT32 b)
{
	UINT8 cy = m_sr.b.l & FLAG_CF;
	UINT32 result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFFFFFFFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


UINT8 tlcs900h_device::sub8( UINT8 a, UINT8 b)
{
	UINT8 result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


UINT16 tlcs900h_device::sub16( UINT16 a, UINT16 b)
{
	UINT16 result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


UINT32 tlcs900h_device::sub32( UINT32 a, UINT32 b)
{
	UINT32 result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


UINT8 tlcs900h_device::and8( UINT8 a, UINT8 b)
{
	UINT8 result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	parity8( result );

	return result;
}


UINT16 tlcs900h_device::and16( UINT16 a, UINT16 b)
{
	UINT16 result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	parity16( result );

	return result;
}


UINT32 tlcs900h_device::and32( UINT32 a, UINT32 b)
{
	UINT32 result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	return result;
}


UINT8 tlcs900h_device::or8( UINT8 a, UINT8 b)
{
	UINT8 result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity8( result );

	return result;
}


UINT16 tlcs900h_device::or16( UINT16 a, UINT16 b)
{
	UINT16 result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity16( result );

	return result;
}


UINT32 tlcs900h_device::or32( UINT32 a, UINT32 b)
{
	UINT32 result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	return result;
}


UINT8 tlcs900h_device::xor8( UINT8 a, UINT8 b)
{
	UINT8 result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity8( result );

	return result;
}


UINT16 tlcs900h_device::xor16( UINT16 a, UINT16 b)
{
	UINT16 result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity16( result );

	return result;
}


UINT32 tlcs900h_device::xor32( UINT32 a, UINT32 b)
{
	UINT32 result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	return result;
}


void tlcs900h_device::ldcf8( UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::ldcf16( UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::andcf8( UINT8 a, UINT8 b )
{
	if ( ( b & ( 1 << ( a & 0x07 ) ) ) && ( m_sr.b.l & FLAG_CF ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::andcf16( UINT8 a, UINT8 b )
{
	if ( ( b & ( 1 << ( a & 0x0f ) ) ) && ( m_sr.b.l & FLAG_CF ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::orcf8( UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		m_sr.b.l |= FLAG_CF;
}


void tlcs900h_device::orcf16( UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		m_sr.b.l |= FLAG_CF;
}


void tlcs900h_device::xorcf8( UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		m_sr.b.l ^= FLAG_CF;
}


void tlcs900h_device::xorcf16( UINT8 a, UINT8 b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		m_sr.b.l ^= FLAG_CF;
}


UINT8 tlcs900h_device::rl8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( a & 0x80 )
		{
			a = ( a << 1 ) | ( m_sr.b.l & FLAG_CF );
			m_sr.b.l |= FLAG_CF;
		}
		else
		{
			a = ( a << 1 ) | ( m_sr.b.l & FLAG_CF );
			m_sr.b.l &= ~ FLAG_CF;
		}
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::rl16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( a & 0x8000 )
		{
			a = ( a << 1 ) | ( m_sr.b.l & FLAG_CF );
			m_sr.b.l |= FLAG_CF;
		}
		else
		{
			a = ( a << 1 ) | ( m_sr.b.l & FLAG_CF );
			m_sr.b.l &= ~ FLAG_CF;
		}
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::rl32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( a & 0x80000000 )
		{
			a = ( a << 1 ) | ( m_sr.b.l & FLAG_CF );
			m_sr.b.l |= FLAG_CF;
		}
		else
		{
			a = ( a << 1 ) | ( m_sr.b.l & FLAG_CF );
			m_sr.b.l &= ~ FLAG_CF;
		}
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( a );

	return a;
}

UINT8 tlcs900h_device::rlc8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x80 ) ? 1 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::rlc16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x8000 ) ? 1 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::rlc32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x80000000 ) ? 1 : 0 );
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity32( a );

	return a;
}


UINT8 tlcs900h_device::rr8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( m_sr.b.l & FLAG_CF )
		{
			m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 ) | 0x80;
		}
		else
		{
			m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 );
		}
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::rr16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( m_sr.b.l & FLAG_CF )
		{
			m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 ) | 0x8000;
		}
		else
		{
			m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 );
		}
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::rr32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		if ( m_sr.b.l & FLAG_CF )
		{
			m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 ) | 0x80000000;
		}
		else
		{
			m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
			a = ( a >> 1 );
		}
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( a );

	return a;
}


UINT8 tlcs900h_device::rrc8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x01 ) ? 0x80 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( a & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::rrc16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x0001 ) ? 0x8000 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( ( a >> 8 ) & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::rrc32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x00000001 ) ? 0x80000000 : 0 );
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( ( a >> 24 ) & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity32( a );

	return a;
}


UINT8 tlcs900h_device::sla8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( ( a & 0x80 ) ? FLAG_CF : 0 );
		a = ( a << 1 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::sla16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( ( a & 0x8000 ) ? FLAG_CF : 0 );
		a = ( a << 1 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::sla32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( ( a & 0x80000000 ) ? FLAG_CF : 0 );
		a = ( a << 1 );
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( a );

	return a;
}


UINT8 tlcs900h_device::sra8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a & 0x80 ) | ( a >> 1 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::sra16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a & 0x8000 ) | ( a >> 1 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::sra32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a & 0x80000000 ) | ( a >> 1 );
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( a );

	return a;
}


UINT8 tlcs900h_device::srl8( UINT8 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a >> 1 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


UINT16 tlcs900h_device::srl16( UINT16 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a >> 1 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


UINT32 tlcs900h_device::srl32( UINT32 a, UINT8 s )
{
	UINT8 count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | ( a & FLAG_CF );
		a = ( a >> 1 );
		m_cycles += 2;
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF );
	m_sr.b.l |= ( ( a >> 24 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF );
	parity32( a );

	return a;
}


UINT16 tlcs900h_device::div8( UINT16 a, UINT8 b )
{
	ldiv_t  result;

	if ( !b )
	{
		m_sr.b.l |= FLAG_VF;
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
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xff ) | ( ( result.rem & 0xff ) << 8 );
}


UINT32 tlcs900h_device::div16( UINT32 a, UINT16 b )
{
	ldiv_t  result;

	if ( !b )
	{
		m_sr.b.l |= FLAG_VF;
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
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xffff ) | ( ( result.rem & 0xffff ) << 16 );
}


UINT16 tlcs900h_device::divs8( INT16 a, INT8 b )
{
	ldiv_t  result;

	if ( !b )
	{
		m_sr.b.l |= FLAG_VF;
		return ( a << 8 ) | ( ( a >> 8 ) ^ 0xff );
	}

	result = ldiv( a, b );

	if ( result.quot > 0xff )
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xff ) | ( ( result.rem & 0xff ) << 8 );
}


UINT32 tlcs900h_device::divs16( INT32 a, INT16 b )
{
	ldiv_t  result;

	if ( !b )
	{
		m_sr.b.l |= FLAG_VF;
		return ( a << 16 ) | ( ( a >> 16 ) ^ 0xffff );
	}

	result = ldiv( a, b );

	if ( result.quot > 0xffff )
		m_sr.b.l |= FLAG_VF;
	else
		m_sr.b.l &= ~ FLAG_VF;

	return ( result.quot & 0xffff ) | ( ( result.rem & 0xffff ) << 16 );
}


void tlcs900h_device::_ADCBMI()
{
	WRMEM( m_ea1.d, adc8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_ADCBMR()
{
	WRMEM( m_ea1.d, adc8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_ADCBRI()
{
	*m_p1_reg8 = adc8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_ADCBRM()
{
	*m_p1_reg8 = adc8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ADCBRR()
{
	*m_p1_reg8 = adc8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_ADCWMI()
{
	WRMEMW( m_ea1.d, adc16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_ADCWMR()
{
	WRMEMW( m_ea1.d, adc16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_ADCWRI()
{
	*m_p1_reg16 = adc16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_ADCWRM()
{
	*m_p1_reg16 = adc16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_ADCWRR()
{
	*m_p1_reg16 = adc16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_ADCLMR()
{
	WRMEML( m_ea1.d, adc32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_ADCLRI()
{
	*m_p1_reg32 = adc32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_ADCLRM()
{
	*m_p1_reg32 = adc32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_ADCLRR()
{
	*m_p1_reg32 = adc32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_ADDBMI()
{
	WRMEM( m_ea1.d, add8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_ADDBMR()
{
	WRMEM( m_ea1.d, add8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_ADDBRI()
{
	*m_p1_reg8 = add8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_ADDBRM()
{
	*m_p1_reg8 = add8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ADDBRR()
{
	*m_p1_reg8 = add8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_ADDWMI()
{
	WRMEMW( m_ea1.d, add16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_ADDWMR()
{
	WRMEMW( m_ea1.d, add16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_ADDWRI()
{
	*m_p1_reg16 = add16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_ADDWRM()
{
	*m_p1_reg16 = add16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_ADDWRR()
{
	*m_p1_reg16 = add16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_ADDLMR()
{
	WRMEML( m_ea1.d, add32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_ADDLRI()
{
	*m_p1_reg32 = add32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_ADDLRM()
{
	*m_p1_reg32 = add32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_ADDLRR()
{
	*m_p1_reg32 = add32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_ANDBMI()
{
	WRMEM( m_ea1.d, and8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_ANDBMR()
{
	WRMEM( m_ea1.d, and8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_ANDBRI()
{
	*m_p1_reg8 = and8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_ANDBRM()
{
	*m_p1_reg8 = and8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ANDBRR()
{
	*m_p1_reg8 = and8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_ANDWMI()
{
	WRMEMW( m_ea1.d, and16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_ANDWMR()
{
	WRMEMW( m_ea1.d, and16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_ANDWRI()
{
	*m_p1_reg16 = and16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_ANDWRM()
{
	*m_p1_reg16 = and16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_ANDWRR()
{
	*m_p1_reg16 = and16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_ANDLMR()
{
	WRMEML( m_ea1.d, and32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_ANDLRI()
{
	*m_p1_reg32 = and32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_ANDLRM()
{
	*m_p1_reg32 = and32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_ANDLRR()
{
	*m_p1_reg32 = and32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_ANDCFBIM()
{
	andcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ANDCFBIR()
{
	andcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::_ANDCFBRM()
{
	andcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ANDCFBRR()
{
	andcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_ANDCFWIR()
{
	andcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::_ANDCFWRR()
{
	andcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::_BITBIM()
{
	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( RDMEM( m_ea2.d ) & ( 1 << ( m_imm1.b.l & 0x07 ) ) )
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900h_device::_BITBIR()
{
	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( *m_p2_reg8 & ( 1 << ( m_imm1.b.l & 0x0f ) ) )
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900h_device::_BITWIR()
{
	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( *m_p2_reg16 & ( 1 << ( m_imm1.b.l & 0x0f ) ) )
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900h_device::_BS1BRR()
{
	UINT16  r = *m_p2_reg16;

	if ( r )
	{
		m_sr.b.l &= ~ FLAG_VF;
		*m_p1_reg8 = 15;
		while( r < 0x8000 )
		{
			r <<= 1;
			*m_p1_reg8 -= 1;
		}
	}
	else
		m_sr.b.l |= FLAG_VF;
}


void tlcs900h_device::_BS1FRR()
{
	UINT16  r = *m_p2_reg16;

	if ( r )
	{
		m_sr.b.l &= ~ FLAG_VF;
		*m_p1_reg8 = 0;
		while( ! ( r & 0x0001 ) )
		{
			r >>= 1;
			*m_p1_reg8 += 1;
		}
	}
	else
		m_sr.b.l |= FLAG_VF;
}


void tlcs900h_device::_CALLI()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, m_pc.d );
	m_pc.d = m_imm1.d;
	m_prefetch_clear = true;
}


void tlcs900h_device::_CALLM()
{
	if ( condition_true( m_op ) )
	{
		m_xssp.d -= 4;
		WRMEML( m_xssp.d, m_pc.d );
		m_pc.d = m_ea2.d;
		m_cycles += 6;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_CALR()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, m_pc.d );
	m_pc.d = m_ea1.d;
	m_prefetch_clear = true;
}


void tlcs900h_device::_CCF()
{
	m_sr.b.l &= ~ FLAG_NF;
	m_sr.b.l ^= FLAG_CF;
}


void tlcs900h_device::_CHGBIM()
{
	WRMEM( m_ea2.d, RDMEM( m_ea2.d ) ^ ( 1 << ( m_imm1.b.l & 0x07 ) ) );
}


void tlcs900h_device::_CHGBIR()
{
	*m_p2_reg8 ^= ( 1 << ( m_imm1.b.l & 0x07 ) );
}


void tlcs900h_device::_CHGWIR()
{
	*m_p2_reg16 ^= ( 1 << ( m_imm1.b.l & 0x0f ) );
}


void tlcs900h_device::_CPBMI()
{
	sub8( RDMEM( m_ea1.d ), m_imm2.b.l );
}


void tlcs900h_device::_CPBMR()
{
	sub8( RDMEM( m_ea1.d ), *m_p2_reg8 );
}


void tlcs900h_device::_CPBRI()
{
	sub8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_CPBRM()
{
	sub8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_CPBRR()
{
	sub8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_CPWMI()
{
	sub16( RDMEMW( m_ea1.d ), m_imm2.w.l );
}


void tlcs900h_device::_CPWMR()
{
	sub16( RDMEMW( m_ea1.d ), *m_p2_reg16 );
}


void tlcs900h_device::_CPWRI()
{
	sub16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_CPWRM()
{
	sub16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_CPWRR()
{
	sub16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_CPLMR()
{
	sub32( RDMEML( m_ea1.d ), *m_p2_reg32 );
}


void tlcs900h_device::_CPLRI()
{
	sub32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_CPLRM()
{
	sub32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_CPLRR()
{
	sub32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_CPD()
{
	UINT8   result = *get_reg8_current( 1 ) - RDMEM( *m_p2_reg32 );
	UINT16  *bc = get_reg16_current( 1 );

	*m_p2_reg32 -= 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::_CPDR()
{
	_CPD();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_CPDW()
{
	UINT16  result = *get_reg16_current( 0 ) - RDMEMW( *m_p2_reg32 );
	UINT16  *bc = get_reg16_current( 1 );

	*m_p2_reg32 -= 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::_CPDRW()
{
	_CPDW();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_CPI()
{
	UINT8   result = *get_reg8_current( 1 ) - RDMEM( *m_p2_reg32 );
	UINT16  *bc = get_reg16_current( 1 );

	*m_p2_reg32 += 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::_CPIR()
{
	_CPI();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_CPIW()
{
	UINT16  result = *get_reg16_current( 0 ) - RDMEMW( *m_p2_reg32 );
	UINT16  *bc = get_reg16_current( 1 );

	*m_p2_reg32 += 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::_CPIRW()
{
	_CPIW();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_CPLBR()
{
	*m_p1_reg8 = ~ *m_p1_reg8;
	m_sr.b.l |= FLAG_HF | FLAG_NF;
}


void tlcs900h_device::_CPLWR()
{
	*m_p1_reg16 = ~ *m_p1_reg16;
	m_sr.b.l |= FLAG_HF | FLAG_NF;
}


void tlcs900h_device::_DAABR()
{
	UINT8   oldval = *m_p1_reg8;
	UINT8   fixval = 0;
	UINT8   carry = 0;
	UINT8   high = *m_p1_reg8 & 0xf0;
	UINT8   low = *m_p1_reg8 & 0x0f;

	if ( m_sr.b.l & FLAG_CF )
	{
		if ( m_sr.b.l & FLAG_HF )
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
		if ( m_sr.b.l & FLAG_HF )
		{
			if ( *m_p1_reg8 < 0x9a )
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
	m_sr.b.l &= ~ ( FLAG_VF | FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_CF );
	if ( m_sr.b.l & FLAG_NF )
	{
		/* after SUB, SBC, or NEG operation */
		*m_p1_reg8 -= fixval;
		m_sr.b.l |= ( ( *m_p1_reg8 > oldval || carry ) ? FLAG_CF : 0 );
	}
	else
	{
		/* after ADD or ADC operation */
		*m_p1_reg8 += fixval;
		m_sr.b.l |= ( ( *m_p1_reg8 < oldval || carry ) ? FLAG_CF : 0 );
	}
	m_sr.b.l |= ( *m_p1_reg8 & FLAG_SF ) | ( *m_p1_reg8 ? 0 : FLAG_ZF ) |
		( ( ( oldval ^ fixval ) ^ *m_p1_reg8 ) & FLAG_HF );

	parity8( *m_p1_reg8 );
}


void tlcs900h_device::_DB()
{
	logerror("%08x: invalid or illegal instruction\n", m_pc.d );
}


void tlcs900h_device::_DECBIM()
{
	UINT8   cy = m_sr.b.l & FLAG_CF;

	WRMEM( m_ea2.d, sub8( RDMEM( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::_DECBIR()
{
	UINT8   cy = m_sr.b.l & FLAG_CF;

	*m_p2_reg8 = sub8( *m_p2_reg8, m_imm1.b.l ? m_imm1.b.l : 8 );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::_DECWIM()
{
	UINT8   cy = m_sr.b.l & FLAG_CF;

	WRMEMW( m_ea2.d, sub16( RDMEMW( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::_DECWIR()
{
	*m_p2_reg16 -= m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::_DECLIR()
{
	*m_p2_reg32 -= m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::_DECF()
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	m_sr.b.h = ( m_sr.b.h & 0xf8 ) | ( ( m_sr.b.h - 1 ) & 0x07 );
	m_regbank = m_sr.b.h & 0x03;
}


void tlcs900h_device::_DIVBRI()
{
	*m_p1_reg16 = div8( *m_p1_reg16, m_imm2.b.l );
}


void tlcs900h_device::_DIVBRM()
{
	*m_p1_reg16 = div8( *m_p1_reg16, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_DIVBRR()
{
	*m_p1_reg16 = div8( *m_p1_reg16, *m_p2_reg8 );
}


void tlcs900h_device::_DIVWRI()
{
	*m_p1_reg32 = div16( *m_p1_reg32, m_imm2.w.l );
}


void tlcs900h_device::_DIVWRM()
{
	*m_p1_reg32 = div16( *m_p1_reg32, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_DIVWRR()
{
	*m_p1_reg32 = div16( *m_p1_reg32, *m_p2_reg16 );
}


void tlcs900h_device::_DIVSBRI()
{
	*m_p1_reg16 = divs8( *m_p1_reg16, m_imm2.b.l );
}


void tlcs900h_device::_DIVSBRM()
{
	*m_p1_reg16 = divs8( *m_p1_reg16, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_DIVSBRR()
{
	*m_p1_reg16 = divs8( *m_p1_reg16, *m_p2_reg8 );
}


void tlcs900h_device::_DIVSWRI()
{
	*m_p1_reg32 = divs16( *m_p1_reg32, m_imm2.w.l );
}


void tlcs900h_device::_DIVSWRM()
{
	*m_p1_reg32 = divs16( *m_p1_reg32, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_DIVSWRR()
{
	*m_p1_reg32 = divs16( *m_p1_reg32, *m_p2_reg16 );
}


void tlcs900h_device::_DJNZB()
{
	*m_p1_reg8 -= 1;
	if ( *m_p1_reg8 )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_DJNZW()
{
	*m_p1_reg16 -= 1;
	if ( *m_p1_reg16 )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_EI()
{
	m_sr.b.h = ( m_sr.b.h & 0x8f ) | ( ( m_imm1.b.l & 0x07 ) << 4 );
	m_check_irqs = 1;
}


void tlcs900h_device::_EXBMR()
{
	UINT8   i = RDMEM( m_ea1.d );

	WRMEM( m_ea1.d, *m_p2_reg8 );
	*m_p2_reg8 = i;
}


void tlcs900h_device::_EXBRR()
{
	UINT8   i = *m_p2_reg8;

	*m_p2_reg8 = *m_p1_reg8;
	*m_p1_reg8 = i;
}


void tlcs900h_device::_EXWMR()
{
	UINT16  i = RDMEMW( m_ea1.d );

	WRMEMW( m_ea1.d, *m_p2_reg16 );
	*m_p2_reg16 = i;
}


void tlcs900h_device::_EXWRR()
{
	UINT16  i = *m_p2_reg16;

	*m_p2_reg16 = *m_p1_reg16;
	*m_p1_reg16 = i;
}


void tlcs900h_device::_EXTSWR()
{
	if ( *m_p1_reg16 & 0x0080 )
		*m_p1_reg16 |= 0xff00;
	else
		*m_p1_reg16 &= 0x00ff;
}


void tlcs900h_device::_EXTSLR()
{
	if ( *m_p1_reg32 & 0x00008000 )
		*m_p1_reg32 |= 0xffff0000;
	else
		*m_p1_reg32 &= 0x0000ffff;
}


void tlcs900h_device::_EXTZWR()
{
	*m_p1_reg16 &= 0x00ff;
}


void tlcs900h_device::_EXTZLR()
{
	*m_p1_reg32 &= 0x0000ffff;
}


void tlcs900h_device::_HALT()
{
	m_halted = 1;
}


void tlcs900h_device::_INCBIM()
{
	UINT8   cy = m_sr.b.l & FLAG_CF;

	WRMEM( m_ea2.d, add8( RDMEM( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::_INCBIR()
{
	UINT8   cy = m_sr.b.l & FLAG_CF;

	*m_p2_reg8 = add8( *m_p2_reg8, m_imm1.b.l ? m_imm1.b.l : 8 );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::_INCWIM()
{
	UINT8   cy = m_sr.b.l & FLAG_CF;

	WRMEMW( m_ea2.d, add16( RDMEMW( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::_INCWIR()
{
	*m_p2_reg16 += m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::_INCLIR()
{
	*m_p2_reg32 += m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::_INCF()
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	m_sr.b.h = ( m_sr.b.h & 0xf8 ) | ( ( m_sr.b.h + 1 ) & 0x07 );
	m_regbank = m_sr.b.h & 0x03;
}


void tlcs900h_device::_JPI()
{
	m_pc.d = m_imm1.d;
	m_prefetch_clear = true;
}


void tlcs900h_device::_JPM()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_JR()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_JRL()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_LDBMI()
{
	WRMEM( m_ea1.d, m_imm2.b.l );
}


void tlcs900h_device::_LDBMM()
{
	WRMEM( m_ea1.d, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_LDBMR()
{
	WRMEM( m_ea1.d, *m_p2_reg8 );
}


void tlcs900h_device::_LDBRI()
{
	*m_p1_reg8 = m_imm2.b.l;
}


void tlcs900h_device::_LDBRM()
{
	*m_p1_reg8 = RDMEM( m_ea2.d );
}


void tlcs900h_device::_LDBRR()
{
	*m_p1_reg8 = *m_p2_reg8;
}


void tlcs900h_device::_LDWMI()
{
	WRMEMW( m_ea1.d, m_imm2.w.l );
}


void tlcs900h_device::_LDWMM()
{
	WRMEMW( m_ea1.d, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_LDWMR()
{
	WRMEMW( m_ea1.d, *m_p2_reg16 );
}


void tlcs900h_device::_LDWRI()
{
	*m_p1_reg16 = m_imm2.w.l;
}


void tlcs900h_device::_LDWRM()
{
	*m_p1_reg16 = RDMEMW( m_ea2.d );
}


void tlcs900h_device::_LDWRR()
{
	*m_p1_reg16 = *m_p2_reg16;
}


void tlcs900h_device::_LDLRI()
{
	*m_p1_reg32 = m_imm2.d;
}


void tlcs900h_device::_LDLRM()
{
	*m_p1_reg32 = RDMEML( m_ea2.d );
}


void tlcs900h_device::_LDLRR()
{
	*m_p1_reg32 = *m_p2_reg32;
}


void tlcs900h_device::_LDLMR()
{
	WRMEML( m_ea1.d, *m_p2_reg32 );
}


void tlcs900h_device::_LDAW()
{
	*m_p1_reg16 = m_ea2.w.l;
}


void tlcs900h_device::_LDAL()
{
	*m_p1_reg32 = m_ea2.d;
}


void tlcs900h_device::_LDCBRR()
{
	*m_p1_reg8 = *m_p2_reg8;
}


void tlcs900h_device::_LDCWRR()
{
	*m_p1_reg16 = *m_p2_reg16;
}


void tlcs900h_device::_LDCLRR()
{
	*m_p1_reg32 = *m_p2_reg32;
}


void tlcs900h_device::_LDCFBIM()
{
	ldcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_LDCFBIR()
{
	ldcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::_LDCFBRM()
{
	ldcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_LDCFBRR()
{
	ldcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_LDCFWIR()
{
	ldcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::_LDCFWRR()
{
	ldcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::_LDD()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEM( *m_p1_reg32, RDMEM( *m_p2_reg32 ) );
	*m_p1_reg32 -= 1;
	*m_p2_reg32 -= 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
	}
}


void tlcs900h_device::_LDDR()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEM( *m_p1_reg32, RDMEM( *m_p2_reg32 ) );
	*m_p1_reg32 -= 1;
	*m_p2_reg32 -= 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_LDDRW()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEMW( *m_p1_reg32, RDMEMW( *m_p2_reg32 ) );
	*m_p1_reg32 -= 2;
	*m_p2_reg32 -= 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_LDDW()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEMW( *m_p1_reg32, RDMEMW( *m_p2_reg32 ) );
	*m_p1_reg32 -= 2;
	*m_p2_reg32 -= 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
	}
}


void tlcs900h_device::_LDF()
{
	m_sr.b.h = ( m_sr.b.h & 0xf8 ) | ( m_imm1.b.l & 0x07 );
	m_regbank = m_imm1.b.l & 0x03;
}


void tlcs900h_device::_LDI()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEM( *m_p1_reg32, RDMEM( *m_p2_reg32 ) );
	*m_p1_reg32 += 1;
	*m_p2_reg32 += 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
	}
}


void tlcs900h_device::_LDIR()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEM( *m_p1_reg32, RDMEM( *m_p2_reg32 ) );
	*m_p1_reg32 += 1;
	*m_p2_reg32 += 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_LDIRW()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEMW( *m_p1_reg32, RDMEMW( *m_p2_reg32 ) );
	*m_p1_reg32 += 2;
	*m_p2_reg32 += 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_LDIW()
{
	UINT16  *bc = get_reg16_current( 1 );

	WRMEMW( *m_p1_reg32, RDMEMW( *m_p2_reg32 ) );
	*m_p1_reg32 += 2;
	*m_p2_reg32 += 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_VF | FLAG_NF );
	if ( *bc )
	{
		m_sr.b.l |= FLAG_VF;
	}
}


void tlcs900h_device::_LDX()
{
	UINT8   a, b;

	RDOP();
	a = RDOP();
	RDOP();
	b = RDOP();
	RDOP();
	WRMEM( a, b );
}


void tlcs900h_device::_LINK()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, *m_p1_reg32 );
	*m_p1_reg32 = m_xssp.d;
	m_xssp.d += m_imm2.sw.l;
}


void tlcs900h_device::_MAX()
{
	m_sr.b.h |= 0x04;
}


void tlcs900h_device::_MDEC1()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 += m_imm1.w.l;
	else
		*m_p2_reg16 -= 1;
}


void tlcs900h_device::_MDEC2()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 += m_imm1.w.l;
	else
		*m_p2_reg16 -= 2;
}


void tlcs900h_device::_MDEC4()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 += m_imm1.w.l;
	else
		*m_p2_reg16 -= 4;
}


void tlcs900h_device::_MINC1()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 -= m_imm1.w.l;
	else
		*m_p2_reg16 += 1;
}


void tlcs900h_device::_MINC2()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 -= m_imm1.w.l;
	else
		*m_p2_reg16 += 2;
}


void tlcs900h_device::_MINC4()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 -= m_imm1.w.l;
	else
		*m_p2_reg16 += 4;
}


void tlcs900h_device::_MIRRW()
{
	UINT16  r = *m_p1_reg16;
	UINT16  s = ( r & 0x01 );
	int i;


	for ( i = 0; i < 15; i++ )
	{
		r >>= 1;
		s <<= 1;
		s |= ( r & 0x01 );
	}

	*m_p1_reg16 = s;
}


void tlcs900h_device::_MULBRI()
{
	*m_p1_reg16 = ( *m_p1_reg16 & 0xff ) * m_imm2.b.l;
}


void tlcs900h_device::_MULBRM()
{
	*m_p1_reg16 = ( *m_p1_reg16 & 0xff ) * RDMEM( m_ea2.d );
}


void tlcs900h_device::_MULBRR()
{
	*m_p1_reg16 = ( *m_p1_reg16 & 0xff ) * *m_p2_reg8;
}


void tlcs900h_device::_MULWRI()
{
	*m_p1_reg32 = ( *m_p1_reg32 & 0xffff ) * m_imm2.w.l;
}


void tlcs900h_device::_MULWRM()
{
	*m_p1_reg32 = ( *m_p1_reg32 & 0xffff ) * RDMEMW( m_ea2.d );
}


void tlcs900h_device::_MULWRR()
{
	*m_p1_reg32 = ( *m_p1_reg32 & 0xffff ) * *m_p2_reg16;
}


void tlcs900h_device::_MULAR()
{
	UINT32  *xde = get_reg32_current( 2 );
	UINT32  *xhl = get_reg32_current( 3 );

	*m_p1_reg32 = *m_p1_reg32 + ( ((INT16)RDMEMW( *xde )) * ((INT16)RDMEMW( *xhl )) );
	*xhl -= 2;

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_VF );
	m_sr.b.l |= ( ( *m_p1_reg32 >> 24 ) & FLAG_SF ) | ( *m_p1_reg32 ? 0 : FLAG_ZF );
}


void tlcs900h_device::_MULSBRI()
{
	*m_p1_reg16 = (INT8)( *m_p1_reg16 & 0xff ) * m_imm2.sb.l;
}


void tlcs900h_device::_MULSBRM()
{
	*m_p1_reg16 = (INT8)( *m_p1_reg16 & 0xff ) * (INT8)RDMEM( m_ea2.d );
}


void tlcs900h_device::_MULSBRR()
{
	*m_p1_reg16 = (INT8)( *m_p1_reg16 & 0xff ) * (INT8)*m_p2_reg8;
}


void tlcs900h_device::_MULSWRI()
{
	*m_p1_reg32 = (INT16)( *m_p1_reg32 & 0xffff ) * m_imm2.sw.l;
}


void tlcs900h_device::_MULSWRM()
{
	*m_p1_reg32 = (INT16)( *m_p1_reg32 & 0xffff ) * (INT16)RDMEMW( m_ea2.d );
}


void tlcs900h_device::_MULSWRR()
{
	*m_p1_reg32 = (INT16)( *m_p1_reg32 & 0xffff ) * (INT16)*m_p2_reg16;
}


void tlcs900h_device::_NEGBR()
{
	*m_p1_reg8 = sub8( 0, *m_p1_reg8 );
}


void tlcs900h_device::_NEGWR()
{
	*m_p1_reg16 = sub16( 0, *m_p1_reg16 );
}


void tlcs900h_device::_NOP()
{
	/* Do nothing */
}


void tlcs900h_device::_NORMAL()
{
	m_sr.b.h &= 0x7F;
}


void tlcs900h_device::_ORBMI()
{
	WRMEM( m_ea1.d, or8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_ORBMR()
{
	WRMEM( m_ea1.d, or8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_ORBRI()
{
	*m_p1_reg8 = or8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_ORBRM()
{
	*m_p1_reg8 = or8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ORBRR()
{
	*m_p1_reg8 = or8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_ORWMI()
{
	WRMEMW( m_ea1.d, or16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_ORWMR()
{
	WRMEMW( m_ea1.d, or16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_ORWRI()
{
	*m_p1_reg16 = or16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_ORWRM()
{
	*m_p1_reg16 = or16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_ORWRR()
{
	*m_p1_reg16 = or16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_ORLMR()
{
	WRMEML( m_ea1.d, or32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_ORLRI()
{
	*m_p1_reg32 = or32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_ORLRM()
{
	*m_p1_reg32 = or32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_ORLRR()
{
	*m_p1_reg32 = or32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_ORCFBIM()
{
	orcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ORCFBIR()
{
	orcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::_ORCFBRM()
{
	orcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_ORCFBRR()
{
	orcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_ORCFWIR()
{
	orcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::_ORCFWRR()
{
	orcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::_PAAWR()
{
	if ( *m_p1_reg16 & 1 )
		*m_p1_reg16 += 1;
}


void tlcs900h_device::_PAALR()
{
	if ( *m_p1_reg32 & 1 )
		*m_p1_reg32 += 1;
}


void tlcs900h_device::_POPBM()
{
	WRMEM( m_ea1.d, RDMEM( m_xssp.d ) );
	m_xssp.d += 1;
}


void tlcs900h_device::_POPBR()
{
	*m_p1_reg8 = RDMEM( m_xssp.d );
	m_xssp.d += 1;
}


void tlcs900h_device::_POPWM()
{
	WRMEMW( m_ea1.d, RDMEMW( m_xssp.d ) );
	m_xssp.d += 2;
}


void tlcs900h_device::_POPWR()
{
	*m_p1_reg16 = RDMEMW( m_xssp.d );
	m_xssp.d += 2;
}


void tlcs900h_device::_POPWSR()
{
	_POPWR();
	m_regbank = m_sr.b.h & 0x03;
	m_check_irqs = 1;
}


void tlcs900h_device::_POPLR()
{
	*m_p1_reg32 = RDMEML( m_xssp.d );
	m_xssp.d += 4;
}


void tlcs900h_device::_PUSHBI()
{
	m_xssp.d -= 1;
	WRMEM( m_xssp.d, m_imm1.b.l );
}


void tlcs900h_device::_PUSHBM()
{
	m_xssp.d -= 1;
	WRMEM( m_xssp.d, RDMEM( m_ea1.d ) );
}


void tlcs900h_device::_PUSHBR()
{
	m_xssp.d -= 1;
	WRMEM( m_xssp.d, *m_p1_reg8 );
}


void tlcs900h_device::_PUSHWI()
{
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, m_imm1.w.l );
}


void tlcs900h_device::_PUSHWM()
{
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, RDMEMW( m_ea1.d ) );
}


void tlcs900h_device::_PUSHWR()
{
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, *m_p1_reg16 );
}


void tlcs900h_device::_PUSHLR()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, *m_p1_reg32 );
}


void tlcs900h_device::_RCF()
{
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_NF | FLAG_CF );
}


void tlcs900h_device::_RESBIM()
{
	WRMEM( m_ea2.d, RDMEM( m_ea2.d ) & ~( 1 << ( m_imm1.d & 0x07 ) ) );
}


void tlcs900h_device::_RESBIR()
{
	*m_p2_reg8 = *m_p2_reg8 & ~( 1 << ( m_imm1.d & 0x07 ) );
}


void tlcs900h_device::_RESWIR()
{
	*m_p2_reg16 = *m_p2_reg16 & ~( 1 << ( m_imm1.d & 0x0f ) );
}


void tlcs900h_device::_RET()
{
	m_pc.d = RDMEML( m_xssp.d );
	m_xssp.d += 4;
	m_prefetch_clear = true;
}


void tlcs900h_device::_RETCC()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = RDMEML( m_xssp.d );
		m_xssp.d += 4;
		m_cycles += 6;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::_RETD()
{
	m_pc.d = RDMEML( m_xssp.d );
	m_xssp.d += 4 + m_imm1.sw.l;
	m_prefetch_clear = true;
}


void tlcs900h_device::_RETI()
{
	m_sr.w.l = RDMEMW( m_xssp.d );
	m_xssp.d += 2;
	m_pc.d = RDMEML( m_xssp.d );
	m_xssp.d += 4;
	m_regbank = m_sr.b.h & 0x03;
	m_check_irqs = 1;
	m_prefetch_clear = true;
}


void tlcs900h_device::_RLBM()
{
	WRMEM( m_ea2.d, rl8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RLWM()
{
	WRMEMW( m_ea2.d, rl16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RLBIR()
{
	*m_p2_reg8 = rl8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_RLBRR()
{
	*m_p2_reg8 = rl8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_RLWIR()
{
	*m_p2_reg16 = rl16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_RLWRR()
{
	*m_p2_reg16 = rl16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_RLLIR()
{
	*m_p2_reg32 = rl32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_RLLRR()
{
	*m_p2_reg32 = rl32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_RLCBM()
{
	WRMEM( m_ea2.d, rlc8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RLCWM()
{
	WRMEMW( m_ea2.d, rlc16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RLCBIR()
{
	*m_p2_reg8 = rlc8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_RLCBRR()
{
	*m_p2_reg8 = rlc8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_RLCWIR()
{
	*m_p2_reg16 = rlc16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_RLCWRR()
{
	*m_p2_reg16 = rlc16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_RLCLIR()
{
	*m_p2_reg32 = rlc32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_RLCLRR()
{
	*m_p2_reg32 = rlc32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_RLDRM()
{
	UINT8   a = *m_p1_reg8 & 0x0f;
	UINT8   b = RDMEM( m_ea2.d );

	*m_p1_reg8 = ( *m_p1_reg8 & 0xf0 ) | ( ( b & 0xf0 ) >> 4 );
	WRMEM( m_ea2.d, ( ( b & 0x0f ) << 4 ) | a );
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( *m_p1_reg8 & FLAG_SF ) | ( *m_p1_reg8 ? 0 : FLAG_ZF );
	parity8( *m_p1_reg8 );
}


void tlcs900h_device::_RRBM()
{
	WRMEM( m_ea2.d, rr8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RRWM()
{
	WRMEMW( m_ea2.d, rr16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RRBIR()
{
	*m_p2_reg8 = rr8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_RRBRR()
{
	*m_p2_reg8 = rr8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_RRWIR()
{
	*m_p2_reg16 = rr16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_RRWRR()
{
	*m_p2_reg16 = rr16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_RRLIR()
{
	*m_p2_reg32 = rr32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_RRLRR()
{
	*m_p2_reg32 = rr32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_RRCBM()
{
	WRMEM( m_ea2.d, rrc8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RRCWM()
{
	WRMEMW( m_ea2.d, rrc16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_RRCBIR()
{
	*m_p2_reg8 = rrc8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_RRCBRR()
{
	*m_p2_reg8 = rrc8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_RRCWIR()
{
	*m_p2_reg16 = rrc16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_RRCWRR()
{
	*m_p2_reg16 = rrc16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_RRCLIR()
{
	*m_p2_reg32 = rrc32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_RRCLRR()
{
	*m_p2_reg32 = rrc32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_RRDRM()
{
	UINT8   a = *m_p1_reg8 & 0x0f;
	UINT8   b = RDMEM( m_ea2.d );

	*m_p1_reg8 = ( *m_p1_reg8 & 0xf0 ) | ( b & 0x0f );
	WRMEM( m_ea2.d, ( ( b & 0xf0 ) >> 4 ) | ( a << 4 ) );
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( *m_p1_reg8 & FLAG_SF ) | ( *m_p1_reg8 ? 0 : FLAG_ZF );
	parity8( *m_p1_reg8 );
}


void tlcs900h_device::_SBCBMI()
{
	WRMEM( m_ea1.d, sbc8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_SBCBMR()
{
	WRMEM( m_ea1.d, sbc8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_SBCBRI()
{
	*m_p1_reg8 = sbc8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_SBCBRM()
{
	*m_p1_reg8 = sbc8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_SBCBRR()
{
	*m_p1_reg8 = sbc8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_SBCWMI()
{
	WRMEMW( m_ea1.d, sbc16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_SBCWMR()
{
	WRMEMW( m_ea1.d, sbc16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_SBCWRI()
{
	*m_p1_reg16 = sbc16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_SBCWRM()
{
	*m_p1_reg16 = sbc16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_SBCWRR()
{
	*m_p1_reg16 = sbc16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_SBCLMR()
{
	WRMEML( m_ea1.d, sbc32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_SBCLRI()
{
	*m_p1_reg32 = sbc32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_SBCLRM()
{
	*m_p1_reg32 = sbc32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_SBCLRR()
{
	*m_p1_reg32 = sbc32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_SCCBR()
{
	*m_p2_reg8 = condition_true( m_op ) ? 1 : 0;
}


void tlcs900h_device::_SCCWR()
{
	*m_p2_reg16 = condition_true( m_op ) ? 1 : 0;
}


void tlcs900h_device::_SCF()
{
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_NF );
	m_sr.b.l |= FLAG_CF;
}


void tlcs900h_device::_SETBIM()
{
	WRMEM( m_ea2.d, RDMEM( m_ea2.d ) | ( 1 << ( m_imm1.d & 0x07 ) ) );
}


void tlcs900h_device::_SETBIR()
{
	*m_p2_reg8 = *m_p2_reg8 | ( 1 << ( m_imm1.d & 0x07 ) );
}


void tlcs900h_device::_SETWIR()
{
	*m_p2_reg16 = *m_p2_reg16 | ( 1 << ( m_imm1.d & 0x0f ) );
}


void tlcs900h_device::_SLABM()
{
	WRMEM( m_ea2.d, sla8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SLAWM()
{
	WRMEMW( m_ea2.d, sla16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SLABIR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_SLABRR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_SLAWIR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_SLAWRR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_SLALIR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_SLALRR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_SLLBM()
{
	WRMEM( m_ea2.d, sla8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SLLWM()
{
	WRMEMW( m_ea2.d, sla16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SLLBIR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_SLLBRR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_SLLWIR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_SLLWRR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_SLLLIR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_SLLLRR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_SRABM()
{
	WRMEM( m_ea2.d, sra8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SRAWM()
{
	WRMEMW( m_ea2.d, sra16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SRABIR()
{
	*m_p2_reg8 = sra8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_SRABRR()
{
	*m_p2_reg8 = sra8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_SRAWIR()
{
	*m_p2_reg16 = sra16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_SRAWRR()
{
	*m_p2_reg16 = sra16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_SRALIR()
{
	*m_p2_reg32 = sra32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_SRALRR()
{
	*m_p2_reg32 = sra32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_SRLBM()
{
	WRMEM( m_ea2.d, srl8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SRLWM()
{
	WRMEMW( m_ea2.d, srl16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::_SRLBIR()
{
	*m_p2_reg8 = srl8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::_SRLBRR()
{
	*m_p2_reg8 = srl8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::_SRLWIR()
{
	*m_p2_reg16 = srl16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::_SRLWRR()
{
	*m_p2_reg16 = srl16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::_SRLLIR()
{
	*m_p2_reg32 = srl32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::_SRLLRR()
{
	*m_p2_reg32 = srl32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::_STCFBIM()
{
	if ( m_sr.b.l & FLAG_CF )
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) | ( 1 << ( m_imm1.b.l & 0x07 ) ) );
	else
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) & ~ ( 1 << ( m_imm1.b.l & 0x07 ) ) );
}


void tlcs900h_device::_STCFBIR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg8 |= ( 1 << ( m_imm1.b.l & 0x07 ) );
	else
		*m_p2_reg8 &= ~ ( 1 << ( m_imm1.b.l & 0x07 ) );
}


void tlcs900h_device::_STCFBRM()
{
	if ( m_sr.b.l & FLAG_CF )
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) | ( 1 << ( *m_p1_reg8 & 0x07 ) ) );
	else
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) & ~ ( 1 << ( *m_p1_reg8 & 0x07 ) ) );
}


void tlcs900h_device::_STCFBRR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg8 |= ( 1 << ( *m_p1_reg8 & 0x07 ) );
	else
		*m_p2_reg8 &= ~ ( 1 << ( *m_p1_reg8 & 0x07 ) );
}


void tlcs900h_device::_STCFWIR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg16 |= ( 1 << ( m_imm1.b.l & 0x0f ) );
	else
		*m_p2_reg16 &= ~ ( 1 << ( m_imm1.b.l & 0x0f ) );
}


void tlcs900h_device::_STCFWRR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg16 |= ( 1 << ( *m_p1_reg8 & 0x0f ) );
	else
		*m_p2_reg16 &= ~ ( 1 << ( *m_p1_reg8 & 0x0f ) );
}


void tlcs900h_device::_SUBBMI()
{
	WRMEM( m_ea1.d, sub8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_SUBBMR()
{
	WRMEM( m_ea1.d, sub8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_SUBBRI()
{
	*m_p1_reg8 = sub8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_SUBBRM()
{
	*m_p1_reg8 = sub8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_SUBBRR()
{
	*m_p1_reg8 = sub8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_SUBWMI()
{
	WRMEMW( m_ea1.d, sub16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_SUBWMR()
{
	WRMEMW( m_ea1.d, sub16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_SUBWRI()
{
	*m_p1_reg16 = sub16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_SUBWRM()
{
	*m_p1_reg16 = sub16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_SUBWRR()
{
	*m_p1_reg16 = sub16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_SUBLMR()
{
	WRMEML( m_ea1.d, sub32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_SUBLRI()
{
	*m_p1_reg32 = sub32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_SUBLRM()
{
	*m_p1_reg32 = sub32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_SUBLRR()
{
	*m_p1_reg32 = sub32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_SWI()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, m_pc.d );
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, m_sr.w.l );
	m_pc.d = RDMEML( 0x00ffff00 + 4 * m_imm1.b.l );
	m_prefetch_clear = true;
}


void tlcs900h_device::_TSETBIM()
{
	UINT8   b = 1 << ( m_imm1.b.l & 0x07 );
	UINT8   a = RDMEM( m_ea2.d );

	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	m_sr.b.l |= ( ( a & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	WRMEM( m_ea2.d, a | b );
}


void tlcs900h_device::_TSETBIR()
{
	UINT8   b = 1 << ( m_imm1.b.l & 0x07 );

	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	m_sr.b.l |= ( ( *m_p2_reg8 & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	*m_p2_reg8 |= b;
}


void tlcs900h_device::_TSETWIR()
{
	UINT16  b = 1 << ( m_imm1.b.l & 0x0f );

	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	m_sr.b.l |= ( ( *m_p2_reg16 & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	*m_p2_reg16 |= b;
}


void tlcs900h_device::_UNLK()
{
	m_xssp.d = *m_p1_reg32;
	*m_p1_reg32 = RDMEML( m_xssp.d );
	m_xssp.d += 4;
}


void tlcs900h_device::_XORBMI()
{
	WRMEM( m_ea1.d, xor8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::_XORBMR()
{
	WRMEM( m_ea1.d, xor8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::_XORBRI()
{
	*m_p1_reg8 = xor8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::_XORBRM()
{
	*m_p1_reg8 = xor8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_XORBRR()
{
	*m_p1_reg8 = xor8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_XORWMI()
{
	WRMEMW( m_ea1.d, xor16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::_XORWMR()
{
	WRMEMW( m_ea1.d, xor16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::_XORWRI()
{
	*m_p1_reg16 = xor16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::_XORWRM()
{
	*m_p1_reg16 = xor16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::_XORWRR()
{
	*m_p1_reg16 = xor16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::_XORLMR()
{
	WRMEML( m_ea1.d, xor32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::_XORLRI()
{
	*m_p1_reg32 = xor32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::_XORLRM()
{
	*m_p1_reg32 = xor32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::_XORLRR()
{
	*m_p1_reg32 = xor32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::_XORCFBIM()
{
	xorcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_XORCFBIR()
{
	xorcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::_XORCFBRM()
{
	xorcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::_XORCFBRR()
{
	xorcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::_XORCFWIR()
{
	xorcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::_XORCFWRR()
{
	xorcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::_ZCF()
{
	m_sr.b.l &= ~ ( FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( m_sr.b.l & FLAG_ZF ) ? 0 : FLAG_CF );
}


void tlcs900h_device::prepare_operands(const tlcs900inst *inst)
{
	switch ( inst->operand1 )
	{
	case _A:
		m_p1_reg8 = &m_xwa[m_regbank].b.l;
		break;
	case _F:
		m_p1_reg8 = &m_sr.b.l;
		break;
	case _SR:
		m_p1_reg16 = &m_sr.w.l;
		break;
	case _C8:
		m_p1_reg8 = get_reg8_current( m_op );
		break;
	case _C16:
		m_p1_reg16 = get_reg16_current( m_op );
		break;
	case _MC16: /* For MUL and DIV operations */
		m_p1_reg16 = get_reg16_current( ( m_op >> 1 ) & 0x03 );
		break;
	case _C32:
		m_p1_reg32 = get_reg32_current( m_op );
		break;
	case _CR8:
		m_imm1.d = RDOP();
		switch( m_imm1.d )
		{
		case 0x22:
			m_p1_reg8 = &m_dmam[0].b.l;
			break;
		case 0x26:
			m_p1_reg8 = &m_dmam[1].b.l;
			break;
		case 0x2a:
			m_p1_reg8 = &m_dmam[2].b.l;
			break;
		case 0x2e:
			m_p1_reg8 = &m_dmam[3].b.l;
			break;
		default:
			m_p1_reg8 = &m_dummy.b.l;
			break;
		}
		break;
	case _CR16:
		m_imm1.d = RDOP();
		switch( m_imm1.d )
		{
		case 0x20:
			m_p1_reg16 = &m_dmac[0].w.l;
			break;
		case 0x24:
			m_p1_reg16 = &m_dmac[1].w.l;
			break;
		case 0x28:
			m_p1_reg16 = &m_dmac[2].w.l;
			break;
		case 0x2c:
			m_p1_reg16 = &m_dmac[3].w.l;
			break;
		default:
			m_p1_reg16 = &m_dummy.w.l;
			break;
		}
		break;
	case _CR32:
		m_imm1.d = RDOP();
		switch( m_imm1.d )
		{
		case 0x00:
			m_p1_reg32 = &m_dmas[0].d;
			break;
		case 0x04:
			m_p1_reg32 = &m_dmas[1].d;
			break;
		case 0x08:
			m_p1_reg32 = &m_dmas[2].d;
			break;
		case 0x0c:
			m_p1_reg32 = &m_dmas[3].d;
			break;
		case 0x10:
			m_p1_reg32 = &m_dmad[0].d;
			break;
		case 0x14:
			m_p1_reg32 = &m_dmad[1].d;
			break;
		case 0x18:
			m_p1_reg32 = &m_dmad[2].d;
			break;
		case 0x1c:
			m_p1_reg32 = &m_dmad[3].d;
			break;
		default:
			m_p1_reg32 = &m_dummy.d;
			break;
		}
		break;
	case _D8:
		m_ea1.d = RDOP();
		m_ea1.d = m_pc.d + m_ea1.sb.l;
		break;
	case _D16:
		m_ea1.d = RDOP();
		m_ea1.b.h = RDOP();
		m_ea1.d = m_pc.d + m_ea1.sw.l;
		break;
	case _I3:
		m_imm1.d = m_op & 0x07;
		break;
	case _I8:
		m_imm1.d = RDOP();
		break;
	case _I16:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		break;
	case _I24:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		m_imm1.b.h2 = RDOP();
		break;
	case _I32:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		m_imm1.b.h2 = RDOP();
		m_imm1.b.h3 = RDOP();
		break;
	case _M:
		m_ea1.d = m_ea2.d;
		break;
	case _M8:
		m_ea1.d = RDOP();
		break;
	case _M16:
		m_ea1.d = RDOP();
		m_ea1.b.h = RDOP();
		break;
	case _R:
		m_p1_reg8 = m_p2_reg8;
		m_p1_reg16 = m_p2_reg16;
		m_p1_reg32 = m_p2_reg32;
		break;
	}

	switch ( inst->operand2 )
	{
	case _A:
		m_p2_reg8 = &m_xwa[m_regbank].b.l;
		break;
	case _F:        /* F' */
		m_p2_reg8 = &m_f2.b.l;
		break;
	case _SR:
		m_p2_reg16 = &m_sr.w.l;
		break;
	case _C8:
		m_p2_reg8 = get_reg8_current( m_op );
		break;
	case _C16:
		m_p2_reg16 = get_reg16_current( m_op );
		break;
	case _C32:
		m_p2_reg32 = get_reg32_current( m_op );
		break;
	case _CR8:
		m_imm1.d = RDOP();
		switch( m_imm1.d )
		{
		case 0x22:
			m_p2_reg8 = &m_dmam[0].b.l;
			break;
		case 0x26:
			m_p2_reg8 = &m_dmam[1].b.l;
			break;
		case 0x2a:
			m_p2_reg8 = &m_dmam[2].b.l;
			break;
		case 0x2e:
			m_p2_reg8 = &m_dmam[3].b.l;
			break;
		default:
			m_p2_reg8 = &m_dummy.b.l;
			break;
		}
		break;
	case _CR16:
		m_imm1.d = RDOP();
		switch( m_imm1.d )
		{
		case 0x20:
			m_p2_reg16 = &m_dmac[0].w.l;
			break;
		case 0x24:
			m_p2_reg16 = &m_dmac[1].w.l;
			break;
		case 0x28:
			m_p2_reg16 = &m_dmac[2].w.l;
			break;
		case 0x2c:
			m_p2_reg16 = &m_dmac[3].w.l;
			break;
		default:
			m_p2_reg16 = &m_dummy.w.l;
			break;
		}
		break;
	case _CR32:
		m_imm1.d = RDOP();
		switch( m_imm1.d )
		{
		case 0x00:
			m_p2_reg32 = &m_dmas[0].d;
			break;
		case 0x04:
			m_p2_reg32 = &m_dmas[1].d;
			break;
		case 0x08:
			m_p2_reg32 = &m_dmas[2].d;
			break;
		case 0x0c:
			m_p2_reg32 = &m_dmas[3].d;
			break;
		case 0x10:
			m_p2_reg32 = &m_dmad[0].d;
			break;
		case 0x14:
			m_p2_reg32 = &m_dmad[1].d;
			break;
		case 0x18:
			m_p2_reg32 = &m_dmad[2].d;
			break;
		case 0x1c:
			m_p2_reg32 = &m_dmad[3].d;
			break;
		default:
			m_p2_reg32 = &m_dummy.d;
			break;
		}
		break;
	case _D8:
		m_ea2.d = RDOP();
		m_ea2.d = m_pc.d + m_ea2.sb.l;
		break;
	case _D16:
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.d = m_pc.d + m_ea2.sw.l;
		break;
	case _I3:
		m_imm2.d = m_op & 0x07;
		break;
	case _I8:
		m_imm2.d = RDOP();
		break;
	case _I16:
		m_imm2.d = RDOP();
		m_imm2.b.h = RDOP();
		break;
	case _I32:
		m_imm2.d = RDOP();
		m_imm2.b.h = RDOP();
		m_imm2.b.h2 = RDOP();
		m_imm2.b.h3 = RDOP();
		break;
	case _M8:
		m_ea2.d = RDOP();
		break;
	case _M16:
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		break;
	}
}


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_80[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_PUSHBM, _M, 0, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_RLDRM, _A, _M, 12 }, { &tlcs900h_device::_RRDRM, _A, _M, 12 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDI, 0, 0, 10 }, { &tlcs900h_device::_LDIR, 0, 0, 10 }, { &tlcs900h_device::_LDD, 0, 0, 10 }, { &tlcs900h_device::_LDDR, 0, 0, 10 },
	{ &tlcs900h_device::_CPI, 0, 0, 8 }, { &tlcs900h_device::_CPIR, 0, 0, 10 }, { &tlcs900h_device::_CPD, 0, 0, 8 }, { &tlcs900h_device::_CPDR, 0, 0, 10 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDBMM, _M16, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADDBMI, _M, _I8, 7 }, { &tlcs900h_device::_ADCBMI, _M, _I8, 7 }, { &tlcs900h_device::_SUBBMI, _M, _I8, 7 }, { &tlcs900h_device::_SBCBMI, _M, _I8, 7 },
	{ &tlcs900h_device::_ANDBMI, _M, _I8, 7 }, { &tlcs900h_device::_XORBMI, _M, _I8, 7 }, { &tlcs900h_device::_ORBMI, _M, _I8, 7 }, { &tlcs900h_device::_CPBMI, _M, _I8, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULBRM, _MC16, _M, 18}, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULBRM, _MC16, _M, 18}, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 },
	{ &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 },
	{ &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 },
	{ &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_RLCBM, _M, 0, 8 }, { &tlcs900h_device::_RRCBM, _M, 0, 8 }, { &tlcs900h_device::_RLBM, _M, 0, 8 }, { &tlcs900h_device::_RRBM, _M, 0, 8 },
	{ &tlcs900h_device::_SLABM, _M, 0, 8 }, { &tlcs900h_device::_SRABM, _M, 0, 8 }, { &tlcs900h_device::_SLLBM, _M, 0, 8 }, { &tlcs900h_device::_SRLBM, _M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_88[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_PUSHBM, _M, 0, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_RLDRM, _A, _M, 12 }, { &tlcs900h_device::_RRDRM, _A, _M, 12 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDBMM, _M16, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADDBMI, _M, _I8, 7 }, { &tlcs900h_device::_ADCBMI, _M, _I8, 7 }, { &tlcs900h_device::_SUBBMI, _M, _I8, 7 }, { &tlcs900h_device::_SBCBMI, _M, _I8, 7 },
	{ &tlcs900h_device::_ANDBMI, _M, _I8, 7 }, { &tlcs900h_device::_XORBMI, _M, _I8, 7 }, { &tlcs900h_device::_ORBMI, _M, _I8, 7 }, { &tlcs900h_device::_CPBMI, _M, _I8, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 },
	{ &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 },
	{ &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 },
	{ &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_RLCBM, _M, 0, 8 }, { &tlcs900h_device::_RRCBM, _M, 0, 8 }, { &tlcs900h_device::_RLBM, _M, 0, 8 }, { &tlcs900h_device::_RRBM, _M, 0, 8 },
	{ &tlcs900h_device::_SLABM, _M, 0, 8 }, { &tlcs900h_device::_SRABM, _M, 0, 8 }, { &tlcs900h_device::_SLLBM, _M, 0, 8 }, { &tlcs900h_device::_SRLBM, _M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_90[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_PUSHWM, _M, 0, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDIW, 0, 0, 10 }, { &tlcs900h_device::_LDIRW, 0, 0, 10 }, { &tlcs900h_device::_LDDW, 0, 0, 10 }, { &tlcs900h_device::_LDDRW, 0, 0, 10 },
	{ &tlcs900h_device::_CPIW, 0, 0, 8 }, { &tlcs900h_device::_CPIRW, 0, 0, 10 }, { &tlcs900h_device::_CPDW, 0, 0, 8 }, { &tlcs900h_device::_CPDRW, 0, 0, 10 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMM, _M16, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADDWMI, _M, _I16, 8 }, { &tlcs900h_device::_ADCWMI, _M, _I16, 8 }, { &tlcs900h_device::_SUBWMI, _M, _I16, 8 }, { &tlcs900h_device::_SBCWMI, _M, _I16, 8 },
	{ &tlcs900h_device::_ANDWMI, _M, _I16, 8 }, { &tlcs900h_device::_XORWMI, _M, _I16, 8 }, { &tlcs900h_device::_ORWMI, _M, _I16, 8 }, { &tlcs900h_device::_CPWMI, _M, _I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 },
	{ &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 },
	{ &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 },
	{ &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_RLCWM, _M, 0, 8 }, { &tlcs900h_device::_RRCWM, _M, 0, 8 }, { &tlcs900h_device::_RLWM, _M, 0, 8 }, { &tlcs900h_device::_RRWM, _M, 0, 8 },
	{ &tlcs900h_device::_SLAWM, _M, 0, 8 }, { &tlcs900h_device::_SRAWM, _M, 0, 8 }, { &tlcs900h_device::_SLLWM, _M, 0, 8 }, { &tlcs900h_device::_SRLWM, _M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_98[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_PUSHWM, _M, 0, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMM, _M16, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADDWMI, _M, _I16, 8 }, { &tlcs900h_device::_ADCWMI, _M, _I16, 8 }, { &tlcs900h_device::_SUBWMI, _M, _I16, 8 }, { &tlcs900h_device::_SBCWMI, _M, _I16, 8 },
	{ &tlcs900h_device::_ANDWMI, _M, _I16, 8 }, { &tlcs900h_device::_XORWMI, _M, _I16, 8 }, { &tlcs900h_device::_ORWMI, _M, _I16, 8 }, { &tlcs900h_device::_CPWMI, _M, _I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 },
	{ &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 },
	{ &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 },
	{ &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_RLCWM, _M, 0, 8 }, { &tlcs900h_device::_RRCWM, _M, 0, 8 }, { &tlcs900h_device::_RLWM, _M, 0, 8 }, { &tlcs900h_device::_RRWM, _M, 0, 8 },
	{ &tlcs900h_device::_SLAWM, _M, 0, 8 }, { &tlcs900h_device::_SRAWM, _M, 0, 8 }, { &tlcs900h_device::_SLLWM, _M, 0, 8 }, { &tlcs900h_device::_SRLWM, _M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_LDBMI, _M, _I8, 5 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMI, _M, _I16, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_POPBM, _M, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_POPWM, _M, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDBMM, _M, _M16, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMM, _M, _M16, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 },
	{ &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDCFBRM, _A, _M, 8 }, { &tlcs900h_device::_ORCFBRM, _A, _M, 8 }, { &tlcs900h_device::_XORCFBRM, _A, _M, 8 }, { &tlcs900h_device::_LDCFBRM, _A, _M, 8 },
	{ &tlcs900h_device::_STCFBRM, _A, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 },
	{ &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 },
	{ &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 },
	{ &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 },

	/* A0 - BF */
	{ &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 },
	{ &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 },
	{ &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 },

	/* C0 - DF */
	{ &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 },
	{ &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 },
	{ &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 },
	{ &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }, { &tlcs900h_device::_RETCC, _CC, 0, 6 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_LDBMI, _M, _I8, 5 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMI, _M, _I16, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_POPBM, _M, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_POPWM, _M, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDBMM, _M, _M16, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMM, _M, _M16, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 },
	{ &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDCFBRM, _A, _M, 8 }, { &tlcs900h_device::_ORCFBRM, _A, _M, 8 }, { &tlcs900h_device::_XORCFBRM, _A, _M, 8 }, { &tlcs900h_device::_LDCFBRM, _A, _M, 8 },
	{ &tlcs900h_device::_STCFBRM, _A, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 },
	{ &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 },
	{ &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 },
	{ &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 8 },

	/* A0 - BF */
	{ &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 },
	{ &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 },
	{ &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 },

	/* C0 - DF */
	{ &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 }, { &tlcs900h_device::_BITBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_PUSHBM, _M, 0, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_RLDRM, _A, _M, 12 }, { &tlcs900h_device::_RRDRM, _A, _M, 12 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDBMM, _M16, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 }, { &tlcs900h_device::_LDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 }, { &tlcs900h_device::_EXBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADDBMI, _M, _I8, 7 }, { &tlcs900h_device::_ADCBMI, _M, _I8, 7 }, { &tlcs900h_device::_SUBBMI, _M, _I8, 7 }, { &tlcs900h_device::_SBCBMI, _M, _I8, 7 },
	{ &tlcs900h_device::_ANDBMI, _M, _I8, 7 }, { &tlcs900h_device::_XORBMI, _M, _I8, 7 }, { &tlcs900h_device::_ORBMI, _M, _I8, 7 }, { &tlcs900h_device::_CPBMI, _M, _I8, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 }, { &tlcs900h_device::_MULSBRM, _MC16, _M, 18 },
	{ &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 },
	{ &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 }, { &tlcs900h_device::_DIVBRM, _MC16, _M, 22 },
	{ &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 },
	{ &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 }, { &tlcs900h_device::_DIVSBRM, _MC16, _M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 }, { &tlcs900h_device::_INCBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 }, { &tlcs900h_device::_DECBIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_RLCBM, _M, 0, 8 }, { &tlcs900h_device::_RRCBM, _M, 0, 8 }, { &tlcs900h_device::_RLBM, _M, 0, 8 }, { &tlcs900h_device::_RRBM, _M, 0, 8 },
	{ &tlcs900h_device::_SLABM, _M, 0, 8 }, { &tlcs900h_device::_SRABM, _M, 0, 8 }, { &tlcs900h_device::_SLLBM, _M, 0, 8 }, { &tlcs900h_device::_SRLBM, _M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 }, { &tlcs900h_device::_ADCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 }, { &tlcs900h_device::_ADCBMR, _M, _C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 }, { &tlcs900h_device::_SUBBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 }, { &tlcs900h_device::_SUBBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 }, { &tlcs900h_device::_SBCBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 }, { &tlcs900h_device::_SBCBMR, _M, _C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 }, { &tlcs900h_device::_ANDBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 }, { &tlcs900h_device::_ANDBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 }, { &tlcs900h_device::_XORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 }, { &tlcs900h_device::_XORBMR, _M, _C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 }, { &tlcs900h_device::_ORBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 }, { &tlcs900h_device::_ORBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 }, { &tlcs900h_device::_CPBRM, _C8, _M, 4 },
	{ &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 },
	{ &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 }, { &tlcs900h_device::_CPBMR, _M, _C8, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDBRI, _R, _I8, 4 },
	{ &tlcs900h_device::_PUSHBR, _R, 0, 6 }, { &tlcs900h_device::_POPBR, _R, 0, 6 }, { &tlcs900h_device::_CPLBR, _R, 0, 4 }, { &tlcs900h_device::_NEGBR, _R, 0, 5 },
	{ &tlcs900h_device::_MULBRI, _R, _I8, 18}, { &tlcs900h_device::_MULSBRI, _R, _I8, 18 }, { &tlcs900h_device::_DIVBRI, _R, _I8, 22 }, { &tlcs900h_device::_DIVSBRI, _R, _I8, 24 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DAABR, _R, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DJNZB, _R, _D8, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_ANDCFBIR, _I8, _R, 4 }, { &tlcs900h_device::_ORCFBIR, _I8, _R, 4 }, { &tlcs900h_device::_XORCFBIR, _I8, _R, 4 }, { &tlcs900h_device::_LDCFBIR, _I8, _R, 4 },
	{ &tlcs900h_device::_STCFBIR, _I8, _R, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_ANDCFBRR, _A, _R, 4 }, { &tlcs900h_device::_ORCFBRR, _A, _R, 4 }, { &tlcs900h_device::_XORCFBRR, _A, _R, 4 }, { &tlcs900h_device::_LDCFBRR, _A, _R, 4 },
	{ &tlcs900h_device::_STCFBRR, _A, _R, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDCBRR, _CR8, _R, 1 }, { &tlcs900h_device::_LDCBRR, _R, _CR8, 1 },
	{ &tlcs900h_device::_RESBIR, _I8, _R, 4 }, { &tlcs900h_device::_SETBIR, _I8, _R, 4 }, { &tlcs900h_device::_CHGBIR, _I8, _R, 4 }, { &tlcs900h_device::_BITBIR, _I8, _R, 4 },
	{ &tlcs900h_device::_TSETBIR, _I8, _R, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULBRR, _MC16, _R, 18 },
	{ &tlcs900h_device::_MULBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULBRR, _MC16, _R, 18 },
	{ &tlcs900h_device::_MULSBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULSBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULSBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULSBRR, _MC16, _R, 18 },
	{ &tlcs900h_device::_MULSBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULSBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULSBRR, _MC16, _R, 18 }, { &tlcs900h_device::_MULSBRR, _MC16, _R, 18 },
	{ &tlcs900h_device::_DIVBRR, _MC16, _R, 22 }, { &tlcs900h_device::_DIVBRR, _MC16, _R, 22 }, { &tlcs900h_device::_DIVBRR, _MC16, _R, 22 }, { &tlcs900h_device::_DIVBRR, _MC16, _R, 22 },
	{ &tlcs900h_device::_DIVBRR, _MC16, _R, 22 }, { &tlcs900h_device::_DIVBRR, _MC16, _R, 22 }, { &tlcs900h_device::_DIVBRR, _MC16, _R, 22 }, { &tlcs900h_device::_DIVBRR, _MC16, _R, 22 },
	{ &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 }, { &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 }, { &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 }, { &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 },
	{ &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 }, { &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 }, { &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 }, { &tlcs900h_device::_DIVSBRR, _MC16, _R, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCBIR, _I3, _R, 4 }, { &tlcs900h_device::_INCBIR, _I3, _R, 4 }, { &tlcs900h_device::_INCBIR, _I3, _R, 4 }, { &tlcs900h_device::_INCBIR, _I3, _R, 4 },
	{ &tlcs900h_device::_INCBIR, _I3, _R, 4 }, { &tlcs900h_device::_INCBIR, _I3, _R, 4 }, { &tlcs900h_device::_INCBIR, _I3, _R, 4 }, { &tlcs900h_device::_INCBIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DECBIR, _I3, _R, 4 }, { &tlcs900h_device::_DECBIR, _I3, _R, 4 }, { &tlcs900h_device::_DECBIR, _I3, _R, 4 }, { &tlcs900h_device::_DECBIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DECBIR, _I3, _R, 4 }, { &tlcs900h_device::_DECBIR, _I3, _R, 4 }, { &tlcs900h_device::_DECBIR, _I3, _R, 4 }, { &tlcs900h_device::_DECBIR, _I3, _R, 4 },
	{ &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 },
	{ &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 },
	{ &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 },
	{ &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 }, { &tlcs900h_device::_SCCBR, _CC, _R, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADDBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_ADDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADDBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_LDBRR, _C8, _R, 4 }, { &tlcs900h_device::_LDBRR, _C8, _R, 4 }, { &tlcs900h_device::_LDBRR, _C8, _R, 4 }, { &tlcs900h_device::_LDBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_LDBRR, _C8, _R, 4 }, { &tlcs900h_device::_LDBRR, _C8, _R, 4 }, { &tlcs900h_device::_LDBRR, _C8, _R, 4 }, { &tlcs900h_device::_LDBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_ADCBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADCBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADCBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADCBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_ADCBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADCBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADCBRR, _C8, _R, 4 }, { &tlcs900h_device::_ADCBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_LDBRR, _R, _C8, 4 }, { &tlcs900h_device::_LDBRR, _R, _C8, 4 }, { &tlcs900h_device::_LDBRR, _R, _C8, 4 }, { &tlcs900h_device::_LDBRR, _R, _C8, 4 },
	{ &tlcs900h_device::_LDBRR, _R, _C8, 4 }, { &tlcs900h_device::_LDBRR, _R, _C8, 4 }, { &tlcs900h_device::_LDBRR, _R, _C8, 4 }, { &tlcs900h_device::_LDBRR, _R, _C8, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBBRR, _C8, _R, 4 }, { &tlcs900h_device::_SUBBRR, _C8, _R, 4 }, { &tlcs900h_device::_SUBBRR, _C8, _R, 4 }, { &tlcs900h_device::_SUBBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_SUBBRR, _C8, _R, 4 }, { &tlcs900h_device::_SUBBRR, _C8, _R, 4 }, { &tlcs900h_device::_SUBBRR, _C8, _R, 4 }, { &tlcs900h_device::_SUBBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_LDBRI, _R, _I3, 4 }, { &tlcs900h_device::_LDBRI, _R, _I3, 4 }, { &tlcs900h_device::_LDBRI, _R, _I3, 4 }, { &tlcs900h_device::_LDBRI, _R, _I3, 4 },
	{ &tlcs900h_device::_LDBRI, _R, _I3, 4 }, { &tlcs900h_device::_LDBRI, _R, _I3, 4 }, { &tlcs900h_device::_LDBRI, _R, _I3, 4 }, { &tlcs900h_device::_LDBRI, _R, _I3, 4 },
	{ &tlcs900h_device::_SBCBRR, _C8, _R, 4 }, { &tlcs900h_device::_SBCBRR, _C8, _R, 4 }, { &tlcs900h_device::_SBCBRR, _C8, _R, 4 }, { &tlcs900h_device::_SBCBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_SBCBRR, _C8, _R, 4 }, { &tlcs900h_device::_SBCBRR, _C8, _R, 4 }, { &tlcs900h_device::_SBCBRR, _C8, _R, 4 }, { &tlcs900h_device::_SBCBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_EXBRR, _C8, _R, 5 }, { &tlcs900h_device::_EXBRR, _C8, _R, 5 }, { &tlcs900h_device::_EXBRR, _C8, _R, 5 }, { &tlcs900h_device::_EXBRR, _C8, _R, 5 },
	{ &tlcs900h_device::_EXBRR, _C8, _R, 5 }, { &tlcs900h_device::_EXBRR, _C8, _R, 5 }, { &tlcs900h_device::_EXBRR, _C8, _R, 5 }, { &tlcs900h_device::_EXBRR, _C8, _R, 5 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ANDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ANDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ANDBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_ANDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ANDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ANDBRR, _C8, _R, 4 }, { &tlcs900h_device::_ANDBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_ADDBRI, _R, _I8, 4 }, { &tlcs900h_device::_ADCBRI, _R, _I8, 4 }, { &tlcs900h_device::_SUBBRI, _R, _I8, 4 }, { &tlcs900h_device::_SBCBRI, _R, _I8, 4 },
	{ &tlcs900h_device::_ANDBRI, _R, _I8, 4 }, { &tlcs900h_device::_XORBRI, _R, _I8, 4 }, { &tlcs900h_device::_ORBRI, _R, _I8, 4 }, { &tlcs900h_device::_CPBRI, _R, _I8, 4 },
	{ &tlcs900h_device::_XORBRR, _C8, _R, 4 }, { &tlcs900h_device::_XORBRR, _C8, _R, 4 }, { &tlcs900h_device::_XORBRR, _C8, _R, 4 }, { &tlcs900h_device::_XORBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_XORBRR, _C8, _R, 4 }, { &tlcs900h_device::_XORBRR, _C8, _R, 4 }, { &tlcs900h_device::_XORBRR, _C8, _R, 4 }, { &tlcs900h_device::_XORBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_CPBRI, _R, _I3, 4 }, { &tlcs900h_device::_CPBRI, _R, _I3, 4 }, { &tlcs900h_device::_CPBRI, _R, _I3, 4 }, { &tlcs900h_device::_CPBRI, _R, _I3, 4 },
	{ &tlcs900h_device::_CPBRI, _R, _I3, 4 }, { &tlcs900h_device::_CPBRI, _R, _I3, 4 }, { &tlcs900h_device::_CPBRI, _R, _I3, 4 }, { &tlcs900h_device::_CPBRI, _R, _I3, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORBRR, _C8, _R, 4 }, { &tlcs900h_device::_ORBRR, _C8, _R, 4 }, { &tlcs900h_device::_ORBRR, _C8, _R, 4 }, { &tlcs900h_device::_ORBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_ORBRR, _C8, _R, 4 }, { &tlcs900h_device::_ORBRR, _C8, _R, 4 }, { &tlcs900h_device::_ORBRR, _C8, _R, 4 }, { &tlcs900h_device::_ORBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_RLCBIR, _I8, _R, 6 }, { &tlcs900h_device::_RRCBIR, _I8, _R, 6 }, { &tlcs900h_device::_RLBIR, _I8, _R, 6 }, { &tlcs900h_device::_RRBIR, _I8, _R, 6 },
	{ &tlcs900h_device::_SLABIR, _I8, _R, 6 }, { &tlcs900h_device::_SRABIR, _I8, _R, 6 }, { &tlcs900h_device::_SLLBIR, _I8, _R, 6 }, { &tlcs900h_device::_SRLBIR, _I8, _R, 6 },
	{ &tlcs900h_device::_CPBRR, _C8, _R, 4 }, { &tlcs900h_device::_CPBRR, _C8, _R, 4 }, { &tlcs900h_device::_CPBRR, _C8, _R, 4 }, { &tlcs900h_device::_CPBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_CPBRR, _C8, _R, 4 }, { &tlcs900h_device::_CPBRR, _C8, _R, 4 }, { &tlcs900h_device::_CPBRR, _C8, _R, 4 }, { &tlcs900h_device::_CPBRR, _C8, _R, 4 },
	{ &tlcs900h_device::_RLCBRR, _A, _R, 6 }, { &tlcs900h_device::_RRCBRR, _A, _R, 6 }, { &tlcs900h_device::_RLBRR, _A, _R, 6 }, { &tlcs900h_device::_RRBRR, _A, _R, 6 },
	{ &tlcs900h_device::_SLABRR, _A, _R, 6 }, { &tlcs900h_device::_SRABRR, _A, _R, 6 }, { &tlcs900h_device::_SLLBRR, _A, _R, 6 }, { &tlcs900h_device::_SRLBRR, _A, _R, 6 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_PUSHWM, _M, 0, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMM, _M16, _M, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 }, { &tlcs900h_device::_LDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 }, { &tlcs900h_device::_EXWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADDWMI, _M, _I16, 8 }, { &tlcs900h_device::_ADCWMI, _M, _I16, 8 }, { &tlcs900h_device::_SUBWMI, _M, _I16, 8 }, { &tlcs900h_device::_SBCWMI, _M, _I16, 8 },
	{ &tlcs900h_device::_ANDWMI, _M, _I16, 8 }, { &tlcs900h_device::_XORWMI, _M, _I16, 8 }, { &tlcs900h_device::_ORWMI, _M, _I16, 8 }, { &tlcs900h_device::_CPWMI, _M, _I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 }, { &tlcs900h_device::_MULSWRM, _C32, _M, 26 },
	{ &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 },
	{ &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 }, { &tlcs900h_device::_DIVWRM, _C32, _M, 30 },
	{ &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 },
	{ &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 }, { &tlcs900h_device::_DIVSWRM, _C32, _M, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 }, { &tlcs900h_device::_INCWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 }, { &tlcs900h_device::_DECWIM, _I3, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_RLCWM, _M, 0, 8 }, { &tlcs900h_device::_RRCWM, _M, 0, 8 }, { &tlcs900h_device::_RLWM, _M, 0, 8 }, { &tlcs900h_device::_RRWM, _M, 0, 8 },
	{ &tlcs900h_device::_SLAWM, _M, 0, 8 }, { &tlcs900h_device::_SRAWM, _M, 0, 8 }, { &tlcs900h_device::_SLLWM, _M, 0, 8 }, { &tlcs900h_device::_SRLWM, _M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 }, { &tlcs900h_device::_ADCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 }, { &tlcs900h_device::_ADCWMR, _M, _C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 }, { &tlcs900h_device::_SUBWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 }, { &tlcs900h_device::_SUBWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 }, { &tlcs900h_device::_SBCWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 }, { &tlcs900h_device::_SBCWMR, _M, _C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 }, { &tlcs900h_device::_ANDWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 }, { &tlcs900h_device::_ANDWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 }, { &tlcs900h_device::_XORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 }, { &tlcs900h_device::_XORWMR, _M, _C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 }, { &tlcs900h_device::_ORWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 }, { &tlcs900h_device::_ORWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 }, { &tlcs900h_device::_CPWRM, _C16, _M, 4 },
	{ &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 },
	{ &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 }, { &tlcs900h_device::_CPWMR, _M, _C16, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWRI, _R, _I16, 4 },
	{ &tlcs900h_device::_PUSHWR, _R, 0, 5 }, { &tlcs900h_device::_POPWR, _R, 0, 6 }, { &tlcs900h_device::_CPLWR, _R, 0, 4 }, { &tlcs900h_device::_NEGWR, _R, 0, 5 },
	{ &tlcs900h_device::_MULWRI, _R, _I16, 26 }, { &tlcs900h_device::_MULSWRI, _R, _I16, 26 }, { &tlcs900h_device::_DIVWRI, _R, _I16, 30 }, { &tlcs900h_device::_DIVSWRI, _R, _I16, 32 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_BS1FRR, _A, _R, 4 }, { &tlcs900h_device::_BS1BRR, _A, _R, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_EXTZWR, _R, 0, 4 }, { &tlcs900h_device::_EXTSWR, _R, 0, 5 },
	{ &tlcs900h_device::_PAAWR, _R, 0, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_MIRRW, _R, 0, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_MULAR, _R, 0, 31 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DJNZW, _R, _D8, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_ANDCFWIR, _I8, _R, 4 }, { &tlcs900h_device::_ORCFWIR, _I8, _R, 4 }, { &tlcs900h_device::_XORCFWIR, _I8, _R, 4 }, { &tlcs900h_device::_LDCFWIR, _I8, _R, 4 },
	{ &tlcs900h_device::_STCFWIR, _I8, _R, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_ANDCFWRR, _A, _R, 4 }, { &tlcs900h_device::_ORCFWRR, _A, _R, 4 }, { &tlcs900h_device::_XORCFWRR, _A, _R, 4 }, { &tlcs900h_device::_LDCFWRR, _A, _R, 4 },
	{ &tlcs900h_device::_STCFWRR, _A, _R, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDCWRR, _CR16, _R, 1 }, { &tlcs900h_device::_LDCWRR, _R, _CR16, 1 },
	{ &tlcs900h_device::_RESWIR, _I8, _R, 4 }, { &tlcs900h_device::_SETWIR, _I8, _R, 4 }, { &tlcs900h_device::_CHGWIR, _I8, _R, 4 }, { &tlcs900h_device::_BITWIR, _I8, _R, 4 },
	{ &tlcs900h_device::_TSETWIR, _I8, _R, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_MINC1, _I16, _R, 8 }, { &tlcs900h_device::_MINC2, _I16, _R, 8 }, { &tlcs900h_device::_MINC4, _I16, _R, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_MDEC1, _I16, _R, 7 }, { &tlcs900h_device::_MDEC2, _I16, _R, 7 }, { &tlcs900h_device::_MDEC4, _I16, _R, 7 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_MULWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULWRR, _C32, _R, 26 },
	{ &tlcs900h_device::_MULWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULWRR, _C32, _R, 26 },
	{ &tlcs900h_device::_MULSWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULSWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULSWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULSWRR, _C32, _R, 26 },
	{ &tlcs900h_device::_MULSWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULSWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULSWRR, _C32, _R, 26 }, { &tlcs900h_device::_MULSWRR, _C32, _R, 26 },
	{ &tlcs900h_device::_DIVWRR, _C32, _R, 30 }, { &tlcs900h_device::_DIVWRR, _C32, _R, 30 }, { &tlcs900h_device::_DIVWRR, _C32, _R, 30 }, { &tlcs900h_device::_DIVWRR, _C32, _R, 30 },
	{ &tlcs900h_device::_DIVWRR, _C32, _R, 30 }, { &tlcs900h_device::_DIVWRR, _C32, _R, 30 }, { &tlcs900h_device::_DIVWRR, _C32, _R, 30 }, { &tlcs900h_device::_DIVWRR, _C32, _R, 30 },
	{ &tlcs900h_device::_DIVSWRR, _C32, _R, 32 }, { &tlcs900h_device::_DIVSWRR, _C32, _R, 32 }, { &tlcs900h_device::_DIVSWRR, _C32, _R, 32 }, { &tlcs900h_device::_DIVSWRR, _C32, _R, 32 },
	{ &tlcs900h_device::_DIVSWRR, _C32, _R, 32 }, { &tlcs900h_device::_DIVSWRR, _C32, _R, 32 }, { &tlcs900h_device::_DIVSWRR, _C32, _R, 32 }, { &tlcs900h_device::_DIVSWRR, _C32, _R, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCWIR, _I3, _R, 4 }, { &tlcs900h_device::_INCWIR, _I3, _R, 4 }, { &tlcs900h_device::_INCWIR, _I3, _R, 4 }, { &tlcs900h_device::_INCWIR, _I3, _R, 4 },
	{ &tlcs900h_device::_INCWIR, _I3, _R, 4 }, { &tlcs900h_device::_INCWIR, _I3, _R, 4 }, { &tlcs900h_device::_INCWIR, _I3, _R, 4 }, { &tlcs900h_device::_INCWIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DECWIR, _I3, _R, 4 }, { &tlcs900h_device::_DECWIR, _I3, _R, 4 }, { &tlcs900h_device::_DECWIR, _I3, _R, 4 }, { &tlcs900h_device::_DECWIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DECWIR, _I3, _R, 4 }, { &tlcs900h_device::_DECWIR, _I3, _R, 4 }, { &tlcs900h_device::_DECWIR, _I3, _R, 4 }, { &tlcs900h_device::_DECWIR, _I3, _R, 4 },
	{ &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 },
	{ &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 },
	{ &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 },
	{ &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 }, { &tlcs900h_device::_SCCWR, _CC, _R, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADDWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_ADDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADDWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_LDWRR, _C16, _R, 4 }, { &tlcs900h_device::_LDWRR, _C16, _R, 4 }, { &tlcs900h_device::_LDWRR, _C16, _R, 4 }, { &tlcs900h_device::_LDWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_LDWRR, _C16, _R, 4 }, { &tlcs900h_device::_LDWRR, _C16, _R, 4 }, { &tlcs900h_device::_LDWRR, _C16, _R, 4 }, { &tlcs900h_device::_LDWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_ADCWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADCWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADCWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADCWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_ADCWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADCWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADCWRR, _C16, _R, 4 }, { &tlcs900h_device::_ADCWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_LDWRR, _R, _C16, 4 }, { &tlcs900h_device::_LDWRR, _R, _C16, 4 }, { &tlcs900h_device::_LDWRR, _R, _C16, 4 }, { &tlcs900h_device::_LDWRR, _R, _C16, 4 },
	{ &tlcs900h_device::_LDWRR, _R, _C16, 4 }, { &tlcs900h_device::_LDWRR, _R, _C16, 4 }, { &tlcs900h_device::_LDWRR, _R, _C16, 4 }, { &tlcs900h_device::_LDWRR, _R, _C16, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBWRR, _C16, _R, 4 }, { &tlcs900h_device::_SUBWRR, _C16, _R, 4 }, { &tlcs900h_device::_SUBWRR, _C16, _R, 4 }, { &tlcs900h_device::_SUBWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_SUBWRR, _C16, _R, 4 }, { &tlcs900h_device::_SUBWRR, _C16, _R, 4 }, { &tlcs900h_device::_SUBWRR, _C16, _R, 4 }, { &tlcs900h_device::_SUBWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_LDWRI, _R, _I3, 4 }, { &tlcs900h_device::_LDWRI, _R, _I3, 4 }, { &tlcs900h_device::_LDWRI, _R, _I3, 4 }, { &tlcs900h_device::_LDWRI, _R, _I3, 4 },
	{ &tlcs900h_device::_LDWRI, _R, _I3, 4 }, { &tlcs900h_device::_LDWRI, _R, _I3, 4 }, { &tlcs900h_device::_LDWRI, _R, _I3, 4 }, { &tlcs900h_device::_LDWRI, _R, _I3, 4 },
	{ &tlcs900h_device::_SBCWRR, _C16, _R, 4 }, { &tlcs900h_device::_SBCWRR, _C16, _R, 4 }, { &tlcs900h_device::_SBCWRR, _C16, _R, 4 }, { &tlcs900h_device::_SBCWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_SBCWRR, _C16, _R, 4 }, { &tlcs900h_device::_SBCWRR, _C16, _R, 4 }, { &tlcs900h_device::_SBCWRR, _C16, _R, 4 }, { &tlcs900h_device::_SBCWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_EXWRR, _C16, _R, 5 }, { &tlcs900h_device::_EXWRR, _C16, _R, 5 }, { &tlcs900h_device::_EXWRR, _C16, _R, 5 }, { &tlcs900h_device::_EXWRR, _C16, _R, 5 },
	{ &tlcs900h_device::_EXWRR, _C16, _R, 5 }, { &tlcs900h_device::_EXWRR, _C16, _R, 5 }, { &tlcs900h_device::_EXWRR, _C16, _R, 5 }, { &tlcs900h_device::_EXWRR, _C16, _R, 5 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ANDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ANDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ANDWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_ANDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ANDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ANDWRR, _C16, _R, 4 }, { &tlcs900h_device::_ANDWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_ADDWRI, _R, _I16, 4 }, { &tlcs900h_device::_ADCWRI, _R, _I16, 4 }, { &tlcs900h_device::_SUBWRI, _R, _I16, 4 }, { &tlcs900h_device::_SBCWRI, _R, _I16, 4 },
	{ &tlcs900h_device::_ANDWRI, _R, _I16, 4 }, { &tlcs900h_device::_XORWRI, _R, _I16, 4 }, { &tlcs900h_device::_ORWRI, _R, _I16, 4 }, { &tlcs900h_device::_CPWRI, _R, _I16, 4 },
	{ &tlcs900h_device::_XORWRR, _C16, _R, 4 }, { &tlcs900h_device::_XORWRR, _C16, _R, 4 }, { &tlcs900h_device::_XORWRR, _C16, _R, 4 }, { &tlcs900h_device::_XORWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_XORWRR, _C16, _R, 4 }, { &tlcs900h_device::_XORWRR, _C16, _R, 4 }, { &tlcs900h_device::_XORWRR, _C16, _R, 4 }, { &tlcs900h_device::_XORWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_CPWRI, _R, _I3, 4 }, { &tlcs900h_device::_CPWRI, _R, _I3, 4 }, { &tlcs900h_device::_CPWRI, _R, _I3, 4 }, { &tlcs900h_device::_CPWRI, _R, _I3, 4 },
	{ &tlcs900h_device::_CPWRI, _R, _I3, 4 }, { &tlcs900h_device::_CPWRI, _R, _I3, 4 }, { &tlcs900h_device::_CPWRI, _R, _I3, 4 }, { &tlcs900h_device::_CPWRI, _R, _I3, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORWRR, _C16, _R, 4 }, { &tlcs900h_device::_ORWRR, _C16, _R, 4 }, { &tlcs900h_device::_ORWRR, _C16, _R, 4 }, { &tlcs900h_device::_ORWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_ORWRR, _C16, _R, 4 }, { &tlcs900h_device::_ORWRR, _C16, _R, 4 }, { &tlcs900h_device::_ORWRR, _C16, _R, 4 }, { &tlcs900h_device::_ORWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_RLCWIR, _I8, _R, 6 }, { &tlcs900h_device::_RRCWIR, _I8, _R, 6 }, { &tlcs900h_device::_RLWIR, _I8, _R, 6 }, { &tlcs900h_device::_RRWIR, _I8, _R, 6 },
	{ &tlcs900h_device::_SLAWIR, _I8, _R, 6 }, { &tlcs900h_device::_SRAWIR, _I8, _R, 6 }, { &tlcs900h_device::_SLLWIR, _I8, _R, 6 }, { &tlcs900h_device::_SRLWIR, _I8, _R, 6 },
	{ &tlcs900h_device::_CPWRR, _C16, _R, 4 }, { &tlcs900h_device::_CPWRR, _C16, _R, 4 }, { &tlcs900h_device::_CPWRR, _C16, _R, 4 }, { &tlcs900h_device::_CPWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_CPWRR, _C16, _R, 4 }, { &tlcs900h_device::_CPWRR, _C16, _R, 4 }, { &tlcs900h_device::_CPWRR, _C16, _R, 4 }, { &tlcs900h_device::_CPWRR, _C16, _R, 4 },
	{ &tlcs900h_device::_RLCWRR, _A, _R, 6 }, { &tlcs900h_device::_RRCWRR, _A, _R, 6 }, { &tlcs900h_device::_RLWRR, _A, _R, 6 }, { &tlcs900h_device::_RRWRR, _A, _R, 6 },
	{ &tlcs900h_device::_SLAWRR, _A, _R, 6 }, { &tlcs900h_device::_SRAWRR, _A, _R, 6 }, { &tlcs900h_device::_SLLWRR, _A, _R, 6 }, { &tlcs900h_device::_SRLWRR, _A, _R, 6 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 }, { &tlcs900h_device::_LDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 }, { &tlcs900h_device::_ADCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 }, { &tlcs900h_device::_ADCLMR, _M, _C32, 10 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 }, { &tlcs900h_device::_SUBLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 }, { &tlcs900h_device::_SUBLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 }, { &tlcs900h_device::_SBCLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 }, { &tlcs900h_device::_SBCLMR, _M, _C32, 10 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 }, { &tlcs900h_device::_ANDLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 }, { &tlcs900h_device::_ANDLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 }, { &tlcs900h_device::_XORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 }, { &tlcs900h_device::_XORLMR, _M, _C32, 10 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 }, { &tlcs900h_device::_ORLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 }, { &tlcs900h_device::_ORLMR, _M, _C32, 10 },
	{ &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 }, { &tlcs900h_device::_CPLRM, _C32, _M, 6 },
	{ &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 }, { &tlcs900h_device::_CPLMR, _M, _C32, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDLRI, _R, _I32, 6 },
	{ &tlcs900h_device::_PUSHLR, _R, 0, 7 }, { &tlcs900h_device::_POPLR, _R, 0, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LINK, _R, _I16, 10 }, { &tlcs900h_device::_UNLK, _R, 0, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_EXTZLR, _R, 0, 4 }, { &tlcs900h_device::_EXTSLR, _R, 0, 5 },
	{ &tlcs900h_device::_PAALR, _R, 0, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDCLRR, _CR32, _R, 1 }, { &tlcs900h_device::_LDCLRR, _R, _CR32, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::_INCLIR, _I3, _R, 4 }, { &tlcs900h_device::_INCLIR, _I3, _R, 4 }, { &tlcs900h_device::_INCLIR, _I3, _R, 4 }, { &tlcs900h_device::_INCLIR, _I3, _R, 4 },
	{ &tlcs900h_device::_INCLIR, _I3, _R, 4 }, { &tlcs900h_device::_INCLIR, _I3, _R, 4 }, { &tlcs900h_device::_INCLIR, _I3, _R, 4 }, { &tlcs900h_device::_INCLIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DECLIR, _I3, _R, 4 }, { &tlcs900h_device::_DECLIR, _I3, _R, 4 }, { &tlcs900h_device::_DECLIR, _I3, _R, 4 }, { &tlcs900h_device::_DECLIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DECLIR, _I3, _R, 4 }, { &tlcs900h_device::_DECLIR, _I3, _R, 4 }, { &tlcs900h_device::_DECLIR, _I3, _R, 4 }, { &tlcs900h_device::_DECLIR, _I3, _R, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ADDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADDLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_ADDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADDLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_LDLRR, _C32, _R, 4 }, { &tlcs900h_device::_LDLRR, _C32, _R, 4 }, { &tlcs900h_device::_LDLRR, _C32, _R, 4 }, { &tlcs900h_device::_LDLRR, _C32, _R, 4 },
	{ &tlcs900h_device::_LDLRR, _C32, _R, 4 }, { &tlcs900h_device::_LDLRR, _C32, _R, 4 }, { &tlcs900h_device::_LDLRR, _C32, _R, 4 }, { &tlcs900h_device::_LDLRR, _C32, _R, 4 },
	{ &tlcs900h_device::_ADCLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADCLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADCLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADCLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_ADCLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADCLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADCLRR, _C32, _R, 7 }, { &tlcs900h_device::_ADCLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_LDLRR, _R, _C32, 4 }, { &tlcs900h_device::_LDLRR, _R, _C32, 4 }, { &tlcs900h_device::_LDLRR, _R, _C32, 4 }, { &tlcs900h_device::_LDLRR, _R, _C32, 4 },
	{ &tlcs900h_device::_LDLRR, _R, _C32, 4 }, { &tlcs900h_device::_LDLRR, _R, _C32, 4 }, { &tlcs900h_device::_LDLRR, _R, _C32, 4 }, { &tlcs900h_device::_LDLRR, _R, _C32, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::_SUBLRR, _C32, _R, 7 }, { &tlcs900h_device::_SUBLRR, _C32, _R, 7 }, { &tlcs900h_device::_SUBLRR, _C32, _R, 7 }, { &tlcs900h_device::_SUBLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_SUBLRR, _C32, _R, 7 }, { &tlcs900h_device::_SUBLRR, _C32, _R, 7 }, { &tlcs900h_device::_SUBLRR, _C32, _R, 7 }, { &tlcs900h_device::_SUBLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_LDLRI, _R, _I3, 4 }, { &tlcs900h_device::_LDLRI, _R, _I3, 4 }, { &tlcs900h_device::_LDLRI, _R, _I3, 4 }, { &tlcs900h_device::_LDLRI, _R, _I3, 4 },
	{ &tlcs900h_device::_LDLRI, _R, _I3, 4 }, { &tlcs900h_device::_LDLRI, _R, _I3, 4 }, { &tlcs900h_device::_LDLRI, _R, _I3, 4 }, { &tlcs900h_device::_LDLRI, _R, _I3, 4 },
	{ &tlcs900h_device::_SBCLRR, _C32, _R, 7 }, { &tlcs900h_device::_SBCLRR, _C32, _R, 7 }, { &tlcs900h_device::_SBCLRR, _C32, _R, 7 }, { &tlcs900h_device::_SBCLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_SBCLRR, _C32, _R, 7 }, { &tlcs900h_device::_SBCLRR, _C32, _R, 7 }, { &tlcs900h_device::_SBCLRR, _C32, _R, 7 }, { &tlcs900h_device::_SBCLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* C0 - DF */
	{ &tlcs900h_device::_ANDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ANDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ANDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ANDLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_ANDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ANDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ANDLRR, _C32, _R, 7 }, { &tlcs900h_device::_ANDLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_ADDLRI, _R, _I32, 7 }, { &tlcs900h_device::_ADCLRI, _R, _I32, 7 }, { &tlcs900h_device::_SUBLRI, _R, _I32, 7 }, { &tlcs900h_device::_SBCLRI, _R, _I32, 7 },
	{ &tlcs900h_device::_ANDLRI, _R, _I32, 7 }, { &tlcs900h_device::_XORLRI, _R, _I32, 7 }, { &tlcs900h_device::_ORLRI, _R, _I32, 7 }, { &tlcs900h_device::_CPLRI, _R, _I32, 7 },
	{ &tlcs900h_device::_XORLRR, _C32, _R, 7 }, { &tlcs900h_device::_XORLRR, _C32, _R, 7 }, { &tlcs900h_device::_XORLRR, _C32, _R, 7 }, { &tlcs900h_device::_XORLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_XORLRR, _C32, _R, 7 }, { &tlcs900h_device::_XORLRR, _C32, _R, 7 }, { &tlcs900h_device::_XORLRR, _C32, _R, 7 }, { &tlcs900h_device::_XORLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_CPLRI, _R, _I3, 6 }, { &tlcs900h_device::_CPLRI, _R, _I3, 6 }, { &tlcs900h_device::_CPLRI, _R, _I3, 6 }, { &tlcs900h_device::_CPLRI, _R, _I3, 6 },
	{ &tlcs900h_device::_CPLRI, _R, _I3, 6 }, { &tlcs900h_device::_CPLRI, _R, _I3, 6 }, { &tlcs900h_device::_CPLRI, _R, _I3, 6 }, { &tlcs900h_device::_CPLRI, _R, _I3, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::_ORLRR, _C32, _R, 7 }, { &tlcs900h_device::_ORLRR, _C32, _R, 7 }, { &tlcs900h_device::_ORLRR, _C32, _R, 7 }, { &tlcs900h_device::_ORLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_ORLRR, _C32, _R, 7 }, { &tlcs900h_device::_ORLRR, _C32, _R, 7 }, { &tlcs900h_device::_ORLRR, _C32, _R, 7 }, { &tlcs900h_device::_ORLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_RLCLIR, _I8, _R, 8 }, { &tlcs900h_device::_RRCLIR, _I8, _R, 8 }, { &tlcs900h_device::_RLLIR, _I8, _R, 8 }, { &tlcs900h_device::_RRLIR, _I8, _R, 8 },
	{ &tlcs900h_device::_SLALIR, _I8, _R, 8 }, { &tlcs900h_device::_SRALIR, _I8, _R, 8 }, { &tlcs900h_device::_SLLLIR, _I8, _R, 8 }, { &tlcs900h_device::_SRLLIR, _I8, _R, 8 },
	{ &tlcs900h_device::_CPLRR, _C32, _R, 7 }, { &tlcs900h_device::_CPLRR, _C32, _R, 7 }, { &tlcs900h_device::_CPLRR, _C32, _R, 7 }, { &tlcs900h_device::_CPLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_CPLRR, _C32, _R, 7 }, { &tlcs900h_device::_CPLRR, _C32, _R, 7 }, { &tlcs900h_device::_CPLRR, _C32, _R, 7 }, { &tlcs900h_device::_CPLRR, _C32, _R, 7 },
	{ &tlcs900h_device::_RLCLRR, _A, _R, 8 }, { &tlcs900h_device::_RRCLRR, _A, _R, 8 }, { &tlcs900h_device::_RLLRR, _A, _R, 8 }, { &tlcs900h_device::_RRLRR, _A, _R, 8 },
	{ &tlcs900h_device::_SLALRR, _A, _R, 8 }, { &tlcs900h_device::_SRALRR, _A, _R, 8 }, { &tlcs900h_device::_SLLLRR, _A, _R, 8 }, { &tlcs900h_device::_SRLLRR, _A, _R, 8 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_LDBMI, _M, _I8, 5 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMI, _M, _I16, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_POPBM, _M, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_POPWM, _M, 0, 6 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDBMM, _M, _M16, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_LDWMM, _M, _M16, 8 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 },
	{ &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 }, { &tlcs900h_device::_LDAW, _C16, _M, 4 },
	{ &tlcs900h_device::_ANDCFBRM, _A, _M, 4 }, { &tlcs900h_device::_ORCFBRM, _A, _M, 4 }, { &tlcs900h_device::_XORCFBRM, _A, _M, 4 }, { &tlcs900h_device::_LDCFBRM, _A, _M, 4 },
	{ &tlcs900h_device::_STCFBRM, _A, _M, 4 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 },
	{ &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 }, { &tlcs900h_device::_LDAL, _C32, _M, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 },
	{ &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 }, { &tlcs900h_device::_LDBMR, _M, _C8, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 },
	{ &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 }, { &tlcs900h_device::_LDWMR, _M, _C16, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 }, { &tlcs900h_device::_LDLMR, _M, _C32, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ANDCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_ORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_ORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_ORCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_XORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_XORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_XORCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_LDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_LDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_LDCFBIM, _I3, _M, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::_STCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_STCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 4 }, { &tlcs900h_device::_STCFBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 },
	{ &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 }, { &tlcs900h_device::_TSETBIM, _I3, _M, 10 },
	{ &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 }, { &tlcs900h_device::_RESBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 }, { &tlcs900h_device::_SETBIM, _I3, _M, 8 },

	/* C0 - DF */
	{ &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 }, { &tlcs900h_device::_CHGBIM, _I3, _M, 8 },
	{ &tlcs900h_device::_BITBIM, _I3, _M, 4 }, { &tlcs900h_device::_BITBIM, _I3, _M, 4 }, { &tlcs900h_device::_BITBIM, _I3, _M, 4 }, { &tlcs900h_device::_BITBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_BITBIM, _I3, _M, 4 }, { &tlcs900h_device::_BITBIM, _I3, _M, 4 }, { &tlcs900h_device::_BITBIM, _I3, _M, 4 }, { &tlcs900h_device::_BITBIM, _I3, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },
	{ &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 }, { &tlcs900h_device::_JPM, _CC, _M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 }, { &tlcs900h_device::_CALLM, _CC, _M, 6 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }
};


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in byte operations */
void tlcs900h_device::_80()
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_p1_reg32 = get_reg32_current( m_op - 1 );
	m_p2_reg32 = get_reg32_current( m_op );

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	inst = &s_mnemonic_80[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in byte operations */
void tlcs900h_device::_88()
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_p1_reg32 = get_reg32_current( m_op - 1 );
	m_p2_reg32 = get_reg32_current( m_op );

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (INT8)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_80[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIXI/XIY/XIZ/XSP) used as source in word operations */
void tlcs900h_device::_90()
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_p1_reg32 = get_reg32_current( m_op - 1 );
	m_p2_reg32 = get_reg32_current( m_op );

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	inst = &s_mnemonic_90[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in word operations */
void tlcs900h_device::_98()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (INT8)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_98[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in long word operations */
void tlcs900h_device::_A0()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	inst = &s_mnemonic_a0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as source in long word operations */
void tlcs900h_device::_A8()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (INT8)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_a0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as destination in operations */
void tlcs900h_device::_B0()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	inst = &s_mnemonic_b0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP + d8) used as destination in operations */
void tlcs900h_device::_B8()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (INT8)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_b8[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* memory used as source in byte operations */
void tlcs900h_device::_C0()
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( m_op & 0x07 )
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += 2;
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += 3;
		break;

	case 0x03:
		m_op = RDOP();
		switch ( m_op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = *get_reg32( m_op );
			m_cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = *get_reg32( m_op ) + m_ea2.sw.l;
			m_cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( m_op )
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT8) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT16) *get_reg16( m_op );
				m_cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		*reg -= ( 1 << ( m_op & 0x03 ) );
		m_ea2.d = *reg;
		m_cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		m_ea2.d = *reg;
		*reg += ( 1 << ( m_op & 0x03 ) );
		m_cycles += 3;
		break;
	}
	m_op = RDOP();
	inst = &s_mnemonic_c0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


void tlcs900h_device::oC8()
{
	const tlcs900inst *inst;

	if ( m_op & 0x08 )
	{
		m_p2_reg8 = get_reg8_current( m_op );
		/* For MUL and DIV operations */
		m_p2_reg16 = get_reg16_current( ( m_op >> 1 ) & 0x03 );
	}
	else
	{
		m_op = RDOP();
		m_p2_reg8 = get_reg8( m_op );
		/* For MUL and DIV operations */
		m_p2_reg16 = get_reg16( m_op );
	}
	m_op = RDOP();
	inst = &s_mnemonic_c8[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* memory used as source in word operations */
void tlcs900h_device::_D0()
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( m_op & 0x07 )
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += 2;
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += 3;
		break;

	case 0x03:
		m_op = RDOP();
		switch ( m_op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = *get_reg32( m_op );
			m_cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = *get_reg32( m_op ) + m_ea2.sw.l;
			m_cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( m_op )
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT8) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT16) *get_reg16( m_op );
				m_cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		*reg -= ( 1 << ( m_op & 0x03 ) );
		m_ea2.d = *reg;
		m_cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		m_ea2.d = *reg;
		*reg += ( 1 << ( m_op & 0x03 ) );
		m_cycles += 3;
		break;
	}
	m_op = RDOP();
	inst = &s_mnemonic_d0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


void tlcs900h_device::oD8()
{
	const tlcs900inst *inst;

	if ( m_op & 0x08 )
	{
		m_p2_reg16 = get_reg16_current( m_op );
		m_p2_reg32 = get_reg32_current( m_op );
	}
	else
	{
		m_op = RDOP();
		m_p2_reg16 = get_reg16( m_op );
		m_p2_reg32 = get_reg32( m_op );
	}
	m_op = RDOP();
	inst = &s_mnemonic_d8[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* memory used as source in long word operations */
void tlcs900h_device::_E0()
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( m_op & 0x07 )
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += 2;
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += 3;
		break;

	case 0x03:
		m_op = RDOP();
		switch ( m_op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = *get_reg32( m_op );
			m_cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = *get_reg32( m_op ) + m_ea2.sw.l;
			m_cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( m_op )
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT8) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT16) *get_reg16( m_op );
				m_cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		*reg -= ( 1 << ( m_op & 0x03 ) );
		m_ea2.d = *reg;
		m_cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		m_ea2.d = *reg;
		*reg += ( 1 << ( m_op & 0x03 ) );
		m_cycles += 3;
		break;
	}
	m_op = RDOP();
	inst = &s_mnemonic_e0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


void tlcs900h_device::_E8()
{
	const tlcs900inst *inst;

	if ( m_op & 0x08 )
	{
		m_p2_reg32 = get_reg32_current( m_op );
	}
	else
	{
		m_op = RDOP();
		m_p2_reg32 = get_reg32( m_op );
	}
	m_op = RDOP();
	inst = &s_mnemonic_e8[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* memory used as destination operations */
void tlcs900h_device::_F0()
{
	const tlcs900inst *inst;
	UINT32 *reg = NULL;

	switch ( m_op & 0x07 )
	{
	case 0x00:  /* (n) */
		m_ea2.d = RDOP();
		m_cycles += 2;
		break;

	case 0x01:  /* (nn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_cycles += 2;
		break;

	case 0x02:  /* (nnn) */
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.b.h2 = RDOP();
		m_cycles += 3;
		break;

	case 0x03:
		m_op = RDOP();
		switch ( m_op & 0x03 )
		{
		/* (xrr) */
		case 0x00:
			m_ea2.d = *get_reg32( m_op );
			m_cycles += 5;
			break;

		/* (xrr+d16) */
		case 0x01:
			m_ea2.b.l = RDOP();
			m_ea2.b.h = RDOP();
			m_ea2.d = *get_reg32( m_op ) + m_ea2.sw.l;
			m_cycles += 5;
			break;

		/* unknown/illegal */
		case 0x02:
			break;

		case 0x03:
			switch ( m_op )
			{
			/* (xrr+r8) */
			case 0x03:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT8) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (INT16) *get_reg16( m_op );
				m_cycles += 8;
				break;

			/* (pc+d16) */
			case 0x13:
				m_ea2.b.l = RDOP();
				m_ea2.b.h = RDOP();
				m_ea2.d = m_pc.d + m_ea2.sw.l;
				m_cycles += 5;
				break;
			}
		}
		break;

	case 0x04:  /* (-xrr) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		*reg -= ( 1 << ( m_op & 0x03 ) );
		m_ea2.d = *reg;
		m_cycles += 3;
		break;

	case 0x05:  /* (xrr+) */
		m_op = RDOP();
		reg = get_reg32( m_op );
		m_ea2.d = *reg;
		*reg += ( 1 << ( m_op & 0x03 ) );
		m_cycles += 3;
		break;
	}

	m_op = RDOP();
	inst = &s_mnemonic_f0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::_NOP, 0, 0, 1 }, { &tlcs900h_device::_NORMAL, 0, 0, 4 }, { &tlcs900h_device::_PUSHWR, _SR, 0, 4 }, { &tlcs900h_device::_POPWSR, _SR, 0, 6 },
	{ &tlcs900h_device::_MAX, 0, 0, 4 }, { &tlcs900h_device::_HALT, 0, 0, 8 }, { &tlcs900h_device::_EI, _I8, 0, 5 }, { &tlcs900h_device::_RETI, 0, 0, 12 },
	{ &tlcs900h_device::_LDBMI, _M8, _I8, 5 }, { &tlcs900h_device::_PUSHBI, _I8, 0, 4 }, { &tlcs900h_device::_LDWMI, _M8, _I16, 6 }, { &tlcs900h_device::_PUSHWI, _I16, 0, 5 },
	{ &tlcs900h_device::_INCF, 0, 0, 2 }, { &tlcs900h_device::_DECF, 0, 0, 2 }, { &tlcs900h_device::_RET, 0, 0, 9 }, { &tlcs900h_device::_RETD, _I16, 0, 9 },
	{ &tlcs900h_device::_RCF, 0, 0, 2 }, { &tlcs900h_device::_SCF, 0, 0, 2 }, { &tlcs900h_device::_CCF, 0, 0, 2 }, { &tlcs900h_device::_ZCF, 0, 0, 2 },
	{ &tlcs900h_device::_PUSHBR, _A, 0, 3 }, { &tlcs900h_device::_POPBR, _A, 0, 4 }, { &tlcs900h_device::_EXBRR, _F, _F, 2 }, { &tlcs900h_device::_LDF, _I8, 0, 2 },
	{ &tlcs900h_device::_PUSHBR, _F, 0, 3 }, { &tlcs900h_device::_POPBR, _F, 0, 4 }, { &tlcs900h_device::_JPI, _I16, 0, 7 }, { &tlcs900h_device::_JPI, _I24, 0, 7 },
	{ &tlcs900h_device::_CALLI, _I16, 0, 12 }, { &tlcs900h_device::_CALLI, _I24, 0, 12 }, { &tlcs900h_device::_CALR, _D16, 0, 12 }, { &tlcs900h_device::_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::_LDBRI, _C8, _I8, 2 }, { &tlcs900h_device::_LDBRI, _C8, _I8, 2 }, { &tlcs900h_device::_LDBRI, _C8, _I8, 2 }, { &tlcs900h_device::_LDBRI, _C8, _I8, 2 },
	{ &tlcs900h_device::_LDBRI, _C8, _I8, 2 }, { &tlcs900h_device::_LDBRI, _C8, _I8, 2 }, { &tlcs900h_device::_LDBRI, _C8, _I8, 2 }, { &tlcs900h_device::_LDBRI, _C8, _I8, 2 },
	{ &tlcs900h_device::_PUSHWR, _C16, 0, 3 }, { &tlcs900h_device::_PUSHWR, _C16, 0, 3 }, { &tlcs900h_device::_PUSHWR, _C16, 0, 3 }, { &tlcs900h_device::_PUSHWR, _C16, 0, 3 },
	{ &tlcs900h_device::_PUSHWR, _C16, 0, 3 }, { &tlcs900h_device::_PUSHWR, _C16, 0, 3 }, { &tlcs900h_device::_PUSHWR, _C16, 0, 3 }, { &tlcs900h_device::_PUSHWR, _C16, 0, 3 },
	{ &tlcs900h_device::_LDWRI, _C16, _I16, 3 }, { &tlcs900h_device::_LDWRI, _C16, _I16, 3 }, { &tlcs900h_device::_LDWRI, _C16, _I16, 3 }, { &tlcs900h_device::_LDWRI, _C16, _I16, 3 },
	{ &tlcs900h_device::_LDWRI, _C16, _I16, 3 }, { &tlcs900h_device::_LDWRI, _C16, _I16, 3 }, { &tlcs900h_device::_LDWRI, _C16, _I16, 3 }, { &tlcs900h_device::_LDWRI, _C16, _I16, 3 },
	{ &tlcs900h_device::_PUSHLR, _C32, 0, 5 }, { &tlcs900h_device::_PUSHLR, _C32, 0, 5 }, { &tlcs900h_device::_PUSHLR, _C32, 0, 5 }, { &tlcs900h_device::_PUSHLR, _C32, 0, 5 },
	{ &tlcs900h_device::_PUSHLR, _C32, 0, 5 }, { &tlcs900h_device::_PUSHLR, _C32, 0, 5 }, { &tlcs900h_device::_PUSHLR, _C32, 0, 5 }, { &tlcs900h_device::_PUSHLR, _C32, 0, 5 },

	/* 40 - 5F */
	{ &tlcs900h_device::_LDLRI, _C32, _I32, 5 }, { &tlcs900h_device::_LDLRI, _C32, _I32, 5 }, { &tlcs900h_device::_LDLRI, _C32, _I32, 5 }, { &tlcs900h_device::_LDLRI, _C32, _I32, 5 },
	{ &tlcs900h_device::_LDLRI, _C32, _I32, 5 }, { &tlcs900h_device::_LDLRI, _C32, _I32, 5 }, { &tlcs900h_device::_LDLRI, _C32, _I32, 5 }, { &tlcs900h_device::_LDLRI, _C32, _I32, 5 },
	{ &tlcs900h_device::_POPWR, _C16, 0, 4 }, { &tlcs900h_device::_POPWR, _C16, 0, 4 }, { &tlcs900h_device::_POPWR, _C16, 0, 4 }, { &tlcs900h_device::_POPWR, _C16, 0, 4 },
	{ &tlcs900h_device::_POPWR, _C16, 0, 4 }, { &tlcs900h_device::_POPWR, _C16, 0, 4 }, { &tlcs900h_device::_POPWR, _C16, 0, 4 }, { &tlcs900h_device::_POPWR, _C16, 0, 4 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 }, { &tlcs900h_device::_DB, 0, 0, 1 },
	{ &tlcs900h_device::_POPLR, _C32, 0, 6 }, { &tlcs900h_device::_POPLR, _C32, 0, 6 }, { &tlcs900h_device::_POPLR, _C32, 0, 6 }, { &tlcs900h_device::_POPLR, _C32, 0, 6 },
	{ &tlcs900h_device::_POPLR, _C32, 0, 6 }, { &tlcs900h_device::_POPLR, _C32, 0, 6 }, { &tlcs900h_device::_POPLR, _C32, 0, 6 }, { &tlcs900h_device::_POPLR, _C32, 0, 6 },

	/* 60 - 7F */
	{ &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 },
	{ &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 },
	{ &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 },
	{ &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 }, { &tlcs900h_device::_JR, _CC, _D8, 4 },
	{ &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 },
	{ &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 },
	{ &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 },
	{ &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 }, { &tlcs900h_device::_JRL, _CC, _D16, 4 },

	/* 80 - 9F */
	{ &tlcs900h_device::_80, 0, 0, 0 }, { &tlcs900h_device::_80, 0, 0, 0 }, { &tlcs900h_device::_80, 0, 0, 0 }, { &tlcs900h_device::_80, 0, 0, 0 },
	{ &tlcs900h_device::_80, 0, 0, 0 }, { &tlcs900h_device::_80, 0, 0, 0 }, { &tlcs900h_device::_80, 0, 0, 0 }, { &tlcs900h_device::_80, 0, 0, 0 },
	{ &tlcs900h_device::_88, 0, 0, 0 }, { &tlcs900h_device::_88, 0, 0, 0 }, { &tlcs900h_device::_88, 0, 0, 0 }, { &tlcs900h_device::_88, 0, 0, 0 },
	{ &tlcs900h_device::_88, 0, 0, 0 }, { &tlcs900h_device::_88, 0, 0, 0 }, { &tlcs900h_device::_88, 0, 0, 0 }, { &tlcs900h_device::_88, 0, 0, 0 },
	{ &tlcs900h_device::_90, 0, 0, 0 }, { &tlcs900h_device::_90, 0, 0, 0 }, { &tlcs900h_device::_90, 0, 0, 0 }, { &tlcs900h_device::_90, 0, 0, 0 },
	{ &tlcs900h_device::_90, 0, 0, 0 }, { &tlcs900h_device::_90, 0, 0, 0 }, { &tlcs900h_device::_90, 0, 0, 0 }, { &tlcs900h_device::_90, 0, 0, 0 },
	{ &tlcs900h_device::_98, 0, 0, 0 }, { &tlcs900h_device::_98, 0, 0, 0 }, { &tlcs900h_device::_98, 0, 0, 0 }, { &tlcs900h_device::_98, 0, 0, 0 },
	{ &tlcs900h_device::_98, 0, 0, 0 }, { &tlcs900h_device::_98, 0, 0, 0 }, { &tlcs900h_device::_98, 0, 0, 0 }, { &tlcs900h_device::_98, 0, 0, 0 },

	/* A0 - BF */
	{ &tlcs900h_device::_A0, 0, 0, 0 }, { &tlcs900h_device::_A0, 0, 0, 0 }, { &tlcs900h_device::_A0, 0, 0, 0 }, { &tlcs900h_device::_A0, 0, 0, 0 },
	{ &tlcs900h_device::_A0, 0, 0, 0 }, { &tlcs900h_device::_A0, 0, 0, 0 }, { &tlcs900h_device::_A0, 0, 0, 0 }, { &tlcs900h_device::_A0, 0, 0, 0 },
	{ &tlcs900h_device::_A8, 0, 0, 0 }, { &tlcs900h_device::_A8, 0, 0, 0 }, { &tlcs900h_device::_A8, 0, 0, 0 }, { &tlcs900h_device::_A8, 0, 0, 0 },
	{ &tlcs900h_device::_A8, 0, 0, 0 }, { &tlcs900h_device::_A8, 0, 0, 0 }, { &tlcs900h_device::_A8, 0, 0, 0 }, { &tlcs900h_device::_A8, 0, 0, 0 },
	{ &tlcs900h_device::_B0, 0, 0, 0 }, { &tlcs900h_device::_B0, 0, 0, 0 }, { &tlcs900h_device::_B0, 0, 0, 0 }, { &tlcs900h_device::_B0, 0, 0, 0 },
	{ &tlcs900h_device::_B0, 0, 0, 0 }, { &tlcs900h_device::_B0, 0, 0, 0 }, { &tlcs900h_device::_B0, 0, 0, 0 }, { &tlcs900h_device::_B0, 0, 0, 0 },
	{ &tlcs900h_device::_B8, 0, 0, 0 }, { &tlcs900h_device::_B8, 0, 0, 0 }, { &tlcs900h_device::_B8, 0, 0, 0 }, { &tlcs900h_device::_B8, 0, 0, 0 },
	{ &tlcs900h_device::_B8, 0, 0, 0 }, { &tlcs900h_device::_B8, 0, 0, 0 }, { &tlcs900h_device::_B8, 0, 0, 0 }, { &tlcs900h_device::_B8, 0, 0, 0 },

	/* C0 - DF */
	{ &tlcs900h_device::_C0, 0, 0, 0 }, { &tlcs900h_device::_C0, 0, 0, 0 }, { &tlcs900h_device::_C0, 0, 0, 0 }, { &tlcs900h_device::_C0, 0, 0, 0 },
	{ &tlcs900h_device::_C0, 0, 0, 0 }, { &tlcs900h_device::_C0, 0, 0, 0 }, { &tlcs900h_device::_DB, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::_D0, 0, 0, 0 }, { &tlcs900h_device::_D0, 0, 0, 0 }, { &tlcs900h_device::_D0, 0, 0, 0 }, { &tlcs900h_device::_D0, 0, 0, 0 },
	{ &tlcs900h_device::_D0, 0, 0, 0 }, { &tlcs900h_device::_D0, 0, 0, 0 }, { &tlcs900h_device::_DB, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },
	{ &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },
	{ &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },

	/* E0 - FF */
	{ &tlcs900h_device::_E0, 0, 0, 0 }, { &tlcs900h_device::_E0, 0, 0, 0 }, { &tlcs900h_device::_E0, 0, 0, 0 }, { &tlcs900h_device::_E0, 0, 0, 0 },
	{ &tlcs900h_device::_E0, 0, 0, 0 }, { &tlcs900h_device::_E0, 0, 0, 0 }, { &tlcs900h_device::_DB, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 },
	{ &tlcs900h_device::_E8, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 },
	{ &tlcs900h_device::_E8, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 }, { &tlcs900h_device::_E8, 0, 0, 0 },
	{ &tlcs900h_device::_F0, 0, 0, 0 }, { &tlcs900h_device::_F0, 0, 0, 0 }, { &tlcs900h_device::_F0, 0, 0, 0 }, { &tlcs900h_device::_F0, 0, 0, 0 },
	{ &tlcs900h_device::_F0, 0, 0, 0 }, { &tlcs900h_device::_F0, 0, 0, 0 }, { &tlcs900h_device::_DB, 0, 0, 0 }, { &tlcs900h_device::_LDX, 0, 0, 9 },
	{ &tlcs900h_device::_SWI, _I3, 0, 16 }, { &tlcs900h_device::_SWI, _I3, 0, 16 }, { &tlcs900h_device::_SWI, _I3, 0, 16 }, { &tlcs900h_device::_SWI, _I3, 0, 16 },
	{ &tlcs900h_device::_SWI, _I3, 0, 16 }, { &tlcs900h_device::_SWI, _I3, 0, 16 }, { &tlcs900h_device::_SWI, _I3, 0, 16 }, { &tlcs900h_device::_SWI, _I3, 0, 16 }
};
