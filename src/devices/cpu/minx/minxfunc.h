// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
uint8_t minx_cpu_device::ADD8( uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 + arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint16_t minx_cpu_device::ADD16( uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 + arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


uint8_t minx_cpu_device::ADDC8( uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 + arg2 + ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint16_t minx_cpu_device::ADDC16( uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 + arg2 + ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


uint8_t minx_cpu_device::INC8( uint8_t arg )
{
	uint8_t old_F = m_F;
	uint8_t res = ADD8( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint16_t minx_cpu_device::INC16( uint16_t arg )
{
	uint8_t old_F = m_F;
	uint16_t res = ADD16( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::SUB8( uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 - arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint16_t minx_cpu_device::SUB16( uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 - arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


uint8_t minx_cpu_device::SUBC8( uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 - arg2 - ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint16_t minx_cpu_device::SUBC16( uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 - arg2 - ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


uint8_t minx_cpu_device::DEC8( uint8_t arg )
{
	uint8_t old_F = m_F;
	uint8_t res = SUB8( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint16_t minx_cpu_device::DEC16( uint16_t arg )
{
	uint8_t old_F = m_F;
	uint16_t res = SUB16( arg, 1 );
	m_F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::AND8( uint8_t arg1, uint8_t arg2 )
{
	uint8_t res = arg1 & arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::OR8( uint8_t arg1, uint8_t arg2 )
{
	uint8_t res = arg1 | arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::XOR8( uint8_t arg1, uint8_t arg2 )
{
	uint8_t res = arg1 ^ arg2;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::NOT8( uint8_t arg )
{
	uint8_t res = ~arg;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::NEG8( uint8_t arg )
{
	uint8_t res = -arg;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::SAL8( uint8_t arg )
{
	uint16_t res = arg << 1;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0 && res == 0 ) ? FLAG_O : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::SAR8( uint8_t arg )
{
	uint16_t res = ( arg >> 1 ) | ( arg & 0x80 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0x80 && res == 0x80 ) ? FLAG_O : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint8_t minx_cpu_device::SHL8( uint8_t arg )
{
	uint16_t res = arg << 1;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


uint8_t minx_cpu_device::SHR8( uint8_t arg )
{
	uint16_t res = arg >> 1;
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint8_t minx_cpu_device::ROLC8( uint8_t arg )
{
	uint16_t res = ( arg << 1 ) | ( ( m_F & FLAG_C ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint8_t minx_cpu_device::RORC8( uint8_t arg )
{
	uint16_t res = ( arg >> 1 ) | ( ( m_F & FLAG_C ) ? 0x80 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint8_t minx_cpu_device::ROL8( uint8_t arg )
{
	uint16_t res = ( arg << 1 ) | ( ( arg & 0x80 ) ? 1 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


uint8_t minx_cpu_device::ROR8( uint8_t arg )
{
	uint16_t res = ( arg >> 1 ) | ( ( arg & 0x01 ) ? 0x80 : 0 );
	m_F = ( m_F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


void minx_cpu_device::PUSH8( uint8_t arg )
{
	m_SP = m_SP - 1;
	WR( m_SP, arg );
}


void minx_cpu_device::PUSH16( uint16_t arg )
{
	PUSH8( arg >> 8 );
	PUSH8( arg & 0x00FF );
}


uint8_t minx_cpu_device::POP8()
{
	uint8_t res = RD( m_SP );
	m_SP = m_SP + 1;
	return res;
}


uint16_t minx_cpu_device::POP16()
{
	return POP8() | ( POP8() << 8 );
}


void minx_cpu_device::JMP( uint16_t arg )
{
	m_V = m_U;
	m_PC = arg;
}


void minx_cpu_device::CALL( uint16_t arg )
{
	PUSH8( m_V );
	PUSH16( m_PC );
	JMP( arg );
}


#define AD1_IHL uint32_t addr1 = ( m_I << 16 ) | m_HL
#define AD1_IN8 uint32_t addr1 = ( m_I << 16 ) | ( m_N << 8 ) | rdop()
#define AD1_I16 uint32_t addr1 = ( m_I << 16 ) | rdop16()
#define AD1_XIX uint32_t addr1 = ( m_XI << 16 ) | m_X
#define AD1_YIY uint32_t addr1 = ( m_YI << 16 ) | m_Y
#define AD1_X8  uint32_t addr1 = ( m_XI << 16 ) | ( m_X + rdop() )
#define AD1_Y8  uint32_t addr1 = ( m_YI << 16 ) | ( m_Y + rdop() )
#define AD1_XL  uint32_t addr1 = ( m_XI << 16 ) | ( m_X + ( m_HL & 0x00FF ) )
#define AD1_YL  uint32_t addr1 = ( m_YI << 16 ) | ( m_Y + ( m_HL & 0x00FF ) )
#define AD2_IHL uint32_t addr2 = ( m_I << 16 ) | m_HL
#define AD2_IN8 uint32_t addr2 = ( m_I << 16 ) | ( m_N << 8 ) | rdop()
#define AD2_I16 uint32_t addr2 = ( m_I << 16 ) | rdop(); addr2 |= ( rdop() << 8 )
#define AD2_XIX uint32_t addr2 = ( m_XI << 16 ) | m_X
#define AD2_YIY uint32_t addr2 = ( m_YI << 16 ) | m_Y
#define AD2_X8  uint32_t addr2 = ( m_XI << 16 ) | ( m_X + rdop() )
#define AD2_Y8  uint32_t addr2 = ( m_YI << 16 ) | ( m_Y + rdop() )
#define AD2_XL  uint32_t addr2 = ( m_XI << 16 ) | ( m_X + ( m_HL & 0x00FF ) )
#define AD2_YL  uint32_t addr2 = ( m_YI << 16 ) | ( m_Y + ( m_HL & 0x00FF ) )
