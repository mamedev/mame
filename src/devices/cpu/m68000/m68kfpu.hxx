// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

static constexpr int FPCC_N          = 0x08000000;
static constexpr int FPCC_Z          = 0x04000000;
static constexpr int FPCC_I          = 0x02000000;
static constexpr int FPCC_NAN        = 0x01000000;

static constexpr int FPES_OE         = 0x00002000;
static constexpr int FPAE_IOP        = 0x00000080;

static constexpr u64 DOUBLE_INFINITY                 = 0x7ff0000000000000U;
static constexpr u64 DOUBLE_EXPONENT                 = 0x7ff0000000000000U;
static constexpr u64 DOUBLE_MANTISSA                 = 0x000fffffffffffffU;

// masks for packed dwords, positive k-factor
const u32 pkmask2[18] =
{
	0xffffffff, 0, 0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff
};

const u32 pkmask3[18] =
{
	0xffffffff, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
};

static inline double fx80_to_double(floatx80 fx)
{
	u64 d;
	double *foo;

	foo = (double *)&d;

	d = floatx80_to_float64(fx);

	return *foo;
}

static inline floatx80 double_to_fx80(double in)
{
	u64 *d;

	d = (u64 *)&in;

	return float64_to_floatx80(*d);
}

inline floatx80 load_extended_float80(u32 ea)
{
	u32 d1,d2;
	u16 d3;
	floatx80 fp;

	d3 = m68ki_read_16(ea);
	d1 = m68ki_read_32(ea+4);
	d2 = m68ki_read_32(ea+8);

	fp.high = d3;
	fp.low = ((u64)d1<<32) | (d2 & 0xffffffff);

	return fp;
}

inline void store_extended_float80(u32 ea, floatx80 fpr)
{
	m68ki_write_16(ea+0, fpr.high);
	m68ki_write_16(ea+2, 0);
	m68ki_write_32(ea+4, (fpr.low>>32)&0xffffffff);
	m68ki_write_32(ea+8, fpr.low&0xffffffff);
}

inline floatx80 load_pack_float80(u32 ea)
{
	u32 dw1, dw2, dw3;
	floatx80 result;
	double tmp;
	char str[128], *ch;

	dw1 = m68ki_read_32(ea);
	dw2 = m68ki_read_32(ea+4);
	dw3 = m68ki_read_32(ea+8);

	ch = &str[0];
	if (dw1 & 0x80000000)   // mantissa sign
	{
		*ch++ = '-';
	}
	*ch++ = (char)((dw1 & 0xf) + '0');
	*ch++ = '.';
	*ch++ = (char)(((dw2 >> 28) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 24) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 20) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 16) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 12) & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 8)  & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 4)  & 0xf) + '0');
	*ch++ = (char)(((dw2 >> 0)  & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 28) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 24) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 20) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 16) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 12) & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 8)  & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 4)  & 0xf) + '0');
	*ch++ = (char)(((dw3 >> 0)  & 0xf) + '0');
	*ch++ = 'E';
	if (dw1 & 0x40000000)   // exponent sign
	{
		*ch++ = '-';
	}
	*ch++ = (char)(((dw1 >> 24) & 0xf) + '0');
	*ch++ = (char)(((dw1 >> 20) & 0xf) + '0');
	*ch++ = (char)(((dw1 >> 16) & 0xf) + '0');
	*ch = '\0';

	sscanf(str, "%le", &tmp);

	result = double_to_fx80(tmp);

	return result;
}

inline void store_pack_float80(u32 ea, int k, floatx80 fpr)
{
	u32 dw1, dw2, dw3;
	char str[128], *ch;
	int i, j, exp;

	dw1 = dw2 = dw3 = 0;
	ch = &str[0];

	sprintf(str, "%.16e", fx80_to_double(fpr));

	if (*ch == '-')
	{
		ch++;
		dw1 = 0x80000000;
	}

	if (*ch == '+')
	{
		ch++;
	}

	dw1 |= (*ch++ - '0');

	if (*ch == '.')
	{
		ch++;
	}

	// handle negative k-factor here
	if ((k <= 0) && (k >= -13))
	{
		exp = 0;
		for (i = 0; i < 3; i++)
		{
			if (ch[18+i] >= '0' && ch[18+i] <= '9')
			{
				exp = (exp << 4) | (ch[18+i] - '0');
			}
		}

		if (ch[17] == '-')
		{
			exp = -exp;
		}

		k = -k;
		// last digit is (k + exponent - 1)
		k += (exp - 1);

		// round up the last significant mantissa digit
		if (ch[k+1] >= '5')
		{
			ch[k]++;
		}

		// zero out the rest of the mantissa digits
		for (j = (k+1); j < 16; j++)
		{
			ch[j] = '0';
		}

		// now zero out K to avoid tripping the positive K detection below
		k = 0;
	}

	// crack 8 digits of the mantissa
	for (i = 0; i < 8; i++)
	{
		dw2 <<= 4;
		if (*ch >= '0' && *ch <= '9')
		{
			dw2 |= *ch++ - '0';
		}
	}

	// next 8 digits of the mantissa
	for (i = 0; i < 8; i++)
	{
		dw3 <<= 4;
		if (*ch >= '0' && *ch <= '9')
		dw3 |= *ch++ - '0';
	}

	// handle masking if k is positive
	if (k >= 1)
	{
		if (k <= 17)
		{
			dw2 &= pkmask2[k];
			dw3 &= pkmask3[k];
		}
		else
		{
			dw2 &= pkmask2[17];
			dw3 &= pkmask3[17];
//          m_fpcr |=  (need to set OPERR bit)
		}
	}

	// finally, crack the exponent
	if (*ch == 'e' || *ch == 'E')
	{
		ch++;
		if (*ch == '-')
		{
			ch++;
			dw1 |= 0x40000000;
		}

		if (*ch == '+')
		{
			ch++;
		}

		j = 0;
		for (i = 0; i < 3; i++)
		{
			if (*ch >= '0' && *ch <= '9')
			{
				j = (j << 4) | (*ch++ - '0');
			}
		}

		dw1 |= (j << 16);
	}

	m68ki_write_32(ea, dw1);
	m68ki_write_32(ea+4, dw2);
	m68ki_write_32(ea+8, dw3);
}

inline void SET_CONDITION_CODES(floatx80 reg)
{
//  u64 *regi;

//  regi = (u64 *)&reg;

	m_fpsr &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (reg.high & 0x8000)
	{
		m_fpsr |= FPCC_N;
	}

	// zero flag
	if (((reg.high & 0x7fff) == 0) && ((reg.low<<1) == 0))
	{
		m_fpsr |= FPCC_Z;
	}

	// infinity flag
	if (((reg.high & 0x7fff) == 0x7fff) && ((reg.low<<1) == 0))
	{
		m_fpsr |= FPCC_I;
	}

	// NaN flag
	if (floatx80_is_nan(reg))
	{
		m_fpsr |= FPCC_NAN;
	}
}

inline int TEST_CONDITION(int condition)
{
	int n = (m_fpsr & FPCC_N) != 0;
	int z = (m_fpsr & FPCC_Z) != 0;
	int nan = (m_fpsr & FPCC_NAN) != 0;
	int r = 0;
	switch (condition)
	{
		case 0x10:
		case 0x00:      return 0;                   // False

		case 0x11:
		case 0x01:      return (z);                 // Equal

		case 0x12:
		case 0x02:      return (!(nan || z || n));          // Greater Than

		case 0x13:
		case 0x03:      return (z || !(nan || n));          // Greater or Equal

		case 0x14:
		case 0x04:      return (n && !(nan || z));          // Less Than

		case 0x15:
		case 0x05:      return (z || (n && !nan));          // Less Than or Equal

		case 0x16:
		case 0x06:      return !nan && !z;

		case 0x17:
		case 0x07:      return !nan;

		case 0x18:
		case 0x08:      return nan;

		case 0x19:
		case 0x09:      return nan || z;

		case 0x1a:
		case 0x0a:      return (nan || !(n || z));          // Not Less Than or Equal

		case 0x1b:
		case 0x0b:      return (nan || z || !n);            // Not Less Than

		case 0x1c:
		case 0x0c:      return (nan || (n && !z));          // Not Greater or Equal Than

		case 0x1d:
		case 0x0d:      return (nan || z || n);             // Not Greater Than

		case 0x1e:
		case 0x0e:      return (!z);                    // Not Equal

		case 0x1f:
		case 0x0f:      return 1;                   // True

		default:        fatalerror("M68kFPU: test_condition: unhandled condition %02X\n", condition);
	}

	return r;
}

u8 READ_EA_8(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return REG_D()[reg];
		}
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			return m68ki_read_8(ea);
		}
		case 3:     // (An)+
		{
			u32 ea = EA_AY_PI_8();
			return m68ki_read_8(ea);
		}
		case 4:     // -(An)
		{
			u32 ea = EA_AY_PD_8();
			return m68ki_read_8(ea);
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_8();
			return m68ki_read_8(ea);
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_8();
			return m68ki_read_8(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					u32 ea = OPER_I_16();
					return m68ki_read_8(ea);
				}
				case 1:     // (xxx).L
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					return m68ki_read_8(ea);
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_8();
					return m68ki_read_8(ea);
				}
				case 3:     // (PC) + (Xn) + d8
				{
					u32 ea =  EA_PCIX_8();
					return m68ki_read_8(ea);
				}
				case 4:     // #<data>
				{
					return  OPER_I_8();
				}
				default:    fatalerror("M68kFPU: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
	}

	return 0;
}

u16 READ_EA_16(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return (u16)(REG_D()[reg]);
		}
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			return m68ki_read_16(ea);
		}
		case 3:     // (An)+
		{
			u32 ea = EA_AY_PI_16();
			return m68ki_read_16(ea);
		}
		case 4:     // -(An)
		{
			u32 ea = EA_AY_PD_16();
			return m68ki_read_16(ea);
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_16();
			return m68ki_read_16(ea);
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_16();
			return m68ki_read_16(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					u32 ea = OPER_I_16();
					return m68ki_read_16(ea);
				}
				case 1:     // (xxx).L
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					return m68ki_read_16(ea);
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_16();
					return m68ki_read_16(ea);
				}
				case 3:     // (PC) + (Xn) + d8
				{
					u32 ea =  EA_PCIX_16();
					return m68ki_read_16(ea);
				}
				case 4:     // #<data>
				{
					return OPER_I_16();
				}

				default:    fatalerror("M68kFPU: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
	}

	return 0;
}

u32 READ_EA_32(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return REG_D()[reg];
		}
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			return m68ki_read_32(ea);
		}
		case 3:     // (An)+
		{
			u32 ea = EA_AY_PI_32();
			return m68ki_read_32(ea);
		}
		case 4:     // -(An)
		{
			u32 ea = EA_AY_PD_32();
			return m68ki_read_32(ea);
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_32();
			return m68ki_read_32(ea);
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_32();
			return m68ki_read_32(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					u32 ea = OPER_I_16();
					return m68ki_read_32(ea);
				}
				case 1:     // (xxx).L
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					return m68ki_read_32(ea);
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_32();
					return m68ki_read_32(ea);
				}
				case 3:     // (PC) + (Xn) + d8
				{
					u32 ea =  EA_PCIX_32();
					return m68ki_read_32(ea);
				}
				case 4:     // #<data>
				{
					return  OPER_I_32();
				}
				default:    fatalerror("M68kFPU: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
	}
	return 0;
}

u64 READ_EA_64(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	u32 h1, h2;

	switch (mode)
	{
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (u64)(h1) << 32 | (u64)(h2);
		}
		case 3:     // (An)+
		{
			u32 ea = REG_A()[reg];
			REG_A()[reg] += 8;
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (u64)(h1) << 32 | (u64)(h2);
		}
		case 4:     // -(An)
		{
			u32 ea = REG_A()[reg]-8;
			REG_A()[reg] -= 8;
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (u64)(h1) << 32 | (u64)(h2);
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_32();
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (u64)(h1) << 32 | (u64)(h2);
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_32();
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (u64)(h1) << 32 | (u64)(h2);
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).L
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					return (u64)(m68ki_read_32(ea)) << 32 | (u64)(m68ki_read_32(ea+4));
				}
				case 3:     // (PC) + (Xn) + d8
				{
					u32 ea =  EA_PCIX_32();
					h1 = m68ki_read_32(ea+0);
					h2 = m68ki_read_32(ea+4);
					return  (u64)(h1) << 32 | (u64)(h2);
				}
				case 4:     // #<data>
				{
					h1 = OPER_I_32();
					h2 = OPER_I_32();
					return  (u64)(h1) << 32 | (u64)(h2);
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_32();
					h1 = m68ki_read_32(ea+0);
					h2 = m68ki_read_32(ea+4);
					return  (u64)(h1) << 32 | (u64)(h2);
				}
				default:    fatalerror("M68kFPU: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
	}

	return 0;
}


floatx80 READ_EA_FPE(int ea)
{
	floatx80 fpr;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			fpr = load_extended_float80(ea);
			break;
		}

		case 3:     // (An)+
		{
			u32 ea = REG_A()[reg];
			REG_A()[reg] += 12;
			fpr = load_extended_float80(ea);
			break;
		}
		case 4:     // -(An)
		{
			u32 ea = REG_A()[reg]-12;
			REG_A()[reg] -= 12;
			fpr = load_extended_float80(ea);
			break;
		}
		case 5:     // (d16, An)
		{
			// FIXME: will fail for fmovem
			u32 ea = EA_AY_DI_32();
			fpr = load_extended_float80(ea);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			// FIXME: will fail for fmovem
			u32 ea = EA_AY_IX_32();
			fpr = load_extended_float80(ea);
			break;
		}

		case 7: // extended modes
		{
			switch (reg)
			{
				case 1:     // (xxx)
					{
						u32 d1 = OPER_I_16();
						u32 d2 = OPER_I_16();
						u32 ea = (d1 << 16) | d2;
						fpr = load_extended_float80(ea);
					}
					break;

				case 2: // (d16, PC)
					{
						u32 ea = EA_PCDI_32();
						fpr = load_extended_float80(ea);
					}
					break;

				case 3: // (d16,PC,Dx.w)
					{
						u32 ea = EA_PCIX_32();
						fpr = load_extended_float80(ea);
					}
					break;

				default:
					fatalerror("M68kFPU: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
					break;
			}
		}
		break;

		default:    fatalerror("M68kFPU: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc); break;
	}

	return fpr;
}

floatx80 READ_EA_PACK(int ea)
{
	floatx80 fpr;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			fpr = load_pack_float80(ea);
			break;
		}

		case 3:     // (An)+
		{
			u32 ea = REG_A()[reg];
			REG_A()[reg] += 12;
			fpr = load_pack_float80(ea);
			break;
		}

		case 7: // extended modes
		{
			switch (reg)
			{
				case 3: // (d16,PC,Dx.w)
					{
						u32 ea = EA_PCIX_32();
						fpr = load_pack_float80(ea);
					}
					break;

				default:
					fatalerror("M68kFPU: READ_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
					break;
			}
		}
		break;

		default:    fatalerror("M68kFPU: READ_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc); break;
	}

	return fpr;
}

void WRITE_EA_8(int ea, u8 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] = data;
			break;
		}
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			m68ki_write_8(ea, data);
			break;
		}
		case 3:     // (An)+
		{
			u32 ea = EA_AY_PI_8();
			m68ki_write_8(ea, data);
			break;
		}
		case 4:     // -(An)
		{
			u32 ea = EA_AY_PD_8();
			m68ki_write_8(ea, data);
			break;
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_8();
			m68ki_write_8(ea, data);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_8();
			m68ki_write_8(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).B
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					m68ki_write_8(ea, data);
					break;
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_16();
					m68ki_write_8(ea, data);
					break;
				}
				default:    fatalerror("M68kFPU: WRITE_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: WRITE_EA_8: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, m_pc);
	}
}

void WRITE_EA_16(int ea, u16 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] = data;
			break;
		}
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			m68ki_write_16(ea, data);
			break;
		}
		case 3:     // (An)+
		{
			u32 ea = EA_AY_PI_16();
			m68ki_write_16(ea, data);
			break;
		}
		case 4:     // -(An)
		{
			u32 ea = EA_AY_PD_16();
			m68ki_write_16(ea, data);
			break;
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_16();
			m68ki_write_16(ea, data);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_16();
			m68ki_write_16(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).W
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					m68ki_write_16(ea, data);
					break;
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_16();
					m68ki_write_16(ea, data);
					break;
				}
				default:    fatalerror("M68kFPU: WRITE_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: WRITE_EA_16: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, m_pc);
	}
}

void WRITE_EA_32(int ea, u32 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] = data;
			break;
		}
		case 1:     // An
		{
			REG_A()[reg] = data;
			break;
		}
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			m68ki_write_32(ea, data);
			break;
		}
		case 3:     // (An)+
		{
			u32 ea = EA_AY_PI_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 4:     // -(An)
		{
			u32 ea = EA_AY_PD_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				{
					u32 ea = OPER_I_16();
					m68ki_write_32(ea, data);
					break;
				}
				case 1:     // (xxx).L
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					m68ki_write_32(ea, data);
					break;
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_32();
					m68ki_write_32(ea, data);
					break;
				}
				default:    fatalerror("M68kFPU: WRITE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: WRITE_EA_32: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, m_pc);
	}
}

void WRITE_EA_64(int ea, u64 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			u32 ea = REG_A()[reg];
			m68ki_write_32(ea, (u32)(data >> 32));
			m68ki_write_32(ea+4, (u32)(data));
			break;
		}
		case 3:     // (An)+
		{
			u32 ea = REG_A()[reg];
			REG_A()[reg] += 8;
			m68ki_write_32(ea+0, (u32)(data >> 32));
			m68ki_write_32(ea+4, (u32)(data));
			break;
		}
		case 4:     // -(An)
		{
			u32 ea;
			REG_A()[reg] -= 8;
			ea = REG_A()[reg];
			m68ki_write_32(ea+0, (u32)(data >> 32));
			m68ki_write_32(ea+4, (u32)(data));
			break;
		}
		case 5:     // (d16, An)
		{
			u32 ea = EA_AY_DI_32();
			m68ki_write_32(ea+0, (u32)(data >> 32));
			m68ki_write_32(ea+4, (u32)(data));
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			u32 ea = EA_AY_IX_32();
			m68ki_write_32(ea+0, (u32)(data >> 32));
			m68ki_write_32(ea+4, (u32)(data));
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:     // (xxx).L
				{
					u32 d1 = OPER_I_16();
					u32 d2 = OPER_I_16();
					u32 ea = (d1 << 16) | d2;
					m68ki_write_32(ea+0, (u32)(data >> 32));
					m68ki_write_32(ea+4, (u32)(data));
					break;
				}
				case 2:     // (d16, PC)
				{
					u32 ea = EA_PCDI_32();
					m68ki_write_32(ea+0, (u32)(data >> 32));
					m68ki_write_32(ea+4, (u32)(data));
					break;
				}
				default:    fatalerror("M68kFPU: WRITE_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: WRITE_EA_64: unhandled mode %d, reg %d, data %08X%08X at %08X\n", mode, reg, (u32)(data >> 32), (u32)(data), m_pc);
	}
}

void WRITE_EA_FPE(int ea, floatx80 fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			u32 ea;
			ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			break;
		}

		case 3:     // (An)+
		{
			u32 ea;
			ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			REG_A()[reg] += 12;
			break;
		}

		case 4:     // -(An)
		{
			u32 ea;
			REG_A()[reg] -= 12;
			ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			break;
		}

		case 5:     // (d16,An)
		{
			u32 ea = EA_AY_DI_32();
			store_extended_float80(ea, fpr);
			break;
		}

		case 7:
		{
			switch (reg)
			{
				default:    fatalerror("M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
			}
		}
		default:    fatalerror("M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
	}
}

void WRITE_EA_PACK(int ea, int k, floatx80 fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:     // (An)
		{
			u32 ea;
			ea = REG_A()[reg];
			store_pack_float80(ea, k, fpr);
			break;
		}

		case 3:     // (An)+
		{
			u32 ea;
			ea = REG_A()[reg];
			store_pack_float80(ea, k, fpr);
			REG_A()[reg] += 12;
			break;
		}

		case 4:     // -(An)
		{
			u32 ea;
			REG_A()[reg] -= 12;
			ea = REG_A()[reg];
			store_pack_float80(ea, k, fpr);
			break;
		}

		case 7:
		{
			switch (reg)
			{
				default:    fatalerror("M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
			}
		}
		default:    fatalerror("M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
	}
}

void fpgen_rm_reg(u16 w2)
{
	int ea = m_ir & 0x3f;
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	floatx80 source;

	// fmovecr #$f, fp0 f200 5c0f

	if (rm)
	{
		switch (src)
		{
			case 0:     // Long-Word Integer
			{
				s32 d = READ_EA_32(ea);
				source = int32_to_floatx80(d);
				break;
			}
			case 1:     // Single-precision Real
			{
				u32 d = READ_EA_32(ea);
				source = float32_to_floatx80(d);
				break;
			}
			case 2:     // Extended-precision Real
			{
				source = READ_EA_FPE(ea);
				break;
			}
			case 3:     // Packed-decimal Real
			{
				source = READ_EA_PACK(ea);
				break;
			}
			case 4:     // Word Integer
			{
				s16 d = READ_EA_16(ea);
				source = int32_to_floatx80((s32)d);
				break;
			}
			case 5:     // Double-precision Real
			{
				u64 d = READ_EA_64(ea);

				source = float64_to_floatx80(d);
				break;
			}
			case 6:     // Byte Integer
			{
				s8 d = READ_EA_8(ea);
				source = int32_to_floatx80((s32)d);
				break;
			}
			case 7:     // FMOVECR load from constant ROM
			{
				switch (w2 & 0x7f)
				{
					case 0x0:   // Pi
						source.high = 0x4000;
						source.low = 0xc90fdaa22168c235U;
						break;

					case 0xb:   // log10(2)
						source.high = 0x3ffd;
						source.low = 0x9a209a84fbcff798U;
						break;

					case 0xc:   // e
						source.high = 0x4000;
						source.low = 0xadf85458a2bb4a9bU;
						break;

					case 0xd:   // log2(e)
						source.high = 0x3fff;
						source.low = 0xb8aa3b295c17f0bcU;
						break;

					case 0xe:   // log10(e)
						source.high = 0x3ffd;
						source.low = 0xde5bd8a937287195U;
						break;

					case 0xf:   // 0.0
						source = int32_to_floatx80((s32)0);
						break;

					case 0x30:  // ln(2)
						source.high = 0x3ffe;
						source.low = 0xb17217f7d1cf79acU;
						break;

					case 0x31:  // ln(10)
						source.high = 0x4000;
						source.low = 0x935d8dddaaa8ac17U;
						break;

					case 0x32:  // 1 (or 100?  manuals are unclear, but 1 would make more sense)
						source = int32_to_floatx80((s32)1);
						break;

					case 0x33:  // 10^1
						source = int32_to_floatx80((s32)10);
						break;

					case 0x34:  // 10^2
						source = int32_to_floatx80((s32)10*10);
						break;

					case 0x35:  // 10^4
						source = int32_to_floatx80((s32)1000*10);
						break;

					case 0x36:  // 1.0e8
						source = int32_to_floatx80((s32)10000000*10);
						break;

					case 0x37:  // 1.0e16 - can't get the right precision from s32 so go "direct" with constants from h/w
						source.high = 0x4034;
						source.low = 0x8e1bc9bf04000000U;
						break;

					case 0x38:  // 1.0e32
						source.high = 0x4069;
						source.low = 0x9dc5ada82b70b59eU;
						break;

					case 0x39:  // 1.0e64
						source.high = 0x40d3;
						source.low = 0xc2781f49ffcfa6d5U;
						break;

					case 0x3a:  // 1.0e128
						source.high = 0x41a8;
						source.low = 0x93ba47c980e98ce0U;
						break;

					case 0x3b:  // 1.0e256
						source.high = 0x4351;
						source.low = 0xaa7eebfb9df9de8eU;
						break;

					case 0x3c:  // 1.0e512
						source.high = 0x46a3;
						source.low = 0xe319a0aea60e91c7U;
						break;

					case 0x3d:  // 1.0e1024
						source.high = 0x4d48;
						source.low = 0xc976758681750c17U;
						break;

					case 0x3e:  // 1.0e2048
						source.high = 0x5a92;
						source.low = 0x9e8b3b5dc53d5de5U;
						break;

					case 0x3f:  // 1.0e4096
						source.high = 0x7525;
						source.low = 0xc46052028a20979bU;
						break;

					default:
						fatalerror("fmove_rm_reg: unknown constant ROM offset %x at %08x\n", w2&0x7f, m_pc-4);
						break;
				}

				// handle it right here, the usual opmode bits aren't valid in the FMOVECR case
				m_fpr[dst] = source;
				m_icount -= 4;
				return;
			}
			default:    fatalerror("fmove_rm_reg: invalid source specifier %x at %08X\n", src, m_pc-4);
		}
	}
	else
	{
		source = m_fpr[src];
	}



	switch (opmode)
	{
		case 0x00:      // FMOVE
		{
			m_fpr[dst] = source;
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 4;
			break;
		}
		case 0x01:      // FINT
		{
			s32 temp;
			temp = floatx80_to_int32(source);
			m_fpr[dst] = int32_to_floatx80(temp);
			break;
		}
		case 0x03:      // FINTRZ
		{
			s32 temp;
			temp = floatx80_to_int32_round_to_zero(source);
			m_fpr[dst] = int32_to_floatx80(temp);
			break;
		}
		case 0x04:      // FSQRT
		{
			m_fpr[dst] = floatx80_sqrt(source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 109;
			break;
		}
		case 0x06:      // FLOGNP1
		{
			m_fpr[dst] = floatx80_flognp1 (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 594; // for MC68881
			break;
		}
		case 0x0e:      // FSIN
		{
			m_fpr[dst] = source;
			floatx80_fsin(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 75;
			break;
		}
		case 0x0f:      // FTAN
		{
			m_fpr[dst] = source;
			floatx80_ftan(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 75;
			break;
		}
		case 0x14:      // FLOGN
		{
			m_fpr[dst] = floatx80_flogn (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 548; // for MC68881
			break;
		}
		case 0x15:      // FLOG10
		{
			m_fpr[dst] = floatx80_flog10 (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 604; // for MC68881
			break;
		}
		case 0x16:      // FLOG2
		{
			m_fpr[dst] = floatx80_flog2 (source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 604; // for MC68881
			break;
		}
		case 0x18:      // FABS
		{
			m_fpr[dst] = source;
			m_fpr[dst].high &= 0x7fff;
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 3;
			break;
		}
		case 0x1a:      // FNEG
		{
			m_fpr[dst] = source;
			m_fpr[dst].high ^= 0x8000;
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 3;
			break;
		}
		case 0x1d:      // FCOS
		{
			m_fpr[dst] = source;
			floatx80_fcos(m_fpr[dst]);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 75;
			break;
		}
		case 0x1e:      // FGETEXP
		{
			s16 temp2;

			temp2 = source.high;    // get the exponent
			temp2 -= 0x3fff;    // take off the bias
			m_fpr[dst] = double_to_fx80((double)temp2);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 6;
			break;
		}
		case 0x20:      // FDIV
		{
			m_fpr[dst] = floatx80_div(m_fpr[dst], source);
			m_icount -= 43;
			break;
		}
		case 0x22:      // FADD
		{
			m_fpr[dst] = floatx80_add(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 9;
			break;
		}
		case 0x23:      // FMUL
		{
			m_fpr[dst] = floatx80_mul(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 11;
			break;
		}
		case 0x24:      // FSGLDIV
		{
			float32 a = floatx80_to_float32( m_fpr[dst] );
			float32 b = floatx80_to_float32( source );
			m_fpr[dst] = float32_to_floatx80( float32_div(a, b) );
			m_icount -= 43; //  // ? (value is from FDIV)
			break;
		}
		case 0x25:      // FREM
		{
			m_fpr[dst] = floatx80_rem(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 43;   // guess
			break;
		}
		case 0x26:      // FSCALE
		{
			m_fpr[dst] = floatx80_scale(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 46;   // (better?) guess
			break;
		}
		case 0x27:      // FSGLMUL
		{
			float32 a = floatx80_to_float32( m_fpr[dst] );
			float32 b = floatx80_to_float32( source );
			m_fpr[dst] = float32_to_floatx80( float32_mul(a, b) );
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 11; // ? (value is from FMUL)
			break;
		}
		case 0x28:      // FSUB
		{
			m_fpr[dst] = floatx80_sub(m_fpr[dst], source);
			SET_CONDITION_CODES(m_fpr[dst]);
			m_icount -= 9;
			break;
		}
		case 0x38:      // FCMP
		{
			floatx80 res;
			res = floatx80_sub(m_fpr[dst], source);
			SET_CONDITION_CODES(res);
			m_icount -= 7;
			break;
		}
		case 0x3a:      // FTST
		{
			floatx80 res;
			res = source;
			SET_CONDITION_CODES(res);
			m_icount -= 7;
			break;
		}

		default:    fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, m_ppc);
	}
}

void fmove_reg_mem(u16 w2)
{
	int ea = m_ir & 0x3f;
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	int k = (w2 & 0x7f);

	switch (dst)
	{
		case 0:     // Long-Word Integer
		{
			s32 d = (s32)floatx80_to_int32(m_fpr[src]);
			WRITE_EA_32(ea, d);
			break;
		}
		case 1:     // Single-precision Real
		{
			u32 d = floatx80_to_float32(m_fpr[src]);
			WRITE_EA_32(ea, d);
			break;
		}
		case 2:     // Extended-precision Real
		{
			WRITE_EA_FPE(ea, m_fpr[src]);
			break;
		}
		case 3:     // Packed-decimal Real with Static K-factor
		{
			// sign-extend k
			k = (k & 0x40) ? (k | 0xffffff80) : (k & 0x7f);
			WRITE_EA_PACK(ea, k, m_fpr[src]);
			break;
		}
		case 4:     // Word Integer
		{
			s32 value = floatx80_to_int32(m_fpr[src]);
			if (value > 0x7fff || value < -0x8000 )
			{
				m_fpsr |= FPES_OE | FPAE_IOP;
			}
			WRITE_EA_16(ea, (s16)value);
			break;
		}
		case 5:     // Double-precision Real
		{
			u64 d;

			d = floatx80_to_float64(m_fpr[src]);

			WRITE_EA_64(ea, d);
			break;
		}
		case 6:     // Byte Integer
		{
			s32 value = floatx80_to_int32(m_fpr[src]);
			if (value > 127 || value < -128)
			{
				m_fpsr |= FPES_OE | FPAE_IOP;
			}
			WRITE_EA_8(ea, (s8) value);
			break;
		}
		case 7:     // Packed-decimal Real with Dynamic K-factor
		{
			WRITE_EA_PACK(ea, REG_D()[k>>4], m_fpr[src]);
			break;
		}
	}

	m_icount -= 12;
}

void fmove_fpcr(u16 w2)
{
	int ea = m_ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int regsel = (w2 >> 10) & 0x7;
	int mode = (ea >> 3) & 0x7;

	if ((mode == 5) || (mode == 6))
	{
		u32 address = 0xffffffff;    // force a bus error if this doesn't get assigned

		if (mode == 5)
		{
			address = EA_AY_DI_32();
		}
		else if (mode == 6)
		{
			address = EA_AY_IX_32();
		}

		if (dir)    // From system control reg to <ea>
		{
			if (regsel & 4) { m68ki_write_32(address, m_fpcr); address += 4; }
			if (regsel & 2) { m68ki_write_32(address, m_fpsr); address += 4; }
			if (regsel & 1) { m68ki_write_32(address, m_fpiar); address += 4; }
		}
		else        // From <ea> to system control reg
		{
			if (regsel & 4) { m_fpcr = m68ki_read_32(address); address += 4; }
			if (regsel & 2) { m_fpsr = m68ki_read_32(address); address += 4; }
			if (regsel & 1) { m_fpiar = m68ki_read_32(address); address += 4; }
		}
	}
	else
	{
		if (dir)    // From system control reg to <ea>
		{
			if (regsel & 4) WRITE_EA_32(ea, m_fpcr);
			if (regsel & 2) WRITE_EA_32(ea, m_fpsr);
			if (regsel & 1) WRITE_EA_32(ea, m_fpiar);
		}
		else        // From <ea> to system control reg
		{
			if (regsel & 4) m_fpcr = READ_EA_32(ea);
			if (regsel & 2) m_fpsr = READ_EA_32(ea);
			if (regsel & 1) m_fpiar = READ_EA_32(ea);
		}
	}

	// FIXME: (2011-12-18 ost)
	// rounding_mode and rounding_precision of softfloat.c should be set according to current fpcr
	// but:  with this code on Apollo the following programs in /systest/fptest will fail:
	// 1. Single Precision Whetstone will return wrong results never the less
	// 2. Vector Test will fault with 00040004: reference to illegal address

	if ((regsel & 4) && dir == 0)
	{
		int rnd = (m_fpcr >> 4) & 3;
		int prec = (m_fpcr >> 6) & 3;

//      logerror("m68k_fpsp:fmove_fpcr fpcr=%04x prec=%d rnd=%d\n", m_fpcr, prec, rnd);

#ifdef FLOATX80
		switch (prec)
		{
		case 0: // Extend (X)
			floatx80_rounding_precision = 80;
			break;
		case 1: // Single (S)
			floatx80_rounding_precision = 32;
			break;
		case 2: // Double (D)
			floatx80_rounding_precision = 64;
			break;
		case 3: // Undefined
			floatx80_rounding_precision = 80;
			break;
		}
#endif

		switch (rnd)
		{
		case 0: // To Nearest (RN)
			float_rounding_mode = float_round_nearest_even;
			break;
		case 1: // To Zero (RZ)
			float_rounding_mode = float_round_to_zero;
			break;
		case 2: // To Minus Infinitiy (RM)
			float_rounding_mode = float_round_down;
			break;
		case 3: // To Plus Infinitiy (RP)
			float_rounding_mode = float_round_up;
			break;
		}
	}

	m_icount -= 10;
}

void fmovem(u16 w2)
{
	int i;
	int ea = m_ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int mode = (w2 >> 11) & 0x3;
	int reglist = w2 & 0xff;

	u32 mem_addr = 0;
	switch (ea >> 3)
	{
		case 5:     // (d16, An)
			mem_addr= EA_AY_DI_32();
			break;
		case 6:     // (An) + (Xn) + d8
			mem_addr= EA_AY_IX_32();
			break;
	}

	if (dir)    // From FP regs to mem
	{
		switch (mode)
		{
			case 1: // Dynamic register list, postincrement or control addressing mode.
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];

			case 0:     // Static register list, predecrement or control addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						switch (ea >> 3)
						{
							case 5:     // (d16, An)
							case 6:     // (An) + (Xn) + d8
								store_extended_float80(mem_addr, m_fpr[i]);
								mem_addr += 12;
								break;
							default:
								WRITE_EA_FPE(ea, m_fpr[i]);
								break;
						}

						m_icount -= 2;
					}
				}
				break;
			}

			case 2:     // Static register list, postdecrement or control addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						switch (ea >> 3)
						{
							case 5:     // (d16, An)
							case 6:     // (An) + (Xn) + d8
								store_extended_float80(mem_addr, m_fpr[7-i]);
								mem_addr += 12;
								break;
							default:
								WRITE_EA_FPE(ea, m_fpr[7-i]);
								break;
						}

						m_icount -= 2;
					}
				}
				break;
			}

			default:    fatalerror("M680x0: FMOVEM: mode %d unimplemented at %08X\n", mode, m_pc-4);
		}
	}
	else        // From mem to FP regs
	{
		switch (mode)
		{
			case 3: // Dynamic register list, predecrement addressing mode.
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];

			case 2:     // Static register list, postincrement or control addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						switch (ea >> 3)
						{
							case 5:     // (d16, An)
							case 6:     // (An) + (Xn) + d8
								m_fpr[7-i] = load_extended_float80(mem_addr);
								mem_addr += 12;
								break;
							default:
								m_fpr[7-i] = READ_EA_FPE(ea);
								break;
						}
						m_icount -= 2;
					}
				}
				break;
			}

			default:    fatalerror("M680x0: FMOVEM: mode %d unimplemented at %08X\n", mode, m_pc-4);
		}
	}
}

void fscc()
{
	int ea = m_ir & 0x3f;
	int condition = (s16)(OPER_I_16());

	WRITE_EA_8(ea, TEST_CONDITION(condition) ? 0xff : 0);
	m_icount -= 7; // ???
}

void fbcc16()
{
	s32 offset;
	int condition = m_ir & 0x3f;

	offset = (s16)(OPER_I_16());

	// TODO: condition and jump!!!
	if (TEST_CONDITION(condition))
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset-2);
	}

	m_icount -= 7;
}

void fbcc32()
{
	s32 offset;
	int condition = m_ir & 0x3f;

	offset = OPER_I_32();

	// TODO: condition and jump!!!
	if (TEST_CONDITION(condition))
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset-4);
	}

	m_icount -= 7;
}


void m68040_fpu_op0()
{
	m_fpu_just_reset = 0;

	switch ((m_ir >> 6) & 0x3)
	{
		case 0:
		{
			u16 w2 = OPER_I_16();
			switch ((w2 >> 13) & 0x7)
			{
				case 0x0:   // FPU ALU FP, FP
				case 0x2:   // FPU ALU ea, FP
				{
					fpgen_rm_reg(w2);
					break;
				}

				case 0x3:   // FMOVE FP, ea
				{
					fmove_reg_mem(w2);
					break;
				}

				case 0x4:   // FMOVEM ea, FPCR
				case 0x5:   // FMOVEM FPCR, ea
				{
					fmove_fpcr(w2);
					break;
				}

				case 0x6:   // FMOVEM ea, list
				case 0x7:   // FMOVEM list, ea
				{
					fmovem(w2);
					break;
				}

				default:    fatalerror("M68kFPU: unimplemented subop %d at %08X\n", (w2 >> 13) & 0x7, m_pc-4);
			}
			break;
		}

		case 1:     // FBcc disp16
		{
			switch ((m_ir >> 3) & 0x7) {
			case 1: // FDBcc
				// TODO:
				break;
			default: // FScc (?)
				fscc();
				return;
			}
			fatalerror("M68kFPU: unimplemented main op %d with mode %d at %08X\n", (m_ir >> 6) & 0x3, (m_ir >> 3) & 0x7, m_ppc);
		}

		case 2:     // FBcc disp16
		{
			fbcc16();
			break;
		}
		case 3:     // FBcc disp32
		{
			fbcc32();
			break;
		}

		default:    fatalerror("M68kFPU: unimplemented main op %d\n", (m_ir >> 6)   & 0x3);
	}
}

int perform_fsave(u32 addr, int inc)
{
	if(m_cpu_type & CPU_TYPE_040)
	{
		if(inc)
		{
			m68ki_write_32(addr, 0x41000000);
			return 4;
		}
		else
		{
			m68ki_write_32(addr-4, 0x41000000);
			return -4;
		}
	}

	if (inc)
	{
		// 68881 IDLE, version 0x1f
		m68ki_write_32(addr, 0x1f180000);
		m68ki_write_32(addr+4, 0);
		m68ki_write_32(addr+8, 0);
		m68ki_write_32(addr+12, 0);
		m68ki_write_32(addr+16, 0);
		m68ki_write_32(addr+20, 0);
		m68ki_write_32(addr+24, 0x70000000);
		return 7*4;
	}
	else
	{
		m68ki_write_32(addr-4, 0x70000000);
		m68ki_write_32(addr-8, 0);
		m68ki_write_32(addr-12, 0);
		m68ki_write_32(addr-16, 0);
		m68ki_write_32(addr-20, 0);
		m68ki_write_32(addr-24, 0);
		m68ki_write_32(addr-28, 0x1f180000);
		return -7*4;
	}
}

// FRESTORE on a nullptr frame reboots the FPU - all registers to NaN, the 3 status regs to 0
void do_frestore_null()
{
	int i;

	m_fpcr = 0;
	m_fpsr = 0;
	m_fpiar = 0;
	for (i = 0; i < 8; i++)
	{
		m_fpr[i].high = 0x7fff;
		m_fpr[i].low = 0xffffffffffffffffU;
	}

	// Mac IIci at 408458e6 wants an FSAVE of a just-restored nullptr frame to also be nullptr
	// The PRM says it's possible to generate a nullptr frame, but not how/when/why.  (need the 68881/68882 manual!)
	m_fpu_just_reset = 1;
}

void m68040_do_fsave(u32 addr, int reg, int inc)
{
	if (m_fpu_just_reset)
	{
		m68ki_write_32(addr, 0);
	}
	else
	{
		// we normally generate an IDLE frame
		int delta = perform_fsave(addr, inc);
		if(reg != -1)
			REG_A()[reg] += delta;
	}
}

void m68040_do_frestore(u32 addr, int reg)
{
	bool m40 = m_cpu_type & CPU_TYPE_040;
	u32 temp = m68ki_read_32(addr);

	// check for nullptr frame
	if (temp & 0xff000000)
	{
		// we don't handle non-nullptr frames
		m_fpu_just_reset = 0;

		if (reg != -1)
		{
			// how about an IDLE frame?
			if (!m40 && ((temp & 0x00ff0000) == 0x00180000))
			{
				REG_A()[reg] += 7*4;
			}
			else if (m40 && ((temp & 0xffff0000) == 0x41000000))
			{
				REG_A()[reg] += 4;
			} // check UNIMP
			else if ((temp & 0x00ff0000) == 0x00380000)
			{
				REG_A()[reg] += 14*4;
			} // check BUSY
			else if ((temp & 0x00ff0000) == 0x00b40000)
			{
				REG_A()[reg] += 45*4;
			}
		}
	}
	else
	{
		do_frestore_null();
	}
}

void m68040_fpu_op1()
{
	int ea = m_ir & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	u32 addr;

	switch ((m_ir >> 6) & 0x3)
	{
		case 0:     // FSAVE <ea>
		{
			switch (mode)
			{
			case 2: // (An)
				addr = REG_A()[reg];
				m68040_do_fsave(addr, -1, 1);
				break;

			case 3: // (An)+
				addr = EA_AY_PI_32();
				m68040_do_fsave(addr, reg, 1);
				break;

			case 4: // -(An)
				addr = EA_AY_PD_32();
				m68040_do_fsave(addr, reg, 0);
				break;

			case 5: // (D16, An)
				addr = EA_AY_DI_16();
				m68040_do_fsave(addr, -1, 1);
				break;

			case 7: //
				switch (reg)
				{
					case 1:     // (abs32)
					{
						addr = EA_AL_32();
						m68040_do_fsave(addr, -1, 1);
						break;
					}
					case 2:     // (d16, PC)
					{
						addr = EA_PCDI_16();
						m68040_do_fsave(addr, -1, 1);
						break;
					}
					default:
						fatalerror("M68kFPU: FSAVE unhandled mode %d reg %d at %x\n", mode, reg, m_pc);
				}

				break;

			default:
				fatalerror("M68kFPU: FSAVE unhandled mode %d reg %d at %x\n", mode, reg, m_pc);
			}
			break;
		}
		break;

		case 1:     // FRESTORE <ea>
		{
			switch (mode)
			{
			case 2: // (An)
				addr = REG_A()[reg];
				m68040_do_frestore(addr, -1);
				break;

			case 3: // (An)+
				addr = EA_AY_PI_32();
				m68040_do_frestore(addr, reg);
				break;

			case 5: // (D16, An)
				addr = EA_AY_DI_16();
				m68040_do_frestore(addr, -1);
				break;

			case 7: //
				switch (reg)
				{
					case 1:     // (abs32)
					{
						addr = EA_AL_32();
						m68040_do_frestore(addr, -1);
						break;
					}
					case 2:     // (d16, PC)
					{
						addr = EA_PCDI_16();
						m68040_do_frestore(addr, -1);
						break;
					}
					default:
						fatalerror("M68kFPU: FRESTORE unhandled mode %d reg %d at %x\n", mode, reg, m_pc);
				}

				break;

			default:
				fatalerror("M68kFPU: FRESTORE unhandled mode %d reg %d at %x\n", mode, reg, m_pc);
			}
			break;
		}
		break;

		default:    fatalerror("m68040_fpu_op1: unimplemented op %d at %08X\n", (m_ir >> 6) & 0x3, m_pc-2);
	}
}

void m68881_ftrap()
{
	u16 w2  = OPER_I_16();

	// now check the condition
	if (TEST_CONDITION(w2 & 0x3f))
	{
		// trap here
		m68ki_exception_trap(EXCEPTION_TRAPV);
	}
	else    // fall through, requires eating the operand
	{
		switch (m_ir & 0x7)
		{
			case 2: // word operand
				OPER_I_16();
				break;

			case 3: // long word operand
				OPER_I_32();
				break;

			case 4: // no operand
				break;
		}
	}
}
