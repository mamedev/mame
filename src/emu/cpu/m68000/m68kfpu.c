#include <math.h>

#define FPCC_N			0x08000000
#define FPCC_Z			0x04000000
#define FPCC_I			0x02000000
#define FPCC_NAN		0x01000000

#define DOUBLE_INFINITY					U64(0x7ff0000000000000)
#define DOUBLE_EXPONENT					U64(0x7ff0000000000000)
#define DOUBLE_MANTISSA					U64(0x000fffffffffffff)

extern flag floatx80_is_nan( floatx80 a );

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
				case 3:	// (d16,PC,Dx.w)
					{
						UINT32 ea = EA_PCIX_32(m68k);
						fpr = load_extended_float80(m68k, ea);
					}
					break;

				default:
					fatalerror("M68kFPU: READ_EA_FPE0: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
					break;
			}
		}
		break;

		default:	fatalerror("M68kFPU: READ_EA_FPE1: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC); break;
	}

	return fpr;
}

INLINE void store_extended_float80(m68ki_cpu_core *m68k, UINT32 ea, floatx80 fpr)
{
	m68ki_write_16(m68k, ea+0, fpr.high);
	m68ki_write_16(m68k, ea+2, 0);
	m68ki_write_32(m68k, ea+4, (fpr.low>>32)&0xffffffff);
	m68ki_write_32(m68k, ea+8, fpr.low&0xffffffff);
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
				fatalerror("fpgen_rm_reg: packed-decimal real load unimplemented at %08X\n", REG_PC-4);
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
	//int kfactor = w2 & 0x7f;

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
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
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
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
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
		switch (reg)
		{
			case 1:		WRITE_EA_32(m68k, ea, REG_FPIAR); break;
			case 2:		WRITE_EA_32(m68k, ea, REG_FPSR); break;
			case 4:		WRITE_EA_32(m68k, ea, REG_FPCR); break;
			default:	fatalerror("fmove_fpcr: unknown reg %d, dir %d\n", reg, dir);
		}
	}
	else		// From <ea> to system control reg
	{
		switch (reg)
		{
			case 1:		REG_FPIAR = READ_EA_32(m68k, ea); break;
			case 2:		REG_FPSR = READ_EA_32(m68k, ea); break;
			case 4:		REG_FPCR = READ_EA_32(m68k, ea); break;
			default:	fatalerror("fmove_fpcr: unknown reg %d, dir %d\n", reg, dir);
		}
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

				case 0x4:	// FMOVE ea, FPCR
				case 0x5:	// FMOVE FPCR, ea
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

void m68040_fpu_op1(m68ki_cpu_core *m68k)
{
	int ea = m68k->ir & 0x3f;

	switch ((m68k->ir >> 6) & 0x3)
	{
		case 0:		// FSAVE <ea>
		{
			WRITE_EA_32(m68k, ea, 0x00000000);
			// TODO: correct state frame
			break;
		}

		case 1:		// FRESTORE <ea>
		{
			READ_EA_32(m68k, ea);
			// TODO: correct state frame
			break;
		}

		default:	fatalerror("m68040_fpu_op1: unimplemented op %d at %08X\n", (m68k->ir >> 6) & 0x3, REG_PC-2);
	}
}



