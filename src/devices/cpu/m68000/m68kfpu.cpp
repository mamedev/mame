// license:BSD-3-Clause
// copyright-holders:Karl Stenerud, R. Belmont

/*
    SoftFloat 3E version, May/June 2024
    - Exception flags now set for all opcodes
    - FREM/FMOD now generate the quotient bits in FPSR, required for SANE to do trigonometry
    - FMOVE of a float to an integer register generates the proper INEXACT exception, required
      for SANE to calculate square roots.
*/

#include <bit>
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
static constexpr u32 FPES_BSUN          = 0x00008000;

static constexpr u32 FPAE_INEXACT       = 0x00000008;
static constexpr u32 FPAE_DIVZERO       = 0x00000010;
static constexpr u32 FPAE_UNDERFLOW     = 0x00000020;
static constexpr u32 FPAE_OVERFLOW      = 0x00000040;
static constexpr u32 FPAE_IOP           = 0x00000080;

// writable bits of the control and status registers; the rest read as zero
static constexpr u32 FPCR_WRITE_MASK    = 0x0000fff0;
static constexpr u32 FPSR_WRITE_MASK    = 0x0ffffff8;

static constexpr u32 EXC_ENB_INEXACT    = 0x00000001;
static constexpr u32 EXC_ENB_UNDFLOW    = 0x00000002;
static constexpr u32 EXC_ENB_OVRFLOW    = 0x00000004;

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

// 10^n as a 64-bit integer
static const u64 pow10_u64[20] =
{
	1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U,
	100000000U, 1000000000U, 10000000000U, 100000000000U, 1000000000000U,
	10000000000000U, 100000000000000U, 1000000000000000U, 10000000000000000U,
	100000000000000000U, 1000000000000000000U, 10000000000000000000U
};

// 10^n in binary128 by squaring; the error is a few ulps of the 113-bit
// significand, far below what survives rounding to extended precision
static float128_t f128_pow10(int n)
{
	float128_t r = i32_to_f128(1);
	float128_t p = i32_to_f128(10);

	while (n != 0)
	{
		if (n & 1)
		{
			r = f128_mul(r, p);
		}
		n >>= 1;
		if (n != 0)
		{
			p = f128_mul(p, p);
		}
	}

	return r;
}

// x * 10^n in binary128; scale in pieces when the power alone would
// overflow the binary128 range (denormal extended values need this)
static float128_t f128_scale_pow10(float128_t x, int n)
{
	while (n > 4900)
	{
		x = f128_mul(x, f128_pow10(4900));
		n -= 4900;
	}

	while (n < -4900)
	{
		x = f128_div(x, f128_pow10(4900));
		n += 4900;
	}

	if (n >= 0)
	{
		return f128_mul(x, f128_pow10(n));
	}
	else
	{
		return f128_div(x, f128_pow10(-n));
	}
}

// The sin/cos/tan helpers give up on arguments of 2^63 and larger; reduce
// by the period (with the accuracy loss the hardware also suffers on huge
// arguments) so they always produce an in-range result.
static void reduce_trig_argument(extFloat80_t &a)
{
	extFloat80_t twopi;
	twopi.signExp = 0x4001;
	twopi.signif = 0xc90fdaa22168c235U;

	// the remainder is partial (x87 style) when the exponents differ by 64
	// or more; iterate until the reduction is complete
	uint64_t quotient;
	while (extFloat80_ieee754_remainder(a, twopi, a, quotient) > 0)
	{
	}
}

// Force the result rounding for the 68040 FSxxx/FDxxx instructions
struct forced_precision
{
	forced_precision(int opmode)
	{
		m_saved = extF80_roundingPrecision;
		extF80_roundingPrecision = (opmode & 0x04) ? 64 : 32;
	}

	~forced_precision()
	{
		extF80_roundingPrecision = m_saved;
	}

	uint_fast8_t m_saved;
};

// directed rounding follows the value's sign, but the packed conversions
// work on the magnitude, so the directed modes swap for negative values
static uint_fast8_t rounding_mode_for_sign(uint_fast8_t mode, bool sign)
{
	if (sign)
	{
		if (mode == softfloat_round_min)
		{
			return softfloat_round_max;
		}
		if (mode == softfloat_round_max)
		{
			return softfloat_round_min;
		}
	}
	return mode;
}

inline extFloat80_t m68000_musashi_device::load_pack_float80(u32 ea)
{
	const u32 dw1 = m68ki_read_32(ea);
	const u32 dw2 = m68ki_read_32(ea+4);
	const u32 dw3 = m68ki_read_32(ea+8);
	extFloat80_t result;

	const u16 sign = BIT(dw1, 31) ? 0x8000 : 0;

	// infinities and NaNs have all ones in the exponent digits; a zero
	// mantissa is an infinity, anything else keeps its NaN payload
	if ((dw1 & 0x7fff0000) == 0x7fff0000)
	{
		result.signExp = sign | 0x7fff;
		result.signif = 0x8000000000000000U | (((u64)dw2 << 32) | dw3);
		return result;
	}

	// gather the 17 mantissa digits into an integer (exact: 10^17 < 2^63)
	u64 mantissa = dw1 & 0xf;
	for (int i = 7; i >= 0; i--)
	{
		mantissa = mantissa * 10 + ((dw2 >> (4 * i)) & 0xf);
	}
	for (int i = 7; i >= 0; i--)
	{
		mantissa = mantissa * 10 + ((dw3 >> (4 * i)) & 0xf);
	}

	if (mantissa == 0)
	{
		result.signExp = sign;
		result.signif = 0;
		return result;
	}

	int exponent = ((dw1 >> 24) & 0xf) * 100 + ((dw1 >> 20) & 0xf) * 10 + ((dw1 >> 16) & 0xf);
	if (BIT(dw1, 30))
	{
		exponent = -exponent;
	}

	// value = mantissa * 10^(exponent - 16), scaled in binary128 so all
	// 17 digits survive; only the final conversion rounds to extended,
	// honoring the FPCR rounding mode
	const uint_fast8_t mode = softfloat_roundingMode;
	softfloat_roundingMode = softfloat_round_near_even;
	const float128_t x = f128_scale_pow10(ui64_to_f128(mantissa), exponent - 16);
	softfloat_roundingMode = rounding_mode_for_sign(mode, sign != 0);
	result = f128_to_extF80(x);
	softfloat_roundingMode = mode;

	result.signExp |= sign;
	return result;
}

inline void m68000_musashi_device::store_pack_float80(u32 ea, int k, extFloat80_t fpr)
{
	u32 dw1 = 0, dw2 = 0, dw3 = 0;
	const bool sign = BIT(fpr.signExp, 15);
	const int exp = fpr.signExp & 0x7fff;

	if (sign)
	{
		dw1 = 0x80000000;
	}

	if (exp == 0x7fff)
	{
		// infinities and NaNs: all ones exponent, NaNs keep their payload
		dw1 |= 0x7fff0000;
		if ((fpr.signif << 1) != 0)
		{
			dw2 = (u32)(fpr.signif >> 32);
			dw3 = (u32)fpr.signif;
		}
	}
	else if (fpr.signif != 0)
	{
		// a positive k is the number of significant digits to produce;
		// k <= 0 puts the rounding point at 10^k (fixed-point notation)
		if (k > 17)
		{
			k = 17;
			m_fpsr |= FPES_OPERR | FPAE_IOP;
		}

		// first estimate of floor(log10(magnitude)) from the binary exponent
		const int e2 = (exp != 0 ? exp : 1) - 16383 - std::countl_zero(fpr.signif);
		int decexp = (int)(((s64)e2 * 30103) / 100000);

		extFloat80_t mag = fpr;
		mag.signExp &= 0x7fff;

		// scale the magnitude to an 18 digit integer, correcting the
		// exponent estimate as needed; this stage truncates but keeps a
		// sticky bit so a value just above a tie at the requested digit is
		// distinguishable from the exact tie, and the requested rounding
		// mode is applied in the exact integer arithmetic below
		const uint_fast8_t mode = softfloat_roundingMode;
		softfloat_roundingMode = softfloat_round_near_even;
		const float128_t x = extF80_to_f128(mag);
		u64 t = 0;
		bool sticky = false;
		for (int tries = 0; tries < 8; tries++)
		{
			const float128_t q = f128_scale_pow10(x, 17 - decexp);
			t = f128_to_ui64(q, softfloat_round_minMag, false);
			if (t >= pow10_u64[18])
			{
				decexp++;
			}
			else if (t < pow10_u64[17])
			{
				decexp--;
			}
			else
			{
				const float128_t back = ui64_to_f128(t);
				sticky = (back.v[0] != q.v[0]) || (back.v[1] != q.v[1]);
				break;
			}
		}
		softfloat_roundingMode = mode;

		// position of the least significant digit to keep
		const uint_fast8_t rmode = rounding_mode_for_sign(mode, sign);
		int lsd = (k > 0) ? (decexp - k + 1) : k;
		int ndigits = decexp - lsd + 1;
		if (ndigits > 17)
		{
			// fixed-point request for more digits than the format holds
			ndigits = 17;
			lsd = decexp - 16;
			m_fpsr |= FPES_OPERR | FPAE_IOP;
		}

		u64 quot = 0;
		if (ndigits < 0)
		{
			// the entire value is below the rounding point
			if (rmode == softfloat_round_max)
			{
				quot = 1;
			}
		}
		else
		{
			const u64 div = pow10_u64[18 - ndigits];
			const u64 rem = t % div;
			quot = t / div;
			switch (rmode)
			{
			case softfloat_round_near_even:
				if (rem > div / 2 || (rem == div / 2 && (sticky || (quot & 1) != 0)))
				{
					quot++;
				}
				break;
			case softfloat_round_max:
				if (rem != 0 || sticky)
				{
					quot++;
				}
				break;
			default:    // round_minMag, round_min: truncate
				break;
			}
		}

		if (quot != 0)
		{
			// a carry out of the top digit leaves an 18 digit result with
			// only trailing zeros to drop
			if (quot == pow10_u64[17])
			{
				quot = pow10_u64[16];
				lsd++;
			}

			int digits = 1;
			while (quot >= pow10_u64[digits])
			{
				digits++;
			}
			decexp = lsd + digits - 1;

			const int aexp = (decexp < 0) ? -decexp : decexp;
			if (decexp < 0)
			{
				dw1 |= 0x40000000;
			}
			// a fourth exponent digit only occurs for huge magnitudes and
			// goes in bits 15-12, as the 68040 FPSP does
			dw1 |= ((aexp / 1000) % 10) << 12;
			dw1 |= ((aexp / 100) % 10) << 24;
			dw1 |= ((aexp / 10) % 10) << 20;
			dw1 |= (aexp % 10) << 16;
			dw1 |= (u32)(quot / pow10_u64[digits - 1]);

			for (int j = 1; j < digits; j++)
			{
				const u32 digit = (u32)((quot / pow10_u64[digits - 1 - j]) % 10);
				if (j <= 8)
				{
					dw2 |= digit << (4 * (8 - j));
				}
				else
				{
					dw3 |= digit << (4 * (16 - j));
				}
			}
		}
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
	// only the exception status byte clears at the start of an operation;
	// the accrued byte is sticky until the FPSR is written directly
	m_fpsr &= ~(FPES_BSUN | FPES_SNAN | FPES_OPERR | FPES_OVERFLOW | FPES_UNDERFLOW | FPES_DIVZERO | FPES_INEXACT | FPES_INEXDEC);
}

// fold the exception status byte into the sticky accrued exception byte
// using the 68881/68882 mapping
void m68000_musashi_device::update_accrued_exceptions()
{
	if (m_fpsr & (FPES_BSUN | FPES_SNAN | FPES_OPERR))
	{
		m_fpsr |= FPAE_IOP;
	}
	if (m_fpsr & FPES_OVERFLOW)
	{
		m_fpsr |= FPAE_OVERFLOW;
	}
	if ((m_fpsr & FPES_UNDERFLOW) && (m_fpsr & FPES_INEXACT))
	{
		m_fpsr |= FPAE_UNDERFLOW;
	}
	if (m_fpsr & FPES_DIVZERO)
	{
		m_fpsr |= FPAE_DIVZERO;
	}
	if (m_fpsr & (FPES_INEXACT | FPES_INEXDEC | FPES_OVERFLOW))
	{
		m_fpsr |= FPAE_INEXACT;
	}
}

void m68000_musashi_device::sync_exception_flags(extFloat80_t op1, extFloat80_t op2, u32 enables)
{
	const bool snan = extF80_isSignalingNaN(op1) || extF80_isSignalingNaN(op2);
	if (snan)
	{
		m_fpsr |= FPES_SNAN;
	}

	// an invalid operation that isn't a signaling NaN is an operand error
	if ((softfloat_exceptionFlags & softfloat_flag_invalid) && !snan)
	{
		m_fpsr |= FPES_OPERR;
	}

	if ((enables & EXC_ENB_INEXACT) && (softfloat_exceptionFlags & softfloat_flag_inexact))
	{
		m_fpsr |= FPES_INEXACT;
	}

	if ((enables & EXC_ENB_UNDFLOW) && (softfloat_exceptionFlags & softfloat_flag_underflow))
	{
		m_fpsr |= FPES_UNDERFLOW;
	}

	if ((enables & EXC_ENB_OVRFLOW) && (softfloat_exceptionFlags & softfloat_flag_overflow))
	{
		m_fpsr |= FPES_OVERFLOW;
	}

	update_accrued_exceptions();
}

int m68000_musashi_device::test_condition(int condition)
{
	int n = (m_fpsr & FPCC_N) != 0;
	int z = (m_fpsr & FPCC_Z) != 0;
	int nan = (m_fpsr & FPCC_NAN) != 0;
	int r = 0;

	// the IEEE-nonaware predicates signal BSUN on an unordered condition
	if ((condition & 0x10) && nan)
	{
		m_fpsr |= FPES_BSUN | FPAE_IOP;
	}
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
	if (softfloat_exceptionFlags & softfloat_flag_invalid)
	{
		// overflow or NaN: the library's saturation value is x86 flavored,
		// the hardware stores the largest value of the source's sign
		result = BIT(source.signExp, 15) ? lowerLimit : upperLimit;
	}
	sync_exception_flags(source, source, EXC_ENB_INEXACT);
	if (result < lowerLimit)
	{
		result = lowerLimit;
		m_fpsr |= FPES_OPERR | FPAE_IOP;
	}
	else if (result > upperLimit)
	{
		result = upperLimit;
		m_fpsr |= FPES_OPERR | FPAE_IOP;
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
					u32 ea = EA_AW_8();
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
					u32 ea = EA_AW_16();
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
		case 1: // An
		{
			return REG_A()[reg];
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
					u32 ea = EA_AW_32();
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
				case 0:     // (xxx).W
				{
					u32 ea = EA_AW_32();
					h1 = m68ki_read_32(ea+0);
					h2 = m68ki_read_32(ea+4);
					return  (u64)(h1) << 32 | (u64)(h2);
				}
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

// Compute the address for a control-mode <ea>, reading any extension words.
// This must happen exactly once per instruction, before FMOVEM's register
// loop; the loop then advances the returned address itself.  (An)+, -(An)
// and #<data> have per-transfer side effects instead and are resolved by
// READ_EA_FPE/WRITE_EA_FPE.
u32 m68000_musashi_device::GET_EA_FPE(int mode, int reg)
{
	switch (mode)
	{
		case 2:     // (An)
			return REG_A()[reg];

		case 3:     // (An)+
		case 4:     // -(An)
			return 0;

		case 5:     // (d16, An)
			return EA_AY_DI_32();

		case 6:     // (An) + (Xn) + d8
			return EA_AY_IX_32();

		case 7:
			switch (reg)
			{
				case 0:     // (xxx).W
					return EA_AW_32();

				case 1:     // (xxx).L
					return EA_AL_32();

				case 2:     // (d16, PC)
					return EA_PCDI_32();

				case 3:     // (d8, PC, Xn)
					return EA_PCIX_32();

				case 4:     // #<data>
					return 0;
			}
			break;

		default:
			break;
	}

	fatalerror("M68kFPU: GET_EA_FPE: unhandled mode %d, reg %d at %08X\n", mode, reg, m_pc);
}

extFloat80_t m68000_musashi_device::READ_EA_FPE(int mode, int reg, u32 address)
{
	extFloat80_t fpr;

	switch (mode)
	{
		case 2:     // (An)
		case 5:     // (d16, An)
		case 6:     // (An) + (Xn) + d8
		{
			fpr = load_extended_float80(address);
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
			REG_A()[reg] -= 12;
			fpr = load_extended_float80(REG_A()[reg]);
			break;
		}

		case 7: // extended modes
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				case 1:     // (xxx).L
				case 2:     // (d16, PC)
				case 3:     // (d8, PC, Xn)
					fpr = load_extended_float80(address);
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

extFloat80_t m68000_musashi_device::READ_EA_PACK(int mode, int reg, u32 address)
{
	extFloat80_t fpr;

	switch (mode)
	{
		case 2:     // (An)
		case 5:     // (d16, An)
		case 6:     // (An) + (Xn) + d8
		{
			fpr = load_pack_float80(address);
			break;
		}

		case 3:     // (An)+
		{
			u32 ea = REG_A()[reg];
			REG_A()[reg] += 12;
			fpr = load_pack_float80(ea);
			break;
		}
		case 4:     // -(An)
		{
			REG_A()[reg] -= 12;
			fpr = load_pack_float80(REG_A()[reg]);
			break;
		}

		case 7: // extended modes
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				case 1:     // (xxx).L
				case 2:     // (d16, PC)
				case 3:     // (d8, PC, Xn)
					fpr = load_pack_float80(address);
					break;

				case 4:     // #<data>
					fpr = load_pack_float80(m_pc);
					m_pc += 12;
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
				case 0:     // (xxx).W
				{
					u32 ea = EA_AW_8();
					m68ki_write_8(ea, data);
					break;
				}
				case 1:     // (xxx).L
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
				case 0:     // (xxx).W
				{
					u32 ea = EA_AW_16();
					m68ki_write_16(ea, data);
					break;
				}
				case 1:     // (xxx).L
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
					u32 ea = EA_AW_32();
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
				case 0:     // (xxx).W
				{
					u32 ea = EA_AW_32();
					m68ki_write_32(ea+0, (u32)(data >> 32));
					m68ki_write_32(ea+4, (u32)(data));
					break;
				}
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

void m68000_musashi_device::WRITE_EA_FPE(int mode, int reg, extFloat80_t fpr, u32 address)
{
	switch (mode)
	{
		case 2:     // (An)
		case 5:     // (d16,An)
		case 6:     // (An) + (Xn) + d8
		{
			store_extended_float80(address, fpr);
			break;
		}

		case 3:     // (An)+
		{
			u32 ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			REG_A()[reg] += 12;
			break;
		}

		case 4:     // -(An)
		{
			REG_A()[reg] -= 12;
			u32 ea = REG_A()[reg];
			store_extended_float80(ea, fpr);
			break;
		}

		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				case 1:     // (xxx).L
					store_extended_float80(address, fpr);
					break;

				// PC-relative and immediate destinations are illegal
				default:    fatalerror("M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: WRITE_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
	}
}

void m68000_musashi_device::WRITE_EA_PACK(int mode, int reg, int k, extFloat80_t fpr, u32 address)
{
	switch (mode)
	{
		case 2:     // (An)
		case 5:     // (d16,An)
		case 6:     // (An) + (Xn) + d8
		{
			store_pack_float80(address, k, fpr);
			break;
		}

		case 3:     // (An)+
		{
			store_pack_float80(REG_A()[reg], k, fpr);
			REG_A()[reg] += 12;
			break;
		}

		case 4:     // -(An)
		{
			REG_A()[reg] -= 12;
			store_pack_float80(REG_A()[reg], k, fpr);
			break;
		}

		case 7:
		{
			switch (reg)
			{
				case 0:     // (xxx).W
				case 1:     // (xxx).L
					store_pack_float80(address, k, fpr);
					break;

				// PC-relative and immediate destinations are illegal
				default:    fatalerror("M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
			}
			break;
		}
		default:    fatalerror("M68kFPU: WRITE_EA_PACK: unhandled mode %d, reg %d, at %08X\n", mode, reg, m_pc);
	}
}

void m68000_musashi_device::fpgen_rm_reg(u16 w2)
{
	// arithmetic instructions record their address for exception handlers
	m_fpiar = m_ppc;

	int ea = m_ir & 0x3f;
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	extFloat80_t source;

	// Verify the opmode before any operand fetch so that undefined
	// encodings and (on 68881/68882) the 68040-only rounding forms
	// take an F-line exception before we do any EA shenanigans.
	if (!(rm && src == 7))
	{
		bool valid;
		if (opmode & 0x40)
		{
			// mask of valid 68040 opmodes
			static constexpr u64 VALID_040 = 0x000011dd55000033U;
			valid = (m_cpu_type & CPU_TYPE_040) && BIT(VALID_040, opmode & 0x3f);
		}
		else
		{
			// mask of valid 6888x opmodes
			static constexpr u64 VALID_6888X = 0xfffffffff777f75fU;
			valid = BIT(VALID_6888X, opmode);
		}

		if (!valid)
		{
			m68ki_exception_1111();
			return;
		}
	}

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
				source = f32_to_extF80(std::bit_cast<float32_t>(READ_EA_32(ea)));
				break;
			}
			case 2:     // Extended-precision Real
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				u32 address = GET_EA_FPE(imode, reg);
				source = READ_EA_FPE(imode, reg, address);
				break;
			}
			case 3:     // Packed-decimal Real
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				u32 address = GET_EA_FPE(imode, reg);
				source = READ_EA_PACK(imode, reg, address);
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
				source = f64_to_extF80(std::bit_cast<float64_t>(READ_EA_64(ea)));
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
			m_fpr[dst] = extF80_roundToInt(source, softfloat_roundingMode, true);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 78;
			break;
		}
		case 0x03:      // FINTRZ
		{
			m_fpr[dst] = extF80_roundToInt(source, softfloat_round_minMag, true);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
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
			if (extFloat80_sin(m_fpr[dst]) == -1)
			{
				reduce_trig_argument(m_fpr[dst]);
				extFloat80_sin(m_fpr[dst]);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 414;
			break;
		}
		case 0x0f:      // FTAN
		{
			m_fpr[dst] = source;
			if (extFloat80_tan(m_fpr[dst]) == -1)
			{
				reduce_trig_argument(m_fpr[dst]);
				extFloat80_tan(m_fpr[dst]);
			}
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
			if (extFloat80_cos(m_fpr[dst]) == -1)
			{
				reduce_trig_argument(m_fpr[dst]);
				extFloat80_cos(m_fpr[dst]);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 414;
			break;
		}
		case 0x1e:      // FGETEXP
		{
			const int exp = source.signExp & 0x7fff;
			if (exp == 0x7fff)
			{
				// infinity is an operand error; NaNs propagate quieted
				if ((source.signif << 1) == 0)
				{
					m_fpr[dst].signExp = 0x7fff;
					m_fpr[dst].signif = 0xffffffffffffffffU;
					m_fpsr |= FPES_OPERR | FPAE_IOP;
				}
				else
				{
					m_fpr[dst] = source;
					m_fpr[dst].signif |= 0x4000000000000000U;
				}
			}
			else if (source.signif == 0 && exp == 0)
			{
				// zero returns zero with the source sign
				m_fpr[dst] = source;
			}
			else
			{
				// remove the bias; denormals count their leading zeroes
				const s32 temp2 = (s32)(exp != 0 ? exp : 1) - 0x3fff - std::countl_zero(source.signif);
				m_fpr[dst] = double_to_fx80((double)temp2);
			}
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
		case 0x20:      // FDIV
		{
			if (extF80_eq(source, i32_to_extF80(0)))
			{
				m_fpsr |= FPES_DIVZERO | FPAE_DIVZERO;
			}
			m_fpr[dst] = extF80_div(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT|EXC_ENB_OVRFLOW|EXC_ENB_UNDFLOW);
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
			// the sign bit is the quotient's sign, not the remainder's
			if ((dstCopy.signExp ^ source.signExp) & 0x8000)
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
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT|EXC_ENB_OVRFLOW|EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FADD: %f + %f = %f\n", fx80_to_double(dstCopy), fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 76;
			break;
		}
		case 0x23:      // FMUL
		{
			m_fpr[dst] = extF80_mul(m_fpr[dst], source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT|EXC_ENB_OVRFLOW|EXC_ENB_UNDFLOW);
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
			// the sign bit is the quotient's sign, not the remainder's
			if ((dstCopy.signExp ^ source.signExp) & 0x8000)
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
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
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
			extFloat80_t sine, cosine;
			if (extFloat80_sincos(source, &sine, &cosine) == -1)
			{
				extFloat80_t reduced = source;
				reduce_trig_argument(reduced);
				extFloat80_sincos(reduced, &sine, &cosine);
			}
			// the sine result wins when FPs and FPc name the same register
			m_fpr[w2 & 7] = cosine;
			m_fpr[dst] = sine;
			set_condition_codes(m_fpr[dst]);    // CCs are set on the sin result
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 474;
			break;
		}

		case 0x38: case 0x39: case 0x3c: case 0x3d:      // FCMP
		{
			// a true comparison, not a subtraction: equal infinities are
			// equal, and quiet NaNs do not signal operand errors.  the
			// infinity bit is always cleared by FCMP, and on equality N
			// reflects the destination's sign (the operation table sets
			// N|Z for -0 vs +0 and -inf vs -inf)
			m_fpsr &= ~(FPCC_N | FPCC_Z | FPCC_I | FPCC_NAN);
			if (extFloat80_is_nan(m_fpr[dst]) || extFloat80_is_nan(source))
			{
				m_fpsr |= FPCC_NAN;
			}
			else if (extF80_eq(m_fpr[dst], source))
			{
				m_fpsr |= FPCC_Z;
				if (BIT(m_fpr[dst].signExp, 15))
				{
					m_fpsr |= FPCC_N;
				}
			}
			else if (extF80_lt(m_fpr[dst], source))
			{
				m_fpsr |= FPCC_N;
			}
			sync_exception_flags(source, dstCopy, 0);

			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FCMP: %f vs %f\n", fx80_to_double(dstCopy), fx80_to_double(source));
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
		case 0x08: // FETOXM1
		{
			m_fpr[dst] = extF80_sub(extFloat80_etox(source), i32_to_extF80(1));
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FETOXM1: e ** %f - 1 = %f\n", fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 568;
			break;
		}
		case 0x10: // FETOX
		{
			m_fpr[dst] = extFloat80_etox(source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FETOX: e ** %f = %f\n", fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 520;
			break;
		}
		case 0x11: // FTWOTOX
		{
			m_fpr[dst] = extFloat80_2tox(source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FTWOTOX: 2 ** %f = %f\n", fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 590;
			break;
		}
		case 0x12: // FTENTOX
		{
			m_fpr[dst] = extFloat80_10tox(source);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_UNDFLOW);
			LOGMASKED(LOG_INSTRUCTIONS_VERBOSE, "FTENTOX: 10 ** %f = %f\n", fx80_to_double(source), fx80_to_double(m_fpr[dst]));
			m_icount -= 590;
			break;
		}

		case 0x02:      // FSINH = (e^x - e^-x) / 2
		{
			if (((source.signExp & 0x7fff) == 0) && (source.signif == 0))
			{
				m_fpr[dst] = source;    // +/-0 passes through
			}
			else
			{
				extFloat80_t mx = source;
				mx.signExp ^= 0x8000;
				const extFloat80_t num = extF80_sub(extFloat80_etox(source), extFloat80_etox(mx));
				m_fpr[dst] = extF80_div(num, i32_to_extF80(2));
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 687;
			break;
		}
		case 0x19:      // FCOSH = (e^x + e^-x) / 2
		{
			extFloat80_t mx = source;
			mx.signExp ^= 0x8000;
			const extFloat80_t num = extF80_add(extFloat80_etox(source), extFloat80_etox(mx));
			m_fpr[dst] = extF80_div(num, i32_to_extF80(2));
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW);
			m_icount -= 607;
			break;
		}
		case 0x09:      // FTANH = (e^2x - 1) / (e^2x + 1)
		{
			const extFloat80_t one = i32_to_extF80(1);
			const extFloat80_t e2x = extFloat80_etox(extF80_add(source, source));
			if (((source.signExp & 0x7fff) == 0) && (source.signif == 0))
			{
				m_fpr[dst] = source;    // +/-0 passes through
			}
			else if (((e2x.signExp & 0x7fff) == 0x7fff) && ((e2x.signif << 1) == 0))
			{
				// e^2x overflowed to infinity: tanh saturates at +/-1
				m_fpr[dst] = one;
				m_fpr[dst].signExp |= source.signExp & 0x8000;
			}
			else
			{
				m_fpr[dst] = extF80_div(extF80_sub(e2x, one), extF80_add(e2x, one));
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 661;
			break;
		}
		case 0x0c:      // FASIN = atan(x / sqrt(1 - x^2))
		{
			const extFloat80_t one = i32_to_extF80(1);
			const extFloat80_t root = extF80_sqrt(extF80_sub(one, extF80_mul(source, source)));
			m_fpr[dst] = extFloat80_68katan(extF80_div(source, root));
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 581;
			break;
		}
		case 0x1c:      // FACOS = 2 * atan(sqrt((1 - x) / (1 + x)))
		{
			const extFloat80_t one = i32_to_extF80(1);
			const extFloat80_t ratio = extF80_div(extF80_sub(one, source), extF80_add(one, source));
			const extFloat80_t half = extFloat80_68katan(extF80_sqrt(ratio));
			m_fpr[dst] = extF80_add(half, half);
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 604;
			break;
		}
		case 0x0d:      // FATANH = ln(1 + (2x / (1 - x))) / 2
		{
			const extFloat80_t one = i32_to_extF80(1);
			const extFloat80_t ratio = extF80_div(extF80_add(source, source), extF80_sub(one, source));
			m_fpr[dst] = extF80_div(extFloat80_lognp1(ratio), i32_to_extF80(2));
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_UNDFLOW);
			m_icount -= 693;
			break;
		}

		// 68040-specific forced-precision forms
		case 0x40:      // FSMOVE
		case 0x44:      // FDMOVE
		{
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_mul(source, i32_to_extF80(1));
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 58;
			break;
		}
		case 0x41:      // FSSQRT
		case 0x45:      // FDSQRT
		{
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_sqrt(source);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT);
			m_icount -= 109;
			break;
		}
		case 0x58:      // FSABS
		case 0x5c:      // FDABS
		{
			extFloat80_t temp = source;
			temp.signExp &= 0x7fff;
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_mul(temp, i32_to_extF80(1));
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 58;
			break;
		}
		case 0x5a:      // FSNEG
		case 0x5e:      // FDNEG
		{
			extFloat80_t temp = source;
			temp.signExp ^= 0x8000;
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_mul(temp, i32_to_extF80(1));
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, source, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 58;
			break;
		}
		case 0x60:      // FSDIV
		case 0x64:      // FDDIV
		{
			if (extF80_eq(source, i32_to_extF80(0)))
			{
				m_fpsr |= FPES_DIVZERO | FPAE_DIVZERO;
			}
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_div(m_fpr[dst], source);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 124;
			break;
		}
		case 0x62:      // FSADD
		case 0x66:      // FDADD
		{
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_add(m_fpr[dst], source);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 76;
			break;
		}
		case 0x63:      // FSMUL
		case 0x67:      // FDMUL
		{
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_mul(m_fpr[dst], source);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 96;
			break;
		}
		case 0x68:      // FSSUB
		case 0x6c:      // FDSUB
		{
			{
				const forced_precision fp(opmode);
				m_fpr[dst] = extF80_sub(m_fpr[dst], source);
			}
			set_condition_codes(m_fpr[dst]);
			sync_exception_flags(source, dstCopy, EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			m_icount -= 76;
			break;
		}

		default:    fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, m_ppc);
	}
}

void m68000_musashi_device::fmove_reg_mem(u16 w2)
{
	// arithmetic instructions record their address for exception handlers
	m_fpiar = m_ppc;

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
			clear_exception_flags();
			const u32 d = std::bit_cast<u32>(extF80_to_f32(m_fpr[src]));
			sync_exception_flags(m_fpr[src], m_fpr[src], EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
			WRITE_EA_32(ea, d);
			break;
		}
		case 2:     // Extended-precision Real
		{
			int mode = (ea >> 3) & 0x7;
			int reg = (ea & 0x7);
			u32 address = GET_EA_FPE(mode, reg);

			WRITE_EA_FPE(mode, reg, m_fpr[src], address);
			break;
		}
		case 3:     // Packed-decimal Real with Static K-factor
		{
			int mode = (ea >> 3) & 0x7;
			int reg = (ea & 0x7);
			u32 address = GET_EA_FPE(mode, reg);
			WRITE_EA_PACK(mode, reg, util::sext(k, 7), m_fpr[src], address);
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
			clear_exception_flags();
			const u64 d = std::bit_cast<u64>(extF80_to_f64(m_fpr[src]));
			sync_exception_flags(m_fpr[src], m_fpr[src], EXC_ENB_INEXACT | EXC_ENB_OVRFLOW | EXC_ENB_UNDFLOW);
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
			int mode = (ea >> 3) & 0x7;
			int reg = (ea & 0x7);
			u32 address = GET_EA_FPE(mode, reg);
			// the k-factor is the low 7 bits of the data register, sign-extended
			WRITE_EA_PACK(mode, reg, util::sext(REG_D()[(k >> 4) & 7], 7), m_fpr[src], address);
			break;
		}
	}

	m_icount -= 12;
}

void m68000_musashi_device::apply_fpcr_rounding()
{
	switch ((m_fpcr >> 6) & 3)
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

	switch ((m_fpcr >> 4) & 3)
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
	case 1:     // An
		break;

	case 2: // (An)
		address = REG_A()[reg];
		break;

	case 3: // (An)+
		break;

	case 4: // -(An)
		// An is decremented by the total size first, then the registers are
		// transferred at ascending addresses.  FPCR is always lowest.
		REG_A()[reg] -= 4 * std::popcount(u32(regsel));
		address = REG_A()[reg];
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
			address = EA_AW_32();
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
			if (regsel & 4)
			{
				m_fpcr = OPER_I_32() & FPCR_WRITE_MASK;
				apply_fpcr_rounding();
			}
			if (regsel & 2) m_fpsr = OPER_I_32() & FPSR_WRITE_MASK;
			if (regsel & 1) m_fpiar = OPER_I_32();
			m_icount -= 30;
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
	case 1: // An
	case 3: // (An)+
		if (dir)    // From system control reg to <ea>
		{
			if (regsel & 4) WRITE_EA_32(ea, m_fpcr);
			if (regsel & 2) WRITE_EA_32(ea, m_fpsr);
			if (regsel & 1) WRITE_EA_32(ea, m_fpiar);
		}
		else        // From <ea> to system control reg
		{
			if (regsel & 4) m_fpcr = READ_EA_32(ea) & FPCR_WRITE_MASK;
			if (regsel & 2) m_fpsr = READ_EA_32(ea) & FPSR_WRITE_MASK;
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
				m_fpcr = m68ki_read_32(address) & FPCR_WRITE_MASK;
				address += 4;
			}
			if (regsel & 2)
			{
				m_fpsr = m68ki_read_32(address) & FPSR_WRITE_MASK;
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

	if ((regsel & 4) && dir == 0)
	{
		apply_fpcr_rounding();
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

	if (dir)    // From FP regs to mem
	{
		switch (mode)
		{
			case 1: // dynamic register list, predecrement addressing mode
				reglist = REG_D()[(reglist >> 4) & 7];
				[[fallthrough]];
			case 0: // static register list, predecrement addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				u32 address = GET_EA_FPE(imode, reg);

				// the predecrement list has FP0 at bit 0; the transfer order is
				// FP7 first at descending addresses, so FP0 ends up at the
				// lowest address
				for (i = 7; i >= 0; i--)
				{
					if (reglist & (1 << i))
					{
						WRITE_EA_FPE(imode, reg, m_fpr[i], address);
						address += 12;

						m_icount -= 2;
					}
				}
				break;
			}

			case 3: // dynamic register list, postincrement or control addressing mode
				reglist = REG_D()[(reglist >> 4) & 7];
				[[fallthrough]];
			case 2: // static register list, postincrement or control addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				u32 address = GET_EA_FPE(imode, reg);

				// the postincrement/control list has FP0 at bit 7; the transfer
				// order is FP0 first at ascending addresses
				for (i = 0; i < 8; i++)
				{
					if (reglist & (0x80 >> i))
					{
						WRITE_EA_FPE(imode, reg, m_fpr[i], address);
						address += 12;

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
			case 3: // dynamic register list, postincrement or control addressing mode
				// FIXME: not really tested, but seems to work
				reglist = REG_D()[(reglist >> 4) & 7];
				[[fallthrough]];
			case 2: // static register list, postincrement or control addressing mode
			{
				int imode = (ea >> 3) & 0x7;
				int reg = (ea & 0x7);
				u32 address = GET_EA_FPE(imode, reg);

				// the postincrement/control list has FP0 at bit 7; the transfer
				// order is FP0 first at ascending addresses
				for (i = 0; i < 8; i++)
				{
					if (reglist & (0x80 >> i))
					{
						m_fpr[i] = READ_EA_FPE(imode, reg, address);
						address += 12;

						m_icount -= 2;
					}
				}
				break;
			}

			default:    fatalerror("M680x0: FMOVEM: mode %d unimplemented at %08X\n", mode, m_pc-4);
		}
	}
}

void m68000_musashi_device::fdbcc()
{
	m_fpu_just_reset = 0;

	const int condition = OPER_I_16() & 0x3f;

	if (!test_condition(condition))
	{
		u32 *r_dst = &REG_D()[m_ir & 7];
		const u32 res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if (res != 0xffff)
		{
			const u32 offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
		}
		else
		{
			m_pc += 2;
		}
	}
	else
	{
		m_pc += 2;
	}

	m_icount -= 7;
}

void m68000_musashi_device::fbcc16()
{
	m_fpu_just_reset = 0;

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
	m_fpu_just_reset = 0;

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
	apply_fpcr_rounding();
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
		// a NULL frame is a single format longword
		if (inc)
		{
			m68ki_write_32(addr, 0);
			if (reg != -1)
			{
				REG_A()[reg] += 4;
			}
		}
		else
		{
			m68ki_write_32(addr - 4, 0);
			if (reg != -1)
			{
				REG_A()[reg] -= 4;
			}
		}
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
			// how about an IDLE frame?  (the postincrement above already
			// consumed the 4 byte format longword)
			if (!m40 && ((temp & 0x00ff0000) == 0x00180000))
			{
				REG_A()[reg] += 6*4;
			}
			else if (m40 && ((temp & 0xffff0000) == 0x41000000))
			{
				// the format longword is the whole frame
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

void m68000_musashi_device::m68881_ftrap()
{
	m_fpu_just_reset = 0;

	const u16 w2 = OPER_I_16();

	// consume the operand before testing so a taken trap stacks the
	// address of the next instruction
	switch (m_ir & 0x7)
	{
		case 2: // word operand
			OPER_I_16();
			break;

		case 3: // long word operand
			OPER_I_32();
			break;

		default: // no operand
			break;
	}

	if (test_condition(w2 & 0x3f))
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);
	}
}

// Read the FPU's Coprocessor Interface Registers (CIRs).
// References: MC68881/68882 Coprocessor User's Manual 1st Edition,
// pages 7-1 to 7-8 and M68030 User's Manual 3rd Edition page 7-69.
u32 m68000_musashi_device::m6888x_read_cir(offs_t offset)
{
	// If no FPU is present, reading any CIRs causes a bus error.
	// Pre-1992 Macintosh ROMs use this method to detect the presence
	// of an FPU.  1992 and later ROMs just execute FNOP and check for
	// an F-line trap, because this mechanism does not exist on the 68040.
	if (!m_has_fpu)
	{
		m68k_cause_bus_error();
	}

	// TODO: actually try to return meaningful values?
	// offset   function
	// 0x00     Response            read-only       16 bit (value in D31-D16)
	// 0x02     Control             write-only      16
	// 0x04     Save                read            16
	// 0x06     Restore             read/write      16
	// 0x08     Operation Word      read/write      16
	// 0x0a     Command             write-only      16
	// 0x0c     (reserved)          N/A             16
	// 0x0e     Condition           write-only      16
	// 0x10     Operand             read/write      32 bit
	// 0x14     Register Select     read-only       16
	// 0x18     Instruction Address write-only      32
	// 0x1c     Operand Address     read/write      32
	return 0;
}

void m68000_musashi_device::m6888x_write_cir(offs_t offset, u32 data)
{
	if (!m_has_fpu)
	{
		m68k_cause_bus_error();
	}

	// TODO: actually do something with these values?
}
