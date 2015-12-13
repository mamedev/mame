// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#define INC_8BIT(x) \
{ \
	UINT8 r,f; \
	x++; \
	r=(x);  \
	f=(UINT8)(m_F&FLAG_C); \
	if( r==0 )       f|=FLAG_Z; \
	if( (r&0xF)==0 ) f|=FLAG_H; \
	m_F=f; \
}

#define DEC_8BIT(x) \
{ \
	UINT8 r,f; \
	x--; \
	r=(x);  \
	f=(UINT8)((m_F&FLAG_C)|FLAG_N); \
	if( r==0 )       f|=FLAG_Z; \
	if( (r&0xF)==0xF ) f|=FLAG_H; \
	m_F=f; \
}

#define INC_16BIT(x,y) \
{ \
	UINT16 r = x << 8 | y; \
	if (++y == 0) x++; \
	m_incdec16_func(r << 16 | x << 8 | y); \
}

#define DEC_16BIT(x,y) \
{ \
	UINT16 r = x << 8 | y; \
	if (--y == 0xff) x--; \
	m_incdec16_func(r << 16 | x << 8 | y); \
}

#define ADD_HL_RR(x) \
{ \
	UINT32 r1,r2; \
	UINT8 f; \
	r1=((m_H<<8)|m_L)+(x); \
	r2=(((m_H<<8)|m_L)&0xFFF)+((x)&0xFFF); \
	f=(UINT8)(m_F&FLAG_Z); \
	if( r1>0xFFFF ) f|=FLAG_C; \
	if( r2>0x0FFF ) f|=FLAG_H; \
	m_L = r1; \
	m_H = r1 >> 8; \
	m_F=f; \
}

#define ADD_A_X(x) \
{ \
	UINT16 r1,r2; \
	UINT8 f; \
	r1=(UINT16)((m_A&0xF)+((x)&0xF)); \
	r2=(UINT16)(m_A+(x)); \
	m_A=(UINT8)r2; \
	if( ((UINT8)r2)==0 ) f=FLAG_Z; \
	else f=0; \
	if( r2>0xFF ) f|=FLAG_C; \
	if( r1>0xF )  f|=FLAG_H; \
	m_F=f; \
}

#define SUB_A_X(x) \
{ \
	UINT16 r1,r2; \
	UINT8 f; \
	r1=(UINT16)((m_A&0xF)-((x)&0xF)); \
	r2=(UINT16)(m_A-(x)); \
	m_A=(UINT8)r2; \
	if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
	else f=FLAG_N; \
	if( r2>0xFF ) f|=FLAG_C; \
	if( r1>0xF )  f|=FLAG_H; \
	m_F=f; \
}

#define CP_A_X(x) \
{ \
	UINT16 r1,r2; \
	UINT8 f; \
	r1=(UINT16)((m_A&0xF)-((x)&0xF)); \
	r2=(UINT16)(m_A-(x)); \
	if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
	else f=FLAG_N; \
	if( r2>0xFF ) f|=FLAG_C; \
	if( r1>0xF )  f|=FLAG_H; \
	m_F=f; \
}

#define SBC_A_X(x) \
{ \
	UINT16 r1,r2; \
	UINT8 f; \
	r1=(UINT16)((m_A&0xF)-((x)&0xF)-((m_F&FLAG_C)?1:0)); \
	r2=(UINT16)(m_A-(x)-((m_F&FLAG_C)?1:0)); \
	m_A=(UINT8)r2; \
	if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
	else f=FLAG_N; \
	if( r2>0xFF ) f|=FLAG_C; \
	if( r1>0xF )  f|=FLAG_H; \
	m_F=f; \
}

#define ADC_A_X(x) \
{ \
	UINT16 r1,r2;  \
	UINT8 f; \
	r1=(UINT16)((m_A&0xF)+((x)&0xF)+((m_F&FLAG_C)?1:0));  \
	r2=(UINT16)(m_A+(x)+((m_F&FLAG_C)?1:0)); \
	if( (m_A=(UINT8)r2)==0 ) f=FLAG_Z; \
	else f=0; \
	if( r2>0xFF )   f|=FLAG_C; \
	if( r1>0xF )    f|=FLAG_H; \
	m_F=f; \
}

#define AND_A_X(x) \
	if( (m_A&=(x))==0 ) \
	m_F=FLAG_H|FLAG_Z; \
	else \
	m_F=FLAG_H;

#define XOR_A_X(x) \
	if( (m_A^=(x))==0 ) \
	m_F=FLAG_Z; \
	else \
	m_F=0;

#define OR_A_X(x) \
	if( (m_A|=(x))==0 ) \
	m_F=FLAG_Z; \
	else \
	m_F=0;

#define POP(x,y) \
	y = mem_read_byte( m_SP++ ); \
	x = mem_read_byte( m_SP++ );

#define PUSH(x,y) \
	m_SP--; \
	mem_write_byte( m_SP, x ); \
	m_SP--; \
	mem_write_byte( m_SP, y );

/**********************************************************/

case 0x00: /*      NOP */
	break;

case 0x01: /*      LD BC,n16 */
	m_C = mem_read_byte( m_PC++ );
	m_B = mem_read_byte( m_PC++ );
	break;

case 0x02: /*      LD (BC),A */
	mem_write_byte( ( m_B << 8 ) | m_C, m_A );
	break;

case 0x03: /*      INC BC */
	INC_16BIT(m_B, m_C);
	cycles_passed( 4 );
	break;

case 0x04: /*      INC B */
	INC_8BIT (m_B)
	break;

case 0x05: /*      DEC B */
	DEC_8BIT (m_B)
	break;

case 0x06: /*      LD B,n8 */
	m_B = mem_read_byte( m_PC++ );
	break;

case 0x07: /*      RLCA */

	m_A = (UINT8) ((m_A << 1) | (m_A >> 7));
	if (m_A & 1)
	{
	m_F = FLAG_C;
	}
	else
	{
	m_F = 0;
	}
	break;

case 0x08: /*      LD (n16),SP */
	mem_write_word (mem_read_word (m_PC), m_SP);
	m_PC += 2;
	break;

case 0x09: /*      ADD HL,BC */
	ADD_HL_RR ((m_B<<8)|m_C)
	cycles_passed( 4 );
	break;
case 0x0A: /*      LD A,(BC) */

	m_A = mem_read_byte ( (m_B<<8)|m_C );
	break;
case 0x0B: /*      DEC BC */
	DEC_16BIT(m_B, m_C);
	cycles_passed( 4 );
	break;
case 0x0C: /*      INC C */

	INC_8BIT (m_C)
	break;
case 0x0D: /*      DEC C */

	DEC_8BIT (m_C)
	break;
case 0x0E: /*      LD C,n8 */

	m_C = mem_read_byte ( m_PC++ );
	break;
case 0x0F: /*      RRCA */

	m_A = (UINT8) ((m_A >> 1) | (m_A << 7));
	m_F = 0;
	if (m_A & 0x80)
	{
	m_F |= FLAG_C;
	}
	break;
case 0x10: /*      STOP */
	if ( m_gb_speed_change_pending ) {
	m_gb_speed = ( m_gb_speed == 1 ) ? 2 : 1;
	}
	m_gb_speed_change_pending = 0;
	break;
case 0x11: /*      LD DE,n16 */
	m_E = mem_read_byte( m_PC++ );
	m_D = mem_read_byte( m_PC++ );
	break;
case 0x12: /*      LD (DE),A */
	mem_write_byte( ( m_D << 8 ) | m_E, m_A );
	break;
case 0x13: /*      INC DE */
	INC_16BIT(m_D, m_E);
	cycles_passed( 4 );
	break;
case 0x14: /*      INC D */

	INC_8BIT (m_D)
	break;
case 0x15: /*      DEC D */

	DEC_8BIT (m_D)
	break;
case 0x16: /*      LD D,n8 */

	m_D = mem_read_byte ( m_PC++ );
	break;
case 0x17: /*      RLA */

	x = (m_A & 0x80) ? FLAG_C : 0;

	m_A = (UINT8) ((m_A << 1) | ((m_F & FLAG_C) ? 1 : 0));
	m_F = x;
	break;
case 0x18: /*      JR      n8 */
	{
	INT8 offset;

	offset = mem_read_byte( m_PC++ );
	m_PC += offset;
	cycles_passed( 4 );
	}
	break;
case 0x19: /*      ADD HL,DE */
	ADD_HL_RR (( m_D << 8 ) | m_E)
	cycles_passed( 4 );
	break;
case 0x1A: /*      LD A,(DE) */

	m_A = mem_read_byte( ( m_D << 8 ) | m_E );
	break;
case 0x1B: /*      DEC DE */
	DEC_16BIT(m_D, m_E);
	cycles_passed( 4 );
	break;
case 0x1C: /*      INC E */

	INC_8BIT (m_E)
	break;
case 0x1D: /*      DEC E */

	DEC_8BIT (m_E)
	break;
case 0x1E: /*      LD E,n8 */

	m_E = mem_read_byte( m_PC++ );
	break;
case 0x1F: /*      RRA */

	x = (m_A & 1) ? FLAG_C : 0;

	m_A = (UINT8) ((m_A >> 1) | ((m_F & FLAG_C) ? 0x80 : 0));
	m_F = x;
	break;
case 0x20: /*      JR NZ,n8 */
	{
		INT8 offset = mem_read_byte( m_PC++ );
		if (! (m_F & FLAG_Z) )
		{
			m_PC += offset;
			cycles_passed( 4 );
		}
	}
	break;
case 0x21: /*      LD HL,n16 */
	m_L = mem_read_byte( m_PC++ );
	m_H = mem_read_byte( m_PC++ );
	break;
case 0x22: /*      LD (HL+),A */
	mem_write_byte( (m_H << 8 ) | m_L, m_A );
	INC_16BIT(m_H, m_L);
	break;
case 0x23: /*      INC HL */
	INC_16BIT(m_H, m_L);
	cycles_passed( 4 );
	break;
case 0x24: /*      INC H */

	INC_8BIT (m_H);
	break;
case 0x25: /*      DEC H */

	DEC_8BIT (m_H);
	break;
case 0x26: /*      LD H,n8 */

	m_H = mem_read_byte( m_PC++ );
	break;
case 0x27: /*      DAA */
	{
		int tmp = m_A;

		if ( ! ( m_F & FLAG_N ) ) {
			if ( ( m_F & FLAG_H ) || ( tmp & 0x0F ) > 9 )
				tmp += 6;
			if ( ( m_F & FLAG_C ) || tmp > 0x9F )
				tmp += 0x60;
		} else {
			if ( m_F & FLAG_H ) {
				tmp -= 6;
				if ( ! ( m_F & FLAG_C ) )
					tmp &= 0xFF;
			}
			if ( m_F & FLAG_C )
					tmp -= 0x60;
		}
		m_F &= ~ ( FLAG_H | FLAG_Z );
		if ( tmp & 0x100 )
			m_F |= FLAG_C;
		m_A = tmp & 0xFF;
		if ( ! m_A )
			m_F |= FLAG_Z;
	}
	break;
case 0x28: /*      JR Z,n8 */
	{
		INT8 offset = mem_read_byte( m_PC++ );

		if (m_F & FLAG_Z)
		{
			m_PC += offset;
			cycles_passed( 4 );
		}
	}
	break;
case 0x29: /*      ADD HL,HL */
	ADD_HL_RR ((m_H << 8 ) | m_L)
	cycles_passed( 4 );
	break;
case 0x2A: /*      LD A,(HL+) */
	m_A = mem_read_byte( ( m_H << 8 ) | m_L );
	INC_16BIT(m_H, m_L);
	break;
case 0x2B: /*      DEC HL */
	DEC_16BIT(m_H, m_L);
	cycles_passed( 4 );
	break;
case 0x2C: /*      INC L */

	INC_8BIT (m_L);
	break;
case 0x2D: /*      DEC L */

	DEC_8BIT (m_L);
	break;
case 0x2E: /*      LD L,n8 */

	m_L = mem_read_byte( m_PC++ );
	break;
case 0x2F: /*      CPL */

	m_A = ~m_A;
	m_F |= FLAG_N | FLAG_H;
	break;
case 0x30: /*      JR NC,n8 */
	{
		INT8 offset = mem_read_byte( m_PC++ );

		if ( ! (m_F & FLAG_C) )
		{
			m_PC += offset;
			cycles_passed( 4 );
		}
	}
	break;
case 0x31: /*      LD SP,n16 */

	m_SP = mem_read_word( m_PC );
	m_PC += 2;
	break;
case 0x32: /*      LD (HL-),A */
	mem_write_byte( ( m_H << 8 ) | m_L, m_A );
	DEC_16BIT(m_H, m_L);
	break;
case 0x33: /*      INC SP */
	m_SP += 1;
	cycles_passed( 4 );
	break;
case 0x34: /*      INC (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;
		UINT8 r, f;

		f = (UINT8) (m_F & FLAG_C);
		r = mem_read_byte( addr );
		r += 1;
		mem_write_byte( addr, r );

		if (r == 0)
			f |= FLAG_Z;

		if ((r & 0xF) == 0)
			f |= FLAG_H;

		m_F = f;
	}
	break;
case 0x35: /*      DEC (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;
		UINT8 r, f;

		f = (UINT8) ((m_F & FLAG_C) | FLAG_N);
		r = mem_read_byte( addr );
		r -= 1;
		mem_write_byte( addr, r );

		if (r == 0)
			f |= FLAG_Z;

		if ((r & 0xF) == 0xF)
			f |= FLAG_H;

		m_F = f;
	}
	break;
case 0x36: /*      LD (HL),n8 */
	{
		UINT8 v = mem_read_byte( m_PC++ );
		mem_write_byte( ( m_H << 8 ) | m_L, v );
	}
	break;
case 0x37: /*      SCF */

	m_F = (UINT8) ((m_F & FLAG_Z) | FLAG_C);
	break;
case 0x38: /*      JR C,n8 */
	{
		INT8 offset = mem_read_byte( m_PC++ );

		if (m_F & FLAG_C)
		{
			m_PC += offset;
			cycles_passed( 4 );
		}
	}
	break;
case 0x39: /*      ADD HL,SP */
	ADD_HL_RR (m_SP)
	cycles_passed( 4 );
	break;
case 0x3A: /*      LD A,(HL-) */
	m_A = mem_read_byte( ( m_H << 8 ) | m_L );
	DEC_16BIT(m_H, m_L);
	break;
case 0x3B: /*      DEC SP */
	m_SP -= 1;
	cycles_passed( 4 );
	break;
case 0x3C: /*      INC     A */

	INC_8BIT (m_A);
	break;
case 0x3D: /*      DEC     A */

	DEC_8BIT (m_A);
	break;
case 0x3E: /*      LD A,n8 */

	m_A = mem_read_byte( m_PC++ );
	break;
case 0x3F: /*      CCF */

	m_F = (UINT8) ((m_F & FLAG_Z) | ((m_F & FLAG_C) ? 0 : FLAG_C));
	break;
case 0x40: /*      LD B,B */
	break;
case 0x41: /*      LD B,C */

	m_B = m_C;
	break;
case 0x42: /*      LD B,D */

	m_B = m_D;
	break;
case 0x43: /*      LD B,E */

	m_B = m_E;
	break;
case 0x44: /*      LD B,H */

	m_B = m_H;
	break;
case 0x45: /*      LD B,L */

	m_B = m_L;
	break;
case 0x46: /*      LD B,(HL) */
	m_B = mem_read_byte( ( m_H << 8 ) | m_L) ;
	break;

case 0x47: /*      LD B,A */

	m_B = m_A;
	break;
case 0x48: /*      LD C,B */

	m_C = m_B;
	break;
case 0x49: /*      LD C,C */
	break;
case 0x4A: /*      LD C,D */

	m_C = m_D;
	break;
case 0x4B: /*      LD C,E */

	m_C = m_E;
	break;
case 0x4C: /*      LD C,H */

	m_C = m_H;
	break;
case 0x4D: /*      LD C,L */

	m_C = m_L;
	break;

case 0x4E: /*      LD C,(HL) */
	m_C = mem_read_byte( ( m_H << 8 ) | m_L );
	break;

case 0x4F: /*      LD C,A */

	m_C = m_A;
	break;
case 0x50: /*      LD D,B */

	m_D = m_B;
	break;
case 0x51: /*      LD D,C */

	m_D = m_C;
	break;
case 0x52: /*      LD D,D */
	break;
case 0x53: /*      LD D,E */

	m_D = m_E;
	break;
case 0x54: /*      LD D,H */

	m_D = m_H;
	break;
case 0x55: /*      LD D,L */

	m_D = m_L;
	break;
case 0x56: /*      LD D,(HL) */

	m_D = mem_read_byte( ( m_H << 8 ) | m_L );
	break;
case 0x57: /*      LD D,A */

	m_D = m_A;
	break;
case 0x58: /*      LD E,B */

	m_E = m_B;
	break;
case 0x59: /*      LD E,C */

	m_E = m_C;
	break;
case 0x5A: /*      LD E,D */

	m_E = m_D;
	break;
case 0x5B: /*      LD E,E */
	break;
case 0x5C: /*      LD E,H */

	m_E = m_H;
	break;
case 0x5D: /*      LD E,L */

	m_E = m_L;
	break;
case 0x5E: /*      LD E,(HL) */

	m_E = mem_read_byte( ( m_H << 8 ) | m_L );
	break;
case 0x5F: /*      LD E,A */

	m_E = m_A;
	break;
case 0x60: /*      LD H,B */

	m_H = m_B;
	break;
case 0x61: /*      LD H,C */

	m_H = m_C;
	break;
case 0x62: /*      LD H,D */

	m_H = m_D;
	break;
case 0x63: /*      LD H,E */

	m_H = m_E;
	break;
case 0x64: /*      LD H,H */
	break;
case 0x65: /*      LD H,L */

	m_H = m_L;
	break;
case 0x66: /*      LD H,(HL) */

	m_H = mem_read_byte( ( m_H << 8 ) | m_L );
	break;
case 0x67: /*      LD H,A */

	m_H = m_A;
	break;
case 0x68: /*      LD L,B */

	m_L = m_B;
	break;
case 0x69: /*      LD L,C */

	m_L = m_C;
	break;
case 0x6A: /*      LD L,D */
	m_L = m_D;
	break;
case 0x6B: /*      LD L,E */

	m_L = m_E;
	break;
case 0x6C: /*      LD L,H */

	m_L = m_H;
	break;
case 0x6D: /*      LD L,L */
	break;
case 0x6E: /*      LD L,(HL) */

	m_L = mem_read_byte( ( m_H << 8 ) | m_L );
	break;
case 0x6F: /*      LD L,A */

	m_L = m_A;
	break;

case 0x70: /*      LD (HL),B */
	mem_write_byte( ( m_H << 8 ) | m_L, m_B );
	break;

case 0x71: /*      LD (HL),C */
	mem_write_byte( ( m_H << 8 ) | m_L, m_C );
	break;
case 0x72: /*      LD (HL),D */
	mem_write_byte( ( m_H << 8 ) | m_L, m_D );
	break;
case 0x73: /*      LD (HL),E */
	mem_write_byte( ( m_H << 8 ) | m_L, m_E );
	break;
case 0x74: /*      LD (HL),H */
	mem_write_byte( ( m_H << 8 ) | m_L, m_H );
	break;
case 0x75: /*      LD (HL),L */
	mem_write_byte( ( m_H << 8 ) | m_L, m_L );
	break;
case 0x76: /*      HALT */
	m_enable |= HALTED;
	m_PC--;
	break;
case 0x77: /*      LD (HL),A */
	mem_write_byte( ( m_H << 8 ) | m_L, m_A );
	break;
case 0x78: /*      LD A,B */

	m_A = m_B;
	break;
case 0x79: /*      LD A,C */

	m_A = m_C;
	break;
case 0x7A: /*      LD A,D */

	m_A = m_D;
	break;
case 0x7B: /*      LD A,E */

	m_A = m_E;
	break;
case 0x7C: /*      LD A,H */

	m_A = m_H;
	break;
case 0x7D: /*      LD A,L */

	m_A = m_L;
	break;
case 0x7E: /*      LD A,(HL) */

	m_A = mem_read_byte( ( m_H << 8 ) | m_L );
	break;
case 0x7F: /*      LD A,A */
	break;
case 0x80: /*      ADD A,B */

	ADD_A_X (m_B)
	break;
case 0x81: /*      ADD A,C */

	ADD_A_X (m_C)
	break;
case 0x82: /*      ADD A,D */

	ADD_A_X (m_D)
	break;
case 0x83: /*      ADD A,E */

	ADD_A_X (m_E)
	break;
case 0x84: /*      ADD A,H */

	ADD_A_X (m_H)
	break;
case 0x85: /*      ADD A,L */

	ADD_A_X (m_L)
	break;
case 0x86: /*      ADD A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	ADD_A_X (x)
	break;
case 0x87: /*      ADD A,A */

	ADD_A_X (m_A)
	break;
case 0x88: /*      ADC A,B */

	ADC_A_X (m_B)
	break;
case 0x89: /*      ADC A,C */

	ADC_A_X (m_C)
	break;
case 0x8A: /*      ADC A,D */

	ADC_A_X (m_D)
	break;
case 0x8B: /*      ADC A,E */

	ADC_A_X (m_E)
	break;
case 0x8C: /*      ADC A,H */

	ADC_A_X (m_H)
	break;
case 0x8D: /*      ADC A,L */

	ADC_A_X (m_L)
	break;
case 0x8E: /*      ADC A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	ADC_A_X (x)
	break;
case 0x8F: /*      ADC A,A */

	ADC_A_X (m_A)
	break;
case 0x90: /*      SUB A,B */

	SUB_A_X (m_B)
	break;
case 0x91: /*      SUB A,C */

	SUB_A_X (m_C)
	break;
case 0x92: /*      SUB A,D */

	SUB_A_X (m_D)
	break;
case 0x93: /*      SUB A,E */

	SUB_A_X (m_E)
	break;
case 0x94: /*      SUB A,H */

	SUB_A_X (m_H)
	break;
case 0x95: /*      SUB A,L */

	SUB_A_X (m_L)
	break;
case 0x96: /*      SUB A,(HL) */


	x = mem_read_byte( ( m_H << 8 ) | m_L );

	SUB_A_X (x)
	break;
case 0x97: /*      SUB A,A */

	SUB_A_X (m_A)
	break;
case 0x98: /*      SBC A,B */

	SBC_A_X (m_B)
	break;
case 0x99: /*      SBC A,C */

	SBC_A_X (m_C)
	break;
case 0x9A: /*      SBC A,D */

	SBC_A_X (m_D)
	break;
case 0x9B: /*      SBC A,E */

	SBC_A_X (m_E)
	break;
case 0x9C: /*      SBC A,H */

	SBC_A_X (m_H)
	break;
case 0x9D: /*      SBC A,L */

	SBC_A_X (m_L)
	break;
case 0x9E: /*      SBC A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	SBC_A_X (x)
	break;
case 0x9F: /*      SBC A,A */

	SBC_A_X (m_A)
	break;
case 0xA0: /*      AND A,B */

	AND_A_X (m_B)
	break;
case 0xA1: /*      AND A,C */

	AND_A_X (m_C)
	break;
case 0xA2: /*      AND A,D */

	AND_A_X (m_D)
	break;
case 0xA3: /*      AND A,E */

	AND_A_X (m_E)
	break;
case 0xA4: /*      AND A,H */

	AND_A_X (m_H)
	break;
case 0xA5: /*      AND A,L */

	AND_A_X (m_L)
	break;
case 0xA6: /*      AND A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	AND_A_X (x)
	break;
case 0xA7: /*      AND A,A */

	m_F = (m_A == 0) ? (FLAG_H | FLAG_Z) : FLAG_H;
	break;
case 0xA8: /*      XOR A,B */

	XOR_A_X (m_B)
	break;
case 0xA9: /*      XOR A,C */

	XOR_A_X (m_C)
	break;
case 0xAA: /*      XOR A,D */

	XOR_A_X (m_D)
	break;
case 0xAB: /*      XOR A,E */

	XOR_A_X (m_E)
	break;
case 0xAC: /*      XOR A,H */

	XOR_A_X (m_H)
	break;
case 0xAD: /*      XOR A,L */

	XOR_A_X (m_L)
	break;
case 0xAE: /*      XOR A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	XOR_A_X (x)
	break;
case 0xAF: /*      XOR A,A */

	XOR_A_X (m_A)
	break;
case 0xB0: /*      OR A,B */

	OR_A_X (m_B)
	break;
case 0xB1: /*      OR A,C */

	OR_A_X (m_C)
	break;
case 0xB2: /*      OR A,D */

	OR_A_X (m_D)
	break;
case 0xB3: /*      OR A,E */

	OR_A_X (m_E)
	break;
case 0xB4: /*      OR A,H */

	OR_A_X (m_H)
	break;
case 0xB5: /*      OR A,L */

	OR_A_X (m_L)
	break;
case 0xB6: /*      OR A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	OR_A_X (x)
	break;
case 0xB7: /*      OR A,A */

	OR_A_X (m_A)
	break;
case 0xB8: /*      CP A,B */

	CP_A_X (m_B)
	break;
case 0xB9: /*      CP A,C */

	CP_A_X (m_C)
	break;
case 0xBA: /*      CP A,D */

	CP_A_X (m_D)
	break;
case 0xBB: /*      CP A,E */

	CP_A_X (m_E)
	break;
case 0xBC: /*      CP A,H */

	CP_A_X (m_H)
	break;
case 0xBD: /*      CP A,L */

	CP_A_X (m_L)
	break;
case 0xBE: /*      CP A,(HL) */

	x = mem_read_byte( ( m_H << 8 ) | m_L );

	CP_A_X (x)
	break;
case 0xBF: /*      CP A,A */

	CP_A_X (m_A)
	break;
case 0xC0: /*      RET NZ */
	cycles_passed( 4 );
	if (!(m_F & FLAG_Z))
	{
		m_PC = mem_read_word( m_SP );
		m_SP += 2;
		cycles_passed( 4 );
	}
	break;
case 0xC1: /*      POP BC */
	POP( m_B, m_C );
	break;
case 0xC2: /*      JP NZ,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if ( ! (m_F & FLAG_Z) )
		{
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xC3: /*      JP n16 */
	m_PC = mem_read_word( m_PC );
	cycles_passed( 4 );
	break;
case 0xC4: /*      CALL NZ,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if ( ! (m_F & FLAG_Z) )
		{
			m_SP -= 2;
			mem_write_word( m_SP, m_PC );
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xC5: /*      PUSH BC */
	PUSH( m_B, m_C );
	cycles_passed( 4 );
	break;
case 0xC6: /*      ADD A,n8 */

	x = mem_read_byte( m_PC++ );
	ADD_A_X (x)
	break;
case 0xC7: /*      RST 0 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0;
	cycles_passed( 4 );
	break;
case 0xC8: /*      RET Z */
	cycles_passed( 4 );
	if (m_F & FLAG_Z)
	{
		m_PC = mem_read_word( m_SP );
		m_SP += 2;
		cycles_passed( 4 );
	}
	break;
case 0xC9: /*      RET */
	m_PC = mem_read_word( m_SP );
	m_SP += 2;
	cycles_passed( 4 );
	break;
case 0xCA: /*      JP Z,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if (m_F & FLAG_Z)
		{
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xCB: /*      PREFIX! */
	x = mem_read_byte( m_PC++ );
	switch (x)
	{
	#include "opc_cb.inc"
	}
	break;
case 0xCC: /*      CALL Z,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if (m_F & FLAG_Z)
		{
			m_SP -= 2;
			mem_write_word( m_SP, m_PC );
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xCD: /*      CALL n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		m_SP -= 2;
		mem_write_word( m_SP, m_PC );
		m_PC = addr;
		cycles_passed( 4 );
	}
	break;
case 0xCE: /*      ADC A,n8 */

	x = mem_read_byte( m_PC++ );
	ADC_A_X (x)
	break;
case 0xCF: /*      RST 8 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 8;
	cycles_passed( 4 );
	break;
case 0xD0: /*      RET NC */
	cycles_passed( 4 );
	if (!(m_F & FLAG_C))
	{
		m_PC = mem_read_word( m_SP );
		m_SP += 2;
		cycles_passed( 4 );
	}
	break;
case 0xD1: /*      POP DE */
	POP( m_D, m_E );
	break;
case 0xD2: /*      JP NC,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if ( ! (m_F & FLAG_C) )
		{
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xD4: /*      CALL NC,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if ( ! (m_F & FLAG_C) )
		{
			m_SP -= 2;
			mem_write_word( m_SP, m_PC );
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xD5: /*      PUSH DE */
	PUSH( m_D, m_E );
	cycles_passed( 4 );
	break;
case 0xD6: /*      SUB A,n8 */

	x = mem_read_byte( m_PC++ );
	SUB_A_X (x)
	break;
case 0xD7: /*      RST     $10 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0x10;
	cycles_passed( 4 );
	break;
case 0xD8: /*      RET C */
	cycles_passed( 4 );
	if (m_F & FLAG_C)
	{
		m_PC = mem_read_word( m_SP );
		m_SP += 2;
		cycles_passed( 4 );
	}
	break;
case 0xD9: /*      RETI */
	m_PC = mem_read_word( m_SP );
	m_SP += 2;
	m_enable |= IME;
	cycles_passed( 4 );
	break;
case 0xDA: /*      JP C,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if (m_F & FLAG_C)
		{
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xDC: /*      CALL C,n16 */
	{
		UINT16 addr = mem_read_word( m_PC );
		m_PC += 2;

		if (m_F & FLAG_C)
		{
			m_SP -= 2;
			mem_write_word( m_SP, m_PC );
			m_PC = addr;
			cycles_passed( 4 );
		}
	}
	break;
case 0xDE: /*      SBC A,n8 */

	x = mem_read_byte( m_PC++ );
	SBC_A_X (x)
	break;
case 0xDF: /*      RST     $18 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0x18;
	cycles_passed( 4 );
	break;
case 0xE0: /*      LD      ($FF00+n8),A */
	{
	UINT8 v = mem_read_byte( m_PC++ );
	mem_write_byte( 0xFF00 + v, m_A );
	}
	break;
case 0xE1: /*      POP HL */
	POP( m_H, m_L );
	break;
case 0xE2: /*      LD ($FF00+C),A */

	mem_write_byte( 0xFF00 + m_C, m_A );
	break;
case 0xE5: /*      PUSH HL */
	PUSH( m_H, m_L );
	cycles_passed( 4 );
	break;
case 0xE6: /*      AND A,n8 */

	x = mem_read_byte( m_PC++ );
	AND_A_X (x)
	break;
case 0xE7: /*      RST $20 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0x20;
	cycles_passed( 4 );
	break;
case 0xE8: /*      ADD SP,n8 */
/*
 *   Z - Reset.
 *   N - Reset.
 *   H - Set or reset according to operation.
 *   C - Set or reset according to operation.
 */

	{
	INT32 n;

	n = (INT8) mem_read_byte( m_PC++ );

	if ( ( m_SP & 0xFF ) + (UINT8)(n & 0xFF) > 0xFF )
	{
		m_F = FLAG_C;
	}
	else
	{
		m_F = 0;
	}

	if ( ( m_SP & 0x0F ) + ( n & 0x0F ) > 0x0F )
	{
		m_F |= FLAG_H;
	}

	m_SP = (UINT16) ( m_SP + n );
	}
	cycles_passed( 8 );
	break;
case 0xE9: /*      JP (HL) */
	m_PC = ( m_H << 8 ) | m_L;
	break;
case 0xEA: /*      LD (n16),A */

	mem_write_byte( mem_read_word( m_PC ), m_A );
	m_PC += 2;
	break;
case 0xEE: /*      XOR A,n8 */

	x = mem_read_byte( m_PC++ );
	XOR_A_X (x)
	break;
case 0xEF: /*      RST $28 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0x28;
	cycles_passed( 4 );
	break;
case 0xF0: /*      LD A,($FF00+n8) */
	{
	UINT8 v = mem_read_byte( m_PC++ );
	m_A = mem_read_byte( 0xFF00 + v );
	}
	break;
case 0xF1: /*      POP AF */
	POP( m_A, m_F );
	m_F &= 0xF0;
	break;
case 0xF2: /*      LD A,($FF00+C) */

	m_A = mem_read_byte( 0xFF00 + m_C );
	break;
case 0xF3: /*      DI */
	m_handle_ei_delay = false;
	m_enable &= ~IME;
	break;
case 0xF5: /*      PUSH AF */
	m_F &= 0xF0;
	PUSH( m_A, m_F );
	cycles_passed( 4 );
	break;
case 0xF6: /*      OR A,n8 */

	x = mem_read_byte( m_PC++ );
	OR_A_X (x)
	break;
case 0xF7: /*      RST $30 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0x30;
	cycles_passed( 4 );
	break;
case 0xF8: /*      LD HL,SP+n8 */
/*
 *   n = one UINT8 signed immediate value.
 * Flags affected:
 *   Z - Reset.
 *   N - Reset.
 *   H - Set or reset according to operation.
 *   C - Set or reset according to operation.
 *
 */

	{
	INT32 n;

	n = (INT8) mem_read_byte( m_PC++ );

	if ( ( m_SP & 0xFF ) + (UINT8)(n & 0xFF) > 0xFF )
	{
		m_F = FLAG_C;
	}
	else
	{
		m_F = 0;
	}

	if ( ( m_SP & 0x0F ) + ( n & 0x0F ) > 0x0F )
	{
		m_F |= FLAG_H;
	}

	UINT16 res = m_SP + n;

	m_L = res & 0xFF;
	m_H = res >> 8;
	}
	cycles_passed( 4 );
	break;
case 0xF9: /*      LD SP,HL */
	m_SP = ( m_H << 8 ) | m_L;
	cycles_passed( 4 );
	break;
case 0xFA: /*      LD A,(n16) */
	m_A = mem_read_byte( mem_read_word( m_PC ) );
	m_PC += 2;
	break;
case 0xFB: /*      EI */
	m_enable |= IME;
	m_handle_ei_delay = true;
	break;
case 0xFE: /*      CP A,n8 */
	x = mem_read_byte( m_PC++ );
	CP_A_X (x)
	break;
case 0xFF: /*      RST $38 */
	m_SP -= 2;
	mem_write_word( m_SP, m_PC );
	m_PC = 0x38;
	cycles_passed( 4 );
	break;
