// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact Core
\*********************************/

#ifndef MAME_CPU_ARCOMPACT_ARCOMPACT_COMMON_H
#define MAME_CPU_ARCOMPACT_ARCOMPACT_COMMON_H

#pragma once


class arcompact_common
{
protected:
	// registers used in 16-bit opcodes have a limited range
	// and can only address registers r0-r3 and r12-r15
	static constexpr uint8_t common_expand_reg(uint8_t reg)
	{
		return (reg > 3) ? (reg + 8) : reg;
	}

	static constexpr uint8_t common16_get_and_expand_breg(uint16_t op)
	{
		return common_expand_reg((op & 0x0700) >> 8);
	}

	static constexpr uint8_t common16_get_and_expand_creg(uint16_t op)
	{
		return common_expand_reg((op & 0x00e0) >> 5);
	}

	static constexpr uint8_t common16_get_and_expand_areg(uint16_t op)
	{
		return common_expand_reg(op & 0x0007);
	}

	static constexpr uint32_t common16_get_u3(uint16_t op)
	{
		return op & 0x0007;
	}

	static constexpr uint32_t common16_get_u5(uint16_t op)
	{
		return op & 0x001f;
	}

	static constexpr uint32_t common16_get_u7(uint16_t op)
	{
		return op & 0x007f;
	}

	static constexpr uint32_t common16_get_u8(uint16_t op)
	{
		return op & 0x00ff;
	}

	static constexpr uint32_t common16_get_s9(uint16_t op)
	{
		return util::sext(op, 9);
	}

	static constexpr uint8_t common32_get_areg(uint32_t op)
	{
		return op & 0x0000003f;
	}

	static constexpr uint8_t common32_get_areg_reserved(uint32_t op)
	{
		return op & 0x0000003f;
	}

	static constexpr uint8_t common32_get_breg(uint32_t op)
	{
		return ((op & 0x07000000) >> 24) | (((op & 0x00007000) >> 12) << 3);
	}

	static constexpr uint8_t common32_get_creg(uint32_t op)
	{
		return (op & 0x00000fc0) >> 6;
	}

	static constexpr uint8_t common32_get_condition(uint32_t op)
	{
		return op & 0x0000001f;
	}

	static constexpr bool common32_get_F(uint32_t op)
	{
		return (op & 0x00008000) ? true : false;
	}

	static constexpr uint8_t common32_get_p(uint32_t op)
	{
		return (op & 0x00c00000) >> 22;
	}

	static constexpr uint32_t common32_get_u6(uint32_t op)
	{
		return (op & 0x00000fc0) >> 6;
	}

	static constexpr uint32_t common32_get_s12(uint32_t op)
	{
		return util::sext(((op & 0x00000fc0) >> 6) | ((op & 0x0000003f) << 6), 12);
	}
};

#endif // MAME_CPU_ARCOMPACT_ARCOMPACT_COMMON_H
