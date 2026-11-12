// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// ARMv6 integer and synchronization instructions shared by ARM11 cores.

void arm7_cpu_device::armv6_cps(uint32_t insn)
{
	if (GET_MODE == eARM7_MODE_USER)
		return;
	uint32_t psr = GET_CPSR;
	if (BIT(insn, 19))
	{
		const uint32_t mask = insn & 0x1c0;
		psr = BIT(insn, 18) ? (psr | mask) : (psr & ~mask);
	}
	if (BIT(insn, 17))
	{
		const unsigned mode = insn & 31;
		if (mode == 0x10 || mode == 0x11 || mode == 0x12 || mode == 0x13 || mode == 0x17 || mode == 0x1b || mode == 0x1f)
			psr = (psr & ~31U) | mode;
	}
	set_cpsr(psr);
}

void arm7_cpu_device::armv6_exclusive(uint32_t insn)
{
	const unsigned kind = (insn >> 21) & 3;
	const unsigned size = std::array<unsigned, 4>{ 4, 8, 1, 2 }[kind];
	const unsigned rn = (insn >> 16) & 15, rd = (insn >> 12) & 15, rm = insn & 15;
	const bool load = BIT(insn, 20);
	if (m_archRev < 6 || (kind && !(m_archFlags & ARCHFLAG_K)) || rn == 15 || rd == 15 ||
			(load ? rm != 15 : (rm == 15 || rd == rn || rd == rm)) || (size == 8 && ((load ? rd : rm) & 1)))
	{
		arm7ops_undef_conditional(insn);
		return;
	}
	const uint32_t addr = GetRegister(rn);
	if (addr & (size - 1))
	{
		m_exclusive_valid = false;
		m_faultStatus[0] = 1 | (load ? 0 : 0x800);
		m_faultAddress = addr;
		m_pendingAbtD = true;
		update_irq_state();
		R15 += 4;
		return;
	}
	offs_t physical = addr;
	if ((m_control & COPRO_CTRL_MMU_EN) && !translate_vaddr_to_paddr(physical, ARM7_TLB_ABORT_D | (load ? ARM7_TLB_READ : ARM7_TLB_WRITE)))
	{
		m_exclusive_valid = false;
		R15 += 4;
		return;
	}
	if (load)
	{
		const uint32_t lo = size == 1 ? READ8(addr) : size == 2 ? READ16(addr) : READ32(addr);
		const uint32_t hi = (size == 8 && !m_pendingAbtD) ? READ32(addr + 4) : 0;
		m_exclusive_valid = !m_pendingAbtD;
		if (m_exclusive_valid)
		{
			SetRegister(rd, lo);
			if (size == 8)
				SetRegister(rd + 1, hi);
			m_exclusive_address = physical;
			m_exclusive_size = size;
		}
	}
	else
	{
		const bool success = m_exclusive_valid && m_exclusive_address == physical && m_exclusive_size == size;
		m_exclusive_valid = false;
		if (success)
		{
			if (size == 1)
				WRITE8(addr, GetRegister(rm));
			else if (size == 2)
				WRITE16(addr, GetRegister(rm));
			else
			{
				WRITE32(addr, GetRegister(rm));
				if (size == 8 && !m_pendingAbtD)
					WRITE32(addr + 4, GetRegister(rm + 1));
			}
		}
		if (!m_pendingAbtD)
			SetRegister(rd, success ? 0 : 1);
	}
	R15 += 4;
}

void arm7_cpu_device::armv6_media(uint32_t insn)
{
	const unsigned rd = (insn >> 12) & 15, rn = (insn >> 16) & 15, rm = insn & 15;
	const uint32_t n = GetRegister(rn), m = GetRegister(rm);
	uint32_t result = 0;
	// REV, REV16 and REVSH.
	const uint32_t reverse = insn & 0x0fff0ff0;
	if (reverse == 0x06bf0f30 || reverse == 0x06bf0fb0 || reverse == 0x06ff0fb0)
	{
		result = ((m & 0x00ff00ff) << 8) | ((m & 0xff00ff00) >> 8);
		if (reverse == 0x06bf0f30)
			result = std::rotl(result, 16);
		if (reverse == 0x06ff0fb0)
			result = uint32_t(int32_t(int16_t(result)));
	}
	// Sign/zero extend, optionally adding to one word or two halfwords.
	else if ((insn & 0x0f8003f0) == 0x06800070 && ((insn >> 20) & 3) != 1)
	{
		const uint32_t value = std::rotr(m, 8 * ((insn >> 10) & 3));
		const unsigned kind = (insn >> 20) & 3;
		const bool uns = BIT(insn, 22);
		const uint32_t base = rn == 15 ? 0 : n;
		if (kind == 0)
		{
			const uint32_t lo = uns ? (value & 0xff) : uint32_t(int32_t(int8_t(value)));
			const uint32_t hi = uns ? ((value >> 16) & 0xff) : uint32_t(int32_t(int8_t(value >> 16)));
			result = ((base + lo) & 0xffff) | ((((base >> 16) + hi) & 0xffff) << 16);
		}
		else if (kind == 2)
			result = base + (uns ? (value & 0xff) : uint32_t(int32_t(int8_t(value))));
		else
			result = base + (uns ? (value & 0xffff) : uint32_t(int32_t(int16_t(value))));
	}
	// PKHBT / PKHTB.
	else if ((insn & 0x0ff00030) == 0x06800010)
	{
		const unsigned shift = (insn >> 7) & 31;
		result = BIT(insn, 6) ? ((n & 0xffff0000) | ((int32_t(m) >> (shift ? shift : 31)) & 0xffff))
				: ((n & 0xffff) | ((m << shift) & 0xffff0000));
	}
	// SSAT / USAT and the two-halfword variants. Q is sticky.
	else if ((insn & 0x0fa00030) == 0x06a00010 || (insn & 0x0fb00ff0) == 0x06a00f30)
	{
		const bool uns = BIT(insn, 22), half = (insn & 0x0fb00ff0) == 0x06a00f30;
		const unsigned bits = ((insn >> 16) & (half ? 15 : 31)) + (uns ? 0 : 1);
		const int64_t low = uns ? 0 : -(int64_t(1) << (bits - 1));
		const int64_t high = (int64_t(1) << (bits - (uns ? 0 : 1))) - 1;
		const unsigned shift = (insn >> 7) & 31;
		const int32_t shifted = BIT(insn, 6) ? int32_t(m) >> (shift ? shift : 31) : int32_t(m << shift);
		for (unsigned lane = 0; lane < (half ? 2 : 1); lane++)
		{
			const int64_t value = half ? int16_t(m >> (lane * 16)) : shifted;
			const int64_t clamped = std::clamp(value, low, high);
			if (clamped != value)
				set_cpsr(GET_CPSR | Q_MASK);
			result |= half ? (uint32_t(clamped) & 0xffff) << (lane * 16) : uint32_t(clamped);
		}
	}
	// SEL chooses each byte using the matching CPSR.GE bit.
	else if ((insn & 0x0ff00ff0) == 0x06800fb0)
	{
		for (unsigned lane = 0; lane < 4; lane++)
			result |= (BIT(GET_CPSR, 16 + lane) ? n : m) & (0xffU << (8 * lane));
	}
	// Parallel signed/unsigned add/subtract, saturating and halving forms.
	else if ((insn & 0x0f800f10) == 0x06000f10)
	{
		const unsigned group = (insn >> 20) & 7, op = (insn >> 5) & 7;
		if ((group & 3) == 0 || op == 5 || op == 6)
		{
			arm7ops_undef_conditional(insn);
			return;
		}
		const bool uns = group >= 5, saturate = (group & 3) == 2, halve = (group & 3) == 3;
		const unsigned bits = op >= 4 ? 8 : 16;
		const uint32_t mask = (1U << bits) - 1;
		uint32_t ge = 0;
		for (unsigned lane = 0; lane < 32 / bits; lane++)
		{
			const unsigned other = (op == 1 || op == 2) ? 1 - lane : lane;
			const bool sub = op == 3 || op == 7 || (op == 1 && lane == 0) || (op == 2 && lane == 1);
			const int64_t a = uns ? int64_t((n >> (bits * lane)) & mask) : int64_t(util::sext(n >> (bits * lane), bits));
			const int64_t b = uns ? int64_t((m >> (bits * other)) & mask) : int64_t(util::sext(m >> (bits * other), bits));
			int64_t value = sub ? a - b : a + b;
			if (saturate)
				value = std::clamp(
						value, uns ? int64_t(0) : -(int64_t(1) << (bits - 1)), uns ? int64_t(mask) : (int64_t(1) << (bits - 1)) - 1);
			else if (halve)
				value >>= 1;
			else if (uns ? (sub ? value >= 0 : value > mask) : value >= 0)
				ge |= (bits == 16 ? 3 : 1) << (lane * bits / 8);
			result |= (uint32_t(value) & mask) << (bits * lane);
		}
		if (!saturate && !halve)
			set_cpsr((GET_CPSR & ~0x000f0000U) | (ge << 16));
	}
	// Dual halfword multiply, optionally accumulating a 32-bit or 64-bit value.
	else if ((insn & 0x0fb00090) == 0x07000010)
	{
		const bool wide = BIT(insn, 22), subtract = BIT(insn, 6);
		const unsigned rs = (insn >> 8) & 15;
		const uint32_t a = GetRegister(rm), b = BIT(insn, 5) ? std::rotl(GetRegister(rs), 16) : GetRegister(rs);
		const int64_t low = int32_t(int16_t(a)) * int32_t(int16_t(b));
		const int64_t high = int32_t(int16_t(a >> 16)) * int32_t(int16_t(b >> 16));
		int64_t product = subtract ? low - high : low + high;
		if (rn == 15 || rm == 15 || rs == 15 || (wide && (rd == 15 || rn == rd)))
		{
			arm7ops_undef_conditional(insn);
			return;
		}
		if (wide)
		{
			const uint64_t accum = uint64_t(GetRegister(rd)) | (uint64_t(GetRegister(rn)) << 32);
			const uint64_t value = accum + uint64_t(product);
			SetRegister(rd, uint32_t(value));
			SetRegister(rn, uint32_t(value >> 32));
		}
		else
		{
			if (product != int64_t(int32_t(product)))
				set_cpsr(GET_CPSR | Q_MASK);
			if (rd != 15)
				product += int32_t(GetRegister(rd));
			if (product != int64_t(int32_t(product)))
				set_cpsr(GET_CPSR | Q_MASK);
			SetRegister(rn, uint32_t(product));
		}
		R15 += 4;
		return;
	}
	// Signed high-word multiply with optional accumulate/subtract and rounding.
	else if ((insn & 0x0ff000d0) == 0x07500010 || (insn & 0x0ff000d0) == 0x075000d0)
	{
		const unsigned rs = (insn >> 8) & 15;
		const bool subtract = BIT(insn, 7);
		if (rn == 15 || rm == 15 || rs == 15 || (subtract && rd == 15))
		{
			arm7ops_undef_conditional(insn);
			return;
		}
		const int64_t product = int64_t(int32_t(GetRegister(rm))) * int64_t(int32_t(GetRegister(rs)));
		const uint64_t accum = rd == 15 ? 0 : uint64_t(GetRegister(rd)) << 32;
		uint64_t value = subtract ? accum - uint64_t(product) : accum + uint64_t(product);
		if (BIT(insn, 5))
			value += 0x80000000;
		SetRegister(rn, uint32_t(value >> 32));
		R15 += 4;
		return;
	}
	else if ((insn & 0x0ff000f0) == 0x07800010) // USAD8 / USADA8
	{
		const uint32_t a = GetRegister(insn & 15), b = GetRegister((insn >> 8) & 15);
		for (unsigned lane = 0; lane < 4; lane++)
			result += std::abs(int((a >> (lane * 8)) & 255) - int((b >> (lane * 8)) & 255));
		if (rd != 15)
			result += GetRegister(rd);
		SetRegister(rn, result);
		R15 += 4;
		return;
	}
	else
	{
		arm7ops_undef_conditional(insn);
		return;
	}
	if (rd == 15 || rm == 15)
	{
		arm7ops_undef_conditional(insn);
		return;
	}
	SetRegister(rd, result);
	R15 += 4;
}
