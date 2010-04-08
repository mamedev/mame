#include <math.h>

#define FPCC_N			0x08000000
#define FPCC_Z			0x04000000
#define FPCC_I			0x02000000
#define FPCC_NAN		0x01000000

#define DOUBLE_INFINITY					U64(0x7ff0000000000000)
#define DOUBLE_EXPONENT					U64(0x7ff0000000000000)
#define DOUBLE_MANTISSA					U64(0x000fffffffffffff)

extern flag floatx80_is_nan( floatx80 a );

// masks for packed dwords, positive k-factor
static UINT32 pkmask2[18] =
{
	0xffffffff, 0, 0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff
};

static UINT32 pkmask3[18] =
{
	0xffffffff, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
};

INLINE double fx80_to_double(floatx80 fx)
{
	UINT64 d;
	double *foo;

	foo = (double *)&d;

	d = floatx80_to_float64(fx);

	return *foo;
}

INLINE floatx80 double_to_fx80(double in)
{
	UINT64 *d;

	d = (UINT64 *)&in;

	return float64_to_floatx80(*d);
}

INLINE floatx80 load_extended_float80(m68ki_cpu_core *m68k, UINT32 ea)
{
	UINT32 d1,d2;
	UINT16 d3;
	floatx80 fp;

	d3 = m68ki_read_16(m68k, ea);
	d1 = m68ki_read_32(m68k, ea+4);
	d2 = m68ki_read_32(m68k, ea+8);

	fp.high = d3;
	fp.low = ((UINT64)d1<<32) | (d2 & 0xffffffff);

	return fp;
}

INLINE void store_extended_float80(m68ki_cpu_core *m68k, UINT32 ea, floatx80 fpr)
{
	m68ki_write_16(m68k, ea+0, fpr.high);
	m68ki_write_16(m68k, ea+2, 0);
	m68ki_write_32(m68k, ea+4, (fpr.low>>32)&0xffffffff);
	m68ki_write_32(m68k, ea+8, fpr.low&0xffffffff);
}

INLINE floatx80 load_pack_float80(m68ki_cpu_core *m68k, UINT32 ea)
{
	UINT32 dw1, dw2, dw3;
	floatx80 result;
	double tmp;
	char str[128], *ch;

	dw1 = m68ki_read_32(m68k, ea);
	dw2 = m68ki_read_32(m68k, ea+4);
	dw3 = m68ki_read_32(m68k, ea+8);

	ch = &str[0];
	if (dw1 & 0x80000000)	// mantissa sign
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
	if (dw1 & 0x40000000)	// exponent sign
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

INLINE void store_pack_float80(m68ki_cpu_core *m68k, UINT32 ea, int k, floatx80 fpr)
{
	UINT32 dw1, dw2, dw3;
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
//          m68k->fpcr |=  (need to set OPERR bit)
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

	m68ki_write_32(m68k, ea, dw1);
	m68ki_write_32(m68k, ea+4, dw2);
	m68ki_write_32(m68k, ea+8, dw3);
}

INLINE void SET_CONDITION_CODES(m68ki_cpu_core *m68k, floatx80 reg)
{
	UINT64 *regi;

	regi = (UINT64 *)&reg;

	REG_FPSR &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (reg.high & 0x8000)
	{
		REG_FPSR |= FPCC_N;
	}

	// zero flag
	if (((reg.high & 0x7fff) == 0) && ((reg.low<<1) == 0))
	{
		REG_FPSR |= FPCC_Z;
	}

	// infinity flag
	if (((reg.high & 0x7fff) == 0x7fff) && ((reg.low<<1) == 0))
	{
		REG_FPSR |= FPCC_I;
	}

	// NaN flag
	if (floatx80_is_nan(reg))
	{
		REG_FPSR |= FPCC_NAN;
	}
}

INLINE int TEST_CONDITION(m68ki_cpu_core *m68k, int condition)
{
	int n = (REG_FPSR & FPCC_N) != 0;
	int z = (REG_FPSR & FPCC_Z) != 0;
	int nan = (REG_FPSR & FPCC_NAN) != 0;
	int r = 0;
	switch (condition)
	{
		case 0x10:
		case 0x00:		return 0;					// False

		case 0x11:
		case 0x01:		return (z);					// Equal

		case 0x12:
		case 0x02:		return (!(nan || z || n));			// Greater Than

		case 0x13:
		case 0x03:		return (z || !(nan || n));			// Greater or Equal

		case 0x14:
		case 0x04:		return (n && !(nan || z));			// Less Than

		case 0x15:
		case 0x05:		return (z || (n && !nan));			// Less Than or Equal

		case 0x1a:
		case 0x0a:		return (nan || !(n || z));			// Not Less Than or Equal

		case 0x1b:
		case 0x0b:		return (nan || z || !n);			// Not Less Than

		case 0x1c:
		case 0x0c:		return (nan || (n && !z));			// Not Greater or Equal Than

		case 0x1d:
		case 0x0d:		return (nan || z || n);				// Not Greater Than

		case 0x1e:
		case 0x0e:		return (!z);					// Not Equal

		case 0x1f:
		case 0x0f:		return 1;					// True

		default:		fatalerror("M68kFPU: test_condition: unhandled condition %02X\n", condition);
	}

	return r;
}

static UINT8 READ_EA_8(m68ki_cpu_core *m68k, int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			return REG_D[reg];
		}
		case 2: 	// (An)
		{
			UINT32 ea = REG_A[reg];
			return m68ki_read_8(m68k, ea);
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_8(m68k);
			return m68ki_read_8(m68k, ea);
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_8(m68k);
			return m68ki_read_8(m68k, ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:		// (xxx).W
				{
					UINT32 ea = (UINT32)OPER_I_16(m68k);
					return m68ki_read_8(m68k, ea);
				}
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					return m68ki_read_8(m68k, ea);
				}
				case 4:		// #<data>
				{
					return  OPER_I_8(m68k);
				}
				default:	fatalerror("M68kFPU: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
}

static UINT16 READ_EA_16(m68ki_cpu_core *m68k, int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			return (UINT16)(REG_D[reg]);
		}
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			return m68ki_read_16(m68k, ea);
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_16(m68k);
			return m68ki_read_16(m68k, ea);
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_16(m68k);
			return m68ki_read_16(m68k, ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:		// (xxx).W
				{
					UINT32 ea = (UINT32)OPER_I_16(m68k);
					return m68ki_read_16(m68k, ea);
				}
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					return m68ki_read_16(m68k, ea);
				}
				case 4:		// #<data>
				{
					return OPER_I_16(m68k);
				}

				default:	fatalerror("M68kFPU: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
}

static UINT32 READ_EA_32(m68ki_cpu_core *m68k, int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			return REG_D[reg];
		}
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			return m68ki_read_32(m68k, ea);
		}
		case 3:		// (An)+
		{
			UINT32 ea = EA_AY_PI_32(m68k);
			return m68ki_read_32(m68k, ea);
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_32(m68k);
			return m68ki_read_32(m68k, ea);
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_32(m68k);
			return m68ki_read_32(m68k, ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 0:		// (xxx).W
				{
					UINT32 ea = (UINT32)OPER_I_16(m68k);
					return m68ki_read_32(m68k, ea);
				}
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					return m68ki_read_32(m68k, ea);
				}
				case 2:		// (d16, PC)
				{
					UINT32 ea = EA_PCDI_32(m68k);
					return m68ki_read_32(m68k, ea);
				}
				case 4:		// #<data>
				{
					return  OPER_I_32(m68k);
				}
				default:	fatalerror("M68kFPU: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
	return 0;
}

static UINT64 READ_EA_64(m68ki_cpu_core *m68k, int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	UINT32 h1, h2;

	switch (mode)
	{
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			h1 = m68ki_read_32(m68k, ea+0);
			h2 = m68ki_read_32(m68k, ea+4);
			return  (UINT64)(h1) << 32 | (UINT64)(h2);
		}
		case 3:		// (An)+
		{
			UINT32 ea = REG_A[reg];
			REG_A[reg] += 8;
			h1 = m68ki_read_32(m68k, ea+0);
			h2 = m68ki_read_32(m68k, ea+4);
			return  (UINT64)(h1) << 32 | (UINT64)(h2);
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_32(m68k);
			h1 = m68ki_read_32(m68k, ea+0);
			h2 = m68ki_read_32(m68k, ea+4);
			return  (UINT64)(h1) << 32 | (UINT64)(h2);
		}
		case 7:
		{
			switch (reg)
			{
				case 4:		// #<data>
				{
					h1 = OPER_I_32(m68k);
					h2 = OPER_I_32(m68k);
					return  (UINT64)(h1) << 32 | (UINT64)(h2);
				}
				case 2:		// (d16, PC)
				{
					UINT32 ea = EA_PCDI_32(m68k);
					h1 = m68ki_read_32(m68k, ea+0);
					h2 = m68ki_read_32(m68k, ea+4);
					return  (UINT64)(h1) << 32 | (UINT64)(h2);
				}
				default:	fatalerror("M68kFPU: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
}


static floatx80 READ_EA_FPE(m68ki_cpu_core *m68k, int ea)
{
	floatx80 fpr;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			fpr = load_extended_float80(m68k, ea);
			break;
		}

		case 3:		// (An)+
		{
			UINT32 ea = REG_A[reg];
			REG_A[reg] += 12;
			fpr = load_extended_float80(m68k, ea);
			break;
		}

		case 7:	// extended modes
		{
			switch (reg)
			{
				case 2:	// (d16, PC)
					{
						UINT32 ea = EA_PCDI_32(m68k);
						fpr = load_extended_float80(m68k, ea);
					}
					break;

				case 3:	// (d16,PC,Dx.w)
					{
						UINT32 ea = EA_PCIX_32(m68k);
						fpr = load_extended_float80(m68k, ea);
					}
					break;

				default:
					fatalerror("M68kFPU: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
					break;
			}
		}
		break;

		default:	fatalerror("M68kFPU: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC); break;
	}

	return fpr;
}

static floatx80 READ_EA_PACK(m68ki_cpu_core *m68k, int ea)
{
	floatx80 fpr;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			fpr = load_pack_float80(m68k, ea);
			break;
		}

		case 3:		// (An)+
		{
			UINT32 ea = REG_A[reg];
			REG_A[reg] += 12;
			fpr = load_pack_float80(m68k, ea);
			break;
		}

		case 7:	// extended modes
		{
			switch (reg)
			{
				case 3:	// (d16,PC,Dx.w)
					{
						UINT32 ea = EA_PCIX_32(m68k);
						fpr = load_pack_float80(m68k, ea);
					}
					break;

				default:
					fatalerror("M68kFPU: READ_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
					break;
			}
		}
		break;

		default:	fatalerror("M68kFPU: READ_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC); break;
	}

	return fpr;
}

static void WRITE_EA_8(m68ki_cpu_core *m68k, int ea, UINT8 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			REG_D[reg] = data;
			break;
		}
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			m68ki_write_8(m68k, ea, data);
			break;
		}
		case 3:		// (An)+
		{
			UINT32 ea = EA_AY_PI_8(m68k);
			m68ki_write_8(m68k, ea, data);
			break;
		}
		case 4:		// -(An)
		{
			UINT32 ea = EA_AY_PD_8(m68k);
			m68ki_write_8(m68k, ea, data);
			break;
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_8(m68k);
			m68ki_write_8(m68k, ea, data);
			break;
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_8(m68k);
			m68ki_write_8(m68k, ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).B
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					m68ki_write_8(m68k, ea, data);
					break;
				}
				case 2:		// (d16, PC)
				{
					UINT32 ea = EA_PCDI_16(m68k);
					m68ki_write_8(m68k, ea, data);
					break;
				}
				default:	fatalerror("M68kFPU: WRITE_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: WRITE_EA_8: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
	}
}

static void WRITE_EA_16(m68ki_cpu_core *m68k, int ea, UINT16 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			REG_D[reg] = data;
			break;
		}
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			m68ki_write_16(m68k, ea, data);
			break;
		}
		case 3:		// (An)+
		{
			UINT32 ea = EA_AY_PI_16(m68k);
			m68ki_write_16(m68k, ea, data);
			break;
		}
		case 4:		// -(An)
		{
			UINT32 ea = EA_AY_PD_16(m68k);
			m68ki_write_16(m68k, ea, data);
			break;
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_16(m68k);
			m68ki_write_16(m68k, ea, data);
			break;
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_16(m68k);
			m68ki_write_16(m68k, ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).W
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					m68ki_write_16(m68k, ea, data);
					break;
				}
				case 2:		// (d16, PC)
				{
					UINT32 ea = EA_PCDI_16(m68k);
					m68ki_write_16(m68k, ea, data);
					break;
				}
				default:	fatalerror("M68kFPU: WRITE_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: WRITE_EA_16: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
	}
}

static void WRITE_EA_32(m68ki_cpu_core *m68k, int ea, UINT32 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			REG_D[reg] = data;
			break;
		}
		case 1:		// An
		{
			REG_A[reg] = data;
			break;
		}
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			m68ki_write_32(m68k, ea, data);
			break;
		}
		case 3:		// (An)+
		{
			UINT32 ea = EA_AY_PI_32(m68k);
			m68ki_write_32(m68k, ea, data);
			break;
		}
		case 4:		// -(An)
		{
			UINT32 ea = EA_AY_PD_32(m68k);
			m68ki_write_32(m68k, ea, data);
			break;
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_32(m68k);
			m68ki_write_32(m68k, ea, data);
			break;
		}
		case 6:		// (An) + (Xn) + d8
		{
			UINT32 ea = EA_AY_IX_32(m68k);
			m68ki_write_32(m68k, ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16(m68k);
					UINT32 d2 = OPER_I_16(m68k);
					UINT32 ea = (d1 << 16) | d2;
					m68ki_write_32(m68k, ea, data);
					break;
				}
				case 2:		// (d16, PC)
				{
					UINT32 ea = EA_PCDI_32(m68k);
					m68ki_write_32(m68k, ea, data);
					break;
				}
				default:	fatalerror("M68kFPU: WRITE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("M68kFPU: WRITE_EA_32: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
	}
}

static void WRITE_EA_64(m68ki_cpu_core *m68k, int ea, UINT64 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			UINT32 ea = REG_A[reg];
			m68ki_write_32(m68k, ea, (UINT32)(data >> 32));
			m68ki_write_32(m68k, ea+4, (UINT32)(data));
			break;
		}
		case 4:		// -(An)
		{
			UINT32 ea;
			REG_A[reg] -= 8;
			ea = REG_A[reg];
			m68ki_write_32(m68k, ea+0, (UINT32)(data >> 32));
			m68ki_write_32(m68k, ea+4, (UINT32)(data));
			break;
		}
		case 5:		// (d16, An)
		{
			UINT32 ea = EA_AY_DI_32(m68k);
			m68ki_write_32(m68k, ea+0, (UINT32)(data >> 32));
			m68ki_write_32(m68k, ea+4, (UINT32)(data));
			break;
		}
		default:	fatalerror("M68kFPU: WRITE_EA_64: unhandled mode %d, reg %d, data %08X%08X at %08X\n", mode, reg, (UINT32)(data >> 32), (UINT32)(data), REG_PC);
	}
}

static void WRITE_EA_FPE(m68ki_cpu_core *m68k, int ea, floatx80 fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			UINT32 ea;
			ea = REG_A[reg];
			store_extended_float80(m68k, ea, fpr);
			break;
		}

		case 3:		// (An)+
		{
			UINT32 ea;
			ea = REG_A[reg];
			store_extended_float80(m68k, ea, fpr);
			REG_A[reg] += 12;
			break;
		}

		case 4:		// -(An)
		{
			UINT32 ea;
			REG_A[reg] -= 12;
			ea = REG_A[reg];
			store_extended_float80(m68k, ea, fpr);
			break;
		}

		case 7:
		{
			switch (reg)
			{
				default:	fatalerror("M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
			}
		}
		default:	fatalerror("M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
	}
}

static void WRITE_EA_PACK(m68ki_cpu_core *m68k, int ea, int k, floatx80 fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			UINT32 ea;
			ea = REG_A[reg];
			store_pack_float80(m68k, ea, k, fpr);
			break;
		}

		case 3:		// (An)+
		{
			UINT32 ea;
			ea = REG_A[reg];
			store_pack_float80(m68k, ea, k, fpr);
			REG_A[reg] += 12;
			break;
		}

		case 4:		// -(An)
		{
			UINT32 ea;
			REG_A[reg] -= 12;
			ea = REG_A[reg];
			store_pack_float80(m68k, ea, k, fpr);
			break;
		}

		case 7:
		{
			switch (reg)
			{
				default:	fatalerror("M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
			}
		}
		default:	fatalerror("M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
	}
}

static void fpgen_rm_reg(m68ki_cpu_core *m68k, UINT16 w2)
{
	int ea = m68k->ir & 0x3f;
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
			case 0:		// Long-Word Integer
			{
				INT32 d = READ_EA_32(m68k, ea);
				source = int32_to_floatx80(d);
				break;
			}
			case 1:		// Single-precision Real
			{
				UINT32 d = READ_EA_32(m68k, ea);
				source = float32_to_floatx80(d);
				break;
			}
			case 2:		// Extended-precision Real
			{
				source = READ_EA_FPE(m68k, ea);
				break;
			}
			case 3:		// Packed-decimal Real
			{
				source = READ_EA_PACK(m68k, ea);
				break;
			}
			case 4:		// Word Integer
			{
				INT16 d = READ_EA_16(m68k, ea);
				source = int32_to_floatx80((INT32)d);
				break;
			}
			case 5:		// Double-precision Real
			{
				UINT64 d = READ_EA_64(m68k, ea);

				source = float64_to_floatx80(d);
				break;
			}
			case 6:		// Byte Integer
			{
				INT8 d = READ_EA_8(m68k, ea);
				source = int32_to_floatx80((INT32)d);
				break;
			}
			case 7:		// FMOVECR load from constant ROM
			{
				switch (w2 & 0x7f)
				{
					case 0x0:	// Pi
						source.high = 0x4000;
						source.low = U64(0xc90fdaa22168c235);
						break;

					case 0xb:	// log10(2)
						source.high = 0x3ffd;
						source.low = U64(0x9a209a84fbcff798);
						break;

					case 0xc:	// e
						source.high = 0x4000;
						source.low = U64(0xadf85458a2bb4a9b);
						break;

					case 0xd:	// log2(e)
						source.high = 0x3fff;
						source.low = U64(0xb8aa3b295c17f0bc);
						break;

					case 0xe:	// log10(e)
						source.high = 0x3ffd;
						source.low = U64(0xde5bd8a937287195);
						break;

					case 0xf:	// 0.0
						source = int32_to_floatx80((INT32)0);
						break;

					case 0x30:	// ln(2)
						source.high = 0x3ffe;
						source.low = U64(0xb17217f7d1cf79ac);
						break;

					case 0x31:	// ln(10)
						source.high = 0x4000;
						source.low = U64(0x935d8dddaaa8ac17);
						break;

					case 0x32:	// 1 (or 100?  manuals are unclear, but 1 would make more sense)
						source = int32_to_floatx80((INT32)1);
						break;

					case 0x33:	// 10^1
						source = int32_to_floatx80((INT32)10);
						break;

					case 0x34:	// 10^2
						source = int32_to_floatx80((INT32)10*10);
						break;

					default:
						fatalerror("fmove_rm_reg: unknown constant ROM offset %x at %08x\n", w2&0x7f, REG_PC-4);
						break;
				}

				// handle it right here, the usual opmode bits aren't valid in the FMOVECR case
				REG_FP[dst] = source;
				m68k->remaining_cycles -= 4;
				return;
			}
			default:	fatalerror("fmove_rm_reg: invalid source specifier %x at %08X\n", src, REG_PC-4);
		}
	}
	else
	{
		source = REG_FP[src];
	}



	switch (opmode)
	{
		case 0x00:		// FMOVE
		{
			REG_FP[dst] = source;
			m68k->remaining_cycles -= 4;
			break;
		}
		case 0x01:		// FINT
		{
			INT32 temp;
			temp = floatx80_to_int32(source);
			REG_FP[dst] = int32_to_floatx80(temp);
			break;
		}
		case 0x03:		// FINTRZ
		{
			INT32 temp;
			temp = floatx80_to_int32_round_to_zero(source);
			REG_FP[dst] = int32_to_floatx80(temp);
			break;
		}
		case 0x04:		// FSQRT
		{
			REG_FP[dst] = floatx80_sqrt(source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 109;
			break;
		}
		case 0x18:		// FABS
		{
			REG_FP[dst] = source;
			REG_FP[dst].high &= 0x7fff;
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 3;
			break;
		}
		case 0x1a:		// FNEG
		{
			REG_FP[dst] = source;
			REG_FP[dst].high ^= 0x8000;
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 3;
			break;
		}
		case 0x20:		// FDIV
		{
			REG_FP[dst] = floatx80_div(REG_FP[dst], source);
			m68k->remaining_cycles -= 43;
			break;
		}
		case 0x22:		// FADD
		{
			REG_FP[dst] = floatx80_add(REG_FP[dst], source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 9;
			break;
		}
		case 0x23:		// FMUL
		{
			REG_FP[dst] = floatx80_mul(REG_FP[dst], source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 11;
			break;
		}
		case 0x25:		// FREM
		{
			REG_FP[dst] = floatx80_rem(REG_FP[dst], source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 43;	// guess
			break;
		}
		case 0x28:		// FSUB
		{
			REG_FP[dst] = floatx80_sub(REG_FP[dst], source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 9;
			break;
		}
		case 0x38:		// FCMP
		{
			floatx80 res;
			res = floatx80_sub(REG_FP[dst], source);
			SET_CONDITION_CODES(m68k, res);
			m68k->remaining_cycles -= 7;
			break;
		}
		case 0x3a:		// FTST
		{
			floatx80 res;
			res = source;
			SET_CONDITION_CODES(m68k, res);
			m68k->remaining_cycles -= 7;
			break;
		}

		default:	fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, REG_PC-4);
	}
}

static void fmove_reg_mem(m68ki_cpu_core *m68k, UINT16 w2)
{
	int ea = m68k->ir & 0x3f;
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	int k = (w2 & 0x7f);

	switch (dst)
	{
		case 0:		// Long-Word Integer
		{
			INT32 d = (INT32)floatx80_to_int32(REG_FP[src]);
			WRITE_EA_32(m68k, ea, d);
			break;
		}
		case 1:		// Single-precision Real
		{
			UINT32 d = floatx80_to_float32(REG_FP[src]);
			WRITE_EA_32(m68k, ea, d);
			break;
		}
		case 2:		// Extended-precision Real
		{
			WRITE_EA_FPE(m68k, ea, REG_FP[src]);
			break;
		}
		case 3:		// Packed-decimal Real with Static K-factor
		{
			// sign-extend k
			k = (k & 0x40) ? (k | 0xffffff80) : (k & 0x7f);
			WRITE_EA_PACK(m68k, ea, k, REG_FP[src]);
			break;
		}
		case 4:		// Word Integer
		{
			WRITE_EA_16(m68k, ea, (INT16)floatx80_to_int32(REG_FP[src]));
			break;
		}
		case 5:		// Double-precision Real
		{
			UINT64 d;

			d = floatx80_to_float64(REG_FP[src]);

			WRITE_EA_64(m68k, ea, d);
			break;
		}
		case 6:		// Byte Integer
		{
			WRITE_EA_8(m68k, ea, (INT8)floatx80_to_int32(REG_FP[src]));
			break;
		}
		case 7:		// Packed-decimal Real with Dynamic K-factor
		{
			WRITE_EA_PACK(m68k, ea, REG_D[k>>4], REG_FP[src]);
			break;
		}
	}

	m68k->remaining_cycles -= 12;
}

static void fmove_fpcr(m68ki_cpu_core *m68k, UINT16 w2)
{
	int ea = m68k->ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int reg = (w2 >> 10) & 0x7;

	if (dir)	// From system control reg to <ea>
	{
		if (reg & 4) WRITE_EA_32(m68k, ea, REG_FPCR);
		if (reg & 2) WRITE_EA_32(m68k, ea, REG_FPSR);
		if (reg & 1) WRITE_EA_32(m68k, ea, REG_FPIAR);
	}
	else		// From <ea> to system control reg
	{
		if (reg & 4) REG_FPCR = READ_EA_32(m68k, ea);
		if (reg & 2) REG_FPSR = READ_EA_32(m68k, ea);
		if (reg & 1) REG_FPIAR = READ_EA_32(m68k, ea);
	}

	m68k->remaining_cycles -= 10;
}

static void fmovem(m68ki_cpu_core *m68k, UINT16 w2)
{
	int i;
	int ea = m68k->ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int mode = (w2 >> 11) & 0x3;
	int reglist = w2 & 0xff;

	if (dir)	// From FP regs to mem
	{
		switch (mode)
		{
			case 0:		// Static register list, predecrement addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						WRITE_EA_FPE(m68k, ea, REG_FP[i]);
						m68k->remaining_cycles -= 2;
					}
				}
				break;
			}

			default:	fatalerror("040fpu0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC-4);
		}
	}
	else		// From mem to FP regs
	{
		switch (mode)
		{
			case 2:		// Static register list, postincrement addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						REG_FP[7-i] = READ_EA_FPE(m68k, ea);
						m68k->remaining_cycles -= 2;
					}
				}
				break;
			}

			default:	fatalerror("040fpu0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC-4);
		}
	}
}

static void fbcc16(m68ki_cpu_core *m68k)
{
	INT32 offset;
	int condition = m68k->ir & 0x3f;

	offset = (INT16)(OPER_I_16(m68k));

	// TODO: condition and jump!!!
	if (TEST_CONDITION(m68k, condition))
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(m68k, offset-2);
	}

	m68k->remaining_cycles -= 7;
}

static void fbcc32(m68ki_cpu_core *m68k)
{
	INT32 offset;
	int condition = m68k->ir & 0x3f;

	offset = OPER_I_32(m68k);

	// TODO: condition and jump!!!
	if (TEST_CONDITION(m68k, condition))
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(m68k, offset-4);
	}

	m68k->remaining_cycles -= 7;
}


void m68040_fpu_op0(m68ki_cpu_core *m68k)
{
	m68k->fpu_just_reset = 0;

	switch ((m68k->ir >> 6) & 0x3)
	{
		case 0:
		{
			UINT16 w2 = OPER_I_16(m68k);
			switch ((w2 >> 13) & 0x7)
			{
				case 0x0:	// FPU ALU FP, FP
				case 0x2:	// FPU ALU ea, FP
				{
					fpgen_rm_reg(m68k, w2);
					break;
				}

				case 0x3:	// FMOVE FP, ea
				{
					fmove_reg_mem(m68k, w2);
					break;
				}

				case 0x4:	// FMOVEM ea, FPCR
				case 0x5:	// FMOVEM FPCR, ea
				{
					fmove_fpcr(m68k, w2);
					break;
				}

				case 0x6:	// FMOVEM ea, list
				case 0x7:	// FMOVEM list, ea
				{
					fmovem(m68k, w2);
					break;
				}

				default:	fatalerror("M68kFPU: unimplemented subop %d at %08X\n", (w2 >> 13) & 0x7, REG_PC-4);
			}
			break;
		}

		case 2:		// FBcc disp16
		{
			fbcc16(m68k);
			break;
		}
		case 3:		// FBcc disp32
		{
			fbcc32(m68k);
			break;
		}

		default:	fatalerror("M68kFPU: unimplemented main op %d\n", (m68k->ir >> 6)	& 0x3);
	}
}

static void perform_fsave(m68ki_cpu_core *m68k, UINT32 addr, int inc)
{
	if (inc)
	{
		// 68881 IDLE, version 0x1f
		m68ki_write_32(m68k, addr, 0x1f180000);
		m68ki_write_32(m68k, addr+4, 0);
		m68ki_write_32(m68k, addr+8, 0);
		m68ki_write_32(m68k, addr+12, 0);
		m68ki_write_32(m68k, addr+16, 0);
		m68ki_write_32(m68k, addr+20, 0);
		m68ki_write_32(m68k, addr+24, 0x70000000);
	}
	else
	{
		m68ki_write_32(m68k, addr, 0x70000000);
		m68ki_write_32(m68k, addr-4, 0);
		m68ki_write_32(m68k, addr-8, 0);
		m68ki_write_32(m68k, addr-12, 0);
		m68ki_write_32(m68k, addr-16, 0);
		m68ki_write_32(m68k, addr-20, 0);
		m68ki_write_32(m68k, addr-24, 0x1f180000);
	}
}

// FRESTORE on a NULL frame reboots the FPU - all registers to NaN, the 3 status regs to 0
static void do_frestore_null(m68ki_cpu_core *m68k)
{
	int i;

	REG_FPCR = 0;
	REG_FPSR = 0;
	REG_FPIAR = 0;
	for (i = 0; i < 8; i++)
	{
		REG_FP[i].high = 0x7fff;
		REG_FP[i].low = U64(0xffffffffffffffff);
	}

	// Mac IIci at 408458e6 wants an FSAVE of a just-restored NULL frame to also be NULL
	// The PRM says it's possible to generate a NULL frame, but not how/when/why.  (need the 68881/68882 manual!)
	m68k->fpu_just_reset = 1;
}

void m68040_fpu_op1(m68ki_cpu_core *m68k)
{
	int ea = m68k->ir & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	UINT32 addr, temp;

	switch ((m68k->ir >> 6) & 0x3)
	{
		case 0:		// FSAVE <ea>
		{
			switch (mode)
			{
				case 3:	// (An)+
		    			addr = EA_AY_PI_32(m68k);

					if (m68k->fpu_just_reset)
					{
						m68ki_write_32(m68k, addr, 0);
					}
					else
					{
						// we normally generate an IDLE frame
						REG_A[reg] += 6*4;
						perform_fsave(m68k, addr, 1);
					}
					break;

				case 4: // -(An)
		    			addr = EA_AY_PD_32(m68k);

					if (m68k->fpu_just_reset)
					{
						m68ki_write_32(m68k, addr, 0);
					}
					else
					{
						// we normally generate an IDLE frame
						REG_A[reg] -= 6*4;
						perform_fsave(m68k, addr, 0);
					}
					break;

				default:
					fatalerror("M68kFPU: FSAVE unhandled mode %d reg %d at %x\n", mode, reg, REG_PC);
			}
			break;
		}
		break;

		case 1:		// FRESTORE <ea>
		{
			switch (mode)
			{
				case 2: // (An)
					addr = REG_A[reg];
					temp = m68ki_read_32(m68k, addr);

					// check for NULL frame
					if (temp & 0xff000000)
					{
						// we don't handle non-NULL frames and there's no pre/post inc/dec to do here
						m68k->fpu_just_reset = 0;
					}
					else
					{
						do_frestore_null(m68k);
					}
					break;

				case 3:	// (An)+
		    			addr = EA_AY_PI_32(m68k);
					temp = m68ki_read_32(m68k, addr);

					// check for NULL frame
					if (temp & 0xff000000)
					{
						m68k->fpu_just_reset = 0;

						// how about an IDLE frame?
						if ((temp & 0x00ff0000) == 0x00180000)
						{
							REG_A[reg] += 6*4;
						} // check UNIMP
						else if ((temp & 0x00ff0000) == 0x00380000)
						{
							REG_A[reg] += 14*4;
						} // check BUSY
						else if ((temp & 0x00ff0000) == 0x00b40000)
						{
							REG_A[reg] += 45*4;
						}
					}
					else
					{
						do_frestore_null(m68k);
					}
					break;

				default:
					fatalerror("M68kFPU: FRESTORE unhandled mode %d reg %d at %x\n", mode, reg, REG_PC);
			}
			break;
		}
		break;

		default:	fatalerror("m68040_fpu_op1: unimplemented op %d at %08X\n", (m68k->ir >> 6) & 0x3, REG_PC-2);
	}
}



