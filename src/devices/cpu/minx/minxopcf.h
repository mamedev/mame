// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

void minx_cpu_device::execute_one_cf()
{
	const UINT8 opcode = rdop();

	switch (opcode)
	{
		case 0x00: { m_BA = ADD16( m_BA, m_BA ); }
			break;
		case 0x01: { m_BA = ADD16( m_BA, m_HL ); }
			break;
		case 0x02: { m_BA = ADD16( m_BA, m_X ); }
			break;
		case 0x03: { m_BA = ADD16( m_BA, m_Y ); }
			break;
		case 0x04: { m_BA = ADDC16( m_BA, m_BA ); }
			break;
		case 0x05: { m_BA = ADDC16( m_BA, m_HL ); }
			break;
		case 0x06: { m_BA = ADDC16( m_BA, m_X ); }
			break;
		case 0x07: { m_BA = ADDC16( m_BA, m_Y ); }
			break;
		case 0x08: { m_BA = SUB16( m_BA, m_BA ); }
			break;
		case 0x09: { m_BA = SUB16( m_BA, m_HL ); }
			break;
		case 0x0A: { m_BA = SUB16( m_BA, m_X ); }
			break;
		case 0x0B: { m_BA = SUB16( m_BA, m_Y ); }
			break;
		case 0x0C: { m_BA = SUBC16( m_BA, m_BA ); }
			break;
		case 0x0D: { m_BA = SUBC16( m_BA, m_HL ); }
			break;
		case 0x0E: { m_BA = SUBC16( m_BA, m_X ); }
			break;
		case 0x0F: { m_BA = SUBC16( m_BA, m_Y ); }
			break;

		case 0x10: { /* illegal instruction? */ }
			break;
		case 0x11: { /* illegal instruction? */ }
			break;
		case 0x12: { /* illegal instruction? */ }
			break;
		case 0x13: { /* illegal instruction? */ }
			break;
		case 0x14: { /* illegal instruction? */ }
			break;
		case 0x15: { /* illegal instruction? */ }
			break;
		case 0x16: { /* illegal instruction? */ }
			break;
		case 0x17: { /* illegal instruction? */ }
			break;
		case 0x18: { SUB16( m_BA, m_BA ); }
			break;
		case 0x19: { SUB16( m_BA, m_HL ); }
			break;
		case 0x1A: { SUB16( m_BA, m_X ); }
			break;
		case 0x1B: { SUB16( m_BA, m_Y ); }
			break;
		case 0x1C: { /* illegal instruction? */ }
			break;
		case 0x1D: { /* illegal instruction? */ }
			break;
		case 0x1E: { /* illegal instruction? */ }
			break;
		case 0x1F: { /* illegal instruction? */ }
			break;

		case 0x20: { m_HL = ADD16( m_HL, m_BA ); }
			break;
		case 0x21: { m_HL = ADD16( m_HL, m_HL ); }
			break;
		case 0x22: { m_HL = ADD16( m_HL, m_X ); }
			break;
		case 0x23: { m_HL = ADD16( m_HL, m_Y ); }
			break;
		case 0x24: { m_HL = ADDC16( m_HL, m_BA ); }
			break;
		case 0x25: { m_HL = ADDC16( m_HL, m_HL ); }
			break;
		case 0x26: { m_HL = ADDC16( m_HL, m_X ); }
			break;
		case 0x27: { m_HL = ADDC16( m_HL, m_Y ); }
			break;
		case 0x28: { m_HL = SUB16( m_HL, m_BA ); }
			break;
		case 0x29: { m_HL = SUB16( m_HL, m_HL ); }
			break;
		case 0x2A: { m_HL = SUB16( m_HL, m_X ); }
			break;
		case 0x2B: { m_HL = SUB16( m_HL, m_Y ); }
			break;
		case 0x2C: { m_HL = SUBC16( m_HL, m_BA ); }
			break;
		case 0x2D: { m_HL = SUBC16( m_HL, m_HL ); }
			break;
		case 0x2E: { m_HL = SUBC16( m_HL, m_X ); }
			break;
		case 0x2F: { m_HL = SUBC16( m_HL, m_Y ); }
			break;

		case 0x30: { /* illegal instruction? */ }
			break;
		case 0x31: { /* illegal instruction? */ }
			break;
		case 0x32: { /* illegal instruction? */ }
			break;
		case 0x33: { /* illegal instruction? */ }
			break;
		case 0x34: { /* illegal instruction? */ }
			break;
		case 0x35: { /* illegal instruction? */ }
			break;
		case 0x36: { /* illegal instruction? */ }
			break;
		case 0x37: { /* illegal instruction? */ }
			break;
		case 0x38: { SUB16( m_HL, m_BA ); }
			break;
		case 0x39: { SUB16( m_HL, m_HL ); }
			break;
		case 0x3A: { SUB16( m_HL, m_X ); }
			break;
		case 0x3B: { SUB16( m_HL, m_Y ); }
			break;
		case 0x3C: { /* illegal instruction? */ }
			break;
		case 0x3D: { /* illegal instruction? */ }
			break;
		case 0x3E: { /* illegal instruction? */ }
			break;
		case 0x3F: { /* illegal instruction? */ }
			break;

		case 0x40: { m_X = ADD16( m_X, m_BA ); }
			break;
		case 0x41: { m_X = ADD16( m_X, m_HL ); }
			break;
		case 0x42: { m_Y = ADD16( m_Y, m_BA ); }
			break;
		case 0x43: { m_Y = ADD16( m_Y, m_HL ); }
			break;
		case 0x44: { m_SP = ADD16( m_SP, m_BA ); }
			break;
		case 0x45: { m_SP = ADD16( m_SP, m_HL ); }
			break;
		case 0x46: { /* illegal instruction? */ }
			break;
		case 0x47: { /* illegal instruction? */ }
			break;
		case 0x48: { m_X = SUB16( m_X, m_BA ); }
			break;
		case 0x49: { m_X = SUB16( m_X, m_HL ); }
			break;
		case 0x4A: { m_Y = SUB16( m_Y, m_BA ); }
			break;
		case 0x4B: { m_Y = SUB16( m_Y, m_HL ); }
			break;
		case 0x4C: { m_SP = SUB16( m_SP, m_BA ); }
			break;
		case 0x4D: { m_SP = SUB16( m_SP, m_HL ); }
			break;
		case 0x4E: { /* illegal instruction? */ }
			break;
		case 0x4F: { /* illegal instruction? */ }
			break;

		case 0x50: { /* illegal instruction? */ }
			break;
		case 0x51: { /* illegal instruction? */ }
			break;
		case 0x52: { /* illegal instruction? */ }
			break;
		case 0x53: { /* illegal instruction? */ }
			break;
		case 0x54: { /* illegal instruction? */ }
			break;
		case 0x55: { /* illegal instruction? */ }
			break;
		case 0x56: { /* illegal instruction? */ }
			break;
		case 0x57: { /* illegal instruction? */ }
			break;
		case 0x58: { /* illegal instruction? */ }
			break;
		case 0x59: { /* illegal instruction? */ }
			break;
		case 0x5A: { /* illegal instruction? */ }
			break;
		case 0x5B: { /* illegal instruction? */ }
			break;
		case 0x5C: { SUB16( m_SP, m_BA ); }
			break;
		case 0x5D: { SUB16( m_SP, m_HL ); }
			break;
		case 0x5E: { /* illegal instruction? */ }
			break;
		case 0x5F: { /* illegal instruction? */ }
			break;

		case 0x60: { ADDC16( m_BA, rdop16() ); /* ??? */ }
			break;
		case 0x61: { ADDC16( m_HL, rdop16() ); /* ??? */ }
			break;
		case 0x62: { ADDC16( m_X, rdop16() ); /* ??? */ }
			break;
		case 0x63: { ADDC16( m_Y, rdop16() ); /* ??? */ }
			break;
		case 0x64: { /* illegal instruction? */ }
			break;
		case 0x65: { /* illegal instruction? */ }
			break;
		case 0x66: { /* illegal instruction? */ }
			break;
		case 0x67: { /* illegal instruction? */ }
			break;
		case 0x68: { m_SP = ADD16( m_SP, rdop16() ); }
			break;
		case 0x69: { /* illegal instruction? */ }
			break;
		case 0x6A: { m_SP = SUB16( m_SP, rdop16() ); }
			break;
		case 0x6B: { /* illegal instruction? */ }
			break;
		case 0x6C: { SUB16( m_SP, rdop16() ); }
			break;
		case 0x6D: { /* illegal instruction? */ }
			break;
		case 0x6E: { m_SP = rdop16(); }
			break;
		case 0x6F: { /* illegal instruction? */ }
			break;

		case 0x70: { UINT8 ofs8 = rdop(); m_BA = rd16( m_SP + ofs8 ); }
			break;
		case 0x71: { UINT8 ofs8 = rdop(); m_HL = rd16( m_SP + ofs8 ); }
			break;
		case 0x72: { UINT8 ofs8 = rdop(); m_X = rd16( m_SP + ofs8 ); }
			break;
		case 0x73: { UINT8 ofs8 = rdop(); m_Y = rd16( m_SP + ofs8 ); }
			break;
		case 0x74: { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_BA ); }
			break;
		case 0x75: { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_HL ); }
			break;
		case 0x76: { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_X ); }
			break;
		case 0x77: { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_Y ); }
			break;
		case 0x78: { AD2_I16; m_SP = rd16( addr2 ); }
			break;
		case 0x79: { /* illegal instruction? */ }
			break;
		case 0x7A: { /* illegal instruction? */ }
			break;
		case 0x7B: { /* illegal instruction? */ }
			break;
		case 0x7C: { AD1_I16; wr16( addr1, m_SP ); }
			break;
		case 0x7D: { /* illegal instruction? */ }
			break;
		case 0x7E: { /* illegal instruction? */ }
			break;
		case 0x7F: { /* illegal instruction? */ }
			break;

		case 0x80: { /* illegal instruction? */ }
			break;
		case 0x81: { /* illegal instruction? */ }
			break;
		case 0x82: { /* illegal instruction? */ }
			break;
		case 0x83: { /* illegal instruction? */ }
			break;
		case 0x84: { /* illegal instruction? */ }
			break;
		case 0x85: { /* illegal instruction? */ }
			break;
		case 0x86: { /* illegal instruction? */ }
			break;
		case 0x87: { /* illegal instruction? */ }
			break;
		case 0x88: { /* illegal instruction? */ }
			break;
		case 0x89: { /* illegal instruction? */ }
			break;
		case 0x8A: { /* illegal instruction? */ }
			break;
		case 0x8B: { /* illegal instruction? */ }
			break;
		case 0x8C: { /* illegal instruction? */ }
			break;
		case 0x8D: { /* illegal instruction? */ }
			break;
		case 0x8E: { /* illegal instruction? */ }
			break;
		case 0x8F: { /* illegal instruction? */ }
			break;

		case 0x90: { /* illegal instruction? */ }
			break;
		case 0x91: { /* illegal instruction? */ }
			break;
		case 0x92: { /* illegal instruction? */ }
			break;
		case 0x93: { /* illegal instruction? */ }
			break;
		case 0x94: { /* illegal instruction? */ }
			break;
		case 0x95: { /* illegal instruction? */ }
			break;
		case 0x96: { /* illegal instruction? */ }
			break;
		case 0x97: { /* illegal instruction? */ }
			break;
		case 0x98: { /* illegal instruction? */ }
			break;
		case 0x99: { /* illegal instruction? */ }
			break;
		case 0x9A: { /* illegal instruction? */ }
			break;
		case 0x9B: { /* illegal instruction? */ }
			break;
		case 0x9C: { /* illegal instruction? */ }
			break;
		case 0x9D: { /* illegal instruction? */ }
			break;
		case 0x9E: { /* illegal instruction? */ }
			break;
		case 0x9F: { /* illegal instruction? */ }
			break;

		case 0xA0: { /* illegal instruction? */ }
			break;
		case 0xA1: { /* illegal instruction? */ }
			break;
		case 0xA2: { /* illegal instruction? */ }
			break;
		case 0xA3: { /* illegal instruction? */ }
			break;
		case 0xA4: { /* illegal instruction? */ }
			break;
		case 0xA5: { /* illegal instruction? */ }
			break;
		case 0xA6: { /* illegal instruction? */ }
			break;
		case 0xA7: { /* illegal instruction? */ }
			break;
		case 0xA8: { /* illegal instruction? */ }
			break;
		case 0xA9: { /* illegal instruction? */ }
			break;
		case 0xAA: { /* illegal instruction? */ }
			break;
		case 0xAB: { /* illegal instruction? */ }
			break;
		case 0xAC: { /* illegal instruction? */ }
			break;
		case 0xAD: { /* illegal instruction? */ }
			break;
		case 0xAE: { /* illegal instruction? */ }
			break;
		case 0xAF: { /* illegal instruction? */ }
			break;

		case 0xB0: { PUSH8( m_BA & 0x00FF ); }
			break;
		case 0xB1: { PUSH8( m_BA >> 8 ); }
			break;
		case 0xB2: { PUSH8( m_HL & 0x00FF ); }
			break;
		case 0xB3: { PUSH8( m_HL >> 8 ); }
			break;
		case 0xB4: { m_BA = ( m_BA & 0xFF00 ) | POP8(); }
			break;
		case 0xB5: { m_BA = ( m_BA & 0x00FF ) | ( POP8() << 8 ); }
			break;
		case 0xB6: { m_HL = ( m_HL & 0xFF00 ) | POP8(); }
			break;
		case 0xB7: { m_HL = ( m_HL & 0x00FF ) | ( POP8() << 8 ); }
			break;
		case 0xB8: { PUSH16( m_BA ); PUSH16( m_HL ); PUSH16( m_X ); PUSH16( m_Y ); PUSH8( m_N ); }
			break;
		case 0xB9: { PUSH16( m_BA ); PUSH16( m_HL ); PUSH16( m_X ); PUSH16( m_Y ); PUSH8( m_N ); PUSH8( m_I ); PUSH8( m_XI ); PUSH8( m_YI ); }
			break;
		case 0xBA: { /* illegal instruction? */ }
			break;
		case 0xBB: { /* illegal instruction? */ }
			break;
		case 0xBC: { m_N = POP8(); m_Y = POP16(); m_X = POP16(); m_HL = POP16(); m_BA = POP16(); }
			break;
		case 0xBD: { m_YI = POP8(); m_XI = POP8(); m_I = POP8(); m_N = POP8(); m_Y = POP16(); m_X = POP16(); m_HL = POP16(); m_BA = POP16(); }
			break;
		case 0xBE: { /* illegal instruction? */ }
			break;
		case 0xBF: { /* illegal instruction? */ }
			break;

		case 0xC0: { AD2_IHL; m_BA = rd16( addr2 ); }
			break;
		case 0xC1: { AD2_IHL; m_HL = rd16( addr2 ); }
			break;
		case 0xC2: { AD2_IHL; m_X = rd16( addr2 ); }
			break;
		case 0xC3: { AD2_IHL; m_Y = rd16( addr2 ); }
			break;
		case 0xC4: { AD1_IHL; wr16( addr1, m_BA ); }
			break;
		case 0xC5: { AD1_IHL; wr16( addr1, m_HL ); }
			break;
		case 0xC6: { AD1_IHL; wr16( addr1, m_X ); }
			break;
		case 0xC7: { AD1_IHL; wr16( addr1, m_Y ); }
			break;
		case 0xC8: { /* illegal instruction? */ }
			break;
		case 0xC9: { /* illegal instruction? */ }
			break;
		case 0xCA: { /* illegal instruction? */ }
			break;
		case 0xCB: { /* illegal instruction? */ }
			break;
		case 0xCC: { /* illegal instruction? */ }
			break;
		case 0xCD: { /* illegal instruction? */ }
			break;
		case 0xCE: { /* illegal instruction? */ }
			break;
		case 0xCF: { /* illegal instruction? */ }
			break;

		case 0xD0: { AD2_XIX; m_BA = rd16( addr2 ); }
			break;
		case 0xD1: { AD2_XIX; m_HL = rd16( addr2 ); }
			break;
		case 0xD2: { AD2_XIX; m_X = rd16( addr2 ); }
			break;
		case 0xD3: { AD2_XIX; m_Y = rd16( addr2 ); }
			break;
		case 0xD4: { AD1_XIX; wr16( addr1, m_BA ); }
			break;
		case 0xD5: { AD1_XIX; wr16( addr1, m_HL ); }
			break;
		case 0xD6: { AD1_XIX; wr16( addr1, m_X ); }
			break;
		case 0xD7: { AD1_XIX; wr16( addr1, m_Y ); }
			break;
		case 0xD8: { AD2_YIY; m_BA = rd16( addr2 ); }
			break;
		case 0xD9: { AD2_YIY; m_HL = rd16( addr2 ); }
			break;
		case 0xDA: { AD2_YIY; m_X = rd16( addr2 ); }
			break;
		case 0xDB: { AD2_YIY; m_Y = rd16( addr2 ); }
			break;
		case 0xDC: { AD1_YIY; wr16( addr1, m_BA ); }
			break;
		case 0xDD: { AD1_YIY; wr16( addr1, m_HL ); }
			break;
		case 0xDE: { AD1_YIY; wr16( addr1, m_X ); }
			break;
		case 0xDF: { AD1_YIY; wr16( addr1, m_Y ); }
			break;

		case 0xE0: { } //{ m_BA = m_BA; }
			break;
		case 0xE1: { m_BA = m_HL; }
			break;
		case 0xE2: { m_BA = m_X; }
			break;
		case 0xE3: { m_BA = m_Y; }
			break;
		case 0xE4: { m_HL = m_BA; }
			break;
		case 0xE5: { } //{ m_HL = m_HL; }
			break;
		case 0xE6: { m_HL = m_X; }
			break;
		case 0xE7: { m_HL = m_Y; }
			break;
		case 0xE8: { m_X = m_BA; }
			break;
		case 0xE9: { m_X = m_HL; }
			break;
		case 0xEA: { } //{ m_X = m_X; }
			break;
		case 0xEB: { m_X = m_Y; }
			break;
		case 0xEC: { m_Y = m_BA; }
			break;
		case 0xED: { m_Y = m_HL; }
			break;
		case 0xEE: { m_Y = m_X; }
			break;
		case 0xEF: { } //{ m_Y = m_Y; }
			break;

		case 0xF0: { m_SP = m_BA; }
			break;
		case 0xF1: { m_SP = m_HL; }
			break;
		case 0xF2: { m_SP = m_X; }
			break;
		case 0xF3: { m_SP = m_Y; }
			break;
		case 0xF4: { m_HL = m_SP; }
			break;
		case 0xF5: { m_HL = m_PC; }
			break;
		case 0xF6: { /* illegal instruction? */ }
			break;
		case 0xF7: { /* illegal instruction? */ }
			break;
		case 0xF8: { m_BA = m_SP; }
			break;
		case 0xF9: { m_BA = m_PC; }
			break;
		case 0xFA: { m_X = m_SP; }
			break;
		case 0xFB: { /* illegal instruction? */ }
			break;
		case 0xFC: { /* illegal instruction? */ }
			break;
		case 0xFD: { /* illegal instruction? */ }
			break;
		case 0xFE: { m_Y = m_SP; }
			break;
		case 0xFF: { /* illegal instruction? */ }
			break;
	}

	m_icount -= insnminx_cycles_CF[opcode];
}


const int minx_cpu_device::insnminx_cycles_CF[256] = {
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	1,  1,  1,  1,  1,  1,  1,  1, 16, 16, 16, 16,  1,  1,  1,  1,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	1,  1,  1,  1,  1,  1,  1,  1, 16, 16, 16, 16,  1,  1,  1,  1,

	16, 16, 16, 16, 16, 16,  1,  1, 16, 16, 16, 16, 16, 16,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 16, 16,  1,  1,
	16, 16, 16, 16,  1,  1,  1,  1, 16,  1, 16,  1, 16,  1, 16,  1,
	24, 24, 24, 24, 24, 24, 24, 24, 24,  1,  1,  1, 24,  1,  1,  1,

	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	12, 12, 12, 12, 12, 12, 12, 12, 48, 60,  1,  1, 32, 40,  1,  1,

	20, 20, 20, 20, 20, 20, 20, 20,  1,  1,  1,  1,  1,  1,  1,  1,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	8,  8,  8,  8,  8,  8,  1,  1,  8,  8,  8,  1,  1,  1,  8,  1
};
