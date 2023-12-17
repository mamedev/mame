// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_XTENSA_XTENSA_HELPER_H
#define MAME_CPU_XTENSA_XTENSA_HELPER_H

#pragma once

class xtensa_helper
{
public:
	static const char *const special_regs[256];
	static const char *const s_st1_ops[16];
	static const char *const s_tlb_ops[16];
	static const char *const s_rst2_ops[16];
	static const char *const s_rst3_ops[16];
	static const char *const s_fp0_ops[16];
	static const char *const s_fp1_ops[16];
	static const char *const s_lsai_ops[16];
	static const char *const s_cache_ops[16];
	static const char *const s_lsci_ops[4];
	static const char *const s_mac16_ops[4];
	static const char *const s_mac16_half[4];
	static const char *const s_bz_ops[4];
	static const char *const s_bi0_ops[4];
	static const int32_t s_b4const[16];
	static const uint32_t s_b4constu[16];
	static const char *const s_b_ops[16];

	static std::string format_imm(uint32_t imm)
	{
		if (s32(imm) < 0)
		{
			if (s32(imm < -9))
			{
				return util::string_format("-0x%X", -imm);
			}
			else
			{
				return util::string_format("-%X", -imm);
			}
		}
		else
		{
			if (imm > 9)
			{
				return util::string_format("0x%X", imm);
			}
			else
			{
				return util::string_format("%X", imm);
			}
		}
	}

	static std::string special_reg(uint8_t n, bool wsr)
	{
		if (n == 226 && !wsr)
			return "interrupt";

		const char *s = xtensa_helper::special_regs[n];
		if (s[0] == '\0')
			return util::string_format("s%u", n);
		else
			return s;
	}
};

#endif // MAME_CPU_XTENSA_XTENSA_HELPER_H
