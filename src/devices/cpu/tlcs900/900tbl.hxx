// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

TLCS-900/H instruction set

*******************************************************************/


enum e_operand
{
	p_A=1,       /* current register set register A */
	p_C8,        /* current register set byte */
	p_C16,       /* current register set word */
	p_C32,       /* current register set long word */
	p_MC16,      /* current register set mul/div register word */
	p_CC,        /* condition */
	p_CR8,
	p_CR16,
	p_CR32,
	p_D8,        /* byte displacement */
	p_D16,       /* word displacement */
	p_F,         /* F register */
	p_I3,        /* immediate 3 bit (part of last byte) */
	p_I8,        /* immediate byte */
	p_I16,       /* immediate word */
	p_I24,       /* immediate 3 byte address */
	p_I32,       /* immediate long word */
	p_M,         /* memory location (defined by extension) */
	p_M8,        /* (8) */
	p_M16,       /* (i16) */
	p_R,         /* register (defined by extension) */
	p_SR         /* status register */
};


int tlcs900h_device::condition_true( uint8_t cond )
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


uint8_t* tlcs900h_device::get_reg8_current( uint8_t reg )
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


uint16_t* tlcs900h_device::get_reg16_current( uint8_t reg )
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


uint32_t* tlcs900h_device::get_reg32_current( uint8_t reg )
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


PAIR* tlcs900h_device::get_reg( uint8_t reg )
{
	uint8_t   regbank;

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


uint8_t* tlcs900h_device::get_reg8( uint8_t reg )
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


uint16_t* tlcs900h_device::get_reg16( uint8_t reg )
{
	PAIR    *r = get_reg( reg );

	return ( reg & 0x02 ) ? &r->w.h : &r->w.l;
}


uint32_t* tlcs900h_device::get_reg32( uint8_t reg )
{
	PAIR    *r = get_reg( reg );

	return &r->d;
}



void tlcs900h_device::parity8( uint8_t a )
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


void tlcs900h_device::parity16( uint16_t a )
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


void tlcs900h_device::parity32( uint32_t a )
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


uint8_t tlcs900h_device::adc8( uint8_t a, uint8_t b)
{
	uint8_t cy = m_sr.b.l & FLAG_CF;
	uint8_t result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


uint16_t tlcs900h_device::adc16( uint16_t a, uint16_t b)
{
	uint8_t cy = m_sr.b.l & FLAG_CF;
	uint16_t result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


uint32_t tlcs900h_device::adc32( uint32_t a, uint32_t b)
{
	uint8_t cy = m_sr.b.l & FLAG_CF;
	uint32_t result = a + b + cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( ( result < a ) || ( ( result == a ) && cy ) ) ? FLAG_CF : 0 );

	return result;
}


uint8_t tlcs900h_device::add8( uint8_t a, uint8_t b)
{
	uint8_t result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


uint16_t tlcs900h_device::add16( uint16_t a, uint16_t b)
{
	uint16_t result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


uint32_t tlcs900h_device::add32( uint32_t a, uint32_t b)
{
	uint32_t result = a + b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( result ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( result < a ) ? FLAG_CF : 0 );

	return result;
}


uint8_t tlcs900h_device::sbc8( uint8_t a, uint8_t b)
{
	uint8_t cy = m_sr.b.l & FLAG_CF;
	uint8_t result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


uint16_t tlcs900h_device::sbc16( uint16_t a, uint16_t b)
{
	uint8_t cy = m_sr.b.l & FLAG_CF;
	uint16_t result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFFFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


uint32_t tlcs900h_device::sbc32( uint32_t a, uint32_t b)
{
	uint8_t cy = m_sr.b.l & FLAG_CF;
	uint32_t result = a - b - cy;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( ( result > a ) || ( cy && b == 0xFFFFFFFF ) ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


uint8_t tlcs900h_device::sub8( uint8_t a, uint8_t b)
{
	uint8_t result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


uint16_t tlcs900h_device::sub16( uint16_t a, uint16_t b)
{
	uint16_t result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( a ^ b ) ^ result ) & FLAG_HF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x8000 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


uint32_t tlcs900h_device::sub32( uint32_t a, uint32_t b)
{
	uint32_t result = a - b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) |
		( ( ( result ^ a ) & ( a ^ b ) & 0x80000000 ) ? FLAG_VF : 0 ) |
		( ( result > a ) ? FLAG_CF : 0 ) | FLAG_NF;

	return result;
}


uint8_t tlcs900h_device::and8( uint8_t a, uint8_t b)
{
	uint8_t result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	parity8( result );

	return result;
}


uint16_t tlcs900h_device::and16( uint16_t a, uint16_t b)
{
	uint16_t result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	parity16( result );

	return result;
}


uint32_t tlcs900h_device::and32( uint32_t a, uint32_t b)
{
	uint32_t result = a & b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF ) | FLAG_HF;

	return result;
}


uint8_t tlcs900h_device::or8( uint8_t a, uint8_t b)
{
	uint8_t result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity8( result );

	return result;
}


uint16_t tlcs900h_device::or16( uint16_t a, uint16_t b)
{
	uint16_t result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity16( result );

	return result;
}


uint32_t tlcs900h_device::or32( uint32_t a, uint32_t b)
{
	uint32_t result = a | b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	return result;
}


uint8_t tlcs900h_device::xor8( uint8_t a, uint8_t b)
{
	uint8_t result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity8( result );

	return result;
}


uint16_t tlcs900h_device::xor16( uint16_t a, uint16_t b)
{
	uint16_t result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	parity16( result );

	return result;
}


uint32_t tlcs900h_device::xor32( uint32_t a, uint32_t b)
{
	uint32_t result = a ^ b;

	m_sr.b.l &= ~(FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF);
	m_sr.b.l |= ( ( result >> 24 ) & FLAG_SF ) | ( result ? 0 : FLAG_ZF );

	return result;
}


void tlcs900h_device::ldcf8( uint8_t a, uint8_t b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::ldcf16( uint8_t a, uint8_t b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::andcf8( uint8_t a, uint8_t b )
{
	if ( ( b & ( 1 << ( a & 0x07 ) ) ) && ( m_sr.b.l & FLAG_CF ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::andcf16( uint8_t a, uint8_t b )
{
	if ( ( b & ( 1 << ( a & 0x0f ) ) ) && ( m_sr.b.l & FLAG_CF ) )
		m_sr.b.l |= FLAG_CF;
	else
		m_sr.b.l &= ~ FLAG_CF;
}


void tlcs900h_device::orcf8( uint8_t a, uint8_t b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		m_sr.b.l |= FLAG_CF;
}


void tlcs900h_device::orcf16( uint8_t a, uint8_t b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		m_sr.b.l |= FLAG_CF;
}


void tlcs900h_device::xorcf8( uint8_t a, uint8_t b )
{
	if ( b & ( 1 << ( a & 0x07 ) ) )
		m_sr.b.l ^= FLAG_CF;
}


void tlcs900h_device::xorcf16( uint8_t a, uint8_t b )
{
	if ( b & ( 1 << ( a & 0x0f ) ) )
		m_sr.b.l ^= FLAG_CF;
}


uint8_t tlcs900h_device::rl8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint16_t tlcs900h_device::rl16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint32_t tlcs900h_device::rl32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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

uint8_t tlcs900h_device::rlc8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x80 ) ? 1 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( a & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity8( a );

	return a;
}


uint16_t tlcs900h_device::rlc16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a << 1 ) | ( ( a & 0x8000 ) ? 1 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( a >> 8 ) & FLAG_SF ) | ( a ? 0 : FLAG_ZF ) | ( a & FLAG_CF );
	parity16( a );

	return a;
}


uint32_t tlcs900h_device::rlc32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint8_t tlcs900h_device::rr8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint16_t tlcs900h_device::rr16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint32_t tlcs900h_device::rr32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint8_t tlcs900h_device::rrc8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x01 ) ? 0x80 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( a & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity8( a );

	return a;
}


uint16_t tlcs900h_device::rrc16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

	for ( ; count > 0; count-- )
	{
		a = ( a >> 1 ) | ( ( a & 0x0001 ) ? 0x8000 : 0 );
	}

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( ( a >> 8 ) & FLAG_SF ) ? FLAG_CF | FLAG_SF : 0 ) | ( a ? 0 : FLAG_ZF );
	parity16( a );

	return a;
}


uint32_t tlcs900h_device::rrc32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint8_t tlcs900h_device::sla8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint16_t tlcs900h_device::sla16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint32_t tlcs900h_device::sla32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint8_t tlcs900h_device::sra8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint16_t tlcs900h_device::sra16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint32_t tlcs900h_device::sra32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint8_t tlcs900h_device::srl8( uint8_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint16_t tlcs900h_device::srl16( uint16_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint32_t tlcs900h_device::srl32( uint32_t a, uint8_t s )
{
	uint8_t count = ( s & 0x0f ) ? ( s & 0x0f ) : 16;

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


uint16_t tlcs900h_device::div8( uint16_t a, uint8_t b )
{
	ldiv_t  result;

	if ( !b )
	{
		m_sr.b.l |= FLAG_VF;
		return ( a << 8 ) | ( ( a >> 8 ) ^ 0xff );
	}

	if ( a >= ( 0x0200 * b ) ) {
		uint16_t diff = a - ( 0x0200 * b );
		uint16_t range = 0x100 - b;

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


uint32_t tlcs900h_device::div16( uint32_t a, uint16_t b )
{
	ldiv_t  result;

	if ( !b )
	{
		m_sr.b.l |= FLAG_VF;
		return ( a << 16 ) | ( ( a >> 16 ) ^ 0xffff );
	}

//  if ( a >= ( 0x02000000 * b ) ) {
//      uint32_t diff = a - ( 0x02000000 * b );
//      uint32_t range = 0x1000000 - b;
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


uint16_t tlcs900h_device::divs8( int16_t a, int8_t b )
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


uint32_t tlcs900h_device::divs16( int32_t a, int16_t b )
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


void tlcs900h_device::op_ADCBMI()
{
	WRMEM( m_ea1.d, adc8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_ADCBMR()
{
	WRMEM( m_ea1.d, adc8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_ADCBRI()
{
	*m_p1_reg8 = adc8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_ADCBRM()
{
	*m_p1_reg8 = adc8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ADCBRR()
{
	*m_p1_reg8 = adc8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_ADCWMI()
{
	WRMEMW( m_ea1.d, adc16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_ADCWMR()
{
	WRMEMW( m_ea1.d, adc16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_ADCWRI()
{
	*m_p1_reg16 = adc16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_ADCWRM()
{
	*m_p1_reg16 = adc16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_ADCWRR()
{
	*m_p1_reg16 = adc16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_ADCLMR()
{
	WRMEML( m_ea1.d, adc32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_ADCLRI()
{
	*m_p1_reg32 = adc32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_ADCLRM()
{
	*m_p1_reg32 = adc32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_ADCLRR()
{
	*m_p1_reg32 = adc32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_ADDBMI()
{
	WRMEM( m_ea1.d, add8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_ADDBMR()
{
	WRMEM( m_ea1.d, add8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_ADDBRI()
{
	*m_p1_reg8 = add8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_ADDBRM()
{
	*m_p1_reg8 = add8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ADDBRR()
{
	*m_p1_reg8 = add8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_ADDWMI()
{
	WRMEMW( m_ea1.d, add16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_ADDWMR()
{
	WRMEMW( m_ea1.d, add16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_ADDWRI()
{
	*m_p1_reg16 = add16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_ADDWRM()
{
	*m_p1_reg16 = add16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_ADDWRR()
{
	*m_p1_reg16 = add16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_ADDLMR()
{
	WRMEML( m_ea1.d, add32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_ADDLRI()
{
	*m_p1_reg32 = add32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_ADDLRM()
{
	*m_p1_reg32 = add32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_ADDLRR()
{
	*m_p1_reg32 = add32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_ANDBMI()
{
	WRMEM( m_ea1.d, and8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_ANDBMR()
{
	WRMEM( m_ea1.d, and8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_ANDBRI()
{
	*m_p1_reg8 = and8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_ANDBRM()
{
	*m_p1_reg8 = and8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ANDBRR()
{
	*m_p1_reg8 = and8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_ANDWMI()
{
	WRMEMW( m_ea1.d, and16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_ANDWMR()
{
	WRMEMW( m_ea1.d, and16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_ANDWRI()
{
	*m_p1_reg16 = and16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_ANDWRM()
{
	*m_p1_reg16 = and16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_ANDWRR()
{
	*m_p1_reg16 = and16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_ANDLMR()
{
	WRMEML( m_ea1.d, and32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_ANDLRI()
{
	*m_p1_reg32 = and32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_ANDLRM()
{
	*m_p1_reg32 = and32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_ANDLRR()
{
	*m_p1_reg32 = and32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_ANDCFBIM()
{
	andcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ANDCFBIR()
{
	andcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::op_ANDCFBRM()
{
	andcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ANDCFBRR()
{
	andcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_ANDCFWIR()
{
	andcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::op_ANDCFWRR()
{
	andcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::op_BITBIM()
{
	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( RDMEM( m_ea2.d ) & ( 1 << ( m_imm1.b.l & 0x07 ) ) )
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900h_device::op_BITBIR()
{
	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( *m_p2_reg8 & ( 1 << ( m_imm1.b.l & 0x0f ) ) )
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900h_device::op_BITWIR()
{
	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	if ( *m_p2_reg16 & ( 1 << ( m_imm1.b.l & 0x0f ) ) )
		m_sr.b.l |= FLAG_HF;
	else
		m_sr.b.l |= FLAG_HF | FLAG_ZF;
}


void tlcs900h_device::op_BS1BRR()
{
	uint16_t  r = *m_p2_reg16;

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


void tlcs900h_device::op_BS1FRR()
{
	uint16_t  r = *m_p2_reg16;

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


void tlcs900h_device::op_CALLI()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, m_pc.d );
	m_pc.d = m_imm1.d;
	m_prefetch_clear = true;
}


void tlcs900h_device::op_CALLM()
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


void tlcs900h_device::op_CALR()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, m_pc.d );
	m_pc.d = m_ea1.d;
	m_prefetch_clear = true;
}


void tlcs900h_device::op_CCF()
{
	m_sr.b.l &= ~ FLAG_NF;
	m_sr.b.l ^= FLAG_CF;
}


void tlcs900h_device::op_CHGBIM()
{
	WRMEM( m_ea2.d, RDMEM( m_ea2.d ) ^ ( 1 << ( m_imm1.b.l & 0x07 ) ) );
}


void tlcs900h_device::op_CHGBIR()
{
	*m_p2_reg8 ^= ( 1 << ( m_imm1.b.l & 0x07 ) );
}


void tlcs900h_device::op_CHGWIR()
{
	*m_p2_reg16 ^= ( 1 << ( m_imm1.b.l & 0x0f ) );
}


void tlcs900h_device::op_CPBMI()
{
	sub8( RDMEM( m_ea1.d ), m_imm2.b.l );
}


void tlcs900h_device::op_CPBMR()
{
	sub8( RDMEM( m_ea1.d ), *m_p2_reg8 );
}


void tlcs900h_device::op_CPBRI()
{
	sub8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_CPBRM()
{
	sub8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_CPBRR()
{
	sub8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_CPWMI()
{
	sub16( RDMEMW( m_ea1.d ), m_imm2.w.l );
}


void tlcs900h_device::op_CPWMR()
{
	sub16( RDMEMW( m_ea1.d ), *m_p2_reg16 );
}


void tlcs900h_device::op_CPWRI()
{
	sub16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_CPWRM()
{
	sub16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_CPWRR()
{
	sub16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_CPLMR()
{
	sub32( RDMEML( m_ea1.d ), *m_p2_reg32 );
}


void tlcs900h_device::op_CPLRI()
{
	sub32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_CPLRM()
{
	sub32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_CPLRR()
{
	sub32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_CPD()
{
	uint8_t   result = *get_reg8_current( 1 ) - RDMEM( *m_p2_reg32 );
	uint16_t  *bc = get_reg16_current( 1 );

	*m_p2_reg32 -= 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::op_CPDR()
{
	op_CPD();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_CPDW()
{
	uint16_t  result = *get_reg16_current( 0 ) - RDMEMW( *m_p2_reg32 );
	uint16_t  *bc = get_reg16_current( 1 );

	*m_p2_reg32 -= 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::op_CPDRW()
{
	op_CPDW();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_CPI()
{
	uint8_t   result = *get_reg8_current( 1 ) - RDMEM( *m_p2_reg32 );
	uint16_t  *bc = get_reg16_current( 1 );

	*m_p2_reg32 += 1;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( result & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::op_CPIR()
{
	op_CPI();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_CPIW()
{
	uint16_t  result = *get_reg16_current( 0 ) - RDMEMW( *m_p2_reg32 );
	uint16_t  *bc = get_reg16_current( 1 );

	*m_p2_reg32 += 2;
	*bc -= 1;
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF );
	m_sr.b.l |= ( ( result >> 8 ) & FLAG_SF ) | ( result ? FLAG_NF : FLAG_NF | FLAG_ZF ) |
		( *bc ? FLAG_VF : 0 );
}


void tlcs900h_device::op_CPIRW()
{
	op_CPIW();

	if ( ( m_sr.b.l & ( FLAG_ZF | FLAG_VF ) ) == FLAG_VF )
	{
		m_pc.d -= 2;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_CPLBR()
{
	*m_p1_reg8 = ~ *m_p1_reg8;
	m_sr.b.l |= FLAG_HF | FLAG_NF;
}


void tlcs900h_device::op_CPLWR()
{
	*m_p1_reg16 = ~ *m_p1_reg16;
	m_sr.b.l |= FLAG_HF | FLAG_NF;
}


void tlcs900h_device::op_DAABR()
{
	uint8_t   oldval = *m_p1_reg8;
	uint8_t   fixval = 0;
	uint8_t   carry = 0;
	uint8_t   high = *m_p1_reg8 & 0xf0;
	uint8_t   low = *m_p1_reg8 & 0x0f;

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


void tlcs900h_device::op_DB()
{
	logerror("%08x: invalid or illegal instruction\n", m_pc.d );
}


void tlcs900h_device::op_DECBIM()
{
	uint8_t   cy = m_sr.b.l & FLAG_CF;

	WRMEM( m_ea2.d, sub8( RDMEM( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::op_DECBIR()
{
	uint8_t   cy = m_sr.b.l & FLAG_CF;

	*m_p2_reg8 = sub8( *m_p2_reg8, m_imm1.b.l ? m_imm1.b.l : 8 );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::op_DECWIM()
{
	uint8_t   cy = m_sr.b.l & FLAG_CF;

	WRMEMW( m_ea2.d, sub16( RDMEMW( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::op_DECWIR()
{
	*m_p2_reg16 -= m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::op_DECLIR()
{
	*m_p2_reg32 -= m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::op_DECF()
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	m_sr.b.h = ( m_sr.b.h & 0xf8 ) | ( ( m_sr.b.h - 1 ) & 0x07 );
	m_regbank = m_sr.b.h & 0x03;
}


void tlcs900h_device::op_DIVBRI()
{
	*m_p1_reg16 = div8( *m_p1_reg16, m_imm2.b.l );
}


void tlcs900h_device::op_DIVBRM()
{
	*m_p1_reg16 = div8( *m_p1_reg16, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_DIVBRR()
{
	*m_p1_reg16 = div8( *m_p1_reg16, *m_p2_reg8 );
}


void tlcs900h_device::op_DIVWRI()
{
	*m_p1_reg32 = div16( *m_p1_reg32, m_imm2.w.l );
}


void tlcs900h_device::op_DIVWRM()
{
	*m_p1_reg32 = div16( *m_p1_reg32, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_DIVWRR()
{
	*m_p1_reg32 = div16( *m_p1_reg32, *m_p2_reg16 );
}


void tlcs900h_device::op_DIVSBRI()
{
	*m_p1_reg16 = divs8( *m_p1_reg16, m_imm2.b.l );
}


void tlcs900h_device::op_DIVSBRM()
{
	*m_p1_reg16 = divs8( *m_p1_reg16, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_DIVSBRR()
{
	*m_p1_reg16 = divs8( *m_p1_reg16, *m_p2_reg8 );
}


void tlcs900h_device::op_DIVSWRI()
{
	*m_p1_reg32 = divs16( *m_p1_reg32, m_imm2.w.l );
}


void tlcs900h_device::op_DIVSWRM()
{
	*m_p1_reg32 = divs16( *m_p1_reg32, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_DIVSWRR()
{
	*m_p1_reg32 = divs16( *m_p1_reg32, *m_p2_reg16 );
}


void tlcs900h_device::op_DJNZB()
{
	*m_p1_reg8 -= 1;
	if ( *m_p1_reg8 )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_DJNZW()
{
	*m_p1_reg16 -= 1;
	if ( *m_p1_reg16 )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_EI()
{
	m_sr.b.h = ( m_sr.b.h & 0x8f ) | ( ( m_imm1.b.l & 0x07 ) << 4 );
	m_check_irqs = 1;
}


void tlcs900h_device::op_EXBMR()
{
	uint8_t   i = RDMEM( m_ea1.d );

	WRMEM( m_ea1.d, *m_p2_reg8 );
	*m_p2_reg8 = i;
}


void tlcs900h_device::op_EXBRR()
{
	uint8_t   i = *m_p2_reg8;

	*m_p2_reg8 = *m_p1_reg8;
	*m_p1_reg8 = i;
}


void tlcs900h_device::op_EXWMR()
{
	uint16_t  i = RDMEMW( m_ea1.d );

	WRMEMW( m_ea1.d, *m_p2_reg16 );
	*m_p2_reg16 = i;
}


void tlcs900h_device::op_EXWRR()
{
	uint16_t  i = *m_p2_reg16;

	*m_p2_reg16 = *m_p1_reg16;
	*m_p1_reg16 = i;
}


void tlcs900h_device::op_EXTSWR()
{
	if ( *m_p1_reg16 & 0x0080 )
		*m_p1_reg16 |= 0xff00;
	else
		*m_p1_reg16 &= 0x00ff;
}


void tlcs900h_device::op_EXTSLR()
{
	if ( *m_p1_reg32 & 0x00008000 )
		*m_p1_reg32 |= 0xffff0000;
	else
		*m_p1_reg32 &= 0x0000ffff;
}


void tlcs900h_device::op_EXTZWR()
{
	*m_p1_reg16 &= 0x00ff;
}


void tlcs900h_device::op_EXTZLR()
{
	*m_p1_reg32 &= 0x0000ffff;
}


void tlcs900h_device::op_HALT()
{
	m_halted = 1;
}


void tlcs900h_device::op_INCBIM()
{
	uint8_t   cy = m_sr.b.l & FLAG_CF;

	WRMEM( m_ea2.d, add8( RDMEM( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::op_INCBIR()
{
	uint8_t   cy = m_sr.b.l & FLAG_CF;

	*m_p2_reg8 = add8( *m_p2_reg8, m_imm1.b.l ? m_imm1.b.l : 8 );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::op_INCWIM()
{
	uint8_t   cy = m_sr.b.l & FLAG_CF;

	WRMEMW( m_ea2.d, add16( RDMEMW( m_ea2.d ), m_imm1.b.l ? m_imm1.b.l : 8 ) );
	m_sr.b.l = ( m_sr.b.l & ~ FLAG_CF ) | cy;
}


void tlcs900h_device::op_INCWIR()
{
	*m_p2_reg16 += m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::op_INCLIR()
{
	*m_p2_reg32 += m_imm1.b.l ? m_imm1.b.l : 8;
}


void tlcs900h_device::op_INCF()
{
	/* 0x03 for MAX mode, 0x07 for MIN mode */
	m_sr.b.h = ( m_sr.b.h & 0xf8 ) | ( ( m_sr.b.h + 1 ) & 0x07 );
	m_regbank = m_sr.b.h & 0x03;
}


void tlcs900h_device::op_JPI()
{
	m_pc.d = m_imm1.d;
	m_prefetch_clear = true;
}


void tlcs900h_device::op_JPM()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_JR()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_JRL()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = m_ea2.d;
		m_cycles += 4;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_LDBMI()
{
	WRMEM( m_ea1.d, m_imm2.b.l );
}


void tlcs900h_device::op_LDBMM()
{
	WRMEM( m_ea1.d, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_LDBMR()
{
	WRMEM( m_ea1.d, *m_p2_reg8 );
}


void tlcs900h_device::op_LDBRI()
{
	*m_p1_reg8 = m_imm2.b.l;
}


void tlcs900h_device::op_LDBRM()
{
	*m_p1_reg8 = RDMEM( m_ea2.d );
}


void tlcs900h_device::op_LDBRR()
{
	*m_p1_reg8 = *m_p2_reg8;
}


void tlcs900h_device::op_LDWMI()
{
	WRMEMW( m_ea1.d, m_imm2.w.l );
}


void tlcs900h_device::op_LDWMM()
{
	WRMEMW( m_ea1.d, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_LDWMR()
{
	WRMEMW( m_ea1.d, *m_p2_reg16 );
}


void tlcs900h_device::op_LDWRI()
{
	*m_p1_reg16 = m_imm2.w.l;
}


void tlcs900h_device::op_LDWRM()
{
	*m_p1_reg16 = RDMEMW( m_ea2.d );
}


void tlcs900h_device::op_LDWRR()
{
	*m_p1_reg16 = *m_p2_reg16;
}


void tlcs900h_device::op_LDLRI()
{
	*m_p1_reg32 = m_imm2.d;
}


void tlcs900h_device::op_LDLRM()
{
	*m_p1_reg32 = RDMEML( m_ea2.d );
}


void tlcs900h_device::op_LDLRR()
{
	*m_p1_reg32 = *m_p2_reg32;
}


void tlcs900h_device::op_LDLMR()
{
	WRMEML( m_ea1.d, *m_p2_reg32 );
}


void tlcs900h_device::op_LDAW()
{
	*m_p1_reg16 = m_ea2.w.l;
}


void tlcs900h_device::op_LDAL()
{
	*m_p1_reg32 = m_ea2.d;
}


void tlcs900h_device::op_LDCBRR()
{
	*m_p1_reg8 = *m_p2_reg8;
}


void tlcs900h_device::op_LDCWRR()
{
	*m_p1_reg16 = *m_p2_reg16;
}


void tlcs900h_device::op_LDCLRR()
{
	*m_p1_reg32 = *m_p2_reg32;
}


void tlcs900h_device::op_LDCFBIM()
{
	ldcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_LDCFBIR()
{
	ldcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::op_LDCFBRM()
{
	ldcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_LDCFBRR()
{
	ldcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_LDCFWIR()
{
	ldcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::op_LDCFWRR()
{
	ldcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::op_LDD()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDDR()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDDRW()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDDW()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDF()
{
	m_sr.b.h = ( m_sr.b.h & 0xf8 ) | ( m_imm1.b.l & 0x07 );
	m_regbank = m_imm1.b.l & 0x03;
}


void tlcs900h_device::op_LDI()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDIR()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDIRW()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDIW()
{
	uint16_t  *bc = get_reg16_current( 1 );

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


void tlcs900h_device::op_LDX()
{
	uint8_t   a, b;

	RDOP();
	a = RDOP();
	RDOP();
	b = RDOP();
	RDOP();
	WRMEM( a, b );
}


void tlcs900h_device::op_LINK()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, *m_p1_reg32 );
	*m_p1_reg32 = m_xssp.d;
	m_xssp.d += m_imm2.sw.l;
}


void tlcs900h_device::op_MAX()
{
	m_sr.b.h |= 0x04;
}


void tlcs900h_device::op_MDEC1()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 += m_imm1.w.l;
	else
		*m_p2_reg16 -= 1;
}


void tlcs900h_device::op_MDEC2()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 += m_imm1.w.l;
	else
		*m_p2_reg16 -= 2;
}


void tlcs900h_device::op_MDEC4()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 += m_imm1.w.l;
	else
		*m_p2_reg16 -= 4;
}


void tlcs900h_device::op_MINC1()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 -= m_imm1.w.l;
	else
		*m_p2_reg16 += 1;
}


void tlcs900h_device::op_MINC2()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 -= m_imm1.w.l;
	else
		*m_p2_reg16 += 2;
}


void tlcs900h_device::op_MINC4()
{
	if ( ( *m_p2_reg16 & m_imm1.w.l ) == m_imm1.w.l )
		*m_p2_reg16 -= m_imm1.w.l;
	else
		*m_p2_reg16 += 4;
}


void tlcs900h_device::op_MIRRW()
{
	uint16_t  r = *m_p1_reg16;
	uint16_t  s = ( r & 0x01 );
	int i;


	for ( i = 0; i < 15; i++ )
	{
		r >>= 1;
		s <<= 1;
		s |= ( r & 0x01 );
	}

	*m_p1_reg16 = s;
}


void tlcs900h_device::op_MULBRI()
{
	*m_p1_reg16 = ( *m_p1_reg16 & 0xff ) * m_imm2.b.l;
}


void tlcs900h_device::op_MULBRM()
{
	*m_p1_reg16 = ( *m_p1_reg16 & 0xff ) * RDMEM( m_ea2.d );
}


void tlcs900h_device::op_MULBRR()
{
	*m_p1_reg16 = ( *m_p1_reg16 & 0xff ) * *m_p2_reg8;
}


void tlcs900h_device::op_MULWRI()
{
	*m_p1_reg32 = ( *m_p1_reg32 & 0xffff ) * m_imm2.w.l;
}


void tlcs900h_device::op_MULWRM()
{
	*m_p1_reg32 = ( *m_p1_reg32 & 0xffff ) * RDMEMW( m_ea2.d );
}


void tlcs900h_device::op_MULWRR()
{
	*m_p1_reg32 = ( *m_p1_reg32 & 0xffff ) * *m_p2_reg16;
}


void tlcs900h_device::op_MULAR()
{
	uint32_t  *xde = get_reg32_current( 2 );
	uint32_t  *xhl = get_reg32_current( 3 );

	*m_p1_reg32 = *m_p1_reg32 + ( ((int16_t)RDMEMW( *xde )) * ((int16_t)RDMEMW( *xhl )) );
	*xhl -= 2;

	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_VF );
	m_sr.b.l |= ( ( *m_p1_reg32 >> 24 ) & FLAG_SF ) | ( *m_p1_reg32 ? 0 : FLAG_ZF );
}


void tlcs900h_device::op_MULSBRI()
{
	*m_p1_reg16 = (int8_t)( *m_p1_reg16 & 0xff ) * m_imm2.sb.l;
}


void tlcs900h_device::op_MULSBRM()
{
	*m_p1_reg16 = (int8_t)( *m_p1_reg16 & 0xff ) * (int8_t)RDMEM( m_ea2.d );
}


void tlcs900h_device::op_MULSBRR()
{
	*m_p1_reg16 = (int8_t)( *m_p1_reg16 & 0xff ) * (int8_t)*m_p2_reg8;
}


void tlcs900h_device::op_MULSWRI()
{
	*m_p1_reg32 = (int16_t)( *m_p1_reg32 & 0xffff ) * m_imm2.sw.l;
}


void tlcs900h_device::op_MULSWRM()
{
	*m_p1_reg32 = (int16_t)( *m_p1_reg32 & 0xffff ) * (int16_t)RDMEMW( m_ea2.d );
}


void tlcs900h_device::op_MULSWRR()
{
	*m_p1_reg32 = (int16_t)( *m_p1_reg32 & 0xffff ) * (int16_t)*m_p2_reg16;
}


void tlcs900h_device::op_NEGBR()
{
	*m_p1_reg8 = sub8( 0, *m_p1_reg8 );
}


void tlcs900h_device::op_NEGWR()
{
	*m_p1_reg16 = sub16( 0, *m_p1_reg16 );
}


void tlcs900h_device::op_NOP()
{
	/* Do nothing */
}


void tlcs900h_device::op_NORMAL()
{
	m_sr.b.h &= 0x7F;
}


void tlcs900h_device::op_ORBMI()
{
	WRMEM( m_ea1.d, or8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_ORBMR()
{
	WRMEM( m_ea1.d, or8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_ORBRI()
{
	*m_p1_reg8 = or8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_ORBRM()
{
	*m_p1_reg8 = or8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ORBRR()
{
	*m_p1_reg8 = or8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_ORWMI()
{
	WRMEMW( m_ea1.d, or16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_ORWMR()
{
	WRMEMW( m_ea1.d, or16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_ORWRI()
{
	*m_p1_reg16 = or16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_ORWRM()
{
	*m_p1_reg16 = or16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_ORWRR()
{
	*m_p1_reg16 = or16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_ORLMR()
{
	WRMEML( m_ea1.d, or32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_ORLRI()
{
	*m_p1_reg32 = or32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_ORLRM()
{
	*m_p1_reg32 = or32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_ORLRR()
{
	*m_p1_reg32 = or32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_ORCFBIM()
{
	orcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ORCFBIR()
{
	orcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::op_ORCFBRM()
{
	orcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_ORCFBRR()
{
	orcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_ORCFWIR()
{
	orcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::op_ORCFWRR()
{
	orcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::op_PAAWR()
{
	if ( *m_p1_reg16 & 1 )
		*m_p1_reg16 += 1;
}


void tlcs900h_device::op_PAALR()
{
	if ( *m_p1_reg32 & 1 )
		*m_p1_reg32 += 1;
}


void tlcs900h_device::op_POPBM()
{
	WRMEM( m_ea1.d, RDMEM( m_xssp.d ) );
	m_xssp.d += 1;
}


void tlcs900h_device::op_POPBR()
{
	*m_p1_reg8 = RDMEM( m_xssp.d );
	m_xssp.d += 1;
}


void tlcs900h_device::op_POPWM()
{
	WRMEMW( m_ea1.d, RDMEMW( m_xssp.d ) );
	m_xssp.d += 2;
}


void tlcs900h_device::op_POPWR()
{
	*m_p1_reg16 = RDMEMW( m_xssp.d );
	m_xssp.d += 2;
}


void tlcs900h_device::op_POPWSR()
{
	op_POPWR();
	m_regbank = m_sr.b.h & 0x03;
	m_check_irqs = 1;
}


void tlcs900h_device::op_POPLR()
{
	*m_p1_reg32 = RDMEML( m_xssp.d );
	m_xssp.d += 4;
}


void tlcs900h_device::op_PUSHBI()
{
	m_xssp.d -= 1;
	WRMEM( m_xssp.d, m_imm1.b.l );
}


void tlcs900h_device::op_PUSHBM()
{
	m_xssp.d -= 1;
	WRMEM( m_xssp.d, RDMEM( m_ea1.d ) );
}


void tlcs900h_device::op_PUSHBR()
{
	m_xssp.d -= 1;
	WRMEM( m_xssp.d, *m_p1_reg8 );
}


void tlcs900h_device::op_PUSHWI()
{
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, m_imm1.w.l );
}


void tlcs900h_device::op_PUSHWM()
{
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, RDMEMW( m_ea1.d ) );
}


void tlcs900h_device::op_PUSHWR()
{
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, *m_p1_reg16 );
}


void tlcs900h_device::op_PUSHLR()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, *m_p1_reg32 );
}


void tlcs900h_device::op_RCF()
{
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_NF | FLAG_CF );
}


void tlcs900h_device::op_RESBIM()
{
	WRMEM( m_ea2.d, RDMEM( m_ea2.d ) & ~( 1 << ( m_imm1.d & 0x07 ) ) );
}


void tlcs900h_device::op_RESBIR()
{
	*m_p2_reg8 = *m_p2_reg8 & ~( 1 << ( m_imm1.d & 0x07 ) );
}


void tlcs900h_device::op_RESWIR()
{
	*m_p2_reg16 = *m_p2_reg16 & ~( 1 << ( m_imm1.d & 0x0f ) );
}


void tlcs900h_device::op_RET()
{
	m_pc.d = RDMEML( m_xssp.d );
	m_xssp.d += 4;
	m_prefetch_clear = true;
}


void tlcs900h_device::op_RETCC()
{
	if ( condition_true( m_op ) )
	{
		m_pc.d = RDMEML( m_xssp.d );
		m_xssp.d += 4;
		m_cycles += 6;
		m_prefetch_clear = true;
	}
}


void tlcs900h_device::op_RETD()
{
	m_pc.d = RDMEML( m_xssp.d );
	m_xssp.d += 4 + m_imm1.sw.l;
	m_prefetch_clear = true;
}


void tlcs900h_device::op_RETI()
{
	m_sr.w.l = RDMEMW( m_xssp.d );
	m_xssp.d += 2;
	m_pc.d = RDMEML( m_xssp.d );
	m_xssp.d += 4;
	m_regbank = m_sr.b.h & 0x03;
	m_check_irqs = 1;
	m_prefetch_clear = true;
}


void tlcs900h_device::op_RLBM()
{
	WRMEM( m_ea2.d, rl8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RLWM()
{
	WRMEMW( m_ea2.d, rl16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RLBIR()
{
	*m_p2_reg8 = rl8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_RLBRR()
{
	*m_p2_reg8 = rl8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_RLWIR()
{
	*m_p2_reg16 = rl16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_RLWRR()
{
	*m_p2_reg16 = rl16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_RLLIR()
{
	*m_p2_reg32 = rl32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_RLLRR()
{
	*m_p2_reg32 = rl32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_RLCBM()
{
	WRMEM( m_ea2.d, rlc8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RLCWM()
{
	WRMEMW( m_ea2.d, rlc16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RLCBIR()
{
	*m_p2_reg8 = rlc8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_RLCBRR()
{
	*m_p2_reg8 = rlc8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_RLCWIR()
{
	*m_p2_reg16 = rlc16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_RLCWRR()
{
	*m_p2_reg16 = rlc16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_RLCLIR()
{
	*m_p2_reg32 = rlc32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_RLCLRR()
{
	*m_p2_reg32 = rlc32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_RLDRM()
{
	uint8_t   a = *m_p1_reg8 & 0x0f;
	uint8_t   b = RDMEM( m_ea2.d );

	*m_p1_reg8 = ( *m_p1_reg8 & 0xf0 ) | ( ( b & 0xf0 ) >> 4 );
	WRMEM( m_ea2.d, ( ( b & 0x0f ) << 4 ) | a );
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( *m_p1_reg8 & FLAG_SF ) | ( *m_p1_reg8 ? 0 : FLAG_ZF );
	parity8( *m_p1_reg8 );
}


void tlcs900h_device::op_RRBM()
{
	WRMEM( m_ea2.d, rr8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RRWM()
{
	WRMEMW( m_ea2.d, rr16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RRBIR()
{
	*m_p2_reg8 = rr8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_RRBRR()
{
	*m_p2_reg8 = rr8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_RRWIR()
{
	*m_p2_reg16 = rr16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_RRWRR()
{
	*m_p2_reg16 = rr16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_RRLIR()
{
	*m_p2_reg32 = rr32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_RRLRR()
{
	*m_p2_reg32 = rr32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_RRCBM()
{
	WRMEM( m_ea2.d, rrc8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RRCWM()
{
	WRMEMW( m_ea2.d, rrc16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_RRCBIR()
{
	*m_p2_reg8 = rrc8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_RRCBRR()
{
	*m_p2_reg8 = rrc8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_RRCWIR()
{
	*m_p2_reg16 = rrc16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_RRCWRR()
{
	*m_p2_reg16 = rrc16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_RRCLIR()
{
	*m_p2_reg32 = rrc32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_RRCLRR()
{
	*m_p2_reg32 = rrc32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_RRDRM()
{
	uint8_t   a = *m_p1_reg8 & 0x0f;
	uint8_t   b = RDMEM( m_ea2.d );

	*m_p1_reg8 = ( *m_p1_reg8 & 0xf0 ) | ( b & 0x0f );
	WRMEM( m_ea2.d, ( ( b & 0xf0 ) >> 4 ) | ( a << 4 ) );
	m_sr.b.l &= ~ ( FLAG_SF | FLAG_ZF | FLAG_HF | FLAG_VF | FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( *m_p1_reg8 & FLAG_SF ) | ( *m_p1_reg8 ? 0 : FLAG_ZF );
	parity8( *m_p1_reg8 );
}


void tlcs900h_device::op_SBCBMI()
{
	WRMEM( m_ea1.d, sbc8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_SBCBMR()
{
	WRMEM( m_ea1.d, sbc8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_SBCBRI()
{
	*m_p1_reg8 = sbc8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_SBCBRM()
{
	*m_p1_reg8 = sbc8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_SBCBRR()
{
	*m_p1_reg8 = sbc8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_SBCWMI()
{
	WRMEMW( m_ea1.d, sbc16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_SBCWMR()
{
	WRMEMW( m_ea1.d, sbc16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_SBCWRI()
{
	*m_p1_reg16 = sbc16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_SBCWRM()
{
	*m_p1_reg16 = sbc16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_SBCWRR()
{
	*m_p1_reg16 = sbc16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_SBCLMR()
{
	WRMEML( m_ea1.d, sbc32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_SBCLRI()
{
	*m_p1_reg32 = sbc32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_SBCLRM()
{
	*m_p1_reg32 = sbc32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_SBCLRR()
{
	*m_p1_reg32 = sbc32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_SCCBR()
{
	*m_p2_reg8 = condition_true( m_op ) ? 1 : 0;
}


void tlcs900h_device::op_SCCWR()
{
	*m_p2_reg16 = condition_true( m_op ) ? 1 : 0;
}


void tlcs900h_device::op_SCF()
{
	m_sr.b.l &= ~ ( FLAG_HF | FLAG_NF );
	m_sr.b.l |= FLAG_CF;
}


void tlcs900h_device::op_SETBIM()
{
	WRMEM( m_ea2.d, RDMEM( m_ea2.d ) | ( 1 << ( m_imm1.d & 0x07 ) ) );
}


void tlcs900h_device::op_SETBIR()
{
	*m_p2_reg8 = *m_p2_reg8 | ( 1 << ( m_imm1.d & 0x07 ) );
}


void tlcs900h_device::op_SETWIR()
{
	*m_p2_reg16 = *m_p2_reg16 | ( 1 << ( m_imm1.d & 0x0f ) );
}


void tlcs900h_device::op_SLABM()
{
	WRMEM( m_ea2.d, sla8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SLAWM()
{
	WRMEMW( m_ea2.d, sla16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SLABIR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_SLABRR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_SLAWIR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_SLAWRR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_SLALIR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_SLALRR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_SLLBM()
{
	WRMEM( m_ea2.d, sla8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SLLWM()
{
	WRMEMW( m_ea2.d, sla16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SLLBIR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_SLLBRR()
{
	*m_p2_reg8 = sla8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_SLLWIR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_SLLWRR()
{
	*m_p2_reg16 = sla16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_SLLLIR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_SLLLRR()
{
	*m_p2_reg32 = sla32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_SRABM()
{
	WRMEM( m_ea2.d, sra8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SRAWM()
{
	WRMEMW( m_ea2.d, sra16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SRABIR()
{
	*m_p2_reg8 = sra8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_SRABRR()
{
	*m_p2_reg8 = sra8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_SRAWIR()
{
	*m_p2_reg16 = sra16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_SRAWRR()
{
	*m_p2_reg16 = sra16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_SRALIR()
{
	*m_p2_reg32 = sra32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_SRALRR()
{
	*m_p2_reg32 = sra32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_SRLBM()
{
	WRMEM( m_ea2.d, srl8( RDMEM( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SRLWM()
{
	WRMEMW( m_ea2.d, srl16( RDMEMW( m_ea2.d ), 1 ) );
}


void tlcs900h_device::op_SRLBIR()
{
	*m_p2_reg8 = srl8( *m_p2_reg8, m_imm1.b.l );
}


void tlcs900h_device::op_SRLBRR()
{
	*m_p2_reg8 = srl8( *m_p2_reg8, *m_p1_reg8 );
}


void tlcs900h_device::op_SRLWIR()
{
	*m_p2_reg16 = srl16( *m_p2_reg16, m_imm1.b.l );
}


void tlcs900h_device::op_SRLWRR()
{
	*m_p2_reg16 = srl16( *m_p2_reg16, *m_p1_reg8 );
}


void tlcs900h_device::op_SRLLIR()
{
	*m_p2_reg32 = srl32( *m_p2_reg32, m_imm1.b.l );
}


void tlcs900h_device::op_SRLLRR()
{
	*m_p2_reg32 = srl32( *m_p2_reg32, *m_p1_reg8 );
}


void tlcs900h_device::op_STCFBIM()
{
	if ( m_sr.b.l & FLAG_CF )
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) | ( 1 << ( m_imm1.b.l & 0x07 ) ) );
	else
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) & ~ ( 1 << ( m_imm1.b.l & 0x07 ) ) );
}


void tlcs900h_device::op_STCFBIR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg8 |= ( 1 << ( m_imm1.b.l & 0x07 ) );
	else
		*m_p2_reg8 &= ~ ( 1 << ( m_imm1.b.l & 0x07 ) );
}


void tlcs900h_device::op_STCFBRM()
{
	if ( m_sr.b.l & FLAG_CF )
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) | ( 1 << ( *m_p1_reg8 & 0x07 ) ) );
	else
		WRMEM( m_ea2.d, RDMEM( m_ea2.d ) & ~ ( 1 << ( *m_p1_reg8 & 0x07 ) ) );
}


void tlcs900h_device::op_STCFBRR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg8 |= ( 1 << ( *m_p1_reg8 & 0x07 ) );
	else
		*m_p2_reg8 &= ~ ( 1 << ( *m_p1_reg8 & 0x07 ) );
}


void tlcs900h_device::op_STCFWIR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg16 |= ( 1 << ( m_imm1.b.l & 0x0f ) );
	else
		*m_p2_reg16 &= ~ ( 1 << ( m_imm1.b.l & 0x0f ) );
}


void tlcs900h_device::op_STCFWRR()
{
	if ( m_sr.b.l & FLAG_CF )
		*m_p2_reg16 |= ( 1 << ( *m_p1_reg8 & 0x0f ) );
	else
		*m_p2_reg16 &= ~ ( 1 << ( *m_p1_reg8 & 0x0f ) );
}


void tlcs900h_device::op_SUBBMI()
{
	WRMEM( m_ea1.d, sub8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_SUBBMR()
{
	WRMEM( m_ea1.d, sub8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_SUBBRI()
{
	*m_p1_reg8 = sub8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_SUBBRM()
{
	*m_p1_reg8 = sub8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_SUBBRR()
{
	*m_p1_reg8 = sub8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_SUBWMI()
{
	WRMEMW( m_ea1.d, sub16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_SUBWMR()
{
	WRMEMW( m_ea1.d, sub16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_SUBWRI()
{
	*m_p1_reg16 = sub16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_SUBWRM()
{
	*m_p1_reg16 = sub16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_SUBWRR()
{
	*m_p1_reg16 = sub16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_SUBLMR()
{
	WRMEML( m_ea1.d, sub32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_SUBLRI()
{
	*m_p1_reg32 = sub32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_SUBLRM()
{
	*m_p1_reg32 = sub32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_SUBLRR()
{
	*m_p1_reg32 = sub32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_SWI()
{
	m_xssp.d -= 4;
	WRMEML( m_xssp.d, m_pc.d );
	m_xssp.d -= 2;
	WRMEMW( m_xssp.d, m_sr.w.l );
	m_pc.d = RDMEML( 0x00ffff00 + 4 * m_imm1.b.l );
	m_prefetch_clear = true;
}


void tlcs900h_device::op_TSETBIM()
{
	uint8_t   b = 1 << ( m_imm1.b.l & 0x07 );
	uint8_t   a = RDMEM( m_ea2.d );

	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	m_sr.b.l |= ( ( a & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	WRMEM( m_ea2.d, a | b );
}


void tlcs900h_device::op_TSETBIR()
{
	uint8_t   b = 1 << ( m_imm1.b.l & 0x07 );

	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	m_sr.b.l |= ( ( *m_p2_reg8 & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	*m_p2_reg8 |= b;
}


void tlcs900h_device::op_TSETWIR()
{
	uint16_t  b = 1 << ( m_imm1.b.l & 0x0f );

	m_sr.b.l &= ~ ( FLAG_ZF | FLAG_NF );
	m_sr.b.l |= ( ( *m_p2_reg16 & b ) ? 0 : FLAG_ZF ) | FLAG_HF;
	*m_p2_reg16 |= b;
}


void tlcs900h_device::op_UNLK()
{
	m_xssp.d = *m_p1_reg32;
	*m_p1_reg32 = RDMEML( m_xssp.d );
	m_xssp.d += 4;
}


void tlcs900h_device::op_XORBMI()
{
	WRMEM( m_ea1.d, xor8( RDMEM( m_ea1.d ), m_imm2.b.l ) );
}


void tlcs900h_device::op_XORBMR()
{
	WRMEM( m_ea1.d, xor8( RDMEM( m_ea1.d ), *m_p2_reg8 ) );
}


void tlcs900h_device::op_XORBRI()
{
	*m_p1_reg8 = xor8( *m_p1_reg8, m_imm2.b.l );
}


void tlcs900h_device::op_XORBRM()
{
	*m_p1_reg8 = xor8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_XORBRR()
{
	*m_p1_reg8 = xor8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_XORWMI()
{
	WRMEMW( m_ea1.d, xor16( RDMEMW( m_ea1.d ), m_imm2.w.l ) );
}


void tlcs900h_device::op_XORWMR()
{
	WRMEMW( m_ea1.d, xor16( RDMEMW( m_ea1.d ), *m_p2_reg16 ) );
}


void tlcs900h_device::op_XORWRI()
{
	*m_p1_reg16 = xor16( *m_p1_reg16, m_imm2.w.l );
}


void tlcs900h_device::op_XORWRM()
{
	*m_p1_reg16 = xor16( *m_p1_reg16, RDMEMW( m_ea2.d ) );
}


void tlcs900h_device::op_XORWRR()
{
	*m_p1_reg16 = xor16( *m_p1_reg16, *m_p2_reg16 );
}


void tlcs900h_device::op_XORLMR()
{
	WRMEML( m_ea1.d, xor32( RDMEML( m_ea1.d ), *m_p2_reg32 ) );
}


void tlcs900h_device::op_XORLRI()
{
	*m_p1_reg32 = xor32( *m_p1_reg32, m_imm2.d );
}


void tlcs900h_device::op_XORLRM()
{
	*m_p1_reg32 = xor32( *m_p1_reg32, RDMEML( m_ea2.d ) );
}


void tlcs900h_device::op_XORLRR()
{
	*m_p1_reg32 = xor32( *m_p1_reg32, *m_p2_reg32 );
}


void tlcs900h_device::op_XORCFBIM()
{
	xorcf8( m_imm1.b.l, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_XORCFBIR()
{
	xorcf8( m_imm1.b.l, *m_p2_reg8 );
}


void tlcs900h_device::op_XORCFBRM()
{
	xorcf8( *m_p1_reg8, RDMEM( m_ea2.d ) );
}


void tlcs900h_device::op_XORCFBRR()
{
	xorcf8( *m_p1_reg8, *m_p2_reg8 );
}


void tlcs900h_device::op_XORCFWIR()
{
	xorcf16( m_imm1.b.l, *m_p2_reg16 );
}


void tlcs900h_device::op_XORCFWRR()
{
	xorcf16( *m_p1_reg8, *m_p2_reg16 );
}


void tlcs900h_device::op_ZCF()
{
	m_sr.b.l &= ~ ( FLAG_NF | FLAG_CF );
	m_sr.b.l |= ( ( m_sr.b.l & FLAG_ZF ) ? 0 : FLAG_CF );
}


void tlcs900h_device::prepare_operands(const tlcs900inst *inst)
{
	switch ( inst->operand1 )
	{
	case p_A:
		m_p1_reg8 = &m_xwa[m_regbank].b.l;
		break;
	case p_F:
		m_p1_reg8 = &m_sr.b.l;
		break;
	case p_SR:
		m_p1_reg16 = &m_sr.w.l;
		break;
	case p_C8:
		m_p1_reg8 = get_reg8_current( m_op );
		break;
	case p_C16:
		m_p1_reg16 = get_reg16_current( m_op );
		break;
	case p_MC16: /* For MUL and DIV operations */
		m_p1_reg16 = get_reg16_current( ( m_op >> 1 ) & 0x03 );
		break;
	case p_C32:
		m_p1_reg32 = get_reg32_current( m_op );
		break;
	case p_CR8:
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
	case p_CR16:
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
	case p_CR32:
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
	case p_D8:
		m_ea1.d = RDOP();
		m_ea1.d = m_pc.d + m_ea1.sb.l;
		break;
	case p_D16:
		m_ea1.d = RDOP();
		m_ea1.b.h = RDOP();
		m_ea1.d = m_pc.d + m_ea1.sw.l;
		break;
	case p_I3:
		m_imm1.d = m_op & 0x07;
		break;
	case p_I8:
		m_imm1.d = RDOP();
		break;
	case p_I16:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		break;
	case p_I24:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		m_imm1.b.h2 = RDOP();
		break;
	case p_I32:
		m_imm1.d = RDOP();
		m_imm1.b.h = RDOP();
		m_imm1.b.h2 = RDOP();
		m_imm1.b.h3 = RDOP();
		break;
	case p_M:
		m_ea1.d = m_ea2.d;
		break;
	case p_M8:
		m_ea1.d = RDOP();
		break;
	case p_M16:
		m_ea1.d = RDOP();
		m_ea1.b.h = RDOP();
		break;
	case p_R:
		m_p1_reg8 = m_p2_reg8;
		m_p1_reg16 = m_p2_reg16;
		m_p1_reg32 = m_p2_reg32;
		break;
	}

	switch ( inst->operand2 )
	{
	case p_A:
		m_p2_reg8 = &m_xwa[m_regbank].b.l;
		break;
	case p_F:        /* F' */
		m_p2_reg8 = &m_f2.b.l;
		break;
	case p_SR:
		m_p2_reg16 = &m_sr.w.l;
		break;
	case p_C8:
		m_p2_reg8 = get_reg8_current( m_op );
		break;
	case p_C16:
		m_p2_reg16 = get_reg16_current( m_op );
		break;
	case p_C32:
		m_p2_reg32 = get_reg32_current( m_op );
		break;
	case p_CR8:
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
	case p_CR16:
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
	case p_CR32:
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
	case p_D8:
		m_ea2.d = RDOP();
		m_ea2.d = m_pc.d + m_ea2.sb.l;
		break;
	case p_D16:
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		m_ea2.d = m_pc.d + m_ea2.sw.l;
		break;
	case p_I3:
		m_imm2.d = m_op & 0x07;
		break;
	case p_I8:
		m_imm2.d = RDOP();
		break;
	case p_I16:
		m_imm2.d = RDOP();
		m_imm2.b.h = RDOP();
		break;
	case p_I32:
		m_imm2.d = RDOP();
		m_imm2.b.h = RDOP();
		m_imm2.b.h2 = RDOP();
		m_imm2.b.h3 = RDOP();
		break;
	case p_M8:
		m_ea2.d = RDOP();
		break;
	case p_M16:
		m_ea2.d = RDOP();
		m_ea2.b.h = RDOP();
		break;
	}
}


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_80[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHBM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_RLDRM, p_A, p_M, 12 }, { &tlcs900h_device::op_RRDRM, p_A, p_M, 12 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDI, 0, 0, 10 }, { &tlcs900h_device::op_LDIR, 0, 0, 10 }, { &tlcs900h_device::op_LDD, 0, 0, 10 }, { &tlcs900h_device::op_LDDR, 0, 0, 10 },
	{ &tlcs900h_device::op_CPI, 0, 0, 8 }, { &tlcs900h_device::op_CPIR, 0, 0, 10 }, { &tlcs900h_device::op_CPD, 0, 0, 8 }, { &tlcs900h_device::op_CPDR, 0, 0, 10 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900h_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_CPBMI, p_M, p_I8, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18}, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18}, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCBM, p_M, 0, 8 }, { &tlcs900h_device::op_RRCBM, p_M, 0, 8 }, { &tlcs900h_device::op_RLBM, p_M, 0, 8 }, { &tlcs900h_device::op_RRBM, p_M, 0, 8 },
	{ &tlcs900h_device::op_SLABM, p_M, 0, 8 }, { &tlcs900h_device::op_SRABM, p_M, 0, 8 }, { &tlcs900h_device::op_SLLBM, p_M, 0, 8 }, { &tlcs900h_device::op_SRLBM, p_M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_88[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHBM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_RLDRM, p_A, p_M, 12 }, { &tlcs900h_device::op_RRDRM, p_A, p_M, 12 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900h_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_CPBMI, p_M, p_I8, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCBM, p_M, 0, 8 }, { &tlcs900h_device::op_RRCBM, p_M, 0, 8 }, { &tlcs900h_device::op_RLBM, p_M, 0, 8 }, { &tlcs900h_device::op_RRBM, p_M, 0, 8 },
	{ &tlcs900h_device::op_SLABM, p_M, 0, 8 }, { &tlcs900h_device::op_SRABM, p_M, 0, 8 }, { &tlcs900h_device::op_SLLBM, p_M, 0, 8 }, { &tlcs900h_device::op_SRLBM, p_M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_90[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHWM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDIW, 0, 0, 10 }, { &tlcs900h_device::op_LDIRW, 0, 0, 10 }, { &tlcs900h_device::op_LDDW, 0, 0, 10 }, { &tlcs900h_device::op_LDDRW, 0, 0, 10 },
	{ &tlcs900h_device::op_CPIW, 0, 0, 8 }, { &tlcs900h_device::op_CPIRW, 0, 0, 10 }, { &tlcs900h_device::op_CPDW, 0, 0, 8 }, { &tlcs900h_device::op_CPDRW, 0, 0, 10 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900h_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCWM, p_M, 0, 8 }, { &tlcs900h_device::op_RRCWM, p_M, 0, 8 }, { &tlcs900h_device::op_RLWM, p_M, 0, 8 }, { &tlcs900h_device::op_RRWM, p_M, 0, 8 },
	{ &tlcs900h_device::op_SLAWM, p_M, 0, 8 }, { &tlcs900h_device::op_SRAWM, p_M, 0, 8 }, { &tlcs900h_device::op_SLLWM, p_M, 0, 8 }, { &tlcs900h_device::op_SRLWM, p_M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_98[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHWM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900h_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCWM, p_M, 0, 8 }, { &tlcs900h_device::op_RRCWM, p_M, 0, 8 }, { &tlcs900h_device::op_RLWM, p_M, 0, 8 }, { &tlcs900h_device::op_RRWM, p_M, 0, 8 },
	{ &tlcs900h_device::op_SLAWM, p_M, 0, 8 }, { &tlcs900h_device::op_SRAWM, p_M, 0, 8 }, { &tlcs900h_device::op_SLLWM, p_M, 0, 8 }, { &tlcs900h_device::op_SRLWM, p_M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPBM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_POPWM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_ORCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_XORCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_LDCFBRM, p_A, p_M, 8 },
	{ &tlcs900h_device::op_STCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 },

	/* A0 - BF */
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 },

	/* C0 - DF */
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 },
	{ &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }, { &tlcs900h_device::op_RETCC, p_CC, 0, 6 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPBM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_POPWM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_ORCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_XORCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_LDCFBRM, p_A, p_M, 8 },
	{ &tlcs900h_device::op_STCFBRM, p_A, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 8 },

	/* A0 - BF */
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 },

	/* C0 - DF */
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHBM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_RLDRM, p_A, p_M, 12 }, { &tlcs900h_device::op_RRDRM, p_A, p_M, 12 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_LDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_EXBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ADCBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SUBBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_SBCBMI, p_M, p_I8, 7 },
	{ &tlcs900h_device::op_ANDBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_XORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_ORBMI, p_M, p_I8, 7 }, { &tlcs900h_device::op_CPBMI, p_M, p_I8, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 }, { &tlcs900h_device::op_MULSBRM, p_MC16, p_M, 18 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 }, { &tlcs900h_device::op_DIVBRM, p_MC16, p_M, 22 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },
	{ &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 }, { &tlcs900h_device::op_DIVSBRM, p_MC16, p_M, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECBIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCBM, p_M, 0, 8 }, { &tlcs900h_device::op_RRCBM, p_M, 0, 8 }, { &tlcs900h_device::op_RLBM, p_M, 0, 8 }, { &tlcs900h_device::op_RRBM, p_M, 0, 8 },
	{ &tlcs900h_device::op_SLABM, p_M, 0, 8 }, { &tlcs900h_device::op_SRABM, p_M, 0, 8 }, { &tlcs900h_device::op_SLLBM, p_M, 0, 8 }, { &tlcs900h_device::op_SRLBM, p_M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ADCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ADCBMR, p_M, p_C8, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SUBBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SUBBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_SBCBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_SBCBMR, p_M, p_C8, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ANDBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ANDBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_XORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_XORBMR, p_M, p_C8, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_ORBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_ORBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 }, { &tlcs900h_device::op_CPBRM, p_C8, p_M, 4 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 },
	{ &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 }, { &tlcs900h_device::op_CPBMR, p_M, p_C8, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDBRI, p_R, p_I8, 4 },
	{ &tlcs900h_device::op_PUSHBR, p_R, 0, 6 }, { &tlcs900h_device::op_POPBR, p_R, 0, 6 }, { &tlcs900h_device::op_CPLBR, p_R, 0, 4 }, { &tlcs900h_device::op_NEGBR, p_R, 0, 5 },
	{ &tlcs900h_device::op_MULBRI, p_R, p_I8, 18}, { &tlcs900h_device::op_MULSBRI, p_R, p_I8, 18 }, { &tlcs900h_device::op_DIVBRI, p_R, p_I8, 22 }, { &tlcs900h_device::op_DIVSBRI, p_R, p_I8, 24 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DAABR, p_R, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DJNZB, p_R, p_D8, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_ANDCFBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_ORCFBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_XORCFBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_LDCFBIR, p_I8, p_R, 4 },
	{ &tlcs900h_device::op_STCFBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_ANDCFBRR, p_A, p_R, 4 }, { &tlcs900h_device::op_ORCFBRR, p_A, p_R, 4 }, { &tlcs900h_device::op_XORCFBRR, p_A, p_R, 4 }, { &tlcs900h_device::op_LDCFBRR, p_A, p_R, 4 },
	{ &tlcs900h_device::op_STCFBRR, p_A, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDCBRR, p_CR8, p_R, 1 }, { &tlcs900h_device::op_LDCBRR, p_R, p_CR8, 1 },
	{ &tlcs900h_device::op_RESBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_SETBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_CHGBIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_BITBIR, p_I8, p_R, 4 },
	{ &tlcs900h_device::op_TSETBIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 },
	{ &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULBRR, p_MC16, p_R, 18 },
	{ &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 },
	{ &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 }, { &tlcs900h_device::op_MULSBRR, p_MC16, p_R, 18 },
	{ &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 },
	{ &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 }, { &tlcs900h_device::op_DIVBRR, p_MC16, p_R, 22 },
	{ &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 },
	{ &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 }, { &tlcs900h_device::op_DIVSBRR, p_MC16, p_R, 24 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCBIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECBIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 },
	{ &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCBR, p_CC, p_R, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADDBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_LDBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ADCBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 },
	{ &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 }, { &tlcs900h_device::op_LDBRR, p_R, p_C8, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SUBBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDBRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_SBCBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 },
	{ &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 }, { &tlcs900h_device::op_EXBRR, p_C8, p_R, 5 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ANDBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_ADDBRI, p_R, p_I8, 4 }, { &tlcs900h_device::op_ADCBRI, p_R, p_I8, 4 }, { &tlcs900h_device::op_SUBBRI, p_R, p_I8, 4 }, { &tlcs900h_device::op_SBCBRI, p_R, p_I8, 4 },
	{ &tlcs900h_device::op_ANDBRI, p_R, p_I8, 4 }, { &tlcs900h_device::op_XORBRI, p_R, p_I8, 4 }, { &tlcs900h_device::op_ORBRI, p_R, p_I8, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I8, 4 },
	{ &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_XORBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPBRI, p_R, p_I3, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_ORBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_RLCBIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_RRCBIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_RLBIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_RRBIR, p_I8, p_R, 6 },
	{ &tlcs900h_device::op_SLABIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_SRABIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_SLLBIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_SRLBIR, p_I8, p_R, 6 },
	{ &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 }, { &tlcs900h_device::op_CPBRR, p_C8, p_R, 4 },
	{ &tlcs900h_device::op_RLCBRR, p_A, p_R, 6 }, { &tlcs900h_device::op_RRCBRR, p_A, p_R, 6 }, { &tlcs900h_device::op_RLBRR, p_A, p_R, 6 }, { &tlcs900h_device::op_RRBRR, p_A, p_R, 6 },
	{ &tlcs900h_device::op_SLABRR, p_A, p_R, 6 }, { &tlcs900h_device::op_SRABRR, p_A, p_R, 6 }, { &tlcs900h_device::op_SLLBRR, p_A, p_R, 6 }, { &tlcs900h_device::op_SRLBRR, p_A, p_R, 6 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_PUSHWM, p_M, 0, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M16, p_M, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_EXWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ADCWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SUBWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_SBCWMI, p_M, p_I16, 8 },
	{ &tlcs900h_device::op_ANDWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_XORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_ORWMI, p_M, p_I16, 8 }, { &tlcs900h_device::op_CPWMI, p_M, p_I16, 6 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 }, { &tlcs900h_device::op_MULSWRM, p_C32, p_M, 26 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 }, { &tlcs900h_device::op_DIVWRM, p_C32, p_M, 30 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 },
	{ &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 }, { &tlcs900h_device::op_DIVSWRM, p_C32, p_M, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_INCWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 }, { &tlcs900h_device::op_DECWIM, p_I3, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_RLCWM, p_M, 0, 8 }, { &tlcs900h_device::op_RRCWM, p_M, 0, 8 }, { &tlcs900h_device::op_RLWM, p_M, 0, 8 }, { &tlcs900h_device::op_RRWM, p_M, 0, 8 },
	{ &tlcs900h_device::op_SLAWM, p_M, 0, 8 }, { &tlcs900h_device::op_SRAWM, p_M, 0, 8 }, { &tlcs900h_device::op_SLLWM, p_M, 0, 8 }, { &tlcs900h_device::op_SRLWM, p_M, 0, 8 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ADCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ADCWMR, p_M, p_C16, 6 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SUBWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SUBWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_SBCWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_SBCWMR, p_M, p_C16, 6 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ANDWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ANDWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_XORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_XORWMR, p_M, p_C16, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_ORWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_ORWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 }, { &tlcs900h_device::op_CPWRM, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 },
	{ &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 }, { &tlcs900h_device::op_CPWMR, p_M, p_C16, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWRI, p_R, p_I16, 4 },
	{ &tlcs900h_device::op_PUSHWR, p_R, 0, 5 }, { &tlcs900h_device::op_POPWR, p_R, 0, 6 }, { &tlcs900h_device::op_CPLWR, p_R, 0, 4 }, { &tlcs900h_device::op_NEGWR, p_R, 0, 5 },
	{ &tlcs900h_device::op_MULWRI, p_R, p_I16, 26 }, { &tlcs900h_device::op_MULSWRI, p_R, p_I16, 26 }, { &tlcs900h_device::op_DIVWRI, p_R, p_I16, 30 }, { &tlcs900h_device::op_DIVSWRI, p_R, p_I16, 32 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_BS1FRR, p_A, p_R, 4 }, { &tlcs900h_device::op_BS1BRR, p_A, p_R, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_EXTZWR, p_R, 0, 4 }, { &tlcs900h_device::op_EXTSWR, p_R, 0, 5 },
	{ &tlcs900h_device::op_PAAWR, p_R, 0, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_MIRRW, p_R, 0, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_MULAR, p_R, 0, 31 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DJNZW, p_R, p_D8, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_ANDCFWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_ORCFWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_XORCFWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_LDCFWIR, p_I8, p_R, 4 },
	{ &tlcs900h_device::op_STCFWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_ANDCFWRR, p_A, p_R, 4 }, { &tlcs900h_device::op_ORCFWRR, p_A, p_R, 4 }, { &tlcs900h_device::op_XORCFWRR, p_A, p_R, 4 }, { &tlcs900h_device::op_LDCFWRR, p_A, p_R, 4 },
	{ &tlcs900h_device::op_STCFWRR, p_A, p_R, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDCWRR, p_CR16, p_R, 1 }, { &tlcs900h_device::op_LDCWRR, p_R, p_CR16, 1 },
	{ &tlcs900h_device::op_RESWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_SETWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_CHGWIR, p_I8, p_R, 4 }, { &tlcs900h_device::op_BITWIR, p_I8, p_R, 4 },
	{ &tlcs900h_device::op_TSETWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_MINC1, p_I16, p_R, 8 }, { &tlcs900h_device::op_MINC2, p_I16, p_R, 8 }, { &tlcs900h_device::op_MINC4, p_I16, p_R, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_MDEC1, p_I16, p_R, 7 }, { &tlcs900h_device::op_MDEC2, p_I16, p_R, 7 }, { &tlcs900h_device::op_MDEC4, p_I16, p_R, 7 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 },
	{ &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULWRR, p_C32, p_R, 26 },
	{ &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 },
	{ &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 }, { &tlcs900h_device::op_MULSWRR, p_C32, p_R, 26 },
	{ &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 },
	{ &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 }, { &tlcs900h_device::op_DIVWRR, p_C32, p_R, 30 },
	{ &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 },
	{ &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 }, { &tlcs900h_device::op_DIVSWRR, p_C32, p_R, 32 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCWIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECWIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 },
	{ &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 }, { &tlcs900h_device::op_SCCWR, p_CC, p_R, 6 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADDWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_LDWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ADCWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 },
	{ &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 }, { &tlcs900h_device::op_LDWRR, p_R, p_C16, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SUBWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDWRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_SBCWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 },
	{ &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 }, { &tlcs900h_device::op_EXWRR, p_C16, p_R, 5 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ANDWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_ADDWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_ADCWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_SUBWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_SBCWRI, p_R, p_I16, 4 },
	{ &tlcs900h_device::op_ANDWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_XORWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_ORWRI, p_R, p_I16, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I16, 4 },
	{ &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_XORWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_CPWRI, p_R, p_I3, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_ORWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_RLCWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_RRCWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_RLWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_RRWIR, p_I8, p_R, 6 },
	{ &tlcs900h_device::op_SLAWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_SRAWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_SLLWIR, p_I8, p_R, 6 }, { &tlcs900h_device::op_SRLWIR, p_I8, p_R, 6 },
	{ &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 }, { &tlcs900h_device::op_CPWRR, p_C16, p_R, 4 },
	{ &tlcs900h_device::op_RLCWRR, p_A, p_R, 6 }, { &tlcs900h_device::op_RRCWRR, p_A, p_R, 6 }, { &tlcs900h_device::op_RLWRR, p_A, p_R, 6 }, { &tlcs900h_device::op_RRWRR, p_A, p_R, 6 },
	{ &tlcs900h_device::op_SLAWRR, p_A, p_R, 6 }, { &tlcs900h_device::op_SRAWRR, p_A, p_R, 6 }, { &tlcs900h_device::op_SLLWRR, p_A, p_R, 6 }, { &tlcs900h_device::op_SRLWRR, p_A, p_R, 6 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_LDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ADCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ADCLMR, p_M, p_C32, 10 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SUBLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SUBLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_SBCLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_SBCLMR, p_M, p_C32, 10 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ANDLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ANDLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_XORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_XORLMR, p_M, p_C32, 10 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_ORLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 }, { &tlcs900h_device::op_ORLMR, p_M, p_C32, 10 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 }, { &tlcs900h_device::op_CPLRM, p_C32, p_M, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_CPLMR, p_M, p_C32, 6 },
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDLRI, p_R, p_I32, 6 },
	{ &tlcs900h_device::op_PUSHLR, p_R, 0, 7 }, { &tlcs900h_device::op_POPLR, p_R, 0, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LINK, p_R, p_I16, 10 }, { &tlcs900h_device::op_UNLK, p_R, 0, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_EXTZLR, p_R, 0, 4 }, { &tlcs900h_device::op_EXTSLR, p_R, 0, 5 },
	{ &tlcs900h_device::op_PAALR, p_R, 0, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDCLRR, p_CR32, p_R, 1 }, { &tlcs900h_device::op_LDCLRR, p_R, p_CR32, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_INCLIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 }, { &tlcs900h_device::op_DECLIR, p_I3, p_R, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADDLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 },
	{ &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 }, { &tlcs900h_device::op_LDLRR, p_C32, p_R, 4 },
	{ &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ADCLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 },
	{ &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 }, { &tlcs900h_device::op_LDLRR, p_R, p_C32, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SUBLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 }, { &tlcs900h_device::op_LDLRI, p_R, p_I3, 4 },
	{ &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_SBCLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* C0 - DF */
	{ &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ANDLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_ADDLRI, p_R, p_I32, 7 }, { &tlcs900h_device::op_ADCLRI, p_R, p_I32, 7 }, { &tlcs900h_device::op_SUBLRI, p_R, p_I32, 7 }, { &tlcs900h_device::op_SBCLRI, p_R, p_I32, 7 },
	{ &tlcs900h_device::op_ANDLRI, p_R, p_I32, 7 }, { &tlcs900h_device::op_XORLRI, p_R, p_I32, 7 }, { &tlcs900h_device::op_ORLRI, p_R, p_I32, 7 }, { &tlcs900h_device::op_CPLRI, p_R, p_I32, 7 },
	{ &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_XORLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 },
	{ &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 }, { &tlcs900h_device::op_CPLRI, p_R, p_I3, 6 },

	/* E0 - FF */
	{ &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_ORLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_RLCLIR, p_I8, p_R, 8 }, { &tlcs900h_device::op_RRCLIR, p_I8, p_R, 8 }, { &tlcs900h_device::op_RLLIR, p_I8, p_R, 8 }, { &tlcs900h_device::op_RRLIR, p_I8, p_R, 8 },
	{ &tlcs900h_device::op_SLALIR, p_I8, p_R, 8 }, { &tlcs900h_device::op_SRALIR, p_I8, p_R, 8 }, { &tlcs900h_device::op_SLLLIR, p_I8, p_R, 8 }, { &tlcs900h_device::op_SRLLIR, p_I8, p_R, 8 },
	{ &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 }, { &tlcs900h_device::op_CPLRR, p_C32, p_R, 7 },
	{ &tlcs900h_device::op_RLCLRR, p_A, p_R, 8 }, { &tlcs900h_device::op_RRCLRR, p_A, p_R, 8 }, { &tlcs900h_device::op_RLLRR, p_A, p_R, 8 }, { &tlcs900h_device::op_RRLRR, p_A, p_R, 8 },
	{ &tlcs900h_device::op_SLALRR, p_A, p_R, 8 }, { &tlcs900h_device::op_SRALRR, p_A, p_R, 8 }, { &tlcs900h_device::op_SLLLRR, p_A, p_R, 8 }, { &tlcs900h_device::op_SRLLRR, p_A, p_R, 8 }
};


const tlcs900h_device::tlcs900inst tlcs900h_device::s_mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ &tlcs900h_device::op_LDBMI, p_M, p_I8, 5 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMI, p_M, p_I16, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPBM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_POPWM, p_M, 0, 6 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDBMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_LDWMM, p_M, p_M16, 8 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 }, { &tlcs900h_device::op_LDAW, p_C16, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBRM, p_A, p_M, 4 }, { &tlcs900h_device::op_ORCFBRM, p_A, p_M, 4 }, { &tlcs900h_device::op_XORCFBRM, p_A, p_M, 4 }, { &tlcs900h_device::op_LDCFBRM, p_A, p_M, 4 },
	{ &tlcs900h_device::op_STCFBRM, p_A, p_M, 4 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 }, { &tlcs900h_device::op_LDAL, p_C32, p_M, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 }, { &tlcs900h_device::op_LDBMR, p_M, p_C8, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 }, { &tlcs900h_device::op_LDWMR, p_M, p_C16, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 }, { &tlcs900h_device::op_LDLMR, p_M, p_C32, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ANDCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_ORCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_XORCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_LDCFBIM, p_I3, p_M, 4 },

	/* A0 - BF */
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_STCFBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 }, { &tlcs900h_device::op_TSETBIM, p_I3, p_M, 10 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_RESBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_SETBIM, p_I3, p_M, 8 },

	/* C0 - DF */
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 }, { &tlcs900h_device::op_CHGBIM, p_I3, p_M, 8 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 }, { &tlcs900h_device::op_BITBIM, p_I3, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },
	{ &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 }, { &tlcs900h_device::op_JPM, p_CC, p_M, 4 },

	/* E0 - FF */
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 }, { &tlcs900h_device::op_CALLM, p_CC, p_M, 6 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }
};


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in byte operations */
void tlcs900h_device::op_80()
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
void tlcs900h_device::op_88()
{
	const tlcs900inst *inst;

	/* For CPI/CPIR/CPD/CPDR/LDI/LDD/LDIR/LDDR operations */
	m_p1_reg32 = get_reg32_current( m_op - 1 );
	m_p2_reg32 = get_reg32_current( m_op );

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (int8_t)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_80[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIXI/XIY/XIZ/XSP) used as source in word operations */
void tlcs900h_device::op_90()
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
void tlcs900h_device::op_98()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (int8_t)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_98[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as source in long word operations */
void tlcs900h_device::op_A0()
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
void tlcs900h_device::op_A8()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (int8_t)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_a0[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* (XWA/XBC/XDE/XHL/XIX/XIY/XIZ/XSP) used as destination in operations */
void tlcs900h_device::op_B0()
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
void tlcs900h_device::op_B8()
{
	const tlcs900inst *inst;

	m_ea2.d = *get_reg32_current( m_op );
	m_op = RDOP();
	m_ea2.d += (int8_t)m_op;
	m_cycles += 2;
	m_op = RDOP();
	inst = &s_mnemonic_b8[m_op];
	prepare_operands( inst );
	(this->*inst->opfunc)();
	m_cycles += inst->cycles;
}


/* memory used as source in byte operations */
void tlcs900h_device::op_C0()
{
	const tlcs900inst *inst;
	uint32_t *reg = nullptr;

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
				m_ea2.d += (int8_t) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (int16_t) *get_reg16( m_op );
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
void tlcs900h_device::op_D0()
{
	const tlcs900inst *inst;
	uint32_t *reg = nullptr;

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
				m_ea2.d += (int8_t) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (int16_t) *get_reg16( m_op );
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
void tlcs900h_device::op_E0()
{
	const tlcs900inst *inst;
	uint32_t *reg = nullptr;

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
				m_ea2.d += (int8_t) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (int16_t) *get_reg16( m_op );
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


void tlcs900h_device::op_E8()
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
void tlcs900h_device::op_F0()
{
	const tlcs900inst *inst;
	uint32_t *reg = nullptr;

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
				m_ea2.d += (int8_t) *get_reg8( m_op );
				m_cycles += 8;
				break;

			/* (xrr+r16) */
			case 0x07:
				m_op = RDOP();
				m_ea2.d = *get_reg32( m_op );
				m_op = RDOP();
				m_ea2.d += (int16_t) *get_reg16( m_op );
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
	{ &tlcs900h_device::op_NOP, 0, 0, 1 }, { &tlcs900h_device::op_NORMAL, 0, 0, 4 }, { &tlcs900h_device::op_PUSHWR, p_SR, 0, 4 }, { &tlcs900h_device::op_POPWSR, p_SR, 0, 6 },
	{ &tlcs900h_device::op_MAX, 0, 0, 4 }, { &tlcs900h_device::op_HALT, 0, 0, 8 }, { &tlcs900h_device::op_EI, p_I8, 0, 5 }, { &tlcs900h_device::op_RETI, 0, 0, 12 },
	{ &tlcs900h_device::op_LDBMI, p_M8, p_I8, 5 }, { &tlcs900h_device::op_PUSHBI, p_I8, 0, 4 }, { &tlcs900h_device::op_LDWMI, p_M8, p_I16, 6 }, { &tlcs900h_device::op_PUSHWI, p_I16, 0, 5 },
	{ &tlcs900h_device::op_INCF, 0, 0, 2 }, { &tlcs900h_device::op_DECF, 0, 0, 2 }, { &tlcs900h_device::op_RET, 0, 0, 9 }, { &tlcs900h_device::op_RETD, p_I16, 0, 9 },
	{ &tlcs900h_device::op_RCF, 0, 0, 2 }, { &tlcs900h_device::op_SCF, 0, 0, 2 }, { &tlcs900h_device::op_CCF, 0, 0, 2 }, { &tlcs900h_device::op_ZCF, 0, 0, 2 },
	{ &tlcs900h_device::op_PUSHBR, p_A, 0, 3 }, { &tlcs900h_device::op_POPBR, p_A, 0, 4 }, { &tlcs900h_device::op_EXBRR, p_F, p_F, 2 }, { &tlcs900h_device::op_LDF, p_I8, 0, 2 },
	{ &tlcs900h_device::op_PUSHBR, p_F, 0, 3 }, { &tlcs900h_device::op_POPBR, p_F, 0, 4 }, { &tlcs900h_device::op_JPI, p_I16, 0, 7 }, { &tlcs900h_device::op_JPI, p_I24, 0, 7 },
	{ &tlcs900h_device::op_CALLI, p_I16, 0, 12 }, { &tlcs900h_device::op_CALLI, p_I24, 0, 12 }, { &tlcs900h_device::op_CALR, p_D16, 0, 12 }, { &tlcs900h_device::op_DB, 0, 0, 1 },

	/* 20 - 3F */
	{ &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 },
	{ &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 }, { &tlcs900h_device::op_LDBRI, p_C8, p_I8, 2 },
	{ &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 },
	{ &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 }, { &tlcs900h_device::op_PUSHWR, p_C16, 0, 3 },
	{ &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 },
	{ &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 }, { &tlcs900h_device::op_LDWRI, p_C16, p_I16, 3 },
	{ &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 },
	{ &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 }, { &tlcs900h_device::op_PUSHLR, p_C32, 0, 5 },

	/* 40 - 5F */
	{ &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 },
	{ &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 }, { &tlcs900h_device::op_LDLRI, p_C32, p_I32, 5 },
	{ &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 },
	{ &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 }, { &tlcs900h_device::op_POPWR, p_C16, 0, 4 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 }, { &tlcs900h_device::op_DB, 0, 0, 1 },
	{ &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 },
	{ &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 }, { &tlcs900h_device::op_POPLR, p_C32, 0, 6 },

	/* 60 - 7F */
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 }, { &tlcs900h_device::op_JR, p_CC, p_D8, 4 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 },
	{ &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 }, { &tlcs900h_device::op_JRL, p_CC, p_D16, 4 },

	/* 80 - 9F */
	{ &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 },
	{ &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 }, { &tlcs900h_device::op_80, 0, 0, 0 },
	{ &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 },
	{ &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 }, { &tlcs900h_device::op_88, 0, 0, 0 },
	{ &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 },
	{ &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 }, { &tlcs900h_device::op_90, 0, 0, 0 },
	{ &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 },
	{ &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 }, { &tlcs900h_device::op_98, 0, 0, 0 },

	/* A0 - BF */
	{ &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 },
	{ &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 }, { &tlcs900h_device::op_A0, 0, 0, 0 },
	{ &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 },
	{ &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 }, { &tlcs900h_device::op_A8, 0, 0, 0 },
	{ &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 },
	{ &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 }, { &tlcs900h_device::op_B0, 0, 0, 0 },
	{ &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 },
	{ &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 }, { &tlcs900h_device::op_B8, 0, 0, 0 },

	/* C0 - DF */
	{ &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 },
	{ &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_C0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 }, { &tlcs900h_device::oC8, 0, 0, 0 },
	{ &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 },
	{ &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_D0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },
	{ &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },
	{ &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 }, { &tlcs900h_device::oD8, 0, 0, 0 },

	/* E0 - FF */
	{ &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 },
	{ &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_E0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 },
	{ &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 },
	{ &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 }, { &tlcs900h_device::op_E8, 0, 0, 0 },
	{ &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 },
	{ &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_F0, 0, 0, 0 }, { &tlcs900h_device::op_DB, 0, 0, 0 }, { &tlcs900h_device::op_LDX, 0, 0, 9 },
	{ &tlcs900h_device::op_SWI, p_I3, 0, 16 }, { &tlcs900h_device::op_SWI, p_I3, 0, 16 }, { &tlcs900h_device::op_SWI, p_I3, 0, 16 }, { &tlcs900h_device::op_SWI, p_I3, 0, 16 },
	{ &tlcs900h_device::op_SWI, p_I3, 0, 16 }, { &tlcs900h_device::op_SWI, p_I3, 0, 16 }, { &tlcs900h_device::op_SWI, p_I3, 0, 16 }, { &tlcs900h_device::op_SWI, p_I3, 0, 16 }
};
