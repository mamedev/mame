// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
UINT8 minx_cpu_device::ADD8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 + arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT16 minx_cpu_device::ADD16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 + arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


UINT8 minx_cpu_device::ADDC8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 + arg2 + ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT16 minx_cpu_device::ADDC16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 + arg2 + ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


UINT8 minx_cpu_device::INC8( UINT8 arg )
{
	UINT8 old_F = m_F;
	UINT8 res = ADD8( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT16 minx_cpu_device::INC16( UINT16 arg )
{
	UINT8 old_F = m_F;
	UINT16 res = ADD16( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::SUB8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 - arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT16 minx_cpu_device::SUB16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 - arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


UINT8 minx_cpu_device::SUBC8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 - arg2 - ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT16 minx_cpu_device::SUBC16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 - arg2 - ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


UINT8 minx_cpu_device::DEC8( UINT8 arg )
{
	UINT8 old_F = m_F;
	UINT8 res = SUB8( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT16 minx_cpu_device::DEC16( UINT16 arg )
{
	UINT8 old_F = m_F;
	UINT16 res = SUB16( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::AND8( UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 & arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::OR8( UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 | arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::XOR8( UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 ^ arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::NOT8( UINT8 arg )
{
	UINT8 res = ~arg;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::NEG8( UINT8 arg )
{
	UINT8 res = -arg;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::SAL8( UINT8 arg )
{
	UINT16 res = arg << 1;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0 && res == 0 ) ? FLAG_O : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::SAR8( UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( arg & 0x80 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0x80 && res == 0x80 ) ? FLAG_O : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT8 minx_cpu_device::SHL8( UINT8 arg )
{
	UINT16 res = arg << 1;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


UINT8 minx_cpu_device::SHR8( UINT8 arg )
{
	UINT16 res = arg >> 1;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT8 minx_cpu_device::ROLC8( UINT8 arg )
{
	UINT16 res = ( arg << 1 ) | ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT8 minx_cpu_device::RORC8( UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( ( m_F & FLAG_C ) ? 0x80 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT8 minx_cpu_device::ROL8( UINT8 arg )
{
	UINT16 res = ( arg << 1 ) | ( ( arg & 0x80 ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


UINT8 minx_cpu_device::ROR8( UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( ( arg & 0x01 ) ? 0x80 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


void minx_cpu_device::PUSH8( UINT8 arg )
{
	m_SP = m_SP - 1;
	WR( m_SP, arg );
}


void minx_cpu_device::PUSH16( UINT16 arg )
{
	PUSH8( arg >> 8 );
	PUSH8( arg & 0x00FF );
}


UINT8 minx_cpu_device::POP8()
{
	UINT8 res = RD( m_SP );
	m_SP = m_SP + 1;
	return res;
}


UINT16 minx_cpu_device::POP16()
{
	return POP8() | ( POP8() << 8 );
}


void minx_cpu_device::JMP( UINT16 arg )
{
	m_V = m_U;
	m_PC = arg;
}


void minx_cpu_device::CALL( UINT16 arg )
{
	PUSH8( m_V );
	PUSH16( m_PC );
	JMP( arg );
}


#define AD1_IHL UINT32 addr1 = ( m_I << 16 ) | m_HL
#define AD1_IN8 UINT32 addr1 = ( m_I << 16 ) | ( m_N << 8 ) | rdop()
#define AD1_I16 UINT32 addr1 = ( m_I << 16 ) | rdop16()
#define AD1_XIX UINT32 addr1 = ( m_XI << 16 ) | m_X
#define AD1_YIY UINT32 addr1 = ( m_YI << 16 ) | m_Y
#define AD1_X8  UINT32 addr1 = ( m_XI << 16 ) | ( m_X + rdop() )
#define AD1_Y8  UINT32 addr1 = ( m_YI << 16 ) | ( m_Y + rdop() )
#define AD1_XL  UINT32 addr1 = ( m_XI << 16 ) | ( m_X + ( m_HL & 0x00FF ) )
#define AD1_YL  UINT32 addr1 = ( m_YI << 16 ) | ( m_Y + ( m_HL & 0x00FF ) )
#define AD2_IHL UINT32 addr2 = ( m_I << 16 ) | m_HL
#define AD2_IN8 UINT32 addr2 = ( m_I << 16 ) | ( m_N << 8 ) | rdop()
#define AD2_I16 UINT32 addr2 = ( m_I << 16 ) | rdop(); addr2 |= ( rdop() << 8 )
#define AD2_XIX UINT32 addr2 = ( m_XI << 16 ) | m_X
#define AD2_YIY UINT32 addr2 = ( m_YI << 16 ) | m_Y
#define AD2_X8  UINT32 addr2 = ( m_XI << 16 ) | ( m_X + rdop() )
#define AD2_Y8  UINT32 addr2 = ( m_YI << 16 ) | ( m_Y + rdop() )
#define AD2_XL  UINT32 addr2 = ( m_XI << 16 ) | ( m_X + ( m_HL & 0x00FF ) )
#define AD2_YL  UINT32 addr2 = ( m_YI << 16 ) | ( m_Y + ( m_HL & 0x00FF ) )
