// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#define RLC_8BIT(x)             \
{                               \
	register UINT8 f;           \
	(x)=(UINT8)(((x)<<1)|((x)>>7));      \
	if( (x)&1 )                 \
		f=FLAG_C;               \
	else                        \
		f=0;                    \
	if( (x)==0 )                \
		f|=FLAG_Z;              \
	m_F=f;          \
}

#define RL_8BIT(x)              \
{                               \
	register UINT8 r;           \
	r=((x)&0x80)?FLAG_C:0;      \
	(x)=(UINT8)(((x)<<1)|((m_F&FLAG_C)?1:0));    \
	if( (x)==0 )                \
		r|=FLAG_Z;              \
	m_F=r;          \
}

#define RRC_8BIT(x)             \
{                               \
	register UINT8 f;           \
	(x)=(UINT8)(((x)>>1)|((x)<<7));      \
	if( (x)&0x80 )              \
		f=FLAG_C;               \
	else                        \
		f=0;                    \
	if( (x)==0 )                \
		f|=FLAG_Z;              \
	m_F=f;          \
}

#define RR_8BIT(x)              \
{                               \
	register UINT8 r;           \
	r=((x)&1)?FLAG_C:0;         \
	(x)=(UINT8)(((x)>>1)|((m_F&FLAG_C)?0x80:0));     \
	if( (x)==0 )                \
		r|=FLAG_Z;              \
	m_F=r;          \
}

#define SLA_8BIT(x)             \
{                               \
	register UINT8 f;           \
	if( (x)&0x80 )              \
		f=FLAG_C;               \
	else                        \
		f=0;                    \
	(x)<<=1;                    \
	if( (x)==0 )                \
		f|=FLAG_Z;              \
	m_F=f;          \
}

#define SRA_8BIT(x)             \
{                               \
	register UINT8 f;           \
	if( (x)&1 )                 \
		f=FLAG_C;               \
	else                        \
		f=0;                    \
	(x)=(UINT8)(((char)(x))>>1);     \
	if( (x)==0 )                \
		f|=FLAG_Z;              \
	m_F=f;          \
}

#define SWAP_8BIT(x)            \
	(x)=(UINT8)(((x)>>4)|((x)<<4));      \
	if( (x)==0 )                \
		m_F=FLAG_Z; \
	else                        \
		m_F=0;


#define SRL_8BIT(x)             \
{                               \
	register UINT8 f;           \
	if( (x)&1 )                 \
		f=FLAG_C;               \
	else                        \
		f=0;                    \
	(x)>>=1;                    \
	if( (x)==0 )                \
		f|=FLAG_Z;              \
	m_F=f;          \
}

#define BIT_8BIT(n,x)           \
	if( (x)&(1<<(n)) )          \
		m_F=(UINT8)(FLAG_H|(m_F&FLAG_C));  \
	else                        \
		m_F=(UINT8)(FLAG_Z|FLAG_H|(m_F&FLAG_C));

#define RES_8BIT(n,x)   (x)&=~(1<<(n));

#define SET_8BIT(n,x)   (x)|=(1<<(n));

/**********************************************************/

case 0x00:
	/*      RLC B */

	RLC_8BIT (m_B)
	break;
case 0x01:
	/*      RLC C */

	RLC_8BIT (m_C)
	break;
case 0x02:
	/*      RLC D */

	RLC_8BIT (m_D)
	break;
case 0x03:
	/*      RLC E */

	RLC_8BIT (m_E)
	break;
case 0x04:
	/*      RLC H */

	RLC_8BIT (m_H)
	break;
case 0x05:
	/*      RLC L */

	RLC_8BIT (m_L)
	break;
case 0x06:
	/*      RLC (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr );
		RLC_8BIT (x)
		mem_write_byte( addr, x );
	}
	break;
case 0x07:
	/*      RLC A */

	RLC_8BIT (m_A)
	break;
case 0x08:
	/*      RRC B */

	RRC_8BIT (m_B)
	break;
case 0x09:
	/*      RRC C */

	RRC_8BIT (m_C)
	break;
case 0x0A:
	/*      RRC D */

	RRC_8BIT (m_D)
	break;
case 0x0B:
	/*      RRC E */

	RRC_8BIT (m_E)
	break;
case 0x0C:
	/*      RRC H */

	RRC_8BIT (m_H)
	break;
case 0x0D:
	/*      RRC L */

	RRC_8BIT (m_L)
	break;
case 0x0E:
	/*      RRC (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RRC_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x0F:
	/*      RRC A */

	RRC_8BIT (m_A)
	break;
case 0x10:
	/*      RL B */

	RL_8BIT (m_B)
	break;
case 0x11:
	/*      RL C */

	RL_8BIT (m_C)
	break;
case 0x12:
	/*      RL D */

	RL_8BIT (m_D)
	break;
case 0x13:
	/*      RL E */

	RL_8BIT (m_E)
	break;
case 0x14:
	/*      RL H */

	RL_8BIT (m_H)
	break;
case 0x15:
	/*      RL L */

	RL_8BIT (m_L)
	break;
case 0x16:
	/*      RL (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RL_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x17:
	/*      RL A */

	RL_8BIT (m_A)
	break;
case 0x18:
	/*      RR B */

	RR_8BIT (m_B)
	break;
case 0x19:
	/*      RR C */

	RR_8BIT (m_C)
	break;
case 0x1A:
	/*      RR D */

	RR_8BIT (m_D)
	break;
case 0x1B:
	/*      RR E */

	RR_8BIT (m_E)
	break;
case 0x1C:
	/*      RR H */

	RR_8BIT (m_H)
	break;
case 0x1D:
	/*      RR L */

	RR_8BIT (m_L)
	break;
case 0x1E:
	/*      RR (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RR_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x1F:
	/*      RR A */

	RR_8BIT (m_A)
	break;
case 0x20:
	/*      SLA B */

	SLA_8BIT (m_B)
	break;
case 0x21:
	/*      SLA C */

	SLA_8BIT (m_C)
	break;
case 0x22:
	/*      SLA D */

	SLA_8BIT (m_D)
	break;
case 0x23:
	/*      SLA E */

	SLA_8BIT (m_E)
	break;
case 0x24:
	/*      SLA H */

	SLA_8BIT (m_H)
	break;
case 0x25:
	/*      SLA L */

	SLA_8BIT (m_L)
	break;
case 0x26:
	/*      SLA (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SLA_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x27:
	/*      SLA A */

	SLA_8BIT (m_A)
	break;
case 0x28:
	/*      SRA B */

	SRA_8BIT (m_B)
	break;
case 0x29:
	/*      SRA C */

	SRA_8BIT (m_C)
	break;
case 0x2A:
	/*      SRA D */

	SRA_8BIT (m_D)
	break;
case 0x2B:
	/*      SRA E */

	SRA_8BIT (m_E)
	break;
case 0x2C:
	/*      SRA H */

	SRA_8BIT (m_H)
	break;
case 0x2D:
	/*      SRA L */

	SRA_8BIT (m_L)
	break;
case 0x2E:
	/*      SRA (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SRA_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x2F:
	/*      SRA A */

	SRA_8BIT (m_A)
	break;
case 0x30:
	/*      SWAP B */

	SWAP_8BIT (m_B)
	break;
case 0x31:
	/*      SWAP C */

	SWAP_8BIT (m_C)
	break;
case 0x32:
	/*      SWAP D */

	SWAP_8BIT (m_D)
	break;
case 0x33:
	/*      SWAP E */

	SWAP_8BIT (m_E)
	break;
case 0x34:
	/*      SWAP H */

	SWAP_8BIT (m_H)
	break;
case 0x35:
	/*      SWAP L */

	SWAP_8BIT (m_L)
	break;
case 0x36:
	/*      SWAP (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SWAP_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x37:
	/*      SWAP A */

	SWAP_8BIT (m_A)
	break;
case 0x38:
	/*      SRL B */

	SRL_8BIT (m_B)
	break;
case 0x39:
	/*      SRL C */

	SRL_8BIT (m_C)
	break;
case 0x3A:
	/*      SRL D */

	SRL_8BIT (m_D)
	break;
case 0x3B:
	/*      SRL E */

	SRL_8BIT (m_E)
	break;
case 0x3C:
	/*      SRL H */

	SRL_8BIT (m_H)
	break;
case 0x3D:
	/*      SRL L */

	SRL_8BIT (m_L)
	break;
case 0x3E:
	/*      SRL (HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SRL_8BIT (x)
		mem_write_byte( addr, x);
	}
	break;
case 0x3F:
	/*      SRL A */

	SRL_8BIT (m_A)
	break;
case 0x40:
	/*      BIT 0,B */

	BIT_8BIT (0, m_B)
	break;
case 0x41:
	/*      BIT 0,C */

	BIT_8BIT (0, m_C)
	break;
case 0x42:
	/*      BIT 0,D */

	BIT_8BIT (0, m_D)
	break;
case 0x43:
	/*      BIT 0,E */

	BIT_8BIT (0, m_E)
	break;
case 0x44:
	/*      BIT 0,H */

	BIT_8BIT (0, m_H)
	break;
case 0x45:
	/*      BIT 0,L */

	BIT_8BIT (0, m_L)
	break;
case 0x46:
	/*      BIT 0,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (0, x)
	}
	break;
case 0x47:
	/*      BIT 0,A */

	BIT_8BIT (0, m_A)
	break;
case 0x48:
	/*      BIT 1,B */

	BIT_8BIT (1, m_B)
	break;
case 0x49:
	/*      BIT 1,C */

	BIT_8BIT (1, m_C)
	break;
case 0x4A:
	/*      BIT 1,D */

	BIT_8BIT (1, m_D)
	break;
case 0x4B:
	/*      BIT 1,E */

	BIT_8BIT (1, m_E)
	break;
case 0x4C:
	/*      BIT 1,H */

	BIT_8BIT (1, m_H)
	break;
case 0x4D:
	/*      BIT 1,L */

	BIT_8BIT (1, m_L)
	break;
case 0x4E:
	/*      BIT 1,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (1, x)
	}
	break;
case 0x4F:
	/*      BIT 1,A */

	BIT_8BIT (1, m_A)
	break;
case 0x50:
	/*      BIT 2,B */

	BIT_8BIT (2, m_B)
	break;
case 0x51:
	/*      BIT 2,C */

	BIT_8BIT (2, m_C)
	break;
case 0x52:
	/*      BIT 2,D */

	BIT_8BIT (2, m_D)
	break;
case 0x53:
	/*      BIT 2,E */

	BIT_8BIT (2, m_E)
	break;
case 0x54:
	/*      BIT 2,H */

	BIT_8BIT (2, m_H)
	break;
case 0x55:
	/*      BIT 2,L */

	BIT_8BIT (2, m_L)
	break;
case 0x56:
	/*      BIT 2,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (2, x)
	}
	break;
case 0x57:
	/*      BIT 2,A */

	BIT_8BIT (2, m_A)
	break;
case 0x58:
	/*      BIT 3,B */

	BIT_8BIT (3, m_B)
	break;
case 0x59:
	/*      BIT 3,C */

	BIT_8BIT (3, m_C)
	break;
case 0x5A:
	/*      BIT 3,D */

	BIT_8BIT (3, m_D)
	break;
case 0x5B:
	/*      BIT 3,E */

	BIT_8BIT (3, m_E)
	break;
case 0x5C:
	/*      BIT 3,H */

	BIT_8BIT (3, m_H)
	break;
case 0x5D:
	/*      BIT 3,L */

	BIT_8BIT (3, m_L)
	break;
case 0x5E:
	/*      BIT 3,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (3, x)
	}
	break;
case 0x5F:
	/*      BIT 3,A */

	BIT_8BIT (3, m_A)
	break;
case 0x60:
	/*      BIT 4,B */

	BIT_8BIT (4, m_B)
	break;
case 0x61:
	/*      BIT 4,C */

	BIT_8BIT (4, m_C)
	break;
case 0x62:
	/*      BIT 4,D */

	BIT_8BIT (4, m_D)
	break;
case 0x63:
	/*      BIT 4,E */

	BIT_8BIT (4, m_E)
	break;
case 0x64:
	/*      BIT 4,H */

	BIT_8BIT (4, m_H)
	break;
case 0x65:
	/*      BIT 4,L */

	BIT_8BIT (4, m_L)
	break;
case 0x66:
	/*      BIT 4,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (4, x)
	}
	break;
case 0x67:
	/*      BIT 4,A */

	BIT_8BIT (4, m_A)
	break;
case 0x68:
	/*      BIT 5,B */

	BIT_8BIT (5, m_B)
	break;
case 0x69:
	/*      BIT 5,C */

	BIT_8BIT (5, m_C)
	break;
case 0x6A:
	/*      BIT 5,D */

	BIT_8BIT (5, m_D)
	break;
case 0x6B:
	/*      BIT 5,E */

	BIT_8BIT (5, m_E)
	break;
case 0x6C:
	/*      BIT 5,H */

	BIT_8BIT (5, m_H)
	break;
case 0x6D:
	/*      BIT 5,L */

	BIT_8BIT (5, m_L)
	break;
case 0x6E:
	/*      BIT 5,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (5, x)
	}
	break;
case 0x6F:
	/*      BIT 5,A */

	BIT_8BIT (5, m_A)
	break;
case 0x70:
	/*      BIT 6,B */

	BIT_8BIT (6, m_B)
	break;
case 0x71:
	/*      BIT 6,C */

	BIT_8BIT (6, m_C)
	break;
case 0x72:
	/*      BIT 6,D */

	BIT_8BIT (6, m_D)
	break;
case 0x73:
	/*      BIT 6,E */

	BIT_8BIT (6, m_E)
	break;
case 0x74:
	/*      BIT 6,H */

	BIT_8BIT (6, m_H)
	break;
case 0x75:
	/*      BIT 6,L */

	BIT_8BIT (6, m_L)
	break;
case 0x76:
	/*      BIT 6,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (6, x)
	}
	break;
case 0x77:
	/*      BIT 6,A */

	BIT_8BIT (6, m_A)
	break;
case 0x78:
	/*      BIT 7,B */

	BIT_8BIT (7, m_B)
	break;
case 0x79:
	/*      BIT 7,C */

	BIT_8BIT (7, m_C)
	break;
case 0x7A:
	/*      BIT 7,D */

	BIT_8BIT (7, m_D)
	break;
case 0x7B:
	/*      BIT 7,E */

	BIT_8BIT (7, m_E)
	break;
case 0x7C:
	/*      BIT 7,H */

	BIT_8BIT (7, m_H)
	break;
case 0x7D:
	/*      BIT 7,L */

	BIT_8BIT (7, m_L)
	break;
case 0x7E:
	/*      BIT 7,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		BIT_8BIT (7, x)
	}
	break;
case 0x7F:
	/*      BIT 7,A */

	BIT_8BIT (7, m_A)
	break;
case 0x80:
	/*      RES 0,B */

	RES_8BIT (0, m_B)
	break;
case 0x81:
	/*      RES 0,C */

	RES_8BIT (0, m_C)
	break;
case 0x82:
	/*      RES 0,D */

	RES_8BIT (0, m_D)
	break;
case 0x83:
	/*      RES 0,E */

	RES_8BIT (0, m_E)
	break;
case 0x84:
	/*      RES 0,H */

	RES_8BIT (0, m_H)
	break;
case 0x85:
	/*      RES 0,L */

	RES_8BIT (0, m_L)
	break;
case 0x86:
	/*      RES 0,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (0, x)
		mem_write_byte( addr, x);
	}
	break;
case 0x87:
	/*      RES 0,A */

	RES_8BIT (0, m_A)
	break;
case 0x88:
	/*      RES 1,B */

	RES_8BIT (1, m_B)
	break;
case 0x89:
	/*      RES 1,C */

	RES_8BIT (1, m_C)
	break;
case 0x8A:
	/*      RES 1,D */

	RES_8BIT (1, m_D)
	break;
case 0x8B:
	/*      RES 1,E */

	RES_8BIT (1, m_E)
	break;
case 0x8C:
	/*      RES 1,H */

	RES_8BIT (1, m_H)
	break;
case 0x8D:
	/*      RES 1,L */

	RES_8BIT (1, m_L)
	break;
case 0x8E:
	/*      RES 1,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (1, x)
		mem_write_byte( addr, x);
	}
	break;
case 0x8F:
	/*      RES 1,A */

	RES_8BIT (1, m_A)
	break;
case 0x90:
	/*      RES 2,B */

	RES_8BIT (2, m_B)
	break;
case 0x91:
	/*      RES 2,C */

	RES_8BIT (2, m_C)
	break;
case 0x92:
	/*      RES 2,D */

	RES_8BIT (2, m_D)
	break;
case 0x93:
	/*      RES 2,E */

	RES_8BIT (2, m_E)
	break;
case 0x94:
	/*      RES 2,H */

	RES_8BIT (2, m_H)
	break;
case 0x95:
	/*      RES 2,L */

	RES_8BIT (2, m_L)
	break;
case 0x96:
	/*      RES 2,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (2, x)
		mem_write_byte( addr, x);
	}
	break;
case 0x97:
	/*      RES 2,A */

	RES_8BIT (2, m_A)
	break;
case 0x98:
	/*      RES 3,B */

	RES_8BIT (3, m_B)
	break;
case 0x99:
	/*      RES 3,C */

	RES_8BIT (3, m_C)
	break;
case 0x9A:
	/*      RES 3,D */

	RES_8BIT (3, m_D)
	break;
case 0x9B:
	/*      RES 3,E */

	RES_8BIT (3, m_E)
	break;
case 0x9C:
	/*      RES 3,H */

	RES_8BIT (3, m_H)
	break;
case 0x9D:
	/*      RES 3,L */

	RES_8BIT (3, m_L)
	break;
case 0x9E:
	/*      RES 3,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (3, x)
		mem_write_byte( addr, x);
	}
	break;
case 0x9F:
	/*      RES 3,A */

	RES_8BIT (3, m_A)
	break;
case 0xA0:
	/*      RES 4,B */

	RES_8BIT (4, m_B)
	break;
case 0xA1:
	/*      RES 4,C */

	RES_8BIT (4, m_C)
	break;
case 0xA2:
	/*      RES 4,D */

	RES_8BIT (4, m_D)
	break;
case 0xA3:
	/*      RES 4,E */

	RES_8BIT (4, m_E)
	break;
case 0xA4:
	/*      RES 4,H */

	RES_8BIT (4, m_H)
	break;
case 0xA5:
	/*      RES 4,L */

	RES_8BIT (4, m_L)
	break;
case 0xA6:
	/*      RES 4,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (4, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xA7:
	/*      RES 4,A */

	RES_8BIT (4, m_A)
	break;
case 0xA8:
	/*      RES 5,B */

	RES_8BIT (5, m_B)
	break;
case 0xA9:
	/*      RES 5,C */

	RES_8BIT (5, m_C)
	break;
case 0xAA:
	/*      RES 5,D */

	RES_8BIT (5, m_D)
	break;
case 0xAB:
	/*      RES 5,E */

	RES_8BIT (5, m_E)
	break;
case 0xAC:
	/*      RES 5,H */

	RES_8BIT (5, m_H)
	break;
case 0xAD:
	/*      RES 5,L */

	RES_8BIT (5, m_L)
	break;
case 0xAE:
	/*      RES 5,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (5, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xAF:
	/*      RES 5,A */

	RES_8BIT (5, m_A)
	break;
case 0xB0:
	/*      RES 6,B */

	RES_8BIT (6, m_B)
	break;
case 0xB1:
	/*      RES 6,C */

	RES_8BIT (6, m_C)
	break;
case 0xB2:
	/*      RES 6,D */

	RES_8BIT (6, m_D)
	break;
case 0xB3:
	/*      RES 6,E */

	RES_8BIT (6, m_E)
	break;
case 0xB4:
	/*      RES 6,H */

	RES_8BIT (6, m_H)
	break;
case 0xB5:
	/*      RES 6,L */

	RES_8BIT (6, m_L)
	break;
case 0xB6:
	/*      RES 6,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (6, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xB7:
	/*      RES 6,A */

	RES_8BIT (6, m_A)
	break;
case 0xB8:
	/*      RES 7,B */

	RES_8BIT (7, m_B)
	break;
case 0xB9:
	/*      RES 7,C */

	RES_8BIT (7, m_C)
	break;
case 0xBA:
	/*      RES 7,D */

	RES_8BIT (7, m_D)
	break;
case 0xBB:
	/*      RES 7,E */

	RES_8BIT (7, m_E)
	break;
case 0xBC:
	/*      RES 7,H */

	RES_8BIT (7, m_H)
	break;
case 0xBD:
	/*      RES 7,L */

	RES_8BIT (7, m_L)
	break;
case 0xBE:
	/*      RES 7,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		RES_8BIT (7, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xBF:
	/*      RES 7,A */

	RES_8BIT (7, m_A)
	break;
case 0xC0:
	/*      SET 0,B */

	SET_8BIT (0, m_B)
	break;
case 0xC1:
	/*      SET 0,C */

	SET_8BIT (0, m_C)
	break;
case 0xC2:
	/*      SET 0,D */

	SET_8BIT (0, m_D)
	break;
case 0xC3:
	/*      SET 0,E */

	SET_8BIT (0, m_E)
	break;
case 0xC4:
	/*      SET 0,H */

	SET_8BIT (0, m_H)
	break;
case 0xC5:
	/*      SET 0,L */

	SET_8BIT (0, m_L)
	break;
case 0xC6:
	/*      SET 0,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (0, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xC7:
	/*      SET 0,A */

	SET_8BIT (0, m_A)
	break;
case 0xC8:
	/*      SET 1,B */

	SET_8BIT (1, m_B)
	break;
case 0xC9:
	/*      SET 1,C */

	SET_8BIT (1, m_C)
	break;
case 0xCA:
	/*      SET 1,D */

	SET_8BIT (1, m_D)
	break;
case 0xCB:
	/*      SET 1,E */

	SET_8BIT (1, m_E)
	break;
case 0xCC:
	/*      SET 1,H */

	SET_8BIT (1, m_H)
	break;
case 0xCD:
	/*      SET 1,L */

	SET_8BIT (1, m_L)
	break;
case 0xCE:
	/*      SET 1,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (1, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xCF:
	/*      SET 1,A */

	SET_8BIT (1, m_A)
	break;
case 0xD0:
	/*      SET 2,B */

	SET_8BIT (2, m_B)
	break;
case 0xD1:
	/*      SET 2,C */

	SET_8BIT (2, m_C)
	break;
case 0xD2:
	/*      SET 2,D */

	SET_8BIT (2, m_D)
	break;
case 0xD3:
	/*      SET 2,E */

	SET_8BIT (2, m_E)
	break;
case 0xD4:
	/*      SET 2,H */

	SET_8BIT (2, m_H)
	break;
case 0xD5:
	/*      SET 2,L */

	SET_8BIT (2, m_L)
	break;
case 0xD6:
	/*      SET 2,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (2, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xD7:
	/*      SET 2,A */

	SET_8BIT (2, m_A)
	break;
case 0xD8:
	/*      SET 3,B */

	SET_8BIT (3, m_B)
	break;
case 0xD9:
	/*      SET 3,C */

	SET_8BIT (3, m_C)
	break;
case 0xDA:
	/*      SET 3,D */

	SET_8BIT (3, m_D)
	break;
case 0xDB:
	/*      SET 3,E */

	SET_8BIT (3, m_E)
	break;
case 0xDC:
	/*      SET 3,H */

	SET_8BIT (3, m_H)
	break;
case 0xDD:
	/*      SET 3,L */

	SET_8BIT (3, m_L)
	break;
case 0xDE:
	/*      SET 3,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (3, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xDF:
	/*      SET 3,A */

	SET_8BIT (3, m_A)
	break;
case 0xE0:
	/*      SET 4,B */

	SET_8BIT (4, m_B)
	break;
case 0xE1:
	/*      SET 4,C */

	SET_8BIT (4, m_C)
	break;
case 0xE2:
	/*      SET 4,D */

	SET_8BIT (4, m_D)
	break;
case 0xE3:
	/*      SET 4,E */

	SET_8BIT (4, m_E)
	break;
case 0xE4:
	/*      SET 4,H */

	SET_8BIT (4, m_H)
	break;
case 0xE5:
	/*      SET 4,L */

	SET_8BIT (4, m_L)
	break;
case 0xE6:
	/*      SET 4,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (4, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xE7:
	/*      SET 4,A */

	SET_8BIT (4, m_A)
	break;
case 0xE8:
	/*      SET 5,B */

	SET_8BIT (5, m_B)
	break;
case 0xE9:
	/*      SET 5,C */

	SET_8BIT (5, m_C)
	break;
case 0xEA:
	/*      SET 5,D */

	SET_8BIT (5, m_D)
	break;
case 0xEB:
	/*      SET 5,E */

	SET_8BIT (5, m_E)
	break;
case 0xEC:
	/*      SET 5,H */

	SET_8BIT (5, m_H)
	break;
case 0xED:
	/*      SET 5,L */

	SET_8BIT (5, m_L)
	break;
case 0xEE:
	/*      SET 5,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (5, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xEF:
	/*      SET 5,A */

	SET_8BIT (5, m_A)
	break;
case 0xF0:
	/*      SET 6,B */

	SET_8BIT (6, m_B)
	break;
case 0xF1:
	/*      SET 6,C */

	SET_8BIT (6, m_C)
	break;
case 0xF2:
	/*      SET 6,D */

	SET_8BIT (6, m_D)
	break;
case 0xF3:
	/*      SET 6,E */

	SET_8BIT (6, m_E)
	break;
case 0xF4:
	/*      SET 6,H */

	SET_8BIT (6, m_H)
	break;
case 0xF5:
	/*      SET 6,L */

	SET_8BIT (6, m_L)
	break;
case 0xF6:
	/*      SET 6,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (6, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xF7:
	/*      SET 6,A */

	SET_8BIT (6, m_A)
	break;
case 0xF8:
	/*      SET 7,B */

	SET_8BIT (7, m_B)
	break;
case 0xF9:
	/*      SET 7,C */

	SET_8BIT (7, m_C)
	break;
case 0xFA:
	/*      SET 7,D */

	SET_8BIT (7, m_D)
	break;
case 0xFB:
	/*      SET 7,E */

	SET_8BIT (7, m_E)
	break;
case 0xFC:
	/*      SET 7,H */

	SET_8BIT (7, m_H)
	break;
case 0xFD:
	/*      SET 7,L */

	SET_8BIT (7, m_L)
	break;
case 0xFE:
	/*      SET 7,(HL) */
	{
		UINT16 addr = ( m_H << 8 ) | m_L;

		x = mem_read_byte( addr);
		SET_8BIT (7, x)
		mem_write_byte( addr, x);
	}
	break;
case 0xFF:
	/*      SET 7,A */

	SET_8BIT (7, m_A)
	break;
