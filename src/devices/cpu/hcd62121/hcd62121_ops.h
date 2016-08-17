// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#define HCD62121_MSK                                                                        \
		{                                                                                   \
			int i;                                                                          \
			UINT8 mskres = 1;                                                               \
																							\
			for ( i = 0; i < size; i++ )                                                    \
			{                                                                               \
				if ( ( m_temp1[i] & m_temp2[i] ) != m_temp2[i] )    \
					mskres = 0;                                                             \
			}                                                                               \
																							\
			if ( mskres )                                                                   \
				m_f &= ~_FLAG_Z;                                                    \
			else                                                                            \
				m_f |= _FLAG_Z;                                                     \
		}

#define HCD62121_IMSK                                                                       \
		{                                                                                   \
			int i;                                                                          \
			UINT8 mskres = 1;                                                               \
			UINT8 set_zero = 0;                                                             \
																							\
			for ( i = 0; i < size; i++ )                                                    \
			{                                                                               \
				if ( ( m_temp1[i] & ~m_temp2[i] ) != ~m_temp2[i] )  \
					mskres = 0;                                                             \
				if ( m_temp1[i] | m_temp2[i] )                              \
					set_zero = 1;                                                           \
			}                                                                               \
																							\
			if ( set_zero )                                                                 \
				m_f |= _FLAG_Z;                                                     \
			else                                                                            \
				m_f &= ~_FLAG_Z;                                                    \
																							\
			if ( mskres )                                                                   \
				m_f &= ~_FLAG_C;                                                    \
			else                                                                            \
				m_f |= _FLAG_C;                                                     \
		}


#define HCD62121_AND                                                            \
		{                                                                       \
			int i;                                                              \
			UINT8 is_zero = 1;                                                  \
																				\
			for ( i = 0; i < size; i++ )                                        \
			{                                                                   \
				m_temp1[i] = m_temp1[i] & m_temp2[i];   \
				if ( m_temp1[i] )                                       \
					is_zero = 0;                                                \
			}                                                                   \
																				\
			if ( is_zero )                                                      \
				m_f |= _FLAG_Z;                                         \
			else                                                                \
				m_f &= ~_FLAG_Z;                                        \
																				\
			if ( m_temp1[0] & 0x0f )                                    \
				m_f &= ~_FLAG_ZL;                                       \
			else                                                                \
				m_f |= _FLAG_ZL;                                        \
																				\
			if ( m_temp1[0] & 0xf0 )                                    \
				m_f &= ~_FLAG_ZH;                                       \
			else                                                                \
				m_f |= _FLAG_ZH;                                        \
		}

#define HCD62121_OR                                                             \
		{                                                                       \
			int i;                                                              \
			UINT8 is_zero = 1;                                                  \
																				\
			for ( i = 0; i < size; i++ )                                        \
			{                                                                   \
				m_temp1[i] = m_temp1[i] | m_temp2[i];   \
				if ( m_temp1[i] )                                       \
					is_zero = 0;                                                \
			}                                                                   \
																				\
			if ( is_zero )                                                      \
				m_f |= _FLAG_Z;                                         \
			else                                                                \
				m_f &= ~_FLAG_Z;                                        \
																				\
			if ( m_temp1[0] & 0x0f )                                    \
				m_f &= ~_FLAG_ZL;                                       \
			else                                                                \
				m_f |= _FLAG_ZL;                                        \
																				\
			if ( m_temp1[0] & 0xf0 )                                    \
				m_f &= ~_FLAG_ZH;                                       \
			else                                                                \
				m_f |= _FLAG_ZH;                                        \
		}

#define HCD62121_XOR                                                            \
		{                                                                       \
			int i;                                                              \
			UINT8 is_zero = 1;                                                  \
																				\
			for ( i = 0; i < size; i++ )                                        \
			{                                                                   \
				m_temp1[i] = m_temp1[i] ^ m_temp2[i];   \
				if ( m_temp1[i] )                                       \
					is_zero = 0;                                                \
			}                                                                   \
																				\
			if ( is_zero )                                                      \
				m_f |= _FLAG_Z;                                         \
			else                                                                \
				m_f &= ~_FLAG_Z;                                        \
																				\
			if ( m_temp1[0] & 0x0f )                                    \
				m_f &= ~_FLAG_ZL;                                       \
			else                                                                \
				m_f |= _FLAG_ZL;                                        \
																				\
			if ( m_temp1[0] & 0xf0 )                                    \
				m_f &= ~_FLAG_ZH;                                       \
			else                                                                \
				m_f |= _FLAG_ZH;                                        \
		}

#define HCD62121_ADD                                                                    \
		{                                                                               \
			int i;                                                                      \
			UINT8 is_zero = 1, carry = 0;                                               \
																						\
			if ( ( m_temp1[0] & 0x0f ) + ( m_temp2[0] & 0x0f ) > 15 )   \
				m_f |= _FLAG_CL;                                                \
			else                                                                        \
				m_f &= ~_FLAG_CL;                                               \
																						\
			for ( i = 0; i < size; i++ )                                                \
			{                                                                           \
				UINT16 res = m_temp1[i] + m_temp2[i] + carry;           \
																						\
				m_temp1[i] = res & 0xff;                                        \
				if ( m_temp1[i] )                                               \
					is_zero = 0;                                                        \
																						\
				carry = ( res & 0xff00 ) ? 1 : 0;                                       \
			}                                                                           \
																						\
			if ( is_zero )                                                              \
				m_f |= _FLAG_Z;                                                 \
			else                                                                        \
				m_f &= ~_FLAG_Z;                                                \
																						\
			if ( carry )                                                                \
				m_f |= _FLAG_C;                                                 \
			else                                                                        \
				m_f &= ~_FLAG_C;                                                \
																						\
			if ( m_temp1[0] & 0x0f )                                            \
				m_f &= ~_FLAG_ZL;                                               \
			else                                                                        \
				m_f |= _FLAG_ZL;                                                \
																						\
			if ( m_temp1[0] & 0xf0 )                                            \
				m_f &= ~_FLAG_ZH;                                               \
			else                                                                        \
				m_f |= _FLAG_ZH;                                                \
		}

/* BCD ADD */
#define HCD62121_ADDB                                                                   \
		{                                                                               \
			int i;                                                                      \
			UINT8 is_zero = 1, carry = 0;                                               \
																						\
			if ( ( m_temp1[0] & 0x0f ) + ( m_temp2[0] & 0x0f ) > 9 )    \
				m_f |= _FLAG_CL;                                                \
			else                                                                        \
				m_f &= ~_FLAG_CL;                                               \
																						\
			for ( i = 0; i < size; i++ )                                                \
			{                                                                           \
				UINT16 res = ( m_temp1[i] & 0x0f ) + ( m_temp2[i] & 0x0f ) + carry; \
																						\
				if ( res > 9 )                                                          \
				{                                                                       \
					res += 6;                                                           \
				}                                                                       \
				res += ( m_temp1[i] & 0xf0 ) + ( m_temp2[i] & 0xf0 );   \
				if ( res > 0x9f )                                                       \
				{                                                                       \
					res += 0x60;                                                        \
				}                                                                       \
				m_temp1[i] = res & 0xff;                                        \
				if ( m_temp1[i] )                                               \
					is_zero = 0;                                                        \
																						\
				carry = ( res & 0xff00 ) ? 1 : 0;                                       \
			}                                                                           \
																						\
			if ( is_zero )                                                              \
				m_f |= _FLAG_Z;                                                 \
			else                                                                        \
				m_f &= ~_FLAG_Z;                                                \
																						\
			if ( carry )                                                                \
				m_f |= _FLAG_C;                                                 \
			else                                                                        \
				m_f &= ~_FLAG_C;                                                \
																						\
			if ( m_temp1[0] & 0x0f )                                            \
				m_f &= ~_FLAG_ZL;                                               \
			else                                                                        \
				m_f |= _FLAG_ZL;                                                \
																						\
			if ( m_temp1[0] & 0xf0 )                                            \
				m_f &= ~_FLAG_ZH;                                               \
			else                                                                        \
				m_f |= _FLAG_ZH;                                                \
		}

#define HCD62121_SUB                                                            \
		{                                                                       \
			int i;                                                              \
			UINT8 is_zero = 1, carry = 0;                                       \
																				\
			if ( ( m_temp1[0] & 0x0f ) < ( m_temp2[0] & 0x0f ) )    \
				m_f |= _FLAG_CL;                                        \
			else                                                                \
				m_f &= ~_FLAG_CL;                                       \
																				\
			for ( i = 0; i < size; i++ )                                        \
			{                                                                   \
				UINT16 res = m_temp1[i] - m_temp2[i] - carry;   \
																				\
				m_temp1[i] = res & 0xff;                                \
				if ( m_temp1[i] )                                       \
					is_zero = 0;                                                \
																				\
				carry = ( res & 0xff00 ) ? 1 : 0;                               \
			}                                                                   \
																				\
			if ( is_zero )                                                      \
				m_f |= _FLAG_Z;                                         \
			else                                                                \
				m_f &= ~_FLAG_Z;                                        \
																				\
			if ( carry )                                                        \
				m_f |= _FLAG_C;                                         \
			else                                                                \
				m_f &= ~_FLAG_C;                                        \
																				\
			if ( m_temp1[0] & 0x0f )                                    \
				m_f &= ~_FLAG_ZL;                                       \
			else                                                                \
				m_f |= _FLAG_ZL;                                        \
																				\
			if ( m_temp1[0] & 0xf0 )                                    \
				m_f &= ~_FLAG_ZH;                                       \
			else                                                                \
				m_f |= _FLAG_ZH;                                        \
		}

#define HCD62121_PUSHW(source)                                                                      \
		{                                                                                           \
			UINT16 address = source;                                                                \
			m_program->write_byte( ( m_sseg << 16 ) | m_sp, ( address ) & 0xff ); \
			m_sp--;                                                                         \
			m_program->write_byte( ( m_sseg << 16 ) | m_sp, ( address ) >> 8 );   \
			m_sp--;                                                                         \
		}

#define HCD62121_POPW(dest)                                                                         \
		{                                                                                           \
			UINT16 res;                                                                             \
			m_sp++;                                                                         \
			res = m_program->read_byte( ( m_sseg << 16 ) | m_sp ) << 8;           \
			m_sp++;                                                                         \
			res |= m_program->read_byte( ( m_sseg << 16 ) | m_sp );               \
			dest = res;                                                                             \
		}

case 0x04:      /* mskb r1,r2 */
case 0x05:      /* mskw r1,r2 */
case 0x06:      /* mskq r1,r2 */
case 0x07:      /* mskt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_MSK;
	}
	break;

case 0x08:      /* shb r1,4 */
case 0x09:      /* shw r1,4 */
case 0x0A:      /* shq r1,4 */
case 0x0B:      /* sht r1,4 */
	/* Shift is a nibble shift! */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 d1 = 0, d2 = 0;

		read_reg( size, reg1 );

		for ( i = 0; i < size; i++ )
		{
			if ( reg1 & 0x80 )
			{
				d1 = ( m_temp1[i] & 0x0f ) << 4;
				m_temp1[i] = ( m_temp1[i] >> 4 ) | d2;
			}
			else
			{
				d1 = ( m_temp1[i] & 0xf0 ) >> 4;
				m_temp1[i] = ( m_temp1[i] << 4 ) | d2;
			}
			d2 = d1;
		}

		write_reg( size, reg1 );
	}
	break;

case 0x0C:      /* testb r1,r2 */
case 0x0D:      /* testw r1,r2 */
case 0x0E:      /* testq r1,r2 */
case 0x0F:      /* testt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_AND;
	}
	break;

case 0x10:      /* xorb r1,r2 */
case 0x11:      /* xorw r1,r2 */
case 0x12:      /* xorq r1,r2 */
case 0x13:      /* xort r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_XOR;

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x14:      /* cmpb r1,r2 */
case 0x15:      /* cmpw r1,r2 */
case 0x16:      /* cmpq r1,r2 */
case 0x17:      /* cmpt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_SUB;
	}
	break;

case 0x18:      /* movb r1,r2 */
case 0x19:      /* movw r1,r2 */
case 0x1A:      /* movq r1,r2 */
case 0x1B:      /* movt r1,r2 */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		for ( i = 0; i < size; i++ )
			m_temp1[i] = m_temp2[i];

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x1C:      /* imskb r1,r2 */
case 0x1D:      /* imskw r1,r2 */
case 0x1E:      /* imskq r1,r2 */
case 0x1F:      /* imskt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_IMSK;
	}
	break;

case 0x20:      /* shrb r1 */
case 0x21:      /* shrw r1 */
case 0x22:      /* shrq r1 */
case 0x23:      /* shrt r1 */
	/* Shift is a single shift! */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 d1 = 0, d2 = 0;

		read_reg( size, reg1 );

		for ( i = 0; i < size; i++ )
		{
			d1 = ( m_temp1[i] & 0x01 ) << 7;
			m_temp1[i] = ( m_temp1[i] >> 1 ) | d2;
			d2 = d1;
		}

		write_reg( size, reg1 );
	}
	break;

case 0x24:      /* orb r1,r2 */
case 0x25:      /* orw r1,r2 */
case 0x26:      /* orq r1,r2 */
case 0x27:      /* ort r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_OR;

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x28:      /* shlb r1 */
case 0x29:      /* shlw r1 */
case 0x2A:      /* shlq r1 */
case 0x2B:      /* shlt r1 */
	/* Shift is a single shift! */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 d1 = 0, d2 = 0;

		read_reg( size, reg1 );

		for ( i = 0; i < size; i++ )
		{
			d1 = ( m_temp1[i] & 0x80 ) >> 7;
			m_temp1[i] = ( m_temp1[i] << 1 ) | d2;
			d2 = d1;
		}

		write_reg( size, reg1 );
	}
	break;

case 0x2C:      /* andb r1,r2 */
case 0x2D:      /* andw r1,r2 */
case 0x2E:      /* andq r1,r2 */
case 0x2F:      /* andt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, true );

		HCD62121_AND;

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x34:      /* subb r1,r2 */
case 0x35:      /* subw r1,r2 */
case 0x36:      /* subq r1,r2 */
case 0x37:      /* subt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_SUB;

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x38:      /* adbb r1,r2 */
case 0x39:      /* adbw r1,r2 */
case 0x3A:      /* adbq r1,r2 */
case 0x3B:      /* adbt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_ADDB;

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x3C:      /* addb r1,r2 */
case 0x3D:      /* addw r1,r2 */
case 0x3E:      /* addq r1,r2 */
case 0x3F:      /* addt r1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_regreg( size, reg1, reg2, false );

		HCD62121_ADD;

		write_regreg( size, reg1, reg2 );
	}
	break;

case 0x4C:      /* testb ir1,r2 */
case 0x4D:      /* testw ir1,r2 */
case 0x4E:      /* testq ir1,r2 */
case 0x4F:      /* testt ir1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		HCD62121_AND;
	}
	break;

case 0x54:      /* cmpb ir1,r2 */
case 0x55:      /* cmpw ir1,r2 */
case 0x56:      /* cmpq ir1,r2 */
case 0x57:      /* cmpt ir1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		HCD62121_SUB;
	}
	break;

case 0x58:      /* movb ir1,r2 */
case 0x59:      /* movw ir1,r2 */
case 0x5A:      /* movq ir1,r2 */
case 0x5B:      /* movt ir1,r2 */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		for ( i = 0; i < size; i++ )
			m_temp1[i] = m_temp2[i];

		write_iregreg( size, reg1, reg2 );
	}
	break;

case 0x64:      /* orb ir1,r2 */
case 0x65:      /* orb ir1,r2 */
case 0x66:      /* orb ir1,r2 */
case 0x67:      /* orb ir1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		HCD62121_OR;

		write_iregreg( size, reg1, reg2 );
	}
	break;

case 0x6C:      /* andb ir1,r2 */
case 0x6D:      /* andw ir1,r2 */
case 0x6E:      /* andq ir1,r2 */
case 0x6F:      /* andt ir1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		HCD62121_AND;

		write_iregreg( size, reg1, reg2 );
	}
	break;

case 0x7C:      /* addb ir1,r2 */
case 0x7D:      /* addw ir1,r2 */
case 0x7E:      /* addq ir1,r2 */
case 0x7F:      /* addt ir1,r2 */
	{
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		HCD62121_ADD;

		write_iregreg( size, reg1, reg2 );
	}
	break;

case 0x88:      /* jump _a16 */
	m_ip = ( read_op() << 8 ) | read_op();
	break;

case 0x89:      /* jumpf cs:a16 */
	{
		UINT8 cs = read_op();
		UINT8 a1 = read_op();
		UINT8 a2 = read_op();

		m_cseg = cs;
		m_ip = ( a1 << 8 ) | a2;
	}
	break;

case 0x8A:      /* call a16 */
	{
		UINT8 a1 = read_op();
		UINT8 a2 = read_op();

		HCD62121_PUSHW( m_ip );

		m_ip = ( a1 << 8 ) | a2;
	}
	break;

case 0x8C:      /* unk_8C */
case 0x8D:      /* unk_8D */
case 0x8E:      /* unk_8E */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	break;

case 0x90:      /* retzh */
case 0x91:      /* retzl */
case 0x92:      /* retc */
case 0x93:      /* retz */
case 0x94:      /* retzc */
case 0x95:      /* retcl */
case 0x96:      /* retnc */
case 0x97:      /* retnz */
	if ( check_cond( op ) )
		HCD62121_POPW( m_ip );
	break;

case 0x98:      /* jump (r1) */
	{
		UINT8 reg1 = read_op();
		UINT16 ad = m_reg[ ( reg1 | 0x40 ) & 0x7f ] << 8;

		if ( reg1 & 0x40 )
			ad |= m_reg[ ( ( reg1 - 1 ) | 0x40 ) & 0x7f ];
		else
			ad |= m_reg[ ( ( reg1 + 1 ) | 0x40 ) & 0x7f ];

		m_ip = ad;
	}
	break;

case 0x9F:      /* ret */
	HCD62121_POPW( m_ip );
	break;

case 0xA0:      /* jmpzh a16 */
case 0xA1:      /* jmpzl a16 */
case 0xA2:      /* jmpc a16 */
case 0xA3:      /* jmpz a16 */
case 0xA4:      /* jmpzc a16 */
case 0xA5:      /* jmpcl a16 */
case 0xA6:      /* jmpnc a16 */
case 0xA7:      /* jmpnz a16 */
	{
		UINT8 a1 = read_op();
		UINT8 a2 = read_op();

		if ( check_cond( op ) )
			m_ip = ( a1 << 8 ) | a2;
	}
	break;

case 0xA8:      /* callzh a16 */
case 0xA9:      /* callzl a16 */
case 0xAA:      /* callc a16 */
case 0xAB:      /* callz a16 */
case 0xAC:      /* callzc a16 */
case 0xAD:      /* callcl a16 */
case 0xAE:      /* callnc a16 */
case 0xAF:      /* callnz a16 */
	{
		UINT8 a1 = read_op();
		UINT8 a2 = read_op();

		if ( check_cond( op ) )
		{
			HCD62121_PUSHW( m_ip );

			m_ip = ( a1 << 8 ) | a2;
		}
	}
	break;

case 0xB1:      /* unk_B1 reg/i8 */
case 0xB3:      /* unk_B3 reg/i8 */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	read_op();
	break;

case 0xB4:      /* out koh,reg */
	m_io->write_byte( HCD62121_KOH, m_reg[ read_op() & 0x7f ] );
	break;

case 0xB5:      /* out koh,i8 */
	m_io->write_byte( HCD62121_KOH, read_op() );
	break;

case 0xB6:      /* out kol,reg */
	m_io->write_byte( HCD62121_KOL, m_reg[ read_op() & 0x7f ] );
	break;

case 0xB7:      /* out kol,i8 */
	m_io->write_byte( HCD62121_KOL, read_op() );
	break;

case 0xB9:      /* unk_B9 reg/i8 */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	read_op();
	break;

case 0xBB:      /* jmpcl? a16 */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	{
		UINT8 a1 = read_op();
		UINT8 a2 = read_op();

		if ( m_f & _FLAG_CL )
			m_ip = ( a1 << 8 ) | a2;
	}
	break;

case 0xBF:      /* jmpncl? a16 */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	{
		UINT8 a1 = read_op();
		UINT8 a2 = read_op();

		if ( ! ( m_f & _FLAG_CL ) )
			m_ip = ( a1 << 8 ) | a2;
	}
	break;

case 0xC0:      /* movb reg,i8 */
case 0xC1:      /* movw reg,i16 */
case 0xC2:      /* movw reg,i64 */
case 0xC3:      /* movw reg,i80 */
	{
		int i;
		int size = datasize( op );
		UINT8 reg = read_op();

		for( i = 0; i < size; i++ )
		{
			m_reg[(reg + i) & 0x7f] = read_op();
		}
	}
	break;

case 0xC4:      /* movb (lar),r1 / r1,(lar) */
case 0xC5:      /* movw (lar),r1 / r1,(lar) */
case 0xC6:      /* movq (lar),r1 / r1,(lar) */
case 0xC7:      /* movt (lar),r1 / r1,(lar) */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();
		int pre_inc = 0;
		int post_inc = 1;

		switch( reg1 & 0x60 )
		{
		case 0x00:
			pre_inc = 0;
			post_inc = 1;
			break;
		case 0x20:
			pre_inc = 1;
			post_inc = 0;
			break;
		case 0x40:
			pre_inc = 0;
			post_inc = -1;
			break;
		case 0x60:
			pre_inc = -1;
			post_inc = 0;
			break;
		}

		if ( ( reg1 & 0x80 ) || ( reg2 & 0x80 ) )
		{
			/* (lar) <- r1 */
			for ( i = 0; i < size; i++ )
			{
				m_lar += pre_inc;
				m_program->write_byte( ( m_dseg << 16 ) | m_lar, m_reg[ ( reg2 + i ) & 0x7f ] );
				m_lar += post_inc;
			}
		}
		else
		{
			/* r1 <- (lar) */
			for ( i = 0; i < size; i++ )
			{
				m_lar += pre_inc;
				m_reg[ ( reg2 + i ) & 0x7f ] = m_program->read_byte( ( m_dseg << 16 ) | m_lar );
				m_lar += post_inc;
			}
		}
	}
	break;

case 0xCC:      /* swapb ir1,r2 */
case 0xCD:      /* swapw ir1,r2 */
case 0xCE:      /* swapq ir1,r2 */
case 0xCF:      /* swapt ir1,r2? */
	{
		int i;
		int size = datasize( op );
		UINT8 reg1 = read_op();
		UINT8 reg2 = read_op();

		read_iregreg( size, reg1, reg2 );

		for ( i = 0; i < size; i++ )
		{
			UINT8 d = m_temp1[i];
			m_temp1[i] = m_temp2[i];
			m_temp2[i] = d;
		}

		write_iregreg( size, reg1, reg2 );
		write_iregreg2( size, reg1, reg2 );
	}
	break;

case 0xD0:      /* movb cs,reg */
	m_cseg = m_reg[ read_op() & 0x7f ];
	break;

case 0xD1:      /* movb cs,i8 */
	m_cseg = read_op();
	break;

case 0xD2:      /* movb dsize,reg */
	m_dsize = m_reg[ read_op() & 0x7f ];
	break;

case 0xD3:      /* movb dsize,i8 */
	m_dsize = read_op();
	break;

case 0xD4:      /* movb ss,reg */
	m_sseg = m_reg[ read_op() & 0x7f ];
	break;

case 0xD5:      /* movb ss,i8 */
	m_sseg = read_op();
	break;

case 0xD6:      /* movw sp,reg */
	{
		UINT8 reg1 = read_op();

		m_sp = m_reg[ reg1 & 0x7f ] | ( m_reg[ ( reg1 + 1 ) & 0x7f ] << 8 );
	}
	break;

case 0xD7:      /* movw sp,i16 */
	m_sp = read_op() << 8;
	m_sp |= read_op();
	break;

case 0xD8:      /* movb f,reg */
	m_f = m_reg[ read_op() & 0x7f ];
	break;

case 0xD9:      /* movb f,i8 */
	m_f = read_op();
	break;

case 0xDC:      /* movb ds,reg */
	m_dseg = m_reg[ read_op() & 0x7f ];
	break;

case 0xDD:      /* movb ds,i8 */
	m_dseg = read_op();
	break;

case 0xDE:      /* movw lar,reg */
	{
		UINT8 reg1 = read_op();

		m_lar = m_reg[ reg1 & 0x7f ] | ( m_reg[ ( reg1 + 1 ) & 0x7f ] << 8 );
	}
	break;

case 0xE0:      /* in0 reg */
	{
		UINT8 reg1 = read_op();

		m_reg[ reg1 & 0x7f ] = m_io->read_byte( HCD62121_IN0 );
	}
	break;

case 0xE1:      /* unk_E1 reg/i8 (in?) */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	read_op();
	break;

case 0xE2:      /* in kb, reg */
	m_reg[ read_op() & 0x7f ] = m_io->read_byte( HCD62121_KI );
	break;

case 0xE3:      /* unk_e3 reg/i8 (in?) */
case 0xE4:      /* unk_e4 reg/i8 (in?) */
case 0xE5:      /* unk_e5 reg/i8 (in?) */
case 0xE6:      /* unk_e6 reg/i8 (in?) */
case 0xE7:      /* unk_e7 reg/i8 (in?) */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	read_op();
	break;

case 0xE8:      /* movw r1,lar */
	{
		UINT8 reg1 = read_op();

		m_reg[ reg1 & 0x7f ] = m_lar & 0xff;
		m_reg[ ( reg1 + 1 ) & 0x7f ] = m_lar >> 8;
	}
	break;

case 0xEB:      /* movw reg,ss */
	{
		UINT8 reg1 = read_op();

		m_reg[ reg1 & 0x7f ] = m_sp & 0xff;
		m_reg[ ( reg1 + 1 ) & 0x7f ] = m_sp >> 8;
	}
	break;

case 0xEF:      /* movb reg,ss */
	m_reg[ read_op() & 0x7f ] = m_sseg;
	break;

case 0xF0:      /* unk_F0 reg/i8 (out?) */
case 0xF1:      /* unk_F1 reg/i8 (out?) */
case 0xF2:      /* unk_F2 reg/i8 (out?) */
case 0xF3:      /* unk_F3 reg/i8 (out?) */
case 0xF4:      /* unk_F4 reg/i8 (out?) */
case 0xF5:      /* unk_F5 reg/i8 (out?) */
case 0xF6:      /* unk_F6 reg/i8 (out?) */
case 0xF7:      /* unk_F7 reg/i8 (out?) */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	read_op();
	break;

case 0xFC:      /* unk_FC */
case 0xFD:      /* unk_FD */
case 0xFE:      /* unk_FE */
	logerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
	break;

case 0xFF:      /* nop */
	break;

default:
	/*logerror*/fatalerror( "%02x:%04x: unimplemented instruction %02x encountered\n", m_cseg, m_ip-1, op );
