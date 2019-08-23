// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett

#include "emu.h"
#include "athlon.h"
#include "i386priv.h"

DEFINE_DEVICE_TYPE(ATHLONXP,    athlonxp_device,    "athlonxp",    "Amd Athlon XP")

athlonxp_device::athlonxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_device(mconfig, ATHLONXP, tag, owner, clock)
	, m_data_config("mmio", ENDIANNESS_LITTLE, 32, 32, 0, 32, 12)
	, m_opcodes_config("debugger", ENDIANNESS_LITTLE, 32, 32, 0, 32, 12)
{
	// TODO: put correct value
	set_vtlb_dynamic_entries(256);
}

/*****************************************************************************/
/* AMD Athlon XP
   Model: Athlon XP 2400+
   Part number: AXDA2400DKV3C
   Stepping code: AIUCP
   Date code: 0240MPMW
*/

void athlonxp_device::device_start()
{
	i386_common_init();
	register_state_i386_x87_xmm();
	m_data = &space(AS_DATA);
	m_opcodes = &space(AS_OPCODES);
	mmacache32 = m_data->cache<2, 0, ENDIANNESS_LITTLE>();
	m_opcodes->install_read_handler(0, 0xffffffff, read32_delegate(FUNC(athlonxp_device::debug_read_memory), this));

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables

	// put savestate calls here
	save_item(NAME(m_processor_name_string));
	save_item(NAME(m_msr_top_mem));
	save_item(NAME(m_msr_sys_cfg));
	save_item(NAME(m_msr_smm_base));
	save_item(NAME(m_msr_smm_mask));
	save_item(NAME(m_msr_mtrrfix));
	save_item(NAME(m_memory_ranges_1m));
}

void athlonxp_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base = 0xffff0000;
	m_sreg[CS].limit = 0xffff;
	m_sreg[CS].flags = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 8, Stepping 1
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (8 << 4) | (1);

	m_cpuid_id0 = ('h' << 24) | ('t' << 16) | ('u' << 8) | 'A';   // Auth
	m_cpuid_id1 = ('i' << 24) | ('t' << 16) | ('n' << 8) | 'e';   // enti
	m_cpuid_id2 = ('D' << 24) | ('M' << 16) | ('A' << 8) | 'c';   // cAMD
	memset(m_processor_name_string, 0, 48);
	strcpy((char *)m_processor_name_string, "AMD Athlon(tm) Processor");
	for (int n = 0; n < 11; n++)
		m_msr_mtrrfix[n] = 0;
	for (int n = 0; n < (1024 / 4); n++)
		m_memory_ranges_1m[n] = 0;
	m_msr_top_mem = 1024 * 1024;
	m_msr_sys_cfg = 0;
	m_msr_smm_base = m_smbase;
	m_msr_smm_mask = 0;

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// see FEATURE_FLAGS enum for bit names
	m_feature_flags = 0x0383fbff;

	CHANGE_PC(m_eip);
}

device_memory_interface::space_config_vector athlonxp_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_OPCODES, &m_opcodes_config)
	};
}

void athlonxp_device::enter_smm()
{
	u64 data;

	if (m_msr_smm_mask & 1)
		data = 0x1818181818181818; // when smm is active
	else
		data = m_msr_mtrrfix[2];
	parse_mtrrfix(data, 0xa0000, 16);
	i386_device::enter_smm();
}

void athlonxp_device::leave_smm()
{
	u64 data;

	i386_device::leave_smm();
	if (m_msr_smm_mask & 1)
		data = 0; // when smm is not active
	else
		data = m_msr_mtrrfix[2];
	parse_mtrrfix(data, 0xa0000, 16);
}

void athlonxp_device::parse_mtrrfix(u64 mtrr, offs_t base, int kblock)
{
	int nb = kblock / 4;
	int range = (int)(base >> 12); // base must never be higher than 1 megabyte

	for (int n = 0; n < 8; n++)
	{
		uint8_t type = mtrr & 0xff;

		for (int b = 0; b < nb; b++)
		{
			m_memory_ranges_1m[range] = type;
			range++;
		}
		mtrr = mtrr >> 8;
	}
}

int athlonxp_device::check_cacheable(offs_t address)
{
	offs_t block;
	int disabled;

	disabled = 0;
	if (m_cr[0] & (1 << 30))
		disabled = 128;
	if (address >= 0x100000)
		return disabled;
	block = address >> 12;
	return m_memory_ranges_1m[block] | disabled;
}

template <int wr>
int athlonxp_device::address_mode(offs_t address)
{
	if (address >= m_msr_top_mem)
		return 1;
	if (address >= 1 * 1024 * 1024)
		return 0;
	if ((m_memory_ranges_1m[address >> 12] & (1 << (3 + wr))) != 0)
		return 0;
	return 1;
}

READ32_MEMBER(athlonxp_device::debug_read_memory)
{
	offs_t address = offset << 2;
	int mode = check_cacheable(address);
	bool nocache = false;
	address_space *m = m_program;
	u8 *data;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = (address & 63);
		data = cache.search<CacheRead>(address);
		if (data)
			return *(u32 *)(data + offset);
	}
	if (address_mode<1>(address))
		m = m_data;
	return m->read_dword(address);
}

template <class dt, offs_t xorle>
dt athlonxp_device::opcode_read_cache(offs_t address)
{
	int mode = check_cacheable(address);
	bool nocache = false;
	memory_access_cache<2, 0, ENDIANNESS_LITTLE> *m = macache32;
	u8 *data;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = (address & 63) ^ xorle;
		data = cache.search<CacheRead>(address);
		if (data)
			return *(dt *)(data + offset);
		if (!(mode & 128))
		{
			bool dirty = cache.allocate<CacheRead>(address, &data);
			address = cache.base(address);
			if (dirty)
			{
				offs_t old_address = cache.old();

				for (int w = 0; w < 64; w += 4)
					m->write_dword(old_address + w, *(u32 *)(data + w));
			}
			for (int r = 0; r < 64; r += 4)
				*(u32 *)(data + r) = m->read_dword(address + r);
			return *(dt *)(data + offset);
		}
	}
	if (address_mode<1>(address))
		m = mmacache32;
	if (sizeof(dt) == 1)
		return m->read_byte(address);
	else if (sizeof(dt) == 2)
		return m->read_word(address);
	else
		return m->read_dword(address);
}

uint32_t athlonxp_device::program_read_cache(offs_t address, uint32_t mask)
{
	int mode = check_cacheable(address);
	bool nocache = false;
	address_space *m = m_program;
	u8 *data;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = address & 63;
		data = cache.search<CacheRead>(address);
		if (data)
			return *(u32 *)(data + offset) & mask;
		if (!(mode & 128))
		{
			bool dirty = cache.allocate<CacheRead>(address, &data);
			address = cache.base(address);
			if (dirty)
			{
				offs_t old_address = cache.old();

				for (int w = 0; w < 64; w += 4)
					m->write_dword(old_address + w, *(u32 *)(data + w));
			}
			for (int r = 0; r < 64; r += 4)
				*(u32 *)(data + r) = m->read_dword(address + r);
			return *(u32 *)(data + offset) & mask;
		}
	}
	if (address_mode<1>(address))
		m = m_data;
	return m->read_dword(address, mask) & mask;
}

void athlonxp_device::program_write_cache(offs_t address, uint32_t data, uint32_t mask)
{
	int mode = check_cacheable(address);
	bool nocache = false;
	address_space *m = m_program;
	u8 *dataw;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = address & 63;
		dataw = cache.search<CacheWrite>(address);
		if (dataw)
		{
			*(u32 *)(dataw + offset) = (*(u32 *)(dataw + offset) & ~mask) | (data & mask);
			return;
		}
		if (!(mode & 128))
		{
			bool dirty = cache.allocate<CacheWrite>(address, &dataw);
			address = cache.base(address);
			if (dirty)
			{
				offs_t old_address = cache.old();

				for (int w = 0; w < 64; w += 4)
					m->write_dword(old_address + w, *(u32 *)(dataw + w));
			}
			for (int r = 0; r < 64; r += 4)
				*(u32 *)(dataw + r) = m->read_dword(address + r);
			*(u32 *)(dataw + offset) = (*(u32 *)(dataw + offset) & ~mask) | (data & mask);
			return;
		}
	}
	if (address_mode<0>(address))
		m = m_data;
	m->write_dword(address, data, mask);
}

void athlonxp_device::cache_writeback()
{
	// dirty cachelines are written back to memory
	address_space *m = m_program;
	u32 base;
	u8 *data;

	data = cache.first_dirty(base, false);
	while (data != nullptr)
	{
		for (int w = 0; w < 64; w += 4)
			m->write_dword(base + w, *(u32 *)(data + w));
		data = cache.next_dirty(base, false);
	}
}

void athlonxp_device::cache_invalidate()
{
	// dirty cachelines are not written back to memory
	cache.reset();
}

void athlonxp_device::cache_clean()
{
	// dirty cachelines are marked as clean but not written back to memory
	u32 base;
	u8 *data;

	data = cache.first_dirty(base, true);
	while (data != nullptr)
		data = cache.next_dirty(base, true);
}

uint8_t athlonxp_device::READ8PL(uint32_t ea, uint8_t privilege)
{
	uint32_t address = ea, error;

	if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
		PF_THROW(error);

	address &= m_a20_mask;

	uint8_t shift = 8 * (ea & 3);
	return program_read_cache(address - (ea & 3), uint32_t(0xff) << shift) >> shift;
}

uint16_t athlonxp_device::READ16PL(uint32_t ea, uint8_t privilege)
{
	uint16_t value;
	uint32_t address = ea, error;

	switch (ea & 3)
	{
	case 0:
	default:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = program_read_cache(address, 0x0000ffff) & 0xffff;
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = (program_read_cache(address - 1, 0x00ffff00) >> 8) & 0xffff;
		break;

	case 2:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = (program_read_cache(address - 2, 0xffff0000) >> 16) & 0xffff;
		break;

	case 3:
		value = READ8PL(ea, privilege);
		value |= READ8PL(ea + 1, privilege) << 8;
		break;
	}

	return value;
}

uint32_t athlonxp_device::READ32PL(uint32_t ea, uint8_t privilege)
{
	uint32_t value;
	uint32_t address = ea, error;

	switch (ea & 3)
	{
	case 0:
	default:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = program_read_cache(address, 0xffffffff);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = program_read_cache(address - 1, 0xffffff00) >> 8;
		value |= READ8PL(ea + 3, privilege) << 24;
		break;

	case 2:
		value = READ16PL(ea, privilege);
		value |= READ16PL(ea + 2, privilege) << 16;
		break;

	case 3:
		value = READ8PL(ea, privilege);

		address = ea + 1;
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value |= program_read_cache(address, 0x00ffffff) << 8;
		break;
	}

	return value;
}

uint64_t athlonxp_device::READ64PL(uint32_t ea, uint8_t privilege)
{
	uint64_t value;
	uint32_t address = ea, error;

	switch (ea & 3)
	{
	case 0:
	default:
		value = READ32PL(ea, privilege);
		value |= uint64_t(READ32PL(ea + 2, privilege)) << 32;
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = program_read_cache(address - 1, 0xffffff00) >> 8;
		value |= uint64_t(READ32PL(ea + 3, privilege)) << 24;
		value |= uint64_t(READ8PL(ea + 7, privilege)) << 56;
		break;

	case 2:
		value = READ16PL(ea, privilege);
		value |= uint64_t(READ32PL(ea + 2, privilege)) << 16;
		value |= uint64_t(READ16PL(ea + 6, privilege)) << 48;
		break;

	case 3:
		value = READ8PL(ea, privilege);
		value |= uint64_t(READ32PL(ea + 1, privilege)) << 8;

		address = ea + 5;
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value |= uint64_t(program_read_cache(address, 0x00ffffff)) << 40;
		break;
	}

	return value;
}

void athlonxp_device::WRITE8PL(uint32_t ea, uint8_t privilege, uint8_t value)
{
	uint32_t address = ea, error;
	if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);

	address &= m_a20_mask;

	uint8_t shift = 8 * (ea & 3);
	program_write_cache(address - (ea & 3), value << shift, uint32_t(0xff) << shift);
}

void athlonxp_device::WRITE16PL(uint32_t ea, uint8_t privilege, uint16_t value)
{
	uint32_t address = ea, error;

	switch(ea & 3)
	{
	case 0:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address, value, 0x0000ffff);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address - 1, value << 8, 0x00ffff00);
		break;

	case 2:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address - 2, value << 16, 0xffff0000);
		break;

	case 3:
		WRITE8PL(ea, privilege, value & 0xff);
		WRITE8PL(ea + 1, privilege, (value >> 8) & 0xff);
		break;
	}
}

void athlonxp_device::WRITE32PL(uint32_t ea, uint8_t privilege, uint32_t value)
{
	uint32_t address = ea, error;

	switch(ea & 3)
	{
	case 0:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address, value, 0xffffffff);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address - 1, (value << 8) & 0xffffff00, 0xffffff00);
		WRITE8PL(ea + 3, privilege, (value >> 24) & 0xff);
		break;

	case 2:
		WRITE16PL(ea, privilege, value & 0xffff);
		WRITE16PL(ea + 2, privilege, (value >> 16) & 0xffff);
		break;

	case 3:
		WRITE8PL(ea, privilege, value & 0xff);

		address = ea + 1;
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address, value >> 8, 0x00ffffff);
		break;
	}
}

void athlonxp_device::WRITE64PL(uint32_t ea, uint8_t privilege, uint64_t value)
{
	uint32_t address = ea, error;

	switch(ea & 3)
	{
	case 0:
		WRITE32PL(ea, privilege, value & 0xffffffff);
		WRITE32PL(ea + 2, privilege, (value >> 32) & 0xffffffff);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address - 1, value << 8, 0xffffff00);
		WRITE32PL(ea + 3, privilege, (value >> 24) & 0xffffffff);
		WRITE8PL(ea + 7, privilege, (value >> 56) & 0xff );
		break;

	case 2:
		WRITE16PL(ea, privilege, value & 0xffff);
		WRITE32PL(ea + 2, privilege, (value >> 16) & 0xffffffff);
		WRITE16PL(ea + 6, privilege, (value >> 48) & 0xffff);
		break;

	case 3:
		WRITE8PL(ea, privilege, value & 0xff);
		WRITE32PL(ea + 1, privilege, (value >> 8) & 0xffffffff);

		address = ea + 5;
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		program_write_cache(address, (value >> 40) & 0x00ffffff, 0x00ffffff);
		break;
	}
}

/**********************************************************************************/

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
			REG32(EDX) = 1; //  Advanced power management information, temperature sensor present
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
		case 0xC0010111: // SMM_BASE
			// address of system management mode area
			ret = (uint64_t)m_msr_smm_base;
			break;
		case 0xC0010113: // SMM_MASK
			// 1     TValid  - Enable TSeg SMRAM Range
			// 0     AValid  - Enable ASeg SMRAM Range
			/* Access to the ASeg (a0000-bffff) depends on bit 0 of smm_mask
			   if the bit is 0 use the associated fixed mtrr
			   if the bit is 1
			       if smm is active
			           access goes to dram (wrmem 1 rdmem 1)
			       if smm not active
			           access goes to mmio (wrmem 0 rdmem 0) */
			ret = m_msr_smm_mask;
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
			if (m_msr_smm_mask & 1)
			{
				if (m_smm)
					data = 0x1818181818181818; // when smm is active
				else
					data = 0; // when smm is not active
			}
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
		case 0xC0010111: // SMM_BASE
			m_msr_smm_base = (offs_t)data;
			m_smbase = m_msr_smm_base;
			break;
		case 0xC0010113: // SMM_MASK
			m_msr_smm_mask = data;
			if (m_msr_smm_mask & 1)
			{
				if (m_smm)
					data = 0x1818181818181818; // when smm is active
				else
					data = 0; // when smm is not active
			}
			else
				data = m_msr_mtrrfix[2];
			parse_mtrrfix(data, 0xa0000, 16);
			break;
	}
	valid_msr = true;
}
