// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#define ARG_R   r1 = mem_readbyte( m_PC++ );

#define ARG_RR  r2 = mem_readbyte( m_PC++ ); \
		r1 = mem_readbyte( m_PC++ );

#define ARG_rR  r1 = op & 0x07; \
		r2 = mem_readbyte( m_PC++ );

#define ARG_iR  r2 = mem_readbyte( m_PC++ ); \
		r1 = mem_readbyte( m_PC++ );

#define ARG_Sw  r1 = mem_readbyte( m_PC++ ); \
		s2 = mem_readword( m_PC ); m_PC += 2;

#define ARG_rrw r1 = sm8500_b2w[op & 0x07]; \
		s2 = mem_readword( m_PC ); m_PC += 2;

#define ARG_ri  r1 = op & 0x07; \
		r2 = mem_readbyte( m_PC++ );

#define ARG_pi  r1 = 0x10 + ( op & 0x07 ); \
		r2 = mem_readbyte( m_PC++ );

#define ARG_rmb r1 = mem_readbyte( m_PC++ ); \
		s2 = 0; \
		switch( r1 & 0xC0 ) { \
		case 0x00: \
			s2 = mem_readbyte( r1 & 0x07 ); \
			break; \
		case 0x40: \
			s2 = mem_readbyte( r1 & 0x07 ); \
			mem_writebyte( r1 & 0x07, s2 + 1 ); \
			break; \
		case 0x80: \
			s2 = mem_readbyte( m_PC++ ); \
			if ( r1 & 0x07 ) { \
				s2 = s2 + mem_readbyte( r1 & 0x07 ); \
			} \
			break; \
		case 0xC0: \
			s2 = mem_readbyte( r1 & 0x07 ); \
			mem_writebyte( r1 & 0x07, s2 - 1 ); \
			break; \
		} \
		r2 = r1; \
		r1 = ( r1 >> 3 ) & 0x07;

#define ARG_rmw r1 = mem_readbyte( m_PC++ ); \
		s2 = 0; \
		switch( r1 & 0xC0 ) { \
		case 0x00: \
			s2 = mem_readword( sm8500_b2w[r1 & 0x07] ); \
			break; \
		case 0x40: \
			s2 = mem_readword( sm8500_b2w[r1 & 0x07] ); \
			mem_writeword( sm8500_b2w[r1 & 0x07], s2 + 1 ); \
			break; \
		case 0x80: \
			s2 = mem_readword( m_PC ); m_PC += 2; \
			if ( r1 & 0x07 ) { \
				s2 = s2 + mem_readword( sm8500_b2w[r1 & 0x07] ); \
			} \
			break; \
		case 0xC0: \
			s2 = mem_readword( sm8500_b2w[r1 & 0x07])-1; \
			mem_writeword( sm8500_b2w[r1 & 0x07], s2 ); \
			break; \
		} \
		r2 = r1; \
		r1 = ( r1 >> 3 ) & 0x07;

#define ARG_smw r1 = mem_readbyte( m_PC++ ); \
		s2 = 0; \
		switch( r1 & 0xC0 ) { \
		case 0x00: \
			s2 = mem_readword( sm8500_b2w[r1 & 0x07] ); \
			break; \
		case 0x40: \
			s2 = mem_readword( sm8500_b2w[r1 & 0x07] ); \
			mem_writeword( sm8500_b2w[r1 & 0x07], s2 + 1 ); \
			break; \
		case 0x80: \
			s2 = mem_readword( m_PC ); m_PC += 2; \
			if ( r1 & 0x07 ) { \
				s2 = s2 + mem_readword( sm8500_b2w[r1 & 0x07] ); \
			} \
			break; \
		case 0xC0: \
			s2 = mem_readword( sm8500_b2w[ r1 & 0x07] ); \
			mem_writeword( sm8500_b2w[r1 & 0x07], s2 - 1 ); \
			break; \
		} \
		r2 = r1; \
		r1 = sm8500_b2w[ ( r1 >> 3 ) & 0x07 ];

#define ARG_d8  r1 = mem_readbyte( m_PC++ ); \
		s2 = m_PC + ((INT8)r1);

#define ARG_Rbr r1 = mem_readbyte( m_PC++ ); \
		r2 = mem_readbyte( m_PC++ ); \
		s2 = m_PC + ((INT8)r2);

#define ARG_ad16    s2 = mem_readword( m_PC ); \
			m_PC += 2;

#define ARG_rr  r1 = mem_readbyte( m_PC++ ); \
		r2 = 0x00; \
		switch( r1 & 0xC0 ) { \
		case 0x00: \
			r2 = r1 & 0x07; \
			r1 = ( r1 >> 3 ) & 0x07; \
			break; \
		case 0x40: \
		case 0x80: \
		case 0xC0: \
			break; \
		}

#define ARG_ss  r1 = mem_readbyte( m_PC++ ); \
		r2 = 0x00; \
		switch( r1 & 0xC0 ) { \
		case 0x00: \
			r2 = sm8500_b2w[r1 & 0x07]; \
			r1 = sm8500_b2w[( r1 >> 3 ) & 0x07]; \
			break; \
		case 0x40: \
		case 0x80: \
		case 0xC0: \
			break; \
		}

#define ARG_2   r1 = mem_readbyte( m_PC++ ); \
		s2 = 0; \
		switch( r1 & 0xC0 ) { \
		case 0x00: \
			s2 = mem_readword( sm8500_b2w[ r1 & 0x07 ] ); \
			break; \
		case 0x40: \
			s2 = mem_readword( m_PC ); m_PC += 2; \
			if ( r1 & 0x38 ) { \
				s2 = s2 + mem_readbyte( ( r1 >> 3 ) & 0x07 ); \
			}  \
			s2 = mem_readword( s2 ); \
		case 0x80: \
		case 0xC0: \
			break; \
		}

#define ARG_RiR r1 = mem_readbyte( m_PC++ ); \
		d1 = mem_readbyte( m_PC++ ); \
		r2 = mem_readbyte( m_PC++ );

#define ARG_Rii r1 = mem_readbyte( m_PC++ ); \
		d1 = mem_readbyte( m_PC++ ); \
		r2 = mem_readbyte( m_PC++ );

#define ARG_riB r1 = mem_readbyte( m_PC++ ); \
		s2 = 1 << ( r1 & 0x07 ); \
		d1 = mem_readbyte( m_PC++ ); \
		if ( r1 & 0x38 ) { \
			s1 = d1 + mem_readbyte( ( r1 >> 3 ) & 0x07 ); \
		} else { \
			s1 = 0xFF00 + d1; \
		}

#define ARG_riBd    r1 = mem_readbyte( m_PC++ ); \
			s2 = 1 << ( r1 & 0x07 ); \
			d1 = mem_readbyte( m_PC++ ); \
			if ( r1 & 0x38 ) { \
				s1 = d1 + mem_readbyte( ( r1 >> 3 ) & 0x07 ); \
			} else { \
				s1 = 0xFF00 + d1; \
			} \
			d1 = mem_readbyte( m_PC++ );

#define OP_INTSUB8(X,Y,MASK)    d1 = X; \
				d2 = Y; \
				res = d1 - d2; \
				m_PS1 = m_PS1 & ( MASK ); \
				m_PS1 = m_PS1 | ( ( res > 0xFF ) ? FLAG_C : 0 ); \
				m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
				m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
				m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & ( res ^ d1 ) ) & 0x80 ) ? FLAG_V : 0 );

#define OP_CMP8(X,Y)    OP_INTSUB8( X, Y, (FLAG_B | FLAG_I | FLAG_H | FLAG_D ) );

#define OP_SUB8(X,Y)    OP_INTSUB8( X, Y, (FLAG_B | FLAG_I ) ); \
			m_PS1 = m_PS1 | FLAG_D; \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x10 ) ? FLAG_H : 0 );

#define OP_INTSUB16(X,Y,MASK)   d1 = X; \
				d2 = Y; \
				res = d1 - d2; \
				m_PS1 = m_PS1 & ( MASK ); \
				m_PS1 = m_PS1 | ( ( res > 0xFFFF ) ? FLAG_C : 0 ); \
				m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 ); \
				m_PS1 = m_PS1 | ( ( res & 0x8000 ) ? FLAG_S : 0 ); \
				m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & (res ^ d1) ) & 0x8000 ) ? FLAG_V : 0 );

#define OP_CMP16(X,Y)   OP_INTSUB16( X, Y, ( FLAG_B | FLAG_I | FLAG_H | FLAG_D ) );

#define OP_SUB16(X,Y)   OP_INTSUB16( X, Y, ( FLAG_B | FLAG_I ) ); \
			m_PS1 = m_PS1 | FLAG_D; \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x0010 ) ? FLAG_H : 0 );

#define OP_SBC8(X,Y)    d1 = X; \
			d2 = Y; \
			res = d1 - d2 - ((m_PS1 & FLAG_C) ? 1 : 0); \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( res > 0xFF ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0); \
			m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & (res ^ d1) ) & 0x80 ) ? FLAG_V : 0 ); \
			m_PS1 = m_PS1 | FLAG_D; \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x10 ) ? FLAG_H : 0 );

#define OP_SBC16(X,Y)   d1 = X; \
			d2 = Y; \
			res = d1 - d2 - ((m_PS1 & FLAG_C) ? 1 : 0); \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( res > 0xFFFF ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x8000 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & ( res ^ d1 ) ) & 0x8000 ) ? FLAG_V : 0 ); \
			m_PS1 = m_PS1 | FLAG_D; \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x10 ) ? FLAG_H : 0 );

#define OP_ADD8(X,Y)    d1 = X; \
			d2 = Y; \
			res = d1 + d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( res > 0xFF ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0); \
			m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ^ 0x80 ) & (res ^ d1) ) & 0x80 ) ? FLAG_V : 0 ); \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x10 ) ? FLAG_H : 0 );

#define OP_ADD16(X,Y)   d1 = X; \
			d2 = Y; \
			res = d1 + d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( res > 0xFFFF ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x8000 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & ( res ^ d1 ) ) & 0x8000 ) ? FLAG_V : 0 ); \
			m_PS1 = m_PS1 | ( ( ( d2 ^ d1 ^ res ) & 0x0010 ) ? FLAG_H : 0 );

#define OP_ADC8(X,Y)    d1 = X; \
			d2 = Y; \
			res = d1 + d2 + ((m_PS1 & FLAG_C) ? 1 : 0); \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( res > 0xFF ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0); \
			m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & (res ^ d1) ) & 0x80 ) ? FLAG_V : 0 ); \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x10 ) ? FLAG_H : 0 );

#define OP_ADC16(X,Y)   d1 = X; \
			d2 = Y; \
			res = d1 + d2 + ((m_PS1 & FLAG_C) ? 1 : 0); \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( res > 0xFFFF ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x8000 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d2 ^ d1 ) & ( res ^ d1) ) & 0x8000 ) ? FLAG_V : 0 ); \
			m_PS1 = m_PS1 | ( ( ( d1 ^ d2 ^ res ) & 0x10 ) ? FLAG_H : 0 );

#define OP_NEG8(X)  res = -X; \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_C | FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0x80 ) ? FLAG_V : 0 );

#define OP_COM8(X)  res = ~X; \
			m_PS1 = m_PS1 & ( FLAG_C | FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_RR8(X)   d1 = X; \
			res = d1 >> 1; \
			if ( d1 & 0x01 ) { \
				res |= 0x80; \
			} \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x01 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d1 ^ res ) & 0x80 ) && ! ( res & 0x80 ) ) ? FLAG_V : 0 );

#define OP_RL8(X)   d1 = X; \
			res = d1 << 1; \
			if ( d1 & 0x80 ) { \
				res |= 0x01; \
			} \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x80 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( d1 ^ res ) & 0x80 ) ? FLAG_V : 0 );

#define OP_RRC8(X)  d1 = X; \
			res = d1 >> 1; \
			if ( m_PS1 & FLAG_C ) { \
				res |= 0x80; \
			} \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x01 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d1 ^ res ) & 0x80 ) && ! ( res & 0x80 ) ) ? FLAG_V : 0 );

#define OP_RLC8(X)  d1 = X; \
			res = d1 << 1; \
			if ( m_PS1 & FLAG_C ) { \
				res |= 0x01; \
			} \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x80 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( d1 ^ res ) & 0x80 ) ? FLAG_V : 0 );

#define OP_SRL8(X)  d1 = X; \
			res = d1 >> 1; \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x01 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 );

#define OP_SRA8(X)  d1 = X; \
			res = d1 >> 1; \
			if ( d1 & 0x80 ) { \
				res |= 0x80; \
			} \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x01 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_SLL8(X)  d1 = X; \
			res = d1 << 1; \
			m_PS1 = m_PS1 & ( FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( d1 & 0x80 ) ? FLAG_C : 0 ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_INC8(X)  d1 = X; \
			res = d1 + 1; \
			m_PS1 = m_PS1 & ( FLAG_C | FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d1 ^ res ) & 0x80 ) && ! ( res & 0x80 ) ) ? FLAG_V : 0 );

#define OP_INC16(X) d1 = X; \
			res = d1 + 1; \
			m_PS1 = m_PS1 & ( FLAG_C | FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x8000 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d1 ^ res ) & 0x8000 ) && ! ( res & 0x8000 ) ) ? FLAG_V : 0 );

#define OP_DEC8(X)  d1 = X; \
			res = d1 - 1; \
			m_PS1 = m_PS1 & ( FLAG_C | FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d1 ^ res ) & 0x80 ) && ( res & 0x80 ) ) ? FLAG_V : 0 );

#define OP_DEC16(X) d1 = X; \
			res = d1 - 1; \
			m_PS1 = m_PS1 & ( FLAG_C | FLAG_D | FLAG_H | FLAG_B | FLAG_I ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x8000 ) ? FLAG_S : 0 ); \
			m_PS1 = m_PS1 | ( ( ( ( d1 ^ res ) & 0x8000 ) && ( res & 0x8000 ) ) ? FLAG_V : 0 );

#define OP_AND8(X,Y)    d1 = X; \
			d2 = Y; \
			res = d1 & d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I | FLAG_H | FLAG_D ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_AND16(X,Y)   d1 = X; \
			d2 = Y; \
			res = d1 & d2; \
			m_PS1 = m_PS1 & ( FLAG_C | FLAG_S | FLAG_B | FLAG_I | FLAG_H | FLAG_D ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 );

#define OP_OR8(X,Y) d1 = X; \
			d2 = Y; \
			res = d1 | d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I | FLAG_H | FLAG_D ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_OR16(X,Y)    d1 = X; \
			d2 = Y; \
			res = d1 | d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I | FLAG_H | FLAG_D | FLAG_C | FLAG_S ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 );

#define OP_XOR8(X,Y)    d1 = X; \
			d2 = Y; \
			res = d1 ^ d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I | FLAG_H | FLAG_D ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_XOR16(X,Y)   d1 = X; \
			d2 = Y; \
			res = d1 ^ d2; \
			m_PS1 = m_PS1 & ( FLAG_B | FLAG_I | FLAG_H | FLAG_D | FLAG_C | FLAG_S ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFFFF ) == 0 ) ? FLAG_Z : 0 );

#define OP_DA8(X)   d1 = X; \
			res = d1; \
			if ( m_PS1 & FLAG_D ) { \
				if ( m_PS1 & FLAG_C ) { \
					if ( m_PS1 & FLAG_H ) { \
						res += 0x9A; \
					} else { \
						res += 0xA0; \
					} \
				} else { \
					if ( m_PS1 & FLAG_H ) { \
						res += 0xFA; \
					} \
				} \
			} else { \
				if ( m_PS1 & FLAG_C ) { \
					if ( m_PS1 & FLAG_H ) { \
						res += 0x66; \
					} else { \
						if ( ( res & 0x0F ) < 10 ) { \
							res += 0x60; \
						} else { \
							res += 0x66; \
						} \
					} \
				} else { \
					if ( m_PS1 & FLAG_H ) { \
						if ( ( res & 0xF0 ) < 0xA0 ) { \
							res += 0x06; \
						} else { \
							res += 0x66; \
							m_PS1 = m_PS1 | FLAG_C; \
						} \
					} else { \
						if ( ( res & 0x0F ) < 10 ) { \
							if ( ( res & 0xF0 ) >= 0xA0 ) { \
								res += 0x60; \
								m_PS1 = m_PS1 | FLAG_C; \
							} \
						} else { \
							if ( ( res & 0xF0 ) < 0x90 ) { \
								res += 0x06; \
							} else { \
								res += 0x66; \
								m_PS1 = m_PS1 | FLAG_C; \
							} \
						} \
					} \
				} \
			} \
			m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_S ); \
			m_PS1 = m_PS1 | ( ( ( res & 0xFF ) == 0x00 ) ? FLAG_Z : 0 ); \
			m_PS1 = m_PS1 | ( ( res & 0x80 ) ? FLAG_S : 0 );

#define OP_SWAP8(X) d1 = X; \
			res = ( d1 << 4 ) | ( d1 >> 4 );

#define CHECK_CC    res = 0; \
			switch( op & 0x0F ) { \
			case 0x00: /* F   */ res = 0; break; \
			case 0x01: /* LT  */ if ( ( m_PS1 & FLAG_S ) ^ ( ( m_PS1 & FLAG_V ) << 1 ) ) res = 1; break; \
			case 0x02: /* LE  */ if ( ( ( m_PS1 & FLAG_S ) && ! ( m_PS1 & FLAG_V ) ) || ( ( m_PS1 & FLAG_S ) && ( m_PS1 & FLAG_V ) && ( m_PS1 & FLAG_Z ) ) || ( ! ( m_PS1 & FLAG_S ) && ( ( m_PS1 & FLAG_Z ) || (m_PS1 & FLAG_V ) ) ) ) res = 1; break; \
			case 0x03: /* ULE */ if ( m_PS1 & FLAG_Z || m_PS1 & FLAG_C ) res = 1; break; \
			case 0x04: /* OV  */ if ( m_PS1 & FLAG_V ) res = 1; break; \
			case 0x05: /* MI  */ if ( m_PS1 & FLAG_S ) res = 1; break; \
			case 0x06: /* Z   */ if ( m_PS1 & FLAG_Z ) res = 1; break; \
			case 0x07: /* C   */ if ( m_PS1 & FLAG_C ) res = 1; break; \
			case 0x08: /* T   */ res = 1; break; \
			case 0x09: /* GE  */ if ( ! ( ( m_PS1 & FLAG_S ) ^ ( ( m_PS1 & FLAG_V ) << 1 ) ) ) res = 1; break; \
			case 0x0A: /* GT  */ if ( ( ! ( m_PS1 & FLAG_Z ) && ( m_PS1 & FLAG_S ) && ( m_PS1 & FLAG_V ) ) || ( ! ( m_PS1 & FLAG_Z ) && ! ( m_PS1 & FLAG_V ) && ! ( m_PS1 & FLAG_S ) ) ) res = 1; break; \
			case 0x0B: /* UGT */ if ( ! ( m_PS1 & FLAG_Z || m_PS1 & FLAG_C ) ) res = 1; break; \
			case 0x0C: /* NOV */ if ( ! (m_PS1 & FLAG_V) ) res = 1; break; \
			case 0x0D: /* PL  */ if ( ! (m_PS1 & FLAG_S) ) res = 1; break; \
			case 0x0E: /* NZ  */ if ( ! (m_PS1 & FLAG_Z) ) res = 1; break; \
			case 0x0F: /* NC  */ if ( ! (m_PS1 & FLAG_C) ) res = 1; break; \
			}

#define PUSH8(X)    m_SP--; \
			if ( ( m_SYS & 0x40 ) == 0 ) m_SP &= 0xFF; \
			mem_writebyte( m_SP, X );

#define POP8(X)     X = mem_readbyte( m_SP ); \
			m_SP++; \
			if ( ( m_SYS & 0x40 ) == 0 ) m_SP &= 0xFF;

case 0x00:  /* CLR R - 4 cycles - Flags affected: -------- */
	ARG_R;
	mem_writebyte( r1, 0 );
		mycycles += 4;
	break;
case 0x01:  /* NEG R - 5 cycles - Flags affected: CZSV---- */
	ARG_R;
	OP_NEG8( mem_readbyte( r1 ) );
	mem_writebyte( r1 , res & 0xFF );
	mycycles += 5;
	break;
case 0x02:  /* COM R - 4 cycles - Flags affected: -ZS0---- */
	ARG_R;
	OP_COM8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x03:  /* RR R - 4 cycles - Flags affected: CZSV---- */
	ARG_R;
	OP_RR8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x04:  /* RL R - 4 cycles - Flags affected: CZSV---- */
	ARG_R;
	OP_RL8( mem_readbyte( r1 ) );
	mem_writebyte( r1 , res & 0xFF );
	mycycles += 4;
	break;
case 0x05:  /* RRC R - 4 cycles - Flags affected: CZSV---- */
	ARG_R;
	OP_RRC8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x06:  /* RLC R - 4 cycles - Flags affected: CZSV---- */
	ARG_R;
	OP_RLC8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x07:  /* SRL R - 4 cycles - Flags affected: CZ00---- */
	ARG_R;
	OP_SRL8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x08:  /* INC R - 4 cycles - Flags affected: -ZSV---- */
	ARG_R;
	OP_INC8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x09:  /* DEC R - 4 cycles - Flags affected: -ZSV---- */
	ARG_R;
	OP_DEC8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x0A:  /* SRA R - 4 cycles - Flags affected: CZS0---- */
	ARG_R;
	OP_SRA8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x0B:  /* SLL R - 4 cycles - Flags affected: CZS0---- */
	ARG_R;
	OP_SLL8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x0C:  /* DA R - 4 cycles - Flags affected: CZS----- */
	ARG_R;
	OP_DA8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 4;
	break;
case 0x0D:  /* SWAP R - 7 cycles - Flags affected: -------- */
	ARG_R;
	OP_SWAP8( mem_readbyte( r1 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 7;
	break;
case 0x0E:  /* PUSH R - 5?/12? (8bit SP),10 (16bit SP) cycles */
	ARG_R;
	PUSH8( mem_readbyte( r1 ) );
	mycycles += ( ( m_SYS & 0x40 ) ? 12 : 10 );
	break;
case 0x0F:  /* POP R - 9,8 cycles */
	ARG_R;
	POP8( r2 );
	mem_writebyte( r1, r2 );
	mycycles += ( ( m_SYS & 0x40 ) ? 9 : 8 );
	break;
case 0x10:  /* CMP Rr,Rs - 5 cycles - Flags affected: CZSV---- */
	ARG_rr;
	OP_CMP8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mycycles += 5;
	break;
case 0x11:  /* ADD Rr,Rs - 5 cycles - Flags affected: CZSV0H-- */
	ARG_rr;
	OP_ADD8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x12:  /* SUB Rr,Rs - 5 cycles - Flags affected: CZSV1H-- */
	ARG_rr;
	OP_SUB8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x13:  /* ADC Rr,Rs - 5 cycles - Flags affected: CZSV0H-- */
	ARG_rr;
	OP_ADC8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x14:  /* SBC Rr,Rs - 5 cycles - Flags affected: CZSV1H-- */
	ARG_rr;
	OP_SBC8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x15:  /* AND Rr,Rs - 5 cycles - Flags affected: -ZS0---- */
	ARG_rr;
	OP_AND8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x16:  /* OR Rr,Rs - 5 cycles - Flags affected: -ZS0---- */
	ARG_rr;
	OP_OR8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x17:  /* XOR Rr,Rs - 5 cycles - Flags affected: -ZS0---- */
	ARG_rr;
	OP_XOR8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 5;
	break;
case 0x18:  /* INCW S - 8 cycles - Flags affected: -ZSV---- */
	ARG_R;
	OP_INC16( mem_readword( r1 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 8;
	break;
case 0x19:  /* DECW S - 8 cycles - Flags affected: -ZSV---- */
	ARG_R;
	OP_DEC16( mem_readword( r1 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 8;
	break;
case 0x1A:  /* CLR/NEG/COM/RR/RL/RRC/RLC/SRL @Rr - 7/8/7/7/7/7/7/6 cycles */
	ARG_rr;
	res = 0;
	s1 = mem_readbyte( r1 );
	switch( r2 ) {
	case 0x00:  /* Flags affected: -------- */
		res = 0;
		mycycles += 7;
		break;
	case 0x01:  /* Flags affected: CZSV---- */
		OP_NEG8( mem_readbyte( s1 ) );
		mycycles += 8;
		break;
	case 0x02:  /* Flags affected: -ZS0---- */
		OP_COM8( mem_readbyte( s1 ) );
		mycycles += 7;
		break;
	case 0x03:  /* Flags affected: CZSV---- */
		OP_RR8( mem_readbyte( s1 ) );
		mycycles += 7;
		break;
	case 0x04:  /* Flags affected: CZSV---- */
		OP_RL8( mem_readbyte( s1 ) );
		mycycles += 7;
		break;
	case 0x05:  /* Flags affected: CZSV---- */
		OP_RRC8( mem_readbyte( s1 ) );
		mycycles += 7;
		break;
	case 0x06:  /* Flags affected: CZSV---- */
		OP_RLC8( mem_readbyte( s1 ) );
		mycycles += 7;
		break;
	case 0x07:  /* Flags affected: CZ00---- */
		OP_SRL8( mem_readbyte( s1 ) );
		mycycles += 6;
		break;
	}
	mem_writebyte( s1, res & 0xFF );
	break;
case 0x1B:  /* INC/DEC/SRA/SLL/DA/SWAP/PUSH/POP @Rr - 7,7,6,7,9,13,8,12,11 cycles */
	ARG_rr;
	s1 = mem_readbyte( r1 );
	switch( r2 ) {
	case 0x00:  /* Flags affected: -ZSV---- */
		OP_INC8( mem_readbyte( s1 ) );
		mem_writebyte( s1, res & 0xFF );
		mycycles += 7;
		break;
	case 0x01:  /* Flags affected: -ZSV---- */
		OP_DEC8( mem_readbyte( s1 ) );
		mem_writebyte( s1, res & 0xFF );
		mycycles += 7;
		break;
	case 0x02:  /* Flags affected: CZS0---- */
		OP_SRA8( mem_readbyte( s1 ) );
		mem_writebyte( s1, res & 0xFF );
		mycycles += 6;
		break;
	case 0x03:  /* Flags affected: CZS0---- */
		OP_SLL8( mem_readbyte( s1 ) );
		mem_writebyte( s1, res & 0xFF );
		mycycles += 6;
		break;
	case 0x04:  /* Flags affected: CZS----- */
		OP_DA8( mem_readbyte( s1 ) );
		mem_writebyte( s1, res & 0xFF );
		mycycles += 7;
		break;
	case 0x05:  /* Flags affected: -------- */
		OP_SWAP8( mem_readbyte( s1 ) );
		mem_writebyte( s1, res & 0xFF );
		mycycles += 9;
		break;
	case 0x06:  /* Flags affected: -------- */
		PUSH8( mem_readbyte( s1 ) );
		mycycles += ( ( m_SYS & 0x40 ) ? 13 : 8 );
		break;
	case 0x07:  /* Flags affected: -------- */
		POP8( res );
		mem_writebyte( s1, res );
		mycycles += ( ( m_SYS & 0x40 ) ? 12 : 11 );
		break;
	}
	break;
case 0x1C:  /* BCLR 0xFFdd/d8(r),#b - 12,8 cycles - Flags affected: -------- */
	ARG_riB;
	mem_writebyte( s1, mem_readbyte( s1 ) & ~s2 );
	mycycles += ( ( r1 & 0x38 ) ? 8 : 12 );
	break;
case 0x1D:  /* BSET 0xFFdd/d8(r),#b - 12,8 cycles - Flags affected: -------- */
	ARG_riB;
	mem_writebyte( s1, mem_readbyte( s1 ) | s2 );
	mycycles += ( ( r1 & 0x38 ) ? 8 : 12 );
	break;
case 0x1E:  /* PUSHW S - 12,9 cycles - Flags affected: -------- */
	ARG_R;
	PUSH8( mem_readbyte( r1 + 1 ) );
	PUSH8( mem_readbyte( r1 ) );
	mycycles += ( ( m_SYS & 0x40 ) ? 12 : 9 );
	break;
case 0x1F:  /* POPW S - 12,13 cycles - Flags affected: -------- */
	ARG_R;
	POP8( r2 );
	mem_writebyte( r1, r2 );
	POP8( r2 );
	mem_writebyte( r1 + 1, r2 );
	mycycles += ( ( m_SYS & 0x40 ) ? 12 : 13 );
	break;
case 0x20:  /* CMP r,@r / CMP r,(r)+ / CMP r,@w / CMP r,w(r) / CMP r,-(r) - 7,8,10,8,9 cycles - Flags affected: CZSV---- */
	ARG_rmb;
	OP_CMP8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x21:  /* ADD r,@r / ADD r,(r)+ / ADD r,@w / ADD r,w(r) / ADD r,-(r) - 7,8,10,8,9 cycles - Flags affected: CZSV0H-- */
	ARG_rmb;
	OP_ADD8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x22:  /* SUB r,@r / SUB r,(r)+ / SUB r,@w / SUB r,w(r) / SUB r,-(r) - 7,8,10,8,9 cycles - Flags affected: CZSV1H-- */
	ARG_rmb;
	OP_SUB8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x23:  /* ADC r,@r / ADC r,(r)+ / ADC r,@w / ADC r,w(r) / ADC r,-(r) - 7,8,10,8,9 cycles - Flags affected: CZSV0H-- */
	ARG_rmb;
	OP_ADC8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x24:  /* SBC r,@r / SBC r,(r)+ / SBC r,@w / SBC r,w(r) / SBC r,-(r) - 7,8,10,8,9 cycles - Flags affected: CZSV1H-- */
	ARG_rmb;
	OP_SBC8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x25:  /* AND r,@r / AND r,(r)+ / AND r,@w / AND r,w(r) / AND r,-(r) - 7,8,10,8,9 cycles - Flags affected: -ZS0---- */
	ARG_rmb;
	OP_AND8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x26:  /* OR r,@r / OR r,(r)+ / OR r,@w / OR r,w(r) / OR r,-(r) - 7,8,10,8,9 cycles - Flags affected: -ZS0---- */
	ARG_rmb;
	OP_OR8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x27:  /* XOR r,@r / XOR r,(r)+ / XOR r,@w / XOR r,w(r) / XOR r,-(r) - 7,8,10,8,9 cycles - Flags affected: -ZS0---- */
	ARG_rmb;
	OP_XOR8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 8 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x28:  /* MOV r,@r / MOV r,(r)+ / MOV r,@w / MOV r,w(r) / MOV r,-(r) - 6,7,10,7,8 cycles - Flags affected: -------- */
	ARG_rmb;
	mem_writebyte( r1, mem_readbyte( s2 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 6; break;
	case 0x40:  mycycles += 7; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 7 : 10 ); break;
	case 0xC0:  mycycles += 8; break;
	}
	break;
case 0x29:  /* MOV @r,r / MOV (r)+,r / MOV @w,r / MOV w(r),r / MOV -(r),r - 8,8,10,9,9 cycles - Flags affected: -------- */
	ARG_rmb;
	mem_writebyte( s2, mem_readbyte( r1 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 8; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 9 : 10 ); break;
	case 0xC0:  mycycles += 9; break;
	}
	break;
case 0x2A:  /* BBC FFii/i(Rr),#b,d8 - 16,12/14,10 cycles - Flags affected: -------- */
	ARG_riBd;
	if ( mem_readbyte( s1 ) & s2 ) {
		mycycles += 10;
	} else {
		m_PC = m_PC + ((INT8)d1);
		mycycles += 14;
	}
	if ( ( r1 & 0x38 ) == 0 ) {
		mycycles += 2;
	}
	break;
case 0x2B:  /* BBS FFii/i(Rr),#b,d8 - 16,12/14,10 cycles - Flags affected: -------- */
	ARG_riBd;
	if ( mem_readbyte( s1 ) & s2 ) {
		m_PC = m_PC + ((INT8)d1);
		mycycles += 14;
	} else {
		mycycles += 10;
	}
	if ( ( r1 & 0x38 ) == 0 ) {
		mycycles += 2;
	}
	break;
case 0x2C:  /* EXTS Rr - 6 cycles - Flags affected: -------- */
	ARG_R;
	res = mem_readword( r1 );
	if ( res & 0x80 ) {
		res = res | 0xFF00;
	} else {
		res = res & 0x00FF;
	}
	mem_writeword( r1, res );
	mycycles += 6;
	break;
case 0x2D:  /* unk2D - 4 cycles */
logerror( "%04X: unk%02x\n", m_PC-1,op );
	mycycles += 4;
	break;
case 0x2E:  /* MOV PS0,#00 - 4 cycles - Flags affected: -------- */
	ARG_R;
	m_PS0 = r1;
		mycycles += 4;
	break;
case 0x2F:  /* BTST R,i - 6 cycles - Flags affected: -Z-0---- */
	ARG_RR;
	m_PS1 = m_PS1 & ~ FLAG_V;
	if ( ( mem_readbyte( r2 ) & r1 ) == 0x00 ) {
		m_PS1 = m_PS1 | FLAG_Z;
	} else {
		m_PS1 = m_PS1 & ( ~ FLAG_Z );
	}
	mycycles += 6;
	break;
case 0x30:  /* CMP r,@rr / CMP r,(rr)+ / CMP r,@ww / CMP r,ww(rr) / CMP r,-(rr) - 8,13,11,15,13 cycles - Flags affected: CZSV---- */
	ARG_rmw;
	OP_CMP8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x31:  /* ADD r,@rr / ADD r,(rr)+ / ADD r,@ww / ADD r,ww(rr) / ADD r,-(rr) - 8,13,11,15,13 cycles - Flags affected: CZSV0H-- */
	ARG_rmw;
	OP_ADD8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x32:  /* SUB r,@rr / SUB r,(rr)+ / SUB r,@ww / SUB r,ww(rr) / SUB r,-(rr) - 8,13,11,15,13 cycles - Flags affected: CZSV1H-- */
	ARG_rmw;
	OP_SUB8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x33:  /* ADC r,@rr / ADC r,(rr)+ / ADC r,@ww / ADC r,ww(rr) / ADC r,-(rr) - 8,13,11,15,13 cycles - Flags affected: CZSV0H-- */
	ARG_rmw;
	OP_ADC8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x34:  /* SBC r,@rr / SBC r,(rr)+ / SBC r,@ww / SBC r,ww(rr) / SBC r,-(rr) - 8,13,11,15,13 cycles - Flags affected: CZSV1H-- */
	ARG_rmw;
	OP_SBC8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x35:  /* AND r,@rr / AND r,(rr)+ / AND r,@ww / AND r,ww(rr) / AND r,-(rr) - 8,13,11,15,13 cycles - Flags affected: -ZS0---- */
	ARG_rmw;
	OP_AND8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x36:  /* OR r,@rr / OR r,(rr)+ / OR r,@ww / OR r,ww(rr) / OR r,-(rr) - 8,13,11,15,13 cycles - Flags affected: -ZS0---- */
	ARG_rmw;
	OP_OR8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x37:  /* XOR? r,@rr / XOR r,(rr)+ / XOR r,@ww / XOR r,ww(rr) / XOR r,-(rr) - 8,13,11,15,13 cycles - Flagsaffected: -ZS0---- */
	ARG_rmw;
	OP_XOR8( mem_readbyte( r1 ), mem_readbyte( s2 ) );
	mem_writebyte( r1, res & 0xFF );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x38:  /* MOV r,@rr / MOV r,(rr)+ / MOV r,@ww / MOV r,ww(rr) / MOV r,-(rr) - 8,13,11,15,13 cycles - Flags affected: -------- */
	ARG_rmw;
	mem_writebyte( r1, mem_readbyte( s2 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x39:  /* MOV @rr,r / MOV (rr)+,r / MOV @ww,r / MOV ww(rr),r /  MOV -(rr),r - 8,13,11,15,13 cycles - Flags affected: -------- */
	ARG_rmw;
	mem_writebyte( s2, mem_readbyte( r1 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 8; break;
	case 0x40:  mycycles += 13; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 11 : 15 ); break;
	case 0xC0:  mycycles += 13; break;
	}
	break;
case 0x3A:  /* MOVW rr,@rr / MOV rr,(rr)+ / MOV rr,@ww / MOV rr,ww(rr) / MOV rr,-(rr) - 11,16,14,18,16 cycles - Flags affected: -------- */
	ARG_smw;
	mem_writeword( r1, mem_readword( s2 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 11; break;
	case 0x40:  mycycles += 16; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 18 : 14 ); break;
	case 0xC0:  mycycles += 16; break;
	}
	break;
case 0x3B:  /* MOVW @rr,rr / MOV (rr)+,rr / MOV @ww,rr / MOV ww(rr),rr / MOV -(rr),rr - 11,16,14,18,16 cycles - Flags affected: -------- */
	ARG_smw;
	mem_writeword( s2, mem_readword( r1 ) );
	switch( r2 & 0xC0 ) {
	case 0x00:  mycycles += 11; break;
	case 0x40:  mycycles += 16; break;
	case 0x80:  mycycles += ( ( r2 & 0x07 ) ? 18 : 14 ); break;
	case 0xC0:  mycycles += 16; break;
	}
	break;
case 0x3C:  /* MOVW RRr,RRs - 7 cycles - Flags affected: -------- */
	ARG_ss;
	mem_writeword( r1, mem_readword( r2 ) );
	mycycles += 7;
	break;
case 0x3D:  /* unk3D DM??? 3D 0E -> DM R0Eh ?? - 4,4 cycles */
logerror( "%04X: unk%02x\n", m_PC-1,op );
	mycycles += 4;
	break;
case 0x3E:  /* JMP RRr/@ww/ww(RRr) - 7/15/19 cycles - Flags affected: -------- */
	ARG_2;
	m_PC = s2;
	switch( r1 & 0xc0 ) {
	case 0x00:  mycycles += 7; break;
	case 0x40:  mycycles += ( ( r1 & 0x38 ) ? 19 : 15 ); break;
	default:    mycycles += 4;
	}
	break;
case 0x3F:  /* CALL RRr/@ww/ww(RRr) - 11,14/22,19/26,23 cycles - Flags affected: -------- */
	ARG_2;
	PUSH8( m_PC & 0xFF );
	PUSH8( m_PC >> 8 );
	m_PC = s2;
	switch( r1 & 0xc0 ) {
	case 0x00:  mycycles += ( ( m_SYS & 0x40 ) ? 14 : 11 ); break;
	case 0x40:  mycycles += ( ( r1 & 0x38 ) ? ( ( m_SYS & 0x40 ) ? 26 : 23 ) : ( ( m_SYS & 0x40 ) ? 22 : 19 ) );break;
	default:    mycycles += 4;
	}
	break;
case 0x40:  /* CMP Rr,Rs - 6 cycles - Flags affected: CZSV---- */
	ARG_RR;
	OP_CMP8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mycycles += 6;
	break;
case 0x41:  /* ADD Rr,Rs - 6 cycles - Flags affected: CZSV0H-- */
	ARG_RR;
	OP_ADD8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x42:  /* SUB Rr,Rs - 6 cycles - Flags affected: CZSV1H-- */
	ARG_RR;
	OP_SUB8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x43:  /* ADC Rr,Rs - 6 cycles - Flags affected: CZSV0H-- */
	ARG_RR;
	OP_ADC8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x44:  /* SBC Rr,Rs - 6 cycles - Flags affected: CZSV1H-- */
	ARG_RR;
	OP_SBC8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x45:  /* AND Rr,Rs - 6 cycles - Flags affected: -ZS0---- */
	ARG_RR;
	OP_AND8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x46:  /* OR Rr,Rs - 6 cycles - Flags affected: -ZS0---- */
	ARG_RR;
	OP_OR8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x47:  /* XOR Rr,Rs - 6 cycles - Flags affected: -ZS0---- */
	ARG_RR;
	OP_XOR8( mem_readbyte( r1 ), mem_readbyte( r2 ) );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x48:  /* MOV Rr,Rs - 6 cycles - Flags affected: -------- */
	ARG_RR;
	mem_writebyte( r1, mem_readbyte( r2 ) );
	mycycles += 6;
	break;
case 0x49:  /* CALL ad16 - 12,10 - Flags affected: -------- */
	ARG_ad16;
	PUSH8( m_PC & 0xFF );
	PUSH8( m_PC >> 8 );
	m_PC = s2;
	mycycles += ( ( m_SYS & 0x40 ) ? 12 : 10 );
	break;
case 0x4A:  /* MOVW RRr,RRs - 8 cycles - Flags affected: -------- */
	ARG_RR;
	mem_writeword( r1, mem_readword( r2 ) );
	mycycles += 8;
	break;
case 0x4B:  /* MOVW RRr,ww - 9 cycles - Flags affected: -------- */
	ARG_Sw;
	mem_writeword( r1, s2 );
		mycycles += 9;
	break;
case 0x4C:  /* MULT Rrr,Rs - 24 cycles - Flags affected: -Z-0---- */
	ARG_RR;
	res = mem_readword( r1 ) * mem_readbyte( r2 );
	mem_writeword( r1, res & 0xFFFF );
	m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V );
	m_PS1 |= ( ( res & 0xFFFF ) == 0x00 ? FLAG_Z : 0 );
	mycycles += 24;
	break;
case 0x4D:  /* MULT RRr,i - 24 cycles - Flags affected: -Z-0---- */
	ARG_iR;
	res = mem_readbyte( r1 + 1 ) * r2;
	mem_writeword( r1, res & 0xFFFF );
	m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V );
	m_PS1 |= ( ( res & 0xFFFF ) == 0x00 ? FLAG_Z : 0 );
	mycycles += 24;
	break;
case 0x4E:  /* BMOV Rr,#b,BF/BF,Rr,#b - 6 cycles - Flags affected: --------/-Z-0--B- */
	r2 = mem_readbyte( m_PC++ );
	r1 = mem_readbyte( m_PC++ );
	switch( r2 & 0xC0 ) {
	case 0x40:
		res = mem_readbyte( r1 );
		if ( m_PS1 & FLAG_B ) {
			res = res | ( 1 << ( r2 & 0x07 ) );
		} else {
			res = res & ~( 1 << ( r2 & 0x07 ) );
		}
		mem_writebyte( r1, res & 0xFF );
		break;
	case 0x00:
		m_PS1 = m_PS1 & ( FLAG_C | FLAG_S | FLAG_D | FLAG_H | FLAG_I );
		if ( mem_readbyte( r1 ) & ( 1 << ( r2 & 0x07 ) ) ) {
			m_PS1 = m_PS1 | FLAG_B;
		} else {
			m_PS1 = m_PS1 | FLAG_Z;
		}
		break;
	case 0x80:
	case 0xC0:
		break;
	}
	mycycles += 6;
	break;
case 0x4F:  /* BCMP/BAND/BOR/BXOR BF,Rr,#b - 6 cycles - Flags affected: -Z-0---- / -Z-0--B- */
	r2 = mem_readbyte( m_PC++ );
	r1 = mem_readbyte( m_PC++ );
	s1 = mem_readbyte( r1 ) & ( 1 << ( r2 & 0x07 ) );
	s2 = ( ( m_PS1 & FLAG_B ) >> 1 ) << ( r2 & 0x07 );
	switch( r2 & 0xC0 ) {
	case 0x00:
		m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V );
		if ( s1 == s2 ) {
			m_PS1 = m_PS1 | FLAG_Z;
		}
		break;
	case 0x40:
		m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V | FLAG_B );
		m_PS1 = m_PS1 | ( ( s1 & s2 ) ? FLAG_B : FLAG_Z );
		break;
	case 0x80:
		m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V | FLAG_B );
		m_PS1 = m_PS1 | ( ( s1 | s2 ) ? FLAG_B : FLAG_Z );
		break;
	case 0xC0:
		m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V | FLAG_B );
		m_PS1 = m_PS1 | ( ( s1 ^ s2 ) ? FLAG_B : FLAG_Z );
		break;
	}
	mycycles += 6;
	break;
case 0x50:  /* CMP Rr,i - 6 cycles - Flags affected: CZSV---- */
	ARG_iR;
	OP_CMP8( mem_readbyte( r1 ), r2 );
	mycycles += 6;
	break;
case 0x51:  /* ADD Rr,i - 6 cycles - Flags affected: CZSV0H-- */
	ARG_iR;
	OP_ADD8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x52:  /* SUB Rr,i - 6 cycles - Flags affected: CZSV1H-- */
	ARG_iR;
	OP_SUB8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x53:  /* ADC Rr,i - 6 cycles - Flags affected: CZSV0H-- */
	ARG_iR;
	OP_ADC8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x54:  /* SBC Rr,i - 6 cycles - Flags affected: CZSV1H-- */
	ARG_iR;
	OP_SBC8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x55:  /* AND Rr,i - 6 cycles - Flags affected: -ZS0---- */
	ARG_iR;
	OP_AND8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x56:  /* OR Rr,i - 6 cycles - Flags affected: -ZS0---- */
	ARG_iR;
	OP_OR8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x57:  /* XOR Rr,i - 6 cycles - Flags affected: -ZS0---- */
	ARG_iR;
	OP_XOR8( mem_readbyte( r1 ), r2 );
	mem_writebyte( r1, res & 0xFF );
	mycycles += 6;
	break;
case 0x58:  /* MOV Rr,i - 6 cycles - Flags affected: -------- */
	ARG_iR;
	mem_writebyte( r1, r2 );
		mycycles += 6;
	break;
case 0x59:  /* Invalid - 2? cycles - Flags affected: --------? */
	logerror( "%04X: 59h: Invalid instruction\n", m_PC-1 );
	mycycles += 2;
	break;
case 0x5A:  /* unk5A - 7,8,12,9,8 cycles */
	logerror( "%04X: unk%02x\n", m_PC-1,op );
/* NOTE: This unknown command is used in the calculator as a compare instruction
       at 0x493A and 0x4941, we set the flags on the 3rd byte, although its real
       function remains a mystery */
	ARG_iR;
	OP_CMP8( 0, r1 );
	mycycles += 7;
	break;
case 0x5B:  /* unk5B - 6,7,11,8,7 cycles */
	logerror( "%04X: unk%02x\n", m_PC-1,op );
/* NOTE: This unknown command is used in several carts, the code below allows those carts to boot */
	ARG_iR;
	r1 = r2 & 7;
	res = mem_readbyte( r1 ) + 1;
	mem_writebyte( r1, res );
	mycycles += 6;
	break;
case 0x5C:  /* DIV RRr,RRs - 47 cycles - Flags affected: -Z-V---- */
		/* lower 8 bits of RRs is used to divide */
		/* remainder in stored upper 8 bits of RRs */
logerror( "%04X: DIV RRr,Rs!\n", m_PC-1 );
	ARG_RR;
	m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V );
	s1 = mem_readbyte( r2 + 1 );
	if ( s1 ) {
		UINT16 div = mem_readword( r1 );
		res = div / s1;
		mem_writebyte( r2, div % s1 );
		mem_writeword( r1, res );
		m_PS1 = m_PS1 | ( ( res == 0 ) ? FLAG_Z : 0 );
	} else {
		m_PS1 = m_PS1 | FLAG_V;
	}
	mycycles += 47;
	break;
case 0x5D:  /* DIV RRr,i - 44 cycles - Flags affected: -Z-V---- */
logerror( "%04X: DIV RRr,i!\n", m_PC-1 );
	ARG_iR;
	m_PS1 = m_PS1 & ~ ( FLAG_Z | FLAG_V );
	if ( r2 ) {
		res = mem_readword( r1 ) / r2;
		mem_writeword( r1, res );
		m_PS1 = m_PS1 | ( ( res == 0 ) ? FLAG_Z : 0 );
	} else {
		m_PS1 = m_PS1 | FLAG_V;
	}
	mycycles += 44;
	break;
case 0x5E:  /* MOVM Rr,i,Rs - 9 cycles - Flags affected: -------- */
	ARG_RiR;
	mem_writebyte( r1, ( mem_readbyte( r1 ) & d1 ) | mem_readbyte( r2 ) );
	mycycles += 9;
	break;
case 0x5F:  /* MOVM Rr,i,j - 8 cycles - Flags affected: -------- */
	ARG_Rii;
	mem_writebyte( r1, ( mem_readbyte( r1 ) & d1 ) | r2 );
	mycycles += 8;
	break;
case 0x60:  /* CMPW RRr,RRs - 9 cycles - Flags affected: CZSV---- */
	ARG_RR;
	OP_CMP16( mem_readword( r1 ), mem_readword( r2 ) );
	mycycles += 9;
	break;
case 0x61:  /* ADDW RRr,RRs - 10 cycles - Flags affected: CZSV0H-- */
	ARG_RR;
	OP_ADD16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x62:  /* SUBW RRr,RRs - 10 cycles - Flags affected: CZSV1H-- */
	ARG_RR;
	OP_SUB16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x63:  /* ADCW RRr,RRs - 10 cycles - Flags affected: CZSV0H-- */
	ARG_RR;
	OP_ADC16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x64:  /* SBCW RRr,RRs - 10 cycles - Flags affected: CZSV1H-- */
	ARG_RR;
	OP_SBC16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x65:  /* ANDW RRr,RRs - 14 cycles - Flags affected: -Z-0---- */
	ARG_RR;
	OP_AND16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 14;
	break;
case 0x66:  /* ORW RRr,RRs - 14 cycles - Flags affected: -Z-0---- */
	ARG_RR;
	OP_OR16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 14;
	break;
case 0x67:  /* XORW RRr,RRs - 14 cycles - Flags affected: -Z-0---- */
	ARG_RR;
	OP_XOR16( mem_readword( r1 ), mem_readword( r2 ) );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 14;
	break;
case 0x68:  /* CMPW RRr,w - 9 cycles - Flags affected: CZSV---- */
	ARG_Sw;
	OP_CMP16( mem_readword( r1 ), s2 );
	mycycles += 9;
	break;
case 0x69:  /* ADDW RRr,w - 10 cycles - Flags affected: CZSV0H-- */
	ARG_Sw;
	OP_ADD16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x6A:  /* SUBW RRr,w - 10 cycles - Flags affected: CZSV1H-- */
	ARG_Sw;
	OP_SUB16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x6B:  /* ADCW RRr,w - 10 cycles - Flags affected: CZSV0H-- */
	ARG_Sw;
	OP_ADC16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x6C:  /* SBCW RRr,w - 10 cycles - Flags affected: CZSV1H-- */
	ARG_Sw;
	OP_SBC16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 10;
	break;
case 0x6D:  /* ANDW RRr,w - 13 cycles - Flags affected: -Z-0---- */
	ARG_Sw;
	OP_AND16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 13;
	break;
case 0x6E:  /* ORW RRr,w - 13 cycles - Flags affected: -Z-0---- */
	ARG_Sw;
	OP_OR16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF );
	mycycles += 13;
	break;
case 0x6F:  /* XORW RRr,w - 13 cycles - Flags affected: -Z-0---- */
	ARG_Sw;
	OP_XOR16( mem_readword( r1 ), s2 );
	mem_writeword( r1, res & 0xFFFF);
	mycycles += 13;
	break;
case 0x70:  /* DBNZ r,rel8 - 10,6 cycles - Flags affected: -------- */
case 0x71:
case 0x72:
case 0x73:
case 0x74:
case 0x75:
case 0x76:
case 0x77:
	ARG_d8;
	r1 = mem_readbyte( op & 0x07 );
	r1--;
	mem_writebyte( op & 0x07, r1 );
	if ( r1 != 0 ) {
		m_PC = s2;
		mycycles += 10;
	} else {
		mycycles += 6;
	}
	break;
case 0x78:  /* MOVW RRx,w - 6 cycles - Flags affected: -------- */
case 0x79:
case 0x7A:
case 0x7B:
case 0x7C:
case 0x7D:
case 0x7E:
case 0x7F:
	ARG_rrw;
	mem_writeword( r1, s2 );
		mycycles += 6;
	break;
case 0x80:  /* BBC R,#b,d8 - 10,6 cycles - Flags affected: -------- */
case 0x81:
case 0x82:
case 0x83:
case 0x84:
case 0x85:
case 0x86:
case 0x87:
	ARG_Rbr;
	if ( ( mem_readbyte( r1 ) & ( 1 << (op & 0x07) ) ) == 0 ) {
		m_PC = s2;
		mycycles += 10;
	} else {
		mycycles += 6;
	}
	break;
case 0x88:  /* BBS R,#b,d8 - 10,6 cycles - Flags affected: -------- */
case 0x89:
case 0x8A:
case 0x8B:
case 0x8C:
case 0x8D:
case 0x8E:
case 0x8F:
	ARG_Rbr;
	if ( ( mem_readbyte( r1 ) & ( 1 << (op & 0x07) ) ) ) {
		m_PC = s2;
		mycycles += 10;
	} else {
		mycycles += 6;
	}
	break;
case 0x90:  /* JMP cc,ad16 - 6 cycles - Flags affected: -------- */
case 0x91:
case 0x92:
case 0x93:
case 0x94:
case 0x95:
case 0x96:
case 0x97:
case 0x98:
case 0x99:
case 0x9A:
case 0x9B:
case 0x9C:
case 0x9D:
case 0x9E:
case 0x9F:
	ARG_ad16;
	CHECK_CC;
	if ( res ) {
		m_PC = s2;
	}
	mycycles += 6;
	break;
case 0xA0:  /* BCLR R,#b - 4 cycles - Flags affected: -------- */
case 0xA1:
case 0xA2:
case 0xA3:
case 0xA4:
case 0xA5:
case 0xA6:
case 0xA7:
	ARG_R;
	mem_writebyte( r1, mem_readbyte( r1 ) & ~ ( 1 << (op & 0x07) ) );
	mycycles += 4;
	break;
case 0xA8:  /* BSET R,#b - 4 cycles - Flags affected: -------- */
case 0xA9:
case 0xAA:
case 0xAB:
case 0xAC:
case 0xAD:
case 0xAE:
case 0xAF:
	ARG_R;
	mem_writebyte( r1, mem_readbyte( r1 ) | ( 1 << (op & 0x07) ) );
	mycycles += 4;
	break;
case 0xB0:  /* MOV Rx,Rr - 4 cycles - Flags affected: -------- */
case 0xB1:
case 0xB2:
case 0xB3:
case 0xB4:
case 0xB5:
case 0xB6:
case 0xB7:
	ARG_rR;
	mem_writebyte( r1, mem_readbyte( r2 ) );
	mycycles += 4;
	break;
case 0xB8:  /* MOV Rr,Rx - 4 cycles - Flags affected: -------- */
case 0xB9:
case 0xBA:
case 0xBB:
case 0xBC:
case 0xBD:
case 0xBE:
case 0xBF:
	ARG_rR;
	mem_writebyte( r2, mem_readbyte( r1 ) );
	mycycles += 4;
	break;
case 0xC0:  /* MOV Rx,i - 4 cycles - Flags affected: -------- */
case 0xC1:
case 0xC2:
case 0xC3:
case 0xC4:
case 0xC5:
case 0xC6:
case 0xC7:
	ARG_ri;
	mem_writebyte( r1, r2 );
		mycycles += 4;
	break;
case 0xC8:  /* MOV IE0/IE1/IR0/IR1/P0/P1/P2/P3,i - 4 cycles - Flags affected: -------- */
case 0xC9:
case 0xCA:
case 0xCB:
case 0xCC:
case 0xCD:
case 0xCE:
case 0xCF:
	ARG_pi;
	mem_writebyte( r1, r2 );
	mycycles += 4;
	break;
case 0xD0:  /* BR cc,rel8 - 8,4 cycles - Flags affected: -------- */
case 0xD1:
case 0xD2:
case 0xD3:
case 0xD4:
case 0xD5:
case 0xD6:
case 0xD7:
case 0xD8:
case 0xD9:
case 0xDA:
case 0xDB:
case 0xDC:
case 0xDD:
case 0xDE:
case 0xDF:
	ARG_d8;
	CHECK_CC;
	if ( res ) {
		m_PC = s2;
		mycycles += 8;
	} else {
		mycycles += 4;
	}
	break;
case 0xE0:  /* CALS - 12,9 cycles */
case 0xE1:
case 0xE2:
case 0xE3:
case 0xE4:
case 0xE5:
case 0xE6:
case 0xE7:
case 0xE8:
case 0xE9:
case 0xEA:
case 0xEB:
case 0xEC:
case 0xED:
case 0xEE:
case 0xEF:  /* CALS 1xWW - 12,9 cycles - Flags affected: -------- */
	ARG_R;
	s2 = 0x1000 + ( ( op & 0x0F ) << 8 ) + r1;
	PUSH8( m_PC & 0xFF );
	PUSH8( m_PC >> 8 );
	m_PC = s2;
	mycycles += ( ( m_SYS & 0x40 ) ? 12 : 9 );
	break;
case 0xF0:  /* STOP - 2 cycles - Flags affected: -------- */
	mycycles += 2;
	if ( m_clock_changed ) {
		/* TODO: Set system clock divider */
		/* TODO: Add a bunch of additional cycles */
		m_clock_changed = 0;
	}
	break;
case 0xF1:  /* HALT - 2 cycles - Flags affected: -------- */
	m_halted = 1;
	mycycles += 2;
	break;
case 0xF2:  /* Invalid - 2? cycles - Flags affected: --------? */
case 0xF3:
case 0xF4:
case 0xF5:
case 0xF6:
case 0xF7:
	mycycles += 2;
	break;
case 0xF8:  /* RET - 10,8 cycles - Flags affected: -------- */
	POP8( r1 );
	POP8( r2 );
	m_PC = ( r1 << 8 ) | r2;
	mycycles += ( ( m_SYS & 0x40 ) ? 10 : 8 );
	break;
case 0xF9:  /* IRET - 12,10 cycles - Flags affected: CZSVDHBI */
	POP8( m_PS1 );
	POP8( r1 );
	POP8( r2 );
	m_PC = ( r1 << 8 ) | r2;
	mycycles += ( ( m_SYS & 0x40 ) ? 12 : 10 );
	break;
case 0xFA:  /* CLRC - 2 cycles - Flags affected: C------- */
	m_PS1 = m_PS1 & ~ ( FLAG_C );
	mycycles += 2;
	break;
case 0xFB:  /* COMC - 2 cycles - Flags affected: C------- */
	m_PS1 = m_PS1 ^ FLAG_C;
	mycycles += 2;
	break;
case 0xFC:  /* SETC - 2 cycles - Flags affected: C------- */
	m_PS1 = m_PS1 | FLAG_C;
	mycycles += 2;
	break;
case 0xFD:  /* EI - 2 cycles - Flags affected: -------I */
	m_PS1 = m_PS1 | FLAG_I;
		mycycles += 2;
	break;
case 0xFE:  /* DI - 2 cycles - Flags affected: -------I */
	m_PS1 = m_PS1 & ~ ( FLAG_I );
		mycycles += 2;
	break;
case 0xFF:  /* NOP - 2 cycles - Flags affected: -------- */
	mycycles += 2;
	break;
default:
	mycycles += 2;
	break;
