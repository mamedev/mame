// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

void minx_cpu_device::execute_one()
{
	const UINT8 opcode = rdop();

	switch (opcode)
	{
		case 0x00: { m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x01: { m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x02: { m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x03: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x04: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x05: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x06: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x07: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x08: { m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x09: { m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x0A: { m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x0B: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0C: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0D: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0E: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x0F: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;

		case 0x10: { m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x11: { m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x12: { m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x13: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x14: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x15: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x16: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x17: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x18: { m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x19: { m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x1A: { m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x1B: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1C: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1D: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1E: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x1F: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;

		case 0x20: { m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x21: { m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x22: { m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x23: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x24: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x25: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x26: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x27: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x28: { m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x29: { m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x2A: { m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x2B: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2C: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2D: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2E: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x2F: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;

		case 0x30: { SUB8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x31: { SUB8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x32: { SUB8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x33: { AD2_IHL; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x34: { AD2_IN8; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x35: { AD2_I16; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x36: { AD2_XIX; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x37: { AD2_YIY; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x38: { m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
			break;
		case 0x39: { m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
			break;
		case 0x3A: { m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), rdop() ); }
			break;
		case 0x3B: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3C: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3D: { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3E: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;
		case 0x3F: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
			break;

		case 0x40: { m_BA = ( m_BA & 0xFF00 ) | ( m_BA & 0x00FF); }
			break;
		case 0x41: { m_BA = ( m_BA & 0xFF00 ) | ( m_BA >> 8 ); }
			break;
		case 0x42: { m_BA = ( m_BA & 0xFF00 ) | ( m_HL & 0x00FF); }
			break;
		case 0x43: { m_BA = ( m_BA & 0xFF00 ) | ( m_HL >> 8 ); }
			break;
		case 0x44: { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x45: { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x46: { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x47: { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x48: { m_BA = ( m_BA & 0x00FF ) | ( ( m_BA & 0x00FF) << 8 ); }
			break;
		case 0x49: { m_BA = ( m_BA & 0x00FF ) | ( ( m_BA >> 8 ) << 8 ); }
			break;
		case 0x4A: { m_BA = ( m_BA & 0x00FF ) | ( ( m_HL & 0x00FF) << 8 ); }
			break;
		case 0x4B: { m_BA = ( m_BA & 0x00FF ) | ( ( m_HL >> 8 ) << 8 ); }
			break;
		case 0x4C: { AD2_IN8; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x4D: { AD2_IHL; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x4E: { AD2_XIX; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x4F: { AD2_YIY; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;

		case 0x50: { m_HL = ( m_HL & 0xFF00 ) | ( m_BA & 0x00FF); }
			break;
		case 0x51: { m_HL = ( m_HL & 0xFF00 ) | ( m_BA >> 8 ); }
			break;
		case 0x52: { m_HL = ( m_HL & 0xFF00 ) | ( m_HL & 0x00FF); }
			break;
		case 0x53: { m_HL = ( m_HL & 0xFF00 ) | ( m_HL >> 8 ); }
			break;
		case 0x54: { AD2_IN8; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x55: { AD2_IHL; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x56: { AD2_XIX; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x57: { AD2_YIY; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
			break;
		case 0x58: { m_HL = ( m_HL & 0x00FF ) | ( ( m_BA & 0x00FF) << 8 ); }
			break;
		case 0x59: { m_HL = ( m_HL & 0x00FF ) | ( ( m_BA >> 8 ) << 8 ); }
			break;
		case 0x5A: { m_HL = ( m_HL & 0x00FF ) | ( ( m_HL & 0x00FF) << 8 ); }
			break;
		case 0x5B: { m_HL = ( m_HL & 0x00FF ) | ( ( m_HL >> 8 ) << 8 ); }
			break;
		case 0x5C: { AD2_IN8; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x5D: { AD2_IHL; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x5E: { AD2_XIX; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;
		case 0x5F: { AD2_YIY; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
			break;

		case 0x60: { AD1_XIX; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x61: { AD1_XIX; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x62: { AD1_XIX; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x63: { AD1_XIX; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x64: { AD1_XIX; AD2_IN8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x65: { AD1_XIX; AD2_IHL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x66: { AD1_XIX; AD2_XIX; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x67: { AD1_XIX; AD2_YIY; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x68: { AD1_IHL; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x69: { AD1_IHL; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x6A: { AD1_IHL; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x6B: { AD1_IHL; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x6C: { AD1_IHL; AD2_IN8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x6D: { AD1_IHL; AD2_IHL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x6E: { AD1_IHL; AD2_XIX; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x6F: { AD1_IHL; AD2_YIY; WR( addr1, RD( addr2 ) ); }
			break;

		case 0x70: { AD1_YIY; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x71: { AD1_YIY; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x72: { AD1_YIY; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x73: { AD1_YIY; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x74: { AD1_YIY; AD2_IN8; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x75: { AD1_YIY; AD2_IHL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x76: { AD1_YIY; AD2_XIX; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x77: { AD1_YIY; AD2_YIY; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x78: { AD1_IN8; WR( addr1, ( m_BA & 0x00FF ) ); }
			break;
		case 0x79: { AD1_IN8; WR( addr1, ( m_BA >> 8 ) ); }
			break;
		case 0x7A: { AD1_IN8; WR( addr1, ( m_HL & 0x00FF ) ); }
			break;
		case 0x7B: { AD1_IN8; WR( addr1, ( m_HL >> 8 ) ); }
			break;
		case 0x7C: { /* illegal operation? */ }
			break;
		case 0x7D: { AD1_IN8; AD2_IHL; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x7E: { AD1_IN8; AD2_XIX; WR( addr1, RD( addr2 ) ); }
			break;
		case 0x7F: { AD1_IN8; AD2_YIY; WR( addr1, RD( addr2 ) ); }
			break;

		case 0x80: { m_BA = ( m_BA & 0xFF00 ) | INC8( m_BA & 0x00FF ); }
			break;
		case 0x81: { m_BA = ( m_BA & 0x00FF ) | ( INC8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x82: { m_HL = ( m_HL & 0xFF00 ) | INC8( m_HL & 0x00FF ); }
			break;
		case 0x83: { m_HL = ( m_HL & 0x00FF ) | ( INC8( m_HL >> 8 ) << 8 ); }
			break;
		case 0x84: { m_N = INC8( m_N ); }
			break;
		case 0x85: { AD1_IN8; WR( addr1, INC8( RD( addr1 ) ) ); }
			break;
		case 0x86: { AD1_IHL; WR( addr1, INC8( RD( addr1 ) ) ); }
			break;
		case 0x87: { m_SP = INC16( m_SP ); }
			break;
		case 0x88: { m_BA = ( m_BA & 0xFF00 ) | DEC8( m_BA & 0x00FF ); }
			break;
		case 0x89: { m_BA = ( m_BA & 0x00FF ) | ( DEC8( m_BA >> 8 ) << 8 ); }
			break;
		case 0x8A: { m_HL = ( m_HL & 0xFF00 ) | DEC8( m_HL & 0x00FF ); }
			break;
		case 0x8B: { m_HL = ( m_HL & 0x00FF ) | ( DEC8( m_HL >> 8 ) << 8 ); }
			break;
		case 0x8C: { m_N = DEC8( m_N ); }
			break;
		case 0x8D: { AD1_IN8; WR( addr1, DEC8( RD( addr1 ) ) ); }
			break;
		case 0x8E: { AD1_IHL; WR( addr1, DEC8( RD( addr1 ) ) ); }
			break;
		case 0x8F: { m_SP = DEC8( m_SP ); }
			break;

		case 0x90: { m_BA = INC16( m_BA ); }
			break;
		case 0x91: { m_HL = INC16( m_HL ); }
			break;
		case 0x92: { m_X = INC16( m_X ); }
			break;
		case 0x93: { m_Y = INC16( m_Y ); }
			break;
		case 0x94: { m_F = ( AND8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z;}
			break;
		case 0x95: { AD1_IHL; m_F = ( AND8( RD( addr1 ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
			break;
		case 0x96: { m_F = ( AND8( ( m_BA & 0x00FF ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
			break;
		case 0x97: { m_F = ( AND8( ( m_BA >> 8 ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
			break;
		case 0x98: { m_BA = DEC16( m_BA ); }
			break;
		case 0x99: { m_HL = DEC16( m_HL ); }
			break;
		case 0x9A: { m_X = DEC16( m_X ); }
			break;
		case 0x9B: { m_Y = DEC16( m_Y ); }
			break;
		case 0x9C: { m_F = m_F & rdop(); }
			break;
		case 0x9D: { m_F = m_F | rdop(); }
			break;
		case 0x9E: { m_F = m_F ^ rdop(); }
			break;
		case 0x9F: { m_F = rdop(); }
			break;

		case 0xA0: { PUSH16( m_BA ); }
			break;
		case 0xA1: { PUSH16( m_HL ); }
			break;
		case 0xA2: { PUSH16( m_X ); }
			break;
		case 0xA3: { PUSH16( m_Y ); }
			break;
		case 0xA4: { PUSH8( m_N ); }
			break;
		case 0xA5: { PUSH8( m_I ); }
			break;
		case 0xA6: { PUSH8( m_XI ); PUSH8( m_YI ); }
			break;
		case 0xA7: { PUSH8( m_F ); }
			break;
		case 0xA8: { m_BA = POP16(); }
			break;
		case 0xA9: { m_HL = POP16();}
			break;
		case 0xAA: { m_X = POP16(); }
			break;
		case 0xAB: { m_Y = POP16(); }
			break;
		case 0xAC: { m_N = POP8(); }
			break;
		case 0xAD: { m_I = POP8(); }
			break;
		case 0xAE: { m_YI = POP8(); m_XI = POP8(); }
			break;
		case 0xAF: { m_F = POP8(); }
			break;

		case 0xB0: { UINT8 op = rdop(); m_BA = ( m_BA & 0xFF00 ) | op; }
			break;
		case 0xB1: { UINT8 op = rdop(); m_BA = ( m_BA & 0x00FF ) | ( op << 8 ); }
			break;
		case 0xB2: { UINT8 op = rdop(); m_HL = ( m_HL & 0xFF00 ) | op; }
			break;
		case 0xB3: { UINT8 op = rdop(); m_HL = ( m_HL & 0x00FF ) | ( op << 8 ); }
			break;
		case 0xB4: { UINT8 op = rdop(); m_N = op; }
			break;
		case 0xB5: { AD1_IHL; UINT8 op = rdop(); WR( addr1, op); }
			break;
		case 0xB6: { AD1_XIX; UINT8 op = rdop(); WR( addr1, op ); }
			break;
		case 0xB7: { AD1_YIY; UINT8 op = rdop(); WR( addr1, op ); }
			break;
		case 0xB8: { AD2_I16; m_BA = rd16( addr2 ); }
			break;
		case 0xB9: { AD2_I16; m_HL = rd16( addr2 ); }
			break;
		case 0xBA: { AD2_I16; m_X = rd16( addr2 ); }
			break;
		case 0xBB: { AD2_I16; m_Y = rd16( addr2 ); }
			break;
		case 0xBC: { AD1_I16; wr16( addr1, m_BA ); }
			break;
		case 0xBD: { AD1_I16; wr16( addr1, m_HL ); }
			break;
		case 0xBE: { AD1_I16; wr16( addr1, m_X ); }
			break;
		case 0xBF: { AD1_I16; wr16( addr1, m_Y ); }
			break;

		case 0xC0: { m_BA = ADD16( m_BA, rdop16() ); }
			break;
		case 0xC1: { m_HL = ADD16( m_HL, rdop16() ); }
			break;
		case 0xC2: { m_X = ADD16( m_X, rdop16() ); }
			break;
		case 0xC3: { m_Y = ADD16( m_Y, rdop16() ); }
			break;
		case 0xC4: { m_BA = rdop16(); }
			break;
		case 0xC5: { m_HL = rdop16(); }
			break;
		case 0xC6: { m_X = rdop16(); }
			break;
		case 0xC7: { m_Y = rdop16(); }
			break;
		case 0xC8: { UINT16 t = m_BA; m_BA = m_HL; m_HL = t; }
			break;
		case 0xC9: { UINT16 t = m_BA; m_BA = m_X; m_X = t; }
			break;
		case 0xCA: { UINT16 t = m_BA; m_BA = m_Y; m_Y = t; }
			break;
		case 0xCB: { UINT16 t = m_BA; m_BA = m_SP; m_SP = t; }
			break;
		case 0xCC: { m_BA = ( m_BA >> 8 ) | ( ( m_BA & 0x00FF ) << 8 ); }
			break;
		case 0xCD: { UINT8 t; AD2_IHL; t = RD( addr2 ); WR( addr2, ( m_BA & 0x00FF ) ); m_BA = ( m_BA & 0xFF00 ) | t; }
			break;
		case 0xCE: { execute_one_ce(); }
			break;
		case 0xCF: { execute_one_cf(); }
			break;

		case 0xD0: { m_BA = SUB16( m_BA, rdop16() ); }
			break;
		case 0xD1: { m_HL = SUB16( m_HL, rdop16() ); }
			break;
		case 0xD2: { m_X = SUB16( m_X, rdop16() ); }
			break;
		case 0xD3: { m_Y = SUB16( m_Y, rdop16() ); }
			break;
		case 0xD4: { SUB16( m_BA, rdop16() ); }
			break;
		case 0xD5: { SUB16( m_HL, rdop16() ); }
			break;
		case 0xD6: { SUB16( m_X, rdop16() ); }
			break;
		case 0xD7: { SUB16( m_Y, rdop16() ); }
			break;
		case 0xD8: { AD1_IN8; WR( addr1, AND8( RD( addr1 ), rdop() ) ); }
			break;
		case 0xD9: { AD1_IN8; WR( addr1, OR8( RD( addr1 ), rdop() ) ); }
			break;
		case 0xDA: { AD1_IN8; WR( addr1, XOR8( RD( addr1 ), rdop() ) ); }
			break;
		case 0xDB: { AD1_IN8; SUB8( RD( addr1 ), rdop() ); }
			break;
		case 0xDC: { AD1_IN8; m_F = ( AND8( RD( addr1 ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
			break;
		case 0xDD: { AD1_IN8; WR( addr1, rdop() ); }
			break;
		case 0xDE: { m_BA = ( m_BA & 0xFF00 ) | ( ( m_BA & 0x000F ) | ( ( m_BA & 0x0F00 ) >> 4 ) ); }
			break;
		case 0xDF: { m_BA = ( ( m_BA & 0x0080 ) ? 0xFF00 : 0x0000 ) | ( m_BA & 0x000F ); }
			break;

		case 0xE0: { INT8 d8 = rdop(); if ( m_F & FLAG_C ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xE1: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_C ) ) { CALL( m_PC + d8- 1  ); m_icount -= 12; } }
			break;
		case 0xE2: { INT8 d8 = rdop(); if ( m_F & FLAG_Z ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xE3: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_Z ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
			break;
		case 0xE4: { INT8 d8 = rdop(); if ( m_F & FLAG_C ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE5: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_C ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE6: { INT8 d8 = rdop(); if ( m_F & FLAG_Z ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE7: { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_Z ) ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xE8: { UINT16 d16 = rdop16(); if ( m_F & FLAG_C ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
			break;
		case 0xE9: { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_C ) ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
			break;
		case 0xEA: { UINT16 d16 = rdop16(); if ( m_F & FLAG_Z ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
			break;
		case 0xEB: { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_Z ) ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
			break;
		case 0xEC: { UINT16 d16 = rdop16(); if ( m_F & FLAG_C ) { JMP( m_PC + d16 - 1 ); } }
			break;
		case 0xED: { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_C ) ) { JMP( m_PC + d16 - 1 ); } }
			break;
		case 0xEE: { UINT16 d16 = rdop16(); if ( m_F & FLAG_Z ) { JMP( m_PC + d16 - 1 ); } }
			break;
		case 0xEF: { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_Z ) ) { JMP( m_PC + d16 - 1 ); } }
			break;

		case 0xF0: { INT8 d8 = rdop(); CALL( m_PC + d8 - 1 ); }
			break;
		case 0xF1: { INT8 d8 = rdop(); JMP( m_PC + d8 - 1 ); }
			break;
		case 0xF2: { UINT16 d16 = rdop16(); CALL( m_PC + d16 - 1 ); }
			break;
		case 0xF3: { UINT16 d16 = rdop16(); JMP( m_PC + d16 - 1 ); }
			break;
		case 0xF4: { JMP( m_HL ); }
			break;
		case 0xF5: { INT8 d8 = rdop(); m_BA = m_BA - 0x0100; if ( m_BA & 0xFF00 ) { JMP( m_PC + d8 - 1 ); } }
			break;
		case 0xF6: { m_BA = ( m_BA & 0xFF00 ) | ( ( m_BA & 0x00F0 ) >> 4 ) | ( ( m_BA & 0x000F ) << 4 ); }
			break;
		case 0xF7: { UINT8 d; AD1_IHL; d = RD( addr1 ); WR( addr1, ( ( d & 0xF0 ) >> 4 ) | ( ( d & 0x0F ) << 4 ) ); }
			break;
		case 0xF8: { m_PC = POP16(); m_V = POP8(); m_U = m_V; }
			break;
		case 0xF9: { m_F = POP8(); m_PC = POP16(); m_V = POP8(); m_U = m_V; }
			break;
		case 0xFA: { m_PC = POP16() + 2; m_V = POP8(); m_U = m_V; }
			break;
		case 0xFB: { AD1_I16; CALL( rd16( addr1 ) ); }
			break;
		case 0xFC: { UINT8 i = rdop() & 0xFE; CALL( rd16( i ) ); PUSH8( m_F ); }
			break;
		case 0xFD: { UINT8 i = rdop() & 0xFE; JMP( rd16( i ) ); /* PUSH8( m_F );?? */ }
			break;
		case 0xFE: { /* illegal operation? */ }
			break;
		case 0xFF: { }
			break;
	}

	m_icount -= insnminx_cycles[opcode];
}


const int minx_cpu_device::insnminx_cycles[256] = {
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,

	4,  4,  4,  4, 12,  8,  8,  8,  4,  4,  4,  4, 12,  8,  8,  8,
	4,  4,  4,  4, 12,  8,  8,  8,  4,  4,  4,  4, 12,  8,  8,  8,
	8,  8,  8,  8, 16, 12, 12, 12,  8,  8,  8,  8, 16, 12, 12, 12,
	8,  8,  8,  8, 16, 12, 12, 12, 12, 12, 12, 12,  1, 16, 16, 16,

	8,  8,  8,  8,  8, 16, 12,  8,  8,  8,  8,  8,  8, 16, 12,  8,
	8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12,
	16, 16, 16, 16, 12, 12, 16, 12, 12, 12, 12, 12,  8,  8, 12,  8,
	8,  8,  8,  8,  8, 12, 12, 12, 20, 20, 20, 20,  1,  1,  1,  1,

	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  0,  0,
	12, 12, 12, 12, 12, 12, 12, 12, 20, 20, 20, 16, 16, 16,  8,  8,
	8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	20,  8, 24, 12,  8,  1,  8, 12,  8,  8,  8, 20, 20,  1,  1,  8
};
