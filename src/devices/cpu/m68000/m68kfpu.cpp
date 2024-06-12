// license:BSD-3-Clause
// copyright-holders:Karl Stenerud, R. Belmont

/*
    SoftFloat 3E version, May/June 2024
    - Exception flags now set for all opcodes
    - FREM/FMOD now generate the quotient bits in FPSR, required for SANE to do trigonometry
    - FMOVE of a float to an integer register generates the proper INEXACT exception, required
      for SANE to calculate square roots.
*/

#include <cstdint>

#include "emu.h"
#include "m68kmusashi.h"

#define LOG_FPSR                    (1U << 1)
#define LOG_INSTRUCTIONS            (1U << 2)
#define LOG_INSTRUCTIONS_VERBOSE    (1U << 3)
#define LOG_LOADSTORE               (1U << 4)

#define VERBOSE (0)

#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

static constexpr int FPCC_N          = 0x08000000;
static constexpr int FPCC_Z          = 0x04000000;
static constexpr int FPCC_I          = 0x02000000;
static constexpr int FPCC_NAN        = 0x01000000;

static constexpr u32 FPES_INEXDEC       = 0x00000100;
static constexpr u32 FPES_INEXACT       = 0x00000200;
static constexpr u32 FPES_DIVZERO       = 0x00000400;
static constexpr u32 FPES_OVERFLOW      = 0x00000800;
static constexpr u32 FPES_UNDERFLOW     = 0x00001000;
static constexpr u32 FPES_OPERR         = 0x00002000;
static constexpr u32 FPES_SNAN          = 0x00004000;

static constexpr u32 FPAE_INEXACT       = 0x00000008;
static constexpr u32 FPAE_DIVZERO       = 0x00000010;
static constexpr u32 FPAE_OVERFLOW      = 0x00000020;
static constexpr u32 FPAE_UNDERFLOW     = 0x00000040;
static constexpr u32 FPAE_OPERR         = 0x00000010;

static constexpr u32 EXC_ENB_INEXACT    = 0x00000001;
static constexpr u32 EXC_ENB_UNDFLOW    = 0x00000002;
static constexpr u32 EXC_ENB_OVRFLOW    = 0x00000004;

// masks for packed dwords, positive k-factor
const u32 m68000_musashi_device::pkmask2[18] =
{
	0xffffffff, 0, 0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff
};

const u32 m68000_musashi_device::pkmask3[18] =
{
	0xffffffff, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
	0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
};

inline extFloat80_t m68000_musashi_device::load_extended_float80(u32 ea)
{
	u32 d1,d2;
	u16 d3;
	extFloat80_t fp;

	d3 = m68ki_read_16(ea);
	d1 = m68ki_read_32(ea+4);
	d2 = m68ki_read_32(ea+8);

	fp.signExp = d3;
	fp.signif = ((u64)d1<<32) | (d2 & 0xffffffff);

	return fp;
}

inline void m68000_musashi_device::store_extended_float80(u32 ea, extFloat80_t fpr)
{
	m68ki_write_16(ea+0, fpr.signExp);
	m68ki_write_16(ea+2, 0);
	m68ki_write_32(ea+4, (fpr.signif>>32)&0xffffffff);
	m68ki_write_32(ea+8, fpr.signif&0xffffffff);
}

inline extFloat80_t m68000_musashi_device::load_pack_float80(u32 ea)
{
	u32 dw1, dw2, dw3;
	extFloat80_t result;
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

inline void m68000_musashi_device::store_pack_float80(u32 ea, int k, extFloat80_t fpr)
{
	u32 dw1, dw2, dw3;
	char str[128], *ch;
	int i, j, exp;

	dw1 = dw2 = dw3 = 0;
	ch = &str[0];

	snprintf(str, sizeof(str), "%.16e", fx80_to_double(fpr));

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

void m68000_musashi_device::set_condition_codes(extFloat80_t reg)
{
	m_fpsr &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (reg.signExp & 0x8000)
	{
		m_fpsr |= FPCC_N;
	}

	// zero flag
	if (((reg.signExp & 0x7fff) == 0) && ((reg.signif<<1) == 0))
	{
		m_fpsr |= FPCC_Z;
	}

	// infinity flag
	if (((reg.signExp & 0x7fff) == 0x7fff) && ((reg.signif<<1) == 0))
	{
		m_fpsr |= FPCC_I;
	}

	// NaN flag
	if (extFloat80_is_nan(reg))
	{
		m_fpsr |= FPCC_NAN;
	}
}

void m68000_musashi_device::clear_exception_flags()
{
	softfloat_exceptionFlags = 0;
	m_fpsr &= ~(FPES_SNAN | FPES_OPERR | FPES_OVERFLOW | FPES_UNDERFLOW | FPES_DIVZERO | FPAE_DIVZERO | FPAE_INEXACT | FPAE_OPERR | FPAE_OVERFLOW | FPAE_UNDERFLOW | FPES_INEXDEC);
}

void m68000_musashi_device::sync_exception_flags(extFloat80_t op1, extFloat80_t op2, u32 enables)
{
	if (extF80_isSignalingNaN(op1) || extF80_isSignalingNaN(op2))
	{
		m_fpsr |= FPES_SNAN;
	}

	if ((enables & EXC_ENB_INEXACT) && (softfloat_exceptionFlags & softfloat_flag_inexact))
	{
		m_fpsr |= FPES_INEXACT | FPAE_INEXACT;
	}

	if ((enables & EXC_ENB_UNDFLOW) && (softfloat_exceptionFlags & softfloat_flag_underflow))
	{
		m_fpsr |= FPES_UNDERFLOW | FPAE_UNDERFLOW;
	}

	if ((enables & EXC_ENB_OVRFLOW) && (softfloat_exceptionFlags & softfloat_flag_overflow))
	{
		m_fpsr |= FPES_OVERFLOW | FPAE_OVERFLOW;
	}
}

int m68000_musashi_device::test_condition(int condition)
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

s32 m68000_musashi_device::convert_to_int(extFloat80_t source, s32 lowerLimit, s32 upperLimit)
{
	clear_exception_flags();
	s32 result = extF80_to_i32(source, softfloat_roundingMode, true);
	sync_exception_flags(source, source, EXC_ENB_INEXACT);
	if (result < lowerLimit)
	{
		result = lowerLimit;
		m_fpsr |= FPES_INEXACT | FPAE_INEXACT;
	}
	else if (result > upperLimit)
	{
		result = upperLimit;
		m_fpsr |= FPES_INEXACT | FPAE_INEXACT;
	}

	return result;
}

u8 m68000_musashi_device::READ_EA_8(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return REG_D()[reg] & 0xff;
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

u16 m68000_musashi_device::READ_EA_16(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			return (u16)(REG_D()[reg] & 0xffff);
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

u32 m68000_musashi_device::READ_EA_32(int ea)
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

u64 m68000_musashi_device::READ_EA_64(int ea)
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

extFloat80_t m68000_musashi_device::READ_EA_FPE(int mode, int reg, uint32_t di_mode_ea)
{
	extFloat80_t fpr;

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
			fpr = load_extended_float80(di_mode_ea);
			break;
		}
		case 6:     // (An) + (Xn) + d8
		{
			fpr = load_extended_float80(di_mode_ea);
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

				case 4:     // #<data>
					fpr = load_extended_float80(m_pc);
					m_pc += 12;
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

extFloat80_t m68000_musashi_device::READ_EA_PACK(int ea)
{
	extFloat80_t fpr;
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

void m68000_musashi_device::WRITE_EA_8(int ea, u8 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] &= 0xffffff00;
			REG_D()[reg] |= data;
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

void m68000_musashi_device::WRITE_EA_16(int ea, u16 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:     // Dn
		{
			REG_D()[reg] &= 0xffff0000;
			REG_D()[reg] |= data;
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

void m68000_musashi_device::WRITE_EA_32(int ea, u32 data)
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

void m68000_musashi_device::WRITE_EA_64(int ea, u64 data)
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

void m68000_musashi_device::WRITE_EA_FPE(int mode, int reg, extFloat80_t fpr, uint32_t di_mode_ea)
{
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
			// EA_AY_DI_32() should not be done here because fmovem would increase
			// PC each time, reading incorrect displacement & advancing PC too much.
			store_extended_float80(di_mode_ea, fpr);
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

void m68000_musashi_device::WRITE_EA_PACK(int ea, int k, extFloat80_t fpr)
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

void m68000_musashi_device::fpgen_rm_reg(u16 w2)
{
	int ea = m_ir & 0x3f;
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	extFloat80_t source;

	// fmovecr #$f, fp0 f200 5c0f

	if (rm)
	{
		switch (src)
		{
			case 0:     // Long-Word Integer
			{
				s32 d = READ_EA_32(ea);
				source = i32_to_extF80(d);
				break;
			}
			case 1:     // Single-precision Real
			{
				u32 d = READ_EA_32(ea);
				float32_t *pF = (float32_t *)&d;
				source = f32_to_extF80(*pF);
				break;
			}
			case 2:     // Extended-precision Real
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				uint32_t di_mode_ea = imode == 5 ? (REG_A()[reg] + MAKE_INT_16(m68ki_read_imm_16())) : 0;
				source = READ_EA_FPE(imode, reg, di_mode_ea);
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
				source = i32_to_extF80((s32)d);
				break;
			}
			case 5:     // Double-precision Real
			{
				u64 d = READ_EA_64(ea);
				float64_t *pF = (float64_t *)&d;
				source = f64_to_extF80(*pF);
				break;
			}
			case 6:     // Byte Integer
			{
				s8 d = READ_EA_8(ea);
				source = i32_to_extF80((s32)d);
				break;
			}
			case 7:     // FMOVECR load from constant ROM
			{
				switch (w2 & 0x7f)
				{
					case 0x0:   // Pi
							source.signExp = 0x4000;
						source.signif = 0xc90fdaa22168c235U;
						break;

					case 0xb:   // log10(2)
						source.signExp = 0x3ffd;
						source.signif = 0x9a209a84fbcff798U;
						break;

					case 0xc:   // e
						source.signExp = 0x4000;
						source.signif = 0xadf85458a2bb4a9bU;
						break;

					case 0xd:   // log2(e)
						source.signExp = 0x3fff;
						source.signif = 0xb8aa3b295c17f0bcU;
						break;

					case 0xe:   // log10(e)
						source.signExp = 0x3ffd;
						source.signif = 0xde5bd8a937287195U;
						break;

					case 0xf:   // 0.0
						source = i32_to_extF80((s32)0);
						break;

					case 0x30:  // ln(2)
						source.signExp = 0x3ffe;
						source.signif = 0xb17217f7d1cf79acU;
						break;

					case 0x31:  // ln(10)
						source.signExp = 0x4000;
						source.signif = 0x935d8dddaaa8ac17U;
						break;

					case 0x32:  // 1 (or 100?  manuals are unclear, but 1 would make more sense)
						source = i32_to_extF80((s32)1);
						break;

					case 0x33:  // 10^1
						source = i32_to_extF80((s32)10);
						break;

					case 0x34:  // 10^2
						source = i32_to_extF80((s32)10 * 10);
						break;

					case 0x35:  // 10^4
						source = i32_to_extF80((s32)1000 * 10);
						break;

					case 0x36:  // 1.0e8
						source = i32_to_extF80((s32)10000000 * 10);
						break;

					case 0x37:  // 1.0e16 - can't get the right precision from s32 so go "direct" with constants from h/w
						source.signExp = 0x4034;
						source.signif = 0x8e1bc9bf04000000U;
						break;

					case 0x38:  // 1.0e32
						source.signExp = 0x4069;
						source.signif = 0x9dc5ada82b70b59eU;
						break;

					case 0x39:  // 1.0e64
						source.signExp = 0x40d3;
						source.signif = 0xc2781f49ffcfa6d5U;
						break;

					case 0x3a:  // 1.0e128
						source.signExp = 0x41a8;
						source.signif = 0x93ba47c980e98ce0U;
						break;

					case 0x3b:  // 1.0e256
						source.signExp = 0x4351;
						source.signif = 0xaa7eebfb9df9de8eU;
						break;

					case 0x3c:  // 1.0e512
						source.signExp = 0x46a3;
						source.signif = 0xe319a0aea60e91c7U;
						break;

					case 0x3d:  // 1.0e1024
						source.signExp = 0x4d48;
						source.signif = 0xc976758681750c17U;
						break;

					case 0x3e:  // 1.0e2048
						source.signExp = 0x5a92;
						source.signif = 0x9e8b3b5dc53d5de5U;
						break;

					case 0x3f:  // 1.0e4096
						source.signExp = 0x7525;
						source.signif = 0xc46052028a20979bU;
						break;

					default:
						fatalerror("fmove_rm_reg: unknown constant ROM offset %x at %08x\n", w2&0x7f, m_pc-4);
						break;
				}

				// handle it right here, the usual opmode bits aren't valid in the FMOVECR case
				m_fpr[dst] = source;
				set_condition_codes(m_fpr[dst]);
				m_icount -= 4;
				return;
			}
			default:    fatalerror("fmove_rm_reg: invalid source specifier %x at %08X\n", src, m_pc-4);
		}

		LOGMASKED(LOG_LOADSTORE, "Load source type %d = %f (PC=%08x)\n", src, fx80_to_double(source), m_ppc);
	}
	else
	{
		source = m_fpr[src];
		LOGMASKED(LOG_LOADSTORE, "Load source from FPR %d = %f (PC=%08x)\n", src, fx80_to_double(source), m_ppc);
	}

	LOGMASKED(LOG_INSTRUCTIONS, "FPU: opmode %02x (PC=%08x)\n", opmode, m_ppc);
	const extFloat80_t dstCopy = m_fpr[dst];
	if (opmode != 0)
	{
		clear_exception_flags();
	}

	switch (opmode)
	{
		case 0x00:      // FMOVE
		{
			m_fpr[dst] = source;
			set_condition_codes(m_fpr[dst]);
			m_icount -= 56;
			break;
		}
		case 0x01:      // FINT
		{
			s32 temp = convert_to_int(source, INT32_MIN, INT32_MAX);
			m_fpr[dst] = i32_to_extF80(temp);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 78;
			break;
		}
		case 0x03:      // FINTRZ
		{
			s32 temp = extF80_to_i32_r_minMag(source, true);
			m_fpr[dst] = i32_to_extF80(temp);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, 0);
			m_icount -= 78;
			break;
		}
		case 0x04:      // FSQRT
		{
			m_fpr[dst] = extF80_sqrt(source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 109;
			break;
		}
		case 0x06:      // FLOGNP1
		{
			m_fpr[dst] = extFloat80_lognp1(source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 594;
			break;
		}
		case 0x0a: // FATAN
		{
			m_fpr[dst] = source;
			m_fpr[dst] = extFloat80_68katan(m_fpr[dst]);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 426;
			break;
		}
		case 0x0e:      // FSIN
		{
			m_fpr[dst] = source;
			extFloat80_sin(m_fpr[dst]);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 414;
			break;
		}
		case 0x0f:      // FTAN
		{
			m_fpr[dst] = source;
			extFloat80_tan(m_fpr[dst]);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 496;
			break;
		}
		case 0x14:      // FLOGN
		{
			m_fpr[dst] = extFloat80_logn(source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 548;
			break;
		}
		case 0x15:      // FLOG10
		{
			m_fpr[dst] = extFloat80_log10 (source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 604;
			break;
		}
		case 0x16:      // FLOG2
		{
			m_fpr[dst] = extFloat80_log2 (source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 604;
			break;
		}
		case 0x18:      // FABS
		{
			m_fpr[dst] = source;
			m_fpr[dst].signExp &= 0x7fff;
			set_condition_codes(m_fpr[dst]);
			m_icount -= 58;
			break;
		}
		case 0x1a:      // FNEG
		{
			m_fpr[dst] = source;
			m_fpr[dst].signExp ^= 0x8000;
			set_condition_codes(m_fpr[dst]);
			m_icount -= 58;
			break;
		}
		case 0x1d:      // FCOS
		{
			m_fpr[dst] = source;
			extFloat80_cos(m_fpr[dst]);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 414;
			break;
		}
		case 0x1e:      // FGETEXP
		{
			s16 temp2;

			temp2 = source.signExp;    // get the exponent
			temp2 -= 0x3fff;    // take off the bias
			m_fpr[dst] = double_to_fx80((double)temp2);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FGETEXP: %f\n", fx80_to_double(m_fpr[dst]));
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, 0); // only NaNs can raise an exception here
			m_icount -= 68;
			break;
		}
		case 0x1f:      // FGETMAN
		{
			m_fpr[dst] = extFloat80_getman(source);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FGETMAN: %f\n", fx80_to_double(m_fpr[dst]));
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, 0);    // only NaNs can raise an exception here
			m_icount -= 54;
			break;
		}
		case 0x60:      // FSDIVS
		{
			float32_t sngSrc, sngDst;
			sngSrc = extF80_to_f32(source);
			sngDst = extF80_to_f32(m_fpr[dst]);
			if (f32_eq(sngSrc, i32_to_f32(0)))
			{
				m_fpsr |= FPES_DIVZERO | FPAE_DIVZERO;
			}
			sngDst = f32_div(sngDst, sngSrc);
			m_fpr[dst] = f32_to_extF80(sngDst);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 124;
			break;
		}
		case 0x64:      // FDDIV
		{
			float64_t sngSrc, sngDst;
			sngSrc = extF80_to_f64(source);
			sngDst = extF80_to_f64(m_fpr[dst]);
			if (f64_eq(sngSrc, i32_to_f64(0)))
			{
				m_fpsr |= FPES_DIVZERO | FPAE_DIVZERO;
			}
			sngDst = f64_div(sngDst, sngSrc);
			m_fpr[dst] = f64_to_extF80(sngDst);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 130;
			break;
		}
		case 0x20:      // FDIV
		{
			if (extF80_eq(source, i32_to_extF80(0)))
			{
				m_fpsr |= FPES_DIVZERO | FPAE_DIVZERO;
			}
			m_fpr[dst] = extF80_div(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_OVRFLOW|EXC_ENB_UNDFLOW);
			m_icount -= 128;
			break;
		}
		case 0x21:      // FMOD
		{
			s8 const mode = softfloat_roundingMode;
			uint64_t quotient;
			softfloat_roundingMode = softfloat_round_minMag;
			extFloat80_remainder(m_fpr[dst], source, m_fpr[dst], quotient);
			set_condition_codes(m_fpr[dst]);
			softfloat_roundingMode = mode;
			m_fpsr &= 0xff00ffff;
			m_fpsr |= (quotient & 0x7f) << 16;
			if (m_fpr[dst].signExp & 0x8000)
			{
				m_fpsr |= 0x00800000;
			}
			m_icount -= 95;
			break;
		}
		case 0x22:      // FADD
		{
			m_fpr[dst] = extF80_add(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_OVRFLOW|EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FADD: %f + %f = %f\n", fx80_to_double(dstCopy), fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 76;
			break;
		}
		case 0x63:      // FSMULS (JFF)
		case 0x23:      // FMUL
		{
			m_fpr[dst] = extF80_mul(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FMUL: %f * %f = %f\n", fx80_to_double(dstCopy), fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 96;
			break;
		}
		case 0x24:      // FSGLDIV
		{
			float32_t a = extF80_to_f32( m_fpr[dst] );
			float32_t  b = extF80_to_f32( source );
			m_fpr[dst] = f32_to_extF80( f32_div(a, b) );
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 94;
			break;
		}
		case 0x25:      // FREM
		{
			s8 const mode = softfloat_roundingMode;
			uint64_t quotient;
			softfloat_roundingMode = softfloat_round_near_even;
			extFloat80_ieee754_remainder(m_fpr[dst], source, m_fpr[dst], quotient);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(dstCopy, source, EXC_ENB_UNDFLOW);
			softfloat_roundingMode = mode;
			m_fpsr &= 0xff00ffff;
			m_fpsr |= (quotient & 0x7f) << 16;
			if (m_fpr[dst].signExp & 0x8000)
			{
				m_fpsr |= 0x00800000;
			}
			m_icount -= 125;
			break;
		}
		case 0x26:      // FSCALE
		{
			m_fpr[dst] = extFloat80_scale(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 66;
			break;
		}
		case 0x27:      // FSGLMUL
		{
			float32_t a = extF80_to_f32( m_fpr[dst] );
			float32_t b = extF80_to_f32( source );
			m_fpr[dst] = f32_to_extF80( f32_mul(a, b) );
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 94;
			break;
		}
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f: // FSUB
		{
			m_fpr[dst] = extF80_sub(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FSUB: %f - %f = %f\n", fx80_to_double(dstCopy), fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 76;
			break;
		}

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35:
		case 0x36: case 0x37: // FSINCOS
		{
			extFloat80_sincos(source, &m_fpr[dst], &m_fpr[w2 & 7]);
			set_condition_codes(m_fpr[dst]);    // CCs are set on the sin result
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 474;
			break;
		}

		case 0x38: case 0x39: case 0x3c: case 0x3d:      // FCMP
		{
			const extFloat80_t res = extF80_sub(m_fpr[dst], source);
			set_condition_codes(res);
			sync_exception_flags(source, dstCopy, 0);

			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FCMP: %f - %f = %f\n", fx80_to_double(dstCopy), fx80_to_double(source), fx80_to_double(res));
			m_icount -= 58;
			break;
		}
		case 0x3a: case 0x3b: case 0x3e: case 0x3f: // FTST
		{
			set_condition_codes(source);
			sync_exception_flags(source, dstCopy, 0);
			m_icount -= 56;
			break;
		}

		default:    fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, m_ppc);
	}
}

void m68000_musashi_device::fmove_reg_mem(u16 w2)
{
	int ea = m_ir & 0x3f;
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	int k = (w2 & 0x7f);

	switch (dst)
	{
		case 0:     // Long-Word Integer
		{
			s32 d = convert_to_int(m_fpr[src], INT32_MIN, INT32_MAX);
			WRITE_EA_32(ea, d);
			break;
		}
		case 1:     // Single-precision Real
		{
			u32 d;
			float32_t *pF = (float32_t *)&d;
			*pF = extF80_to_f32(m_fpr[src]);
			WRITE_EA_32(ea, d);
			break;
		}
		case 2:     // Extended-precision Real
		{
			int mode = (ea >> 3) & 0x7;
			int reg = (ea & 0x7);
			uint32_t di_mode_ea = mode == 5 ? (REG_A()[reg] + MAKE_INT_16(m68ki_read_imm_16())) : 0;

			WRITE_EA_FPE(mode, reg, m_fpr[src], di_mode_ea);
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
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FMOVE: %f to reg %d\n", fx80_to_double(m_fpr[src]), dst);
			s16 value = (s16)convert_to_int(m_fpr[src], INT16_MIN, INT16_MAX);
			WRITE_EA_16(ea, value);
			break;
		}
		case 5:     // Double-precision Real
		{
			u64 d;
			float64_t *pF = (float64_t *)&d;
			clear_exception_flags();
			*pF = extF80_to_f64(m_fpr[src]);
			sync_exception_flags(m_fpr[src], m_fpr[src], 0);
			WRITE_EA_64(ea, d);
			break;
		}
		case 6:     // Byte Integer
		{
			s8 value = (s8)convert_to_int(m_fpr[src], INT8_MIN, INT8_MAX);
			WRITE_EA_8(ea, value);
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

void m68000_musashi_device::fmove_fpcr(u16 w2)
{
	int ea = m_ir & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int regsel = (w2 >> 10) & 0x7;
	int reg = ea & 7;
	int mode = (ea >> 3) & 0x7;

	LOGMASKED(LOG_FPSR, "FMOVE FP*R: EA %x dir %x reg %d mode %d regsel %x\n", ea, dir, reg, mode, regsel);

	u32 address = 0;
	switch (mode)
	{
	case 0:     // Dn
	#if 0
		if (dir)
		{
			if (regsel & 4) WRITE_EA_32(ea, m_fpcr);
			if (regsel & 2) WRITE_EA_32(ea, m_fpsr);
			if (regsel & 1) WRITE_EA_32(ea, m_fpiar);
		}
		else
		{
			if (regsel & 4) m_fpcr = READ_EA_32(ea);
			if (regsel & 2) m_fpsr = READ_EA_32(ea);
			if (regsel & 1) m_fpiar = READ_EA_32(ea);
		}
	#endif
		break;

	case 2: // (An)
		address = REG_A()[reg];
		break;

	case 3: // (An)+
	case 4: // -(An)
		break;

	case 5: // (d16, An)
		address = EA_AY_DI_32();
		break;

	case 6: // (An) + (Xn) + d8
		address = EA_AY_IX_32();
		break;

	case 7:
	{
		switch (reg)
		{
		case 0: // (xxx).W
			address = OPER_I_16();
			break;

		case 1: // (xxx).L
			{
				u32 d1 = OPER_I_16();
				u32 d2 = OPER_I_16();
				address = (d1 << 16) | d2;
			}
			break;
		case 2: // (d16, PC)
			address = EA_PCDI_32();
			break;

		case 3: // (PC) + (Xn) + d8
			address = EA_PCIX_32();
			break;

		case 4: // #<data>
		{
			if (regsel & 4) m_fpcr = READ_EA_32(ea);
			else if (regsel & 2) m_fpsr = READ_EA_32(ea);
			else if (regsel & 1) m_fpiar = READ_EA_32(ea);
			return;
		}

		default:
			fatalerror("M68kFPU: fmove_fpcr: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
			break;
		}
	}
	break;

	default:
		fatalerror("M68kFPU: fmove_fpcr: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
		break;
	}

	switch (mode)
	{
	case 0: // Dn
	case 3: // (An)+
	case 4: // -(An)
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
		break;

	default:
		if (dir) // From system control reg to <ea>
		{
			if (regsel & 4)
			{
				m68ki_write_32(address, m_fpcr);
				address += 4;
			}
			if (regsel & 2)
			{
				m68ki_write_32(address, m_fpsr);
				address += 4;
			}
			if (regsel & 1)
			{
				m68ki_write_32(address, m_fpiar);
				address += 4;
			}
		}
		else // From <ea> to system control reg
		{
			if (regsel & 4)
			{
				m_fpcr = m68ki_read_32(address);
				address += 4;
			}
			if (regsel & 2)
			{
				m_fpsr = m68ki_read_32(address);
				address += 4;
			}
			if (regsel & 1)
			{
				m_fpiar = m68ki_read_32(address);
				address += 4;
			}
		}
		break;
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

		//      logerror("fmove_fpcr: fpcr=%04x prec=%d rnd=%d\n", m_fpcr, prec, rnd);

		switch (prec)
		{
		case 0: // Extend (X)
			extF80_roundingPrecision = 80;
			break;
		case 1: // Single (S)
			extF80_roundingPrecision = 32;
			break;
		case 2: // Double (D)
			extF80_roundingPrecision = 64;
			break;
		case 3: // Undefined
			extF80_roundingPrecision = 80;
			break;
		}

		switch (rnd)
		{
		case 0: // To Nearest (RN)
			softfloat_roundingMode = softfloat_round_near_even;
			break;
		case 1: // To Zero (RZ)
			softfloat_roundingMode = softfloat_round_minMag;
			break;
		case 2: // To Minus Infinitiy (RM)
			softfloat_roundingMode = softfloat_round_min;
			break;
		case 3: // To Plus Infinitiy (RP)
			softfloat_roundingMode = softfloat_round_max;
			break;
		}
	}

	m_icount -= 30;
}

void m68000_musashi_device::fmovem(u16 w2)
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
				[[fallthrough]];
			case 0:     // Static register list, predecrement or control addressing mode
			{
				// the "di_mode_ea" parameter kludge is required here else WRITE_EA_FPE would have
				// to call EA_AY_DI_32() (that advances PC & reads displacement) each time
				// when the proper behaviour is 1) read once, 2) increment ea for each matching register
				// this forces to pre-read the mode (named "imode") so we can decide to read displacement, only once
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				int di_mode = imode == 5;
				uint32_t di_mode_ea = di_mode ? (REG_A()[reg] + MAKE_INT_16(m68ki_read_imm_16())) : 0;

				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						WRITE_EA_FPE(imode, reg, m_fpr[i], di_mode_ea);
						if (di_mode)
						{
							di_mode_ea += 12;
						}

						m_icount -= 2;
					}
				}
				break;
			}

			case 3: // Dynamic register list, postincrement or control addressing mode.
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];
				[[fallthrough]];
			case 2:     // Static register list, postdecrement or control addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				int di_mode = imode == 5;

				uint32_t di_mode_ea = di_mode ? (REG_A()[reg] + MAKE_INT_16(m68ki_read_imm_16())) : 0;

				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						WRITE_EA_FPE(imode, reg, m_fpr[7 - i], di_mode_ea);
						if (di_mode)
						{
							di_mode_ea += 12;
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
				[[fallthrough]];
			case 2:     // Static register list, postincrement or control addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				int di_mode = imode == 5;
				uint32_t di_mode_ea = di_mode ? (REG_A()[reg] + MAKE_INT_16(m68ki_read_imm_16())) : 0;

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
								m_fpr[7 - i] = READ_EA_FPE(imode, reg, di_mode_ea);
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

void m68000_musashi_device::fscc()
{
	const int mode = (m_ir & 0x38) >> 3;
	const int condition = OPER_I_16() & 0x3f;
	const int v = (test_condition(condition) ? 0xff : 0x00);

	switch (mode)
	{
		case 0: // Dx (handled specially because it only changes the low byte of Dx)
			{
				const int reg = m_ir & 7;
				REG_D()[reg] = (REG_D()[reg] & 0xffffff00) | v;
			}
			break;

		default:
			WRITE_EA_8(m_ir & 0x3f, v);
			break;
	}

	m_icount -= 7; // ???
}

void m68000_musashi_device::fbcc16()
{
	s32 offset;
	int condition = m_ir & 0x3f;

	offset = (s16)(OPER_I_16());

	// TODO: condition and jump!!!
	if (test_condition(condition))
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset-2);
	}

	m_icount -= 7;
}

void m68000_musashi_device::fbcc32()
{
	s32 offset;
	int condition = m_ir & 0x3f;

	offset = OPER_I_32();

	// TODO: condition and jump!!!
	if (test_condition(condition))
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset-4);
	}

	m_icount -= 7;
}

void m68000_musashi_device::m68040_fpu_op0()
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

int m68000_musashi_device::perform_fsave(u32 addr, int inc)
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
void m68000_musashi_device::do_frestore_null()
{
	int i;

	m_fpcr = 0;
	m_fpsr = 0;
	m_fpiar = 0;
	for (i = 0; i < 8; i++)
	{
		m_fpr[i].signExp = 0x7fff;
		m_fpr[i].signif = 0xffffffffffffffffU;
	}

	// Mac IIci at 408458e6 wants an FSAVE of a just-restored nullptr frame to also be nullptr
	// The PRM says it's possible to generate a nullptr frame, but not how/when/why.  (need the 68881/68882 manual!)
	m_fpu_just_reset = 1;
}

void m68000_musashi_device::m68040_do_fsave(u32 addr, int reg, int inc)
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

void m68000_musashi_device::m68040_do_frestore(u32 addr, int reg)
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

void m68000_musashi_device::m68040_fpu_op1()
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

			case 6: // (An) + (Xn) + d8
				addr = EA_AY_IX_16();
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

			case 6: // (An) + (Xn) + d8
				addr = EA_AY_IX_16();
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

void m68000_musashi_device::m68881_ftrap()
{
	u16 w2  = OPER_I_16();

	// now check the condition
	if (test_condition(w2 & 0x3f))
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
