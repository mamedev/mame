// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett, Samuele Zannoli

uint64_t pentium_device::opcode_rdmsr(bool &valid_msr)
{
	uint32_t offset = REG32(ECX);

	switch (offset)
	{
		// Machine Check Exception (TODO)
	case 0x00:
		valid_msr = true;
		popmessage("RDMSR: Reading P5_MC_ADDR");
		return 0;
	case 0x01:
		valid_msr = true;
		popmessage("RDMSR: Reading P5_MC_TYPE");
		return 0;
		// Time Stamp Counter
	case 0x10:
		valid_msr = true;
		popmessage("RDMSR: Reading TSC");
		return m_tsc;
		// Event Counters (TODO)
	case 0x11:  // CESR
		valid_msr = true;
		popmessage("RDMSR: Reading CESR");
		return 0;
	case 0x12:  // CTR0
		valid_msr = true;
		return m_perfctr[0];
	case 0x13:  // CTR1
		valid_msr = true;
		return m_perfctr[1];
	default:
		if (!(offset & ~0xf)) // 2-f are test registers
		{
			valid_msr = true;
			logerror("RDMSR: Reading test MSR %x", offset);
			return 0;
		}
		logerror("RDMSR: invalid P5 MSR read %08x at %08x\n", offset, m_pc - 2);
		valid_msr = false;
		return 0;
	}
	return -1;
}

void pentium_device::opcode_wrmsr(uint64_t data, bool &valid_msr)
{
	uint32_t offset = REG32(ECX);

	switch (offset)
	{
		// Machine Check Exception (TODO)
	case 0x00:
		popmessage("WRMSR: Writing P5_MC_ADDR");
		valid_msr = true;
		break;
	case 0x01:
		popmessage("WRMSR: Writing P5_MC_TYPE");
		valid_msr = true;
		break;
		// Time Stamp Counter
	case 0x10:
		m_tsc = data;
		popmessage("WRMSR: Writing to TSC");
		valid_msr = true;
		break;
		// Event Counters (TODO)
	case 0x11:  // CESR
		popmessage("WRMSR: Writing to CESR");
		valid_msr = true;
		break;
	case 0x12:  // CTR0
		m_perfctr[0] = data;
		valid_msr = true;
		break;
	case 0x13:  // CTR1
		m_perfctr[1] = data;
		valid_msr = true;
		break;
	default:
		if (!(offset & ~0xf)) // 2-f are test registers
		{
			valid_msr = true;
			logerror("WRMSR: Writing test MSR %x", offset);
			break;
		}
		logerror("WRMSR: invalid MSR write %08x (%08x%08x) at %08x\n", offset, (uint32_t)(data >> 32), (uint32_t)data, m_pc - 2);
		valid_msr = false;
		break;
	}
}

uint64_t pentium_pro_device::opcode_rdmsr(bool &valid_msr)
{
	uint32_t offset = REG32(ECX);

	switch (offset)
	{
		// Machine Check Exception (TODO)
	case 0x00:
		valid_msr = true;
		popmessage("RDMSR: Reading P5_MC_ADDR");
		return 0;
	case 0x01:
		valid_msr = true;
		popmessage("RDMSR: Reading P5_MC_TYPE");
		return 0;
		// Time Stamp Counter
	case 0x10:
		valid_msr = true;
		popmessage("RDMSR: Reading TSC");
		return m_tsc;
		// Performance Counters (TODO)
	case 0xc1:  // PerfCtr0
		valid_msr = true;
		return m_perfctr[0];
	case 0xc2:  // PerfCtr1
		valid_msr = true;
		return m_perfctr[1];
	default:
		logerror("RDMSR: unimplemented register called %08x at %08x\n", offset, m_pc - 2);
		valid_msr = true;
		return 0;
	}
	return -1;
}

void pentium_pro_device::opcode_wrmsr(uint64_t data, bool &valid_msr)
{
	uint32_t offset = REG32(ECX);

	switch (offset)
	{
		// Time Stamp Counter
	case 0x10:
		m_tsc = data;
		popmessage("WRMSR: Writing to TSC");
		valid_msr = true;
		break;
		// Performance Counters (TODO)
	case 0xc1:  // PerfCtr0
		m_perfctr[0] = data;
		valid_msr = true;
		break;
	case 0xc2:  // PerfCtr1
		m_perfctr[1] = data;
		valid_msr = true;
		break;
	default:
		logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n", offset, (uint32_t)(data >> 32), (uint32_t)data, m_pc - 2);
		valid_msr = true;
		break;
	}
}

uint64_t pentium4_device::opcode_rdmsr(bool &valid_msr)
{
	switch (REG32(ECX))
	{
	default:
		logerror("RDMSR: unimplemented register called %08x at %08x\n", REG32(ECX), m_pc - 2);
		valid_msr = true;
		return 0;
	}
	return -1;
}

void pentium4_device::opcode_wrmsr(uint64_t data, bool &valid_msr)
{
	switch (REG32(ECX))
	{
	default:
		logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n", REG32(ECX), (uint32_t)(data >> 32), (uint32_t)data, m_pc - 2);
		valid_msr = true;
		break;
	}
}
