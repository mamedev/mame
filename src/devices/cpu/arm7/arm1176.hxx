// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// ARM1176 system control, short-descriptor translation and data accesses.
// The memory system is uncached: page tables are walked for each translation.

// TODO: TrustZone banking, TCM, cache timing, internal DMA

void arm1176jzf_s_cpu_device::device_start()
{
	arm11_cpu_device::device_start();
	save_item(NAME(m_vbar));
	save_item(NAME(m_aux_control));
	save_item(NAME(m_cpacr));
	save_item(NAME(m_ttbr1));
	save_item(NAME(m_ttbcr));
	save_item(NAME(m_context_id));
	save_item(NAME(m_thread_id));
	save_item(NAME(m_ifar));
	save_item(NAME(m_prrr));
	save_item(NAME(m_nmrr));
	save_item(NAME(m_vfp_regs));
	save_item(NAME(m_fpscr));
	save_item(NAME(m_fpexc));
	for (unsigned i = 0; i < 32; i++)
		state_add(ARM7_LOGTLB + 1 + i, string_format("S%d", i).c_str(), m_vfp_regs[i]).formatstr("%08X");
	state_add(ARM7_LOGTLB + 33, "FPSCR", m_fpscr).formatstr("%08X");
	state_add(ARM7_LOGTLB + 34, "FPEXC", m_fpexc).formatstr("%08X");
	state_add(ARM7_LOGTLB + 35, "CPACR", m_cpacr).formatstr("%08X");
	state_add(ARM7_LOGTLB + 36, "TTBR0", m_tlbBase).formatstr("%08X");
	state_add(ARM7_LOGTLB + 37, "TTBR1", m_ttbr1).formatstr("%08X");
	state_add(ARM7_LOGTLB + 38, "TTBCR", m_ttbcr).formatstr("%08X");
	state_add(ARM7_LOGTLB + 39, "DACR", m_domainAccessControl).formatstr("%08X");
	state_add(ARM7_LOGTLB + 40, "DFSR", m_faultStatus[0]).formatstr("%08X");
	state_add(ARM7_LOGTLB + 41, "IFSR", m_faultStatus[1]).formatstr("%08X");
	state_add(ARM7_LOGTLB + 42, "DFAR", m_faultAddress).formatstr("%08X");
	state_add(ARM7_LOGTLB + 43, "IFAR", m_ifar).formatstr("%08X");
}

void arm1176jzf_s_cpu_device::device_reset()
{
	m_vbar = 0;
	arm11_cpu_device::device_reset();
	m_control = 0x00050078;
	m_aux_control = 7;
	m_cpacr = m_ttbr1 = m_ttbcr = m_context_id = m_ifar = 0;
	m_prrr = m_nmrr = m_fpscr = m_fpexc = 0;
	std::fill(std::begin(m_thread_id), std::end(m_thread_id), 0);
	std::fill(std::begin(m_vfp_regs), std::end(m_vfp_regs), 0);
	m_insn_prefetch_count = 0;
	m_insn_prefetch_index = 0;
}

uint32_t arm1176jzf_s_cpu_device::vector_base() const
{
	return m_vectorbase | ((m_control & 0x2000) ? 0xffff0000 : m_vbar);
}

void arm1176jzf_s_cpu_device::undefined_coprocessor()
{
	m_pendingUnd = true;
	update_irq_state();
}

bool arm1176jzf_s_cpu_device::handle_coprocessor(uint32_t insn)
{
	const unsigned cp = (insn >> 8) & 15;
	if (cp == 15 && (insn & 0x0fe00000) == 0x0c400000)
	{
		// ARM DDI 0301H, 3.2.22, table 3-73: MCRR/MCRR2 cache ranges.
		// Rd is the inclusive end VA, Rn the start VA, before FCSE translation.
		const unsigned crm = insn & 15, rd = (insn >> 12) & 15, rn = (insn >> 16) & 15;
		if (BIT(insn, 20) || (insn & 0xf0) || rd == 15 || rn == 15 || (crm != 5 && crm != 6 && crm != 12 && crm != 14) ||
				(GET_MODE == eARM7_MODE_USER && crm != 12))
		{
			undefined_coprocessor();
			return true;
		}
		const uint32_t start = GetRegister(rn) & ~31U, end = GetRegister(rd) & ~31U;
		if (BIT(m_aux_control, 5) || start > end)
			return true;
		// There are no dirty data-cache lines in this interpreter. Do not read
		// or write the target memory (it may be MMIO), but retain synchronous
		// translation faults. Check each 1KB region for legacy subpage AP bits.
		if (m_control & 1)
		{
			for (uint64_t va = start; va <= end; va = (va | 0x3ff) + 1)
			{
				offs_t physical = uint32_t(va);
				if (!walk_page_table(physical, ARM7_TLB_ABORT_D | ARM7_TLB_READ, true))
					return true;
			}
		}
		if (crm == 5)
		{
			m_insn_prefetch_count = 0;
			m_insn_prefetch_index = 0;
		}
		return true;
	}
	if (cp != 10 && cp != 11)
		return false;
	vfp_execute(insn);
	return true;
}

uint32_t arm1176jzf_s_cpu_device::arm7_rt_r_callback(offs_t insn)
{
	const unsigned crn = (insn >> 16) & 15, crm = insn & 15, op1 = (insn >> 21) & 7, op2 = (insn >> 5) & 7;
	if (((insn >> 8) & 15) != 15 || op1 != 0 || (GET_MODE == eARM7_MODE_USER && !(crn == 13 && crm == 0 && (op2 == 2 || op2 == 3))))
	{
		undefined_coprocessor();
		return 0;
	}
	if (crn == 0 && crm == 0)
	{
		switch (op2)
		{
		case 0:
			return m_copro_id;
		case 1:
			return 0x1d992992; // 32KB, four-way I/D caches (IGS38 configuration)
		case 2:
			return 0; // no TCM configured
		case 3:
			return 0x00000800; // eight lockable unified TLB entries
		case 5:
			return 0; // CPU ID in a uniprocessor
		}
	}
	// ARM1176 CPUID feature registers, ARM DDI 0301H section 3.2.6.
	if (crn == 0 && crm == 1)
		return std::array<uint32_t, 8>{ 0x00000111, 0x00000011, 0x00000033, 0, 0x01130003, 0x10030302, 0x01222100, 0 }[op2];
	if (crn == 0 && crm == 2 && op2 <= 5)
		return std::array<uint32_t, 6>{ 0x00140011, 0x12002111, 0x11231121, 0x01102131, 0x00001141, 0 }[op2];
	if (crn == 7 && crm == 10 && op2 == 6)
		return 0; // cache dirty status
	if (crm == 0)
	{
		switch (crn)
		{
		case 1:
			if (op2 == 0)
				return m_control;
			if (op2 == 1)
				return m_aux_control;
			if (op2 == 2)
				return m_cpacr;
			break;
		case 2:
			if (op2 == 0)
				return m_tlbBase;
			if (op2 == 1)
				return m_ttbr1;
			if (op2 == 2)
				return m_ttbcr;
			break;
		case 3:
			if (op2 == 0)
				return m_domainAccessControl;
			break;
		case 5:
			if (op2 < 2)
				return m_faultStatus[op2];
			break;
		case 6:
			if (op2 == 0)
				return m_faultAddress;
			if (op2 == 2)
				return m_ifar;
			break;
		case 12:
			if (op2 == 0)
				return m_vbar;
			break;
		case 13:
			if (op2 == 0)
				return m_fcsePID;
			if (op2 == 1)
				return m_context_id;
			if (op2 >= 2 && op2 <= 4)
				return m_thread_id[op2 - 2];
			break;
		}
	}
	if (crn == 10 && crm == 2 && op2 < 2)
		return op2 ? m_nmrr : m_prrr;
	// Cache-test operations complete immediately in the uncached implementation.
	if (crn == 7 && (crm == 10 || crm == 14) && op2 == 3)
		return 0x40000000;
	undefined_coprocessor();
	return 0;
}

void arm1176jzf_s_cpu_device::arm7_rt_w_callback(offs_t insn, uint32_t data)
{
	const unsigned crn = (insn >> 16) & 15, crm = insn & 15, op1 = (insn >> 21) & 7, op2 = (insn >> 5) & 7;
	if (((insn >> 8) & 15) != 15 || op1 != 0 ||
			(GET_MODE == eARM7_MODE_USER && !(crn == 13 && crm == 0 && op2 == 2) &&
					!(crn == 7 && ((crm == 5 && op2 == 4) || (crm == 10 && (op2 == 4 || op2 == 5))))))
	{
		undefined_coprocessor();
		return;
	}
	if (crm == 0)
	{
		switch (crn)
		{
		case 1:
			if (op2 == 0)
			{
				m_control = data;
				return;
			}
			if (op2 == 1)
			{
				m_aux_control = data;
				return;
			}
			if (op2 == 2)
			{
				m_cpacr = data & 0x00f00000;
				return;
			}
			break;
		case 2:
			if (op2 == 0)
			{
				m_tlbBase = data & 0xffffff9b;
				return;
			}
			if (op2 == 1)
			{
				m_ttbr1 = data & 0xffffc01b;
				return;
			}
			if (op2 == 2)
			{
				m_ttbcr = data & 0x37;
				return;
			}
			break;
		case 3:
			if (op2 == 0)
			{
				m_domainAccessControl = data;
				for (unsigned i = 0; i < 16; i++)
					m_decoded_access_control[i] = (data >> (2 * i)) & 3;
				return;
			}
			break;
		case 5:
			if (op2 < 2)
			{
				m_faultStatus[op2] = data;
				return;
			}
			break;
		case 6:
			if (op2 == 0)
			{
				m_faultAddress = data;
				return;
			}
			if (op2 == 2)
			{
				m_ifar = data;
				return;
			}
			break;
		case 12:
			if (op2 == 0)
			{
				m_vbar = data & ~31U;
				return;
			}
			break;
		case 13:
			if (op2 == 0)
			{
				m_fcsePID = m_pid_offset = data & 0xfe000000;
				return;
			}
			if (op2 == 1)
			{
				m_context_id = data;
				return;
			}
			if (op2 >= 2 && op2 <= 4)
			{
				m_thread_id[op2 - 2] = data;
				return;
			}
			break;
		}
	}
	if (crn == 10 && crm == 2 && op2 < 2)
	{
		(op2 ? m_nmrr : m_prrr) = data;
		return;
	}
	if (crn == 7)
	{
		if (crm == 0 && op2 == 4)
		{
			// DDI 0301H, 10.2.2: CP15 WFI resumes on an asserted IRQ/FIQ,
			// including one masked in CPSR. Memory accesses are synchronous here.
			if (!m_pendingIrq && !m_pendingFiq && !m_pendingAbtD && !m_pendingAbtP && !m_pendingUnd)
				spin_until_interrupt();
			return;
		}
		// ARM DDI 0301H, 3.2.22 (p. 3-71): these unified-cache
		// operations are defined to have no effect on ARM1176. In particular,
		// they do not flush the prefetch queue. The privilege check above applies.
		if ((crm == 7 && op2 != 0) || crm == 11 || crm == 15)
			return;
		// No write buffer/data cache. ISB and instruction-cache maintenance
		// discard the interpreter's instruction prefetch queue.
		if ((crm == 5 && (op2 == 0 || op2 == 1 || op2 == 4 || op2 == 6 || op2 == 7)) || (crm == 7 && op2 == 0))
		{
			m_insn_prefetch_count = 0;
			m_insn_prefetch_index = 0;
			return;
		}
		if ((crm == 13 && op2 == 1) || (crm == 5 && op2 == 2) || ((crm == 6 || crm == 10 || crm == 14) && op2 <= 2) ||
				(crm == 10 && (op2 == 4 || op2 == 5)))
			return;
	}
	// Translation is uncached, so all documented TLB invalidations are complete.
	if (crn == 8 && (crm == 5 || crm == 6 || crm == 7) && op2 <= 3)
		return;
	undefined_coprocessor();
}

bool arm1176jzf_s_cpu_device::translation_fault(uint32_t addr, uint32_t status, int flags, bool side_effects)
{
	if (side_effects)
	{
		if (flags & ARM7_TLB_ABORT_P)
		{
			m_faultStatus[1] = status;
			m_ifar = addr;
		}
		else
		{
			m_faultStatus[0] = status | ((flags & ARM7_TLB_WRITE) ? 0x800 : 0);
			m_faultAddress = addr;
			m_pendingAbtD = true;
			update_irq_state();
		}
	}
	return false;
}

bool arm1176jzf_s_cpu_device::walk_page_table(offs_t &addr, int flags, bool side_effects)
{
	const uint32_t va = addr < 0x02000000 ? addr + m_pid_offset : addr;
	const unsigned split = m_ttbcr & 7;
	const bool use_ttbr1 = split && (va >> (32 - split));
	const unsigned shift = use_ttbr1 ? 14 : 14 - split;
	if (BIT(m_ttbcr, use_ttbr1 ? 5 : 4))
		return translation_fault(va, 5, flags, side_effects);
	const uint32_t table = (use_ttbr1 ? m_ttbr1 : m_tlbBase) & (~0U << shift);
	const auto descriptor = [this](uint32_t address)
	{
		uint32_t value = m_program->read_dword(address);
		if (BIT(m_control, 25))
			value = swapendian_int32(value);
		return value;
	};
	const uint32_t l1 = descriptor(table | ((va >> 18) & ((1U << shift) - 4)));
	const bool xp = BIT(m_control, 23);
	const bool section = (l1 & 3) == 2;
	unsigned domain = (l1 >> 5) & 15;
	unsigned ap = 0;
	bool xn = false;
	uint32_t pa;
	if (section)
	{
		const bool super = BIT(l1, 18);
		const uint32_t mask = super ? 0xff000000 : 0xfff00000;
		pa = (l1 & mask) | (va & ~mask);
		if (super)
			domain = 0; // ARM1176 has a 32-bit physical address bus.
		ap = ((l1 >> 10) & 3) | (xp ? ((l1 >> 13) & 4) : 0);
		xn = xp && BIT(l1, 4);
	}
	else if ((l1 & 3) == 1)
	{
		const uint32_t l2 = descriptor((l1 & 0xfffffc00) | ((va >> 10) & 0x3fc));
		const unsigned type = l2 & 3;
		if (!type)
			return translation_fault(va, (domain << 4) | 7, flags, side_effects);
		const bool large = type == 1, extended_small = !xp && type == 3;
		const uint32_t mask = large ? 0xffff0000 : 0xfffff000;
		pa = (l2 & mask) | (va & ~mask);
		if (xp)
		{
			ap = ((l2 >> 4) & 3) | ((l2 >> 7) & 4);
			xn = large ? BIT(l2, 15) : BIT(l2, 0);
		}
		else
			ap = (l2 >> (4 + (extended_small ? 0 : large ? ((va >> 13) & 6) : ((va >> 9) & 6)))) & 3;
	}
	else
		return translation_fault(va, 5, flags, side_effects);
	const unsigned access = (m_domainAccessControl >> (domain * 2)) & 3;
	if (access == 0 || access == 2)
		return translation_fault(va, (domain << 4) | (section ? 9 : 11), flags, side_effects);
	if (access == 1)
	{
		if (xp && BIT(m_control, 29) && !(ap & 1))
			return translation_fault(va, (domain << 4) | (section ? 3 : 6), flags, side_effects);
		if (xp && BIT(m_control, 29))
			ap |= 1;
		const bool user = GET_MODE == eARM7_MODE_USER, write = flags & ARM7_TLB_WRITE;
		bool permitted = false;
		switch (ap)
		{
		case 0:
			if (!xp)
				permitted = !write && ((m_control & 0x200) || ((m_control & 0x100) && !user));
			break;
		case 1:
			permitted = !user;
			break;
		case 2:
			permitted = !user || !write;
			break;
		case 3:
			permitted = true;
			break;
		case 5:
			permitted = !user && !write;
			break;
		case 6:
		case 7:
			permitted = !write;
			break;
		}
		if (!permitted || (xn && (flags & ARM7_TLB_ABORT_P)))
			return translation_fault(va, (domain << 4) | (section ? 13 : 15), flags, side_effects);
	}
	addr = pa;
	return true;
}

bool arm1176jzf_s_cpu_device::translate_vaddr_to_paddr(offs_t &addr, const int flags)
{
	// Prefetch is speculative. Record IFSR/IFAR only if the CPU executes it.
	return walk_page_table(addr, flags, (flags & ARM7_TLB_ABORT_D) != 0);
}

void arm1176jzf_s_cpu_device::prefetch_abort(uint32_t pc)
{
	walk_page_table(pc, ARM7_TLB_ABORT_P | ARM7_TLB_READ, true);
}

bool arm1176jzf_s_cpu_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
	if (spacenum != AS_PROGRAM || !(m_control & 1))
		return true;
	const int flags =
			(intention == TR_FETCH ? ARM7_TLB_ABORT_P : ARM7_TLB_ABORT_D) | (intention == TR_WRITE ? ARM7_TLB_WRITE : ARM7_TLB_READ);
	return walk_page_table(address, flags, false);
}

bool arm1176jzf_s_cpu_device::data_address(offs_t &addr, unsigned size, bool write)
{
	if ((addr & (size - 1)) && (m_control & 2))
		return translation_fault(addr, 1, ARM7_TLB_ABORT_D | (write ? ARM7_TLB_WRITE : ARM7_TLB_READ), true);
	return !(m_control & 1) || translate_vaddr_to_paddr(addr, ARM7_TLB_ABORT_D | (write ? ARM7_TLB_WRITE : ARM7_TLB_READ));
}

uint32_t arm1176jzf_s_cpu_device::read_data(uint32_t addr, unsigned size)
{
	const bool unaligned = addr & (size - 1), modern = BIT(m_control, 22);
	if (unaligned && (m_control & 2))
	{
		data_address(addr, size, false);
		return 0;
	}
	const uint32_t original = addr;
	if (!modern)
		addr &= ~(size - 1);
	const bool big = BIT(GET_CPSR, 9);
	uint32_t result = 0;
	if (!unaligned || !modern)
	{
		if (!data_address(addr, size, false))
			return 0;
		result = size == 4 ? m_program->read_dword(addr) : size == 2 ? m_program->read_word(addr) : m_program->read_byte(addr);
		if (big && size == 4)
			result = swapendian_int32(result);
		if (big && size == 2)
			result = swapendian_int16(uint16_t(result));
	}
	else
	{
		for (unsigned i = 0; i < size; i++)
		{
			offs_t physical = addr + i;
			if (!data_address(physical, 1, false))
				return 0;
			result |= uint32_t(m_program->read_byte(physical)) << (8 * (big ? size - 1 - i : i));
		}
	}
	if (!modern && size == 4)
		result = std::rotr(result, (original & 3) * 8);
	return result;
}

void arm1176jzf_s_cpu_device::write_data(uint32_t addr, uint32_t data, unsigned size)
{
	const bool unaligned = addr & (size - 1), modern = BIT(m_control, 22);
	if (unaligned && (m_control & 2))
	{
		data_address(addr, size, true);
		return;
	}
	if (!modern)
		addr &= ~(size - 1);
	const bool big = BIT(GET_CPSR, 9);
	if (!unaligned || !modern)
	{
		if (!data_address(addr, size, true))
			return;
		if (big && size == 4)
			data = swapendian_int32(data);
		if (big && size == 2)
			data = swapendian_int16(uint16_t(data));
		if (size == 4)
			m_program->write_dword(addr, data);
		else if (size == 2)
			m_program->write_word(addr, data);
		else
			m_program->write_byte(addr, data);
	}
	else
	{
		for (unsigned i = 0; i < size; i++)
		{
			offs_t physical = addr + i;
			if (!data_address(physical, 1, true))
				return;
			m_program->write_byte(physical, data >> (8 * (big ? size - 1 - i : i)));
		}
	}
}

uint32_t arm1176jzf_s_cpu_device::arm7_cpu_read32(offs_t addr)
{
	return read_data(addr, 4);
}

uint32_t arm1176jzf_s_cpu_device::arm7_cpu_read16(offs_t addr)
{
	return read_data(addr, 2);
}

uint8_t arm1176jzf_s_cpu_device::arm7_cpu_read8(offs_t addr)
{
	return read_data(addr, 1);
}

void arm1176jzf_s_cpu_device::arm7_cpu_write32(offs_t addr, uint32_t data)
{
	write_data(addr, data, 4);
}

void arm1176jzf_s_cpu_device::arm7_cpu_write16(offs_t addr, uint16_t data)
{
	write_data(addr, data, 2);
}

void arm1176jzf_s_cpu_device::arm7_cpu_write8(offs_t addr, uint8_t data)
{
	write_data(addr, data, 1);
}

#include "arm1176vfp.hxx"
