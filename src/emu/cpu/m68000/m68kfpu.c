#include <math.h>

#define FPCC_N			0x08000000
#define FPCC_Z			0x04000000
#define FPCC_I			0x02000000
#define FPCC_NAN		0x01000000

#define DOUBLE_INFINITY					U64(0x7ff0000000000000)
#define DOUBLE_EXPONENT					U64(0x7ff0000000000000)
#define DOUBLE_MANTISSA					U64(0x000fffffffffffff)

INLINE void SET_CONDITION_CODES(m68ki_cpu_core *m68k, fp_reg reg)
{
	REG_FPSR &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (reg.i & U64(0x8000000000000000))
	{
		REG_FPSR |= FPCC_N;
	}

	// zero flag
	if ((reg.i & U64(0x7fffffffffffffff)) == 0)
	{
		REG_FPSR |= FPCC_Z;
	}

	// infinity flag
	if ((reg.i & U64(0x7fffffffffffffff)) == DOUBLE_INFINITY)
	{
		REG_FPSR |= FPCC_I;
	}

	// NaN flag
	if (((reg.i & DOUBLE_EXPONENT) == DOUBLE_EXPONENT) && ((reg.i & DOUBLE_MANTISSA) != 0))
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
		case 0x00:		return 0;							// False
		case 0x01:		return (z);							// Equal
		case 0x0e:		return (!z);						// Not Equal
		case 0x0f:		return 1;							// True
		case 0x12:		return (!(nan || z || n));			// Greater Than
		case 0x13:		return (z || !(nan || n));			// Greater or Equal
		case 0x14:		return (n && !(nan || z));			// Less Than
		case 0x15:		return (z || (n && !nan));			// Less Than or Equal
		case 0x1a:		return (nan || !(n || z));			// Not Less Than or Equal
		case 0x1b:		return (nan || z || !n);			// Not Less Than
		case 0x1c:		return (nan || (n && !z));			// Not Greater or Equal Than
		case 0x1d:		return (nan || z || n);				// Not Greater Than

		default:		fatalerror("M68040: test_condition: unhandled condition %02X\n", condition);
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
				default:	fatalerror("MC68040: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
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

				default:	fatalerror("MC68040: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
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
				default:	fatalerror("MC68040: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
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
				default:	fatalerror("MC68040: WRITE_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_8: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
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
				default:	fatalerror("MC68040: WRITE_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_16: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
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
				default:	fatalerror("MC68040: WRITE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_32: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
	}
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
				default:	fatalerror("MC68040: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
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
		default:	fatalerror("MC68040: WRITE_EA_64: unhandled mode %d, reg %d, data %08X%08X at %08X\n", mode, reg, (UINT32)(data >> 32), (UINT32)(data), REG_PC);
	}
}

static fp_reg READ_EA_FPE(m68ki_cpu_core *m68k, int ea)
{
	fp_reg r;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	// TODO: convert to extended floating-point!

	switch (mode)
	{
		case 3:		// (An)+
		{
			UINT32 d1,d2,d3;
			UINT32 ea = REG_A[reg];
			REG_A[reg] += 12;
			d1 = m68ki_read_32(m68k, ea+0);
			d2 = m68ki_read_32(m68k, ea+4);
			d3 = m68ki_read_32(m68k, ea+8);
			r.i = (UINT64)(d1) << 32 | (UINT64)(d2);
			break;
		}
		default:	fatalerror("MC68040: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
	}

	return r;
}

static void WRITE_EA_FPE(m68ki_cpu_core *m68k, int ea, fp_reg fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	// TODO: convert to extended floating-point!

	switch (mode)
	{
		case 4:		// -(An)
		{
			UINT32 ea;
			REG_A[reg] -= 12;
			ea = REG_A[reg];
			m68ki_write_32(m68k, ea+0, (UINT32)(fpr.i >> 32));
			m68ki_write_32(m68k, ea+4, (UINT32)(fpr.i));
			m68ki_write_32(m68k, ea+8, 0);
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_FPE: unhandled mode %d, reg %d, data %f at %08X\n", mode, reg, fpr.f, REG_PC);
	}
}


static void fpgen_rm_reg(m68ki_cpu_core *m68k, UINT16 w2)
{
	int ea = m68k->ir & 0x3f;
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	double source;

	if (rm)
	{
		switch (src)
		{
			case 0:		// Long-Word Integer
			{
				INT32 d = READ_EA_32(m68k, ea);
				source = (double)(d);
				break;
			}
			case 1:		// Single-precision Real
			{
				UINT32 d = READ_EA_32(m68k, ea);
				source = (double)(*(float*)&d);
				break;
			}
			case 2:		// Extended-precision Real
			{
				fatalerror("fpgen_rm_reg: extended-precision real load unimplemented at %08X\n", REG_PC-4);
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
				source = (double)(d);
				break;
			}
			case 5:		// Double-precision Real
			{
				UINT64 d = READ_EA_64(m68k, ea);
				source = *(double*)&d;
				break;
			}
			case 6:		// Byte Integer
			{
				INT8 d = READ_EA_8(m68k, ea);
				source = (double)(d);
				break;
			}
			default:	fatalerror("fmove_rm_reg: invalid source specifier at %08X\n", REG_PC-4);
		}
	}
	else
	{
		source = REG_FP[src].f;
	}

	switch (opmode)
	{
		case 0x00:		// FMOVE
		{
			REG_FP[dst].f = source;
			m68k->remaining_cycles -= 4;
			break;
		}
		case 0x04:		// FSQRT
		{
			REG_FP[dst].f = sqrt(source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 109;
			break;
		}
		case 0x18:		// FABS
		{
			REG_FP[dst].f = fabs(source);
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 3;
			break;
		}
		case 0x1a:		// FNEG
		{
			REG_FP[dst].f = -source;
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 3;
			break;
		}
		case 0x20:		// FDIV
		{
			REG_FP[dst].f /= source;
			m68k->remaining_cycles -= 43;
			break;
		}
		case 0x22:		// FADD
		{
			REG_FP[dst].f += source;
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 9;
			break;
		}
		case 0x23:		// FMUL
		{
			REG_FP[dst].f *= source;
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 11;
			break;
		}
		case 0x28:		// FSUB
		{
			REG_FP[dst].f -= source;
			SET_CONDITION_CODES(m68k, REG_FP[dst]);
			m68k->remaining_cycles -= 9;
			break;
		}
		case 0x38:		// FCMP
		{
			fp_reg res;
			res.f = REG_FP[dst].f - source;
			SET_CONDITION_CODES(m68k, res);
			m68k->remaining_cycles -= 7;
			break;
		}
		case 0x3a:		// FTST
		{
			fp_reg res;
			res.f = source;
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
			INT32 d = (INT32)(REG_FP[src].f);
			WRITE_EA_32(m68k, ea, d);
			break;
		}
		case 1:		// Single-precision Real
		{
			float f = (float)(REG_FP[src].f);
			UINT32 d = *(UINT32 *)&f;
			WRITE_EA_32(m68k, ea, d);
			break;
		}
		case 2:		// Extended-precision Real
		{
			fatalerror("fmove_reg_mem: extended-precision real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 3:		// Packed-decimal Real with Static K-factor
		{
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 4:		// Word Integer
		{
			INT16 d = (INT16)(REG_FP[src].f);
			WRITE_EA_16(m68k, ea, d);
			break;
		}
		case 5:		// Double-precision Real
		{
			UINT64 d = REG_FP[src].i;
			WRITE_EA_64(m68k, ea, d);
			break;
		}
		case 6:		// Byte Integer
		{
			INT8 d = (INT16)(REG_FP[src].f);
			WRITE_EA_8(m68k, ea, d);
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

				default:	fatalerror("m68040_fpu_op0: unimplemented subop %d at %08X\n", (w2 >> 13) & 0x7, REG_PC-4);
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

		default:	fatalerror("m68040_fpu_op0: unimplemented main op %d\n", (m68k->ir >> 6)	& 0x3);
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



