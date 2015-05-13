// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

void minx_cpu_device::execute_one_ce()
{
	const UINT8 opcode = rdop();

	switch (opcode)
	{
		case 0x00: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x01: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x02: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x03: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x04: { AD1_IHL; WR( addr1, ADD8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x05: { AD1_IHL; WR( addr1, ADD8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x06: { AD1_IHL; AD2_XIX; WR( addr1, ADD8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x07: { AD1_IHL; AD2_YIY; WR( addr1, ADD8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x08: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x09: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0A: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0B: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0C: { AD1_IHL; WR( addr1, ADDC8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x0D: { AD1_IHL; WR( addr1, ADDC8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x0E: { AD1_IHL; AD2_XIX; WR( addr1, ADDC8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x0F: { AD1_IHL; AD2_YIY; WR( addr1, ADDC8( RD( addr1 ), RD( addr2 ) ) ); }
			break;

		case 0x10: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x11: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x12: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x13: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x14: { AD1_IHL; WR( addr1, SUB8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x15: { AD1_IHL; WR( addr1, SUB8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x16: { AD1_IHL; AD2_XIX; WR( addr1, SUB8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x17: { AD1_IHL; AD2_YIY; WR( addr1, SUB8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x18: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x19: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1A: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1B: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1C: { AD1_IHL; WR( addr1, SUBC8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x1D: { AD1_IHL; WR( addr1, SUBC8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x1E: { AD1_IHL; AD2_XIX; WR( addr1, SUBC8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x1F: { AD1_IHL; AD2_YIY; WR( addr1, SUBC8( RD( addr1 ), RD( addr2 ) ) ); }
			break;

		case 0x20: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x21: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x22: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x23: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x24: { AD1_IHL; WR( addr1, AND8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x25: { AD1_IHL; WR( addr1, AND8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x26: { AD1_IHL; AD2_XIX; WR( addr1, AND8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x27: { AD1_IHL; AD2_YIY; WR( addr1, AND8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x28: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x29: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2A: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2B: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2C: { AD1_IHL; WR( addr1, OR8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x2D: { AD1_IHL; WR( addr1, OR8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x2E: { AD1_IHL; AD2_XIX; WR( addr1, OR8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x2F: { AD1_IHL; AD2_YIY; WR( addr1, OR8( RD( addr1 ), RD( addr2 ) ) ); }
			break;

		case 0x30: { AD2_X8; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x31: { AD2_Y8; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x32: { AD2_XL; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x33: { AD2_YL; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x34: { AD1_IHL; SUB8( RD( addr1 ), ( m_BA & 0x00FF ) ); }
			break;
		case 0x35: { AD1_IHL; SUB8( RD( addr1 ), rdop() ); }
			break;
		case 0x36: { AD1_IHL; AD2_XIX; SUB8( RD( addr1 ), RD( addr2 ) ); }
			break;
		case 0x37: { AD1_IHL; AD2_YIY; SUB8( RD( addr1 ), RD( addr2 ) ); }
			break;
		case 0x38: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x39: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3A: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3B: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3C: { AD1_IHL; WR( addr1, XOR8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
			break;
		case 0x3D: { AD1_IHL; WR( addr1, XOR8( RD( addr1 ), rdop() ) ); }
			break;
		case 0x3E: { AD1_IHL; AD2_XIX; WR( addr1, XOR8( RD( addr1 ), RD( addr2 ) ) ); }
			break;
		case 0x3F: { AD1_IHL; AD2_YIY; WR( addr1, XOR8( RD( addr1 ), RD( addr2 ) ) ); }
			break;

		case 0x40: { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x41: { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x42: { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x43: { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x44: { AD1_X8; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x45: { AD1_Y8; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x46: { AD1_XL; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x47: { AD1_YL; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x48: { AD2_X8; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x49: { AD2_Y8; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x4A: { AD2_XL; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x4B: { AD2_YL; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x4C: { AD1_X8; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x4D: { AD1_Y8; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x4E: { AD1_XL; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x4F: { AD1_YL; WR( addr1, ( m_BA >> 8 ) ); }
			break;

		case 0x50: { AD2_X8; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x51: { AD2_Y8; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x52: { AD2_XL; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x53: { AD2_YL; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x54: { AD1_X8; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x55: { AD1_Y8; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x56: { AD1_XL; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x57: { AD1_YL; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x58: { AD2_X8; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x59: { AD2_Y8; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x5A: { AD2_XL; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x5B: { AD2_YL; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x5C: { AD1_X8; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x5D: { AD1_Y8; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x5E: { AD1_XL; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x5F: { AD1_YL; WR( addr1, ( m_HL >> 8 ) ); }
			break;

		case 0x60: { AD1_IHL; AD2_X8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x61: { AD1_IHL; AD2_Y8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x62: { AD1_IHL; AD2_XL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x63: { AD1_IHL; AD2_YL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x64: { /* illegal operation? */ }
			break;
		case 0x65: { /* illegal operation? */ }
			break;
		case 0x66: { /* illegal operation? */ }
			break;
		case 0x67: { /* illegal operation? */ }
			break;
		case 0x68: { AD1_XIX; AD2_X8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x69: { AD1_XIX; AD2_Y8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x6A: { AD1_XIX; AD2_XL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x6B: { AD1_XIX; AD2_YL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x6C: { /* illegal operation? */ }
			break;
		case 0x6D: { /* illegal operation? */ }
			break;
		case 0x6E: { /* illegal operation? */ }
			break;
		case 0x6F: { /* illegal operation? */ }
			break;

		case 0x70: { /* illegal operation? */ }
			break;
		case 0x71: { /* illegal operation? */ }
			break;
		case 0x72: { /* illegal operation? */ }
			break;
		case 0x73: { /* illegal operation? */ }
			break;
		case 0x74: { /* illegal operation? */ }
			break;
		case 0x75: { /* illegal operation? */ }
			break;
		case 0x76: { /* illegal operation? */ }
			break;
		case 0x77: { /* illegal operation? */ }
			break;
		case 0x78: { AD1_YIY; AD2_X8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x79: { AD1_YIY; AD2_Y8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x7A: { AD1_YIY; AD2_XL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x7B: { AD1_YIY; AD2_YL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x7C: { /* illegal operation? */ }
			break;
		case 0x7D: { /* illegal operation? */ }
			break;
		case 0x7E: { /* illegal operation? */ }
			break;
		case 0x7F: { /* illegal operation? */ }
			break;

		case 0x80: { m_BA = ( m_BA & 0xFF00 ) | SAL8( m_BA & 0x00FF ); }
			break;
		case 0x81: { m_BA = ( m_BA & 0x00FF ) | ( SAL8( m_BA >> 8 )<< 8 ); }
			break;
		case 0x82: { AD1_IN8; WR( addr1, SAL8( RD( addr1 ) ) ); }
			break;
		case 0x83: { AD1_IHL; WR( addr1, SAL8( RD( addr1 ) ) ); }
			break;
		case 0x84: { m_BA = ( m_BA & 0xFF00 ) | SHL8( m_BA & 0x00FF ); }
			break;
		case 0x85: { m_BA = ( m_BA & 0x00FF ) | ( SHL8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x86: { AD1_IN8; WR( addr1, SHL8( RD( addr1 ) ) ); }
			break;
		case 0x87: { AD1_IHL; WR( addr1, SHL8( RD( addr1 ) ) ); }
			break;
		case 0x88: { m_BA = ( m_BA & 0xFF00 ) | SAR8( m_BA & 0x00FF ); }
			break;
		case 0x89: { m_BA = ( m_BA & 0x00FF ) | ( SAR8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x8A: { AD1_IN8; WR( addr1, SAR8( RD( addr1 ) ) ); }
			break;
		case 0x8B: { AD1_IHL; WR( addr1, SAR8( RD( addr1 ) ) ); }
			break;
		case 0x8C: { m_BA = ( m_BA & 0xFF00 ) | SHR8( m_BA & 0x00FF ); }
			break;
		case 0x8D: { m_BA = ( m_BA & 0x00FF ) | ( SHR8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x8E: { AD1_IN8; WR( addr1, SHR8( RD( addr1 ) ) ); }
			break;
		case 0x8F: { AD1_IHL; WR( addr1, SHR8( RD( addr1 ) ) ); }
			break;

		case 0x90: { m_BA = ( m_BA & 0xFF00 ) | ROLC8( m_BA & 0x00FF ); }
			break;
		case 0x91: { m_BA = ( m_BA & 0x00FF ) | ( ROLC8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x92: { AD1_IN8; WR( addr1, ROLC8( RD( addr1 ) ) ); }
			break;
		case 0x93: { AD1_IHL; WR( addr1, ROLC8( RD( addr1 ) ) ); }
			break;
		case 0x94: { m_BA = ( m_BA & 0xFF00 ) | ROL8( m_BA & 0x00FF ); }
			break;
		case 0x95: { m_BA = ( m_BA & 0x00FF ) | ( ROL8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x96: { AD1_IN8; WR( addr1, ROL8( RD( addr1 ) ) ); }
			break;
		case 0x97: { AD1_IHL; WR( addr1, ROL8( RD( addr1 ) ) ); }
			break;
		case 0x98: { m_BA = ( m_BA & 0xFF00 ) | RORC8( m_BA & 0x00FF ); }
			break;
		case 0x99: { m_BA = ( m_BA & 0x00FF ) | ( RORC8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x9A: { AD1_IN8; WR( addr1, RORC8( RD( addr1 ) ) ); }
			break;
		case 0x9B: { AD1_IHL; WR( addr1, RORC8( RD( addr1 ) ) ); }
			break;
		case 0x9C: { m_BA = ( m_BA & 0xFF00 ) | ROR8( m_BA & 0x00FF ); }
			break;
		case 0x9D: { m_BA = ( m_BA & 0x00FF ) | ( ROR8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x9E: { AD1_IN8; WR( addr1, ROR8( RD( addr1 ) ) ); }
			break;
		case 0x9F: { AD1_IHL; WR( addr1, ROR8( RD( addr1 ) ) ); }
			break;

		case 0xA0: { m_BA = ( m_BA & 0xFF00 ) | NOT8( m_BA & 0x00FF ); }
			break;
		case 0xA1: { m_BA = ( m_BA & 0x00FF ) | ( NOT8( m_BA >> 8 ) << 8 ); }
			break;
		case 0xA2: { AD1_IN8; WR( addr1, NOT8( RD( addr1 ) ) ); }
			break;
		case 0xA3: { AD1_IHL; WR( addr1, NOT8( RD( addr1 ) ) ); }
			break;
		case 0xA4: { m_BA = ( m_BA & 0xFF00 ) | NEG8( m_BA & 0x00FF ); }
			break;
		case 0xA5: { m_BA = ( m_BA & 0x00FF ) | ( NEG8( m_BA >> 8 ) << 8 ); }
			break;
		case 0xA6: { AD1_IN8; WR( addr1, NEG8( RD( addr1 ) ) ); }
			break;
		case 0xA7: { AD1_IHL; WR( addr1, NEG8( RD( addr1 ) ) ); }
			break;
		case 0xA8: { m_BA = ( ( m_BA & 0x0080 ) ? ( 0xFF00 | m_BA ) : ( m_BA & 0x00FF ) ); }
			break;
		case 0xA9: { /* illegal operation? */ }
			break;
		case 0xAA: { /* illegal operation? */ }
			break;
		case 0xAB: { /* illegal operation? */ }
			break;
		case 0xAC: { /* illegal operation? */ }
			break;
		case 0xAD: { /* illegal operation? */ }
			break;
		case 0xAE: { /* HALT */ m_halted = 1; }
			break;
		case 0xAF: { }
			break;

		case 0xB0: { m_BA = ( m_BA & 0x00FF ) | ( AND8( ( m_BA >> 8 ), rdop() ) << 8 ); }
			break;
		case 0xB1: { m_HL = ( m_HL & 0xFF00 ) | AND8( ( m_HL & 0x00FF ), rdop() ); }
			break;
		case 0xB2: { m_HL = ( m_HL & 0x00FF ) | ( AND8( ( m_HL >> 8 ), rdop() ) << 8 ); }
			break;
		case 0xB3: { /* illegal operation? */ }
			break;
		case 0xB4: { m_BA = ( m_BA & 0x00FF ) | ( OR8( ( m_BA >> 8 ), rdop() ) << 8 ); }
			break;
		case 0xB5: { m_HL = ( m_HL & 0xFF00 ) | OR8( ( m_HL & 0x00FF ), rdop() ); }
			break;
		case 0xB6: { m_HL = ( m_HL & 0x00FF ) | ( OR8( ( m_HL >> 8 ), rdop() ) << 8 ); }
			break;
		case 0xB7: { /* illegal operation? */ }
			break;
		case 0xB8: { m_BA = ( m_BA & 0x00FF ) | ( XOR8( ( m_BA >> 8 ), rdop() ) << 8 ); }
			break;
		case 0xB9: { m_HL = ( m_HL & 0xFF00 ) | XOR8( ( m_HL & 0x00FF ), rdop() ); }
			break;
		case 0xBA: { m_HL = ( m_HL & 0x00FF ) | ( XOR8( ( m_HL >> 8 ), rdop() ) << 8 ); }
			break;
		case 0xBB: { /* illegal operation? */ }
			break;
		case 0xBC: { SUB8( ( m_BA >> 8 ), rdop() ); }
			break;
		case 0xBD: { SUB8( ( m_HL & 0x00FF), rdop() ); }
			break;
		case 0xBE: { SUB8( ( m_HL >> 8 ), rdop() ); }
			break;
		case 0xBF: { SUB8( m_N, rdop() ); }
			break;

		case 0xC0: { m_BA = ( m_BA & 0xFF00 ) | m_N; }
			break;
		case 0xC1: { m_BA = ( m_BA & 0xFF00 ) | m_F; }
			break;
		case 0xC2: { m_N = ( m_BA & 0x00FF ); }
			break;
		case 0xC3: { m_F = ( m_BA & 0x00FF ); }
			break;
		case 0xC4: { m_U = rdop(); }
			break;
		case 0xC5: { m_I = rdop(); }
			break;
		case 0xC6: { m_XI = rdop(); }
			break;
		case 0xC7: { m_YI = rdop(); }
			break;
		case 0xC8: { m_BA = ( m_BA & 0xFF00 ) | m_V; }
			break;
		case 0xC9: { m_BA = ( m_BA & 0xFF00 ) | m_I; }
			break;
		case 0xCA: { m_BA = ( m_BA & 0xFF00 ) | m_XI; }
			break;
		case 0xCB: { m_BA = ( m_BA & 0xFF00 ) | m_YI; }
			break;
		case 0xCC: { m_U = ( m_BA & 0x00FF ); }
			break;
		case 0xCD: { m_I = ( m_BA & 0x00FF ); }
			break;
		case 0xCE: { m_XI = ( m_BA & 0x00FF ); }
			break;
		case 0xCF: { m_YI = ( m_BA & 0x00FF ); }
			break;

		case 0xD0: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0xD1: { AD2_I16; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0xD2: { AD2_I16; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0xD3: { AD2_I16; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0xD4: { AD1_I16; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0xD5: { AD1_I16; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0xD6: { AD1_I16; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0xD7: { AD1_I16; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0xD8: { m_HL = ( m_HL & 0x00FF ) * ( m_BA & 0x00FF );  }
			break;
		case 0xD9: { int d = m_HL / ( m_BA & 0x00FF ); m_HL = ( ( m_HL - ( ( m_BA & 0x00FF ) * d ) ) << 8 ) | d; }
			break;
		case 0xDA: { /* illegal operation? */ }
			break;
		case 0xDB: { /* illegal operation? */ }
			break;
		case 0xDC: { /* illegal operation? */ }
			break;
		case 0xDD: { /* illegal operation? */ }
			break;
		case 0xDE: { /* illegal operation? */ }
			break;
		case 0xDF: { /* illegal operation? */ }
			break;

		case 0xE0: { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE1: { INT8 d8 = rdop(); if ( ( m_F & FLAG_Z ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE2: { INT8 d8 = rdop(); if ( !( m_F & FLAG_Z ) && ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE3: { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE4: { INT8 d8 = rdop(); if ( ( m_F & FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE5: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE6: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_S ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE7: { INT8 d8 = rdop(); if ( ( m_F & FLAG_S ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE8: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X0 ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE9: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X1 ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xEA: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X2 ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xEB: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_DZ ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xEC: { INT8 d8 = rdop(); if ( ( m_E & EXEC_X0 ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xED: { INT8 d8 = rdop(); if ( ( m_E & EXEC_X1 ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xEE: { INT8 d8 = rdop(); if ( ( m_E & EXEC_X2 ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xEF: { INT8 d8 = rdop(); if ( ( m_E & EXEC_DZ ) ) { JMP( m_PC + d8 - 1 ); } }
			break;

		case 0xF0: { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF1: { INT8 d8 = rdop(); if ( ( m_F & FLAG_Z ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF2: { INT8 d8 = rdop(); if ( !( m_F & FLAG_Z ) && ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF3: { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) { CALL( m_PC + d8 - 1 ); } }
			break;
		case 0xF4: { INT8 d8 = rdop(); if ( ( m_F & FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF5: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF6: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_S ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF7: { INT8 d8 = rdop(); if ( ( m_F & FLAG_S ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF8: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X0 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xF9: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X1 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xFA: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X2 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xFB: { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_DZ ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xFC: { INT8 d8 = rdop(); if ( ( m_E & EXEC_X0 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xFD: { INT8 d8 = rdop(); if ( ( m_E & EXEC_X1 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xFE: { INT8 d8 = rdop(); if ( ( m_E & EXEC_X2 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xFF: { INT8 d8 = rdop(); if ( ( m_E & EXEC_DZ ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
	}

	m_icount -= insnminx_cycles_CE[opcode];
}


const int minx_cpu_device::insnminx_cycles_CE[256] = {
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,

	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	20, 20, 20, 20,  1,  1,  1,  1, 20, 20, 20, 20,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1, 20, 20, 20, 20,  1,  1,  1,  1,

	12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16,
	12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16,
	12, 12, 20, 16, 12, 12, 20, 16, 12,  1,  1,  1,  1,  1,  8,  8,
	12, 12, 12,  1, 12, 12, 12,  1, 20, 20, 20, 20, 12, 12, 12,  1,

	8,  8,  8, 12, 16, 12, 12, 12,  8,  8,  8,  8, 12,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 48, 52,  1,  1,  1,  1,  1,  1,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};
