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

void athlonxp_device::opcode_cpuid()
{
	switch (REG32(EAX))
	{
		case 0x80000000:
		{
			REG32(EAX) = 0x80000008;
			REG32(EBX) = m_cpuid_id0;
			REG32(ECX) = m_cpuid_id2;
			REG32(EDX) = m_cpuid_id1;
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 0x80000001:
		{
			REG32(EAX) = m_cpu_version + 0x100; // family+1 as specified in AMD documentation
			REG32(EDX) = m_feature_flags;
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 0x80000002:
		case 0x80000003:
		case 0x80000004:
		{
			int offset = (REG32(EAX) - 0x80000002) << 4;
			uint8_t *b = m_processor_name_string + offset;
			REG32(EAX) = b[ 0] + (b[ 1] << 8) + (b[ 2] << 16) + (b[ 3] << 24);
			REG32(EBX) = b[ 4] + (b[ 5] << 8) + (b[ 6] << 16) + (b[ 7] << 24);
			REG32(ECX) = b[ 8] + (b[ 9] << 8) + (b[10] << 16) + (b[11] << 24);
			REG32(EDX) = b[12] + (b[13] << 8) + (b[14] << 16) + (b[15] << 24);
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 0x80000005:
		{
			REG32(EAX) = 0x0408FF08; // 2M/4M data tlb associativity 04 data tlb number of entries 08 instruction tlb associativity FF instruction tlb number of entries 08
			REG32(EBX) = 0xFF20FF10; // 4K data tlb associativity FF data tlb number of entries 20 instruction tlb associativity FF instruction tlb number of entries 10
			REG32(ECX) = 0x40020140; // L1 data cache size in K 40 associativity 02 lines per tag 01 line size in bytes 40
			REG32(EDX) = 0x40020140; // L1 instruction cache size in K 40 associativity 02 lines per tag 01 line size in bytes 40
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 0x80000006:
		{
			REG32(EAX) = 0;
			REG32(EBX) = 0x41004100; // 4 100 4 100
			REG32(ECX) = 0x01008140; // L2 cache size in K 0100 associativity 8=16-way lines per tag 1 line size in bytes 40
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 0x80000007:
		{
			REG32(EDX) = 1;
			CYCLES(CYCLES_CPUID);
			break;
		}

		case 0x80000008:
		{
			REG32(EAX) = 0x00002022;
			CYCLES(CYCLES_CPUID);
			break;
		}

		default:
			i386_device::opcode_cpuid();
	}
}

uint64_t athlonxp_device::opcode_rdmsr(bool &valid_msr)
{
	uint64_t ret;
	uint32_t offset = REG32(ECX);

	ret = 0;
	switch (offset)
	{
		case 0x10: // TSC
			break;
		case 0x1b: // APIC_BASE
			break;
		case 0xfe: // MTRRcap
			// 7-0   MTRRCapVCnt - Number of variable range MTRRs (8)
			// 8     MtrrCapFix  - Fixed range MTRRs available (1)
			// 10    MtrrCapWc   - Write combining memory type available (1)
			ret = 0x508;
			break;
		case 0x17b: // MCG_CTL
			break;
		case 0x200: // MTRRphysBase0-7
		case 0x202:
		case 0x204:
		case 0x206:
		case 0x208:
		case 0x20a:
		case 0x20c:
		case 0x20e:
			// 7-0   Type        - Memory type for this memory range
			// 39-12 PhyBase27-0 - Base address for this memory range
			/* Format of type field:
			    Bits 2-0 specify the memory type with the following encoding
			     0 UC Uncacheable
			     1 WC Write Combining
			     4 WT Write Through
			     5 WP Write Protect
			     6 WB Write Back
			     7 UC Uncacheable used only in PAT register
			    Bit 3 WrMem 1 write to memory 0 write to mmio, present only in fixed range MTRRs
			    Bit 4 RdMem 1 read from memory 0 read from mmio, present only in fixed range MTRRs
			    Other bits are unused
			*/
			break;
		case 0x201: // MTRRphysMask0-7
		case 0x203:
		case 0x205:
		case 0x207:
		case 0x209:
		case 0x20b:
		case 0x20d:
		case 0x20f:
			// 11    Valid       - Memory range active
			// 39-12 PhyMask27-0 - Address mask
			break;
		case 0x2ff: // MTRRdefType
			// 7-0   MtrrDefMemType   - Default memory type
			// 10    MtrrDefTypeFixEn - Enable fixed range MTRRs
			// 11    MtrrDefTypeEn    - Enable MTRRs
			break;
		case 0x250: // MTRRfix64K_00000
			// 8 bits for each 64k block starting at address 0
			ret = m_msr_mtrrfix[0];
			break;
		case 0x258: // MTRRfix16K_80000
			// 8 bits for each 16k block starting at address 0x80000
			ret = m_msr_mtrrfix[1];
			break;
		case 0x259: // MTRRfix16K_A0000
			// 8 bits for each 16k block starting at address 0xa0000
			ret = m_msr_mtrrfix[2];
			break;
		case 0x268: // MTRRfix4K_C0000
		case 0x269: // MTRRfix4K_C8000
		case 0x26a: // MTRRfix4K_D0000
		case 0x26b: // MTRRfix4K_D8000
		case 0x26c: // MTRRfix4K_E0000
		case 0x26d: // MTRRfix4K_E8000
		case 0x26e: // MTRRfix4K_F0000
		case 0x26f: // MTRRfix4K_F8000
			// 8 bits for each 4k block
			ret = m_msr_mtrrfix[3 + offset - 0x268];
			break;
		case 0x400: // MC0_CTL
			break;
		case 0x404: // MC1_CTL
			break;
		case 0x408: // MC2_CTL
			break;
		case 0x40c: // MC3_CTL
			break;
		case 0xC0010010: // SYS_CFG
			// 20    MtrrVarDramEn    - Enable top of memory address and I/O range registers
			// 19    MtrrFixDramModEn - Enable modification of RdDram and WrDram bits in fixed MTRRs
			// 18    MtrrFixDramEn    - Enable RdDram and WrDram attributes in fixed MTRRs
			ret = m_msr_sys_cfg;
			break;
		case 0xC0010015: // HWCR
			break;
		case 0xC0010016: // IORRBase0-1
		case 0xC0010018:
			// 39-12 Base27-0 - Base address for this memory range
			// 4     RdDram   - Read from DRAM
			// 3     WrDram   - Write to DRAM
			break;
		case 0xC0010017: // IORRMask0-1
		case 0xC0010019:
			// 39-12 Mask27-0 - Address mask
			// 11    V        - Register enabled
			break;
		case 0xC001001A: // TOP_MEM
			// 39-23 TOM16-0 - Top of Memory, accesses from this address onward are directed to mmio
			ret = (uint64_t)m_msr_top_mem;
			break;
		case 0xC001001D: // TOP_MEM2
			break;
		case 0xC0010113: // SMM_MASK
			break;
	}
	valid_msr = true;
	return ret;
}

void athlonxp_device::opcode_wrmsr(uint64_t data, bool &valid_msr)
{
	uint32_t offset = REG32(ECX);

	switch (offset)
	{
		case 0x1b: // APIC_BASE
			break;
		case 0x17b: // MCG_CTL
			break;
		case 0x200: // MTRRphysBase0-7
		case 0x201: // MTRRphysMask0-7
		case 0x202:
		case 0x203:
		case 0x204:
		case 0x205:
		case 0x206:
		case 0x207:
		case 0x208:
		case 0x209:
		case 0x20a:
		case 0x20b:
		case 0x20c:
		case 0x20d:
		case 0x20e:
		case 0x20f:
			break;
		case 0x2ff: // MTRRdefType
			break;
		case 0x250: // MTRRfix64K_00000
			m_msr_mtrrfix[0] = data;
			parse_mtrrfix(data, 0, 64);
			break;
		case 0x258: // MTRRfix16K_80000
			m_msr_mtrrfix[1] = data;
			parse_mtrrfix(data, 0x80000, 16);
			break;
		case 0x259: // MTRRfix16K_A0000
			m_msr_mtrrfix[2] = data;
			parse_mtrrfix(data, 0xa0000, 16);
			break;
		case 0x268: // MTRRfix4K_C0000-F8000
		case 0x269:
		case 0x26a:
		case 0x26b:
		case 0x26c:
		case 0x26d:
		case 0x26e:
		case 0x26f:
			m_msr_mtrrfix[3 + offset - 0x268] = data;
			parse_mtrrfix(data, 0xc0000 + (offset - 0x268) * 0x8000, 4);
			break;
		case 0x400: // MC0_CTL
			break;
		case 0x404: // MC1_CTL
			break;
		case 0x408: // MC2_CTL
			break;
		case 0x40c: // MC3_CTL
			break;
		case 0xC0010010: // SYS_CFG
			m_msr_sys_cfg = data;
			break;
		case 0xC0010015: // HWCR
			break;
		case 0xC0010016: // IORRBase
		case 0xC0010017: // IORRMask
		case 0xC0010018:
		case 0xC0010019:
			break;
		case 0xC001001A: // TOP_MEM
			m_msr_top_mem = (offs_t)data;
			break;
		case 0xC0010113: // SMM_MASK
			break;
	}
	valid_msr = true;
}
