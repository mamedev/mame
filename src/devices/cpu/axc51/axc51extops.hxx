// license:BSD-3-Clause
// copyright-holders:David Haywood

void axc51base_cpu_device::do_ez_flags(uint16_t val)
{
	if (!val)
		SET_EZ(1);
	else
		SET_EZ(0);
}

void axc51base_cpu_device::do_ec_ez_flags(uint32_t res)
{
	if (res & 0xffff0000)
		SET_EC(1);
	else
		SET_EC(0);

	res &= 0xffff;

	if (!res)
		SET_EZ(1);
	else
		SET_EZ(0);
}

uint16_t axc51base_cpu_device::get_erx(int m)
{
	switch (m & 3)
	{
	case 0x00: return (ER0);
	case 0x01: return (ER1);
	case 0x02: return (ER2);
	case 0x03: return (ER3);
	}
	return 0;
}

void axc51base_cpu_device::set_erx(int n, uint16_t val)
{
	switch (n & 3)
	{
	case 0x00: SET_ER0(val); break;
	case 0x01: SET_ER1(val); break;
	case 0x02: SET_ER2(val); break;
	case 0x03: SET_ER3(val); break;
	}
}

uint16_t axc51base_cpu_device::get_dpt(int i)
{
	switch (i & 1)
	{
	case 0x00: return DPTR0; break;
	case 0x01: return DPTR1; break;
	}
	return 0x0000;
}

void axc51base_cpu_device::axc51_extended_a5(uint8_t r)
{
	uint8_t prm = m_program.read_byte(m_pc++);

	switch (prm)
	{
	case 0x00:
	{
		// INCDP0
		uint16_t dptr = (DPTR0)+1;
		SET_DPTR0(dptr);
		break;
	}

	case 0x01:
	{
		// INCDP1
		uint16_t dptr = (DPTR1)+1;
		SET_DPTR1(dptr);
		break;
	}

	case 0x02:
	{
		// DECDP0
		uint16_t dptr = (DPTR0)-1;
		SET_DPTR0(dptr);
		break;
	}

	case 0x03:
	{
		// DECDP1
		uint16_t dptr = (DPTR1)-1;
		SET_DPTR1(dptr);
		break;
	}

	case 0x04:
	{
		// ADDDP0
		uint16_t increment = (B) | ((ER8) << 8);
		uint16_t dptr = (DPTR0)+increment;
		SET_DPTR0(dptr);

		break;
	}

	case 0x05:
		fatalerror("%s: ADDDP1", machine().describe_context());
		break;

	case 0x06:
		fatalerror("%s: SUBDP0", machine().describe_context());
		break;

	case 0x07:
		fatalerror("%s: SUBDP1", machine().describe_context());
		break;

	case 0x08:
	{
		// INC2DP0
		uint16_t dptr = (DPTR0)+2;
		SET_DPTR0(dptr);
		break;
	}

	case 0x09:
		fatalerror("%s: INC2DP1", machine().describe_context());
		break;

	case 0x0a:
	{
		// DEC2DP0
		uint16_t dptr = (DPTR0)-2;
		SET_DPTR0(dptr);
		break;
	}

	case 0x0b:
		fatalerror("%s: DEC2DP1", machine().describe_context());
		break;

	case 0x0c:
	{
		// ROTR8 EACC, ER8
		uint8_t acc = ACC;
		uint8_t shift = (ER8) & 0x7;
		uint8_t newacc = (acc >> shift) | (acc << (8 - shift));
		SET_ACC(newacc);

		break;
	}

	case 0x0d:
	{
		// ROTL8 EACC, ER8
		uint8_t acc = ACC;
		uint8_t shift = (ER8) & 0x7;
		uint8_t newacc = (acc << shift) | (acc >> (8 - shift));
		SET_ACC(newacc);

		break;
	}

	case 0x0e: // ADD16
		extended_a5_0e();
		break;

	case 0x0f: // SUB16
		extended_a5_0f();
		break;

	case 0x10: case 0x14: case 0x18: case 0x1c:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		fatalerror("%s: NOT16 ER%01x", machine().describe_context(), n);
		break;
	}

	case 0x11: case 0x15: case 0x19: case 0x1d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		fatalerror("%s: CLR16 ER%01x", machine().describe_context(), n);
		break;
	}

	case 0x12: case 0x16: case 0x1a: case 0x1e:
	{
		// INC16 ERn
		uint8_t n = (prm & 0x0c) >> 2;

		uint16_t val = get_erx(n);
		val = val + 1;
		set_erx(n, val);

		do_ez_flags(val);

		break;
	}

	case 0x13: case 0x17: case 0x1b: case 0x1f:
	{
		// DEC16 ERn
		uint8_t n = (prm & 0x0c) >> 2;

		uint16_t val = get_erx(n);
		val = val - 1;
		set_erx(n, val);

		do_ez_flags(val);

		break;
	}

	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		fatalerror("%s: ANL16 ER%01x, EDP%01x", machine().describe_context(), n, i);
		break;
	}

	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		fatalerror("%s: ANL16 EDP%01x, ER%01x", machine().describe_context(), i, n);
		break;
	}

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		fatalerror("%s: ANL16 ER%01x, ER%01x", machine().describe_context(), n, m);
		break;
	}

	case 0x40: case 0x41: case 0x44: case 0x45: case 0x48: case 0x49: case 0x4c: case 0x4d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		fatalerror("%s: ORL16 ER%01x, EDP%01x", machine().describe_context(), n, i);
		break;
	}

	case 0x42: case 0x43: case 0x46: case 0x47: case 0x4a: case 0x4b: case 0x4e: case 0x4f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		fatalerror("%s: ORL16 EDP%01x, ER%01x", machine().describe_context(), i, n);
		break;
	}

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		fatalerror("%s: ORL16 ER%01x, ER%01x", machine().describe_context(), n, m);
		break;
	}

	case 0x60: case 0x61: case 0x64: case 0x65: case 0x68: case 0x69: case 0x6c: case 0x6d:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		fatalerror("%s: XRL16 ER%01x, EDP%01x", machine().describe_context(), n, i);
		break;
	}

	case 0x62: case 0x63: case 0x66: case 0x67: case 0x6a: case 0x6b: case 0x6e: case 0x6f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		fatalerror("%s: XRL16 EDP%01x, ER%01x", machine().describe_context(), i, n);
		break;
	}

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		fatalerror("%s: XRL16 ER%01x, ER%01x", machine().describe_context(), n, m);
		break;
	}

	case 0x80: case 0x81: case 0x84: case 0x85: case 0x88: case 0x89: case 0x8c: case 0x8d:
	{
		// MOV16 ERn, DPTRi

		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		uint16_t dpt = get_dpt(i);
		uint16_t val = m_io.read_word(dpt);

		set_erx(n, val);

		do_ez_flags(val);

		break;
	}

	case 0x82: case 0x83: case 0x86: case 0x87: case 0x8a: case 0x8b: case 0x8e: case 0x8f:
	{
		// MOV16 DPTRi, ERn
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t i = (prm & 0x01) >> 0;

		uint16_t dpt = get_dpt(i);
		uint16_t val = get_erx(n);

		m_io.write_word(dpt, val);  

		do_ez_flags(val);

		break;
	}

	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	{
		// MOV16 ERn, ERm
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		uint16_t val = get_erx(m);
		set_erx(n, val);

		do_ez_flags(val);

		break;
	}

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	{
		// MUL16 ERn, ERm;

		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		int16_t a = (int16_t)get_erx(n);
		int16_t b = (int16_t)get_erx(m);

		int32_t res = a * b;
		uint32_t res2 = (uint32_t)res;

		set_erx(n, (res2 >> 16));
		set_erx(m, (res2 & 0xffff));

		break;
	}

	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		uint8_t m = (prm & 0x03) >> 0;

		fatalerror("%s: MULS16 ER%01x, ER%01x", machine().describe_context(), n, m);
		break;
	}

	case 0xc0: case 0xc4: case 0xc8: case 0xcc:
	{
		// ROTR16 ERn, ER8
		uint8_t n = (prm & 0x0c) >> 2;
		uint16_t val = get_erx(n);
		uint8_t shift = (ER8) & 0xf;
		uint16_t newval = (val >> shift) | (val << (16 - shift));
		set_erx(n, newval);

		break;
	}

	case 0xc1: case 0xc5: case 0xc9: case 0xcd:
	{
		// ROTL16 ERn, ER8
		uint8_t n = (prm & 0x0c) >> 2;
		uint16_t val = get_erx(n);
		uint8_t shift = (ER8) & 0xf;
		uint16_t newval = (val << shift) | (val >> (16 - shift));
		set_erx(n, newval);

		break;
	}

	case 0xc2: case 0xc6: case 0xca: case 0xce:
	{
		// SHIFTL ERn, ER8
		uint8_t n = (prm & 0x0c) >> 2;

		uint16_t val = get_erx(n);
		uint8_t shift = (ER8) & 0xf;

		uint16_t newval = val >> shift;
		set_erx(n, newval);

		break;
	}

	case 0xc3: case 0xc7: case 0xcb: case 0xcf:
	{
		uint8_t n = (prm & 0x0c) >> 2;
		fatalerror("%s: SHIFTA ER%01x, ER8", machine().describe_context(), n);
		break;
	}

	case 0xd0: // ADDS16
		extended_a5_d0();
		break;

	case 0xd1: // SUBS16
		extended_a5_d1();
		break;

	case 0xd2: case 0xd6: case 0xda: case 0xde:
	{
		// SWAP16 ERn
		uint8_t n = (prm & 0x0c) >> 2;
		uint16_t val = get_erx(n);
		uint16_t newval = (val << 8) | (val >> 8);
		set_erx(n, newval);
		break;
	}

	case 0xd3: case 0xd4: case 0xd5: case 0xd7: case 0xd8: case 0xd9: case 0xdb: case 0xdc: case 0xdd: case 0xdf:
		fatalerror("%s: invalid ax208 a5 $%02X", machine().describe_context(), prm);
		break;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		fatalerror("%s: invalid ax208 a5 $%02X", machine().describe_context(), prm);
		break;

	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		fatalerror("%s: invalid ax208 a5 $%02X", machine().describe_context(), prm);
		break;

	default:
		fatalerror("%s: unknown ax208 a5 $%02X", machine().describe_context(), prm);
		break;
	}
}


void axc51base_cpu_device::extended_a5_0e()
{
	uint8_t prm2 = m_program.read_byte(m_pc++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		fatalerror("%s: ADD16 ER%01x, EDP%01x, ER%01x", machine().describe_context(), p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		fatalerror("%s: ADD16 EDP%01x, ER%01x, ER%01x", machine().describe_context(), i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		// ADD16 ERp, ERn, ERm

		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;

		uint16_t val = get_erx(n);
		uint16_t val2 = get_erx(m);
		uint32_t res = val + val2 + (GET_EC);
		set_erx(p, res);

		do_ec_ez_flags(res);

		break;
	}

	default:
		fatalerror("%s: illegal ax208 a5 0e $%02X", machine().describe_context(), prm2);
		break;
	}
}

void axc51base_cpu_device::extended_a5_0f()
{
	uint8_t prm2 = m_program.read_byte(m_pc++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		// SUB16 ERp, EDPi, ERn

		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		uint16_t dpt = get_dpt(i);
		uint16_t val = m_io.read_word(dpt);
		uint16_t val2 = get_erx(n);

		uint32_t res = val - val2 - (GET_EC);
		set_erx(p, res);

		do_ec_ez_flags(res);

		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		// SUB16 EDPi, ERn, ERp;
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		uint16_t val = get_erx(n);
		uint16_t val2 = get_erx(p);
		uint16_t dpt = get_dpt(i);
		uint32_t res = val - val2 - (GET_EC);

		m_io.write_word(dpt, res);  

		do_ec_ez_flags(res);

		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		// SUB16 ERp, ERn, ERm

		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;

		uint16_t val = get_erx(n);
		uint16_t val2 = get_erx(m);
		uint32_t res = val - val2 - (GET_EC);
		set_erx(p, res);

		do_ec_ez_flags(res);

		break;
	}


	default:
		fatalerror("%s: illegal ax208 a5 0f $%02X", machine().describe_context(), prm2);
		break;
	}
}

void axc51base_cpu_device::extended_a5_d0()
{
	uint8_t prm2 = m_program.read_byte(m_pc++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		fatalerror("%s: ADDS16 ER%01x, EDP%01x, ER%01x", machine().describe_context(), p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		fatalerror("%s: ADDS16 EDP%01x, ER%01x, ER%01x", machine().describe_context(), i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		fatalerror("%s: ADDS16 ER%01x, ER%01x, ER%01x", machine().describe_context(), p, n, m);
		break;
	}

	default:
		fatalerror("%s: illegal ax208 a5 d0 $%02X", machine().describe_context(), prm2);
		break;
	}
}

void axc51base_cpu_device::extended_a5_d1()
{
	uint8_t prm2 = m_program.read_byte(m_pc++);

	switch (prm2)
	{

	case 0x00: case 0x01: case 0x04: case 0x05: case 0x08: case 0x09: case 0x0c: case 0x0d:
	case 0x10: case 0x11: case 0x14: case 0x15: case 0x18: case 0x19: case 0x1c: case 0x1d:
	case 0x20: case 0x21: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2c: case 0x2d:
	case 0x30: case 0x31: case 0x34: case 0x35: case 0x38: case 0x39: case 0x3c: case 0x3d:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		fatalerror("%s: SUBS16 ER%01x, EDP%01x, ER%01x", machine().describe_context(), p, i, n);
		break;
	}

	case 0x02: case 0x03: case 0x06: case 0x07: case 0x0a: case 0x0b: case 0x0e: case 0x0f:
	case 0x12: case 0x13: case 0x16: case 0x17: case 0x1a: case 0x1b: case 0x1e: case 0x1f:
	case 0x22: case 0x23: case 0x26: case 0x27: case 0x2a: case 0x2b: case 0x2e: case 0x2f:
	case 0x32: case 0x33: case 0x36: case 0x37: case 0x3a: case 0x3b: case 0x3e: case 0x3f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t i = (prm2 & 0x01) >> 0;

		fatalerror("%s: SUBS16 EDP%01x, ER%01x, ER%01x", machine().describe_context(), i, n, p);
		break;
	}

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	{
		uint8_t p = (prm2 & 0x30) >> 4;
		uint8_t n = (prm2 & 0x0c) >> 2;
		uint8_t m = (prm2 & 0x03) >> 0;
		fatalerror("%s: SUBS16 ER%01x, ER%01x, ER%01x", machine().describe_context(), p, n, m);
		break;
	}

	default:
		fatalerror("%s: illegal ax208 a5 d1 $%02X", machine().describe_context(), prm2);
		break;
	}
}
