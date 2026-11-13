// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// ARM1176 VFP2 register transfers, memory transfers and scalar arithmetic.
#include "softfloat3/source/include/softfloat.h"

bool arm1176jzf_s_cpu_device::vfp_enabled(bool system_register)
{
	const unsigned cp10 = (m_cpacr >> 20) & 3, cp11 = (m_cpacr >> 22) & 3;
	if (cp10 != cp11 || !(cp10 == 3 || (cp10 == 1 && GET_MODE != eARM7_MODE_USER)) || (!system_register && !BIT(m_fpexc, 30)))
	{
		undefined_coprocessor();
		return false;
	}
	return true;
}

void arm1176jzf_s_cpu_device::vfp_execute(uint32_t insn)
{
	const bool dp = BIT(insn, 8), load = BIT(insn, 20);
	const unsigned rd = (insn >> 12) & 15, rn = (insn >> 16) & 15, rm = insn & 15;
	const auto get_double = [this](unsigned reg)
	{
		return uint64_t(m_vfp_regs[2 * reg]) | (uint64_t(m_vfp_regs[2 * reg + 1]) << 32);
	};
	const auto put_double = [this](unsigned reg, uint64_t value)
	{
		m_vfp_regs[2 * reg] = uint32_t(value);
		m_vfp_regs[2 * reg + 1] = uint32_t(value >> 32);
	};
	if ((insn & 0x0f000010) == 0x0e000010) // MCR / MRC
	{
		if (dp)
		{
			undefined_coprocessor();
			return;
		}
		if ((insn & 0x00e0007f) == 0x10) // VMOV Sn <-> Rt
		{
			if (!vfp_enabled())
				return;
			if (rd == 15)
			{
				undefined_coprocessor();
				return;
			}
			const unsigned sn = rn * 2 + BIT(insn, 7);
			if (load)
				SetRegister(rd, m_vfp_regs[sn]);
			else
				m_vfp_regs[sn] = GetRegister(rd);
			return;
		}
		if ((insn & 0x00e000ff) == 0x00e00010 && (rn == 0 || rn == 1 || rn == 6 || rn == 7 || rn == 8))
		{
			// FPSID/MVFR can be read in User mode when EN is set; only
			// privileged modes can access them with EN clear (DDI 0301H 20-4).
			if (!vfp_enabled(rn != 1 && GET_MODE != eARM7_MODE_USER))
				return;
			if (rn == 8 && GET_MODE == eARM7_MODE_USER)
			{
				undefined_coprocessor();
				return;
			}
			if (load)
			{
				const uint32_t value = rn == 0 ? 0x410120b5 : rn == 1 ? m_fpscr : rn == 6 ? 0 : rn == 7 ? 0x11111111 : m_fpexc;
				if (rd == 15 && rn == 1)
					set_cpsr((GET_CPSR & 0x0fffffff) | (value & 0xf0000000));
				else if (rd != 15)
					SetRegister(rd, value);
				else
					undefined_coprocessor();
			}
			else if (rd == 15)
				undefined_coprocessor();
			else if (rn == 1)
			{
				// This implementation provides non-trapping VFP arithmetic. Trap
				// enable bits are RAZ/WI until VFP11 support-code bounce is modeled.
				m_fpscr = GetRegister(rd) & 0xf3f7009f;
			}
			else if (rn == 8)
				m_fpexc = GetRegister(rd) & 0x40000000;
			return;
		}
		undefined_coprocessor();
		return;
	}
	if (!vfp_enabled())
		return;
	if ((insn & 0x0fe00000) == 0x0c400000) // MRRC / MCRR
	{
		const unsigned first = dp ? rm * 2 : rm * 2 + BIT(insn, 5);
		if ((insn & 0xd0) != 0x10 || (dp && BIT(insn, 5)) || first >= 31 || rd == 15 || rn == 15)
		{
			undefined_coprocessor();
			return;
		}
		if (load)
		{
			SetRegister(rd, m_vfp_regs[first]);
			SetRegister(rn, m_vfp_regs[first + 1]);
		}
		else
		{
			m_vfp_regs[first] = GetRegister(rd);
			m_vfp_regs[first + 1] = GetRegister(rn);
		}
		return;
	}
	if ((insn & 0x0e000000) == 0x0c000000) // VLDR/VSTR/VLDM/VSTM
	{
		const unsigned first = dp ? rd * 2 : rd * 2 + BIT(insn, 22);
		const bool pre = BIT(insn, 24), up = BIT(insn, 23), wb = BIT(insn, 21);
		const bool single = pre && !wb;
		const unsigned count = single ? (dp ? 2 : 1) : dp ? (insn & 254) : (insn & 255);
		if ((dp && BIT(insn, 22)) || !count || first + count > 32 || (!single && !(up ? !pre : (pre && wb))) || (wb && rn == 15))
		{
			undefined_coprocessor();
			return;
		}
		const uint32_t base = rn == 15 ? (R15 + 8) & ~3U : GetRegister(rn);
		const uint32_t offset = (insn & 255) * 4;
		uint32_t addr = single ? (up ? base + offset : base - offset) : (up ? base : base - offset);
		if (addr & 3)
		{
			translation_fault(addr, 1, ARM7_TLB_ABORT_D | (load ? ARM7_TLB_READ : ARM7_TLB_WRITE), true);
			return;
		}
		for (unsigned i = 0; i < count; i++, addr += 4)
		{
			const unsigned reg = first + (dp && BIT(GET_CPSR, 9) ? (i ^ 1) : i);
			if (load)
			{
				const uint32_t value = READ32(addr);
				if (m_pendingAbtD)
					return;
				m_vfp_regs[reg] = value;
			}
			else
			{
				WRITE32(addr, m_vfp_regs[reg]);
				if (m_pendingAbtD)
					return;
			}
		}
		if (wb)
			SetRegister(rn, up ? base + offset : base - offset);
		return;
	}
	if ((insn & 0x0f000010) != 0x0e000000)
	{
		undefined_coprocessor();
		return;
	}
	// Arithmetic supports scalar and VFP2 short-vector register banks.
	const unsigned d = dp ? rd : rd * 2 + BIT(insn, 22);
	const unsigned n = dp ? rn : rn * 2 + BIT(insn, 7);
	const unsigned m = dp ? rm : rm * 2 + BIT(insn, 5);
	const unsigned operation = ((insn >> 20) & 0xb);
	const unsigned ext = rn;
	if (dp && (BIT(insn, 22) || BIT(insn, 5)) && !(operation == 11 && (ext == 7 || ext == 8 || ext == 12 || ext == 13)))
	{
		undefined_coprocessor();
		return;
	}
	const unsigned bank_size = dp ? 4 : 8;
	const bool vector_operation = operation <= 3 || operation == 8 || (operation == 11 && ext <= 1);
	if (vector_operation && d >= bank_size && (m_fpscr & 0x00070000))
	{
		const unsigned length = ((m_fpscr >> 16) & 7) + 1, stride_code = (m_fpscr >> 20) & 3;
		const unsigned stride = stride_code == 3 ? 2 : 1;
		if ((stride_code != 0 && stride_code != 3) || length * stride > bank_size)
		{
			undefined_coprocessor();
			return;
		}
		const uint32_t vector_control = m_fpscr & 0x00370000;
		m_fpscr &= ~0x00370000U;
		for (unsigned i = 0; i < length; i++)
		{
			const auto advance = [bank_size, stride, i](unsigned reg)
			{
				return (reg & ~(bank_size - 1)) | ((reg + i * stride) & (bank_size - 1));
			};
			const unsigned vd = advance(d), vn = advance(n), vm = m < bank_size ? m : advance(m);
			uint32_t scalar = insn & ~0x0040f02fU;
			if (dp)
				scalar |= (vd << 12) | vm;
			else
				scalar |= ((vd >> 1) << 12) | ((vd & 1) << 22) | (vm >> 1) | ((vm & 1) << 5);
			if (operation != 11)
			{
				scalar &= ~0x000f0080U;
				scalar |= dp ? (vn << 16) : ((vn >> 1) << 16) | ((vn & 1) << 7);
			}
			vfp_execute(scalar);
			if (m_pendingUnd)
				break;
		}
		m_fpscr |= vector_control;
		return;
	}

	// SoftFloat state is shared with other CPUs. Restore it on every exit.
	struct float_environment
	{
		uint_fast8_t round = softfloat_roundingMode, flags = softfloat_exceptionFlags, tiny = softfloat_detectTininess;

		~float_environment()
		{
			softfloat_roundingMode = round;
			softfloat_exceptionFlags = flags;
			softfloat_detectTininess = tiny;
		}
	} environment;

	softfloat_roundingMode = std::array<uint_fast8_t, 4>{ softfloat_round_near_even, softfloat_round_max, softfloat_round_min,
		softfloat_round_minMag }[(m_fpscr >> 22) & 3];
	softfloat_exceptionFlags = 0;
	softfloat_detectTininess = softfloat_tininess_beforeRounding;
	const auto input32 = [this](uint32_t value)
	{
		if ((m_fpscr & 0x01000000) && !(value & 0x7f800000) && (value & 0x007fffff))
		{
			m_fpscr |= 0x80;
			return value & 0x80000000;
		}
		return value;
	};
	const auto input64 = [this](uint64_t value)
	{
		if ((m_fpscr & 0x01000000) && !(value & 0x7ff0000000000000ULL) && (value & 0x000fffffffffffffULL))
		{
			m_fpscr |= 0x80;
			return value & uint64_t(0x8000000000000000ULL);
		}
		return value;
	};
	const auto put32 = [this](unsigned reg, uint32_t value)
	{
		if ((m_fpscr & 0x02000000) && (value & 0x7fffffff) > 0x7f800000)
			value = 0x7fc00000;
		if ((m_fpscr & 0x01000000) && !(value & 0x7f800000) && (value & 0x007fffff))
		{
			value &= 0x80000000;
			m_fpscr |= 8;
		}
		m_vfp_regs[reg] = value;
	};
	const auto put64 = [this, &put_double](unsigned reg, uint64_t value)
	{
		if ((m_fpscr & 0x02000000) && (value & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL)
			value = 0x7ff8000000000000ULL;
		if ((m_fpscr & 0x01000000) && !(value & 0x7ff0000000000000ULL) && (value & 0x000fffffffffffffULL))
		{
			value &= 0x8000000000000000ULL;
			m_fpscr |= 8;
		}
		put_double(reg, value);
	};
	const auto binary32 = [](float32_t a, float32_t b, auto operation)
	{
		const bool an = (a.v & 0x7fffffff) > 0x7f800000, bn = (b.v & 0x7fffffff) > 0x7f800000;
		const bool as = an && !(a.v & 0x00400000), bs = bn && !(b.v & 0x00400000);
		if (as || bs)
			softfloat_exceptionFlags |= softfloat_flag_invalid;
		if (an || bn)
			return float32_t{ ((as ? a.v : bs ? b.v : an ? a.v : b.v) | 0x00400000) };
		const float32_t result = operation(a, b);
		return (result.v & 0x7fffffff) > 0x7f800000 ? float32_t{ 0x7fc00000 } : result;
	};
	const auto binary64 = [](float64_t a, float64_t b, auto operation)
	{
		const bool an = (a.v & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL, bn = (b.v & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL;
		const bool as = an && !(a.v & 0x0008000000000000ULL), bs = bn && !(b.v & 0x0008000000000000ULL);
		if (as || bs)
			softfloat_exceptionFlags |= softfloat_flag_invalid;
		if (an || bn)
			return float64_t{ ((as ? a.v : bs ? b.v : an ? a.v : b.v) | 0x0008000000000000ULL) };
		const float64_t result = operation(a, b);
		return (result.v & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL ? float64_t{ 0x7ff8000000000000ULL } : result;
	};
	if (operation == 11)
	{
		if (ext <= 1 && BIT(insn, 6)) // VMOV, VABS, VNEG, VSQRT
		{
			if (dp)
			{
				uint64_t value = get_double(m);
				if (ext == 0 && BIT(insn, 7))
					value &= 0x7fffffffffffffffULL;
				if (ext == 1)
					value = BIT(insn, 7) ? f64_sqrt(float64_t{ input64(value) }).v : value ^ 0x8000000000000000ULL;
				if (ext == 1 && BIT(insn, 7) && (get_double(m) & 0x7fffffffffffffffULL) <= 0x7ff0000000000000ULL &&
						(value & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL)
					value = 0x7ff8000000000000ULL;
				if (ext == 1 && BIT(insn, 7))
					put64(d, value);
				else
					put_double(d, value);
			}
			else
			{
				uint32_t value = m_vfp_regs[m];
				if (ext == 0 && BIT(insn, 7))
					value &= 0x7fffffff;
				if (ext == 1)
					value = BIT(insn, 7) ? f32_sqrt(float32_t{ input32(value) }).v : value ^ 0x80000000;
				if (ext == 1 && BIT(insn, 7) && (m_vfp_regs[m] & 0x7fffffff) <= 0x7f800000 && (value & 0x7fffffff) > 0x7f800000)
					value = 0x7fc00000;
				if (ext == 1 && BIT(insn, 7))
					put32(d, value);
				else
					m_vfp_regs[d] = value;
			}
		}
		else if ((ext == 4 || ext == 5) && BIT(insn, 6)) // VCMP/VCMPE
		{
			bool nan, eq, lt;
			if (dp)
			{
				const float64_t a{ input64(get_double(d)) }, b{ ext == 5 ? 0 : input64(get_double(m)) };
				nan = (a.v & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL || (b.v & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL;
				eq = f64_eq(a, b);
				lt = BIT(insn, 7) ? f64_lt(a, b) : f64_lt_quiet(a, b);
			}
			else
			{
				const float32_t a{ input32(m_vfp_regs[d]) }, b{ ext == 5 ? 0 : input32(m_vfp_regs[m]) };
				nan = (a.v & 0x7fffffff) > 0x7f800000 || (b.v & 0x7fffffff) > 0x7f800000;
				eq = f32_eq(a, b);
				lt = BIT(insn, 7) ? f32_lt(a, b) : f32_lt_quiet(a, b);
			}
			m_fpscr = (m_fpscr & 0x0fffffff) | (nan ? 0x30000000 : eq ? 0x60000000 : lt ? 0x80000000 : 0x20000000);
		}
		else if (ext == 7 && (insn & 0xc0) == 0xc0) // precision conversion
		{
			if (dp && !BIT(insn, 5))
				put32(rd * 2 + BIT(insn, 22), f64_to_f32(float64_t{ input64(get_double(rm)) }).v);
			else if (!dp && !BIT(insn, 22))
				put64(rd, f32_to_f64(float32_t{ input32(m_vfp_regs[rm * 2 + BIT(insn, 5)]) }).v);
			else
				undefined_coprocessor();
		}
		else if (ext == 8 && BIT(insn, 6)) // integer to float
		{
			const uint32_t value = m_vfp_regs[rm * 2 + BIT(insn, 5)];
			if (dp && !BIT(insn, 22))
				put64(rd, BIT(insn, 7) ? i32_to_f64(int32_t(value)).v : ui32_to_f64(value).v);
			else if (!dp)
				put32(d, BIT(insn, 7) ? i32_to_f32(int32_t(value)).v : ui32_to_f32(value).v);
			else
				undefined_coprocessor();
		}
		else if ((ext == 12 || ext == 13) && BIT(insn, 6)) // float to integer
		{
			const auto rounding = BIT(insn, 7) ? softfloat_round_minMag : softfloat_roundingMode;
			uint32_t value;
			if (dp && !BIT(insn, 5))
			{
				const float64_t src{ input64(get_double(rm)) };
				value = ext == 13 ? uint32_t(f64_to_i32(src, rounding, true)) : uint32_t(f64_to_ui32(src, rounding, true));
				if (softfloat_exceptionFlags & softfloat_flag_invalid)
					value = (src.v & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL ? 0
							: ext == 13 ? (BIT(src.v, 63) ? 0x80000000 : 0x7fffffff)
							: (BIT(src.v, 63) ? 0 : 0xffffffff);
			}
			else if (!dp)
			{
				const float32_t src{ input32(m_vfp_regs[m]) };
				value = ext == 13 ? uint32_t(f32_to_i32(src, rounding, true)) : uint32_t(f32_to_ui32(src, rounding, true));
				if (softfloat_exceptionFlags & softfloat_flag_invalid)
					value = (src.v & 0x7fffffff) > 0x7f800000 ? 0
							: ext == 13 ? (BIT(src.v, 31) ? 0x80000000 : 0x7fffffff)
							: (BIT(src.v, 31) ? 0 : 0xffffffff);
			}
			else
			{
				undefined_coprocessor();
				return;
			}
			m_vfp_regs[rd * 2 + BIT(insn, 22)] = value;
		}
		else
		{
			undefined_coprocessor();
			return;
		}
	}
	else if (operation == 0 || operation == 1 || operation == 2 || operation == 3 || (operation == 8 && !BIT(insn, 6)))
	{
		if (dp && BIT(insn, 7))
		{
			undefined_coprocessor();
			return;
		}
		if (dp)
		{
			const float64_t a{ input64(get_double(n)) }, b{ input64(get_double(m)) };
			float64_t value;
			if (operation == 3)
				value = BIT(insn, 6) ? binary64(a, b, f64_sub) : binary64(a, b, f64_add);
			else if (operation == 8)
				value = binary64(a, b, f64_div);
			else
			{
				value = binary64(a, b, f64_mul);
				if (operation <= 1)
					value = (BIT(insn, 6) ^ (operation == 1)) ? binary64(float64_t{ input64(get_double(d)) }, value, f64_sub)
							: binary64(float64_t{ input64(get_double(d)) }, value, f64_add);
				if (operation == 1 || (operation == 2 && BIT(insn, 6)))
					value.v ^= 0x8000000000000000ULL;
			}
			put64(d, value.v);
		}
		else
		{
			const float32_t a{ input32(m_vfp_regs[n]) }, b{ input32(m_vfp_regs[m]) };
			float32_t value;
			if (operation == 3)
				value = BIT(insn, 6) ? binary32(a, b, f32_sub) : binary32(a, b, f32_add);
			else if (operation == 8)
				value = binary32(a, b, f32_div);
			else
			{
				value = binary32(a, b, f32_mul);
				if (operation <= 1)
					value = (BIT(insn, 6) ^ (operation == 1)) ? binary32(float32_t{ input32(m_vfp_regs[d]) }, value, f32_sub)
							: binary32(float32_t{ input32(m_vfp_regs[d]) }, value, f32_add);
				if (operation == 1 || (operation == 2 && BIT(insn, 6)))
					value.v ^= 0x80000000;
			}
			put32(d, value.v);
		}
	}
	else
	{
		undefined_coprocessor();
		return;
	}
	const uint32_t exceptions = ((softfloat_exceptionFlags & softfloat_flag_invalid) ? 1 : 0) |
			((softfloat_exceptionFlags & softfloat_flag_infinite) ? 2 : 0) |
			((softfloat_exceptionFlags & softfloat_flag_overflow) ? 4 : 0) |
			((softfloat_exceptionFlags & softfloat_flag_underflow) ? 8 : 0) |
			((softfloat_exceptionFlags & softfloat_flag_inexact) ? 16 : 0);
	m_fpscr |= exceptions;
}
